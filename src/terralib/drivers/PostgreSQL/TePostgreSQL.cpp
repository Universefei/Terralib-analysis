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

#include "TePostgreSQL.h"
#include "TePostgreSQLConnection.h"
#include "TePGUtils.h"
#include "TePGInterface.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "../../kernel/TeSpatialOperations.h"
#include "../../kernel/TeGeometryAlgorithms.h"
#include "../../kernel/TeTimeInterval.h"
#include "../../kernel/TeProject.h"
#include <string>
using namespace std;

// Create a block of doubles for the bytea
unsigned char* getByteArray(const TeLine2D& l)
{
	unsigned int size = l.size();

	double* points = new double[2 * size];

	int iac = 0;

	for(unsigned int i = 0; i < size; ++i)
	{
		points[iac++] = l[i].x();
		points[iac++] = l[i].y();
	}

	size_t newLen = 0;

	// N�mero de doubles = 2 * size
	// N�mero de bytes = 2 * size * sizeof(double)
	unsigned char* newbuf = TePGConnection::escapeBytea((unsigned char*)points, 2 * size * sizeof(double) , &newLen);

	delete points;

	return newbuf;
}


//----- TePostgreSQL methods ---
TePostgreSQL::TePostgreSQL() :
        TeDatabase(),
        transactionCounter_(0)
{
	errorMessage_ = "";
	gistBoxOps_ = "";
	dbmsName_ = "PostgreSQL";
    tepg_connection_ = new TePGConnection();
    tepg_connection_ref = tepg_connection_;
}

TePostgreSQL::~TePostgreSQL()
{
	if(isConnected_)
		close();
    delete tepg_connection_ref;
}

void TePostgreSQL::setConnection(TeConnection* c)
{
    TeDatabase::setConnection(c);
	if(c != 0)
	{
		TePostgreSQLConnection* pconn = dynamic_cast<TePostgreSQLConnection*>(c);
		tepg_connection_ = pconn->getPGConnection();
		isConnected_ = tepg_connection_->state();
	}
	else
	{
		tepg_connection_ = tepg_connection_ref;
		isConnected_ = tepg_connection_->state();
	}
}

TeConnection* TePostgreSQL::getConnection()
{
    TePostgreSQLConnection* c = new TePostgreSQLConnection();
    c->setUser(user_);
    c->setHost(host_);
    c->setDatabaseName(database_);
    c->setDBMS(dbmsName_);
    c->setPassword(password_);
    c->setPortNumber(portNumber_);
    c->setPGConnection(tepg_connection_);
    
    return c;
}

string TePostgreSQL::errorMessage()
{
	if(errorMessage_.empty())
		return tepg_connection_->err_msg();
	else
		return errorMessage_;
	
}

bool TePostgreSQL::newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port, bool terralibModel, const std::string& characterSet)
{
	errorMessage_ = "";

	std::string createDB  = "CREATE DATABASE ";
	createDB += "\"" + database + "\"";
	if(!characterSet.empty())
	{
		createDB += " ENCODING = '" + characterSet + "'";
	}

	//if(!realConnect(host, user, password, "template1", port))
	//	return false;	

	if(!connect(host, user, password, "template1", port))
	{
		close();
		return false;
	}

	
	bool connected = false;
	
	if(execute(createDB))
	{
		connected = connect(host, user, password, database, port);
	}
	else
	{
		createDB += " TEMPLATE = template0";		
		if(execute(createDB))
		{
			connected = connect(host, user, password, database, port);
		}
		else
		{
			std::string err = "Error creating a new database: " + errorMessage();
			close();
			errorMessage_ = err;
			return false;
		}
	}

	if(terralibModel)
	{
		//create conceptual model
		if(connected)
		{
			if(!createConceptualModel())
			{
				close();
				return false;
			}
		}
		else
		{
			close();
			return false;
		}
	}

	return true;
}


bool TePostgreSQL::connect(const string& host, const string& user, const string& password, const string& database, int port)
{
	if(TePostgreSQL::realConnect(host, user, password, database, port))
	{
// see if PostGIS is present
		TePGRecordset rec;

		string sql = "SELECT postgis_version()";

		if(rec.open(sql.c_str(), tepg_connection_) && rec.recordCount() > 0)
		{
			rec.close();

			this->close();

			errorMessage_ = "PostGIS extension found! Please use a PostGIS driver!";

			return false;	
		}

// see if RTree GiST is present for box: it must be present
		int version = PQserverVersion(tepg_connection_->c_ptr());
		if(version >= 80100)
		{
			gistBoxOps_ = "box_ops";
			if(version >= 90000)
			{
				//modification because of the new default bytea type
				execute("set bytea_output = 'escape'");
			}
			return true;
		}

		sql = "SELECT opcname FROM pg_opclass WHERE (opcname = 'gist_box_ops') ORDER BY opcname DESC";

		if(rec.open(sql.c_str(), tepg_connection_) && rec.recordCount() > 0)
		{
			gistBoxOps_ = rec.value(0);
			rec.close();

			return true;
		}

		rec.close();

		this->close();

		errorMessage_ = "GiST extension not found! Please, install R-Tree GiST in your database and try again!";

		return false;	
	}
	
	return false;
}

bool TePostgreSQL::connect(pg_conn* conn)
{
	this->close();

	if(tepg_connection_->open(conn))
	{	
		TePGRecordset rec;

		string sql = "SELECT postgis_version()";

		if(rec.open(sql.c_str(), tepg_connection_) && rec.recordCount() > 0)
		{
			rec.close();

			this->close();

			errorMessage_ = "PostGIS extension found! Please use a PostGIS driver!";

			return false;	
		}

// see if RTree GiST is present for box: it must be present
		int version = PQserverVersion(tepg_connection_->c_ptr());
		if(version >= 80100)
		{
			gistBoxOps_ = "box_ops";
			if(version >= 90000)
			{
				//modification because of the new default bytea type
				execute("set bytea_output = 'escape'");
			}
			return true;
		}

		sql = "SELECT opcname FROM pg_opclass WHERE (opcname = 'gist_box_ops') ORDER BY opcname DESC";

		if(rec.open(sql.c_str(), tepg_connection_) && rec.recordCount() > 0)
		{
			gistBoxOps_ = rec.value(0);
			rec.close();

//			host_     = tepg_connection_->.dbHost();
//			user_     = tepg_connection_->dbUser();
			password_ = "";
			database_ = tepg_connection_->dbname();	

			return true;
		}

		rec.close();

		this->close();

		errorMessage_ = "GiST extension not found! Please, install R-Tree GiST in your database and try again!";
	}
	return false;	
}

bool TePostgreSQL::realConnect(const string& host, const string& user, const string& password, const string& database, int port)
{
	errorMessage_ = "";

	host_             = (host == "")     ? (string("")) : (" host = " + host + " ");
	user_             = (user == "")     ? (string("")) : (" user = " + user + " ");
	password_         = (password == "") ? (string("")) : (" password = " + password + " ");
	database_         = (database == "") ? (string("")) : (" dbname = '" + database + "' ");
	string portNumber = (port <= 0)     ? (string("")) : (" port = " + Te2String(port)  + " ");

	portNumber_ = port;

        std::string connInfo = host_ + user_ + password_ + database_ + portNumber;

	host_     = host;
	user_     = user;
	password_ = password;
	database_ = database;

	this->close();

	if(!tepg_connection_->open(connInfo))
	{
		errorMessage_ = "Could not connect to the database server!";
		return false;
	}

	isConnected_ = true;

	return true;
}

bool TePostgreSQL::showDatabases(const string& host, const string& user, const string& password, vector<string>& dbNames, int port)
{
	errorMessage_ = "";

	if(realConnect(host, user, password, "template1", port))
	{
		string sql = "SELECT datname FROM pg_database WHERE datname NOT IN ('template0', 'template1', 'template_postgis', 'postgres') ORDER BY datname";

		TePostgreSQLPortal p(this);

		if(p.query(sql) && p.fetchRow())
		{
			do
			{
				dbNames.push_back(p.getData("datname"));
			}while(p.fetchRow());

			return true;
		}
		else
			errorMessage_ = "Didn't find any database!";
	}
	
	return false;
}

void TePostgreSQL::close(void)
{
	errorMessage_ = "";

	tepg_connection_->close();
	isConnected_ = false;
	transactionCounter_ = 0;

	return;
}

bool TePostgreSQL::listTables(vector<string>& tableList)
{
	errorMessage_ = "";

	TePGRecordset rec;

	string sql = "SELECT tablename FROM pg_table WHERE schemaname = 'public' ORDER BY tablename";

	if(!rec.open(sql, tepg_connection_) || rec.recordCount() <= 0)
		return false;

	while(!rec.eof())
	{
		tableList.push_back(rec.getData(0));
		rec.moveNext();
	}
	
	return true;

}

bool TePostgreSQL::listDatabases(std::vector<std::string> &databaseList)
{
	TePGRecordset	rec;
	std::string		sql;

	if(tepg_connection_->state() == false) return false;
	errorMessage_.clear();
	databaseList.clear();
	sql = "SELECT datname FROM pg_database WHERE datistemplate IS FALSE AND datallowconn IS TRUE AND datname!='postgres' order by datname";
	if(!rec.open(sql, tepg_connection_) || rec.recordCount() <= 0)	return false;
	while(!rec.eof())
	{
		databaseList.push_back(rec.getData(0));
		rec.moveNext();
	}
	rec.close();
	return true;
}

bool TePostgreSQL::dropDatabase(const std::string &databaseName)
{
	std::string	sql;
	if( TeConvertToUpperCase(this->database_) == TeConvertToUpperCase(databaseName) )
	{
		errorMessage_="Impossible drop the current database";
		return false;
	}

	if(tepg_connection_->state() == false) return false;
	errorMessage_.clear();
	sql="DROP DATABASE " + databaseName;
	return this->execute(sql);
}

bool TePostgreSQL::tableExist(const string& table)
{
	if (table.empty())
		return false;

	errorMessage_ = "";

	TePGRecordset rec;

	string sql = "SELECT relname FROM pg_class WHERE lower(relname) = lower('" + table + "')";

	if(!rec.open(sql, tepg_connection_))
		return false;

	return (rec.recordCount() > 0);
}

bool TePostgreSQL::deleteTable(const string& table)
{
	int f =	table.find ("te_collection");

	if( table=="te_theme" ||
		table=="te_layer" ||
		table=="te_representation" ||
		table=="te_tables_relation" ||
		table=="te_layer_table" ||
		table=="te_raster_metadata" ||
		table=="te_projection" ||
		table=="te_view" ||
		table=="te_legend" ||
		table=="te_visual" ||
		table=="te_database" ||
		f == 0)
	{
		errorMessage_ = "N�o � poss�vel deletar tabelas do modelo!";
		return false;
	}

	if (table.empty())
		return false;

	errorMessage_ = "";

	std::string sql = "SELECT relname, relkind FROM pg_class WHERE lower(relname) = lower('" + table + "')";

	TePGRecordset rec;
	if(!rec.open(sql, tepg_connection_))
		return false;

	if(rec.recordCount() == 0)
	{
		return false;
	}

	std::string dbTableName = rec.getData(0);
	std::string kind = rec.getData(1);

	rec.close();

	std::string del = "DROP TABLE \"" + dbTableName + "\"";
	if(kind == "v")
	{
		del = "DROP VIEW \"" + dbTableName + "\"";
	}

	if(!execute(del))
	{
		return false;
	}

	return true;
}

bool TePostgreSQL::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	errorMessage_ = "";

	TePGRecordset rec;

	string sql  = "SELECT atttypid, attname, atttypmod, attlen  FROM pg_class, pg_attribute ";
	       sql += "WHERE lower(pg_class.relname) = lower('" + table + "') AND ";
		   sql += "      pg_class.oid = pg_attribute.attrelid AND ";
		   sql += "      pg_attribute.attname = lower('" + column + "')";

	if(!rec.open(sql, tepg_connection_))
		return false;

	if(rec.recordCount() > 0)
	{
		attr.rep_.name_ = rec.getData("attname");

		Oid nType = static_cast<Oid>(rec.getInt("atttypid"));
		switch(nType)
		{
			case 16:    //BOOL
			case 20:    //INT8
			case 21:    //INT2
			case 23:    //INT4
			case 26:    //OID			
						attr.rep_.type_ = TeINT;
						attr.rep_.numChar_ = 15;
						break;

			case 700 :  //float4  -> float(p)
			case 701 :  //float8  -> float(p)
			case 790 :  //money   -> decimal(9, 2)  
			case 1700:  //numeric -> numeric(p, s)  
						attr.rep_.type_ = TeREAL;
						attr.rep_.numChar_ = 15;
						break;

			case 1082:	//date -> date
			case 1083:  //time -> time
			case 1114:  //timestamp
			case 1186:  //interval
			case 1266:	//
						attr.rep_.type_ = TeDATETIME;
						attr.rep_.numChar_ = rec.getInt("attlen");
						break;

			//case 26:    //OID
			case 17:	//bytea
						attr.rep_.type_ = TeBLOB;
						attr.rep_.numChar_ = 0;
						break;
			case 1042:
						attr.rep_.type_ = TeCHARACTER;
						attr.rep_.numChar_ = rec.getInt("atttypmod") - 4;
						break;

			case 1043:  //varchar(n)
						attr.rep_.type_ = TeSTRING;
						attr.rep_.numChar_ = rec.getInt("atttypmod") - 4;
						break;

			case 25:    //text
						attr.rep_.type_ = TeSTRING;
						break;

			default:
						attr.rep_.type_ = TeUNKNOWN;
						attr.rep_.numChar_ = 0;
						break;
		}

		return true;
	}

	return false;
}

