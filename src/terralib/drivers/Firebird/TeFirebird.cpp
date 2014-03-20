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

#include "TeFirebird.h"
#include <stdio.h>
#include <sys/types.h>
#include <TeProject.h>
#include <cstring>

#ifndef MAX
  #define MAX(a,b) ((a>b)?a:b)
#endif
#ifndef MIN
  #define MIN(a,b) ((a<b)?a:b)
#endif

void convertChar2Blob(char* buffer, const int& size, IBPP::Blob& blob)
{
	int bufferSize = 10240,
		segments = size / bufferSize;

	char* aux = buffer;

	blob->Create();

	for(int i=0; i<segments; i++)
	{
		blob->Write(aux, bufferSize);
		aux += bufferSize;
	}
		
	int resto = size - (segments*bufferSize);

	if(resto > 0)
	{
		blob->Write(aux, resto);
	}
}

void convertBlob2Char(const IBPP::Blob& blob, char*& buffer, int& size)
{
	blob->Open();

	int bufferSize,
		segments;

	blob->Info(&size, &bufferSize, &segments);
	buffer = new char[size];

	segments = size/bufferSize;

	char* aux = buffer;

	for(int i=0; i<segments; i++)
	{		
		blob->Read(aux, bufferSize);
		aux += bufferSize;
	}

	int resto = size - bufferSize*segments;
	if(resto > 0)
	{
		blob->Read(aux, resto);
	}
}

void teRing2Buffer(const TeLinearRing& line, char*& buffer, int& bufferSize)
{
	double* points = NULL;
	TeLinearRing::iterator it;
	points = new double[2*line.size()]; 
	int iac = 0;

	for(unsigned int i = 0; i < line.size(); i++)
	{
		points[iac++]=line[i].x();
		points[iac++]=line[i].y();
	}

	bufferSize = 2*sizeof(double)*line.size();
	buffer = (char*)(points);
}

void buffer2TeRing(const char* buffer, const int& numCoords, TeLinearRing& line)
{
	for(int i=0; i<numCoords; i++)
	{
		double x,
			   y;

		memcpy(&x, buffer+((2*i)*sizeof(double)),sizeof(double));
		memcpy(&y, buffer+((2*i+1)*sizeof(double)),sizeof(double));

		line.add(TeCoord2D(x, y));
	}	
}

void loadFromBlob(IBPP::Statement& st, TeLinearRing& ring)
{
	int	nc;

	st->Get("num_coords", nc);

	IBPP::Blob blob = IBPP::BlobFactory(st->DatabasePtr(), st->TransactionPtr());
	st->Get("spatial_data", blob);

	int size;
	char* buffer;

	convertBlob2Char(blob, buffer, size);
	buffer2TeRing(buffer, size/(2*sizeof(double)), ring);

	blob->Close();

	delete []buffer;
}

void saveLineInBlob(IBPP::Blob& blob, TeLinearRing& ring)
{
	int bLen = 0; //Buffer size.

	char* buffer;

	teRing2Buffer(ring, buffer, bLen);

	convertChar2Blob(buffer, bLen, blob);

	blob->Close();

	delete []buffer;	
}

bool generateLablesForPolygonal(IBPP::Database db, TeTheme* theme, const TeGeomRep& geomType, 
								std::string& errorMsg, const std::string& objId)
{
	std::string collTable = theme->collectionTable(),
				geomTable = theme->layer()->tableName(geomType),
				query = "SELECT object_id, lower_x, lower_y, upper_x, upper_y FROM " + geomTable;

	TeDatabasePortal* portal = theme->layer()->database()->getPortal();

	bool res = true;

	if(!portal->query(query))
	{
		throw TeException(UNKNOWN_ERROR_TYPE, "Error on query:\n" + portal->errorMessage());
	}

	if(!portal->fetchRow())
	{
		return res;
	}

	IBPP::Transaction tr = IBPP::TransactionFactory(db);

	try
	{
		query = "UPDATE " + collTable + " SET label_x=?, label_y=? WHERE c_object_id=?";
		
		if(!objId.empty())
		{
			query += " AND c_object_id=?";
		}
		
		tr->Start();

		IBPP::Statement st = IBPP::StatementFactory(db, tr);

		st->Prepare(query);

		do
		{
			TeBox box(portal->getDouble(1), portal->getDouble(2), portal->getDouble(3), portal->getDouble(4));
			std::string oId = portal->getData(0);

			st->Set(1, box.center().x());
			st->Set(2, box.center().y());
			st->Set(3, oId);

			if(!objId.empty())
			{
				st->Set(4, objId);
			}

			st->Execute();

		} while(portal->fetchRow());		

		tr->Commit();
	}
	catch(IBPP::SQLException& e)
	{
		tr->Rollback();
		errorMsg = e.ErrorMessage();
		res = false;
	}
	catch(IBPP::LogicException& e)
	{
		tr->Rollback();
		errorMsg = e.ErrorMessage();
		res = false;
	}

	delete portal;
	return res;
}


void createGeneratorTrigger(TeFirebird* db)
{
	TeDatabasePortal* portal = db->getPortal();
	std::string sql = "SELECT RDB$TRIGGER_NAME FROM RDB$TRIGGERS WHERE RDB$TRIGGER_NAME = 'REM_AUTO_INCR'";

	if(!portal->query(sql))
	{
		throw;
	}

	bool exists = portal->fetchRow();
	delete portal;

	if(exists)
	{
		sql = "DELETE FROM RDB$TRIGGERS WHERE RDB$TRIGGER_NAME = 'REM_AUTO_INCR'";
		if(!db->execute(sql))
		{
			throw TeException(UNKNOWN_ERROR_TYPE, ("Error removing trigger REM_AUTO_INCR!"));
		}
	}
		
	sql = "CREATE TRIGGER REM_AUTO_INCR FOR RDB$GENERATORS ";
	sql += "ACTIVE BEFORE INSERT POSITION 0 ";
	sql += "AS BEGIN "; 
	sql += " IF (EXISTS (SELECT RDB$GENERATOR_NAME FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME = NEW.RDB$GENERATOR_NAME)) THEN ";
	sql += " DELETE FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME = NEW.RDB$GENERATOR_NAME;";
	sql += "END "; 

	if(!db->execute(sql))
	{
		throw TeException(UNKNOWN_ERROR_TYPE, ("Error creating trigger to remove generators!"));
	}
}

TeFirebird::TeFirebird() :
TeDatabase()
{
	firebird_ = 0;
	firebird_ref_ = 0;
	isConnected_ = false;
	dbmsName_ = "Firebird";
}


TeFirebird::~TeFirebird()
{
	if (isConnected_)
		close ();
}

IBPP::Database TeFirebird::getIBPPFirebird()
{
	return firebird_;
}

void
TeFirebird::close()
{
	if(firebird_ == NULL)
	{
		return;
	}
	if(firebird_->Connected())
	{
		if(transaction_->Started())
		{
			transaction_->Commit();
		}

		firebird_->Disconnect();
	}
}

bool TeFirebird::beginTransaction()
{
	transaction_->Start();
	return true;
}

bool TeFirebird::commitTransaction()
{
	if(transaction_->Started())
	{
		transaction_->Commit();
	}

	return true;
}

bool TeFirebird::rollbackTransaction()
{
	if(transaction_->Started())
	{
		transaction_->Rollback();
	}

	return true;
}

std::string TeFirebird::mapToFirebirdType(const TeAttribute& teRep) const
{
	std::string type;

	switch(teRep.rep_.type_)
	{
		case TeREAL:
			type = "DOUBLE PRECISION";
		break;

		case TeINT:
		case TeUNSIGNEDINT:
			type = "INTEGER ";							
		break;

		case TeCHARACTER:
			type = "CHAR(1) ";							
		break;

		case TeDATETIME:
			type = "TIMESTAMP ";  //time
		break;

		case TeBOOLEAN:
			type = "SMALLINT ";
		break;
	
		case TeBLOB:
			type = "BLOB SUB_TYPE 0";
		break;

		case TePOINTTYPE:
		case TePOINTSETTYPE:
			type += "x DOUBLE PRECISION DEFAULT 0.0";
			type += ", y DOUBLE PRECISION DEFAULT 0.0";
		break;

		case TeLINE2DTYPE:
		case TeLINESETTYPE:
			type += "num_coords INTEGER NOT NULL,";
			type += "lower_x DOUBLE PRECISION NOT NULL,";
			type += "lower_y DOUBLE PRECISION NOT NULL,";
			type += "upper_x DOUBLE PRECISION NOT NULL,";
			type += "upper_y DOUBLE PRECISION NOT NULL,";
			type += "ext_max DOUBLE PRECISION NOT NULL, ";
			type += "spatial_data BLOB NOT NULL";
		break;

		case TePOLYGONTYPE:
		case TePOLYGONSETTYPE:
			type += "num_coords INTEGER NOT NULL,";
			type += "num_holes INTEGER NOT NULL,";
			type += "parent_id INTEGER NOT NULL,";
			type += "lower_x DOUBLE PRECISION NOT NULL,";
			type += "lower_y DOUBLE PRECISION NOT NULL,";
			type += "upper_x DOUBLE PRECISION NOT NULL,";
			type += "upper_y DOUBLE PRECISION NOT NULL,";
			type += "ext_max DOUBLE PRECISION NOT NULL,";
			type += "spatial_data BLOB NOT NULL";
		break;

		case TeCELLTYPE:
		case TeCELLSETTYPE:
			type += "lower_x DOUBLE PRECISION NOT NULL,";
			type += "lower_y DOUBLE PRECISION NOT NULL,";
			type += "upper_x DOUBLE PRECISION NOT NULL,";
			type += "upper_y DOUBLE PRECISION NOT NULL,";
			type += "col_number	INTEGER NOT NULL,";
			type += "row_number	INTEGER NOT NULL ";
		break;

		case TeRASTERTYPE:
			type += "lower_x DOUBLE PRECISION NOT NULL, ";
			type += "lower_y DOUBLE PRECISION NOT NULL, ";
			type += "upper_x DOUBLE PRECISION NOT NULL, ";
			type += "upper_y DOUBLE PRECISION NOT NULL, ";
			type += "band_id INTEGER NOT NULL, ";
			type += "resolution_factor INTEGER, ";
			type += "subband INTEGER,";
			type += "spatial_data BLOB NOT NULL, ";
			type += "block_size INTEGER NOT NULL ";
		break;

		case TeNODETYPE:
		case TeNODESETTYPE:
			type += "x DOUBLE PRECISION DEFAULT 0.0";
			type += ", y DOUBLE PRECISION DEFAULT 0.0";
		break;

		case TeTEXTTYPE:
		case TeTEXTSETTYPE:
		case TeSTRING:
		default:
			if(teRep.rep_.numChar_ > 0)
			{
				type = "VARCHAR(" + Te2String(teRep.rep_.numChar_) + ") ";
			}
			else
			{
				type = "VARCHAR(4096)"; //de forma que possam ser usadas ate 8 colunas deste tamanho. o tamanho de uma linha = 64K
			}
		break;
	}

	return type;
}

