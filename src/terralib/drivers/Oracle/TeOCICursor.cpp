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


#include "TeOCICursor.h"
#include "TeOCISDO.h"
#include "TeUtils.h"

//C include files
#include <cstring>
#include <cstdlib>


string getSQLToLoadBlobLength(const string& sql, const vector<string>& blobNames)
{
	string sqlSelectAux, sqlFromAux, sqlWhereAux;
	
	//Select clause
	size_t posFrom = sql.find(" FROM ", 0);
	sqlSelectAux = sql.substr(0, posFrom);
	
	//From and Where clause
	sqlWhereAux = " ";
	size_t posWhere = sql.find(" WHERE ", 0);
	if(posWhere != string::npos)
	{
		sqlFromAux = sql.substr(posFrom, (posWhere-posFrom)+1);
		sqlWhereAux = sql.substr(posWhere);
	}
	else
		sqlFromAux = sql.substr(posFrom);
	 
	string beforeBlobName="";
	//Verify if there is  string "SELECT *" 
	size_t pos =  sqlSelectAux.find (" *", 0);
	if(pos != string::npos) 
	{
		//two option:
		//1: if there is only 1 table -> include an alias in the FROM clause  
		//2: if there are more than 1 table -> include SELECT * ... and alias in the FROM clause
		size_t posJoin = sqlFromAux.find(" JOIN ", 0);
		size_t posVirg = sqlFromAux.find(",", 0);

		if((posJoin!=string::npos) || (posVirg!=string::npos)) //there are more than 1
		{
			string aux = " FROM ( SELECT * "+ sqlFromAux + sqlWhereAux +" ) t ";
			sqlFromAux = aux;
			sqlWhereAux = "";
		}
		else //there are only 1
			sqlFromAux += " t ";
			
		sqlSelectAux.replace(pos+1, 1, " t.* ");
		beforeBlobName = "t.";
	}
	
	for(unsigned int i=0; i<blobNames.size(); ++i)
		sqlSelectAux += " , LENGTH("+ beforeBlobName + blobNames[i] +") ";
		
	return (sqlSelectAux+sqlFromAux+sqlWhereAux);
	
}

//Given a string, remove all blanks and tabs (rigth)
char* StrClean(char *name)
{
    int j;
	
	if ((name == NULL)) /*SGError.Handler(NULLPOINTER,FATAL);*/ 
		return 0;

	for (j = strlen(name)-1; j>=0; j--)
	{
		if ( !((name[j]==' ') || (name[j]=='\t') || (name[j]== '\0')) ) 
			break;
	}
	
	name[j+1] = '\0';

	return name;
}


TeOCICursor::TeOCICursor(TeOCIConnection* connec, const unsigned long& maxBlobSize, const unsigned long& maxRowNum)
{
	if(!connec)
		return;
	
	maxRows_ = MAX_ROWS;
	maxBuflen_ = MAX_BLOB_LENGTH;

	//size in bytes to each blob
	if(maxBlobSize>0)
		maxBuflen_ = maxBlobSize;

	if(maxRowNum>0)
		maxRows_ = maxRowNum;
	
	connection_ = connec;
	stmthpToQuery_ = NULL;
	dschp_ = NULL;
	fieldValue_ = "";
	ordinates_ = 0;
	isOpen_ = false;
	hasBlob_ = false;
	row_Index_ = -1;
	rows_Fetched_ = 0;
	rows_Mem_ = 0;
	row_Cur_ = -1;
	last_Row_ = false;
	errorMessage_ = "";
	numColls_ = -1;
	colBlobName_.clear();

	if(connection_->useSDOType())
	{
		for(int i=0;i<maxRows_;i++)
		{
			global_geom_obj_[i] = NULL;
			global_geom_ind_[i] = NULL;
		}
	}
}

