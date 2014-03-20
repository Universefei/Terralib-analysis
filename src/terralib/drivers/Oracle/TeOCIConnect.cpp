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


#include "TeOCIConnect.h"
#include "TeOCISDO.h"
#include <cstring>


TeOCIConnection::TeOCIConnection() 
{
	connected_ = false;
	useSDO_ = false;

	envhp_ = 0;
	errhp_ = 0;
	srvhp_ = 0;
	svchp_ = 0;
	usrhp_ = 0;
	txnhp_ = 0;
	stmthp_ = 0;
	tdo_geometry_ = 0;
	tdo_ordinates_ = 0;
	ordinates_ = 0;
	blobValue_ = 0;
}


bool TeOCIConnection::connect(const char* dblink, const char* username, const char* password, const bool& useSDO) 
{
	sword status;

	int mode = OCI_THREADED|OCI_OBJECT;
	
	//create and initialize the environment
	status = SDO_OCIEnvCreate(&envhp_, (ub4)mode, (dvoid *)0, 
				      (dvoid*(*)(dvoid *ctxp, size_t size))0, 
				      (dvoid*(*)(dvoid *ctxp, dvoid *memptr, size_t newsize))0, 
				      (void (*)(dvoid *ctxp, dvoid *memptr))0, 
					  (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	// Allocate handles - errhp_(OCIError) and srvhp_ (OCIServer)
	status = SDO_OCIHandleAlloc((dvoid*)envhp_, (dvoid**)&errhp_, (ub4)OCI_HTYPE_ERROR,
					   (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	status = SDO_OCIHandleAlloc((dvoid*)envhp_, (dvoid**)&srvhp_, (ub4)OCI_HTYPE_SERVER,
					   (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	// Multiple Sessions or Connections
	status = SDO_OCIServerAttach(srvhp_, errhp_, (text*)dblink, 
						 (sb4)strlen(dblink), (ub4)OCI_DEFAULT);
	if(!checkError(errhp_, status))
		return false;

	// Allocate handles: svchp_(OCISvcCtx)
	status = SDO_OCIHandleAlloc((dvoid*)envhp_, (dvoid**)&svchp_, (ub4)OCI_HTYPE_SVCCTX, 
					   (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	// Set the server attribute in the service context handle
	status = SDO_OCIAttrSet((dvoid*)svchp_, (ub4)OCI_HTYPE_SVCCTX, (dvoid*)srvhp_, (ub4)0, 
				   (ub4)OCI_ATTR_SERVER, errhp_);
	if(!checkError(errhp_, status))
		return false;

	/// Allocate a user session handle
	status = SDO_OCIHandleAlloc((dvoid*)envhp_, (dvoid**)&usrhp_, (ub4)OCI_HTYPE_SESSION, 
					   (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	// set user name attribute in user session handle 
	status = SDO_OCIAttrSet((dvoid*)usrhp_, (ub4)OCI_HTYPE_SESSION, (dvoid*)username, 
				   (ub4)strlen(username), (ub4)OCI_ATTR_USERNAME, errhp_);
	if(!checkError(errhp_, status))
		return false;
	
	// set password attribute in user session handle
	status = SDO_OCIAttrSet((dvoid*)usrhp_, (ub4)OCI_HTYPE_SESSION, (dvoid*)password, 
				   (ub4)strlen(password), (ub4)OCI_ATTR_PASSWORD, errhp_);
	if(!checkError(errhp_, status))
		return false;

	// Session begins - establishes a session for a user against a particular
	// server. This call is required for the user to execute operations on the server.
	status = SDO_OCISessionBegin(svchp_, errhp_, usrhp_, OCI_CRED_RDBMS, 
								 OCI_DEFAULT);
	if(!checkError(errhp_, status))
		return false;

	// set the user session attribute in the service context handle
	status = SDO_OCIAttrSet((dvoid*)svchp_, (ub4)OCI_HTYPE_SVCCTX, (dvoid*)usrhp_, (ub4)0, 
				   (ub4)OCI_ATTR_SESSION, errhp_);
	if(!checkError(errhp_, status))
		return false;

	// allocate transaction handle 1 and set it in the service handle (OCITrans)
	status = OCIHandleAlloc((dvoid *)envhp_, (dvoid **)&txnhp_, OCI_HTYPE_TRANS, 0, 0);
	if(!checkError(errhp_, status))
		return false;

	status = OCIAttrSet((dvoid *)svchp_, OCI_HTYPE_SVCCTX, (dvoid *)txnhp_, 0,OCI_ATTR_TRANS, errhp_);
	if(!checkError(errhp_, status))
		return false;

	// locate stmthp_
	status = SDO_OCIHandleAlloc((dvoid*)envhp_, (dvoid**)&stmthp_, 
								  (ub4)OCI_HTYPE_STMT, (size_t)0, (dvoid**)0);
	if(!checkError(errhp_, status))
		return false;

	if(useSDO)
	{
		//load the tdo type to sdo_geometry (OCIType)
		status = OCITypeByName(envhp_, errhp_, svchp_, (const text *)
			"MDSYS", (ub4) strlen((char *)"MDSYS"),
			(const text *) "SDO_GEOMETRY",	(ub4) strlen((char *)"SDO_GEOMETRY"),
			(text *)0, 0, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &tdo_geometry_);
		if(!checkError(errhp_, status))
			return false;

		//load the tdo type to sdo_ordinates (OCIType)
		status = OCITypeByName(envhp_, errhp_, svchp_, (const text *)
			"MDSYS", (ub4) strlen((char *)"MDSYS"),
			(const text *) "SDO_ORDINATE_ARRAY", (ub4) strlen((char *)"SDO_ORDINATE_ARRAY"),
			(text *)0, 0, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &tdo_ordinates_);
		if(!checkError(errhp_, status))
			return false;
	}
	connected_ = true;
	useSDO_ = useSDO;
	return true;
}


void TeOCIConnection::disconnect()  
{
	
	OCIObjectFree(envhp_, errhp_, (dvoid*)ordinates_, (ub4)OCI_OBJECTFREE_FORCE);
	
	// Session ends
	SDO_OCISessionEnd(svchp_, errhp_, usrhp_, (ub4)OCI_DEFAULT);
	SDO_OCIServerDetach(srvhp_, errhp_, (ub4)OCI_DEFAULT);


	SDO_OCIHandleFree((dvoid*)tdo_geometry_, (ub4)OCI_HTYPE_DESCRIBE);
	SDO_OCIHandleFree((dvoid*)tdo_ordinates_, (ub4)OCI_HTYPE_DESCRIBE);

	// Finalize svchp_, srvhp_, errhp_ and stmthp_
	SDO_OCIHandleFree((dvoid*)txnhp_, (ub4)OCI_HTYPE_TRANS);
	SDO_OCIHandleFree((dvoid*)srvhp_, (ub4)OCI_HTYPE_SERVER);
	SDO_OCIHandleFree((dvoid*)svchp_, (ub4)OCI_HTYPE_SVCCTX);
	SDO_OCIHandleFree((dvoid*)errhp_, (ub4)OCI_HTYPE_ERROR);
	SDO_OCIHandleFree((dvoid*)usrhp_, (ub4)OCI_HTYPE_SESSION);
	SDO_OCIHandleFree((dvoid*)stmthp_, (ub4)OCI_HTYPE_STMT);
	
	SDO_OCIHandleFree((dvoid*)envhp_, (ub4)OCI_HTYPE_ENV);

	

	connected_ = false;		
	envhp_ = 0;
	errhp_ = 0;
	srvhp_ = 0;
	svchp_ = 0;
	usrhp_ = 0;
	txnhp_ = 0;
	tdo_geometry_ = 0;
	tdo_ordinates_ = 0;
	ordinates_ = 0;
	stmthp_ = 0;
	if(blobValue_)
		delete [] blobValue_;
	blobValue_ = 0;
}


bool TeOCIConnection::prepare(const string& stmt)
{	
	if (!connected_)
		return false;
	
	ub4 size = stmt.size();
	sword status = SDO_OCIStmtPrepare(svchp_, (OCIStmt *)stmthp_, errhp_, (text*)stmt.c_str(), (ub4)size,  
		(text*)0, (ub4)0, (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT);
	if(!checkError(errhp_, status))
		return false;

	return true;
}


bool TeOCIConnection::bindByPos(int pos, void* value, int size, void* indicator, int type)
{
	if (!connected_)
		return false;

	OCIBind *bindp = NULL;

	sword status = SDO_OCIBindByPos(stmthp_, &bindp, errhp_, pos, (dvoid *)value, 
								(sb4)size, type, (dvoid *)indicator, (ub2 *)0, 
								(ub2 *)0, (ub4)0, (ub4 *)0, (ub4)OCI_DEFAULT);
	if(!checkError(errhp_, status))
		return false;
	return true;
}

bool TeOCIConnection::bindOrdinates()
{
	OCIBind		*bnd1p = NULL;
	
	/* bind coordinate varray object */
	sword status = OCIBindByName(stmthp_, &bnd1p, errhp_, 
	    (text *)":ordinates_", (sb4)-1, (dvoid *)0, (sb4)0, SQLT_NTY, (dvoid *)0, 
		(ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, (ub4)OCI_DEFAULT);
	if(!checkError(errhp_, status))
		return false;

	status = OCIBindObject(bnd1p, errhp_, tdo_ordinates_, (dvoid **)&ordinates_, (ub4 *)0, 
	    (dvoid **)0, (ub4 *)0);
	if(!checkError(errhp_, status))
		return false;

	return true;
}

int TeOCIConnection::queryType()  
{	
	ub2 querytype_;
		
	sword status = OCIAttrGet((dvoid *)stmthp_, (ub4)OCI_HTYPE_STMT, (ub2 *)&querytype_,
	(ub4*)NULL, (ub4)OCI_ATTR_STMT_TYPE, (OCIError *)errhp_);
	if(!checkError(errhp_, status))
		return -1;
	return (querytype_);
}


bool TeOCIConnection::allocateObjectOrdinates()
{
	sword status = SDO_OCIObjectNew(envhp_, errhp_, svchp_, OCI_TYPECODE_VARRAY, tdo_ordinates_, (dvoid*)NULL, 
					OCI_DURATION_SESSION, TRUE, (dvoid**)&ordinates_);

	if(!checkError(errhp_, status))
		return false;

	return true;
}


bool TeOCIConnection::appendOrdinates(const double& val)
{
	
	OCINumber	oci_number;
	sword       status;
	
	status = OCINumberFromReal(errhp_, (dvoid *)&(val), 
		(uword)sizeof(double),&oci_number);

	status = OCICollAppend(envhp_, errhp_, 
		(dvoid *) &oci_number,
		(dvoid *)0, (OCIColl *)ordinates_);

	if(!checkError(errhp_, status))
		return false;

	return true;
}

bool TeOCIConnection::transStart()
{
	sword status = OCITransStart (svchp_, errhp_, 60, OCI_TRANS_NEW); 
	if(!checkError(errhp_, status))
		return false;

	return true;
}

bool TeOCIConnection::transRollback()
{
	sword status = OCITransRollback(svchp_, errhp_, (ub4) 0);
	if(!checkError(errhp_, status))
		return false;

	return true;
}

bool TeOCIConnection::transCommit()
{
	sword status = OCITransCommit(svchp_, errhp_, (ub4) 0);
	if(!checkError(errhp_, status))
		return false;

	return true;
}

bool TeOCIConnection::execute(const string& stm)
{
	if(!prepare(stm))
		return false;
	
	if ((queryType()<0) || (queryType()==1))
		return false;  //return records (query)
	
	transStart();
	sword status = OCIStmtExecute(svchp_, stmthp_, errhp_, (ub4)1, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, (ub4)OCI_DEFAULT);
	
	if(!checkError(errhp_, status))
	{
		transRollback();
		return false;
	}

	transCommit();
	return true; 
}


bool TeOCIConnection::executeSDOSTM(const string& stm)  
{
	if(!prepare(stm))
		return false;

	if(!bindOrdinates())
		return false;

	if((queryType()<0) || (queryType()==1))
		return false;  //return records (query)
		
	transStart();
	sword status = OCIStmtExecute(svchp_, stmthp_, errhp_, (ub4)1, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, (ub4)OCI_DEFAULT);
	
	if(!checkError(errhp_, status))
	{		
		OCIObjectFree(envhp_, errhp_, (dvoid *)ordinates_, (ub2)OCI_OBJECTFREE_FORCE);
		transRollback();
		return false;
	}

	transCommit();
	OCIObjectFree(envhp_, errhp_, (dvoid *)ordinates_, (ub2)OCI_OBJECTFREE_FORCE);
	return true; 
}

bool 
TeOCIConnection::executeBLOBSTM(const string& stm, unsigned char* blobValue, const int& blobLength, const string& name)
{
	//prepare the statement
	if(!prepare(stm))
		return false;
	
	if((queryType()<0) || (queryType()==1))
		return false;  //return records (query)

	//Bind blob
	OCIBind		*bnd1p = NULL;
	sword status = OCIBindByName(stmthp_, &bnd1p, errhp_,
                (text *) name.c_str(), (sb4) name.size(),
                (dvoid *) blobValue, (sb4) blobLength, SQLT_LBI,
                (dvoid *) 0, (ub2 *)0, (ub2 *) 0,
                (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	status = OCIStmtExecute(svchp_, stmthp_, errhp_, (ub4)1, (ub4)0, (OCISnapshot *)NULL, 
						  (OCISnapshot *)NULL, (ub4)OCI_DEFAULT);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	return true;
}



bool TeOCIConnection::checkError(OCIError* errhp_, int status) 
{
	char errbuf[512];
	sb4 errcode = 0;
	bool returnedVal = false;
		
	if (status == OCI_ERROR)
	{
		SDO_OCIErrorGet((dvoid*)errhp_, (ub4)1, (text*)NULL, &errcode, 
						(text*)errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
		errorMess_ = errbuf;
		return false;
	}

	switch (status)
	{ 
		case OCI_SUCCESS:
			//errorMess_ = "Success!";
			returnedVal = true;
			break;

		case OCI_SUCCESS_WITH_INFO:
			//errorMess_ = "Success with information!";
			returnedVal = true;
			break;

		case OCI_NEED_DATA:
			errorMess_ = "Need data!";
			break;
		
		case OCI_NO_DATA:
			errorMess_ = "No data!";
			break;

		//An invalid handle was passed as a parameter or a user callback is passed an
		//invalid handle or invalid context. No further diagnostics are available.
		case OCI_INVALID_HANDLE:
			errorMess_ = "Invalid handle!";
			break;

		case OCI_STILL_EXECUTING:
			errorMess_ = "Still executing!";
			break;

		case OCI_CONTINUE:
			errorMess_ = "Continue!";
			break;
		default:
			break;
	}

	return returnedVal;
}
