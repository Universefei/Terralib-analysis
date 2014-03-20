#include "TePDIParallelSegmenter.hpp"

#include "TePDIParaSegStrategyFactory.hpp"
#include "TePDIPIManager.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TeThreadFunctor.h"
#include "../kernel/TeAgnostic.h"


TePDIParallelSegmenter::TePDIParallelSegmenter()
{
}


TePDIParallelSegmenter::~TePDIParallelSegmenter()
{
}

bool TePDIParallelSegmenter::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking input_image_ptr */
    
  TePDITypes::TePDIRasterPtrType input_image_ptr;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_image_ptr", 
    input_image_ptr ), "Missing parameter: input_image_ptr" );
  TEAGN_TRUE_OR_RETURN( input_image_ptr.isActive(),
    "Invalid parameter: input_image_ptr inactive" );
  TEAGN_TRUE_OR_RETURN( input_image_ptr->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: input_image_ptr not ready" );
  
  /* Checking input_channels */
  
  std::vector< unsigned int > input_channels;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_channels", 
    input_channels ), "Missing parameter: input_channels" );
  for( unsigned inputChannelsIdx = 0 ; inputChannelsIdx < input_channels.size() ;
    ++inputChannelsIdx )
  {
    TEAGN_TRUE_OR_RETURN( ( (int)input_channels[ inputChannelsIdx ] <
      input_image_ptr->params().nBands() ), 
      "Invalid parameter: input_channels" );  
  }
  
  /* Checking strategy_name */
  
  std::string strategy_name;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "strategy_name", 
    strategy_name ), "Missing parameter: strategy_name" );
  TEAGN_TRUE_OR_RETURN( TePDIParaSegStrategyFactory::instance().find(
    strategy_name ) != TePDIParaSegStrategyFactory::instance().end(),
    "Invalid parameter: strategy_name" ); 
       
  /* Checking output_image_ptr */
    
  TePDITypes::TePDIRasterPtrType output_image_ptr;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_image_ptr", 
    output_image_ptr ),
    "Missing parameter: output_image_ptr" );
  TEAGN_TRUE_OR_RETURN( output_image_ptr.isActive(),
    "Invalid parameter: output_image_ptr inactive" );
  TEAGN_TRUE_OR_RETURN( output_image_ptr->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: output_image_ptr not ready" );           
  
  /* Checking max_block_size */
  
  if( parameters.CheckParameter< unsigned int >( "max_block_size" ) )
  {
    unsigned int max_block_size = 0;
    parameters.GetParameter( "max_block_size", max_block_size );
    TEAGN_TRUE_OR_RETURN( max_block_size > 0, 
      "Invalid parameter: max_block_size" );     
  }     
  
  return true;
}