bool TeOCICursor::open()  
{
	if(!connection_)
		return false;

	sword status;
	if(isOpen_)
		this->close();
	isOpen_ = false;

	// Initialize statement handle 
	status = SDO_OCIHandleAlloc((dvoid*)connection_->envhp_, (dvoid**)&stmthpToQuery_, 
								  (ub4)OCI_HTYPE_STMT, (size_t)0, (dvoid**)0);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;
	
	// describe spatial object types (OCIDescribe)
	status = SDO_OCIHandleAlloc((dvoid*)connection_->envhp_, (dvoid**)&dschp_, 
								(ub4)OCI_HTYPE_DESCRIBE, (size_t)0, (dvoid **)0);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
	{
		//free stmthpToQuery_ located previously
		SDO_OCIHandleFree((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT);
		return false;
	}

	if(connection_->useSDOType())
	{
		status = SDO_OCIObjectNew(connection_->envhp_, connection_->errhp_, connection_->svchp_, OCI_TYPECODE_VARRAY, 
					connection_->tdo_ordinates_, (dvoid*)NULL, OCI_DURATION_SESSION, TRUE, 
					(dvoid**)&ordinates_);

		if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		{
			//free dschp_ located previously
			SDO_OCIHandleFree((dvoid*)dschp_, (ub4)OCI_HTYPE_DESCRIBE);
			//free stmthpToQuery_ located previously
			SDO_OCIHandleFree((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT);
            return false;
		}
	}
	isOpen_ = true;
	return true;
}


void TeOCICursor::close()
{
	freeResult(); //free all located memory
	// Finalize stmthpToQuery_ and dschp_
	if(isOpen_)
	{
		SDO_OCIHandleFree((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT);
		SDO_OCIHandleFree((dvoid*)dschp_, (ub4)OCI_HTYPE_DESCRIBE);
		stmthpToQuery_ = NULL;
		dschp_ = NULL;
	}
	isOpen_ = false;
	numColls_ = -1;
}

void TeOCICursor::freeResult()
{	
	//vector of buffers to store blob
	for(unsigned int b=0; b<lobBuffer_.size(); ++b)
	{
		if(lobBuffer_[b])
		{
			delete [] lobBuffer_[b];
			lobBuffer_[b] = NULL;
		}
	}
	lobBuffer_.clear();
	
	//buffer to store alphanumeric data
	for(unsigned int i=0; i<buffers_.size();i++)
	{
		int coltype = colType_[i];
		switch (coltype)
		{
			case 3: //INTEGER
				delete [] ((signed int*) buffers_[i]);
				break;
			
			case 2: //NUMBER
				delete [] ((OCINumber*) buffers_[i]);
				break;
			
			case 4: //FLOAT DOUBLE
				delete [] ((double*) buffers_[i]);
				break;
			
			case 96: //CHAR
			case 9: //VARCHAR:
			case 1: //VARCHAR2:	
				delete [] ((char *) buffers_[i]);
				break;
			
			case 12: //Date
					delete [] ((OCIDate *) buffers_[i]);
					break;
			default:
					break;
			
		} //switch
	} //for

	//sdo geometry
	if(connection_->useSDOType())  
	{
		for(int i=0;i<maxRows_;i++) 
		{
			/// free the spatial object instance 
			OCIObjectFree(connection_->envhp_, connection_->errhp_, (dvoid *)global_geom_obj_[i], 
				   (ub2)OCI_OBJECTFREE_FORCE);
			
			global_geom_obj_[i] = NULL;
			global_geom_ind_[i] = NULL;
		}

		OCIObjectFree(connection_->envhp_, connection_->errhp_, (dvoid *)ordinates_, (ub2)OCI_OBJECTFREE_FORCE);
		ordinates_ = 0;
	}

	buffers_.clear();
	defines_.clear(); // ver se o freestmtp desaloca os OCIDefines 
	ind_.clear();
	colType_.clear();
	colSize_.clear();
	colScale_.clear();
	colName_.clear();
	numColls_ = -1;
	hasBlob_ = false;
	colBlobName_.clear();
}


void TeOCICursor::defineByPos(int pos, void* value, int size, void* indicator, 
						 int type)
{
	OCIDefine *defnp = NULL;

	SDO_OCIDefineByPos(stmthpToQuery_, &defnp, connection_->errhp_, (ub4)pos, 
								  (dvoid *)value, (sb4)size, type, 
								  (dvoid *)indicator, (ub2 *)0, (ub2 *)0,
								  (ub4)OCI_DEFAULT);
}


bool TeOCICursor::fetch(int rows)
{
	sword status;
	status = OCIStmtFetch(stmthpToQuery_, connection_->errhp_, (ub4) rows, (ub4) OCI_FETCH_NEXT,
                               (ub4) OCI_DEFAULT);

	if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO)
		return true;
	else
		return false;

}

bool TeOCICursor::appendOrdinates(const double& val)
{
	
	OCINumber	oci_number;
	sword       status;

	if(!ordinates_)
	{
		status = SDO_OCIObjectNew(connection_->envhp_, connection_->errhp_, connection_->svchp_, OCI_TYPECODE_VARRAY, 
					connection_->tdo_ordinates_, (dvoid*)NULL, OCI_DURATION_SESSION, TRUE, 
					(dvoid**)&ordinates_);

		if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
			return false;
	}
	
	status = OCINumberFromReal(connection_->errhp_, (dvoid *)&(val), 
		(uword)sizeof(double),&oci_number);

	status = OCICollAppend(connection_->envhp_, connection_->errhp_, 
		(dvoid *) &oci_number,
		(dvoid *)0, (OCIColl *)ordinates_);

	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	return true;
}

bool TeOCICursor::bindOrdinates()
{
	OCIBind		*bnd1p = NULL;
	sword		status; 

	if(!ordinates_)
	{
		status = SDO_OCIObjectNew(connection_->envhp_, connection_->errhp_, connection_->svchp_, OCI_TYPECODE_VARRAY, 
					connection_->tdo_ordinates_, (dvoid*)NULL, OCI_DURATION_SESSION, TRUE, 
					(dvoid**)&ordinates_);

		if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
			return false;
	}
	
	/* bind coordinate varray object */
	status = OCIBindByName(stmthpToQuery_, &bnd1p, connection_->errhp_, 
	    (text *)":ordinates_", (sb4)-1, (dvoid *)0, (sb4)0, SQLT_NTY, (dvoid *)0, 
		(ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, (ub4)OCI_DEFAULT);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	status = OCIBindObject(bnd1p, connection_->errhp_, connection_->tdo_ordinates_, (dvoid **)&ordinates_, (ub4 *)0, 
	    (dvoid **)0, (ub4 *)0);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	return true;
}

bool TeOCICursor::moveFirst()
{		
	sword status;
	if(!hasBlob_)
		// this is for scroolable cursor
		status = OCIStmtFetch2(stmthpToQuery_, connection_->errhp_, (ub4)maxRows_, OCI_FETCH_FIRST, (sb4) 0, OCI_DEFAULT);
	else
		status = OCIStmtFetch2(stmthpToQuery_, connection_->errhp_, (ub4)maxRows_, OCI_FETCH_NEXT, (sb4) 0, OCI_DEFAULT);

	checkError(status);
	if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO  || status == OCI_NO_DATA)
	{
		OCIAttrGet((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT,
			     (dvoid *)&rows_Fetched_, (ub4 *)0, 
			     (ub4)OCI_ATTR_ROW_COUNT, connection_->errhp_);

		if(!rows_Fetched_)  
			return false;

		if(status == OCI_NO_DATA)
			last_Row_ = true;

		row_Index_ = 0;

		if(rows_Fetched_ < maxRows_)  
			rows_Mem_ = rows_Fetched_;
		else
			rows_Mem_ = maxRows_;

		row_Cur_ = 0;
		return true;
	}
	return false;
}

bool TeOCICursor::moveNext()
{
	sword status;
		
	if((row_Index_+1) >= rows_Mem_) 
	{
		if(last_Row_)
			return false;
		
		// this is for scroolable cursor
		status = OCIStmtFetch2(stmthpToQuery_, connection_->errhp_, (ub4) maxRows_, OCI_FETCH_NEXT, (sb4) 0, OCI_DEFAULT);

		if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO || status == OCI_NO_DATA)
		{
			OCIAttrGet((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT,
					(dvoid *)&rows_Fetched_, (ub4 *)0, 
					(ub4)OCI_ATTR_ROWS_FETCHED, connection_->errhp_);

			if(!rows_Fetched_)
				return false;

			if(status == OCI_NO_DATA)
			{
				if(last_Row_)
					return false;
				else
					last_Row_ = true;
			}

			row_Index_ = 0;

			if(rows_Fetched_ < maxRows_)
				rows_Mem_ = rows_Fetched_;
			else
				rows_Mem_ = maxRows_;

			row_Cur_++;
			return true;
		}
		else
			return false;
	}
	else
		row_Index_++;
	
	row_Cur_++;
	return true;
}

bool TeOCICursor::moveLast()  
{
	sword status = OCIStmtFetch2(stmthpToQuery_, connection_->errhp_, (ub4) 1, 
                               OCI_FETCH_LAST, (sb4) 0, OCI_DEFAULT);
	//podemos pegar informacao da ultima linha
	if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO)
	{
		last_Row_ = true;
		return true;
	}
	else
		return false;
}

bool TeOCICursor::moveTo(int pos) //begin in 0
{
	// It is not possible use scrollable cursor with blob
	if(hasBlob_) 
	{
		// for non-scroolable cursor, needs to query again
		// if pos is at the end of recordset may be slow
		int topos;
		if(pos > 0)
			topos = pos-1;
		else
			topos = row_Cur_ + pos;
		if(topos < 0)
			topos = 0;

		if(!moveFirst())
			return false;

		while(row_Cur_ != topos)
		{
			if(!moveNext())
				return false;
		}
		return true;
	}

	sword status;
	int auxPos = maxRows_*int(pos/maxRows_);

	// row_Cur_ = absolute current row 
	// row_Index_ = relative current row - client side
	if(int(row_Cur_/maxRows_) == int(pos/maxRows_)) 
	{
		row_Index_ = pos - (maxRows_*int(pos/maxRows_));
		row_Cur_ = pos;
		return true;
	}

	if(pos<row_Cur_)
	{
		if(!moveFirst())
			return false;
	}

	status = OCIStmtFetch2(stmthpToQuery_, connection_->errhp_, (ub4) maxRows_, OCI_FETCH_ABSOLUTE, (sb4)auxPos+1, OCI_DEFAULT); 
	row_Index_ = pos-auxPos;
	row_Cur_ = pos;

	if (status == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO || status == OCI_NO_DATA)
	{
		OCIAttrGet((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT,
				 (dvoid *)&rows_Fetched_, (ub4 *)0, 
				 (ub4)OCI_ATTR_ROWS_FETCHED, connection_->errhp_);

		if(!rows_Fetched_)
			return false;
		
		if(status == OCI_NO_DATA)
			last_Row_ = true;
		return true;
	}

	return false;
}

// Throw CCursorException if OCI error found
bool TeOCICursor::checkError(sword status)
{
	sb4 errcode = 0;
	char message[256];
	bool returnedVal = false;

	if (status == OCI_ERROR)
	{
		SDO_OCIErrorGet((dvoid*)connection_->errhp_, (ub4)1, (text*)NULL, &errcode, 
						(text*)message, (ub4)256, OCI_HTYPE_ERROR);

		errorMessage_ = message;
		return false;
	}
	
	switch (status)
	{ 
		case OCI_SUCCESS:
			errorMessage_ = "Success!";
			returnedVal = true;
			break;

		case OCI_SUCCESS_WITH_INFO:
			errorMessage_ = "Success with information!";
			returnedVal = true;
			break;

		case OCI_NEED_DATA:
			errorMessage_ = "Need data!";
			break;
		
		case OCI_NO_DATA:
			errorMessage_ = "No data!";
			break;

		//An invalid handle was passed as a parameter or a user callback is passed an
		//invalid handle or invalid context. No further diagnostics are available.
		case OCI_INVALID_HANDLE:
			errorMessage_ = "Invalid handle!";
			break;

		case OCI_STILL_EXECUTING:
			errorMessage_ = "Still executing!";
			break;

		case OCI_CONTINUE:
			errorMessage_ = "Continue!";
			break;
		default:
			break;
	}

	return returnedVal;

}


bool TeOCICursor::prepare(const string& stmt)
{	
	ub4 size = stmt.size();
	sword status = SDO_OCIStmtPrepare(connection_->svchp_, (OCIStmt *)stmthpToQuery_, connection_->errhp_, (text*)stmt.c_str(), (ub4)size,  
		(text*)0, (ub4)0, (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT);
	if((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	return true;
}


int TeOCICursor::queryType()  
{	
	ub2 querytype_;
		
	sword status = OCIAttrGet((dvoid *)stmthpToQuery_, (ub4)OCI_HTYPE_STMT, (ub2 *)&querytype_,
	(ub4*)NULL, (ub4)OCI_ATTR_STMT_TYPE, (OCIError *)connection_->errhp_);
	if((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return -1;
	return (querytype_);
}

bool TeOCICursor::query(const string& query)
{
	if (!isOpen_)
		open();
			
	row_Index_ = -1;
	rows_Fetched_ = 0;
	rows_Mem_ = 0;
	row_Cur_ = -1;
	last_Row_ = false;

	if(!prepare(query))
		return false;	
	
	// query type equal to 1 = OCI_STMT_SELECT
	if(queryType()!=1) 
		return false;
		
	 //iters equal to zero because the defines_ (OCIDefines) have not located yet 
	sword status = OCIStmtExecute(connection_->svchp_, stmthpToQuery_, connection_->errhp_, (ub4)0, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, OCI_STMT_SCROLLABLE_READONLY);
	checkError(status);
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;

	loadCollDescription(); // load columns description
	
	if(hasBlob_)
	{
		vector<string> auxBlobColls;
		string sql ="";
		for(unsigned int blobIndex=0; blobIndex<colType_.size(); ++blobIndex)
		{
			if(colType_[blobIndex]==113) //blob type
				auxBlobColls.push_back(colName_[blobIndex]);
		}
		
		if(!auxBlobColls.empty())
			sql = getSQLToLoadBlobLength(query, auxBlobColls);

		freeResult();
		colBlobName_ = auxBlobColls;

		maxRows_ = 10; //get only 10 rows from server when there is blob type.
		
		if(!prepare(sql))
			return false;
		
		// Is is not possible use the scrollable cursor with blob data type
		status = OCIStmtExecute(connection_->svchp_, stmthpToQuery_, connection_->errhp_, (ub4)0, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, OCI_DEFAULT);
		if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
			return false;

		loadCollDescription(); // to get the new attribute in SQL
	}

	if(!allocateCursor())
		return false;
	return true; 
}


bool TeOCICursor::querySDO(const string& query)
{
	if (!isOpen_)
		open();

	row_Index_ = -1;
	rows_Fetched_ = 0;
	rows_Mem_ = 0;
	row_Cur_ = -1;
	last_Row_ = false;

	if(!prepare(query))
		return false;

	if(!bindOrdinates())
		return false;	

	// query type equal to 1 = OCI_STMT_SELECT
	if (queryType() != 1) //must be executed by TeOCIConnect->execute()
		return false;
		
	//iters equal to zero because the defines_ (OCIDefines) have not located yet 
	sword status = OCIStmtExecute(connection_->svchp_, stmthpToQuery_, connection_->errhp_, (ub4)0, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, OCI_STMT_SCROLLABLE_READONLY);
	
	if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
		return false;
	
	loadCollDescription(); // load columns description

	if(hasBlob_)
	{
		vector<string> auxBlobColls;
		string sql ="";
		for(unsigned int blobIndex=0; blobIndex<colType_.size(); ++blobIndex)
		{
			if(colType_[blobIndex]==113) //blob type
				auxBlobColls.push_back(colName_[blobIndex]);
		}
		
		if(!auxBlobColls.empty())
			sql = getSQLToLoadBlobLength(query, auxBlobColls);

		freeResult();
		colBlobName_ = auxBlobColls;

		maxRows_ = 10; //get only 10 rows from server when there is blob type.

		if(!prepare(sql))
			return false;
		
		// Is is not possible use the scrollable cursor with blob data type
		status = OCIStmtExecute(connection_->svchp_, stmthpToQuery_, connection_->errhp_, (ub4)0, (ub4)0, (OCISnapshot *)NULL, 
								  (OCISnapshot *)NULL, OCI_DEFAULT);
		if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
			return false;

		loadCollDescription(); // to get the new attribute in SQL
	}

	if(!allocateCursor())
		return false;

	return true; 
}


int TeOCICursor::numCol()  
{	
	if(numColls_>=0)
		return (numColls_-(colBlobName_.size()));
	
	/* Get the number of columns in the query */
	OCIAttrGet(stmthpToQuery_, OCI_HTYPE_STMT, &numColls_,
	0, OCI_ATTR_PARAM_COUNT, connection_->errhp_);
	
	return (numColls_-(colBlobName_.size()));
}


int TeOCICursor::numRows()
{
	int numrows_ = 0;
	
	/* Get the number of rows in the query */
	SDO_OCIAttrGet((dvoid*)stmthpToQuery_, (ub4)OCI_HTYPE_STMT, (dvoid*)&numrows_,
	(ub4*)0, (ub4)OCI_ATTR_ROW_COUNT, (OCIError*)connection_->errhp_);
	return (numrows_);

}


void TeOCICursor::loadCollDescription() 
{
	OCIParam* colhd=NULL;
	//int		dtype;
	ub2	dtype = 0;
	string	colname;
	//int		colsize;
	ub2  colsize = (ub2) 0;
	//int		colscale;
	sb1  colscale = (sb1) 0;
	
	numCol(); // atualize the numColls_ variable
	hasBlob_ = false;

	int i;
	for(i=1; i<=numColls_; ++i)
	{
		// get parameter for i-th column
		SDO_OCIParamGet((dvoid*)stmthpToQuery_, (ub4)OCI_HTYPE_STMT, (OCIError *)connection_->errhp_, (dvoid**)&colhd, (ub4)i);

		// get data type 
		SDO_OCIAttrGet((dvoid *)colhd, (ub4)OCI_DTYPE_PARAM, (dvoid *)&dtype, (ub4*)0, (ub4)OCI_ATTR_DATA_TYPE, (OCIError *)connection_->errhp_);

		// get coll name 
		text *colname_ = NULL;
		ub4	colnamesz_;
				
		OCIAttrGet((dvoid *)colhd, (ub4)OCI_DTYPE_PARAM, (dvoid **)&colname_, (ub4*)&colnamesz_, (ub4)OCI_ATTR_NAME, (OCIError *)connection_->errhp_ );

		char temp[100];
		for(int j=0;j<(int)colnamesz_;j++)
			temp[j] = colname_[j];
		temp[colnamesz_] = '\0';
		colname = temp;

		// retrieve the column size attribute
		SDO_OCIAttrGet((dvoid *)colhd, (ub4)OCI_DTYPE_PARAM, (dvoid *)&colsize, (ub4*)0, (ub4)OCI_ATTR_DATA_SIZE, (OCIError *)connection_->errhp_ );

		// retrieve the column scale attribute
		SDO_OCIAttrGet((dvoid *)colhd, (ub4)OCI_DTYPE_PARAM, (dvoid *)&colscale, (ub4*)0, (ub4)OCI_ATTR_SCALE, (OCIError *)connection_->errhp_ );

		colName_.push_back(colname);
		colType_.push_back((int)dtype);
		colSize_.push_back((int)colsize);
		colScale_.push_back((int)colscale);

		if(dtype==113)
			hasBlob_ = true;
	}
}


int TeOCICursor::colType (int colnumber)   
{
	//first coll number is 1
	if((colnumber==0) || (colnumber>(int)colType_.size()))
		return 0;

	return colType_[colnumber-1];
}


string TeOCICursor::colName (int colnumber) 
{	
	//first coll number is 1
	if((colnumber==0) || (colnumber>(int)colName_.size()))
		return "";

	return colName_[colnumber-1];
}


int TeOCICursor::colSize (int colnumber) 
{
	//first coll number is 1
	if((colnumber==0) || (colnumber>(int)colSize_.size()))
		return 0;

	return colSize_[colnumber-1];
}


int TeOCICursor::colScale (int colnumber) 
{
	//first coll number is 1
	if((colnumber==0) || (colnumber>(int)colScale_.size()))
		return 0;

	return colScale_[colnumber-1];
}


 bool TeOCICursor::allocateCursor()
 {
	int			size;
	int			coltype=0;
	int			colsize=0;
	int			colscale=0;
	sword		status;
	
	try
	{
		ind_.resize(numColls_);

		for(int nc=0; nc<numColls_; ++nc)
		{
			defines_.push_back (0);
			buffers_.push_back (0);
		}

		for(int i=1; i<=numColls_;i++)
		{
			coltype = colType_[i-1];
			colsize = colSize_[i-1];
			colscale = colScale_[i-1];

			switch (coltype)
			{
				case 3: //INTEGER
					buffers_[i-1] = new signed int[maxRows_];
					colsize = sizeof(signed int);

					status = OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)(signed int*)buffers_[i-1], (sb4)colsize, coltype, 
							(dvoid *)&ind_[i-1], (ub2 *)0, (ub2 *)0,(ub4)OCI_DEFAULT);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
						return false;

					break;

			
				case 2: //NUMBER
										
					buffers_[i-1] = (OCINumber *) new OCINumber[maxRows_]; //();								
					status = OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)buffers_[i-1], sizeof(OCINumber), SQLT_VNU, 
							(dvoid *)&ind_[i-1], (ub2 *)0, (ub2 *)0,(ub4)OCI_DEFAULT);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
						return false;
			
					break;
							

				case 4: //FLOAT DOUBLE
					
					buffers_[i-1] = new double[maxRows_];
					colsize = sizeof(double);
					coltype = 4;
				
					status = SDO_OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)(double*)buffers_[i-1], (sb4)colsize, coltype, 
							(dvoid *)&ind_[i-1], (ub2 *)0, (ub2 *)0, (ub4)OCI_DEFAULT);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
						return false;
				
				
					break;

				case 96: //CHAR
				case 9: //VARCHAR:
				case 1: //VARCHAR2:
					buffers_[i-1] = (char *) new char[maxRows_*(colsize+1)];
			
					status = SDO_OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)buffers_[i-1], (sb4)(colsize+1), SQLT_STR, 
							(dvoid *)&ind_[i-1], (ub2 *)0, (ub2 *)0,(ub4)OCI_DEFAULT);
						if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;
				
					break;

				case 12: //Date
					buffers_[i-1] = (OCIDate *) new OCIDate[maxRows_]; //();								
					status = OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)buffers_[i-1], sizeof(OCIDate), SQLT_ODT, 
							(dvoid *)&ind_[i-1], (ub2 *)0, (ub2 *)0,(ub4)OCI_DEFAULT);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;
				
					break;


				case 108: //OBJECT SDO_GEOMETRY
				
					status = SDO_OCIObjectNew(connection_->envhp_, connection_->errhp_, connection_->svchp_,
							OCI_TYPECODE_OBJECT, connection_->tdo_geometry_, (dvoid*)NULL, 
							OCI_DURATION_SESSION, TRUE, 
							(dvoid**)global_geom_obj_);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;
					
					defines_[i-1] = NULL;
								
					status = SDO_OCIDefineByPos(stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
							(dvoid *)0, (sb4)0, SQLT_NTY, (dvoid *)&ind_[i-1],
							(ub2 *)0, (ub2 *)0, (ub4)OCI_DEFAULT);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;
					
					status = OCIDefineObject(defines_[i-1], connection_->errhp_, connection_->tdo_geometry_, 
							(dvoid **)global_geom_obj_, (ub4 *)0, 
							(dvoid **)global_geom_ind_, (ub4 *)0);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;

					break;

				case 113 :	//SQLT_BLOB
					
					hasBlob_ = true;
					lobBuffer_.push_back(NULL);
					lobBuffer_[lobBuffer_.size()-1] = new unsigned char[maxBuflen_*maxRows_]; 
					size = sizeof(unsigned char)*maxBuflen_;
										
					status = OCIDefineByPos (stmthpToQuery_, &(defines_[i-1]), connection_->errhp_, (ub4)i, 
						(dvoid *)lobBuffer_[lobBuffer_.size()-1], size, SQLT_LBI, (void *)&ind_[i-1], (ub2 *)0, (ub2 *)0, 
						(ub4)OCI_DEFAULT); 
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
							return false;
					
					status = OCIDefineArrayOfStruct(defines_[i-1], connection_->errhp_, size, 0, 0, 0);
					if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
						return false;
					break;
				default:
					break;
			
			} //switch
	
		} //for
	}//try

	catch(...) 
	{
		return false;
	}

	return true;
}	