bool
TeFirebird::getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes)
{
	
	std::vector<TeDatabaseIndex> indexes;

	string dbName = this->databaseName();

	string query1 = "select I.RDB$INDEX_NAME as INDEX_NAME, S.RDB$FIELD_NAME as COLUMN_NAME ";
		query1 += "from RDB$INDICES I, RDB$INDEX_SEGMENTS S ";
		query1 += "where I.RDB$RELATION_NAME = '"+tableName+"' and I.RDB$INDEX_NAME = S.RDB$INDEX_NAME ";
		query1 += "and I.RDB$INDEX_NAME not in ( select RDB$INDEX_NAME from RDB$RELATION_CONSTRAINTS ";
		query1 += "where RDB$INDEX_NAME IS NOT NULL)";

	TeDatabasePortal* portal = getPortal();

	std::vector<std::string> names;
	std::vector<std::string> cols;

	std::set<std::string> idxNames;

	std::multimap<std::string, std::string> idxAttrs;
	
	if(portal->query(query1))
	{
		while(portal->fetchRow())
		{
			std::string idxName = portal->getData("INDEX_NAME");
			std::string attrName = portal->getData("COLUMN_NAME");

			idxAttrs.insert(std::pair<std::string, std::string>(idxName, attrName));
			
			idxNames.insert(idxName);
		}		

	}else
	{
		delete portal;
		return false;
	}

	portal->freeResult();

	delete portal;
	portal =  NULL;

	std::set<std::string>::iterator it = idxNames.begin();
	while(it != idxNames.end())
	{
		std::string idxName = *it;

		TeDatabaseIndex idxx;
		idxx.setIndexName(idxName);
		idxx.setIsPrimaryKey(false);
		
		std::vector<string> attNames;
		std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> itAttrs = idxAttrs.equal_range(idxName);
		std::multimap<std::string, std::string>::iterator itA;
		for (itA=itAttrs.first; itA!=itAttrs.second; ++itA)
		{
			std::string attrName = itA->second;
			attNames.push_back(attrName);
		}

		idxx.setColumns(attNames);

		indexes.push_back(idxx);

		++it;
	}	

	vecIndexes = indexes;	

	return true;

}

bool TeFirebird::createAutoIncrement(const std::string& table, const std::string& column)
{
	/** Needs to create a generator for firebird to do auto increment. **/
	/** The name of the generator is composed by the name of table and the column name. **/
	std::string gen = "CREATE GENERATOR ",
				genName = ((table.size() <= 28) ? table : table.substr(0, 27))+ "_tr"; 
	gen += genName + "; \n";
	
	if(!execute(gen))
	{
		return false;
	}

	gen = "SET GENERATOR " + genName + " TO 0";

	if(!execute(gen))
	{
		return false;
	}

	gen = "CREATE TRIGGER " + genName + " FOR " + table + " ACTIVE BEFORE ";
	gen += "INSERT POSITION 0 AS \n" 
		+ std::string("BEGIN \n ")
		+ "if((NEW." + column + " is NULL) OR (NEW." + column + "<=0)) then "
		+ "NEW." + column + " = GEN_ID(" + genName + ", 1); \n END";

	if(!execute(gen))
	{
		return false;
	}

	return true;
}

int TeFirebird::getLastGeneratedAutoNumber(const std::string& table)
{
	int id = -1;

	std::string genName = ((table.size() <= 21) ? table : table.substr(0, 20))+ "_tr"; 
	genName = TeConvertToUpperCase(genName);
	std::string sql = "SELECT GEN_ID(" + genName + ",1) FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME='" + genName + "'";

	TeDatabasePortal* portal = getPortal();
	
	if(portal->query(sql) && portal->fetchRow())
	{
		id = portal->getInt(0);
	}

	delete portal;

	sql = "SET GENERATOR " + genName + " TO " + Te2String(id-1);
	execute(sql);

	return id;
}

bool TeFirebird::newDatabase(const string& database, const string& user, const string& password, const string& host, const int & port, bool terralibModel, const std::string& characterSet)
{
	try
	{
#ifdef WIN32
		DeleteFile((const TCHAR*) database.c_str());
#else
		DeleteFile(database.c_str());
#endif
		firebird_ = IBPP::DatabaseFactory(host, database, user, password, "", "", "PAGE_SIZE = 8192");
		firebird_ref_ = firebird_;
		firebird_->Create(3);		// 3 is the dialect of the database (could have been 1)

		IBPP::Service svc = IBPP::ServiceFactory(host, user, password);

		svc->Connect();
		svc->SetPageBuffers(database, 256);	// Instead of default 2048
		svc->SetSweepInterval(database, 5000);	// instead of 20000 by default
		svc->SetReadOnly(database, false);	// That's the default anyway
		svc->Disconnect();

		if(!connect(host, user, password, database, port))
		{
			firebird_->Drop();
			return false;
		}
		
		if(terralibModel)
		{
			//create conceptual model
			if(!createConceptualModel())
			{
				firebird_->Drop();
				return false;
			}
		}
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(IBPP::SQLException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}

	return true;
}


bool 
TeFirebird::connect (const string& host, const string& user, const string& password, const string& database, int port)
{
	try 
	{
		firebird_ = IBPP::DatabaseFactory(host, database, user, password);
		firebird_->Connect();
		firebird_ref_ = firebird_;

		isConnected_ = firebird_->Connected();

		if(!isConnected_)
		{
			return false;
		}

		// The following transaction configuration values are the defaults and
		// those parameters could have as well be omitted to simplify writing.
		transaction_ = IBPP::TransactionFactory(firebird_, IBPP::amWrite, IBPP::ilConcurrency, IBPP::lrWait);

		user_ = user;
		host_ = host;
		password_ = password;
		database_ = database;
		portNumber_ = port;

		createGeneratorTrigger(this);
	}
	catch(IBPP::SQLException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}

	return true;
}


bool 
TeFirebird::showDatabases (const string&/* host*/, const string&/* user*/, const string& /*password*/, vector<string>& /*dbNames*/, int /*port*/)
{
	/** There is function for this method in Firebird DBMS. It can't list its own databases. **/
	return true;
}


bool 
TeFirebird::execute (const string &q)
{
	try
	{
		bool started = true;

		if(!transaction_->Started())
		{
			transaction_->Start();
			started = false;
		}

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_);
		st->ExecuteImmediate(q);

		(started) ? transaction_->CommitRetain() : transaction_->Commit();
	}
	catch(IBPP::SQLException& e)
	{
		transaction_->Rollback();
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e)
	{
		transaction_->Rollback();
		errorMessage_ = e.ErrorMessage();
		return false;
	}

	return true;
}

bool 
TeFirebird::createTable(const string& table, TeAttributeList &attr)
{
	string createStr = " CREATE TABLE " + table + " (";
	string type;
	string pkeys,
		   autoNumColumn;

	for(unsigned int i = 0; i < attr.size(); i++)
	{
		if(i>0)
		{
			createStr += ",";
		}

		type = "";

		string name = attr[i].rep_.name_;
		
		bool complexType = (attr[i].rep_.type_ == TePOINTTYPE) ||
							(attr[i].rep_.type_ == TePOINTSETTYPE) ||
							(attr[i].rep_.type_ == TeLINE2DTYPE) ||
							(attr[i].rep_.type_ == TeLINESETTYPE) ||
							(attr[i].rep_.type_ == TePOLYGONTYPE) ||
							(attr[i].rep_.type_ == TePOLYGONSETTYPE) ||
							(attr[i].rep_.type_ == TeCELLTYPE) ||
							(attr[i].rep_.type_ == TeCELLSETTYPE) ||
							(attr[i].rep_.type_ == TeRASTERTYPE) ||
							(attr[i].rep_.type_ == TeNODETYPE) ||
							(attr[i].rep_.type_ == TeNODESETTYPE);

		createStr += (!complexType) ? name + " " + mapToFirebirdType(attr[i]) : mapToFirebirdType(attr[i]);

		if(!(attr[i].rep_.defaultValue_.empty()) && !complexType)
		{
			type = " DEFAULT '" + attr[i].rep_.defaultValue_ + "' ";
		}

		if(!(attr[i].rep_.null_) && !complexType)
		{
			type = " NOT NULL ";
		}

		if(attr[i].rep_.isAutoNumber_)
		{
			autoNumColumn = attr[i].rep_.name_;
		}

		createStr += " " + type;

		// check if column is part of primary key
		if((attr[i].rep_.isPrimaryKey_) && (attr[i].rep_.type_ != TeBLOB))
		{
			if(!pkeys.empty())
				pkeys += ",";

			pkeys += attr[i].rep_.name_;
		}
	}

	if(!pkeys.empty())
		createStr += ",PRIMARY KEY (" + pkeys + ") ";

	createStr += ")";

	if(!execute(createStr))
	{
		return false;
	}

	if(!autoNumColumn.empty())
	{
		if(!createAutoIncrement(table, autoNumColumn))
		{
			return  false;
		}
	}

	return true;
}

bool
TeFirebird::tableExist(const string& table)
{
	TeDatabasePortal* portal = getPortal();
	bool res = true;
	
	try
	{		
		std::string sql = "SELECT RDB$RELATION_NAME FROM RDB$RELATIONS WHERE RDB$RELATION_NAME='" 
			+ TeConvertToUpperCase(table) + "'";
		
		if(!portal->query(sql))
		{
			throw TeException(UNKNOWN_ERROR_TYPE, "Error on query statement.");
		}

		if(!portal->fetchRow())
		{
			res = false;
		}
	}
	catch(TeException& e)
	{
		errorMessage_ = e.message();
		res = false;
	}

	delete portal;

	return res;
}

bool 
TeFirebird::columnExist(const string& table, const string&  column ,TeAttribute& /*attr*/)
{
	TeDatabasePortal* portal = getPortal();
	bool res = true;

	try
	{
		std::string sql = "SELECT RDB$FIELD_NAME FROM RDB$RELATION_FIELDS";
		sql += " WHERE RDB$RELATION_NAME='" + TeConvertToUpperCase(table) + "'";
		sql += " AND RDB$FIELD_NAME='" + TeConvertToUpperCase(column) + "'";

		if(!portal->query(sql))
		{
			throw TeException(UNKNOWN_ERROR_TYPE, "Error on query statement.");
		}

		if(!portal->fetchRow())
		{
			res = false;
		}
	}
	catch(TeException& e)
	{
		errorMessage_ = e.message();
		res = false;
	}

	delete portal;

	return res;
}

bool TeFirebird::addColumn (const string& table, TeAttributeRep &rep)
{
	string q ="ALTER TABLE " + table +" ADD ";
	//string type;

	string field = TeGetExtension(rep.name_.c_str());
	if(field.empty())
		field = rep.name_;

	bool complexType = (rep.type_ == TePOINTTYPE) ||
							(rep.type_ == TePOINTSETTYPE) ||
							(rep.type_ == TeLINE2DTYPE) ||
							(rep.type_ == TeLINESETTYPE) ||
							(rep.type_ == TePOLYGONTYPE) ||
							(rep.type_ == TePOLYGONSETTYPE) ||
							(rep.type_ == TeCELLTYPE) ||
							(rep.type_ == TeCELLSETTYPE) ||
							(rep.type_ == TeRASTERTYPE) ||
							(rep.type_ == TeNODETYPE) ||
							(rep.type_ == TeNODESETTYPE);

	TeAttribute attr;
	attr.rep_ = rep;

	q += (!complexType) ? field + " " + mapToFirebirdType(attr) : mapToFirebirdType(attr);

	if (execute(q))
	{
		alterTableInfoInMemory(table);
		return true;
	}
	
	return false;
}

