/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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
#include <TeAdoSqlServer.h>
#include <sys/stat.h>

#include <TeUtils.h>

#define CHUNKSIZE	240

typedef map<int,TeNode> TeNodeMap;

inline void TESTHR( HRESULT hr )
{
	if( FAILED(hr) ) _com_issue_error( hr );
}

TeSqlServer::TeSqlServer()  
{
    HRESULT hr = CoInitialize(0);
	dbmsName_ = "SqlServerAdo";
    if(FAILED(hr))
    {
        cout << "Can't start COM!? " << endl;
    }
}

TeSqlServer::~TeSqlServer()
{
}

bool 
TeSqlServer::newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port, bool terralibModel, const std::string& /*characterSet*/)
{
	if ( connect(host,user,password,database,port) == false )
		return false;
	
	if ( terralibModel )
	{
		//create conceptual model
		if(!this->createConceptualModel())
			return false;
	}	
	return true;
}


bool 
TeSqlServer::connect (const string& host, const string& user, const string& password, const string& database, int /* port */)
{
	
	string  connectionString = "Provider=SQLOLEDB.1;Password=" + password + ";Persist Security Info=True;User ID=" + user + ";Initial Catalog=" + database + ";Data Source=" + host;

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
TeSqlServer::tableExist(const string& table)
{
	if (table.empty())
		return false;
	TeSqlServerPortal *sqlp = (TeSqlServerPortal*)getPortal();
	if(!sqlp)
		return false;

	string exist = "SELECT name FROM sysobjects WHERE UPPER(name) = UPPER('" + table + "') ";
	exist+=" and (UPPER(xtype)='U' or UPPER(xtype)='V') ";
	
	if(!sqlp->query(exist))
	{
		delete sqlp;
		return false;
	}

	if(sqlp->fetchRow())
	{
		delete sqlp;
		return true;
	}
	else
	{
		delete sqlp;
		return false;
	}
}


bool 
TeSqlServer::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	TeSqlServerPortal *sqlp = (TeSqlServerPortal*)getPortal();
	if(!sqlp)
		return false;

	string exist ="select name,xtype,length,xscale  from syscolumns where name='" + column + "' and ";
	exist+=" id in(select id from sysobjects where xtype='U' and name='" + table + "')";

	
	if(!sqlp->query(exist))
	{
		delete sqlp;
		return false;
	}

	if(sqlp->fetchRow())
	{	
		attr.rep_.name_ = column;
		
		int	dataType = atoi(sqlp->getData(1)); 
		int		dataLength = atoi(sqlp->getData(2)); 
		attr.rep_.numChar_ = dataLength;
		if(dataType == 167 && dataLength < 256)
		{
			attr.rep_.type_ = TeSTRING;
		}
		else if (dataType == 167 && dataLength >= 256)
		{
			attr.rep_.type_ = TeBLOB;
		}
		else if (dataType == 34)
		{
			attr.rep_.type_ = TeBLOB;
		}
		else if (dataType == 56)
		{
			attr.rep_.type_ = TeREAL;
		}
		else if (dataType == 62)
		{
			attr.rep_.type_ = TeINT;
		}
		else if (dataType == 167 && dataLength == 0)
		{	
			attr.rep_.type_ = TeCHARACTER;
		}
		else if (dataType == 61)
		{
			attr.rep_.type_ = TeDATETIME;
		}
		else
		{
			attr.rep_.type_ = TeSTRING;
			attr.rep_.numChar_ = dataLength;
		}
		delete sqlp;
		return true;
	}
	delete sqlp;
	return false;
}

bool 
TeSqlServer::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	return(TeDatabase::getAttributeList (tableName, attList));
}