char* TeOCICursor::getFieldValue(int i)  //inicia em 1
{ 
	
	int				tempInt;
	double			tempDouble;
	char			str[30];
	unsigned int	size; 
	string			tempDate;
	sword			status;
		
	if(i < 1)
		return (char*)0;

	int coltype = colType_[i-1]; 
	int colsize = colSize_[i-1]; 
	
	//OBS:	When the SQL use grouping functions (MAX, MIN, etc), the returned value always has
	//		colscale equal to zero, even when it is double
	int indica = ind_[i-1].sbind[row_Index_];

	std::string format = "DD/MM/YYYY HH24:MI:SS";
	size = format.size();

	switch (coltype)
	{
		case 3: //INTEGER
			if(indica == -1)
				return "";

			tempInt = *((int*)buffers_[i-1]+row_Index_);
			fieldValue_ = Te2String(tempInt);
			return ((char*)fieldValue_.c_str());
			break;

		case 2: //NUMBER
			
			if(indica == -1)
				return "";
			status = OCINumberToReal(connection_->errhp_, ((OCINumber *)buffers_[i-1]+row_Index_), 
				(uword)sizeof(double), (dvoid *)&tempDouble);
			if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
				return "";
			
			fieldValue_ = Te2String(tempDouble, 10);
			return ((char*)fieldValue_.c_str());
			break;
			

		case 4: //FLOAT DOUBLE
				
			if(indica == -1)
				return "";
			
			tempDouble = *((const double*)buffers_[i-1]+row_Index_);
			fieldValue_ = Te2String(tempDouble, 10);
			return ((char*)fieldValue_.c_str());
			break;

		case 96: //CHAR
		case 9: //VARCHAR:
		case 1: //VARCHAR2:
			
			if(indica == -1)
				return "";

			fieldValue_ = ((char*)buffers_[i-1]+((colsize+1)*row_Index_));
			fieldValue_ = StrClean((char*)fieldValue_.c_str());
			return ((char*)fieldValue_.c_str());
			break;

		case 12: //Date
			
			if(indica == -1)
				return "";

			fieldValue_ = "";
			//size = sizeof(OCIDate);
													
			status =  OCIDateToText(connection_->errhp_, ((OCIDate *)buffers_[i-1]+row_Index_), 
				(unsigned char*)format.c_str(), format.size(), NULL, 0, &size, (unsigned char*)tempDate.c_str());
			if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
				return "";
			memcpy(str, tempDate.c_str(), size);
			strncat((char*)fieldValue_.c_str(), str, size);

			return ((char*)fieldValue_.c_str());
			break;

		case 108: //OBJECT SDO_GEOMETRY
		default:
			break;
							
	} //switch
		
	return (char*)"";
}

