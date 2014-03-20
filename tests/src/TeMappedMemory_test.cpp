
#include <terralib/kernel/TeMappedMemory.h>
#include <terralib/kernel/TeUtils.h>
#include <terralib/kernel/TeException.h>

#include <iostream>

#include <stdlib.h>

int main()
{
  std::cout << std::endl << "Test started." << std::endl;
  
  try{
    TeMappedMemory mm;
    
    unsigned int index_bound = 1024 * 1024 * 10;
    unsigned int elem_size = sizeof( unsigned long long int );
    unsigned int size = index_bound  * elem_size;
    
    if( ! mm.reset( size, true ) ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Reset error", 
        false ) );    
    }
    
    if( mm.size() != size ) {
      throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid size=" +
        Te2String( mm.size() ), false ) );    
    }    
    
    unsigned long long int* ptr = (unsigned long long int*) mm.getPointer();
    
    unsigned int index;
    
    for( index = 0 ; index < index_bound ; ++index ) {
      ptr[ index ] = (unsigned long long int)index;
    }
    
    TEAGN_TRUE_OR_THROW( mm.toggle( false ), "Toggle error" )
    
    TEAGN_TRUE_OR_THROW( mm.toggle( true ), "Toggle error" )
    
    ptr = (unsigned long long int*) mm.getPointer();
    
    for( index = 0 ; index < index_bound ; ++index ) {
      if( ptr[ index ] != ((unsigned long long int)index) ) {
        throw( TeException( UNKNOWN_ERROR_TYPE , "Invalid value", 
          false ) );        
      }
    }
  }
  catch( TeException& excpt ){
    std::cout << std::endl << excpt.message() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << std::endl << "Test OK" << std::endl;
  return EXIT_SUCCESS;
}
