#include "TePDIStatistic.hpp"

#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeGeometryAlgorithms.h"

#include <math.h>
#include <float.h>


TePDIStatistic::TePDIStatistic()
{
  iterator_strat_ = TeBoxPixelIn;
  
  polygonset_.reset( new TePolygonSet );
}


TePDIStatistic::~TePDIStatistic()
{
  clear();
}


void TePDIStatistic::ResetState( const TePDIParameters& params )
{
  if( params_ != params ) {
    clear();
    
    /* Updating the iterator strategy ( if available ) */
    
    if( params.CheckParameter< TeStrategicIterator > ( 
      "iterator_strat" ) ) {    
      
      params.GetParameter( "iterator_strat", iterator_strat_ );   
    }
    
    
    /* Extracting parameters */
    
    TEAGN_TRUE_OR_THROW( 
      params.GetParameter( "rasters", rasters_ ),
      "Missing parameter: rasters" );
    
    TEAGN_TRUE_OR_THROW( 
      params.GetParameter( "bands", bands_ ),
      "Missing parameter: bands" );     
    
    /* updating the local polygon set pointer */
    
    if( params.CheckParameter< TePDITypes::TePDIPolygonSetPtrType > ( 
      "polygonset" ) ) {
      
      params.GetParameter( "polygonset", polygonset_ );
    } else {
      polygonset_.reset( new TePolygonSet );
      
      TeBox rasterbox = rasters_[ 0 ]->params().boundingBox();
      TePolygon rasterpol = polygonFromBox( rasterbox );
      
      polygonset_->add( rasterpol );
    }
    
    /* Building histogram cache if histograms are provided */
  
    if( params.CheckParameter< std::vector< TePDIHistogram::pointer > >( 
      "histograms" ) ) {
      
      std::vector< TePDIHistogram::pointer > histograms;
      TEAGN_TRUE_OR_THROW( params.GetParameter( "histograms", 
        histograms ), "Missing parameter: histograms" );
      TePDIHistogram* histoPtr = 0;
        
      for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) 
      {
        SingleRasterCachesKeyT key;
        key.first = index;
        key.second = 0;
        
        histoPtr = new TePDIHistogram;
        (*histoPtr) = (*histograms[ index ]);
      
        histo_cache_[ key ] = histoPtr;
      }
    }
  }
}


bool TePDIStatistic::RunImplementation()
{
  TEAGN_LOG_AND_THROW( "This function cannot be used for this class" );
  return false;
}


