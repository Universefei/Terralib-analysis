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
#include "TeSTEFunctionsSHP.h"
#include "TeSTElementSet.h"
#include <vector>

bool TeDecodeShape(SHPObject* psShape, TeMultiGeometry& geomestries, string& objectId)
{
	int shpType = psShape->nSHPType;
	switch (shpType)
	{
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
			TeSHPPolygonDecode(psShape,geomestries.polygons_,objectId);
			return true;
		case SHPT_ARC:
		case SHPT_ARCZ:
			TeSHPPolyLineDecode(psShape,geomestries.lines_,objectId);
			return true;
		case SHPT_POINT:
		case SHPT_POINTZ:
		case SHPT_MULTIPOINT:
		case SHPT_MULTIPOINTZ:
			TeSHPPointDecode(psShape,geomestries.points_,objectId);
			return true;
	}
	return false;
}

bool
TeSTOSetBuildSHP(TeSTElementSet& stoset, const string& fileName)
{
	// Read some information about the shapefile
	string filePrefix = TeGetName(fileName.c_str());
	string shpfileName = filePrefix + ".shp";

	SHPHandle	hSHP;
	hSHP = SHPOpen( shpfileName.c_str(), "rb" );

	if( hSHP == 0 )
		return false;

	int		nShapeType, nEntities;
	double 	adfMinBound[4], adfMaxBound[4];

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

	string dbffileName = filePrefix + ".dbf";
  DBFHandle hDBF = DBFOpen( dbffileName.c_str(), "rb" );
  if( hDBF == 0  || DBFGetFieldCount(hDBF) == 0) {
    SHPClose( hSHP );
  
		return false;
	}
    
	int natt = DBFGetFieldCount(hDBF);
	TeAttributeList attList;
	TeReadDBFAttributeList(shpfileName, attList);

	TeAttributeRep repobjid;
	repobjid.name_ = "object_id";
	repobjid.numChar_ = 16;

	TeAttribute attobjid;
	attobjid.rep_ = repobjid;

	TeProperty propobjid;
	propobjid.attr_ = attobjid;
	
	TeAttributeList attrs; 
	attrs.push_back (attobjid);
	for(unsigned int j=0; j<attList.size(); ++j)
	  attrs.push_back(attList[j]);
	 
	stoset.setAttributeList(attrs); 	
	
	int i,n;
	SHPObject* psShape;
	for (i=0; i<nEntities; i++)
	{
		string objectid = Te2String(i);
		TeSTInstance curObj;
		curObj.objectId(objectid);

		vector<string> prop; //values of the attributes
		psShape = SHPReadObject(hSHP,i);
		if (TeDecodeShape(psShape,curObj.geometries(),objectid))
		{
			prop.push_back(objectid);
			for(n=0;n<natt;n++)
			{
				string value = DBFReadStringAttribute(hDBF,i,n);
				prop.push_back(value);
			}
			curObj.setProperties(prop);
			stoset.insertSTInstance(curObj);
			curObj.theme(0);
		}
		SHPDestroyObject(psShape);
	}
	
	DBFClose( hDBF );
	SHPClose( hSHP );
	
	return true;
} 
