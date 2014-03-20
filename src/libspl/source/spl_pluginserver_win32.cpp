
/*!

\section terms_of_use Terms of Use

libspl - the simple plugin layer library.
Copyright (C) 2004 Andreas Loeffler and Ren� Stuhr (www.unitedbytes.de).

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
/// @file		spl_pluginserver_win32.cpp
/// @version	1.0
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLATFORM_H__
#include	"../include/spl_platform.h"
#endif


#if			SPL_PLATFORM == SPL_PLATFORM_WIN32

#define		_WIN32_WINNT 0x0400

#ifndef		_WINDOWS_
#include	<windows.h>
#endif

#ifndef		__SPL_PLUGINSERVER_WIN32_H__
#include	"../include/spl_pluginserver_win32.h"
#endif

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif

//
// Disable warning messages
//
#pragma warning( disable : 4311 4312 4267 )  

slcPluginServer::slcPluginServer( void )
{

}

slcPluginServer::~slcPluginServer( void )
{
	if( this->GetLoadedPlugins() > 0 )
		this->UnloadAllPlugins();
}

slcPluginInfo slcPluginServer::GetPluginInfo( const long a_lIndex )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	if( ( a_lIndex < 0 ) ||
		( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
	{
		slcPluginInfo invalidInfo;
		return invalidInfo;
	}

	this->itIds  = this->m_mapPluginIds.find( a_lIndex ); 
	if( this->itIds != this->m_mapPluginIds.end() )
	{
		SPL_INFO_FUNC pInfoFunc = ( SPL_INFO_FUNC )::GetProcAddress( ( HMODULE )( SPL_BITTYPE )this->itIds->second, 	SPL_GETINFO_NAME );
		if( NULL != pInfoFunc )
		{
			slcPluginInfo plugInfo = *pInfoFunc();
			this->m_mapPluginInfos.insert( pair<long, slcPluginInfo>( this->m_mapPluginInfos.size() , plugInfo ) );
		}
		else
		{
			//
			// INFO FUNCTION NOT LOADED, INSERT DUMMY ENTRY TO AVOID ASYNCRON MAPS 
			//
			slcPluginInfo nullInfo;
			this->m_mapPluginInfos.insert( pair<long, slcPluginInfo>( this->m_mapPluginInfos.size(), nullInfo ) );
		}
	}

	return this->m_mapPluginInfos[ a_lIndex ];
}

void slcPluginServer::GetAllPluginInfos( void )
{
	long l = 0;

	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	for( this->itIds  = this->m_mapPluginIds.begin(); 
		 this->itIds != this->m_mapPluginIds.end(); this->itIds++ )
	{
		SPL_INFO_FUNC pInfoFunc = ( SPL_INFO_FUNC )::GetProcAddress( ( HMODULE )( SPL_BITTYPE )this->itIds->second, 
																	 SPL_GETINFO_NAME );
		if( NULL != pInfoFunc )
		{
			slcPluginInfo plugInfo = *pInfoFunc();
			this->m_mapPluginInfos.insert( pair<long, slcPluginInfo>( l++, plugInfo ) );
		}
		else
		{
			//
			// INFO FUNCTION NOT LOADED, INSERT DUMMY ENTRY TO AVOID ASYNCRON MAPS 
			//
			slcPluginInfo nullInfo;
			this->m_mapPluginInfos.insert( pair<long, slcPluginInfo>( l++, nullInfo ) );
		}
	}
}

long slcPluginServer::LoadAllPlugins( const string&		   a_strPath, 
									  const string&		   a_strWildcard, bool init_plugin,
									  const	slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	WIN32_FIND_DATA FindData;
	string strWildcard;

	if( a_strWildcard.empty() )
		strWildcard = "*.dll";
	else
		strWildcard = a_strWildcard;

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	::SetCurrentDirectory( a_strPath.c_str() );
	HANDLE hDirectory = ::FindFirstFileEx( strWildcard.c_str(), FindExInfoStandard, 
		&FindData, FindExSearchNameMatch, NULL, 0 );

	if( hDirectory != INVALID_HANDLE_VALUE )
	{
		do {
			if( ( strcmp( FindData.cFileName, "." ) != 0 ) &&
				( strcmp( FindData.cFileName, ".." ) != 0 ) )
			{
				HMODULE hModule = ::LoadLibrary( FindData.cFileName );
				if ( NULL != hModule )
				{ 
					  SPL_INIT_FUNC pInitFunc =( SPL_INIT_FUNC )::GetProcAddress( ( HINSTANCE )hModule, 
																				SPL_INIT_NAME );
					  if( NULL != pInitFunc )
					  {
              if( init_plugin ) {
						   pInitFunc( pPluginArgs );
              }
                                                  this->m_mapPluginIds.insert( pair< long, SPL_BITTYPE >( ( long )this->m_mapPluginIds.size(), ( SPL_BITTYPE )hModule ) );
					  }
				}
			}	
		} 
		while( ::FindNextFile( hDirectory, &FindData ) );

		::FindClose( hDirectory );
	}

	return ( long )this->m_mapPluginIds.size();
}

long slcPluginServer::LoadPlugin( const string& a_strFileName, bool init_plugin, const slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	if( a_strFileName.empty() )
		return -2;

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	HMODULE hModule = ::LoadLibrary( a_strFileName.c_str() );

	if ( NULL != hModule )
	{ 
		SPL_INIT_FUNC pInitFunc =( SPL_INIT_FUNC )::GetProcAddress( ( HINSTANCE )hModule, 
																	SPL_INIT_NAME );

                this->m_mapPluginIds.insert( pair< long, SPL_BITTYPE >( ( long )this->m_mapPluginIds.size(), ( SPL_BITTYPE )hModule ) );
		if( NULL != pInitFunc )
		{
      if( init_plugin ) {
			 return pInitFunc( pPluginArgs ) ? 1 : 0;
      } else {
        return 1;
      }
		}

		return -1;
	}

	return -2;
}

bool slcPluginServer::UnloadPlugin( const long a_lIndex, bool shutdown_plugin, const slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	if( ( a_lIndex < 0 ) ||
		( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
	{
		return false;
	}

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	this->itIds = this->m_mapPluginIds.find( a_lIndex );

	if( this->itIds != this->m_mapPluginIds.end() )
	{
		SPL_SHUTDOWN_FUNC pShutdownFunc =( SPL_SHUTDOWN_FUNC )::GetProcAddress( ( HINSTANCE )this->itIds->second, 
																				SPL_SHUTDOWN_NAME );
		if( NULL != pShutdownFunc )
		{
      if( shutdown_plugin ) {
			  pShutdownFunc( pPluginArgs );
      }
		}

		//
		// UNLOAD LIBRARY
		//
		::FreeLibrary( ( HMODULE )this->itIds->second ); 

		//
		// REMOVE LIBRARY FROM ID-MAP AND INFO-MAP
		//
		this->m_mapPluginInfos.erase( this->itIds->first );
		this->m_mapPluginIds.erase( this->itIds->first );

		//
		// REFRESH ALL MAP IDS (CREATE NEW KEYS FOR ALL VALUES)
		//
		int i = 0;

		long lSize = this->m_mapPluginIds.size();
		map< long, SPL_BITTYPE > tempIds;
		map< long, SPL_BITTYPE >::iterator itTempIds;
		
		tempIds = this->m_mapPluginIds;
		this->m_mapPluginIds.clear();
		itTempIds = tempIds.begin();
		
		for( i = 0 ; i < lSize; i++, itTempIds++ )
			this->m_mapPluginIds.insert( pair<long, SPL_BITTYPE> (i, itTempIds->second ) );

		lSize = this->m_mapPluginInfos.size();
		map< long, slcPluginInfo > tempInfos;
		map< long, slcPluginInfo >::iterator itTempInfos;
		
		tempInfos = this->m_mapPluginInfos; 
		this->m_mapPluginInfos.clear();
		itTempInfos = tempInfos.begin();
		
		for( i = 0 ; i < lSize; i++, itTempInfos++ )
			this->m_mapPluginInfos.insert( pair<long, slcPluginInfo> (i, itTempInfos->second ) );

		return true;
	}

	return false;
}

void slcPluginServer::UnloadAllPlugins( const slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	long lCount =  ( long )this->m_mapPluginIds.size();
	for( long l = 0; l < lCount;  l++ )
	{
		this->UnloadPlugin( 0, true, a_pPluginArgs );
	}

	this->Clear();
}


bool slcPluginServer::RunPluginInit( long a_lIndex, const slcPluginArgs* a_pPluginArgs )
{
  //
  // LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
  // IMPLICIT USAGE.
  //
  slcThreadLock threadLock( this, true );

  const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

  if( NULL == pPluginArgs )
    pPluginArgs = &this->m_nullArgs;

  if( ( a_lIndex < 0 ) ||
    ( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
  {
    return false;
  }

  this->itIds = this->m_mapPluginIds.find( a_lIndex );
  if( this->itIds != this->m_mapPluginIds.end() )
  {
    SPL_INIT_FUNC pInitFunc =( SPL_INIT_FUNC )::GetProcAddress( ( HINSTANCE )this->itIds->second, 
                                  SPL_INIT_NAME );

    if( NULL != pInitFunc )
    {
      return pInitFunc( pPluginArgs ) ? 1 : 0;
    } else {
      return false;
    }
  }

  return false;
}


bool slcPluginServer::RunPlugin( long a_lIndex, const slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	if( ( a_lIndex < 0 ) ||
		( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
	{
		return false;
	}

	this->itIds = this->m_mapPluginIds.find( a_lIndex );
	if( this->itIds != this->m_mapPluginIds.end() )
	{
		SPL_RUN_FUNC pRunFunc =( SPL_RUN_FUNC )::GetProcAddress( ( HINSTANCE )this->itIds->second, 
																 SPL_RUN_NAME );
		if( NULL != pRunFunc )
		{
			return pRunFunc( pPluginArgs );
		}
	}

	return false;
}

long slcPluginServer::RunAllPlugins( const slcPluginArgs* a_pPluginArgs )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	long lPluginsRunned = 0;
	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	if( this->m_mapPluginIds.empty() )
		return 0;

	for( long l = 0; l < ( long )this->m_mapPluginIds.size(); l++ )
	{
		if( this->RunPlugin( l, pPluginArgs ) )
			lPluginsRunned++;
	}

	return lPluginsRunned;
}

long slcPluginServer::Refresh( const string&	    a_strPath, 
							   const string&	    a_strWildcard, 
							   const slcPluginArgs* a_pPluginShutdownArgs, 
							   const slcPluginArgs*	a_pPluginInitArgs )
{
	this->UnloadAllPlugins( a_pPluginShutdownArgs );

	//
	// Lock this object for exclusive use.
	// Explicitly usage.
	//
	slcThreadLock threadLock( this );

	threadLock.Lock();
	this->Clear();
	threadLock.Unlock();

	return( this->LoadAllPlugins( a_strPath, a_strWildcard, true, a_pPluginInitArgs ) > 0 ); 
}

//
// Enable warning messages
//
#pragma warning( default : 4311 4312 4267 )  


#endif	//	SPL_PLATFORM_WIN32
