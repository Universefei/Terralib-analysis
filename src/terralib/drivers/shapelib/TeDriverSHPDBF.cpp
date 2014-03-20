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

#include <assert.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

const int MAXCHARNUM = 66540;

// TerraLib

#include "TeDriverSHPDBF.h"
#include "../../kernel/TeTable.h"
#include "../../kernel/TeException.h"
#include "../../kernel/TeUtils.h"
#include "../../kernel/TeGeometry.h"
#include "../../kernel/TeGeometryAlgorithms.h"
#include "../../kernel/TeLayer.h"
#include "../../kernel/TeProgress.h"
#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeAsciiFile.h"

void TeWriteDataProjectionToFile(TeProjection* projection, const std::string& fileName)
{
	string filePrefix = TeGetName(fileName.c_str());
	string prjFileName_ = filePrefix + ".prj";

	std::string wktProj = TeGetWKTFromTeProjection(projection);

	TeWriteToFile(prjFileName_, wktProj, "w");
}

bool
TeGetSHPInfo(std::string shpfileName, unsigned int& nShapes, TeBox& box, TeGeomRep& rep)
{
	nShapes = 0;
	rep = TeGEOMETRYNONE;
	box = TeBox();

	SHPHandle	hSHP;
	hSHP = SHPOpen( shpfileName.c_str(), "rb" );

	if( hSHP == 0 )
		return false;

	int		nShapeType, nentities=0;
	double 	adfMinBound[4], adfMaxBound[4];

	SHPGetInfo(hSHP, &nentities, &nShapeType, adfMinBound, adfMaxBound);
	nShapes = nentities;
	
	box.x1_ = adfMinBound[0];
	box.y1_ = adfMinBound[1];
	box.x2_ = adfMaxBound[0];
	box.y2_ = adfMaxBound[1];

	switch (nShapeType) 
	{
		case SHPT_NULL:
			rep = TeGEOMETRYNONE;
		break;
		// --- point geometries ---
		case SHPT_POINTZ:	
		case SHPT_MULTIPOINTZ:
		case SHPT_POINTM:
		case SHPT_MULTIPOINTM:
		case SHPT_POINT:
		case SHPT_MULTIPOINT:
			rep = TePOINTS;
		break;
		// --- line geometries ---
		case SHPT_ARCZ:
		case SHPT_ARCM:
		case SHPT_ARC:
			rep = TeLINES;
		break;
		// --- polygon geometries ---
		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:
			rep = TePOLYGONS;
	}
	SHPClose(hSHP);
	return true;
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
    	if( hDBF == 0  || DBFGetFieldCount(hDBF) == 0)
		return false;
    
	int natt = DBFGetFieldCount(hDBF);
	TeAttributeList attList;
	TeReadDBFAttributeList(shpfileName, attList);
	
	stoset.setAttributeList( attList );

	TeAttributeRep repobjid;
	repobjid.name_ = "object_id";
	repobjid.numChar_ = 16;

	TeAttribute attobjid;
	attobjid.rep_ = repobjid;

	TeProperty propobjid;
	propobjid.attr_ = attobjid;
	
	int i,n;
	SHPObject* psShape;
	for (i=0; i<nEntities; i++)
	{
		string objectid = Te2String(i);
		TeSTInstance curObj;
		curObj.objectId(objectid);

		TePropertyVector prop;
		psShape = SHPReadObject(hSHP,i);
		if (TeDecodeShape(psShape,curObj.geometries(),objectid))
		{
			propobjid.value_ = objectid;
			prop.push_back(propobjid);
			for(n=0;n<natt;n++)
			{
				string value = DBFReadStringAttribute(hDBF,i,n);
				TeProperty p;
				p.attr_ = attList[n];
				p.value_ = value;
				prop.push_back(p);
			}
			curObj.properties(prop);
			stoset.insertSTInstance(curObj);
			curObj.theme(0);
		}
		SHPDestroyObject(psShape);
	}
	return true;
} 

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
					string objectIdAtt, unsigned int chunkSize , const bool& useTransaction )
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
	string shpFileN = filePrefix + ".shp";
	string dbfFileN = filePrefix + ".dbf";

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
	
	if(useTransaction == true)
	{
		if (!layer->database()->beginTransaction())  
			return false;
	}

	//read the attribute list information 
	TeAttributeList attList;
	TeReadDBFAttributeList(shpFileName, attList);
	TeRenameInvalidFieldName(attList);

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
					(*it).rep_.numChar_ = 255;
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
		if(useTransaction == true)
		{
			layer->database()->rollbackTransaction();  
		}
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
			try
			{
				row.clear();
				TeDBFRowDecode(hDBF,j,row);
				if (autoIndex)	// --- create automatic index if needed
					row.push_back(Te2String(j+indexOffset));
				if (row[linkCol].empty())
					row[linkCol] = "te_nulo";
				objid = row[linkCol];
				attTable.add(row);
				if ((j%chunkSize) == 0)
				{
					if(layer->saveAttributeTable(attTable) == false)
					{
						throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
					}
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
			catch(TeException& e)
			{
				if(useTransaction == true)
				{
					layer->database()->rollbackTransaction();
					SHPClose(hSHP);
					DBFClose(hDBF);
				}
				throw e;
			}
			catch(...)
			{
				if(useTransaction == true)
				{
					layer->database()->rollbackTransaction();
					SHPClose(hSHP);
					DBFClose(hDBF);
				}
				throw TeException(UNKNOWN_ERROR_TYPE);
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
				try
				{
					row.clear();
					TeDBFRowDecode(hDBF,j,row);
					if (autoIndex)	// --- create automatic index if needed
						row.push_back(Te2String(j+indexOffset));
					if (row[linkCol].empty())
						row[linkCol] = "te_nulo";
					objid = row[linkCol];
					attTable.add(row);
					psShape = SHPReadObject(hSHP,j);
					TeSHPPointDecode(psShape,points,objid);
					SHPDestroyObject(psShape);
					if ((j%chunkSize) == 0)
					{
						if(layer->addPoints(points) == false)
						{
							throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
						}
						points.clear();
						if(layer->saveAttributeTable(attTable) == false)
						{
							throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
						}						
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
				catch(TeException& e)
				{
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw e;
				}
				catch(...)
				{
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw TeException(UNKNOWN_ERROR_TYPE);
				}
			}
			break;
		// --- line geometries ---
	  case SHPT_ARCZ:
	  case SHPT_ARCM:
	  case SHPT_ARC:
			for (j=0; j<(unsigned int)nEntities; j++)
			{
				try
				{
					row.clear();
					TeDBFRowDecode(hDBF,j,row);
					if (autoIndex)	// --- create automatic index if needed
						row.push_back(Te2String(j+indexOffset));
					if (row[linkCol].empty())
						row[linkCol] = "te_nulo";
					objid = row[linkCol];
					attTable.add(row);
					psShape = SHPReadObject(hSHP,j);
					TeSHPPolyLineDecode(psShape,lines,objid);
					SHPDestroyObject(psShape);
					if ((j%chunkSize) == 0)
					{
						if(layer->addLines(lines) == false)
						{
							throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
						}
						lines.clear();
						if(layer->saveAttributeTable(attTable) == false)
						{
							throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
						}						
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
				catch(TeException& e)
			  {
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw e;
			  }
			  catch(...)
			  {
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw TeException(UNKNOWN_ERROR_TYPE);
			  }
			}
			break;
	// --- polygon geometries ---
	  case SHPT_POLYGON:
	  case SHPT_POLYGONM:
	  case SHPT_POLYGONZ:
		  for (j=0; j<(unsigned int)nEntities; j++)
		  {
			  try
			  {
				row.clear();
				TeDBFRowDecode(hDBF,j,row);
				if (autoIndex)	// --- create automatic index if needed
					row.push_back(Te2String(j+indexOffset));
				if (row[linkCol].empty())
					row[linkCol] = "te_nulo";
				objid = row[linkCol];
				attTable.add(row);
				psShape = SHPReadObject(hSHP,j);
				TeSHPPolygonDecode(psShape,polygons,objid);
				SHPDestroyObject(psShape);
				if ((j%chunkSize) == 0)
				{
					if(layer->addPolygons(polygons) == false)
					{
						throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
					}
					polygons.clear();
					if(layer->saveAttributeTable(attTable) == false)
					{
						throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
					}
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
			  catch(TeException& e)
			  {
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw e;
			  }
			  catch(...)
			  {
					if(useTransaction == true)
					{
						layer->database()->rollbackTransaction();
						SHPClose(hSHP);
						DBFClose(hDBF);
					}
					throw TeException(UNKNOWN_ERROR_TYPE);
			  }
		  }
		  break;
	  default:
			/* todo: handle multipatch geometries */
			{
				if(useTransaction == true)
				{
					layer->database()->rollbackTransaction();
				}
				throw TeException ( UNHANDLED_SHP_TYPE );
			}
	}

	SHPClose(hSHP);
	DBFClose(hDBF);

	try
	{
		if (points.size() > 0)
		{
			if(layer->addPoints(points) == false)
			{
				throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
			}
			points.clear();
		}
		if (lines.size() > 0)
		{
			if(layer->addLines(lines) == false)
			{
				throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
			}
			lines.clear();
		}
		if (polygons.size() > 0)
		{
			if(layer->addPolygons(polygons) == false)
			{
				throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
			}
			polygons.clear();
		}
		if (attTable.size() > 0)
		{
			if(layer->saveAttributeTable(attTable) == false)
			{
				throw TeException(UNKNOWN_ERROR_TYPE, layer->database()->errorMessage());
			}
			attTable.clear();
		}
	}
	catch(...)
	{
		if(useTransaction == true)
		{
			layer->database()->rollbackTransaction();
		}
		throw TeException(UNKNOWN_ERROR_TYPE);
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();
	
	if(useTransaction == true)
	{
		if (!layer->database()->commitTransaction())  
			return false;
	}

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

DBFHandle TeCreateDBFFile (const string& dbfFilename, TeAttributeList& attList)
{
	DBFHandle hDBF = DBFCreate( dbfFilename.c_str() );
	if( hDBF == 0 )
		return 0;
	
	int i =0;
	TeAttributeList::iterator it=attList.begin();

	std::set<std::string> setNames;
	while ( it != attList.end() )
	{
		TeAttribute at = (*it);
		string atName = at.rep_.name_;
		TeReplaceSpecialChars(atName);

		if (atName.size() > 10)
		{
			int extra = (int)(atName.size() - 10)/2;
			int middle = (int)(atName.size()/2);
			string str = atName.substr(0,middle-extra-1);
			str += atName.substr(middle+extra);
			atName = str;
			unsigned int count = 0;
			while(setNames.find(atName) != setNames.end())
			{
				std::string strNum = Te2String(count);
				if(strNum.length() == 1)
				{
					atName[atName.size()-1] = strNum[0];
				}
				else
				{
					atName[atName.size()-2] = strNum[0];
					atName[atName.size()-1] = strNum[1];
				}

				++count;
			}
		}

		setNames.insert(atName);

		basic_string <char>::size_type	index;
		if((index=atName.find(".")) !=string::npos)
		{
			atName.replace(index,1,"_");
		}

		if (at.rep_.type_ == TeSTRING )
		{
			int numChar = at.rep_.numChar_;
			if(numChar == 0)
			{
				numChar = 255;
			}

			if(numChar > MAXCHARNUM)
			{
				numChar = MAXCHARNUM;
			}

			if (DBFAddField( hDBF, atName.c_str(), FTString, numChar, 0 ) == -1 )
				return 0;			
		}
		else if (at.rep_.type_ == TeINT)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTInteger, 32, 0 ) == -1 )
				return 0;
		}
		else if (at.rep_.type_ == TeREAL)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTDouble, 40, 15 ) == -1 )
				return 0;
		}
		// OBS: shapelib doesnt deal with xBase field type for Date 
		// we are transforming it to string
		else if (at.rep_.type_ == TeDATETIME)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTDate, 8, 0 ) == -1 )
				return 0;
		}
		else if(at.rep_.type_ == TeCHARACTER)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTString, 1, 0 ) == -1 )
				return 0;
		}
		else if(at.rep_.type_ == TeBOOLEAN)
		{
			if (DBFAddField( hDBF, atName.c_str(), FTLogical, 32, 0 ) == -1 )
				return 0;
		}
		else
		{
			int a = 0;
		}

		if(hDBF->pszCurrentRecord == 0)
		{
			DBFClose(hDBF);
			return 0;
		}

		++i;
		++it;
	}
	return hDBF;
}

