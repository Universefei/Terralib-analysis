/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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

#include "TeCoordAlgorithms.h"
#include <math.h>
#include <cstdlib>

bool TeLongDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec)
{
	short  	posat,lim = 180;

	double	dd,pi;			
	posat = 0;
	pi = 4.*atan((double)1.);

	// Longitude 
	if(dg < 0 || dg > lim)	
		return false;

	if(mn < 0 || mn > 60)
		return false;

	if(sc < 0. || sc > 60.)
		return false;

	if(hem == 'W' || hem == 'w'|| hem == 'O'|| hem == 'o')
	{
		if(dg > 0)
			dg = dg * -1;
		else if(mn > 0)
			mn = mn * -1;
		else
			sc = (float)(sc * -1.);
	}

	dd = (double)(abs(dg)) + ((double)abs(mn)/60.) + fabs(sc)/3600.;
	if (dg < 0 || mn < 0 || (int)sc < 0)
		dd = -dd;
	grauDec = dd;
	return true;
}


bool
TeLatDMS2DD(char hem, short& dg, short& mn, float& sc, double& grauDec)
{
	short  	posat,lim = 90;

	double	dd,pi;			
	posat = 0;
	pi = 4.*atan((double)1.);

	// Longitude 
	if(dg < 0 || dg > lim)	
		return false;

	if(mn < 0 || mn > 60)
		return false;

	if(sc < 0. || sc > 60.)
		return false;

	if(hem == 's' || hem == 'S')
	{
		if(dg > 0)
			dg = dg * -1;
		else if(mn > 0)
			mn = mn * -1;
		else
			sc = (float)(sc * -1.);
	}

	dd = (double)(abs(dg)) + ((double)abs(mn)/60.) + fabs(sc)/3600.;
	if (dg < 0 || mn < 0 || (int)sc < 0)
		dd = -dd;

//	dd = dd*pi/180.;
	grauDec = dd;
	return true;
}
