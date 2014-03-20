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

#include "../kernel/TeException.h"
#include "TeDriverMIDMIF.h"
#include "../kernel/TeAsciiFile.h"

#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeQuerier.h"

int TeExportMIFLineSet ( TeAsciiFile& mifFile, TeLineSet& ls);
int TeExportMIFPointSet ( TeAsciiFile& mifFile, TePointSet& ps, bool isMulti=false);
int TeExportMIFPolygonSet ( TeAsciiFile& mifFile, TePolygonSet& ps);
int TeExportMIFCellSet (TeAsciiFile& mifFile, TeCellSet& cells, bool isMulti=false);
void TeExportMIFProjection ( TeProjection* proj, TeAsciiFile& mifFile);
void TeExportMIFAttributeList (TeAttributeList& attList, TeAsciiFile& mifFile);
void TeExportMIFRow(TeTableRow& row, TeAsciiFile& midFile);
TeProjInfo TeMIFProjectionInfo (const string& projName );


bool
TeExportQuerierToMIF(TeQuerier* querier, const std::string& base, TeProjection* proj)
{
	// check initial conditions
	if (!querier || base.empty())
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

	TeAsciiFile mifFile(base+".MIF", "w+");
	TeAsciiFile midFile(base+".MID", "w+");
	
	// write header;
	mifFile.writeString("Version 300\n");
	mifFile.writeString("Charset \"WindowsLatin1\" \n");
	mifFile.writeString("Delimiter \";\"\n");
	if (proj)
	{
		TeBox box = querier->getBox();
		if(box.isValid() == false)
		{
			if(querier->theme() != 0)
			{
				box=querier->theme()->box();
				if(box.isValid() == false)
				{
					if(querier->theme()->layer() != 0)
					{
						box=querier->theme()->layer()->box();
					}
				}
			}
		}
		char txt[300];
		sprintf(txt,"Bounds (%.5f, %5f) (%.5f, %5f)\n",box.x1_,box.y1_,box.x2_,box.y2_);
		if (proj->name() == "NoProjection")
		{
			mifFile.writeString("CoordSys Nonearth\n");
			mifFile.writeString("Units ");
			mifFile.writeString(proj->units());
			mifFile.writeNewLine();
			mifFile.writeString(string(txt));
		}
		else
		{
			TeExportMIFProjection(proj, mifFile);
			mifFile.writeString(" ");
			mifFile.writeString(string(txt));
		}
	}
	TeExportMIFAttributeList(qAttList,mifFile);
	mifFile.writeString("Data\n");

	TeTableRow row;
	TePolygonSet	objectPols;		// polygons of the region
	TeLineSet		objectLines;		// lines of the region
	TePointSet		objectPoints;	// points of the region
	TeCellSet		objectCells;	// points of the region

	// progress information
	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;
	unsigned int nIProcessed = 0;
	TeSTInstance st;
	while(querier->fetchInstance(st))
	{
		if (st.hasPolygons())
			objectPols = st.getPolygons();
		if (st.hasCells())
			objectCells = st.getCells();
		if (st.hasLines())
			objectLines = st.getLines();
		if (st.hasPoints())
			objectPoints = st.getPoints();
		
		int col = 0;
		if (objectPoints.size() > 0)
			col++;
		if (objectLines.size() > 0)
			col++;
		if (objectPols.size() > 0)
			col++;
		if (objectCells.size() > 0)
			col++;
		if (col == 0)
		{
			mifFile.writeString("NONE\n");
		}
		else
		{
			if (col > 1)	// it is a collection
				mifFile.writeString("Collection " + Te2String(col) + "\n");

			if (objectPols.size() > 0)
				TeExportMIFPolygonSet(mifFile,objectPols);

			if (objectLines.size() > 0)
				TeExportMIFLineSet(mifFile,objectLines);

			if (objectPoints.size() > 0)
				TeExportMIFPointSet(mifFile,objectPoints,(col>1));

			if (objectCells.size() > 0)
				TeExportMIFCellSet(mifFile,objectCells,(col>1));
				
			objectCells.clear();
			objectPols.clear();
			objectPoints.clear();
			objectLines.clear();
		}

		row.clear();	// export attributes
		if (onlyObjId)
		{
			row.push_back(st.objectId());
		}	
		else
		{
			unsigned int aa = 0;
			string val;
			for (aa=0; aa<qAttList.size(); ++aa)
			{
				st.getPropertyValue(val,aa);
				row.push_back(val);
			}
			TeExportMIFRow(row,midFile);
		}
		++nIProcessed;
		t2 = clock();
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
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}


bool 
TeExportThemeToMIF(TeTheme* theme, TeSelectedObjects selOb, const std::string& baseName)
{
	if (!theme)
		return false;

	string fbase = baseName;
	if (baseName.empty())
		fbase = theme->name();

	TeQuerierParams qPar(true, true);
	qPar.setParams(theme);
	qPar.setSelecetObjs(selOb);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToMIF(tQuerier, fbase, theme->layer()->projection());
	delete tQuerier;
	return res;
}

bool 
TeExportLayerToMIF(TeLayer* layer, const string& baseName)
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
	bool res = TeExportQuerierToMIF(tQuerier, baseName, layer->projection());
	delete tQuerier;
	delete tempTheme;
    return res ;
}

