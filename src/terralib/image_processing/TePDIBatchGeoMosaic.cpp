#include "TePDIBatchGeoMosaic.hpp"

#include "TePDIGeoMosaic.hpp"
#include "TePDIUtils.hpp"
#include "TePDIBlender.hpp"

#include "../kernel/TeAgnostic.h"
#include "../kernel/TeVectorRemap.h"
#include "../kernel/TeOverlay.h"

TePDIBatchGeoMosaic::RastersListNodeT::RastersListNodeT()
{
}

TePDIBatchGeoMosaic::RastersListNodeT::~RastersListNodeT()
{
}

const TePDIBatchGeoMosaic::RastersListNodeT& 
TePDIBatchGeoMosaic::RastersListNodeT::operator=( const RastersListNodeT& 
  other )
{
  rasterPtr_ = other.rasterPtr_;
  
  dataPolygonRefProj_.clear();
  dataPolygonRefProj_.copyElements( other.dataPolygonRefProj_ );
  
  dataPolygonRaProj_.clear();
  dataPolygonRaProj_.copyElements( other.dataPolygonRaProj_ );  
  
  channelsVec_ = other.channelsVec_;
  
  return *this;
}

TePDIBatchGeoMosaic::TePDIBatchGeoMosaic()
{
}


TePDIBatchGeoMosaic::~TePDIBatchGeoMosaic()
{
}


void TePDIBatchGeoMosaic::ResetState( const TePDIParameters& )
{
}


bool TePDIBatchGeoMosaic::CheckParameters( const TePDIParameters& parameters ) const
{
  unsigned int index = 0;

  /* Checking input_rasters */
  
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_rasters", 
    input_rasters ),
    "Missing parameter: input_rasters" );
  TEAGN_TRUE_OR_RETURN( ( input_rasters.size() > 1 ),
    "Invalid parameter: input_rasters" );
    
  for( index = 0 ; index < input_rasters.size() ; ++index ) {
    TEAGN_TRUE_OR_RETURN( input_rasters[ index ].isActive(),
      "Invalid parameter: input_rasters[" +  Te2String( index ) + 
      "] inactive" );
      
    TEAGN_TRUE_OR_RETURN( input_rasters[ index ]->params().status_ != 
      TeRasterParams::TeNotReady, "Invalid parameter: input_rasters[" +
      Te2String( index ) + "] not ready" );
      
    TEAGN_TRUE_OR_RETURN( ( input_rasters[ index ]->params().projection() != 0 ),
      "Missing input_rasters[" + Te2String( index ) + "] projection" );
      
    TEAGN_TRUE_OR_RETURN( 
      ( input_rasters[ index ]->params().projection()->name() != "NoProjection" ),
      "Invalid input_rasters[" + Te2String( index ) + "] projection" );        
  }
  
  /* Checking output_raster */
  
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "output_raster", 
    output_raster ),
    "Missing parameter: output_raster" );
  TEAGN_TRUE_OR_RETURN( output_raster.isActive(),
    "Invalid parameter: output_raster inactive" );
  TEAGN_TRUE_OR_RETURN( output_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready" );
    
  /* Checking bands */
  
  std::vector< unsigned int > bands;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "bands", 
    bands ), "Missing parameter: bands" );
  TEAGN_TRUE_OR_RETURN( ( bands.size() >= input_rasters.size() ),
    "Invalid parameter: bands" );
  TEAGN_TRUE_OR_RETURN( ( ( bands.size() % input_rasters.size() ) == 0 ),
    "Invalid paramter: bands" );
    
  unsigned int bands_per_raster = bands.size() / input_rasters.size();
    
  for( unsigned int bands_index = 0 ; bands_index < bands.size() ; 
    ++bands_index ) {
    
    unsigned int current_raster_index = (unsigned int) floor( 
      ( (double)bands_index ) / ( (double)bands_per_raster ) );
    TEAGN_TRUE_OR_RETURN( (
      ( (int)bands[ bands_index ] < input_rasters[ 
      current_raster_index ]->nBands() ) ), 
      "Invalid parameter: bands[" + Te2String( bands_index ) + "]" );
  }
    
  return true;
}


