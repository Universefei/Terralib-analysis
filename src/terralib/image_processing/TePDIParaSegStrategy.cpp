#include "TePDIParaSegStrategy.hpp"

#include "../kernel/TeAgnostic.h"

#include <limits.h>

TePDIParaSegStrategy::TePDIParaSegStrategy( const TePDIParaSegStrategyParams& 
  params )
  : lastGenSegID_( 0 ), params_( params )
{
}


TePDIParaSegStrategy::~TePDIParaSegStrategy()
{
}

TePDIParaSegSegment::SegIdDataType TePDIParaSegStrategy::acquireNewSegID()
{
  params_.globalMutexPtr_->lock();
  
  if( (*params_.segmentsIdsCounterPtr_) == UINT_MAX )
  {
    params_.globalMutexPtr_->unLock();
    
    return 0;
  }
  else
  {
    ++(*params_.segmentsIdsCounterPtr_);
    lastGenSegID_ = (*params_.segmentsIdsCounterPtr_);
    params_.globalMutexPtr_->unLock();
    return lastGenSegID_;
  }
}

TePDIParaSegStrategy* TePDIParaSegStrategy::DefaultObject( 
  const TePDIParaSegStrategyFactoryParams& ) 
{ 
  TEAGN_LOG_AND_THROW( "Unable to build a TePDIParaSegStrategy instance");
  return 0;
}

