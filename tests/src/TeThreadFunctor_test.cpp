
#include <terralib/kernel/TeThreadFunctor.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>


bool threadEntry( TeThreadParameters const& params )
{
  std::string* thread_str_ptr = 0;
  TEAGN_TRUE_OR_RETURN( params.retrive( "thread_str_ptr", thread_str_ptr ),
    "Error retriving parameter" );

  (*thread_str_ptr) = "thread finished";

  TeDelayThread(5);
  
  return true;
}


int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
    
    TeThreadFunctor thread;
    thread.setStartFunctPtr( threadEntry );

    // Test 1

    std::string thread1_str;
  
    TeThreadParameters thread1_params;
    thread1_params.store( "thread_str_ptr", &thread1_str );

    TEAGN_TRUE_OR_THROW( ( thread.getStatus() == TeThreadStopped ),
      "Invalid thread state" )   
      
    thread.setParameters( thread1_params );

    TEAGN_TRUE_OR_THROW( thread.start(),
      "Invalid thread start" )
      
    TeDelayThread(1);

    TEAGN_TRUE_OR_THROW( ( thread.getStatus() == TeThreadRunning ),
      "Invalid thread state" )

    TEAGN_TRUE_OR_THROW( thread.waitToFinish(),
      "Invalid waitToFinish return" )
      
    TEAGN_TRUE_OR_THROW( thread.getReturnValue(),
      "Invalid thread return value" )
     
    TEAGN_TRUE_OR_THROW( ( thread.getStatus() == TeThreadStopped ),
      "Invalid thread state" )      

    TEAGN_TRUE_OR_THROW( ( thread1_str == "thread finished" ),
      "Invalid thread string" )

    // Test 2

    std::string thread2_str;

    TeThreadParameters thread2_params;
    thread2_params.store( "thread_str_ptr", &thread2_str );

    thread.setParameters( thread2_params );

    TEAGN_TRUE_OR_THROW( thread.start(),
      "Invalid thread start" );
      
    TeDelayThread(10);

    TEAGN_TRUE_OR_THROW( ( thread.getStatus() == TeThreadStopped ),
      "Invalid thread state" )
      
    TEAGN_TRUE_OR_THROW( thread.getReturnValue(),
      "Invalid thread return value" )
      
    TEAGN_TRUE_OR_THROW( ( thread1_str == "thread finished" ),
      "Invalid thread string" )
      
    /* Dummy thread instance test */
    
    TeThreadFunctor dummy_thread;
    dummy_thread.setStartFunctPtr( threadEntry );
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
