/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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

#include "TeDriverSHPDBF.h"
#include "TeTable.h"
#include "TeException.h"
#include "TeUtils.h"
#include "TeLayer.h"
#include "TeProgress.h"
#include "TeDatabase.h"

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
