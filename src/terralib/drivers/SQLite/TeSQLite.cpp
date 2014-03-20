#include "TeSQLite.h"
#include <TeDatabase.h>

#include "sqlite3/sqlite3.h"
#include <TeWKBGeometryDecoder.h>

#include <string.h>

TeSQLite::TeSQLite() 
: TeDatabase()
, _conn(0)
, _transactionCounter(0)
{
	sqlite3_initialize();
	dbmsName_ = "SQLite";
}

TeSQLite::~TeSQLite()
{
	close();

	sqlite3_shutdown();
}

bool TeSQLite::newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port, bool terralibModel, const std::string& characterSet)
{
	close();

	errorMessage_ = "";

	FILE* file = fopen(database.c_str(), "r");
	if(file != 0)
	{
		fclose(file);
		errorMessage_ = "Database already exists";
		return false;
	}

	int retVal = sqlite3_open_v2(database.c_str(), &_conn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
	if(retVal != SQLITE_OK)
	{
		close();
		return false;
	}

	if(!connect(host, user, password, database, port))
	{
		return false;
	}

	if(terralibModel)
	{
		//create conceptual model
		if(!createConceptualModel())
		{
			close();
			return false;
		}
	}

	return true;
}

bool TeSQLite::connect(const string& host, const string& user, const string& password, const string& database, int port)
{
	errorMessage_ = "";

	FILE* file = fopen(database.c_str(), "r");
	if(file == 0)
	{
		errorMessage_ = "Database does not exist!";
		return false;
	}
	fclose(file);

	close();

	int retVal = sqlite3_open_v2(database.c_str(), &_conn, SQLITE_OPEN_READWRITE, 0);
	if(retVal != SQLITE_OK)
	{
		close();
		return false;
	}

	execute("PRAGMA foreign_keys = ON;");

	isConnected_ = true;
	user_ = user;
	host_ = host;
	password_ = password;
	database_ = database;
	portNumber_ = port;

	return true;
}

void TeSQLite::close()
{
	errorMessage_ = "";
	if(_conn != 0)
	{
		int result = sqlite3_close(_conn);
		if(result != SQLITE_OK)
		{
			int a = 0;
		}
		_conn = 0;
	}
	isConnected_ = false;
}

bool TeSQLite::tableExist(const string& table)
{
	std::string sql = "SELECT tbl_name FROM sqlite_master";
	sql += " WHERE upper(tbl_name) = upper('" + table + "')";
	sql += " AND type = 'table'";

	bool exists = false;

	TeDatabasePortal* portal = getPortal();
	if(portal->query(sql) && portal->fetchRow())
	{
		exists = true;
	}

	delete portal;

	return exists;
}

bool TeSQLite::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	return true;
}

