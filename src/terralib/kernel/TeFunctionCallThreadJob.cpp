#include "TeFunctionCallThreadJob.h"

TeFunctionCallThreadJob::TeFunctionCallThreadJob() 
{
  jobFunc_ = 0;
  jobFuncParsPtr_ = 0;
};

TeFunctionCallThreadJob::~TeFunctionCallThreadJob() 
{
};

void TeFunctionCallThreadJob::executeJobCode() 
{
  if( jobFunc_ ) jobFunc_( jobFuncParsPtr_);
};

TeThreadJob* TeFunctionCallThreadJob::clone() const
{
  TeFunctionCallThreadJob* clonedJobPtr = new TeFunctionCallThreadJob;
  clonedJobPtr->jobFunc_ = jobFunc_;
  clonedJobPtr->jobFuncParsPtr_ = jobFuncParsPtr_;
  
  return clonedJobPtr;
}
