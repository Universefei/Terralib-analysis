#include "TePDIArithmetic.hpp"
#include "TePDIPIManager.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeUtils.h"
#include "../kernel/TeAgnostic.h"

#include <stack>


TePDIArithmetic::TePDIArithmetic()
{
}


TePDIArithmetic::~TePDIArithmetic()
{
}


void TePDIArithmetic::ResetState( const TePDIParameters& )
{
  progress_.Reset();
}


bool TePDIArithmetic::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  /* Checking input rasters */
  
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "input_rasters", input_rasters ),
    "Missing parameter: input_rasters" );
    
  TEAGN_TRUE_OR_RETURN( input_rasters.size() > 0, "Invalid number of rasters" );
    
  for( unsigned int index = 0 ; index < input_rasters.size() ; ++index ) 
  {
    TEAGN_TRUE_OR_RETURN( input_rasters[ index ].isActive(),
      "Invalid parameter: raster " + 
      Te2String( index ) + " inactive" );
      
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ index ]->params().status_ != TeRasterParams::TeNotReady,
      "Invalid parameter: raster " + 
      Te2String( index ) + " not ready" );
      
    /* Checking the number of lines */
    
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ index ]->params().nlines_ == 
      input_rasters[ 0 ]->params().nlines_,
      "All rasters must have the same number of lines" ); 
    TEAGN_TRUE_OR_RETURN( 
      input_rasters[ index ]->params().ncols_ == 
      input_rasters[ 0 ]->params().ncols_,
      "All rasters must have the same number of columns" );         
         
  }
  
  /* Checking output_image */

  TePDITypes::TePDIRasterPtrType output_raster;
  if( ! parameters.GetParameter( "output_raster", output_raster ) ) 
  {
    TEAGN_LOGERR( "Missing parameter: output_raster" );
    return false;
  }
  if( ! output_raster.isActive() ) 
  {
    TEAGN_LOGERR( "Invalid parameter: output_raster inactive" );
    return false;
  }
  if( output_raster->params().status_ == TeRasterParams::TeNotReady ) 
  {
    TEAGN_LOGERR( "Invalid parameter: output_image not ready" );
    return false;
  }
  
  // Checking arithmetic_string
  
  std::string arithmetic_string;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "arithmetic_string", 
    arithmetic_string ), "Missing parameter: arithmetic_string" );
    
  TePDITypes::TePDIRasterPtrType auxRaster;
  TEAGN_TRUE_OR_RETURN( executeString( arithmetic_string, input_rasters, 
    auxRaster, false ),
    "Invalid parameter: arithmetic_string" );

  return true;
}

