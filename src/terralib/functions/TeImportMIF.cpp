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

const int MAXCHARNUM = 66560;

using namespace std;

#include "../kernel/TeTable.h"
#include "../kernel/TeGeometry.h"
#include "../kernel/TeAsciiFile.h"
#include "TeMIFProjection.h"
#include "../kernel/TeException.h"
#include "../kernel/TeProjection.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeTable.h"
#include "../kernel/TeAssertions.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeDatabase.h"

#include "TeDriverMIDMIF.h"

void TeMIFRegionDecode ( TeAsciiFile& mifFile, TePolygonSet& temp, string& index);

void TeMIFLineDecode (TeAsciiFile& mifFile, TeLineSet& temp, string& index);

void TeMIFPlineDecode (TeAsciiFile& mifFile, TeLineSet& temp, string& index);

void TeMIFPointDecode (TeAsciiFile& mifFile, TePointSet& temp, string& index);

void TeMIFMultiPointDecode (TeAsciiFile& mifFile, TePointSet& temp, string& index);

void TeMIFTextDecode (TeAsciiFile& mifFile, TeTextSet& temp, string& index);

void TeMIFCollectionDecode(TeAsciiFile& mifFile, TePointSet& ps, TeLineSet& ls, TePolygonSet& pols, string& index);

void TeMIFRectDecode (TeAsciiFile& mifFile, TePolygonSet& temp, string& index);

void TeMIFOthersDecode    ( TeAsciiFile& );

void TeMIFTransformDecode ( TeAsciiFile& );

bool TeReadMIFAttributes(TeAsciiFile& midFile, TeTable& attTable, 
						 int ncolumns, int nrecords=-1, int delta = 0, bool createAuto=false);

bool TeImportMIFGeometries(TeLayer* layer,const string& mifFileName, 
						   vector<string> &indexes, unsigned int chunckSize);

char TeReadMIFSeparator(const string& mifFileName);

double glXmulti = 1., glYmulti = 1;

TeCoord2D glAdd (0., 0. );


TeLayer* TeImportMIF(const string& mifFileName, TeDatabase* db, const string& layerName)
{
	if (!db || mifFileName.empty()) 
		return 0;

	// check if format is complete (MID and MIF files exist)
	string filePrefix = TeGetName(mifFileName.c_str());
	string ftest = filePrefix + ".mid";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".MID";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	ftest = filePrefix + ".mif";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".MIF";
		if (access(ftest.c_str(),04) == -1)
			return 0;
	}

	// found a valid layer name
	string lName;
	string baseName = TeGetBaseName(mifFileName.c_str());
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
	TeProjection* mifProj = TeReadMIFProjection(ftest);
	TeLayer* newLayer = new TeLayer(newLayerName,db,mifProj);
	if (newLayer->id() <= 0 )
		return 0;				// couldn't create new layer

    // check if format is complete
	bool res = TeImportMIF(newLayer,ftest);
	delete mifProj;
	if (res)
		return newLayer;        // create new layer successfully, is complete.
	else
	{
		db->deleteLayer(newLayer->id());
		delete newLayer;        // created new layer is not complete
		return 0;
	}
}

