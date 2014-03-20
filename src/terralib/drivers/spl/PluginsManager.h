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

#ifndef PLUGINSMANAGER_H
  #define PLUGINSMANAGER_H
  
  #include "../../../libspl/spl.h"
  #include "PluginInfo.h"
  #include "PluginParameters.h"
  
  #include <string>
  #include <vector>

  #include "TeSPLDefines.h"

/* This is to enable the error messages when loading plugins for debug mode  */
#ifndef NDEBUG
  #if SPL_PLATFORM == SPL_PLATFORM_WIN32
    #include <windows.h>
  #elif SPL_PLATFORM == SPL_PLATFORM_LINUX || SPL_PLATFORM == SPL_PLATFORM_APPLE
    #include <dlfcn.h>
  #else
    #error "Unsuported Plataform"
  #endif
#endif

  /**
   * @brief A class to deal with dynamic plugin loading/unloading.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   */
  class TLSPL_DLL PluginsManager
  {
    public :
    
      /**
       * Default constructor.
       */    
      PluginsManager();

      /**
       * Default destructor.
       */       
      virtual ~PluginsManager();
      
      /**
       * Set the default plugins parameters that will be used when 
       * calling the runPlugin function.
       */    
      virtual void setDefaultPluginsParameters( const PluginParameters& pp );      
      
      /**
       * Load all plugins from the supplied directory path.
       *
       * @param path The directory path.
       * @return The number of loaded plugins (including invalid/incompatible).
       */       
      virtual unsigned int loadPlugins( const std::string& path );
      
      /**
       * Unload all plugins.
       */       
      virtual void unloadPlugins();      

      /**
       * Execute a plugin code.
       *
       * @param pinf The plugin's info.
       */       
      virtual bool runPlugin( const PluginInfo& pinfo ) const;
      
      /**
       * Build a vector of the currently loaded valid plugins info.
       *
       * @param plist A output vector reference.
       */       
      virtual void getValidPluginsList( std::vector< PluginInfo >& plist ) const;
      
      /**
       * Build a vector of the currently loaded invalid plugins info.
       *
       * @param plist A output vector reference.
       */       
      virtual void getInvalidPluginsList( std::vector< PluginInfo >& plist ) const;      
      
    protected :
      
      /**
       * A definition of loaded plugins info mapping to SPL internal
       * list indexes.
       */       
      typedef std::pair< PluginInfo, long > SPLPluginInfoT;
      
      /**
       * A pointer to the SPL server.
       */        
      void* pluginServerHandlerPtr_;

      /**
       * A vector holding information of all loaded plugins.
       */        
      std::vector< SPLPluginInfoT > valid_plugins_;
      
      /**
       * A vector holding information of all loaded plugins.
       */        
      std::vector< SPLPluginInfoT > invalid_plugins_;      
      
      /**
       * The default plugins parameters that will be used when calling
       * the plugins run function.
       */        
      PluginParameters default_pp_;
  };

#endif
