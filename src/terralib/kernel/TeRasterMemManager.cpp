/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
#include "TeRasterMemManager.h"

#include "TeUtils.h"
#include "TeErrorLog.h"
#include "TeTempFilesRemover.h"
#include "TeAgnostic.h"


TeRasterMemManager::TeRasterMemManager()
{
  maxMemPercentUsage_ = 40;
  maxTempFileSize_ = 2ul * 1024ul * 1024ul * 1024ul;
  getTilePointer_tilePtr_ = 0;
  getTilePointer_reqTileIdx_ = 0;
  getTilePointer_swapTileIdx_ = 0;
  tilesPerBand_ = 0;
  bandsNmb_ = 0;
  nextSwapTileRamTilesIndexesVecIdx_ = 0;
  swapTilePtr_ = 0;
}


TeRasterMemManager::~TeRasterMemManager()
{
  clear();
}


void TeRasterMemManager::clear()
{
  getTilePointer_tilePtr_ = 0;
  
  getTilePointer_reqTileIdx_ = 0;
  
  getTilePointer_swapTileIdx_ = 0;

  tilesPerBand_ = 0;
  
  bandsNmb_ = 0;
  
  bandsTileSizes_.clear();
  
  for( unsigned int allTilesPtrsVecIdx = 0 ; allTilesPtrsVecIdx < 
    allTilesPtrsVec_.size() ; ++allTilesPtrsVecIdx )
  {
    if( allTilesPtrsVec_[ allTilesPtrsVecIdx ] )
      delete[] ( allTilesPtrsVec_[ allTilesPtrsVecIdx ] );
  }  
  allTilesPtrsVec_.clear();
  
  for( unsigned int openDiskFilesVecIdx = 0 ; openDiskFilesVecIdx < 
    openDiskFilesVec_.size() ; ++openDiskFilesVecIdx )
  {
    TeTempFilesRemover::instance().removeFile( openDiskFilesVec_[ 
      openDiskFilesVecIdx ].second );      
  } 
  openDiskFilesVec_.clear();
  
  diskTilesDataVec_.clear();
  
  ramTilesIndexesVec_.clear();
  
  nextSwapTileRamTilesIndexesVecIdx_ = 0;
  
  if( swapTilePtr_ ) delete[]( swapTilePtr_ );
  swapTilePtr_ = 0;
}

