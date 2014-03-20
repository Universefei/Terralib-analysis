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
/*! \file TeRasterMemManager.h
    \brief This file contains a class that deals with a raster memory management
*/

#ifndef TERASTERMEMMANAGER_H
  #define TERASTERMEMMANAGER_H

  #include "TeSharedPtr.h"
  
  #include <vector>
  
  /**
   * @brief This class deals with a raster memory management.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   */
  class TL_DLL TeRasterMemManager {
  
    public :
    
      /**
       * @enum Memory policy.
       */ 
      enum MemoryPolicy {
        /**
         * @brief Automatic memory policy ( Try to use RAM or DISK, 
         * if there is no avaliable RAM ).
         */
        AutoMemPol = 1,
        /**
         * @brief RAM memory policy.
         */
        RAMMemPol = 2,
        /**
         * @brief Disk memory policy.
         */
        DiskMemPol = 3
      };    
    
      /**
       * @brief Default Constructor.
       */
      TeRasterMemManager();    

      /**
       * @brief Default Destructor
       */
      ~TeRasterMemManager();
      
      /**
       * @brief Clear all data structures.
       */
      void clear();
      
      /**
       * @brief Reset the instance following new raster parameters.
       * @param bands The number of bands (channels).
       * @param tilesPerBand The tiles inside each band.
       * @param tilesSizesVec The tile size (bytes) for each band.
       * @param memoryPolicy Memory policy.
       * @param maxMemPercentUsage The max amount of RAM memory percentage 
       * to use when necessary - valid range: [0,100] suggested default:40.       
       * @param maxMemPercentUsage The the maximum temp file size (bytes) 
       * (sugested default:2GB=2ul * 1024ul * 1024ul * 1024ul).       
       * @return true if OK, false on errors.
       */
      bool reset( unsigned int bands, unsigned int tilesPerBand,
        const std::vector< unsigned int >& tilesSizesVec,
        MemoryPolicy memoryPolicy, unsigned char maxMemPercentUsage,
        unsigned long int maxTempFileSize );

      /**
       * @brief Returnas a pointer to a internal allocated tile.
       * @param band Band index.
       * @param tile Tile index.
       * @return a pointer to a internal allocated tile. 
       * @note This pointer is only valid until the next call to this function.
       */
      void* getTilePointer( const unsigned int& band, 
        const unsigned int& tile );    
      
    protected :
    
      /**
       * @brief Disk tile data.
       */         
      class DiskTileData
      {
        public :
          FILE* filePtr_;
          unsigned long int fileOff_;
          unsigned int size_;
          
          DiskTileData() : filePtr_( 0 ), fileOff_( 0 ), size_( 0 ) {};
          
          ~DiskTileData() {};
      };      

      /**
       * @typedef unsigned char TileDataT 
       * Tile data type.
       */    
      typedef unsigned char TileDataT;    
    
      /**
       * @typedef TileDataT* TilePtrT
       * Tile pointer type.
       */    
      typedef TileDataT* TilePtrT;
      
      /**
       * @typedef std::vector< TilePtrT > TilesPtrsVecT 
       * Tiles pointers vector type.
       */    
      typedef std::vector< TilePtrT > TilesPtrsVecT;   
      
      /**
       * @typedef std::list< DiskTileData >
       * Disk tile data vector type.
       */         
      typedef std::vector< DiskTileData > 
        DiskTileDataVecT;   
        
      /**
       * @typedef std::vector< std::pair< FILE*, unsigned int > >
       * Openend disk files data vector type.
       */         
      typedef std::vector< std::pair< FILE*, std::string > > 
        OpenDiskFilesVecT;   
             
      /**
       * @brief the max amount of free memory to use when necessary
       * (default:50).
       */      
      unsigned char maxMemPercentUsage_;
      
      /**
       * @brief The the maximum temp file size in bytes (default:2GB).
       */      
      unsigned long int maxTempFileSize_;      
      
      // Temp variables used by the getTilePointer method.
      TilePtrT getTilePointer_tilePtr_;
      unsigned int getTilePointer_reqTileIdx_;
      unsigned int getTilePointer_swapTileIdx_;
      
      /**
       * @brief Tiles per band (default:0).
       */    
      unsigned int tilesPerBand_;
      
      /**
       * @brief The number of bands (default:0).
       */    
      unsigned int bandsNmb_;
      
      /**
       * @brief A vector of tiles sizes for each band.
       */        
      std::vector< unsigned int > bandsTileSizes_;
    
      /**
       * @brief A vector of pointers to all allocated tiles.
       * @note Declared as a simple vector to optimize the
       * tile access.
       */     
      TilesPtrsVecT allTilesPtrsVec_;
      
      /**
       * @brief Openend disk files data vector.
       */        
      OpenDiskFilesVecT openDiskFilesVec_;   
      
      /**
       * @brief Disk tiles data vector.
       */   
      DiskTileDataVecT diskTilesDataVec_;
      
      /**
       * @brief The indexes inside allTilesPtrsVec_ of all RAM tiles.
       */        
      std::vector< unsigned int > ramTilesIndexesVec_;
      
      /**
       * @brief The index inside ramTilesIndexesVec_ of the next RAM
       * tile index that will be swapped to disk when a disk tile
       * is required.
       */        
      unsigned int nextSwapTileRamTilesIndexesVecIdx_;
      
      /**
       * @brief A pointer to the auxiliar tile used when swapping
       * data to/from disk.
       */              
      TilePtrT swapTilePtr_;
      
      /**
       * @brief Allocate disk files .
       * @param tileSize The tile size.
       * @param tilesNmb The tiles number.
       * @param openDiskFilesVec The output vector with all the
       * created files info.
       * @param diskTilesData The output tiles info.
       * @return true if OK, false on errors.
       */    
      bool allocateDiskFiles( unsigned int tileSize,
        unsigned int tilesNmb, OpenDiskFilesVecT& openDiskFilesVec, 
        DiskTileDataVecT& diskTilesData );
        
      /**
       * @brief Create a new disk temp file.
       * @param filename The file name.
       * @param size The file size.
       * @param fileptr The file pointer.
       * @return true if OK. false on errors.
       */
      bool createNewDiskFile( unsigned long int size,
        std::string& filename, FILE** fileptr );
      
    private :
    
      /**
       * @brief Alternative Constructor.
       * @param ext External reference.
       */    
      TeRasterMemManager( const TeRasterMemManager& ) {};
       
      /**
       * @brief =operator implementation.
       * @param ext External reference.
       */    
      const TeRasterMemManager& operator=( const TeRasterMemManager& )
        { return *this; };
      
  };
  
#endif

