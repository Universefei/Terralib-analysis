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

#include <stdio.h>
#include <direct.h>
#include <sys/stat.h>
#include <TeAdoOracle.h>
#include <sys/stat.h>
#include <TeProject.h>

#include <TeUtils.h>

#define CHUNKSIZE	240

typedef map<int,TeNode> TeNodeMap;

inline void TESTHR( HRESULT hr )
{
	if( FAILED(hr) ) _com_issue_error( hr );
}

TeOracle::TeOracle()  
{
    HRESULT hr = CoInitialize(0);
	dbmsName_ = "OracleAdo";
    if(FAILED(hr))
    {
        cout << "Can't start COM!? " << endl;
    }
}

TeOracle::~TeOracle()
{
}

bool 
TeOracle::newDatabase(const string& database, const string& user, const string& password, 
					  const string& host, const int& port, bool terralibModel)
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
TeOracle::connect (const string& host, const string& user, const string& password, const string& database, int /* port */)
{
	
	string  connectionString = "Provider = OraOLEDB.Oracle.1;FetchSize=300;Password="+password+";Persist Security Info=True;User ID="+user+";Data Source="+database+"";

	try
	{
		connection_.CreateInstance(__uuidof(ADODB::Connection));
		HRESULT hr  = connection_->Open (connectionString.c_str(),"","",-1);
		TESTHR( hr );
	}
	catch(_com_error &e)
	{
		isConnected_ = false;
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		isConnected_ = false;
		errorMessage_ = "Oppps !";
		return false;
	}
	isConnected_ = true;
	database_ = database;
	host_ = host;
	user_ = user;
	password_ = password;
	return true;
}


bool
TeOracle::tableExist(const string& table)
{
	if (table.empty())
		return false;
	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
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
	{
		delete ocip;
		return true;
	}
	else
	{
		delete ocip;
		return false;
	}
}


bool 
TeOracle::listTables(vector<string>& tableList)
{
	tableList.clear();
	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
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
TeOracle::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
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
		delete ocip;
		return true;
	}
	delete ocip;
	return false;
}

bool
TeOracle::createTable(const string& table, TeAttributeList &attr)
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
TeOracle::deleteTable (const string& table)
{
//	int f =	table.find ("te_collection", std::string::npos);
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

	string del = "DROP TABLE " + table;
	if(tableExist(table))
	{
		if(!execute(del))
			return false;
	}

	string seq = "DROP SEQUENCE " + getNameSequence(table);
	execute(seq);
		
	string trig = "DROP TRIGGER " + getNameTrigger(table);
	execute(trig);
		
	return true;
}

bool 
TeOracle::addColumn (const string& table, TeAttributeRep &rep)
{
	if(!tableExist(table))
		return false;

	string tab;
	tab = " ALTER TABLE " + table + " ADD ( ";
	tab += rep.name_ + "  ";
	
	switch (rep.type_)
	{
		case TeSTRING:
			if (rep.numChar_ > 0)
				tab  += "VARCHAR2(" + Te2String(rep.numChar_) + ") ";
			else
				tab += "VARCHAR2(4000) ";
			break;
		
		case TeREAL:
			if(rep.decimals_>0)
				tab += " NUMBER(*," + Te2String(rep.decimals_) +") ";
			else
				tab += " NUMBER(*,38) ";
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
	if (rep.isAutoNumber_)
		tab += " AUTO_INCREMENT)";
	else
		tab += " NULL)";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting a column to table " + table + " !";
		return false;
	}

	alterTableInfoInMemory(table);
	return true;
}

