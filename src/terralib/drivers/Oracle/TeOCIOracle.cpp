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

#include "TeOCIConnect.h"
#include "TeOCICursor.h"
#include "TeOCIOracle.h"
#include "TeProject.h"

#include <sys/stat.h>

bool	
TeOCIOracle::createSequence(const string &tableName)
{
	string nameSeq = getNameSequence(tableName);
	
	string seq = " CREATE SEQUENCE " + nameSeq;
	seq += " START WITH 1 INCREMENT BY 1 ORDER ";
	if (!execute(seq))
	{	
		errorMessage_ = "Error creating sequence to table " + tableName + " !";
		return false;
	}

	return true;
}

bool
TeOCIOracle::createAutoIncrementTrigger(const string &tableName, const string &fieldName)
{
	string nameTri = getNameTrigger(tableName);
	string nameSeq = getNameSequence(tableName);

	string tri;
	tri = "CREATE TRIGGER " + nameTri; 
	tri += " BEFORE INSERT ON "+tableName; 
	tri += " for each row";
	tri += " begin";
	tri += " select "+nameSeq+".NEXTVAL";
	tri += " into :new."+fieldName;
	tri += " from dual;";
	tri += " end;";

	if(!execute(tri))
	{	
		errorMessage_ = "Error creating trigger to table " + tableName + " !";
		return false;
	}
	return true;
}

string
TeOCIOracle::getNameSequence(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_seq";
	else
		name = tableName + "_seq";

	return name;
}

string
TeOCIOracle::getNameTrigger(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_tri";
	else
		name = tableName + "_tri";

	return name;
}

string  
TeOCIOracle::escapeSequence (const string& from)
{
	int  fa = 0;
    string to = from;
    to.insert(0, " ");
    string::iterator it = to.begin();
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

TeOCIOracle::TeOCIOracle() 
{
	connection_ = new TeOCIConnection();  
	dbmsName_ = "OracleOCI";
	sequenceCont_ = -1;
	sequenceName_ = "";
}

TeOCIOracle::~TeOCIOracle() 
{
	if (connection_)
		delete (connection_); //disconnect
	connection_ = NULL;
}

bool TeOCIOracle::beginTransaction()
{
	if(!connection_)
	{
		return false;
	}

	transactionCounter_++;
	if (transactionCounter_ > 1)
	{
		return true;	
	}

	return (connection_->transStart());
}

bool TeOCIOracle::commitTransaction()
{
	if(!connection_)
	{
		return false;
	}

	transactionCounter_--;
	if (transactionCounter_ > 0)
	{
		return true;
	}

	return (connection_->transCommit());
}

bool TeOCIOracle::rollbackTransaction()
{
	if(!connection_)
		return false;

	if(transactionCounter_ == 0)
	{
		return true;
	}

	transactionCounter_ = 0;
	return (connection_->transRollback());
}

bool 
TeOCIOracle::newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port, bool terralibModel, const std::string& characterSet) 
{ 
	if (!connect(host,user,password,database,port))
       return false;
    if (terralibModel)
    {
        //create conceptual model
        if(!this->createConceptualModel())  
			return false;
    }
    return true;
}

bool
TeOCIOracle::connect(const string& host, const string& user, const string& password, const string& database, int port)
{
	if(connection_ == NULL || connection_->isConnected())
	{ 
		delete (connection_);
		connection_ = new TeOCIConnection();
	}

	isConnected_ = false;
	if (connection_->connect(host.c_str(),user.c_str(),password.c_str()))
	{
		isConnected_ = true;
		host_ = host;
		user_ = user;
		password_ = password;
		database_ = database;
		portNumber_ = port;
		return true;
	}
	else
	{
		isConnected_ = false;
		errorMessage_ = "Error connecting to database server!";
		delete (connection_); //disconect
		connection_ = NULL;
		return false;
	}
}

void
TeOCIOracle::close() 
{
	clear();
	connection_->disconnect();
	isConnected_ = false;
}

bool 
TeOCIOracle::listTables(vector<string>& tableList)
{
	tableList.clear();
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string exist = " SELECT table_name FROM all_tables WHERE ";
	exist += " OWNER = '" + TeConvertToUpperCase(user_) + "'";

	if(!ocip->query(exist))
	{
		delete ocip;
		return false;
	}

	while(ocip->fetchRow())
		tableList.push_back (ocip->getData(0));
			
	delete ocip;
	return true;
}


bool 
TeOCIOracle::tableExist(const string& table)
{
	bool	status;
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string exist = " SELECT table_name FROM all_tables WHERE";
	exist += " TABLE_NAME = '" + TeConvertToUpperCase(table) + "'";
	exist += " AND OWNER = '" + TeConvertToUpperCase(user_) + "'";

	if(!ocip->query(exist))
	{
		delete ocip;
		return false;
	}

	if(ocip->fetchRow())
		status = true;
	else
		status = false;

	delete ocip;
	return (status);
}

bool 
TeOCIOracle::viewExist(const string& view)
{
	bool	status;
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string exist = " SELECT view_name FROM user_views WHERE";
	exist += " VIEW_NAME = '" + TeConvertToUpperCase(view) + "'";

	if(!ocip->query(exist))
	{
		delete ocip;
		return false;
	}

	if(ocip->fetchRow())
		status = true;
	else
		status = false;

	delete ocip;
	return (status);
}

bool 
TeOCIOracle::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	bool	status = false;
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string exist = " SELECT DATA_TYPE, DATA_LENGTH, DATA_SCALE FROM ALL_TAB_COLUMNS WHERE";
	exist += " TABLE_NAME = '" + TeConvertToUpperCase(table) + "'";
	exist += " AND COLUMN_NAME = '" + TeConvertToUpperCase(column) + "'";
	exist += " AND OWNER = '" + TeConvertToUpperCase(user_) + "'";

	if(!ocip->query(exist))
	{
		delete ocip;
		return false;
	}
	
	if(ocip->fetchRow())
	{	
		attr.rep_.name_ = column;
		
		string	dataType = string(ocip->getData(0)); 
		int		dataLength = atoi(ocip->getData(1)); 
		int		dataScale = atoi(ocip->getData(2));
		bool    number = false;
		
		if(dataType=="VARCHAR2")
		{
			attr.rep_.type_ = TeSTRING;
			attr.rep_.numChar_ = dataLength;
		}
		else if (dataType=="BLOB")
		{
			attr.rep_.type_ = TeBLOB;
			attr.rep_.numChar_ = dataLength;
		}
		else if (dataType=="NUMBER")
		{
			number = true;
		}
		else if (dataType=="SDO_GEOMETRY")
		{
			attr.rep_.type_ = TeOBJECT;
			attr.rep_.numChar_ = dataLength;
		}
		else if (dataType== "CHAR")
		{	
			attr.rep_.type_ = TeCHARACTER;
			attr.rep_.numChar_ = dataLength;
		}
		else if (dataType=="DATE")
		{
			attr.rep_.type_ = TeDATETIME;
		}
		else
		{
			attr.rep_.type_ = TeSTRING;
			attr.rep_.numChar_ = dataLength;
		}

		if(number)
		{
			if(dataScale > 0)
				attr.rep_.type_ = TeREAL;
			else
				attr.rep_.type_ = TeINT;
		}
		status = true;
	}
	
	delete ocip;
	return (status);
}

