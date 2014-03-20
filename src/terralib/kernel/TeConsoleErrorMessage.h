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
/*! \file TeConsoleErrorMessage.h
    \brief This file deals with the display of error messages in a console
*/
#ifndef TeConsoleErrorMessage_H
#define TeConsoleErrorMessage_H

#include <iostream>
#include "TeErrorMessage.h"

using namespace std;

//! A class to handle the display of error messages in a console
class TeConsoleErrorMessage: public TeErrorMessage
{
public:
   
	//! Constructor
	TeConsoleErrorMessage()
	{}

	//! Destructor
	~TeConsoleErrorMessage()
	{}

	//! Display the message
	virtual TeMessageReturn apply ( 
				  const string& appName, 
				  const string & msgText, 
				  TeMessageType	msgType,
				  TeMessageIcon iconType )
	{ cout << appName << msgText; return Te_IDOK; }

};

//! A prototype of a Console Error Message handler
class TeConsoleErrorMessagePrototype: public TeErrorMessagePrototype
{
	virtual TeErrorMessage* build()
	{ return new TeConsoleErrorMessage(); }

};

//! A single instance of the console error message handler
static TeConsoleErrorMessagePrototype consoleInstance;
#endif

