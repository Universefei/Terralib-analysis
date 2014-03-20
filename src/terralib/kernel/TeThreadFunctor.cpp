#include "TeThreadFunctor.h"
#include "TeAgnostic.h"

  
TeThreadFunctor::TeThreadFunctor()
	: threadReturnValue_(false), threadStartFuncPtr1_(0), 
	threadStartFuncPtr2_(0), userParamsPtr_( 0 )
{
}

TeThreadFunctor::~TeThreadFunctor()
{
}

bool TeThreadFunctor::getReturnValue() const
{
  return threadReturnValue_;
}

void TeThreadFunctor::setStartFunctPtr( TeThreadStartFunctT1 startFuncPtr )
{
  TEAGN_TRUE_OR_THROW( ( threadStatus_ == TeThreadStopped ),
    "Cannot change start function pointer of a running thread" )

	TEAGN_TRUE_OR_THROW( ( startFuncPtr != 0 ),
	  "Invalid thread start function pointer" )

  threadStartFuncPtr1_ = startFuncPtr;
  threadStartFuncPtr2_ = 0;
}

void TeThreadFunctor::setStartFunctPtr( TeThreadStartFunctT2 startFuncPtr )
{
  TEAGN_TRUE_OR_THROW( ( threadStatus_ == TeThreadStopped ),
    "Cannot change start function pointer of a running thread" )

  TEAGN_TRUE_OR_THROW( ( startFuncPtr != 0 ),
             "Invalid thread start function pointer" )

  threadStartFuncPtr1_ = 0;
  threadStartFuncPtr2_ = startFuncPtr;
}

void TeThreadFunctor::setParameters(const TeThreadParameters& params)
{
	TEAGN_TRUE_OR_THROW( ( threadStatus_ == TeThreadStopped ),
    "Cannot change thread parameters of a running thread" )

	threadUserParams_ = params;
}


void TeThreadFunctor::setParametersPtr( void* userParamsPtr )
{
  TEAGN_TRUE_OR_THROW( ( threadStatus_ == TeThreadStopped ),
    "Cannot change thread parameters of a running thread" )
  
  userParamsPtr_ = userParamsPtr;
}


void TeThreadFunctor::run()
{
  threadReturnValue_ = false;

	if( threadStartFuncPtr1_ )
	{
	  threadReturnValue_ = threadStartFuncPtr1_(threadUserParams_);
	  threadUserParams_.clear();
	}
	else if( threadStartFuncPtr2_ )
	{
	  threadReturnValue_ = threadStartFuncPtr2_(userParamsPtr_);
	}
}

