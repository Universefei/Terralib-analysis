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
/*! \file TeDatabaseIndex.h
    \brief This file contains structures and definitions about a database index support in TerraLib
*/
#ifndef  __TERRALIB_INTERNAL_DATABASEINDEX_H
#define  __TERRALIB_INTERNAL_DATABASEINDEX_H

#include "TeDefines.h"

#include <vector>
#include <string>


//! An abstract database index class
/*!
	Instances of this classes represent a database index.
	It includes the index name, a string vector of 
	attributes names and a boolean that says if is a 
	primary	key or not.	It should be used when is necessary
	work with database indexes in TerraLib applications.
  
*/
class TL_DLL TeDatabaseIndex
{

public:

	//! Empty constructor
	TeDatabaseIndex();

	//! Destructor
	virtual ~TeDatabaseIndex();

private:

	//! A string with the index name
	std::string	_indexName;

	//! A string Vector of attributes names 
	std::vector<std::string> _indexColumns;

	//! A boolean that says if is primary key or not
	bool _isPrimaryKey;

public:
	
	/** @name Members
	*  Methods to access class private members
	*/
	//@{

	//! Set the name of the index
	void setIndexName(const std::string& name);

	//! Return the name of the index
	std::string getIndexName();

	//! Set the attributes column name
	void setColumn(const std::string& column);

	//! Set the attributes vector
	void setColumns(const std::vector<std::string>& columns);

	//! Return the attributes vector
	std::vector<std::string> getColumns();

	//! Return the columns list with a 'column' separating each value
	std::string getCSVColumns();

	//! Set the primary key boolean
	void setIsPrimaryKey(bool isPk);

	//! return the primary key boolean
	bool isPrimaryKey();		
	//@}
};

#endif