bool TePDIStatistic::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking input rasters */
  
  TePDITypes::TePDIRasterVectorType rasters;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "rasters", rasters ),
    "Missing parameter: rasters" );
    
  std::vector< int > bands;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "bands", bands ),
    "Missing parameter: bands" );
    
  TEAGN_TRUE_OR_RETURN( rasters.size() == bands.size(),
    "The number of rasters doesn't match the number of bands" );
    
  TEAGN_TRUE_OR_RETURN( rasters.size() > 0, "Invalid number of rasters" );
    
  for( unsigned int index = 0 ; index < rasters.size() ; ++index ) {
    TEAGN_TRUE_OR_RETURN( rasters[ index ].isActive(),
      "Invalid parameter: raster " + 
      Te2String( index ) + " inactive" );
      
    TEAGN_TRUE_OR_RETURN( 
      rasters[ index ]->params().status_ != TeRasterParams::TeNotReady,
      "Invalid parameter: raster " + 
      Te2String( index ) + " not ready" );
            
    TEAGN_TRUE_OR_RETURN( 
      bands[ index ] >= 0, "Invalid band number (" + 
      Te2String( index ) + ")" );
    TEAGN_TRUE_OR_RETURN( 
      bands[ index ] < rasters[ index ]->nBands(), "Invalid band number (" + 
      Te2String( index ) + ")" );
      
    /* Checking photometric interpretation */
    
    TEAGN_TRUE_OR_RETURN( ( 
      ( rasters[ index ]->params().photometric_[ 
        bands[ index ] ] == TeRasterParams::TeRGB ) ||
      ( rasters[ index ]->params().photometric_[ 
        bands[ index ] ] == TeRasterParams::TeMultiBand ) ),
      "Invalid parameter - rasters (invalid photometric "
      "interpretation)" );      
  }
  
  /* Checking the restriction polygon set if avaliable */
  
  TePDITypes::TePDIPolygonSetPtrType polygonset;  
  
  if( parameters.CheckParameter< TePDITypes::TePDIPolygonSetPtrType >( 
    "polygonset" ) ) {
    
    parameters.GetParameter( "polygonset", polygonset );
    
    TEAGN_TRUE_OR_RETURN( polygonset.isActive(), 
      "Invalid parameter : polygonset" );
      
    TEAGN_TRUE_OR_RETURN( ( polygonset->size() > 0 ), 
      "Invalid parameter : polygonset" );      
      
    TeBox rasterbbox = rasters[ 0 ]->params().boundingBox();
    TePolygon raster_bpol = polygonFromBox( rasterbbox );
    
    TePolygonSet::iterator it = polygonset->begin();
    TePolygonSet::iterator it_end = polygonset->end();
    
    while( it != it_end ) {
      TEAGN_TRUE_OR_RETURN( 
        ( TeRelation( raster_bpol, *it ) != TeDISJOINT ),
        "Invalid parameter : polygonset" );
      
      ++it;
    }
  }
  
  /* Checking user histograms if available */
  
  if( parameters.CheckParameter< std::vector< TePDIHistogram::pointer > > ( 
    "histograms" ) ) {
    
    TEAGN_TRUE_OR_RETURN( ( ! polygonset.isActive() ),
      "Invalid parameter: histograms cannot be used when polygonset is "
      "present" );
    
    std::vector< TePDIHistogram::pointer > histograms;
    
    TEAGN_TRUE_OR_THROW( parameters.GetParameter( "histograms", 
      histograms ), "Invalid parameter: histograms" );
    TEAGN_TRUE_OR_RETURN( ( histograms.size() == rasters.size() ),
      "Invalid parameter: histograms" );
      
    std::vector< TePDIHistogram::pointer >::iterator it =
      histograms.begin();
    std::vector< TePDIHistogram::pointer >::iterator it_end =
      histograms.end();
      
    while( it != it_end ) {
      TEAGN_TRUE_OR_RETURN( ( (*it)->size() > 0 ),
        "Invalid parameter: histograms" );    
        
      ++it;
    }
  }  
  
  return true;
}


const TePDIHistogram& TePDIStatistic::getHistogram( 
  unsigned int raster_index,  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  std::pair< unsigned int, unsigned int  > temp_pair;
  temp_pair.first = raster_index;
  temp_pair.second = pol_index;
  
  HistoCacheT::const_iterator it_cache = 
    histo_cache_.find( temp_pair );
    
  if( it_cache == histo_cache_.end() ) {
    unsigned int histo_levels = 0; // auto levels
    
    if( ( rasters_[ raster_index ]->params().dataType_[ bands_[ raster_index ] ]
      == TeFLOAT ) ||
      ( rasters_[ raster_index ]->params().dataType_[ bands_[ raster_index ] ]
      == TeDOUBLE ) ) 
    {
      histo_levels = 256;
    }
        
    TePDITypes::TePDIPolygonSetPtrType local_polygonset( new TePolygonSet );
    local_polygonset->add( polygonset_->operator[]( pol_index ) );
    
    TePDIHistogram* output_histogram = new TePDIHistogram;
    output_histogram->ToggleProgressInt(progress_enabled_);
    
    TEAGN_TRUE_OR_LOG( output_histogram->reset( 
      rasters_[ raster_index ],
      bands_[ raster_index ], histo_levels, iterator_strat_,
      local_polygonset ),
      "Histogram generation error" );
      
    histo_cache_[ temp_pair ] = output_histogram;
      
    return *(histo_cache_[ temp_pair ]);
  } else {
    return *(it_cache->second);
  }
}