bool TePDIParallelSegmenter::RunImplementation()
{
  /* Retriving parameters */

  TePDITypes::TePDIRasterPtrType input_image_ptr;
  params_.GetParameter( "input_image_ptr", input_image_ptr );
  
  std::vector< unsigned int > input_channels;
  params_.GetParameter( "input_channels", input_channels );  
  
  TePDIParameters strategy_params;
  if( params_.CheckParameter< TePDIParameters >( "strategy_params" ) )
  {
    params_.GetParameter( "strategy_params", strategy_params );
  }      
  
  std::string strategy_name;
  params_.GetParameter( "strategy_name", strategy_name );
  
  TePDITypes::TePDIRasterPtrType output_image_ptr;
  params_.GetParameter( "output_image_ptr", output_image_ptr );
  
  unsigned int max_seg_threads = TeGetPhysProcNumber();
  if( params_.CheckParameter< unsigned int >( "max_seg_threads" ) )
  {
    params_.GetParameter( "max_seg_threads", max_seg_threads );
  }    
  
  bool merge_adjacent_blocks = true;
  if( params_.CheckParameter< bool >( "merge_adjacent_blocks" ) )
  {
    params_.GetParameter( "merge_adjacent_blocks", merge_adjacent_blocks );
  }      
  
  // Globals
  
  TeMutex globalMutex;
  TeThreadSignal globalSignal;
  volatile bool abortSegmentationFlag = false;  
  // The number of generated segments IDs
  volatile TePDIParaSegSegment::SegIdDataType segmentsIdsCounter = 0;
  
  const unsigned int pIManagerSteps = ( max_seg_threads ?
    (max_seg_threads+1) : 2 );
  TePDIPIManager progress( "Segmenting", pIManagerSteps, progress_enabled_ );
  
  // Initiate output raster

  {
    TeRasterParams outputImgParams = output_image_ptr->params();
    outputImgParams.nBands( 1 );
    outputImgParams.projection( 
      input_image_ptr->params().projection() );
    outputImgParams.boundingBoxLinesColumns( 
      input_image_ptr->params().boundingBox().x1(), 
      input_image_ptr->params().boundingBox().y1(), 
      input_image_ptr->params().boundingBox().x2(), 
      input_image_ptr->params().boundingBox().y2(), 
      input_image_ptr->params().nlines_, 
      input_image_ptr->params().ncols_ );    
    
    TEAGN_TRUE_OR_RETURN( output_image_ptr->init( outputImgParams ),
      "Output raster init error" );  
  }  
  
  // Calc block width & height
  
  unsigned int max_block_width = 0;
  unsigned int max_block_height = 0;
  
  if( params_.CheckParameter< unsigned int >( "max_block_size" ) )
  {
    unsigned int user_max_block_width = 0;
    params_.GetParameter( "max_block_size", user_max_block_width );
    
    TEAGN_TRUE_OR_RETURN( calcBestBlockSize( input_image_ptr->params().nlines_,
      input_image_ptr->params().ncols_, 1000,
      user_max_block_width * user_max_block_width, max_block_width, 
      max_block_height ),
      "Unable to define sub-image blocks size" );    
  }
  else
  {
    // Guessing memory limits
    
    const double totalPhysMem = (double)TeGetTotalPhysicalMemory();
    
    const double usedVMem = (double)TeGetUsedVirtualMemory();
    
    const double totalVMem = ( (double)TeGetTotalVirtualMemory() ) / 
      2.0;
    
    const double freeVMem = MIN( totalPhysMem, 
      ( ( totalVMem <= usedVMem ) ? 0.0 : ( totalVMem - usedVMem ) ) );
              
    const double pixelRequiredRam = 
      (double)(input_channels.size() * sizeof( double ) );
      
    const double totalAvaiPixels = ( 0.25 * freeVMem ) / pixelRequiredRam;
      
    // all blocks from the first image blocks matrix line plus 1
    // block from the second line
    unsigned int maxBlockPixels = (unsigned int)
      ( totalAvaiPixels / ( (double)MAX(max_seg_threads + 1, 4 ) ) );
    
    // updating maxBlockPixels considering the minimum number 
    // of 3x3 blocks
    
    const unsigned int rasterPixelsNmb = (unsigned int)( 
      input_image_ptr->params().nlines_ * input_image_ptr->params().ncols_ );
      
    maxBlockPixels = MIN( maxBlockPixels, (unsigned int)( rasterPixelsNmb / 
      MAX( 9,max_seg_threads ) ) );
      
    // updating maxBlockPixels considering the the strategy
    // memory requirements
    
    TePDIParaSegStrategyFactoryParams facParams;
    facParams.stratParams_.globalMutexPtr_ = &globalMutex;
    facParams.stratParams_.segmentsIdsCounterPtr_ = &segmentsIdsCounter;
    facParams.stratParams_.blockMergingEnabled_ = merge_adjacent_blocks;   
    
    TeSharedPtr< TePDIParaSegStrategy > strategyPtr(
      TePDIParaSegStrategyFactory::make( strategy_name, 
      facParams ) );
    TEAGN_TRUE_OR_THROW( strategyPtr.isActive(), 
    "Unable to create an segmentation strategy" ); 
    TEAGN_TRUE_OR_RETURN( strategyPtr->setParameters( 
      strategy_params ), 
      "Unable to set segmentation strategy parameters" );    
      
    const double stratMemUsageFactor = strategyPtr->getMemUsageFactor();  
    TEAGN_DEBUG_CONDITION( stratMemUsageFactor > 0.0,
      "Invalid stratMemUsageFactor" );
      
    maxBlockPixels = (unsigned int)( ((double)maxBlockPixels ) /
      stratMemUsageFactor );
      
    // calculating the max block dimentions
    
    TEAGN_TRUE_OR_RETURN( calcBestBlockSize( 
      input_image_ptr->params().nlines_,
      input_image_ptr->params().ncols_, 1000, maxBlockPixels, 
      max_block_width, max_block_height ),
      "Unable to define sub-image blocks size" );           
  }
  
  // Creating image blocks matrix
  
  SegmentsBlocksMatrixT segmentsblocksMatrix;
  segmentsblocksMatrix.Reset( SegmentsBlocksMatrixT::RAMMemPol );
  
  {
    const unsigned int hBlocksNumber = (unsigned int)ceil( 
      ((double)input_image_ptr->params().ncols_) / ((double)max_block_width) );
    const unsigned int vBlocksNumber = (unsigned int)ceil( 
      ((double)input_image_ptr->params().nlines_) / ((double)max_block_height) );
       
    TEAGN_TRUE_OR_RETURN( segmentsblocksMatrix.Reset( vBlocksNumber, 
      hBlocksNumber ), "Blocks matrix reset error" );
      
    const unsigned int linesBound = (unsigned int)
      input_image_ptr->params().nlines_;
    const unsigned int colsBound = (unsigned int)
      input_image_ptr->params().ncols_;
    unsigned int blockXBound = 0;
    unsigned int blockYBound = 0;
      
    for( unsigned int segmentsMatrixLine = 0 ; segmentsMatrixLine < 
      segmentsblocksMatrix.GetLines() ; ++segmentsMatrixLine )
    {
      for( unsigned int segmentsMatrixCol = 0 ; segmentsMatrixCol < 
        segmentsblocksMatrix.GetColumns() ; ++segmentsMatrixCol )
      {
        TePDIParaSegSegmentsBlock& segmentsBlock = segmentsblocksMatrix( 
          segmentsMatrixLine, segmentsMatrixCol );
          
        segmentsBlock.status_ = TePDIParaSegSegmentsBlock::BlockNotProcessed;
        segmentsBlock.startX_ = segmentsMatrixCol * max_block_width;
        segmentsBlock.startY_ = segmentsMatrixLine * max_block_height;          
          
        blockXBound = MIN( segmentsBlock.startX_ + max_block_width, 
          colsBound );
        blockYBound = MIN( segmentsBlock.startY_ + max_block_height, 
          linesBound );          
      
        segmentsBlock.width_ = blockXBound - segmentsBlock.startX_;
        segmentsBlock.height_ = blockYBound - segmentsBlock.startY_;
      }
    }
  }
  
  // Spawning segmenter threads
  
  bool returnValue = true;

  std::vector< TeSharedPtr< TeThreadFunctor > > threadsVec;
  
  SegmenterThreadEntryParams segThreadParams;
  segThreadParams.inputRasterPtr_ = input_image_ptr.nakedPointer();
  segThreadParams.inputChannelsVecPtr_ = &input_channels;
  segThreadParams.segsBlocksMatrixPtr_ = &segmentsblocksMatrix;
  segThreadParams.mutexPtr_ = &globalMutex;
  segThreadParams.stratParamsPtr_ = &strategy_params;
  segThreadParams.stratNamePtr_ = &strategy_name;
  segThreadParams.abortSegmentationFlagPtr_ = &abortSegmentationFlag;
  segThreadParams.segmentsIdsCounterPtr_ = &segmentsIdsCounter;
  segThreadParams.signalPtr_ = &globalSignal;
  segThreadParams.progressEnabled_ = progress_enabled_;
  segThreadParams.mergeAdjacentBlocks_ = merge_adjacent_blocks;
    
  if( max_seg_threads )
  {    
    for( unsigned int threadIdx = 0 ; threadIdx < max_seg_threads ;
      ++threadIdx )
    {
      TeSharedPtr< TeThreadFunctor > threadPtr( new TeThreadFunctor );
      threadPtr->setStartFunctPtr( segmenterThreadEntry );
      threadPtr->setParametersPtr( &segThreadParams );
      threadPtr->start();
      threadsVec.push_back( threadPtr );
    }
  }
  else
  {
    if( ! segmenterThreadEntry( (void*)&segThreadParams ) )
    {
      TEAGN_LOGERR( "Segmentation error" );
      returnValue = false;
    }
    
    progress.Increment();
  }
    
  // Spawning the thread to group the segmented data
  
  TeSharedPtr< TeThreadFunctor > groupDataThreadPtr( new TeThreadFunctor );

  if( returnValue )
  {
    GroupDataThreadEntryParams groupThreadParams;
    groupThreadParams.outputRasterPtr_ = output_image_ptr.nakedPointer();
    groupThreadParams.segsBlocksMatrixPtr_ = &segmentsblocksMatrix;
    groupThreadParams.mutexPtr_ = &globalMutex;
    groupThreadParams.stratParamsPtr_ = &strategy_params;
    groupThreadParams.stratNamePtr_ = &strategy_name;
    groupThreadParams.abortSegmentationFlagPtr_ = &abortSegmentationFlag;
    groupThreadParams.signalPtr_ = &globalSignal;
    groupThreadParams.segmentsIdsCounterPtr_ = &segmentsIdsCounter;
    groupThreadParams.waitForSignals_ = true;
    groupThreadParams.progressEnabled_ = progress_enabled_;
    groupThreadParams.mergeAdjacentBlocks_ = merge_adjacent_blocks;
    
    if( max_seg_threads )
    {   
      groupDataThreadPtr.reset( new TeThreadFunctor );
      groupDataThreadPtr->setStartFunctPtr( groupDataThreadEntry );
      groupDataThreadPtr->setParametersPtr( &groupThreadParams );
      groupDataThreadPtr->start();
    }
    else
    {
      if( returnValue )
      {
        groupThreadParams.waitForSignals_ = false;
        
        if( ! groupDataThreadEntry( (void*)&groupThreadParams ) )
        {
          TEAGN_LOGERR( "Segmentation error" );
          returnValue = false;
        }
      }
      
      progress.Increment();
    }
  }

  // Wait all threads to finish
      
  if( max_seg_threads )
  {        
    for( unsigned int threadsVecIdx = 0 ; threadsVecIdx < max_seg_threads ;
      ++threadsVecIdx )
    {
      threadsVec[ threadsVecIdx ]->waitToFinish();
      returnValue = returnValue && threadsVec[ threadsVecIdx ]->getReturnValue();
      
      progress.Increment();
    }  
    
    while( groupDataThreadPtr->getStatus() != TeThreadStopped )
    {
      globalSignal.waitSignal( 250 );
      globalSignal.emitSignal();
    }
    
    returnValue = returnValue && groupDataThreadPtr->getReturnValue();
    
    progress.Increment();
  }
  
  // So far.... so good !!
  
  return returnValue;
}


