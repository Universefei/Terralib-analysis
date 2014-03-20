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

#include <string>
#include <iostream>
#include <list>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace std;

#include "TeDriverBNA.h"
#include "../kernel/TeTable.h"
#include "../kernel/TeGeometry.h"
#include "../kernel/TeAsciiFile.h"
#include "../kernel/TeException.h"
#include "../kernel/TeProjection.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeTable.h"
#include "../kernel/TeAssertions.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeDatabase.h"

bool TeReadBNAObjects(TeAsciiFile& bnaFile, TeTable& attTable, TePointSet& ptSet,
					  TeLineSet& lnSet, TePolygonSet& pSet, 
					  int ncolumns, int nrecords, int delta, int attLinkColumnIndex);

bool TeBNARegionDecode(TeAsciiFile& bnaFile, TePolygonSet& temp, string& index, const int& npoints);

bool TeBNALineDecode(TeAsciiFile& bnaFile, TeLineSet& temp, string& index, const int& npoints);

bool TeBNAPointDecode(TeAsciiFile& bnaFile, TePointSet& temp, string& index);



TeLayer* TeImportBNA(const string& bnaFileName, TeDatabase* db, const string& layerName, const std::string& linkName)
{
	if(!db || bnaFileName.empty()) 
		return 0;

	// check if format is complete (BNA file exist)
	string filePrefix = TeGetName(bnaFileName.c_str());
	string ftest = filePrefix + ".bna";
	if(access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".BNA";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	// found a valid layer name
	string lName = layerName;

	if(layerName.empty())
		lName = TeGetBaseName(bnaFileName.c_str());	

	string newLayerName = lName;

	TeLayerMap& layerMap = db->layerMap();
	TeLayerMap::iterator it;

	bool flag = true;

	int n = 1;

	while(flag)
	{
		for(it = layerMap.begin(); it != layerMap.end(); ++it)
		{
			if(TeStringCompare(it->second->name(), newLayerName))
				break;
		}

		if(it == layerMap.end())
			flag = 0;
		else
			newLayerName = lName + "_" + Te2String(n);

		n++;	
	}

	// Use no projection
	TeLayer* newLayer = new TeLayer(newLayerName, db);

	if(newLayer->id() <= 0)
		return 0;				// couldn�t create new layer

	TeAttributeList attList;

	std::string tableName = "";
	std::string linkNameCopy = linkName;
	bool res = TeImportBNA(newLayer, bnaFileName, tableName, attList, 60, linkNameCopy);

	if (res)
		return newLayer;
	else
	{
		db->deleteLayer(newLayer->id());

		delete newLayer;

		return 0;
	}
}

bool TeImportBNA(TeLayer* layer, const string& bnaFileName, string attrTableName,
				 TeAttributeList& attList, int unsigned chunkSize, std::string& linkName)
{
	if(chunkSize <= 0)
		chunkSize = 1;

	//-- Step 1: Read any adicional information (ex. projection) 
	char separator = ',';


	if(!TeReadBNAAttributeList(bnaFileName, attList))
		return false;	// Attributelist read error

	bool autoIndex = true;
	int linkIndex = -1;
	unsigned int count = 0;
	int ncol = attList.size();
	if(linkName.empty())
	{
		linkName = "object_id_" + Te2String(layer->id());
		std::string s2 = TeConvertToUpperCase(linkName);
		while(true)
		{
			unsigned int j = 0;
			for (j=0; j<attList.size(); j++)
			{
				string s0 = attList[j].rep_.name_;
				string s1 = TeConvertToUpperCase(s0);
				if ( s1 == s2)
				{
					break;
				}
			}
			if(j < attList.size())
			{
				++count;
				linkName = "object_id_" + Te2String(count);
			}
			else
			{
				break;
			}
		}

		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 16;
		at.rep_.name_ = linkName;
		at.rep_.isPrimaryKey_ = true;
		attList.push_back(at);
		linkIndex = attList.size()-1;
		autoIndex = true;
	}
	else
	{
		unsigned int j = 0;
		// check if given index is valid
		TeAttributeList::iterator it = attList.begin();
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
				linkIndex = j;
				break;
			}
			++it;
			++j;
		}
		if (it == attList.end())			// index not found
		{
			linkName = "object_id_" + Te2String(layer->id());;
			while(true)
			{
				for(j=0; j<attList.size(); j++)
				{
					if (TeConvertToUpperCase(attList[j].rep_.name_) == TeConvertToUpperCase(linkName))
					{
						break;
					}
				}
				if(j < attList.size())
				{
					++count;
					linkName = "object_id_" + Te2String(count);
				}
				else
				{
					break;
				}
			}
			TeAttribute at;
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_ = 16;
			at.rep_.name_ = linkName;
			at.rep_.isPrimaryKey_ = true;
			attList.push_back(at);
			autoIndex = true;
		}
	}

	// define a TeAttributeTable
	if (attrTableName.empty())
	{
		if (layer->name().empty())
			return false;
		else
			attrTableName = layer->name();
	}
	
	TeTable attTable(attrTableName, attList, linkName, linkName, TeAttrStatic);

	attTable.setSeparator(separator);

	// insert the table into the database
	if(!layer->createAttributeTable(attTable))
	{
		return false;
	}	
	
	TeAsciiFile bnaFile(bnaFileName);

	int delta = 0;

	TePointSet ptSet;
	TeLineSet lnSet;
	TePolygonSet pSet;

	while(TeReadBNAObjects(bnaFile, attTable, ptSet, lnSet, pSet, ncol, chunkSize, delta, linkIndex))
	{
		// save table
		if (!layer->saveAttributeTable( attTable ))
		{
			attTable.clear();
			break;
		}

		// save the remaining geometries
		if(ptSet.size() > 0)
		{
			layer->addPoints(ptSet); 
			ptSet.clear();
		}

		if(lnSet.size() > 0)
		{
			layer->addLines(lnSet);
			lnSet.clear();
		}

		if(pSet.size() > 0)
		{
			layer->addPolygons(pSet); 
			pSet.clear();
		}

		delta += attTable.size();
		
		attTable.clear();
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
	return true;
}


