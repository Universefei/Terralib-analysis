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

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace std;

// TerraLib

#include "TeDriverSHPDBF.h"
#include "TeTable.h"
#include "TeStdFile.h"
#include "TeException.h"
#include "TeUtils.h"
#include "TeGeometry.h"
#include "TeGeometryAlgorithms.h"
#include "TeLayer.h"
#include "TeProgress.h"
#include "TeDatabase.h"


// from SHAPELIB
#include "shapefil.h" 
 
// Internal functions
bool TeImportSHPGeometries (TeLayer* layer,const string& shpFileName, vector<string> &indexes, unsigned int chunckSize);
void TeSHPPointDecode    (TeLayer*, SHPHandle, vector<string>&, unsigned int);
void TeSHPPolyLineDecode (TeLayer*, SHPHandle, vector<string>&, unsigned int);
void TeSHPPolygonDecode  (TeLayer*, SHPHandle, vector<string>&, unsigned int);

TeLayer* TeImportShape(const string& shpFileName, TeDatabase* db, const string& layerName)
{
	if (!db || shpFileName.empty()) 
		return 0;

	// check if format is complete (SHP, SHX e DBF files exist)
	string filePrefix = TeGetName(shpFileName.c_str());
	string ftest = filePrefix + ".dbf";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".DBF";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}
	ftest = filePrefix + ".shx";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".SHX";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	// found a valid layer name
	string lName;
	string baseName = TeGetBaseName(shpFileName.c_str());
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

	bool res = TeImportShape(newLayer,shpFileName);
	if (res)
		return newLayer;
	else
	{
		db->deleteLayer(newLayer->id());
		delete newLayer;
		return 0;
	}
}

