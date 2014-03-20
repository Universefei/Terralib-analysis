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

// STL 
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <map>

using namespace std;

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

// TerraLib

#include "TeDriverSPRING.h"

#include "../kernel/TeAsciiFile.h"
#include "../kernel/TeBox.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeException.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeDatabase.h"

//! Import only the attributes of a GEO/TAB data to a layer TerraLib
/*!
	\param layer pointer to a layer
	\param geoFileName GEO/TAB file name
	\param attrTableName the name that the attribute table will be saved
	\param objectIdAttr name of the attribute that has the identification of objects
	\param chunckSize the number of objects in memory before save them in the layer
*/
bool TeImportGeoAttributes(TeLayer* layer, const string& geoFileName, string attrTableName="", unsigned int chunckSize=60);

//! Import only the geometries of a GEO/TAB data to a layer TerraLib
/*!
	\param layer pointer to a layer
	\param geoFileName GEO/TAB file name
	\param chunckSize the number of objects in memory before save them in the layer
*/
bool TeImportGeoGeometries(TeLayer* layer, const string& geoFileName, unsigned int chunckSize=60);

TeLayer* 
TeImportGEO(const string& geoFileName, TeDatabase* db, const string& layerName)
{
	if (!db || geoFileName.empty()) 
		return 0;

	// check if format is complete (GEO e TAB files exist)
	string filePrefix = TeGetName(geoFileName.c_str());
	string ftest = filePrefix + ".geo";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".GEO";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}
	ftest = filePrefix + ".tab";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".TAB";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	// found a valid layer name
	string lName;
	string baseName = TeGetBaseName(geoFileName.c_str());
	if (layerName.empty())
		lName = baseName;
	else
		lName = layerName;

	string newLayerName = lName;
	TeLayerMap& layerMap = db->layerMap();
	TeLayerMap::iterator it;
	bool flag = true;
	int n = 1;
	while (flag)
	{
		for (it = layerMap.begin(); it != layerMap.end(); ++it)
		{
			if (TeStringCompare(it->second->name(),newLayerName))
				break;
		}
		if (it == layerMap.end())
			flag = 0;
		else
			newLayerName = lName + "_" +Te2String(n);
		n++;	
	}

	// find projection
	TeLayer* newLayer = new TeLayer(newLayerName,db);
	if (newLayer->id() <= 0 )
		return 0;				// couldn�t create new layer

	bool res = TeImportGEO(newLayer,geoFileName);
	if (res)
		return newLayer;
	else
	{
		db->deleteLayer(newLayer->id());
		delete newLayer;
		return 0;
	}
}

bool 
TeImportGEO(TeLayer* layer, const string& geoFileName, string attrTableName, unsigned int chunckSize)
{
	// Import the attribute table in chuncks
	if (!TeImportGeoAttributes (layer,geoFileName,attrTableName,chunckSize))
		return false;

	//Import the geometries
	if (!TeImportGeoGeometries (layer,geoFileName,chunckSize))
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TePOINTS)
	{
		layer->database()->insertMetadata(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), 0.0005,0.0005,layer->box());
		layer->database()->createSpatialIndex(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), (TeSpatialIndexType)TeRTREE);
	}
	if (rep & TeLINES)
	{
		layer->database()->insertMetadata(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), 0.0005,0.0005,layer->box());
		layer->database()->createSpatialIndex(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), (TeSpatialIndexType)TeRTREE);
	}
	if (rep & TePOLYGONS)
	{
		layer->database()->insertMetadata(layer->tableName(TePOLYGONS),layer->database()->getSpatialIdxColumn(TePOLYGONS), 0.0005,0.0005,layer->box());
		layer->database()->createSpatialIndex(layer->tableName(TePOLYGONS),layer->database()->getSpatialIdxColumn(TePOLYGONS), (TeSpatialIndexType)TeRTREE);
	}

	return true;
}

void
TeReadGeoAttributeList(const string& geoFileName,TeAttributeList& attList, string& labelName )
{
	string filePrefix = TeGetName (geoFileName.c_str()) ;
	string fileName = filePrefix + ".tab";
	try
	{
		TeAsciiFile	pFile (fileName);
		string name ;

		name = pFile.readString();	// "LABEL"
		pFile.findNewLine();
		name = pFile.readString();	// Spring category
		labelName = pFile.readString();	// label name

		while ( pFile.isNotAtEOF() && name != "SEPARATOR" )
			name = pFile.readString ();

		char separator = ',';
		if ( name == "SEPARATOR" )
		{
			separator = pFile.readString ().c_str()[0];
		}
		pFile.findNewLine();

		TeAttribute attribute;
		while (pFile.isNotAtEOF())
		{
			name = pFile.readStringCSVNoSpace (separator);
			if ( name == "INFO_END" )
				break;

			attribute.rep_.name_ = name;

			name = pFile.readStringCSVNoSpace (separator);
			if (name == "TEXT")
				attribute.rep_.type_ = TeSTRING;
			else if (name == "INTEGER")
				attribute.rep_.type_ = TeINT;
			else if (name == "TeDATETIME")
					attribute.rep_.type_  = TeDATETIME;
			else if (name == "REAL")
				attribute.rep_.type_  = TeREAL;
			else
				attribute.rep_.type_  = TeUNKNOWN;

			int size = pFile.readIntCSV (separator);
			attribute.rep_.numChar_ = size;

			attList.push_back ( attribute );
			pFile.findNewLine();
		}
	}
	catch(...)
	{
		return;
	}
}

