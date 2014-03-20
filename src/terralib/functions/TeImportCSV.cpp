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

#include "TeDriverCSV.h"
#include "../kernel/TeTable.h"
#include "../kernel/TeException.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeDatabase.h"
#include "../kernel/TeAsciiFile.h"


bool decoderCoordinate( const std::string &value, double &retValue )
{
	int								degree;
	int								minute;
	float							second;
	int								signal=1;
	int								status;
	basic_string <char>::size_type	index;
	std::string						strCoordinate;
	std::string						strFinalCoordinate;


	if(value.empty())	return false;

	strCoordinate=TeConvertToUpperCase(value);
	if((index=strCoordinate.find("O")) !=string::npos)	
	{
		signal=-1;
		strCoordinate.replace(index,1,"");
	}
	if((index=strCoordinate.find("W")) !=string::npos)	
	{
		signal=-1;
		strCoordinate.replace(index,1,"");
	}
	if((index=strCoordinate.find("S")) !=string::npos)	
	{
		signal=-1;
		strCoordinate.replace(index,1,"");
	}
	if((index=strCoordinate.find("-")) !=string::npos)	
	{
		signal=-1;
		strCoordinate.replace(index,1,"");
	}
	if(strCoordinate[0] == '"') strCoordinate.replace(index,1,"");


	for(unsigned i=0;i<strCoordinate.size();i++)
	{
		if(strCoordinate[i]==',')										strFinalCoordinate+=".";
		else if((strCoordinate[i]!=34) && (strCoordinate[i]!=39))		strFinalCoordinate+=strCoordinate[i];
	}


	status=sscanf(strFinalCoordinate.c_str(),"%d %d %f",&degree,&minute,&second);
	if(status!=3)	return false;
	retValue=abs(degree) + fabs(minute/60.0) + fabs(second/3600.0);
	retValue*=signal;
	return true;
}

