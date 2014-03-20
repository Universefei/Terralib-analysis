/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/


#ifndef __TERRALIB_INTERNAL_TETRHEADJOBSMANAGER_H
#define __TERRALIB_INTERNAL_TETRHEADJOBSMANAGER_H

#include "TeThreadJob.h"
#include "TeThreadSignal.h"
#include "TeMutex.h"
#include "TeDefines.h"

#include <list>

class TeThreadFunctor;

/** \brief A class to control the execution of concurrent jobs using
 *  spawned threads.
 *  \author Emiliano F. Castejon <castejon@dpi.inpe.br>
 *  \ingroup MultProgToolsGroup
 */  
class TL_DLL TeThreadJobsManager
{
  public :
    /*! Job status */
    enum JobStatus { JobRunning, JobAwaiting, JobNotRunning };
        
    TeThreadJobsManager();             

    ~TeThreadJobsManager();
    
    /*! \brief Add a job to the waiting jobs list to be executed 
     * when a thread becomes available or execute the job if the
     * maximum number of simultâneous jobs is set to zero.
     * \param job Job to be processed.
     * \return A unique job ID as an intenal address for the added 
     * job.
     */    
    TeThreadJob const* executeJob( const TeThreadJob& job );
    
    /*! Set the number of threaded jobs that can be executed simultaneously.
     * \param maxSimulJobs The number of jobs that can be executed simultaneously
     * (if set to zero, no thread will be spawned and the job execution
     * will block the current process until it finishes).
     */
    void setMaxSimulJobs( unsigned long int maxSimulJobs );
    
    /*! Set the number of jobs that can be executed simultaneously using
     * the number of physical processing units (this is the class default).
     */
    void setAutoMaxSimulJobs();    
    
    /*! Get the number of jobs that can be executed simultaneously.
     * \return The number of jobs that can be executed simultaneously.
     */
    unsigned long int getMaxSimulJobs() const;    
    
    /*! \brief A reference to a signal that will be emitted on
     * each job completion.
     * \return A reference to a signal that will be emitted on
     * each job completion.
     */     
    TeThreadSignal& getJobFinishedSignal();
    
    /*! \brief Return a job status.
     * \param jobId Job ID.
     * \return The job status.
     */
    JobStatus getJobStatus( TeThreadJob const* const jobId );
    
    /*! \brief Return number of running jobs.
     * \return The number of running jobs.
     */    
    unsigned long int getRunningJobsNumber();

    /*! \brief Return number of awaiting jobs.
     * \return The number of awaiting jobs.
     */    
    unsigned long int getAwaitingJobsNumber();
    
    /*! \brief Wait all jobs to finish.
     */      
    void waitAllToFinish();

    /*! \brief Clear all waiting jobs (not executed yet).
     */      
    void clearAwaitingJobs();
      
  protected :
    
    /*! Jobs list type definition */
    typedef std::list< TeThreadJob* > JobsListT;
    
    struct InternalManagerDataStruct
    {
      /*! Maximum number of simultaneous running jobs */
      unsigned long int maxSimulJobs_;
      
      /*! A signal emitted on each job completion. */
      TeThreadSignal jobFinishedSig_;
      
      /*! The awaiting jobs list. */
      JobsListT awaitingJobsList_;
      
      /*! The running jobs list. */
      JobsListT runningJobsList_;
      
      /*! The thread instances list. */
      std::list< TeThreadFunctor* > threadsList_;
      
      /*! A mutex instance to control access to internal resources. */
      TeMutex mutex_;
    };
    
    InternalManagerDataStruct managerData_;

    /*! \brief Entry point for all threads.
     * \param parsPtr Parameters pointer.
     * \return true if OK.
     */
    static bool threadFunction( void* parsPtr );

};
  
#endif

