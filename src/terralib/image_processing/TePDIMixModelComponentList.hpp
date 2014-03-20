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

#ifndef TEPDIMIXMODELCOMPONENTLIST_HPP
#define TEPDIMIXMODELCOMPONENTLIST_HPP

#include <string>
#include <map>

#include "../kernel/TeSharedPtr.h"
#include "TePDIMixModelComponent.hpp"

using namespace std;

/**
* @class TePDIMixModelComponentList
* @brief This is the class contains the list of components for mixmodel algorithms.
* @author Felipe Castro da Silva <felipe@dpi.inpe.br>
* @ingroup PDIMixModel
*/
class PDI_DLL TePDIMixModelComponentList
{
	public:
/**Default constructor.*/
		TePDIMixModelComponentList();
/**Default destructor.*/
		~TePDIMixModelComponentList();
/**
* Insert component.
* @param cn component number where the component will be stored.
* @param c component to be stored.
* @return true if the component was inserted, false if not.
*/
		bool insertComponent(unsigned cn, TePDIMixModelComponent c);
/**
* Insert a blank component called l.
* @param cn component number where the component will be stored.
* @param l name of the component (e.g. cloud, vegetation, ...).
* @return true if the component was inserted, false if not.
*/
		bool insertComponent(unsigned cn, string l);
/**
* Remove component.
* @param cn component number to be removed.
* @return true if the component was removed, false if not.
*/
		bool removeComponent(unsigned cn);
/**
* Return component list size.
* @return return the componentlist size.
*/
		unsigned getSize();
/**
* Insert pixel.
* @param cn component number where the pixel will be stored.
* @param bn band number where the pixel will be stored.
* @param p value of the pixel.
* @return true if the pixel was inserted, false if not.
*/
		bool insertPixel(unsigned cn, unsigned bn, double p);
/**
* Remove pixel.
* @param cn component number where the pixel will be removed.
* @param bn band number where the pixel will be removed.
* @return true if the pixel was removed, false if not.
*/
		bool removePixel(unsigned cn, unsigned bn);
/**
* Return pixel value.
* @param cn component number.
* @param bn band number.
* @return if cn and bn exist, pixel value, if not -1.0
*/
		double getPixel(unsigned cn, unsigned bn);
/**
* Return component size.
* @param cn component number.
* @return if cn exists, component size, if not -1.
*/
		int getComponentSize(unsigned cn);
    
    // overload
    bool operator==( const TePDIMixModelComponentList& external ) const;
    
	private:
/**Store component number and component.*/
		map<unsigned, TePDIMixModelComponent> components;
};

#endif