bool TePDIArithmetic::RunImplementation()
{
  /* Parameters aquirement */

  TePDITypes::TePDIRasterVectorType input_rasters;
  params_.GetParameter( "input_rasters", input_rasters );

  std::string arithmetic_string;
  params_.GetParameter( "arithmetic_string", arithmetic_string );

  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter( "output_raster", output_raster );
  
  bool normalize_output = false;
  if( params_.CheckParameter< int >( "normalize_output" ) )
  {
    normalize_output = true;
  }
  
  /* Configuring the output raster */
  
  TeRasterParams outRaster_params = output_raster->params();
  
  outRaster_params.nBands( 1 );
  outRaster_params.projection( input_rasters[ 0 ]->projection() );
  outRaster_params.boundingBoxLinesColumns( 
    input_rasters[ 0 ]->params().boundingBox().x1(), 
    input_rasters[ 0 ]->params().boundingBox().y1(),
    input_rasters[ 0 ]->params().boundingBox().x2(), 
    input_rasters[ 0 ]->params().boundingBox().y2(), 
    input_rasters[ 0 ]->params().nlines_,
    input_rasters[ 0 ]->params().ncols_, 
    TeBox::TeUPPERLEFT );
  outRaster_params.setPhotometric( TeRasterParams::TeMultiBand, -1 );

  TEAGN_TRUE_OR_RETURN( output_raster->init( outRaster_params ),
    "Output raster reset error" );
  
  // Counting the number of operations to be done
    
  unsigned int operationsNumber = 0;
  {
    for( unsigned sIdx = 0 ; sIdx < arithmetic_string.size() ; ++sIdx )
    { 
      std::string auxStr;
      auxStr.push_back( arithmetic_string[ sIdx ] );
      
      if( isOperator( auxStr ) ) ++operationsNumber;
    }
  }
    
  // Creating the progress instance
  
  progress_.Reset( "Processing", (unsigned int)( 
    input_rasters[ 0 ]->params().nlines_ * ( operationsNumber + 
    ( normalize_output ? 2 : 1 ) ) ) );
  progress_.Toggle( progress_enabled_ );
    
  // Executing the arithmetic string
  
  TePDITypes::TePDIRasterPtrType auxRaster;
  TEAGN_TRUE_OR_RETURN( executeString( arithmetic_string, input_rasters, 
    auxRaster, true ), 
    "Arithmetic string execution error" );
    
  // Copy result data to output raster
  
  const unsigned int nLines = (unsigned int)auxRaster->params().nlines_;
  const unsigned int nCols = (unsigned int)auxRaster->params().ncols_;
  unsigned int line = 0;
  unsigned int col = 0;
  double value = 0;
  TeRaster& auxRasterRef = *auxRaster;
  TeRaster& outRasterRef = *output_raster;    
  
  double outAllowedMin = 0;
  double outAllowedMax = 0;
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds( output_raster,
    0, outAllowedMin, outAllowedMax ), "Execution error" );    
  
  if( normalize_output && ( output_raster->params().dataType_[ 0 ] !=
    TeDOUBLE ) )
  {
    // Calculating the output gain and offset
    
    double auxMin = DBL_MAX;
    double auxMax = -1.0 * auxMin;  
    
    for( line = 0 ; line < nLines ; ++line )
    {
      for( col = 0 ; col < nCols ; ++col )
      {
        auxRasterRef.getElement( col, line, value, 0 );
        
        if( auxMin > value ) auxMin = value;
        if( auxMax < value ) auxMax = value;
      }
      
      TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
    } 
    
    double outputOffset = 0;  
    double outputGain = 1.0;
    if( auxMax != auxMin ) 
    {
      outputOffset = -1.0 * auxMin;
      outputGain = ( ( outAllowedMax - outAllowedMin ) / ( auxMax - auxMin ) );       
    }
      
    for( line = 0 ; line < nLines ; ++line )
    {
      for( col = 0 ; col < nCols ; ++col )
      {
        auxRasterRef.getElement( col, line, value, 0 );
        
        value += outputOffset;
        value *= outputGain;
        
        value = MIN( value, outAllowedMax );
        value = MAX( value, outAllowedMin );
        
        outRasterRef.setElement( col, line, value, 0 );
      }
      
      TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
    }        
  }
  else
  {
    for( line = 0 ; line < nLines ; ++line )
    {
      for( col = 0 ; col < nCols ; ++col )
      {
        auxRasterRef.getElement( col, line, value, 0 );
        
        value = MIN( value, outAllowedMax );
        value = MAX( value, outAllowedMin );        
        
        outRasterRef.setElement( col, line, value, 0 );
      }
      
      TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
    }  
  }

  return true;
}

