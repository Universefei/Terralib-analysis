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


#ifndef TEPDIMIXMODELCOMPONENT_HPP
#define TEPDIMIXMODELCOMPONENT_HPP

#include "TePDIDefines.hpp"
#include <string>
#include <map>

using namespace std;

/**
* @class TePDIMixModelComponent
* @brief This is the class contains the component information for mixmodel algorithms.
* @author Felipe Castro da Silva <felipe@dpi.inpe.br>
* @ingroup PDIMixModel
*/
class PDI_DLL TePDIMixModelComponent
{
	public:
/**
* Default constructor.
* @param l name of the component (e.g. cloud, vegetation, ...).
*/
		TePDIMixModelComponent(string l);
/**Default destructor.*/
		~TePDIMixModelComponent();

/**
* Insert pixel.
* @param bn band number where the pixel will be stored.
* @param p pixel value.
* @return true if pixel was inserted, false if not.
*/
		bool insertPixel(unsigned int bn, double p);
/**
* Remove pixel.
* @param bn band number of the pixel to be removed.
* @return true if pixel was removed, false if not.
*/
		bool removePixel(unsigned int bn);
/**
* Return pixel value.
* @param bn pixel band number.
* @return if bn exists, pixel value, if note -1.0.
*/
		double getPixel(unsigned int bn);
/**
* Return bands number.
* @return bands number in the component.
*/
		unsigned getSize();
/**Clear all the component.*/
		void clearPixels();
    
    // overload
    bool operator==( const TePDIMixModelComponent& external ) const;    

	private:
/**The component name (e.g. cloud, vegetation, ...).*/
		string label;
/**Store band number and pixel value.*/
		map<unsigned int, double> pixels;
};

#endif


