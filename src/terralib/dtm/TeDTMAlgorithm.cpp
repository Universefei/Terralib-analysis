#include "TeDTMAlgorithm.h"

#include <TeAgnostic.h>

TeDTMAlgorithm::TeDTMAlgorithm()
{
  progress_enabled_ = true;
}


TeDTMAlgorithm::~TeDTMAlgorithm()
{
}


bool TeDTMAlgorithm::reset( const TeDTMParameters& params )
{
  if( checkParameters( params ) ) {
    resetState( params );

    /* This line must come after the ResetState line, because some algorithms
       checks the difference between the current internal parameters and the
       new supplied parameters */
    params_.clear();
    params_ = params;

    return true;
  } else {
    TEAGN_LOG_AND_RETURN( "Invalid supplied parameters" );
  }
}


bool TeDTMAlgorithm::apply()
{
  bool return_value = false;
  
  if( checkInternalParameters() ) {
    return_value = runImplementation();
  } else {
    TEAGN_LOG_AND_RETURN( "Invalid supplied parameters" );
  }
  
  return return_value;
}


const TeDTMParameters& TeDTMAlgorithm::getParameters() const
{
  return params_;
}


bool TeDTMAlgorithm::checkInternalParameters() const
{
  return checkParameters( params_ );
}


const TeDTMAlgorithm& TeDTMAlgorithm::operator=( 
  const TeDTMAlgorithm& external )
{
  TEAGN_LOG_AND_THROW( "Algorithms cannot be copied" );  
  
  return external;
}


void TeDTMAlgorithm::toggleProgInt( bool enabled )
{
  progress_enabled_ = enabled;
}