bool TePostgreSQL::createTable(const string& table, TeAttributeList &attr)
{
	errorMessage_ = "";

	bool first = true;

	TeAttributeList::iterator it = attr.begin();
	
	string createTable ="CREATE TABLE " + table +" (";
	
	string type;

	string pkeys;

	while(it != attr.end())
	{
		switch ((*it).rep_.type_)
		{
			case TeSTRING:		if((*it).rep_.numChar_ > 0)
								{
									type = "VARCHAR(" + Te2String((*it).rep_.numChar_) + ")";
								}
								else
								{
									type = "TEXT";
								}
								break;

			case TeREAL:		type = "FLOAT8";
								break;

			case TeINT:
			case TeUNSIGNEDINT:
								type = ((*it).rep_.isAutoNumber_) ? "SERIAL" : "INTEGER";
								break;

			case TeBLOB:		//type = "OID";
				                type = "BYTEA";
								break;

			case TeDATETIME:	type = "TIMESTAMP(0)";
								break;

			case TeCHARACTER:	type = "CHAR(" + (((*it).rep_.numChar_ == 0)? std::string("1") : Te2String((*it).rep_.numChar_)) + ") ";
								break;

			case TeBOOLEAN:		type = "BOOLEAN";
								break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
								if(!first)
									createTable += ", ";
								else
									first = false;

								createTable += " spatial_box  BOX           NOT NULL,";
								createTable += " x		      FLOAT8        DEFAULT 0.0,";
								createTable += " y            FLOAT8        DEFAULT 0.0 ";
								++it;
								continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " num_coords   INTEGER      NOT NULL,";
							createTable += " spatial_box  BOX          NOT NULL,";
							createTable += " ext_max      FLOAT8       NOT NULL,";
							createTable += " spatial_data POLYGON ";
							++it;
							continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " num_coords   INTEGER      NOT NULL,";
							createTable += " num_holes    INTEGER      NOT NULL,";
							createTable += " parent_id    INTEGER      NOT NULL,";
							createTable += " spatial_box  BOX          NOT NULL,";
							createTable += " ext_max      FLOAT8       NOT NULL,";
							createTable += " spatial_data POLYGON ";
							++it;
							continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " spatial_box  BOX          NOT NULL,";
							createTable += " col_number   INTEGER      NOT NULL,";
							createTable += " row_number	  INTEGER      NOT NULL ";
							++it;
							continue;					

			case TeRASTERTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " block_box         BOX         NOT NULL,";
							createTable += " band_id		     INTEGER     NOT NULL,";		   
							createTable += " resolution_factor INTEGER     NOT NULL,";
							createTable += " subband		     INTEGER     NOT NULL,";		   
							createTable += " spatial_data      BYTEA,";
							createTable += " block_size        INTEGER  NOT NULL ";
		   					++it;
							continue;

			case TeNODETYPE:
			case TeNODESETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " spatial_box  BOX           NOT NULL,";
							createTable += " x		    FLOAT8        DEFAULT 0.0,";
							createTable += " y            FLOAT8        DEFAULT 0.0 ";
							++it;
							continue;

			case TeTEXTTYPE:
			case TeTEXTSETTYPE:

			default:			type += "VARCHAR(" + Te2String((*it).rep_.numChar_) + ")";
								break;
		}

		if(!((*it).rep_.defaultValue_.empty()))
			type += " DEFAULT '" + (*it).rep_.defaultValue_ + "' ";

		if(!((*it).rep_.null_))
			type += " NOT NULL ";
		
		if(!first)
			createTable += ",  ";
		else
			first = false;

		createTable += (*it).rep_.name_ + " ";
		createTable += type;

		// check if column is part of primary key
		if((*it).rep_.isPrimaryKey_ && (*it).rep_.type_ != TeBLOB )
		{
			if(!pkeys.empty())
				pkeys += ", ";
			
			pkeys += (*it).rep_.name_;
		}

		++it;
	}

	if(!pkeys.empty())
	{	string pk = ", PRIMARY KEY(";
	           pk += pkeys;
			   pk += ")";

		createTable += pk;
	}


	createTable += ");";

	return execute(createTable);
}

bool TePostgreSQL::addColumn(const string& table, TeAttributeRep &rep)
{
	errorMessage_ = "";
	
	string field = TeGetExtension(rep.name_.c_str());
	if(field.empty())
		field = rep.name_;

	string new_column  = "ALTER TABLE " + table + " ADD COLUMN " + field + " ";
	switch(rep.type_)
	{
		case TeSTRING:		if(rep.numChar_ > 0)
							{
								new_column += "VARCHAR(" + Te2String(rep.numChar_) + ")";
							}
							else
							{
								new_column += "TEXT";
							}
							
							break;
		case TeREAL:		new_column += "FLOAT8";
							break;
		case TeBOOLEAN:		new_column += "BOOLEAN";
							break;
		case TeINT:			new_column += (rep.isAutoNumber_ ? "SERIAL" : "INTEGER");
							break;
		case TeDATETIME:	new_column += "TIMESTAMP(0)";
							break;
		case TeBLOB:		//new_column += "OID";
							new_column += "BYTEA";
							break;
		default:			new_column += "VARCHAR";
							break;
	}

	if(!rep.defaultValue_.empty())
	{
		new_column += " DEFAULT ";
		
		new_column += rep.defaultValue_;
	}

	if(rep.isPrimaryKey_ == true)
	{
		new_column += " PRIMARY KEY";
	}
	
	if(!this->execute(new_column))
        return false;

	alterTableInfoInMemory(table);

	return true;
}

bool TePostgreSQL::createRelation(const string& relName, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
{
	errorMessage_ = "";

	string alter  = "ALTER TABLE " +  table + " ADD CONSTRAINT " + relName + " ";
	       alter += "FOREIGN KEY (" + fieldName + ") "; 
	       alter += "REFERENCES " + relatedTable + "(" + relatedField + ") ";

	if(cascadeDeletion)
		alter += " ON DELETE CASCADE";

	return execute(alter);
}

TeDBRelationType TePostgreSQL::existRelation(const string& /*tableName*/, const string& relName)
{
	errorMessage_ = "";
	std::string sql = "select confdeltype from pg_constraint where conname = '" + relName + "'";
	
	TeDBRelationType resp = TeNoRelation;

	TePGRecordset r(sql, tepg_connection_);
	if(r.recordCount() > 0)
	{
		std::string delType = r.getData(0);
		if(delType == "c")
		{
			resp = TeRICascadeDeletion; 
		}
		else
		{
			resp = TeRINoCascadeDeletion; 
		}
	}	

	r.close();
	
	return resp;
}

bool TePostgreSQL::execute(const string &sql)
{
	try
	{
		this->tepg_connection_->exec_cmd(sql);
	}

	catch(...)
	{
		TeWriteToFile("erros42.txt", "-------------------\nsql=" + sql + "\nerror="+ this->errorMessage() + "\n\n", "a+");
		return false;
	}

	return true;
}

TeDatabasePortal* TePostgreSQL::getPortal()
{
	errorMessage_ = "";

	TeDatabasePortal *portal = new TePostgreSQLPortal(this);

	return portal;
}

bool TePostgreSQL::insertRelationInfo(const int tableId, const string& tField, const string& rTable, const string& rField, int& relId)
{
	if(tableId < 0)
	{
		return false;
	}
	// check if relation already exists
	TePostgreSQLPortal* portal = (TePostgreSQLPortal*)this->getPortal();

	if(!portal)
		return false;

	relId = -1;
	
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " related_table_id = " + Te2String(tableId);
	sel += " AND related_attr = '" + tField + "'";
	sel += " AND external_table_name = '" + rTable + "'";
	sel += " AND external_attr = '" + rField + "'";

	if(!portal->query(sel))
	{
		delete portal;
		return false;
	}

	if(portal->fetchRow())
	{
		relId = atoi(portal->getData("relation_id"));
		delete portal;
		return true;
	}

	delete portal;

	string sql =  "INSERT INTO te_tables_relation (related_table_id, related_attr, external_table_name, external_attr) VALUES(";
	       sql += Te2String(tableId);
		   sql += ", '";
		   sql += tField;
		   sql += "', '";
		   sql += rTable;
		   sql += "', '";
		   sql += rField;
		   sql += "')";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_tables_relation_relation_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);

		if(rec.recordCount() > 0)
			relId = rec.getInt(0);

		rec.close();
	}
	else
		return false;

	return true;
}

bool 
TePostgreSQL::insertTable(TeTable &table)
{
	string tableName = table.name();
	int size = table.size();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	TeAttributeList::iterator itEnd = att.end();
	TeTableRow row;
	
	if (!beginTransaction())
		return false;

	size_t newLen = 0;
	unsigned char* newbuf = 0;
	int i;
	unsigned int j;
	for ( i = 0; i < size; i++ )	
	{
		row = table[i];
		it = att.begin();
		string attrs;
		string values;
		j = 1;
		int jj = 0;
		while ( it != itEnd )
		{
			if (row[jj].empty() || (*it).rep_.isAutoNumber_)
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
			switch ((*it).rep_.type_)
			{
			case TeSTRING:
				values += "'"+ escapeSequence( row[jj] ) +"'";
				break;
			case TeREAL:
				{
					std::string strValue = row[jj];
					replace(strValue.begin(), strValue.end(), ',', '.');
					values += strValue;
				}
				break;
			case TeUNSIGNEDINT:
			case TeINT:
				values += row[jj];
				break;
			case TeDATETIME:
				{
					const string temp_dt = string(row[jj].c_str());
					TeTime t(temp_dt, (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_, (*it).timeSeparator_, (*it).indicatorPM_);
					values += this->getSQLTime(t);
				}
				break;
			case TeCHARACTER:
				values += "'" + escapeSequence(row[jj]) + "'";
				break;

			case TeBOOLEAN: "'" + row[jj] + "'";
				break;

			case TeBLOB:
				newbuf = TePGConnection::escapeBytea((unsigned char*)row[jj].c_str(), row[jj].size(), &newLen);
				values += "'";
				values += (char*)newbuf; 
				values += "'";
				TePGConnection::freeMem(newbuf);
				newbuf = NULL;
			default:
				values += "'"+ escapeSequence(row[jj]) +"'";
				break;
			}
			++it;
			j++;
			jj++;
		}

		if (values.empty()) 
			continue;

		string q = "INSERT INTO "+tableName + " ("+ attrs+") " + " VALUES ("+values+") ";
		if(!execute(q))
		{
			rollbackTransaction();
			return false;
		}
	}

	if (!commitTransaction())
	{
		rollbackTransaction();
		if(newbuf)
		{
			TePGConnection::freeMem(newbuf);
			newbuf = NULL;
		}
		return false;
	}
	if(newbuf)
	{
		TePGConnection::freeMem(newbuf);
		newbuf = NULL;
	}
	return true;
}

bool
TePostgreSQL::updateTable(TeTable &table)
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

	size_t newLen = 0;
	unsigned char* newbuf = 0;
	TeAttributeList::iterator it;
	TeTableRow row;
	unsigned int j;	
	for (i = 0; i < table.size(); i++  )
	{
		row = table[i];
		it = att.begin();
		string q;
		j = 1;
		int jj = 0;
		bool useComma = false;
		while ( it != att.end() )
		{
			if (TeConvertToUpperCase(uniqueName) != TeConvertToUpperCase((*it).rep_.name_))
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
					case TeUNSIGNEDINT:
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
						newbuf = TePGConnection::escapeBytea((unsigned char*)row[jj].c_str(), row[jj].size(), &newLen);
						q += "'";
						q += (char*)newbuf; 
						q += "'";
						TePGConnection::freeMem(newbuf);
						newbuf = NULL;
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
			if(newbuf)
			{
				TePGConnection::freeMem(newbuf);
				newbuf = NULL;
			}
			return false;
		}
	}
	if (!commitTransaction())
	{
		rollbackTransaction();
		if(newbuf)
		{
			TePGConnection::freeMem(newbuf);
			newbuf = NULL;
		}
		return false;
	}
	if(newbuf)
	{
		TePGConnection::freeMem(newbuf);
		newbuf = NULL;
	}
	return true;
}

