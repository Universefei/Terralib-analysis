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
#include "../kernel/TeExternalTheme.h"
#include "../kernel/TeDatabase.h"

#include "TeThemeFunctions.h"

bool TeCreateThemeFromTheme(TeTheme* inTheme, const string& newThemeName, int selObj, TeTheme* newTheme)
{
	// if no source theme or not a name for the new theme
	if (!inTheme || newThemeName.empty())
		return false;

	// if there no pointer theme was passed create a new instance
	if (!newTheme)
		newTheme = new TeTheme();

	// builds a valid theme name
	TeDatabase* curDb = inTheme->layer()->database();
	string validThemeName = newThemeName;
	int i=1;
	while (curDb->themeExist(validThemeName))
	{
		validThemeName = newThemeName + "_" + Te2String(i);
		i++;
	}

	// create new theme with the same definitions of the input theme
	newTheme->id(0);
	newTheme->name(validThemeName);
	newTheme->layer(inTheme->layer());
	newTheme->setAttTables(inTheme->attrTables());
	curDb->viewMap()[inTheme->view()]->add(newTheme);
	newTheme->visibleRep(inTheme->visibleRep());

	// save its definition in the database
	// creates collection and collection_aux table
	if (!newTheme->save())
	{
		delete newTheme;
		newTheme = 0;
    return false;
	}

	// if its a theme of raster data there's nothing else to be done
	if (inTheme->layer()->hasGeometry(TeRASTER) || inTheme->layer()->hasGeometry(TeRASTERFILE))
	{
		TeBox tbox = curDb->getThemeBox(newTheme);
		newTheme->setThemeBox(tbox);
		curDb->updateTheme(newTheme);
		return true;
	}

	// this SQL selects instances of objects according to input, so it uses te_collection_aux
	string inputInst;

	// this SQL selects objects according to input, so it uses te_collection
	string inputObj; 
	if(selObj == TeSelectedByPointing) // queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 2";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 2";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 2) // pointed
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 1";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 1";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 3) // not queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status <> 2";
		inputObj += " AND " + inTheme->collectionTable() + ".c_object_status <> 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status <> 2";
		inputInst += " AND " + inTheme->collectionAuxTable() + ".grid_status <> 3)";
	}
	else if(selObj == 4) // not pointed
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status <> 1";
		inputObj += " AND " + inTheme->collectionTable() + ".c_object_status <> 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status <> 1";
		inputInst += " AND " + inTheme->collectionAuxTable() + ".grid_status <> 3)";
	}
	else if(selObj == 5) // pointed and queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 6) // pointed or queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 2";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 1";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 2";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 1";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}

	// fills collection table from the selection requested

	string sql = "INSERT INTO " + newTheme->collectionTable() + " ( ";
	sql += " c_object_id, c_legend_id, label_x, label_y, c_legend_own, c_object_status ) ";
	sql += " SELECT * FROM " + inTheme->collectionTable();
	
	if(inputObj.empty() == false)
		sql += " WHERE " + inputObj;
	if (!curDb->execute(sql))
	{
		curDb->deleteTheme(newTheme->id());
		return false;
	}
	// sets theme objects settings to default
	sql = "UPDATE " + newTheme->collectionTable() + " SET c_legend_id = 0";
	if (!curDb->execute(sql))
	{
		curDb->deleteTheme(newTheme->id());
		return false;
	}
	sql = "UPDATE " + newTheme->collectionTable() + " SET c_object_status = 0";
	if (!curDb->execute(sql))
	{
		curDb->deleteTheme(newTheme->id());
		return false;
	}

	//fills collectin aux
	if(!newTheme->populateCollectionAux())
	{
		curDb->deleteTheme(newTheme->id());
		return false;
	}

	TeBox tbox = curDb->getThemeBox(newTheme);
	newTheme->setThemeBox(tbox);
	curDb->updateTheme(newTheme);

	return true;
}