bool TeSQLite::createTable(const string& table, TeAttributeList &attr)
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
			case TeSTRING:		type = "TEXT";
								break;

			case TeREAL:		type = "REAL";
								break;

			case TeINT:
			case TeUNSIGNEDINT:
								type = "INTEGER";
								break;

			case TeBLOB:		//type = "OID";
				                type = "BLOB";
								break;

			case TeDATETIME:	type = "NUMERIC";
								break;

			case TeCHARACTER:	type = "TEXT";
								break;

			case TeBOOLEAN:		type = "NUMERIC";
								break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
								if(!first)
									createTable += ", ";
								else
									first = false;

								createTable += " x		      REAL        DEFAULT 0.0,";
								createTable += " y            REAL        DEFAULT 0.0 ";
								++it;
								continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += "lower_x REAL not null ,";
							createTable += "lower_y REAL not null ,";
							createTable += "upper_x REAL not null ,";
							createTable += "upper_y REAL not null ,";
							createTable += "spatial_data BLOB not null ";
							++it;
							continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;							

							createTable += "lower_x REAL not null ,";
							createTable += "lower_y REAL not null ,";
							createTable += "upper_x REAL not null ,";
							createTable += "upper_y REAL not null ,";
							createTable += "spatial_data BLOB not null ";

							++it;
							continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += "lower_x REAL not null ,";
							createTable += "lower_y REAL not null ,";
							createTable += "upper_x REAL not null ,";
							createTable += "upper_y REAL not null ,";
							createTable += " col_number   INTEGER      NOT NULL,";
							createTable += " row_number	  INTEGER      NOT NULL ";
							++it;
							continue;					

			case TeRASTERTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += "lower_x REAL not null ,";
							createTable += "lower_y REAL not null ,";
							createTable += "upper_x REAL not null ,";
							createTable += "upper_y REAL not null ,";
							createTable += "band_id integer unsigned not null, ";
							createTable += "resolution_factor integer unsigned , ";
							createTable += "subband integer unsigned,";
							createTable += "spatial_data BLOB NOT NULL, ";
							createTable += "block_size integer unsigned not null ";
		   					++it;
							continue;

			case TeNODETYPE:
			case TeNODESETTYPE:
							if(!first)
								createTable += ", ";
							else
								first = false;

							createTable += " x		    REAL        DEFAULT 0.0,";
							createTable += " y          REAL        DEFAULT 0.0 ";
							++it;
							continue;

			case TeTEXTTYPE:
			case TeTEXTSETTYPE:

			default:			type += "TEXT";
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

bool TeSQLite::addColumn(const string& table, TeAttributeRep &rep)
{
	return true;
}

bool TeSQLite::alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName)
{
	return true;
}

bool TeSQLite::alterTable(const string& oldTableName, const string& newTablename)
{
	return true;
}

bool TeSQLite::getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes)
{
	TeDatabasePortal* portal = getPortal();

	std::string sqlIndexes = "PRAGMA index_list(" + tableName + ");";
	if(portal->query(sqlIndexes) == false)
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
	{
		TeDatabaseIndex dbIndex;
		dbIndex.setIndexName(portal->getData("name"));

		vecIndexes.push_back(dbIndex);
	}

	portal->freeResult();

	for(unsigned int i = 0; i < vecIndexes.size(); ++i)
	{
		std::string sqlIndex = "PRAGMA index_info(" + vecIndexes[i].getIndexName() + ");";

		if(portal->query(sqlIndex) == false)
		{
			delete portal;
			return false;
		}

		std::vector<std::string> vecColumns;
		while(portal->fetchRow())
		{
			vecColumns.push_back(portal->getData("name"));
		}
		portal->freeResult();

		vecIndexes[i].setColumns(vecColumns);		
	}

	delete portal;

	return true;
}

bool TeSQLite::createRelation(const string& relName, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
{
	return true;
}

TeDBRelationType TeSQLite::existRelation(const string& tableName, const string& relName)
{
	return TeNoRelation;
}

bool TeSQLite::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	if(!tableExist(tableName))
	{
		return false;
	}

	TeDatabasePortal* portal = this->getPortal();
	if (!portal)
		return false;

	string sql = "PRAGMA table_info(" + tableName + ")";
	if (!portal->query(sql))
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
	{
		TeAttribute attr;
		attr.rep_.name_ = portal->getData("name");
		attr.rep_.isPrimaryKey_ = portal->getBool("pk");
		attr.rep_.null_ = !portal->getBool("notnull");
		attr.rep_.defaultValue_ = portal->getData("dflt_value");

		std::string type = portal->getData("type");
		if(type == "TEXT")
		{
			attr.rep_.type_ = TeSTRING;
		}
		else if(type == "INTEGER")
		{
			attr.rep_.type_ = TeINT;
		}
		else if(type == "REAL")
		{
			attr.rep_.type_ = TeREAL;
		}
		else if(type == "BLOB")
		{
			attr.rep_.type_ = TeBLOB;
		}
		else
		{
			attr.rep_.type_ = TeSTRING;
		}

		attList.push_back(attr);
	}

	delete portal;

	return true;
}