bool TeExportPolygonSet2SHP( const TePolygonSet& ps,const std::string& base_file_name  )
{
    // creating files names
    std::string dbfFilename = base_file_name + ".dbf";
    std::string shpFilename = base_file_name + ".shp";

    // creating polygons attribute list ( max attribute size == 25 )
    TeAttributeList attList;
    TeAttribute at;
    at.rep_.type_ = TeSTRING;               //the id of the cell
    at.rep_.numChar_ = 25;
    at.rep_.name_ = "object_id_";
    at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);
    
    /* DBF output file handle creation */
	DBFHandle hDBF = TeCreateDBFFile (dbfFilename, attList);
	if ( hDBF == 0 )
		return false;
    
    /* SHP output file handle creation */
    SHPHandle hSHP = SHPCreate( shpFilename.c_str(), SHPT_POLYGON );
    if( hSHP == 0 ) 
	{
      DBFClose( hDBF );
      return false;
    }
    
    /* Writing polygons */
    int iRecord = 0;
    int totpoints = 0;
    double  *padfX, *padfY;
    SHPObject       *psObject;
    int posXY, npoints, nelem;
    int nVertices;
    int* panParts;

    TePolygonSet::iterator itps;
    TePolygon poly;

    for (itps = ps.begin() ; itps != ps.end() ; itps++ ) {
      poly=(*itps);
      totpoints = 0;
      nVertices = poly.size();
      for (unsigned int n=0; n<poly.size();n++) {
        totpoints += poly[n].size();
      }

      panParts = (int *) malloc(sizeof(int) * nVertices);
      padfX = (double *) malloc(sizeof(double) * totpoints);
      padfY = (double *) malloc(sizeof(double) * totpoints);
      posXY = 0;
      nelem = 0;
      
      for (unsigned int l=0; l<poly.size(); ++l) {
        if (l==0) {
          if (TeOrientation(poly[l]) == TeCOUNTERCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        } else {
          if (TeOrientation(poly[l]) == TeCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        }
        
        npoints = poly[l].size();
        panParts[nelem]=posXY;
        
        for (int m=0; m<npoints; m++ ) {
          padfX[posXY] = poly[l][m].x_;
          padfY[posXY] = poly[l][m].y_;
          posXY++;
        }
        nelem++;
      }
                
      psObject = SHPCreateObject( SHPT_POLYGON, -1, nelem, panParts, NULL,
        posXY, padfX, padfY, NULL, NULL );
        
      int shpRes = SHPWriteObject( hSHP, -1, psObject );
      if (shpRes == -1 )
	  {
	      DBFClose( hDBF );
		  SHPClose( hSHP );
		  return false;
	  }
        
      SHPDestroyObject( psObject );
      free( panParts );
      free( padfX );
      free( padfY );

      // writing attributes - same creation order
      DBFWriteStringAttribute(hDBF, iRecord, 0, poly.objectId().c_str() );
            
      iRecord++;
    }
    
    DBFClose( hDBF );
    SHPClose( hSHP );
    return true;  
}


bool
TeExportQuerierToShapefile(TeQuerier* querier, const std::string& base)
{
	// check initial conditions
	if (!querier)
		return false;

	if (!querier->loadInstances())
		return false;

	// Get the list of attributes defined by the input querier
	bool onlyObjId = false;
	TeAttributeList qAttList = querier->getAttrList();
	if (qAttList.empty())
	{
		TeAttributeList qAttList;
		TeAttribute at;
		at.rep_.type_ = TeSTRING;               
		at.rep_.numChar_ = 100;
		at.rep_.name_ = "ID";
		at.rep_.isPrimaryKey_ = true;
		qAttList.push_back(at);
		onlyObjId = true;
	}

	// Handles to each type of geometries that will be created if necessary
	DBFHandle hDBFPol = 0;
	SHPHandle hSHPPol = 0;
	DBFHandle hDBFLin = 0;
	SHPHandle hSHPLin = 0;
	DBFHandle hDBFPt = 0;
	SHPHandle hSHPPt = 0;

	// Some auxiliary variables
    int totpoints;
    double*	padfX;
	double*	padfY;
    unsigned int nVertices;
    int* panPart;
    SHPObject *psObject;
    unsigned int posXY, npoints, nelem;
	int shpRes;

	// progress information
	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;

	TeProjection* outputProj = 0;
	if(querier->params().layer())
	{
		outputProj = querier->params().layer()->projection();
	}
	else
	{
		outputProj = querier->params().theme()->layer()->projection();
	}

	// Loop through the instances writting their geometries and attributes
	unsigned int iRecPol=0, iRecLin=0, iRecPt=0;
	unsigned int n, l, m;
	unsigned int nIProcessed = 0;
	TeSTInstance st;
	while(querier->fetchInstance(st))
	{
		totpoints = 0;
		nVertices = 0;
		if (st.hasPolygons())
		{
			TePolygonSet& polSet = st.getPolygons();
			TePolygonSet::iterator itps;
			int nVerticesCount = 0;
			for (itps = polSet.begin(); itps != polSet.end(); ++itps) 
			{
				nVertices = (*itps).size();
				nVerticesCount += nVertices;
				for (n=0; n<nVertices;++n) 
					totpoints += (*itps)[n].size();
			}

			panPart = (int *) malloc(sizeof(int) * nVerticesCount);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);

			posXY = 0;
			nelem = 0;
			for (itps = polSet.begin(); itps != polSet.end(); ++itps) 
			{
				TePolygon poly = *itps;
				for (l=0; l<poly.size(); ++l) 
				{
					if (l==0) 
					{
						if (TeOrientation(poly[l]) == TeCOUNTERCLOCKWISE) 
							TeReverseLine(poly[l]);
					}
					else 
					{
						if (TeOrientation(poly[l]) == TeCLOCKWISE)
							TeReverseLine(poly[l]);
					}
					npoints = poly[l].size();
					panPart[nelem]=posXY;
        
					for (m=0; m<npoints; ++m) 
					{
						padfX[posXY] = poly[l][m].x_;
						padfY[posXY] = poly[l][m].y_;
						posXY++;
					}
					nelem++;
				}
			}
			psObject = SHPCreateObject(SHPT_POLYGON, -1, nelem, panPart, NULL, posXY, padfX, padfY, NULL, NULL);
			if (hSHPPol == 0)
			{				
				string fname = base + "_pol.shp";
				hSHPPol = SHPCreate(fname.c_str(), SHPT_POLYGON);
   				assert (hSHPPol != 0);
				
				TeWriteDataProjectionToFile(outputProj, TeGetName(fname.c_str()));
			}	
			shpRes = SHPWriteObject(hSHPPol, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);   
			if (hDBFPol == 0)
			{
				hDBFPol = TeCreateDBFFile(base + "_pol.dbf", qAttList);
				if(hDBFPol == 0)
				{
					return false;
				}
				assert (hDBFPol != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPol, iRecPol, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPol, iRecPol, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPol, iRecPol, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPol, iRecPol, n, atof(val.c_str()));
					}
				}
			}
			++iRecPol;
		}
		if (st.hasCells())
		{
			TeCellSet& cellSet = st.getCells();
			nVertices = cellSet.size();
			totpoints = nVertices*5;
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);
			posXY = 0;
			nelem = 0;
			TeCellSet::iterator itcs;
			for (itcs=cellSet.begin(); itcs!=cellSet.end(); ++itcs)
			{
				panPart[nelem]=posXY;
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().upperRight().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().upperRight().x();
				padfY[posXY] = (*itcs).box().upperRight().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().upperRight().y();
				posXY++;		
				padfX[posXY] = (*itcs).box().lowerLeft().x();
				padfY[posXY] = (*itcs).box().lowerLeft().y();
				++posXY;	
				++nelem;
			} 
			psObject = SHPCreateObject(SHPT_POLYGON, -1, nelem, panPart, NULL,posXY, padfX, padfY, NULL, NULL);
			if (hSHPPol == 0)
			{
				string fname = base + "_pol.shp";
				hSHPPol = SHPCreate(fname.c_str(), SHPT_POLYGON);
   				assert (hSHPPol != 0);

				TeWriteDataProjectionToFile(outputProj, TeGetName(fname.c_str()));
			}	
			shpRes = SHPWriteObject(hSHPPol, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFPol == 0)
			{
				hDBFPol = TeCreateDBFFile(base + "_pol.dbf", qAttList);
				assert (hDBFPol != 0);
			}

			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPol, iRecPol, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPol, iRecPol, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPol, iRecPol, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPol, iRecPol, n, atof(val.c_str()));
					}
				}
			}
			++iRecPol;
		}
		if (st.hasLines())
		{
			TeLineSet& lineSet = st.getLines();
			nVertices = lineSet.size();
			TeLineSet::iterator itls;
			for (itls=lineSet.begin(); itls!=lineSet.end(); ++itls)
				totpoints += (*itls).size();
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * totpoints);
			padfY = (double *) malloc(sizeof(double) * totpoints);
			posXY = 0;
			nelem = 0;
			for (itls=lineSet.begin(); itls!=lineSet.end(); ++itls)
			{
				panPart[nelem]=posXY;
				for (l=0; l<(*itls).size(); ++l)
				{
					padfX[posXY] = (*itls)[l].x();
					padfY[posXY] = (*itls)[l].y();
					++posXY;
				}
				++nelem;
			}
			psObject = SHPCreateObject(SHPT_ARC, -1, nVertices, panPart, NULL,	posXY, padfX, padfY, NULL, NULL);
			if (hSHPLin == 0)
			{
				string fname = base + "_lin.shp"; 
				hSHPLin = SHPCreate(fname.c_str(), SHPT_ARC);
   				assert (hSHPLin != 0);

				TeWriteDataProjectionToFile(outputProj, TeGetName(fname.c_str()));
			}		
			shpRes = SHPWriteObject(hSHPLin, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFLin == 0)
			{
				hDBFLin = TeCreateDBFFile(base + "_lin.dbf", qAttList);
				assert (hDBFLin != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFLin, iRecLin, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
					if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFLin, iRecLin, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFLin, iRecLin, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFLin, iRecLin, n, atof(val.c_str()));
					}
				}
			}
			++iRecLin;
		}
		if (st.hasPoints())
		{
			TePointSet& pointSet = st.getPoints();
			nVertices = pointSet.size();
			panPart = (int *) malloc(sizeof(int) * nVertices);
			padfX = (double *) malloc(sizeof(double) * nVertices);
			padfY = (double *) malloc(sizeof(double) * nVertices);
			nelem = 0;
			TePointSet::iterator itpts;
			for (itpts=pointSet.begin(); itpts!=pointSet.end(); ++itpts)
			{
				panPart[nelem] = nelem;
				padfX[nelem] = (*itpts).location().x();
				padfY[nelem] = (*itpts).location().y();
				++nelem;
			}
			psObject = SHPCreateObject(SHPT_POINT, -1, nVertices, panPart, NULL, nVertices, padfX, padfY, NULL, NULL );
			if (hSHPPt == 0)
			{
				string fname = base + "_pt.shp";
				hSHPPt = SHPCreate(fname.c_str(), SHPT_POINT);
   				assert (hSHPPt != 0);

				TeWriteDataProjectionToFile(outputProj, TeGetName(fname.c_str()));
			}		
			shpRes = SHPWriteObject(hSHPPt, -1, psObject);
			SHPDestroyObject(psObject);
			free(panPart);
			free(padfX);
			free(padfY);
			assert(shpRes != -1);
			if (hDBFPt == 0)
			{
				hDBFPt = TeCreateDBFFile(base + "_pt.dbf", qAttList);
				assert (hDBFPt != 0);
			}
			if (onlyObjId)
			{
				DBFWriteStringAttribute(hDBFPt, iRecPt, 0, st.objectId().c_str());
			}
			else
			{
				string val;
				for (n=0; n<qAttList.size();++n) 
				{
					st.getPropertyValue(val,n);
						if (qAttList[n].rep_.type_ == TeSTRING || qAttList[n].rep_.type_ == TeDATETIME)
					{
						DBFWriteStringAttribute(hDBFPt, iRecPt, n, val.c_str());
					}
					else if (qAttList[n].rep_.type_ == TeINT)
					{
						DBFWriteIntegerAttribute(hDBFPt, iRecPt, n, atoi(val.c_str()));
					}
					else if (qAttList[n].rep_.type_ == TeREAL)
					{
						DBFWriteDoubleAttribute(hDBFPt, iRecPt, n, atof(val.c_str()));
					}
				}
			}
			++iRecPt;
		}
		++nIProcessed;
		if (TeProgress::instance() && int(t2-t1) > dt)
		{
			t1 = t2;
			if(((int)(t2-t0) > dt2))
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(nIProcessed);
			}
		}

	}
	if (hDBFPol != 0)
		DBFClose(hDBFPol);
	if (hSHPPol != 0)
        SHPClose(hSHPPol);
	if (hDBFLin != 0)
		DBFClose(hDBFLin);
	if (hSHPLin != 0)
        SHPClose(hSHPLin);
	if (hDBFPt != 0)
		DBFClose(hDBFPt);
	if (hSHPPt != 0)
        SHPClose(hSHPPt);

	if (TeProgress::instance())
		TeProgress::instance()->reset();	

	return true;
}