int TeOCICursor::getDimArraySize()
{
	int ndim=-1;

	// Get the size of the elem info array
    OCICollSize(connection_->envhp_, connection_->errhp_, 
				(OCIColl *)(global_geom_obj_[row_Index_]->sdo_elem_info), &ndim);

	return(ndim);
}

bool TeOCICursor::getDimElement(int i,int &elem)
{
	int				exists;
	OCINumber		*oci_number;
	double			el;


	OCICollGetElem(connection_->envhp_, connection_->errhp_, 
	   (OCIColl *)(global_geom_obj_[row_Index_]->sdo_elem_info), 
	   (sb4)i-1, (boolean *)&exists, 
	   (dvoid **)&oci_number, (dvoid **)0);

	OCINumberToReal(connection_->errhp_, oci_number, (uword)sizeof(double),
		(dvoid *)&el);
	
	elem = (int)el;
	return true;
}


int TeOCICursor::getNumberOrdinates()
{
	int nOrds=-1;

	/* Get the size of the ordinates_ array */
    OCICollSize(connection_->envhp_, connection_->errhp_, 
				(OCIColl *)(global_geom_obj_[row_Index_]->sdo_ordinates), &nOrds);

	return(nOrds);
}

bool TeOCICursor::getCoordinates(int i, TeCoord2D& coord)
{
	int				exists;
	OCINumber		*oci_number;
	double			coor_x;
	double			coor_y;

	int pos = i;

	OCICollGetElem(connection_->envhp_, connection_->errhp_, 
	   (OCIColl *)(global_geom_obj_[row_Index_]->sdo_ordinates), 
	   (sb4)(pos-1), (boolean *)&exists, 
	   (dvoid **)&oci_number, (dvoid **)0);

	OCINumberToReal(connection_->errhp_, oci_number, (uword)sizeof(double),
		(dvoid *)&coor_x);
	
	pos++;

	OCICollGetElem(connection_->envhp_, connection_->errhp_, 
	   (OCIColl *)(global_geom_obj_[row_Index_]->sdo_ordinates),  
	   (sb4)(pos-1), (boolean *)&exists, 
	   (dvoid **)&oci_number, (dvoid **)0);

	OCINumberToReal(connection_->errhp_, oci_number, (uword)sizeof(double),
		(dvoid *)&coor_y);

	coord.x(coor_x);
	coord.y(coor_y);

	return true;
}