bool TePDIBatchGeoMosaic::RunImplementation()
{
  // Extracting parameters
  
  TePDITypes::TePDIRasterVectorType input_rasters;
  params_.GetParameter( "input_rasters", input_rasters );

  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  std::vector< unsigned int > bands;
  params_.GetParameter( "bands", bands );
  
  TePDIBlender::BlendMethod blend_method = TePDIBlender::NoBlendMethod;
  if( params_.CheckParameter< TePDIBlender::BlendMethod >(
    "blend_method" ) )
  {
    params_.GetParameter( "blend_method", blend_method );
  }  
    
  TePDIInterpolator::InterpMethod interp_method = TePDIInterpolator::NNMethod;
  if( params_.CheckParameter< TePDIInterpolator::InterpMethod >(
    "interp_method" ) )
  {
    params_.GetParameter( "interp_method", interp_method );
  }   
   
  bool auto_equalize = false;
  if( params_.CheckParameter< int >( "auto_equalize" ) )
  {
    auto_equalize = true;
  }   
  
  /* Dumyy value definition */
 
  double dummy_value = 0;
  if( params_.CheckParameter< double >( "dummy_value" ) ) 
  {
    params_.GetParameter( "dummy_value", dummy_value );
  }
  else if( output_raster->params().useDummy_ )
  {
    dummy_value = output_raster->params().dummy_[ 0 ];
  }
  
  /* Building a internal rasters list */
  
  TeProjection* refProjPtr = getFreqProjection( input_rasters );
  TEAGN_TRUE_OR_RETURN( refProjPtr, "Invalid reference projection pointer" );
  
  RastersListT rastersList;
  TEAGN_TRUE_OR_RETURN( initRastersList( bands, input_rasters, refProjPtr,
    rastersList ), "Error initializing rasters list" );
  
  /* Processing each pair of rasters... */
  
  RastersListNodeT inputNode1;
  RastersListNodeT inputNode2;
  RastersListNodeT outputNode;
  TePDIGeoMosaic geo_mos_instance;
  TePDIParameters geo_mos_params;
  const unsigned int bandsPerRaster = bands.size() / input_rasters.size();
  
  while( rastersList.size() > 0 ) 
  {
    // Guessing input rasters 
    
    if( rastersList.size() == input_rasters.size() ) 
    {
      //The process starts choosing the two closest rasters to avid the
      //creation of a intermediate mosaic result with disjoint
      //rasters areas
      
      TEAGN_TRUE_OR_RETURN( extractClosestRastersNodes( rastersList,
        inputNode1, inputNode2 ), "Internal processing error" );
    } 
    else
    {
      inputNode1 = outputNode;
      
      TEAGN_TRUE_OR_RETURN( extractCloserRasterNode( rastersList,
        inputNode1, inputNode2 ), "Internal processing error" );
    }
    
    // updating the output raster pointer
    
    if( rastersList.size() == 0 )
    {
      outputNode.rasterPtr_ = output_raster;
    }    
    else
    {
      TeRasterParams current_output_raster_params;
      current_output_raster_params.setDataType( TeUNSIGNEDCHAR, -1 );
      current_output_raster_params.setNLinesNColumns( 1, 1 );
      current_output_raster_params.nBands( 1 );
      current_output_raster_params.setDummy( dummy_value, -1 );
      current_output_raster_params.setPhotometric( 
        TeRasterParams::TeMultiBand, -1 );
        
      outputNode.rasterPtr_.reset();  
      TEAGN_TRUE_OR_RETURN( TePDIUtils::TeAllocRAMRaster( 
        current_output_raster_params, outputNode.rasterPtr_ ), 
        "Unable to allocate temporary mem raster" );
    }
    
    /* Building geomosaic parameters */
    
    geo_mos_params.Clear();
    geo_mos_params.SetParameter( "blend_method", blend_method );  
    geo_mos_params.SetParameter( "input_raster1", inputNode1.rasterPtr_ );
    geo_mos_params.SetParameter( "input_raster2", inputNode2.rasterPtr_ );
    geo_mos_params.SetParameter( "output_raster", outputNode.rasterPtr_ );
    geo_mos_params.SetParameter( "channels1", inputNode1.channelsVec_ );
    geo_mos_params.SetParameter( "channels2", inputNode2.channelsVec_ );
    geo_mos_params.SetParameter( "interp_method", interp_method );
    geo_mos_params.SetParameter( "dummy_value", dummy_value );
    if( auto_equalize )
    {
      geo_mos_params.SetParameter( "auto_equalize", (int)1 );
    }    
    geo_mos_params.SetParameter( "raster1_pol", 
      inputNode1.dataPolygonRaProj_ );
    geo_mos_params.SetParameter( "raster2_pol", 
      inputNode2.dataPolygonRaProj_ );      
    
//TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff(
//  inputNode1.rasterPtr_, "inputRaster1.tif" ),"" );
//TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff(
//  inputNode2.rasterPtr_, "inputRaster2.tif" ),"" );
      
    TEAGN_TRUE_OR_RETURN( geo_mos_instance.Apply( geo_mos_params ),
      "Unable to apply geographic mosaic strategy" );
      
//TEAGN_TRUE_OR_THROW( TePDIUtils::TeRaster2Geotiff(
//  outputNode.rasterPtr_, "outputRaster.tif" ),"" );
      
    TEAGN_DEBUG_CONDITION( outputNode.rasterPtr_->nBands() == 
      (int)bandsPerRaster, "Invalid mosaic result" );
      
    // Updating the output node bands vector
    
    outputNode.channelsVec_.clear();
    for( unsigned int cVIdx = 0 ; cVIdx < bandsPerRaster ; ++cVIdx )
    {
      outputNode.channelsVec_.push_back( cVIdx );
    }
    
    // Updating the output node data polygons
    
    TePolygonSet r1PS;
    r1PS.add( inputNode1.dataPolygonRefProj_ );
    
    TePolygonSet r2PS;
    r2PS.add( inputNode2.dataPolygonRefProj_ );    
    
    TePolygonSet unionPS;
    TEAGN_TRUE_OR_RETURN( TeOVERLAY::TeUnion( r1PS, r2PS, unionPS ),
      "Polygon set union error" );
    
    if( ( unionPS.size() == 1 ) && ( unionPS[ 0 ].size() == 1 ) )
    {
      outputNode.dataPolygonRefProj_ = unionPS[ 0 ];
          
      if( refProjPtr->operator==( *( outputNode.rasterPtr_->params().
        projection() ) ) )
      {
        outputNode.dataPolygonRaProj_ = outputNode.dataPolygonRefProj_;
      }
      else
      {
        TeVectorRemap( outputNode.dataPolygonRefProj_, refProjPtr,
        outputNode.dataPolygonRaProj_, outputNode.rasterPtr_->params().
        projection() );
      }          
    }
    else
    {
      outputNode.dataPolygonRaProj_ = polygonFromBox( 
        outputNode.rasterPtr_->params().boundingBox() );
        
      if( refProjPtr->operator==( *( outputNode.rasterPtr_->params().
        projection() ) ) )
      {
        outputNode.dataPolygonRefProj_ = outputNode.dataPolygonRaProj_;
      }
      else
      {
        TeVectorRemap( outputNode.dataPolygonRaProj_, outputNode.rasterPtr_->
        params().projection(), outputNode.dataPolygonRefProj_,
        refProjPtr );
      }          
    }
  }      
  
  return true;
}