bool TeImportShape (TeLayer* layer, const string& shpFileName, string attrTableName,  
					string objectIdAtt, int unsigned chunkSize )
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}


	string filePrefix = TeGetName(shpFileName.c_str());
	string ftest = filePrefix + ".dbf";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".DBF";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}
	string dbfFileN = ftest;

	ftest = filePrefix + ".shx";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".SHX";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	ftest = filePrefix + ".shp";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".SHP";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}
	string shpFileN = ftest;

	SHPHandle	hSHP = SHPOpen(shpFileN.c_str(),"rb");
	if (hSHP == 0)
	{
		TePrecision::instance().setPrecision(oldPrec);
		return false;
	}

    	DBFHandle hDBF = DBFOpen(dbfFileN.c_str(), "rb");
    	if (hDBF == 0)
    	{
		TePrecision::instance().setPrecision(oldPrec);
		SHPClose(hSHP);
		return false;
    	}

	// Read some information about the shapefile
	int nShapeType, nEntities;
	// --- get the shape and decode it
	double 	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );
	string shpName = SHPTypeName( nShapeType );
	if (shpName == "UnknownShapeType")
	{
		TePrecision::instance().setPrecision(oldPrec);
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}

	if (attrTableName.empty())
	{
		if (layer->name().empty())
		{
			TePrecision::instance().setPrecision(oldPrec);
			return false;
		}
		else
			attrTableName = layer->name();
	}
	
	int linkCol=-1;
    	unsigned int j=0;
	bool autoIndex = false;
	int indexOffset = 0;  
	
	if (!layer->database()->beginTransaction())  
		return false;

	//read the attribute list information 
	TeAttributeList attList;
	TeReadDBFAttributeList(shpFileName, attList);

	// if no geometry link name is given, creates one called 'object_id_'
	string ext = "";
	int count = 0;
	if (objectIdAtt.empty())
	{
		objectIdAtt = "object_id_" + Te2String(layer->id());
		string s2 = TeConvertToUpperCase(objectIdAtt);
		while (true)
		{
			for (j=0; j<attList.size(); j++)
			{
				string s0 = attList[j].rep_.name_;
				string s1 = TeConvertToUpperCase(s0);
				if ( s1 == s2)
					break;
			}
			if (j < attList.size())
			{
				++count;
				objectIdAtt = "object_id_" + Te2String(count);
			}
			else
				break;
		}
		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 16;
		at.rep_.name_ = objectIdAtt;
		at.rep_.isPrimaryKey_ = true;
		attList.push_back(at);
		linkCol = attList.size()-1;
		autoIndex = true;
	}
	else
	{
		// check if given index is valid
		TeAttributeList::iterator it = attList.begin();
		while (it != attList.end())
		{
			if (TeConvertToUpperCase((*it).rep_.name_) == TeConvertToUpperCase(objectIdAtt))		// index found
			{
				if ((*it).rep_.type_ != TeSTRING)	// make sure it is a String type
				{
					(*it).rep_.type_ = TeSTRING;
					(*it).rep_.numChar_ = 16;
				}
				(*it).rep_.isPrimaryKey_ = true;
				linkCol=j;
				break;
			}
			++it;
			++j;
		}
		if (it == attList.end())			// index not found
		{
			objectIdAtt = "object_id_" + Te2String(layer->id());;
			while (true)
			{
				for (j=0; j<attList.size(); j++)
				{
					if (TeConvertToUpperCase(attList[j].rep_.name_) == TeConvertToUpperCase(objectIdAtt))
						break;
				}
				if (j < attList.size())
				{
					++count;
					objectIdAtt = "object_id_" + Te2String(count);
				}
				else
					break;
			}
			TeAttribute at;
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_ = 16;
			at.rep_.name_ = objectIdAtt;
			at.rep_.isPrimaryKey_ = true;
			attList.push_back(at);
			autoIndex = true;
		}
	}
	if (autoIndex)
	{
		linkCol = attList.size()-1;
		indexOffset = layer->getNewObjectId(); 
    	}

	TeTable attTable (attrTableName,attList,objectIdAtt,objectIdAtt,TeAttrStatic);
	if (!layer->createAttributeTable(attTable))			// create the table in the database
	{
		SHPClose(hSHP);  
		DBFClose(hDBF);  
		layer->database()->rollbackTransaction();  
		TePrecision::instance().setPrecision(oldPrec);
		return false;
	}
	
	string objid, value;
	TePointSet points;
	TeLineSet lines;
	TePolygonSet polygons;
	TeTableRow row;
	SHPObject* psShape;

	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	// OBS1: we discard Z and M values when available, because TerraLib
	// doesn�t have the concept of 3D or measured geometries
	// OBS2: non identified geometries are mapped to a object_id_
	// "te_nulo"
	switch (nShapeType) 
	{
		// --- null geometries ---
	  case SHPT_NULL:							// read only the attributes
		for (j=0; j<(unsigned int)nEntities; j++)
		{
		  row.clear();
		  TeDBFRowDecode(hDBF,j,row);
		  if (autoIndex)	// --- create automatic index if needed
			  row.push_back(Te2String(j));
		  if (row[linkCol].empty())
			 row[linkCol] = "te_nulo";
		  objid = row[linkCol];
		  attTable.add(row);
		  if ((j%chunkSize) == 0)
		  {
			  layer->saveAttributeTable(attTable);
			  attTable.clear();
			  if(TeProgress::instance())
			  {
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(j+1);
			  }	
		  }
		}
		break;
		// --- point geometries ---
	  case SHPT_POINTZ:	
	  case SHPT_MULTIPOINTZ:
	  case SHPT_POINTM:
	  case SHPT_MULTIPOINTM:
	  case SHPT_POINT:
	  case SHPT_MULTIPOINT:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();
			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
				  row.push_back(Te2String(j));
			  if (row[linkCol].empty())
				 row[linkCol] = "te_nulo";
			  objid = row[linkCol];
			  attTable.add(row);
			  psShape = SHPReadObject(hSHP,j);
              TeSHPPointDecode(psShape,points,objid);
     		  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {
				  layer->addPoints(points);
				  points.clear();
				  layer->saveAttributeTable(attTable);
				  attTable.clear();
				  if(TeProgress::instance())
				  {
					if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }	
			  }
		  }
		  break;
		// --- line geometries ---
	  case SHPT_ARCZ:
	  case SHPT_ARCM:
	  case SHPT_ARC:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();
			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
				  row.push_back(Te2String(j));
			  if (row[linkCol].empty())
				 row[linkCol] = "te_nulo";
			  objid = row[linkCol];
			  attTable.add(row);
			  psShape = SHPReadObject(hSHP,j);
			  TeSHPPolyLineDecode(psShape,lines,objid);
     		  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {
			 	  layer->addLines(lines);
				  lines.clear();
				  layer->saveAttributeTable(attTable);
				  attTable.clear();
				  if(TeProgress::instance())
				  {
					if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }	
			  }
		  }
		  break;
	// --- polygon geometries ---
	  case SHPT_POLYGON:
	  case SHPT_POLYGONM:
	  case SHPT_POLYGONZ:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();
			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
				  row.push_back(Te2String(j));
			  if (row[linkCol].empty())
				 row[linkCol] = "te_nulo";
			  objid = row[linkCol];
			  attTable.add(row);
			  psShape = SHPReadObject(hSHP,j);
			  TeSHPPolygonDecode(psShape,polygons,objid);
			  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {
				  layer->addPolygons(polygons);
				  polygons.clear();
				  layer->saveAttributeTable(attTable);
				  attTable.clear();
				  if(TeProgress::instance())
				  {
					if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }
			  }

		  }
		  break;
	  default:
		  /* todo: handle multipatch geometries */
		  throw TeException ( UNHANDLED_SHP_TYPE );
	}
	if (points.size() > 0)
	{
		layer->addPoints(points);
		points.clear();
	}
	if (lines.size() > 0)
	{
		layer->addLines(lines);
		lines.clear();
	}
	if (polygons.size() > 0)
	{
		layer->addPolygons(polygons);
		polygons.clear();
	}
	if (attTable.size() > 0)
	{
		layer->saveAttributeTable(attTable);
		attTable.clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	SHPClose(hSHP);
	DBFClose(hDBF);
	
	if (!layer->database()->commitTransaction())  
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

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

bool
TeImportSHPGeometries(TeLayer* layer,const string& shpFileName, 
					  vector<string> &indexes, unsigned int chunckSize)
{

	string filePrefix = TeGetName(shpFileName.c_str());
	string fileName = filePrefix + ".shp";
	//Open the        shapefile.                                      
	SHPHandle	hSHP;
	hSHP = SHPOpen( fileName.c_str(), "rb" );

	if( hSHP == 0 )
		return false;


//    Get information about the file              
	int		nShapeType, nEntities;
	double 	adfMinBound[4], adfMaxBound[4];

	SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

//	choose the appropriate decoding function

	string name  = SHPTypeName( nShapeType );

	if ( name == "Arc" || name == "ArcZ" )
	{
		TeSHPPolyLineDecode ( layer, hSHP,indexes,chunckSize );
	}
	else if ( name == "Point" || name == "PointZ" ) 
	{
		TeSHPPointDecode ( layer, hSHP,indexes,chunckSize );
    }
	else if ( name == "Polygon" )
	{
		TeSHPPolygonDecode ( layer, hSHP,indexes,chunckSize );
	}
#if 0  // todo...
	else if ( name ==  "MultiPoint" ) {}
	else if ( name ==  "PolyLineZ") {}
	else if ( name ==  "PolygonZ")  {}
	else if ( name ==  "MultiPointZ") {}
	else if ( name ==  "PointM"  ) {}
	else if ( name ==  "PolyLineM" ) {}
	else if ( name ==  "PolygonM") {}
	else if ( name ==  "MultiPointM") {}
#endif
	else
		throw TeException ( UNHANDLED_SHP_TYPE );

    SHPClose( hSHP );
	return true;


}

bool TeImportShapeGeometry (TeLayer* layer, const string& shpFileName, string attrTableName,  
					string objectIdAtt, int unsigned chunkSize )
{

	string filePrefix = TeGetName(shpFileName.c_str());
	string shpFileN = filePrefix + ".shp";
	string dbfFileN = filePrefix + ".dbf";

	SHPHandle	hSHP = SHPOpen(shpFileN.c_str(),"rb");
	if (hSHP == 0)
		return false;

    DBFHandle hDBF = DBFOpen(dbfFileN.c_str(), "rb");
    if (hDBF == 0)
    {
		SHPClose(hSHP);
		return false;
    }

	// Read some information about the shapefile
	int nShapeType, nEntities;
	// --- get the shape and decode it
	double 	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );
	string shpName = SHPTypeName( nShapeType );
	if (shpName == "UnknownShapeType")
	{
		SHPClose(hSHP);
		return false;
	}

	if (attrTableName.empty())
	{
		if (layer->name().empty())
			return false;
		else
			attrTableName = layer->name();
	}
	
	int linkCol=-1;
    unsigned int j=0;
	bool autoIndex = false;
    int indexOffset = 0;

	if (!layer->database()->beginTransaction())
		return false;

	//read the attribute list information 
	TeAttributeList attList;
  TeReadDBFAttributeList(shpFileName, attList);

	// if no geometry link name is given, creates one called 'object_id_'
	string ext = "";
	int count = 0;
	if (objectIdAtt.empty())
	{
		objectIdAtt = "object_id_" + Te2String(layer->id());
		string s2 = TeConvertToUpperCase(objectIdAtt);
		while (true)
		{
			for (j=0; j<attList.size(); j++)
			{
				string s0 = attList[j].rep_.name_;
				string s1 = TeConvertToUpperCase(s0);
				if ( s1 == s2)
					break;
			}
			if (j < attList.size())
			{
				++count;
				objectIdAtt = "object_id_" + Te2String(count);
			}
			else
				break;
		}
		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 16;
		at.rep_.name_ = objectIdAtt;
		at.rep_.isPrimaryKey_ = true;
		attList.push_back(at);
		linkCol = attList.size()-1;
		autoIndex = true;
	}
	else
	{
		// check if given index is valid
		TeAttributeList::iterator it = attList.begin();
		while (it != attList.end())
		{
			if (TeConvertToUpperCase((*it).rep_.name_) == TeConvertToUpperCase(objectIdAtt))		// index found
			{
				if ((*it).rep_.type_ != TeSTRING)	// make sure it is a String type
				{
					(*it).rep_.type_ = TeSTRING;
					(*it).rep_.numChar_ = 16;
				}
				(*it).rep_.isPrimaryKey_ = true;
				linkCol=j;
				break;
			}
			++it;
			++j;
		}
		if (it == attList.end())			// index not found
		{
			objectIdAtt = "object_id_" + Te2String(layer->id());;
			while (true)
			{
				for (j=0; j<attList.size(); j++)
				{
					if (TeConvertToUpperCase(attList[j].rep_.name_) == TeConvertToUpperCase(objectIdAtt))
						break;
				}
				if (j < attList.size())
				{
					++count;
					objectIdAtt = "object_id_" + Te2String(count);
				}
				else
					break;
			}
			TeAttribute at;
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_ = 16;
			at.rep_.name_ = objectIdAtt;
			at.rep_.isPrimaryKey_ = true;
			attList.push_back(at);
			autoIndex = true;
		}
	}
	if (autoIndex)
    {
		linkCol = attList.size()-1;
		indexOffset = layer->getNewObjectId();
    }

	TeTable attTable (attrTableName,attList,objectIdAtt,objectIdAtt,TeAttrStatic);

	string objid, value;
	TePointSet points;
	TeLineSet lines;
	TePolygonSet polygons;
	TeTableRow row;
	SHPObject* psShape;

	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	// OBS1: we discard Z and M values when available, because TerraLib
	// doesn�t have the concept of 3D or measured geometries
	// OBS2: non identified geometries are mapped to a object_id_
	// "te_nulo"
	switch (nShapeType) 
	{
		// --- null geometries ---
	  case SHPT_NULL:							// read only the attributes
		for (j=0; j<(unsigned int)nEntities; j++)
		{
		  row.clear();
		  TeDBFRowDecode(hDBF,j,row);
		  if (autoIndex)	// --- create automatic index if needed
			  row.push_back(Te2String(j + indexOffset));
		  if (row[linkCol].empty())
			 row[linkCol] = "te_nulo";
		  objid = row[linkCol];

		  if ((j%chunkSize) == 0)
		  {
			  if(TeProgress::instance())
			  {
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(j+1);
			  }	
		  }
		}
		break;
		// --- point geometries ---
	  case SHPT_POINTZ:	
	  case SHPT_MULTIPOINTZ:
	  case SHPT_POINTM:
	  case SHPT_MULTIPOINTM:
	  case SHPT_POINT:
	  case SHPT_MULTIPOINT:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();
			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
				  row.push_back(Te2String(j + indexOffset));
			  if (row[linkCol].empty())
				 row[linkCol] = "te_nulo";
			  objid = row[linkCol];

			  psShape = SHPReadObject(hSHP,j);

			  TeSHPPointDecode(psShape,points,objid);
     		  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {
				  if (!layer->addPoints(points))
				  {
					  SHPClose(hSHP);
					  layer->database()->rollbackTransaction(); 
					  return false;
				  }
				  points.clear();

				  if(TeProgress::instance())
				  {
					  if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }	
			  }
		  }
		  break;
		// --- line geometries ---
	  case SHPT_ARCZ:
	  case SHPT_ARCM:
	  case SHPT_ARC:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();
			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
				  row.push_back(Te2String(j + indexOffset));
			  if (row[linkCol].empty())
				 row[linkCol] = "te_nulo";
			  objid = row[linkCol];

			  psShape = SHPReadObject(hSHP,j);

			  TeSHPPolyLineDecode(psShape,lines,objid);
		  
			  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {
				  if (!layer->addLines(lines))
				  {
					  SHPClose(hSHP);
					  layer->database()->rollbackTransaction(); 
					  return false;
				  }
				  lines.clear();

				  if(TeProgress::instance())
				  {
					if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }	
			  }
		  }
		  break;
	// --- polygon geometries ---
	  case SHPT_POLYGON:
	  case SHPT_POLYGONM:
	  case SHPT_POLYGONZ:

		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  row.clear();

			  TeDBFRowDecode(hDBF,j,row);
			  if (autoIndex)	// --- create automatic index if needed
			    row.push_back(Te2String(j + indexOffset));
			  if (row[linkCol].empty())
				  row[linkCol] = "te_nulo";
			  objid = row[linkCol];
			  
			  psShape = SHPReadObject(hSHP,j);

			  TeSHPPolygonDecode(psShape,polygons,objid);

			  SHPDestroyObject(psShape);
			  if ((j%chunkSize) == 0)
			  {

				  if (!layer->addPolygons(polygons))
				  {
					  SHPClose(hSHP);
					  layer->database()->rollbackTransaction(); 
					  return false;
				  }

				  polygons.clear();

				  if(TeProgress::instance())
				  {
					if (TeProgress::instance()->wasCancelled())
						break;
					else
						TeProgress::instance()->setProgress(j+1);
				  }
			  }

		  }
		  break;
	  default:
		  // todo: handle multipatch geometries 
		  throw TeException ( UNHANDLED_SHP_TYPE );
	}
	if (points.size() > 0)
	{
		if (!layer->addPoints(points))
		{
			SHPClose(hSHP);
			layer->database()->rollbackTransaction(); 
			return false;
		}

		points.clear();
	}
	if (lines.size() > 0)
	{
		if (!layer->addLines(lines))
		{
			SHPClose(hSHP);
			layer->database()->rollbackTransaction();
			return false;
		}

		lines.clear();
	}
	if (polygons.size() > 0)
	{
		if (!layer->addPolygons(polygons))
		{
			SHPClose(hSHP);
			layer->database()->rollbackTransaction();
			return false;
		}
		polygons.clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	SHPClose(hSHP);

	if (!layer->database()->commitTransaction())
		return false;

	return true;
}


