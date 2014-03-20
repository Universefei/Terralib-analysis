
#include <terralib/kernel/TeThreadSignal.h>
#include <terralib/kernel/TeThreadFunctor.h>
#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>

bool threadEntry1( TeThreadParameters const& params )
{
  std::string* thread_str_ptr = 0;
  TEAGN_TRUE_OR_RETURN( params.retrive( "thread_str_ptr", thread_str_ptr ),
    "Error retriving parameter" );

  TeThreadSignal* thread_signal_ptr = 0;
  TEAGN_TRUE_OR_RETURN( params.retrive( "thread_signal_ptr", 
    thread_signal_ptr ), "Error retriving parameter" );
    
  thread_signal_ptr->waitSignal(100000);

  (*thread_str_ptr) = "thread finished";
  
  return true;
}


bool threadEntry2( TeThreadParameters const& params )
{
  TeThreadSignal* thread_signal_ptr = 0;
  TEAGN_TRUE_OR_RETURN( params.retrive( "thread_signal_ptr", 
    thread_signal_ptr ), "Error retriving parameter" );
    
  thread_signal_ptr->emitSignal();
  
  return true;
}


int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
    
    TeThreadSignal signal1;
    
    /* Starting the thread1 */

    TeThreadParameters thread1_params;

    std::string thread1_str;
    thread1_params.store( "thread_str_ptr", &thread1_str );
    
    thread1_params.store( "thread_signal_ptr", &signal1 );    
    
    TeThreadFunctor thread1;
    thread1.setStartFunctPtr( threadEntry1 );
    thread1.setParameters( thread1_params );
    TEAGN_TRUE_OR_THROW( thread1.start(),
      "Invalid thread start" )
      
    TeDelayThread( 5 );  
    
    TEAGN_TRUE_OR_THROW( ( thread1.getStatus() == TeThreadRunning ),
      "Invalid thread state" )
    
    TEAGN_TRUE_OR_THROW( ( thread1_str.empty() ),
      "Invalid thread string" )  
      
    /* Starting the thread2 */

    TeThreadParameters thread2_params;

    std::string thread2_str;
    thread2_params.store( "thread_str_ptr", &thread2_str );
    
    thread2_params.store( "thread_signal_ptr", &signal1 );    
    
    TeThreadFunctor thread2;
    thread2.setStartFunctPtr( threadEntry1 );
    thread2.setParameters( thread2_params );
    TEAGN_TRUE_OR_THROW( thread2.start(),
      "Invalid thread start" )
      
    TeDelayThread( 5 );  
    
    TEAGN_TRUE_OR_THROW( ( thread2.getStatus() == TeThreadRunning ),
      "Invalid thread state" )
    
    TEAGN_TRUE_OR_THROW( ( thread2_str.empty() ),
      "Invalid thread string" )
      
    /* Checking the signal reception from the two threads */
    
    signal1.emitSignal();  
    
    TeDelayThread( 5 );      
    
    TEAGN_TRUE_OR_THROW( ( thread1.getStatus() == TeThreadStopped ),
      "Invalid thread state" )        
    TEAGN_TRUE_OR_THROW( ( thread2.getStatus() == TeThreadStopped ),
      "Invalid thread state" )          
      
    TEAGN_TRUE_OR_THROW( ( thread1_str == "thread finished" ),
      "Invalid thread string" )  
      
    TEAGN_TRUE_OR_THROW( ( thread2_str == "thread finished" ),
      "Invalid thread string" )
      
    /* Starting the thread3 - 
       Testing the waiting of a already emitted signal*/
    
    TeThreadSignal signal2;

    TeThreadParameters thread3_params;

    thread3_params.store( "thread_signal_ptr", &signal2 );    
    
    TeThreadFunctor thread3;
    thread3.setStartFunctPtr( threadEntry2 );
    thread3.setParameters( thread3_params );
    TEAGN_TRUE_OR_THROW( thread3.start(),
      "Invalid thread start" )
      
    TeDelayThread( 5 );  
    
    TEAGN_TRUE_OR_THROW( ( thread3.getStatus() == TeThreadStopped ),
      "Invalid thread state" )
    
    TEAGN_TRUE_OR_THROW( ( ! signal2.waitSignal( 1000 ) ),
      "Invalid signal waitSignal return" )      

  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