const TePDIJointHistogram& TePDIStatistic::getJointHistogram( 
  unsigned int raster1_index, unsigned int raster2_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster1_index < rasters_.size(), 
    "Invalid raster 1 index" );
  TEAGN_TRUE_OR_THROW( raster2_index < rasters_.size(), 
    "Invalid raster 2 index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  CoupleRasterCachesKeyT temp_pair;
  temp_pair.first = raster1_index;
  temp_pair.second.first = raster2_index;
  temp_pair.second.second = pol_index;
  
  JHistoCacheT::iterator it_cache = jHistoCache_.find( temp_pair );
    
  if( it_cache == jHistoCache_.end() ) 
  {
    // Cleaning the inverse histogram if it exists */
    
    CoupleRasterCachesKeyT inv_temp_pair;
    inv_temp_pair.first = raster2_index;
    inv_temp_pair.second.first = raster1_index;
    inv_temp_pair.second.second = pol_index;  
    
    if( jHistoCache_[ inv_temp_pair ] ) 
      delete jHistoCache_[ inv_temp_pair ];
      
    // Generating the joint histogram
  
    unsigned int histo_levels = 0; // auto levels
    
    if( ( rasters_[ raster1_index ]->params().dataType_[ bands_[ raster1_index ] ]
      == TeFLOAT ) ||
      ( rasters_[ raster1_index ]->params().dataType_[ bands_[ raster1_index ] ]
      == TeDOUBLE ) ||
      ( rasters_[ raster2_index ]->params().dataType_[ bands_[ raster2_index ] ]
      == TeFLOAT ) ||
      ( rasters_[ raster2_index ]->params().dataType_[ bands_[ raster2_index ] ]
      == TeDOUBLE ) ) 
    {
      histo_levels = 256;
    }
      
    TePolygonSet local_polygonset;
    local_polygonset.add( polygonset_->operator[]( pol_index ) );

    TePDIJointHistogram* newR1R2HistoPtr = new TePDIJointHistogram;
    newR1R2HistoPtr->toggleProgress( progress_enabled_ );
      
    TEAGN_TRUE_OR_LOG( newR1R2HistoPtr->update( *rasters_[ raster1_index ],
      bands_[ raster1_index ], *rasters_[ raster2_index ],
      bands_[ raster2_index ], iterator_strat_, histo_levels, local_polygonset ),
      "Joint Histogram generation error" );
        
    jHistoCache_[ temp_pair ] = newR1R2HistoPtr;

    // Updating each raster histogram cache too

    TePDIHistogram* r1HistPtr = new TePDIHistogram;
    *r1HistPtr = newR1R2HistoPtr->getRaster1Hist();
    SingleRasterCachesKeyT r1key;
    r1key.first = raster1_index;
    r1key.second = pol_index;
    if( histo_cache_[ r1key ] != 0 ) delete histo_cache_[ r1key ];
    histo_cache_[ r1key ] = r1HistPtr;

    TePDIHistogram* r2HistPtr( new TePDIHistogram );
    *r2HistPtr = newR1R2HistoPtr->getRaster2Hist();
    SingleRasterCachesKeyT r2key;
    r2key.first = raster2_index;
    r2key.second = pol_index;
    if( histo_cache_[ r2key ] != 0 ) delete histo_cache_[ r2key ];
    histo_cache_[ r2key ] = r2HistPtr;
    
    // Generating the joint histogram of raster2 x raster1 using the
    // joint histogram of raster1 x raster 2

    TePDIJointHistogram* newR2R1HistoPtr = new TePDIJointHistogram;
    newR1R2HistoPtr->getInverseJHist( *newR2R1HistoPtr );
    
    jHistoCache_[ inv_temp_pair ] = newR2R1HistoPtr;    
      
    return *newR1R2HistoPtr;
  } else {
    return *(it_cache->second);
  }
}

double TePDIStatistic::getSum( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {
    TePDIHistogram::const_iterator it_hist = hist.begin();
    TePDIHistogram::const_iterator it_hist_end = hist.end();
     
    double result = 0;
          
    while( it_hist != it_hist_end ) {
      result += it_hist->first * (double)it_hist->second;
          
      ++it_hist;
    }
          
    return result;
  }
}


double TePDIStatistic::getSum3( unsigned int raster_index, 
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {  
    TePDIHistogram::const_iterator it = hist.begin();
    TePDIHistogram::const_iterator it_end = hist.end();
    
    double sum = 0;
    double value = 0;
      
    while( it != it_end ) {
      value = it->first;
      
      sum += ( value * value * value * ( ( double ) it->second ) );
      
      ++it;
    }
    
    return sum;     
  }
}


double TePDIStatistic::getSum4( unsigned int raster_index, 
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {  
    TePDIHistogram::const_iterator it = hist.begin();
    TePDIHistogram::const_iterator it_end = hist.end();
    
    double sum = 0;
    double value = 0;
      
    while( it != it_end ) {
      value = it->first;
      
      sum += ( value * value * value * value * ( ( double ) it->second ) );
      
      ++it;
    }
    
    return sum;  
  }
}