bool 
TeImportMIF(TeLayer* layer, const string& mifFileName, string attrTableName, 
			string objectIdAttr, unsigned int chunkSize)
{
	// check if format is complete (MID and MIF files exist)
	string filePrefix = TeGetName(mifFileName.c_str());
	string ftest = filePrefix + ".mid";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".MID";
		if (access(ftest.c_str(),04) == -1)
			return false;
	}
	string midfilename = ftest;

	ftest = filePrefix + ".mif";
	if (access(ftest.c_str(),04) == -1)
	{
		ftest = filePrefix + ".MIF";
		if (access(ftest.c_str(),04) == -1)
			return false;
	}
	string miffilename = ftest;
	

	//-- Step 1: Read any adicional information (ex. projection) 
	if (!layer->projection())
	{
		TeProjection* mifProj = TeReadMIFProjection(miffilename);
		if (mifProj)
			layer->setProjection(mifProj);
	}

	char separator = TeReadMIFSeparator(miffilename);

	//read the attribute list information 
	TeAttributeList attList;
	TeReadMIFAttributeList(miffilename, attList);

	// define a TeAttributeTable
	if (attrTableName.empty())
	{
		if (layer->name().empty())
			return false;
		else
			attrTableName = layer->name();
	}
	string aux = attrTableName;
	int c = 1;
	while (layer->database()->tableExist(aux))
	{
		aux = attrTableName + "_" + Te2String(c);
		++c;
	}
	attrTableName = aux;
	
	int linkCol=-1, j=0;
	bool autoIndex = false;

	// if no geometry link name is given, creates one called 'object_id'
	if (objectIdAttr.empty())
	{
		objectIdAttr = "object_id_"+Te2String(layer->id());
		TeAttribute at;
		at.rep_.type_ = TeSTRING;
		at.rep_.numChar_ = 16;
		at.rep_.name_ = objectIdAttr;
		at.rep_.isPrimaryKey_ = true;
		attList.push_back(at);
		autoIndex = true;
	}
	else
	{
		// check if given index is valid
		TeAttributeList::iterator it = attList.begin();
		while (it != attList.end())
		{
			if (TeConvertToUpperCase((*it).rep_.name_) == TeConvertToUpperCase(objectIdAttr))		// index found
			{
				if ((*it).rep_.type_ != TeSTRING)	// make sure it is a String type
				{
					(*it).rep_.type_ = TeSTRING;
					(*it).rep_.numChar_ = 16;
				}
				linkCol=j;
				(*it).rep_.isPrimaryKey_ = true;
				break;
			}
			it++;
			j++;
		}
		if (it == attList.end())			// index not found
		{
			TeAttribute at;
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_ = 16;
			at.rep_.name_ = objectIdAttr;
			at.rep_.isPrimaryKey_ = true;
			attList.push_back(at);
			autoIndex = true;
		}
	}

	if (autoIndex)
		linkCol = attList.size()-1;

	TeTable attTable (attrTableName,attList,objectIdAttr,objectIdAttr,TeAttrStatic);
	attTable.setSeparator(separator);

	// insert the table into the database
	if (!layer->createAttributeTable(attTable))
		return false;

	vector<string> objectIds;
	int ncol = attList.size();
	if (autoIndex)
		ncol--;

	TeAsciiFile midFile(midfilename);
	int delta = 0;
	while (TeReadMIFAttributes(midFile,attTable,ncol,chunkSize,delta, autoIndex))
	{
		for (unsigned int n=0; n<attTable.size();++n)
			objectIds.push_back(attTable.operator ()(n,linkCol));

		// save table
		if (!layer->saveAttributeTable( attTable ))
		{
			attTable.clear();
			break;
		}
		delta += attTable.size();
		attTable.clear();
	}

	// Import the geometries
	bool res = TeImportMIFGeometries(layer,miffilename,objectIds,chunkSize);
	
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
	return res;
}

bool TeReadMIFAttributes(TeAsciiFile& midFile, TeTable& attTable, 
						 int ncolumns, int nrecords, int delta, bool createAuto )
{
	if (!midFile.isNotAtEOF() || nrecords <=0 || ncolumns <= 0)
		return false;
	char separator = attTable.separator();

	int count;
	for (count=0; count<nrecords; count++)
	{
		if (!midFile.isNotAtEOF())
			 break;

		TeTableRow row;
		for (int n=0; n<ncolumns; n++)
		{
			string value;
			try 
			{
				value = midFile.readStringCSVNoQuote(separator);
			}
			catch(...)
			{
				if (count > 0)
					return true;
				else
					return false;
			}
			row.push_back ( value ); 
		}

		if (createAuto)
			row.push_back(Te2String(count+delta));

		attTable.add(row);
		midFile.findNewLine();
	}
	return true;
}