vector<string> getObjects(TeTheme* theme, vector<string>& itens)
{
	vector<string> svec;
	if(!theme)
		return svec;

	TeDatabase* db = 0;
	if(theme->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
	else if(theme->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme)->layer()->database();

	if(db == 0)
		return svec;

	TeDatabasePortal* portal = db->getPortal();
	string C = theme->collectionTable();
	string CA = theme->collectionAuxTable();

	string query = "SELECT " + C + ".c_object_id FROM " + C + " LEFT JOIN " + CA;
	query += " ON " + C + ".c_object_id = " + CA + ".object_id";
	query += " WHERE " + CA + ".unique_id IN ";

	set<string> idSet;
  
  vector< string >::iterator it_begin = itens.begin();
  vector< string >::iterator it_end = itens.end();
  
	vector<string> inVec = generateInClauses(it_begin, it_end, db, false);
	vector<string>::iterator it;
	for(it=inVec.begin(); it!=inVec.end(); ++it)
	{
		if((*it).empty() == false)
		{
			string sel = query + *it;
			if (portal->query(sel) == false)
			{
				delete portal;
				return svec;
			}
			while (portal->fetchRow())
				idSet.insert(portal->getData(0));
			portal->freeResult();
		}
	}
	delete portal;

	set<string>::iterator sit;
	for(sit=idSet.begin(); sit!=idSet.end(); ++sit)
		svec.push_back(*sit);
	return svec;
}


vector<string> getObjects(TeTheme* theme, int sel)
{
	vector<string> svec;
	if(!theme)
		return svec;

	TeDatabase* db = 0;
	if(theme->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
	else if(theme->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme)->layer()->database();

	if(db == 0)
		return svec;

	string C = theme->collectionTable();
	string CA = theme->collectionAuxTable();
	string input;

	if(sel == TeSelectedByPointing)
		input += " WHERE " + C + ".c_object_status = 1 OR " + C + ".c_object_status = 3";
	else if(sel == TeNotSelectedByPointing)
		input += " WHERE " + C + ".c_object_status = 0 OR " + C + ".c_object_status = 2";
	else if(sel == TeSelectedByQuery)
		input += " WHERE " + C + ".c_object_status = 2 OR " + C + ".c_object_status = 3";
	else if(sel == TeNotSelectedByQuery)
		input += " WHERE " + C + ".c_object_status = 0 OR " + C + ".c_object_status = 1";
	else if(sel == TeSelectedByPointingAndQuery)
		input += " WHERE " + C + ".c_object_status = 3";
	else if(sel == TeSelectedByPointingOrQuery)
		input += " WHERE " + C + ".c_object_status <> 0";
	else if(sel == TeGrouped)
		input += " WHERE " + C + ".c_legend_id <> 0";
	else if(sel == TeNotGrouped)
		input += " WHERE " + C + ".c_legend_id = 0";

	string query = "SELECT " + C + ".c_object_id FROM " + C + " LEFT JOIN " + CA;
	query += " ON " + C + ".c_object_id = " + CA + ".object_id" + input;

	TeDatabasePortal* portal = db->getPortal();
	if (portal->query(query) == false)
	{
		delete portal;
		return svec;
	}
	while (portal->fetchRow())
		svec.push_back(portal->getData(0));

	delete portal;
	return svec;
}

vector<string> getItems(TeTheme* theme, vector<string>& objects)
{
	vector<string> svec;
	if(!theme)
		return svec;

	TeDatabase* db = 0;
	if(theme->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
	else if(theme->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme)->layer()->database();

	if(db == 0)
		return svec;

	TeDatabasePortal* portal = db->getPortal();
	string C = theme->collectionTable();
	string CA = theme->collectionAuxTable();

	string query = "SELECT " + CA + ".unique_id FROM " + C + " LEFT JOIN " + CA;
	query += " ON " + C + ".c_object_id = " + CA + ".object_id";
	query += " WHERE " + C + ".c_object_id IN ";
  
  vector< string >::iterator it_begin = objects.begin();
  vector< string >::iterator it_end = objects.end();  

	vector<string> inVec = generateInClauses(it_begin, it_end, db);
	vector<string>::iterator it;
	for(it=inVec.begin(); it!=inVec.end(); ++it)
	{
		if((*it).empty() == false)
		{
			string sel = query + *it;
			if (portal->query(sel) == false)
			{
				delete portal;
				return svec;
			}
			while (portal->fetchRow())
				svec.push_back(portal->getData(0));
			portal->freeResult();
		}
	}
	delete portal;
	return svec;
}

vector<string> getItems(TeTheme* theme, int sel)
{
	vector<string> svec;
	if(!theme)
		return svec;

	TeDatabase* db = 0;
	if(theme->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
	else if(theme->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme)->layer()->database();

	if(db == 0)
		return svec;

	string C = theme->collectionTable();
	string CA = theme->collectionAuxTable();
	string input;

	if(sel == TeSelectedByPointing)
		input += " WHERE " + CA + ".grid_status = 1 OR " + CA + ".grid_status = 3";
	else if(sel == TeNotSelectedByPointing)
		input += " WHERE " + CA + ".grid_status = 0 OR " + CA + ".grid_status = 2";
	else if(sel == TeSelectedByQuery)
		input += " WHERE " + CA + ".grid_status = 2 OR " + CA + ".grid_status = 3";
	else if(sel == TeNotSelectedByQuery)
		input += " WHERE " + CA + ".grid_status = 0 OR " + CA + ".grid_status = 1";
	else if(sel == TeSelectedByPointingAndQuery)
		input += " WHERE " + CA + ".grid_status = 3";
	else if(sel == TeSelectedByPointingOrQuery)
		input += " WHERE " + CA + ".grid_status <> 0";
	else if(sel == TeGrouped)
		input += " WHERE " + C + ".c_legend_id <> 0";
	else if(sel == TeNotGrouped)
		input += " WHERE " + C + ".c_legend_id = 0";

	string query = "SELECT " + CA + ".unique_id FROM " + C + " LEFT JOIN " + CA;
	query += " ON " + C + ".c_object_id = " + CA + ".object_id" + input;

	TeDatabasePortal* portal = db->getPortal();
	if (portal->query(query) == false)
	{
		delete portal;
		return svec;
	}
	while (portal->fetchRow())
		svec.push_back(portal->getData(0));

	delete portal;
	return svec;
}