double TePDIStatistic::getSumPB1B2( unsigned int raster1_index,
  unsigned int raster2_index, unsigned int pol_index )
{
  const TePDIJointHistogram& jHisto = getJointHistogram( raster1_index,
    raster2_index, pol_index );

  TePDIJointHistogram::const_iterator it = jHisto.begin();
  TePDIJointHistogram::const_iterator it_end = jHisto.end();

  double sum = 0;

  while( it != it_end )
  {
    sum += ( it->first.first * it->first.second ) * (double)it->second;
    ++it;
  }

  return sum;
}


double TePDIStatistic::getMean( unsigned int raster_index, 
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), "Invalid index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  /* Trying to use the histogram method */
  
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {    
    TePDIHistogram::const_iterator it_hist = hist.begin();
    TePDIHistogram::const_iterator it_hist_end = hist.end();
     
    double sum = 0;
    double elem_nmb = 0.0;
          
    while( it_hist != it_hist_end ) {
      sum += it_hist->first * ( (double)it_hist->second );
      elem_nmb += (double)it_hist->second;
          
      ++it_hist;
    }
        
    if( elem_nmb != 0.0 ) {
      return ( sum / ( (double)elem_nmb ) ) ;
    } else {
      return DBL_MAX;
    }
  }
}


TeMatrix TePDIStatistic::getMeanMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );

  TeMatrix outmatrix;
  outmatrix.Init( 1, rasters_.size(), 0. );

  for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) {
    outmatrix( 0, index ) = getMean( index, pol_index );
  }

  return outmatrix;
}


double TePDIStatistic::getCovariance( unsigned int raster1_index,  
  unsigned int raster2_index, unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster1_index < rasters_.size(), 
    "Invalid raster 1 index" );
  TEAGN_TRUE_OR_THROW( raster2_index < rasters_.size(), 
    "Invalid raster 2 index" );    
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  const TePDIJointHistogram& jHist = getJointHistogram( raster1_index, 
    raster2_index, pol_index );
    
  double mean1 = getMean( raster1_index );
  double mean2 = getMean( raster2_index );  
  
  if( ( mean1 == DBL_MAX ) || ( mean2 == DBL_MAX) )
  {
    return DBL_MAX;
  }
    
  /* Finding the elements number */
  
  double elem_nmb = (double)jHist.getFreqSum();
  
  if( elem_nmb != 0.0 ) 
  {
    double covariance = 0;  
    TePDIJointHistogram::const_iterator jHIt = jHist.begin();
    TePDIJointHistogram::const_iterator jHItEnd = jHist.end();
    
    while( jHIt != jHItEnd )
    {
      if( jHIt->second )
      {
        covariance += ( ( jHIt->first.first - mean1 ) *
          ( jHIt->first.second - mean2 ) *
          ( (double)jHIt->second ) );
      }
      
      ++jHIt;
    }
    
    covariance /= elem_nmb;
    
    return covariance;
  } 
  else 
  {
    return DBL_MAX;
  }
}


double TePDIStatistic::getVariance( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  double elements_number = (double)hist.getTotalCount();    
  
  if( elements_number == 0.0 ) {
    return DBL_MAX;
  } else {
    /* Calculating variance */
      
    TePDIHistogram::const_iterator it_hist = hist.begin();
    TePDIHistogram::const_iterator it_hist_end = hist.end();
    
    double mean = getMean( raster_index, pol_index );
    double variance = 0;
    
    while( it_hist != it_hist_end ) {
      variance += ( ( (double)it_hist->second ) / elements_number ) *
        ( it_hist->first - mean ) * ( it_hist->first - mean );
      
      ++it_hist;
    }
    
    return variance;  
  }
}


double TePDIStatistic::getStdDev( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  double variance = getVariance( raster_index, pol_index );
    
  if( variance == DBL_MAX )
  {
    return DBL_MAX;
  }
  else
  {
    return sqrt( variance );
  }
}


