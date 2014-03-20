/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/** \file TeThreadFunctor.h
 *  \brief This file contains a base class for thread in function style manner.
 *  \author Emiliano F. Castejon <castejon@dpi.inpe.br>
 *  \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
 */

#ifndef __TERRALIB_INTERNAL_TETRHEADFUNCTOR_H
#define __TERRALIB_INTERNAL_TETRHEADFUNCTOR_H
  
#include "TeThread.h"
#include "TeThreadParameters.h"
#include "TeSharedPtr.h"
#include "TeDefines.h"

/** \class TeThreadFunctor
 *  \brief A base class for thread in function style manner.
 *
 *
 *  If you are interested in using TerraLib thread
 *  support in a function style model, you should create
 *  objects of this class by specifying a function to be called by
 *  the thread when it starts.<br>
 *  To start the thread, call start method,
 *  it is non-blocking.<br>
 *  If you want to use the thread support
 *  in a object oriented way, see TeThread class.
 *  
 *
 *  \sa TeThread
 *  \author Emiliano F. Castejon <castejon@dpi.inpe.br>
 *  \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
 *  \ingroup MultProgToolsGroup
 *
 */  
class TL_DLL TeThreadFunctor : public TeThread
{
	public:

    /** \brief Type definition for a thread start function pointer.
     *  \param params The thread parameters.
     *  \return true if OK, false on erros.
     */
    typedef bool (*TeThreadStartFunctT1)( const TeThreadParameters& params );
    
    /** \brief Type definition for a thread start function pointer.
     *  \param userParamsPtr A pointer to anything required by the user.
     *  \return true if OK, false on erros.
     */
    typedef bool (*TeThreadStartFunctT2)( void* userParamsPtr );        

    /** \deprecated Use TeThreadStartFunctT1. */
    typedef TeThreadStartFunctT1 TeThreadStartFunctT;
    
    /** @typedef TeSharedPtr< TeThread > pointer
    * Type definition for an thread instance pointer. 
    */
    typedef TeSharedPtr< TeThreadFunctor > pointer;         

    /** \brief Default Constructor.
     */
    TeThreadFunctor();             

    /** \brief Default Destructor
     */
    ~TeThreadFunctor();

    /** \brief Returns the current thread execution return value.
     */      
    bool getReturnValue() const;

		/** \brief Change the internal thread start function pointer.
		 *  \param startFuncPtr The new thread start function pointer.
		 *  \note The thread mus be stopped for calling this method.
     */      
    void setStartFunctPtr( TeThreadStartFunctT1 startFuncPtr );
    
    /** \brief Change the internal thread start function pointer.
     *  \param startFuncPtr The new thread start function pointer.
     *  \note The thread mus be stopped for calling this method.
     */      
    void setStartFunctPtr( TeThreadStartFunctT2 startFuncPtr );    

		/** \brief Sets the parameter that will be passed to the function after
		 *         the thread startup.
		 *  \param params The parameters to the thread function.
		 *  \note The thread must be stopped before calling this method.
		 */
    void setParameters(const TeThreadParameters& params);
    
    /** \brief Sets a pointer that will be passed to the function after
     *         the thread startup.
     *  \param userParamsPtr The user parameters pointer.
     *  \note The thread must be stopped before calling this method.
     */
    void setParametersPtr( void* userParamsPtr );    

	protected:

		// Overloaded method from TeThread.
		void run();

	protected:

		volatile bool threadReturnValue_;					//!< User function return value.
    TeThreadParameters threadUserParams_;		//!< The current user parameters instance.
    TeThreadStartFunctT1 threadStartFuncPtr1_;	//!< A pointer to the current user thread start function.
    TeThreadStartFunctT2 threadStartFuncPtr2_; //!< A pointer to the current user thread start function.
    void* userParamsPtr_;//!< A pointer to the current user thread start function parameters pointer.
};
  
#endif	// __TERRALIB_INTERNAL_TETRHEADFUNCTOR_H