void TeDBFRowDecode(DBFHandle& hDBF, int nr, TeTableRow& row)
{
	string value;
	int i;
	for (i = 0; i < DBFGetFieldCount(hDBF); i++)
	{
		value =  DBFReadStringAttribute(hDBF,nr,i);
		row.push_back(value); 
	}
}

void TeSHPPointDecode(SHPObject* psShape, TePointSet& points, string& objid)
{
	int nv = psShape->nVertices;
	for (int i = 0; i<nv; i++)
	{
		TePoint point(psShape->padfX[i], psShape->padfY[i]);
		point.objectId(objid);
		points.add(point);
	}
}


void TeSHPPolyLineDecode(SHPObject* psShape, TeLineSet& lines, string& objid)
{
	int iPart,j=0,nextStart=0; 
	for (iPart = 0; iPart < psShape->nParts; iPart++)
	{
		if (iPart == psShape->nParts-1)
			nextStart = psShape->nVertices;
		else 
			nextStart = psShape->panPartStart[iPart+1];

		TeLine2D line;
		while (j<nextStart)
		{
			TeCoord2D pt ( psShape->padfX[j], psShape->padfY[j] );
			line.add ( pt );
			j++;
		}
		line.objectId (objid);	
		lines.add ( line );
	}
}

void
TeSHPPolygonDecode(SHPObject* psShape, TePolygonSet& polys, string& objid)
{
	int iPart = 0;
	vector<int> partStart;

	// Build an array whose components point to the starting point
	// of the rings that compose the polygon shape
	for ( iPart = 0; iPart < psShape->nParts; iPart++ )
		partStart.push_back ( psShape->panPartStart[iPart] );
	partStart.push_back ( psShape->nVertices ); // point to the end

	vector<TePolygon> pList;
	vector<TeLinearRing> holes;
	int j = 0;
	iPart = 0;
	while (j < psShape->nVertices)
	{
		TeLine2D line;
		iPart++;						// while it doesn�t reach the start of next part
		while ( j < partStart[iPart] )	// build a line from the coordinates
		{
			TeCoord2D pt ( psShape->padfX[j], psShape->padfY[j] );
			line.add ( pt );
			j++;
		}								
		if (!line.isRing())				// force the closing of opened rings
			line.add(line[0]);

		// decide if a ring is an outter ring or an inner ring based on its orientation
		TeLinearRing ring(line);
		short orient = TeOrientation(ring);
		if(orient == TeCLOCKWISE)	// outter ring: start a new polygon
		{
			TePolygon p;
			p.add(ring);
			p.objectId(objid);
			pList.push_back(p);
		}
		else	// COUNTERCLOCKWISE => inner ring: put on the list of holes
			holes.push_back(ring);
	} // read all vertices


	// even though there are rings that are in counter clockwise orientation, 
	// there aren't any clockwise orientation rings to be possible parents of 
	// these inner rings. The decision here: treat all of them as outter rings...
	if (pList.empty() && !holes.empty())
	{
		for (unsigned int i=0; i<holes.size(); ++i)
		{
			TePolygon p;
			p.add(holes[i]);
			p.objectId(objid);
			polys.add(p);
		}
		return;
	}

	if (pList.size() == 1)	// shape has only one polygon that should
	{						// contain all the inner rings
		for(unsigned int i = 0; i < holes.size(); ++i)
			pList[0].add(holes[i]);
	}
	else
	{	// shape has multiple outter ring and inner rings
		// we have to find a parent to each inner ring 
		for(unsigned int i = 0; i < holes.size(); ++i)
		{
			vector<TePolygon> candidates;
			TeLinearRing ring = holes[i];
			
			// step 1: consider as a inner ring parent candidate every polygon 
			// that intercepts the ring MBR
			unsigned int j = 0;
			for(j = 0; j < pList.size(); ++j)
			{
				if(TeWithinOrCoveredByOrEquals(ring.box(), pList[j].box()))
					candidates.push_back(pList[j]);
			}

			// if there is only one candidate this is the parent
			if(candidates.size() == 1)
			{
				candidates[0].add(ring);
				continue;
			}

			// step 2: refine the candidates to parent: consider only those 
			// that contains the ring (not just its MBR)
			vector<TePolygon> newCandidates;
			for(j = 0; j < candidates.size(); ++j)
			{
				short rel = TeBOUNDARY;
				bool inside = false;

				unsigned int nthVert = ring.size();
				unsigned int iVert = 0u;

				while(iVert < nthVert)
				{
					rel = TeRelation(ring[iVert], candidates[j][0]);

					if(rel & TeINSIDE)
					{
						inside = true;
						newCandidates.push_back(candidates[j]);
						break;
					}
					else if(rel & TeOUTSIDE)
						break;
					++iVert;
				}

				if(iVert == nthVert)	
					break;
			}
			
			// sort the new candidates according to their areas
			int idxMinArea = 0;
			double minArea = TeMAXFLOAT;
			for(j = 0; j < newCandidates.size(); ++j)
			{
				if(TeGeometryArea(newCandidates[j].box()) < minArea)
				{
					idxMinArea = j;
					minArea = TeGeometryArea(newCandidates[j].box());
				}
			}
			// choose as parent the candidate that has the smaller area
			if (!newCandidates.empty())
				newCandidates[idxMinArea].add(ring);
		}
	}

	// add polygons to the list passed as parameter
	vector<TePolygon>::iterator it = pList.begin();
	while (it != pList.end())
	{
		polys.add(*it);
		++it;
	}
}

