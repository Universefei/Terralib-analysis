/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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

#ifndef TESEMAPHORE_H
  #define TESEMAPHORE_H

  #include "TeDefines.h"
  #include "TeAgnostic.h"
  #include "TeDefines.h"
  
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    #include <windows.h>
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    #include <semaphore.h>
  #else
    #error "ERROR: Unsupported platform"
  #endif  
 
  /**
   * @brief Inter process communication semaphore.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup MultProgToolsGroup
   */
  class TL_DLL TeSemaphore
  {
    public :
    
      /**
       * @brief Creates an unnamed semaphore.
       * @param count Semaphore initial counter value.
       */    
      TeSemaphore( unsigned int count );
      
      /**
       * @brief Creates an named semaphore.
       * @param count Semaphore initial counter value.
       * @param semName Semaphore name.
       * @note  Creates a new semaphore or opens an existing semaphore
       * that can be shared among processes.
       */    
      TeSemaphore( unsigned int count, const std::string& semName );

      /**
       * @brief Default destructor.
       */       
      ~TeSemaphore();
      
      /**
       * @brief Increments the semaphore's counter.  
       */          
      inline void post()
      {
        #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
          TEAGN_TRUE_OR_THROW( ReleaseSemaphore( semHandlerPtr_, 1, 0 ),
            "Error in semaphore post" );
        #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
          TEAGN_TRUE_OR_THROW( 0 == sem_post( semHandlerPtr_ ),
            "Error in semaphore post" );
        #else
          #error "Unsuported plataform"
        #endif           
      };
      
      /**
       * @brief Wait the semaphore's counter to become zero.  
       * @note If the semaphore's value is greater than zero, then the 
       * decrement proceeds, and  the  function  returns,  immediately.
       * If the semaphore currently has the value zero, then the call blocks 
       * until either it becomes possible to perform the decrement.
       */          
      inline void wait()
      {
        #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
          TEAGN_TRUE_OR_THROW( WAIT_OBJECT_0 == WaitForSingleObject( 
            semHandlerPtr_, INFINITE ), "Error waiting semaphore" );
        #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
          TEAGN_TRUE_OR_THROW( 0 == sem_wait( semHandlerPtr_ ),
            "Error in semaphore wait" );
        #else
          #error "Unsuported plataform"
        #endif            
      };
      
      /**
       * @brief Wait the semaphore's counter to become zero.
       * @param waitingTime The maximum waiting time in milliseconds
       * ( 0 == INFINITE ).
       * @return true if the semaphore's counter is zero or false 
       * if the waiting time has finished or an error occurred.
       */       
      bool wait( unsigned int waitingTime );  
           
    protected :
   
      #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
        /** A pointer to the handler instance. */      
        HANDLE semHandlerPtr_;    
      #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
        /** A pointer to the handler instance. */      
        sem_t* semHandlerPtr_;
      #else
        #error "Unsuported plataform"
      #endif   
      
      /** This instance name */
      std::string semName_;
   
    private :
    
      /**
       * Alternative constructor.
       */    
      TeSemaphore(  const TeSemaphore& ) {};
   
      /**
       * operator= overload.
       * @return A const reference to the current instance.
       */      
      const TeSemaphore& operator=( const TeSemaphore& ) { return *this; };
 
  };

#endif