bool
TeOCIOracle::createTable(const string& table, TeAttributeList &attr)
{
	short	cont=0;
	string pkeys ="";
	bool	hasAutoNumber=false;
	string	fieldName="";

	TeAttributeList::iterator it = attr.begin();
	string tablec;
	tablec = "CREATE TABLE " + table +" (";
	
	while ( it != attr.end())
	{
		if (cont)
			tablec += ", ";
			
		switch ((*it).rep_.type_)
		{
			case TeSTRING:
				if((*it).rep_.numChar_ > 0)
				{
					tablec += (*it).rep_.name_ + " VARCHAR2(" + Te2String((*it).rep_.numChar_) + ")";
				}
				else
				{
					tablec += (*it).rep_.name_ + " VARCHAR2(4000)";
				}
			break;
			
			case TeREAL:
				if((*it).rep_.decimals_>0)
					tablec += (*it).rep_.name_ +" NUMBER(*,"+ Te2String((*it).rep_.decimals_) +") ";
				else
					tablec += (*it).rep_.name_ +" NUMBER(*,15) ";
			break;
			
			case TeINT:
			case TeUNSIGNEDINT:
				tablec += (*it).rep_.name_ + " NUMBER(32) ";
			break;

			case TeDATETIME:
				tablec += (*it).rep_.name_ + " DATE ";
			break;

			case TeCHARACTER:
				tablec += (*it).rep_.name_ + " CHAR ";
			break;

			case TeBOOLEAN:
				tablec += (*it).rep_.name_ + " NUMBER(1) ";
			break;

			case TeBLOB:
				tablec += (*it).rep_.name_ + " BLOB ";
			break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
			case TeNODETYPE:
			case TeNODESETTYPE:
				tablec += " x		NUMBER(*,15), ";
				tablec += " y		NUMBER(*,15) ";
				++it;
				cont++;
				continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:
				tablec += " num_coords NUMBER(32) NOT NULL, ";
				tablec += " lower_x NUMBER(*,15) NOT NULL, ";
				tablec += " lower_y NUMBER(*,15) NOT NULL, ";
				tablec += " upper_x NUMBER(*,15) NOT NULL, ";
				tablec += " upper_y NUMBER(*,15) NOT NULL, ";
				tablec += " ext_max NUMBER(*,15) NOT NULL, ";
				tablec += " spatial_data BLOB ";
				++it;
				cont++;
				continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
				tablec += " num_coords NUMBER(32) NOT NULL,";
				tablec += " num_holes  NUMBER(32) NOT NULL,";
				tablec += " parent_id  NUMBER(32) NOT NULL,";
				tablec += " lower_x    NUMBER(*,15) NOT NULL,";
				tablec += " lower_y    NUMBER(*,15) NOT NULL,";
				tablec += " upper_x    NUMBER(*,15) NOT NULL,";
				tablec += " upper_y    NUMBER(*,15) NOT NULL,";
				tablec += " ext_max    NUMBER(*,15) NOT NULL,";
				tablec += " spatial_data BLOB ";
				++it;
				cont++;
				continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
				tablec += " col_number	NUMBER(32) NOT NULL,";
				tablec += " row_number	NUMBER(32) NOT NULL,";
				tablec += " lower_x		NUMBER(*,15),";
				tablec += " lower_y		NUMBER(*,15),";
				tablec += " upper_x		NUMBER(*,15),";
				tablec += " upper_y		NUMBER(*,15) ";
				++it;
				cont++;
				continue;					

			case TeRASTERTYPE:
				tablec += " band_id NUMBER(32) NOT NULL, ";
				tablec += " resolution_factor NUMBER(32), ";
				tablec += " subband NUMBER(32),";
				tablec += " lower_x NUMBER(*,15) DEFAULT 0.0, ";
				tablec += " lower_y NUMBER(*,15) DEFAULT 0.0, ";
				tablec += " upper_x NUMBER(*,15) DEFAULT 0.0, ";
				tablec += " upper_y NUMBER(*,15) DEFAULT 0.0, ";							
				tablec += " block_size NUMBER(32), ";
				tablec += " spatial_data BLOB ";
				++it;
				cont++;
				continue;

			case TeTEXTTYPE:
			case TeTEXTSETTYPE:
			default:
				tablec += (*it).rep_.name_ + " VARCHAR2(255) ";
			break;
		}

		//default values
		if(!((*it).rep_.defaultValue_.empty()))
			tablec += " DEFAULT '" + (*it).rep_.defaultValue_ + "' ";

		//not null values
		if(!((*it).rep_.null_))
			tablec += " NOT NULL ";

		// auto number
		if((*it).rep_.isAutoNumber_ && ((*it).rep_.type_==TeINT || (*it).rep_.type_==TeUNSIGNEDINT))
		{
			hasAutoNumber=true;
			fieldName=(*it).rep_.name_;
		}

		// check if column is part of primary key
		if ((*it).rep_.isPrimaryKey_ && (*it).rep_.type_ != TeBLOB )
		{
			if (!pkeys.empty())
				pkeys += ", ";
			pkeys += (*it).rep_.name_;
		}

		++it;
		cont++;
	}

	if(!pkeys.empty())
		tablec += ", PRIMARY KEY (" + pkeys + ") ";

	tablec += ")";

	if(!execute(tablec))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error creating table " + table;

		return false;
	}

	if(hasAutoNumber)
	{
		string dropSql = " DROP TRIGGER "+ getNameTrigger(table); 
		execute(dropSql);
		dropSql = " DROP SEQUENCE "+ getNameSequence(table); 
		execute(dropSql);

		if(!createSequence(table))
		{
			deleteTable(table);
			return false;
		}
		
		if(!createAutoIncrementTrigger(table,fieldName))
		{
			deleteTable(table);
			string sql= "DROP SEQUENCE "+ getNameSequence(table);
			execute(sql); 
			return false;
		}
	}
	return true;
}

bool 
TeOCIOracle::addColumn (const string& table, TeAttributeRep &rep)
{
	if(!tableExist(table))
		return false;

	string field = TeGetExtension(rep.name_.c_str());
	if(field.empty())
		field = rep.name_;

	string tab;
	tab = " ALTER TABLE " + table + " ADD ( ";
	tab += field + "  ";
	
	switch (rep.type_)
	{
		case TeSTRING:
			if(rep.numChar_ > 0)
			{
				tab += "VARCHAR2(" + Te2String(rep.numChar_) + ") ";
			}
			else
			{
				tab += "VARCHAR2(4000) ";
			}
			break;
			
		case TeREAL:
			tab += "NUMBER(*,38)";	
			break;
			
		case TeINT:
			tab += "NUMBER(32)";
			break;

		case TeDATETIME:
			tab += "DATE";
			break;

		case TeCHARACTER:
			tab += "CHAR";
			break;

		case TeBOOLEAN:
			tab += "NUMBER(1)";
		break;
		
		case TeBLOB:
			tab += "BLOB";
			break;
		
		default:
			tab += "VARCHAR2(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	tab += " ) ";

	if(!connection_->execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting a column to table " + table + " !";
		return false;
	}

	alterTableInfoInMemory(table);
	return true;
}

bool
TeOCIOracle::deleteTable (const string& table)
{
	int f =	table.find ("te_collection");

	if( table=="te_theme" ||
		table=="te_layer" ||
		table=="te_representation" ||
		table=="te_tables_relation" ||
		table=="te_layer_table" ||
		table=="te_raster_metadata" ||
		table=="te_datum" ||
		table=="te_projection" ||
		table=="te_view" ||
		table=="te_chart_params" ||
		table=="te_legend" ||
		table=="te_visual" ||
		f == 0)
	{
		errorMessage_ = "Não é possível deletar tabelas do modelo!";
		return false;
	}
	
	if(tableExist(table))
	{
		std::string del = "DROP TABLE " + table;
		if(!execute(del))
			return false;
	}
	else if(viewExist(table))
	{
		std::string delView = "DROP VIEW " + table;
		if(!execute(delView))
			return false;
	}

	string seq = "DROP SEQUENCE " + getNameSequence(table);
	if(!execute(seq))
		return false;

	return true;
}

bool 
TeOCIOracle::execute (const string &q)
{
	bool result = connection_->execute(q);
	
	if(!result)
		errorMessage_ = connection_->getErrorMessage();

	return result;
}

TeDatabasePortal*  
TeOCIOracle::getPortal ()
{
	TeOCIOraclePortal* ocip = new TeOCIOraclePortal (this);
	return ocip;
}