int TeSQLite::getLastInsertedSerial()
{
	return sqlite3_last_insert_rowid(_conn);
}

int getMaxValue(TeDatabase* database, const std::string& tableName, const std::string& intColumnName)
{	
	TeDatabasePortal* portal = database->getPortal();

	std::string sql = "SELECT MAX(" + intColumnName + ") FROM " + tableName;
	
	int maxVal = 0;
	if(portal->query(sql) && portal->fetchRow())
	{
		maxVal = portal->getInt(0);
	}

	delete portal;

	return maxVal;
}

bool TeSQLite::insertProject(TeProject *project)
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
		//int newId = getMaxValue(this, "te_project", "project_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			project->setId(newId);
		}
	}
	else
		return false;
	projectMap()[project->id()] = project;
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);
	return true;
}

bool TeSQLite::insertRelationInfo(const int tableId, const string& tField, const string& rTable, const string& rField, int& relId)
{
	if(tableId < 0)
	{
		return false;
	}
	// check if relation already exists
	TeDatabasePortal* portal = this->getPortal();

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
		//int newId = getMaxValue(this, "te_tables_relation", "relation_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			relId = newId;
		}
	}
	else
		return false;

	return true;
}

bool TeSQLite::insertTableInfo(int layerId, TeTable &table, const string& user)
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
			//int newId = getMaxValue(this, "te_layer_table", "table_id");
			int newId = getLastInsertedSerial();
			if(newId >= 0)
			{
				table.setId(newId);
			}
		}
		else
			return false;
	}
	
	return true;
}

bool TeSQLite::insertBlob(const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size)
{
	return true;
}

bool TeSQLite::insertProjection(TeProjection* proj)
{
	errorMessage_ = "";

	TeProjectionParams par = proj->params();

	string sql  = "INSERT INTO te_projection (name, long0, lat0, offx, ";
		   sql += "offy, stlat1, stlat2, unit, scale, ";
		   sql += "hemis, datum)";
		   sql += " VALUES('";
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
		//int newId = getMaxValue(this, "te_projection", "projection_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			proj->id(newId);
		}
	}
	else
		return false;

	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}

bool TeSQLite::insertLayer(TeLayer* layer)
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
		   sql += this->getSQLTime(editionTime);
	       sql += ")";

	if(this->execute(sql))
	{
		//int newId = getMaxValue(this, "te_layer", "layer_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			layer->id(newId);
		}
	}
	else
		return false;

	layerMap()[layer->id()] = layer;

	return true;
}

bool TeSQLite::insertRepresentation(int layerId, TeRepresentation& rep)
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
		//int newId = getMaxValue(this, "te_representation", "repres_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			rep.id_ = newId;
		}
	}
	else
		return false;

	return true;
}

bool TeSQLite::insertView(TeView *view)
{
	errorMessage_ = "";

	// save it's projection
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
		//int newId = getMaxValue(this, "te_view", "view_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			view->id(newId);
		}
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

bool TeSQLite::insertViewTree(TeViewTree *tree)
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
		//int newId = getMaxValue(this, "te_theme", "theme_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			tree->id(newId);
		}
	}
	else
		return false;
		
	return true;
}

bool TeSQLite::insertThemeGroup(TeViewTree* tree)
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
		//int newId = getMaxValue(this, "te_theme", "theme_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			tree->id(newId);
		}
	}
	else
		return false;
		
	return true;
}

bool 
TeSQLite::generateLabelPositions(TeTheme *theme, const std::string& objectId)
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

bool TeSQLite::insertTheme(TeAbstractTheme *theme)
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
		//int newId = getMaxValue(this, "te_theme", "theme_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			theme->id(newId);
		}
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

bool TeSQLite::insertThemeTable(int themeId, int tableId, int relationId, int tableOrder)
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

