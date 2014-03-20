
#include <terralib/kernel/TeSemaphore.h>
#include <terralib/kernel/TeThreadFunctor.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>


bool threadEntry( TeThreadParameters const& params )
{
  unsigned int* test_value_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "test_value_ptr", test_value_ptr ),
    "Error retriving test_value_ptr" );

  TeSemaphore* sem_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "sem_ptr", 
    sem_ptr ), "Error retriving parameter" );
    
  sem_ptr->wait();
  
  TEAGN_LOGMSG( "gone")

  (*test_value_ptr) = 1;
  
  return true;
}

bool threadEntry2( TeThreadParameters const& params )
{
  unsigned int* test_value_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "test_value_ptr", test_value_ptr ),
    "Error retriving test_value_ptr" );

  TeSemaphore namedSem( (unsigned int)0, std::string( "TeSemaphore_test" ) );
    
  namedSem.wait();
  
  TEAGN_LOGMSG( "gone")

  (*test_value_ptr) = 1;
  
  return true;
}

int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    
    // Unnamed semaphore test 
    {
      TeSemaphore unnamedSem( 0 );
  
      /* Starting the thread1 */
  
      TeThreadParameters thread1_params;
  
      unsigned int test_value = 0;
      
      thread1_params.store( "test_value_ptr", &test_value );
      thread1_params.store( "sem_ptr", &unnamedSem );    
      
      TeThreadFunctor thread1;
      thread1.setStartFunctPtr( threadEntry );
      thread1.setParameters( thread1_params );
      TEAGN_TRUE_OR_THROW( thread1.start(),
        "Invalid thread start" )
  
      TeDelayThread( 5 );
      
      TEAGN_CHECK_EPS( test_value, 0, 0, "Invalid test_value" )
          
      unnamedSem.post();
      
      TeDelayThread( 5 );
      
      TEAGN_CHECK_EPS( test_value, 1, 0, "Invalid test_value" )       
          
      unnamedSem.post();
                 
      TEAGN_TRUE_OR_THROW( unnamedSem.wait( 1000 ), "Wait error" );
    }
    
    // named semaphore test 
    {
      TeSemaphore namedSem( (unsigned int)0, std::string( "TeSemaphore_test" ) );
  
      /* Starting the thread1 */
  
      TeThreadParameters thread2_params;
  
      unsigned int test_value = 0;
      
      thread2_params.store( "test_value_ptr", &test_value );
      
      TeThreadFunctor thread2;
      thread2.setStartFunctPtr( threadEntry2 );
      thread2.setParameters( thread2_params );
      TEAGN_TRUE_OR_THROW( thread2.start(),
        "Invalid thread start" )
  
      TeDelayThread( 5 );
      
      TEAGN_CHECK_EPS( test_value, 0, 0, "Invalid test_value" )
          
      namedSem.post();
      
      TeDelayThread( 5 );
      
      TEAGN_CHECK_EPS( test_value, 1, 0, "Invalid test_value" )       
          
      namedSem.post();
                 
      TEAGN_TRUE_OR_THROW( namedSem.wait( 1000 ), "Wait error" );
    }    
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