bool
TeSqlServer::createTable(const string& table, TeAttributeList &attr)
{
	bool first = true;
	string pkeys ="";

	if(tableExist(table))
	{
		errorMessage_= "Table already exist!";
		return false;
	}

	TeAttributeList::iterator it = attr.begin();
	string tablec;
	tablec = "CREATE TABLE dbo." + table +" (";
	
	while ( it != attr.end())
	{
		if (first == false)
		{
			tablec += ", ";
		}
			
		switch ((*it).rep_.type_)
		{
			case TeSTRING:
				if ( (*it).rep_.numChar_ > 0 && (*it).rep_.numChar_ <= 256)
				   tablec += (*it).rep_.name_ + " VARCHAR(" + Te2String((*it).rep_.numChar_) + ") ";
				else
				   tablec += (*it).rep_.name_ + " TEXT "; 
			break;
			
			case TeREAL:
				tablec += (*it).rep_.name_ + " DOUBLE PRECISION ";
			break;
			
			case TeINT:
			case TeUNSIGNEDINT:
				tablec += (*it).rep_.name_ + " INT ";
			break; 

			case TeDATETIME:
				tablec += (*it).rep_.name_ + " DATETIME ";
			break;

			case TeCHARACTER:
				tablec += (*it).rep_.name_ + " CHAR ";
			break;

			case TeBOOLEAN:
				tablec += (*it).rep_.name_ + " BIT ";
			break;

			case TeBLOB:
				tablec += (*it).rep_.name_ + " IMAGE ";
			break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
			case TeNODETYPE:
			case TeNODESETTYPE:
				tablec += "x	 DOUBLE PRECISION  DEFAULT 0.000 ";
				tablec += ", y	 DOUBLE PRECISION  DEFAULT 0.000 ";
				++it;
				continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:
				tablec += " num_coords INT NOT NULL ,";
				tablec += " lower_x DOUBLE PRECISION NOT NULL ,";
				tablec += " lower_y DOUBLE PRECISION NOT NULL ,";
				tablec += " upper_x DOUBLE PRECISION NOT NULL ,";
				tablec += " upper_y DOUBLE PRECISION NOT NULL ,";
				tablec += " ext_max DOUBLE PRECISION NOT NULL, ";
				tablec += " spatial_data IMAGE ";
				++it;
				continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
				tablec += " num_coords INT NOT NULL ,";
				tablec += " num_holes INT NOT NULL ,";
				tablec += " parent_id INT NOT NULL ,";
				tablec += " lower_x DOUBLE PRECISION NOT NULL ,";
				tablec += " lower_y DOUBLE PRECISION NOT NULL ,";
				tablec += " upper_x DOUBLE PRECISION NOT NULL ,";
				tablec += " upper_y DOUBLE PRECISION NOT NULL ,";
				tablec += " ext_max DOUBLE PRECISION NOT NULL ,";
				tablec += " spatial_data IMAGE ";
				++it;
				continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
				tablec += " lower_x	DOUBLE PRECISION NOT NULL ,";
				tablec += " lower_y	DOUBLE PRECISION NOT NULL  ,";
				tablec += " upper_x	DOUBLE PRECISION NOT NULL ,";
				tablec += " upper_y	DOUBLE PRECISION NOT NULL ,";
				tablec += " col_number	INT NOT NULL ,";
				tablec += " row_number	INT NOT NULL ";
				++it;
				continue;			

			case TeRASTERTYPE:
				tablec += " lower_x DOUBLE PRECISION NOT NULL, ";
				tablec += " lower_y DOUBLE PRECISION NOT NULL, ";
				tablec += " upper_x DOUBLE PRECISION NOT NULL, ";
				tablec += " upper_y DOUBLE PRECISION NOT NULL, ";
				tablec += " band_id INT NOT NULL, ";
				tablec += " resolution_factor INT NOT NULL , ";
				tablec += " subband INT ,";
				tablec += " spatial_data IMAGE, ";
				tablec += " block_size INT NOT NULL ";
				++it;
				continue;

            case TeTEXTTYPE:
			case TeTEXTSETTYPE:
			default:
				tablec += (*it).rep_.name_ + " VARCHAR(255) ";
			break;
		}

		//default value
		if(!((*it).rep_.defaultValue_.empty()))
            tablec += " DEFAULT '" + (*it).rep_.defaultValue_ +"' " ;

		//not null
		if(!((*it).rep_.null_))
			tablec += " NOT NULL ";
		
		// auto number
		if((*it).rep_.isAutoNumber_ && ((*it).rep_.type_==TeINT || (*it).rep_.type_==TeUNSIGNEDINT))  
			tablec += " IDENTITY(1,1) "; 

		// check if column is part of primary key
		if ((*it).rep_.isPrimaryKey_ && (*it).rep_.type_ != TeBLOB )
		{
			if (!pkeys.empty())
				pkeys += ", ";
			pkeys += (*it).rep_.name_;
		}

		++it;
		first = false;
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
	return true;
}

bool 
TeSqlServer::alterTable (const string& table, TeAttributeRep &rep, const string& oldColName)
{
	if(!tableExist(table))
		return false;

	if(!oldColName.empty() && oldColName != rep.name_)
	{
		HRESULT hr = S_OK;
		ADOX::_CatalogPtr m_pCatalog   = NULL;
		ADOX::_TablePtr m_pTable  = NULL;
		ADOX::_ColumnPtr m_pColumn  = NULL;
		try
		{
			TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 			m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));

			m_pTable= m_pCatalog->Tables->GetItem(table.c_str());
			m_pColumn = m_pTable->Columns->GetItem(oldColName.c_str()); 
			m_pColumn->Name = rep.name_.c_str();
			m_pCatalog->Tables->Refresh();								// Refresh the database.
			m_pCatalog = NULL;
			m_pTable  = NULL;
		}
		catch(_com_error &e)
		{
 			errorMessage_ = e.Description();
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}

		catch(...)
		{
			errorMessage_ = "Error alter table";
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}
	}

	string tab = " ALTER TABLE " + table + " ALTER COLUMN ";
	tab += rep.name_ + "  ";
	switch (rep.type_)
	{
		case TeSTRING:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
		
		case TeREAL:
			tab += "FLOAT";	
			break;
		
		case TeINT:
			tab += "INT";
			break;

		case TeDATETIME:
			tab += "DATE";
			break;

		case TeCHARACTER:
			tab += "CHAR";
			break;

		case TeBOOLEAN:
			tab += "BIT";
			break;
		
		default:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	tab += " NULL ";

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
TeSqlServer::alterTable(const string& oldTableName, const string& newTablename)
{
	string sql = "sp_rename '"+ oldTableName +"', '"+ newTablename +"'";
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
TeSqlServer::deleteTable (const string& table)
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
	string trig = "DROP TRIGGER " + getNameTrigger(table);
	execute(trig);
	return true;
}

bool TeSqlServer::listDatabases(std::vector<std::string> &databaseList) 
{ 	
	TeSqlServerPortal	*sqlPortal;
	std::string			sql;
	std::string			value;

	databaseList.clear();
	sql="SELECT name FROM sys.databases WHERE name NOT IN ('master', 'tempdb', 'model', 'msdb')";
	sqlPortal=(TeSqlServerPortal*)getPortal();
	if(sqlPortal)
	{
		if( sqlPortal->query(sql) == true)
		{
			while(sqlPortal->fetchRow())
			{
				value=sqlPortal->getData("name");
				databaseList.push_back(value);
			}
			sqlPortal->freeResult();
		}
		delete sqlPortal;
	}
	return !databaseList.empty(); 
}


bool TeSqlServer::dropDatabase(const std::string & databaseName)
{
	std::string		sql;

	sql="DROP DATABASE " + databaseName;
	return this->execute(sql);
}


bool 
TeSqlServer::addColumn (const string& table, TeAttributeRep &rep)
{
	if(!tableExist(table))
		return false;

	string tab;
	tab = " ALTER TABLE " + table + " ADD  ";
	tab += rep.name_ + "  ";
	
	switch (rep.type_)
	{
		case TeSTRING:
			if ( rep.numChar_ > 0 && rep.numChar_ < 256 )
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			else
				tab += "TEXT "; 
		break;
			
		case TeREAL:
			tab += "DOUBLE PRECISION";	
			break;
			
		case TeINT:
			tab += "INT";
			break;

		case TeDATETIME:
			tab += "DATETIME";
			break;

		case TeCHARACTER:
			tab += "CHAR";
			break;

		case TeBOOLEAN:
			tab += "BIT";
			break;
		
		case TeBLOB:
			tab += "IMAGE";
			break;
		
		default:
			tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	alterTableInfoInMemory(table);
	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting a column to table " + table + " !";
		return false;
	}

	return true;
}

bool TeSqlServer::deleteColumn (const string& table, const string& colName)
{
	this->beginTransaction();

	std::string sql = "select t.name as tabela, c.name as coluna, d.name as nome_constraint";
	sql += " from syscolumns c, sysobjects d, sysobjects t where cdefault = d.id and d.xtype = 'D' and d.parent_obj = t.id";
	sql += " and t.name = '" + table + "'";
	sql += " and c.name = '" + colName + "'";

	TeDatabasePortal* portal = this->getPortal();
	if(portal->query(sql) == false)
	{
		this->rollbackTransaction();
		delete portal;
		return false;
	}
	
	while(portal->fetchRow())
	{
		std::string cName = portal->getData("nome_constraint");
		std::string alterTable = "alter table " + table;
		alterTable += " drop constraint " + cName;

		if(this->execute(alterTable) == false)
		{
			this->rollbackTransaction();
			delete portal;
			return false;
		}
	}

	delete portal;

	if(TeAdo::deleteColumn(table, colName) == false)
	{
		this->rollbackTransaction();
		return false;
	}

	if(this->commitTransaction() == false)
	{
		this->rollbackTransaction();
		return false;
	}

	return true;
}


bool
TeSqlServer::createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
{
	
	string relation = "ALTER TABLE " + table + " ADD ";
	relation += " CONSTRAINT " + name;
	relation += " FOREIGN KEY ( " + fieldName + ") "; 
	relation += " REFERENCES " + relatedTable + "(" + relatedField + ")";
		
	if (cascadeDeletion)
		relation += " ON DELETE CASCADE ON UPDATE NO ACTION"; 

	if(!execute(relation))
		return false;

	return true;
}


TeDBRelationType 
TeSqlServer::existRelation(const string& /* tableName */, const string& relName)
{

	TeSqlServerPortal  *sqlp = (TeSqlServerPortal*)getPortal();
	if(!sqlp)
		return TeNoRelation;

	string integ="select name from sysobjects where upper(name)=upper('" + relName + "')";
	integ +=" and upper(xtype)='F'";
	
	if(!sqlp->query(integ))
	{
		delete sqlp;
		return TeNoRelation;
	}
	

	if ( sqlp->fetchRow() )		
	{
		int status = sqlp->getInt("status");
		if ( (status & 14338) == 14338 ) //Cascade Update Delete 
		{
			delete sqlp;
			return TeRICascadeDeletion; 
		}
		if ( (status & 4098) == 4098 ) //Cascade Delete
		{
			delete sqlp;
			return TeRICascadeDeletion;
		}
		delete sqlp;
		return TeRINoCascadeDeletion;
	}

	delete sqlp;
	return TeNoRelation;	
}


TeDatabasePortal*  
TeSqlServer::getPortal ()
{
	TeSqlServerPortal* portal = new TeSqlServerPortal (this);
	return portal;
}

bool
TeSqlServer::deleteLayer(int layerId)
{
	//Delete attributes tables
	if(!deleteLayerTable(layerId))
		return false;

	TeSqlServerPortal* portal = (TeSqlServerPortal*) this->getPortal();
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
		if (rep == TeRASTER)
		{
			TeSqlServerPortal* portal2 = (TeSqlServerPortal*)this->getPortal();
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

				}
				// remove raster table
				tabName = portal2->getData(1);
				if (!tabName.empty() && this->tableExist(tabName))
				{
					sql = "DROP TABLE " + tabName;
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
TeSqlServer::deleteTheme(int themeId)
{
   string sql;
   // delete the collection table associated to this theme
   TeDatabasePortal* portal = this->getPortal();
   if(!portal)
       return false;

   sql = "SELECT collection_table FROM te_theme WHERE theme_id = " + Te2String(themeId);
   if(!portal->query(sql) || !portal->fetchRow())
   {          
	   delete portal;
       return false;
   }
    string colTab = portal->getData("collection_table");
   //delete collection table
   if (this->tableExist(colTab))      
   {
       sql = "DROP TABLE " + colTab;
       if (!this->execute(sql) )
       {   
		   delete portal;
           return false;
       }
   }
   //delete auxiliar collection table
   if (this->tableExist(colTab +"_aux"))      
   {
       sql = "DROP TABLE " +colTab +"_aux";
       if (!this->execute(sql) )
       {              
		   delete portal;
           return false;
       }
   }
   portal->freeResult();

   //delete the visual associated to this theme
   if (existRelation("te_visual","fk_visual_legend_id") != TeRICascadeDeletion)
   {
       sql = "SELECT legend_id FROM te_legend WHERE theme_id = " + Te2String(themeId);
       if(!portal->query(sql))
       {              
		   delete portal;
           return false;
       }
       string wherec;
       int c = 0;
       while (portal->fetchRow())
       {
           if (c)
               wherec += ",";
           c++;
           wherec += portal->getData(0);
       }
       portal->freeResult();
       if (!wherec.empty()) 
       {
           sql = "DELETE FROM te_visual WHERE legend_id IN (" + wherec + ")";
           if (!this->execute(sql))
           {                  
			   delete portal;
               return false;
           }
       }
   }

   //delete all visuals of raster associated to this theme
   if (existRelation("te_visual_raster","fk_visrast_theme_id") != TeRICascadeDeletion)
   {
       sql = "DELETE FROM te_visual_raster WHERE theme_id =" + Te2String(themeId);
       if (!this->execute (sql))
       {              
		   delete portal;
           return false;
       }
   }
     //delete all legends associated to this theme
   if (existRelation("te_legend","fk_legend_theme_id") != TeRICascadeDeletion)
   {
       sql = "DELETE FROM te_legend WHERE theme_id =" + Te2String(themeId);
       if (!this->execute (sql))
       {              
		   delete portal;
           return false;
       }
   }
     //select the view of this theme
   sql = "SELECT view_id FROM te_theme WHERE theme_id = " + Te2String(themeId);
   portal->freeResult();
   if(!portal->query(sql) ||!portal->fetchRow())
   {          
	   delete portal;
       return false;
   }
 
   int viewId = portal->getInt("view_id");
   TeView* view = viewMap()[viewId];
   sql = "SELECT name FROM te_theme WHERE theme_id = " + Te2String(themeId);
   portal->freeResult();
   if (!portal->query(sql) || !portal->fetchRow())
   {          
	   delete portal;
       return false;
   }
  
   TeTheme* tema = NULL;
   if (view != 0)
	   tema = (TeTheme*)view->remove(themeId);
   if (tema != 0) 
   {
       unsigned int i;
       TeLegendEntryVector& legendVector = tema->legend();
       for (i = 0; i < legendVector.size(); ++i)
           legendMap().erase(legendVector[i].id());
       themeMap().erase(themeId);
       delete tema;
   }
   delete portal;

   //delete the tables associated to this theme
   if (existRelation("te_theme_table","fk_thmtable_theme_id") != TeRICascadeDeletion)
   {
       sql = "DELETE FROM te_theme_table WHERE theme_id =" + Te2String(themeId);
       if (!this->execute (sql))
           return false;
   }
   //delete the grouping
   if (existRelation("te_grouping","fk_group_theme_id")  != TeRICascadeDeletion)
   {
       sql = "DELETE FROM te_grouping WHERE theme_id =" + Te2String(themeId);
       if (!this->execute (sql))
           return false;
   }   

	sql = " UPDATE te_view SET current_theme = NULL WHERE current_theme = "+ Te2String(themeId);
	this->execute(sql);

	if(tableExist("te_theme_application"))
	{
		sql=" DELETE FROM te_theme_application where theme_id=" + Te2String(themeId);
		if(!this->execute(sql))
		{
			return false;
		}
	}

   // delete the theme
   sql = "DELETE FROM te_theme WHERE theme_id = " + Te2String(themeId);
   if (!this->execute (sql))
       return false;

   return true;
} 

bool 
TeSqlServer::deleteView (int viewId)
{
	TeDatabasePortal* portal = this->getPortal();

	// view projection should be deleted manually
	string sql =  "SELECT projection_id FROM te_view WHERE view_id=" + Te2String(viewId);
	portal->freeResult();
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
	
	// delete themes belonging to this view 
	sql = "SELECT theme_id FROM te_theme WHERE view_id=" + Te2String(viewId);
	if (!portal->query(sql))
	{
		delete portal;
		return false;
	}
	while (portal->fetchRow())
	{
		int id = atoi(portal->getData(0));
		if(deleteTheme(id) == false)
		{	
			delete portal;
			return false;
		}
	}

	// delete view
	sql = "DELETE FROM te_view WHERE view_id = " + Te2String(viewId);
	if (!this->execute (sql))
	{
		delete portal;
		return false;
	}

	sql = "DELETE FROM te_projection WHERE  projection_id = " + projId;
	if (!this->execute (sql))
	{
		delete portal;
		return false;
	}

	// Delete the view and its themes
	TeView* view = viewMap()[viewId];
	viewMap().erase(viewId);
	delete view;
	delete portal;
	return true;
}
string
TeSqlServer::getNameTrigger(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_tri";
	else
		name = tableName + "_tri";

	return name;
}

bool
TeSqlServer::insertTable(TeTable &table)
{
	string tableName = table.name();

	string sql = "select * from ";
	sql += tableName;
	sql += " where 1=0";

	TeAttributeList		att = table.attributeList();
	TeAttributeList::iterator	it;

	TeTableRow row;
	unsigned int i;

	_variant_t index;
	index.vt = VT_I4;

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(sql.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdText);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error insert table!";
		return false;
	}

	bool rec_open = false;
	for ( i = 0; i < table.size(); i++  )
	{
		try 
		{
			if (!rec_open)
				recset_->AddNew();
			row = table[i];
			it = att.begin();

			for(unsigned int j=0; j<row.size(); j++) 
			{
				if (row[j].empty() || (*it).rep_.isAutoNumber_)
				{
					++it;
					continue;
				}

				TeTime dt;
				index.lVal = j;
				string value = row[j];
				std::string name = (*it).rep_.name_;

				if((*it).rep_.type_==TeDATETIME)
				{
					TeTime t(string(row[j].c_str()), (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_,  (*it).timeSeparator_, (*it).indicatorPM_);
					dt=t;
				}

				switch ((*it).rep_.type_)
				{
				case TeREAL:
					recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) atof(row[j].c_str());
					break;
				case TeINT:
					recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) atol(row[j].c_str());
					break;
				case TeBOOLEAN:
					{
						int value = 0;
						if(row[j] == "1" || TeConvertToLowerCase(row[j]) == "t" || TeConvertToLowerCase(row[j]) == "true")
						{
							value = 1;
						}
						recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) value;
					}
					break;

				case TeDATETIME:
					recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_bstr_t) (dt.getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str()); 
					break;
				case TeBLOB:
					{
						BYTE *pByte;
						VARIANT var;

						SAFEARRAY FAR* psa;
						SAFEARRAYBOUND rgsabound[1];
						rgsabound[0].lLbound = 0;	
						rgsabound[0].cElements =  value.size();

						// create a single dimensional byte array
						psa = SafeArrayCreate(VT_I1, 1, rgsabound);

						// set the data of the array with data in the edit box
						if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
							memcpy((LPVOID)pByte,(LPVOID)(char*)value.c_str(), value.size() * sizeof(char));
						SafeArrayUnaccessData(psa);
						var.vt = VT_ARRAY | VT_UI1;
						var.parray = psa;

						recset_->Fields->Item[(*it).rep_.name_.c_str()]->AppendChunk (var);
						SafeArrayDestroy (var.parray);						
						break;	
					}
				default:
					if (value.size() > (unsigned int)((*it).rep_.numChar_))
						value = value.substr(0,(*it).rep_.numChar_);
					recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_bstr_t) (value.c_str());
					break;
				}				
				++it;				
			}
			recset_->Update();			
			rec_open =  false;
		}
		catch(_com_error &e)
		{
			errorMessage_ = e.Description();
			rec_open = true;
			recset_->Close();
			return false; 
		}
		catch(...)
		{
			errorMessage_ = "Error insert table!";
			rec_open = true;
			recset_->Close();
			return false; 
		}
	}
	recset_->Close();
	return true;
}

