
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
/// @file		spl_typedefs.h
/// @version	1.0
/// @brief		Type definitions
/// @author		Andreas Loeffler
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_TYPEDEFS_H__
#define		__SPL_TYPEDEFS_H__

#ifndef		__SPL_PLATFORM_H__
#include	"spl_platform.h"
#endif

//
// NULL DEFINE
//

#ifdef		NULL
	#undef	NULL
#endif

#if SPL_PLATFORM == SPL_PLATFORM_WIN32

	#define	NULL			0L

#else

	#define NULL			0L

#endif	//	SPL_PLATFORM

//
// DEFINE 64-BIT DATATYPE
//
#define			SPL_BITTYPE_32		long

#if SPL_COMPILER == SPL_COMPILER_VCPLUSPLUS  || SPL_COMPILER == SPL_COMPILER_BCPLUSPLUS

	#define		SPL_BITTYPE_64		__int64

#else

	#define		SPL_BITTYPE_64		long long

#endif	// SPL_COMPILER

//
// TODO: ADD YOUR BITTYPE BELOW THIS COMMENT.
//		 EXAMPLES: 
//				
//		 #define SPL_BITTYPE		SPL_BITTYPE_32		---> for 32 bit platforms.
//		 #define SPL_BITTYPE		SPL_BITTYPE_64		---> for 64 bit platforms.
//

// TODO: INSERT YOUR BITTYPE HERE!
#define SPL_BITTYPE		SPL_BITTYPE_32

//
// BIT TYPE CHECKING, DO NOT REMOVE!
//
#ifndef			SPL_BITTYPE
	#error		"Bit type NOT specified yet, please edit spl_typedefs.h line 84 to compile!"
#endif

#endif	//	__SPL_TYPEDEFS_H__

