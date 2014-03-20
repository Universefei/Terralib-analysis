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

#ifndef TEPDIMIXMODELSPECTRALBAND_HPP
#define TEPDIMIXMODELSPECTRALBAND_HPP

#include "TePDIDefines.hpp"
#include <string>

using namespace std;

/**
* @class TePDIMixModelSpectralBand
* @brief This is the class contains the spectral band information for mixmodel algorithms.
* @author Felipe Castro da Silva <felipe@dpi.inpe.br>
* @ingroup PDIMixModel
*/
class PDI_DLL TePDIMixModelSpectralBand
{
	public:
/**
* Default constructor.
* @param s sensor number.
* @param b band number.
* @param ll lower spectral range bound.
* @param ul upper spectral range bound.
* @note In "princo" strategy case, the parameters s, ll and ul, can be zero.
* @param l spectral band name (e.g. CBERS_RED, CBERS_BLUE, CBERS_GREEN, ...).
*/
		TePDIMixModelSpectralBand(unsigned int s, unsigned int b, double ll, double ul, string l);
/**Default constructor.*/
		~TePDIMixModelSpectralBand();
/**
* Return the sensor.
* @return sensor number.
*/
		unsigned int getSensor();
/**
* Return band number.
* @return band number.
*/
		unsigned int getBand();
/**
* Return lower spectral range bound.
* @return lower spectral range bound.
*/
		double getLowerLimit();
/**
* Return upper spectral range bound.
* @return upper spectral range bound.
*/
		double getUpperLimit();
/**
* Return spectral band name.
* @return spectral band name.
*/
		string getLabel();
    
    // overload
    bool operator==( const TePDIMixModelSpectralBand& external ) const;        

	private:
    /** @brief Sensor number.*/	
		unsigned int	sensor;

    /** @brief Band number.*/			
		unsigned int	band;

    /** @brief Lower spectral range bound.*/			
		double	lowerlimit;

    /** @brief Upper spectral range bound.*/			
		double	upperlimit;

    /** @brief Spectral band name.*/			
		string label;

};

#endif