bool TePDIArithmetic::executeString( const std::string& aStr, 
  const TePDITypes::TePDIRasterVectorType& inRasters,
  TePDITypes::TePDIRasterPtrType& outRaster,
  bool generateOutput ) const
{
  std::vector< std::string > infixTokensVec;
  getTokensStrs( aStr, infixTokensVec );
//  printTokens( infixTokensVec );
  
  if( infixTokensVec.size() )
  {
    std::vector< std::string > postfixTokensVec;
    
//    printTokens( infixTokensVec );
    inFix2PostFix( infixTokensVec, postfixTokensVec );
//    printTokens( postfixTokensVec );
    
    ExecStackT execStack;
    unsigned int auxRasterIdx = 0;
    unsigned int auxBandIdx = 0;
    double auxRealValue = 0;
    
    for( unsigned int tIdx = 0 ; tIdx < postfixTokensVec.size() ; ++tIdx )
    {
      const std::string& curToken = postfixTokensVec[ tIdx ];
      
      if( isRasterBandToken( curToken, auxRasterIdx, auxBandIdx ) )
      {
        TEAGN_TRUE_OR_RETURN( auxRasterIdx < inRasters.size(), 
          "Invalid raster index found at " + curToken );
        TEAGN_TRUE_OR_RETURN( (int)auxBandIdx < 
          inRasters[ auxRasterIdx ]->params().nBands(), 
          "Invalid band index" );
      
        ExecStackElement auxEle;
        auxEle.isRaster_ = true;
        auxEle.rasterPtr_ = inRasters[ auxRasterIdx ];
        auxEle.rasterBand_ = auxBandIdx;
          
        execStack.push( auxEle );
      }
      else if( isRealNumberToken( curToken, auxRealValue ) )
      {
        ExecStackElement auxEle;
        auxEle.isRealNumber_ = true;
        auxEle.realNumberValue_ = auxRealValue;
          
        execStack.push( auxEle );
      }      
      else if( isBinaryOperator( curToken ) )
      {
        TEAGN_TRUE_OR_RETURN( execBinaryOperator( curToken, execStack,
          generateOutput ),
          "Operator " + curToken + " execution error" );
      }
      else if( isUnaryOperator( curToken ) )
      {
        TEAGN_TRUE_OR_RETURN( execUnaryOperator( curToken, execStack,
          generateOutput ),
          "Operator " + curToken + " execution error" );
      }
      else
      {
        TEAGN_LOG_AND_RETURN( "Invalid operator found: " + curToken );
      }
    }
    
    TEAGN_TRUE_OR_RETURN( execStack.size() == 1, "Invalid stack size" );
    TEAGN_TRUE_OR_RETURN( execStack.top().isRaster_, "Stack result error" );
      
    outRaster = execStack.top().rasterPtr_;
  }
  
  return true;
}

void TePDIArithmetic::inFix2PostFix( const std::vector< std::string >& input, 
  std::vector< std::string >& output ) const
{
  output.clear();
  
  std::stack< std::string > auxStack;
  const unsigned int inputSize = (unsigned int)input.size();
  
  for( unsigned int inIdx = 0 ; inIdx < inputSize ; ++inIdx )
  {
    const std::string& currInToken = input[ inIdx ];
    
    if( isOperator( currInToken ) )
    {
      while( ( ! auxStack.empty() ) && ( auxStack.top() != "(" ) )
      {
        if ( op1HasGreaterOrEqualPrecedence( auxStack.top(), currInToken ) )
        {
          output.push_back( auxStack.top() );
          
          auxStack.pop();
        }
        else 
        {
          break;
        }
      }
      
      auxStack.push( currInToken );
    }
    else if( currInToken == "(" )
    {
      auxStack.push( currInToken );
    }
    else if( currInToken == ")" )
    {
      while ( ( ! auxStack.empty() ) && ( auxStack.top() != "(" ) )
      {
        output.push_back( auxStack.top() );
        
        auxStack.pop();
      }
      
      if ( ! auxStack.empty() ) 
      {
        auxStack.pop();
      }
    }    
    else
    {
      output.push_back( currInToken );
    }
  }
  
  while ( ! auxStack.empty() )
  {
    output.push_back( auxStack.top() );
    
    auxStack.pop();
  }  
}

void TePDIArithmetic::printTokens( const std::vector< std::string >& input ) 
  const
{
  std::cout << std::endl;
  
  for( unsigned int idx = 0 ; idx < input.size() ; ++idx )
  {
    std::cout << "[" << input[ idx ] << "]";
  }
  
  std::cout << std::endl;
}

bool TePDIArithmetic::isOperator( const std::string& inputToken ) const
{
  return ( isBinaryOperator( inputToken ) || 
    isUnaryOperator( inputToken ) );
}  

bool TePDIArithmetic::isBinaryOperator( const std::string& inputToken ) const
{
  return ( ( inputToken == "+" ) || ( inputToken == "-" ) || 
    ( inputToken == "*" ) || ( inputToken == "/" ) ) ? 
    true : false;
}

bool TePDIArithmetic::isUnaryOperator( const std::string& ) const
{
  return false;
}

