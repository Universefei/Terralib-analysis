#include "PluginsManager.h"

#include "../../kernel/TeUtils.h"

#include <algorithm>

PluginsManager::PluginsManager()
{
  pluginServerHandlerPtr_ = 0;
}


PluginsManager::~PluginsManager()
{
  unloadPlugins();
}


void PluginsManager::setDefaultPluginsParameters( const PluginParameters& pp )
{
  default_pp_ = pp;
}


unsigned int PluginsManager::loadPlugins( const std::string& path )
{
  unloadPlugins();
  
  /* Creating the new plugins server instance */
  
  pluginServerHandlerPtr_ = (void*) new slcPluginServer;
  
  slcPluginServer* pluginServerPtr = 
    ( slcPluginServer* ) pluginServerHandlerPtr_;
    
  /* Creating the loading time plugin parameters pointer using the current 
     internal parameters */
     
  slcPluginArgs pargs;
  long arg_index = -1;
  arg_index = pargs.SetArg( (void*)( &default_pp_ ) );    
  
  long loaded_dlls = 0;

  #if SPL_PLATFORM == SPL_PLATFORM_WIN32
    loaded_dlls = pluginServerPtr->LoadAllPlugins( path.c_str(), "*.dll",
      false, &pargs );
  #elif SPL_PLATFORM == SPL_PLATFORM_LINUX
    loaded_dlls = pluginServerPtr->LoadAllPlugins( path.c_str(), "*.so",
      false, &pargs );
    
    #ifndef NDEBUG
      const char* plugins_load_error_str = dlerror();
      if( plugins_load_error_str != 0 ) {
        std::cout << std::endl << "TerraView: Error loading plugins - " <<
          plugins_load_error_str << std::endl;
      }
    #endif
  #else
    #error "Unsuported Plataform"
  #endif
  
  if( loaded_dlls > 0 ) {
    pluginServerPtr->GetAllPluginInfos();
    
    for( long plugin_index = 0 ; plugin_index < loaded_dlls ; 
        ++plugin_index ) {
         
      slcPluginInfo pluginInfo = 
        pluginServerPtr->GetSafedPluginInfo( plugin_index );
        
      SPLPluginInfoT pinfo;
      pinfo.first.plugin_name_ =  pluginInfo.GetName();
      pinfo.first.plugin_type_ = pluginInfo.GetUUID();
      pinfo.first.plugin_interface_version_ = pluginInfo.GetInfo();
      pinfo.first.plugin_build_number_ = 
        Te2String( pluginInfo.GetBuildVersion() );
      pinfo.first.plugin_major_version_ = 
        Te2String( pluginInfo.GetMajorVersion() );
      pinfo.first.plugin_minor_version_ = 
        Te2String( pluginInfo.GetMinorVersion() );
      pinfo.first.plugin_description_ = pluginInfo.GetDescription();
      pinfo.first.plugin_vendor_ = pluginInfo.GetVendor();
      pinfo.first.plugin_vendor_email_ = pluginInfo.GetEmail();
      pinfo.first.plugin_url_ = pluginInfo.GetHomepage();
      pinfo.second = plugin_index;
        
      std::vector< std::string > plugin_menu_entries;
      TeSplitString( pinfo.first.plugin_type_, ".", plugin_menu_entries );
        
      if( plugin_menu_entries.size() > 0 ) {
        if( plugin_menu_entries[ 0 ] == "TerraViewPlugin" ) {
          if( pinfo.first.plugin_interface_version_ == 
            PLUGINPARAMETERS_VERSION ) {
            
            if( pluginServerPtr->RunPluginInit( plugin_index, &pargs ) ) {
              valid_plugins_.push_back( pinfo );
            } else {
              invalid_plugins_.push_back( pinfo );
            }
          } else {
            invalid_plugins_.push_back( pinfo );
          }
        }
      }
    }
  }
  
  return loaded_dlls;
}


void PluginsManager::unloadPlugins()
{
  if( pluginServerHandlerPtr_ != 0 ) {
    /* Casting back the plugin server pointer */
    
    slcPluginServer* pluginServerPtr = 
      ( slcPluginServer* ) pluginServerHandlerPtr_;
      
    /* Creating the unloading time plugin parameters pointer using the current 
      internal parameters */
       
    slcPluginArgs pargs;
    long arg_index = -1;
    arg_index = pargs.SetArg( (void*)( &default_pp_ ) );       
    
    /* Unloading valid plugins */
    
    std::vector< SPLPluginInfoT >::iterator it = valid_plugins_.begin();
    std::vector< SPLPluginInfoT >::iterator it_end = valid_plugins_.end();
    
    while( it != it_end ) {
      pluginServerPtr->UnloadPlugin( it->second, true, &pargs );
      ++it;
    }
  
    /* Unloading invalid plugins */
    
    it = invalid_plugins_.begin();
    it_end = invalid_plugins_.end();
    
    while( it != it_end ) {
      pluginServerPtr->UnloadPlugin( it->second, false, &pargs );
      ++it;
    }
    
    /* Releasing memory */
  
    delete pluginServerPtr;
    
    pluginServerHandlerPtr_ = 0;
    
    valid_plugins_.clear();
    invalid_plugins_.clear();
  }
}


void PluginsManager::getValidPluginsList( 
  std::vector< PluginInfo >& plist ) const
{
  std::vector< SPLPluginInfoT >::const_iterator it = valid_plugins_.begin();
  std::vector< SPLPluginInfoT >::const_iterator it_end = valid_plugins_.end();
  
  plist.clear();
  
  while( it != it_end ) {
    plist.push_back( it->first );
  
    ++it;
  }
}


void PluginsManager::getInvalidPluginsList( 
  std::vector< PluginInfo >& plist ) const
{
  std::vector< SPLPluginInfoT >::const_iterator it = invalid_plugins_.begin();
  std::vector< SPLPluginInfoT >::const_iterator it_end = 
    invalid_plugins_.end();
  
  plist.clear();
  
  while( it != it_end ) {
    plist.push_back( it->first );
  
    ++it;
  }
}


bool PluginsManager::runPlugin( const PluginInfo& pinfo ) const
{
  if( pluginServerHandlerPtr_ != 0 ) {
    slcPluginServer* pluginServerPtr = 
      ( slcPluginServer* ) pluginServerHandlerPtr_;
  
    for( unsigned int index = 0 ; index < valid_plugins_.size() ; ++index ) {
      if( valid_plugins_[ index ].first == pinfo ) {
      
        slcPluginArgs pargs;
        long arg_index = -1;
        arg_index = pargs.SetArg( (void*)( &default_pp_ ) );
        
        return pluginServerPtr->RunPlugin( 
          valid_plugins_[ index ].second, &pargs );
      }
    }
  }

  return false;
}