void
TeSHPPointDecode( TeLayer* layer, SHPHandle hSHP, vector<string>& indexVect, unsigned int chunckSize  )
{
	int nEntities = hSHP->nRecords; // indicates how many point sets are there
	TePointSet points;

/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, reading all the vertices.	            */
/* -------------------------------------------------------------------- */
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(nEntities);

    int j;
	for(j= 0; j < nEntities; j++ )
    {
        SHPObject* psShape = SHPReadObject( hSHP, j );
		
		int i;
		for (i = 0; i < ( psShape->nVertices ); i++ )
		{
			TePoint point;
			TeCoord2D xy ( psShape->padfX[i], psShape->padfY[i] );
			point.add ( xy );
			point.objectId ( indexVect[j] );
			points.add ( point );
		}
		// check if there is a chunk of points to be inserted into the database
		if ( points.size() >= chunckSize )
		{
			layer->addPoints( points ); 
			points.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
						break;
				else
					TeProgress::instance()->setProgress(j+1);
			}
		}	
		SHPDestroyObject( psShape );
	}
	layer->addPoints( points ); 
	points.clear();
	if(TeProgress::instance())
		TeProgress::instance()->setProgress(j+1);
}

void
TeSHPPolyLineDecode ( TeLayer*layer, SHPHandle hSHP, vector<string>& indexVect, unsigned int chunckSize )
{
	int nEntities = hSHP->nRecords; // indicates how many regions are there
	TeLineSet lines;

/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, reading all the vertices.	            */
/* -------------------------------------------------------------------- */
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(nEntities);

	int i;
    for(i = 0; i < nEntities; i++ )
    {
        SHPObject* psShape = SHPReadObject( hSHP, i );

		// Build an array whose components point to the starting point
		// of the rings that compose the shapepoly
		int iPart;
		vector<int> partStart;

		// Build an array whose components point to the starting point
		// of the segments that compose the shapeline
		//
		for ( iPart = 0; iPart < psShape->nParts; iPart++ )
		{
			partStart.push_back ( psShape->panPartStart[iPart] );
		}
		partStart.push_back ( psShape->nVertices ); // point to the end

		iPart = 0;

		int j = 0; 

		while ( j < psShape->nVertices )
		{
			TeLine2D line;

			iPart++;  // indicates the different segments of the line
			// Read each segment
			while ( j < partStart[iPart] )
			{
				TeCoord2D pt ( psShape->padfX[j], psShape->padfY[j] );
				line.add ( pt );
				j++;
			}
			// set the geometry Id for the line 
			line.objectId ( indexVect[i] ); // set the object index
			
			lines.add ( line );
		}
		
		// check if there is a chunk of lines to be inserted into the database
		if ( lines.size() >= chunckSize )
		{
			layer->addLines( lines ); 
			lines.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(i+1);
			}
		}		
		SHPDestroyObject( psShape );
	}
	
	// saves the remaining lines into the database
	layer->addLines( lines ); 
	lines.clear();
	if(TeProgress::instance())
		TeProgress::instance()->setProgress(i+1);
}