bool TePDIArithmetic::op1HasGreaterOrEqualPrecedence( const std::string& 
  operator1, const std::string& operator2 ) const
{
 if( ( operator1 == "*" ) || ( operator1 == "/" ) )
  {
    return true;
  }
  else
  {
    if( ( operator2 == "+" ) || ( operator2 == "-" ) )
    {
      return true;
    }
  }
  
  return false; 
}

bool TePDIArithmetic::isRasterBandToken( const std::string& token,
  unsigned int& rasterIdx, unsigned int& bandIdx ) const
{
  if( token.size() < 4 ) return false;
  if( token[ 0 ] != 'R' ) return false;
  
  std::string rasterIdxStr;
  unsigned int tIdx = 1;
  
  while( ( tIdx < token.size() ) && isdigit( token[ tIdx ] ) )
  {
    rasterIdxStr.push_back( token[ tIdx ] );
    ++tIdx;
  }
  
  if( token[ tIdx ] != ':' ) return false;
  ++tIdx;
  
  std::string bandIdxStr;
  
  while( ( tIdx < token.size() ) && isdigit( token[ tIdx ] ) )
  {
    bandIdxStr.push_back( token[ tIdx ] );
    ++tIdx;
  }
  
  if( ( rasterIdxStr.size() ) && ( bandIdxStr.size() ) )
  {
    rasterIdx = (unsigned int)atoi( rasterIdxStr.c_str() );
    bandIdx = (unsigned int)atoi( bandIdxStr.c_str() );
    return true;
  }
  else
  {
    return false;
  }
}

