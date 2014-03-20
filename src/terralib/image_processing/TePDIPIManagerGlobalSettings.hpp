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

#ifndef TEPDIPIMANAGERGLOBALSETTINGS_HPP
  #define TEPDIPIMANAGERGLOBALSETTINGS_HPP
  
  #include "TePDIDefines.hpp"
  #include "TePDIPIManager.hpp"
  
  #include "../kernel/TeMutex.h"
  #include "../kernel/TeSingleton.h"
  
  #include <string>

  /**
    * @brief Progress interface management global settings.
    * @ingroup PDIAux
    */    
  class PDI_DLL TePDIPIManagerGlobalSettings : public 
    TeSingleton< TePDIPIManagerGlobalSettings > {
    
      friend class TePDIPIManager;
      
    public :
    
      /**
        * @brief Default constructor.
        */
      TePDIPIManagerGlobalSettings()
      {
        total_steps_ = 0;
        curr_step_ = 0;
        curr_prop_step_ = 0;
        active_manager_instances_ = 0;
      };         
    
      /**
        * @brief Default desstructor.
        */
      ~TePDIPIManagerGlobalSettings() {};
     
    protected :
    
      /**
        * @brief The lock instance needed to update the global
        * settings.
        */
      TeMutex lock_;
              
      /**
        * @brief The total steps number.
        */
      volatile unsigned long int total_steps_;
      
      /**
        * @brief The current step.
        */
      volatile unsigned long int curr_step_;
      
      /**
        * @brief The current proportional step ( 0 up to 100 ).
        */
      volatile unsigned long int curr_prop_step_;    
      
      /**
        * @brief The number of TePDIPIManager active instances.
        */
      volatile unsigned long int active_manager_instances_;                   
      
      /**
        * @brief The progress interface message (label).
        */
      std::string curr_message_;          
  
  };     
      
#endif    
