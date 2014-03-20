#include "TePDISAMClassifier.hpp"

#include "TePDIPIManager.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TeAgnostic.h"

#include <math.h>

TePDISAMClassifier::ClassSpectralSamples::ClassSpectralSamples()
: classId_( 0 ), maxAngularDist_( 0.1 )
{
  samplesMatrix_.Reset( TePDIMatrix< double >::RAMMemPol );
}

TePDISAMClassifier::ClassSpectralSamples::~ClassSpectralSamples()
{
}

TePDISAMClassifier::SegThread::SegThread( bool progEnabled, TeRaster& inRaster, 
  const std::vector< unsigned int >& inRasterBands,
  const std::vector< ClassReferenceData >& refDataVector,
  TeMutex& inMutex, TeMutex& outMutex, std::vector< bool >& linesProcStatusVec,
  volatile bool& abortFlag, TeRaster& outRaster )
  : progEnabled_( progEnabled ), returnValue_( false ), inRaster_( inRaster ),
  inRasterBands_( inRasterBands ), refDataVector_( refDataVector ),
  inMutex_( inMutex ), outMutex_( outMutex ), 
  linesProcStatusVec_( linesProcStatusVec ), abortFlag_( abortFlag ),
  outRaster_( outRaster )
{
}

TePDISAMClassifier::ClassReferenceData::ClassReferenceData()
: classId_( 0 ), maxAngularDist_( 0.1 )
{
}

TePDISAMClassifier::ClassReferenceData::~ClassReferenceData()
{
}

TePDISAMClassifier::SegThread::~SegThread()
{
}

void TePDISAMClassifier::SegThread::run()
{
  returnValue_ = false;
  
  const long int rasterLines = (long int)inRaster_.params().nlines_;
  const unsigned int rasterCols = (unsigned int)inRaster_.params().ncols_;
  const unsigned int inRasterBandsSize = (unsigned int)inRasterBands_.size();
  
  TePDIPIManager progress( "Classifying", rasterLines , progEnabled_ );
  
  // The memory buffer for one readed raster data
  
  TePDIMatrix< double > lineDataMtx; // One band per line
  if( ! lineDataMtx.Reset( inRasterBandsSize, rasterCols, 
    TePDIMatrix< double >::RAMMemPol ) )
  {
    TEAGN_LOGERR( "Matrix creation error" );
    abortFlag_ = true;
    return;
  }
  
  // The memory buffer for the generated classified pixels for one raster line
  
  std::vector< unsigned int > classifyingResultVector;
  classifyingResultVector.resize( rasterCols, 0 );
  
  // classifying the entire raster....
  
  long int rasterLine = 0;
  unsigned int rasterCol = 0;
  unsigned int inRasterBandsIdx = 0;
  unsigned int inRasterBand = 0;
  unsigned int lDMLine = 0;
  unsigned int lDMCol = 0;
  const unsigned int refDataVectorSize = (unsigned int)refDataVector_.size();
  unsigned int refDataVectorIdx = 0;
  double angularTR = 0;
  double angularTT = 0;
  double angularRR = 0;
  double angularDist = 0;
  double minAngularDist = DBL_MAX;
  unsigned int minAngularDistClassID = 0;
  double meanValue = 0;
  double dataValue = 0;
  
  for( rasterLine = 0 ; rasterLine < rasterLines ; ++rasterLine )
  {
    if( progress.Increment() || abortFlag_ )
    { 
      TEAGN_LOGMSG( "Canceled by the user" );
      abortFlag_ = true;
      return;
    }
    else
    {
      inMutex_.lock();
      
      if( linesProcStatusVec_[ rasterLine ] == 0 )
      {
        linesProcStatusVec_[ rasterLine ] = 1;
        
        // load line data into lineData matrix (all used bands)
        
        for( inRasterBandsIdx = 0 ; inRasterBandsIdx < inRasterBandsSize ;
          ++inRasterBandsIdx )
        {
          inRasterBand = inRasterBands_[ inRasterBandsIdx ];
          
          for( rasterCol = 0 ; rasterCol < rasterCols ; ++rasterCol )
          {
            inRaster_.getElement( rasterCol, rasterLine, lineDataMtx[ 
              inRasterBandsIdx ][ rasterCol ], inRasterBand );
          }
        }
        
        inMutex_.unLock();
        
        // Classifying each pixel of the loaded line
        
        for( lDMCol = 0 ; lDMCol < rasterCols ; ++lDMCol )
        {
          // looking for the best class (min angular distance)
          
          minAngularDist = DBL_MAX;
          minAngularDistClassID = 0;          
          
          for(  refDataVectorIdx = 0 ;  refDataVectorIdx <  refDataVectorSize ;
            ++refDataVectorIdx )
          {
            const ClassReferenceData& classRefData = 
              refDataVector_[ refDataVectorIdx ];
              
            angularTR = 0;
            angularTT = 0;
            angularRR = 0;   
            
            for( lDMLine = 0 ; lDMLine < inRasterBandsSize ; ++lDMLine )
            {
              meanValue = classRefData.samplesMeansVec_[ lDMLine ];
              dataValue = lineDataMtx[ lDMLine ][ lDMCol ];
              
              angularTR += dataValue * meanValue;
              angularRR += meanValue * meanValue;
              angularTT += dataValue * dataValue;
            }
            
            angularDist = angularTR / ( sqrt( angularTT ) *
              sqrt( angularRR ) );
            if( ABS( angularDist ) > 1.0 )
            {
              angularDist = DBL_MAX;
            }
            else
            {
              angularDist = acos( angularDist );
            }
          
            if( ( angularDist < minAngularDist ) && ( angularDist < 
              classRefData.maxAngularDist_ ) )
            {
              minAngularDist = angularDist;
              minAngularDistClassID = classRefData.classId_;
            }
          }
          
          // Saving the best class id to the output result vector
          
          classifyingResultVector[ lDMCol ] = minAngularDistClassID;
        }
        
        // Saving the classification result for the current line
        
        outMutex_.lock();
        
        for( rasterCol = 0 ; rasterCol < rasterCols ; ++rasterCol )
        {
          if( ! outRaster_.setElement( rasterCol, rasterLine, 
            classifyingResultVector[ rasterCol ], 0 ) )
          {
            outMutex_.unLock();
            TEAGN_LOGMSG( "Error writing to the output raster" );
            abortFlag_ = true;
            return;
          }
        }   
             
        outMutex_.unLock();
      }
      else
      {
        inMutex_.unLock();
      }
    }
  }

  returnValue_ = true;
}

