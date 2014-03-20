
/*!

\section terms_of_use Terms of Use

libspl - the simple plugin layer library.
Copyright (C) 2004 Andreas Loeffler and Ren�Stuhr (www.unitedbytes.de).

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
/// @file		spl_platform.h
/// @version	1.0
/// @brief		Platform handling & detection.
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLATFORM_H__
#define		__SPL_PLATFORM_H__

//
// PLATFORM DEFINES
//
#define		SPL_PLATFORM_WIN32			1
#define		SPL_PLATFORM_LINUX			2
#define		SPL_PLATFORM_APPLE			3
#define 	SPL_PLATFORM_SOLARIS			4

//
// COMPILER DEFINES
//
#define		SPL_COMPILER_VCPLUSPLUS		1
#define		SPL_COMPILER_BCPLUSPLUS		2
#define		SPL_COMPILER_GNUC			3
#define		SPL_COMPILER_CODEWARRIOR	4
#define		SPL_COMPILER_SUNFORTE7		5

//
// PLATFORM DETECTION
//
#if	defined( WIN32 ) || defined( __WIN32__ ) || defined ( _WIN32 ) || defined( WIN64 ) || defined( _WIN32_WCE )

	#define SPL_PLATFORM				SPL_PLATFORM_WIN32
	
	//
	// Save compile date and time to lib
	//
	#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ )

#elif defined( __LINUX__ ) || defined( linux ) || defined( __FreeBSD__ )

	#define SPL_PLATFORM				SPL_PLATFORM_LINUX

#elif defined( __APPLE__ ) || defined( APPLE_CC )

	#define	SPL_PLATFORM				SPL_PLATFORM_LINUX

#elif defined(sparc) && defined(sun) && defined(unix)	

	//
	// (karg) Solaris port stuff	
	//
	#define SPL_PLATFORM				SPL_PLATFORM_SOLARIS

#else

	#error "ERROR: Operating system is unknown!"

#endif	//	PLATFORM

//
// COMPILER DETECTION
//
#if defined( _MSC_VER )

	#define SPL_COMPILER				SPL_COMPILER_VCPLUSPLUS

#elif defined( __BORLANDC__ ) || defined( __BCPLUSPLUS__ )

	#define SPL_COMPILER				SPL_COMPILER_BCPLUSPLUS

#elif defined( __GNUC__ )

	#define SPL_COMPILER				SPL_COMPILER_GNUC

#elif defined( __MWERKS__ )

	#define SPL_COMPILER				SPL_COMPILER_CODEWARRIOR

#elif defined( __SUNPRO_CC)

	//
	// Sun forte compiler, the solaris port (karg)
	//
	#define SPL_COMPILER				SPL_COMPILER_SUNFORTE7

#else

	#error "ERROR: Compiler is unknown!"

#endif	//	COMPILER


#endif	//	__SPL_PLATFORM_H__

