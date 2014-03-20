
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
/// @file		spl_pluginargs.h
/// @version	1.0
/// @brief		The plugin argument class.
/// @author		René Stuhr
//---------------------------------------------------------------------------------------------------------------------------

#ifndef		__SPL_PLUGINARGS_H__
#define		__SPL_PLUGINARGS_H__

#ifndef		__SPL_TYPEDEFS_H__
#include	"spl_typedefs.h"
#endif

#ifndef		__SPL_MUTEX_H__
#include	"spl_mutex.h"
#endif

#ifndef		_MAP_
#include	<map>
#endif

#ifndef		_STRING_
#include	<string>
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


using namespace std;

namespace SPL {

//---------------------------------------------------------------------------------------------------------------------------
///	@brief		The plugin argument class.
///	@ingroup	SPL
//---------------------------------------------------------------------------------------------------------------------------
class slcPluginArgs : public slcMutex
{
	//
	//-----------------------------------------------------------------------------------------------------------------------
	//***********************************************     CON/DESTRUCTION     ***********************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default constructor.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcPluginArgs( void );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Copy constructor.
	//-----------------------------------------------------------------------------------------------------------------------
	SPL_API slcPluginArgs( const slcPluginArgs& a_pluginArgs );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Default destructor.
	//-----------------------------------------------------------------------------------------------------------------------
	virtual SPL_API ~slcPluginArgs( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//************************************************     MODIFICATION     *************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
public:

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Sets an argument at the specified index (0-based).
	///	@param		a_lIndex	[in] Index where to fill the argument in.
	///	@param		a_pvArg		[in] Pointer to an argument.
	///	@return		The argument's index (should not differ from a_lIndex!).
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API SetArg( long a_lIndex, void* a_pvArg );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Sets an argument at the next free position.
	///	@param		a_pvArg		[in] Pointer to an argument.
	///	@return		The argument's index.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API SetArg( void* a_pvArg );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the number of arguments.
	///	@return		Number of arguments.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetCount( void );// const;

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Gets the argument at the specified index.
	///	@param		a_lIndex	[in] Index of argument to get.
	///	@param		a_pvArg		[out] Pointer to a valid, allocated memory block holding the argument.
	///	@return		The argument's index (should not differ from a_lIndex!).
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API GetArg( long a_lIndex, void* a_pvArg );

	//-----------------------------------------------------------------------------------------------------------------------
	///	@brief		Returns the number of deleted arguments.
	///	@return		Number of arguments.
	//-----------------------------------------------------------------------------------------------------------------------
	long SPL_API Clear( void );

	//
	//-----------------------------------------------------------------------------------------------------------------------
	//*************************************************     ATTRIBUTES     **************************************************
	//-----------------------------------------------------------------------------------------------------------------------
	//
private:
	
	map< long, SPL_BITTYPE >	m_mapArgs;		///< Map for arguments.
};

} // End of namespace SPL

#endif	// End __SPL_PLUGINARGS_H__
