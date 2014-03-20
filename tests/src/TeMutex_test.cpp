
#include <terralib/kernel/TeMutex.h>
#include <terralib/kernel/TeThreadFunctor.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>


bool threadEntry( TeThreadParameters const& params )
{
  unsigned int* test_value_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "test_value_ptr", test_value_ptr ),
    "Error retriving test_value_ptr" );

  TeMutex* lock_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "thread_lock_ptr", 
    lock_ptr ), "Error retriving parameter" );
    
  lock_ptr->lock();

  (*test_value_ptr) = 1;

  lock_ptr->unLock();
  
  return true;
}

int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
    
    TeMutex lock1;

    lock1.lock();

    /* Starting the thread1 */

    TeThreadParameters thread1_params;

    unsigned int test_value = 0;
    
    thread1_params.store( "test_value_ptr", &test_value );
    
    thread1_params.store( "thread_lock_ptr", &lock1 );    
    
    TeThreadFunctor thread1;
    thread1.setStartFunctPtr( threadEntry );
    
    thread1.setParameters( thread1_params );

    TEAGN_TRUE_OR_THROW( thread1.start(),
      "Invalid thread start" )

    TeDelayThread( 5 );
    
    TEAGN_CHECK_EPS( test_value, 0, 0,
      "Invalid test_value" ) 
    
    lock1.unLock();
    
    TEAGN_TRUE_OR_THROW( thread1.waitToFinish(),
      "Thread return error" )

    TEAGN_CHECK_EPS( test_value, 1, 0,
      "Invalid test_value" ) 
    
    TEAGN_TRUE_OR_THROW( lock1.tryLock(), "Locking error" );
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
