#include "TeThreadJobsManager.h"
#include "TeUtils.h"
#include "TeThreadFunctor.h"

#include <algorithm>

TeThreadJobsManager::TeThreadJobsManager()
{
  managerData_.maxSimulJobs_ = TeGetPhysProcNumber();
}

TeThreadJobsManager::~TeThreadJobsManager()
{
  clearAwaitingJobs();
  
  managerData_.mutex_.lock();
  
  TEAGN_TRUE_OR_THROW( managerData_.runningJobsList_.size() == 0,
    "Trying to delete an TeThreadJobsManager with active jobs running" );
  
  managerData_.mutex_.unLock();  
}

TeThreadJob const* TeThreadJobsManager::executeJob( 
  const TeThreadJob& job )
{
  managerData_.mutex_.lock();
  
  TeThreadJob* newJobPtr = 0;
  
  if( managerData_.maxSimulJobs_ == 0 )
  {
    newJobPtr = job.clone();
    newJobPtr->executeJobCode();
    delete newJobPtr;
  }
  else
  {
    // Inserting job into awaiting jobs list
    
    newJobPtr = job.clone();
    managerData_.awaitingJobsList_.push_back( newJobPtr );
    
    // Starting a thread to do the job
    
    if( managerData_.threadsList_.size() == 0 )
    {
      TeThreadFunctor* newThreadPtr = new TeThreadFunctor();
      newThreadPtr->setParametersPtr( (void*)(&managerData_) );
      newThreadPtr->setStartFunctPtr( threadFunction );
      
      managerData_.threadsList_.push_back( newThreadPtr );
      
      TEAGN_TRUE_OR_THROW(newThreadPtr->start(), "Thread start error" );
    }
    else if( managerData_.threadsList_.size() < 
      managerData_.maxSimulJobs_ )
    {
      // locating a stopped thread
      
      std::list< TeThreadFunctor* >::iterator it = 
        managerData_.threadsList_.begin();
      std::list< TeThreadFunctor* >::iterator it_end = 
        managerData_.threadsList_.end();
      
      bool noStoppedThreadWasFound = true;
      
      while( it != it_end )
      {
        if( (*it)->getStatus() == TeThreadStopped )
        {
          noStoppedThreadWasFound = false;
          TEAGN_TRUE_OR_THROW( (*it)->start(), "Thread start error" );
          break;
        }
        
        ++it;
      }
      
      // no stopped threads found
      
      if( noStoppedThreadWasFound )
      {
        TeThreadFunctor* newThreadPtr = new TeThreadFunctor();
        newThreadPtr->setParametersPtr( (void*)(&managerData_) );
        newThreadPtr->setStartFunctPtr( threadFunction );
        
        managerData_.threadsList_.push_back( newThreadPtr );
        
        TEAGN_TRUE_OR_THROW(newThreadPtr->start(), "Thread start error" );      
      }
    }
    else
    {
      // counting the number of working threads
      
      std::list< TeThreadFunctor* >::iterator it = 
        managerData_.threadsList_.begin();
      std::list< TeThreadFunctor* >::iterator it_end = 
        managerData_.threadsList_.end();
      
      unsigned long int runningThreads = 0;
      
      while( it != it_end )
      {
        if( (*it)->getStatus() == TeThreadRunning )
        {
          ++runningThreads;
        }
        
        ++it;
      }
      
      // try to activate a new thread       
      
      if( runningThreads < managerData_.maxSimulJobs_ )
      {
        it = managerData_.threadsList_.begin();
        
        while( it != it_end )
        {
          if( (*it)->getStatus() == TeThreadStopped )
          {
            TEAGN_TRUE_OR_THROW( (*it)->start(), "Thread start error" );
            break;
          }
          
          ++it;
        }      
      }
    }
  }
  
  // Delete other stopped threads following maxSimulJobs_
  
  if( managerData_.threadsList_.size() > 
    managerData_.maxSimulJobs_ )
  {
    std::list< TeThreadFunctor* >::iterator it = 
      managerData_.threadsList_.begin();
    std::list< TeThreadFunctor* >::iterator it_end = 
      managerData_.threadsList_.end();    
    
    while( it != it_end )
    {
      if( (*it)->getStatus() == TeThreadStopped )
      {
        std::list< TeThreadFunctor* >::iterator it_aux = it;
        --it_aux;
        
        delete (*it);
        
        managerData_.threadsList_.erase( it );
        
        it = it_aux;
      }
      
      if( managerData_.threadsList_.size() == 
        managerData_.maxSimulJobs_)
      {
        break;
      }
      
      ++it;
    }    
  }  
  
  managerData_.mutex_.unLock();
  
  return newJobPtr;
}

