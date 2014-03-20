
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
/// @file		spl_pluginserver_solaris.cpp
/// @version	1.0
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLATFORM_H__
#include	"../include/spl_platform.h"
#endif

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif


#if			SPL_PLATFORM == SPL_PLATFORM_SOLARIS

#include	<dlfcn.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>

#include	"../include/spl_pluginserver_solaris.h"


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
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
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
		SPL_INFO_FUNC pInfoFunc = ( SPL_INFO_FUNC )dlsym( ( void* )( SPL_BITTYPE )itIds->second, 
														  SPL_GETINFO_NAME );
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
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
	//
	slcThreadLock threadLock( this, true );

	for( this->itIds  = this->m_mapPluginIds.begin(); 
		 this->itIds != this->m_mapPluginIds.end(); this->itIds++ )
	{
		SPL_INFO_FUNC pInfoFunc = ( SPL_INFO_FUNC )dlsym( ( void* )( SPL_BITTYPE )itIds->second, 
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
									  const string&		   a_strWildcard,  bool init_plugin,
									  const	slcPluginArgs* a_pPluginArgs )
{
	//
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
	//
	slcThreadLock threadLock( this, true );

	string strWildcard;

	if( a_strWildcard.empty() )
		strWildcard = "*.so";
	else
		strWildcard = a_strWildcard;

	DIR* dirHandle;
	struct dirent* dirEntry;
	struct stat statBuffer;
	string strFileName;
	string strExtension;

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	//
	// OPEN PATH
	//
	if( false == ( dirHandle = opendir( a_strPath.c_str() ) ) )
		return 0;

	//
	// REMOVE * FROM EXTENSION
	//
	strExtension = a_strWildcard;

	unsigned int iPos = 0;
	if( ( iPos = strExtension.find( '*', 0 ) ) != string::npos )
		strExtension.erase( iPos, 1 );

	//
	// STEP TROUGH DIRECTORY AND FILTER ALL UNWANTED ENTRIES AND DIRECTORIES
	//
	while( 0 != ( dirEntry = readdir( dirHandle ) ) )
	{
		strFileName = a_strPath;

		//
		// CHECK IF GIVEN PATH HAS '/' AT END OF STRING
		// IF NOT APPEND IT
		//
		if( strFileName.find( '/', strFileName.length() - 1 ) == string::npos )
			strFileName += '/';

		//
		// ADD FOUNDED FILENAME TO FILEPATH
		//
		strFileName += dirEntry->d_name;

		//
		// GET STATUS FROM FOUND FILE TO FILTER ALL DIRECTORY ENTRIES
		//
		stat( strFileName.c_str(), &statBuffer );

		//
		// IS CURRENT ENTRY A DIRECTORY? SKIP IT!
		//
		if( S_ISDIR( statBuffer.st_mode ) )
			continue;

		//
		// CHECK FOR VALID EXTENSION, SKIP WRONG EXTENSIONS
		//
		if( strFileName.find( strExtension.c_str(),
			strFileName.length() - strExtension.length() -1 ) == string::npos )
			continue;

		//
		// GET PLUGIN HANDLE
		//
		void* pvHandle = NULL;
		if( NULL == ( pvHandle = dlopen( strFileName.c_str(), RTLD_NOW ) ) )
		{
			continue;
		}
		else
		{
			//
			// ADD HANDLE TO MAP
			//
			this->m_mapPluginIds.insert( pair<long, SPL_BITTYPE >( ( long )this->m_mapPluginIds.size(), ( SPL_BITTYPE )pvHandle ) );

			//
			// EXECUTE INITIALISATION FUNCTION
			//
			SPL_INIT_FUNC pInitFunc = ( SPL_INIT_FUNC )dlsym( pvHandle , SPL_INIT_NAME );
			if( pInitFunc != NULL )
			{
        if( init_plugin ) {
				  pInitFunc( pPluginArgs );
        }
			}
		}
	}

	closedir( dirHandle );
	return( long )m_mapPluginIds.size();
}

long slcPluginServer::LoadPlugin( const string& a_strFileName, bool init_plugin, const slcPluginArgs* a_pPluginArgs )
{
	if( a_strFileName.empty() )
		return -2;

	//
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
	//
	slcThreadLock threadLock( this, true );

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	void* pvHandle = NULL;
	if( NULL == ( pvHandle = dlopen( a_strFileName.c_str(), RTLD_NOW ) ) )
		return -2;

	this->m_mapPluginIds.insert( pair<long, SPL_BITTYPE >( ( long )this->m_mapPluginIds.size(), ( SPL_BITTYPE )pvHandle ) );

	SPL_INIT_FUNC pInitFunc = ( SPL_INIT_FUNC )dlsym( pvHandle , SPL_INIT_NAME );
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

bool slcPluginServer::UnloadPlugin( const long a_lIndex, bool shutdown_plugin, const slcPluginArgs* a_pPluginArgs )
{
	//
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
	//
	slcThreadLock threadLock( this, true );

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
		pPluginArgs = &this->m_nullArgs;

	this->itIds = this->m_mapPluginIds.find( a_lIndex );

	if( this->itIds != this->m_mapPluginIds.end() )
	{
		SPL_SHUTDOWN_FUNC pShutdownFunc = ( SPL_SHUTDOWN_FUNC )dlsym( ( void* )( SPL_BITTYPE )this->itIds->second,
																	  SPL_SHUTDOWN_NAME );
		if( NULL != pShutdownFunc )
		{
      if( shutdown_plugin ) {
			  pShutdownFunc( pPluginArgs );
      }
		}

		//
		// DECREASE KERNEL HANDLE COUNTER (0 = LIBRARY IS COMPLETELY UNLOADED)
		//
		if( dlclose( ( void* )( SPL_BITTYPE )this->itIds->second ) != 0 )
			return false;

		//
		// REMOVE LIBRARY FROM ID-MAP AND INFO-MAP
		//
		this->m_mapPluginIds.erase( this->itIds->first );
		this->m_mapPluginInfos.erase( this->itIds->first );

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
	long lCount =  ( long )this->m_mapPluginIds.size();
	for( long l = 0; l < lCount;  l++ )
	{
		this->UnloadPlugin( 0, true, a_pPluginArgs );
	}

	this->Clear();
}


bool slcPluginServer::RunPluginInit(const long a_lIndex, const slcPluginArgs* a_pPluginArgs )
{
  if( ( a_lIndex < 0 ) ||
    ( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
  {
    return false;
  }

  const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

  if( NULL == pPluginArgs )
  {
    pPluginArgs = &this->m_nullArgs;
  } 

  //
  // Lock this object for exclusive use, unlocks automatical at end of function.
  // Implicitly usage.
  //
  slcThreadLock threadLock( this, true );

  this->itIds = this->m_mapPluginIds.find( a_lIndex );
  if( this->itIds != this->m_mapPluginIds.end() )
  {
    SPL_INIT_FUNC pInitFunc = ( SPL_INIT_FUNC )dlsym( ( void* )( SPL_BITTYPE )this->itIds->second , SPL_INIT_NAME );
    if( NULL != pInitFunc )
    {
      return pInitFunc( pPluginArgs ) ? 1 : 0;
    } else {
      return false;
    }
  }
  
  return false;
}


// Karg - linking error, the first parameter was long not const long :D
bool slcPluginServer::RunPlugin(const long a_lIndex, const slcPluginArgs* a_pPluginArgs )
{
	if( ( a_lIndex < 0 ) ||
		( a_lIndex > ( long )this->m_mapPluginInfos.size() ) )
	{
		return false;
	}

	const slcPluginArgs* pPluginArgs = a_pPluginArgs; 

	if( NULL == pPluginArgs )
	{
		pPluginArgs = &this->m_nullArgs;
	}	

	//
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Implicitly usage.
	//
	slcThreadLock threadLock( this, true );

	this->itIds = this->m_mapPluginIds.find( a_lIndex );
	if( this->itIds != this->m_mapPluginIds.end() )
	{
		SPL_RUN_FUNC pRunFunc = ( SPL_RUN_FUNC )dlsym( ( void* )( SPL_BITTYPE )this->itIds->second,
													   SPL_RUN_NAME );
		
		// Karg - not finding a symbol in a plugin is a fairly serious error - maybe log it with dlerror()
		if( NULL != pRunFunc )
		{
			return pRunFunc( pPluginArgs );
		}
	}
	
	return false;
}

long slcPluginServer::RunAllPlugins( const slcPluginArgs* a_pPluginArgs )
{
	long lPluginsRunned = 0;
	
	if( this->m_mapPluginIds.empty() )
	{
		return 0;
	}	

	for( long l = 0; l < ( long )this->m_mapPluginIds.size(); l++ )
	{
		if( this->RunPlugin( l, a_pPluginArgs ) )
			lPluginsRunned++;
	}

	return lPluginsRunned;	
}

long slcPluginServer::Refresh( const string&	    a_strPath, 
							   const string&	    a_strWildcard, 
							   const slcPluginArgs* a_pPluginShutdownArgs, 
							   const slcPluginArgs* a_pPluginInitArgs )
{
	this->UnloadAllPlugins( a_pPluginShutdownArgs );

	//
	// Lock this object for exclusive use, unlocks automatical at end of function.
	// Explicitly usage.
	//
	slcThreadLock threadLock( this );

	threadLock.Lock();
	this->Clear();
	threadLock.Unlock();

	return( this->LoadAllPlugins( a_strPath, a_strWildcard, a_pPluginInitArgs ) ); 
}

#endif	//	SPL_PLATFORM_SOLARIS