bool TePostgreSQL::insertTableInfo(int layerId, TeTable& table, const string& user)
{
	errorMessage_ = "";

	if(table.name().empty())
		return false;

	if(table.id() <= 0)
	{
		string sql  = "INSERT INTO te_layer_table (";
		
		if(layerId > 0)
		{
			sql += "layer_id, ";
		}

		sql += "attr_table, unique_id, attr_link, attr_initial_time, attr_final_time, ";
		sql += "attr_time_unit, attr_table_type, user_name) VALUES(";
		
		if(layerId > 0)
		{
			sql += Te2String(layerId) + ", ";
		}	
		
		sql += "'" + table.name();
		sql += "', '" + table.uniqueName();		
		sql += "', '";
		sql += table.linkName();
		sql += "', '";
		sql += table.attInitialTime();
		sql += "', '";
		sql += table.attFinalTime();
		sql += "', ";
		sql += Te2String(table.attTimeUnit());
		sql += ", ";
		sql += Te2String(table.tableType());
		sql += ", '";
		sql += user;
		sql += "')";

		if(this->execute(sql))
		{
			sql = "SELECT currval('te_layer_table_table_id_seq')";
			TePGRecordset rec;
			rec.open(sql, tepg_connection_);

			if(rec.recordCount() > 0)
				table.setId(rec.getInt(0));

			rec.close();
		}
		else
			return false;
	}
	
	return true;	
}


bool TePostgreSQL::alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName)
{
	if(!tableExist(tableName))
		return false;

	string tab;

	if(!oldColName.empty())
	{
		tab  = " ALTER TABLE " + tableName + " RENAME COLUMN ";
		tab += oldColName +  " TO " + rep.name_;
	}
	else
	{
		tab  = " ALTER TABLE " + tableName + " ALTER COLUMN ";
		tab += rep.name_ + " TYPE ";

		switch (rep.type_)
		{
			case TeSTRING:
				if(rep.numChar_ > 0)
				{
					tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
				}
				else
				{
					tab += "TEXT ";
				}	
				break;
				
			case TeREAL:
				tab += "FLOAT8";	
				break;
				
			case TeINT:
				tab += (rep.isAutoNumber_ ? "SERIAL" : "INTEGER");
				break;

			case TeDATETIME:
				tab += "TIMESTAMP(0)";
				break;

			case TeCHARACTER:
				tab += "CHAR";
				break;
			case TeBOOLEAN:		
				tab += "BOOLEAN";
				break;

			case TeBLOB:
				tab += "BYTEA";
				break; 
			
			default:
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
				break;
		}	
	}

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

bool TePostgreSQL::alterTable(const string& oldTableName, const string& newTableName)
{
	string sql = " ALTER TABLE \""+ oldTableName +"\" RENAME TO "+ newTableName;
	if(!this->execute(sql))
		return false;

	//update te_layer_table
	sql = " UPDATE te_layer_table ";
	sql += " SET attr_table = '"+ newTableName +"'";
	sql += " WHERE attr_table = '"+ oldTableName +"'";
	execute(sql);

	//update te_tables_relation
	sql = " UPDATE te_tables_relation ";
	sql += " SET external_table_name = '"+ newTableName +"'";
	sql += " WHERE external_table_name = '"+ oldTableName +"'";
	execute(sql);
	
	alterTableInfoInMemory(newTableName, oldTableName);
	return true;
}

bool TePostgreSQL::insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size)
{
	if (whereClause.empty())
		return false;

	errorMessage_ = "";

	TePostgreSQLPortal* portal = static_cast<TePostgreSQLPortal*>(this->getPortal());
	
	if(!portal)
		return false;

	string q = "SELECT * FROM "+ tableName +" WHERE "+ whereClause;
	if((!portal->query(q)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}

//	int exist = portal->getInt(columnBlob);

	delete portal;

	size_t newLen = 0;

	unsigned char* newbuf = TePGConnection::escapeBytea(data, size, &newLen);

	string sql  = "UPDATE ";
	       sql += tableName;
	       sql += " SET ";
	       sql += columnBlob;
	       sql += " = '";
	       sql += (char*)newbuf;
		   sql += "' WHERE ";
		   sql += whereClause;

	if (!TePostgreSQL::execute(sql))
	{
		TePGConnection::freeMem(newbuf);
		newbuf = NULL;

		errorMessage_ = "Couldn't update blob!";

		return false;
	}

	TePGConnection::freeMem(newbuf);
	newbuf = NULL;

	return true;
}

bool TePostgreSQL::insertProjection(TeProjection *proj)
{
	errorMessage_ = "";

	TeProjectionParams par = proj->params();

	string sql  = "INSERT INTO te_projection (name, long0, lat0, offx, ";
		   sql += "offy, stlat1, stlat2, unit, scale, ";
		   sql += "hemis, datum) VALUES('";
		   sql += proj->name();
		   sql += "', ";
		   sql += Te2String(par.lon0*TeCRD, 15);
		   sql += ", ";
		   sql += Te2String(par.lat0*TeCRD, 15);
		   sql += ", ";
		   sql += Te2String(par.offx, 15);
		   sql += ", ";
		   sql += Te2String(par.offy, 15);
		   sql += ", ";
		   sql += Te2String(par.stlat1*TeCRD, 15);
		   sql += ", ";
		   sql += Te2String(par.stlat2*TeCRD, 15);
		   sql += ", '";
		   sql += par.units;
		   sql += "', ";
		   sql += Te2String(par.scale, 15);
		   sql += ", ";
		   sql += Te2String(par.hemisphere);
		   sql += ", '";
		   sql += proj->datum().name();
		   sql += "')";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_projection_projection_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			proj->id(rec.getInt(0));

		rec.close();
	}
	else
		return false;

	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}


bool TePostgreSQL::insertLayer(TeLayer *layer)
{
	errorMessage_ = "";

	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Error inserting projection!";
		return false;
	}

	string sql  = "INSERT INTO te_layer (projection_id, name, lower_x, lower_y, upper_x, ";
		   sql += "upper_y, edition_time) VALUES(";
		   sql += Te2String(proj->id());
		   sql += ", '";
		   sql += layer->name();
	       sql += "', ";
	       sql += Te2String(layer->box().x1(), 15);
	       sql += ", ";
	       sql += Te2String(layer->box().y1(), 15);
	       sql += ", ";
	       sql += Te2String(layer->box().x2(), 15);
	       sql += ", ";
	       sql += Te2String(layer->box().y2(), 15);
		   sql += ", ";
		   TeTime editionTime = layer->getEditionTime();
		   sql += getSQLTime(editionTime);
	       sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_layer_layer_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
		{
			layer->id(rec.getInt(0));
		}
		rec.close();
	}
	else
		return false;

	layerMap()[layer->id()] = layer;

	return true;	
}


bool TePostgreSQL::insertProject(TeProject *project)
{
	errorMessage_ = "";

	if (!project)
		return false;

	string sql  = "INSERT INTO te_project (name, description, current_view) VALUES(";
		   sql += " '";
		   sql += project->name();
	       sql += "', '";
		   sql += project->description();
	       sql += "', ";
		   sql += Te2String(project->getCurrentViewId());
	       sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_project_project_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
		{
			project->setId(rec.getInt(0));
		}
		rec.close();
	}
	else
		return false;
	projectMap()[project->id()] = project;
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);
	return true;	
}



bool TePostgreSQL::insertRepresentation(int layerId, TeRepresentation& rep)
{	
	errorMessage_ = "";

	if(layerId <= 0)
		return false;
	

	string sql  = "INSERT INTO te_representation (layer_id, geom_type, geom_table, description, lower_x, ";
		   sql += "lower_y, upper_x, upper_y, res_x, res_y, num_cols, num_rows) VALUES(";
		   sql += Te2String(layerId);
		   sql += ", ";
		   sql += Te2String(static_cast<long>(rep.geomRep_));
		   sql += ", '";
		   sql += rep.tableName_;
		   sql += "', '";
		   sql += rep.description_;		   
		   sql += "', ";
           sql += Te2String(rep.box_.x1(), 15);
		   sql += ", ";
		   sql += Te2String(rep.box_.y1(), 15);
		   sql += ", ";
		   sql += Te2String(rep.box_.x2(), 15);
		   sql += ", ";
		   sql += Te2String(rep.box_.y2(), 15);
		   sql += ", ";
		   sql += Te2String(rep.resX_, 15);
		   sql += ", ";
		   sql += Te2String(rep.resY_, 15);
		   sql += ", ";
		   sql += Te2String(static_cast<long>(rep.nCols_));
		   sql += ", ";
		   sql += Te2String(static_cast<long>(rep.nLins_));
		   sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_representation_repres_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			rep.id_ = rec.getInt(0);

		rec.close();
	}
	else
		return false;

	return true;		
}

bool TePostgreSQL::insertView(TeView *view)
{
	errorMessage_ = "";

	// save it�s projection
	TeProjection* proj = view->projection();
	if ( !proj || !insertProjection(proj))
	{
		errorMessage_ = "Error inserting projection";
		return false;
	}

	string sql  = "INSERT INTO te_view (projection_id, name, user_name, visibility, lower_x, lower_y, upper_x, upper_y, current_theme) VALUES(";
	       sql += Te2String(proj->id());
		   sql += ", '";
		   sql += view->name();
		   sql += "', '";
		   sql += view->user();
		   sql += "', ";
		   sql += Te2String(view->isVisible()) + ", ";
		   sql += Te2String(view->getCurrentBox().lowerLeft().x(),15) + ", ";
		   sql += Te2String(view->getCurrentBox().lowerLeft().y(),15) + ", ";
		   sql += Te2String(view->getCurrentBox().upperRight().x(),15) + ", ";
		   sql += Te2String(view->getCurrentBox().upperRight().y(),15) + ", ";
		   if(view->getCurrentTheme() == -1)
				sql += "null ";
		   else
				sql += Te2String(view->getCurrentTheme());
		   sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_view_view_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
		{
			view->id(rec.getInt(0));
		}
		rec.close();
	}
	else
		return false;

	for(unsigned int th = 0; th < view->size(); ++th)
	{
		TeViewNode* node = view->get (th);
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

	viewMap()[view->id()] = view;

	return true;
}

bool TePostgreSQL::insertViewTree(TeViewTree *tree)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO te_theme (view_id, name, parent_id, priority, node_type) VALUES(";

		   sql += Te2String(tree->view());
		   sql += ", '";
	       sql += tree->name();
	       sql += "', ";
	       sql += Te2String(tree->parentId());
	       sql += ", ";
		   sql += Te2String(tree->priority());
	       sql += ", ";
	       sql += Te2String(tree->type());
	       
	       sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_theme_theme_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			tree->id(rec.getInt(0));

		rec.close();
	}
	else
		return false;
		
	return true;	
}

bool TePostgreSQL::insertThemeGroup(TeViewTree* tree)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO te_theme (name, parent_id, node_type, view_id, priority) VALUES(";
	       sql += "'";
		   sql += tree->name();
		   sql += "', 1, 1, ";
		   sql += Te2String(tree->view());
		   sql += ", ";
	       sql += Te2String(tree->priority());
	       sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_theme_theme_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			tree->id(rec.getInt(0));

		rec.close();
	}
	else
		return false;
		
	return true;	
}

bool TePostgreSQL::insertTheme(TeAbstractTheme *theme)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO te_theme (layer_id, view_id, name, parent_id, priority, node_type, ";
	       sql += "min_scale, max_scale, generate_attribute_where, generate_spatial_where, ";
		   sql += "generate_temporal_where, collection_table, visible_rep, enable_visibility, ";
		   sql += "lower_x, lower_y, upper_x, upper_y, creation_time) VALUES(";
		   
		   if(theme->type()==TeTHEME)
			   sql += Te2String(static_cast<TeTheme*>(theme)->layerId());
		   else
			   sql += " NULL ";
		   sql += ", ";
		   sql += Te2String(theme->view());
		   sql += ", '";
		   sql += theme->name();
		   sql += "', ";
		   sql += Te2String(theme->parentId());
		   sql += ", ";
		   sql += Te2String(theme->priority());
		   sql += ", ";
		   sql += Te2String(theme->type());
		   sql += ", ";
		   sql += Te2String(theme->minScale(), 15);
		   sql += ", ";
		   sql += Te2String(theme->maxScale(), 15);
		   sql += ", '";
		   sql += escapeSequence(theme->attributeRest());
		   sql += "', '";
		   sql += escapeSequence(theme->spatialRest());
		   sql += "', '";
		   sql += escapeSequence(theme->temporalRest());
		   sql += "', '";
		   if(theme->type()==TeTHEME)
				sql += static_cast<TeTheme*>(theme)->collectionTable();
		   sql += "', ";
		   sql += Te2String(theme->visibleRep());
		   sql += ", ";
		   sql += Te2String(theme->visibility());
		   sql += ", ";
		   sql += Te2String(theme->box().x1(), 15);
		   sql += ", ";
		   sql += Te2String(theme->box().y1(), 15);
		   sql += ", ";
		   sql += Te2String(theme->box().x2(), 15);
		   sql += ", ";
		   sql += Te2String(theme->box().y2(), 15);
		   sql += ", ";
		   TeTime creationTime = theme->getCreationTime();
		   sql += getSQLTime(creationTime);
		   sql += " )";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_theme_theme_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
		{
			theme->id(rec.getInt(0));
		}
		rec.close();
	}
	else
		return false;


	if((theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME) && static_cast<TeTheme*>(theme)->collectionTable().empty())
	{
		sql  = "UPDATE te_theme SET";
		sql += " collection_table = 'te_collection_" + Te2String(theme->id()) + "'";
		sql += " WHERE theme_id = ";
		sql += Te2String(theme->id());

		static_cast<TeTheme*>(theme)->collectionTable(string("te_collection_") + Te2String(theme->id()));

		if(!this->execute(sql))
			return false;
	}
	if(theme->parentId() == 0)
	{
		std::string sql = "UPDATE te_theme SET";
		sql += "  parent_id = " + Te2String(theme->id());
		sql += " WHERE theme_id = ";
		sql += Te2String(theme->id());

		theme->parentId(theme->id());

		if(!this->execute(sql))
			return false;
	}

	bool status;

	// insert grouping
	int numSlices = 0;
	if(theme->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (theme->id(), theme->grouping()))
			return false;
		numSlices = theme->grouping().groupNumSlices_;
	}
		
	// insert legend
	theme->outOfCollectionLegend().group(-1); 
	theme->outOfCollectionLegend().theme(theme->id()); 
	status = insertLegend (&(theme->outOfCollectionLegend())); 
	if (!status)
		return status;

	theme->withoutDataConnectionLegend().group(-2); 
	theme->withoutDataConnectionLegend().theme(theme->id()); 
	status = insertLegend (&(theme->withoutDataConnectionLegend())); 
	if (!status)
		return status;

	theme->defaultLegend().group(-3); 
	theme->defaultLegend().theme(theme->id()); 
	status = insertLegend (&(theme->defaultLegend())); 
	if (!status)
		return status;

	theme->pointingLegend().group(-4); 
	theme->pointingLegend().theme(theme->id()); 
	status = insertLegend (&(theme->pointingLegend())); 
	if (!status)
		return status;

	theme->queryLegend().group(-5); 
	theme->queryLegend().theme(theme->id()); 
	status = insertLegend (&(theme->queryLegend())); 
	if (!status)
		return status;

	theme->queryAndPointingLegend().group(-6); 
	theme->queryAndPointingLegend().theme(theme->id()); 
	status = insertLegend (&(theme->queryAndPointingLegend())); 
	if (!status)
		return status;

	for (int i = 0; i < numSlices; i++)
	{
		theme->legend()[i].group(i);
		theme->legend()[i].theme(theme->id());
		status = insertLegend (&(theme->legend()[i]));
		if (!status)
			return status;
	}
	if (!status)
		return status;

	//insert metadata theme
	if(!theme->saveMetadata(this))
		return false;

	themeMap()[theme->id()] = theme;

	if(theme->type()==TeTHEME && !updateThemeTable(static_cast<TeTheme*>(theme)))
		return false;

	return true;
}