bool TeSQLite::insertLegend(TeLegendEntry *legend)
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
		//int newId = getMaxValue(this, "te_legend", "legend_id");
		int newId = getLastInsertedSerial();
		if(newId >= 0)
		{
			legend->id(newId);
		}
	}
	else
		return false;	

	legendMap()[legend->id()] = legend;

	return insertVisual(legend);
}

bool TeSQLite::insertPolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	std::string objectId = p.objectId();

	char* wkb = 0;
	unsigned int wkbSize = 0;

	TeWKBGeometryDecoder::encodePolygon(p, wkb, wkbSize);

	std::string command = "INSERT INTO ";
	command += table;
	command += " (object_id, lower_x, lower_y, upper_x, upper_y, spatial_data)"; 
	command += " VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

	sqlite3_stmt* recordSet = 0;
	int retValue = sqlite3_prepare_v2(_conn, command.c_str(), -1, &recordSet, 0);

	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	sqlite3_bind_text(recordSet, 1, objectId.c_str(), objectId.size(), SQLITE_TRANSIENT);
	sqlite3_bind_double(recordSet, 2, p.box().x1());
	sqlite3_bind_double(recordSet, 3, p.box().y1());
	sqlite3_bind_double(recordSet, 4, p.box().x2());
	sqlite3_bind_double(recordSet, 5, p.box().y2());
	sqlite3_bind_blob(recordSet, 6, wkb, wkbSize, SQLITE_TRANSIENT);

	delete wkb;

	retValue = sqlite3_step(recordSet);
	if(retValue != SQLITE_DONE)
	{
		return false;
	}

	sqlite3_finalize(recordSet);

	//int newId = getMaxValue(this, table, "geom_id");
	int newId = getLastInsertedSerial();
	if(newId >= 0)
	{
		p.geomId(newId);
	}

	return true;
}
bool TeSQLite::updatePolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	std::string command = "UPDATE " + table + " SET ";
	command += "object_id=?";
	command += ", lower_x=?, lower_y=?, upper_x=?, upper_y=?, "; 
	command += ", spatial_data=? WHERE geom_id = " + Te2String(p.geomId());


	std::string objectId = p.objectId();

	char* wkb = 0;
	unsigned int wkbSize = 0;

	TeWKBGeometryDecoder::encodePolygon(p, wkb, wkbSize);

	sqlite3_stmt* recordSet = 0;
	int retValue = sqlite3_prepare_v2(_conn, command.c_str(), -1, &recordSet, 0);

	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	sqlite3_bind_text(recordSet, 1, objectId.c_str(), objectId.size(), SQLITE_TRANSIENT);
	sqlite3_bind_double(recordSet, 2, p.box().x1());
	sqlite3_bind_double(recordSet, 3, p.box().y1());
	sqlite3_bind_double(recordSet, 4, p.box().x2());
	sqlite3_bind_double(recordSet, 5, p.box().y2());
	sqlite3_bind_blob(recordSet, 6, wkb, wkbSize, SQLITE_TRANSIENT);

	delete wkb;

	retValue = sqlite3_step(recordSet);
	if(retValue != SQLITE_DONE)
	{
		return false;
	}

	sqlite3_finalize(recordSet);

	return true;
}

bool TeSQLite::insertLine(const string& table, TeLine2D &l)
{
	errorMessage_ = "";

	std::string objectId = l.objectId();

	char* wkb = 0;
	unsigned int wkbSize = 0;

	TeWKBGeometryDecoder::encodeLine(l, wkb, wkbSize);

	std::string command = "INSERT INTO ";
	command += table;
	command += " (object_id, lower_x, lower_y, upper_x, upper_y, spatial_data)"; 
	command += " VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

	sqlite3_stmt* recordSet = 0;
	int retValue = sqlite3_prepare_v2(_conn, command.c_str(), -1, &recordSet, 0);

	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	sqlite3_bind_text(recordSet, 1, objectId.c_str(), objectId.size(), SQLITE_TRANSIENT);
	sqlite3_bind_double(recordSet, 2, l.box().x1());
	sqlite3_bind_double(recordSet, 3, l.box().y1());
	sqlite3_bind_double(recordSet, 4, l.box().x2());
	sqlite3_bind_double(recordSet, 5, l.box().y2());
	sqlite3_bind_blob(recordSet, 6, wkb, wkbSize, SQLITE_TRANSIENT);

	delete wkb;

	retValue = sqlite3_step(recordSet);
	if(retValue != SQLITE_DONE)
	{
		return false;
	}

	sqlite3_finalize(recordSet);

	//int newId = getMaxValue(this, table, "geom_id");
	int newId = getLastInsertedSerial();
	if(newId >= 0)
	{
		l.geomId(newId);
	}

	return true;
}