bool 
TeSqlServer::insertTheme(TeAbstractTheme *theme)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("select * from te_theme where 1=0",
			_variant_t((IDispatch*)connection_, true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdText);
		recset_->AddNew();
		if(theme->type()==TeTHEME)
			recset_->Fields->GetItem("layer_id")->Value = (_variant_t) ((long)static_cast<TeTheme*>(theme)->layerId());
		recset_->Fields->GetItem("view_id")->Value = (_variant_t) ((long)theme->view());
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (theme->name().c_str ());
		recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)theme->parentId());
		recset_->Fields->GetItem("priority")->Value = (_variant_t) ((long)theme->priority());
		recset_->Fields->GetItem("node_type")->Value = (_variant_t) ((long)theme->type());
		recset_->Fields->GetItem("min_scale")->Value = (_variant_t) (TeRoundD(theme->minScale(),15));
		recset_->Fields->GetItem("max_scale")->Value = (_variant_t) (TeRoundD(theme->maxScale(),15));
		
		if(!theme->attributeRest().empty())
			recset_->Fields->GetItem("generate_attribute_where")->Value = (_bstr_t) (theme->attributeRest().c_str());
		if(!theme->spatialRest().empty())
			recset_->Fields->GetItem("generate_spatial_where")->Value = (_bstr_t) (theme->spatialRest().c_str());
		if(!theme->temporalRest().empty())
			recset_->Fields->GetItem("generate_temporal_where")->Value = (_bstr_t) (theme->temporalRest().c_str());
		
		if(theme->type()==TeTHEME && !static_cast<TeTheme*>(theme)->collectionTable().empty())
			recset_->Fields->GetItem("collection_table")->Value = (_bstr_t) (static_cast<TeTheme*>(theme)->collectionTable().c_str());
		
		recset_->Fields->GetItem("visible_rep")->Value = (_variant_t) ((long)theme->visibleRep());
		recset_->Fields->GetItem("enable_visibility")->Value = (_variant_t) ((long)theme->visibility());

		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(theme->box().x1(),15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(theme->box().y1(),15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(theme->box().x2(),15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(theme->box().y2(),15));
		recset_->Fields->GetItem("creation_time")->Value = (_bstr_t) (theme->getCreationTime().getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str());
		
		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("theme_id");
		theme->id(newID.intVal);

		//recset_->Update();
		recset_->Close();

		if((theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME)&& static_cast<TeTheme*>(theme)->collectionTable().empty())
		{
			static_cast<TeTheme*>(theme)->collectionTable("te_collection_" + Te2String(theme->id()));
			string strSQL="update te_theme set collection_table='te_collection_" + Te2String(theme->id()) + "'";
			if(theme->parentId() == 0)
			{
				theme->parentId(theme->id());
				strSQL += " , parent_id = " + Te2String(theme->parentId());				
			}
			strSQL+= " where theme_id=" + Te2String(theme->id());
			execute(strSQL);
		}
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
	
	bool status = false;

	// insert grouping
	int numSlices = 0;
	if(theme->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (theme->id(), theme->grouping()))
			return false;
		numSlices = theme->grouping().groupNumSlices_;
	}

//	if(theme->rasterVisual()!=0)
//	{
//		if(!insertRasterVisual(theme->id(), theme->rasterVisual()))
//			return false;
//	}
		
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
	if(theme->type() == TeTHEME && !updateThemeTable(static_cast<TeTheme*>(theme)))
		return false;

	return true;
}