char 
TeReadMIFSeparator(const string& mifFileName)
{
	TeAsciiFile mifFile(mifFileName);
	string name = mifFile.readString ();

	while ( mifFile.isNotAtEOF() && !TeStringCompare(name,"Delimiter"))
		name = mifFile.readString ();

	if (mifFile.isNotAtEOF())
		return mifFile.readQuotedChar();
	else 
		return '\t';
}


TeProjection* 
TeReadMIFProjection(const string& mifFileName)
{
	TeProjection* proj = 0;

	TeAsciiFile mifFile(mifFileName);
	string name = mifFile.readString ();

	while ( mifFile.isNotAtEOF() && !TeStringCompare(name,"CoordSys")
		                         && !TeStringCompare(name,"Columns"))
		name = mifFile.readString ();
	
	if (mifFile.isNotAtEOF() && TeStringCompare(name,"CoordSys"))  
	{
		string option = mifFile.readString();

		vector<string> argList;

		if (TeStringCompare(option,"Earth"))	// expect Projection clause
		{
			string projRef =  mifFile.readString ();
			if (!TeStringCompare(projRef,"Projection"))
			{
				proj = new TeNoProjection();
				return proj;
			}
			mifFile.readStringListCSV(argList,',');
			proj = TeMIFProjectionFactory::make ( argList );	// create projection from parameters
		}
		else if (TeStringCompare(option,"Nonearth"))	// non earth
		{
			// look for units specification
			while (mifFile.isNotAtEOF() && !TeStringCompare(name,"Units")
		                                && !TeStringCompare(name,"Columns"))
				name = mifFile.readString ();
			if (TeStringCompare(name,"Units"))
			{
				name = mifFile.readString();
				proj = new TeNoProjection(name);
			}
			else
				proj = new TeNoProjection(name);
		}
	}
	else	// no CoordSys clause specified 
	{		// it should be assumed that the data is stored in Lat Long
		TeDatum  datum = TeDatumFactory::make("Spherical");
		proj = new TeLatLong(datum);
	}
	return proj;
}

void
TeReadMIFAttributeList ( const string& mifFileName, TeAttributeList& attList )
{
	TeAsciiFile mifFile(mifFileName);
	string name = mifFile.readString ();
	
	while ( mifFile.isNotAtEOF() && !TeStringCompare(name,"Columns"))
		name = mifFile.readString ();

	if (mifFile.isNotAtEOF()) 
	{
		int ncol = mifFile.readInt();
		mifFile.findNewLine();
		TeAttribute attribute;

		for ( int i = 0; i< ncol; i++ )
		{
			attribute.rep_.name_  = mifFile.readString();
			string attType = mifFile.readStringCSVNoSpace ('(');
			if ( TeStringCompare(attType,"Integer") || TeStringCompare(attType,"Smallint") 
				 || TeStringCompare(attType,"Logical") )
			{
				attribute.rep_.type_ = TeINT;
			}
			else if (TeStringCompare(attType,"Float"))
			{
				attribute.rep_.type_ = TeREAL;
				attribute.rep_.numChar_ = 20;
				attribute.rep_.decimals_ = 1;
			}
			else if (TeStringCompare(attType,"Date"))
			{
				attribute.rep_.type_ = TeDATETIME;
				attribute.dateTimeFormat_="YYYYMMDD";
			}
			else if (TeStringCompare(attType,"Decimal"))
			{
				attribute.rep_.type_  = TeREAL;
				attribute.rep_.numChar_ = mifFile.readIntCSV();
				attribute.rep_.decimals_ = mifFile.readIntCSV(')');
			}
			else
			{
				attribute.rep_.type_  = TeSTRING;
				attribute.rep_.numChar_  = mifFile.readIntCSV(')');
				if (attribute.rep_.numChar_ > MAXCHARNUM)
				{
					attribute.rep_.numChar_ = MAXCHARNUM;
				}
			}
			attList.push_back ( attribute );
			mifFile.findNewLine();
		}
	}
}

