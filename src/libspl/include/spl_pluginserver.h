
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
/// @file		spl_pluginserver.h
/// @version	1.0
/// @brief		The plugin server class.
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGINSERVER_H__
#define		__SPL_PLUGINSERVER_H__

#ifndef		__SPL_TYPEDEFS_H__
#include	"spl_typedefs.h"
#endif

#ifndef		__SPL_PLUGININFO_H__
#include	"spl_plugininfo.h"
#endif

#ifndef		__SPL_PLUGINARGS_H__
#include	"spl_pluginargs.h"
#endif

#ifndef		__SPL_MUTEX_H__
#include	"spl_mutex.h"
#endif

#ifndef		_MAP_
#include	<map>
#endif

#ifndef		_STRING_
#include	<string>
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

using namespace std;

namespace SPL 
{

//---------------------------------------------------------------------------------------------------------------------------
///	@brief		Abstract plugin server class.
///	@ingroup	SPL
//---------------------------------------------------------------------------------------------------------------------------
class slaPluginServer : public slcMutex
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
	SPL_API slaPluginServer( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default destructor.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API ~slaPluginServer( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//************************************************     MODIFICATION     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//

protected:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Cleanup all internal maps.
	//  @remarks	This function is only for internal use!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API Clear( void );

public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the current number of loaded plugins.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API GetLoadedPlugins( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns information about a plugin from internal plugin info map.
	/// @remarks	Index is 0-based.
	///	@param		a_lIndex		[in] The plugin's index.
	///	@retval		A slcPluginInfo structure containing the plugin's information.
	/// @sa			slcPluginInfo
	//-----------------------------------------------------------------------------------------------------------------------
	virtual slcPluginInfo SPL_API GetSafedPluginInfo( const long a_lIndex );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns information about a plugin, use this to safe the info to info map.
	/// @remarks	Index is 0-based.
	///	@param		a_lIndex		[in] The plugin's index.
	///	@retval		A slcPluginInfo structure containing the plugin's information.
	/// @sa			slcPluginInfo
	//-----------------------------------------------------------------------------------------------------------------------
	virtual slcPluginInfo SPL_API GetPluginInfo( const long a_lIndex ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Automatically retrieves information about all loaded plugins.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API GetAllPluginInfos( void ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Loads all plugins with a given wildcard in a specified path.
	///	@param		a_strPath		[in] Valid path for searching the plugins.
	///	@param		a_strWildcard	[in] File wildcard (e.g. *.*, *.dll) for the plugins.
  /// @param    init_plugin After plugin loading, run the init function.
	///	@param		a_pPluginArgs	[in] Pointer to arguments for initializing all plugins after they have been loaded.
	///	@return		Number of successfully loaded plugins.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API LoadAllPlugins( const string&		  a_strPath, 
								 const string&		  a_strWildcard, bool init_plugin,
								 const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

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
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API LoadPlugin( const string& a_strFileName, bool init_plugin, const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads a plugin with the given index (0-based).
	///	@param		a_lIndex		[in] The plugin's index.
  /// @param    shutdown_plugin Run the plugin's shutdown function before unloading it.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's shutdown function arguments.
	///	@return		The method returns one of the following values:
	///	@retval		true	| The plugin was unloaded successfully.
	///	@retval		false	| Could not unload plugin.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual bool SPL_API UnloadPlugin( const long a_lIndex, bool shutdown_plugin, const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads all plugins.
	///	@param		a_pPluginArgs	[in] Pointer to arguments for shutting down all plugins after they have been unloaded.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API UnloadAllPlugins( const slcPluginArgs* a_pPluginArgs = NULL ) = 0;
  
  //-----------------------------------------------------------------------------------------------------------------------
  /// @brief    Runs the plugin's init function.
  /// @param    a_lIndex    [in] The plugin's index.
  /// @param    a_pPluginArgs [in] Pointer to plugin's run function arguments.
  /// @return   The method returns one of the following values:
  /// @retval   true  | The plugin execution was successful.
  /// @retval   false | An error occured while executing the plugin.
  //  @remarks  This function is pure virtual and must be overwritten!
  //-----------------------------------------------------------------------------------------------------------------------
  virtual bool SPL_API RunPluginInit( const long a_lIndex, const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Runs a plugin with the given index (0-based) and arguments.
	///	@param		a_lIndex		[in] The plugin's index.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's run function arguments.
	///	@return		The method returns one of the following values:
	///	@retval		true	| The plugin execution was successful.
	///	@retval		false	| An error occured while executing the plugin.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual bool SPL_API RunPlugin( const long a_lIndex, const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Runs all plugins with the given arguments.
	///	@param		a_pPluginArgs	[in] Pointer to plugin's run function arguments.
	///	@return		Number of successfully executed plugins.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API RunAllPlugins( const slcPluginArgs* a_pPluginArgs = NULL ) = 0;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unloads all plugins and looks for new plugins in the given path.
	///	@param		a_strPath				[in] Valid path for searching the plugins.
	///	@param		a_strWildcard			[in] File wildcard (e.g. *.*, *.dll) for the plugins.
	///	@param		a_pPluginShutdownArgs	[in] Pointer to arguments for shutting down all plugins before they will be unloaded.
	///	@param		a_pPluginInitArgs		[in] Pointer to arguments for initializing all plugins after they have been loaded.
	///	@return		Number of successfully loaded plugins.
	//  @remarks	This function is pure virtual and must be overwritten!
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API Refresh( const string&		   a_strPath, 
						  const string&		   a_strWildcard, 
						  const	slcPluginArgs* a_pPluginShutdownArgs = NULL, 
						  const slcPluginArgs* a_pPluginInitArgs = NULL ) = 0;


	//
	//-----------------------------------------------------------------------------------------------------------------------
	//*************************************************     ATTRIBUTES     **************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
protected:

	map< long, SPL_BITTYPE >				m_mapPluginIds;		///< Map containing the plugin IDs.
	map< long, SPL_BITTYPE > ::iterator		itIds;				///< Iterator for plugin IDs.

	map< long, slcPluginInfo >				m_mapPluginInfos;	///< Map containing the plugin infos.
	map< long, slcPluginInfo > ::iterator	itInfos;			///< Iterator for plugin infos.

	slcPluginArgs							m_nullArgs;			///< Dummy argument for internal use.
	slcPluginInfo							m_nullInfo;			///< Dummy argument for internal use.
};

#define		SPL_INIT_NAME			"splInitialize"					///< The initialisation function's name.
#define		SPL_INIT_NAME_CODE		splInitialize					///< The initialisation function's name for internal macros.
#define		SPL_RUN_NAME			"splRun"						///< The run function's name.
#define		SPL_RUN_NAME_CODE		splRun							///< The run function's name for internal macros.
#define		SPL_SHUTDOWN_NAME		"splShutdown"					///< The shutdown function's name.
#define		SPL_SHUTDOWN_NAME_CODE	splShutdown						///< The shutdown function's name for internal macros.
#define		SPL_GETINFO_NAME		"splGetInfo"					///< The information function's name.
#define		SPL_GETINFO_NAME_CODE	splGetInfo						///< The information function's name for internal macros.

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin's initialisation function.
//-----------------------------------------------------------------------------------------------------------------------
typedef bool( *SPL_INIT_FUNC )( const slcPluginArgs* );

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin's run function.
//-----------------------------------------------------------------------------------------------------------------------
typedef bool( *SPL_RUN_FUNC )( const slcPluginArgs* );

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin's shutdown function.
//-----------------------------------------------------------------------------------------------------------------------
typedef bool( *SPL_SHUTDOWN_FUNC )( const slcPluginArgs* );

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin's information function.
//-----------------------------------------------------------------------------------------------------------------------
typedef slcPluginInfo*( *SPL_INFO_FUNC )( void );

} // End of namespace SPL

using namespace SPL;

#endif	//	__SPL_PLUGINSERVER_H__
