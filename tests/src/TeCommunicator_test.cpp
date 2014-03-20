
#include <terralib/kernel/TeCommunicator.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>

class ReceiverClass
{
  TECOMMWRAPPER( SetInt , int , ReceiverClass )
  
  public :
    TeCommunicator< int > comm_;
    TeCommunicator< int > inactive_comm_;
    
    ReceiverClass()
    { 
      value_ = 0; 
      
      comm_.setHostObj( this, SetInt );
    };
  
    double GetValue() { return value_; };
    
  protected :
    double value_;
};


void ReceiverClass::SetInt( const int& x )
{ 
  value_ = (double)x;
}


class SenderClass
{
  public :
    TeCommunicator< int > comm_;
    
    void SetInt( const int x )
    { 
      comm_.send( x ); 
    };
    
    SenderClass()
    {
    };
};


int main()
{
  TEAGN_LOGMSG( "Test started." );

  TEAGN_DEBUG_MODE_CHECK;

  try{
    SenderClass sender;
    ReceiverClass* receiver1 = new ReceiverClass;
    ReceiverClass* receiver2 = new ReceiverClass;
    
    sender.SetInt( (int) 100 );
    
    TEAGN_TRUE_OR_THROW( receiver1->comm_.connect( sender.comm_ ),
      "Unable to connect" );
    
    sender.SetInt( (int) 10 );
    
    TEAGN_TRUE_OR_THROW( ( receiver1->GetValue() == 10.0 ),
      "Invalid value" );
      
    TEAGN_TRUE_OR_THROW( receiver2->comm_.connect( sender.comm_ ),
      "Unable to connect" );
    
    sender.SetInt( (int) 20 );
    
    TEAGN_TRUE_OR_THROW( ( receiver1->GetValue() == 20.0 ),
      "Invalid value" );
      
    TEAGN_TRUE_OR_THROW( ( receiver2->GetValue() == 20.0 ),
      "Invalid value" );
      
    delete receiver2;
      
    sender.SetInt( (int) 30 );
    
    TEAGN_TRUE_OR_THROW( ( receiver1->GetValue() == 30.0 ),
      "Invalid value" );
      
    receiver1->comm_.disconnect( sender.comm_ );
    
    sender.SetInt( (int) 40 );
      
    delete receiver1;
      
    sender.SetInt( (int) 40 );
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
