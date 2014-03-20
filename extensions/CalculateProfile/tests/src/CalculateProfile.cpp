/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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
/* 
        This file showns an example of how to calculate a Terrain Profile
        using a TeContourLineSet and a TeLine2D

        Author: Luiz Claudio Oliveira de Andrade  
*/
#include "TeGeometry.h"
#include "TeProfile.h"
#include "TeIntersector.h"

void main(int argc, char* argv[])
{
	TeLine2D l[4];
	//
	l[0].add(TeCoord2D(2,2));
	l[0].add(TeCoord2D(6,4));
	l[0].add(TeCoord2D(5,8));
	l[0].add(TeCoord2D(1,7));
	l[0].add(TeCoord2D(2,2));
	//
	l[1].add(TeCoord2D(4,4));
	l[1].add(TeCoord2D(5,6));
	l[1].add(TeCoord2D(2,6));
	l[1].add(TeCoord2D(4,4));
	//
	l[2].add(TeCoord2D(9,7));
	l[2].add(TeCoord2D(12,7));
	l[2].add(TeCoord2D(11,4));
	l[2].add(TeCoord2D(9,7));
	//
	l[3].add(TeCoord2D(1,3));
	l[3].add(TeCoord2D(12,8));
	//
	TeContourLine iso1(l[0],500);
	TeContourLine iso2(l[1],550);
	TeContourLine iso3(l[2],450);
	//
	TeContourLineSet isoSet;
	isoSet.add(iso1);
	isoSet.add(iso2);
	isoSet.add(iso3);
	//
	TeProfile p(&isoSet,&l[3]);
	TeLine2D *terrainProfile = p.calculateProfile();
	getchar();
}