bool TePostgreSQL::insertThemeTable(int themeId, int tableId, int relationId, int tableOrder)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO te_theme_table (theme_id, table_id, ";

	if(relationId > 0)
		sql += "relation_id, ";

	sql += "table_order) VALUES(";
	sql += Te2String(themeId);
	sql += ", ";
	sql += Te2String(tableId);
	sql += ", ";

	if(relationId > 0)
	{
		sql += Te2String(relationId);
		sql += ", ";
	}

	sql += Te2String(tableOrder);
	sql += ")";

	if(this->execute(sql))
	{
		//sql = "SELECT currval('te_theme_table_theme_table_id_seq')";
		//TePGRecordset rec;
		//rec.open(sql, tepg_connection_);
		//if(rec.recordCount() > 0)
		//{
		//	tree->id(atoi(rec.value(0)));
		//}
		//rec.close();
	}
	else
		return false;
		
	return true;	
}

bool TePostgreSQL::generateLabelPositions(TeTheme *theme, const std::string& objectId)
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if(theme->layer()->hasGeometry(TeCELLS)    || 
	   theme->layer()->hasGeometry(TePOLYGONS) ||
	   theme->layer()->hasGeometry(TeLINES))
	{
		geomTable = theme->layer()->tableName(TeCELLS);
		
		if(geomTable.empty())
		{
			geomTable = theme->layer()->tableName(TePOLYGONS);
			if(geomTable.empty())
			{
				geomTable = theme->layer()->tableName(TeLINES);

				if(geomTable.empty())
					return false;
			}
		}
		
		string lower_x =  "(((" + geomTable + ".spatial_box[1])[0]))";
		string lower_y =  "(((" + geomTable + ".spatial_box[1])[1]))";
		string upper_x =  "(((" + geomTable + ".spatial_box[0])[0]))";
		string upper_y =  "(((" + geomTable + ".spatial_box[0])[1]))";
		
		upd  = " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(" + lower_x + " + (" + upper_x;
		upd += " - " + lower_x + ") / 2.0) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(" + lower_y + " + (" + upper_y;
		upd += " - " + lower_y + ") / 2.0) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";

		upd += " WHERE (label_x IS NULL) OR (label_y IS NULL)";
	}	
	else if(theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);
	
		upd  = " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(x)";
		upd += " FROM " + geomTable + " WHERE object_id = c_object_id), ";


		upd += " label_y = (SELECT MAX(y)";
		upd += " FROM " + geomTable + " WHERE object_id = c_object_id) ";

		upd += " WHERE (label_x IS NULL) OR (label_y IS NULL)";
	}

	if (!upd.empty())
	{
		if (!objectId.empty())
		{
			upd += " AND c_object_id='"+objectId+"'";
		}
		if(!execute(upd))
			return false;
	}

	return true;
}

bool TePostgreSQL::insertLegend(TeLegendEntry *legend)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO te_legend (theme_id, group_id, num_objs, lower_value, upper_value, ";
		   sql += "label) VALUES(";
		   sql += Te2String(legend->theme());
		   sql += ", ";
		   sql += Te2String(legend->group());
		   sql += ", ";
	   	   sql += Te2String(legend->count());
		   sql += ", '";
		   sql += escapeSequence(legend->from());
		   sql += "', '";
		   sql += escapeSequence(legend->to());
		   sql += "', '";
		   sql += escapeSequence(legend->label());
		   sql += "')";

	if(this->execute(sql))
	{
		sql = "SELECT currval('te_legend_legend_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
		{
			legend->id(rec.getInt(0));
		}
		rec.close();
	}
	else
		return false;	

	legendMap()[legend->id()] = legend;

	return insertVisual(legend);
}

bool TePostgreSQL::loadPolygonSet(const string& table, TeBox& box, TePolygonSet& ps)
{
	TeDatabasePortal *portal = this->getPortal();

	if(!portal)
		return false;

	std::string q = "SELECT * FROM " + table + " WHERE ";
	if(box.isValid())
	{
		q += this->getSQLBoxWhere(box, TePOLYGONS, table);
	}
	q += " ORDER BY parent_id, num_holes DESC, ext_max ASC";

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}
	
	bool flag = true;

	do
	{
		TePolygon poly;
		flag = portal->fetchGeometry(poly);
		ps.add(poly);
	}
	while (flag);

	delete portal;

	return true;
}

TeDatabasePortal* TePostgreSQL::loadPolygonSet(const string& table, TeBox& box)
{
	TeDatabasePortal *portal = this->getPortal();
	
	if(!portal)
		return 0;

	string q = "SELECT * FROM " + table + " WHERE ";
	       q += this->getSQLBoxWhere(box, TePOLYGONS, table);
	       q += " ORDER BY parent_id, num_holes DESC, ext_max ASC";

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return 0;
	}
	else 
		return portal;
}

bool TePostgreSQL::locatePolygon(const string& table, TeCoord2D& pt, TePolygon& polygon, const double& tol)
{
	TeDatabasePortal *portal = this->getPortal();

	if (!portal)
		return false;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);


	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += getSQLBoxWhere(box, TePOLYGONS, table);
		   sql += " ORDER BY parent_id, num_holes DESC, ext_max ASC";
		   
	
	if(!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}

	bool flag = true;

	do
	{
		TePolygon poly;

		flag = portal->fetchGeometry(poly);

		if(TeWithin(TePoint(pt), poly))
		{
			polygon = poly;
			delete portal;
			return true;
		}
	}
	while(flag);
	
	delete portal;
	
	return false;
}

bool TePostgreSQL::locatePolygonSet(const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons)
{
	TeDatabasePortal *portal = this->getPortal();
	
	if (!portal)
		return false;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);


	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += getSQLBoxWhere(box, TePOLYGONS, table);
		   sql += " ORDER BY object_id, parent_id, num_holes DESC, ext_max ASC";

	if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}
	
	bool flag = true;
	
	polygons.clear();
	
	do
	{
		TePolygon poly;

		flag = portal->fetchGeometry(poly);

		if(TeWithin(TePoint(pt), poly))
			polygons.add(poly);
	}
	while (flag);

	delete portal;

	if(polygons.size())
		return (true);

	return false;
}

bool TePostgreSQL::insertPolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	int parentId = 0;

	for(unsigned int k = 0u; k < p.size(); ++k)
	{
		TeLinearRing& ring = p[k];

		TeBox b = ring.box();

		unsigned int ringSize = ring.size();

		double extMax = MAX(b.width(), b.height());

		unsigned int numberOfHoles = 0u;
		
		if(k == 0u)
			numberOfHoles = p.size() - 1u;

		unsigned int sizeBinaryRing = 0;
		unsigned int sizeBinaryBox = 0;
		char* binaryRing = TeLine2DToPGBinary(ring, sizeBinaryRing);
		char* binaryBox = TeBoxToPGBinary(b, sizeBinaryBox);

		int nParams = 7;
		Oid paramTypes[7];
		const char *paramValues[7];
		int paramLengths[7];
		int paramFormats[7];
		
		//tipos dos parametros
		paramTypes[0] = PG_VARCHAR_TYPE;
		paramTypes[1] = PG_INT4_TYPE;
		paramTypes[2] = PG_INT4_TYPE;
		paramTypes[3] = PG_INT4_TYPE;
		paramTypes[4] = PG_BOX_TYPE;
		paramTypes[5] = PG_FLOAT8_TYPE;
		paramTypes[6] = PG_POLYGON_TYPE;

		string strOid = escapeSequence(p.objectId());
		paramValues[0] = strOid.c_str();

		string strSize = Te2String(ringSize);
		paramValues[1] = strSize.c_str();		

		string strHoles = Te2String(numberOfHoles);
		paramValues[2] = strHoles.c_str();

		string strParent = Te2String(parentId);
		paramValues[3] = strParent.c_str();		

		paramValues[4] = binaryBox;

		char* chrExtMax = new char[sizeof(double)];
		double swappedDouble = TeConvertToBigEndian(extMax);
		memcpy(chrExtMax, &swappedDouble, sizeof(double));
		paramValues[5] = chrExtMax;

		paramValues[6] = binaryRing;

		paramLengths[0] = 0;
		paramLengths[1] = 0;
		paramLengths[2] = 0;
		paramLengths[3] = 0;
		paramLengths[4] = sizeBinaryBox;
		paramLengths[5] = sizeof(double);
		paramLengths[6] = sizeBinaryRing;
		
		paramFormats[0] = 0;
		paramFormats[1] = 0;
		paramFormats[2] = 0;
		paramFormats[3] = 0;
		paramFormats[4] = 1;
		paramFormats[5] = 1;
		paramFormats[6] = 1;
		

		string command = "INSERT INTO ";
				command += table;
				command += " (object_id, num_coords, num_holes, parent_id, spatial_box, ext_max, spatial_data)"; 
				command += " VALUES ($1, $2, $3, $4,$5, $6, $7)";
		
		int result = tepg_connection_->exec_cmd_params(command.c_str(), 
                       nParams,
                       paramTypes,
                       paramValues,
                       paramLengths,
                       paramFormats,
                       0);

		delete [] binaryRing;
		delete [] binaryBox;
		delete [] chrExtMax;

		if(result == 1)
		{
			string sql = "SELECT currval('" + table + "_geom_id_seq')";
		
			TePGRecordset rec;
			rec.open(sql, tepg_connection_);

			if(rec.recordCount() > 0)
				ring.geomId(rec.getInt(0));

			if(k == 0u)
			{
				parentId = ring.geomId();

				p.geomId(ring.geomId());

				string newSQL  = "UPDATE " + table + " SET parent_id = ";
				       newSQL += Te2String(parentId);
					   newSQL += " WHERE geom_id = ";
					   newSQL += Te2String(parentId);

				if(!this->execute(newSQL))
					return false;
			}

			rec.close();
		}
		else
			return false;
	}

	return true;
}

