
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
/// @file		spl_pluginargs.cpp
/// @version	1.0
/// @author		Ren� Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGINARGS_H__
#include	"../include/spl_pluginargs.h"
#endif

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif

using namespace SPL;

slcPluginArgs::slcPluginArgs( void ) 
{

}

slcPluginArgs::slcPluginArgs( const slcPluginArgs& a_pluginArgs )
{
	if( a_pluginArgs.m_mapArgs.size() == 0 )
	{
		return;
	}

	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	//
	// COPY MAP ELEMENTS
	//
	this->m_mapArgs = a_pluginArgs.m_mapArgs;
}

slcPluginArgs::~slcPluginArgs( void )
{
}

long slcPluginArgs::SetArg( long a_lIndex, void* a_pvArg )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	if( ( a_lIndex < 0 ) ||
		( NULL == a_pvArg ) )
	{
		return -1;
	}

	//
	// IF a_lIndex OUT OF RANGE FIX IT
	//
	if( a_lIndex > (long) this->m_mapArgs.size() )
		a_lIndex = (long) this->m_mapArgs.size();

	//
	// SAVE ARGUMENT
	//
	this->m_mapArgs.insert( pair< long, SPL_BITTYPE> ( a_lIndex, (SPL_BITTYPE) a_pvArg ) );

	return a_lIndex;
}

long slcPluginArgs::SetArg( void* a_pvArg )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	//
	// SAVE ARGUMENT
	//
	this->m_mapArgs.insert( pair< long, SPL_BITTYPE> ( (long) this->m_mapArgs.size(), (SPL_BITTYPE) a_pvArg ) );

	return (long) this->m_mapArgs.size() - 1;
}

long slcPluginArgs::GetCount( void ) 
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	return (long) this->m_mapArgs.size();
}

long slcPluginArgs::GetArg( long a_lIndex, void* a_pvArg )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	if( ( a_lIndex > (long) this->m_mapArgs.size() ) ||
		( a_lIndex < 0 ) )
	{
		return -1;
	}

	*( SPL_BITTYPE* )a_pvArg = ( SPL_BITTYPE )this->m_mapArgs[ a_lIndex ];
	return a_lIndex;
}

long slcPluginArgs::Clear( void )
{
	//
	// LOCKS OBJECT FOR EXCLUSIVE USE, UNLOCKS AUTOMATICALLY AT THE END OF FUNCTION.
	// IMPLICIT USAGE.
	//
	slcThreadLock threadLock( this, true );

	long lArguments = (long) this->m_mapArgs.size();
	this->m_mapArgs.clear();
	return lArguments;
}

