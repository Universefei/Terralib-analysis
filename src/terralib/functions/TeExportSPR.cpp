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
#include "TeDriverSPRING.h"
#include "TeSPRFile.h"

#include "../kernel/TeDataTypes.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeTable.h"
#include "../kernel/TeGeometry.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeDatabase.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeQuerier.h"
#include <time.h>
#include <stdio.h>

// --- Spring

bool TeExportQuerierToSPRING(TeQuerier* querier, const std::string& base, TeSpringModels cat, const string& name, 
							 TeProjection* proj)
{
	// check initial conditions
	if (!querier || base.empty())
		return false;

	if (!querier->loadInstances())
		return false;
	
	// try to open the files
	std::string baseFileName = TeGetName(base.c_str());
	TeSPRFile* sprFileSamp = 0;
	TeSPRFile* sprFileLin = 0;
	TeSPRFile* sprFilePt = 0;
	TeSPRFile* sprFileLab = 0;
	TeSPRFile* sprFileTab = 0;
	TeSPRFile* sprFileNet = 0;
	TeSPRFile* sprFileNetObj = 0;
	try
	{
		sprFileSamp = new TeSPRFile(baseFileName+"_L3D.spr", "w+");
		sprFileLin = new TeSPRFile(baseFileName+"_L2D.spr", "w+");
		sprFileLab = new TeSPRFile(baseFileName+"_LAB.spr", "w+");
		sprFilePt = new TeSPRFile(baseFileName+"_P2D.spr", "w+");
		sprFileTab = new TeSPRFile(baseFileName+"_TAB.spr", "w+");
		sprFileNet = new TeSPRFile(baseFileName+"_NET.spr", "w+");
		sprFileNetObj = new TeSPRFile(baseFileName+"_NETOBJ.spr", "w+");
	}
	catch (...)
	{
		return false;
	}

	// verify if the specific column is present
	unsigned int qpos = 0;
	unsigned int aa = 0;
	unsigned int nattr;
	TeAttributeList qAttList = querier->getAttrList();
	nattr = qAttList.size();
	for (aa=0; aa<nattr; ++aa)
	{
		if (qAttList[aa].rep_.name_ == name)
		{
			qpos = aa;
			break;
		}
	}	
	if (cat != TeSPRNetwork && cat != TeSPRThematic && qpos == nattr)
		return false;	
	
	// write headers
	sprFileNet->writeString("NETWORK\n");
	sprFileSamp->writeString("SAMPLE\n");
	sprFileLin->writeString("LINES\n");
	sprFilePt->writeString("POINT2D\n");
	sprFileLab->writeString("POINTS\n");
	sprFileNetObj->writeString("NETWORK_OBJECTS\n");

	sprFileNet->writeString("INFO\n");
	sprFileSamp->writeString("INFO\n");
	sprFileLin->writeString("INFO\n");
	sprFilePt->writeString("INFO\n");
	sprFileLab->writeString("INFO\n");	
	sprFileNetObj->writeString("INFO\n");
	if (proj)
	{
		string projs = "// " + proj->name() + " " + proj->describe() + " DATUM "+ proj->datum().name() + "\n";
		sprFileSamp->writeString(projs);
		sprFileLin->writeString(projs);
		sprFilePt->writeString(projs);
		sprFileLab->writeString(projs);
		sprFileNet->writeString(projs);
		sprFileNetObj->writeString(projs);
	}
	TeBox bb = querier->getBox();
	if (bb.isValid())
	{
		sprFileSamp->writeBox(bb);
		sprFileLin->writeBox(bb);
		sprFilePt->writeBox(bb);
		sprFileLab->writeBox(bb);
		sprFileNet->writeBox(bb);
		sprFileNetObj->writeBox(bb);
	}
	sprFileLab->writeString("SEPARATOR ;\n");
	sprFileNetObj->writeString("SEPARATOR ;\n");
	if (cat == TeSPRThematic || cat == TeSPRNetwork || cat == TeSPRCadastral)
		sprFilePt->writeString("SEPARATOR ;\n");

	sprFileSamp->writeInfoEnd();
	sprFileLin->writeInfoEnd();
	sprFilePt->writeInfoEnd();
	sprFileLab->writeInfoEnd();
	sprFileNet->writeInfoEnd();
	sprFileNetObj->writeInfoEnd();

	if (cat == TeSPRNetwork || cat == TeSPRCadastral)
	{
		sprFileTab->writeTableHeader(cat);
		sprFileTab->writeString("CATEGORY_OBJ "); 	
		sprFileTab->writeString(name); 
		sprFileTab->writeString("\n"); 
		sprFileTab->writeString("T_KEY ; TEXT; 50; 0\n");
		sprFileTab->writeString("T_NAME ; TEXT; 50; 0\n");
		TeAttributeList::iterator it = qAttList.begin();
		for ( ;it != qAttList.end();++it )
		{
			switch ((*it).rep_.type_)
			{
				case TeSTRING:
					sprFileTab->writeTextParameterInfo((*it).rep_.name_, (*it).rep_.numChar_);
					break;
				case TeREAL:
					sprFileTab->writeRealParameterInfo((*it).rep_.name_);
					break;
				case TeINT:
					sprFileTab->writeIntParameterInfo((*it).rep_.name_);
					break;
				case TeCHARACTER:
					sprFileTab->writeTextParameterInfo((*it).rep_.name_,1);
					break;
				case TeDATETIME:
					sprFileTab->writeTextParameterInfo((*it).rep_.name_,25);
				default:
					break;
			}
		}  
		sprFileTab->writeInfoEnd();
	}
    
	// variaveis auxiliares
	TePolygonSet	objectPols;		// polygons of the region
	TeLineSet		objectLines;	// lines of the region
	TePointSet		objectPoints;	// points of the region
	TeCellSet		objectCells;	// points of the region
	unsigned int nlines2d=0, npoints2d=0, nsamps=0, nlabs=0, ntabs=0, nnetlines=1, nnetlabs=0;

	bool writePointHeader = true;
	bool endPointHeader = false;
	unsigned int k, l;
	unsigned int nIProcessed = 0;
	TeSTInstance st;
	string val;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;
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

		if (cat == TeSPRMNT ) 
		{
			st.getPropertyValue(val,qpos);
			if (!objectLines.empty() && endPointHeader)
			{
				sprFileSamp->writeEnd();
				endPointHeader = false;
			}

			for (aa=0; aa<objectLines.size(); ++aa)
			{
				sprFileSamp->writeLine3DHeader();
				sprFileSamp->writeString("HEIGHT ");
				sprFileSamp->writeString(val);
				sprFileSamp->writeString("\n");
				for (l=0; l<objectLines[aa].size(); ++l)
					sprFileSamp->writePoint(objectLines[aa][l]);
				sprFileSamp->writeEnd();
				++nsamps;
			}
			if (!objectPoints.empty() && writePointHeader)
			{
				sprFileSamp->writeString("POINT3D\n");
				writePointHeader = false;
				endPointHeader = true;
			}
			for (aa=0; aa<objectPoints.size(); ++aa)
			{
				sprFileSamp->writePoint3D(objectPoints[aa].location().x(), objectPoints[aa].location().y(), atof(val.c_str()));
				++nsamps;
			}
		}
		else if (cat == TeSPRThematic)
		{
			st.getPropertyValue(val,qpos);
			for (aa=0; aa<objectLines.size(); ++aa)
			{
				for (l=0; l<objectLines[aa].size(); ++l)
					sprFileLin->writePoint(objectLines[aa][l]);
				sprFileLin->writeString("END\n");	
				TeCoord2D pt = TeFindCentroid(objectLines[aa]);
				sprFileLab->writeThematicLabel(pt.x(), pt.y(),val);
				++nlines2d;
				++nlabs;
			}
			for (aa=0; aa<objectPols.size(); ++aa)
			{
				for (l=0; l<objectPols[aa].size(); ++l)
				{
					for (k=0; k<objectPols[aa][l].size(); ++k)
						sprFileLin->writePoint(objectPols[aa][l][k]);
					sprFileLin->writeString("END\n");
				}
				TeCoord2D pt = TeFindCentroid(objectPols[aa]);
				sprFileLab->writeThematicLabel(pt.x(),pt.y(),val);
				++nlines2d;
				++nlabs;
			}
			for (aa=0; aa<objectPoints.size(); ++aa)
			{
				sprFilePt->writeThematicLabel(objectPoints[aa].location().x(), objectPoints[aa].location().y(),val);
				++npoints2d;
			}

			for (aa=0; aa<objectCells.size(); ++aa)
			{
				TeCoord2D ll = objectCells[aa].box().lowerLeft();
				TeCoord2D ur = objectCells[aa].box().upperRight();
				sprFileLin->writePoint (ll);
                TeCoord2D p1(ur.x(), ll.y());
				sprFileLin->writePoint (p1);
				sprFileLin->writePoint (ur);
                TeCoord2D p2(ll.x(),ur.y());                   
				sprFileLin->writePoint (p2);
				sprFileLin->writePoint (ll);
				sprFileLin->writeEnd();
				TeCoord2D pt = TeFindCentroid(objectCells[aa]);
				sprFileLab->writeThematicLabel(pt.x(),pt.y(),val);
				++nlines2d;
				++nlabs;
			}
		}
		else if (cat == TeSPRNetwork)
		{
			for (aa=0; aa<objectLines.size(); ++aa)
			{
				sprFileNet->writeNetworkLineHeader(nnetlines);
				for (l=0; l<objectLines[aa].size(); ++l)
					sprFileNet->writePoint(objectLines[aa][l]);
				sprFileNet->writeString("END\n");	
				TeCoord2D pt = TeFindCentroid(objectLines[aa]);
				sprFileNetObj->writeNetworkLabel(st.objectId(),pt.x(), pt.y(),name);
				++nnetlines;
				++nnetlabs;
			}
			
			for (aa=0; aa<objectPoints.size(); ++aa)
			{
				sprFileNetObj->writeNetworkLabel(st.objectId(),objectPoints[aa].location().x(), objectPoints[aa].location().y(),name,false);
				++nnetlabs;
			}
		}
		else 
		{
			for (aa=0; aa<objectLines.size(); ++aa)
			{
				for (l=0; l<objectLines[aa].size(); ++l)
					sprFileLin->writePoint(objectLines[aa][l]);
				sprFileLin->writeString("END\n");	
				TeCoord2D pt = TeFindCentroid(objectLines[aa]);
				sprFileLab->writeCadastralLabel(st.objectId(),pt.x(), pt.y(),name);
				++nlines2d;
				++nlabs;
			}
			for (aa=0; aa<objectPols.size(); ++aa)
			{
				for (l=0; l<objectPols[aa].size(); ++l)
				{
					for (k=0; k<objectPols[aa][l].size(); ++k)
						sprFileLin->writePoint(objectPols[aa][l][k]);
					sprFileLin->writeString("END\n");
				}
				TeCoord2D pt = TeFindCentroid(objectPols[aa]);
				sprFileLab->writeCadastralLabel(st.objectId(),pt.x(), pt.y(),name);
				++nlines2d;
				++nlabs;
			}
			for (aa=0; aa<objectPoints.size(); ++aa)
			{
				sprFilePt->writeCadastralLabel(st.objectId(),objectPoints[aa].location().x(), objectPoints[aa].location().y(),name);
				++npoints2d;
			}

			for (aa=0; aa<objectCells.size(); ++aa)
			{
				TeCoord2D ll = objectCells[aa].box().lowerLeft();
				TeCoord2D ur = objectCells[aa].box().upperRight();
				sprFileLin->writePoint (ll);
                TeCoord2D p1(ur.x(), ll.y());
				sprFileLin->writePoint (p1);
				sprFileLin->writePoint (ur);
                TeCoord2D p2(ll.x(),ur.y());                   
				sprFileLin->writePoint (p2);
				sprFileLin->writePoint (ll);
				sprFileLin->writeEnd();
				TeCoord2D pt = TeFindCentroid(objectCells[aa]);
				sprFileLab->writeCadastralLabel(st.objectId(),pt.x(), pt.y(),name);
				++nlines2d;
				++nlabs;
			}
		}
		if (cat == TeSPRNetwork || cat == TeSPRCadastral)
		{
			string attline = st.objectId() + "; " + st.objectId();
			for (aa=0; aa<nattr; ++aa)
			{		
				st.getPropertyValue(val,aa);
				attline += "; ";
				attline += val;
			}
			sprFileTab->writeString(attline);
			sprFileTab->writeNewLine();
			++ntabs;
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

	if (endPointHeader)
		sprFileSamp->writeString("END\n");

	vector<string> filesToDelete;
	if (nnetlines == 1)
		filesToDelete.push_back(sprFileNet->name());
	else
		sprFileNet->writeEnd();

	if (nnetlabs == 0)
		filesToDelete.push_back(sprFileNetObj->name());
	else
		sprFileNetObj->writeEnd();

	if (nlines2d == 0)
		filesToDelete.push_back(sprFileLin->name());
	else
		sprFileLin->writeEnd();

	if (npoints2d==0)
		filesToDelete.push_back(sprFilePt->name());
	else
		sprFilePt->writeEnd();

	if (nsamps==0)
		filesToDelete.push_back(sprFileSamp->name());
	else
		sprFileSamp->writeEnd();

	if (ntabs==0)
		filesToDelete.push_back(sprFileTab->name());
	else
		sprFileTab->writeEnd();

	if (nlabs==0)
		filesToDelete.push_back(sprFileLab->name());
	else
		sprFileLab->writeEnd();

	delete sprFileLab;
	delete sprFilePt;
	delete sprFileLin;
	delete sprFileTab;
	delete sprFileSamp;
	delete sprFileNet;
	delete sprFileNetObj;

	for (unsigned s=0; s<filesToDelete.size();++s)
		remove(filesToDelete[s].c_str());

	if (TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

bool
TeExportSPR (TeLayer* layer, const string& sprFileName, const string& tableName, TeSpringModels cat, const string& name)
{
	if (tableName.empty() || sprFileName.empty())
		return false;

  	if (!layer)
		return false;

	string fbase = sprFileName;
	if (sprFileName.empty())
		fbase = layer->name();

	TeTheme* tempTheme = new TeTheme();
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	TeAttrTableVector& layerTables = layer->attrTables();
	TeAttrTableVector aTable;
	for (unsigned int nn=0; nn<layerTables.size(); ++nn)
	{
		if (layerTables[nn].name() == tableName)
		{
			aTable.push_back(layerTables[nn]);
			break;
		}
	}
	tempTheme->setAttTables(aTable);
	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToSPRING(tQuerier,fbase,cat,name,layer->projection());
	delete tQuerier;
	delete tempTheme;
    return res ;
}

bool 
TeExportThemeToSPRING(TeTheme* theme, TeSpringModels cat, const string& name, TeSelectedObjects selOb, const string& baseName)
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
	bool res = TeExportQuerierToSPRING(tQuerier,fbase,cat,name,theme->layer()->projection());
	delete tQuerier;
	return res;
}
