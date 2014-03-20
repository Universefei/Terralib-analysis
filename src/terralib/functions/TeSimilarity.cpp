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

#include "../kernel/TeMatrix.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

double
SmallestOf (double a, double b, double c)
{
	double	smallest = a;
	
	if (b < smallest) smallest = b;
	if (c < smallest) smallest = c;
	return smallest;
}

double
TeSimilarity (const char *streetname, const char *rotuloname)
{
	short	Del = 1, Ins = 1, Sub = 1;
	double		a, b, c;
	short	i, j;
	int	m = strlen(streetname) + 1;
	int n = strlen(rotuloname) + 1;
	TeMatrix	T;
	if( T.Init(m,n) == false)
		return -1.;

	T(0,0) = 0;

	for (j = 1; j<n; j++)
		T(0,j) = T(0, j-1) + Ins;

	for (i = 1; i<m; i++)
		T(i,0) = T(i-1,0) + Del;

	for (i = 1; i<m; i++)
		for (j = 1; j<n; j++)
		{
			if (toupper(streetname[i-1]) == toupper(rotuloname[j-1]))
				a = T(i-1,j-1);
			else
				a = T(i-1,j-1) + Sub;
			b = T(i,j-1) + Ins;
			c = T(i-1,j) + Del;
			T(i,j) = SmallestOf(a, b, c);
		}
		
	double perc = (1.0 - (T(m-1,n-1) / (double)(n-1))) * 100.0;
	return	perc;
}
