
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
Ren� Stuhr (www.unitedbytes.de)

*/

//---------------------------------------------------------------------------------------------------------------------------
/// @file		spl_plugininfo.cpp
/// @version	1.0
/// @author		Ren� Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGININFO_H__
#include	"../include/spl_plugininfo.h"
#endif

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif

using namespace SPL;

slcPluginInfo::slcPluginInfo( void ) : m_lMajor( -1 ),
									   m_lMinor( -1 ),
									   m_lBuild( -1 ),
									   m_bPuplicArgInfos( false )
{

}

slcPluginInfo::slcPluginInfo( const slcPluginInfo& a_pluginInfo )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	this->m_lMajor = a_pluginInfo.m_lMajor;
	this->m_lMinor = a_pluginInfo.m_lMinor;
	this->m_lBuild = a_pluginInfo.m_lBuild;

	this->m_bPuplicArgInfos = a_pluginInfo.m_bPuplicArgInfos;

	this->m_strDescription	= a_pluginInfo.m_strDescription;
	this->m_strEmail		= a_pluginInfo.m_strEmail;
	this->m_strHomepage		= a_pluginInfo.m_strHomepage;
	this->m_strInfo			= a_pluginInfo.m_strInfo;
	this->m_strName			= a_pluginInfo.m_strName;
	this->m_strUUID			= a_pluginInfo.m_strUUID;
	this->m_strVendor		= a_pluginInfo.m_strVendor;

    this->m_mapInitArgs		= a_pluginInfo.m_mapInitArgs;
	this->m_mapRunArgs		= a_pluginInfo.m_mapRunArgs;
	this->m_mapShutdownArgs = a_pluginInfo.m_mapShutdownArgs;
	this->itmapArgs			= a_pluginInfo.itmapArgs;
}

slcPluginInfo::~slcPluginInfo( void )
{

}

void slcPluginInfo::Clear( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	this->m_lMajor = -1;
	this->m_lMinor = -1;
	this->m_lBuild = -1;
	this->m_bPuplicArgInfos = false;

	//
	// CLEANUP ALL MAP ENTRYS
	//
	this->m_mapInitArgs.clear();
	this->m_mapShutdownArgs.clear();
	this->m_mapRunArgs.clear();
}

//
// ARGUMENT INFO FUNCTIONS
//

long slcPluginInfo::GetArgCount( sleFunction a_enuFunc ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	long lCount = 0;
	
	switch( a_enuFunc )
	{
		case sleFunction_Init:
			lCount = ( long )this->m_mapInitArgs.size( );
			break;

		case sleFunction_Run:
			lCount = ( long )this->m_mapRunArgs.size( );
			break;

		case sleFunction_Shutdown:
			lCount = ( long )this->m_mapShutdownArgs.size( );
			break;
	}

	return lCount;
}

string slcPluginInfo::GetArgDescription( sleFunction a_enuFunc, const long a_lIndex )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	switch( a_enuFunc )
	{
		case sleFunction_Init:

			this->itmapArgs = this->m_mapInitArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapInitArgs.end() )
				return string( "" );

			break;

		case sleFunction_Shutdown:

			this->itmapArgs = this->m_mapShutdownArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapShutdownArgs.end() )
				return string( "" );

			break;
		
		case sleFunction_Run:
		default:

			this->itmapArgs = this->m_mapRunArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapRunArgs.end() )
				return string( "" );

			break;
	}

	//
	// SERCH DESCRIPTION SEPARATOR
	//
	size_t lPos = this->itmapArgs->second.find_first_of( ';' );

	//
	// NOT FOUND, RETURN A EMPTY STRING
	//
	if ( lPos == string::npos )
		return string( "" );
		
	//
	// FOUND, RETURN SECOND SUBSTRING
	//
	return this->itmapArgs->second.substr( lPos + 1, this->itmapArgs->second.size() - lPos );
}

string slcPluginInfo::GetArgName( sleFunction a_enuFunc, const long a_lIndex )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	switch( a_enuFunc )
	{
		case sleFunction_Init:

			this->itmapArgs = this->m_mapInitArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapInitArgs.end() )
				return string( "" );

			break;

		case sleFunction_Shutdown:

			this->itmapArgs = this->m_mapShutdownArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapShutdownArgs.end() )
				return string( "" );

			break;

		case sleFunction_Run:
		default:

			this->itmapArgs = this->m_mapRunArgs.find( a_lIndex );

			//
			// NO MATCH FOUND, RETURN AN EMPTY STRING
			//
			if( this->itmapArgs == this->m_mapRunArgs.end() )
				return string( "" );

			break;
	}
		
	//
	// SERCH DESCRIPTION SEPARATOR
	//
	size_t lPos = this->itmapArgs->second.find_first_of( ';' );

	//
	// NOT FOUND - NO DESCRIPTION USED, RETURN FULL STRING
	//
	if ( lPos == string::npos )
		return this->itmapArgs->second;

	//
	// FOUND, RETURN FIRST SUBSTRING
	//
	return this->itmapArgs->second.substr( 0, lPos );
}

//
// PLUGIN INFO FUNCTIONS
//

string slcPluginInfo::GetName( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strName;
}

string slcPluginInfo::GetVendor( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strVendor;
}

string slcPluginInfo::GetInfo( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strInfo;
}

string slcPluginInfo::GetDescription( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strDescription;
}

string slcPluginInfo::GetHomepage( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strHomepage;
}

string slcPluginInfo::GetEmail( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strEmail;
}

long slcPluginInfo::GetMajorVersion( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_lMajor;
}

long slcPluginInfo::GetMinorVersion( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_lMinor;
}

long slcPluginInfo::GetBuildVersion( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_lBuild;
}

string slcPluginInfo::GetUUID( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_strUUID;
}


bool slcPluginInfo::HasPublicArgs( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return this->m_bPuplicArgInfos;
}

slcPluginInfo slcPluginInfo::operator=( const slcPluginInfo& pluginInfo )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	this->m_lMajor = pluginInfo.m_lMajor;
	this->m_lMinor = pluginInfo.m_lMinor;
	this->m_lBuild = pluginInfo.m_lBuild;

	this->m_bPuplicArgInfos = pluginInfo.m_bPuplicArgInfos;

	this->m_strDescription	= pluginInfo.m_strDescription;
	this->m_strEmail		= pluginInfo.m_strEmail;
	this->m_strHomepage		= pluginInfo.m_strHomepage;
	this->m_strInfo			= pluginInfo.m_strInfo;
	this->m_strName			= pluginInfo.m_strName;
	this->m_strUUID			= pluginInfo.m_strUUID;
	this->m_strVendor		= pluginInfo.m_strVendor;

	this->m_mapInitArgs		= pluginInfo.m_mapInitArgs;
	this->m_mapRunArgs		= pluginInfo.m_mapRunArgs;
	this->m_mapShutdownArgs = pluginInfo.m_mapShutdownArgs;
	this->itmapArgs			= pluginInfo.itmapArgs;

	return *this;
}