void
TeSHPPolygonDecode ( TeLayer* layer, SHPHandle hSHP, vector<string>& indexVect, unsigned int chunckSize )
{

	int nEntities = hSHP->nRecords; // indicates how many regions are there
	TePolygonSet polys;

/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, reading all the vertices.	            */
/* -------------------------------------------------------------------- */
	if(TeProgress::instance())
		TeProgress::instance()->setMessage("Importando poligonos, aguarde...      ");

	int i;
    for(i = 0; i < nEntities; i++ )
    {
		SHPObject* psShape = SHPReadObject( hSHP, i );

		int iPart;
		vector<int> partStart;

		// Auxiliary variables

		list<TePolygon> pList;

		// Build an array whose components point to the starting point
		// of the rings that compose the shapepoly
		//
		for ( iPart = 0; iPart < psShape->nParts; iPart++ )
		{
			partStart.push_back ( psShape->panPartStart[iPart] );
		}
		partStart.push_back ( psShape->nVertices ); // point to the end

		iPart = 0;

		int j = 0;

		while ( j < psShape->nVertices )
		{
			iPart++;  // indicates the different rings of the shapepoly
			TeLine2D line;

			// Read each ring
			while ( j < partStart[iPart] )
			{
				TeCoord2D pt ( psShape->padfX[j], psShape->padfY[j] );
				line.add ( pt );
				j++;
			}
			// end of a ring

			if (!line.isRing())
			{
				line.add(line[0]);
//				throw TeException ( POLYSHAPE_IS_NOT_RING );
			}

			TeLinearRing ring ( line );

			// Is it an OUTER or an INNER RING ?

			bool inside = false;

			list<TePolygon>::iterator it = pList.begin();


			// Each ring is assigned to a polygon
			// Each ring and polygon have a unique "geometrical index"
			//
			// All rings and polygons that are part of a same object
			// have the same "object index"

			while ( it != pList.end() )
			{
				TePolygon aux;
				aux.add(ring);
				if ( TeWithin ( aux, (*it)))
				{
					inside = true;
					ring.objectId ( (*it)[0].objectId() ); // sets the object index
					(*it).add ( ring );// add a closed region
					break;
				}
				++it;
			}
			if ( ! inside )
			{
				TePolygon poly;
				ring.objectId (indexVect[i]);
				
				// add a pair (objectId, geometryId) to the layer
				poly.add ( ring ); // add an outer region
				poly.objectId ( indexVect[i] ); // set the object index

				pList.push_back ( poly );
			}
		} // read all vertices

		list<TePolygon>::iterator it = pList.begin();

		while ( it != pList.end() )
		{
			polys.add(*it);
			++it;
		}
		if ( polys.size() >= chunckSize )
		{
			layer->addPolygons( polys ); 
			polys.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
						break;
				else
					TeProgress::instance()->setProgress(i+1);
			}
		}
		SHPDestroyObject( psShape );
	} // read all entities

	// save the remaining polygons
	layer->addPolygons( polys ); 
	polys.clear();
	if(TeProgress::instance())
		TeProgress::instance()->setProgress(i+1);
}