bool TePDIArithmetic::execBinaryOperator( const std::string& token, 
  ExecStackT& execStack, bool generateOutput ) const
{
  if( execStack.size() < 2 )
  {
    return false;
  }
  else
  {
    ExecStackElement sElem1 = execStack.top();
    execStack.pop();
    
    ExecStackElement sElem2 = execStack.top();
    execStack.pop();    
    
    if( token == "+" )
    {
      if( ( sElem1.isRaster_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;  
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value1 + value2, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }
      else if( ( sElem1.isRaster_ ) && ( sElem2.isRealNumber_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              
              outRasterRef.setElement( col, line, value1 + 
                sElem2.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }   
      else if( ( sElem1.isRealNumber_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem2.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem2.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 + 
                sElem1.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }          
      else
      {
        TEAGN_LOG_AND_RETURN( "Invalid operation" );
      }
    }
    else if( token == "-" )
    {
      if( ( sElem1.isRaster_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;  
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 - value1, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }
      else if( ( sElem1.isRaster_ ) && ( sElem2.isRealNumber_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              
              outRasterRef.setElement( col, line, sElem2.realNumberValue_ - 
                value1, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }   
      else if( ( sElem1.isRealNumber_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem2.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem2.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 -
                sElem1.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }       
      else
      {
        TEAGN_LOG_AND_RETURN( "Invalid operation" );
      }    
    }
    else if( token == "*" )
    {
      if( ( sElem1.isRaster_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;  
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value1 * value2, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }
      else if( ( sElem1.isRaster_ ) && ( sElem2.isRealNumber_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              
              outRasterRef.setElement( col, line, value1 * 
                sElem2.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }   
      else if( ( sElem1.isRealNumber_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem2.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem2.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 * 
                sElem1.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }        
      else
      {
        TEAGN_LOG_AND_RETURN( "Invalid operation" );
      }    
    }
    else if( token == "/" )
    {
      if( ( sElem1.isRaster_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;  
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 / value1, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }
      else if( ( sElem1.isRaster_ ) && ( sElem2.isRealNumber_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem1.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem1.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster1Ref = *sElem1.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value1 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster1Ref.getElement( col, line, value1, sElem1.rasterBand_ );
              
              outRasterRef.setElement( col, line, sElem2.realNumberValue_ / 
                value1, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }   
      else if( ( sElem1.isRealNumber_ ) && ( sElem2.isRaster_ ) )
      {
        const unsigned int nLines = (unsigned int)
          sElem2.rasterPtr_->params().nlines_;
        const unsigned int nCols = (unsigned int)
          sElem2.rasterPtr_->params().ncols_;
        
        ExecStackElement outElement;
        outElement.isRaster_ = true;
        TEAGN_TRUE_OR_RETURN( allocResultRaster( nLines, nCols, 
          outElement.rasterPtr_ ), "Raster allocation error" );
          
        TeRaster& outRasterRef = *outElement.rasterPtr_;
        TeRaster& inRaster2Ref = *sElem2.rasterPtr_;
              
        if( generateOutput )
        {
          unsigned int line = 0;
          unsigned int col = 0;
          double value2 = 0;
          
          for( line = 0 ; line < nLines ; ++line )
          {
            for( col = 0 ; col < nCols ; ++col )
            {
              inRaster2Ref.getElement( col, line, value2, sElem2.rasterBand_ );
              
              outRasterRef.setElement( col, line, value2 /
                sElem1.realNumberValue_, 0 );
            }
            
            TEAGN_FALSE_OR_RETURN( progress_.Increment(), "Canceled by the user" );
          }
        }
           
        execStack.push( outElement );
      }       
      else
      {
        TEAGN_LOG_AND_RETURN( "Invalid operation" );
      }
    }
    else
    {
      TEAGN_LOG_AND_RETURN( "Invalid operator" );
    }
  }
  
  return true;
}

bool TePDIArithmetic::execUnaryOperator( const std::string& , 
  ExecStackT& , bool ) const
{
  return false;
}

bool TePDIArithmetic::isRealNumberToken( const std::string& token,
  double& realValue ) const
{
  if( token.size() == 0 ) return false;
  bool hasDot = false;

  unsigned int initIdx = 0;
  if( ( token[ 0 ] == '+' ) || ( token[ 0 ] == '-' ) )
  {
    if( token.size() == 1 ) return false;
    initIdx = 1;
  }
  
  for( unsigned int tIdx = initIdx ; tIdx < token.size() ; ++tIdx )
  {
    if( token[ tIdx ] == '.' )
    {
      if( hasDot ) return false;
      hasDot = true;
    }
    else if( ! isdigit( token[ tIdx ] ) ) return false;
  }
  
  realValue = atof( token.c_str() );
  
  return true;
}

bool TePDIArithmetic::allocResultRaster( unsigned int nLines, unsigned int nCols,
  TePDITypes::TePDIRasterPtrType& rasterPtr ) const
{
  TeRasterParams pars;
  pars.mode_ = 'c';
  pars.setDataType( TeDOUBLE, -1 );
  pars.setNLinesNColumns( nLines, nCols );
  pars.decoderIdentifier_ = "SMARTMEM";

  rasterPtr.reset( new TeRaster( pars ) );
  if( rasterPtr->init() )
  {
    return true;
  }
  else
  {
    rasterPtr.reset();
    return false;
  }
}

void TePDIArithmetic::getTokensStrs( const std::string& inputStr, 
  std::vector< std::string >& outTokens ) const
{
  outTokens.clear();
  
  const unsigned int inputStrSize = (unsigned int)inputStr.size();
  std::string bufferStr;
  
  for( unsigned int inputStrIdx = 0 ; inputStrIdx < inputStrSize ;
    ++inputStrIdx )
  {
    if( inputStr[ inputStrIdx ] == ' ' )
    {
      // flush the last formed token
      if( bufferStr.size() )
      {
        outTokens.push_back( bufferStr );
        bufferStr.clear();
      }
    }
    else
    {
      if( ( inputStr[ inputStrIdx ] == '+' ) ||
        ( inputStr[ inputStrIdx ] == '-' ) )
      {
        bufferStr.append( inputStr.substr( inputStrIdx, 1 ) );
      }      
      else if( ( inputStr[ inputStrIdx ] == ')' ) ||
        ( inputStr[ inputStrIdx ] == '(' ) ||
        isOperator( inputStr.substr( inputStrIdx, 1 ) ) )
      {
        // flush the last formed token
        if( bufferStr.size() )
        {
          outTokens.push_back( bufferStr );
          bufferStr.clear();
        }
        
        outTokens.push_back( inputStr.substr( inputStrIdx, 1 ) );
      }    
      else
      {
        bufferStr.append( inputStr.substr( inputStrIdx, 1 ) );
      }
    }
  }
  
  if( bufferStr.size() )
  {
    outTokens.push_back( bufferStr );
  }  
}
