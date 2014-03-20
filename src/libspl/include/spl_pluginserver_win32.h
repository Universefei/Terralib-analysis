
/*!

\section terms_of_use Terms of Use

libspl - the simple plugin layer library.
Copyright (C) 2004 Andreas Loeffler and René Stuhr (www.unitedbytes.de).

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

\section author Author
Andreas Loeffler (www.unitedbytes.de)

*/

//---------------------------------------------------------------------------------------------------------------------------
/// @file		spl_pluginserver_win32.h
/// @version	1.0
/// @brief		The plugin server's Win32 implementation.
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGINSERVER_WIN32_H__
#define		__SPL_PLUGINSERVER_WIN32_H__

#ifndef		__SPL_PLUGINSERVER_H__
#include	"spl_pluginserver.h"
#endif


#if SPL_PLATFORM == SPL_PLATFORM_WIN32
	#if		defined ( SPL_MAKE_DLL )
		#define SPL_API		__declspec( dllexport )
	#elif	defined ( SPL_USE_DLL )
		#define SPL_API		__declspec( dllimport )
	#else 
		#define SPL_API	
	#endif
#else
	#define SPL_API 
#endif

namespace SPL 
{

//---------------------------------------------------------------------------------------------------------------------------
///	@brief		Plugin server class for Win32.
///	@ingroup	SPL
//---------------------------------------------------------------------------------------------------------------------------
class slcPluginServer : public slaPluginServer
{
	//
	//-----------------------------------------------------------------------------------------------------------------------
	//***********************************************     CON/DESTRUCTION     ***********************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default constructor.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcPluginServer( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default destructor.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API ~slcPluginServer( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//************************************************     MODIFICATION     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Retrieves information about one loaded plugin.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API slcPluginInfo GetPluginInfo( const long a_lIndex );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Automatically retrieves information about all loaded plugins.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API GetAllPluginInfos( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Loads all plugins with a given wildcard in a specified path.
	///	@param		a_strPath		[in] Valid path for searching the plugins.
	///	@param		a_strWildcard	[in] File wildcard (e.g. *.*, *.dll) for the plugins.
  /// @param    init_plugin After plugin loading, run the init function.
	///	@param		a_pPluginArgs	[in] Pointer to arguments for initializing all plugins after they have been loaded.
	///	@return		Number of successfully loaded plugins.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API LoadAllPlugins( const string&		  a_strPath, 
								 const string&		  a_strWildcard, bool init_plugin,
								 const slcPluginArgs* a_pPluginArgs = NULL );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Loads a plugin with the specified file name.
	///	@param		a_strFileName	[in] The plugin's file name.
  /// @param    init_plugin After plugin loading, run the init function.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's start function arguments.
	///	@return		The method returns one of the following values:
	///	@retval		-2 | Plugin was not found.
	///	@retval		-1 | Plugin's initialize function was not found.
	///	@retval		 0 | Error while executing the intialisation function.
	///	@retval		 1 | Plugin was successfully loaded.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API LoadPlugin( const string& a_strFileName, bool init_plugin, const slcPluginArgs* a_pPluginArgs = NULL );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads a plugin with the given index (0-based).
	///	@param		a_lIndex		[in] The plugin's index.
  /// @param    shutdown_plugin Run the plugin's shutdown function before unloading it.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's shutdown function arguments.
	///	@return		The method returns one of the following values:
	///	@retval		true	| The plugin was unloaded successfully.
	///	@retval		false	| Could not unload plugin.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual bool SPL_API UnloadPlugin( const long a_lIndex, bool shutdown_plugin, const slcPluginArgs* a_pPluginArgs = NULL );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads all plugins.
	///	@param		a_pPluginArgs	[in] Arguments for shutting down all plugins after they have been unloaded.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API UnloadAllPlugins( const slcPluginArgs* a_pPluginArgs = NULL );
  
  //-----------------------------------------------------------------------------------------------------------------------
  /// @brief    Runs the plugin's init function.
  /// @param    a_lIndex    [in] The plugin's index.
  /// @param    a_pPluginArgs [in] Pointer to plugin's run function arguments.
  /// @return   The method returns one of the following values:
  /// @retval   true  | The plugin execution was successful.
  /// @retval   false | An error occured while executing the plugin.
  //  @remarks  This function is pure virtual and must be overwritten!
  //-----------------------------------------------------------------------------------------------------------------------
  virtual bool SPL_API RunPluginInit( const long a_lIndex, const slcPluginArgs* a_pPluginArgs = NULL );  

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Runs a plugin with the given index (0-based) and arguments.
	///	@param		a_lIndex		[in] The plugin's index.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's run function arguments.
	///	@return		The method returns one of the following values:
	///	@retval		true	| The plugin execution was successful.
	///	@retval		false	| An error occured while executing the plugin.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual bool SPL_API RunPlugin( const long a_lIndex, const slcPluginArgs* a_pPluginArgs = NULL );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Runs all plugins with the given arguments.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's run function arguments.
	///	@return		Number of successfully executed plugins.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API RunAllPlugins( const slcPluginArgs* a_pPluginArgs = NULL );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads all plugins and looks for new plugins in the given path.
	///	@param		a_strPath				[in] Valid path for searching the plugins.
	///	@param		a_strWildcard			[in] File wildcard (e.g. *.*, *.dll) for the plugins.
	///	@param		a_pPluginShutdownArgs	[in] Pointer to arguments for shutting down all plugins before they will be unloaded.
	///	@param		a_pPluginInitArgs		[in] Pointer to arguments for initializing all plugins after they have been loaded.
	///	@return		Number of successfully loaded plugins.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API Refresh( const string&		   a_strPath, 
						  const string&		   a_strWildcard, 
						  const	slcPluginArgs* a_pPluginShutdownArgs = NULL, 
						  const	slcPluginArgs* a_pPluginInitArgs = NULL );
};

} // End of namespace SPL

using namespace SPL;

#endif	//	__SPL_PLUGINSERVER_H__