bool 
TeFirebird::alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName)
{
	if(!tableExist(tableName))
		return false;

	string tab;

	if(oldColName.empty())
	{
		tab = " ALTER TABLE " + tableName + " ALTER ";		
	}
	else
	{
		tab = " ALTER TABLE " + tableName + " ALTER ";
		tab += oldColName + " ";
	}

	bool complexType = (rep.type_ == TePOINTTYPE) ||
							(rep.type_ == TePOINTSETTYPE) ||
							(rep.type_ == TeLINE2DTYPE) ||
							(rep.type_ == TeLINESETTYPE) ||
							(rep.type_ == TePOLYGONTYPE) ||
							(rep.type_ == TePOLYGONSETTYPE) ||
							(rep.type_ == TeCELLTYPE) ||
							(rep.type_ == TeCELLSETTYPE) ||
							(rep.type_ == TeRASTERTYPE) ||
							(rep.type_ == TeNODETYPE) ||
							(rep.type_ == TeNODESETTYPE);

	if(!complexType)
	{
		tab += rep.name_ +" ";
	}
	tab += "TYPE ";

	TeAttribute attr;
	attr.rep_ = rep;

	tab += mapToFirebirdType(attr);

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error alter table " + tableName + " !";
		return false;
	}

	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + tableName + "'";
	if(portal->query(sql) && portal->fetchRow())
		tableId = portal->getData(0);

	delete portal;

	if(tableId.empty() == false)
	{
		if(oldColName.empty() == false) // column name changed
		{
			 // update relation
			sql = "UPDATE te_tables_relation SET related_attr = '" + rep.name_ + "'";
			sql += " WHERE related_table_id = " + tableId;
			sql += " AND related_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

			sql = "UPDATE te_tables_relation SET external_attr = '" + rep.name_ + "'";
			sql += " WHERE external_table_name = '" + tableName + "'";
			sql += " AND external_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

			 // update grouping
			sql = "UPDATE te_grouping SET grouping_attr = '" + tableName + "." + rep.name_ + "'";
			sql += " WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
		else // column type changed
		{
			// delete relation
			sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
			sql += " AND related_attr = '" + rep.name_ + "')";
			sql += " OR (external_table_name = '" + tableName + "'";
			sql += " AND external_attr = '" + rep.name_ + "')";
			if(execute(sql) == false)
				return false;

			// delete grouping
			TeDatabasePortal* portal = getPortal();
			sql = "SELECT theme_id FROM te_grouping WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(portal->query(sql) && portal->fetchRow())
			{
				string themeId = portal->getData(0);

				sql = "DELETE FROM te_legend WHERE theme_id = " + themeId + " AND group_id >= 0";
				if(execute(sql) == false)
				{
					delete portal;
					return false;
				}
			}
			delete portal;

			sql = "DELETE FROM te_grouping";
			sql += " WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
	}

	alterTableInfoInMemory(tableName);
	return true;
}

TeDatabasePortal*  
TeFirebird::getPortal ()
{
	return new TeFirebirdPortal (this);
}

string TeFirebird::getConcatFieldsExpression(const vector<string>& fNamesVec)
{
	std::string value;

	for(unsigned int i = 0; i < fNamesVec.size(); i++)
	{
		if(i > 0)
		{
			value += " || ";
		}
		value += fNamesVec[i];
	}

	return value;
}




TeFirebirdPortal::TeFirebirdPortal (TeFirebird* firebird) :
TeDatabasePortal(),
currRow_(0)
{
	firebird_ = firebird->getIBPPFirebird();
	numRows_ = 0;
	numFields_ = 0;
	db_ = firebird;
}


TeFirebirdPortal::TeFirebirdPortal ( const TeFirebirdPortal& p) : 
TeDatabasePortal(),
currRow_(p.currRow_)
{
	firebird_ = p.firebird_;
	numRows_ = 0;
	numFields_ = 0;
}


TeFirebirdPortal::~TeFirebirdPortal ()
{
	freeResult ();
}


string 
TeFirebird::escapeSequence(const string& from)
{
	int  fa = 0;
	std::string to = from;
    to.insert(0, " ");
    std::string::iterator it = to.begin();
    while(it != to.end())
    {
            int f = to.find("'", fa);
            if(f > fa)
            {
                    to.insert(f, "'");
                    fa = f + 2;
            }
            else
                    break;
    }
    to = to.substr(1, to.size() - 1);
    return to;
}

bool TeFirebirdPortal::query (const string &q, TeCursorLocation /*l*/, TeCursorType  /*t*/, TeCursorEditType e, TeCursorDataType /*dt*/)
{
	currRow_ = 0;

	try
	{
		attList_.clear ();

		IBPP::TAM tam = (e == TeREADONLY) ? IBPP::amRead : IBPP::amWrite;
		transaction_ = IBPP::TransactionFactory(firebird_, tam, IBPP::ilConcurrency, IBPP::lrWait);
		transaction_->Start();
		result_ = IBPP::StatementFactory(firebird_, transaction_);
		result_->Execute(q);

		numFields_ = result_->Columns();

		query_ = q;

		for(int i = 1; i <= numFields_; i++)
		{
			TeAttribute att;

			IBPP::SDT ftype = result_->ColumnType(i);

			switch(ftype)
			{
				case IBPP::sdInteger :
				case IBPP::sdLargeint:
					att.rep_.type_ = TeINT;
					att.rep_.numChar_ = result_->ColumnSize(i);
					att.rep_.name_ = result_->ColumnName(i);
				break;

				case IBPP::sdDouble :
				case IBPP::sdFloat :
					att.rep_.type_ = TeREAL;
					att.rep_.decimals_ = result_->ColumnScale(i);
					att.rep_.numChar_ = result_->ColumnSize(i);
				break;

				case IBPP::sdDate :
				case IBPP::sdTime:
				case IBPP::sdTimestamp :
					att.rep_.type_ = TeDATETIME;
					att.dateTimeFormat_ = "YYYYsMMsDDsHHsmmsSS24";
					att.dateChronon_ = TeSECOND;
				break;

				case IBPP::sdSmallint :
					att.rep_.type_ = TeBOOLEAN;
					break;

				case IBPP::sdBlob :
					if(result_->ColumnSubtype(i) == 0)
					{
						att.rep_.type_ = TeBLOB;
					}
					else
					{
						att.rep_.type_ = TeSTRING;
					}
				break;

				case IBPP::sdString :
					att.rep_.type_ = TeSTRING;
					att.rep_.numChar_ = result_->ColumnSize(i);
				break;

				default:
					att.rep_.type_ = TeUNKNOWN;
				break;
			}

			att.rep_.name_ = result_->ColumnAlias(i);
			attList_.push_back(att);
		}
	}
	catch(IBPP::SQLException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}

	return true;
}


bool 
TeFirebirdPortal::fetchRow ()
{
	bool res = true;

	try
	{
		res = result_->Fetch();
		if(res) 
		{
			numRows_++;
			currRow_++;
		}
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = e.ErrorMessage();
		res = false;
	}

	return res;
}


bool 
TeFirebirdPortal::fetchRow (int i)
{
	bool res = true;

	if(query_.empty())
	{
		return false;
	}

	if(i == currRow_+1)
	{
		res = fetchRow();
		currRow_++;
	}
	else
	{
		try
		{
			if(!transaction_->Started())
			{
				transaction_->Start();
			}

			int pos = query_.find("SELECT");

			if(pos < 0)
			{
				return false;
			}

			std::string sql = "SELECT SKIP " + Te2String(i) + query_.substr(pos + 6);
			
			result_ = IBPP::StatementFactory(firebird_, transaction_);

			result_->Execute(sql);
			numFields_ = result_->Columns();

			res = fetchRow();

			if(res) 
			{
				currRow_ = i;
			}
		}
		catch(IBPP::LogicException& e)
		{
			errorMessage_ = e.ErrorMessage();
			res = false;
		}
		catch(IBPP::SQLException& e)
		{
			errorMessage_ = e.ErrorMessage();
			res = false;
		}
	}

	return res;
}


void TeFirebirdPortal::freeResult ()
{
	numRows_ = 0;
	numFields_= 0;

	result_ = IBPP::StatementFactory(firebird_, transaction_);
}

bool TeFirebird::insertProjection (TeProjection *proj)
{
	TeProjectionParams par = proj->params();

	int id = getLastGeneratedAutoNumber("te_projection");

	string insert = "INSERT INTO te_projection (projection_id, name, long0, lat0,";
	insert += " offx, offy, stlat1, stlat2, unit, scale, hemis, datum )";
	insert += " VALUES ( ";
	insert += Te2String(proj->id());
	insert += ", '" + escapeSequence(par.name) + "'";
	insert += ", " + Te2String(par.lon0*TeCRD,10);
	insert += ", " + Te2String(par.lat0*TeCRD,10);
	insert += ", " + Te2String(par.offx,10);
	insert += ", " + Te2String(par.offy,10);
	insert += ", " + Te2String(par.stlat1*TeCRD,10);
	insert += ", " + Te2String(par.stlat2*TeCRD,10);
	insert += ", '" + escapeSequence(par.units) + "'";
	insert += ", " + Te2String(par.scale,10);
	insert += ", " + Te2String(par.hemisphere);
	insert += ", '" + escapeSequence(par.datum.name()) + "'";
	insert += ")";

	if (!execute(insert))
	{
		return false;
	}

	proj->id(id);

	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}

bool TeFirebird::updateProjection (TeProjection *proj)
{
	if (proj->id() <= 0)
		return false;
	string sql;
	sql = "UPDATE te_projection SET ";
	sql += "name='" + proj->name() + "',";
	sql += " long0=" + Te2String(proj->params().lon0*TeCRD)+ ",";
	sql += " lat0=" + Te2String(proj->params().lat0*TeCRD) + ",";
	sql += " offx=" +Te2String(proj->params().offx) + ",";
	sql += " offy=" +Te2String(proj->params().offy) + ",";
	sql += " stlat1="+ Te2String(proj->params().stlat1*TeCRD) + ",";
	sql += " stlat2=" +Te2String(proj->params().stlat2*TeCRD) + ",";
	sql += " unit='" + proj->params().units + "',";
	sql += " scale=" + Te2String(proj->params().scale) + ",";
	sql += " hemis=" + Te2String(proj->params().hemisphere) + ",";
	sql += " datum='" + proj->datum().name() + "'";
	sql += " WHERE projection_id = " + Te2String(proj->id());
	return execute(sql);
}

bool
TeFirebird::insertView (TeView *view)
{
	std::string ins;
	int id = getLastGeneratedAutoNumber("te_view");

	TeProjection* proj = view->projection();
	if ( !proj || !insertProjection(proj))
	{
		errorMessage_ = "N�o � poss�vel inserir vista sem proje��o";
		return false;
	}
	
	ins = "INSERT INTO te_view (view_id, projection_id, name, user_name, visibility)";
	ins += " VALUES (";
	ins += Te2String(view->id());
	ins += ", " + Te2String(proj->id());
	ins += ", '" + escapeSequence(view->name ()) + "'";
	ins += ", '" + escapeSequence(view->user ()) + "'";
	ins += ", " + Te2String((int)view->isVisible());
	ins += " )";
	
	if(!execute (ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_view!";   
		return false;
	}


	int size = view->size();

	for (int th=0; th<size; th++)
	{
		TeViewNode* node = view->get(th);
		if (node->type() == TeTHEME)
		{
			TeTheme *theme = (TeTheme*) node;
			insertTheme (theme);
		}
		else
		{
			TeViewTree* tree = (TeViewTree*)node;
			insertViewTree (tree);
		}
	}

	view->id(id);

	// Insert view in the view map
	viewMap()[view->id()] = view;
	return true;
}


bool 
TeFirebird::insertViewTree (TeViewTree *tree)
{
	int id = getLastGeneratedAutoNumber("te_theme");

	string save;
	save = "INSERT INTO te_theme (name,node_type,view_id,parent_id,priority) VALUES('";
	save += tree->name() +  "'," + Te2String((int)tree->type()) + "," + Te2String(tree->view()) + 
		"," + Te2String(tree->parentId()) + "," + Te2String(tree->priority()) + ")";
	
	if (!execute(save))
		return false;

	tree->id(id);

	return true;
}


bool 
TeFirebird::insertTheme (TeAbstractTheme *tem)
{
	string save;
	save = "INSERT INTO te_theme ";
	save += "(layer_id, view_id, name, parent_id, priority, node_type, min_scale, max_scale, ";
	save += "generate_attribute_where, generate_spatial_where, generate_temporal_where, ";
	save += "collection_table, visible_rep, enable_visibility, lower_x, lower_y, upper_x, upper_y, creation_time) ";
	save += " VALUES(";
	
	if(tem->type()==TeTHEME)
		save += Te2String(static_cast<TeTheme*>(tem)->layerId()) +", " ;
	else
		save += " NULL, ";

	save += Te2String(tem->view()) +", ";
	save += "'" + escapeSequence(tem->name()) + "', ";
	save += Te2String(tem->parentId()) +", ";
	save += Te2String(tem->priority()) +", ";
	save += Te2String(tem->type()) +", ";
	save += Te2String(tem->minScale()) +", ";
	save += Te2String(tem->maxScale()) +", ";
	save += "'" + escapeSequence(tem->attributeRest()) + "', ";
	save += "'" + escapeSequence(tem->spatialRest()) + "', ";
	save += "'" + escapeSequence(tem->temporalRest()) + "', ";

	if(tem->type()==TeTHEME)
		save += " '" + escapeSequence(static_cast<TeTheme*>(tem)->collectionTable()) + "', ";
	else
		save += " '', ";

	save += Te2String(tem->visibleRep()) +", ";
	save += Te2String(tem->visibility()) +", ";
	save += Te2String(tem->box().x1(), 10) +", ";
	save += Te2String(tem->box().y1(), 10) +", ";
	save += Te2String(tem->box().x2(), 10) +", ";
	save += Te2String(tem->box().y2(), 10) +",  ";

	TeTime creationTime = tem->getCreationTime();
	save += getSQLTime(creationTime);

	save += " ) ";

	int id = getLastGeneratedAutoNumber("te_theme");
	if (!execute(save))
		return false;

	tem->id(id);

	if((tem->type()==TeTHEME || tem->type()==TeEXTERNALTHEME)&& static_cast<TeTheme*>(tem)->collectionTable().empty())
	{
		string colTab = "te_collection_" + Te2String(tem->id());
		static_cast<TeTheme*>(tem)->collectionTable(colTab);
		save = "UPDATE te_theme SET collection_table='" + colTab + "' WHERE theme_id=" ;
		save += Te2String(tem->id());
		execute(save);
	}
	if(tem->parentId() == 0)
	{
		std::string sql = "UPDATE te_theme SET";
		sql += "  parent_id = " + Te2String(tem->id());
		sql += " WHERE theme_id = ";
		sql += Te2String(tem->id());

		tem->parentId(tem->id());

		if(!this->execute(sql))
			return false;
	}

	bool status;

	// insert grouping
	int numSlices = 0;
	if(tem->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (tem->id(), tem->grouping()))
			return false;
		numSlices = tem->grouping().groupNumSlices_;
	}
		
	tem->outOfCollectionLegend().group(-1); 
	tem->outOfCollectionLegend().theme(tem->id()); 
	status = insertLegend (&(tem->outOfCollectionLegend())); 
	if (!status)
		return status;

	tem->withoutDataConnectionLegend().group(-2); 
	tem->withoutDataConnectionLegend().theme(tem->id()); 
	status = insertLegend (&(tem->withoutDataConnectionLegend())); 
	if (!status)
		return status;

	tem->defaultLegend().group(-3); 
	tem->defaultLegend().theme(tem->id()); 
	status = insertLegend (&(tem->defaultLegend())); 
	if (!status)
		return status;

	tem->pointingLegend().group(-4); 
	tem->pointingLegend().theme(tem->id()); 
	status = insertLegend (&(tem->pointingLegend())); 
	if (!status)
		return status;

	tem->queryLegend().group(-5); 
	tem->queryLegend().theme(tem->id()); 
	status = insertLegend (&(tem->queryLegend())); 
	if (!status)
		return status;

	tem->queryAndPointingLegend().group(-6); 
	tem->queryAndPointingLegend().theme(tem->id()); 
	status = insertLegend (&(tem->queryAndPointingLegend())); 
	if (!status)
		return status;

	for (int i = 0; i < numSlices; i++)
	{
		tem->legend()[i].group(i);
		tem->legend()[i].theme(tem->id());
		status = insertLegend (&(tem->legend()[i]));
		if (!status)
			return status;
	}
	if (!status)
		return status;

	//insert metadata theme
	if(!tem->saveMetadata(this))
		return false;

	themeMap()[tem->id()] = tem;

	if(tem->type()==TeTHEME && !updateThemeTable(static_cast<TeTheme*>(tem)))
		return false;

	return true;
}

bool 
TeFirebird::insertThemeGroup (TeViewTree* tree)
{
	string save;
	save = "INSERT INTO te_theme ";
	save += "(name,parent_id,node_type,view_id,priority) ";
	save += " VALUES ('" + tree->name() + "',1,1, ";
	save += Te2String(tree->view()) +", ";
	save += Te2String(tree->priority()) +") ";

	int id = getLastGeneratedAutoNumber("te_theme");

	if (!execute(save))
		return false;

	tree->id(id);

	return true;
}

bool 
TeFirebird::generateLabelPositions (TeTheme *theme, const std::string& objectId)
{
	std::string collTable = theme->collectionTable();

	if((collTable.empty()) || (!tableExist(collTable)))
	{
		return false;
	}

	if (theme->layer()->hasGeometry(TeLINES))
	{	
		generateLablesForPolygonal(firebird_, theme, TeLINES, errorMessage_, objectId);
	}

	if (theme->layer()->hasGeometry(TePOINTS))
	{
		std::string geomTable = theme->layer()->tableName(TePOINTS),
					query = "SELECT object_id, x, y FROM " + geomTable;

		TeDatabasePortal* portal = theme->layer()->database()->getPortal();

		if(!portal->query(query))
		{
			throw TeException(UNKNOWN_ERROR_TYPE, "Error on query:\n" + portal->errorMessage());
		}

		if(!portal->fetchRow())
		{
			return true;
		}

		IBPP::Transaction tr = IBPP::TransactionFactory(firebird_);
		
		try
		{
			query = "UPDATE " + collTable + " SET label_x=?, label_y=? WHERE c_object_id=?";
			if(!objectId.empty())
			{
				query += " AND c_object_id=?";
			}

			tr->Start();
		
			IBPP::Statement st = IBPP::StatementFactory(firebird_, tr);

			st->Prepare(query);

			do
			{
				double x = portal->getDouble(1),
					y = portal->getDouble(2);

				std::string oId = portal->getData(0);

				st->Set(1, x);
				st->Set(2, y);
				st->Set(3, oId);
				if(!objectId.empty())
				{
					st->Set(4, objectId);
				}

				st->Execute();

			} while(portal->fetchRow());		

			tr->Commit();
			delete portal;
		}
		catch(IBPP::SQLException& e)
		{
			tr->Rollback();
			errorMessage_ = e.ErrorMessage();
			return false;
		}
		catch(IBPP::LogicException& e)
		{
			tr->Rollback();
			errorMessage_ = e.ErrorMessage();
			return false;
		}

		return true;
	}
	
	if (theme->layer()->hasGeometry(TePOLYGONS))
	{	
		return generateLablesForPolygonal(firebird_, theme, TePOLYGONS, errorMessage_, objectId);
	}
	
	if(theme->layer()->hasGeometry(TeCELLS))
	{
		return generateLablesForPolygonal(firebird_, theme, TeCELLS, errorMessage_, objectId);
	}

	return true;
}

bool TeFirebird::layerExist	(string layerName)
{
	if(layerName.size() > 31)
	{
		layerName.erase(30, layerName.size());		
	}

	return TeDatabase::layerExist(layerName);
}

bool
TeFirebird::insertLayer (TeLayer* layer)
{
	std::string		ins;
	char			value[1024];

	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "N�o � poss�vel inserir layer sem proje��o";
		return false;
	}

	if(layer->name().size() > 31)
	{
		std::string name = layer->name();
		name.erase(30, name.size());	
		layer->name(name);
	}

	ins = "INSERT INTO te_layer (projection_id, name ";
	ins += ", lower_x, lower_y, upper_x, upper_y, edition_time) ";
	ins += " VALUES ( ";
	ins += Te2String(proj->id());
	ins += ", '" + escapeSequence(layer->name()) + "'";

	sprintf(value,",%g,%g,%g,%g",layer->box().x1(),layer->box().y1(),layer->box().x2(),layer->box().y2());
	ins +=value;

	TeTime editionTime = layer->getEditionTime();
	ins += ", " + getSQLTime(editionTime) + ")";

	int id = getLastGeneratedAutoNumber("te_layer");

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_layer!";   
		return false;
	}

	layer->id(id);

	layerMap()[layer->id()] = layer;
	return true;
}