bool TePDIBatchGeoMosaic::getRUsedChannels( 
  const unsigned int& inputRastersIdx,
  const std::vector< unsigned int >& bands,
  const TePDITypes::TePDIRasterVectorType& input_rasters,
  std::vector< unsigned int >& channels ) const
{
  TEAGN_DEBUG_CONDITION( ( inputRastersIdx < input_rasters.size() ),
    "Invalid input_rasters or bands" );
    
  TEAGN_DEBUG_CONDITION( ( bands.size() % input_rasters.size() ) == 0,
    "Invalid input_rasters or bands" );
    
  channels.clear();
    
  unsigned int bands_per_raster = bands.size() / input_rasters.size();
  
  for( unsigned int rasters_index = 0 ; rasters_index < input_rasters.size() ;
    ++rasters_index ) 
  {
    if( rasters_index == inputRastersIdx ) 
    {
      unsigned int bands_start_index = ( rasters_index * bands_per_raster );
      
      for( unsigned int offset = 0 ; offset < bands_per_raster ; ++offset ) 
      {
        channels.push_back(  bands[ bands_start_index + offset ] );
        
        TEAGN_DEBUG_CONDITION( (int)channels.back() <
          input_rasters[ rasters_index ]->params().nBands(),
          "Invalid raster band" );        
      }
      
      return true;
    }
  }
  
  return false;
}

bool TePDIBatchGeoMosaic::initRastersList( 
  const std::vector< unsigned int >& bands,
  const TePDITypes::TePDIRasterVectorType& input_rasters,      
  TeProjection*  referenceProjPtr,
  RastersListT& rList ) const
{
  TEAGN_DEBUG_CONDITION( referenceProjPtr, "Invalid projection pointer" );
  TEAGN_DEBUG_CONDITION( ( bands.size() % input_rasters.size() ) == 0,
    "Invalid input_rasters or bands" );  
  
  rList.clear();
    
  for( unsigned int iRVIdx = 0 ; iRVIdx < input_rasters.size() ; ++iRVIdx )
  {
    RastersListNodeT dummyNode;
    
    dummyNode.rasterPtr_ = input_rasters[ iRVIdx ];
  
    TEAGN_TRUE_OR_RETURN( getRUsedChannels( iRVIdx, bands, input_rasters, 
      dummyNode.channelsVec_ ), "Internal processing error" );
      
    dummyNode.dataPolygonRaProj_ = polygonFromBox( 
      dummyNode.rasterPtr_->params().boundingBox() );
      
    if( referenceProjPtr->operator==( *( dummyNode.rasterPtr_->params().
      projection() ) ) )
    {
      dummyNode.dataPolygonRefProj_ = dummyNode.dataPolygonRaProj_;
    }
    else
    {
      TeVectorRemap( dummyNode.dataPolygonRaProj_, dummyNode.rasterPtr_->
      params().projection(), dummyNode.dataPolygonRefProj_,
      referenceProjPtr );
    }
      
    rList.push_back( dummyNode );
  }
  
  return true;
}