bool
TeExportMIF( TeLayer* layer, const string& mifFileName, const string& tableName, const string& restriction)
{
	if (!layer || mifFileName.empty() || tableName.empty())
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
	bool res = TeExportQuerierToMIF(tQuerier, mifFileName, layer->projection());
	delete tQuerier;
	delete tempTheme;
    return res ;
	return true;
}

void
TeExportMIFRow(TeTableRow& row, TeAsciiFile& midFile)
{
	TeTableRow::iterator it = row.begin();
	if (it != row.end())
	{
		midFile.writeString((*it));
		++it;
	}

	while ( it!= row.end())
	{
		midFile.writeString(";"+(*it));
		++it;
	}
	midFile.writeNewLine();
}

void
TeExportMIFProjection ( TeProjection* proj, TeAsciiFile& mifFile)
{
	if (!proj)
		return ;

	map<string,string> mifProjCode;
	mifProjCode["LatLong"] = "1";
	mifProjCode["LambertConformal"] = "3";
	mifProjCode["UTM"] = "8";
	mifProjCode["Albers"] = "9";
	mifProjCode["Mercator"] = "10";
	mifProjCode["Miller"] = "11";
	mifProjCode["Polyconic"] = "27";
	
	map<string,string> mifDatumCode;
	mifDatumCode["Spherical"]="0";
	mifDatumCode["Astro-Chua"]="23";
	mifDatumCode["CorregoAlegre"]="24";
	mifDatumCode["Indian"]="40";
	mifDatumCode["Indian"]="41";
	mifDatumCode["NAD27"]="62";
	mifDatumCode["NAD27"]="63";
	mifDatumCode["NAD27"]="64";
	mifDatumCode["NAD27"]="65";
	mifDatumCode["NAD27"]="66";
	mifDatumCode["NAD27"]="67";
	mifDatumCode["NAD27"]="68";
	mifDatumCode["NAD27"]="69";
	mifDatumCode["NAD27"]="70";
	mifDatumCode["NAD27"]="71";
	mifDatumCode["NAD27"]="72";
	mifDatumCode["NAD27"]="74";
	mifDatumCode["SAD69"]="92";
	mifDatumCode["WGS84"]="104";

	TeProjInfo pjInfo;
	try
	{
		pjInfo = TeMIFProjectionInfo (proj->name());
	}
	catch(...)
	{
		return;
	}

	mifFile.writeString("CoordSys Earth Projection ");
	mifFile.writeString(mifProjCode[proj->name()]);
	mifFile.writeString(", " + mifDatumCode[proj->datum().name()]);
	
	string mess;

	if (pjInfo.hasUnits)
	{
		if(TeConvertToUpperCase(proj->units()) == "METERS") mess=", \"m\"";
		else												mess = ", " + proj->units();
	}
		
	if ( pjInfo.hasLon0 )
	{
		if (!mess.empty())
			mess += ", ";
		mess +=  Te2String(proj->lon0()*TeCRD);
	}

	if ( pjInfo.hasLat0 )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->lat0()*TeCRD);
	}

	if ( pjInfo.hasStlat1 )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->stLat1()*TeCRD);
	}

	if ( pjInfo.hasStlat2 )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->stLat2()*TeCRD);
	}

	if ( pjInfo.hasScale )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->scale());
	}

	if ( pjInfo.hasOffx )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->offX());
	}

	if ( pjInfo.hasOffy )
	{
		if (!mess.empty())
			mess += ", ";
		mess += Te2String(proj->offY());
	}
	if (!mess.empty())
		mifFile.writeString(mess);
}

void
TeExportMIFAttributeList (TeAttributeList& attList, TeAsciiFile& mifFile)
{
	int n = attList.size();
	mifFile.writeString("Columns " + Te2String(n));
	mifFile.writeNewLine();

	TeAttributeList::iterator it = attList.begin();
	while ( it != attList.end())
	{
		TeAttribute att = *it;
		mifFile.writeString("  "+att.rep_.name_ + " ");
		if ( att.rep_.type_ == TeINT )
			mifFile.writeString("Integer");
		else if ( att.rep_.type_ == TeREAL )
			mifFile.writeString("Float");
		else if ( att.rep_.type_ == TeDATETIME )
			mifFile.writeString("Char(25)"); 
		else if ( att.rep_.type_ == TeSTRING )
			mifFile.writeString("Char(" + Te2String(att.rep_.numChar_) + ")"); 
		mifFile.writeNewLine();
		++it;
	}
}


