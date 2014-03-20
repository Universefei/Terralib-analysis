
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
René Stuhr (www.unitedbytes.de)

*/

//---------------------------------------------------------------------------------------------------------------------------
/// @file		spl_plugininfo.h
/// @version	1.0
/// @brief		The plugin info class.
/// @author		René Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGININFO_H__
#define		__SPL_PLUGININFO_H__

#ifndef		__SPL_TYPEDEFS_H__
#include	"spl_typedefs.h"
#endif

#ifndef		__SPL_PLATFORM_H__
#include	"spl_platform.h"
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

namespace SPL {


//---------------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin information class.
///	@ingroup	SPL
//---------------------------------------------------------------------------------------------------------------------------
class slcPluginInfo : public slcMutex
{
	//
	//-----------------------------------------------------------------------------------------------------------------------
	//**************************************************     STRUCTURES     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//---------------------------------------------------------------------------------------------------------------------------
	///	@brief		Function enumeration for argument lookups.
	///	@ingroup	SPL
	//---------------------------------------------------------------------------------------------------------------------------
	enum sleFunction
	{
		sleFunction_Init = 0,
		sleFunction_Run,
		sleFunction_Shutdown
	};

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//***********************************************     CON/DESTRUCTION     ***********************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default constructor.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcPluginInfo( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Copy constructor.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcPluginInfo( const slcPluginInfo& a_pluginInfo );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default destructor.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API ~slcPluginInfo( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//************************************************     MODIFICATION     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Clears the structure.
	//-----------------------------------------------------------------------------------------------------------------------
	void SPL_API Clear( void ); 

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the argument count of the specified function.
	///	@param		a_enuFunc	[in] Function to get the argument count from.
	///	@return		Number of arguments of the specified function.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetArgCount( sleFunction a_enuFunc );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns an argument's description.
	///	@param		a_enuFunc	[in] Function to get the argument description from.
	///	@param		a_lIndex	[in] Index (0-based) of argument.
	///	@return		The argument's description.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetArgDescription( sleFunction a_enuFunc, const long a_lIndex );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns an argument's name.
	///	@param		a_enuFunc	[in] Function to get the argument description from.
	///	@param		a_lIndex	[in] Index (0-based) of argument.
	///	@return		The argument's name.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetArgName( sleFunction a_enuFunc, const long a_lIndex );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's name.
	///	@return		The plugin's name.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetName( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's vendor.
	///	@return		The plugin's vendor.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetVendor( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's information string.
	///	@return		The plugin's information string.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetInfo( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's description.
	///	@return		The plugin's description.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetDescription( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's homepage.
	///	@return		The plugin's homepage.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetHomepage( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's e-mail address.
	///	@return		The plugin's e-mail address.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetEmail( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's major version (e.g. 1.xx).
	///	@return		The plugin's major version.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetMajorVersion( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's minor version (e.g. x.12).
	///	@return		The plugin's minor version.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetMinorVersion( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's build version.
	///	@return		The plugin's build version.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetBuildVersion( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the plugin's UUID.
	///	@return		The plugin's UUID.
	//-----------------------------------------------------------------------------------------------------------------------
	string SPL_API GetUUID( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Reports whether the plugin has public arguments or not.
	///	@return		The method returns one of the following values:
	///	@retval		true	| The plugin has public arguments.
	///	@retval		false	| The plugin has no public arguments.
	//-----------------------------------------------------------------------------------------------------------------------
	bool SPL_API HasPublicArgs( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Copy operator.
	///	@param		a_pluginInfo	[in] Plugin class to copy from.
	///	@return		The new plugin class.
	//-----------------------------------------------------------------------------------------------------------------------
	slcPluginInfo SPL_API operator=( const slcPluginInfo& a_pluginInfo );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//*************************************************     ATTRIBUTES     **************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
protected:

	string							m_strName;				// The plugin's name.
	string							m_strVendor;			// The plugin's vendor.
	string							m_strInfo;				// The plugin's information string.
	string							m_strDescription;		// The plugin's description.
	string							m_strHomepage;			// The vendor's homepage.
	string							m_strEmail;				// The vendor's e-mail address.
	long							m_lMajor;				// The plugin's major version (e.g. 1.xx)
	long							m_lMinor;				// The plugin's minor version (e.g. x.12)
	long							m_lBuild;				// The plugin's build version.
	string							m_strUUID;				// The plugin's UUID.
	bool							m_bPuplicArgInfos;		// The plugin's argument infos flag.	

	map< long, string >				m_mapInitArgs;			// Map for holding the plugin initialize arguments and their description.
	map< long, string >				m_mapShutdownArgs;		// Map for holding the plugin shutdown arguments and their description.
	map< long, string >				m_mapRunArgs;			// Map for holding the plugin run arguments and their description.
	map< long, string >::iterator	itmapArgs;				// The map's iterator.
};


} // End of namespace SPL

#endif	//	__SPL_PLUGININFO_H__