bool 
TeSqlServer::generateLabelPositions (TeTheme *theme, const std::string& objectId )
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if( theme->layer()->hasGeometry(TeCELLS) )
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

bool 
TeSqlServer::insertPolygonSet (const string& table, TePolygonSet &ps)
{
	if (ps.empty())
		return true;
    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	
    try
	{
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

        unsigned int i;
        unsigned int n = ps.size();

		// for each polygon in the polygon set
		for (i = 0; i < n; ++i)
		{
			unsigned int k;
			int parentId = 0;
			unsigned int numberHoles = ps[i].size()-1;
			// for each ring 
			for (k=0; k<ps[i].size(); ++k)
			{
				recset_->AddNew();
				recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ps[i].objectId().c_str());
				recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)ps[i][k].size());
				recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)numberHoles);
				recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
				recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(ps[i][k].box().lowerLeft().x_,15));
				recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(ps[i][k].box().lowerLeft().y_,15));
				recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(ps[i][k].box().upperRight().x_,15));
				recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(ps[i][k].box().upperRight().y_,15));
				recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(ps[i][k].box().width(),ps[i][k].box().height()));

				// create and save the BLOB
				VARIANT varPoints;
				if(!LinearRingToVariant(varPoints, ps[i][k]))
				{
					recset_->Close();
					return false;
				}

				recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
				recset_->Update();
				SafeArrayDestroy (varPoints.parray);

				// retrieve the geometry id of the ring (given by the auto increment property)
				ps[i][k].geomId(recset_->GetCollect("geom_id").intVal);
 
				if (k==0)
				{
					parentId = ps[i][k].geomId(); // save the geometry id of the parent
					ps[i].geomId(parentId);
					numberHoles=0;
				}
			}
		}
		recset_->Close();
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
	// update in the database the parent id of an outter ring 
	string strSQL="update " + table + " set parent_id=geom_id where parent_id=0";
	execute(strSQL);
	return true;
}