bool TePostgreSQL::updatePolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	int parentId = 0;

	for(unsigned int k = 0u; k < p.size(); ++k)
	{
		TeLinearRing& ring = p[k];

		TeBox b = ring.box();

		unsigned int ringSize = ring.size();

		double extMax = MAX(b.width(), b.height());

		unsigned int numberOfHoles = 0u;
		
		if(k == 0u)
		{
			parentId = ring.geomId();

			numberOfHoles = p.size() - 1u;
		}

		unsigned int sizeBinaryRing = 0;
		unsigned int sizeBinaryBox = 0;
		char* binaryRing = TeLine2DToPGBinary(ring, sizeBinaryRing);
		char* binaryBox = TeBoxToPGBinary(b, sizeBinaryBox);

		int nParams = 7;
		Oid paramTypes[7];
		const char *paramValues[7];
		int paramLengths[7];
		int paramFormats[7];
		
		//tipos dos parametros
		paramTypes[0] = PG_VARCHAR_TYPE;
		paramTypes[1] = PG_INT4_TYPE;
		paramTypes[2] = PG_INT4_TYPE;
		paramTypes[3] = PG_INT4_TYPE;
		paramTypes[4] = PG_BOX_TYPE;
		paramTypes[5] = PG_FLOAT8_TYPE;
		paramTypes[6] = PG_POLYGON_TYPE;

		string strOid = escapeSequence(p.objectId());
		paramValues[0] = strOid.c_str();

		string strSize = Te2String(ringSize);
		paramValues[1] = strSize.c_str();		

		string strHoles = Te2String(numberOfHoles);
		paramValues[2] = strHoles.c_str();

		string strParent = Te2String(parentId);
		paramValues[3] = strParent.c_str();		

		paramValues[4] = binaryBox;

		char* chrExtMax = new char[sizeof(double)];
		double swappedDouble = TeConvertToBigEndian(extMax);
		memcpy(chrExtMax, &swappedDouble, sizeof(double));
		paramValues[5] = chrExtMax;

		paramValues[6] = binaryRing;

		paramLengths[0] = 0;
		paramLengths[1] = 0;
		paramLengths[2] = 0;
		paramLengths[3] = 0;
		paramLengths[4] = sizeBinaryBox;
		paramLengths[5] = sizeof(double);
		paramLengths[6] = sizeBinaryRing;
		
		paramFormats[0] = 0;
		paramFormats[1] = 0;
		paramFormats[2] = 0;
		paramFormats[3] = 0;
		paramFormats[4] = 1;
		paramFormats[5] = 1;
		paramFormats[6] = 1;

		string command  = "UPDATE " + table + " SET ";
		command += "object_id = $1";
		command += ", num_coords = $2";
		command += ", num_holes = $3";
		command += ", parent_id = $4";
		command += ", spatial_box = $5";
		command += ", ext_max = $6";
		command += ", spatial_data = $7";
		command += " WHERE geom_id = ";
		command += Te2String(ring.geomId());
		
		int result = tepg_connection_->exec_cmd_params(command.c_str(), 
                       nParams,
                       paramTypes,
                       paramValues,
                       paramLengths,
                       paramFormats,
                       0);

		delete [] binaryRing;
		delete [] binaryBox;
		delete [] chrExtMax;

		if(result != 1)
		{
			return false;
		}
	}

	return true;
}

bool TePostgreSQL::loadLineSet(const string& table, TeBox& box, TeLineSet& linSet)
{
	TeDatabasePortal *portal = this->getPortal();
	
	if(!portal)
		return false;

	string q  = "SELECT * FROM " + table + " WHERE ";
	       q += this->getSQLBoxWhere (box, TeLINES, table);

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}

	bool flag = true;

	do
	{
		TeLine2D lin;
		flag = portal->fetchGeometry(lin);
		linSet.add(lin);
	}
	while (flag);

	delete portal;

	return true;
}

TeDatabasePortal* TePostgreSQL::loadLineSet(const string& table, TeBox& box)
{
	TeDatabasePortal *portal = this->getPortal();

	if(!portal)
		return 0;

	string q  = "SELECT * FROM " + table + " WHERE ";
	       q += this->getSQLBoxWhere (box, TeLINES, table);

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;

		return 0;
	}

	return portal;
}

bool TePostgreSQL::insertLine(const string& table, TeLine2D& l)
{
	errorMessage_ = "";

	double extMax = MAX(l.box().width(), l.box().height());

	unsigned int sizeBinaryRing = 0;
	unsigned int sizeBinaryBox = 0;
	char* binaryRing = TeLine2DToPGBinary(l, sizeBinaryRing);
	char* binaryBox = TeBoxToPGBinary(l.box(), sizeBinaryBox);

	int nParams = 5;
	Oid paramTypes[5];
	const char *paramValues[5];
	int paramLengths[5];
	int paramFormats[5];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_INT4_TYPE;
	paramTypes[2] = PG_BOX_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;
	paramTypes[4] = PG_POLYGON_TYPE;

	string strOid = escapeSequence(l.objectId());
	paramValues[0] = strOid.c_str();

	string strSize = Te2String(l.size());
	paramValues[1] = strSize.c_str();		

	paramValues[2] = binaryBox;

	char* chrExtMax = new char[sizeof(double)];
	double swappedDouble = TeConvertToBigEndian(extMax);
	memcpy(chrExtMax, &swappedDouble, sizeof(double));
	paramValues[3] = chrExtMax;
	
	paramValues[4] = binaryRing;

	paramLengths[0] = 0;
	paramLengths[1] = 0;
	paramLengths[2] = sizeBinaryBox;
	paramLengths[3] = sizeof(double);
	paramLengths[4] = sizeBinaryRing;
	
	paramFormats[0] = 0;
	paramFormats[1] = 0;
	paramFormats[2] = 1;
	paramFormats[3] = 1;
	paramFormats[4] = 1;
	

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, num_coords, spatial_box, ext_max, spatial_data)";
			command += " VALUES ($1, $2, $3, $4, $5)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryRing;
	delete [] binaryBox;
	delete [] chrExtMax;

	if(result == 1)
	{
		string sql = "SELECT currval('" + table + "_geom_id_seq')";

		TePGRecordset rec;
		rec.open(sql, tepg_connection_);

		if(rec.recordCount() > 0)
			l.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;

	return true;
}

bool TePostgreSQL::updateLine(const string& table, TeLine2D& l)
{
	errorMessage_ = "";

	double extMax = MAX(l.box().width(), l.box().height());

	unsigned int sizeBinaryRing = 0;
	unsigned int sizeBinaryBox = 0;
	char* binaryRing = TeLine2DToPGBinary(l, sizeBinaryRing);
	char* binaryBox = TeBoxToPGBinary(l.box(), sizeBinaryBox);

	int nParams = 5;
	Oid paramTypes[5];
	const char *paramValues[5];
	int paramLengths[5];
	int paramFormats[5];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_INT4_TYPE;
	paramTypes[2] = PG_BOX_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;
	paramTypes[4] = PG_POLYGON_TYPE;

	string strOid = escapeSequence(l.objectId());
	paramValues[0] = strOid.c_str();

	string strSize = Te2String(l.size());
	paramValues[1] = strSize.c_str();		

	paramValues[2] = binaryBox;

	char* chrExtMax = new char[sizeof(double)];
	double swappedDouble = TeConvertToBigEndian(extMax);
	memcpy(chrExtMax, &swappedDouble, sizeof(double));
	paramValues[3] = chrExtMax;
	
	paramValues[4] = binaryRing;

	paramLengths[0] = 0;
	paramLengths[1] = 0;
	paramLengths[2] = sizeBinaryBox;
	paramLengths[3] = sizeof(double);
	paramLengths[4] = sizeBinaryRing;
	
	paramFormats[0] = 0;
	paramFormats[1] = 0;
	paramFormats[2] = 1;
	paramFormats[3] = 1;
	paramFormats[4] = 1;

	string command = "UPDATE " + table + " SET";
		command += " object_id = $1";
		command += ", num_coords = $2";
		command += ", spatial_box = $3";
		command += ", ext_max = $4";
		command += ", spatial_data = $5";
		command += " WHERE geom_id = ";
		command += Te2String(l.geomId());
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(), 
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryRing;
	delete [] binaryBox;
	delete [] chrExtMax;

	return (result == 1);
}

bool TePostgreSQL::locateLine(const string& table, TeCoord2D &pt, TeLine2D &line, const double& tol)
{
	TeDatabasePortal* portal = this->getPortal();

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);

	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += this->getSQLBoxWhere(box, TeLINES, table);

	
	if(!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	// Get all lines
	TeLineSet ls;
	int k;
	bool flag = true;
	do 
	{
		TeLine2D l;
		flag = portal->fetchGeometry( l );
		ls.add ( l );
	} while(flag);

	delete portal;

	TeCoord2D paux;

	if(TeNearest(pt, ls, k, paux, tol))
	{
		line = ls[k];
		return true;
	}

	return false;
}

