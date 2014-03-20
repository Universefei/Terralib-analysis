#include "TePDIHaralick.hpp"

#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeAgnostic.h"

#include <math.h>

#define INSERT_GLCM_ELEMENT( concpixel_line, concpixel_col ) \
  if( input_raster_->getElement( concpixel_col, concpixel_line, \
    auxkey.second, band ) ) \
  { \
    auxkey.first = itp.operator*(band); \
    \
    ++( mat[ auxkey ] ); \
    \
    ++( cacheNode.coOcurrences_ ); \
    \
    inverseAuxKey.first = auxkey.second; \
    inverseAuxKey.second = auxkey.first; \
    \
    ++( mat[ inverseAuxKey ] ); \
    \
    ++( cacheNode.coOcurrences_ ); \
    \
    if( auxkey.first < cacheNode.rasterMinGray_ ) \
      cacheNode.rasterMinGray_ = auxkey.first; \
    if( auxkey.second < cacheNode.rasterMinGray_ ) \
      cacheNode.rasterMinGray_ = auxkey.second; \
    \
    if( auxkey.first > cacheNode.rasterMaxGray_ ) \
      cacheNode.rasterMaxGray_ = auxkey.first; \
    if( auxkey.second > cacheNode.rasterMaxGray_ ) \
      cacheNode.rasterMaxGray_ = auxkey.second; \
  };
  
TePDIHaralick::TePDIHaralick()
{
  polygonset_.reset( new TePolygonSet );
  direction_ = NoDirection;
}


TePDIHaralick::~TePDIHaralick()
{
  clear();
}


void TePDIHaralick::ResetState( const TePDIParameters& params )
{
  clear();
  
  TEAGN_TRUE_OR_THROW( params.GetParameter( "input_raster", input_raster_ ),
    "Missing parameter input_raster" )
  TEAGN_TRUE_OR_THROW( params.GetParameter( "direction", direction_ ),
    "Missing parameter direction" )    
  TEAGN_TRUE_OR_THROW( params.GetParameter( "polygonset", polygonset_ ),
    "Missing parameter polygonset" )
}


bool TePDIHaralick::RunImplementation()
{
  TEAGN_LOG_AND_THROW( "This function cannot be used for this class" );
  return false;
}


bool TePDIHaralick::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking input_raster1 */
  
  TePDITypes::TePDIRasterPtrType input_raster;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_raster", 
    input_raster ),
    "Missing parameter: input_raster" );
  TEAGN_TRUE_OR_RETURN( input_raster.isActive(),
    "Invalid parameter: input_raster inactive" );
  TEAGN_TRUE_OR_RETURN( input_raster->params().status_ != 
    TeRasterParams::TeNotReady, "Invalid parameter: input_raster not ready" );
  
  for( int band = 0 ; band < input_raster->params().nBands() ;
    ++band )
  {
    TEAGN_TRUE_OR_RETURN( input_raster->params().dataType_[ band ] !=
      TeFLOAT, "Invalid raster data type" )
    TEAGN_TRUE_OR_RETURN( input_raster->params().dataType_[ band ] !=
      TeDOUBLE, "Invalid raster data type" )
  }
  
  /* Checking the direction parameter */
  
  DirectionT direction = NoDirection;
  
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "direction", direction ),
    "Missing parameter : direction" );    

  /* Checking the restriction polygon set */
  
  TePDITypes::TePDIPolygonSetPtrType polygonset;  
  
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "polygonset", polygonset ),
    "Missing parameter : polygonset" );
  
  TEAGN_TRUE_OR_RETURN( polygonset.isActive(), 
    "Invalid parameter : polygonset" );
    
  TEAGN_TRUE_OR_RETURN( polygonset->size() > 0, 
    "Invalid parameter : polygonset" );    
      
  return true;
}


bool TePDIHaralick::getGLCMEntropy( unsigned int band, unsigned int pol_index,
  double& entropyValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
   
  if( nodePtr )
  {
    entropyValue = 0.0;
  
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
  
    while ( itFirst != itEnd )
    {
      const double& ocurrences = itFirst->second;
      
      entropyValue += ( ocurrences * log(ocurrences) );
      
      ++itFirst;
    }
    
    entropyValue *= -1.0;
  
    return true;
  }
  else
  {
    return false;
  }
}



bool TePDIHaralick::getGLCMEnergy( unsigned int band, unsigned int pol_index,
  double& energyValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr == 0 )
  {
    return false;
  }
  else
  {
    energyValue = 0.0;
  
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
      
    while ( itFirst != itEnd )
    {
      energyValue += ( (itFirst->second) * (itFirst->second) );
      ++itFirst;
    }
    
    return true;
  }
}