double TePDIStatistic::getEntropy( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
  
  double elements_number = (double)hist.getTotalCount();
  
  if( elements_number == 0.0 ) {
    return DBL_MAX;
  }  
  
  /* Calculating entropy */
    
  TePDIHistogram::const_iterator it = hist.begin();
  TePDIHistogram::const_iterator it_end = hist.end();
  
  double entropy = 0;
  double probability = 0;
  
  while( it != it_end ) {
    probability = ( (double)(it->second) ) / elements_number;
    
    if( probability > 0.0 ) {
      entropy += ( probability * ( log( probability ) /
        log( (double)2 ) ) );
    }

    ++it;
  }
  
  entropy = (-1.0) * entropy;

  return entropy;
}


double TePDIStatistic::getMin( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {
    return hist.GetMinLevel();
  }
}


double TePDIStatistic::getMax( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  const TePDIHistogram& hist = getHistogram( raster_index, 
    pol_index );
    
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {
    return hist.GetMaxLevel();
  }
}


double TePDIStatistic::getMode( unsigned int raster_index,
  unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster_index < rasters_.size(), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  const TePDIHistogram& hist = getHistogram( raster_index, pol_index );
  
  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else
  {
    TePDIHistogram::const_iterator it_hist = hist.begin();
    TePDIHistogram::const_iterator it_hist_end = hist.end();
    
    unsigned int frequency = 0;
    double mode = 0;
    
    while( it_hist != it_hist_end ) {
      if( it_hist->second > frequency ) {
        mode = it_hist->first;
        frequency = it_hist->second;
      }
      
      ++it_hist;
    }
    
    return mode;
  }
}


double TePDIStatistic::getCorrelation( unsigned int raster1_index,  
  unsigned int raster2_index, unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( raster1_index < rasters_.size(), 
    "Invalid raster 1 index" );
  TEAGN_TRUE_OR_THROW( raster2_index < rasters_.size(), 
    "Invalid raster 2 index" );    
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  double covariance = getCovariance( raster1_index, raster2_index, 
    pol_index );
    
  if( covariance == DBL_MAX )
  {
    return DBL_MAX;
  }
  else
  {
    double cov1 = getCovariance( raster1_index, raster1_index, 
      pol_index );
    double cov2 = getCovariance( raster2_index, raster2_index, 
      pol_index );
      
    if( ( cov1 == DBL_MAX ) || ( cov2 == DBL_MAX ) )
    {
      return DBL_MAX;
    }
    else
    {
      double sqrt_c_1 = sqrt( cov1 );
      double sqrt_c_2 = sqrt( cov2 );
      double multi = ( sqrt_c_1 * sqrt_c_2 );

      if( multi == 0.0 ) {
        return DBL_MAX;
      } else {
        return ( covariance / multi );
      }
    }
  }
}



double TePDIStatistic::getPercentile( double sample_index, 
  unsigned int raster_index, unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( ( raster_index < rasters_.size() ), 
    "Invalid raster index" );
  TEAGN_TRUE_OR_THROW( ( pol_index < polygonset_->size() ), 
    "Invalid polygon index" );
  TEAGN_TRUE_OR_THROW( ( ( sample_index >= 0.0 ) && 
    ( sample_index <= 100.0 ) ), "Invalid sample index" );    
  
  const TePDIHistogram& hist = getHistogram( raster_index, pol_index );

  if( hist.size() == 0 )
  {
    return DBL_MAX;
  }
  else if( hist.size() == 1 ) 
  {
    return hist.begin()->first;
  } 
  else 
  {
    TePDIHistogram::const_iterator it;
    TePDIHistogram::const_iterator it_end = hist.end();
    
    unsigned int lastindex = hist.getTotalCount() - 1;
    
    double target_sample_index_double = 
      ( (double)( lastindex ) ) * ( sample_index / 100.0 );
    unsigned int target_sample_index_floor = 
      (unsigned int)ceil( target_sample_index_double );
    
    if( target_sample_index_double == 
      ( (double) target_sample_index_floor ) ) {
      
      it = hist.begin();
      
      unsigned int counted_elements = 0;
      unsigned int target_index1 = target_sample_index_floor;
      unsigned int target_index2 = ( target_index1 == lastindex ) ?
        target_index1 : ( target_index1 + 1 );
      double target1_value = 0;
      double target2_value = 0;
      unsigned int curr_index_range_bound = 0;
      
      while( it != it_end ) {
        curr_index_range_bound = counted_elements + it->second;
        
        if( ( counted_elements <= target_index1 ) &&
          ( target_index1 < curr_index_range_bound ) ) {
          
          target1_value = it->first;
        }
        
        if( ( counted_elements <= target_index2 ) &&
          ( target_index2 < curr_index_range_bound ) ) {
          
          target2_value = it->first;
          
          break;
        }
        
        counted_elements += it->second;
        
        ++it;
      }
      
      return ( ( target1_value + target2_value ) / 2.0 );
    } else {
      it = hist.begin();
      
      unsigned int counted_elements = 0;
      unsigned int target_index1 = target_sample_index_floor + 1;
      unsigned int curr_index_range_bound = 0;
      
      while( it != it_end ) {
        curr_index_range_bound = counted_elements + it->second;
        
        if( ( counted_elements <= target_index1 ) &&
          ( target_index1 < curr_index_range_bound ) ) {
          
          return it->first;
        }
        
        counted_elements += it->second;
        
        ++it;
      }
      
      TEAGN_LOG_AND_THROW( "Target value not found" );
      
      return 0;
    }
  }
}

