
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
/// @file		spl.h
/// @version	1.0
/// @brief		SPL's main header.
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_H__
#define		__SPL_H__

#ifndef		__SPL_PLATFORM_H__
#include	"include/spl_platform.h"
#endif

#ifndef		__SPL_TYPEDEFS_H__
#include	"include/spl_typedefs.h"
#endif

#ifndef		__SPL_PLUGINARGS_H__
#include	"include/spl_pluginargs.h"
#endif

#ifndef		__SPL_PLUGININFO_H__
#include	"include/spl_plugininfo.h"
#endif

#ifndef		__SPL_PLUGINSERVER_H__
#include	"include/spl_pluginserver.h"
#endif

#if SPL_PLATFORM == SPL_PLATFORM_WIN32

	#ifndef		__SPL_PLUGINSERVER_WIN32_H__
	#include	"include/spl_pluginserver_win32.h"
	#endif

#elif SPL_PLATFORM == SPL_PLATFORM_LINUX

	#ifndef		__SPL_PLUGINSERVER_LINUX_H__
	#include	"include/spl_pluginserver_linux.h"
	#endif

#elif SPL_PLATFORM == SPL_PLATFORM_SOLARIS

	#ifndef		__SPL_PLUGINSERVER_SOLARIS_H__
	#include	"include/spl_pluginserver_solaris.h"
	#endif

#endif

#include		<string>

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for exporting SPL functions.
//-----------------------------------------------------------------------------------------------------------------------
#if SPL_PLATFORM == SPL_PLATFORM_WIN32

	#define SPL_PLUGIN_API		__declspec( dllexport )

#elif SPL_PLATFORM == SPL_PLATFORM_LINUX

	#define SPL_PLUGIN_API		extern "C"

#elif SPL_PLATFORM == SPL_PLATFORM_SOLARIS

	#define SPL_PLUGIN_API		extern "C"

#else

	#define SPL_PLUGIN_API

#endif

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for exporting SPL's default functions.
//-----------------------------------------------------------------------------------------------------------------------
#if SPL_PLATFORM == SPL_PLATFORM_WIN32
	
	#define SPL_DEFINE_PLUGIN_EXPORTS()													\
																						\
		extern "C" SPL_PLUGIN_API bool splInitialize( slcPluginArgs* a_pPluginArgs );	\
		extern "C" SPL_PLUGIN_API bool splRun( slcPluginArgs* a_pPluginArgs );			\
		extern "C" SPL_PLUGIN_API bool splShutdown( slcPluginArgs* a_pPluginArgs );		\
		extern "C" SPL_PLUGIN_API slcPluginInfo* splGetInfo( void );

	#define SPL_DEFINE_PLUGIN_DLLMAIN()							\
																\
		BOOL APIENTRY DllMain( HANDLE a_hModule,				\
							   DWORD  a_dwReasonForCall,		\
							   LPVOID a_lpReserved )			\
		{														\
			switch( a_dwReasonForCall )							\
			{													\
				case DLL_PROCESS_ATTACH:						\
				case DLL_THREAD_ATTACH:							\
				case DLL_THREAD_DETACH:							\
				case DLL_PROCESS_DETACH:						\
					break;										\
			}													\
																\
			return TRUE;										\
		}

#else 

	#define SPL_DEFINE_PLUGIN_DLLMAIN()
	#define SPL_DEFINE_PLUGIN_EXPORTS()