bool
TeExportShapefile(TeLayer* layer, const string& shpFileName, const string& tableName, const string& restriction)
{
  	if (!layer || shpFileName.empty())
		return false;

	// check if asked table exist
	TeAttrTableVector& vTables = layer->attrTables();
	TeAttrTableVector::iterator it = vTables.begin();
	while (it != vTables.end())
	{
		if (it->name() == tableName) 
			break;
		++it;
	}
	if (it == vTables.end())
		return false;
	TeAttrTableVector askedTable;
	askedTable.push_back(*it);

	TeTheme* tempTheme = new TeTheme();
	tempTheme->attributeRest(restriction);
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	tempTheme->setAttTables(askedTable);

	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, shpFileName);
	delete tQuerier;
	delete tempTheme;

    return res ;
}

bool 
TeExportLayerToShapefile(TeLayer* layer, const string& baseName)
{
  	if (!layer)
		return false;

	string fbase = baseName;
	if (baseName.empty())
		fbase = layer->name();

	TeTheme* tempTheme = new TeTheme();
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	tempTheme->setAttTables(layer->attrTables());

	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, baseName);
	delete tQuerier;
	delete tempTheme;

	return res ;
}


bool 
TeExportThemeToShapefile(TeTheme* theme, TeSelectedObjects selOb, const std::string& baseName, const std::vector<std::string> attributes)
{
	if (!theme)
		return false;

	string fbase = baseName;
	if (baseName.empty())
		fbase = theme->name();

	TeQuerierParams qPar(true, true);
	qPar.setParams(theme);
	qPar.setSelecetObjs(selOb);

	if(!attributes.empty())
	{
		qPar.setFillParams(true, false, attributes);		
	}

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToShapefile(tQuerier, fbase);
	delete tQuerier;

	return res;
}

