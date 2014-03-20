#include "TePDIAlgorithm.hpp"

#include "../kernel/TeAgnostic.h"

TePDIAlgorithm::TePDIAlgorithm()
{
  progress_enabled_ = true;
}


TePDIAlgorithm::~TePDIAlgorithm()
{
}


bool TePDIAlgorithm::Reset( const TePDIParameters& params )
{
  if( CheckParameters( params ) ) {
    ResetState( params );

    /* This line must come after the ResetState line, because some algorithms
       checks the difference between the current internal parameters and the
       new supplied parameters */
    params_.Clear();
    params_ = params;

    return true;
  } else {
    TEAGN_LOG_AND_RETURN( "Invalid supplied parameters" );
  }
}


bool TePDIAlgorithm::Apply()
{
  bool return_value = false;
  
  if( CheckInternalParameters() ) {
    #ifdef NDEBUG
      try
      {
        return_value = RunImplementation();
      }
      catch( const TeException& exc )
      {
        TEAGN_LOGERR( "Exception raised from algorithm - " +
          exc.message() );
      }
      catch(...)
      {
        TEAGN_LOGERR( "Unhandled exception raised from algorithm" );
      }
    #else
      return_value = RunImplementation();
    #endif
    
    StopProgInt();
  } else {
    TEAGN_LOG_AND_RETURN( "Invalid supplied parameters" );
  }
  
  return return_value;
}


bool TePDIAlgorithm::Apply( const TePDIParameters& params )
{
  TEAGN_TRUE_OR_RETURN( CheckParameters( params ),
    "Invalid parameters" )
    
  ResetState( params );

  /* This line must come after the ResetState line, because some algorithms
     checks the difference between the current internal parameters and the
     new supplied parameters */
  params_.Clear();
  params_ = params;
  
  #ifdef NDEBUG
    try
    {
      return RunImplementation();
    }
    catch( const TeException& exc )
    {
      TEAGN_LOG_AND_RETURN( "Exception raised from algorithm - " +
        exc.message() );
    }
    catch(...)
    {
      TEAGN_LOG_AND_RETURN( "Unhandled exception raised from algorithm" );
    }
  #else
    return RunImplementation();
  #endif    
}


const TePDIParameters& TePDIAlgorithm::GetParameters() const
{
  return params_;
}


bool TePDIAlgorithm::CheckInternalParameters() const
{
  return CheckParameters( params_ );
}


const TePDIAlgorithm& TePDIAlgorithm::operator=( 
  const TePDIAlgorithm& external )
{
  TEAGN_LOG_AND_THROW( "Algorithms cannot be copied" );  
  
  return external;
}

TePDIAlgorithm* TePDIAlgorithm::DefaultObject( const TePDIParameters& params )
{
  TEAGN_LOG_AND_THROW( "Trying to create an invalid algorithm instance" );
  
  TePDIParameters dummy_params = params;

  return 0;
}


void TePDIAlgorithm::ToggleProgInt( bool enabled )
{
  progress_enabled_ = enabled;
  
  if( ! enabled ) {
    pi_manager_.Toggle( false );
  }
}


void TePDIAlgorithm::StartProgInt( const std::string& message, 
  unsigned int steps )
{
  if( progress_enabled_ ) {
    pi_manager_.Reset( message, steps );
    pi_manager_.Toggle( true );
  }
}


bool TePDIAlgorithm::UpdateProgInt( unsigned int step )
{
  return pi_manager_.Update( step );
}


bool TePDIAlgorithm::IncProgInt()
{
  return pi_manager_.Increment();
}


void TePDIAlgorithm::StopProgInt()
{
  pi_manager_.Toggle( false );
}