bool 
TeOCIOracle::insertRelationInfo(const int tableId, const string& tField,
						    const string& eTable, const string& eField, int& relId)
{
	if(tableId < 0)
	{
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	relId = -1;
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " related_table_id = " + Te2String(tableId);
	sel += " AND related_attr = '" + tField + "'";
	sel += " AND external_table_name = '" + eTable + "'";
	sel += " AND external_attr = '" + eField + "'";
	if (!ocip->query(sel))
	{
		delete ocip;
		return false;
	}

	if (ocip->fetchRow())
	{
		relId = atoi(ocip->getData(0));
		delete ocip;
		return true;
	}

	string seq = getNameSequence("te_tables_relation");	
	string sql = " INSERT INTO te_tables_relation( ";
	sql += " relation_id, related_table_id, related_attr, ";
	sql += " external_table_name, external_attr) ";
	sql += " VALUES( ";
	sql += seq +".NEXTVAL "; 
	sql += ", "+ Te2String(tableId);
	sql += ",'" + escapeSequence(tField) + "'";
	sql += ",'" + escapeSequence(eTable) + "'";
	sql += ",'" + escapeSequence(eField) + "')";

	if(!execute(sql))
	{
		errorMessage_ = "Error inserting tables information!";   
		delete ocip;
		return false;
	}

	ocip->freeResult();
	sql = "SELECT "+ seq +".CURRVAL FROM DUAL";
	if (!ocip->query(sql))
	{
		errorMessage_ = "Error in the sequence te_table_relation_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	relId = atoi((const char*)ocip->getData(0));
	delete ocip;
	return true;
}

bool 
TeOCIOracle::insertTableInfo (int layerId, TeTable &table, const string& user)
{
	string ins, seq;

	seq = getNameSequence("te_layer_table");
	ins = "INSERT INTO te_layer_table ( "; 
	ins += " table_id, "; 
	ins += " layer_id, attr_table, unique_id, attr_link, ";
	ins += " attr_initial_time, attr_final_time, attr_time_unit, ";
	ins += " attr_table_type, user_name) VALUES ( ";
	ins += seq + ".NEXTVAL "; 
	
	if(layerId>0)
		ins += ","+ Te2String(layerId);
	else
		ins += ", NULL ";
	
	ins += ", '" + escapeSequence(table.name()) + "'";
	ins += ", '" + escapeSequence(table.uniqueName()) + "'";
	ins += ", '" + escapeSequence(table.linkName()) + "'";
	ins += ", '" + escapeSequence(table.attInitialTime()) + "'";
	ins += ", '" + escapeSequence(table.attFinalTime()) + "'";
	ins += ",  " + Te2String(table.attTimeUnit());
	ins += ",  " + Te2String(table.tableType());
	ins += ", '" +  escapeSequence(user) + "'";
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting tables information!";   
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT "+ seq +".CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		errorMessage_ = "Error in the sequence te_layer_table_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0));
	table.setId(index);
	delete ocip;
	return true;
}


TeDBRelationType 
TeOCIOracle::existRelation(const string& tableName, const string& relName)
{
	TeOCIOraclePortal  *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return TeNoRelation;
	
	string integ = " SELECT DELETE_RULE  ";
	integ += " FROM   all_constraints ";
	integ += " WHERE  CONSTRAINT_TYPE = 'R'"; 
	integ += " AND    OWNER = '" + TeConvertToUpperCase(user_) + "'";
	integ += " AND    CONSTRAINT_NAME = '" + TeConvertToUpperCase(relName) + "'";
	
	string temp = " AND    TABLE_NAME = '" + TeConvertToUpperCase(tableName) + "' ";   
	integ += temp;
	
	if(!ocip->query(integ))
	{
		delete ocip;
		return TeNoRelation;
	}

	string cascade;
	if(ocip->fetchRow())
	{
		cascade = string(ocip->getData(0));
		if(cascade== "CASCADE")
		{
			delete ocip;
			return TeRICascadeDeletion;
		}
		
		delete ocip;
		return TeRINoCascadeDeletion;
	}

	delete ocip;
	return TeNoRelation;	
}


bool 
TeOCIOracle::createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
{
	string relation = "ALTER TABLE " + table + " ADD ";
	relation += " CONSTRAINT " + name;
	relation += " FOREIGN KEY ( " + fieldName + ") "; 
	relation += " REFERENCES " + relatedTable + "(" + relatedField + ")";
		
	if (cascadeDeletion)
		relation += " ON DELETE CASCADE ";

	if(!execute(relation))
		return false;

	return true;
}

string
TeOCIOracle::getSQLStatistics (TeGroupingAttr& attrs)
{
	string sql = "";
	string virg = "";

	TeGroupingAttr::iterator it = attrs.begin();
	int count = 0;
	while(it != attrs.end())
	{
		if(count>0)
			virg = ",";

		switch ((*it).second)
		{
			case TeSUM:
				sql += virg +" SUM( "+ (*it).first.name_ +") AS SUM_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMAXVALUE:
				sql += virg +" MAX( "+ (*it).first.name_ +") AS MAX_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMINVALUE:
				sql += virg +" MIN( "+ (*it).first.name_ +") AS MIN_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeCOUNT:
				sql += virg +" COUNT( "+ (*it).first.name_ +") AS COUNT_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeMEAN:
				sql += virg +" AVG( "+ (*it).first.name_ +") AS AVG_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeSTANDARDDEVIATION:
				sql += virg +" STDDEV( "+ (*it).first.name_ +") AS STDDEV_"+ Te2String(count);
				(*it).second = TeNOSTATISTIC;
				++count;
				break;
			case TeVARIANCE:
				sql += virg +" VARIANCE( "+ (*it).first.name_ +") AS VAR_"+ Te2String(count);
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


string 
TeOCIOracle::getSQLAutoNumber(const string& table)
{
	return (getNameSequence(table) +".NEXTVAL");
}


bool 
TeOCIOracle::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, 
								   unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{
	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeOCIOraclePortal* portal = (TeOCIOraclePortal*) this->getPortal();
	if (!portal)
		return false;

	bool update = false;
	string q =" SELECT * FROM " + table; 
	q += " WHERE block_id='" + blockId + "'";

	if (!portal->query(q))
	{
		delete portal;
		return false;
	}
	// check if this block is alread in the database
	if (portal->fetchRow())
		update = true;

	delete portal;

	try
	{
		if (!update)
		{
			q = "INSERT INTO "+ table +" (block_id, band_id, subband, ";
			q += " resolution_factor, lower_x, lower_y, upper_x, upper_y, ";
			q += " block_size, spatial_data) VALUES ( ";
			q += "'" + blockId + "'";
			q += ", " + Te2String(band);
			q += ", " + Te2String(subband);
			q += ", " + Te2String(res);
			q += ", " + Te2String(ll.x(), 15);
			q += ", " + Te2String(ll.y(), 15);
			q += ", " + Te2String(ur.x(), 15);
			q += ", " + Te2String(ur.y(), 15);
			q += ", " + Te2String(size);
			q += ", :blobValue";
			q += ")";
			if (!connection_->executeBLOBSTM(q, buf, size, ":blobValue"))
				return false;
		}
		else
		{
			q = " UPDATE "+ table +" SET spatial_data=:blobValue ";
			q += " WHERE block_id='" + blockId + "'";
			if (!connection_->executeBLOBSTM(q, buf, size, ":blobValue"))
				return false;
		}
	}
	catch(...)
	{
		errorMessage_ = "Error inserting raster block!";
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size)
{
	TeOCIOraclePortal* portal = (TeOCIOraclePortal*) getPortal();
	if (!portal)
		return false;

	try
	{
		string q = "SELECT * FROM "+ tableName +" WHERE "+ whereClause;
		if((!portal->query(q)) || (!portal->fetchRow()))
		{
			delete portal;
			return false;
		}
		delete portal;

		q = " UPDATE "+ tableName +" SET "+ columnBlob +" = :blobValue";
		q += " WHERE "+ whereClause;
		if (!connection_->executeBLOBSTM(q, data, size, ":blobValue"))
			return false;
	}
	catch(...)
	{
		errorMessage_ = "Error!";
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertTable(TeTable &table)
{
	string tableName = table.name();
	int size = table.size();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	TeAttributeList::iterator itEnd = att.end();

	if (!beginTransaction())
		return false;

	TeTableRow row;
	int i;
	unsigned int j;
	int blobIndex = -1;
	for ( i = 0; i < size; i++  )
	{
		row = table[i];
		it = att.begin();

		j = 1;
		int jj = 0;

		std::string columnNames = "";
		std::string columnValues = "";
		while ( it != itEnd )
		{
			/*if (j != 1)
				q += ", ";*/

			if(!columnNames.empty())
			{
				columnNames += ",";
				columnValues += ",";
			}

			columnNames += it->rep_.name_;

			if(row[jj].empty())

			{
				columnValues += " NULL ";
				++it;
				j++;
				jj++;
				continue;
			}

  			string oracleFormat="";
			string dateTime="";
			if(((*it).rep_.type_==TeDATETIME) && (!row[jj].empty()))
			{
				TeTime t  (row[jj], (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_,
						   (*it).timeSeparator_, (*it).indicatorPM_);
				
				dateTime = t.getDateTime("DDsMMsYYYYsHHsmmsSS");
				oracleFormat = " TO_DATE('" + dateTime + "', 'DD/MM/YYYY HH24:MI:SS')";
			}
			
			switch ((*it).rep_.type_)
  			{
  				case TeSTRING:
					columnValues += "'"+ escapeSequence(row[jj]) +"'";
  				break;
  				case TeREAL:
					columnValues += row[jj]; 
  				break;
  				case TeINT:
					columnValues += row[jj];
  				break;
				case TeDATETIME:
					columnValues += oracleFormat;
				break;
				case TeCHARACTER:
					columnValues += "'" + escapeSequence(row[jj]) + "'";
  				break;
				case TeBOOLEAN:
				{
					std::string value = "0";
					if(row[jj] == "1" || TeConvertToLowerCase(row[jj]) == "t" || TeConvertToLowerCase(row[jj]) == "true")
					{
						value = "1";
					}
					columnValues += value;
				}
				break;
				case TeBLOB:
					blobIndex = jj;
					columnValues += " :blobValue ";
				break;
  				default:
					columnValues += "'"+ escapeSequence(row[jj]) +"'";
  				break;
  			}
			++it;
			j++;
			jj++;
		}
		std::string q = "INSERT INTO "+tableName+"("+columnNames+") VALUES("+columnValues+")";
		
		// Verify if there is blob type
		if(blobIndex>=0)
		{
			if(!connection_->executeBLOBSTM(q, (unsigned char *) row[blobIndex].c_str(), row[blobIndex].size(), ":blobValue"))
			{
				rollbackTransaction();
				return false;
			}
		}
		else 
		{
			if(!execute(q))
			{
				rollbackTransaction();
				return false;
			}
		}
	}
	if (!commitTransaction())
		return false;
	return true;
}
	
bool 
TeOCIOracle::alterTable (const string& table, TeAttributeRep &rep, const string& oldColName)
{
	if(!tableExist(table))
		return false;

	if(!oldColName.empty() && oldColName != rep.name_ ) //rename column
	{
		string add = " ALTER TABLE "+ table +" RENAME COLUMN "+ oldColName +" TO "+ rep.name_;
		if(!execute(add))
		{
			if(errorMessage_.empty())
				errorMessage_ = "Error alter table " + table + " !";
			return false;
		}
	}

	string tab = " ALTER TABLE "+ table +" MODIFY ( ";
	tab += rep.name_ + "  ";
	switch (rep.type_)
	{
		case TeSTRING:
			if ( rep.numChar_ > 0 )
				tab  += "VARCHAR2(" + Te2String(rep.numChar_) + ") ";
			else
				tab += "VARCHAR2(4000) "; 
			break;

		case TeREAL:
			if(rep.decimals_>0)
				tab += " NUMBER(*,"+ Te2String(rep.decimals_) +") ";
			else
				tab += " NUMBER(*,38) ";
		break;
		
		case TeINT:
			tab += " NUMBER(32) ";
			break;

		case TeDATETIME:
			tab += " DATE ";
			break;

		case TeCHARACTER:
			tab += " CHAR ";
			break;
			
		case TeBOOLEAN:
			tab += " NUMBER(1) ";
			break;
		
		default:
			tab += " VARCHAR2(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	tab += " )";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error alter table " + table + " !";
		return false;
	}
	
	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + table + "'";
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
			sql += " WHERE external_table_name = '" + table + "'";
			sql += " AND external_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

			 // update grouping
			sql = "UPDATE te_grouping SET grouping_attr = '" + table + "." + rep.name_ + "'";
			sql += " WHERE grouping_attr = '" + table + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
		else // column type changed
		{
			// delete relation
			sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
			sql += " AND related_attr = '" + rep.name_ + "')";
			sql += " OR (external_table_name = '" + table + "'";
			sql += " AND external_attr = '" + rep.name_ + "')";
			if(execute(sql) == false)
				return false;

			// delete grouping
			TeDatabasePortal* portal = getPortal();
			sql = "SELECT theme_id FROM te_grouping WHERE grouping_attr = '" + table + "." + oldColName + "'";
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
			sql += " WHERE grouping_attr = '" + table + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
	}
	alterTableInfoInMemory(table);
	return true;
}

bool 
TeOCIOracle::alterTable(const string& oldTableName, const string& newTableName)
{
	string sql = " ALTER TABLE "+ oldTableName +" RENAME TO "+ newTableName;
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


bool
TeOCIOracle::updateTable (TeTable &table)
{
	string tableName = table.name();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	
	int blobIndex = -1;
	TeTableRow row;
	unsigned int i;
	unsigned int j;
	string uniqueName = table.uniqueName();
	string uniqueVal;

	if (!beginTransaction())
		return false;

	for ( i = 0; i < table.size(); i++  )
	{
		row = table[i];
		it = att.begin();
		string q = 	"UPDATE "+tableName+" SET ";
		j = 1;
		int jj = 0;
		while ( it != att.end() )
		{
			string oracleFormat;
			string dateTime; 
			if((*it).rep_.type_==TeDATETIME)
			{
				TeTime t  (row[jj], (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_,
						   (*it).timeSeparator_, (*it).indicatorPM_);
				
				dateTime = t.getDateTime();
				oracleFormat = "DD/MM/YYYY HH24:MI:SS"; 
			}
			
			if (uniqueName != (*it).rep_.name_)
			{
				q += (*it).rep_.name_ + "=";
  				switch ((*it).rep_.type_)
  				{
  					case TeSTRING:
						q += "'"+escapeSequence(row[jj])+"'";
  					break;
  					case TeREAL:
						q += row[jj];
  					break;
  					case TeINT:
						q += row[jj];
  					break;
					case TeDATETIME:
						q += " TO_DATE('" + dateTime + "', '"+ oracleFormat +"')";
  					break;
					case TeCHARACTER:
						q += "'" + escapeSequence(row[jj]) + "'";
  					break;
					case TeBLOB:
						blobIndex = jj;
						q += " :blobValue ";
					break;
  					default:
						q += "'"+escapeSequence(row[jj])+"'";
  					break;
  				}
				if (j<att.size())
					q+= ",";
			}
			else
				uniqueVal = row[jj];

			++it;
			j++;
			jj++;
		}
		q += " WHERE " + uniqueName + " = " + uniqueVal;
		
		// Verify if there is blob type
		if(blobIndex>=0)
		{
			if (!connection_->executeBLOBSTM(q, (unsigned char *) row[blobIndex].c_str(), 
				row[blobIndex].size(), ":blobValue"))
				continue;
		}
		else 
		{
			if (!execute(q))
				continue;
		}
	}
	if (!commitTransaction())
		return false;
	return true;
}
	

bool 
TeOCIOracle::insertProjection (TeProjection *proj)
{
	string insert = "INSERT INTO te_projection (";
	insert += " projection_id, "; 
	insert += " name, long0, lat0,";
	insert += " offx, offy, stlat1, stlat2, unit, scale, hemis, datum )";
	insert += " VALUES ( ";
	insert += " te_projection_seq.NEXTVAL ";
	insert += ", '" + escapeSequence(proj->name()) + "'";
	insert += ", " + Te2String(proj->params().lon0*TeCRD,15);
	insert += ", " + Te2String(proj->params().lat0*TeCRD,15);
	insert += ", " + Te2String(proj->params().offx,15);
	insert += ", " + Te2String(proj->params().offy,15);
	insert += ", " + Te2String(proj->params().stlat1*TeCRD,15);
	insert += ", " + Te2String(proj->params().stlat2*TeCRD,15);
	insert += ", '" + escapeSequence(proj->params().units) + "'";
	insert += ", " + Te2String(proj->params().scale,15);
	insert += ", " + Te2String(proj->params().hemisphere, 15);
	insert += ", '" + escapeSequence(proj->params().datum.name()) + "'";
	insert += ")";

	if(!execute(insert))
	{
		errorMessage_ = "Error inserting in the table te_projection!";   
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*) getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_projection_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		errorMessage_ = "Error in the sequence te_projection_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	proj->id(atoi((const char*)ocip->getData(0))); 
	delete ocip;

	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}


bool
TeOCIOracle::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;
	
	string ins;
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*) getPortal();
	if(!ocip)
		return false;

	ins = " INSERT INTO te_representation ( ";
	ins += " repres_id, ";
	ins += " layer_id, geom_type, geom_table, ";
	ins += " description, lower_x, lower_y, upper_x, upper_y, res_x, res_y, num_cols, ";
	ins += " num_rows) VALUES (";
	ins += " te_representation_seq.NEXTVAL ";
	ins += ", " + Te2String(layerId);
	ins += ", " + Te2String(static_cast<int>(rep.geomRep_));
	ins += ", '" + escapeSequence(rep.tableName_) + "'";
	ins += ", '" + escapeSequence(rep.description_) + "'";
	ins += ", " + Te2String(rep.box_.x1(),15);
	ins += ", " + Te2String(rep.box_.y1(),15);
	ins += ", " + Te2String(rep.box_.x2(),15);
	ins += ", " + Te2String(rep.box_.y2(),15);
	ins += ", " + Te2String(rep.resX_,15);
	ins += ", " + Te2String(rep.resY_,15);
	ins += ", " + Te2String(rep.nCols_);
	ins += ", " + Te2String(rep.nLins_);
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_representation!";  
		delete ocip;
		return false;
	}
	
	string seq = "SELECT te_representation_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		errorMessage_ = "Error in the sequence te_representation_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int id = atoi((const char*)ocip->getData(0)); 
	rep.id_ = id;
	delete ocip;
	return true;
}


bool
TeOCIOracle::insertLegend (TeLegendEntry* leg)
{
	TeOCIOraclePortal  *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string ins = "INSERT INTO te_legend ( ";
	ins += " legend_id, ";
	ins += " theme_id, group_id, ";
	ins += " num_objs, lower_value, upper_value, label) VALUES ( ";
	ins += "te_legend_seq.NEXTVAL";
	ins += ", " + Te2String(leg->theme());
	ins += ", " + Te2String(leg->group());
	ins += ", " + Te2String(leg->count());
	ins += ", '" + escapeSequence(leg->from()) + "'";
	ins += ", '" + escapeSequence(leg->to()) + "'";
	ins += ", '" + escapeSequence(leg->label()) + "'";
	ins += ")";
		
	if (!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_legend!"; 
		delete ocip;
		return false;
	}

	string seq = "SELECT te_legend_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		errorMessage_ = "Error in the sequence te_theme_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";;
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0)); 
	leg->id(index);
	
	delete ocip;
	legendMap()[leg->id()] = leg;
	return insertVisual(leg);
}

bool
TeOCIOracle::insertPolygonSet(const string& table, TePolygonSet &ps)
{
	if(!getValueSequence(table))
		return false;

	for (unsigned int i = 0; i < ps.size(); i++ )
	{
		TePolygon& poly = ps [i];
		if (!insertPolygon (table,poly))		
			return false;
	}
	return true;
}

bool 
TeOCIOracle::insertPolygon (const string& table, TePolygon &poly)
{
	double	extmax;
	unsigned int i, k, ni, size;
	double	*points	= 0;
	try
	{
		long parentId = 0;

		for ( k = 0; k < poly.size(); k++ )
		{
			TeLinearRing ring (poly[k]);
			TeBox b = ring.box();
			size = ring.size(); 
			ni = 0;
			
			// max extent 
			extmax = MAX(b.width(),b.height());
		
			// number of holes
			if (k==0)
				ni = poly.size()-1;
			else
				ni = 0;
			
			++sequenceCont_;
			if(k==0)
				parentId=sequenceCont_;

			// insert data
			string ins = "INSERT INTO " + table + " ( ";
			ins += " geom_id, ";
			ins += " object_id, num_coords, num_holes, ";
			ins += " parent_id, lower_x, lower_y, upper_x, upper_y, "; 
			ins += " ext_max, spatial_data) VALUES ( ";
			ins +=  getNameSequence(table) +".NEXTVAL";
			ins += ", '" + escapeSequence(poly.objectId()) + "'";
			ins += ","+ Te2String(size);
			ins += ","+ Te2String(ni);
			ins += ","+ Te2String((long)parentId);
			ins += ","+ Te2String(b.lowerLeft().x(), 15);
			ins += ","+ Te2String(b.lowerLeft().y(), 15);
			ins += ","+ Te2String(b.upperRight().x(), 15);
			ins += ","+ Te2String(b.upperRight().y(), 15);
			ins += ","+ Te2String(extmax);
			ins += ", :blobValue ";
			ins += ")";

			//get coord
			points = new double[2*ring.size()];  //2*sizeof(double)*size
			int iac = 0;
			for (i=0;i<ring.size();i++)
			{
				points[iac++]=ring[i].x();
				points[iac++]=ring[i].y();
			}
			unsigned long bLen = 2*(ring.size())*sizeof(double)*sizeof(unsigned char);

			if (!connection_->executeBLOBSTM(ins, (unsigned char*)points, bLen, ":blobValue"))
			{
				errorMessage_ = "Error inserting in the table " + table + "!";  
				delete[] points;
				return false;
			}

			poly.geomId(sequenceCont_);
			if(points)
				delete[] points;
			points=NULL;
		}
	}
	catch(...)
	{
		errorMessage_ = "Error inserting polygon!";
		if(points)
			delete[] points;
		return false;
	}

	return true;
}

bool 
TeOCIOracle::updatePolygon (const string& table, TePolygon &poly)
{
	double	extmax;
	unsigned int k, ni, size, i;
	double	*points	= NULL;

	try
	{
		TeBox b = poly[0].box();
		for ( k = 0; k < poly.size(); k++ )
		{
			//Gets each ring
			TeLinearRing ring ( poly[k] );
			size = ring.size();
			ni = 0;
			if (k==0)
			{
				extmax = MAX(b.width(),b.height());
				ni = poly.size()-1;
			}
			else
				ni = 0;

			//update information about each ring
			string sql;
			sql =  " UPDATE " + table + " SET ";
			sql += ", object_id = '" + poly.objectId() + "'";
			sql += ", num_coords = " + Te2String(size);
			sql += ", num_holes = " + Te2String(ni);
			sql += ", parent_id = " + ((long)poly[0].geomId());
			sql += ", lower_x = " + Te2String(b.lowerLeft().x(), 15);
			sql += ", lower_y = " + Te2String(b.lowerLeft().y(), 15);
			sql += ", upper_x = " + Te2String(b.upperRight().x(), 15);
			sql += ", upper_y = " + Te2String(b.upperRight().y(), 15);
			sql += ", ext_max = " + Te2String(extmax);
			sql += ", spatial_data = :blobValue ";
			sql += " WHERE geom_id = "+ Te2String(ring.geomId());

			//get coord
			points = new double[2*ring.size()];  //2*sizeof(double)*size
			int iac = 0;
			for (i=0;i<ring.size();i++)
			{
				points[iac++]=ring[i].x();
				points[iac++]=ring[i].y();
			}

			int bLen = 2*(ring.size())*sizeof(double)*sizeof(unsigned char);

			if (!connection_->executeBLOBSTM(sql, (unsigned char*)points, bLen, ":blobValue"))
			{
				errorMessage_ = "Error inserting in the table " + table + "!";  
				delete[] points;
				return false;
			}

			if(points)
				delete[] points;
			points=NULL;
		}
	}
	catch(...)
	{
		errorMessage_ = "Error updating polygons!";
		if(points)
			delete[] points;
		return false;
	}
	return true;
}

bool
TeOCIOracle::getValueSequence(const string& table)
{
	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	if (getNameSequence(table) != sequenceName_)
	{
		string seq = "SELECT "+ getNameSequence(table) +".NEXTVAL FROM DUAL";
		
		if(!ocip->query(seq) || !ocip->fetchRow())
		{
			errorMessage_ = "Error in the sequence " + table + "_seq!"; 
			sequenceName_ = "";
			delete ocip;
			return false;
		}

		sequenceName_ = getNameSequence(table);
		sequenceCont_ = atoi(ocip->getData(0));
	}

	delete ocip;
	return true;
}

bool 
TeOCIOracle::insertLineSet(const string& table, TeLineSet &ls)
{
	if(!getValueSequence(table))
		return false;

	for (unsigned int i = 0; i < ls.size(); i++ )
	{
		TeLine2D& line = ls [i];
		if (!insertLine (table,line))
			return false;
	}
	return true;
}

bool
TeOCIOracle::insertLine (const string& table, TeLine2D &line)
{
	double	extmax;
	unsigned int i, size;
	double	*points	= 0;
	
	try
	{
		TeBox b = line.box();
		size = line.size(); 
		// max extent 
		extmax = MAX(b.width(),b.height());
		
		// insert data
		string ins = "INSERT INTO " + table + " ( ";
		ins += " geom_id, ";
		ins += " object_id, num_coords, ";
		ins += " lower_x, lower_y, upper_x, upper_y, "; 
		ins += " ext_max, spatial_data) VALUES ( ";
		ins +=  getNameSequence(table) +".NEXTVAL";
		ins += ", '" + escapeSequence(line.objectId()) + "'";
		ins += ","+ Te2String(size);
		ins += ","+ Te2String(b.lowerLeft().x(), 15);
		ins += ","+ Te2String(b.lowerLeft().y(), 15);
		ins += ","+ Te2String(b.upperRight().x(), 15);
		ins += ","+ Te2String(b.upperRight().y(), 15);
		ins += ","+ Te2String(extmax);
		ins += ", :blobValue";
		ins += ")";

		//get coord
		points = new double[2*line.size()];  //2*sizeof(double)*size
		int iac = 0;
		for (i=0;i<line.size();i++)
		{
			points[iac++]=line[i].x();
			points[iac++]=line[i].y();
		}
		unsigned long bLen = 2*(line.size())*sizeof(double)*sizeof(unsigned char);

		if (!connection_->executeBLOBSTM(ins, (unsigned char*)points, bLen, ":blobValue"))
		{
			errorMessage_ = "Error inserting in the table " + table + "!";  
			delete[] points;
			return false;
		}
		++sequenceCont_;
		line.geomId(sequenceCont_);
		if(points)
			delete[] points;
		points=NULL;
	}
	catch(...)
	{
		errorMessage_ = "Error inserting line!";
		if(points)
			delete[] points;
		return false;
	}

	return true;
}


bool 
TeOCIOracle::updateLine(const string& table, TeLine2D &line)
{
	double	extmax;
	unsigned int size, i;
	double	*points	= NULL;

	try
	{
		TeBox b = line.box();
		size = line.size();
		extmax = MAX(b.width(),b.height());
		
		//update information about each line
		string sql =  "UPDATE " + table + " SET ";
		sql += "  object_id= '" + line.objectId() + "'";
		sql += ", num_coords= " + ((long)size);
		sql += ", lower_x= " + Te2String(b.lowerLeft().x(), 15);
		sql += ", lower_y= " + Te2String(b.lowerLeft().y(), 15);
		sql += ", upper_x= " + Te2String(b.upperRight().x(), 15);
		sql += ", upper_y= " + Te2String(b.upperRight().y(), 15);
		sql += ", ext_max= " + Te2String(extmax);
		sql += ", spatial_data = :blobValue ";
		sql += " WHERE geom_id = "+ Te2String(line.geomId());

		//get coord
		points = new double[2*line.size()];  //2*sizeof(double)*size
		int iac = 0;
		for (i=0;i<line.size();i++)
		{
			points[iac++]=line[i].x();
			points[iac++]=line[i].y();
		}
		int bLen = 2*(line.size())*sizeof(double)*sizeof(unsigned char);

		if (!connection_->executeBLOBSTM(sql, (unsigned char*)points, bLen, ":blobValue"))
		{
			errorMessage_ = "Error inserting in the table " + table + "!";  
			delete[] points;
			return false;
		}
		
		if(points)
			delete[] points;
		points = NULL;
	}
	catch(...)
	{
		errorMessage_ = "Error updating lines!";
		if(points)
			delete[] points;
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertPoint(const string& table, TePoint &point)
{
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, ";
	ins += " object_id, x, y) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(point.objectId()) + "'";
	ins += ",  " + Te2String(point.location().x(), 15);
	ins += ",  " + Te2String(point.location().y(), 15);
	ins += " ) ";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting points!"; 
		return false;
	}
	return true;
}


bool 
TeOCIOracle::insertText	(const string& table, TeText &text)
{
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, ";
	ins += " object_id, x, y, text_value, angle, height, alignment_vert, ";
	ins += " alignment_horiz) VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(text.objectId()) + "'";
	ins += ",  " + Te2String(text.location().x(),15);
	ins += ",  " + Te2String(text.location().y(),15);
	ins += ", '" + escapeSequence(text.textValue()) + "'";
	ins += ",  " + Te2String(text.angle(),15);
	ins += ",  " + Te2String(text.height(),15);
	ins += ",  " + Te2String(text.alignmentVert(),15);
	ins += ",  " + Te2String(text.alignmentHoriz(),15);
	ins += " )";
	
	if(!execute(ins))
	{
		errorMessage_ = "Error inserting texts!"; 
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertArc (const string& table, TeArc &arc)
{
	string ins = "INSERT INTO " + table + " (";
	ins += " geom_id, ";
	ins += " object_id, from_node, to_node ) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(arc.objectId()) + "'";
	ins += ",  " + Te2String(arc.fromNode().geomId());
	ins += ",  " + Te2String(arc.toNode().geomId());
	ins += " )";
	
	if(!execute(ins))
	{
		errorMessage_ = "Error inserting arc!"; 
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertNode (const string& table, TeNode &node)
{	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, "; 
	ins += " object_id, x, y) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(node.objectId()) + "'";
	ins += ",  " + Te2String(node.location().x(), 15);
	ins += ",  " + Te2String(node.location().y(), 15);
	ins += " ) ";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting nodes!"; 
		return false;
	}
	return true;
}
	
bool 
TeOCIOracle::insertCell (const string& table, TeCell &cell )
{
	TeBox b = cell.box();
	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, ";
	ins += " object_id, col_number, row_number, lower_x, ";
	ins += " lower_y, upper_x, upper_y ) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(cell.objectId ()) + "'";
	ins += ",  " + Te2String(cell.column());
	ins += ",  " + Te2String(cell.line());
	ins += ",  " + Te2String(b.lowerLeft().x(), 15);
	ins += ",  " + Te2String(b.lowerLeft().y(), 15);
	ins += ",  " + Te2String(b.upperRight().x(), 15);
	ins += ",  " + Te2String(b.upperRight().y(), 15);
	ins += " )";
		
	if(!execute(ins))
	{
		errorMessage_ = "Error inserting cells!"; 
		return false;
	}
	return true;
}

bool 
TeOCIOracle::insertLayer(TeLayer* layer)
{	
	int index;
	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir layer sem projeção";
		return false;
	}
	string ins = "INSERT INTO te_layer (";
	ins += " layer_id, ";
	ins += " projection_id, name ";
	ins += ", lower_x, lower_y, upper_x, upper_y, edition_time) ";
	ins += " VALUES ( ";
	ins += "te_layer_seq.NEXTVAL";
	ins += ", "+ Te2String(proj->id());
	ins += ", '" + escapeSequence(layer->name()) + "'";
	ins += ", " + Te2String(layer->box().x1(),15);
	ins += ", " + Te2String(layer->box().y1(),15);
	ins += ", " + Te2String(layer->box().x2(),15);
	ins += ", " + Te2String(layer->box().y2(),15);
	TeTime editionTime = layer->getEditionTime();
	ins += ", " + getSQLTime(editionTime);
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_layer!";   
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT te_layer_seq.CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		errorMessage_ = "Error in the sequence te_layer_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	index = atoi((const char*)ocip->getData(0));
	layer->id(index);
	delete ocip;
	layerMap()[layer->id()] = layer;
	return true;
}

bool 
TeOCIOracle::insertProject(TeProject* project)
{	
	if (!project)
		return false;

	string ins = "INSERT INTO te_project (";
	ins += " project_id, ";
	ins += " name, description, current_view) ";
	ins += " VALUES ( ";
	ins += "te_project_seq.NEXTVAL";
	ins += ", '" + escapeSequence(project->name()) + "'";
	ins += ", '" + escapeSequence(project->description()) + "'";
	ins += ", " + Te2String(project->getCurrentViewId());
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_project!";   
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT te_project_seq.CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		errorMessage_ = "Error in the sequence te_project_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0));
	project->setId(index);
	projectMap()[project->id()] = project;
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);
	delete ocip;
	return true;
}

bool
TeOCIOracle::deleteLayer(int layerId)
{
	//Delete attributes tables
	if(!deleteLayerTable(layerId))
		return false;

	TeOCIOraclePortal* portal = (TeOCIOraclePortal*) this->getPortal();
	if (!portal)
		return false;

	string geomTable;
	string sql = "SELECT projection_id FROM te_layer WHERE layer_id = ";
	sql += Te2String(layerId);

	if (!portal->query(sql))
	{	
		delete portal;
		return false;
	}

	if (!portal->fetchRow())
	{
		delete portal;
		return false;
	}
	string projId = portal->getData("projection_id");
	portal->freeResult();

	// Get all representations that are associated to this layer
	sql = "SELECT * FROM te_representation WHERE layer_id = "+ Te2String(layerId);
	if (!portal->query (sql))
	{
		delete portal;
		return false;
	}

	while (portal->fetchRow())
	{	// Delete the geometry tables
		geomTable = portal->getData("geom_table");
	
		// Delete lut table
		TeGeomRep rep = TeGeomRep(atoi(portal->getData("geom_type")));
		if (rep == TeRASTER || rep == TeRASTERFILE)
		{
			TeOCIOraclePortal* portal2 = (TeOCIOraclePortal*)this->getPortal();
			sql = "SELECT lut_table, raster_table FROM " + geomTable;
			string tabName;
			if (!portal2->query (sql))
			{
				delete portal2;
				continue;
			}

			while (portal2->fetchRow())
			{
				// remove lut table
				tabName = portal2->getData(0);
				if (!tabName.empty() && this->tableExist(tabName))
				{
					sql = "DROP TABLE " + tabName;
					this->execute(sql);

					sql= "DROP SEQUENCE "+ getNameSequence(tabName);
					this->execute(sql); 
				}
				// remove raster table
				tabName = portal2->getData(1);
				if (!tabName.empty() && this->tableExist(tabName))
				{
					sql = "DROP TABLE " + tabName;
					this->execute(sql);

					sql= "DROP SEQUENCE "+ getNameSequence(tabName);
					this->execute(sql); 
				}
			}
			delete portal2;
			// remove raster metadata table
			tabName = geomTable + "_metadata";
			if (!tabName.empty() && this->tableExist(tabName))
			{
				sql = "DROP TABLE " + tabName;
				this->execute(sql);

				sql= "DROP SEQUENCE "+ getNameSequence(tabName);
				this->execute(sql); 

			}

		}
		if (this->tableExist(geomTable))
		{
			sql = "DROP TABLE " + geomTable;
			if(tableExist(geomTable))
			{
				if (!this->execute(sql) )
				{
					delete portal;
					return false;
				}
			}
			
			sql= "DROP SEQUENCE "+ getNameSequence(geomTable);
			this->execute(sql);
		}
	}

	portal->freeResult();
	if (existRelation("te_representation","fk_rep_layer_id") != TeRICascadeDeletion)
	{
		// Delete entries into representations table
		sql = "DELETE FROM te_representation WHERE layer_id = " +Te2String(layerId);
		if (!this->execute(sql) )
		{
			delete portal;
			return false;
		}
	}

	// delete layer themes
	sql = "SELECT theme_id FROM te_theme WHERE layer_id=" + Te2String(layerId);
	if (!portal->query (sql))
	{
		delete portal;
		return false;
	}
	
	int themeId;
	while (portal->fetchRow())
	{	
		themeId = atoi(portal->getData("theme_id"));
		this->deleteTheme(themeId);
	}
	
	sql = "DELETE FROM te_layer WHERE layer_id=" + Te2String(layerId);
	if (!this->execute(sql))
	{
		delete portal;
		return false;
	}

	// delete layer projection
	sql = "DELETE FROM te_projection WHERE projection_id = "+ projId;
	if (!this->execute(sql))
	{	
		delete portal;
		return false;
	}

	// remove all the items´ themes associated to the layer to be removed
	TeThemeMap::iterator it;
	for (it = themeMap().begin(); it != themeMap().end(); ++it)
	{
		TeTheme *theme = (TeTheme*)it->second;
		if (theme->layer()->id() == layerId)
		{
			themeMap().erase(theme->id());
			delete theme;
		}
	}

	// Delete layer and its entry in the layer map
	TeLayer* layer = layerMap()[layerId];
	layerMap().erase(layerId);
	delete layer;

	delete portal;
	return true;
}

bool
TeOCIOracle::insertTheme (TeAbstractTheme *theme)
{
	double maxScale = theme->maxScale ();
	if(maxScale==TeMAXFLOAT)
		maxScale = 0.;

	string ins = "INSERT INTO te_theme (";
	ins += " theme_id, ";
	ins += " layer_id, view_id, name, ";
	ins += " parent_id, priority, node_type, min_scale, max_scale, ";
	ins += " generate_attribute_where, generate_spatial_where, generate_temporal_where, ";
	ins += " collection_table, visible_rep, enable_visibility, lower_x, lower_y, upper_x, upper_y, creation_time) ";
	ins += " VALUES (";
	ins += "te_theme_seq.NEXTVAL";
	
	if(theme->type()==TeTHEME)
		ins += ", " + Te2String(static_cast<TeTheme*>(theme)->layerId());
	else
		ins += ", NULL ";
	
	ins += ", " + Te2String(theme->view());
	ins += ", '" + escapeSequence(theme->name()) + "'";
	ins += ", " + Te2String(theme->parentId ());
	ins += ", " + Te2String(theme->priority());
	ins += ", " + Te2String(theme->type ());
	ins += ", " + Te2String (theme->minScale(),15);
	ins += ", " + Te2String (maxScale,15);
	ins += ", '" + escapeSequence(theme->attributeRest()) + "'";
	ins += ", '" + escapeSequence(theme->spatialRest()) + "'";
	ins += ", '" + escapeSequence(theme->temporalRest()) + "'";
	
	if(theme->type()==TeTHEME)
		ins += ", '" + escapeSequence(static_cast<TeTheme*>(theme)->collectionTable()) + "'";
	else
		ins += ", NULL ";

	ins += ", " + Te2String(theme->visibleRep());
	ins += ", " + Te2String(theme->visibility());
	ins += ", " + Te2String (theme->box().x1(),15);
	ins += ", " + Te2String (theme->box().y1(),15);
	ins += ", " + Te2String (theme->box().x2(),15);
	ins += ", " + Te2String (theme->box().y2(),15);
	TeTime creationTime = theme->getCreationTime();
	ins += ", " + getSQLTime(creationTime);
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_theme!";   
		return false;
	}

	TeOCIOraclePortal  *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		errorMessage_ = "Error in the sequence te_theme_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0)); 
	theme->id(index);
	delete ocip;	

	//insert collection name
	if((theme->type() == TeTHEME || theme->type()==TeEXTERNALTHEME)&& static_cast<TeTheme*>(theme)->collectionTable().empty())
	{
		string colName = "te_collection_" + Te2String(index);
		static_cast<TeTheme*>(theme)->collectionTable(colName);	
		
		ins = "UPDATE te_theme SET ";
		ins += " collection_table = '" + escapeSequence(colName) + "'";
		ins += " WHERE theme_id = " + Te2String (index);
		
		if(!execute(ins))
		{
			errorMessage_ = "Error inserting in the table te_theme!";   
			return false;
		}
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


bool 
TeOCIOracle::insertThemeTable (int themeId, int tableId, int relationId, int tableOrder)
{
	string ins = "INSERT INTO te_theme_table ( ";
	ins += " theme_table_id, ";
	ins += " theme_id, table_id, relation_id, table_order)";
	ins += " VALUES ( ";
	ins += getNameSequence("te_theme_table") +".NEXTVAL ";
	ins += ", "+ Te2String(themeId);
	ins += ", "+ Te2String(tableId);
	
	if(relationId>0)
		ins += ", "+ Te2String(relationId);
	else
		ins += ", null ";

	ins += ", "+ Te2String(tableOrder);
	ins += ")";

	if(!execute(ins))
	{
		errorMessage_ = "Error inserting in the table te_theme_table!";   
		return false;
	}

	return true;
}

bool 
TeOCIOracle::insertThemeGroup(TeViewTree* tree)
{
	string ins = "INSERT INTO te_theme (theme_id, view_id, name, ";
	ins += " parent_id, priority, node_type ) VALUES (";
	ins += "te_theme_seq.NEXTVAL";
	ins += ", " + Te2String(tree->view());
	ins += ", '" + escapeSequence(tree->name()) + "'";
	ins += ", " + Te2String(tree->parentId ());
	ins += ", " + Te2String(tree->priority());
	ins += ", " + Te2String(1);
	ins += ")";

	TeOCIOraclePortal  *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;
	
	try
	{
		if(!execute(ins))
		{
			errorMessage_ = "Error inserting in the table te_theme!";   
			delete ocip;
			return false;
		}

		string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
		if(!ocip->query(seq))
		{
			errorMessage_ = "Error in the sequence te_theme_seq!";  
			delete ocip;
			return false;
		}

		if(!ocip->fetchRow())
		{
			errorMessage_ = "Sequence value not found!";
			delete ocip;
			return false;
		}

		int index = atoi((const char*)ocip->getData(0)); 
		tree->id(index);
		
	}
	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_theme!";
		if(ocip)
			delete ocip;
		return false;
	}

	delete ocip;
	return true;
}

bool 
TeOCIOracle::generateLabelPositions (TeTheme *theme, const std::string& objectId)
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if( theme->layer()->hasGeometry(TeCELLS)    || 
		theme->layer()->hasGeometry(TePOLYGONS) ||
		theme->layer()->hasGeometry(TeLINES) )
	{
		geomTable = theme->layer()->tableName(TeCELLS);
		
		if(geomTable.empty())
		{
			geomTable = theme->layer()->tableName(TePOLYGONS);
			if(geomTable.empty())
				geomTable = theme->layer()->tableName(TeLINES);
		}

		upd= "UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(lower_x + (upper_x - lower_x)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(lower_y + (upper_y - lower_y)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}
	else if(theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);
		
		upd= " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(x) ";
		upd += " FROM " + geomTable + " p WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(y) ";
		upd += " FROM " + geomTable + " p WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
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

bool 
TeOCIOracle::insertView(TeView *view)
{
	// save it´s projection
	TeProjection* proj = view->projection();
	if ( !proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir vista sem projeção";
		return false;
	}

	string ins = "INSERT INTO te_view (view_id, projection_id, name, user_name, visibility, lower_x, lower_y, upper_x, upper_y, current_theme)";
	ins += " VALUES (";
	ins += "te_view_seq.NEXTVAL";
	ins += ", " + Te2String(proj->id());
	ins += ", '" + escapeSequence(view->name ()) + "'";
	ins += ", '" + escapeSequence(view->user ()) + "'";
	ins += ", " + Te2String((int)view->isVisible());
	ins += ", " + Te2String(view->getCurrentBox().lowerLeft().x(),15);
	ins += ", " + Te2String(view->getCurrentBox().lowerLeft().y(),15);
	ins += ", " + Te2String(view->getCurrentBox().upperRight().x(),15);
	ins += ", " + Te2String(view->getCurrentBox().upperRight().y(),15);
	if(view->getCurrentTheme() == -1)
		ins += ", null";
	else	
	ins += ", " + Te2String(view->getCurrentTheme());
	ins += " )";
	
	if(!execute (ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_view!";   
		return false;
	}

	TeOCIOraclePortal *ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT te_view_seq.CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence te_view_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0));
	view->id(index);
	delete ocip;

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

	// Insert view in the view map
	viewMap()[view->id()] = view;
	return true;
}

bool
TeOCIOracle::insertViewTree (TeViewTree *tree)
{
	
	string ins = "INSERT INTO te_theme (theme_id, view_id, name, ";
	ins += " parent_id, node_type, priority) VALUES (";
	ins += " te_theme_seq.NEXTVAL";
	ins += ", " + Te2String(tree->view());
	ins += ", '" + escapeSequence(tree->name()) + "'";
	ins += ", " + Te2String(tree->parentId());
	ins += ", " + Te2String(tree->type());
	ins += ", " + Te2String(tree->priority());
	ins += ")";
	
	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_theme!";
		return false;
	}

	TeOCIOraclePortal	*ocip = (TeOCIOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		errorMessage_ = "Error in the sequence te_theme_seq!";  
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ocip;
		return false;
	}

	int index = atoi((const char*)ocip->getData(0)); 
	tree->id(index);
	delete ocip;
	
	return true;
}

string TeOCIOracle::concatValues(vector<string>& values, const string& unionString)
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

bool TeOCIOracle::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	attList.clear();

	std::string owner = this->user();
	std::string plainTableName = tableName;
	std::set<std::string> setPrimaryKeys;
		
	std::vector<std::string> vecTableName;
	TeSplitString(tableName, ".", vecTableName);

	if(vecTableName.size() == 2)
	{
		owner = vecTableName[0];
		plainTableName = vecTableName[1];		
	}

	std::string sqlKey = "SELECT c.column_name as col_primary from ALL_CONS_COLUMNS c"; 
	sqlKey += " JOIN ALL_CONSTRAINTS ac ON c.constraint_name = ac.constraint_name";
	sqlKey += " WHERE upper(c.table_name) = upper('" + plainTableName + "')";
	sqlKey += "	AND upper(c.OWNER) = upper('"+ owner + "') and constraint_type = 'P'";

	std::string sqlColumns = "SELECT COLUMN_NAME, DATA_TYPE, DATA_LENGTH, DATA_SCALE";
	sqlColumns += " FROM ALL_TAB_COLUMNS";
	sqlColumns += " WHERE upper(TABLE_NAME) = upper('" + plainTableName + "')";
	sqlColumns += "	AND upper(OWNER) = upper('"+ owner + "')";

	TeDatabasePortal* portal = this->getPortal();
	if(!portal)
		return false;
	
	if(!portal->query(sqlKey))
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
	{
		setPrimaryKeys.insert(portal->getData(0));
	}
	
	portal->freeResult();
		
	if(!portal->query(sqlColumns))
	{
		delete portal;
		return false;
	}
	
	while(portal->fetchRow())
	{
		TeAttribute at;
		at.rep_.name_ = portal->getData(0);
		string type = TeConvertToUpperCase(portal->getData(1));
		int scale = portal->getInt(3);
		if((type=="NUMBER" && scale>0) || (type=="FLOAT"))
	        at.rep_.type_ = TeREAL;
		else if(type=="NUMBER")
			at.rep_.type_ = TeINT;
		else if(type=="VARCHAR2")
		{
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_=portal->getInt(2);
		}
		else if(type=="BLOB")
			at.rep_.type_ = TeBLOB;
		else if(type=="CHAR")
			at.rep_.type_ = TeCHARACTER;
		else if(type=="DATE")
			at.rep_.type_ = TeDATETIME;
		else 
		{
			at.rep_.type_ = TeSTRING;
			at.rep_.numChar_=portal->getInt(2);
		}

		if(setPrimaryKeys.find(at.rep_.name_) != setPrimaryKeys.end())
		{
			at.rep_.isPrimaryKey_ = true;
		}

		attList.push_back(at);
	}
	delete portal;
	return true;
}

string TeOCIOracle::toUpper(const string& value)
{
	string result  = "upper(";
	       result += value;
		   result += ")";

	return result;
}

string  
TeOCIOracle::getSQLTime(const TeTime& time) const
{
	string dateTime = time.getDateTime();	
	string result;
	result = "TO_DATE('";
	result += dateTime;
	result += "  ";
	result += "','DD/MM/YYYY HH24:MI:SS')"; 
	return result;
}


bool
TeOCIOracle::getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes)
{
	std::vector<TeDatabaseIndex> indexes;

	string query = "select index_name, column_name from ALL_IND_COLUMNS  where table_name = '"+tableName+"' ";
	query += "and index_name not like ( select constraint_name from ALL_CONSTRAINTS "; 
	query += "where table_name = '"+tableName+"' and constraint_type = 'P') order by index_name";
		
	TeDatabasePortal* portal = getPortal();

	std::vector<std::string> names;
	std::vector<std::string> cols;

	std::set<std::string> idxNames;

	std::multimap<std::string, std::string> idxAttrs;
	
	if(portal->query(query))
	{
		while(portal->fetchRow())
		{
			std::string idxName = portal->getData("index_name");
			std::string attrName = portal->getData("column_name");

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


//---------------------------------- Oracle portal

TeOCIOraclePortal::TeOCIOraclePortal(TeOCIOracle *pDatabase) 
{
	cursor_ = new TeOCICursor(pDatabase->connection_);
	db_ = pDatabase;
	numRows_ = 0;
	numFields_ = 0;
	curRow_ = 0;
}


TeOCIOraclePortal::~TeOCIOraclePortal () 
{
	if(cursor_)
		delete cursor_;
	cursor_ = NULL;
}


bool TeOCIOraclePortal::isConnected()
{
	return (cursor_->conn()->isConnected());
}


bool TeOCIOraclePortal::isEOF() 
{
	if (!isConnected())
		return true;

	return false;
}

bool
TeOCIOraclePortal::getLinearRing (TeLine2D& line)
{

	double	*points	= NULL;
	try
	{
		int index = atoi (getData ("geom_id"));
		TeBox b (getDouble("lower_x"),getDouble("lower_y"),getDouble("upper_x"),getDouble("upper_y"));
		line.objectId (string (getData ("object_id")));
		line.geomId (index);
		line.setBox (b);

		unsigned int ncoords = getInt("num_coords");
		unsigned int nords = ncoords*2;
			
		if (!cursor_->readBlob(&points))
			return false;
		
		line.clear();

		for (unsigned int i=0;i<nords;++i) 
		{
			double x = points[i];
			++i;
			double y = points[i];
			
			TeCoord2D c = TeCoord2D(x,y);
			line.add(c);
		}
	}
	catch(...)
	{
		errorMessage_ = "Error getting media!";
		return false;
	}
	return true;
}

bool
TeOCIOraclePortal::getLinearRing (TeLine2D& line, const unsigned int& initIndex)
{
	double	*points	= NULL;
	try
	{
		line.objectId (string (getData (initIndex+1)));
		line.geomId (atoi (getData(initIndex)));
		
		unsigned int ncoords = getInt(initIndex+2);
		unsigned int nords = ncoords*2;
			
		if (!cursor_->readBlob(&points))
			return false;
		
		line.clear();

		for (unsigned int i=0;i<nords;++i) 
		{
			double x = points[i];
			++i;
			double y = points[i];
			
			TeCoord2D c = TeCoord2D(x,y);
			line.add(c);
		}
	}
	catch(...)
	{
		errorMessage_ = "Error getting media!";
		return false;
	}
	return true;
}


bool
TeOCIOraclePortal::moveFirst() 
{
	if (cursor_->moveFirst())
		return true;
	else
		return false;
}


bool 
TeOCIOraclePortal::moveNext() 
{
	if(cursor_->moveNext())
		return true;
	return false;
}

bool TeOCIOraclePortal::query (const string &q,TeCursorLocation /* l */ , TeCursorType /* t */, TeCursorEditType /* e */, TeCursorDataType /* dt */)  //Ok
{
	errorMessage_.clear ();

	if (!cursor_->isOpen())
	{
		if(!cursor_->open())
		{
			numRows_ = 0;
			return false;
		}
	}

	if (!cursor_->query(q))
	{
		this->errorMessage_ = cursor_->getErrorMessage();
		numRows_ = 0;
		return false;
	}
	
	numFields_= this->cursor_->numCol();

	attList_.clear ();
	int i;
	for(i = 1; i <= numFields_ ; i++)
	{
		TeAttribute attribute;

		switch (cursor_->colType(i))
		{
			case 3: //INTEGER
				attribute.rep_.type_ = TeINT;
			break;

			case 2:  //NUMBER
			case 4: //FLOAT DOUBLE
				attribute.rep_.type_ = TeREAL;
			break;

			case 12: //Date
			attribute.rep_.type_ = TeDATETIME;
			attribute.dateChronon_ = TeSECOND;
			attribute.dateTimeFormat_ = "DD/MM/YYYY HH24:MI:SS";
			break;

			case 113: //Blob
			attribute.rep_.type_ = TeBLOB;
			break;

			case 96: //CHAR
			case 9: //VARCHAR:
			case 1: //VARCHAR2:
			attribute.rep_.type_ = TeSTRING;
			break;

			case 108: //OBJECT: // SDO_GEOMETRY
			attribute.rep_.type_ = TeOBJECT;
			break;
			default :
			attribute.rep_.type_ = TeUNKNOWN;
			break;
		} 
		
		//attribute.rep_.name_ = TeConvertToLowerCase(cursor_->ColName(i)); 
		attribute.rep_.name_ = cursor_->colName(i); 
		attribute.rep_.numChar_ = cursor_->colSize(i);
		attList_.push_back ( attribute );
	}
	curRow_=-1;
	return true;
}


void TeOCIOraclePortal::freeResult () 
{
	cursor_->freeResult();
}


bool TeOCIOraclePortal::fetchRow () 
{
	try
	{	
		if( !isConnected() )
			return false;
		if( numFields_ == 0)
			return false;
		
		if (curRow_ == -1)
		{
			if(moveFirst())
				curRow_++;
			else
				return false;
		}
		else
		
		if (moveNext())
		{
			curRow_++;
			return true;
		}
		else
			return false;
	}
	
	catch(...) //_com_error &e)
	{
		return false;
	}

	return true;
}

bool TeOCIOraclePortal ::fetchRow (int i)
{
	try
	{	
		if( !isConnected() || (numFields_ == 0))
			return false;
		
		if (curRow_ == -1)
		{
			if(moveFirst())
				curRow_++;
			else
				return false;
		}

		if(curRow_ == i)
			return true;

		if (cursor_->moveTo(i))
		{
			curRow_ = cursor_->currentRow();
			return true;
		}
		return false;
	}
	catch(...)
	{
		return false;
	}
}

char* TeOCIOraclePortal::getData (int i) 
{
	char* result;
	if (i > numFields_ || i < 0)
		return "";
	
	try
	{
		result = cursor_->getFieldValue(i+1);
	}
	catch(...)
	{
		errorMessage_ = "Error!";
		return "";
	}
	return result;
	
}

char* TeOCIOraclePortal::getData (const string &s)
{
	char* result;

	string fieldName;
	size_t pos = s.find(".", 0, 1);
	if (pos != string::npos)
		fieldName = s.substr(pos+1);
	else
		fieldName = s;

	int index = getColumnIndex (fieldName);
	if(index == -1)
		return "";
	else
		result = getData(index);

	return result;
}


int TeOCIOraclePortal::getInt (int i) 
{
	int value = atoi(getData(i));
	return value;
}

int  
TeOCIOraclePortal::getInt (const string& s)
{
	int value = atoi(getData(s));
	return value;
}

double TeOCIOraclePortal ::getDouble (int i) 
{
	string strValue=getData(i);
	size_t find;
	find = strValue.find(",");
	if(find<strValue.size())
		strValue.replace(find, 1, ".");
		
	return atof(strValue.c_str());
}

double  
TeOCIOraclePortal::getDouble (const string& s)
{
	string strValue=getData(s);
	size_t find;
	find = strValue.find(",");
	if(find<strValue.size())
		strValue.replace(find, 1, ".");
	
	return atof(strValue.c_str());
}


bool
TeOCIOraclePortal::getBool(int i)
{
	char* fieldChar=0;
	
	fieldChar = cursor_->getFieldValue(i+1);
	if(fieldChar == 0)
		return false;
	
	return true;
}


bool    
TeOCIOraclePortal::getBool (const string& s)
{
	string fieldName;
	size_t pos = s.find(".", 0, 1);
	if (pos != string::npos)
		fieldName = s.substr(pos+1);
	else
		fieldName = s;

	int index = getColumnIndex (fieldName);
	if(index == -1)
		return false;
	return (getBool(index));
}

bool		
TeOCIOraclePortal::getBlob(const string& s, unsigned char* &data, long& size)
{
	try
	{
		unsigned int len = cursor_->getBlobSize(s);
		data = new unsigned char[len];
		if(!cursor_->readBlob(data, len, s))
			return false;
		size = len;
	}
	catch(...)
	{
		errorMessage_ = "Error getting media!";
		return false;
	}
	return false;
}

TeTime 
TeOCIOraclePortal::getDate (int i) 
{ 
	TeTime temp;
	string result;

	if (i > numFields_ || i < 0)
		return temp;
	
	try
	{
		result = cursor_->getFieldValue(i+1);
	}
	catch(...)
	{
		errorMessage_ = "Error!";
		return temp;
	}
		
	TeTime t(result, TeSECOND, "DDsMMsYYYYsHHsmmsSS");
	return t;
}
	

TeTime 
TeOCIOraclePortal::getDate (const string& s) 
{ 
	string  result;
	TeTime temp;

	string fieldName;
	size_t pos = s.find(".", 0, 1);
	if (pos != string::npos)
		fieldName = s.substr(pos+1);
	else
		fieldName = s;

	int index = getColumnIndex (fieldName);
	if(index == -1)
		return temp;
	else
		result = getData(index);

	TeTime t(result, TeSECOND, "DDsMMsYYYYsHHsmmsSS") ;
	return t;
}

string
TeOCIOraclePortal::getDateAsString(int i)
{
	TeTime t = this->getDate(i);
	string date = t.getDateTime ();

	if (!date.empty())
	{		string tval = " TO_DATE ('"+ date +"','DDsMMsYYYYsHHsmmsSS') ";
		return tval;
	}
	else
		return "";
}

string 
TeOCIOraclePortal::getDateAsString(const string& s)
{
	TeTime t = this->getDate(s);
	string date = t.getDateTime ();

	if (!date.empty())
	{		string tval = " TO_DATE ('"+ date +"','DDsMMsYYYYsHHsmmsSS') ";
		return tval;
	}
	else
		return "";
}


bool
TeOCIOraclePortal::fetchGeometry (TePolygon& poly)
{
	try 
	{
		TeLine2D line;
		int numHoles = atoi(this->getData("num_holes"));
		bool flag = this->fetchGeometry(line);

		TeLinearRing ring(line);
		poly.objectId ( ring.objectId());
		poly.geomId ( ring.geomId() );
		poly.add ( ring );
		int parentId = poly.geomId();

		while (flag && numHoles>0 && (atoi(this->getData("parent_id")) == parentId)) // get holes
		{
			TeLine2D aux;
			this->getLinearRing(aux);
			TeLinearRing hole(aux);
			poly.add (hole);
			flag = this->fetchRow();
		}
		return flag;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool
TeOCIOraclePortal::fetchGeometry (TePolygon& poly, const unsigned int& initIndex)
{
	try 
	{
		TeLine2D line;
		int numHoles = atoi(this->getData(initIndex+3));
		getLinearRing(line, initIndex);
		bool flag = this->fetchRow();
		
		TeLinearRing ring(line);
		poly.objectId ( ring.objectId());
		poly.geomId ( ring.geomId() );
		poly.add ( ring );
		int parentId = poly.geomId();

		while (flag && numHoles>0 && (atoi(this->getData(initIndex+4)) == parentId)) // get holes
		{
			TeLine2D aux;
			this->getLinearRing(aux, initIndex);
			TeLinearRing hole(aux);
			poly.add (hole);
			flag = this->fetchRow();
		}
		return flag;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeOCIOraclePortal::fetchGeometry (TeLine2D& line)
{	
	try
	{
		getLinearRing(line);
		return (this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOCIOraclePortal::fetchGeometry (TeLine2D& line, const unsigned int& initIndex)
{	
	try
	{
		getLinearRing(line, initIndex);
		return (this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}


bool 
TeOCIOraclePortal::fetchGeometry(TePoint& p)
{

	try 
	{
		p.geomId( atoi(getData("geom_id")));
		p.objectId( string(getData("object_id")));
		double x = getDouble("x");
		double y = getDouble("y");
		TeCoord2D c(x, y);
		p.add(c);
		return(this->fetchRow());
	} 
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeOCIOraclePortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	try 
	{
		p.geomId( atoi(getData(initIndex)));
		p.objectId( string(getData(initIndex+1)));
		double x = getDouble(initIndex+2);
		double y = getDouble(initIndex+3);
		TeCoord2D c(x, y);
		p.add(c);
		return(this->fetchRow());
	} 
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeOCIOraclePortal::fetchGeometry(TeNode& n)
{
	try 
	{
		TeCoord2D c(getDouble("x"), getDouble("y"));
		n.geomId( atol(getData("geom_id")));
		n.objectId( string(getData("object_id")));
		n.add(c);
		return(this->fetchRow());
	} 
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	return true;
}

bool 
TeOCIOraclePortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	try 
	{
		TeCoord2D c(getDouble(initIndex+2), getDouble(initIndex+3));
		n.geomId( atol(getData(initIndex)));
		n.objectId( string(getData(initIndex+1)));
		n.add(c);
		return(this->fetchRow());
	} 
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	return true;
}

bool
TeOCIOraclePortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{
	// gets the spatial data
	// expects that the data size will be a block of double maximum
	try
	{
		unsigned int len = 0;
		if(!getCursor()->readBlob(ptData, len, "spatial_data"))
			return false;
		size = len;
	}
	catch(...)
	{
		size = 0;
		errorMessage_ = "ERRO!";
		return false;
	}
	return true;
}




