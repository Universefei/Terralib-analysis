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
#include "TeDriverCSV.h"
#include "../kernel/TeAsciiFile.h"

#include "../kernel/TeUtils.h"
#include "../kernel/TeLayer.h"
#include "../kernel/TeQuerier.h"

bool TeExportQuerierToCSV(TeQuerier* querier, const std::string& filename, const char& sep)
{
	// check initial conditions
	if (!querier || filename.empty())
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
	string line = "COORD_X";
	line += sep;
	line += "COORD_Y";
	line += sep;
	line += qAttList[0].rep_.name_;
	unsigned int i=0;
	TeAsciiFile csv(filename, "w+");
	for (i=1;i<qAttList.size();++i)
	{
		line += sep;
		line += qAttList[i].rep_.name_;
	}
	csv.writeString(line);
	csv.writeNewLine();

	TePointSet		objectPoints;	// points of the region

	// progress information
	if (TeProgress::instance())
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;
	unsigned int nIProcessed = 0;
	char aux[50];
	TeSTInstance st;
	while(querier->fetchInstance(st))
	{
		if (!st.hasPoints())
			continue;

		objectPoints = st.getPoints();
		for (i=0;i<objectPoints.size();++i)
		{
			TeCoord2D xy = objectPoints[i].location();
			sprintf(aux,"%.6f%c%.6f", xy.x(),sep, xy.y());
			line = aux;
			if (onlyObjId)
			{
				line += sep;
				line += st.objectId();
			}	
			else
			{
				unsigned int aa = 0;
				string val;
				for (aa=0; aa<qAttList.size(); ++aa)
				{
					st.getPropertyValue(val,aa);
					line += sep;
					line += val;
				}
			}
			csv.writeString(line);
			csv.writeNewLine();
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
TeExportThemeToCSV(TeTheme* theme, TeSelectedObjects selOb, const std::string& filename, const char& sep)
{
	if (!theme)
		return false;

	if (!(theme->visibleGeoRep() & TePOINTS))
		return false;

	string fbase = filename;
	if (filename.empty())
		fbase = theme->name();

	TeQuerierParams qPar(true, true);
	qPar.setParams(theme);
	qPar.setSelecetObjs(selOb);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToCSV(tQuerier, filename, sep);
	delete tQuerier;
	return res;
}

bool 
TeExportLayerToCSV(TeLayer* layer, const string& filename, const char& sep)
{
	if (!layer)
		return false;

	if (!layer->hasGeometry(TePOINTS))
		return false;

	string fbase = filename;
	if (filename.empty())
		fbase = layer->name();

	TeTheme* tempTheme = new TeTheme();
	tempTheme->layer(layer);
	tempTheme->collectionTable("");
	tempTheme->collectionAuxTable("");
	tempTheme->setAttTables(layer->attrTables());

	TeQuerierParams qPar(true, true);
	qPar.setParams(tempTheme);

	TeQuerier* tQuerier = new TeQuerier(qPar);
	bool res = TeExportQuerierToCSV(tQuerier, filename, sep);
	delete tQuerier;
	delete tempTheme;
    return res ;
}