bool TePostgreSQL::insertPoint(const string& table, TePoint &p)
{
	errorMessage_ = "";

	unsigned int sizeBinaryBox = 0;
	char* binaryBox = TeBoxToPGBinary(p.box(), sizeBinaryBox);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_BOX_TYPE;
	paramTypes[2] = PG_FLOAT8_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;

	char* chrX = new char[sizeof(double)];
	double swappedDoubleX = TeConvertToBigEndian(p.location().x_);
	memcpy(chrX, &swappedDoubleX, sizeof(double));

	char* chrY = new char[sizeof(double)];
	double swappedDoubleY = TeConvertToBigEndian(p.location().y_);
	memcpy(chrY, &swappedDoubleY, sizeof(double));

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();

	paramValues[1] = binaryBox;

	paramValues[2] = chrX;

	paramValues[3] = chrY;
	
	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryBox;
	paramLengths[2] = sizeof(double);
	paramLengths[3] = sizeof(double);
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 1;
	paramFormats[3] = 1;
	
	
	string command  = "INSERT INTO " + table + " (object_id, spatial_box, x, y)";
	command += " VALUES($1, $2, $3, $4)";
		
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryBox;
	delete [] chrX;
	delete [] chrY;

	if(result == 1)
	{
		string sql = "SELECT currval('" + table + "_geom_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			p.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;
	
	return true;
}

bool TePostgreSQL::updatePoint(const string& table, TePoint &p)
{
	unsigned int sizeBinaryBox = 0;
	char* binaryBox = TeBoxToPGBinary(p.box(), sizeBinaryBox);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos par�metros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_BOX_TYPE;
	paramTypes[2] = PG_FLOAT8_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;

	char* chrX = new char[sizeof(double)];
	double swappedDoubleX = TeConvertToBigEndian(p.location().x_);
	memcpy(chrX, &swappedDoubleX, sizeof(double));

	char* chrY = new char[sizeof(double)];
	double swappedDoubleY = TeConvertToBigEndian(p.location().y_);
	memcpy(chrY, &swappedDoubleY, sizeof(double));

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();

	paramValues[1] = binaryBox;

	paramValues[2] = chrX;

	paramValues[3] = chrY;
	
	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryBox;
	paramLengths[2] = sizeof(double);
	paramLengths[3] = sizeof(double);
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 1;
	paramFormats[3] = 1;

	string command  = "UPDATE " + table + " SET ";
	       command += " object_id = $1";
	       command += ", spatial_box = $2";
           command += ", x = $3";
           command += ", y = $4";
           command += " WHERE geom_id = " + Te2String(p.geomId());
		
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryBox;
	delete [] chrX;
	delete [] chrY;

	return (result == 1);
}

bool TePostgreSQL::locatePoint(const string& table, TeCoord2D& pt, TePoint& point, const double& tol)
{
	TeDatabasePortal* portal = this->getPortal();

	TeBox bbox(pt.x() - tol, pt.y() - tol, pt.x() + tol, pt.y() + tol);

	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += this->getSQLBoxWhere(bbox, TePOINTS, table);

	if(!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	TePointSet ps;
	
	bool flag = true;

	do 
	{
		TePoint point;
		flag = portal->fetchGeometry(point);
		ps.add(point);
	}while(flag);

	delete portal;
	int k;
	if(TeNearest(pt, ps, k, tol))
	{
		point = ps[k];
		return true;
	}
	return false;
}

bool TePostgreSQL::insertText(const string& table, TeText& t)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO " + table + " (object_id, x, y, text_value, angle, height, alignment_vert, alignment_horiz) VALUES('";
		   sql += escapeSequence(t.objectId());
		   sql += "', ";
		   sql += Te2String(t.location().x(), 15);
		   sql += ", ";
		   sql += Te2String(t.location().y(), 15);
		   sql += ", '";
		   sql += escapeSequence(t.textValue());
		   sql += "', ";
		   sql += Te2String(t.angle(), 15);
		   sql += ", ";
		   sql += Te2String(t.height(), 15);
		   sql += ", ";
		   sql += Te2String(t.alignmentVert(), 15);
		   sql += ", ";
		   sql += Te2String(t.alignmentHoriz(), 15);
		   sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('" + table + "_geom_id_seq')";

		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			t.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;

	return true;
}

bool TePostgreSQL::insertArc(const string& table, TeArc& arc)
{
	errorMessage_ = "";

	string sql  = "INSERT INTO " + table + " (object_id, from_node, to_node) VALUES('";
		   sql += escapeSequence(arc.objectId());
		   sql += "', ";
		   sql += Te2String(arc.fromNode().geomId());
	       sql += ", ";
		   sql += Te2String(arc.toNode().geomId());
		   sql += ")";

	if(this->execute(sql))
	{
		sql = "SELECT currval('" + table + "_geom_id_seq')";
		TePGRecordset rec;
		rec.open(sql, tepg_connection_);
		if(rec.recordCount() > 0)
			arc.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;
	

	return true;
}

bool TePostgreSQL::insertNode(const string& table, TeNode& node)
{
	errorMessage_ = "";

	unsigned int sizeBinaryBox = 0;
	char* binaryBox = TeBoxToPGBinary(node.box(), sizeBinaryBox);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos par�metros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_BOX_TYPE;
	paramTypes[2] = PG_FLOAT8_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;

	char* chrX = new char[sizeof(double)];
	double swappedDoubleX = TeConvertToBigEndian(node.location().x_);
	memcpy(chrX, &swappedDoubleX, sizeof(double));

	char* chrY = new char[sizeof(double)];
	double swappedDoubleY = TeConvertToBigEndian(node.location().y_);
	memcpy(chrY, &swappedDoubleY, sizeof(double));

	string strOid = escapeSequence(node.objectId());
	paramValues[0] = strOid.c_str();

	paramValues[1] = binaryBox;

	paramValues[2] = chrX;

	paramValues[3] = chrY;
	
	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryBox;
	paramLengths[2] = sizeof(double);
	paramLengths[3] = sizeof(double);
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 1;
	paramFormats[3] = 1;
	
	
	string command  = "INSERT INTO " + table + " (object_id, spatial_box, x, y)";
	command += " VALUES($1, $2, $3, $4)";
		
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryBox;
	delete [] chrX;
	delete [] chrY;

	if(result != 1)
		return false;

	string sql = "SELECT currval('" + table + "_geom_id_seq')";
	TePGRecordset rec;
	rec.open(sql, tepg_connection_);
	if(rec.recordCount() > 0)
		node.geomId(rec.getInt(0));

	rec.close();

	return true;
}

bool TePostgreSQL::updateNode(const string& table, TeNode &node)
{
	unsigned int sizeBinaryBox = 0;
	char* binaryBox = TeBoxToPGBinary(node.box(), sizeBinaryBox);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos par�metros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_BOX_TYPE;
	paramTypes[2] = PG_FLOAT8_TYPE;
	paramTypes[3] = PG_FLOAT8_TYPE;

	char* chrX = new char[sizeof(double)];
	double swappedDoubleX = TeConvertToBigEndian(node.location().x_);
	memcpy(chrX, &swappedDoubleX, sizeof(double));

	char* chrY = new char[sizeof(double)];
	double swappedDoubleY = TeConvertToBigEndian(node.location().y_);
	memcpy(chrY, &swappedDoubleY, sizeof(double));

	string strOid = escapeSequence(node.objectId());
	paramValues[0] = strOid.c_str();

	paramValues[1] = binaryBox;

	paramValues[2] = chrX;

	paramValues[3] = chrY;
	
	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryBox;
	paramLengths[2] = sizeof(double);
	paramLengths[3] = sizeof(double);
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 1;
	paramFormats[3] = 1;
	
	
	string command = "UPDATE " + table + " SET ";
    command += "  object_id = $1";
    command += ", spatial_box = $2";
	command += ", x = $3";
	command += ", y = $4";
    command += " WHERE geom_id = " + Te2String(node.geomId());
		
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryBox;
	delete [] chrX;
	delete [] chrY;

	return (result == 1);

}

bool TePostgreSQL::insertCell(const string& table, TeCell &c)
{
	errorMessage_ = "";

	unsigned int sizeBinaryBox = 0;
	char* binaryBox = TeBoxToPGBinary(c.box(), sizeBinaryBox);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos par�metros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = PG_BOX_TYPE;
	paramTypes[2] = PG_INT4_TYPE;
	paramTypes[3] = PG_INT4_TYPE;

	string strOid = escapeSequence(c.objectId());
	paramValues[0] = strOid.c_str();

	paramValues[1] = binaryBox;

	string strCol = Te2String(c.column());
	paramValues[2] = strCol.c_str();		

	string strRow = Te2String(c.line());
	paramValues[3] = strRow.c_str();		

	
	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryBox;
	paramLengths[2] = 0;
	paramLengths[3] = 0;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 0;
	paramFormats[3] = 0;
	
	
	string command  = "INSERT INTO " + table + " (object_id, spatial_box, col_number, row_number)";
	command += " VALUES($1, $2, $3, $4)";
		
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryBox;

	if(result != 1)
		return false;

	string sql = "SELECT currval('" + table + "_geom_id_seq')";
	TePGRecordset rec;
	rec.open(sql, tepg_connection_);
	if(rec.recordCount() > 0)
		c.geomId(rec.getInt(0));
	
	rec.close();	

	return true;
}

bool TePostgreSQL::updateCell(const string& table, TeCell &c)
{
	TeBox b = c.box();

	string sql;
	sql = "UPDATE " + table + " SET ";
	sql += "spatial_box = '" + PGBoxRtree_encode(b) + "', ";
	sql += "col_number=" + Te2String(c.column()) + ", ";
	sql += "row_number=" + Te2String(c.line());
	sql += " WHERE geom_id = " + c.geomId();
	
	return (this->execute(sql));
}

bool TePostgreSQL::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char* buf, unsigned long size, int band, unsigned int res, unsigned int subband)
{
	errorMessage_ = "";

	if (blockId.empty()) 
	{
		errorMessage_ = "No block identifier provided!";
		return false;
	}

	TePostgreSQLPortal* portal = static_cast<TePostgreSQLPortal*>(this->getPortal());
	if(!portal)
		return false;

	bool update = false;
	string q ="SELECT * FROM " + table; 
	q += " WHERE block_id='" + blockId + "'";

	if (!portal->query(q))
	{
		delete portal;
		return false;
	}

	// check if this block is alread in the database
	if(portal->fetchRow())
		update = true;

	delete portal;


	string sql = "";
		
	size_t newLen = 0;

	unsigned char* newbuf = TePGConnection::escapeBytea(buf, size, &newLen);

	if(!update)
		getInsertRasterBlock(table, blockId, ll, ur, band, res, subband, (char*)newbuf, size, sql);
	else
		getUpdateRasterBlock(table, blockId, ll, ur, band, res, subband, (char*)newbuf, size, sql);


	if(!execute(sql))
	{
		TePGConnection::freeMem(newbuf);
		newbuf = NULL;
		errorMessage_ = "Couldn't insert/update a rasterblock!";
		return false;
	}

	TePGConnection::freeMem(newbuf);
	newbuf = NULL;

	return true;
}

string TePostgreSQL::getSQLStatistics(TeGroupingAttr& attrs)
{
	string sql = "";
	string virg = "";

	TeGroupingAttr::iterator it = attrs.begin();
	int count = 0;
	while(it != attrs.end())
	{
		if(count > 0)
			virg = ",";

		string strAux = (*it).first.name_;
		size_t pos = strAux.find(".");
		if(pos != string::npos)
			strAux[pos] = '_';

		switch((*it).second)
		{
			case TeSUM:
				sql += virg +" SUM( "+ (*it).first.name_ +") AS SUM_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMAXVALUE:
				sql += virg +" MAX( "+ (*it).first.name_ +") AS MAX_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMINVALUE:
				sql += virg +" MIN( "+ (*it).first.name_ +") AS MIN_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeCOUNT:
				sql += virg +" COUNT( "+ (*it).first.name_ +") AS COUNT_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMEAN:
				sql += virg +" AVG( "+ (*it).first.name_ +") AS AVG_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeSTANDARDDEVIATION:
				sql += virg +" STDDEV( "+ (*it).first.name_ +") AS STDDEV_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeVARIANCE:
				sql += virg +" VARIANCE( "+ (*it).first.name_ +") AS VAR_"+ strAux;
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			default:
				break;
		}
		++it;

	}

	return sql;
}

string TePostgreSQL::getSQLTemporalWhere(TeTimeInterval& timeInterval, TeTemporalRelation timeOperator, const string& initialTime, const string& finalTime)
{
	string sql;
	
	string t1 = timeInterval.getInitialDateTime("YYYYsMMsDDsHHsmmsSS");
	string t2 = timeInterval.getFinalDateTime ("YYYYsMMsDDsHHsmmsSS");
	
	switch(timeOperator)
	{
		case TeTIMEBEFORE:		sql = finalTime +" < '"+ t1 +"'";
								break;

		case TeTIMEAFTER:		sql = initialTime +" > '"+ t2 +"'";
								break;

		case TeTIMEEQUAL:       sql = "("+ initialTime +" >= '"+ t1 +"'";
								sql += " AND "+ initialTime +" <= '"+ t2 +"')";
								sql += " AND ";
								sql += "("+ finalTime +" >= '"+ t1 +"'";
								sql += " AND "+ finalTime +" <= '"+ t2 +"')";
								break;

		case TeTIMEMEETS:       sql = finalTime +" = '"+ t1 +"'";
								sql += " OR "+ initialTime +" = '"+ t2 +"'";
								break;

		case TeTIMEDURING:		sql = initialTime +" > '"+ t1 +"'";
								sql += " AND "+ initialTime +" < '"+ t2 +"'";
								sql += " AND "+ finalTime +" > '"+ t1 +"'";
								sql += " AND "+ finalTime +" < '"+ t2 +"'";
								break;

		case TeTIMEOVERLAPS:    sql = "("+ initialTime +" < '"+ t1 +"'";
								sql += " AND "+ finalTime +" > '"+ t1 +"'";
								sql += " AND "+ finalTime +" < '"+ t2 +"')";
								sql += " OR ";
								sql += "("+ initialTime + " > '"+ t1 +"'";
								sql += " AND "+ initialTime +" < '"+ t2 +"'";
								sql += " AND "+ finalTime +" > '"+ t2 +"')";
								break;

		case TeTIMEENDS:		sql = finalTime +" = '"+ t2 +"'";
								break;

		case TeTIMESTARTS:		sql = initialTime +" = '"+ t1 +"'";
								break;
		case TeTIMEUNDEFINED:	
				return sql;				
	}

	return sql; 
}

string TePostgreSQL::getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, const string& initialTime, const string& finalTime)
{
	string func = " EXTRACT(";
	string sql;

	switch(chr)
	{
		case TeSECOND:		func = "second FROM "; 
							break;
		
		case TeMINUTE:		func = "minute FROM ";
							break;
		
		case TeHOUR:        func = "hour FROM ";
							break;
		
		case TeDAY:         func = "day FROM ";
							break;

		case TeMONTH:		func = "month FROM ";
							break;

		case TeYEAR:        func = "year FROM ";	
							break;
		default:			return "";
	}

	switch(rel)
	{
		case TeTIMEBEFORE:		sql = func + finalTime + ") < " + Te2String(time1);
								break;
		
		case TeTIMEAFTER:		sql = func + initialTime + ") > " + Te2String(time2);
								break;
		
		case TeTIMEEQUAL:		sql  = func + initialTime + ") = " + Te2String(time1);
								sql += " AND "+ func + finalTime +") = " + Te2String(time2);
								break;

		case TeTIMEMEETS:		sql  = func + finalTime + ") = " + Te2String(time1);
								sql += " OR "+ func + initialTime +") = " + Te2String(time2);
								break;

		case TeTIMEDURING:		sql  = func + initialTime + ") >= " + Te2String(time1);
								sql += " AND " + func + initialTime + ") <= " + Te2String(time2);
								sql += " AND " + func + finalTime + ") >= " + Te2String(time1);
								sql += " AND " + func + finalTime + ") <= " + Te2String(time2);
								break;

		case TeTIMEOVERLAPS:    sql  =  "("+ func + initialTime + ") <= " + Te2String(time1);
								sql += " AND " + func + finalTime + ") >= " + Te2String(time1);
								sql += " AND " + func + finalTime + ") <= " + Te2String(time2) + ")";
								sql += " OR ";
								sql += "(" + func + initialTime + ") >= " + Te2String(time1);
								sql += " AND " + func + initialTime + ") <= " + Te2String(time2);
								sql += " AND " + func + finalTime + ") >= " + Te2String(time2) +")";
								break;

		case TeTIMEENDS:		sql = func + finalTime + ") = " + Te2String(time2);
								break;

		case TeTIMESTARTS:		sql = func + initialTime + ") = " + Te2String(time1);
								break;
		case TeTIMEUNDEFINED:	
				return sql;				
	}
	
	return sql;
}

string TePostgreSQL::getAutoNumberSQL(const string& table)
{
	errorMessage_ = "";

	TePGRecordset rec;

	string sql = "SELECT adsrc FROM pg_class, pg_attrdef  WHERE lower(pg_class.relname) = lower('" + table + "') AND pg_attrdef.adnum = 1 AND pg_class.oid = pg_attrdef.adrelid";

	if(!rec.open(sql, tepg_connection_))
		return "";

	if(rec.recordCount() > 0)
	{
		return rec.getData(0);
	}
	else
		return "";
}

string TePostgreSQL::getSQLTime(const TeTime& time) const
{
	return "'" + time.getDateTime("YYYYsMMsDDsHHsmmsSS", "-") + "'";
}

string TePostgreSQL::concatValues(vector<string>& values, const string& unionString)
{
	string concat = "";
	
	for(unsigned int i = 0; i < values.size(); ++i)
	{
		if(i != 0)
		{
			concat += " || ";

			if(!unionString.empty())
			{
				concat += "'";
				concat += unionString;
				concat += "'";
				concat += " || ";
			}
		}

		concat += values[i];
	}

	return concat;
}

string TePostgreSQL::toUpper(const string& value)
{
	string result  = "upper(";
	       result += value;
		   result += ")";

	return result;
}

string TePostgreSQL::leftString(const string& name, const int& length)
{
    string substringClause = "SUBSTRING(" + name+ ", 1, " + Te2String(length) + ")";
    return substringClause;
}

bool TePostgreSQL::locateCell(const string& table, TeCoord2D& pt, TeCell& c, const double& tol)
{
	TeDatabasePortal* portal = this->getPortal();

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);


	string sql  ="SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += this->getSQLBoxWhere(box, TeCELLS, table);

	if(!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	portal->fetchGeometry(c);

	delete portal;

	return true;
}

bool TePostgreSQL::createSpatialIndex(const string& table, const string& column, TeSpatialIndexType /* type */, short /* level */, short /* tile */)
{
	string create = "CREATE INDEX sp_idx_gist_" + table + " ON " + table + " USING GIST (" + column + " " + gistBoxOps_ + ")";

	return this->execute(create);
}

string TePostgreSQL::getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName)
{
	if(rep == TeTEXT)
		return TeDatabase::getSQLBoxWhere(box, rep, tableName);

	string colname = "spatial_box";

	if(rep & TeRASTER)
		colname = "block_box";
	

	string urx = Te2String(box.x2(), 15);
	string llx = Te2String(box.x1(), 15);
	string ury = Te2String(box.y2(), 15);
	string lly = Te2String(box.y1(), 15);

	string wherebox  = " ( " + colname + " && '(" + urx + ", " + ury + ", " + llx + ", " + lly + ")'::box)";

	return wherebox;
}