bool
TeImportMIFGeometries(TeLayer* layer,const string& mifFileName, 
					  vector<string> &indexes, unsigned int chunckSize)
{
	TePointSet pointSet;
	TeLineSet lineSet;
	TePolygonSet polySet;
	TeTextSet textSet;
	string textTableName = layer->name() + "Texto";

	double oldPrec = TePrecision::instance().precision();
	try
	{
		if(layer)
		{
			TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
		}
		else
		{
			TePrecision::instance().setPrecision(TeGetPrecision(0));
		}

		TeAsciiFile mifFile(mifFileName);
		int n = 0;
		while ( mifFile.isNotAtEOF()  )
		{
			string name = mifFile.readString ();
			if (TeStringCompare(name,"Region"))
			{
				TeMIFRegionDecode ( mifFile, polySet, indexes[n]);
				n++;
				if ( polySet.size() == chunckSize )
				{
					layer->addPolygons( polySet ); 
					polySet.clear();
				}
			} 
			else if (TeStringCompare(name,"Pline"))
			{
				TeMIFPlineDecode ( mifFile, lineSet, indexes[n] );
				n++;
				if ( lineSet.size() == chunckSize )
				{
					layer->addLines( lineSet ); 
					lineSet.clear();
				}
			}
			else if (TeStringCompare(name,"Line"))
			{
				TeMIFLineDecode ( mifFile, lineSet, indexes[n] );
				n++;
				if ( lineSet.size() == chunckSize )
				{
					layer->addLines( lineSet ); 
					lineSet.clear();
				}
			}
			else if (TeStringCompare(name,"Point"))
			{
				TeMIFPointDecode ( mifFile, pointSet, indexes[n] );
				n++;
				if ( pointSet.size() == chunckSize )
				{
					layer->addPoints( pointSet ); 
					pointSet.clear();
				}
			}
			else if (TeStringCompare(name,"Multipoint"))
			{
				TeMIFMultiPointDecode ( mifFile, pointSet, indexes[n] );
				n++;
				if ( pointSet.size() == chunckSize )
				{
					layer->addPoints( pointSet ); 
					pointSet.clear();
				}
			}
			else if (TeStringCompare(name,"Text"))
			{
				TeMIFTextDecode ( mifFile, textSet,indexes[n]);
				n++;
				if ( textSet.size() == chunckSize )
				{
					layer->addText( textSet,textTableName ); 
					textSet.clear();
				}
			}
			else if (TeStringCompare(name,"Rect"))
			{
				TeMIFRectDecode(mifFile,polySet,indexes[n]);
				n++;
				if (polySet.size() == chunckSize )
				{
					layer->addPolygons(polySet); 
					polySet.clear();
				}
			}
			else if (TeStringCompare(name,"Collection"))
			{
				TeMIFCollectionDecode(mifFile,pointSet,lineSet, polySet,indexes[n]);
				n++;
				if (lineSet.size() == chunckSize )
				{
					layer->addLines(lineSet); 
					lineSet.clear();
				}
				if (polySet.size() == chunckSize )
				{
					layer->addPolygons(polySet); 
					polySet.clear();
				}
				if (pointSet.size() == chunckSize )
				{
					layer->addPoints(pointSet); 
					pointSet.clear();
				}
			}
			else if (TeStringCompare(name,"Transform"))
				TeMIFTransformDecode ( mifFile );			
			else if (TeStringCompare(name,"NONE"))
				n++;
			else
				TeMIFOthersDecode ( mifFile );
		}
		
		// save the remaining geometries
		if (pointSet.size() > 0 )
		{
			layer->addPoints( pointSet ); 
			pointSet.clear();
		}

		if (lineSet.size() > 0 )
		{
			layer->addLines( lineSet ); 
			lineSet.clear();
		}

		if (polySet.size() > 0 )
		{
			layer->addPolygons( polySet ); 
			polySet.clear();
		}
		
		if (textSet.size() > 0)
		{
			layer->addText( textSet,textTableName ); 
			textSet.clear();
		}
	}
	catch(...)
	{
		TePrecision::instance().setPrecision(oldPrec);
		return false;
	}

	TePrecision::instance().setPrecision(oldPrec);
	return true;
}