bool TeReadBNAAttributeList(const string& bnaFileName, TeAttributeList& attList)
{
	TeAsciiFile bnaFile(bnaFileName);

	if(!bnaFile.isNotAtEOF())
		 return false;

	vector<string> strList;

	bnaFile.readStringListCSV(strList, ',');

	unsigned int nFields = (strList.size() - 1u);

	if(nFields <= 0)
		return false;

	if(attList.size() > 0u)	// Has already defined attributes, only check the number
	{
		if(attList.size() != nFields)
			return false;
		else
			return true;
	}

	// In future, look for a better way to find the field name
	// see in the specification of file format there is something about the field names!

	TeAttribute at1;
	at1.rep_.type_ = TeSTRING;
	at1.rep_.numChar_ = 255;		
	at1.rep_.name_ = "IBGE_CODE";
	at1.rep_.isPrimaryKey_ = false;
	attList.push_back(at1);

	if(nFields > 1)
	{
		TeAttribute at2;
		at2.rep_.type_ = TeSTRING;
		at2.rep_.numChar_ = 255;		
		at2.rep_.name_ = "IBGE_NAME";
		at2.rep_.isPrimaryKey_ = false;
		attList.push_back(at2);
	}

	for(unsigned int i = 2; i < nFields; ++i)
	{
		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 255;		
		at.rep_.name_ = string("FIELD_") + Te2String(i);
		at.rep_.isPrimaryKey_ = false;
		attList.push_back(at);
	}

	return true;
}