string TePostgreSQL::getSQLBoxWhere(const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1)
{
	if((rep1 == TeTEXT) || (rep2 == TeTEXT))
	{
		return TeDatabase::getSQLBoxWhere(table1, table2, rep2, rep1);
	}
	else
	{
		string col1name = "spatial_box";
		string col2name = "spatial_box";;

		if(rep1 & TeRASTER)
			col1name = "block_box";

		if(rep2 & TeRASTER)
			col2name = "block_box";

		string wherebox  = "(";
		       wherebox += table1;
               wherebox += "." + col1name + " && ";
               wherebox += table2;
               wherebox += "." + col2name + ")";

	   return wherebox;
	}
}

string TePostgreSQL::getSQLBoxSelect(const string& tableName, TeGeomRep rep)
{
	if(rep == TeTEXT)
	{
		return TeDatabase::getSQLBoxSelect(tableName, rep);
	}
	else
	{
		string colname = "spatial_box";

		if(rep & TeRASTER)
			colname = "block_box";

		string select  = tableName + ".* , ";
		//string select  = "";
			   select += "((" + tableName + "." + colname + "[1])[0]) as lower_x, ";
			   select += "((" + tableName + "." + colname + "[1])[1]) as lower_y, ";
			   select += "((" + tableName + "." + colname + "[0])[0]) as upper_x, ";
			   select += "((" + tableName + "." + colname + "[0])[1]) as upper_y ";

		return select;
	}
}

bool TePostgreSQL::getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom)
{		
	TeDatabasePortal* portal = getPortal();

	if(!portal)
		return false;

	string sel  = "SELECT MIN(";
	       sel += "((" + tableGeom + "." + colGeom + "[1])[0])), MIN(";
           sel += "((" + tableGeom + "." + colGeom + "[1])[1])), MAX(";
           sel += "((" + tableGeom + "." + colGeom + "[0])[0])), MAX(";
           sel += "((" + tableGeom + "." + colGeom + "[0])[1])) ";
           sel += " FROM " + tableGeom;
           sel += " WHERE object_id = '" + object_id + "'";

	if(!(portal->query(sel)) || !(portal->fetchRow()))
	{
		delete portal;

		return false;
	}
		
	double xmin = portal->getDouble(0);
	double ymin = portal->getDouble(1);
	double xmax = portal->getDouble(2);
	double ymax = portal->getDouble(3);
				
	TeBox bb(xmin, ymin, xmax, ymax);

	box = bb;

	delete portal;

	return true;
}

bool TePostgreSQL::getMBRSelectedObjects(string geomTable, string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType, TeBox& bout, const double& tol)
{
	if(repType == TeTEXT)
		return TeDatabase::getMBRSelectedObjects(geomTable, colGeom, fromClause, whereClause, afterWhereClause, repType, bout, tol);

	TeDatabasePortal* portal = getPortal();
	
	if (!portal)
		return false;

	colGeom = "spatial_box";

	if(repType & TeRASTER)
		colGeom = "block_box";

	string sel  = "SELECT MIN(";
	       sel += "((" + geomTable + "." + colGeom + "[1])[0])), MIN(";
           sel += "((" + geomTable + "." + colGeom + "[1])[1])), MAX(";
           sel += "((" + geomTable + "." + colGeom + "[0])[0])), MAX(";
           sel += "((" + geomTable + "." + colGeom + "[0])[1])) ";
           sel += " FROM " + fromClause;

	if(!whereClause.empty())
		sel += " WHERE " + whereClause;  
	
	if(!(portal->query(sel)) || !(portal->fetchRow())) 
	{
		delete portal;

		return false;
	}

	double xmin = portal->getDouble(0);
	double ymin = portal->getDouble(1);
	double xmax = portal->getDouble(2);
	double ymax = portal->getDouble(3);

	TeBox b(xmin, ymin, xmax, ymax);

	bout = b;
	
	delete portal;

	return true;
}

void TePostgreSQL::getInsertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, const int& band, const unsigned int& res, const unsigned int& subband, char* buf, const unsigned long& size, string& sql)
{

	sql += "INSERT INTO " + table + " (block_id, block_box, band_id, resolution_factor, subband, spatial_data, block_size) VALUES('";
	sql += blockId;
	sql += "', '";
	sql += PGBoxRtree_encode(TeBox(ll, ur));
	sql += "', ";
	sql += Te2String(band);
	sql += ", ";
	sql += Te2String(res);
	sql += ", ";
	sql += Te2String(subband);

	//Adapting the sql to prevent errors that can occur on each platform
	#ifdef __unix__
	sql += ", '";
	#elif defined _WIN32
	sql += ", E'";
	#endif

	sql += buf;
	sql += "', ";
	sql += Te2String(size);
	sql += ")";

	return;
}

void TePostgreSQL::getUpdateRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, const int& band, const unsigned int& res, const unsigned int& subband, char* buf, const unsigned long& size, string& sql)
{
	sql += "UPDATE ";
	sql += table;
	sql += " SET block_box = '";
	sql += PGBoxRtree_encode(TeBox(ll, ur));
	sql += "', band_id = ";
	sql += Te2String(band);
	sql += ", resolution_factor = ";
	sql += Te2String(res);
	sql += ", subband = ";
	sql += Te2String(subband);

	//Adapting the sql to prevent errors that can occur on each platform
	#ifdef __unix__
	sql += ", spatial_data = '";
	#elif defined _WIN32
	sql += ", spatial_data = E'";
	#endif

	//sql += Te2String(static_cast<long>(obj_oid));;
	sql += buf;
	sql += "', block_size = ";
	sql += Te2String(size);
	sql += " WHERE block_id = '";
	sql += blockId;
	sql += "'";

	return;
}


string TePostgreSQL::escapeSequence(const string& from)
{
	size_t newLen = 0;


	char* aux = TePGConnection::escapeString(from.c_str(), from.length(), newLen);

	string str = aux;

	delete [] aux;

	return str;
}

string TePostgreSQL::getSpatialIdxColumn(TeGeomRep rep)
{
	if(rep == TeRASTER)
		return "block_box";

	return "spatial_box";
}

bool TePostgreSQL::beginTransaction()
{
	transactionCounter_++;

	if (transactionCounter_ > 1)
		return true;
	else
		return tepg_connection_->beginTransaction();
}

bool TePostgreSQL::commitTransaction()
{
	transactionCounter_--;

	if (transactionCounter_ > 0)
		return true;
	else
		return tepg_connection_->commitTransaction();
}

bool TePostgreSQL::rollbackTransaction()
{
	transactionCounter_--;

	if (transactionCounter_ > 0)
		return true;
	else
		return tepg_connection_->rollBackTransaction();
}

std::string TePostgreSQL::getClientEncoding()
{
	return tepg_connection_->getClientEncoding();

	/*int clientEncodingId = tepg_connection_->getClientEncodingId();

	std::string clientEncoding = "";

	std::string sql = "select pg_encoding_to_char("+Te2String(clientEncodingId)+")";

	TeDatabasePortal* portal = getPortal();

	if(portal->query(sql) && portal->fetchRow())
	{
		clientEncoding = portal->getData(0);
	}

	delete portal;
	portal =  NULL;

	return clientEncoding;*/
}

bool TePostgreSQL::setClientEncoding(const std::string& characterSet)
{
	std::string sql = "SET CLIENT_ENCODING TO '" + characterSet + "'";
	return execute(sql);
}

bool TePostgreSQL::getEncodingList(std::vector<std::string>& vecEncodingList)
{
	vecEncodingList.clear();

	std::string sql = "SELECT DISTINCT pg_catalog.pg_encoding_to_char(conforencoding)";
	sql += " FROM pg_catalog.pg_conversion";
	sql += " ORDER BY pg_catalog.pg_encoding_to_char(conforencoding)";

	TeDatabasePortal* portal = getPortal();

	bool returnValue = false;
	if(portal->query(sql))
	{
		returnValue = true;
		while(portal->fetchRow())
		{
			vecEncodingList.push_back(portal->getData(0));
		}
	}

	delete portal;
	portal =  NULL;

	return returnValue; 
}

bool TePostgreSQL::getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList)
{ 
	if(!connect(host, user, password, "template1", port))
	{
		close();
		return false;
	}

	bool returnValue = getEncodingList(vecEncodingList);

	close();

	return returnValue; 
}

bool
TePostgreSQL::getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes)
{
	
	std::string query1 = "select t.relname as tabela , u.relname as indice, i.indkey as posicao ";
		query1 += "from pg_class t, pg_class u, pg_index i ";
		query1 += "where t.oid = i.indrelid and i.indexrelid = u.oid and " + toUpper("t.relname") + " = "+ toUpper("'" + tableName + "'");
		query1 += "and indisprimary = 'f'";


	TeDatabasePortal* portal = getPortal();

	std::string tabName = "";
	std::string indName = "";
	std::string posicao = "";
	
	vector<string> indexesNames;
	vector<string> posicoes;
	vector<TeDatabaseIndex> indexes;

	if(portal->query(query1))
	{		
		while(portal->fetchRow())
		{
			tabName = portal->getData("tabela");
			indName = portal->getData("indice");
			posicao = portal->getData("posicao");
			indexesNames.push_back(indName);

			std::vector<std::string> vecParts;
			TeSplitString(posicao, ",", vecParts);

			for(unsigned int i=0; i < vecParts.size(); ++i)
			{
				std::string str = vecParts[i];
				TeTrim(str);

				std::vector<std::string> vecPartsComposto;
				TeSplitString(str, " ", vecPartsComposto);

				std::string strComposto = "";
				for(unsigned int j = 0; j < vecPartsComposto.size(); ++j)
				{
					if(!strComposto.empty())
					{
						strComposto += ",";
					}
					strComposto += vecPartsComposto[j];
				}

				posicoes.push_back(strComposto.c_str());
			}
		}
	}
	else
	{
		delete portal;
		return false;
	}
		

	portal->freeResult();

	
	for(unsigned int i=0; i < posicoes.size(); i++)
	{
		std::string query2 = "select a.attname as colName from pg_attribute a, pg_class c where a.attrelid = c.oid ";
		query2 += "and " + toUpper("c.relname") + " = " + toUpper("'" + tabName + "'") + " and a.attnum > 0 ";
		query2 += "and attnum in(" + posicoes[i] + ")";

		if(portal->query(query2))
		{
			
			vector<string> attributes;
			while(portal->fetchRow())
			{
				attributes.push_back(portal->getData("colName"));
			}

			TeDatabaseIndex dbi;
			dbi.setIndexName(indexesNames[i]);
			dbi.setColumns(attributes);
			dbi.setIsPrimaryKey(false);

			indexes.push_back(dbi);
			
		}
		else
		{
			delete portal;
			return false;
		}

		portal->freeResult();

	}

	vecIndexes = indexes;

	delete portal;
	portal =  NULL;

	return true;
}

bool 
TePostgreSQL::getAttributeList(const string& tableName, TeAttributeList& attList)
{
//recupera o nome da tabela da forma que ela esta salva no banco. 
	if (tableName.empty())
		return false;

	errorMessage_ = "";

	string sql = "SELECT relname FROM pg_class WHERE lower(relname) = lower('" + tableName + "')";
	TePGRecordset rec;
	if(!rec.open(sql, tepg_connection_) || rec.recordCount() == 0)
	{
		rec.close();
		TeDatabase::getAttributeList(tableName, attList);
		return false;
	}

	std::string tableNameDB = rec.getData(0);

	rec.close();

//em seguida recupera a lista de atributos
	TeDatabasePortal* portal = this->getPortal();
	if (!portal)
		return false;

	sql = "SELECT * FROM \"" + tableNameDB + "\" WHERE 1=2";
	if (!portal->query(sql))
	{
		delete portal;
		return false;
	}
	else
	{
		attList = portal->getAttributeList();
	}

	portal->freeResult();
	
//por fim preenchemos as informacoes com relacao as chaves
	sql = "SELECT ta.attname AS column_name";
	sql += " FROM pg_class bc, pg_class ic, pg_index i, pg_attribute ta, pg_attribute ia";
	sql += " WHERE bc.oid = i.indrelid AND ic.oid = i.indexrelid AND ia.attrelid = i.indexrelid AND ta.attrelid = bc.oid";
	sql += " AND " + toUpper("bc.relname") + " = '" + TeConvertToUpperCase(tableName) + "'";
	sql += " AND ta.attrelid = i.indrelid AND ta.attnum = i.indkey[ia.attnum-1]";
	sql += " AND  i.indisprimary = 't'";
	
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
			if(attList[i].rep_.name_ == columnName)
			{
				attList[i].rep_.isPrimaryKey_ = true;
                break;
			}
		}
	}

	delete portal;

	return true;
}