bool 
TeSqlServer::insertPolygon (const string& table, TePolygon &pol)
{
    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

		unsigned int k;
		int parentId = 0;
		unsigned int numberHoles = pol.size()-1;
		// for each ring
		for (k=0; k<pol.size(); ++k)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (pol.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)pol[k].size());
			recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)numberHoles);
			recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(pol[k].box().lowerLeft().x_,15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(pol[k].box().lowerLeft().y_,15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(pol[k].box().upperRight().x_,15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(pol[k].box().upperRight().y_,15));
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(pol[k].box().width(),pol[k].box().height()));

			// create and save the BLOB
			VARIANT varPoints;
			if(!LinearRingToVariant(varPoints,pol[k]))
			{
				recset_->Close();
				return false;
			}

			recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			recset_->Update();
			SafeArrayDestroy (varPoints.parray);

			// retrieve the geometry id of the ring (given by the auto increment property)
			pol[k].geomId(recset_->GetCollect("geom_id").intVal);

			if (k==0)
			{
				parentId = pol[k].geomId();// save the geometry id of the parent
				pol.geomId(parentId);
				numberHoles=0;
			}
		}
        recset_->Close();
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

	// update in the database the parent id of the outter rings 
	string strSQL="update " + table + " set parent_id=geom_id where parent_id=0";
	return (execute(strSQL));
}