void TeThreadJobsManager::setMaxSimulJobs( unsigned long int maxSimulJobs )
{
  managerData_.maxSimulJobs_ = maxSimulJobs;
}

void TeThreadJobsManager::setAutoMaxSimulJobs()
{
  managerData_.maxSimulJobs_ = TeGetPhysProcNumber();
}

unsigned long int TeThreadJobsManager::getMaxSimulJobs() const
{
  return managerData_.maxSimulJobs_;
}

TeThreadSignal& TeThreadJobsManager::getJobFinishedSignal()
{
  return managerData_.jobFinishedSig_;
}

TeThreadJobsManager::JobStatus TeThreadJobsManager::getJobStatus( 
  TeThreadJob const* const jobId )
{
  managerData_.mutex_.lock();
  
  // trying to locate inside awaiting jobs list
  
  JobsListT::iterator it = managerData_.awaitingJobsList_.begin(); 
  JobsListT::iterator it_end = managerData_.awaitingJobsList_.end();
  
  while( it != it_end )
  {
    if( *it == jobId )
    {
      managerData_.mutex_.unLock();
      return JobAwaiting;
    }
    
    ++it;
  }
  
  // trying to locate inside running jobs list
  
  it = managerData_.runningJobsList_.begin(); 
  it_end = managerData_.runningJobsList_.end();
  
  while( it != it_end )
  {
    if( *it == jobId )
    {
      managerData_.mutex_.unLock();
      return JobRunning;
    }
    ++it;
  }
  
  // job not found

  managerData_.mutex_.unLock();
  return JobNotRunning;
}

unsigned long int TeThreadJobsManager::getRunningJobsNumber()
{
  managerData_.mutex_.lock();
  
  unsigned long int number = managerData_.runningJobsList_.size();
  
  managerData_.mutex_.unLock();
  
  return number;
}

unsigned long int TeThreadJobsManager::getAwaitingJobsNumber()
{
  managerData_.mutex_.lock();
  
  unsigned long int number = managerData_.awaitingJobsList_.size();
  
  managerData_.mutex_.unLock();
  
  return number;
}

void TeThreadJobsManager::waitAllToFinish()
{
  std::list< TeThreadFunctor* >::iterator it = 
    managerData_.threadsList_.begin();
  std::list< TeThreadFunctor* >::iterator it_end = 
    managerData_.threadsList_.end();
  
  while( it != it_end )
  {
    (*it)->waitToFinish();
    
    ++it;
  }  
}

void TeThreadJobsManager::clearAwaitingJobs()
{
  managerData_.mutex_.lock();
  
  JobsListT::iterator it = managerData_.awaitingJobsList_.begin(); 
  JobsListT::iterator it_end = managerData_.awaitingJobsList_.end();
  
  while( it != it_end )
  {
    delete (*it);
    
    ++it;
  } 
  
  managerData_.mutex_.unLock();
}

bool TeThreadJobsManager::threadFunction( void* parsPtr )
{
  InternalManagerDataStruct* managerData = (InternalManagerDataStruct*)
    parsPtr;
  
  managerData->mutex_.lock();
  
  if( managerData->awaitingJobsList_.size() == 0 )
  {
    managerData->mutex_.unLock();
    
    return true;
  }
  else
  {
    do
    {
      // Pick up a job
      
      TeThreadJob* jobPtr = managerData->awaitingJobsList_.front();
      managerData->awaitingJobsList_.pop_front();
      
      // move it to running jobs list
      
      managerData->runningJobsList_.push_back( jobPtr );
      
      managerData->mutex_.unLock();
      
      // Execute job
      
      jobPtr->executeJobCode();
      
      // delete job object and remove job from running jobs list
      
      managerData->mutex_.lock();
      
      managerData->runningJobsList_.erase( std::find(
        managerData->runningJobsList_.begin(),
        managerData->runningJobsList_.end(),
        jobPtr ) );
      
      delete jobPtr;
      
      // Emitting a job complete signal
      
      managerData->jobFinishedSig_.emitSignal();
      
    } while( managerData->awaitingJobsList_.size() > 0 );
      
    managerData->mutex_.unLock();
    
    return true;
  }
}

