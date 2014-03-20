
#include <terralib/kernel/TeFunctionCallThreadJob.h>
#include <terralib/kernel/TeThreadJobsManager.h>
#include <terralib/kernel/TeThread.h>
#include <terralib/kernel/TeAgnostic.h>
#include <terralib/kernel/TeUtils.h>

#include <stdlib.h>

TeMutex globalMutex_;

void jobFunction( void* parsPtr )
{
  globalMutex_.lock();
  
  unsigned int* counterPtr = (unsigned int*)parsPtr;
  
  (*counterPtr) = (*counterPtr) + 1;
  
  TEAGN_LOGMSG( "Job Running, counter=" + Te2String( (*counterPtr) ) );
  
  globalMutex_.unLock();
  
  TeDelayThread(5);
}

int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
    
    unsigned int counter = 0;
    
    TeFunctionCallThreadJob job;
    job.jobFunc_ = jobFunction;
    job.jobFuncParsPtr_ = (void*)&counter;
    
    TeThreadJobsManager jobsMan;
    jobsMan.setMaxSimulJobs( 5 );
    
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    jobsMan.executeJob( job );
    
    jobsMan.waitAllToFinish();
    
    TEAGN_TRUE_OR_THROW( counter == 10, "invalid counter value" );
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