void TePDIParallelSegmenter::ResetState( const TePDIParameters& )
{
}

bool TePDIParallelSegmenter::segmenterThreadEntry(void* paramsPtr)
{
  TEAGN_DEBUG_CONDITION( paramsPtr, "Invalid pointer" );
  SegmenterThreadEntryParams& segThreadParams =
    *((SegmenterThreadEntryParams*)paramsPtr);
    
  TEAGN_DEBUG_CONDITION( segThreadParams.inputRasterPtr_,
    "Invalid raster pointer" );
  TEAGN_DEBUG_CONDITION( segThreadParams.inputChannelsVecPtr_,
    "Invalid input channels" );
  TEAGN_DEBUG_CONDITION( segThreadParams.segsBlocksMatrixPtr_,
    "Invalid block matrix pointer" );
  TEAGN_DEBUG_CONDITION( segThreadParams.mutexPtr_,
    "Invalid mutex pointer" )
  TEAGN_DEBUG_CONDITION( segThreadParams.stratParamsPtr_, 
    "Invalid strategy params pointer" );
  TEAGN_DEBUG_CONDITION( segThreadParams.stratNamePtr_,
    "Invalid strategy name pointer" );
  TEAGN_DEBUG_CONDITION( segThreadParams.abortSegmentationFlagPtr_,
    "Invalid abort flag pointer" );
  TEAGN_DEBUG_CONDITION( segThreadParams.segmentsIdsCounterPtr_,
    "Invalid abort flag pointer" ); 
       
  // Creating the segmentation strategy instance
  
  TePDIParaSegStrategyFactoryParams facParams;
  facParams.stratParams_.globalMutexPtr_ = segThreadParams.mutexPtr_;
  facParams.stratParams_.segmentsIdsCounterPtr_ = 
    segThreadParams.segmentsIdsCounterPtr_;
  facParams.stratParams_.blockMergingEnabled_ = 
    segThreadParams.mergeAdjacentBlocks_;
  
  TeSharedPtr< TePDIParaSegStrategy > strategyPtr(
    TePDIParaSegStrategyFactory::make( *segThreadParams.stratNamePtr_, 
    facParams ) );
  TEAGN_TRUE_OR_THROW( strategyPtr.isActive(), 
    "Unable to create an segmentation strategy" ); 
  TEAGN_TRUE_OR_RETURN( strategyPtr->setParameters( 
    *segThreadParams.stratParamsPtr_ ), 
    "Unable to set segmentation strategy parameters" );    
    
  // Pre-initializing raster data matrix
    
  TePDIParaSegStrategy::RasterDataVecT rasterDataVec;
  {
    for( unsigned int bandIdx = 0 ; bandIdx < 
      segThreadParams.inputChannelsVecPtr_->size() ;
      ++bandIdx )
    {
      rasterDataVec.push_back( TePDIParaSegStrategy::RasterBandDataT() );
      rasterDataVec.back().Reset( 
        TePDIParaSegStrategy::RasterBandDataT::RAMMemPol );
    }
  }    
    
  // Looking for a non processed segments block
  
  TePDIPIManager progress( "Segmenting", 
    segThreadParams.segsBlocksMatrixPtr_->GetLines() *
    segThreadParams.segsBlocksMatrixPtr_->GetColumns() , 
    segThreadParams.progressEnabled_ );
  
  for( unsigned int sBMLine = 0 ; sBMLine < 
    segThreadParams.segsBlocksMatrixPtr_->GetLines() ; ++sBMLine )
  {
    for( unsigned int sBMCol = 0 ; sBMCol < 
      segThreadParams.segsBlocksMatrixPtr_->GetColumns() ;  ++sBMCol )  
    {
      if( *(segThreadParams.abortSegmentationFlagPtr_ ) )
      {
        return false;
      }
      else
      {
        segThreadParams.mutexPtr_->lock();
        
        TePDIParaSegSegmentsBlock& segsBlk = 
          segThreadParams.segsBlocksMatrixPtr_->operator()( sBMLine, sBMCol );
        
        if( segsBlk.status_ == TePDIParaSegSegmentsBlock::BlockNotProcessed )
        {
          segsBlk.status_ = TePDIParaSegSegmentsBlock::BlockUnderSegmentation;
          
          segThreadParams.mutexPtr_->unLock();
          
          // Initializing raster data matrix
          
          for( unsigned int rasterDataVecIdx = 0 ; rasterDataVecIdx < 
            segThreadParams.inputChannelsVecPtr_->size() ; ++rasterDataVecIdx )
          {
            TePDIParaSegStrategy::RasterBandDataT& bandData = rasterDataVec[ rasterDataVecIdx ];
            if( ! bandData.Reset( segsBlk.height_, segsBlk.width_, 
              TePDIParaSegStrategy::RasterBandDataT::RAMMemPol ) )
            {
              *(segThreadParams.abortSegmentationFlagPtr_ ) = true;
              TEAGN_LOGERR( "Error initializing raster data matrix" );
              return false;
            }
          }
          
          // Load raster data into data matrix
          
          {
            segThreadParams.mutexPtr_->lock();
            
            TeRaster& inputRaster = *(segThreadParams.inputRasterPtr_);
            unsigned int blockLine = 0;
            unsigned int blockCol = 0;
            double* bandDataLinePtr = 0;
            unsigned int rasterLine = 0;
            
            for( unsigned int inputChannelsVecIdx = 0 ; inputChannelsVecIdx <
              segThreadParams.inputChannelsVecPtr_->size() ;
              ++inputChannelsVecIdx )
            {
              const unsigned int& bandIdx = 
                segThreadParams.inputChannelsVecPtr_->operator[]( 
                inputChannelsVecIdx );
                
              TePDIParaSegStrategy::RasterBandDataT& bandData = rasterDataVec[ inputChannelsVecIdx ];
                
              for( blockLine = 0 ; blockLine < segsBlk.height_ ; ++blockLine )
              {
                bandDataLinePtr = bandData[ blockLine ];
                rasterLine = blockLine + segsBlk.startY_;
                
                for( blockCol = 0 ; blockCol < segsBlk.width_ ; ++blockCol )
                {
                  inputRaster.getElement( blockCol + segsBlk.startX_,
                    rasterLine, bandDataLinePtr[ blockCol ],
                    bandIdx );
                }
              }   
            }
                 
            segThreadParams.mutexPtr_->unLock();     
          }
          
          // Executing the strategy
          
          if( ! strategyPtr->execute( rasterDataVec, segsBlk.segsPtrsMatrix_ ) )
          {
            *(segThreadParams.abortSegmentationFlagPtr_ ) = true;
            TEAGN_LOGERR( "Segmentation strategy execution error" );
            return false;
          }
          
          // Check block dimensions
          
          TEAGN_DEBUG_CONDITION( segsBlk.segsPtrsMatrix_.GetLines()
            == segsBlk.height_, "Invalid generated matrix lines number " )
          TEAGN_DEBUG_CONDITION( segsBlk.segsPtrsMatrix_.GetColumns()
            == segsBlk.width_, "Invalid generated matrix columns number " )
          
          // Updating block information
          
          segsBlk.status_ = TePDIParaSegSegmentsBlock::BlockSegmented;
          
          // Emiting block processed signal
          
          segThreadParams.signalPtr_->emitSignal();
        }
        else
        {
          segThreadParams.mutexPtr_->unLock();
        }
      }
      
      // Updating the progress interface
      
      if( progress.Increment() )
      {
        *(segThreadParams.abortSegmentationFlagPtr_ ) = true;
        TEAGN_LOGERR( "Canceled by the user" );
        return false;
      }         
    }
  }

  return true;
}