bool TeFirebird::updateLayer(TeLayer *layer)
{
	if (!layer)
		return false;

	if (layer->projection())
		updateProjection(layer->projection());

	string sql;
	sql = "UPDATE te_layer SET ";
	sql += "name = '" + layer->name() + "' ";
	if (layer->box().isValid())
	{
		sql += ", lower_x = " + Te2String(layer->box().x1()) + " ";
		sql += ", lower_y = " + Te2String(layer->box().y1()) + " ";
		sql += ", upper_x = " + Te2String(layer->box().x2()) + " ";
		sql += ", upper_y = " + Te2String(layer->box().y2()) + " ";
	}else
	{
		sql += ", lower_x = " + Te2String(layer->box().x1()) + " ";
		sql += ", lower_y = " + Te2String(layer->box().y1()) + " ";
		sql += ", upper_x = " + Te2String(layer->box().x2()) + " ";
		sql += ", upper_y = " + Te2String(layer->box().y2()) + " ";
	}
	if(layer->getEditionTime().isValid())
	{
		TeTime editionTime = layer->getEditionTime();
		sql += ", edition_time = " + this->getSQLTime(editionTime);
	}

	sql +=  " WHERE layer_id = " + Te2String(layer->id());

	return (this->execute (sql));
}

bool TeFirebird::insertDatum(const TeDatum& datum)
{
	std::string sql  = "INSERT INTO te_datum (name, radius, flattening, ";
	sql += "dx, dy, dz) VALUES('";
	sql += datum.name();
	sql += "', ";
	sql += Te2String(datum.radius(), 10);
	sql += ", ";
	sql += Te2String(datum.flattening(), 10);
	sql += ", ";
	sql += Te2String(datum.xShift(), 10);
	sql += ", ";
	sql += Te2String(datum.yShift(), 10);
	sql += ", ";
	sql += Te2String(datum.zShift(), 10);
	sql += ")";

	bool result = this->execute(sql);
	if(result)
	{
		// Add the new datum to DatumMap
		TeDatumMap& datumMap = TeDatumFactory::getDatumMap();
		datumMap[datum.name()] = datum;
	}
	return result;
}

bool TeFirebird::insertProject (TeProject* project)
{
	if (!project)
		return false;

	string save = "INSERT INTO te_project (project_id, name, description, current_view) VALUES(0,";
	save += "'" + project->name() + "', ";
	save += "'" + project->description() + "', ";
	save += Te2String(project->getCurrentViewId())+ ")";

	int id = getLastGeneratedAutoNumber("te_project");

	// if layer insertion fails remove projection
	if (!execute (save))
		return false;
	
	project->setId(id);

	projectMap()[project->id()] = project;
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);

	return true;
}

bool TeFirebird::updateRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;

	string sql;
	sql  = "UPDATE te_representation SET ";
	sql += " lower_x= " + Te2String(rep.box_.x1());
	sql += ", lower_y= " + Te2String(rep.box_.y1());
	sql += ", upper_x= " + Te2String(rep.box_.x2());
	sql += ", upper_y= " + Te2String(rep.box_.y2());
	sql += ", description= '" + rep.description_ + "'";
	sql += ", res_x= " + Te2String(rep.resX_);
	sql += ", res_y= " + Te2String(rep.resY_);
	sql += ", num_cols=" + Te2String(rep.nCols_);
	sql += ", num_rows=" + Te2String(rep.nLins_);

	if (rep.geomRep_ != TeTEXT)
		sql += ", geom_table='" + rep.tableName_ + "'";

	sql += " WHERE layer_id=" + Te2String(layerId);
	sql += " AND geom_type= " + Te2String(rep.geomRep_);

	if (rep.geomRep_ == TeTEXT)
		sql += " AND geom_table='" + rep.tableName_ + "'";

	return execute(sql);
}