bool TePDIHaralick::getGLCMContrast( unsigned int band, unsigned int pol_index,
  double& contrastValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
  
  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr )
  {
    contrastValue = 0.0;
    
    double dif = 0;
    
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
      
    while ( itFirst != itEnd )
    {
      dif = ((itFirst->first.first) - (itFirst->first.second));
      
      contrastValue += ( dif * dif ) * itFirst->second;
      
      ++itFirst;
    }    
    
    return true;
  }
  else
  {
    return false;
  }
}


bool TePDIHaralick::getGLCMHomogeneity( unsigned int band, unsigned int pol_index,
  double& homogeneityValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr )
  {
    homogeneityValue = 0.0;
    
    // The dif stores difference i - j.
    double dif;
    
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
      
    while ( itFirst != itEnd )
    {
      dif = ((itFirst->first.first) - (itFirst->first.second));
      
      homogeneityValue += ( (itFirst->second) /
        ( 1 + ( dif * dif ) ) );
      
      ++itFirst;
    }
    
    return true;
  }
  else
  {
    return false;
  }
}


bool TePDIHaralick::getGLCMQuiSquare( unsigned int band, unsigned int pol_index,
  double& QuiSquareValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr )
  {
    QuiSquareValue = 0.0;
  
    // total_col stores the sum of the probabilities of column j
    // total_ln stores the sum of the probabilities of line i
    double total_col =  0.0, total_ln = 0.0;
    
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
  
    //iterators and map auxiliaries
    GLCMMatrixT::const_iterator itAux1 = nodePtr->matrix_.begin();
        
    map <double, double> totalLine;
    map <double, double> totalColumn;
  
    while(itAux1 != itEnd)
    { 
      if (totalLine.find(itAux1->first.first)== totalLine.end())
      {
        totalLine[itAux1->first.first] = itAux1->second;
      }
      else
      {
        totalLine[itAux1->first.first] += itAux1->second;
      }
  
      if (totalColumn.find(itAux1->first.second) == totalColumn.end())
      {
        totalColumn[itAux1->first.second] = itAux1->second;
      }
      else
      {
        totalColumn[itAux1->first.second] += itAux1->second;
      }
  
      ++itAux1;
  
    }
    
    while ( itFirst != itEnd )
    {
      total_col = (totalColumn.find(itFirst->first.second))->second;
      total_ln = (totalLine.find(itFirst->first.first))->second;
  
      QuiSquareValue += ((pow(itFirst->second,2)))/( total_col * total_ln);
      ++itFirst;
    }
  
    return true;
  }
  else
  {
    return false;
  }
}

bool TePDIHaralick::getGLCMMean( unsigned int band, unsigned int pol_index,
  double& meanValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr && ( nodePtr->coOcurrences_ ) )
  {
    GLCMMatrixT::const_iterator it = nodePtr->matrix_.begin();
    GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
    
    meanValue = 0;
      
    while ( it != itEnd )
    {
      meanValue += ( it->first.first + it->first.second ) * it->second * 
        ((double)nodePtr->coOcurrences_);
                
      ++it;
    }
    
    meanValue /= ( 2.0 *((double)nodePtr->coOcurrences_) );
    
    return true;
  }
  else
  {
    return false;
  }  
}