bool TeRasterMemManager::reset( unsigned int bands, unsigned int tilesPerBand,
  const std::vector< unsigned int >& tilesSizesVec,
  MemoryPolicy memoryPolicy, unsigned char maxMemPercentUsage,
  unsigned long int maxTempFileSize )
{
  clear();
  
  TEAGN_TRUE_OR_THROW( maxMemPercentUsage < 101, "Invalid maxMemPercentUsage value" );
  maxMemPercentUsage_ = maxMemPercentUsage;  
  
  maxTempFileSize_ = maxTempFileSize;  
  
  if( ( bands > 0 ) && ( tilesPerBand > 0 ) )
  {
    tilesPerBand_ = tilesPerBand;
    bandsNmb_ = bands;
    bandsTileSizes_ = tilesSizesVec;
    
    const unsigned int totalTilesNumber = tilesPerBand * bandsNmb_;
    
    // Guessing the max tile size
    
    unsigned int maxTileSize = 0;
    for( unsigned int currBand = 0 ; currBand < bandsNmb_ ; ++currBand ) 
    {    
      maxTileSize = MAX( bandsTileSizes_[ currBand ], maxTileSize );
    }    
    
    /* Allocating */
    
    try
    {
      /* Allocating global objects */ 
         
      allTilesPtrsVec_.resize( totalTilesNumber, 0 );
      
      // Allocating ram tiles
    
      if( ( memoryPolicy == AutoMemPol ) || ( memoryPolicy == DiskMemPol) )
      {
        // Allocating the swap tile
        
        swapTilePtr_ = (TilePtrT) new TileDataT[ maxTileSize ];
        if( swapTilePtr_ == 0 ) 
        {
          clear();
          
          return false;        
        }
      
        // Defining the number of RAM tiles
        
        unsigned int maxRAMTiles = 1;
        
        if( memoryPolicy == AutoMemPol )
        {
          // Defining the max number of RAM tiles
          
          const double totalPhysMem = (double)TeGetTotalPhysicalMemory();
          
          const double usedVMem = (double)TeGetUsedVirtualMemory();
          
          const double totalVMem = (double)TeGetTotalVirtualMemory();
          
          const double freeVMem = MIN( totalPhysMem, 
            ( totalVMem - usedVMem ) );
          
          const double tilesMem = ( ((double)maxMemPercentUsage_) / 100.0 ) *
            freeVMem;
            
          maxRAMTiles = (unsigned int)MAX( 1, tilesMem / ((double)maxTileSize) );
        }
        
        // Allocating RAM tiles and updating ramTilesIndexesVec_
        
        const unsigned int ramTilesNmb = MIN( maxRAMTiles, totalTilesNumber );
        
        ramTilesIndexesVec_.resize( ramTilesNmb, 0 );
        nextSwapTileRamTilesIndexesVecIdx_ = 0;
        
        TilePtrT newTilePtr = 0;
        
        for( unsigned int tileIdx = 0 ; tileIdx < ramTilesNmb ; ++tileIdx )
        {
          newTilePtr = (TilePtrT) new TileDataT[ maxTileSize ];
          
          if( newTilePtr )
          {
            allTilesPtrsVec_[ tileIdx ] = newTilePtr;
          }
          else
          {
            clear();
            
            return false;
          }
          
          ramTilesIndexesVec_[ tileIdx ] = tileIdx;
        }
          
        // Allocating the disk files
        
        DiskTileDataVecT freeDiskTilesData;
        const unsigned int diskTilesNumber = totalTilesNumber - 
          ramTilesNmb;
      
        if( ! allocateDiskFiles( maxTileSize, diskTilesNumber, 
          openDiskFilesVec_, freeDiskTilesData ) )
        {
          clear();
          
          return false;
        }
                    
        // Updating the disk tiles data vector
        
        diskTilesDataVec_.resize( totalTilesNumber );
        
        TEAGN_DEBUG_CONDITION( freeDiskTilesData.size() == diskTilesNumber, 
          "Internal error" );
          
        unsigned int tilesSizesVecIdx = 0;
        unsigned int freeDiskTilesDataIdx = 0;
          
        for( unsigned int diskTilesDataVecIdx = 0 ; 
          diskTilesDataVecIdx < totalTilesNumber ; ++diskTilesDataVecIdx )
        {
          if( diskTilesDataVecIdx >= ramTilesNmb )
          {
            freeDiskTilesDataIdx = diskTilesDataVecIdx - ramTilesNmb;
 
            diskTilesDataVec_[ diskTilesDataVecIdx ] = freeDiskTilesData[ 
              freeDiskTilesDataIdx ];
          }
          
          tilesSizesVecIdx = diskTilesDataVecIdx / tilesPerBand;
          TEAGN_DEBUG_CONDITION( tilesSizesVecIdx < tilesSizesVec.size(),
            "Internal error" );
          
          diskTilesDataVec_[ diskTilesDataVecIdx ].size_ = tilesSizesVec[
            tilesSizesVecIdx ];        
        }
      }
      else if( memoryPolicy == RAMMemPol )
      {
        TilePtrT newTilePtr = 0;
        unsigned int allTilesPtrsVecIdx = 0;
      
        for( unsigned int currBand = 0 ; currBand < bandsNmb_ ; ++currBand )
        {
          unsigned int tileBytes = bandsTileSizes_[ currBand ];        
          
          for( unsigned int tileIdx = 0 ; tileIdx < tilesPerBand_ ;
            ++tileIdx ) 
          {
            newTilePtr = (TilePtrT) new TileDataT[ tileBytes ];
              
            if( newTilePtr == 0 ) 
            {
              clear();
              
              return false;
            } 
            else 
            {
              allTilesPtrsVec_[ allTilesPtrsVecIdx ] = newTilePtr;
              ++allTilesPtrsVecIdx;
            }              
          }          
        }
      }
      else
      {
        TEAGN_LOG_AND_THROW( "Invalid memory policy" );
      }     
    }
    catch(...)
    {
      clear();
      
      TEAGN_LOG_AND_RETURN( "Tiles allocation error" )
    } 
    
  }

  return true;
}

