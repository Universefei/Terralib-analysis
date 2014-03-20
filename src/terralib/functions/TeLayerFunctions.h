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
/*! \file TeLayerFunctions.h
    \brief This file contains functions to manipulate layers 
*/
#ifndef  __TERRALIB_INTERNAL_LAYERFUNCTIONS_H
#define  __TERRALIB_INTERNAL_LAYERFUNCTIONS_H

#include "../kernel/TeLayer.h"
#include "../kernel/TeVectorRemap.h"
#include "../kernel/TeTheme.h"
#include "../kernel/TeDatabase.h"
#include "../kernel/TeProgress.h"

#include "TeFunctionsDefines.h"

/** Imports a geometry set to a layer, creating an empty attribute table
	This function imports a geometry set to a layer.
	An attribute table is automatically created with only the object_id field.
	If geometries don�t have object ids a unique identification will be automatically generated
	\param layer pointer to a layer previously created 
    \param geomSet the geometry set
    \return TRUE or FALSE whether the geometry set was imported successfully
*/
template <class G>
bool TeImportGeometriesToLayer(TeLayer* layer, G& geomSet)
{
	if (!layer)

		return false;

	TeDatabase* db = layer->database();
	int nTab = 0;
	TeAttrTableVector attTables;
	if (layer->getAttrTables(attTables))
		nTab = attTables.size();
	
	string tabName = "AttributeTable_" + Te2String(layer->id()) + "_" + Te2String(nTab); 
	TeAttributeList attList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 16;
	at.rep_.name_ = "object_id";
	at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);

	TeTable attTable(tabName,attList,"object_id", "object_id",TeAttrStatic);

	TeTableRow row;
	for (int i=0; i<geomSet.size(); i++)
	{
		row.clear();
		string id = geomSet[i].objectId();
		if (id.empty())
		{
			id = Te2String(i);
			geomSet[i].objectId(id);
		}
		row.push_back(id);
		attTable.add(row);
	}
	
	if (!layer->addGeometrySet(geomSet))
	{
		attTable.clear();
		db->deleteLayer(layer->id());
		return false;
	}
	
	if (!layer->createAttributeTable(attTable))
	{
		attTable.clear();
		db->deleteLayer(layer->id());
		return false;
	}

	if (!layer->saveAttributeTable(attTable))
	{
		attTable.clear();
		db->deleteLayer(layer->id());
		return false;
	}
	
	attTable.clear();
	return true;
}

/** Copy the geometries retrieved in a portal to layer
	\param portal pointer to a portal that has executed a query for geometries
	\param proj pointer to the projection associated to the geometries
	\param layer pointer to the destination layer 
	\param tname optional parameter indicating the name of a table to store the geometries
*/
template<typename GS>
GS* 
TeCopyPortalGeometriesToLayer(TeDatabasePortal* portal, TeProjection* proj, TeLayer* layer, const string& tName="")
{
	if (!portal ||!layer)
		return 0;

	TeProjection* pfrom = TeProjectionFactory::make(proj->params());
	TeProjection* pto = TeProjectionFactory::make(layer->projection()->params());
	bool doRemap;
	if (!pfrom || !pto || (pfrom->name() == "NoProjection" ||  pto->name() == "NoProjection") && 
		!(pfrom->name() == pto->name()))
		doRemap = false;
	else
		doRemap= !(*pfrom == *pto);

	if (doRemap)
		pfrom->setDestinationProjection(pto);
	
	if (TeProgress::instance())
			TeProgress::instance()->setTotalSteps(portal->numRows());

	GS set1;
	int n = 1; 
	bool flag=false;
	do
	{
		typename GS::value_type elemI;
		flag = portal->fetchGeometry(elemI);
		if (doRemap)
		{
			typename GS::value_type elemO;
			TeVectorRemap(elemI,pfrom,elemO,pto);
			elemO.objectId(elemI.objectId());
			set1.add(elemO);
		}
		else
		{
			set1.add(elemI);
			set1[set1.size()-1].objectId(elemI.objectId()); 
		}

		if (((n%100) == 0) || !flag)
		{	
			layer->addGeometrySet(set1,tName);
			set1.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(n);
			}	
		}
		n++;
	} while (flag);
	delete pfrom;
	delete pto;
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return 0;
}

/** This function fills a layer with the objects selected in a theme
	\param inTheme pointer to the source theme
	\param outLayer pointer to the destination layer (already created)
	\param selObj flag to indicate with objects of the theme should be copied.
	\param txtTable pointer to a map<string,string> to return the old and
	new names of the text tables
*/
TLFUNCTIONS_DLL bool TeCopyThemeToLayer(TeTheme* inTheme, TeLayer* outLayer, 
							int selObj, map<string,string> *txtTable=0);

/** This function fills a layer with the objects selected in a theme
	\param inTheme pointer to the source theme
	\param outLayer pointer to the destination layer (already created)
	\param selObj flag to indicate with objects of the theme should be copied.
	\param txtTable pointer to a map<string,string> to return the old and
	new names of the text tables
*/
TLFUNCTIONS_DLL bool TeCopyExternThemeToLayer(TeTheme* inTheme, TeLayer* outLayer, 
							int selObj, map<string,string> *txtTable=0);

/** This function copies a extern layer to another local
	\param inLayer pointer to the source theme
	\param outLayer pointer to the destination layer (already created)
	\param txtTable pointer to a map<string,string> to return the old and
	new names of the text tables
*/
TLFUNCTIONS_DLL bool TeCopyLayerToLayer(TeLayer* inLayer, TeLayer* outLayer, map<string,string> *txtTable=0);

/** \example copyLayer.cpp
	Shows how to duplicate a layer changing its projection.
 */
#endif
