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
/*! \file TeDatabaseUtils.h
    \brief This file contains some utilitary functions to extended the TerraLib data model for TerraView like applications.
*/
#ifndef __TERRALIB_INTERNAL_DATABASEUTILS_H
#define __TERRALIB_INTERNAL_DATABASEUTILS_H


#include "../kernel/TeDatabase.h"
#include "../kernel/TeExternalTheme.h"
#include <string>
#include <vector>

#include "TeUtilsDefines.h"

using std::string;
using std::vector;


/** @defgroup DBUtils Utilitary functions
    @ingroup  DatabaseUtils
	A set of utilitary functions
 *  @{
 */
/**  This function copies the contents of a TerraLib database to another TerraLib database
	\param dbFrom A pointer to the source database
	\param dbTo	  A pointer to the destination database
*/
TLUTILS_DLL bool TeCopyDatabase(TeDatabase* dbFrom, TeDatabase* dbTo);


/** Returns true whether a database contains the TerraLib data model
	\param db	pointer do a TerraLib database
*/
TLUTILS_DLL bool isTerralibModel(TeDatabase* db);

TLUTILS_DLL vector<string> generateItemsInClauseVec(TeTheme* theme, string& where);

TLUTILS_DLL map<string, vector<string> > getObject2ItemsMap(TeTheme* theme, vector<string>& itens);

template <typename Iterator>
vector<string>
generateItemsInClauses(Iterator& begin, Iterator& end, TeTheme* theme)
{
	vector<string> itenVec;
	Iterator temp = begin;
	while (temp != end)
	{
		itenVec.push_back(*temp);
		temp++;
	}

	map<string, vector<string> > objMap = getObject2ItemsMap(theme, itenVec);
	map<string, vector<string> >::iterator mit;
	vector<string>::iterator it;

	int i, chunkSize = 200;
	string inClause;
	vector<string> inClauseVector;
	
	i = 0;
	bool chunk = true;
	for(mit=objMap.begin(); mit!=objMap.end(); ++mit)
	{
		if (chunk == true)
		{
			chunk = false;
			if (!inClause.empty())
			{
				inClause[inClause.size() - 1] = ')';
				inClauseVector.push_back(inClause);
				inClause.clear();
			}
			inClause = "(";
		}
		for(it=mit->second.begin(); it!=mit->second.end(); ++it)
		{
			inClause += *it + ",";
			i++;
			if (i%chunkSize == 0)
				chunk = true;
		}
	}
	if (!inClause.empty())
	{
		inClause[inClause.size() - 1] = ')';
		inClauseVector.push_back(inClause);
	}
	return inClauseVector;
}

TLUTILS_DLL bool loadViewSetAndThemeGroups(TeDatabase* db, const std::string& userName);

TLUTILS_DLL bool TeCopyView(TeView* view, TeDatabase* inputDatabase, const std::string& newViewName, const std::string& newViewUser, const std::set<int>& setThemeIds, TeDatabase* outputDatabase, std::vector<bool>& vecResult);

TLUTILS_DLL TeViewNode* TeCopyViewNode(TeViewNode* node, TeDatabase* inputDatabase, TeDatabase* outputDatabase, TeView* newView);

TLUTILS_DLL bool TeCopyTable(TeDatabase* inputDatabase, const std::string& inputTable, TeDatabase* outputDatabase, const std::string& outputTable);

//@}

#endif