void 
TeMIFRectDecode (TeAsciiFile& mifFile, TePolygonSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";
	TeCoord2D ll = mifFile.readCoord2D();
	TeCoord2D ur = mifFile.readCoord2D();

	TeBox rec(ll.x_,ll.y_,ur.x_,ur.y_);
	TePolygon pol = polygonFromBox(rec);
	pol.objectId(index);
	temp.add(pol);
}

void
TeMIFRegionDecode ( TeAsciiFile& mifFile, TePolygonSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";

	int npolyg = mifFile.readInt();
	mifFile.findNewLine();

	TeCoord2D pt;
	TePolygon pol; 
	typedef list<TePolygon> PolyList;
	PolyList pList;
	for ( int i = 0; i < npolyg; i++ )
	{
		TeLine2D line;

		int ncoord = mifFile.readInt();
		mifFile.findNewLine();

		for ( int j = 0; j < ncoord; j++ )
		{
			pt = mifFile.readCoord2D();
			pt.scale( glXmulti, glYmulti);
			pt += glAdd;
			mifFile.findNewLine();
			line.add ( pt );

		}
		if ( line.size() <= 3 ) continue; // to avoid dangling rings
		if ( !line.isRing() )		//close the ring
		{
			line.add(line[0]);
//			throw TeException ( MIF_REGION_CLOSE );
		}

		TeLinearRing ring (line);
		
		bool inside = false;

		PolyList::iterator it = pList.begin();

		while ( it != pList.end() )
		{
			TePolygon p1;
			p1.add(ring);
			TePolygon p2 = (*it);
			short rel = TeRelation(p1, p2);

			if((rel & TeWITHIN) || (rel & TeCOVEREDBY))
			{
				inside = true;
				ring.objectId(p2[0].objectId());
				p2.add ( ring );// add a closed region
			}
			++it;
		}
		if ( !inside )
		{
			TePolygon poly;
			ring.objectId(index);
			poly.add ( ring ); // add an outer region
			poly.objectId(index);
			pList.push_back ( poly );
		}
	}

	PolyList::iterator it = pList.begin();
	while ( it != pList.end() )
	{
		temp.add ( *it ); // add a polygon to a layer
		++it;
	}
}	

void
TeMIFLineDecode (TeAsciiFile& mifFile, TeLineSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";

	TeLine2D line;
	TeCoord2D pt;

	for ( int i= 0; i < 2; i++ )
	{
		pt = mifFile.readCoord2D();
		pt.scale( glXmulti, glYmulti);
		pt += glAdd;
		line.add ( pt );
	}

	line.objectId( index );
	temp.add ( line );
	mifFile.findNewLine();
}

