#include "TePDIPIManager.hpp"

#include "../kernel/TeAgnostic.h"

/* Forcing the instantiation of global settings 
   to avoid thread racing condition */
namespace {
  static TePDIPIManagerGlobalSettings& globalsettings = 
    TePDIPIManagerGlobalSettings::instance();
}

TePDIPIManager::TePDIPIManager( const std::string& message, 
  unsigned long int tsteps, bool enabled )
  : global_settings_( TePDIPIManagerGlobalSettings::instance() )
{
  enabled_ = false;
  total_steps_ = tsteps;
  curr_step_ = 0;
  prog_intef_ptr_ = TeProgress::instance();
  message_ = message;
  
  if( enabled ) {
    Toggle( true );
  }
}

TePDIPIManager::TePDIPIManager( const TePDIPIManager& )
  : global_settings_( TePDIPIManagerGlobalSettings::instance() )
{
}


TePDIPIManager::~TePDIPIManager()
{
  Toggle( false );
}


void TePDIPIManager::Toggle( bool enabled )
{
  if( prog_intef_ptr_ ) 
  {
    if( ( ! enabled_ ) && enabled && ( total_steps_ > 0 ) ) {
      /* Need to enable this instance */
     
      global_settings_.lock_.lock();

      if( global_settings_.active_manager_instances_ == 0 ) {
        /* No instance has control */
        
        global_settings_.total_steps_ = total_steps_;
        global_settings_.curr_step_ = curr_step_;
        /* keeping global_settings.curr_prop_step_ */
        global_settings_.active_manager_instances_ = 1;
        global_settings_.curr_message_ = message_;
      } else {
        /* More instances are controling */
        
        global_settings_.total_steps_ += total_steps_;
        global_settings_.curr_step_ += curr_step_;
        /* Keeping global_settings.curr_prop_step_ */
        global_settings_.active_manager_instances_ += 1;
        global_settings_.curr_message_.clear();
      }
      
      enabled_ = true;
      
      updateProgressInterface();
      
      global_settings_.lock_.unLock();
    } if( enabled_ && ( ! enabled ) ) {
      /* Need to disable this instance */
      
      global_settings_.lock_.lock();

      if( global_settings_.active_manager_instances_ == 1 ) {
        /* Just this instance has control */
        
        global_settings_.total_steps_ = 0;
        global_settings_.curr_step_ = 0;
        /* keeping global_settings.curr_prop_step_ */
        global_settings_.active_manager_instances_ = 0;
        global_settings_.curr_message_.clear();
      } else {
        /* More instance are controling */
        
        /* keeping global_settings.total_steps_ */
        global_settings_.curr_step_ += ( total_steps_ - curr_step_ );
        /* keeping global_settings.curr_prop_step_ */
        global_settings_.active_manager_instances_ -= 1;
      }
      
      enabled_ = false;
      
      updateProgressInterface();
      
      global_settings_.lock_.unLock();      
    }
  }
}


bool TePDIPIManager::updateProgressInterface() const
{
  if( prog_intef_ptr_->wasCancelled() ) {
    if( global_settings_.active_manager_instances_ == 0 ) {
      prog_intef_ptr_->reset();
      return false;
    } else {
      return true;
    }
  } else {
    if( global_settings_.active_manager_instances_ == 0 ) 
    {
      global_settings_.curr_prop_step_ = 0;
      prog_intef_ptr_->reset();    
    } else {
      unsigned long int new_prop_step = (unsigned long int)
        ( 100.0 * 
        ( ((double)global_settings_.curr_step_) / 
        ( (double)global_settings_.total_steps_ ) ) );
          
      if( new_prop_step > global_settings_.curr_prop_step_ ) {
        prog_intef_ptr_->setMessage( global_settings_.curr_message_ );
        prog_intef_ptr_->setTotalSteps( 100 );
        prog_intef_ptr_->setProgress( new_prop_step );
        
        global_settings_.curr_prop_step_ = new_prop_step;
      }
    } 
    
    return false; 
  }
}


bool TePDIPIManager::Update( unsigned long int step )
{
  if( step > total_steps_ ) {
    step = total_steps_;
  }
  
  if( enabled_ && prog_intef_ptr_ && ( step > curr_step_ ) ) 
  {
    global_settings_.lock_.lock();

    /* keeping global_settings.total_steps_ */
    global_settings_.curr_step_ += ( step - curr_step_ );
    /* keeping global_settings.curr_prop_step_ */
    /* keeping global_settings.active_manager_instances_ */
    
    curr_step_ = step;
    
    bool return_value = updateProgressInterface();
    
    global_settings_.lock_.unLock();      
  
    return return_value;
  } else {
    return false;
  }
}

void TePDIPIManager::Reset( const std::string& message, 
  unsigned long int tsteps )
{
  bool old_enabled_state = enabled_;
  
  Toggle( false );
  
  total_steps_ = tsteps; 
  curr_step_ = 0;
  message_ = message;

  if( old_enabled_state ) {
    Toggle( true );
  }  
}