bool
TeSqlServer::insertLineSet(const string& table, TeLineSet &ls)
{
    if (ls.empty())
        return true;

    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));

    try
    {
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

        // for each line
        for (unsigned int i = 0; i < ls.size(); ++i)
        {
            recset_->AddNew();
            recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ls[i].objectId().c_str());
            recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)ls[i].size());
            recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(ls[i].box().lowerLeft().x_,15));
            recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(ls[i].box().lowerLeft().y_,15));
            recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(ls[i].box().upperRight().x_,15));
            recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(ls[i].box().upperRight().y_,15));
            recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(ls[i].box().width(),ls[i].box().height()));

            // create and save the BLOB
            VARIANT varPoints;
            if (!LineToVariant(varPoints,ls[i]))
            {
                recset_->Close();
                return false;
            }

            recset_->Fields->Item["spatial_data"]->AppendChunk(varPoints);
            recset_->Update();
            SafeArrayDestroy(varPoints.parray);

            // retrieve the geometry id of the line (given by the auto increment property)		
            ls[i].geomId (recset_->GetCollect("geom_id").intVal);
        }
        
        recset_->Close();
    }
    catch(_com_error &e)
    {
        recset_->Close();
        errorMessage_ = e.Description();
        return 0;
    }
    catch(...)
    {
        recset_->Close();
        errorMessage_ = "Oppps !";
        return 0;
    }

    return true;
}