bool TeSQLite::updateLine(const string& table, TeLine2D &l)
{
	errorMessage_ = "";

	std::string command = "UPDATE " + table + " SET ";
	command += "object_id=?";
	command += ", lower_x=?, lower_y=?, upper_x=?, upper_y=?, "; 
	command += ", spatial_data=? WHERE geom_id = " + Te2String(l.geomId());

	std::string objectId = l.objectId();

	char* wkb = 0;
	unsigned int wkbSize = 0;

	TeWKBGeometryDecoder::encodeLine(l, wkb, wkbSize);

	sqlite3_stmt* recordSet = 0;
	int retValue = sqlite3_prepare_v2(_conn, command.c_str(), -1, &recordSet, 0);

	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	sqlite3_bind_text(recordSet, 1, objectId.c_str(), objectId.size(), SQLITE_TRANSIENT);
	sqlite3_bind_double(recordSet, 2, l.box().x1());
	sqlite3_bind_double(recordSet, 3, l.box().y1());
	sqlite3_bind_double(recordSet, 4, l.box().x2());
	sqlite3_bind_double(recordSet, 5, l.box().y2());
	sqlite3_bind_blob(recordSet, 6, wkb, wkbSize, SQLITE_TRANSIENT);

	delete wkb;

	retValue = sqlite3_step(recordSet);
	if(retValue != SQLITE_DONE)
	{
		return false;
	}

	sqlite3_finalize(recordSet);

	return true;
}

bool TeSQLite::insertPoint(const string& table, TePoint &p)
{
	std::string q = "INSERT INTO " + table + "(object_id, x, y)";
	q += " VALUES(";
	q += "'" + escapeSequence(p.objectId()) + "',";
	q += Te2String(p.location().x_) + ",";
	q += Te2String(p.location().y_) + ")";
	
	if(execute(q) == false)
	{
		return false;
	}

	//int newId = getMaxValue(this, table, "geom_id");
	int newId = getLastInsertedSerial();
	if(newId >= 0)
	{
		p.geomId(newId);
	}
	return true;
}

bool TeSQLite::insertText(const string& table, TeText &t)
{
	return true;
}

bool TeSQLite::insertArc(const string& table,TeArc &arc)
{
	return true;
}

bool TeSQLite::insertNode(const string& table, TeNode &node)
{
	return true;
}

bool TeSQLite::insertCell(const string& table, TeCell &c)
{
	return true;
}