inline bool TeReadCSVFile(const string& csvFileName, TeTable& attrTable, TeDatabase* db, TeLayer* layer, 
					 const int& attributeX, const int& attributeY, const char& separator, 
					 const bool& firstLineIsHeader, const unsigned int& chunkSize, 
					 const bool& createAutoNum)
{

	//Get the indexes. There is already an attribute autoNumber.
	int indexUniqueId, indexLink;
	indexUniqueId = indexLink = -1;
	indexLink = attrTable.attrLinkPosition();
	indexUniqueId = attrTable.attrUniquePosition();
	
	//insert the data from csvFile
	vector<string> row;
	TeAsciiFile* csvFile = new TeAsciiFile(csvFileName);

	unsigned int totalSteps = 0;
	while(csvFile->isNotAtEOF())
	{
		csvFile->findNewLine();
		++totalSteps;
	}

	csvFile->rewind();

	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(totalSteps);
		
	// discard header row;
	if (firstLineIsHeader && csvFile->isNotAtEOF())
	{
		csvFile->readStringListCSV(row,separator);
		csvFile->findNewLine();
		row.clear();
	}

	attrTable.clear();
	
	unsigned int nattr = attrTable.attributeList().size();
	unsigned int nn=0;
	vector<int> numCharsField;
	for (nn=0; nn<nattr; ++nn)
	{
		if (attrTable.attributeList()[nn].rep_.type_ == TeSTRING)
			numCharsField.push_back(attrTable.attributeList()[nn].rep_.numChar_);
		else
			numCharsField.push_back(-1);
	}
	if (createAutoNum)
		--nattr;

	unsigned int count = 0;
	bool endOfFile = false;
	while(!endOfFile)
	{
		unsigned int nrec = 0;

		TePointSet ps;
		//Gets chunkSize rows 
		while (nrec < chunkSize && csvFile->isNotAtEOF())
		{
			TeTableRow trow;
			csvFile->readStringListCSV(trow,separator);
			if (trow.size() < nattr)
			{
				if(csvFile->isNotAtEOF())
				{
					csvFile->findNewLine();
				}
        continue;
			}
			
			for (nn=0; nn<nattr;++nn)
			{
				if (numCharsField[nn] > -1)
				{
					string aux = trow[nn].substr(0,numCharsField[nn]);
					trow[nn] = aux;
				}
			}

			//if the first attribute is autonumber
			if(createAutoNum)
				trow.push_back (Te2String(count));
            
			//fill attributes 
			attrTable.add(trow);

			//fill points
			if(attributeX>=0 && attributeY>=0 && layer)
			{
				double x;
				double y;
				TePoint p;

				if(decoderCoordinate(trow[attributeX].c_str(),x) && decoderCoordinate(trow[attributeY].c_str(),y))
					p=TePoint(x,y);
				else 
					p=TePoint(atof(trow[attributeX].c_str()), atof(trow[attributeY].c_str()));

				p.objectId(trow[indexLink]);
				p.geomId(count);
				ps.add(p);
			}
			csvFile->findNewLine();
			trow.clear();
			++nrec;
			++count;
		}
		
		endOfFile = !csvFile->isNotAtEOF();
		//insert into attribute table
		if (!db->insertTable(attrTable))
		{
			if (TeProgress::instance())
				TeProgress::instance()->reset();
			return false;
		}
		
		//insert into point table
		if(attributeX>=0 && attributeY>=0 && layer)
		{
			if(!layer->addPoints(ps))
			{
				if (TeProgress::instance())
					TeProgress::instance()->reset();
				return false;
			}
		}
		attrTable.clear();
		ps.clear();

		if (TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
				break;
			else
				TeProgress::instance()->setProgress(count);
		}
	}
	if (csvFile)
	{
		delete csvFile;
		csvFile = 0; 
	}


	if (TeProgress::instance())
		TeProgress::instance()->reset();
	
	//insert metadata : te_layer_table
	int layerId = -1; //external table
	if(layer)
		layerId = layer->id();
	
	//attribute table metadata 
	if(!db->insertTableInfo(layerId, attrTable))
		return false;
	
	if(layer)
		layer->addAttributeTable(attrTable);

	//point table metadata 
	if(attributeX>=0 && attributeY>=0 && layer)
	{
		 if(!db->insertMetadata(layer->tableName(TePOINTS),db->getSpatialIdxColumn(TePOINTS), 0.0005,0.0005,layer->box()))
			return false;

		 if(!db->createSpatialIndex(layer->tableName(TePOINTS),db->getSpatialIdxColumn(TePOINTS), (TeSpatialIndexType)TeRTREE))
			return false;
	}
	return true;
}


bool TeImportCSVFile(const string& csvFileName, TeTable& attrTable, TeDatabase* db, TeLayer* layer, 
					 const char& separator, const bool& firstLineIsHeader,  const unsigned int& chunkSize, 
					 const bool& createAutoNum)
{
	//test database pointer
	if(!db)
		return false;
	
	//create attribute table
	if(!db->createTable(attrTable.name(), attrTable.attributeList()))
		return false;

	//insert data
	if(!TeReadCSVFile(csvFileName, attrTable, db, layer, -1, -1, separator, firstLineIsHeader, chunkSize, createAutoNum))
	{
		db->deleteTable(attrTable.name());
		return false;
	}
	return true;
}


bool TeImportCSVFile(const string& csvFileName, const int& attributeX, const int& attributeY, 
					 TeTable& attrTable, TeDatabase* db, TeLayer* layer, const char& separator, 
					 const bool& firstLineIsHeader,  const unsigned int& chunkSize,  const bool& createAutoNum)
{

	//test database pointer
	if(!db || !layer)
		return false;
	
	//create attribute table
	if(!db->createTable(attrTable.name(), attrTable.attributeList()))
		return false;

	//create geometry table
	if(!layer->addGeometry(TePOINTS))
		return false;
		
	//insert data
	if(!TeReadCSVFile(csvFileName, attrTable, db, layer, attributeX, attributeY, separator, firstLineIsHeader, chunkSize, createAutoNum))
	{
		db->deleteTable(attrTable.name());
		return false;
	}
	return true;
}