void TeRemoveNonAsciiChar(std::string& text)
{
	std::string::iterator it = text.begin();
	while(it != text.end())
	{
		char character = *it;
		if(character < 0)
		{
			text.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void TeDBFRowDecode(DBFHandle& hDBF, int nr, TeTableRow& row)
{
	string value;
	int i;
	int total = DBFGetFieldCount(hDBF);
	for (i = 0; i < total; i++)
	{
		if(DBFIsAttributeNULL(hDBF, nr, i) == true)
		{
			value = "";
		}
		else
		{
			value =  DBFReadStringAttribute(hDBF,nr,i);

			char	szTitle[12];
			int		nWidth, nDecimals;
			DBFFieldType	eType = DBFGetFieldInfo(hDBF, i, szTitle, &nWidth, &nDecimals );

			if(eType == FTInteger)
			{
				if(nWidth <= 9)
				{
					int iValue = atoi(value.c_str());
					if(iValue == 0 && value != "0")
					{
						value = "";
					}
				}
				else
				{
					eType = FTDouble;
				}
			}
			if(eType == FTDouble)
			{
				double dValue = atof(value.c_str());
				if(dValue == 0. && value != "0" && value != "0." && value != "0.0")
				{
					value = "";
				}
			}
			else
			{
				//removido temporarimante ate que se entenda claramente os problema na importacao de DBFs com acentuacao
				//TeRemoveNonAsciiChar(value);
			}
		}
		row.push_back(value); 
	}
}

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

//! Fills a vector with the values of choosen column named , to be used as object identifications
/*!
	\param dbfFileName DBF file name
	\param indexes a vector that has the identification of the objects
	\param objectIdAttr name of the attribute that has the identification of objects
	\note if no objectIdAttr is provided or there is no column with this name return
	an incremental sequence of numbers.
*/
void TeReadDBFIndexes(const string& dbfFileName, vector<string>& indexes, const string objectIdAttr="");


bool TeReadDBFAttributeList ( const string& dbfFileName, TeAttributeList& attList )
{
	string filePrefix = TeGetName (dbfFileName.c_str()) ;
	string fileName = filePrefix + ".dbf";

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    DBFHandle hDBF = DBFOpen( fileName.c_str(), "rb" );
    if( hDBF == NULL )
    {
			throw TeException ( UNABLE_TO_OPEN_DBF_FILE );
    }
    
/* -------------------------------------------------------------------- */
/*	If there is no data in this file let the user know.		*/
/* -------------------------------------------------------------------- */
    if( DBFGetFieldCount(hDBF) == 0 )
    {
			throw TeException ( NO_DATA_IN_DBF_FILE );
    }

/* -------------------------------------------------------------------- */
/*	Get Attributes header definitions.									*/
/* -------------------------------------------------------------------- */
	TeAttribute attribute;
    
	char	szTitle[12];
	int		nWidth, nDecimals;
	int 	i;

	vector<string> names;

	for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
    {
		DBFFieldType	eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );

		string atname = TeConvertToUpperCase(szTitle);
	
		vector<string>::iterator it = find(names.begin(),names.end(),atname);
		if (it != names.end())
		{
			string newname = atname + "_" + Te2String(i);
			int c = i+1;
			while (true)
			{
				it = find(names.begin(),names.end(),newname);
				if (it == names.end())
					break;
				newname = atname + "_" + Te2String(c);
				++c;
			}
			atname = newname;
		}
		names.push_back(atname);
		attribute.rep_.name_ = atname;

        if( eType == FTDate )
		{
			attribute.rep_.type_     = TeDATETIME;
			attribute.dateChronon_   = TeDAY;
			attribute.dateTimeFormat_ = "YYYYMMDD";
			attribute.dateSeparator_ = 'c';
		}
        else if( eType == FTString )
		{
			attribute.rep_.type_     = TeSTRING;
			attribute.rep_.numChar_  = nWidth;
			if (attribute.rep_.numChar_ > MAXCHARNUM) 
			{
				attribute.rep_.numChar_ = MAXCHARNUM;
			}
		}
		else if( eType == FTInteger )
		{
			if (nWidth > 9)
			{
				attribute.rep_.type_ = TeREAL;
				attribute.rep_.numChar_ = nWidth;
				attribute.rep_.decimals_ = 0;
			}
			else
			{
				attribute.rep_.type_ = TeINT;
				attribute.rep_.numChar_ = nWidth;
			}
		}
        else if( eType == FTDouble )
		{
            attribute.rep_.type_ = TeREAL;
			attribute.rep_.numChar_ = nWidth;
			attribute.rep_.decimals_ = nDecimals;
		}

		// keep track of the index name 

		attribute.rep_.name_ = TeReplaceSpecialChars(attribute.rep_.name_);
		attribute.rep_.name_ = TeConvertToUpperCase(attribute.rep_.name_);

		attList.push_back ( attribute );
    }
	names.clear();
    DBFClose( hDBF );
	return true;
}


bool TeImportDBFTable(const string& dbFileName,TeDatabase* db, TeLayer* layer,TeAttrTableType tableType, string objectIdAtt)
{
	if (!db || dbFileName.empty())
		return false;

    DBFHandle hDBF = DBFOpen(dbFileName.c_str(), "rb" );
    if( hDBF == NULL )
		return false;

	string tabName = TeGetBaseName(dbFileName.c_str());
	int n=0;
	while (db->tableExist(tabName))
	{
		tabName = TeGetBaseName(dbFileName.c_str()) + "_" + Te2String(n);
		n++;
	}
	TeAttributeList attList;
	TeReadDBFAttributeList (dbFileName, attList);

	if (objectIdAtt.empty())
		objectIdAtt = "object_id_";

	// check if object id column is already in the list
	unsigned int j;
	for (j=0; j<attList.size(); j++)
	{
		if ( TeConvertToUpperCase(objectIdAtt) ==  TeConvertToUpperCase(attList[j].rep_.name_))
		{
			objectIdAtt = TeConvertToUpperCase(attList[j].rep_.name_);
			attList[j].rep_.name_ =  objectIdAtt;
			if (attList[j].rep_.type_ != TeSTRING)
			{
				attList[j].rep_.type_ = TeSTRING;
				attList[j].rep_.numChar_ = 255;
			}
			attList[j].rep_.isPrimaryKey_ = true;
			break;
		}
	}

	// if not found create one automatically
	bool createObjId = false;
	if (j==attList.size())
	{
		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 16;
		at.rep_.name_ = objectIdAtt;
		at.rep_.isPrimaryKey_ = true;
		attList.push_back(at);
		createObjId = true;
	}

	if (!db->createTable(tabName,attList))
	{
	    DBFClose(hDBF);
		return false;
	}

	TeTable table(tabName);
	table.setAttributeList(attList);
	table.setUniqueName(objectIdAtt);
	if (layer)
		table.setTableType(tableType);
	else
		table.setTableType(TeAttrExternal);

	int nrec = DBFGetRecordCount(hDBF);
	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(nrec);
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int iRecord;
	for (iRecord = 0; iRecord < nrec; iRecord++)
    {
		TeTableRow row;
		int i;
		for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
		{
			string value =  DBFReadStringAttribute(hDBF, iRecord, i);
			row.push_back ( value ); // inserts a value into the row
		}
		if (createObjId)
			row.push_back(Te2String(iRecord));
		table.add(row); 
		if ((iRecord+1 % 100) == 0)
		{
			if (!db->insertTable(table))
			{
				DBFClose( hDBF );
				table.clear();
				if (TeProgress::instance())
					TeProgress::instance()->reset();		
				return false;
			}
			table.clear();
			if (TeProgress::instance())
			{
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					if (TeProgress::instance()->wasCancelled())
						break;
					if ((int)(t2-t0) > dt2)
						TeProgress::instance()->setProgress(iRecord);
				}
			}
		}
    }
	if (table.size() > 0)
	{
		db->insertTable(table);
		table.clear();
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();		
    DBFClose( hDBF );
	if (iRecord == 0)
		return false;
	if (layer)
		db->insertTableInfo(layer->id(),table);
	else
		db->insertTableInfo(0,table);

	return true;
}

void TeReadSHPAttributes (const string& dbfFileName, TeTable& table)
{
	string filePrefix = TeGetName (dbfFileName.c_str()) ;
	string fileName = filePrefix + ".dbf";

/* -------------------------------------------------------------------- */
/*	Get Attributes definitions.											*/
/* -------------------------------------------------------------------- */
	TeAttributeList attList;
	TeReadDBFAttributeList ( fileName, attList );
	table.setAttributeList( attList );

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    DBFHandle hDBF = DBFOpen( fileName.c_str(), "rb" );
    if( hDBF == NULL )
    {
			throw TeException ( UNABLE_TO_OPEN_DBF_FILE, fileName, true );
    }
    
/* -------------------------------------------------------------------- */
/*	If there is no data in this file let the user know.		*/
/* -------------------------------------------------------------------- */
    if( DBFGetFieldCount(hDBF) == 0 )
    {
			throw TeException ( NO_DATA_IN_DBF_FILE, fileName );
    }

/* -------------------------------------------------------------------- */
/*	Read all the records 						                        */
/* -------------------------------------------------------------------- */
	for( int iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++ )
    {
		TeTableRow row;
		int 	i;

		for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
		{
			string value =  DBFReadStringAttribute( hDBF, iRecord, i ) ;
			row.push_back ( value ); // inserts a value into the row
		}
		table.add ( row ); // adds a row to a table
    }
    DBFClose( hDBF );
}


bool TeReadDBFAttributes (const string& dbfFileName, TeTable& table, 
						  int nrecords, int rinitial, bool createAuto)
{
	string filePrefix = TeGetName (dbfFileName.c_str()) ;
	string fileName = filePrefix + ".dbf";

// -------------------------------------------------------------------- 
//      Open the file.                                                  
// -------------------------------------------------------------------- 
    DBFHandle hDBF = DBFOpen( fileName.c_str(), "rb" );
    if( hDBF == NULL )
    {
			throw TeException ( UNABLE_TO_OPEN_DBF_FILE, fileName, true );
    }
    
// -------------------------------------------------------------------- 
//	If there is no data in this file let the user know.		
// -------------------------------------------------------------------- 
    if( DBFGetFieldCount(hDBF) == 0 )
    {
			throw TeException ( NO_DATA_IN_DBF_FILE, fileName );
    }

	if (rinitial >= DBFGetRecordCount(hDBF))
		return false;

	if (nrecords < 1 || nrecords > DBFGetRecordCount(hDBF))
		nrecords = DBFGetRecordCount(hDBF);

// -------------------------------------------------------------------- 
//	Read all the records 						                        
// -------------------------------------------------------------------- 
	int count = 0;
	for( int iRecord = rinitial; (iRecord < DBFGetRecordCount(hDBF) && count < nrecords); iRecord++ )
    {
		TeTableRow row;
		int 	i;
		for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
		{
			string value =  DBFReadStringAttribute( hDBF, iRecord, i ) ;
			row.push_back ( value ); // inserts a value into the row
		}
		
		// if asked create a auto number extra column at the end
		if (createAuto)
			row.push_back(Te2String(iRecord));

		table.add ( row ); // adds a row to a table
		count++;
    }
    DBFClose( hDBF );
	if (count > 0)
		return true;
	return false;
}

void
TeImportDBFIndexes(const string& shpFileName, vector<string>& indexVect, const string objectIdAttr )
{
	string filePrefix = TeGetName(shpFileName.c_str());
	string fileName = filePrefix + ".dbf";

/* -------------------------------------------------------------------- */
/*	Get index position.													*/
/* -------------------------------------------------------------------- */
	int i=0, indexPos=-1;

	if (!objectIdAttr.empty())
	{
		TeAttributeList attList;
		TeReadDBFAttributeList ( fileName, attList );
		TeAttributeList::iterator it = attList.begin();
		while ( it != attList.end() )
		{
			TeAttribute att = *it;
			if (TeConvertToUpperCase(att.rep_.name_) == TeConvertToUpperCase(objectIdAttr))
			{
				indexPos = i;
				break;
			}
			it++;
			i++;
		}
	}

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    DBFHandle hDBF = DBFOpen( fileName.c_str(), "rb" );
    if( hDBF == NULL )
    {
			throw TeException ( UNABLE_TO_OPEN_DBF_FILE, fileName, true );
    }
    
/* -------------------------------------------------------------------- */
/*	If there is no data in this file let the user know.		*/
/* -------------------------------------------------------------------- */
	int nRecords = DBFGetFieldCount(hDBF);
    if( nRecords  == 0 )
    {
			throw TeException ( NO_DATA_IN_DBF_FILE, fileName );
    }

/* -------------------------------------------------------------------- */
/*	Read all the records 						                        */
/* -------------------------------------------------------------------- */
    
	for( int iRecord = 0; iRecord < nRecords; iRecord++ )
	{
		if (indexPos == -1)
			indexVect.push_back (Te2String(iRecord+1)); 
		else
			indexVect.push_back ( DBFReadStringAttribute( hDBF, iRecord, indexPos ) );
	}

    DBFClose( hDBF );
}

TeShapefileDriver::TeShapefileDriver(std::string fileName):
	hSHP_(0),
	hDBF_(0), 
	shpFileName_(fileName)
{
	
}

std::string
TeShapefileDriver::getFileName()
{
	return shpFileName_;
}

TeShapefileDriver::~TeShapefileDriver()
{
	close();
}

bool
TeShapefileDriver::isDataAccessible()
{
	if(initMetadata() == false)
		return false;
	else
		return true;
}

bool TeShapefileDriver::loadData()
{
	if(isDataInited() == true)
	{
		return true;
	}

	isDataInited_ = true;

	if(dataSet_ == 0)
	{
		dataSet_ = new TeSTElementSet(getBox(), getAttributesList());
	}

	int		nShapeType, nentities=0;
	double 	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP_, &nentities, &nShapeType, adfMinBound, adfMaxBound);
    
	int natt = DBFGetFieldCount(hDBF_);
  attrList_.clear();
	TeReadDBFAttributeList(dbfFileName_, attrList_);

	TeAttributeRep repobjid;
	repobjid.name_ = "object_id";
	repobjid.numChar_ = 16;

	TeAttribute attobjid;
	attobjid.rep_ = repobjid;

	TeProperty propobjid;
	propobjid.attr_ = attobjid;
	
	int i,n;
	SHPObject* psShape;
	int geomId = 1;
	for (i=0; i<nentities; ++i)
	{
		string objectid = Te2String(i);
		TeSTInstance curObj;
		curObj.objectId(objectid);

		TePropertyVector prop;
		psShape = SHPReadObject(hSHP_,i);
		if (TeDecodeShape(psShape,curObj.geometries(),objectid))
		{
			for(unsigned int j = 0; j < curObj.geometries().getPolygons().size(); ++j)
			{
				curObj.geometries().getPolygons()[j].geomId(geomId);
				++geomId;
			}
			for(unsigned int j = 0; j < curObj.geometries().getLines().size(); ++j)
			{
				curObj.geometries().getLines()[j].geomId(geomId);
				++geomId;
			}
			for(unsigned int j = 0; j < curObj.geometries().getPoints().size(); ++j)
			{
				curObj.geometries().getPoints()[j].geomId(geomId);
				++geomId;
			}

			propobjid.value_ = objectid;
			prop.push_back(propobjid);
			for(n=0;n<natt;++n)
			{
				string value = DBFReadStringAttribute(hDBF_,i,n);
				TeProperty p;
				p.attr_ = attrList_[n];
				p.value_ = value;
				prop.push_back(p);
			}
			curObj.properties(prop);
			dataSet_->insertSTInstance(curObj);
			curObj.theme(0);
		}
		SHPDestroyObject(psShape);
	}	

	return true;
}

bool TeShapefileDriver::initData() 
{ 
	if(isDataInited_ == true)
	{
		return true;
	}

	isDataInited_ = loadData();

	return isDataInited_;
}

bool TeShapefileDriver::initMetadata() 
{ 
	if(isMetadataInited_ == true)
	{
		return true;
	}

	isMetadataInited_ = true;

	FILE* fp = fopen(shpFileName_.c_str(), "r");
	if(fp == 0)
	{
		return false;
	}
	
	fclose(fp);

	// check if format is complete (SHP, SHX e DBF files exist)
	string filePrefix = TeGetName(shpFileName_.c_str());

	shpFileName_ = filePrefix + ".shp";
	if (access(shpFileName_.c_str(),04) == -1)
	{
		shpFileName_ = filePrefix + ".SHP";
		if (access(shpFileName_.c_str(),04) == -1)
			return false;
	}
	dbfFileName_ = filePrefix + ".dbf";
	if (access(dbfFileName_.c_str(),04) == -1)
	{
		dbfFileName_ = filePrefix + ".DBF";
		if (access(dbfFileName_.c_str(),04) == -1)
			return false;
	}
	string ftest = filePrefix + ".shx";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".SHX";
		if (access(ftest.c_str(),04) == -1)
			return false;
	}
	hSHP_ = SHPOpen(shpFileName_.c_str(),"rb");
	if (hSHP_==0)
		return false;

    hDBF_ = DBFOpen(dbfFileName_.c_str(), "rb");
    if (hDBF_==0)
    {
		SHPClose(hSHP_);
		return false;
    }

  attrList_.clear();
	TeReadDBFAttributeList(dbfFileName_, attrList_);

	int		nShapeType, nentities=0;
	double 	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP_, &nentities, &nShapeType, adfMinBound, adfMaxBound);
	numObjects_ = nentities;
	
	box_ = TeBox(adfMinBound[0], adfMinBound[1], adfMaxBound[0], adfMaxBound[1]);

	switch (nShapeType) 
	{
		case SHPT_NULL:
			rep_ = TeGEOMETRYNONE;
		break;
		// --- point geometries ---
		case SHPT_POINTZ:	
		case SHPT_MULTIPOINTZ:
		case SHPT_POINTM:
		case SHPT_MULTIPOINTM:
		case SHPT_POINT:
		case SHPT_MULTIPOINT:
			rep_ = TePOINTS;
		break;
		// --- line geometries ---
		case SHPT_ARCZ:
		case SHPT_ARCM:
		case SHPT_ARC:
			rep_ = TeLINES;
		break;
		// --- polygon geometries ---
		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:
			rep_ = TePOLYGONS;
	}	

	string prjFileName_ = filePrefix + ".prj";
	if (access(prjFileName_.c_str(),04) == -1)
	{
		prjFileName_ = filePrefix + ".PRJ";
		if (access(prjFileName_.c_str(),04) == -1)
		{
			dataProjection_ = new TeNoProjection();
			return true;
		}
	}
	TeAsciiFile prjFile(prjFileName_);
	string prjdesc = prjFile.readAll();
	dataProjection_ = TeGetTeProjectionFromWKT(prjdesc);
	if (dataProjection_ == 0)
		dataProjection_ = new TeNoProjection();

	return true;
}

void TeShapefileDriver::close()
{	
	if (hSHP_)
	{
		SHPClose(hSHP_);
		hSHP_ = 0;
	}
	if (hDBF_)
	{
		DBFClose(hDBF_);
		hDBF_ = 0;
	}
	shpFileName_.clear();
	dbfFileName_.clear();
}


