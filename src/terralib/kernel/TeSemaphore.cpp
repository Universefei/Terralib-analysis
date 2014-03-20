#include "TeSemaphore.h"

#if TePLATFORM == TePLATFORMCODE_MSWINDOWS
  #include <limits.h>
#elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/time.h>
#else
    #error "Unsuported plataform"
#endif   

TeSemaphore::TeSemaphore( unsigned int count )
{
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    SECURITY_ATTRIBUTES secAtts;
    secAtts.lpSecurityDescriptor = NULL;
    secAtts.bInheritHandle = TRUE;    
    secAtts.nLength = sizeof( secAtts );
      
    semHandlerPtr_ = CreateSemaphore( &secAtts, (LONG)count, (long)LONG_MAX, 0);
    TEAGN_TRUE_OR_THROW( ( semHandlerPtr_ != 0 ),
      "Unable to acquire semaphore handler" )
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    semHandlerPtr_ = new sem_t;
    
    TEAGN_TRUE_OR_THROW( sem_init( semHandlerPtr_, 0, count ) == 0,
      "Error initializing semaphore" )
  #else
    #error "Unsuported plataform"
  #endif   
}

TeSemaphore::TeSemaphore( unsigned int count, const std::string& semName )
{
  TEAGN_TRUE_OR_THROW( !semName.empty(), "Invalid semaphore name" );
  
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    TEAGN_TRUE_OR_THROW( semName.size() < MAX_PATH, "Invalid semaphore name" );
  
    SECURITY_ATTRIBUTES secAtts;
    secAtts.lpSecurityDescriptor = NULL;
    secAtts.bInheritHandle = TRUE;    
    secAtts.nLength = sizeof( secAtts );
      
    semHandlerPtr_ = CreateSemaphore( &secAtts, (LONG)count, (long)LONG_MAX, 
      semName.c_str() );
    TEAGN_TRUE_OR_THROW( ( semHandlerPtr_ != 0 ),
      "Unable to acquire semaphore handler" )
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    semHandlerPtr_ = sem_open( semName.c_str(), O_CREAT,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 
      (unsigned)count );     
    TEAGN_TRUE_OR_THROW( ( semHandlerPtr_ != SEM_FAILED ),
      "Unable to acquire semaphore handler" ) 
  #else
    #error "Unsuported plataform"
  #endif  
               
  semName_ = semName;            
}


TeSemaphore::~TeSemaphore()
{
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    CloseHandle( semHandlerPtr_ );
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    if( semName_.size() )
    {
      sem_unlink( semName_.c_str() );
      sem_close( semHandlerPtr_ );
    }
    else
    {
      sem_destroy( semHandlerPtr_ );
      delete semHandlerPtr_;
    }
  #else
    #error "Unsuported plataform"
  #endif   
}

bool TeSemaphore::wait( unsigned int waitingTime )
{
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    DWORD return_value = 0;
    return_value = WaitForSingleObject( semHandlerPtr_, (DWORD)waitingTime );
    
    if( ( return_value == WAIT_ABANDONED ) || ( return_value == 
      WAIT_OBJECT_0 ) )
    {
      return true;
    }
    else
    {
      return false;
    }
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    struct timeval timevalstr;

    if( gettimeofday( &timevalstr, 0 ) == 0 ) 
    {
      struct timespec timespecstr;

      /* seconds */
      timespecstr.tv_sec = timevalstr.tv_sec + ( waitingTime / 1000 );
      /* nano-seconds */
      timespecstr.tv_nsec = ( timevalstr.tv_usec * 1000 ) +
        ( ( waitingTime % 1000 ) * 1000000 );
      
#if TePLATFORM == TePLATFORMCODE_APPLE
      if( 0 == sem_wait( semHandlerPtr_ ) )		//CTX
#else
			if( 0 == sem_timedwait( semHandlerPtr_, &timespecstr ) )
#endif
      {
        return true;
      }
      else
      {
        return false;
      }
    } 
    else 
    {
      TEAGN_LOGWARN( "Unable to get the current time" );
      return false;
    }
  #else
    #error "Unsuported plataform"
  #endif   
}; 


