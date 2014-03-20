/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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

/*! \file TeThemeFunctions.h
    This file contains functions to deal with themes 
*/
#ifndef  __TERRALIB_INTERNAL_THEMEFUNCTIONS_H
#define  __TERRALIB_INTERNAL_THEMEFUNCTIONS_H

#include "TeFunctionsDefines.h"

class TeTheme;

#include <vector>
#include <string>

using namespace std;

//! Creates a new theme from the objects of an existing theme
/*!
	\param inTheme input theme 
    \param newThemeName the name of the new theme
	\param selObj the criteria to include objects of the existing theme in the new one
    \param newTheme pointer to the new theme created if it succeed and null otherwise
*/
TLFUNCTIONS_DLL bool TeCreateThemeFromTheme(TeTheme* inTheme, const string& newThemeName, int selObj=0, TeTheme* newTheme=0);

TLFUNCTIONS_DLL vector<string> getObjects(TeTheme* theme, int sel);

TLFUNCTIONS_DLL vector<string> getItems(TeTheme* theme, int sel);

TLFUNCTIONS_DLL vector<string> getObjects(TeTheme* theme, vector<string>& itens);

TLFUNCTIONS_DLL vector<string> getItems(TeTheme* theme, vector<string>& objcts);

#endif