bool TeFirebird::insertRasterGeometry(const string& tableName, TeRasterParams& par, const string& objectId)
{
	if (tableName.empty())
		return false;
	
	string objId; 
	if (objectId.empty())
		objId = "O1";
	else
		objId = objectId;

	//------ this check is made for compatibility reasons with old versions of TerraLib databases
	TeAttributeRep	attrRep;
	attrRep.name_ = "tiling_type";
	attrRep.type_ = TeINT;
		
	TeAttribute att;
	if(!columnExist(tableName, attrRep.name_,att))
	{
		addColumn (tableName, attrRep);
		string sql = "UPDATE " + tableName + " SET tiling_type = " + Te2String(static_cast<int>(TeRasterParams::TeExpansible));
		this->execute(sql);
	}

	//------

	// finds the name of the raster geometry table
	TeDatabasePortal* portal = getPortal();
	//if(!portal)
	//	return false;

	TeBox box = par.boundingBox();

	string ins = "INSERT INTO " + tableName + " (object_id, raster_table, lut_table, ";
	ins += "res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, ";
	ins += "lower_x, lower_y, upper_x, upper_y, tiling_type) ";
	ins += " VALUES ('" + objId + "', '" + par.fileName_+ "', '" +  par.lutName_ + "', ";
	ins += Te2String(par.resx_) + ", " + Te2String(par.resy_) + ", ";
	ins += Te2String(par.nBands()) + ", " + Te2String(par.ncols_) + ", " + Te2String(par.nlines_) + ", ";
	ins += Te2String(par.blockHeight_) + ", " + Te2String(par.blockWidth_) + ", ";
	ins += Te2String(box.x1_) +", " + Te2String(box.y1_) + ", ";
	ins += Te2String(box.x2_) +", " + Te2String(box.y2_) + ", ";
	ins	+= Te2String(par.tiling_type_) + ")";
	if (!this->execute(ins))
	{
		delete portal;
		return false;
	}

	// save the pallete associated to the raster
	// if it doesn?t exist yet
	if (par.photometric_[0] == TeRasterParams::TePallete && !par.lutName_.empty()) 
	{
		 if (!this->tableExist(par.lutName_))
		 {
			 if (this->createLUTTable(par.lutName_))
			{
				for (unsigned int i=0; i<par.lutb_.size(); i++)
				{
					string sql = "INSERT INTO " + par.lutName_ + " VALUES(";
					sql += Te2String(i) + ", ";
					sql += Te2String(par.lutr_[i]) + ", ";
					sql += Te2String(par.lutg_[i]) + ", ";
					sql += Te2String(par.lutb_[i]) + ", '";
					sql += par.lutClassName_[i] + "')";
					this->execute(sql);
				}
			 }
		 }
	}
	
	ins = "SELECT geom_id FROM " + tableName + " WHERE object_id='" + objId + "'";
	ins += " AND raster_table='" + par.fileName_+ "'";
	if(!portal->query(ins) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}
	int geomId = atoi(portal->getData(0));
	delete portal;
	string 	metadataTableName = tableName+"_metadata";
	insertRasterMetadata(metadataTableName, geomId,par);
	 return true;
}

bool TeFirebird::updateRasterRepresentation(int layerId, TeRasterParams& par, const string& objectId)
{
	TeDatabasePortal* portal = this->getPortal();
	if(!portal)
		return false;

	string sql = "SELECT repres_id, lower_x, lower_y, upper_x, upper_y, geom_table ";
	sql += " FROM te_representation WHERE layer_id= " + Te2String(layerId);
	sql += " AND geom_type= " + Te2String(TeRASTER);

	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}
	TeBox box (portal->getDouble(1),portal->getDouble(2),
		       portal->getDouble(3),portal->getDouble(4));
	int represId = atoi(portal->getData(0));
	string rasterrep = portal->getData(5);
	portal->freeResult();

	updateBox(box,par.boundingBox());
	sql = "UPDATE te_representation SET lower_x = " + Te2String(box.x1_);
	sql += ", lower_y = " + Te2String(box.y1_) + ", upper_x = " + Te2String(box.x2_);
	sql += ", upper_y = " + Te2String(box.y2_);

	if(par.date_.isValid())
	{	
		std::string sqlTime = getSQLTime(par.date_);
		if(!sqlTime.empty())
		{
			sql += ", initial_time = " + sqlTime;
			sql += ", final_time = " + sqlTime;
		}
	}
	sql += " WHERE repres_id=" + Te2String(represId);
	if(!execute(sql))
	{
		delete portal;
		return false;
	}	 

	string objId; 
	if (objectId.empty())
		objId = "O1";
	else
		objId = objectId;

	box = par.boundingBox();

	sql = "UPDATE " + rasterrep + " SET lut_table ='" + par.lutName_ + "'";
	sql += ", res_x= " + Te2String(par.resx_) + ", res_y=" + Te2String(par.resy_);
	sql += ", num_bands=" + Te2String(par.nBands()) + ", num_cols=" + Te2String(par.ncols_);
	sql += ", num_rows=" + Te2String(par.nlines_) + ", block_height=" +  Te2String(par.blockHeight_);
	sql += ", block_width= " + Te2String(par.blockWidth_) + ", lower_x = " + Te2String(box.x1_);
	sql += ", lower_y = " + Te2String(box.y1_) +  ", upper_x = " + Te2String(box.x2_);
	sql += ", upper_y = " + Te2String(box.y2_);
	sql += "  WHERE object_id='" + objId + "' AND raster_table='" + par.fileName_ + "'";
	if (!this->execute(sql))
	{
		delete portal;
		return false;
	}
	
	sql = "SELECT geom_id FROM " + rasterrep + " WHERE object_id='" + objId + "'";
	sql+= " AND raster_table='" + par.fileName_+ "'";
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	// save the pallete associated to the raster
	// if it doesn�t exist yet
	if (par.photometric_[0] == TeRasterParams::TePallete && !par.lutName_.empty()) 
	{

		 if (!this->tableExist(par.lutName_))
		 {
			 if (this->createLUTTable(par.lutName_))
			{
				for (unsigned int i=0; i<par.lutb_.size(); i++)
				{
					string sql = "INSERT INTO " + par.lutName_ + " VALUES(";
					sql += Te2String(i) + ", ";
					sql += Te2String(par.lutr_[i]) + ", ";
					sql += Te2String(par.lutg_[i]) + ", ";
					sql += Te2String(par.lutb_[i]) + ", '";
					sql += par.lutClassName_[i] + "')";
					if (!this->execute(sql) )	
					{
						delete portal;
						return false;						
					}
				}
			 }
		 }
	}

	int geomId = atoi(portal->getData(0));
	delete portal;
	string metadatatabel = rasterrep + "_metadata";
	return updateRasterMetadata(metadatatabel,geomId,par);
}

bool TeFirebird::insertLegend (TeLegendEntry *legend)
{
//	if(legend->id())
	string ins = "INSERT INTO te_legend (legend_id, theme_id, group_id, ";
	ins += " num_objs, lower_value, upper_value, label) VALUES (0";
//	ins += Te2String(legend->id());
	ins += ", " + Te2String(legend->theme());
	ins += ", " + Te2String(legend->group());
	ins += ", " + Te2String(legend->count());
	ins += ", '" + escapeSequence(legend->from()) + "'";
	ins += ", '" + escapeSequence(legend->to()) + "'";
	ins += ", '" + escapeSequence(legend->label()) + "'";
	ins += ")";
		
	int id = getLastGeneratedAutoNumber("te_legend");

	if (!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_legend!"; 
		return false;
	}
		
	legend->id(id);

	legendMap()[legend->id()] = legend;
	return insertVisual(legend);
}


bool
TeFirebird::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;
	
	string ins;

	try
	{
		char value[1024];

		ins = " INSERT INTO te_representation (layer_id, geom_type, geom_table, ";
		ins += " description, lower_x, lower_y, upper_x, upper_y, res_x, res_y, num_cols, ";
		ins += " num_rows) VALUES (";
		ins += Te2String(layerId);
		ins += ", " + Te2String(static_cast<int>(rep.geomRep_));
		ins += ", '" + escapeSequence(rep.tableName_) + "'";
		ins += ", '" + escapeSequence(rep.description_) + "'";

		sprintf(value,",%g,%g,%g,%g",rep.box_.x1(),rep.box_.y1(),rep.box_.x2(),rep.box_.y2());
		ins += value;
		ins += ", " + Te2String(rep.resX_,10);
		ins += ", " + Te2String(rep.resY_,10);
		ins += ", " + Te2String(rep.nCols_);
		ins += ", " + Te2String(rep.nLins_);
		ins += ")";

		int id = getLastGeneratedAutoNumber("te_representation");

		if(!execute(ins))
		{
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_representation!";   
			return false;
		}

		rep.id_ = id;
	}
	catch(IBPP::SQLException& e)
	{
		errorMessage_ = "Error inserting in the table te_representation.\n";
		errorMessage_ += e.ErrorMessage();

		return false;
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = "Error inserting in the table te_representation.\n";
		errorMessage_ += e.ErrorMessage();

		return false;
	}

	return true;
}

bool 
TeFirebird::insertRelationInfo(const int tableId, const string& tField,
						    const string& eTable, const string& eField, int& relId)
{
	//// check if relation already exists
	TeDatabasePortal* portal = getPortal();

	relId = -1;
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " related_table_id = " + Te2String(tableId);
	sel += " AND related_attr = '" + tField + "'";
	sel += " AND external_table_name = '" + eTable + "'";
	sel += " AND external_attr = '" + eField + "'";

	if (!portal->query(sel))
	{
		delete portal;
		return false;
	}

	if (portal->fetchRow())
	{
		relId = atoi(portal->getData(0));
		delete portal;
		return true;
	}

	delete portal;

	string sql = "INSERT INTO te_tables_relation(related_table_id, related_attr, external_table_name, external_attr) ";
	sql += " VALUES( ";
	sql += Te2String(tableId);
	sql += ",'" + tField + "'";
	sql += ",'" + eTable + "'";
	sql += ",'" + eField + "')";

	relId = getLastGeneratedAutoNumber("te_tables_relation");

	if (!execute(sql))
		return false;

	return true;
}