bool TeSQLite::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf, unsigned long size, int band, unsigned int res, unsigned int subband)
{
	errorMessage_ = "";

	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeDatabasePortal* portal = getPortal();
	bool insert = true;

	std::string sql = "SELECT block_id FROM " + table + " WHERE block_id='" + blockId + "'";
	if(!portal->query(sql))
	{
		delete portal;
		errorMessage_ = portal->errorMessage();
		
		return false;
	}

	if(portal->fetchRow())
	{
		insert = false;
	}

	delete portal;

	if(insert)
	{
		sql = "INSERT INTO " + table + " (lower_x, lower_y, upper_x, upper_y, band_id, resolution_factor, subband, block_size, spatial_data, block_id) ";
		sql += " VALUES (?,?,?,?,?,?,?,?,?,?)"; 
	}
	else
	{
		sql = "UPDATE " + table + " SET lower_x=?, lower_y=?, upper_x=?, upper_y=?, band_id=?,";
		sql += "resolution_factor=?, subband=?, block_size=?, spatial_data=? WHERE block_id=? ";
	}


	sqlite3_stmt* recordSet = 0;
	int retValue = sqlite3_prepare_v2(_conn, sql.c_str(), -1, &recordSet, 0);

	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	sqlite3_bind_double(recordSet, 1, ll.x());
	sqlite3_bind_double(recordSet, 2, ll.y());
	sqlite3_bind_double(recordSet, 3, ur.x());
	sqlite3_bind_double(recordSet, 4, ur.y());
	sqlite3_bind_int(recordSet, 5, band);
	sqlite3_bind_int(recordSet, 6, (int)res);
	sqlite3_bind_int(recordSet, 7, (int)subband);
	sqlite3_bind_int(recordSet, 8, (int)size);
	sqlite3_bind_blob(recordSet, 9, buf, size, SQLITE_TRANSIENT);
	sqlite3_bind_text(recordSet, 10, blockId.c_str(), blockId.size(), SQLITE_TRANSIENT);

	retValue = sqlite3_step(recordSet);
	if(retValue != SQLITE_DONE)
	{
		return false;
	}

	sqlite3_finalize(recordSet);

	return true;
}

bool TeSQLite::execute(const string &sql)
{
	errorMessage_ = "";

	int retVal = sqlite3_exec(_conn, sql.c_str(), 0, 0, 0);

	if(retVal != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		return false;
	}

	return true;
}

bool TeSQLite::beginTransaction()
{
	++_transactionCounter;

	if (_transactionCounter > 1)
		return true;
	else
		return execute("BEGIN TRANSACTION");
}

bool TeSQLite::commitTransaction()
{
	_transactionCounter--;

	if (_transactionCounter > 0)
		return true;
	else
		return execute("COMMIT TRANSACTION");
}

bool TeSQLite::rollbackTransaction()
{
	_transactionCounter = 0;

	return execute("ROLLBACK TRANSACTION");
}

string TeSQLite::escapeSequence(const string& from)
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

string TeSQLite::concatValues(vector<string>& values, const string& unionString)
{
	return "";
}

string TeSQLite::toUpper(const string& value)
{
	std::string upper = "upper(" + value + ")";
	return upper;
}

string TeSQLite::getSQLTime(const TeTime& time) const
{
	return "'" + time.getDateTime("YYYYsMMsDDsHHsmmsSS", "-") + "'";
}

std::string TeSQLite::getSQLOrderBy(const TeGeomRep& rep) const
{
	std::string orderBy = "object_id ASC";
	return orderBy;
}

string TeSQLite::errorMessage()
{
	if(errorMessage_.empty())
	{
		errorMessage_ =std::string(sqlite3_errmsg(_conn));
	}

	return errorMessage_;
}

TeDatabasePortal* TeSQLite::getPortal()
{
	return new TeSQLitePortal(this);
}


//*********************************************************
//TeSQLitePortal
//*********************************************************

TeSQLitePortal::TeSQLitePortal() : TeDatabasePortal()
, _recordSet(0)
, _conn(0)
, _skipFirstFetch(false)
, _firstFetchResult(false)
, _currentRow(-1)
{
}

TeSQLitePortal::TeSQLitePortal(TeDatabase* pDatabase) : TeDatabasePortal()
, _recordSet(0)
, _skipFirstFetch(false)
, _firstFetchResult(false)
, _currentRow(-1)
{
	db_ = pDatabase;
	_conn = ((static_cast<TeSQLite*>(pDatabase))->_conn);
}

TeSQLitePortal::~TeSQLitePortal()
{
	freeResult();
}

