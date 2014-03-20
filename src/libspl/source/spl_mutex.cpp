
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
/// @file		spl_mutex.cpp
/// @version	1.0
/// @author		Ren� Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_MUTEX_H__
#include	"../include/spl_mutex.h"
#endif

#ifndef		__SPL_TYPEDEFS_H__
#include	"../include/spl_typedefs.h"
#endif

#include <stdio.h>

using namespace SPL;

slcMutex::slcMutex( void )
{
#if		SPL_PLATFORM == SPL_PLATFORM_LINUX	

	pthread_mutexattr_t access_attr;
	pthread_mutexattr_init( &access_attr );
	pthread_mutexattr_settype( &access_attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &this->m_access, &access_attr );

#elif	SPL_PLATFORM == SPL_PLATFORM_WIN32	

	this->m_sa.nLength = sizeof( m_sa );
	this->m_sa.lpSecurityDescriptor = NULL;
	this->m_sa.bInheritHandle = TRUE;

	//
	// CREATE A RANDOM MUTEX UUID
	//
	char szUUID[ 41 ] = { 0 };
	SYSTEMTIME st;
	::GetSystemTime( &st );
	sprintf( szUUID, "UUID:%02d:%02d:%02d:%03d:%02d:%02d:%02d:%04d", 
		st.wDay, st.wDayOfWeek, st.wHour, st.wMilliseconds, st.wMinute,
		st.wMonth, st.wSecond, st.wYear );

	this->m_access = ::CreateMutex( &this->m_sa, FALSE, szUUID );

#endif
}

slcMutex::~slcMutex( void )
{
#if		SPL_PLATFORM == SPL_PLATFORM_LINUX	

	pthread_mutex_destroy( &this->m_access );

#elif	SPL_PLATFORM == SPL_PLATFORM_WIN32	

	::CloseHandle( this->m_access );

#endif
}

void slcMutex::Lock( void )
{
#if		SPL_PLATFORM == SPL_PLATFORM_LINUX	

	pthread_mutex_lock( &this->m_access );

#elif	SPL_PLATFORM == SPL_PLATFORM_WIN32	

	::WaitForSingleObject( this->m_access, INFINITE );

#endif
}

long slcMutex::TryLock( void )
{
	// 
	// Set default to signaled.
	//
	long lResult = 0;

#if		SPL_PLATFORM == SPL_PLATFORM_LINUX	

	//
	// Check if object signaled and lock if success.
	// If object is non signaled return busy otherwise object is now locked.
	//
	if( ( lResult = pthread_mutex_trylock( &this->m_access ) ) == EBUSY )
		 return EBUSY;		///< Return busy.

#elif	SPL_PLATFORM == SPL_PLATFORM_WIN32	

	//
	// Wait 10 ms of signaled state and lock if success.
	// If object is non signaled return busy otherwise object is now locked.
	//
	if( ( lResult = ::WaitForSingleObject( this->m_access, 10 ) ) == WAIT_TIMEOUT )
		return EBUSY;			///< Return busy.

#endif

	return lResult;		///< If 0 object is successfully locked otherwise error, return result.
}

void slcMutex::Unlock( void )
{
#if		SPL_PLATFORM == SPL_PLATFORM_LINUX	

	pthread_mutex_unlock( &this->m_access );

#elif	SPL_PLATFORM == SPL_PLATFORM_WIN32	

	::ReleaseMutex( this->m_access );

#endif
}