//----- TePostgreSQLPortal methods ---
TePostgreSQLPortal::TePostgreSQLPortal() : con_(0), curRow_(-1)
{
	tepg_recordset_ = new TePGRecordset();
}

TePostgreSQLPortal::TePostgreSQLPortal(TeDatabase *pDatabase) : curRow_(-1)
{
	db_ = pDatabase;
	con_ = ((static_cast<TePostgreSQL*>(pDatabase))->tepg_connection_);
	tepg_recordset_ = new TePGRecordset();
}

TePostgreSQLPortal::~TePostgreSQLPortal()
{
	this->freeResult();
	con_ = 0;

	delete tepg_recordset_;
}

bool TePostgreSQLPortal::query(const string &qry, TeCursorLocation l, TeCursorType t, TeCursorEditType /* e */, TeCursorDataType dt)
{
	errorMessage_ = "";

	freeResult();	

	if(!tepg_recordset_->open(qry, con_, t, l, dt))
	{
		TeWriteToFile("erros42.txt", "-------------------\nsql=" + qry + "\nerror="+ this->errorMessage() + "\n\n", "a+");
		return false;
	}

	numRows_ = tepg_recordset_->recordCount();

	numFields_ = tepg_recordset_->getFieldCount(); 

	attList_.clear ();

	for(int i = 0; i < numFields_; ++i)
	{
		TeAttribute attribute;

		Oid nType = tepg_recordset_->fieldType(i);

		switch(nType)
		{
			case 16:    //BOOL			
						attribute.rep_.type_ = TeBOOLEAN;
						break;

			case 20:    //INT8
			case 21:    //INT2
			case 23:    //INT4
			case 26:    //OID			
						attribute.rep_.type_ = TeINT;
						break;
			case 700 :  //float4  -> float(p)
			case 701 :  //float8  -> float(p)
			case 790 :  //money   -> decimal(9, 2)
			case 1700:  //numeric -> numeric(p, s)
						attribute.rep_.type_ = TeREAL;
						break;

			case 1082:	//date -> date
			case 1083:  //time -> time
			case 1114:  //timestamp
			case 1186:  //interval
			case 1266:	//
						attribute.rep_.type_ = TeDATETIME;
						attribute.dateTimeFormat_ = "YYYYsMMsDDsHHsmmsSS24";
						attribute.dateChronon_ = TeSECOND;
						attribute.dateSeparator_ = "-";	
						break;

			//case 26:    //OID
			case 17:	//bytea
						attribute.rep_.type_ = TeBLOB;
						break;
			case 1042:
						attribute.rep_.type_ = TeCHARACTER;
						break;

			case 1043:  //varchar(n)
			case 25:    //text
						attribute.rep_.type_ = TeSTRING;
						break;

			default:
						attribute.rep_.type_ = TeUNKNOWN;
						break;
		}

		attribute.rep_.name_ = tepg_recordset_->fieldName(i);
		pair<int, int> len(tepg_recordset_->fieldSize(i), tepg_recordset_->fieldSizeFractionaryPart(i));
		
		if(nType != 25) //text
		{
			attribute.rep_.numChar_ = len.first + len.second;
		}
		attList_.push_back(attribute);
	}

	curRow_ = 0;	

	return true;
}

bool TePostgreSQLPortal::fetchRow()
{
	errorMessage_ = "";

	if(numFields_ <= 0)
	{
		errorMessage_ = "The PostgreSQL portal is empty!";
		return false;
	}

	if(curRow_ > 0)
		tepg_recordset_->moveNext();

	if(tepg_recordset_->eof())
		return false;

	++curRow_;	

	return true;
}

bool TePostgreSQLPortal::fetchRow(int i)
{
	errorMessage_ = "";

	curRow_ = i + 1;
	return tepg_recordset_->moveTo(curRow_);
}

void TePostgreSQLPortal::freeResult()
{
	tepg_recordset_->close();

	return;
}

string TePostgreSQLPortal::errorMessage()
{
	if(errorMessage_.empty())
		return con_->err_msg();	
	else
		return errorMessage_;
}

char* TePostgreSQLPortal::getData(int i)
{
	errorMessage_ = "";
	
	try
	{
		return tepg_recordset_->getData(i);
	}

	catch(...)
	{
		return "";
	}

	return "";

}

char* TePostgreSQLPortal::getData(const string& s)
{
	errorMessage_ = "";

	try
	{
		return tepg_recordset_->getData(s);
	}

	catch(...)
	{
		return "";
	}

	return "";
}

double TePostgreSQLPortal::getDouble(int i)
{
	errorMessage_ = "";

	try
	{
		return tepg_recordset_->getDouble(i);
	}

	catch(...)
	{
		return 0.0;
	}

	return 0.0;
}

double TePostgreSQLPortal::getDouble(const string& s)
{
	errorMessage_ = "";

	try
	{
		return tepg_recordset_->getDouble(s);
	}

	catch(...)
	{
		return 0.0;
	}

	return 0.0;
}

int TePostgreSQLPortal::getInt(int i)
{
	errorMessage_ = "";

	try
	{
		return tepg_recordset_->getInt(i);
	}

	catch(...)
	{
		return 0;
	}

	return 0;
}

int TePostgreSQLPortal::getInt(const string& s)
{
	errorMessage_ = "";

	try
	{
		return tepg_recordset_->getInt(s);
	}

	catch(...)
	{
		return 0;
	}

	return 0;
}

bool TePostgreSQLPortal::getBool(const string& s)
{
	errorMessage_ = "";

	try
	{
		if(tepg_recordset_->getBool(s))
			return true;
		else
			return false;
	}

	catch(...)
	{
		return false;
	}
	
	return false;
}

bool TePostgreSQLPortal::getBool(int i)
{
	errorMessage_ = "";

	try
	{
		if(tepg_recordset_->getInt(i))
			return true;
		else
			return false;
	}

	catch(...)
	{
		return false;
	}

	return false;
}

TeTime TePostgreSQLPortal::getDate(int i)
{
	string s = getData(i);

	TeTime t(s, TeSECOND, "YYYYsMMsDDsHHsmmsSS", "-"); 

	return t;
}

TeTime TePostgreSQLPortal::getDate(const string& s)
{
	string ss = getData(s);

	TeTime t(ss, TeSECOND, "YYYYsMMsDDsHHsmmsSS", "-"); 

	return t;
}

string TePostgreSQLPortal::getDateAsString(int i)
{
	return getData(i);
}

string TePostgreSQLPortal::getDateAsString(const string& s)
{
	return getData(s);
}

bool TePostgreSQLPortal::getBlob(const string& s, unsigned char*& data, long& size)
{
	errorMessage_ = "";

	data = 0;

	char* ptDataAux = (char*)data;

	unsigned long newLen = tepg_recordset_->getBytea(s, ptDataAux);

	if(newLen > 0)
	{
		size = newLen;
		data = (unsigned char*)ptDataAux;
		return true;
	}

	errorMessage_ = "Couldn't read blob! Blob size error!";

	if(data)
	{
		delete [] data;
		data = 0;
	}

	return false;
}

bool TePostgreSQLPortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{
	errorMessage_ = "";

	// get the actual length of the compressed data
	size = tepg_recordset_->getInt("block_size");
	
	
	if(size > 0)
	{
		char* ptDataAux = (char*)ptData;

		unsigned long newLen = tepg_recordset_->getBytea("spatial_data", ptDataAux);

		if(newLen == size)
		{
			ptData = (unsigned char*)ptDataAux;
			return true;
		}
	}

	return false;
}


bool TePostgreSQLPortal::fetchGeometry(TePolygon& pol)
{
	errorMessage_ = "";

	int numberOfHoles;

	TeLinearRing ring = this->getLinearRing(numberOfHoles);

	pol.objectId(ring.objectId());
	pol.geomId(ring.geomId());
	pol.add(ring);
	int parentId = pol.geomId();

	while(fetchRow())
	{
		if(this->getInt("parent_id") == parentId)
		{
			int dummy;

			TeLinearRing ring = getLinearRing(dummy);
			pol.add(ring);
		}
		else
			return true;
	}

	return false;
}

bool TePostgreSQLPortal::fetchGeometry(TePolygon& pol, const unsigned int& initIndex)
{
	errorMessage_ = "";

	int numberOfHoles;

	TeLinearRing ring = this->getLinearRing(numberOfHoles, initIndex);

	pol.objectId(ring.objectId());
	pol.geomId(ring.geomId());
	pol.add(ring);
	int parentId = pol.geomId();

	while(fetchRow())
	{
		if(this->getInt(initIndex+4) == parentId)
		{
			int dummy;

			TeLinearRing ring = getLinearRing(dummy, initIndex);
			pol.add(ring);
		}
		else
			return true;
	}

	return false;
}

bool TePostgreSQLPortal::fetchGeometry(TeLine2D& line)
{
	errorMessage_ = "";

	tepg_recordset_->getPGLine2D("spatial_data", line);

	int geomId = tepg_recordset_->getInt("geom_id");

	string objectId = tepg_recordset_->getData("object_id");

	line.objectId(objectId);
	
	line.geomId(geomId);

	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TeLine2D& line, const unsigned int& initIndex )
{
	errorMessage_ = "";

	tepg_recordset_->getPGLine2D(initIndex+5, line); //spatial_data

	int geomId = tepg_recordset_->getInt(initIndex);

	string objectId = tepg_recordset_->getData(initIndex+1);

	line.objectId(objectId);
	
	line.geomId(geomId);

	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TeNode& n)
{
	errorMessage_ = "";
	
	TeCoord2D pt(getDouble("x"), getDouble("y"));

	n.add(pt);
	n.geomId(atoi(getData("geom_id")));
	n.objectId(string(getData("object_id")));

	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	errorMessage_ = "";
	
	TeCoord2D pt(getDouble(initIndex+3), getDouble(initIndex+4)); 

	n.add(pt);
	n.geomId(atoi(getData(initIndex)));
	n.objectId(string(getData(initIndex+1)));

	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TePoint& p)
{
	errorMessage_ = "";

	TeCoord2D c(getDouble("x"), getDouble("y"));
	p.add(c);
	p.geomId(atoi(getData("geom_id")));
	p.objectId(string(getData("object_id")));
	
	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	errorMessage_ = "";

	TeCoord2D c(getDouble(initIndex+3), getDouble(initIndex+4));
	p.add(c);
	p.geomId(atoi(getData(initIndex)));
	p.objectId(string(getData(initIndex+1)));
	
	return fetchRow();
}

TeLinearRing TePostgreSQLPortal::getLinearRing(int& numberOfHoles)
{
	int geomId = tepg_recordset_->getInt("geom_id");
	string objectId = tepg_recordset_->getData("object_id");

	numberOfHoles = tepg_recordset_->getInt("num_holes");	

	TeLine2D line;

	tepg_recordset_->getPGLine2D("spatial_data", line);

	TeLinearRing ring = line;

	ring.objectId(objectId);
	
	ring.geomId(geomId);

	return ring;
}

TeLinearRing TePostgreSQLPortal::getLinearRing(int& numberOfHoles, const unsigned int& initIndex)
{
	int geomId = tepg_recordset_->getInt(initIndex);
	string objectId = tepg_recordset_->getData(initIndex+1);

	numberOfHoles = tepg_recordset_->getInt(initIndex+3);	

	TeLine2D line;

	tepg_recordset_->getPGLine2D(initIndex+7, line); //spatial_data

	TeLinearRing ring = line;

	ring.objectId(objectId);
	
	ring.geomId(geomId);

	return ring;
}

string TePostgreSQLPortal::escapeSequence(const string& from)
{
	size_t newLen = 0;

	char* aux = TePGConnection::escapeString(from.c_str(), from.length(), newLen);

	string str = aux;

	delete [] aux;

	return str;
}

bool TePostgreSQLPortal::fetchGeometry(TeCell& cell)
{
	errorMessage_ = "";

	TeBox b;
	tepg_recordset_->getPGBox("spatial_box", b);

	cell.geomId(tepg_recordset_->getInt("geom_id"));
	cell.objectId(tepg_recordset_->getData("object_id"));
	cell.setBox (b);
	cell.column(tepg_recordset_->getInt("col_number"));
	cell.line(tepg_recordset_->getInt("row_number"));

	return fetchRow();
}

bool TePostgreSQLPortal::fetchGeometry(TeCell& cell, const unsigned int& initIndex)
{
	errorMessage_ = "";

	TeBox b;
	tepg_recordset_->getPGBox(initIndex+2, b);

	cell.geomId(tepg_recordset_->getInt(initIndex));
	cell.objectId(tepg_recordset_->getData(initIndex+1));
	cell.setBox (b);
	cell.column(tepg_recordset_->getInt(initIndex+3));
	cell.line(tepg_recordset_->getInt(initIndex+4));

	return fetchRow();
}

