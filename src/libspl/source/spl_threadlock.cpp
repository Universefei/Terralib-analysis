
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
/// @file		spl_threadlock.cpp
/// @version	1.0
/// @author		Ren� Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_THREADLOCK_H__
#include	"../include/spl_threadlock.h"
#endif

using namespace SPL;

slcThreadLock::slcThreadLock( slcMutex* a_pMutex, bool a_bLock ) : m_lLockCount( 0 )
{
	this->m_pMutex = a_pMutex;

	if( a_bLock )
		this->Lock();
}

slcThreadLock::~slcThreadLock( void )
{
	while( this->m_lLockCount > 0 )
		this->Unlock();
}

void slcThreadLock::Lock( void )
{
	if( this->m_pMutex )
	{
		this->m_pMutex->Lock();
		this->m_lLockCount ++;
	}
}

long slcThreadLock::TryLock( void )
{
	long lResult = 0;

	if( this->m_pMutex )
	{
		lResult = this->m_pMutex->TryLock();
		if( lResult != EBUSY )		///< Lock success?
			this->m_lLockCount ++;  ///< Incrase lock counter
	}

	return lResult;
}

void slcThreadLock::Unlock( void )
{
	if( this->m_pMutex )
	{
		this->m_pMutex->Unlock();
		this->m_lLockCount--;
	}
}