bool TeOCICursor::getCoordinates(vector<TeCoord2D>& result)
{
	
	OCIIter		*iterator;
	dvoid		*elem;
	OCIInd		*elemind ;
	double		ordinate1, ordinate2; 
	OCINumber	*aux1;
	OCINumber	*aux2;
	//boolean		eoc;
	int			eoc;

	sword status = OCIIterCreate(connection_->envhp_, connection_->errhp_, 
		(OCIArray *)(global_geom_obj_[row_Index_]->sdo_ordinates), &iterator);
	if (status != OCI_SUCCESS)
		return false;
	
	/* Get the first and second element of the clients varray */
	status = OCIIterNext(connection_->envhp_, connection_->errhp_, iterator, &elem,
		(dvoid **) &elemind, (boolean *)&eoc);
	if (eoc || status != OCI_SUCCESS)
	{
		OCIIterDelete(connection_->envhp_, connection_->errhp_, &iterator);
		return false;
	}

	aux1 = (OCINumber *)elem;
	OCINumberToReal(connection_->errhp_, (OCINumber *)aux1, (uword)sizeof(double),
					(dvoid *)&ordinate1);

	status = OCIIterNext(connection_->envhp_, connection_->errhp_, iterator, &elem,
				(dvoid **)&elemind, (boolean *)&eoc);
	if (eoc || status != OCI_SUCCESS)
	{
		OCIIterDelete(connection_->envhp_, connection_->errhp_, &iterator);
		return false;
	}

	aux2 = (OCINumber *)elem;
	OCINumberToReal(connection_->errhp_, (OCINumber *) aux2, (uword)sizeof(double),
					(dvoid *)&ordinate2);

	TeCoord2D coord(ordinate1, ordinate2);
	result.push_back (coord);
	
	while (!eoc && status == OCI_SUCCESS)
	{
		status = OCIIterNext(connection_->envhp_, connection_->errhp_, iterator, &elem,
				(dvoid **)&elemind, (boolean *)&eoc);
		if (status != OCI_SUCCESS)
		{
			OCIIterDelete(connection_->envhp_, connection_->errhp_, &iterator);
			return false;
		}

		aux1 = (OCINumber *)elem;
		OCINumberToReal(connection_->errhp_, (OCINumber *)aux1, (uword)sizeof(double),
					(dvoid *)&ordinate1);

		status = OCIIterNext(connection_->envhp_, connection_->errhp_, iterator, &elem,
				(dvoid **)&elemind, (boolean *)&eoc);
		if (status != OCI_SUCCESS)
		{
			OCIIterDelete(connection_->envhp_, connection_->errhp_, &iterator);
			return false;
		}

		aux2 = (OCINumber *)elem;
		OCINumberToReal(connection_->errhp_, (OCINumber *) aux2, (uword)sizeof(double),
					(dvoid *)&ordinate2);

		TeCoord2D coord(ordinate1, ordinate2);
		result.push_back (coord);
	}

	/* destroy the iterator */
	status = OCIIterDelete(connection_->envhp_, connection_->errhp_, &iterator);
	return true;
}