TeProjection* TePDIBatchGeoMosaic::getFreqProjection( 
  const TePDITypes::TePDIRasterVectorType& rastersVec ) const
{
  std::map< std::string, std::pair< unsigned int, TeProjection* > > 
    projsMap;
  std::string projStr;
  
  for( unsigned int rVecIdx = 0 ; rVecIdx < rastersVec.size() ; ++rVecIdx )
  {
    TEAGN_DEBUG_CONDITION( rastersVec[ rVecIdx ]->projection(), 
      "Invalid projection" );
    
    projStr = rastersVec[ rVecIdx ]->projection()->describe();
    
    if( projsMap[ projStr ].second == 0 )
    {
      projsMap[ projStr ].first = 1;
      projsMap[ projStr ].second = rastersVec[ rVecIdx ]->projection();
    }
    else
    {
      ++(projsMap[ projStr ].first);
    }
  }
  
  std::map< std::string, std::pair< unsigned int, 
    TeProjection* > >::iterator mIt = projsMap.begin();
  const std::map< std::string, std::pair< unsigned int, 
    TeProjection* > >::iterator mItEnd = projsMap.end();    
  unsigned int bestProjCount = 0;  
  TeProjection* bestProjPtr = 0;  
    
  if( mIt == mItEnd )
  {
    return 0;
  }
  else
  {
    while( mIt != mItEnd )
    {
      if( mIt->second.first > bestProjCount )
      {
        bestProjCount = mIt->second.first;
        bestProjPtr = mIt->second.second;
      }
      
      ++mIt;
    }
    
    return bestProjPtr;
  }
}

  
bool TePDIBatchGeoMosaic::extractCloserRasterNode( RastersListT& rList,
  const RastersListNodeT& inputNode,
  RastersListNodeT& outputNode ) const
{
  if( rList.size() )
  {
    // Locating the raster closest to the raster bounding box center
    
    double minDist = DBL_MAX;
    double curDist = 0;
    const TeCoord2D iNodeRasterCenter = inputNode.dataPolygonRefProj_.box().center();
    TeCoord2D curCenter;
    
    RastersListT::iterator it = rList.begin();
    const RastersListT::iterator itEnd = rList.end(); 
    RastersListT::iterator foundIt = itEnd;   
    
    while( it != itEnd )
    {
      curCenter = it->dataPolygonRefProj_.box().center();
      
      curDist = TeDistance( curCenter, iNodeRasterCenter );
      
      if( curDist < minDist )
      {
        minDist = curDist;
        foundIt = it;
      }
      
      ++it;
    }
    
    if( foundIt != itEnd )
    {
      outputNode = *foundIt;
      rList.erase( foundIt );
      
      return true;
    }
    else
    {
      return false;
    }    
  }
  else
  {
    return false;
  }
}

bool TePDIBatchGeoMosaic::extractClosestRastersNodes( RastersListT& rList,
   RastersListNodeT& outputNode1, RastersListNodeT& outputNode2 ) const
{
  if( rList.size() )
  {
    // Locating the raster closest to the raster bounding box center
    
    double minDist = DBL_MAX;
    double curDist = 0;
    
    RastersListT::iterator it1 = rList.begin();
    const RastersListT::iterator itEnd = rList.end(); 
    RastersListT::iterator it2;
    RastersListT::iterator foundIt1;
    RastersListT::iterator foundIt2;
    
    while( it1 != itEnd )
    {
      it2 = it1;
      ++it2;
      
      while( it2 != itEnd )
      {
        curDist = TeDistance( it1->dataPolygonRefProj_.box().center(), 
          it2->dataPolygonRefProj_.box().center() );
      
        if( curDist < minDist )
        { 
          minDist = curDist;
          
          foundIt1 = it1;
          foundIt2 = it2;
        }
        
        ++it2;
      }
      
      ++it1;
    }
    
    if( minDist != DBL_MAX )
    {
      outputNode1 = *foundIt1;
      outputNode2 = *foundIt2;
      
      rList.erase( foundIt2 );
      rList.erase( foundIt1 );
      
      return true;
    }
    else
    {
      return false;
    }    
  }
  else
  {
    return false;
  }
}


