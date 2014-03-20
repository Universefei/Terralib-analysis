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

/*! \file TeConnection.h
    \brief This file contains a base class for TerraLib Database connections.
    \author Douglas Uba <douglas@dpi.inpe.br>
    \author Mario Rocco Pettinati <mario@dpi.inpe.br>
*/

#ifndef  __TERRALIB_INTERNAL_CONNECTION_H
#define  __TERRALIB_INTERNAL_CONNECTION_H

// TerraLib
#include "TeDefines.h"

// STL
#include <string>

class TL_DLL TeConnection 
{

public: 
	
    /** @name Constructor
     *  Initializer methods.
     */
    //@{

    /*! \brief Default constructor. */
    TeConnection();

    /*! \brief Virtual destructor. */
	virtual ~TeConnection();

    //@}

    /** @name Access methods.
	*   Methods to access class private members.
	*/
	//@{

    unsigned int getId() const; 
    void setId(const unsigned int& id);

    std::string	getUser() const;
    void setUser(const std::string& user);

    std::string getHost() const;
    void setHost(const std::string& host);

    std::string getPassword() const;
    void setPassword(const std::string& psw);

    std::string	getDBMS() const; 
    void setDBMS(const std::string& name);

    std::string getDatabaseName() const;
    void setDatabaseName(const std::string& name);

	int	getPortNumber() const; 
    void setPortNumber(const int& portNumber);

    //@}

protected :

    unsigned int id_; //!< Internal identify for the connection.

    /** Connections parameters */
	//@{
    
    std::string host_;					//!< Host name of the database server
	std::string	user_;					//!< User name 
	std::string	password_;				//!< User password
	std::string	database_;				//!< Database name
	std::string	dbmsName_;				//!< DBMS name
    int portNumber_;	        		//!< Port number
    
    //@}
};

#endif // __TERRALIB_INTERNAL_CONNECTION_H
	
