#define TEAGN_ENABLE_STDOUT_LOG

#include <terralib/kernel/TeSharedPtr.h>

#include <terralib/kernel/TeAgnostic.h>

#include <string>

#include <stdlib.h>

// Shared verification test
void test1()
{
  TeSharedPtr< int > sptr1( new int, false );
  TEAGN_TRUE_OR_THROW( sptr1.isShared(), "Shared verification failed" );
  TEAGN_TRUE_OR_THROW( ( sptr1.getRefCount() == 1 ), 
    "Invalid number of references" );
}

// Copy operator and operator= test
void test2()
{
  TeSharedPtr< int > sptr1( new int, false );
  *sptr1 = 12345;
  
  TeSharedPtr< int > sptr2;
  sptr2 = sptr1;
  
  TEAGN_TRUE_OR_THROW( sptr2.isShared(), "Shared verification failed" );
  TEAGN_TRUE_OR_THROW( ( sptr2 == sptr1 ), 
    "Invalid pointers" );     
  TEAGN_TRUE_OR_THROW( ( sptr1.getRefCount() == 2 ), 
    "Invalid number of references" );     
  TEAGN_TRUE_OR_THROW( ( sptr2.getRefCount() == 2 ), 
    "Invalid number of references" );          
  TEAGN_TRUE_OR_THROW( ( sptr1.nakedPointer() == sptr2.nakedPointer() ), 
    "Invalid pointers" );          
}

// Reset of a shared pointer test
void test3()
{
  TeSharedPtr< int > sptr1( new int, false );
  *sptr1 = 12345;
  
  TeSharedPtr< int > sptr2;
  sptr2 = sptr1;
  
  sptr1.reset( new int, false );
  *sptr1 = 54321;
  
  TEAGN_TRUE_OR_THROW( sptr1.isShared(), "Shared verification failed" );
  TEAGN_TRUE_OR_THROW( sptr2.isShared(), "Shared verification failed" );

  TEAGN_TRUE_OR_THROW( ( sptr1.getRefCount() == 1 ), 
    "Invalid number of references" ); 
  TEAGN_TRUE_OR_THROW( ( sptr2.getRefCount() == 1 ), 
    "Invalid number of references" );             

  TEAGN_TRUE_OR_THROW( *sptr1 == 54321,
    "Invalid pointer 1 reference" );

  TEAGN_TRUE_OR_THROW( *sptr2 == 12345,
    "Invalid pointer 2 reference" );
}

// Cascade shared pointers test
void test4()
{
  TeSharedPtr< int > sptr1( new int, false );
  *sptr1 = 12345;
  
  TeSharedPtr< int > sptr2;
  sptr2 = sptr1;
  
  TeSharedPtr< int > sptr3;
  sptr3 = sptr2;  

  TEAGN_TRUE_OR_THROW( sptr1.isShared(), "Shared verification failed" );
  TEAGN_TRUE_OR_THROW( sptr2.isShared(), "Shared verification failed" );
  TEAGN_TRUE_OR_THROW( sptr3.isShared(), "Shared verification failed" );
  
  TEAGN_TRUE_OR_THROW( ( sptr1.getRefCount() == 3 ), 
    "Invalid number of references" ); 
  TEAGN_TRUE_OR_THROW( ( sptr2.getRefCount() == 3 ), 
    "Invalid number of references" );       
  TEAGN_TRUE_OR_THROW( ( sptr3.getRefCount() == 3 ), 
    "Invalid number of references" );        

  TEAGN_TRUE_OR_THROW( ( sptr1.nakedPointer() == sptr3.nakedPointer() ), 
    "Invalid pointers" );    
      
  sptr1.reset();
  
  TEAGN_TRUE_OR_THROW( ( sptr1.getRefCount() == 0 ), 
    "Invalid number of references" ); 
  TEAGN_TRUE_OR_THROW( ( sptr2.getRefCount() == 2 ), 
    "Invalid number of references" );       
  TEAGN_TRUE_OR_THROW( ( sptr3.getRefCount() == 2 ), 
    "Invalid number of references" );        
}


// not shared pointer instance creation using a naked pointer
void test5()
{
  int* temp_int_ptr = new int;    
  *temp_int_ptr = 54321;

  TeSharedPtr< int > sptr4( temp_int_ptr, true );
  TEAGN_TRUE_OR_THROW( ( sptr4.getRefCount() == 1 ), 
    "Invalid number of references" );  
  TEAGN_TRUE_OR_THROW( sptr4.nakedPointer() == temp_int_ptr,
    "Invalid pointer reference" );      
  TEAGN_TRUE_OR_THROW( ( ! sptr4.isShared() ) , 
    "Shared verification failed" );
  sptr4.reset();
  
  delete temp_int_ptr;
}


// Creation of a non-shared pointer from a non-shared pointer
void test6()
{
  int* temp_int_ptr = new int;    
  *temp_int_ptr = 54321;   
  
  TeSharedPtr< int > sptr4( temp_int_ptr, true );
    
  TeSharedPtr< int > sptr5;
  sptr5 = sptr4;
  
  TEAGN_TRUE_OR_THROW( sptr5.nakedPointer() == temp_int_ptr,
    "Invalid pointer reference" );      
  TEAGN_TRUE_OR_THROW( ( ! sptr5.isShared() ) , 
    "Shared verification failed" );   
  TEAGN_TRUE_OR_THROW( ( sptr5.getRefCount() == 1 ), 
    "Invalid number of references" );            
    
  delete temp_int_ptr;
}

// alternative constructor tests
void test7()
{
  TeSharedPtr< int > acp1( new int );
  
  TeSharedPtr< int > acp2( acp1 );

  TEAGN_TRUE_OR_THROW( acp1.nakedPointer() == acp2.nakedPointer(), 
    "Invalid pointer" )
  TEAGN_TRUE_OR_THROW( acp2.isShared(), 
    "Shared verification failed" );   
  TEAGN_TRUE_OR_THROW( acp2.getRefCount() == 2, 
    "Invalid number of references" );     
}

int main()
{
  TEAGN_LOGMSG( "Test started." );
  
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
