/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/


#ifndef TEPDIMIXMODEL_HPP
#define TEPDIMIXMODEL_HPP

#include "TePDIStrategyFactory.hpp"
#include "../kernel/TeAgnostic.h"
#include "TePDIAlgorithm.hpp"
#include "TePDIParameters.hpp"
#include "../kernel/TeSharedPtr.h"

/**
* @brief This is the base class for image mixmodel algorithms.
* @author Felipe Castro da Silva <felipe@dpi.inpe.br>
* @ingroup TePDIGeneralAlgoGroup
*
* @note The general required parameters:
* @param mixmodel_type (std::string) - MixModel Strategy type
*
* @note The specific parameters: See each mixmodel strategy for reference.   
*/

class PDI_DLL TePDIMixModel : public TePDIAlgorithm
{
	
	public :
		
/*		typedef TeSharedPtr< TePDIMixModel > pointer;
		typedef const TeSharedPtr< TePDIMixModel > const_pointer;*/
		
		/**
		* Default Constructor.
		*
		*/
		TePDIMixModel();
		
		/**
		* Default Destructor
		*/
		virtual ~TePDIMixModel();
		
		/**
		* Checks if the supplied parameters fits the requirements of each
		* PDI algorithm implementation.
		*
		* Error log messages will be generated. No exceptions generated.
		*
		* @param parameters The parameters to be checked.
		* @return true if the parameters are OK. false if not.
		*/
		bool CheckParameters( const TePDIParameters& parameters ) const;

	protected :
	
		/**
		* Reset the internal state to the initial state.
		*
		* @param params The new parameters referente at initial state.
		*/
		void ResetState( const TePDIParameters& params );

		/**
		* Runs the current algorithm implementation.
		*
		* @return true if OK. false on error.
		*/
		bool RunImplementation();

};
  
/** @example TePDIMixModel_test.cpp
 *    Shows how to use this class.
 */

#endif