int
TeOCICursor::getGeometryType()
{
	int gtype=-1;

	OCINumberToInt(connection_->errhp_, &(global_geom_obj_[row_Index_]->sdo_gtype),
				 (uword)sizeof(int), OCI_NUMBER_SIGNED,
				 (dvoid *)&gtype);

	return gtype;
}

int
TeOCICursor::getSpatialReferenceId()
{
	int srid=-1;

	OCINumberToInt(connection_->errhp_, &(global_geom_obj_[row_Index_]->sdo_srid),
				 (uword)sizeof(int), OCI_NUMBER_SIGNED,
				 (dvoid *)&srid);

	return srid;
}

bool
TeOCICursor::getXYZcoord(double& x, double& y)
{
	if (global_geom_ind_[row_Index_]->sdo_point._atomic == OCI_IND_NOTNULL)
	{
		if (global_geom_ind_[row_Index_]->sdo_point.x == OCI_IND_NOTNULL)
		{
			sword status = OCINumberToReal(connection_->errhp_, &(global_geom_obj_[row_Index_]->sdo_point.x), 
										 (uword)sizeof(double),(dvoid *)&x);

			if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
				return false;
		}

		if (global_geom_ind_[row_Index_]->sdo_point.y == OCI_IND_NOTNULL)
		{
			sword status = OCINumberToReal(connection_->errhp_, &(global_geom_obj_[row_Index_]->sdo_point.y), 
										 (uword)sizeof(double),(dvoid *)&y);
			if ((status != OCI_SUCCESS) && (status!=OCI_SUCCESS_WITH_INFO))
				return false;
		}
		return true;
	}
	else
		return false;
}

