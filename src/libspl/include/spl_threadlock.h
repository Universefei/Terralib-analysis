
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
/// @file		spl_threadlock.h
/// @version	1.0
/// @brief		Simple utility locking class.
/// @author		René Stuhr
/// @remarks	Lock object to make operations thread safe. 
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_THREADLOCK_H__
#define		__SPL_THREADLOCK_H__

#ifndef		__SPL_TYPEDEFS_H__
#include	"spl_typedefs.h"
#endif

#ifndef		__SPL_MUTEX_H__
#include	"spl_mutex.h"
#endif

#if 		SPL_PLATFORM == SPL_PLATFORM_LINUX
#include	<errno.h>
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

namespace SPL 
{

//---------------------------------------------------------------------------------------------------------------------------
///	@brief		Thread safe utility class.
///	@ingroup	SPL
//---------------------------------------------------------------------------------------------------------------------------
class slcThreadLock
{
	//
	//-----------------------------------------------------------------------------------------------------------------------
	//***********************************************     CON/DESTRUCTION     ***********************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Initialisation constructor.
	///	@param		a_pMutex		[in] Pointer to a slcMutex object for locking (usually "this").
	///	@param		a_bLock			[in] Flag to lock automatically for implicitly or explicitly usage.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcThreadLock( slcMutex* a_pMutex, bool a_bLock = false );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default destructor.
	/// @remarks	Clears all open (forgotten) locks for deadlock safety.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API ~slcThreadLock( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//************************************************     MODIFICATION     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Locks a section - If section is already busy then block and wait until it's ready again.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API Lock( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Locks a section - If section is busy, return without blocking.
	///	@return		The method returns one of the following values:
	///	@retval		EBUSY | Section is still locked from another caller.
	///	@retval		0     | Section is now locked by current call.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual long SPL_API TryLock( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Unlocks a section.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual void SPL_API Unlock( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//*************************************************     ATTRIBUTES     **************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
private:
	
	slcMutex*		m_pMutex;			///< Pointer to mutex object.
	long			m_lLockCount;		///< Internal lock counter.
};

} // End of namespace SPL

#endif	// __SPL_THREADLOCK_H__