bool 
TeFirebird::insertTable(TeTable &table)
{
	string tableName = table.name();
	int size = table.size();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	TeAttributeList::iterator itEnd = att.end();
	TeTableRow row;

	
	int realMaxSize = 15; //limitacao do firebird. 18 digitos, sem contar o separador decimal
	if(firebird_->Dialect() == 3)
	{
		realMaxSize = 18;
	}

	
	if (!beginTransaction())
		return false;

	int i;
	unsigned int j;
	for ( i = 0; i < size; i++ )	
	{
		row = table[i];
		it = att.begin();
		string attrs;
		string values;
		j = 1;
		unsigned int jj = 0;
		while ( it != itEnd )
		{
			TeAttributeRep rep = (*it).rep_;
			if ((jj>=row.size()) || row[jj].empty() || (*it).rep_.isAutoNumber_)
			{
				++it;
				j++;
				jj++;
				continue;
			}
			
			if (!values.empty())
			{
				attrs += ", ";
				values += ", ";
			}			
			attrs += (*it).rep_.name_;			

			std::string strValue,
						temp_dt;
			TeTime t;

			int maxSizeToUse = realMaxSize;

			switch ((*it).rep_.type_)
			{
				case TeSTRING:
					values += "'"+ escapeSequence( row[jj] ) +"'";
				break;

				case TeREAL:
					strValue = row[jj];
					replace(strValue.begin(), strValue.end(), ',', '.');
					if((int)strValue.size() > realMaxSize)
					{
						basic_string <char>::size_type index;
						if ( (index=strValue.find(".")) != string::npos )
						{
							maxSizeToUse += 1;// caso tenha separador decinal, podemos usar mais um caractere
						}
						strValue = strValue.substr(0, maxSizeToUse);
					}
					else
					{
						strValue = row[jj];
					}

					values += strValue;
				break;

				case TeINT:
					values += row[jj];
				break;

				case TeDATETIME:
					temp_dt = string(row[jj].c_str());
					t = TeTime(temp_dt, (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_, (*it).timeSeparator_, (*it).indicatorPM_);
					values += getSQLTime(t);
				break;

				case TeCHARACTER:
					values += "'" + escapeSequence(row[jj]) + "'";
				break;

				case TeBOOLEAN:
				{
					std::string value = "0";
					if(row[jj] == "1" || TeConvertToLowerCase(row[jj]) == "t" || TeConvertToLowerCase(row[jj]) == "true")
					{
						value = "1";
					}
					values += value;
				}
				break;

				case TeBLOB:
					values += "'" + escapeSequence(row[jj]) + "'";
				break;

				default:
					values += "'"+ escapeSequence(row[jj]) +"'";
			}
			++it;
			j++;
			jj++;
		}

		if (values.empty()) 
			continue;

		string q = "INSERT INTO "+tableName + " ("+ attrs+") " + " VALUES ("+values+") ";
		if (!execute(q)) 
			continue;
	}

	if (!commitTransaction())
	{
		rollbackTransaction();
		return false;
	}

	return true;
}

bool
TeFirebird::updateTable(TeTable &table)
{
	TeAttributeList& att = table.attributeList();
	unsigned int i;
	string uniqueVal;
	bool isUniqueValString = false;
	
	if (!beginTransaction())
		return false;

	string uniqueName = table.uniqueName(); // primary key explicitly defined or 
	if (table.uniqueName().empty())			// check in the attribute list
	{								
		for (i=0; i<att.size(); ++i)
			if (att[i].rep_.isPrimaryKey_)
			{
				uniqueName = att[i].rep_.name_;
				table.setUniqueName(uniqueName);
				break;
			}
	}

	TeAttributeList::iterator it;
	TeTableRow row;
	unsigned int j;
	bool useComma = false;
	for (i = 0; i < table.size(); i++  )
	{
		row = table[i];
		it = att.begin();
		string q;
		j = 1;
		int jj = 0;
		while ( it != att.end() )
		{
			if (uniqueName != (*it).rep_.name_)
			{
				if ((*it).rep_.isAutoNumber_)
				{
					++it;
					j++;
					jj++;
					continue;
				}			
				if(useComma == true)
				{
					q += ", ";
				}
				else
				{
					useComma = true;
				}
				q += (*it).rep_.name_ + " = ";
				  				
				if(row[jj].empty())
				{
					q += " null";

					++it;
					j++;
					jj++;
					continue;
				}

  				switch ((*it).rep_.type_)
  				{
  					case TeSTRING:
						q += "'"+escapeSequence(row[jj])+"'";
  					break;
  					case TeREAL:
					{
						std::string value = row[jj];
						replace(value.begin(), value.end(), ',', '.');
						q += value;
					}
  					break;
  					case TeINT:
						q += row[jj];
  					break;
					case TeDATETIME:
					{
						const string temp_dt = string(row[jj].c_str());
						TeTime t(temp_dt, (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_, (*it).timeSeparator_, (*it).indicatorPM_);
						q += this->getSQLTime(t);
					}
  					break;
					case TeCHARACTER:
						q += "'" + escapeSequence(row[jj]) + "'";
  					break;
					case TeBLOB:
						q += "'" + escapeSequence(row[jj]) + "'";
					break;
  					default:
						q += "'"+escapeSequence(row[jj])+"'";
  					break;
  				}
			}
			else
			{
				uniqueVal = row[jj];
				isUniqueValString = ((*it).rep_.type_ == TeSTRING);
			}
			++it;
			j++;
			jj++;
		}
		if (q.empty())
			continue;
		
		if (!uniqueName.empty() && !uniqueVal.empty())  
		{
			if(isUniqueValString)
				q += " WHERE " + uniqueName + " = '" + uniqueVal +"'";
			else
				q += " WHERE " + uniqueName + " = " + uniqueVal;
		}
		string sql = "UPDATE "+ table.name() + " SET " + q;
		if (!execute(sql))
		{
			rollbackTransaction();
			return false;
		}
	}
	if (!commitTransaction())
	{
		rollbackTransaction();
		return false;
	}
	return true;
}

bool 
TeFirebird::insertTableInfo (int layerId, TeTable &table, const string& user)
{
	string sql;

	sql = "INSERT INTO te_layer_table (unique_id, attr_link,attr_initial_time, attr_final_time, attr_time_unit,attr_table_type, ";
	sql += " user_name, attr_table ";
	if (layerId > 0)
		sql += ",layer_id";
	sql += ") VALUES( ";
	sql += "'" + table.uniqueName() + "', ";
	sql += "'" + table.linkName() + "', ";
	sql += "'" + table.attInitialTime() + "', ";
	sql += "'" + table.attFinalTime() + "', ";
	sql += "" + Te2String(table.attTimeUnit()) + ", ";
	sql += ""+ Te2String(table.tableType()) + ", ";
	sql += "'" + user + "','" + table.name() +"'";
	if (layerId > 0)
		sql += ", " + Te2String(layerId) ;
	sql += ")";

	int id = getLastGeneratedAutoNumber("te_layer_table");

	if (!execute(sql))
		return false;

	table.setId(id);

	return true;
}

bool
TeFirebird::updateTheme (TeAbstractTheme *theme)
{
	string sql;

	if (theme->id() <= 0 )  // theme doesn?t exist in the database yet
	{
		return this->insertTheme(theme);
	}
	
	// update theme metadata
	sql = "UPDATE te_theme SET ";
	
	if(theme->type()==TeTHEME)
	{
		sql += " layer_id=" + Te2String (static_cast<TeTheme*>(theme)->layerId());
		sql += ", ";
	}
	
	sql += "  view_id=" + Te2String (theme->view());
	sql += ", name='" + escapeSequence(theme->name())+"'";
	sql += ", parent_id=" + Te2String (theme->parentId());
	sql += ", priority=" + Te2String (theme->priority());
	sql += ", node_type=" + Te2String (theme->type());
	sql += ", min_scale=" + Te2String (theme->minScale());
	sql += ", max_scale=" + Te2String (theme->maxScale());
	sql += ", generate_attribute_where='" + escapeSequence(theme->attributeRest())+"'";
	sql += ", generate_spatial_where='" + escapeSequence(theme->spatialRest())+"'";
	sql += ", generate_temporal_where='" + escapeSequence(theme->temporalRest())+"'";

	if(theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME )
		sql += ", collection_table='" + static_cast<TeTheme*>(theme)->collectionTable() + "'";

	sql += ", visible_rep= " + Te2String(theme->visibleRep ());
	sql += ", enable_visibility= " + Te2String(theme->visibility()); 
	sql += ", lower_x = " + Te2String(theme->box().x1()); 
	sql += ", lower_y = " + Te2String(theme->box().y1()); 
	sql += ", upper_x = " + Te2String(theme->box().x2()); 
	sql += ", upper_y = " + Te2String(theme->box().y2()); 
	
	if(theme->getCreationTime().isValid())
	{
		TeTime creationTime = theme->getCreationTime();
		sql += ", creation_time = " + this->getSQLTime(creationTime);
	}

	sql += " WHERE theme_id=" + Te2String (theme->id());

	if (!this->execute (sql))
		return false;
         
	//delete grouping 
	sql = "DELETE FROM te_grouping WHERE theme_id= "+ Te2String(theme->id());
	this->execute (sql);
		
	if(theme->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping(theme->id(), theme->grouping()))
			return false;
	}
	
	// update each of its legends
	bool status = true;
	
	if(theme->legend().size() == 0)
	{
		if(!deleteLegend(theme->id()))
			return false;
	}
	else
	{
		status = updateLegend(theme->legend());
		if (!status)
			return status;
	}

	status = updateLegend(&(theme->withoutDataConnectionLegend()));
	if (!status)
		return status;

	status = updateLegend(&(theme->outOfCollectionLegend()));
	if (!status)
		return status;

	status = updateLegend(&(theme->defaultLegend()));
	if (!status)
		return status;

	status = updateLegend(&(theme->pointingLegend()));
	if (!status)
		return status;

	status = updateLegend(&(theme->queryLegend()));
	if (!status)
		return status;

	status = updateLegend(&(theme->queryAndPointingLegend()));
	if (!status)
		return status;
	
	//insert metadata theme
	if(!theme->saveMetadata(this))
		return false;

	// theme tables
	if(theme->type()==TeTHEME && !updateThemeTable(static_cast<TeTheme*>(theme)))
		return false;

	return true;
}

bool 
TeFirebird::insertThemeTable (int themeId, int tableId, int relationId, int tableOrder)
{
	string relId;
	if (relationId > 0)
		relId = Te2String(relationId);
	else
		relId = "NULL";

	string sql = "INSERT INTO te_theme_table VALUES (0,";
	sql += Te2String(themeId) + "," + Te2String(tableId) + ",";
	sql += relId + "," + Te2String(tableOrder) + ")";

	if (!execute(sql))
		return false;

	return true;
}

bool 
TeFirebird::insertPolygon (const string& table, TePolygon &poly )
{
	try
	{
		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);

				// insert data
		std::string ins = "INSERT INTO " + table + " ( ";
		ins += "object_id, num_coords, num_holes, ";
		ins += " parent_id, lower_x, lower_y, upper_x, upper_y, "; 
		ins += " ext_max, spatial_data) VALUES (?,?,?,?,?,?,?,?,?,?)";

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_, ins);
		st->Prepare(ins);

		int polyGeomId = -1; 
		
		for(unsigned int i=0; i<poly.size(); i++)
		{
			string obId = escapeSequence(poly.objectId());
			TeBox b = poly.box();

			saveLineInBlob(blob, poly[i]);
			int geomId = getLastGeneratedAutoNumber(table);
			poly[i].geomId(geomId);

			if(i == 0)
			{
				polyGeomId = geomId;
				poly.geomId(polyGeomId);
			}

			st->Set(1,obId);
			st->Set(2, (int)poly[i].size());
			st->Set(3, (i != 0) ? 0 : (int)poly.size()-1);
			st->Set(4, polyGeomId);
			st->Set(5, b.lowerLeft().x());
			st->Set(6, b.lowerLeft().y());
			st->Set(7, b.upperRight().x());
			st->Set(8, b.upperRight().y());
			st->Set(9, MAX(b.width(), b.height()));
			st->Set(10, blob);

			st->Execute();
		}
	}
	catch(IBPP::SQLException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}

	return true;
}

bool 
TeFirebird::updatePolygon (const string& table, TePolygon &poly )
{
	try
	{
		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);

				// insert data
		std::string ins = "UPDATE " + table + " SET ";
		ins += "object_id=?, num_coords=?, num_holes=?, ";
		ins += " parent_id=?, lower_x=?, lower_y=?, upper_x=?, upper_y=?, "; 
		ins += " ext_max=?, spatial_data=? WHERE geom_id = " + Te2String(poly.geomId());

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_, ins);
		st->Prepare(ins);

		for(unsigned int i=0; i<poly.size(); i++)
		{
			string obId = escapeSequence(poly.objectId());
			TeBox b = poly.box();

			saveLineInBlob(blob, poly[i]);

			st->Set(1, obId);
			st->Set(2, (int)poly[i].size());
			st->Set(3, (i != 0) ? 0 : (int)poly.size()-1);
			st->Set(4, poly[0].geomId());
			st->Set(5, b.lowerLeft().x());
			st->Set(6, b.lowerLeft().y());
			st->Set(7, b.upperRight().x());
			st->Set(8, b.upperRight().y());
			st->Set(9, MAX(b.width(), b.height()));
			st->Set(10, blob);

			st->Execute();
		}
	}
	catch(IBPP::SQLException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}

	return true;
}

bool 
TeFirebirdPortal::fetchGeometry (TePolygon& poly)
{
	int geomId = getInt("geom_id");
	string oId = getData("object_id");

	TeBox b(getDouble("lower_x"), getDouble("lower_y"), getDouble("upper_x"), getDouble("upper_x"));

	TeLinearRing l;

	loadFromBlob(result_, l);
	l.geomId(geomId);

	poly.geomId(geomId);
	poly.objectId (oId);
	poly.add (l);

	int parentId = poly.geomId();

	while (fetchRow())
	{
		if (getInt("parent_id") == parentId)
		{
			TeLinearRing hole;
			loadFromBlob(result_, hole);

			hole.geomId(getInt("geom_id"));
			hole.objectId (getData("object_id"));

			poly.add (hole);
		}
		else
			return true;
	}

	return false;
}

bool 
TeFirebirdPortal::fetchGeometry (TePolygon& poly, const unsigned int& initIndex)
{
	TeLinearRing l;

	loadFromBlob(result_, l);

	poly.geomId(getInt(initIndex));
	poly.objectId (getData(initIndex+1));
	poly.add (l);

	int parentId = poly.geomId();

	while (fetchRow())
	{
		if (getInt(initIndex+4) == parentId)
		{
			l.clear();
			loadFromBlob(result_, l);
			l.geomId(getInt(initIndex));
			l.objectId (getData(initIndex+1));

			poly.add (l);
		}
		else
			return true;
	}

	return false;
}


