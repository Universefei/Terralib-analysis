/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

#ifndef PLUGININFO_H
  #define PLUGININFO_H
  
  #include <string>

  #include "TeSPLDefines.h"
  
  /**
   * @brief A class to deal with plugin information.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   */
  class TLSPL_DLL PluginInfo
  {
    public :
    
      /**
       * Plugin name.
       */
      std::string plugin_name_;
      
      /**
       * Plugin type (UUID).
       */
      std::string plugin_type_;
      
      /**
       * Plugin interface version.
       */
      std::string plugin_interface_version_;
      
      /**
       * Plugin build number.
       */
      std::string plugin_build_number_;      

      /**
       * Plugin major version.
       */
      std::string plugin_major_version_;      
      
      /**
       * Plugin minor version.
       */
      std::string plugin_minor_version_;           
      
      /**
       * Plugin general description.
       */
      std::string plugin_description_;           

      /**
       * Plugin vendor.
       */
      std::string plugin_vendor_;      
      
      /**
       * Plugin vendor e-mail.
       */
      std::string plugin_vendor_email_;              

      /**
       * Plugin URL.
       */
      std::string plugin_url_;           

      /**
       * Default Constructor.
       */
      PluginInfo() {};
      
      /**
       * @brief operator== overload.
       * @param external External reference.
       * @return true if equal.
       */
      bool operator==( const PluginInfo& external ) const
      {
        if( 
            ( plugin_name_ == external.plugin_name_ ) &&
            ( plugin_type_ == external.plugin_type_ ) &&
            ( plugin_interface_version_ == 
              external.plugin_interface_version_ ) &&
            ( plugin_build_number_ == external.plugin_build_number_ ) &&
            ( plugin_major_version_ == external.plugin_major_version_ ) &&
            ( plugin_minor_version_ == external.plugin_minor_version_ ) &&
            ( plugin_description_ == external.plugin_description_ ) &&
            ( plugin_vendor_ == external.plugin_vendor_ ) &&
            ( plugin_vendor_email_ == external.plugin_vendor_email_ ) &&
            ( plugin_url_ == external.plugin_url_ ) ) {
          
          return true;          
        } else {
          return false;        
        }
      };
  };

#endif
