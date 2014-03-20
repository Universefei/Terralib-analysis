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
/*! \file  TeOCIConnect.h
    \brief This file contains the class OCIConnect that is responsible for the connection to ORACLE DBMS using OCI (Oracle Call Interface) library. 
*/

#ifndef OCI_CONNECT_H
#define OCI_CONNECT_H

#include <oci.h>
#include <string>

#include "TeOracleDefines.h"

using namespace std;

/*! \class TeOCIConnection
	\brief This class is used to connect to a ORACLE DBMS

	This class is reponsible for the connection to ORACLE DBMS using OCI (Oracle Call Interface) library.
*/
class TLORACLE_DLL TeOCIConnection
{
protected:
	bool	     connected_;		//!< indicates if there is a connection opened or not
	bool		 useSDO_;			//!< indicates if the connected Oracle DBMS uses the SDO_GEOMETRY type 
	string       errorMess_;		//!< error message returned by the ORACLE DBMS

public:
	/** @name OCI connection handles
	*/
	//@{ 	
	OCIEnv*			envhp_;		//!< environment OCI handle        
	OCIError*		errhp_;		//!< error OCI handle

	OCIServer*		srvhp_;		//!< service context OCI handle - represents a connection 
	OCISvcCtx*		svchp_;		//!< service context OCI handle - defines a service
	OCISession*		usrhp_;		//!< service context OCI handle - defines the permissions and privileges associated to an user
	OCITrans*		txnhp_;		//!< service context OCI handle - controls the transactions 

	OCIStmt*		stmthp_;	//!< environment OCI handle - stores SQL statement
	//@}

	OCIType*		tdo_geometry_; 		//!< describes the SDO_GEOMETRY type 
	OCIType*		tdo_ordinates_;		//!< describes the SDO_ORDINATES type
	OCIArray*		ordinates_;			//!< stores the ordinates to insert into database 	

	unsigned char*	blobValue_;			//!< stores the blob value that will be inserted into database  

public:
	
	//! Constructor
	TeOCIConnection();

	//! Destructor
	~TeOCIConnection() { disconnect(); }

	//! Connects to ORACLE DBMS
	bool connect(const char* dblink, const char* username, const char* password, const bool& useSDO = false);
	
	//! Disconnects to ORACLE DBMS
	void disconnect();

	//! Prepares the SQL statement to be executed
	bool prepare(const string& stmt);

	//! Binds objects by position
	bool bindByPos(int pos, void* value, int size, void* indicator, int type);
	
	//! Binds objects by name
	bool bindByName(char* name, void* value, int size, void* indicator, int type);
	
	//! Binds ordinate object
	bool bindOrdinates();

	//! Appends ordinates to be inserted or updated
	bool appendOrdinates(const double& val);

	//! Locates memory to the ordinates 
	bool allocateObjectOrdinates();

	//! Returns the query (statement SQL) type 
	int	queryType(void);

	//! Starts a transaction 
	bool transStart();

	//! Rollbacks a transaction
	bool transRollback();

	//! Commits a transaction
	bool transCommit();
	
	//! Executes only SQL statements that do not return rows (INSERT, UPDATE, DELETE, etc)
	bool execute(const string& stm);
	
	//! Executes only SQL statements that insert or update rows with SDO Objects 
	bool executeSDOSTM(const string& stm);

	//! Executes only SQL statements that insert or update rows with BLOB
	bool executeBLOBSTM(const string& stm, unsigned char* blobValue, const int& blobLength, const string& name);

	//! Checks if there is an opened conection
	bool isConnected() { return connected_; }

	//! Returns if the connected Oracle DBMS uses the SDO_GEOMETRY type
	bool useSDOType() { return useSDO_; } 

	//! Checks OCI error handle
	bool checkError(OCIError* errhp, int status);

	//! Gets the error message
	string getErrorMessage() { return errorMess_; }
};

#endif
