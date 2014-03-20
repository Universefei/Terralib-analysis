/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeCoordAlgorithms.h
    \brief This file contains algorithms to deal with geographical coordinates
*/
#ifndef  __TERRALIB_INTERNAL_COORDALGORITHMS_H
#define  __TERRALIB_INTERNAL_COORDALGORITHMS_H

#include "TeFunctionsDefines.h"

#include <string>
using namespace std;
/*! 
   \fn bool TeLongDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec) 
   \brief Transform a Longitude Coordinate in degrees, minutes and seconds to decimal degrees
   \param hem a character that indicates North or South hemisphere
   \param dg dregrees value
   \param mn minutes value
   \param sc seconds value
   \param grauDec returns the decimal degree value
   \return TRUE if input is a valid longitude coordinate
*/
TLFUNCTIONS_DLL bool TeLongDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec);

/*! 
   \fn bool TeLatDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec) 
   \brief Transform a Latitude Coordinate in degrees, minutes and seconds to decimal degrees
   \param hem a character that indicates West or East
   \param dg dregrees value
   \param mn minutes value
   \param sc seconds value
   \param grauDec returns the decimal degree value
   \return TRUE if input is a valid latitude coordinate
*/
TLFUNCTIONS_DLL bool TeLatDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec);

#endif
