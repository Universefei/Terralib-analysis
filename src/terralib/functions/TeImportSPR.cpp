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

// STL 
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <map>

using namespace std;

// TerraLib

#include "TeImportSPR.h"

#include "TeAsciiFile.h"
#include "TeBox.h"
#include "TeLayer.h"
#include "TeException.h"
#include "TeUtils.h"


void 
TeImportSPR ( TeLayer& layer, const string& fileName )
{

	TePolygonSet& regions	= layer.polygonSet();
	TeLineSet& lines		= layer.lineSet();
	TePointSet& points		= layer.pointSet();
	TeTextSet& textos		= layer.textSet();

/* -------------------------------------------------------------------- */
/*      Open the  file.                                                 */
/* -------------------------------------------------------------------- */

	TeAsciiFile	pFile (fileName);

	int geomId = 1;
	int size,nrings;

	string geoId;

/* -------------------------------------------------------------------- */
/*      Get information about the file                                  */
/* -------------------------------------------------------------------- */
	string repType = pFile.readString ();
	pFile.findNewLine();

	if (repType == "POLIGONOS")
	{
		layer.addGeometry ( TePOLYGONS );
		while ( 1 )
		{
			TePolygon poly;
			nrings = pFile.readInt ();
			if (!pFile.isNotAtEOF())
				break;
			geoId = pFile.readString ();
			poly.objectId ( geoId );
			int i,j;
			for (j=0;j<nrings;j++)
			{
				TeLine2D line;
				size = pFile.readInt ();
				pFile.findNewLine();
				for (i=0;i<size;i++)
				{
// Build the polygon
					TeCoord2D xy = pFile.readCoord2D ();
					line.add ( xy );
				}
				TeLinearRing ring (line);
				ring.geomId ( geomId );
				poly.add ( ring );
				geomId++;
			}
			regions.add ( poly );
		}
		layer.setBox ( regions.box() );
	}
	if (repType == "LINHAS" || repType == "LINES")
	{
		TeDatum sad69 = TeDatumFactory::make("SAD69");
		TeProjection* proj = new TeUtm ( sad69 , -39.*TeCDR);
		layer.setProjection(proj);
		string infoend;
		char separator = ';';
		while (infoend != "INFO_END" && pFile.isNotAtEOF())
			infoend = pFile.readString ();
		layer.addGeometry ( TeLINES );
		while ( 1 )
		{
			geoId = pFile.readString ();
			if (!pFile.isNotAtEOF())
				break;
			int i;
			TeLine2D line;
			size = pFile.readInt ();
			pFile.findNewLine();
			for (i=0;i<size;i++)
			{
// Build the line
				TeCoord2D xy = pFile.readCoord2D ();
				line.add ( xy );
			}
			line.objectId ( geoId );
			line.geomId ( geomId );
			geomId++;
			lines.add ( line );
		}
		layer.setBox ( lines.box() );
	}
	if (repType == "POINT2D")
	{
		TeDatum sad69 = TeDatumFactory::make("SAD69");
		TeProjection* proj = new TeUtm ( sad69 , -39.*TeCDR);
		layer.addGeometry ( TePOINTS );
		string infoend;
		char separator = ';';
		while (infoend != "INFO_END" && pFile.isNotAtEOF())
			infoend = pFile.readString ();
		pFile.findNewLine();
		try
		{
			while ( pFile.isNotAtEOF() )
			{
				double x,y;
				char rotulo [128], lixo[4];
				string input = pFile.readLine();
				if (!pFile.isNotAtEOF() || input == "END")
					break;
				sscanf (input.c_str (), "%lf %s %lf %s %s",&x, lixo, &y, lixo, rotulo);
				geoId = rotulo;

// Build the point
				TePoint point;
				TeCoord2D xy (x,y);
//				xy = proj->PC2LL (xy);
//				TeCoord2D xyd (xy.x()*TeCRD,xy.y()*TeCRD);
				point.add ( xy );
				point.objectId ( geoId );
//				point.geomId ( geomId );	
				points.add ( point );
				geomId++;
			}
		}
		catch (...)
		{
		}
		layer.setBox ( points.box() );
	}
	if (repType == "TEXT")
	{
		TeDatum sad69 = TeDatumFactory::make("SAD69");
		TeProjection* proj = new TeUtm ( sad69 , -39.*TeCDR);
		layer.setProjection(proj);
		layer.addGeometry ( TeTEXT );
		string infoend;
		char separator = ';';
		while (infoend != "INFO_END" && pFile.isNotAtEOF())
			infoend = pFile.readString ();
		pFile.findNewLine();
		try
		{
			while ( pFile.isNotAtEOF() )
			{
				double x,y,ang,exp,hgt;
//				char rotulo [128], lixo[4];
//				string input = pFile.readLine();
//				if (!pFile.isNotAtEOF() || input == "END")
//					break;
//				sscanf (input.c_str (), "%lf %s %lf %1s %s %s %lf %s %lf %s %lf %s",
//					&x, lixo, &y, lixo, rotulo, lixo, &ang, lixo, &exp, lixo, &hgt);
				x = pFile.readFloatCSV(separator);
				if (!pFile.isNotAtEOF() || x == 0.0)
					break;
				y = pFile.readFloatCSV(separator);
				string rotulo = pFile.readStringCSV(separator);
				ang = pFile.readFloatCSV(separator);
				exp = pFile.readFloatCSV(separator);
				hgt = pFile.readFloatCSV(separator);

// Build the point
				TeText texto;
				TeCoord2D xy (x,y);
//				xy = proj->PC2LL (xy);
//				TeCoord2D xyd (xy.x()*TeCRD,xy.y()*TeCRD);
				TeCoord2D xyd (x,y);
				texto.add ( xyd );
				texto.objectId ( geoId );
				texto.setText ( rotulo );	
				texto.setAngle ( ang );	
				texto.setHeight ( hgt );	
				textos.add ( texto );
				pFile.findNewLine();
			}
		}
		catch (...)
		{
		}
		layer.setBox ( textos.box() );
	}
}
