
#include <terralib/kernel/TeMultiContainer.h>

#include <terralib/kernel/TeAgnostic.h>

#include <string>

#include <stdlib.h>

void index_str_test()
{
  TeMultiContainer< std::string > obj_cont_copy;
  
  const int x1_int = 1;
  const double x1_double = 1;
  
  {
    TeMultiContainer< std::string > obj_cont;
    
    obj_cont.store( "int", x1_int );
    obj_cont.store( "double", x1_double );    
    
    TEAGN_TRUE_OR_THROW( obj_cont.isStored< int >( "int" ), "" )
    TEAGN_TRUE_OR_THROW( obj_cont.isStored< double >( "double" ), "" )
    
    int x2_int = 0;
    obj_cont.retrive( "int", x2_int );    
    TEAGN_CHECK_EPS( x2_int, x1_int, 0, "" )
    
    double x2_double = 0;
    obj_cont.retrive( "double", x2_double );
    TEAGN_CHECK_EPS( x2_double, x1_double, 0, "" )      
    
    obj_cont_copy = obj_cont;
    
    TEAGN_TRUE_OR_THROW( ( obj_cont_copy == obj_cont ),
      "Invalid ==operator result" )
  }
  
  TEAGN_TRUE_OR_THROW( obj_cont_copy.isStored< int >( "int" ), "" )
  TEAGN_TRUE_OR_THROW( obj_cont_copy.isStored< double >( "double" ), "" )
  
  int x2_int = 0;
  obj_cont_copy.retrive( "int", x2_int );    
  TEAGN_CHECK_EPS( x2_int, x1_int, 0, "" )
  
  std::vector< std::pair< std::string, int > > intvect;
  obj_cont_copy.multiRetrive( intvect );    
  TEAGN_TRUE_OR_THROW( ( intvect.size() == 1 ), "Invalid vector size" )
  TEAGN_TRUE_OR_THROW( ( intvect[ 0 ].first == "int" ), 
    "Invalid vector index" )  
  TEAGN_TRUE_OR_THROW( ( intvect[ 0 ].second == x1_int ), 
    "Invalid vector element" )  
  
  double x2_double = 0;
  obj_cont_copy.retrive( "double", x2_double );
  TEAGN_CHECK_EPS( x2_double, x1_double, 0, "" )
}


void index_int_test()
{
  TeMultiContainer< int > obj_cont_copy;
  
  const int x1_int = 1;
  const double x1_double = 1;
  
  {
    TeMultiContainer< int > obj_cont;
    
    obj_cont.store( 1, x1_int );
    obj_cont.store( 2, x1_double );    
    
    TEAGN_TRUE_OR_THROW( obj_cont.isStored< int >( 1 ), "" )
    TEAGN_TRUE_OR_THROW( obj_cont.isStored< double >( 2 ), "" )
    
    int x2_int = 0;
    obj_cont.retrive( 1, x2_int );    
    TEAGN_CHECK_EPS( x2_int, x1_int, 0, "" )
    
    double x2_double = 0;
    obj_cont.retrive( 2, x2_double );
    TEAGN_CHECK_EPS( x2_double, x1_double, 0, "" )      
    
    obj_cont_copy = obj_cont;
    
    TEAGN_TRUE_OR_THROW( ( obj_cont_copy == obj_cont ),
      "Invalid ==operator result" )
  }
  
  TEAGN_TRUE_OR_THROW( obj_cont_copy.isStored< int >( 1 ), "" )
  TEAGN_TRUE_OR_THROW( obj_cont_copy.isStored< double >( 2 ), "" )
  
  int x2_int = 0;
  obj_cont_copy.retrive( 1, x2_int );    
  TEAGN_CHECK_EPS( x2_int, x1_int, 0, "" )
  
  std::vector< std::pair< int, int > > intvect;
  obj_cont_copy.multiRetrive( intvect );    
  TEAGN_TRUE_OR_THROW( ( intvect.size() == 1 ), "Invalid vector size" )
  TEAGN_TRUE_OR_THROW( ( intvect[ 0 ].first == 1 ), 
    "Invalid vector index" )  
  TEAGN_TRUE_OR_THROW( ( intvect[ 0 ].second == x1_int ), 
    "Invalid vector element" )  
  
  double x2_double = 0;
  obj_cont_copy.retrive( 2, x2_double );
  TEAGN_CHECK_EPS( x2_double, x1_double, 0, "" )
  
  /* Testing the remove method */
  
  TEAGN_TRUE_OR_THROW( obj_cont_copy.isStored< double >( 2 ),
    "Check error" )  
  obj_cont_copy.remove( 2 );
  TEAGN_TRUE_OR_THROW( ! obj_cont_copy.isStored< double >( 2 ),
    "Remove test error" )
}


int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
  
    index_str_test();
    index_int_test();
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }
  catch(...){
    TEAGN_LOGERR( "Test Failed - Unhandled exception");
    return EXIT_FAILURE;
  }  

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