bool TeSQLitePortal::query(const string &qry, TeCursorLocation l, TeCursorType t, TeCursorEditType e, TeCursorDataType dt)
{
	errorMessage_ = "";
	freeResult();

	_skipFirstFetch = false;
	_firstFetchResult = false;
	_mapColumnNames.clear();

	int retValue = sqlite3_prepare_v2(_conn, qry.c_str(), -1, &_recordSet, 0);
	if(retValue != SQLITE_OK)
	{
		errorMessage_ = errorMessage();
		freeResult();
		return false;
	}

	_query = qry;	

	_firstFetchResult = fetchRow();
	_skipFirstFetch = true;
	_currentRow = -1;	

	numFields_ = sqlite3_column_count(_recordSet);
	for(int i = 0; i < numFields_; ++i)
	{
		int type = sqlite3_column_type(_recordSet, i);
		std::string cName = sqlite3_column_name(_recordSet, i);		

		_mapColumnNames[cName] = i;
		
		TeAttribute attr;
		attr.rep_.name_ = cName;

		if(type == SQLITE3_TEXT)
		{
			attr.rep_.type_ = TeSTRING;
		}
		else if(type == SQLITE_INTEGER)
		{
			attr.rep_.type_ = TeINT;
		}
		else if(type == SQLITE_FLOAT)
		{
			attr.rep_.type_ = TeREAL;
		}
		else if(type == SQLITE_BLOB)
		{
			attr.rep_.type_ = TeBLOB;
		}
		else
		{
			attr.rep_.type_ = TeSTRING;
		}

		attList_.push_back(attr);
	}

	return true;
}

bool TeSQLitePortal::fetchRow()
{
	if(_skipFirstFetch == true)
	{
		_currentRow = 0;
		_skipFirstFetch = false;
		return _firstFetchResult;
	}

	++_currentRow;
	int retValue = sqlite3_step(_recordSet);

	bool hasNext = false;
	if(retValue == SQLITE_ROW) //there are still more data
	{
		hasNext = true;
	}
	else if(retValue == SQLITE_DONE) //no more data on the recordset
	{
		hasNext = false;
	}

	return hasNext;
}