TeMatrix TePDIStatistic::getCMMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( 3, rasters_.size(), 0. );

  double sum3 = 0;
  double sum4 = 0;
  double mean = 0;
  double sumpb1b2 = 0;
  double sumpb1b2_norm = 0;
  double pixels_nmb = 0;

  for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) {
    sumpb1b2 = getSumPB1B2( index, index, pol_index );
    const TePDIHistogram& histo = getHistogram( index, pol_index );
    
    if( histo.size() == 0 )
    {
      outmatrix( 0, index ) = DBL_MAX;
      outmatrix( 1, index ) = DBL_MAX;
      outmatrix( 2, index ) = DBL_MAX;
    }
    else
    {
      sum3 = getSum3( index, pol_index );
      sum4 = getSum4( index, pol_index );
      mean = getMean( index, pol_index );
      pixels_nmb = (double)histo.getTotalCount();
      sumpb1b2_norm = sumpb1b2 / pixels_nmb;    
      
      outmatrix( 0, index ) = getCovariance( index, index, pol_index );

      if( pixels_nmb == 0 ) {
        outmatrix( 1, index ) = DBL_MAX;
      } else {
        outmatrix( 1, index ) = sum3 / pixels_nmb - 3 * mean * sumpb1b2_norm +
                              2 * pow( mean, 3 );
      }

      if( pixels_nmb == 0 ) {
        outmatrix( 2, index ) = DBL_MAX;
      } else {
        outmatrix( 2, index ) = sum4 / pixels_nmb - 4 * mean * sum3 / pixels_nmb +
                              6 * pow( mean, 2 ) * sumpb1b2_norm -
                              3 * pow( mean, 4 );
      }
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getAssimetryMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );

  TeMatrix outmatrix;
  outmatrix.Init( 1, rasters_.size(), 0. );

  double sum3 = 0;
  double mean = 0;
  double cov = 0;
  double sigma = 0;
  double sigma3 = 0;
  double pixels_nmb = 0;
  double sumpb1b2 = 0;
  double sumpb1b2_norm = 0;  

  for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) {
    cov = getCovariance( index, index, pol_index );
    const TePDIHistogram& histo = getHistogram( index, pol_index );
    
    if( histo.size() == 0 )
    {
      outmatrix( 0, index ) = DBL_MAX;
    }
    else
    {
      sigma = sqrt( cov );
      sigma3 = sigma * sigma * sigma;

      if( sigma3 != 0 ) {
        sumpb1b2 = getSumPB1B2( index, index, pol_index );
        sum3 = getSum3( index, pol_index );
        mean = getMean( index, pol_index );
        pixels_nmb = (double)histo.getTotalCount();
        sumpb1b2_norm = sumpb1b2 / pixels_nmb;      
        
        if( ( pixels_nmb == 0 ) || ( sigma3 == 0 ) ) {
          outmatrix( 0, index ) = 0;
        } else {
          outmatrix( 0, index ) = ( sum3 / pixels_nmb -  3 * mean * sumpb1b2_norm +
                                  2 * pow( mean, 3 ) ) / sigma3;
        }
      } else {
        outmatrix( 0, index ) = 0;
      }
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getKurtosisMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );

  TeMatrix outmatrix;
  outmatrix.Init( 1, rasters_.size(), 0. );

  double sum3;
  double sum4;
  double mean;
  double cov;
  double sigma;
  double sigma4;
  double sumpb1b2 = 0;
  double sumpb1b2_norm = 0;
  double pixels_nmb = 0;  

  for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) {
    cov = getCovariance( index, index, pol_index );
    const TePDIHistogram& histo = getHistogram( index, pol_index );
    
    if( histo.size() == 0 )
    {
      outmatrix( 0, index ) = DBL_MAX;
    }
    else
    {
      sigma = sqrt( cov );
      sigma4 = sigma * sigma * sigma * sigma;

      if( sigma4 != 0 ) {
        sumpb1b2 = getSumPB1B2( index, index, pol_index );
        sum3 = getSum3( index, pol_index );
        sum4 = getSum4( index, pol_index );
        mean = getMean( index, pol_index );
        pixels_nmb = (double)histo.getTotalCount();
        sumpb1b2_norm = sumpb1b2 / pixels_nmb;
        
        if( pixels_nmb == 0 ) {
          outmatrix( 0, index ) = DBL_MAX;
        } else {
          outmatrix( 0, index ) = ( sum4 / pixels_nmb - 4 * mean * sum3 /
            pixels_nmb  + 6 * pow( mean, 2 ) * sumpb1b2_norm - 3 * pow( mean, 4 ) ) /
            sigma4 - 3.;
        }
      } else {
        outmatrix( 0, index ) = DBL_MAX;
      }
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getVarCoefMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( 1, rasters_.size(), 0. );

  double mean = 0.0;
  double cov = 0.0;

  for( unsigned int index = 0 ; index < rasters_.size() ; ++index ) {
    mean = getMean( index, pol_index );

    if( mean == DBL_MAX ) 
    {
      outmatrix( 0, index ) = DBL_MAX;
    }
    else if( mean == 0 ) 
    {
      outmatrix( 0, index ) = DBL_MAX;
    }
    else
    {
      cov = getCovariance( index, index, pol_index );
      
      if( cov == DBL_MAX )
      {
        outmatrix( 0, index ) = DBL_MAX;
      }
      else
      {
        outmatrix( 0, index ) = sqrt(
          getCovariance( index, index, pol_index ) ) / mean;
      }
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getCovMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( rasters_.size(), rasters_.size(), 0. );

  for( unsigned int band1 = 0 ; band1 < rasters_.size() ; ++band1 ) {
    for( unsigned int band2 = 0 ; band2 < rasters_.size() ; ++band2 ) {
      outmatrix( band1, band2 ) = 
        getCovariance( band1, band2, pol_index );
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getCorMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( rasters_.size(), rasters_.size(), 0. );

  for( unsigned int band1 = 0 ; band1 < rasters_.size() ; ++band1 ) {
    for( unsigned int band2 = 0 ; band2 < rasters_.size() ; ++band2 ) {
      outmatrix( band1, band2 ) = 
        getCorrelation( band1, band2, pol_index );
    }
  }

  return outmatrix;
}


TeMatrix TePDIStatistic::getVarMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( rasters_.size(), 1, 0. );

  for( unsigned int band = 0 ; band < rasters_.size() ; ++band ) {
    outmatrix( band, 0 ) = getVariance( band, pol_index );
  }

  return outmatrix;
}

TeMatrix TePDIStatistic::getStdDevMatrix( unsigned int pol_index )
{
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), "Invalid index" );
  
  TeMatrix outmatrix;
  outmatrix.Init( rasters_.size(), 1, 0. );

  for( unsigned int band = 0 ; band < rasters_.size() ; ++band ) {
    outmatrix( band, 0 ) = getStdDev( band, pol_index );
  }

  return outmatrix;
}

void TePDIStatistic::clear()
{
  polygonset_.reset();
  rasters_.clear();
  bands_.clear();
  
  // Clean joint histograms

  JHistoCacheT::iterator jhcit =
    jHistoCache_.begin();
  while( jhcit != jHistoCache_.end() )
  {
    delete jhcit->second;
    ++jhcit;
  }
  jHistoCache_.clear();
  
  // Clean histograms
  
  HistoCacheT::iterator histCacheIt = histo_cache_.begin();
  while( histCacheIt != histo_cache_.end() )
  {
    delete histCacheIt->second;
    ++histCacheIt;
  }

  histo_cache_.clear();
}