TePDISAMClassifier::TePDISAMClassifier()
{
};

TePDISAMClassifier::~TePDISAMClassifier()
{
};

bool TePDISAMClassifier::CheckParameters(const TePDIParameters& parameters) const
{
  // Checking input_raster
  
  TePDITypes::TePDIRasterPtrType input_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_raster", input_raster), 
    "Missing parameter: input_raster");
  TEAGN_TRUE_OR_RETURN( input_raster.isActive(), 
    "Invalid parameter: input_raster" );
  TEAGN_TRUE_OR_RETURN( input_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: input_raster" );    

  /* bands parameter checking */

  std::vector< unsigned int > bands;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "bands", bands ), 
    "Missing parameter: bands" );
  for( unsigned int bands_index = 0 ; bands_index < bands.size() ; 
    ++bands_index ) 
  {
    TEAGN_TRUE_OR_RETURN(
      ( (int)bands[ bands_index ] < input_raster->nBands() ),
      "Invalid parameter: channels" );
  }
  
  // Checking output_raster
  
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_raster", output_raster), 
    "Missing parameter: output_raster");
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(), 
    "Invalid parameter: output_raster" );
  TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: output_raster" );   
    
  // Checking spectral_samples
    
  SpectralSamplesVecPtrT spectral_samples;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("spectral_samples", 
    spectral_samples), "Missing parameter: spectral_samples");
  TEAGN_TRUE_OR_RETURN( spectral_samples.isActive(), 
    "Invalid parameter: spectral_samples" );
  TEAGN_TRUE_OR_RETURN( spectral_samples->size(), 
    "Invalid parameter: spectral_samples" );
    
  return true;
}

void TePDISAMClassifier::ResetState( const TePDIParameters& )
{
}