TeLinearRing 
TeFirebirdPortal::getLinearRing (int &/*ni*/)
{
	int index = atoi(getData("geom_id"));
	string object_id = getData("object_id");
	//int np = atoi (getData("num_coords"));
	TeBox b (getDouble("lower_x"),getDouble("lower_y"),getDouble("upper_x"),getDouble("upper_y"));
	TeLinearRing line;

	loadFromBlob(result_, line);

	line.objectId(object_id);
	line.geomId (index);
	line.setBox (b);
	
	return line;
}

bool 
TeFirebird::insertLine (const string& table, TeLine2D &l)
{
	try
	{
		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);

				// insert data
		std::string ins = "INSERT INTO " + table + " ( ";
		ins += " object_id, num_coords, ";
		ins += " lower_x, lower_y, upper_x, upper_y, "; 
		ins += " ext_max, spatial_data) VALUES (?,?,?,?,?,?,?,?)";

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_, ins);
		st->Prepare(ins);

		std::string obId = escapeSequence(l.objectId());
		TeBox b = l.box();

		int index = getLastGeneratedAutoNumber(table);
	
		TeLinearRing ring;
		ring.copyElements(l);

		saveLineInBlob(blob, ring);

		st->Set(1, obId);
		st->Set(2, (int)ring.size());
		st->Set(3, b.lowerLeft().x());
		st->Set(4, b.lowerLeft().y());
		st->Set(5, b.upperRight().x());
		st->Set(6, b.upperRight().y());
		st->Set(7, MAX(b.width(), b.height()));
		st->Set(8, blob);

		st->Execute();

		l.geomId(index);
	}
	catch(IBPP::SQLException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}

	return true;
}


bool 
TeFirebird::updateLine (const string& table, TeLine2D &l)
{
	try
	{
		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);

				// insert data
		std::string ins = "UPDATE " + table + " SET (";
		ins += "object_id=?, num_coords=?, ";
		ins += "lower_x=?, lower_y=?, upper_x=?, upper_y=?, "; 
		ins += "ext_max=?, spatial_data=?) WHERE geom_id="+Te2String(l.geomId());

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_, ins);
		st->Prepare(ins);

		std::string obId = escapeSequence(l.objectId());
		TeBox b = l.box();

		TeLinearRing ring = l;
		saveLineInBlob(blob, ring);

		st->Set(1, obId);
		st->Set(2, (int)l.size());
		st->Set(3, b.lowerLeft().x());
		st->Set(4, b.lowerLeft().y());
		st->Set(5, b.upperRight().x());
		st->Set(6, b.upperRight().y());
		st->Set(7, MAX(b.width(), b.height()));
		st->Set(8, blob);

		st->Execute();
	}
	catch(IBPP::SQLException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}

	return true;
}

bool
TeFirebirdPortal::fetchGeometry(TeLine2D& line)
{
	int ni;
	line = getLinearRing(ni);

	return(fetchRow());
}

bool
TeFirebirdPortal::fetchGeometry(TeLine2D& line, const unsigned int& initIndex)
{
	//int index = getInt(initIndex);
	string object_id = getData (initIndex+1);
	//int np = getInt(initIndex+2);

	TeLinearRing l;
	loadFromBlob(result_, l);

	l.geomId(getInt(initIndex));
	l.objectId(getData(initIndex+1));

	line.copyElements(l);

	return (fetchRow());
}


bool 
TeFirebird::insertPoint(const string& table, TePoint &p)
{
	string q = "INSERT INTO " + table + " VALUES(0,";
	q += "'" + escapeSequence(p.objectId()) + "',";
	q += Te2String(p.location().x_) + ",";
	q += Te2String(p.location().y_) + ")";
	
	int geomId = getLastGeneratedAutoNumber(table);

	if (!execute(string(q)))
	{
		return false;
	}

	p.geomId(geomId);

	return true;
}


bool 
TeFirebirdPortal::fetchGeometry(TePoint& p)
{
	TeCoord2D c(getDouble("x"), getDouble("y"));
	p.geomId(getInt("geom_id"));
	p.objectId(getData("object_id"));
	p.add(c);

	return (fetchRow());
}

bool 
TeFirebirdPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	TeCoord2D c(getDouble(initIndex+2), getDouble(initIndex+3));
	p.geomId(getInt(initIndex));
	p.objectId(getData(initIndex+1));
	p.add(c);

	return (fetchRow());
}


bool 
TeFirebird::insertText(const string& table, TeText &t)
{
	char q[256];

	int geomId = getLastGeneratedAutoNumber(table);

	sprintf (q,"INSERT INTO %s values(0,'%s',%.12f,%.12f,'%s',%.12f, %.12f, %.12f, %.12f)",
		table.c_str(), escapeSequence(t.objectId()).c_str(),
		t.location().x_, t.location().y_,
		t.textValue().c_str(), t.angle(), t.height(),t.alignmentVert(),t.alignmentHoriz());
	
	if (!execute(string(q)))
		return false;
	
	t.geomId(geomId);

	return true;
}


bool 
TeFirebird::insertArc(const string& table, TeArc &arc)
{
	char q[256];
	int geomId = getLastGeneratedAutoNumber(table);

	sprintf (q,"INSERT INTO %s values(0,'%s',%d,%d)",
		table.c_str(),escapeSequence(arc.objectId()).c_str(),arc.fromNode().geomId(),arc.toNode().geomId());
	
	if (!this->execute(string(q)))
	{
		return false;
	}

	arc.geomId(geomId);
	return true;
}


bool
TeFirebird::insertNode(const string& table, TeNode &node)
{
	char q[256];
	int geomId = getLastGeneratedAutoNumber(table);

	sprintf (q,"INSERT INTO %s values(0,'%s',%.12f,%.12f)",table.c_str(),node.objectId().c_str(),node.location().x(),node.location().y());
	if (!execute(string(q)))
	{
		return false;
	}

	node.geomId(geomId);
	return true;
}

bool 
TeFirebird::insertBlob (const string& tableName, const string& columnBlob , const string& whereClause, unsigned char* data, int size)
{
	if (whereClause.empty())
		return false;

	TeDatabasePortal* portal = getPortal();
	
	string q = "SELECT * FROM "+ tableName +" WHERE "+ whereClause;

	if((!portal->query(q)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}

	delete portal;

	std::string sql  = "UPDATE ";
	sql += tableName;
	sql += " SET ";
	sql += columnBlob;
	sql += " = ?";
	sql += " WHERE ";
	sql += whereClause;

	try
	{
		bool started = true;
		
		if(!transaction_->Started())
		{
			transaction_->Start();
			started = false;
		}

		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);

				// insert data

		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_, sql);

		convertChar2Blob((char*)data, size, blob);

		blob->Close();
		
		st->Prepare(sql);
		st->Set(1, blob);
		st->Execute();

		(started) ? transaction_->CommitRetain() : transaction_->Commit();
	}
	catch(IBPP::SQLException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e1)
	{
		errorMessage_ = e1.ErrorMessage();
		return false;
	}

	return true;
}

bool
TeFirebird::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, 
						   unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{
	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeDatabasePortal* portal = getPortal();
	bool update = false;

	std::string sql = "SELECT block_id FROM " + table + " WHERE block_id='" + blockId + "'";
	if(!portal->query(sql))
	{
		delete portal;
		errorMessage_ = portal->errorMessage();
		
		return false;
	}

	if(portal->fetchRow())
	{
		update = true;
	}

	delete portal;

	try
	{
		IBPP::Statement st = IBPP::StatementFactory(firebird_, transaction_);

		if(!update)
		{
			sql = "INSERT INTO " + table + " (lower_x, lower_y, upper_x, upper_y, band_id, resolution_factor, subband, block_size, spatial_data, block_id) ";
			sql += " VALUES (?,?,?,?,?,?,?,?,?,?)"; 
		}
		else
		{
			sql = "UPDATE " + table + " SET lower_x=?, lower_y=?, upper_x=?, upper_y=?, band_id=?,";
			sql += "resolution_factor=?, subband=?, block_size=?, spatial_data=? WHERE block_id=? ";
		}

		bool trStarted = true;

		if(!transaction_->Started())
		{
			transaction_->Start();
			trStarted = false;
		}

		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);
		convertChar2Blob(reinterpret_cast<char*>(buf), size, blob);
		blob->Close();

		st->Prepare(sql);

		st->Set(1, ll.x());
		st->Set(2, ll.y());
		st->Set(3, ur.x());
		st->Set(4, ur.y());
		st->Set(5, band);
		st->Set(6, (int)res);
		st->Set(7, (int)subband);
		st->Set(8, (int)size);
		st->Set(9, blob);
		st->Set(10, blockId);

		st->Execute();
		(trStarted)? transaction_->CommitRetain() : transaction_->Commit();
	}
	catch(IBPP::SQLException& e)
	{
		transaction_->Rollback();
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(IBPP::LogicException& e)
	{
		transaction_->Rollback();
		errorMessage_ = e.ErrorMessage();
		return false;
	}

	return true;
}

bool 
TeFirebird::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	if(!TeDatabase::getAttributeList(tableName, attList))
	{
		return false;
	}

	std::string sql = "SELECT trim(ixs.RDB$FIELD_NAME) from rdb$RELATION_CONSTRAINTS con";
	sql += " JOIN RDB$INDEX_SEGMENTS ixs On ixs.RDB$INDEX_NAME = con.RDB$INDEX_NAME";
	sql += " WHERE upper(con.RDB$RELATION_NAME) = upper('" + tableName + "') and con.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY'";

	TeDatabasePortal* portal = getPortal();

	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
	{
		std::string columnName = portal->getData(0);

		for(unsigned int i = 0; i < attList.size(); ++i)
		{
			if(columnName == attList[i].rep_.name_)
			{
				attList[i].rep_.isPrimaryKey_ = true;
			}
		}
	}

	delete portal;

	return true;
}

bool
TeFirebirdPortal::fetchGeometry(TeNode& n)
{
	TeCoord2D c(getDouble("x"), getDouble("y"));
	n.geomId(getInt("geom_id"));
	n.objectId(getData("object_id"));
	n.add(c);
	return (fetchRow());
}

bool
TeFirebirdPortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	TeCoord2D c(getDouble(initIndex+2), getDouble(initIndex+3));
	n.geomId( getInt(initIndex));
	n.objectId(getData(initIndex+1));
	n.add(c);
	return (fetchRow());
}

bool
TeFirebird::insertCell(const string& table, TeCell &c)
{
	TeBox	b=c.box();
	char	q[256];
	int geomId = getLastGeneratedAutoNumber(table);
	
	sprintf (q,"INSERT INTO %s values(0,'%s',%.15f,%.15f,%.15f,%.15f,%d,%d)",
		table.c_str(),c.objectId().c_str(),
		b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),
		c.column(),c.line());

	if (!execute (q))
	{
		return false;
	}

	c.geomId(geomId);
	return true;
}


TeTime
TeFirebirdPortal::getDate (int i)
{ 
	string s = getData(i);
	TeTime t(s, TeSECOND, "YYYYsMMsDDsHHsmmsSS","."); 
	return t; 
}

TeTime
TeFirebirdPortal::getDate (const string& s)
{ 
	string sv = getData(s);
	TeTime t(sv, TeSECOND, "YYYYsMMsDDsHHsmmsSS","."); 
	return t; 
}

string 
TeFirebirdPortal::getDateAsString(const string& s)
{
	TeTime t = this->getDate(s);
	if (t.isValid())
	{		
		string tval = "\'"+t.getDateTime("YYYYsMMsDDsHHsmmsSS", ".")+"\'";
		return tval;
	}
	else
		return "";
}

string 

