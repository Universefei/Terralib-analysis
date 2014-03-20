
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
/// @file		spl_pluginserver.cpp
/// @version	1.0
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGINSERVER_H__
#include	"../include/spl_pluginserver.h"
#endif

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif

slaPluginServer::slaPluginServer( void )
{

}

slaPluginServer::~slaPluginServer( void )
{

}

void slaPluginServer::Clear( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	//
	// cleanup all internal maps
	//
	this->m_mapPluginIds.clear();
	this->m_mapPluginInfos.clear();
}

slcPluginInfo slaPluginServer::GetSafedPluginInfo( const long a_lIndex ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	this->itInfos  = this->m_mapPluginInfos.find( a_lIndex ); 

	if( this->itInfos != this->m_mapPluginInfos.end() )
	{
		return this->itInfos->second;
	}
	
	return this->m_nullInfo;
}

long slaPluginServer::GetLoadedPlugins( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return( long )this->m_mapPluginIds.size();
}