bool TeRasterMemManager::allocateDiskFiles( 
  unsigned int tileSize,
  unsigned int tilesNmb, OpenDiskFilesVecT& openDiskFilesVec, 
  DiskTileDataVecT& diskTilesData )
{
  openDiskFilesVec.clear();
  diskTilesData.clear();
  
  if( ( tileSize > 0 ) && ( tilesNmb > 0 ) )
  {
    const unsigned long int maxTilesPerFile = ( unsigned long int )
      floor( ( (double)maxTempFileSize_ ) / ( (double) tileSize ) );
        
    const unsigned long int maxFileSize = (unsigned long int)
      ( maxTilesPerFile * tileSize );
      
    const unsigned int filesNumber = (unsigned int)ceil( ((double)tilesNmb) /
      ((double)maxTilesPerFile) );
      
    for( unsigned int fileIdx = 0 ; fileIdx < filesNumber ; ++fileIdx )
    {
      // Defining the current file size
      
      unsigned int remainingTilesNmb = tilesNmb - 
        (unsigned int)diskTilesData.size();
      
      unsigned int fileSize = maxFileSize;
      unsigned int fileTilesNumber = maxTilesPerFile;
      
      if( remainingTilesNmb < maxTilesPerFile )
      {
        fileSize = (unsigned long int)( tileSize * remainingTilesNmb );
        fileTilesNumber = remainingTilesNmb;
      }
      
      // allocating the file 
      
      std::pair< FILE*, std::string > newFileData;
      
      if( ! createNewDiskFile( fileSize, newFileData.second, 
        &( newFileData.first) ) )
      {
        TEAGN_LOGERR( "Unable to create temporary disk file" );
        
        return false;           
      }
      else
      {
        openDiskFilesVec.push_back( newFileData );
        
        TeTempFilesRemover::instance().addFile( newFileData.second,
          newFileData.first );    
              
        DiskTileData auxTileData;
        auxTileData.filePtr_ = newFileData.first;
        
        for( unsigned int tileIdx = 0; tileIdx < fileTilesNumber ; ++tileIdx )
        {
          auxTileData.fileOff_ = tileIdx * tileSize;
          
          diskTilesData.push_back( auxTileData );
        }
      }
    }
  }
  
  return true;
}