int
TeExportMIFLineSet (TeAsciiFile& mifFile, TeLineSet& lines)
{
	if (lines.size() == 0)
		return 0;
	
	mifFile.writeString("Pline ");
	char aux[100];
	int n = lines.size();
	if (n > 1)
	{
		sprintf(aux,"Multiple %d\n",n);
		mifFile.writeString(string(aux));
	}
	for ( int i = 0; i < n; i++ )
	{
		TeLine2D line = lines[i];
		sprintf(aux,"%d\n",line.size());
		mifFile.writeString(string(aux));
		for (unsigned int k = 0; k < line.size(); k++ )
		{
			sprintf(aux,"%.6f  %.6f \n", line[k].x(),line[k].y());
			mifFile.writeString(string(aux));
		}
	}
	return lines.size();
}

int 
TeExportMIFPointSet (TeAsciiFile& mifFile, TePointSet& points, bool isMulti)
{
	if ( points.size() == 0 ) // layer has points
		return 0;

	int n = points.size();
	char aux[50];
	if (n == 1)
	{
		if (isMulti)
			mifFile.writeString("Multipoint 1\n");
		else
			mifFile.writeString("Point ");
		TePoint pt = points [0];
		TeCoord2D xy = pt.location();
		sprintf(aux,"%.6f  %.6f \n", xy.x(), xy.y());
		mifFile.writeString(string(aux));
	}
	else
	{
		mifFile.writeString("Multipoint ");
		sprintf(aux,"%d\n",n);
		mifFile.writeString(string(aux));
		unsigned int i;
		for ( i = 0; i < points.size(); i++ )
		{
			TePoint pt = points [i];
			TeCoord2D xy = pt.location();
			sprintf(aux,"%.6f  %.6f \n", xy.x(), xy.y());
			mifFile.writeString(string(aux));
		}
	}
	return points.size();
}
 
int
TeExportMIFPolygonSet (TeAsciiFile& mifFile, TePolygonSet& polygons)
{
	if (polygons.size() == 0)
		return 0;
	char aux[50];

	unsigned int nrings = 0;

	for(unsigned int j = 0; j < polygons.size(); ++j)
	{
		nrings += polygons[j].size();
	}

	sprintf(aux,"Region %d\n",nrings);
	mifFile.writeString(string(aux));
	unsigned int i;
	for (i = 0; i < polygons.size(); i++ )
	{
		TePolygon poly = polygons[i];
		unsigned int k;
		for (k = 0; k < poly.size(); k++ )
		{
			TeLinearRing ring = poly[k]; 
			sprintf(aux,"%d\n",ring.size());
			mifFile.writeString(string(aux));
			unsigned int l;
			for (l = 0 ; l < ring.size(); l++)
			{
				sprintf(aux,"%.6f  %.6f \n", ring[l].x(), ring[l].y());
				mifFile.writeString(string(aux));
			}
		}
	}
	return polygons.size();
}

int
TeExportMIFCellSet (TeAsciiFile& mifFile, TeCellSet& cells, bool isColl)
{
	if (cells.size() == 0)
		return 0;
	char aux[250];

	if (cells.size() > 1 || isColl)
	{
		sprintf(aux,"Region %d\n",cells.size());
		mifFile.writeString(string(aux));
		unsigned int i;
		for (i = 0; i < cells.size(); i++ )
		{
			TeCell cell = cells[i];
			TeCoord2D ll = cell.box().lowerLeft();
			TeCoord2D ur = cell.box().upperRight();
			mifFile.writeString("5");
			sprintf(aux,"%.6f  %.6f \n", ll.x(), ll.y());
			mifFile.writeString(string(aux));
			sprintf(aux,"%.6f  %.6f \n", ur.x(), ll.y());
			mifFile.writeString(string(aux));
			sprintf(aux,"%.6f  %.6f \n", ur.x(), ur.y());
			mifFile.writeString(string(aux));
			sprintf(aux,"%.6f  %.6f \n", ll.x(), ur.y());
			mifFile.writeString(string(aux));
			sprintf(aux,"%.6f  %.6f \n", ll.x(), ll.y());
			mifFile.writeString(string(aux));
		}
		return cells.size();
	}
	else
	{
		sprintf(aux,"Rect %.6f  %.6f %.6f  %.6f\n",
				cells[0].box().lowerLeft().x(), cells[0].box().lowerLeft().y(),
				cells[0].box().upperRight().x(), cells[0].box().upperRight().y());
		mifFile.writeString(string(aux));
		return 1;
	}
}