bool
TeSqlServer::insertLine(const string& table, TeLine2D &l)
{
    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));

    try
    {	
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

        recset_->AddNew();
        recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (l.objectId().c_str());
        recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)l.size());
        recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(l.box().lowerLeft().x_,15));
        recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(l.box().lowerLeft().y_,15));
        recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(l.box().upperRight().x_,15));
        recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(l.box().upperRight().y_,15));
        recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(l.box().width(),l.box().height()));

        // create and save the BLOB
        VARIANT varPoints;
        if (!LineToVariant(varPoints, l))
        {
            recset_->Close();
            return false;
        }

        recset_->Fields->Item["spatial_data"]->AppendChunk(varPoints);
        recset_->Update();
        SafeArrayDestroy(varPoints.parray);

        // retrieve the geometry id of the line (given by the auto increment property)		
        l.geomId (recset_->GetCollect("geom_id").intVal);
        
        recset_->Close();
    }
    catch(_com_error &e)
    {
        recset_->Close();
        errorMessage_ = e.Description();
        return 0;
    }
    catch(...)
    {
        recset_->Close();
        errorMessage_ = "Oppps !";
        return 0;
    }

    return true;
}

bool
TeSqlServer::insertPointSet(const string& table, TePointSet &ps)
{
    if (ps.empty())
        return true;

    string sql = "select * from ";
    sql += table;
    sql += " where 1=0"; 
    
    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));
    try
    {
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

        for (unsigned int i = 0; i < ps.size(); ++i)
        {
            recset_->AddNew();
            recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ps[i].objectId().c_str());
            recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(ps[i].location().x_,15));
            recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(ps[i].location().y_,15));
            recset_->Update();
			ps[i].geomId(recset_->GetCollect("geom_id").intVal);
        }

        recset_->Close();
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
    
    return true;
}

bool
TeSqlServer::insertPoint(const string& table, TePoint &p)
{
    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	
    
    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));
    try
    {
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);
        recset_->AddNew();
        recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
        recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(p.location().x_,15));
        recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(p.location().y_,15));
        recset_->Update();
        p.geomId (recset_->GetCollect("geom_id").intVal);
        recset_->Close();
    }
    catch(_com_error &e)
    {
        errorMessage_ = e.Description();
        return 0;
    }
    catch(...)
    {
        errorMessage_ = "Oppps !";
        return 0;
    }
    return true;
}