#endif

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for exporting SPL's default functions.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_DEFINE_PLUGIN_INFO( a_lBuildNumber,					\
								a_lMajorVersion,				\
								a_lMinorVersion,				\
								a_bPublicArgInfos,				\
								a_strName,						\
								a_strVendor,					\
								a_strDescription,				\
								a_strInfo,						\
								a_strHomepage,					\
								a_strEmail,						\
								a_strUUID )						\
																\
	class _pluginInfo : public slcPluginInfo					\
	{															\
		public:													\
																\
			_pluginInfo( void )					\
			{													\
				this->m_lBuild = a_lBuildNumber;				\
				this->m_lMajor = a_lMajorVersion;				\
				this->m_lMinor = a_lMinorVersion;				\
				this->m_bPuplicArgInfos = a_bPublicArgInfos;	\
				this->m_strName = a_strName;					\
				this->m_strVendor = a_strVendor;				\
				this->m_strDescription = a_strDescription;		\
				this->m_strInfo = a_strInfo;					\
				this->m_strHomepage = a_strHomepage;			\
				this->m_strEmail = a_strEmail;					\
				this->m_strUUID = a_strUUID;					\
																\
				this->m_mapInitArgs.clear();					\
				this->m_mapRunArgs.clear();						\
				this->m_mapShutdownArgs.clear();				\
			};													\
																\
			bool SetArgument( int         a_iFunc,				\
							  string	  a_strArgument,		\
							  string	  a_strDesc )			\
			{														\
				string strFinal = a_strArgument + ";" + a_strDesc;	\
																	\
				switch( a_iFunc )								\
				{												\
					case 0:										\
																\
						this->m_mapInitArgs.insert( pair< long, string >( ( long )this->m_mapInitArgs.size(), strFinal ) );					\
						break;																												\
																																			\
					case 2:																								\
																																			\
						this->m_mapShutdownArgs.insert( pair< long, string >( ( long )this->m_mapShutdownArgs.size(), strFinal ) );			\
						break;																												\
																																			\
					case 1:																									\
					default:																												\
																																			\
						this->m_mapRunArgs.insert( pair< long, string >( ( long )this->m_mapRunArgs.size(), strFinal ) );					\
						break;																												\
				}												\
																\
				return true;									\
			};													\
	};															\
																\
	_pluginInfo g_pluginInfo;

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for setting the plugin-side arguments for the "splInitialize" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_SET_PLUGIN_INIT_ARGUMENT( a_strArgumentName, a_strArgumentDescription )		\
																						\
	g_pluginInfo.SetArgument( slcPluginInfo::sleFunction_Init, a_strArgumentName, a_strArgumentDescription );										

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for setting the plugin-side arguments for the "splRun" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_SET_PLUGIN_RUN_ARGUMENT( a_strArgumentName, a_strArgumentDescription )		\
																						\
	g_pluginInfo.SetArgument( slcPluginInfo::sleFunction_Run, a_strArgumentName, a_strArgumentDescription );

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for setting the plugin-side arguments for the "splShutdown" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_SET_PLUGIN_SHUTDOWN_ARGUMENT( a_strArgumentName, a_strArgumentDescription )		\
																							\
	g_pluginInfo.SetArgument( slcPluginInfo::sleFunction_Shutdown, a_strArgumentName, a_strArgumentDescription );

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for implementing a default "splGetInfo" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_IMPLEMENT_PLUGIN_GETINFO()								\
																	\
	SPL_PLUGIN_API slcPluginInfo* SPL_GETINFO_NAME_CODE( void )		\
	{																\
		return &g_pluginInfo;										\
	}

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for implementing a default "splInitialize" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_IMPLEMENT_PLUGIN_INITIALIZE()									\
																			\
	SPL_PLUGIN_API bool SPL_INIT_NAME_CODE( slcPluginArgs* a_pPluginArgs )	\
	{																		\
		return true;														\
	}

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		Macro for implementing a default "splShutdown" function.
//-----------------------------------------------------------------------------------------------------------------------
#define SPL_IMPLEMENT_PLUGIN_SHUTDOWN()											\
																				\
	SPL_PLUGIN_API bool SPL_SHUTDOWN_NAME_CODE( slcPluginArgs* a_pPluginArgs )	\
	{																			\
		return true;															\
	}

//-----------------------------------------------------------------------------------------------------------------------
///	@brief		We're using SPL's namespace from now on ... 
//-----------------------------------------------------------------------------------------------------------------------
//using namespace SPL;

#endif	//	__SPL_H__