void
TeMIFPlineDecode (TeAsciiFile& mifFile, TeLineSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";

	int numSections = 1; 
	int ncoord = 0;

	// Read the Pline parameters
	vector<string>  strList;
	mifFile.readStringList ( strList );
	
	// Are we dealing with MULTIPLE sections ??
	// If there are two parameters for the pline,
	// the first MUST be a "Multiple" keyword
	
	bool hasMultiple = false;

	if ( strList.size() == 2 )
	{
		hasMultiple = true;
		ensure (TeStringCompare(strList[0],"Multiple"));
		numSections = atoi ( strList[1].c_str() );
	}
	else
		ncoord = atoi ( strList[0].c_str());

	mifFile.findNewLine(); // go to the next line
	// If there is a single PLine, there is a single
	// section ( numSections = 1 ) and will read
	// the number of points within the loop


	for ( int i= 0;  i < numSections; i++ )
	{
		TeLine2D line;	
		TeCoord2D pt;
		if ( hasMultiple == true )
		{
			ncoord = mifFile.readInt();
			mifFile.findNewLine(); // go to the next line
		}

		for ( int j = 0; j < ncoord; j++ )
		{
			pt = mifFile.readCoord2D();
			pt.scale( glXmulti, glYmulti);
			pt += glAdd;
			mifFile.findNewLine();
			line.add ( pt );
		}
		line.objectId( index );
		temp.add ( line );
	}
}

void
TeMIFPointDecode (TeAsciiFile& mifFile, TePointSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";

	TePoint point;
	
	// Read the coordinates

	TeCoord2D pt = mifFile.readCoord2D();
	pt.scale( glXmulti, glYmulti);
	pt += glAdd;
	point.add ( pt );
	point.objectId( index );

	// Add a point to the Point Set
			
	temp.add ( point );

	mifFile.findNewLine();
}

void
TeMIFMultiPointDecode (TeAsciiFile& mifFile, TePointSet& temp, string& index)
{
	if (index.empty())
		index = "te_nulo";

	int	npts = mifFile.readInt();
	mifFile.findNewLine();
	for (int i=0; i<npts;i++)
	{
		// Read points
		TePoint point;
		TeCoord2D pt = mifFile.readCoord2D();
		pt.scale( glXmulti, glYmulti);
		pt += glAdd;
		point.add ( pt );
		point.objectId( index );

		// Add a point to the Point Set		
		temp.add ( point );
	}
	mifFile.findNewLine();
}

void
TeMIFTransformDecode ( TeAsciiFile& mifFile )
{
	// read the tansformation params

	double param = mifFile.readFloatCSV();

	if ( param != 0. )
		glXmulti = param;

	param = mifFile.readFloatCSV();

	if ( param != 0. )
		glYmulti = param;

	double x = mifFile.readFloatCSV();

	double y  = mifFile.readFloatCSV();

	TeCoord2D pt ( x, y );

	glAdd =  pt;
	
	// Go to the next line
	mifFile.findNewLine ();
	
}

void
TeMIFTextDecode (TeAsciiFile& mifFile, TeTextSet& temp, string& index)
{
	std::string		strFont;
	double	angle=0;
	string txt = mifFile.readQuotedString(); // read the text string
	mifFile.findNewLine ();
	TeBox box = mifFile.readBox();			// read the text box 
	
	strFont=mifFile.readFont();
	angle=mifFile.readAngle(strFont);

	TeCoord2D lowerLeft = box.lowerLeft();
	TeText text(lowerLeft,txt);
	text.setBox(box);
	text.setAngle(angle);
	text.objectId(index);
	text.setHeight(box.height());
	temp.add(text);
	//mifFile.findNewLine ();					// go to the next line
}

void
TeMIFOthersDecode ( TeAsciiFile& mifFile )
{
	// Simply go to the next line
	mifFile.findNewLine ();
}

void 
TeMIFCollectionDecode(TeAsciiFile& mifFile, TePointSet& ps, TeLineSet& ls, TePolygonSet& pols, string& index)
{
	int	nparts = mifFile.readInt();
	if (nparts == 0)
		nparts = 3;
	mifFile.findNewLine();
	for (int i=0; i<nparts; i++)
	{
		string part = mifFile.readString ();
		if (TeStringCompare(part,"Region"))
			TeMIFRegionDecode (mifFile, pols, index);
		else if(TeStringCompare(part,"Pline"))
			TeMIFPlineDecode (mifFile, ls, index);
		else if (TeStringCompare(part,"Multipoint"))
			TeMIFMultiPointDecode (mifFile,ps, index);
	}
}