void* TeRasterMemManager::getTilePointer( const unsigned int& band, 
  const unsigned int& tile)
{
  getTilePointer_reqTileIdx_ = ( band * tilesPerBand_ ) + tile;
  
  TEAGN_DEBUG_CONDITION( ( getTilePointer_reqTileIdx_ < 
    allTilesPtrsVec_.size() ), "Internal error" );
  getTilePointer_tilePtr_ = allTilesPtrsVec_[ getTilePointer_reqTileIdx_ ];
  
  if( getTilePointer_tilePtr_ == 0 ) 
  {
    TEAGN_DEBUG_CONDITION( nextSwapTileRamTilesIndexesVecIdx_ <
      ramTilesIndexesVec_.size(), "Internal error" );
    getTilePointer_swapTileIdx_ = ramTilesIndexesVec_[ 
      nextSwapTileRamTilesIndexesVecIdx_ ];
    
    TEAGN_DEBUG_CONDITION( getTilePointer_reqTileIdx_ < 
      diskTilesDataVec_.size(), "Internal error" )
    DiskTileData& inTileData = diskTilesDataVec_[ getTilePointer_reqTileIdx_ ];
    
    TEAGN_DEBUG_CONDITION( getTilePointer_swapTileIdx_ < 
      diskTilesDataVec_.size(), "Internal error" )
    DiskTileData& outTileData = diskTilesDataVec_[ 
      getTilePointer_swapTileIdx_ ];
      
    /* Reading the required tile into RAM (swapTilePtr_) */
    
    TEAGN_DEBUG_CONDITION( inTileData.filePtr_, "Internal error" );
    TEAGN_TRUE_OR_THROW( 0 == fseek( inTileData.filePtr_, 
      (long)( inTileData.fileOff_ ), SEEK_SET ),
      "File seek error" )
      
    TEAGN_DEBUG_CONDITION( swapTilePtr_, "Internal error" );
    TEAGN_TRUE_OR_THROW( 1 == fread( (void*)swapTilePtr_, 
      (size_t)( inTileData.size_ ), 1, inTileData.filePtr_ ),
      "File read error" )
  
    /* Flushing the swap tile to disk */
      
    TEAGN_TRUE_OR_THROW( 0 == fseek( inTileData.filePtr_, 
      (long)( inTileData.fileOff_ ), SEEK_SET ),
      "File seek error" );
      
    TEAGN_DEBUG_CONDITION( getTilePointer_swapTileIdx_ < 
      allTilesPtrsVec_.size(), "Internal error" );          
    TEAGN_TRUE_OR_THROW( 1 == fwrite( (void*)allTilesPtrsVec_[ 
      getTilePointer_swapTileIdx_ ], 
      (size_t)( outTileData.size_ ), 1, inTileData.filePtr_ ),
      "File write error" )        
    
    // Updating the tile pointers
    
    getTilePointer_tilePtr_ = allTilesPtrsVec_[ getTilePointer_swapTileIdx_ ];
    
    allTilesPtrsVec_[ getTilePointer_swapTileIdx_ ] = 0;
    
    allTilesPtrsVec_[ getTilePointer_reqTileIdx_ ] = swapTilePtr_;
    
    swapTilePtr_ = getTilePointer_tilePtr_;
    
    /* Updating the info vectors */
        
    outTileData.filePtr_ = inTileData.filePtr_;
    outTileData.fileOff_ = inTileData.fileOff_;
    
    inTileData.filePtr_ = 0;
    inTileData.fileOff_ = 0;    
    
    ramTilesIndexesVec_[ nextSwapTileRamTilesIndexesVecIdx_ ] =
      getTilePointer_reqTileIdx_;
    nextSwapTileRamTilesIndexesVecIdx_ = 
      ( ( nextSwapTileRamTilesIndexesVecIdx_ + 1 ) % 
      ((unsigned int)ramTilesIndexesVec_.size()) );
    TEAGN_DEBUG_CONDITION( nextSwapTileRamTilesIndexesVecIdx_ <
      ramTilesIndexesVec_.size(), "Internal error" );          
        
    return allTilesPtrsVec_[ getTilePointer_reqTileIdx_ ];
  }
  else
  {
    return getTilePointer_tilePtr_;
  }
}

bool TeRasterMemManager::createNewDiskFile( unsigned long int size,
  std::string& filename, FILE** fileptr )
{
  TEAGN_TRUE_OR_RETURN( TeGetTempFileName( filename ),
    "Unable to get temporary file name" );
    
  (*fileptr) = fopen( filename.c_str(), "wb+" );
  TEAGN_TRUE_OR_RETURN( (*fileptr) != 0, "Invalid file pointer" )
  
  long seekoff = (long)( size - 1 );
  
  if( 0 != fseek( (*fileptr), seekoff, SEEK_SET ) )
  {
    fclose( (*fileptr) );
    TEAGN_LOGERR( "File seek error" );
    return false;
  }

  unsigned char c = '\0';
  if( 1 != fwrite( &c, 1, 1, (*fileptr) ) )
  {
    fclose( (*fileptr) );
    TEAGN_LOGERR( "File write error" );
    return false;
  }
    
  return true;
}