bool 
TeImportGeoAttributes(TeLayer* layer,const string& geoFileName, string attrTableName, unsigned  int chunckSize)
{
	string filePrefix = TeGetName (geoFileName.c_str()) ;
	string fileName = filePrefix + ".tab";

	// Read attribute list information and key field name
	TeAttributeList attList;
	string linkName;
	TeReadGeoAttributeList(geoFileName,attList,linkName);

	if (attrTableName.empty())
	{
		if (layer->name().empty())
			return false;
		else
			attrTableName = layer->name();
	}

	// check if given index is valid
	TeAttributeList::iterator it = attList.begin();
	int i = -1, j=0;
	while (it != attList.end())
	{
		if (TeConvertToUpperCase((*it).rep_.name_) == TeConvertToUpperCase(linkName))		// index found
		{
			if ((*it).rep_.type_ != TeSTRING)	// make sure it is a String type
			{
				(*it).rep_.type_ = TeSTRING;
				(*it).rep_.numChar_ = 16;
			}
			(*it).rep_.isPrimaryKey_ = true;
			i=j;
			break;
		}
		++it;
		j++;
	}
	if (it == attList.end())
		return false;

	TeAsciiFile	pFile (fileName);
	string name = pFile.readString ();
	pFile.findNewLine();

	if (name != "TABLE")
		return false;

	TeTable attTable (attrTableName,attList,linkName,linkName,TeAttrStatic);
	if (!layer->createAttributeTable(attTable))
		return false;


	int nAtt = attList.size();

	// Identify the separator again
	while ( pFile.isNotAtEOF() && name != "SEPARATOR" )
		name = pFile.readString ();

	char separator = ',';
	if ( name == "SEPARATOR" )
	{
		separator = pFile.readString ().c_str()[0];
	}
	pFile.findNewLine();

	// Skip the information lines
	while ( pFile.isNotAtEOF() && name != "INFO_END" )
		name = pFile.readString ();

	if (name != "INFO_END")
		return false;

	pFile.findNewLine();
	string value = pFile.readStringCSVNoQuote(separator);
	try {
		// Read the attribute lines
		while (pFile.isNotAtEOF())
		{
			TeTableRow row;
			for (int n=0; n<nAtt; n++)
			{
				row.push_back ( value ); 
				if(pFile.isNotAtEOF())
					value = pFile.readStringCSVNoQuote(separator);
			}
			attTable.add( row );

			if ( attTable.size() == chunckSize )
			{
				layer->saveAttributeTable( attTable );
				attTable.clear();
			}
			pFile.findNewLine();
			value = pFile.readStringCSVNoQuote(separator);
		}
	}
	catch (...)
	{
//		int a = 0;  end of file reached
	}
	// save the remaining rows
	if (attTable.size() > 0 )
		layer->saveAttributeTable( attTable );
	attTable.clear();
	return true;
}

bool 
TeImportGeoGeometries ( TeLayer* layer, const string& geoFileName, unsigned int chunckSize)
{
	string filePrefix = TeGetName (geoFileName.c_str()) ;
	string fileName = filePrefix + ".geo";

	TePointSet pointSet;
	TeLineSet lineSet;
	TePolygonSet polySet;
	TeTextSet textSet;


/* -------------------------------------------------------------------- */
/*      Open the  file.                                                 */
/* -------------------------------------------------------------------- */

	TeAsciiFile	pFile (fileName);

	int size,nrings;
	string geoId;

/* -------------------------------------------------------------------- */
/*      Get information about the file                                  */
/* -------------------------------------------------------------------- */
  try
  {
	while (pFile.isNotAtEOF())
	{
		string repType = pFile.readString ();
		pFile.findNewLine();
		if (repType == "POLIGONOS")
		{
			while ( 1 )
			{
				TePolygon poly;
				string rings = pFile.readString ();
				if (rings == "END" )
					break;
				nrings = atoi(rings.c_str());
				if (!pFile.isNotAtEOF())
					break;
				geoId = pFile.readString ();
				pFile.findNewLine();
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
					poly.add ( ring );
				}
				polySet.add ( poly );
				if ( polySet.size() == chunckSize )
				{
					layer->addPolygons( polySet ); 
					polySet.clear();
				}
			}
		}
		if (repType == "LINHAS")
		{
			while ( 1 )
			{
				geoId = pFile.readStringCSVNoQuote (',');
				pFile.findNewLine();
				if (!pFile.isNotAtEOF() || geoId == "END")
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
				lineSet.add ( line );
				if ( lineSet.size() == chunckSize )
				{
					layer->addLines( lineSet ); 
					lineSet.clear();
				}
				pFile.findNewLine();
			}
		}
		if (repType == "PONTOS")
		{
			while ( 1 )
			{
				geoId = pFile.readString ();
				if (!pFile.isNotAtEOF() || geoId == "END" )
					break;
				TePoint point;
	// Build the point
				TeCoord2D xy = pFile.readCoord2D ();
				point.add ( xy );
				point.objectId ( geoId );
				pointSet.add ( point );
				if ( pointSet.size() == chunckSize )
				{
					layer->addPoints( pointSet ); 
					pointSet.clear();
				}
				pFile.findNewLine();
			}
			if (repType.empty())
				break;
		}
	}
  }
  catch(...)
  {
//	  int a = 0;
  }
	// save the remaining geometries
	if (pointSet.size() > 0)
	{
		layer->addPoints( pointSet ); 
		pointSet.clear();
	}

	if (lineSet.size() > 0 )
	{
		layer->addLines( lineSet ); 
		lineSet.clear();
	}

	if (polySet.size() > 0)
	{
		layer->addPolygons( polySet ); 
		polySet.clear();
	}
	return true;
}