bool TePDISAMClassifier::RunImplementation()
{
  // Extracting parameters
  
  TePDITypes::TePDIRasterPtrType input_raster;
  params_.GetParameter("input_raster", input_raster);  
  
  std::vector< unsigned int > bands;
  params_.GetParameter("bands", bands);  
    
  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter("output_raster", output_raster);
  
  SpectralSamplesVecPtrT spectral_samples;
  params_.GetParameter("spectral_samples", spectral_samples);  
  
  // Calc samples means
  
  std::vector< ClassReferenceData > refDataVector;
  TEAGN_TRUE_OR_RETURN( calcClassRefData( *spectral_samples, refDataVector ), 
    "Internal error" );
    
  // Init the output image
  
  TeRasterParams outputImgParams = output_raster->params();
  outputImgParams.nBands( 1 );
  outputImgParams.setDataType( TeUNSIGNEDLONG, -1 );
  outputImgParams.projection( input_raster->params().projection() );
  outputImgParams.boundingBoxLinesColumns( 
    input_raster->params().boundingBox().x1(), 
    input_raster->params().boundingBox().y1(), 
    input_raster->params().boundingBox().x2(), 
    input_raster->params().boundingBox().y2(), 
    input_raster->params().nlines_, 
    input_raster->params().ncols_ );    
  
  TEAGN_TRUE_OR_RETURN( output_raster->init( outputImgParams ),
    "Output raster init error" ); 
     
  // Starting segmentation threads
  
  TeMutex inMutex;
  TeMutex& outMutex = inMutex;
  volatile bool abortFlag = false;
  const unsigned int maxSegThreads = TeGetPhysProcNumber();
  
  std::vector< bool > linesProcStatusVec;
  linesProcStatusVec.resize( input_raster->params().nlines_, 0 );
  
  if( params_.CheckParameter< int >( "enable_multi_thread" ) &&
    ( maxSegThreads > 1 ) )
  {
    std::vector< TeSharedPtr< SegThread > > threadsVec;
    
    unsigned int threadIdx = 0;
    
    for( threadIdx = 0 ; threadIdx < maxSegThreads ; ++threadIdx )
    {
      TeSharedPtr< SegThread > threadPtr( new SegThread( progress_enabled_,
        *input_raster, bands, refDataVector, inMutex, outMutex, 
        linesProcStatusVec, abortFlag, *output_raster ) );
      threadPtr->start();
      threadsVec.push_back( threadPtr );
    }
  
    // Wait all threads to finish
    
    bool returnValue = true;
    
    for( threadIdx = 0 ; threadIdx < maxSegThreads ; ++threadIdx )
    {
      threadsVec[ threadIdx ]->waitToFinish();
      returnValue &= threadsVec[ threadIdx ]->getReturnValue();
    }    
    
    return returnValue;  
  }
  else
  {
    SegThread threadInstance( progress_enabled_, *input_raster, bands, 
      refDataVector, inMutex, outMutex, linesProcStatusVec, abortFlag, 
      *output_raster );
      
    threadInstance.run();
    
    return threadInstance.getReturnValue();
  }
}

bool TePDISAMClassifier::calcClassRefData( const SpectralSamplesVectorT& 
  spectralSamples, std::vector< ClassReferenceData >& refDataVector ) const
{
  refDataVector.clear();
  
  const ClassReferenceData dummyRefData;
  
  for( unsigned int sSIdx = 0 ; sSIdx < spectralSamples.size() ; ++sSIdx )
  {
    const ClassSpectralSamples& cSSamples = spectralSamples[ sSIdx ];
    const unsigned int mCols = cSSamples.samplesMatrix_.GetColumns();
    const unsigned int mLines = cSSamples.samplesMatrix_.GetLines();
    unsigned int sMtxCol = 0;
    unsigned int sMtxLine = 0;
    double colMean = 0;
    
    refDataVector.push_back( dummyRefData );
    ClassReferenceData& newRefData = refDataVector[ refDataVector.size() - 1 ];
    
    TEAGN_TRUE_OR_RETURN( cSSamples.classId_ > 0, "Invalid class ID" );
    newRefData.classId_ = cSSamples.classId_;
    
    newRefData.maxAngularDist_ = cSSamples.maxAngularDist_;
    
    for( sMtxCol = 0 ; sMtxCol < mCols ; ++sMtxCol )
    {
      colMean = 0;
      
      for( sMtxLine = 0 ; sMtxLine < mLines ; ++sMtxLine )
      {
        colMean += cSSamples.samplesMatrix_[ sMtxLine ][ sMtxCol ];
      }
    
      newRefData.samplesMeansVec_.push_back( colMean / ((double)mLines) );
    }
  }
  
  return true;
}