bool TePDIHaralick::getGLCMDissimilarity( unsigned int band, 
  unsigned int pol_index, double& dissimilarityValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr )
  {
    if( nodePtr->matrix_.size() )
    {
      dissimilarityValue = 0;
      
      GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
      GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
        
      while ( itFirst != itEnd )
      {
        dissimilarityValue += ( itFirst->second * (
          ABS( itFirst->first.first - itFirst->first.second ) ) );
        
        ++itFirst;
      }
      
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


bool TePDIHaralick::getGLCMStdDev( unsigned int band, 
  unsigned int pol_index, double& stdDevValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );

  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( nodePtr )
  {
    double mean = 0;
    
    if( ( nodePtr->matrix_.size() ) && getGLCMMean( band, pol_index, mean ) )
    {
      stdDevValue = 0;
      
      GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
      GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
      double mean = 0;
        
      while ( itFirst != itEnd )
      {
        stdDevValue += ( itFirst->second * ( itFirst->first.first - mean ) );
        
        ++itFirst;
      }
      
      stdDevValue = sqrt( stdDevValue );
      
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

bool TePDIHaralick::getGLCMCorrelation( unsigned int band, 
  unsigned int pol_index, double& correlationValue )
{
  TEAGN_TRUE_OR_THROW( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_TRUE_OR_THROW( pol_index < polygonset_->size(), 
    "Invalid polygon index" );
    
  double mean = 0;
  double stddev = 0;
  GLCMMatrixCacheNodeT const* nodePtr = getGLCMMatrixNodePtr( band, pol_index );
  
  if( getGLCMMean( band, pol_index, mean ) && getGLCMStdDev( band, pol_index, 
    stddev ) && ( nodePtr != 0 ) )
  {
    // Calc correlation 
    
    correlationValue = 0;
    
    GLCMMatrixT::const_iterator itFirst = nodePtr->matrix_.begin();
    const GLCMMatrixT::const_iterator itEnd = nodePtr->matrix_.end();
    
    double stddev2 = stddev * stddev;
        
    while ( itFirst != itEnd )
    {
      correlationValue += ( ( ( itFirst->first.first - mean ) *
        ( itFirst->first.second - mean ) * itFirst->second ) /
        stddev2 ) ;
      
      ++itFirst;
    }
      
    return true;
  }
  else
  {
    return false;
  }
}


TePDIHaralick::GLCMMatrixCacheNodeT const* TePDIHaralick::getGLCMMatrixNodePtr( 
  unsigned int band, unsigned int pol_index )
{
  TEAGN_DEBUG_CONDITION( ( ((int)band) < input_raster_->params().nBands() ),
    "Invalid raster band" );
  TEAGN_DEBUG_CONDITION( ( pol_index < polygonset_->size() ), 
    "Invalid polygon index" );
    
  GLCMMatrixCacheKeyT key;
  key.first = band;
  key.second = pol_index;
  const GLCMMatrixCacheNodeT dummyNode;
  GLCMMatrixCacheT::iterator it = conc_matrix_cache_.find( key );
  
  if( it == conc_matrix_cache_.end() ) 
  {
    TePolygon& pol = (*polygonset_)[ pol_index ];

    conc_matrix_cache_[ key ] = dummyNode;
    GLCMMatrixCacheNodeT& cacheNode = conc_matrix_cache_[ key ];
  
    TeRaster::iteratorPoly itp = input_raster_->begin(pol,TeBoxPixelIn, band);

    GLCMMatrixT& mat = cacheNode.matrix_;

    GLCMMatrixKeyT auxkey;
    GLCMMatrixKeyT inverseAuxKey;
    
    int curr_line = 0;
    int curr_col = 0;
    
    //enquanto no acabar os pixels do polgono n
    while ( ! itp.end() ) 
    {
      curr_line =  itp.currentLine();
      curr_col = itp.currentColumn();
    
      if( direction_ & North ) 
        INSERT_GLCM_ELEMENT( curr_line - 1, curr_col )
      if( direction_ & NorthEast ) 
        INSERT_GLCM_ELEMENT( curr_line - 1, curr_col + 1 )
      if( direction_ & East ) 
        INSERT_GLCM_ELEMENT( curr_line, curr_col + 1 )
      if( direction_ & SouthEast ) 
        INSERT_GLCM_ELEMENT( curr_line + 1, curr_col + 1 )        
      if( direction_ & South ) 
        INSERT_GLCM_ELEMENT( curr_line + 1, curr_col )         
      if( direction_ & SouthWest ) 
        INSERT_GLCM_ELEMENT( curr_line + 1, curr_col - 1 )        
      if( direction_ & West ) 
        INSERT_GLCM_ELEMENT( curr_line, curr_col - 1 )         
      if( direction_ & NorthWest ) 
        INSERT_GLCM_ELEMENT( curr_line - 1, curr_col - 1 )           
    
      ++itp;
    } 
      
    /* Normalizing matrix */
      
    GLCMMatrixT::iterator ccm_it = mat.begin();
    const GLCMMatrixT::iterator ccm_it_end = mat.end();
      
    while ( ccm_it != ccm_it_end )
    {
      ( ccm_it->second ) /= ( (double)cacheNode.coOcurrences_ );
        
      ++ccm_it;
    }
    
    /* Updading concurrence matrix cache */

    return &(cacheNode);  
  } else {
    return &(it->second);
  }
}

void TePDIHaralick::clear()
{
  conc_matrix_cache_.clear();
  input_raster_.reset();
  polygonset_.reset();
}