bool TeReadBNAObjects(TeAsciiFile& bnaFile, TeTable& attTable, TePointSet& ptSet,
					  TeLineSet& lnSet, TePolygonSet& pSet, 
					  int ncolumns, int nrecords, int delta , int attLinkColumnIndex)
{
	if (!bnaFile.isNotAtEOF() || nrecords <=0 || ncolumns <= 0)
		return false;

	char separator = attTable.separator();

	int count;

	for(count=0; count < nrecords; ++count)
	{
		if(!bnaFile.isNotAtEOF())
			 break;

		// Ler os atributos do objeto e o tipo/tamanho da geometria

		TeTableRow row;
		for(int n = 0; n < ncolumns; ++n)
		{
			std::string value;

			try 
			{
				value = bnaFile.readStringCSVNoQuote(separator);
			}
			catch(...)
			{
				if(count > 0)
					return true;
				else
					return false;
			}

			row.push_back (value);
		}

		if(attLinkColumnIndex == row.size())
		{
			row.push_back(Te2String(delta));
		}
		attTable.add(row);

		string bnaGeometryTypeLen = bnaFile.readStringCSVNoQuote(separator);

		int bnaGTypeLen = atoi(bnaGeometryTypeLen.c_str());

		std::string index = row[attLinkColumnIndex];

		// Go to the geometry line begin
		bnaFile.findNewLine();

		if(bnaGTypeLen > 2)
		{
			if(!TeBNARegionDecode(bnaFile, pSet, index, bnaGTypeLen))
				return false;
		}
		else if(bnaGTypeLen < -1)
		{
			if(!TeBNALineDecode(bnaFile, lnSet, index, -bnaGTypeLen))
				return false;
		}
		else if(bnaGTypeLen == 1)
		{
			if(!TeBNAPointDecode(bnaFile, ptSet, index))
				return false;
		}
		else
			return false;

		++delta;
		bnaFile.findNewLine();
	}

	return true;
}

bool TeBNARegionDecode(TeAsciiFile& bnaFile, TePolygonSet& temp, string& index, const int& npoints)
{
	int i;
	unsigned int j;

	vector<TePolygon> pVec;
	TeLine2D l;

	TeCoord2D coordMainArea;
	TeCoord2D firstCoord;
	bool first = false;

	for(i = 0; i < npoints; ++i)
	{
		if(!bnaFile.isNotAtEOF())
			return false;

		double x = bnaFile.readFloat();
		bnaFile.readChar();
		double y = bnaFile.readFloat();

		//bnaFile.findNewLine();

		TeCoord2D c(x, y);

		if(i == 0)
		{
			coordMainArea.setXY(x, y);
			firstCoord = coordMainArea;
			first = false;
			l.add(c);
		}
		else
		{
			if(c == firstCoord)
			{
				l.add(c);
				l.objectId(index);
				TeLinearRing r(l);
				r.objectId(index);
				TePolygon p;
				p.add(r);
				p.objectId(index);

				if(pVec.size() == 0)
					pVec.push_back(p);
				else
				{
					for(j = 0; j < pVec.size(); ++j)
					{	
						TePolygon paux = 	pVec[j];				
						if(TeWithin(p, paux))
						{
							pVec[j].add(p[0]);
							break;
						}		
						
					}

					if(j == pVec.size())
						pVec.push_back(p);
				}

				// Make new line - Don't clear!
				l = TeLine2D();

				first = true;

			}
			else if(c == coordMainArea)
			{
				continue;
			}
			else
			{
				if(first)
				{
					firstCoord = c;
					first = false;
				}

				l.add(c);
			}
		}
	}

	for(unsigned int k = 0; k < pVec.size(); ++k)
		temp.add(pVec[k]);


	return true;
}

bool TeBNALineDecode(TeAsciiFile& bnaFile, TeLineSet& temp, string& index, const int& npoints)
{
	TeLine2D l;

	l.objectId(index);

	for(int i = 0; i < npoints; ++i)
	{
		if(!bnaFile.isNotAtEOF())
			return false;

		double x = bnaFile.readFloat();
		bnaFile.readChar();
		double y = bnaFile.readFloat();

		//bnaFile.findNewLine();

		TeCoord2D c(x, y);

		l.add(c);
	}

	temp.add(l);

	return true;
}

bool TeBNAPointDecode(TeAsciiFile& bnaFile, TePointSet& temp, string& index)
{
	if(!bnaFile.isNotAtEOF())
		return false;

	double x = bnaFile.readFloat();
	bnaFile.readChar();
	double y = bnaFile.readFloat();

	//bnaFile.findNewLine();

	TeCoord2D c(x, y);

	TePoint pt(c);

	pt.objectId(index);

	temp.add(pt);

	return true;
}