TeFirebirdPortal::getDateAsString(int i)
{
	TeTime t = this->getDate(i);
	if (t.isValid())
	{		
		string tval = "\'"+t.getDateTime("YYYYsMMsDDsHHsmmsSS", ".")+"\'";
		return tval;
	}
	else
		return "";
}

double 
TeFirebirdPortal::getDouble (const string& s)
{
	return atof(getData(s));
}


double 
TeFirebirdPortal::getDouble (int i)
{
	return atof(getData(i));
}

int 
TeFirebirdPortal::getInt (const string& s)
{
	return atoi(getData(s));
}


int 
TeFirebirdPortal::getInt (int i)
{
	return atoi(getData(i));
}


char* 
TeFirebirdPortal::getData (int i)
{
	double fValue;
	int iValue;
	IBPP::Date dValue;
	IBPP::Time tValue;
	IBPP::Timestamp tsValue;
	IBPP::Blob bValue;
	std::string sValue;

	data_.clear();

	try
	{
		i++;
		int numcols = result_->Columns();

		if((i>0) && (i <= numcols))
		{
			IBPP::SDT fbType = result_->ColumnType(i);

			switch(fbType)
			{
				case IBPP::sdFloat :
				case IBPP::sdDouble :
					if(!result_->Get(i, fValue))
					{
						char result[100];
						sprintf( result, "%f", fValue );
						data_ = result;
					}
				break;

				case IBPP::sdInteger :
				case IBPP::sdLargeint :
				case IBPP::sdSmallint :
					if(!result_->Get(i, &iValue))
					{
						data_ = Te2String(iValue);
					}
				break;

				case IBPP::sdString :
					if(!result_->Get(i, sValue))
					{
						data_ = sValue;
					}
				break;

				case IBPP::sdDate:
					if(!result_->Get(i, dValue))
					{
						//data_ = Te2String(dValue.GetDate());
						data_ = Te2String(dValue.Year()) + "." + Te2String(dValue.Month()) + "." + Te2String(dValue.Day());
					}
				break;

				case IBPP::sdTime:
					if(!result_->Get(i, tValue))
					{
						//data_ = Te2String(tValue.GetTime());
						data_ = Te2String(tValue.Hours()) + ":" + Te2String(tValue.Minutes()) + ":" + Te2String(tValue.Seconds());
					}
				break;

				case IBPP::sdTimestamp:
					if(!result_->Get(i, tsValue))
					{
						//data_ = Te2String(tsValue.GetTime());

						data_ = Te2String(tsValue.Year()) + "." + Te2String(tsValue.Month()) + "." + Te2String(tsValue.Day());
						data_ += " " + Te2String(tsValue.Hours()) + ":" + Te2String(tsValue.Minutes()) + ":" + Te2String(tsValue.Seconds());						
					}
				break;

				case IBPP::sdBlob:
					bValue  = IBPP::BlobFactory(firebird_, transaction_);
					
					if(!result_->Get(i, bValue))
					{
						bValue->Load(data_);
					}
					bValue->Close();
				break;

				default:
				break;
			}
		}
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = std::string("Error on function getData():\n") + e.ErrorMessage();
		return "-1";
	}
	
	return (char*) data_.c_str();
}

char* 
TeFirebirdPortal::getData (const string& s)
{
	std::string::size_type pos = s.find(".");
	std::string data = (pos == std::string::npos) ? s : s.substr(pos+1);

	int index = result_->ColumnNum(data);

	return getData(index-1);
}

bool TeFirebirdPortal::getBlob(const int& column, unsigned char* &data, unsigned long& size)
{
	try
	{
		IBPP::Blob blob = IBPP::BlobFactory(firebird_, transaction_);
//		std::string blobS;
		
		result_->Get(column, blob);

		char* buffer;
		int bufSize;
		convertBlob2Char(blob, buffer, bufSize);
		blob->Close();

		if(data == 0)
		{
			data = new unsigned char[bufSize];
		}

		size = bufSize;
		memcpy(data, buffer, bufSize);

		delete []buffer;
	}
	catch(IBPP::LogicException& e)
	{
		errorMessage_ = e.ErrorMessage();
		return false;
	}
	catch(...)
	{
	}

	return true;
}

bool
TeFirebirdPortal::getBlob (const string& s, unsigned char* &data, long& size) 
{
	std::string::size_type pos = s.find(".");
	std::string fieldS = (pos == std::string::npos) ? s : s.substr(pos+1);
	int field = result_->ColumnNum(fieldS);
	unsigned long aux;
	bool res = getBlob(field, data, aux);
	
	if(res)
	{
		size = static_cast<unsigned long>(aux);
	}

	return res;
}


bool 
TeFirebirdPortal::getBool (const string& s)
{
	int val = getInt(s);
	return val ? true : false;
}


bool 
TeFirebirdPortal::getBool (int i)
{
	int val = getInt(i);
	return val ? true : false;
}

bool 
TeFirebirdPortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{
	int idx = getColumnIndex("spatial_data")+1;
	return getBlob(idx, ptData, size);
}

bool
TeFirebird::inClauseValues(const string& query, const string& attribute, vector<string>& inClauseVector)
{
	int i = 0, numRows = 0, count = 0, chunk = 200;
	string id, inClause = "(", plic = "'";

	TeDatabasePortal *pt = getPortal();
	if(pt->query(query) == false)
	{
		delete pt;
		return false;
	}

	while (numRows < pt->numRows())
	{
		if(count < chunk && pt->fetchRow())
		{
			id = pt->getData(attribute);
			inClause = inClause + plic + id + "',";
			++count;
			++numRows;
		}
		else
		{
			inClause = inClause.substr(0, inClause.size()-1);
			inClause += ")";
			inClauseVector.push_back(inClause);
			count = 0;
			++i;
			inClause = "(";
		}
	}
	if (numRows == 0)
	{
		delete pt;
		return true;
	}
	else
	{
		inClause = inClause.substr(0, inClause.size()-1);
		inClause += ")";
		inClauseVector.push_back(inClause);
	}

	delete pt;
	return true;
}

string 
TeFirebird::getSQLTime(const TeTime& time) const
{
	string result = "'"+ time.getDateTime("YYYYsMMsDDsHHsmmsSS", ".", ":") +"'";
	return result;
}

string
TeFirebird::getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, 
						   const string& initialTime, const string& finalTime)
{
	
	//In Firebird the first day of the week is monday with index 0
	if(chr==TeDAYOFWEEK)
	{
		time1 = time1-1;
		if(time1<0)
			time1=6;
		time2 = time2-1;
		if(time2<0)
			time2=6;
	}

	return(TeDatabase::getSQLTemporalWhere(time1, time2, chr, rel, initialTime, finalTime));
}
	
string TeFirebird::getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName, const std::string& columnName)
{
	string wherebox;
	string lowerX, lowerY, upperX, upperY;
	if(rep == TePOLYGONS || rep == TeLINES || rep == TeCELLS || rep == TeRASTER || rep == TeRASTERFILE)
	{
		lowerX = "lower_x";
		lowerY = "lower_y";
		upperX = "upper_x";
		upperY = "upper_y";
	}
	else if(rep == TePOINTS || rep == TeTEXT)
	{
		lowerX = "x";
		lowerY = "y";
		upperX = "x";
		upperY = "y";
	}
	wherebox = "NOT("+ lowerX +" > " + Te2String(box.x2_, 10) + " OR ";
	wherebox += upperX +" < " + Te2String(box.x1_, 10) + " OR ";
	wherebox += lowerY +" > " + Te2String(box.y2_, 10) + " OR ";
	wherebox += upperY +" < " + Te2String(box.y1_, 10) + ")";
	return wherebox;
}

string TeFirebird::concatValues(vector<string>& values, const string& unionString)
{
	string concat = "CONCAT(";
	
	for(unsigned int i = 0; i < values.size(); ++i)
	{
		if(i != 0)
		{
			concat += ", ";

			if(!unionString.empty())
			{
				concat += "'";
				concat += unionString;
				concat += "'";
				concat += ", ";
			}
		}

		concat += values[i];
	}

	concat += ")";

	return concat;
}

bool 
TeFirebird::updateLayerBox(TeLayer* layer)
{
	if (!layer)
		return false;
	TeBox box = layer->box();
	string sql = "UPDATE te_layer SET lower_x = " + Te2String(box.x1());
		   sql += ", lower_y = " + Te2String(box.y1());
		   sql += ", upper_x = " + Te2String(box.x2());
		   sql += ", upper_y = " + Te2String(box.y2());
		   sql += " WHERE layer_id=" + Te2String(layer->id());

	return this->execute(sql);
}

string TeFirebird::toUpper(const string& value)
{
	string result  = "upper(";
	       result += value;
		   result += ")";

	return result;
}

void TeFirebird::setConnection(TeConnection* c)
{
    TeDatabase::setConnection(c);
	if(c != 0)
	{
		TeFirebirdConnection* fconn = dynamic_cast<TeFirebirdConnection*>(c);

		firebird_ = *fconn->getFBConnection();
		isConnected_ = firebird_->Connected();
		transaction_ = IBPP::TransactionFactory(firebird_, IBPP::amWrite, IBPP::ilConcurrency, IBPP::lrWait);
	}
	else
	{
		firebird_ = 0;
		isConnected_ = false;
	}
}

TeConnection* TeFirebird::getConnection()
{
    TeFirebirdConnection* c = new TeFirebirdConnection();
    c->setUser(user_);
    c->setHost(host_);
    c->setDatabaseName(database_);
    c->setDBMS(dbmsName_);
    c->setPassword(password_);
    c->setPortNumber(portNumber_);
    c->setFBConnection(&firebird_);

    return c;
}

bool
TeFirebird::createSpatialIndex(const string& table, const string& columns, TeSpatialIndexType /*type*/, short /*level*/, short /*tile*/)
{
	std::vector<std::string> columnsVector;
	TeSplitString(columns, ",", columnsVector);

	for (unsigned int i = 0; i < columnsVector.size(); i++) {

		std::string column = columnsVector[i];
		TeTrim(column);
		string idxName = "sp_idx_" + table + "_" + Te2String(i);
		bool created=false;
		if(column.find("lower")!=string::npos||
				column.find("resolution")!=string::npos) {
			created = createDescIndex(table, idxName, column);
		} else {
			created = createIndex(table, idxName, column);
		}
		if(!created) {
			throw TeException(UNKNOWN_ERROR_TYPE, ("Error creating spatial index for column '" + column + "'. " + errorMessage_));
		}
	}
	return true;

}

bool
TeFirebird::createDescIndex(const string& tableName, const string& indexName, const string& columnsName)
{
	string sql = "CREATE DESCENDING INDEX " +indexName + " ON " + tableName + "(" + columnsName + ")";
	return execute (sql);
}

bool
TeFirebird::deleteColumn (const string& table, const string& colName)
{
	if(!tableExist(table))
		return false;
	TeAttribute attr;
	if (!columnExist(table,colName,attr))
		return true;
	string drop = "ALTER  TABLE "+ table +" DROP "+ colName;
	if(execute(drop) == false)
		return false;

	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + table + "'";
	if(portal->query(sql) && portal->fetchRow())
		tableId = portal->getData(0);

	delete portal;
	if(tableId.empty() == false)
	{
		// delete relation
		sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
		sql += " AND related_attr = '" + colName + "')";
		sql += " OR (external_table_name = '" + table + "'";
		sql += " AND external_attr = '" + colName + "')";
		if(execute(sql) == false)
			return false;

		// delete grouping
		TeDatabasePortal* portal = getPortal();
		sql = "SELECT theme_id FROM te_grouping WHERE grouping_attr = '" + table + "." + colName + "'";
		if(portal->query(sql) && portal->fetchRow())
		{
			string themeId = portal->getData(0);

			sql = "DELETE FROM te_legend WHERE theme_id = " + themeId + " AND group_id >= 0";
			if(execute(sql) == false)
			{
				delete portal;
				return false;
			}
		}
		delete portal;

		sql = "DELETE FROM te_grouping";
		sql += " WHERE grouping_attr = '" + table + "." + colName + "'";
		if(execute(sql) == false)
			return false;
	}
	alterTableInfoInMemory(table);
	return true;
}