bool 
TeOracle::alterTable (const string& table, TeAttributeRep &rep, const string& oldColName)
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

	string tab = " ALTER TABLE " + table + " MODIFY ( ";
	tab += rep.name_ + "  ";
	switch (rep.type_)
	{
		case TeSTRING:
			if ( rep.numChar_ > 0)
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
TeOracle::alterTable(const string& oldTableName, const string& newTablename)
{
	string sql = " ALTER TABLE "+ oldTableName +" RENAME TO "+ newTablename;
	if(!this->execute(sql))
		return false;

	//update te_layer_table
	sql = " UPDATE te_layer_table ";
	sql += " SET attr_table = '"+ newTablename +"'";
	sql += " WHERE attr_table = '"+ oldTableName +"'";
	execute(sql);

	//update te_tables_relation
	sql = " UPDATE te_tables_relation ";
	sql += " SET external_table_name = '"+ newTablename +"'";
	sql += " WHERE external_table_name = '"+ oldTableName +"'";
	execute(sql);
	
	alterTableInfoInMemory(newTablename, oldTableName);
	return true;
}


bool
TeOracle::createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
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


TeDBRelationType 
TeOracle::existRelation(const string& tableName, const string& relName)
{

	TeOraclePortal  *ocip = (TeOraclePortal*)getPortal();
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


TeDatabasePortal*  
TeOracle::getPortal ()
{
	TeOraclePortal* portal = new TeOraclePortal (this);
	return portal;
}

bool
TeOracle::createAutoIncrementTrigger(const string &tableName, const string &fieldName)
{
	string nameTri = getNameTrigger(tableName);
	string nameSeq = getNameSequence(tableName);

	string tri;
	tri = "CREATE TRIGGER " + nameTri; 
	tri += " BEFORE INSERT ON "+tableName; 
	tri += " for each row";
	tri += " begin";
	tri += " select "+nameSeq+".nextval";
	tri += " into :new."+fieldName;
	tri += " from dual;";
	tri += " end;";

	if(!execute(tri))
	{	
		if(errorMessage_.empty())
			errorMessage_ = "Error creating trigger to table " + tableName + " !";
		return false;
	}
	return true;
}


bool
TeOracle::createSequence(const string &tableName)
{
	string nameSeq = getNameSequence(tableName);
	
	string seq = " CREATE SEQUENCE " + nameSeq;
	seq += " START WITH 1 INCREMENT BY 1 ORDER ";
	if (!execute(seq))
	{	
		if(errorMessage_.empty())
			errorMessage_ = "Error creating sequence to table " + tableName + " !";
		return false;
	}

	return true;
}

string
TeOracle::getNameSequence(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_seq";
	else
		name = tableName + "_seq";

	return name;
}

string
TeOracle::getNameTrigger(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_tri";
	else
		name = tableName + "_tri";

	return name;
}

bool
TeOracle::deleteLayer(int layerId)
{
	//Delete attributes tables
	if(!deleteLayerTable(layerId))
		return false;

	TeOraclePortal* portal = (TeOraclePortal*) this->getPortal();
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
			TeOraclePortal* portal2 = (TeOraclePortal*)this->getPortal();
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
			if (!this->execute(sql) )
			{
				delete portal;
				return false;
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
TeOracle::insertProjection(TeProjection* proj)
{
	string insert = "INSERT INTO te_projection (projection_id, name, long0, lat0,";
	insert += " offx, offy, stlat1, stlat2, unit, scale, hemis, datum )";
	insert += " VALUES ( ";
	insert += "te_projection_seq.NEXTVAL";
	insert += ", '" + escapeSequence(proj->name()) + "'";
	insert += ", " + Te2String(proj->params().lon0*TeCRD,15);
	insert += ", " + Te2String(proj->params().lat0*TeCRD,15);
	insert += ", " + Te2String(proj->params().offx,15);
	insert += ", " + Te2String(proj->params().offy,15);
	insert += ", " + Te2String(proj->params().stlat1*TeCRD,15);
	insert += ", " + Te2String(proj->params().stlat2*TeCRD,15);
	insert += ", '" + escapeSequence(proj->params().units) + "'";
	insert += ", " + Te2String(proj->params().scale,15);
	insert += ", " + Te2String(proj->params().hemisphere);
	insert += ", '" + escapeSequence(proj->params().datum.name()) + "'";
	insert += ")";

	if(!execute(insert))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_projection!";   
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*) getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_projection_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
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
TeOracle::insertTableInfo(int layerId, TeTable &table, const string& user)
{
	string seq = getNameSequence("te_layer_table");
	string ins = "INSERT INTO te_layer_table ( "; 
	ins += " table_id, layer_id, attr_table, unique_id, attr_link, ";
	ins += " attr_initial_time, attr_final_time, attr_time_unit, ";
	ins += " attr_table_type, user_name) VALUES ( ";
	ins += seq + ".NEXTVAL ";
	
	if(layerId>0)
		ins += ", " + Te2String(layerId);
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
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting tables information!";   
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT "+ seq +".CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence te_layertable_seq!";  
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

bool 
TeOracle::insertRelationInfo(const int tableId, const string& tField,
						  const string& rName, const string& rField, int& relId)
{
	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	relId = -1;

	relId = -1;
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " related_table_id = " + Te2String(tableId);
	sel += " AND related_attr = '" + tField + "'";
	sel += " AND external_table_name = '" + rName + "'";
	sel += " AND external_attr = '" + rField + "'";
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

	string seq, ins;
	seq = getNameSequence("te_tables_relation");	
	ins = " INSERT INTO te_tables_relation ( "; 
	ins += " relation_id, related_table_id, ";
	ins += " related_attr, external_table_name, external_attr) VALUES ( ";
	ins += seq +".NEXTVAL "; 
	ins += ",  " + Te2String(tableId);
	ins += ", '" + escapeSequence(tField) + "'";
	ins += ", '" +  escapeSequence(rName) + "'";
	ins += ", '" +  escapeSequence(rField) + "'";
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting tables information!";   
		delete ocip;
		return false;
	}

	ocip->freeResult();
	ins = "SELECT "+ seq +".CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		if(errorMessage_.empty())
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
TeOracle::insertView (TeView *view)
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
	if(view->getCurrentTheme() > 0)
		ins += ", " + Te2String(view->getCurrentTheme());
	else
		ins += ", null";
	ins += " )";
	
	if(!execute (ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_view!";   
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
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
TeOracle::insertViewTree (TeViewTree *tree)
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

	TeOraclePortal	*ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
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


bool 
TeOracle::insertTheme (TeAbstractTheme *theme)
{

	double maxScale = theme->maxScale ();
	if(maxScale==TeMAXFLOAT)
		maxScale = 0.;

	string ins = "INSERT INTO te_theme (theme_id, layer_id, view_id, name, ";
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
		ins += ", '' ";
	
	ins += ", " + Te2String(theme->visibleRep());
	ins += ", " + Te2String(theme->visibility());
	ins += ", " + Te2String (theme->box().x1(),15);
	ins += ", " + Te2String (theme->box().y1(),15);
	ins += ", " + Te2String (theme->box().x2(),15);
	ins += ", " + Te2String (theme->box().y2(),15);
	ins += ", " + getSQLTime(theme->getCreationTime());
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_theme!";   
		return false;
	}

	TeOraclePortal  *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
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
	
	if(theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME)
	{
		string colName = static_cast<TeTheme*>(theme)->collectionTable(); 
		//insert collection name
		if(colName.empty())
		{
			colName = "te_collection_" + Te2String(index);
			static_cast<TeTheme*>(theme)->collectionTable(colName);	
			
			ins = "UPDATE te_theme SET ";
			ins += " collection_table = '" + escapeSequence(colName) + "'";
			ins += " WHERE theme_id = " + Te2String (index);
			
			if(!execute(ins))
			{
				if(errorMessage_.empty())
					errorMessage_ = "Error inserting in the table te_theme!";   
				return false;
			}
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
TeOracle::insertThemeTable (int themeId, int tableId, int relationId, int tableOrder)
{
	string ins = "INSERT INTO te_theme_table ";
	ins += " (theme_table_id, theme_id, table_id, relation_id, table_order)";
	ins += " VALUES ( ";
	ins += getNameSequence("te_theme_table") +".NEXTVAL ";
	ins += ", "+ Te2String(themeId);
	ins += ", "+ Te2String(tableId);
	
	if(relationId>0)
		ins += ", "+ Te2String(relationId);
	else
		ins += ", NULL ";

	ins += ", "+ Te2String(tableOrder);
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_theme_table!";   
		return false;
	}

	return true;
}

bool 
TeOracle::insertThemeGroup(TeViewTree* tree)
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

	TeOraclePortal  *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;
	
	try
	{
		if(!execute(ins))
		{
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_theme!";   
			return false;
		}

		string seq = "SELECT te_theme_seq.CURRVAL FROM DUAL";
		if(!ocip->query(seq))
		{
			if(errorMessage_.empty())
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
TeOracle::generateLabelPositions (TeTheme *theme, const std::string& objectId )
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if( theme->layer()->hasGeometry(TeCELLS)  )
	{
		geomTable = theme->layer()->tableName(TeCELLS);
		
		upd= "UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(lower_x + (upper_x - lower_x)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(lower_y + (upper_y - lower_y)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}

	if( theme->layer()->hasGeometry(TePOLYGONS) )
	{
		geomTable = theme->layer()->tableName(TePOLYGONS);
		
		upd= "UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(lower_x + (upper_x - lower_x)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(lower_y + (upper_y - lower_y)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}
	
	if( theme->layer()->hasGeometry(TeLINES) )
	{
		geomTable = theme->layer()->tableName(TeLINES);

		upd= "UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(lower_x + (upper_x - lower_x)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(lower_y + (upper_y - lower_y)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}
	
	if(theme->layer()->hasGeometry(TePOINTS))
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

bool TeOracle::updateBBox(const string& tableName, const string& idName, 
							 int idValue, const TeBox& box)
{
	string sql;
	sql = "UPDATE "+tableName+" SET ";
	//the same precision that the insertpoint
	sql += " lower_x = " + Te2String(box.x1(),15) + " ";
	sql += ", lower_y = " + Te2String(box.y1(),15) + " ";
	sql += ", upper_x = " + Te2String(box.x2(),15) + " ";
	sql += ", upper_y = " + Te2String(box.y2(),15) + " ";
	sql += " WHERE "+idName+" = " + Te2String(idValue);
	return (this->execute (sql));
}

bool
TeOracle::updateLayerBox(TeLayer *layer)
{
	if (!layer)
		return false;

	string sql;
	sql = "UPDATE te_layer SET ";
	//the same precision that the insertpoint
	sql += " lower_x = " + Te2String(layer->box().x1(),15) + " ";
	sql += ", lower_y = " + Te2String(layer->box().y1(),15) + " ";
	sql += ", upper_x = " + Te2String(layer->box().x2(),15) + " ";
	sql += ", upper_y = " + Te2String(layer->box().y2(),15) + " ";
	sql += " WHERE layer_id = " + Te2String(layer->id(),15);
	return (this->execute (sql));
}

bool
TeOracle::updateLayer(TeLayer *layer)
{
	if (!layer)
		return false;

	string sql;
	sql = "UPDATE te_layer SET ";
	sql += "name = '" + layer->name() + "' ";
	//the same precision that the insertpoint
	sql += ", lower_x = " + Te2String(layer->box().x1(),15) + " ";
	sql += ", lower_y = " + Te2String(layer->box().y1(),15) + " ";
	sql += ", upper_x = " + Te2String(layer->box().x2(),15) + " ";
	sql += ", upper_y = " + Te2String(layer->box().y2(),15) + " ";
	if(layer->getEditionTime().isValid())
	{
		sql += ", edition_time = " + this->getSQLTime(layer->getEditionTime());
	}
	sql += " WHERE layer_id = " + Te2String(layer->id());

	if (layer->projection())
		updateProjection(layer->projection());
	return (this->execute (sql));
}

bool 
TeOracle::insertProject(TeProject* project)
{
	if (!project)
	{
		errorMessage_ = "Não é possível inserir layer sem projeção";
		return false;
	}
	
	string ins = "INSERT INTO te_project (project_id, name, description, current_view) ";
	ins += " VALUES ( ";
	ins += "te_project_seq.NEXTVAL";
	ins += ", '" + escapeSequence(project->name()) + "'";
	ins += ", '" + escapeSequence(project->description()) + "'";
	ins += ", " + Te2String(project->getCurrentViewId());
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_project!";   
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT te_project_seq.CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		if(errorMessage_.empty())
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
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);
	return true;
}

bool 
TeOracle::insertLayer (TeLayer* layer)
{
	int index;
	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir layer sem projeção";
		return false;
	}
	
	string ins = "INSERT INTO te_layer (projection_id, name ";
	ins += ", lower_x, lower_y, upper_x, upper_y, edition_time) ";
	ins += " VALUES ( ";
	ins += Te2String(proj->id());
	ins += ", '" + escapeSequence(layer->name()) + "'";
	ins += ", " + Te2String(layer->box().x1(),15);
	ins += ", " + Te2String(layer->box().y1(),15);
	ins += ", " + Te2String(layer->box().x2(),15);
	ins += ", " + Te2String(layer->box().y2(),15);
	ins += ", " + getSQLTime(layer->getEditionTime());
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_layer!";   
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	ins = "SELECT te_layer_seq.CURRVAL FROM DUAL";
	if (!ocip->query(ins))
	{
		if(errorMessage_.empty())
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
TeOracle::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;
	
	string ins;
	TeOraclePortal *ocip = (TeOraclePortal*) getPortal();
	if(!ocip)
		return false;

	try
	{
		ins = " INSERT INTO te_representation (repres_id, layer_id, geom_type, geom_table, ";
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
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_representation!";   
			return false;
		}
		
		string seq = "SELECT te_representation_seq.CURRVAL FROM DUAL";
		if(!ocip->query(seq))
		{
			if(errorMessage_.empty())
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
	}

	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_representation!"; 
		delete ocip;
		return false;
	}

	delete ocip;
	return true;
}	


bool 
TeOracle::insertLegend (TeLegendEntry* leg)
{
	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	try
	{
		string ins = "INSERT INTO te_legend (legend_id, theme_id, group_id, ";
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
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_legend!"; 
			return false;
		}

		string seq = "SELECT te_legend_seq.CURRVAL FROM DUAL";
		if(!ocip->query(seq))
		{
			if(errorMessage_.empty())
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
	}
	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_legend!"; 
		delete ocip;
		return false;
	}
		
	delete ocip;
	legendMap()[leg->id()] = leg;
	return insertVisual(leg);
}

bool 
TeOracle::insertPolygonSet (const string& table, TePolygonSet &ps)
{
	
	for (unsigned int i = 0; i < ps.size(); i++ )
	{
		TePolygon& poly = ps [i];
		if (!insertPolygon (table,poly))
			return false;
	}
	return true;
}


bool 
TeOracle::insertPolygon (const string& table, TePolygon &poly)
{
	
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	
	recset_->CursorLocation=ADODB::adUseServer;
	
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenKeyset, 
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);
	
	TeOraclePortal  *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	double	extmax;
	unsigned int k, ni, size;
//	double	*points	= NULL;

	try
	{
		int parentId = 0;
		for ( k = 0; k < poly.size(); k++ )
		{
			TeLinearRing ring ( poly[k] );
			TeBox b = ring.box();
			size = ring.size();
			ni = 0;
			extmax = MAX(b.width(),b.height());
			if (k==0)
				ni = poly.size()-1;
			else
				ni = 0;
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (poly.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)ni);
			recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(b.lowerLeft().x(),15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(b.lowerLeft().y(),15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(b.upperRight().x(),15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(b.upperRight().y(),15));
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

			// Create and save the BLOB
			VARIANT varPoints;

			if(!LinearRingToVariant(varPoints, ring))
			{
				recset_->Close();
				delete ocip;
				return false;
			}

			//recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);
			recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			recset_->Update();
			
			string seq = "SELECT " + getNameSequence(table)+ ".CURRVAL FROM DUAL";
			if(!ocip->query(seq))
			{
				if(errorMessage_.empty())
					errorMessage_ = "Error in the sequence " + table+ "!";  
				delete ocip;
				recset_->Close();
				return false;
			}

			if(!ocip->fetchRow())
			{
				errorMessage_ = "Sequence value not found!";
				recset_->Close();
				delete ocip;
				return false;
			}

			int index = atoi((const char*)ocip->getData(0)); 
			ring.geomId (index);
			ocip->freeResult();
			
			//ring.geomId (recset_->GetCollect("geom_id").intVal);			
			SafeArrayDestroy (varPoints.parray);
			if (k==0)
			{
				parentId = ring.geomId();
				string sql = "UPDATE "+table+" SET parent_id  ="+Te2String(parentId)+" WHERE geom_id = "+Te2String(ring.geomId());
				if(!execute(sql))
				{
					if(errorMessage_.empty())
						errorMessage_ = "Error inserting in table " + table;
					delete ocip;
					recset_->Close();
					return false;
				}
				
			}
			
		} //for
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		delete ocip;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		delete ocip;
		return false;
	}

	delete ocip;
	recset_->Close();
	return true;
}

bool 
TeOracle::insertLineSet(const string& table, TeLineSet &ls)
{		
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
		
	recset_->CursorLocation=ADODB::adUseServer;
	recset_->CacheSize = 300;
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenStatic,	
		ADODB::adLockBatchOptimistic,
		ADODB::adCmdTable);
	
	double	extmax;
	int		size;
//	double	*points	= NULL;

	for ( unsigned int i = 0; i < ls.size(); i++ )
	{
		TeLine2D& line = ls [i];
		try
		{
			size = line.size();
			TeBox b = line.box();
			extmax = MAX(b.width(),b.height());
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(b.lowerLeft().x(),15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(b.lowerLeft().y(),15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(b.upperRight().x(),15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(b.upperRight().y(),15));
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

			// Create and save the BLOB
			VARIANT varPoints;

			if (!LineToVariant(varPoints, line))
			{
				recset_->Close();
				return false;
			}

			recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			//recset_->Update();
			
			SafeArrayDestroy (varPoints.parray);
		}
		catch(_com_error &e)
		{
			errorMessage_ = e.Description();
			recset_->Close();
			return false;
		}
		catch(...)
		{
			errorMessage_ = "Oppps !";
			recset_->Close();
			return false;
		}
	}
	
	recset_->UpdateBatch(ADODB::adAffectAll);
	recset_->Close();
	return true;
}


bool 
TeOracle::insertLine (const string& table, TeLine2D &line)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	
	recset_->CursorLocation=ADODB::adUseServer;
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenKeyset, 
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);

	double	extmax;
	int		size;
//	double	*points	= NULL;

	try
	{
		size = line.size();
		TeBox b = line.box();
		extmax = MAX(b.width(),b.height());
		recset_->AddNew();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
		recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(b.lowerLeft().x(),15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(b.lowerLeft().y(),15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(b.upperRight().x(),15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(b.upperRight().y(),15));
		recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);
		
		// Create and save the BLOB
		VARIANT varPoints;
		if (!LineToVariant(varPoints, line))
		{
			recset_->Close();
			return false;
		}
		
		//recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);		
		recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
		recset_->Update();

		SafeArrayDestroy (varPoints.parray);
	}
	catch(_com_error &e)
	{
		recset_->Close();
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		recset_->Close();
		errorMessage_ = "Oppps !";
		return false;
	}
	recset_->Close();
	return true;
}

bool 
TeOracle::insertPointSet (const string& table, TePointSet &ps)
{
	for (unsigned int i = 0; i < ps.size(); i++ )
	{
		TePoint& point = ps [i];
		if (!insertPoint (table,point))
			return false;
	}
	return true;
}

bool 
TeOracle::insertPoint (const string& table, TePoint &p)
{
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, x, y) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(p.objectId()) + "'";
	ins += ",  " + Te2String(p.location().x_,15);
	ins += ",  " + Te2String(p.location().y_,15);
	ins += " ) ";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT "+ getNameSequence(table) + ".CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence " + table + "_seq!";  
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
	p.geomId(index); 
	delete ocip;
	return true;
}


bool 
TeOracle::insertCellSet (const string& table, TeCellSet &cs)
{
	for (unsigned int i = 0; i < cs.size(); i++ )
	{
		TeCell& cell = cs [i];
		if (!insertCell (table,cell))
			return false;
	}
	return true;
}


bool 
TeOracle::insertCell (const string& table, TeCell &cell)
{
	TeBox b = cell.box();
	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, col_number, row_number, lower_x, ";
	ins += " lower_y, upper_x, upper_y ) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(cell.objectId ()) + "'";
	ins += ",  " + Te2String(cell.column ());
	ins += ",  " + Te2String(cell.line ());
	ins += ",  " + Te2String(b.x1(),15);
	ins += ",  " + Te2String(b.y1(),15);
	ins += ",  " + Te2String(b.x2(),15);
	ins += ",  " + Te2String(b.y2(),15);
	ins += " )";
		
	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT "+ getNameSequence(table) +".CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence " + table + "_seq!";  
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
	cell.geomId(index); 
	delete ocip;
	
	return true;
}

bool 
TeOracle::insertTextSet (const string& table, TeTextSet &ts)
{
	for (unsigned int i = 0; i < ts.size(); i++ )
	{
		TeText& text = ts [i];
		if (!insertText (table,text))
			return false;
	}
	return true;
}

bool 
TeOracle::insertText(const string& table, TeText &text)
{
	string ins = "INSERT INTO " + table + " (geom_id, ";
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
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT "+ getNameSequence(table) +".CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence " + table + "_seq!";  
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
	text.geomId(index); 
	delete ocip;
	return true;
}

bool 
TeOracle::insertArcSet (const string& table, TeArcSet &as)
{
	for (unsigned int i = 0; i < as.size(); i++ )
	{
		TeArc& arc = as [i];
		if (!insertArc (table,arc))
			return false;
	}
	return true;
}

bool 
TeOracle::insertArc (const string& table, TeArc &arc)
{
	string ins = "INSERT INTO " + table + " (geom_id, ";
	ins += " object_id, from_node, to_node ) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(arc.objectId()) + "'";
	ins += ",  " + Te2String(arc.fromNode().geomId());
	ins += ",  " + Te2String(arc.toNode().geomId());
	ins += " )";
	
	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT "+ getNameSequence(table) +".CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence " + table + "_seq!";  
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
	arc.geomId(index); 
	delete ocip;
	return true;
}

bool 
TeOracle::insertNodeSet (const string& table, TeNodeSet &ns)
{
	for (unsigned int i = 0; i < ns.size(); i++ )
	{
	     TeNode& no = ns [i];
	     if (!insertNode (table,no))
		return false;
	}
	return true;
}

bool 
TeOracle::insertNode (const string& table, TeNode &node)
{
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, x, y) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(node.objectId()) + "'";
	ins += ",  " + Te2String(node.location().x(),15);
	ins += ",  " + Te2String(node.location().y(),15);
	ins += " ) ";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	TeOraclePortal *ocip = (TeOraclePortal*)getPortal();
	if(!ocip)
		return false;

	string seq = "SELECT "+ getNameSequence(table) + ".CURRVAL FROM DUAL";
	if(!ocip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence " + table + "_seq!";  
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
	node.geomId(index); 
	delete ocip;
	return true;
}

string 
TeOracle::getSQLAutoNumber(const string& table)
{
	string aut = getNameSequence(table) +".NEXTVAL";
	return aut;
}


bool 
TeOracle::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	vector<string> pks;
	TeDatabasePortal* portal = getPortal();
	if(!portal)
		return false;

	string keys = " SELECT COLUMN_NAME "; 
	keys += " FROM ALL_CONS_COLUMNS, ALL_CONSTRAINTS "; 
	keys += " WHERE ALL_CONS_COLUMNS.CONSTRAINT_NAME = ALL_CONSTRAINTS.CONSTRAINT_NAME ";
	keys += " AND ALL_CONSTRAINTS.table_name = '"+ TeConvertToUpperCase(tableName) +"' AND ";
	keys += " ALL_CONSTRAINTS.OWNER= '"+ TeConvertToUpperCase(user()) +"' AND ";
	keys += " ALL_CONSTRAINTS.CONSTRAINT_TYPE = 'P' ";

	if(!portal->query(keys))
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
		pks.push_back(portal->getData(0));

	portal->freeResult();

	string sql = "SELECT * FROM " + tableName + " WHERE 1=2";
	
	if (!portal->query(sql))
	{
		delete portal;
		return false;
	}
	attList = portal->getAttributeList();

	vector<string>::iterator itn;
	TeAttributeList::iterator it = attList.begin();
	while (it != attList.end())
	{
		itn = find(pks.begin(),pks.end(), TeConvertToUpperCase((*it).rep_.name_));
		if (itn != pks.end())
			(*it).rep_.isPrimaryKey_ = true;
		++it;
	}
	delete portal;
	return true;
}

string  
TeOracle::getSQLTime(const TeTime& time) const
{
	string dateTime = time.getDateTime();	
	string result;
	result = "TO_DATE('";
	result += dateTime;
	result += "  ";
	result += "','DD/MM/YYYY HH24:MI:SS')"; 
	return result;
}


//----- TeOraclePortal methods ---

TeOraclePortal::TeOraclePortal ( TeDatabase*  pDatabase) 
{
	db_ = pDatabase;
	connection_ = ((TeOracle*)pDatabase)->connection_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
}


TeOraclePortal::TeOraclePortal()
{
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	curRow_ = 0;
}

TeOraclePortal::~TeOraclePortal ()
{
	freeResult();
}


bool 
TeOraclePortal::query (const string &qry, TeCursorLocation l, TeCursorType /* t */, TeCursorEditType /* e */, TeCursorDataType /*dt */ )
{
	try
	{
		if(l==TeCLIENTESIDE)
			recset_->CursorLocation=ADODB::adUseClient;
		else
			recset_->CursorLocation=ADODB::adUseServer;

		recset_->Open(qry.c_str(),connection_.GetInterfacePtr(),
					ADODB::adOpenForwardOnly,  //adOpenKeyset  //adOpenStatic
					ADODB::adLockOptimistic,
					ADODB::adCmdText);
		numRows_ = recset_->GetRecordCount();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}

	ADODB::FieldsPtr ptrFields = NULL;
	ADODB::FieldPtr ptrField = NULL;
	ptrFields = recset_->Fields;
	numFields_ = ptrFields->Count; 

	attList_.clear ();
	int i;
	for(i = 0; i < numFields_; i++)
	{
		_variant_t vCol((long)i);
//		HRESULT hr = ptrFields->get_Item(vCol, &ptrField);
		ptrFields->get_Item(vCol, &ptrField);
		TeAttribute attribute;
		
		int nType = ptrField->Type;
//		int precision = ptrField->Precision;
		int numericScale = ptrField->NumericScale;

		if((nType==ADODB::adNumeric)||(nType==ADODB::adVarNumeric))
		{
			if(numericScale>0)
				nType = ADODB::adDouble;
			else
				nType = ADODB::adInteger;
		}
		
		switch (nType)
		{
		case ADODB::adBoolean:
			attribute.rep_.type_ = TeBOOLEAN;
			break;

		case ADODB::adNumeric:
		case ADODB::adInteger:		
		case ADODB::adTinyInt:
		case ADODB::adUnsignedInt:
		case ADODB::adUnsignedSmallInt:
		case ADODB::adUnsignedTinyInt:
		case ADODB::adVarNumeric: 
			attribute.rep_.type_ = TeINT;
			break;

		case ADODB::adSingle:
		case ADODB::adDouble:
			attribute.rep_.type_ = TeREAL;
			attribute.rep_.decimals_ = numericScale; 
			break;

		case ADODB::adDate:
		case ADODB::adDBDate:
		case ADODB::adDBTime:
		case ADODB::adDBTimeStamp:
			attribute.rep_.type_ = TeDATETIME;
			attribute.dateChronon_ = TeSECOND;
			attribute.dateTimeFormat_ = TeAdo::systemDateTimeFormat(attribute.indicatorAM_, attribute.indicatorPM_, attribute.dateSeparator_, attribute.timeSeparator_);
			break;

		case ADODB::adLongVarBinary:
		case ADODB::adVarBinary:
		case ADODB::adBinary:
			attribute.rep_.type_ = TeBLOB;
			break;

		case ADODB::adChar: 
		case ADODB::adWChar:
		case ADODB::adVarChar:
		case ADODB::adVarWChar:
		case ADODB::adLongVarChar:
		case ADODB::adLongVarWChar: 
			attribute.rep_.type_ = TeSTRING;
			break;

		case ADODB::adCurrency: 
		default :
			attribute.rep_.type_ = TeUNKNOWN;
			break;
		}
		attribute.rep_.name_ = ptrField->Name;
		attribute.rep_.numChar_ = ptrField->DefinedSize;

		attList_.push_back ( attribute );
	}

	curRow_=-1;

	return true;
}


TeTime
TeOraclePortal::getDate (const string& name)
{
	_variant_t value;

	TeTime t;
	try
	{
		value = recset_->GetCollect(name.c_str());
	}
	catch(_com_error &e)
	{
		string field = TeGetExtension(name.c_str());
		if (!field.empty())
		{
			try
			{
				value = recset_->GetCollect(field.c_str());
			}
			catch(_com_error &e)
			{
				errorMessage_ = e.Description();
				return t;
			}
			catch(...)
			{
				errorMessage_ = "Error getDate!";
				return t;
			}
		}
		else
		{
			errorMessage_ = e.Description();
			return t;
		}
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return t;
	}

	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		TeAdo* adoDb = (TeAdo*) this->getDatabase();
		t = TeTime (string((char*)bvalue_), TeSECOND, adoDb->systemDateTimeFormat_, 
			adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_);
	}
	return t;
}


TeTime
TeOraclePortal::getDate (int i)
{
    _variant_t vtIndex;
	_variant_t value;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	TeTime t;
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return t;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return t;
	}
	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		TeAdo* adoDb = (TeAdo*) this->getDatabase();
		t = TeTime (string((char*)bvalue_), TeSECOND, adoDb->systemDateTimeFormat_, 
			adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_);
	}
	return t;
}


string
TeOraclePortal::getDateAsString(int i)
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
TeOraclePortal::getDateAsString(const string& s)
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



