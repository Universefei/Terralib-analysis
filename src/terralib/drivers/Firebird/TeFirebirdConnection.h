/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright 2001-2007 INPE and Tecgraf/PUC-Rio.

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

/*! \file TeFirebirdConnection.h
    \brief This file contains a concrete class for TerraLib Firebird Database connections.
    \author Claudio Henrique Bogossian <claudio.bogossian@funcate.org.br>
*/

#ifndef  __TERRALIB_INTERNAL_POSTGRESQL_CONNECTION_H
#define  __TERRALIB_INTERNAL_POSTGRESQL_CONNECTION_H

// TerraLib
#include "../../kernel/TeConnection.h"
#include "TeFirebirdDefines.h"
#include <ibpp/core/ibpp.h>

// STL
#include <string>

// forward declarations

class TLFIREBIRD_DLL TeFirebirdConnection : public TeConnection
{

public: 
    
    /** @name Constructor
     *  Initializer methods.
     */
    //@{

    /*! \brief Default constructor. */
	TeFirebirdConnection();

    /*! \brief Destructor. */
	~TeFirebirdConnection();

    //@}

    /** @name Access methods.
	*  Methods to access class private members.
	*/
	//@{

    void setFBConnection(IBPP::Database* conn);
    IBPP::Database* getFBConnection() const;

    //@}

private:

    IBPP::Database* fbConnection_; //!< Internal fb connection interface.
};

#endif // __TERRALIB_INTERNAL_POSTGRESQL_CONNECTION_H
	
