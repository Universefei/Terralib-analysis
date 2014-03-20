/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/

#ifndef TEPDIPIMANAGER_HPP
  #define TEPDIPIMANAGER_HPP
  
  #include "TePDIPIManagerGlobalSettings.hpp"
  
  #include "../kernel/TeProgress.h"
  
  #include <string>

  /**
   * @class TePDIPIManager
   * @brief This is the class for the active progress interface management.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @note This class is not thread safe by itself but multiple threads
   * can access the global progress interface by instantiating their own
   * TePDIPIManager instances.
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIPIManager {
  
    public :
    
      /**
       * @brief Default Constructor
       *
       * @param message Progress window message (label).
       * @param tsteps Progress window total steps.
       * @param enabled Flag to enable ( true ) or disable ( false ).
       */
      TePDIPIManager( const std::string& message = std::string(), 
        unsigned long int tsteps = 0, bool enabled = false );    
    
      /**
       * @brief Default Destructor
       */
      ~TePDIPIManager();
      
      /**
       * @brief Enable / Disable the current progress interface manager.
       *
       * @param enabled Flag to enable ( true ) or disable ( false ).
       */
      void Toggle( bool enabled );  
      
      /**
       * @brief Update the current progress interface to the supplied step.
       *
       * @param step Current step.
       * @return true if the interface was canceled, false otherwise.
       */
      bool Update( unsigned long int step );      
      
      /**
       * @brief Incriments the the current progress by one step.
       * @return true if the interface was canceled, false otherwise.
       */
      inline bool Increment()
      {
        return Update( (int)( curr_step_ + 1 ) );
      };

      /**
       * @brief Reset the current instance state.
       * 
       * @param message Message (label).
       * @param tsteps Total steps.
       */
      void Reset( const std::string& message = std::string(), 
        unsigned long int tsteps = 0 );
        
      /**
       * @brief Returns true if the progress was canceled.
       * @return Returns true if the progress was canceled.
       */
      inline bool wasCanceled()
      {
        return ( prog_intef_ptr_ ? prog_intef_ptr_->wasCancelled() : false );
      };         
        
    protected :
    
      /**
        * @brief A flag indicating if the PI manager is enabled.
        */
      bool enabled_;    

      /**
        * @brief The total steps number.
        */
      unsigned long int total_steps_;

      /**
        * @brief The current step.
        */
      unsigned long int curr_step_;
      
      /**
        * @brief A reference to the global settings instance.
        */      
      TePDIPIManagerGlobalSettings& global_settings_;
      
      /**
        * @brief A pointer to the active progress instance.
        * @note null if no active instance is present.
        */        
      TeProgressBase* prog_intef_ptr_;
      
      /**
        * @brief The progress interface message (label).
        */
      std::string message_;    
     
      /**
       * @brief Updates the progress interface with the new settings.
       * @note NO LOCK DONE !!
       * @param settings The new progress interface manager to be applied.
       * @param prog_intef_ptr Progres interface pointer.
       * @return true if the interface was canceled, false otherwise.
       */         
      bool updateProgressInterface() const;
        
    private :
    
      /**
       * @brief Alternative Constructor
       */    
      TePDIPIManager( const TePDIPIManager& );
      
      /**
       * @brief operator= overload.
       */    
      const TePDIPIManager& operator=( const TePDIPIManager& ) 
      { 
        return *this;
      };
  };

#endif