bool TePDIParallelSegmenter::groupDataThreadEntry(void* paramsPtr)
{
  TEAGN_DEBUG_CONDITION( paramsPtr, "Invalid pointer" );
  GroupDataThreadEntryParams& groupDataThreadParams =
    *((GroupDataThreadEntryParams*)paramsPtr);
    
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.outputRasterPtr_,
    "Invalid outputRasterPtr_" );
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.segsBlocksMatrixPtr_,
    "Invalid segsBlocksMatrixPtr_" );
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.mutexPtr_,
    "Invalid mutexPtr_" );
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.stratParamsPtr_,
    "Invalid stratParamsPtr_" )
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.stratNamePtr_, 
    "Invalid stratNamePtr_" );
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.abortSegmentationFlagPtr_,
    "Invalid abortSegmentationFlagPtr_" );
  TEAGN_DEBUG_CONDITION( groupDataThreadParams.signalPtr_,
    "Invalid signalPtr_" );    
    
  // Creating the segmentation strategy instance
  
  TePDIParaSegStrategyFactoryParams facParams;
  facParams.stratParams_.globalMutexPtr_ = groupDataThreadParams.mutexPtr_;
  facParams.stratParams_.segmentsIdsCounterPtr_ = 
    groupDataThreadParams.segmentsIdsCounterPtr_;
  facParams.stratParams_.blockMergingEnabled_ = 
    groupDataThreadParams.mergeAdjacentBlocks_;
  
  TeSharedPtr< TePDIParaSegStrategy > strategyPtr(
    TePDIParaSegStrategyFactory::make( *groupDataThreadParams.stratNamePtr_, 
    facParams ) );
  TEAGN_TRUE_OR_THROW( strategyPtr.isActive(), 
    "Unable to create an segmentation strategy" );     
  TEAGN_TRUE_OR_RETURN( strategyPtr->setParameters(
    *groupDataThreadParams.stratParamsPtr_ ), 
    "Unable to set segmentation strategy parameters" );    
    
  // Creating the progress instance
    
  TePDIPIManager progress( "Grouping blocks", 
    2 * groupDataThreadParams.segsBlocksMatrixPtr_->GetColumns() *
    groupDataThreadParams.segsBlocksMatrixPtr_->GetLines(), 
    groupDataThreadParams.progressEnabled_ );    

  // Processing blocks
  
  bool notAllBlocksProcessed = true;
      
  do
  {
    // Wait for a signal, if required
    if( groupDataThreadParams.waitForSignals_ )
    { 
      groupDataThreadParams.signalPtr_->waitSignal();
    }
    
    if( *(groupDataThreadParams.abortSegmentationFlagPtr_) )
    {
      TEAGN_LOGERR( "Segmentation aborted ");
      return false;
    }
    else
    {
      // Locking for blocks to merge border segments
      
      unsigned int segsBlkMtxLine = 0 ;
      unsigned int segsBlkMtxCol = 0;
      
      for( segsBlkMtxLine = 0 ; segsBlkMtxLine < 
        groupDataThreadParams.segsBlocksMatrixPtr_->GetLines() ; 
        ++segsBlkMtxLine )
      {
        for( segsBlkMtxCol = 0 ; segsBlkMtxCol < 
          groupDataThreadParams.segsBlocksMatrixPtr_->GetColumns() ; 
          ++segsBlkMtxCol )
        {
          groupDataThreadParams.mutexPtr_->lock();
          
          TePDIParaSegSegmentsBlock& currBlk = groupDataThreadParams.
            segsBlocksMatrixPtr_->operator()( segsBlkMtxLine, segsBlkMtxCol );
          
          // Checking the block requirements 
          
          if( currBlk.status_ == TePDIParaSegSegmentsBlock::BlockSegmented )
          {
            // Does this block depends on other blocks ???
            
            bool blokDependsOk = true;
            bool requireUpBlk = false;
            bool requireLeftBlk = false;
            
            if( segsBlkMtxLine > 0 )
            {
              requireUpBlk = true;
              
              if( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                segsBlkMtxLine - 1 , segsBlkMtxCol ).status_ != 
                TePDIParaSegSegmentsBlock::BlockMerged )
              {
                // Cannot process this block since the required block
                // from the above line was not merged yet

                blokDependsOk = false;
              }
            }
            
            if( segsBlkMtxCol > 0 )
            {
              requireLeftBlk = true;
              
              if( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                segsBlkMtxLine, segsBlkMtxCol - 1 ).status_ != 
                TePDIParaSegSegmentsBlock::BlockMerged )
              {
                // Cannot process this block since the required left block
                // merged yet

                blokDependsOk = false;
              }
            }
            
            // Ok to merge ??
            
            if( blokDependsOk )
            {
              TePDIParaSegSegmentsBlock* upBlkPtr = 0;
              if( requireUpBlk )
              { 
                upBlkPtr = &( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                  segsBlkMtxLine - 1, segsBlkMtxCol ) );
              }
              
              
              TePDIParaSegSegmentsBlock* leftBlkPtr = 0;
              if( requireLeftBlk )
              {
                leftBlkPtr = &( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                  segsBlkMtxLine, segsBlkMtxCol - 1 ) );
              }
              
              groupDataThreadParams.mutexPtr_->unLock();
              
              // Merging block
              
              if( groupDataThreadParams.mergeAdjacentBlocks_)
              {
                if( ! strategyPtr->mergeSegments( currBlk.segsPtrsMatrix_,
                  ( requireUpBlk ? &(upBlkPtr->segsPtrsMatrix_) : 0 ), 
                  ( requireLeftBlk ? &(leftBlkPtr->segsPtrsMatrix_) : 0 ) ) )
                {
                  TEAGN_LOGERR( "Block merging error");
                  *(groupDataThreadParams.abortSegmentationFlagPtr_ ) = true;
                  return false;
                }
              }              
              
              currBlk.status_ = TePDIParaSegSegmentsBlock::BlockMerged;
              
              if( progress.Increment() )
              {
                TEAGN_LOGERR( "Canceled by the user");
                *(groupDataThreadParams.abortSegmentationFlagPtr_ ) = true;
                return false;
              }
            }
            else
            {
              groupDataThreadParams.mutexPtr_->unLock();
              
              // Unable to process the current block
              // break the loop and wait until the next oportunity
              
              segsBlkMtxLine = 
                groupDataThreadParams.segsBlocksMatrixPtr_->GetLines();
                
              break;
            }
          }
          else
          { // This block was not segmented yet !
            groupDataThreadParams.mutexPtr_->unLock();
          }
        } 
      }
            
      // Locking for blocks to flush
      
      for( segsBlkMtxLine = 0 ; segsBlkMtxLine < 
        groupDataThreadParams.segsBlocksMatrixPtr_->GetLines() ; ++segsBlkMtxLine )
      {
        for( segsBlkMtxCol = 0 ; segsBlkMtxCol < 
          groupDataThreadParams.segsBlocksMatrixPtr_->GetColumns() ; ++segsBlkMtxCol )
        {
          groupDataThreadParams.mutexPtr_->lock();
          
          TePDIParaSegSegmentsBlock& currBlk = groupDataThreadParams.
            segsBlocksMatrixPtr_->operator()( segsBlkMtxLine, segsBlkMtxCol );
          
          // Checking the block requirements 
          
          if( currBlk.status_ == TePDIParaSegSegmentsBlock::BlockMerged )
          {
            // Does this block depends on other blocks ???
            
            bool blokDependsOk = true;
            bool requireDownBlk = false;
            bool requireRightBlk = false;
            
            if( segsBlkMtxLine < ( groupDataThreadParams.segsBlocksMatrixPtr_->
              GetLines() - 1 ) )
            {
              requireDownBlk = true;
              
              if( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                segsBlkMtxLine + 1 , segsBlkMtxCol ).status_ != 
                TePDIParaSegSegmentsBlock::BlockMerged )
              {
                // Cannot process this block since the required block
                // from the above line was not segmented yet

                blokDependsOk = false;
              }
            }
            
            if( segsBlkMtxCol < ( groupDataThreadParams.segsBlocksMatrixPtr_->
              GetColumns() - 1 ) )
            {
              requireRightBlk = true;
              
              if( groupDataThreadParams.segsBlocksMatrixPtr_->operator()( 
                segsBlkMtxLine, segsBlkMtxCol + 1 ).status_ != 
                TePDIParaSegSegmentsBlock::BlockMerged )
              {
                // Cannot process this block since the required left block
                // segmented yet

                blokDependsOk = false;
              }
            }
            
            // Ok to flush ??
            
            if( blokDependsOk )
            {
              groupDataThreadParams.mutexPtr_->unLock();
              
              // Flushing block

              if( ! flushBlock( currBlk, 
                *(groupDataThreadParams.outputRasterPtr_), progress ) )
              {
                TEAGN_LOGERR( "Block flushing error");
                *(groupDataThreadParams.abortSegmentationFlagPtr_ ) = true;
                return false;
              }
              
              currBlk.status_ = TePDIParaSegSegmentsBlock::BlockFlushed;
              
              if( progress.Increment() )
              {
                TEAGN_LOGERR( "Canceled by the user");
                *(groupDataThreadParams.abortSegmentationFlagPtr_ ) = true;
                return false;
              }              
              
              // Is this the last block to flush ??
              
              if( ( segsBlkMtxLine == groupDataThreadParams.segsBlocksMatrixPtr_->
                GetLines() - 1 ) && ( segsBlkMtxCol == groupDataThreadParams.
                segsBlocksMatrixPtr_->GetColumns() - 1 ) )
              {
                notAllBlocksProcessed = false;
              }
            }
            else
            {
              groupDataThreadParams.mutexPtr_->unLock();
              
              // Unable to process the current block
              // break the loop and wait until the next oportunity
              
              segsBlkMtxLine = 
                groupDataThreadParams.segsBlocksMatrixPtr_->GetLines();
                
              break;              
            }
          }
          else
          { // This block was not segmented yet !
            groupDataThreadParams.mutexPtr_->unLock();
          }
        } 
      }            
    }
  } while( notAllBlocksProcessed );

  return true;
}