bool
TeSqlServer::insertTextSet(const string& table, TeTextSet &ts)
{
    if (ts.empty())
        return true;

    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));
    try
    {
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);

        for (unsigned int i = 0; i < ts.size(); ++i)
        {
            recset_->AddNew();
            recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ts[i].objectId().c_str());
            recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(ts[i].location().x_,15));
            recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(ts[i].location().y_,15));
            recset_->Fields->GetItem("text_value")->Value = (_bstr_t) (ts[i].textValue().c_str());
            recset_->Fields->GetItem("angle")->Value = (_variant_t) (ts[i].angle());
            recset_->Fields->GetItem("height")->Value = (_variant_t) (ts[i].height());
            recset_->Fields->GetItem("alignment_vert")->Value = (_variant_t) (ts[i].alignmentVert());
            recset_->Fields->GetItem("alignment_horiz")->Value = (_variant_t) (ts[i].alignmentHoriz());
			recset_->Update();
            ts[i].geomId (recset_->GetCollect("geom_id").intVal);
        }
        recset_->Close();
    }
    catch(_com_error &e)
    {
        errorMessage_ = e.Description();
        recset_->Close();
        return 0;
    }
    catch(...)
    {
        errorMessage_ = "Oppps !";
        recset_->Close();
        return 0;
    }
    return true;
}

bool
TeSqlServer::insertText(const string& table, TeText &t)
{
    string sql = "select * from ";
    sql += table;
    sql += " where 1=0";	

    ADODB::_RecordsetPtr recset_;
    recset_.CreateInstance(__uuidof(ADODB::Recordset));
    try
    {
        recset_->Open(_bstr_t(sql.c_str()),
            _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenKeyset,
            ADODB::adLockOptimistic,
            ADODB::adCmdText);
        recset_->AddNew();
        recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (t.objectId().c_str());
        recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(t.location().x(),15));
        recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(t.location().y(),15));
        recset_->Fields->GetItem("text_value")->Value = (_bstr_t) (t.textValue().c_str());
        recset_->Fields->GetItem("angle")->Value = (_variant_t) (t.angle());
        recset_->Fields->GetItem("height")->Value = (_variant_t) (t.height());
        recset_->Fields->GetItem("alignment_vert")->Value = (_variant_t)(t.alignmentVert());
        recset_->Fields->GetItem("alignment_horiz")->Value = (_variant_t)(t.alignmentHoriz());
        recset_->Update();
        t.geomId (recset_->GetCollect("geom_id").intVal);
        recset_->Close();
    }
    catch(_com_error &e)
    {
        errorMessage_ = e.Description();
        recset_->Close();
        return 0;
    }
    catch(...)
    {
        errorMessage_ = "Oppps !";
        recset_->Close();
        return 0;
    }
    return true;
}
string 
TeSqlServer::getSQLTime(const TeTime& time) const
{
	std::string dateStr = time.getDate(); //yyyy-mm-dd
	std::string timeStr = time.getTime();	
	string result = "'"+dateStr+" "+timeStr+"'";
	return result;
}

string TeSqlServer::toUpper(const string& value)
{
	string result  = " UPPER(";
	       result += value;
		   result += ")";
	return result;
}


//----- TeSqlServerPortal methods ---

TeSqlServerPortal::TeSqlServerPortal ( TeDatabase*  pDatabase) 
{
	db_ = pDatabase;
	connection_ = ((TeSqlServer*)pDatabase)->connection_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
}


TeSqlServerPortal::TeSqlServerPortal ()
{
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	curRow_ = 0;
}

TeSqlServerPortal::~TeSqlServerPortal ()
{
	freeResult();
}

TeTime
TeSqlServerPortal::getDate (const string& name)
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
		return t;
	}
	else
		return t;
}


TeTime
TeSqlServerPortal::getDate (int i)
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
		return t;
	}
	else
		return t;
}


string
TeSqlServerPortal::getDateAsString(int i)
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
TeSqlServerPortal::getDateAsString(const string& s)
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

int getLastGeomId ( TeSqlServer* db, const string& table )
{
	TeSqlServerPortal *sqlp = NULL;
	try
	{
		int lastGeomId = 0;

		if (db == NULL)
			return lastGeomId;
		//gets the last geom id
	
		sqlp = (TeSqlServerPortal*)db->getPortal();
		if(!sqlp)
			return false;
		sqlp->freeResult();
		std::string sql= "SELECT MAX(geom_id) AS MaxGeomId FROM " + table;
		
		if(!sqlp->query(sql))
		{
			delete sqlp;
			return false;
		}		
		
		if( sqlp->fetchRow() )
		{
			lastGeomId = atoi( sqlp->getData("MaxGeomId") ); //the string must be empty
		}
		
		delete sqlp;
		sqlp = NULL;
		
		return lastGeomId;
	}
	catch(...)
	{
		if (sqlp)
		{
			delete sqlp;
			sqlp = NULL;
		}
		return 0;
	}
}