bool TeSQLitePortal::fetchRow(int i)
{
	if(i == _currentRow)
	{
		return true;
	}
	else if(i == (_currentRow + 1))
	{
		return fetchRow();
	}
	else if(i > _currentRow)
	{
		while(_currentRow < i)
		{
			if(fetchRow() == false)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		std::string queryCopy = _query;
		if(query(queryCopy) == false)
		{
			return false;
		}
		return fetchRow(i);
	}

	return true;
}

void TeSQLitePortal::freeResult()
{
	if(_recordSet != 0)
	{
		sqlite3_reset(_recordSet);
		sqlite3_finalize(_recordSet);
	}
	_recordSet = 0;
	_currentRow = -1;
	_query = "";
	attList_.clear();
	_mapColumnNames.clear();
}

char* TeSQLitePortal::getData(int i)
{
	const unsigned char* value = sqlite3_column_text(_recordSet, i);

	if(value == 0)
	{
		static char emptyStr = '\0';
		return &emptyStr;
	}
	return (char*) value;
}

char* TeSQLitePortal::getData(const string& s)
{
	return getData(getColumnIndex(s));
}

int TeSQLitePortal::getInt(int i)
{
	return sqlite3_column_int(_recordSet, i);
}

int TeSQLitePortal::getInt(const string& s)
{
	return getInt(getColumnIndex(s));
}

bool TeSQLitePortal::getBool(int i)
{
	if(sqlite3_column_int(_recordSet, i) == 0)
	{
		return false;
	}
	return true;
}

bool TeSQLitePortal::getBool(const string& s)
{
	return getBool(getColumnIndex(s));
}

TeTime TeSQLitePortal::getDate(int i)
{
	char* value = (char*) sqlite3_column_text(_recordSet, i);

	TeTime time;
	if(value != 0)
	{
		time = TeTime(value, TeChronon(), "YYYYsMMsDDsHHsmmsSS", "-");
	}

	return time;
}

TeTime TeSQLitePortal::getDate(const string& s)
{
	return getDate(getColumnIndex(s));
}

string TeSQLitePortal::getDateAsString(int i)
{
	return "";
}

string TeSQLitePortal::getDateAsString(const string& s)
{
	return "";
}

bool TeSQLitePortal::getBlob(const string& s, unsigned char* &data, long& size)
{
	errorMessage_ = "";
	
	const char* bValue = (const char*) sqlite3_column_blob(_recordSet, getColumnIndex("spatial_data"));
	size = (unsigned int)sqlite3_column_bytes(_recordSet, getColumnIndex("spatial_data"));	

	if(data == NULL)
	{
		data = new unsigned char[size];
	}

	memcpy(data, bValue, size);

	return true;
}

bool TeSQLitePortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{
	long lSize = 0;
	bool result = getBlob("spatial_data", ptData, lSize);

	size = static_cast<unsigned long>(lSize);

	return result;
}

int TeSQLitePortal::getColumnIndex(const string& s)
{
	std::map<std::string, int>::iterator it = _mapColumnNames.find(s);
	if(it != _mapColumnNames.end())
	{
		return it->second;
	}

	return TeDatabasePortal::getColumnIndex(s);
}

bool TeSQLitePortal::fetchGeometry(TePolygon& geom)
{
	return fetchGeometry(geom, 0);
}

bool TeSQLitePortal::fetchGeometry(TePolygon& geom, const unsigned int& initIndex)
{
	errorMessage_ = "";
	
	const char* bValue = (const char*) sqlite3_column_blob(_recordSet, initIndex + 6);
	unsigned int size = (unsigned int)sqlite3_column_bytes(_recordSet, initIndex + 6);	

	TeWKBGeometryDecoder::decodePolygon(bValue, geom, size);

	geom.geomId(getInt(initIndex + 0));
	geom.objectId(getData(initIndex + 1));
	

	return fetchRow();
}

bool TeSQLitePortal::fetchGeometry(TeLine2D& geom)
{
	return fetchGeometry(geom, 0);
}

bool TeSQLitePortal::fetchGeometry(TeLine2D& geom, const unsigned int& initIndex)
{
	errorMessage_ = "";
	
	const char* bValue = (const char*) sqlite3_column_blob(_recordSet, initIndex + 6);
	unsigned int size = (unsigned int) sqlite3_column_bytes(_recordSet, initIndex + 6);

	TeWKBGeometryDecoder::decodeLine(bValue, geom, size);

	geom.geomId(getInt(initIndex + 0));
	geom.objectId(getData(initIndex + 1));
	

	return fetchRow();
}

bool TeSQLitePortal::fetchGeometry(TeNode& geom)
{
	return fetchGeometry(geom, 0);
}

bool TeSQLitePortal::fetchGeometry(TeNode& geom, const unsigned int& initIndex)
{
	errorMessage_ = "";
	
	TeCoord2D pt(getDouble(initIndex + 2), getDouble(initIndex + 3)); 

	geom.add(pt);
	geom.geomId(getInt(initIndex + 0));
	geom.objectId(string(getData(initIndex + 1)));

	return fetchRow();
}

bool TeSQLitePortal::fetchGeometry(TePoint& geom)
{
	return fetchGeometry(geom, 0);
}

bool TeSQLitePortal::fetchGeometry(TePoint& geom, const unsigned int& initIndex)
{
	errorMessage_ = "";
	
	TeCoord2D pt(getDouble(initIndex + 2), getDouble(initIndex + 3)); 

	geom.add(pt);
	geom.geomId(getInt(initIndex + 0));
	geom.objectId(string(getData(initIndex + 1)));

	return fetchRow();
}


string TeSQLitePortal::errorMessage()
{
	if(errorMessage_.empty())
	{
		errorMessage_ =std::string(sqlite3_errmsg(_conn));
	}

	return errorMessage_;
}