bool TePDIParallelSegmenter::flushBlock( TePDIParaSegSegmentsBlock& currBlock,
  TeRaster& outRaster, TePDIPIManager& progress )
{
  TEAGN_DEBUG_CONDITION( outRaster.params().status_ !=  
    TeRasterParams::TeNotReady, 
    "Output raster not initiated" );
  TEAGN_DEBUG_CONDITION( currBlock.width_ == currBlock.segsPtrsMatrix_.
    GetColumns(), 
    "Block size mismatch" );
  TEAGN_DEBUG_CONDITION( currBlock.height_ == currBlock.segsPtrsMatrix_.
    GetLines(), 
    "Block size mismatch" );
  TEAGN_DEBUG_CONDITION( currBlock.status_ == 
    TePDIParaSegSegmentsBlock::BlockMerged, "Cannot flush a not merged block" );
    
  unsigned int curBLine = 0;
  unsigned int curBCol = 0;
  unsigned int rasterLine = 0;
  unsigned int rasterCol = 0;
  TePDIParaSegSegment* const* bLinePtr = 0;
  
  // Flushing data to output raster
  
  for( curBLine = 0 ; curBLine < currBlock.height_ ; ++curBLine )
  {
    rasterLine = curBLine + currBlock.startY_;
    bLinePtr = currBlock.segsPtrsMatrix_.operator[]( curBLine );
    
    for( curBCol = 0 ; curBCol < currBlock.width_ ; ++curBCol )
    {
      rasterCol = curBCol + currBlock.startX_;
      
      if( ! outRaster.setElement( rasterCol, rasterLine, (double)
        (bLinePtr[ curBCol ]->id_), 0 ) )
      {
        TEAGN_LOGERR( "Error writing to output raster" );
        return false;
      }
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" )
  }
  
  // Cleanning data
  
  currBlock.clear();
    
  // So far... so GOOD !!
  
  return true;
}

bool TePDIParallelSegmenter::calcBestBlockSize( 
  const unsigned int totalImageLines, const unsigned totalImageCols, 
  const unsigned int minBlockPixels, const unsigned int maxBlockPixels, 
  unsigned int& blockWidth, unsigned int& blockHeight ) const
{
  TEAGN_DEBUG_CONDITION( minBlockPixels < maxBlockPixels, 
    "Invalid blocl pixels number" )
    
  const double rasterPixelsNmb = ((double)( totalImageCols * 
    totalImageLines ) );
    
  const double maxScaleFactor = floor( 
    sqrt( rasterPixelsNmb / (double)minBlockPixels ) );
    
  double scaleFactor = TeRound( sqrt( rasterPixelsNmb / (double)maxBlockPixels ) 
    );
  
  scaleFactor = MIN( scaleFactor, maxScaleFactor );
  
  scaleFactor = MAX( scaleFactor, 1.0 );
  
  blockWidth = (unsigned int)( ((double)( totalImageCols )) / scaleFactor );
  blockHeight = (unsigned int)( ((double)( totalImageLines )) / scaleFactor );
  
  blockWidth = MIN( blockWidth, totalImageCols );
  blockHeight = MIN( blockHeight, totalImageLines );
  
  return true;
}