bool
TeOCICursor::readBlob(unsigned char* buffer, unsigned int& bufsize, unsigned int& blobCol)
{
	string blobColS = colName_[blobCol];
	return(readBlob(buffer, bufsize, blobColS));
}

bool
TeOCICursor::readBlob(unsigned char* buffer, unsigned int& bufsize, const string& blobCol)
{
	int blobIndex = -1;
	if(colBlobName_.size()==1)
		blobIndex=0;
	else
	{
		for(unsigned int j=0; j<colBlobName_.size(); j++)
		{
			if(TeStringCompare(blobCol, colBlobName_[j]))
			{
				blobIndex = j;
				break;
			}
		}
	}

	if(blobIndex<0)
		return false;

	int index = numColls_-(colBlobName_.size()-(blobIndex+1)); 
	
	//bufsize em bytes
	bufsize = atoi(getFieldValue(index))*sizeof(unsigned char);
	//buffer = new unsigned char[bufsize];
	memcpy(buffer,&(lobBuffer_[blobIndex][row_Index_*maxBuflen_]),bufsize);
	return true;
}


bool
TeOCICursor::readBlob(double *buffer, unsigned int& bufsize)
{
	//bufsize em double
	//size em bytes
	int size = bufsize*sizeof(double);
	memcpy(buffer,&(lobBuffer_[0][row_Index_*maxBuflen_]),size);
	return true;
}

bool
TeOCICursor::readBlob(double **buffer)
{
	*buffer = (double*)&(lobBuffer_[0][row_Index_*maxBuflen_]);
	return true;
}

unsigned int 
TeOCICursor::getBlobSize(const string& blobCol)
{
	int blobIndex = -1;
	if(colBlobName_.empty())
		return 0;
	if(colBlobName_.size()==1)
		blobIndex=0;
	else
	{
		for(unsigned int j=0; j<colBlobName_.size(); j++)
		{
			if(TeStringCompare(blobCol, colBlobName_[j]))
			{
				blobIndex = j;
				break;
			}
		}
	}

	if(blobIndex<0)
		return 0;

	int index = numColls_-(colBlobName_.size()-(blobIndex+1)); 
	return (atoi(getFieldValue(index))*sizeof(unsigned char));
}

