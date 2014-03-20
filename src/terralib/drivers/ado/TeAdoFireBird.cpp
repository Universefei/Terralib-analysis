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
#include <TeAdoFireBird.h>
#include <sys/stat.h>

#include <TeUtils.h>

#define CHUNKSIZE	240

typedef map<int,TeNode> TeNodeMap;

inline void TESTHR( HRESULT hr )
{
	if( FAILED(hr) ) _com_issue_error( hr );
}

TeFireBird::TeFireBird()  
{
    HRESULT hr = CoInitialize(0);
	dbmsName_ = "FireBirdAdo";
    if(FAILED(hr))
    {
        cout << "Can't start COM!? " << endl;
    }
}

TeFireBird::~TeFireBird()
{
}

bool 
TeFireBird::newDatabase(const string& database, const string& user, const string& password , const string&  host , const int& /* port */, bool terralibModel)
{
    if (!connect(host,user,password,database))
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
TeFireBird::connect (const string& host, const string& user, const string& password, const string& database, int port)
{
	string connectionString="Provider=SIBPROvider.2;Password=" + password + ";User ID=" + user + ";Data Source=" + host + ":" + database + ";Persist Security Info=True";

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
TeFireBird::tableExist(const string& table)
{
	bool	status=true;
	TeFireBirdPortal *ip = (TeFireBirdPortal*)getPortal();
	if(!ip)
		return false;

	string exist = " SELECT rdb$relation_name FROM rdb$relations WHERE";
	exist += " rdb$relation_name = '" + TeConvertToUpperCase(table) + "'";
	exist += " AND rdb$owner_name = '" + TeConvertToUpperCase(user_) + "'";

	if(!ip->query(exist))
	{
		delete ip;
		return false;
	}


	if(!ip->fetchRow())	status = false;
	delete ip;
	return (status);
}


bool 
TeFireBird::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	bool status;
	TeFireBirdPortal *ip = (TeFireBirdPortal*)getPortal();
	if(!ip)
		return false;

	string exist="select RDB$TYPES.RDB$TYPE_NAME,RDB$FIELDS.RDB$FIELD_LENGTH,RDB$FIELDS.RDB$FIELD_SCALE from RDB$RELATION_FIELDS ";
	exist+=" join RDB$FIELDS on  RDB$RELATION_FIELDS.RDB$FIELD_SOURCE=RDB$FIELDS.RDB$FIELD_NAME ";
	exist+=" join RDB$TYPES on RDB$FIELDS.RDB$FIELD_TYPE=RDB$TYPES.RDB$TYPE ";
	exist+=" where RDB$RELATION_FIELDS.RDB$RELATION_NAME='" +  TeConvertToUpperCase(table) + "' ";
	exist+=" and RDB$TYPES.RDB$FIELD_NAME='RDB$FIELD_TYPE' ";
	exist+=" and RDB$RELATION_FIELDS.RDB$FIELD_NAME='" + TeConvertToUpperCase(column) + "'";

	if(!ip->query(exist))
	{
		delete ip;
		return false;
	}

	if(ip->fetchRow())
	{	
		attr.rep_.name_ = column;
		
		string	dataType = string(ip->getData(0)); 
		int		dataLength = atoi(ip->getData(1)); 
		int		dataScale = atoi(ip->getData(2));
		bool    number = false;
		
		if(dataType=="VARCHAR" || dataType=="VARYING")
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
		else if(dataType=="FLOAT" || dataType=="DOUBLE")
		{
			attr.rep_.type_ = TeREAL;
		}
		else if(dataType=="LONG")
		{
			attr.rep_.type_ = TeINT;
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
	
	delete ip;
	return (status);
}


bool
TeFireBird::createTable(const string& table, TeAttributeList &attr)
{
	short	cont=0;
	string	pkeys ="";
	bool	hasAutoNumber=false;
	string	fieldName="";

	if(tableExist(table))
	{
		errorMessage_= "Table already exist!";
		return false;
	}

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
				{
					int iSize;
					if((*it).rep_.isPrimaryKey_ && (*it).rep_.numChar_>200) iSize=200;
					else													iSize=(*it).rep_.numChar_;
					tablec += (*it).rep_.name_ + " VARCHAR(" + Te2String(iSize) + ")";
				}break;

			case TeREAL:
				tablec += (*it).rep_.name_ + " DOUBLE PRECISION ";
				break;

			case TeUNSIGNEDINT:
				{
					tablec += (*it).rep_.name_ + " INTEGER ";
					if((*it).rep_.isAutoNumber_) 
						{
							hasAutoNumber=true;
							fieldName=(*it).rep_.name_;
						}
				}break;

			case TeINT:
				{
					tablec += (*it).rep_.name_ + " INTEGER ";
					if((*it).rep_.isAutoNumber_) 
						{
							hasAutoNumber=true;
							fieldName=(*it).rep_.name_;
						}
				}break;

			case TeDATETIME:
				tablec += (*it).rep_.name_ + " TIMESTAMP ";
				break;

			case TeCHARACTER:
				tablec += (*it).rep_.name_ + " CHAR ";
				break;

			case TeBLOB:
				tablec += (*it).rep_.name_ + " BLOB ";
				break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
			case TeNODETYPE:
			case TeNODESETTYPE:
				{
					tablec += "x	 DOUBLE PRECISION  DEFAULT 0.000 ";
					tablec += ", y	 DOUBLE PRECISION  DEFAULT 0.000 ";
					++it;
					continue;
				}
			case TeLINE2DTYPE:
			case TeLINESETTYPE:
				{
					tablec += " num_coords INT NOT NULL ,";
					tablec += " lower_x DOUBLE PRECISION NOT NULL ,";
					tablec += " lower_y DOUBLE PRECISION NOT NULL ,";
					tablec += " upper_x DOUBLE PRECISION NOT NULL ,";
					tablec += " upper_y DOUBLE PRECISION NOT NULL ,";
					tablec += " ext_max DOUBLE PRECISION NOT NULL, ";
					tablec += " spatial_data BLOB ";
					++it;
					continue;
				}
			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
				{
					tablec += " num_coords INT NOT NULL ,";
					tablec += " num_holes INT NOT NULL ,";
					tablec += " parent_id INT NOT NULL ,";
					tablec += " lower_x DOUBLE PRECISION NOT NULL ,";
					tablec += " lower_y DOUBLE PRECISION NOT NULL ,";
					tablec += " upper_x DOUBLE PRECISION NOT NULL ,";
					tablec += " upper_y DOUBLE PRECISION NOT NULL ,";
					tablec += " ext_max DOUBLE PRECISION NOT NULL ,";
					tablec += " spatial_data BLOB ";
					++it;
					continue;
				}
			case TeCELLTYPE:
			case TeCELLSETTYPE:
				{
					tablec += " lower_x	DOUBLE PRECISION NOT NULL ,";
					tablec += " lower_y	DOUBLE PRECISION NOT NULL  ,";
					tablec += " upper_x	DOUBLE PRECISION NOT NULL ,";
					tablec += " upper_y	DOUBLE PRECISION NOT NULL ,";
					tablec += " col_number	INT NOT NULL ,";
					tablec += " row_number	INT NOT NULL ";
					++it;
					continue;	
				}
			case TeRASTERTYPE:
				{
					tablec += " lower_x DOUBLE PRECISION NOT NULL, ";
					tablec += " lower_y DOUBLE PRECISION NOT NULL, ";
					tablec += " upper_x DOUBLE PRECISION NOT NULL, ";
					tablec += " upper_y DOUBLE PRECISION NOT NULL, ";
					tablec += " band_id INT NOT NULL, ";
					tablec += " resolution_factor INT NOT NULL , ";
					tablec += " subband INT ,";
					tablec += " spatial_data BLOB, ";
					tablec += " block_size INT NOT NULL ";
					++it;
					continue;
				}
			default:
				tablec += (*it).rep_.name_ + " VARCHAR(200) ";
			break;
		}

		// check if column is part of primary key
		if ((*it).rep_.isPrimaryKey_ && (*it).rep_.type_ != TeBLOB )
		{
			if (!pkeys.empty())
				pkeys += ", ";
			pkeys += (*it).rep_.name_;
			tablec += " NOT NULL";
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
			createSequence(table);	
	return true;
}

bool
TeFireBird::deleteTable (const string& table)
{
	int f =	table.find ("te_collection", std::string::npos);

	if( table=="te_theme" ||
		table=="te_layer" ||
		table=="te_representation" ||
		table=="te_tables_relation" ||
		table=="te_layer_table" ||
		table=="te_color_scheme" ||
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
	if(!execute(del))
		return false;

	string seq = "DELETE FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME ='"+ getNameSequence(table) + "'";
	if(!execute(seq))
		return false;

	return true;
}

bool 
TeFireBird::addColumn (const string& table, TeAttributeRep &rep)
{
	if(!tableExist(table))
		return false;

	string tab;
	tab = " ALTER TABLE " + table + " ADD ( ";
	tab += rep.name_ + "  ";
	
	switch (rep.type_)
	{
		case TeSTRING:
			tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
			
		case TeREAL:
			tab += "FLOAT";	
			break;
			
		case TeINT:
			tab += "INTEGER";
			break;

		case TeDATETIME:
			tab += "TIMESTAMP";

		case TeCHARACTER:
			tab += "CHAR";
		
		case TeBLOB:
			tab += "BLOB";
		
		default:
			tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	tab += " ) ";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting a column to table " + table + " !";
		return false;
	}

	return true;
}


bool
TeFireBird::createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion)
{
	
	string relation = "ALTER TABLE " + table + " ADD ";
	relation += " CONSTRAINT " + name;
	relation += " FOREIGN KEY (" + fieldName + ") "; 
	relation += " REFERENCES " + relatedTable + "(" + relatedField + ")";
		
	if (cascadeDeletion)
		relation += " ON DELETE CASCADE ";

	if(!execute(relation))
		return false;

	return true;
}


TeDBRelationType 
TeFireBird::existRelation(const string& tableName, const string& relName)
{

	TeFireBirdPortal  *ip = (TeFireBirdPortal*)getPortal();
	if(!ip)
		return TeNoRelation;
	


	string integ="select * from RDB$RELATION_CONSTRAINTS ";
	integ+=" where  rdb$relation_name='" + TeConvertToUpperCase(tableName) + "'";
	integ+=" and	rdb$constraint_name='" + TeConvertToUpperCase(relName) + "'";

	if(!ip->query(integ))
	{
		delete ip;
		return TeNoRelation;
	}
	string cascade;
	if(ip->fetchRow())
	{
		cascade = string(ip->getData(0));
		if(cascade== "CASCADE")
		{
			delete ip;
			return TeRICascadeDeletion;
		}
		
		delete ip;
		return TeRINoCascadeDeletion;
	}
	delete ip;
	return TeNoRelation;	
}


TeDatabasePortal*  
TeFireBird::getPortal ()
{
	TeFireBirdPortal* portal = new TeFireBirdPortal (this);
	return portal;
}

bool
TeFireBird::createSequence(const string &tableName)
{
	string nameSeq = getNameSequence(tableName);
	string seq = " CREATE GENERATOR  " + nameSeq;
	
	if (!execute(seq))
	{	
		if(errorMessage_.empty())
			errorMessage_ = "Error creating sequence to table " + tableName + " !";
		return false;
	}
	return true;
}

string
TeFireBird::getNameSequence(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_TR";
	else
		name = tableName + "_TR";

	return name;
}

string
TeFireBird::getNameTrigger(const string &tableName)
{
	string name;
	if(tableName.size()>21)
		name = tableName.substr(0,20) + "_tri";
	else
		name = tableName + "_tri";

	return name;
}

bool
TeFireBird::deleteLayer(int layerId)
{
	//Delete attributes tables
	if(!deleteLayerTable(layerId))
		return false;

	TeFireBirdPortal* portal = (TeFireBirdPortal*) this->getPortal();
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
			TeFireBirdPortal* portal2 = (TeFireBirdPortal*)this->getPortal();
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

					sql= "DELETE FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME ='"+ getNameSequence(tabName) + "'";
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
			
			sql= "DELETE FROM RDB$GENERATORS WHERE RDB$GENERATOR_NAME ='"+ getNameSequence(geomTable) + "'";
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

unsigned int 
TeFireBird::getSequenceValue(const std::string &seqName)
{
	unsigned int value;
	TeFireBirdPortal *ip = (TeFireBirdPortal*) getPortal();
	if(!ip)		return false;

	string seq = "select gen_id(" + seqName + ",1) from rdb$generators where rdb$generator_name='" + seqName + "'";
	if(!ip->query(seq))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence te_projection_seq!";  
		delete ip;
		return false;
	}

	if(!ip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ip;
		return false;
	}

	value=(unsigned int)atoi((const char*)ip->getData(0)); 
	return value;
}

bool
TeFireBird::insertProjection(TeProjection* proj)
{
	proj->id(getSequenceValue("TE_PROJECTION_TR")); 

	string insert = "INSERT INTO te_projection (projection_id, name, long0, lat0,";
	insert += " offx, offy, stlat1, stlat2, unit, scale, hemis, datum, ";
	insert += " radius, flattening, dx , dy, dz ) VALUES ( ";
	insert += Te2String(proj->id());
	insert += ", '" + escapeSequence(proj->name()) + "'";
	insert += ", " + Te2String(proj->params().lon0*TeCRD,10);
	insert += ", " + Te2String(proj->params().lat0*TeCRD,10);
	insert += ", " + Te2String(proj->params().offx,10);
	insert += ", " + Te2String(proj->params().offy,10);
	insert += ", " + Te2String(proj->params().stlat1*TeCRD,10);
	insert += ", " + Te2String(proj->params().stlat2*TeCRD,10);
	insert += ", '" + escapeSequence(proj->params().units) + "'";
	insert += ", " + Te2String(proj->params().scale,10);
	insert += ", " + Te2String(proj->params().hemisphere);
	insert += ", '" + escapeSequence(proj->params().datum.name()) + "'";
	insert += ", " + Te2String(proj->params().datum.radius(),10);
	insert += ", " + Te2String(proj->params().datum.flattening(),10);
	insert += ", " + Te2String(proj->params().datum.xShift(),10);
	insert += ", " + Te2String(proj->params().datum.yShift(),10);
	insert += ", " + Te2String(proj->params().datum.zShift(),10);
	insert += ")";

	if(!execute(insert))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_projection!";   
		return false;
	}
	return true;
}

bool 
TeFireBird::insertTableInfo(int layerId, TeTable &table, const string& user)
{
	string ins;
	string seq = TeConvertToUpperCase(getNameSequence("te_layer_table"));
	
	table.setId(getSequenceValue(seq));
	ins = "INSERT INTO te_layer_table ( "; 
	ins += " table_id, layer_id, attr_table, unique_id, attr_link, ";
	ins += " attr_initial_time, attr_final_time, attr_time_unit, ";
	ins += " attr_table_type, user_name) VALUES ( ";
	ins += Te2String(table.id()); 
	ins += ", " + Te2String(layerId);
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
	return true;
}

bool 
TeFireBird::insertRelationInfo(const int tableId, const string& tField,
						  const string& rName, const string& rField, int& relId)
{
	TeFireBirdPortal *ip = (TeFireBirdPortal*)getPortal();
	if(!ip)
		return false;

	relId = -1;
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " table_id = " + Te2String(tableId);
	sel += " AND attr_link = '" + tField + "'";
	sel += " AND related_table = '" + rName + "'";
	sel += " AND related_link = '" + rField + "'";
	if (!ip->query(sel))
	{
		delete ip;
		return false;
	}

	if (ip->fetchRow())
	{
		relId = atoi(ip->getData(0));
		delete ip;
		return true;
	}

	string seq, ins;
	seq = getNameSequence("te_tables_relation");	
	ins = " INSERT INTO te_tables_relation ( "; 
	ins += " relation_id, attr_link, table_id, ";
	ins += " related_table, related_link) VALUES ( ";
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
		delete ip;
		return false;
	}

	ins = "SELECT "+ seq +".CURRVAL FROM DUAL";
	if (!ip->query(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error in the sequence te_table_relation_seq!";  
		delete ip;
		return false;
	}

	if(!ip->fetchRow())
	{
		errorMessage_ = "Sequence value not found!";
		delete ip;
		return false;
	}

	relId = atoi((const char*)ip->getData(0));
	delete ip;
	return true;
}


bool 
TeFireBird::insertView (TeView *view)
{
		// save it´s projection
	std::string ins;
	TeProjection* proj = view->projection();
	if ( !proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir vista sem projeção";
		return false;
	}
	
	view->id(getSequenceValue("TE_VIEW_TR"));

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

	// Insert view in the view map
	viewMap()[view->id()] = view;
	return true;
}

bool
TeFireBird::insertViewTree (TeViewTree *tree)
{
	
	tree->id(getSequenceValue("Te_THEME_TR"));
	string ins = "INSERT INTO te_theme (theme_id, view_id, name, ";
	ins += " parent_id, node_type, priority) VALUES (";
	ins += Te2String(tree->id());
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
	return true;
}


bool 
TeFireBird::insertTheme (TeAbstractTheme *theme)
{

	double maxScale = theme->maxScale ();
	if(maxScale==TeMAXFLOAT)
		maxScale = 0.;


	
	theme->id(getSequenceValue("TE_THEME_TR"));
	

	string ins = "INSERT INTO te_theme (theme_id, layer_id, view_id, name, ";
	ins += " parent_id, priority, node_type, min_scale, max_scale, ";
	ins += " generate_attribute_where, generate_spatial_where, generate_temporal_where, ";
	ins += " collection_table, visible_rep, enable_visibility, lower_x, lower_y, upper_x, upper_y) VALUES (";
	ins += Te2String(theme->id());
	if(theme->type()==TeTHEME)		ins += ", " + Te2String(((TeTheme*)theme)->layerId());
	else							ins +=",NULL";
	ins += ", " + Te2String(theme->view());
	ins += ", '" + escapeSequence(theme->name()) + "'";
	ins += ", " + Te2String(theme->parentId ());
	ins += ", " + Te2String(theme->priority());
	ins += ", " + Te2String(theme->type ());
	ins += ", " + Te2String (theme->minScale(),9);
	ins += ", " + Te2String (maxScale);
	ins += ", '" + escapeSequence(theme->attributeRest()) + "'";
	ins += ", '" + escapeSequence(theme->spatialRest()) + "'";
	ins += ", '" + escapeSequence(theme->temporalRest()) + "'";
	if((theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME) && static_cast<TeTheme*>(theme)->collectionTable().empty())
		ins += ", '" + escapeSequence(((TeTheme*)theme)->collectionTable()) + "'";
	else 
		ins +=",NULL";
	ins += ", " + Te2String(theme->visibleRep());
	ins += ", " + Te2String(theme->visibility());
	ins += ", " + Te2String(theme->box().x1());
	ins += ", " + Te2String(theme->box().y1());
	ins += ", " + Te2String(theme->box().x2());
	ins += ", " + Te2String(theme->box().x2());
	ins += ")";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_theme!";   
		return false;
	}

	string colName;
	if((theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME) && static_cast<TeTheme*>(theme)->collectionTable().empty())
	{
		colName = ((TeTheme*)theme)->collectionTable(); 
		//insert collection name
		if(colName.empty())
		{
			colName = "te_collection_" + Te2String(theme->id());
			((TeTheme*)theme)->collectionTable(colName);	
		
			ins = "UPDATE te_theme SET ";
			ins += " collection_table = '" + escapeSequence(colName) + "'";
			ins += " WHERE theme_id = " + Te2String (theme->id());
			
			if(!execute(ins))
			{
				if(errorMessage_.empty())
					errorMessage_ = "Error inserting in the table te_theme!";   
				return false;
			}
		}
	}
	
	unsigned int numSlices = 0;
	if(theme->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (theme->id(), theme->grouping()))
			return false;
		numSlices = theme->grouping().groupNumSlices_;
	}
		
	// insert legend
	theme->outOfCollectionLegend().group(-1); 
	theme->outOfCollectionLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->outOfCollectionLegend()))) 
		return false;

	theme->withoutDataConnectionLegend().group(-2); 
	theme->withoutDataConnectionLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->withoutDataConnectionLegend()))) 
		return false;

	theme->defaultLegend().group(-3); 
	theme->defaultLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->defaultLegend()))) 
		return false;

	theme->pointingLegend().group(-4); 
	theme->pointingLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->pointingLegend()))) 
		return false;

	theme->queryLegend().group(-5); 
	theme->queryLegend().theme(theme->id()); 
	if (!insertLegend(&(theme->queryLegend()))) 
		return false;

	theme->queryAndPointingLegend().group(-6); 
	theme->queryAndPointingLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->queryAndPointingLegend())))
		return false;

    if(theme->legend().size() >= numSlices)
    {
        for (unsigned int i = 0; i < numSlices; ++i)
	    {
		    theme->legend()[i].group(i);
		    theme->legend()[i].theme(theme->id());
		    if (!insertLegend (&(theme->legend()[i])))
			    return false;
	    }
    }

    //insert metadata theme
	if(!theme->saveMetadata(this))
		return false;
	
	themeMap()[theme->id()] = theme;
	return true;
}


bool 
TeFireBird::insertThemeTable (int themeId, int tableId, int relationId, int tableOrder)
{
	string ins = "INSERT INTO te_theme_table ";
	ins += " (theme_table_id, theme_id, table_id, relation_id, table_order)";
	ins += " VALUES (gen_id(TE_THEME_TABLE_TR,1) ";
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
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_theme_table!";   
		return false;
	}

	return true;
}

bool 
TeFireBird::insertThemeGroup(TeViewTree* tree)
{
	try
	{
		tree->id(getSequenceValue("TE_THEME_TR"));
		string ins = "INSERT INTO te_theme (theme_id, view_id, name, ";
		ins += " parent_id, priority, node_type ) VALUES (";
		ins += Te2String(tree->id());
		ins += ", " + Te2String(tree->view());
		ins += ", '" + escapeSequence(tree->name()) + "'";
		ins += ", " + Te2String(tree->parentId ());
		ins += ", " + Te2String(tree->priority());
		ins += ", " + Te2String(1);
		ins += ")";
		if(!execute(ins))
		{
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_theme!";   
			return false;
		}
	}
	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_theme!";
		return false;
	}
	return true;
}

bool 
TeFireBird::generateLabelPositions (TeTheme *theme)
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
		upd += "FROM " + geomTable + " WHERE object_id = " +  collTable + ".c_object_id), ";
		
		upd += " label_y = (SELECT MAX(lower_y + (upper_y - lower_y)/2) ";
		upd += "FROM " + geomTable + " WHERE object_id =" + collTable + ".c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}
	else if(theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);
		
		upd= " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(x) ";
		upd += " FROM " + geomTable + " p WHERE object_id =" + collTable + ".c_object_id), ";
		
		upd += " label_y = (SELECT MAX(y) ";
		upd += " FROM " + geomTable + " p WHERE object_id =" + collTable + ".c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}


	if(!execute(upd))
		return false;

	return true;
}


bool 
TeFireBird::insertLayer (TeLayer* layer)
{
	
	std::string		ins;
	char			value[1024];

	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir layer sem projeção";
		return false;
	}

	layer->id(getSequenceValue("TE_LAYER_TR"));

	ins = "INSERT INTO te_layer (layer_id, projection_id, name ";
	ins += ", lower_x, lower_y, upper_x, upper_y) ";
	ins += " VALUES ( ";
	ins += Te2String(layer->id());
	ins += ", " + Te2String(proj->id());
	ins += ", '" + escapeSequence(layer->name()) + "'";

	sprintf(value,",%g,%g,%g,%g)",layer->box().x1(),layer->box().y1(),layer->box().x2(),layer->box().y2());
	ins +=value;
	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table te_layer!";   
		return false;
	}
	layerMap()[layer->id()] = layer;
	return true;
}

bool 
TeFireBird::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;
	
	string ins;
	rep.id_ = getSequenceValue("TE_REPRESENTATION_TR");

	try
	{
		char value[1024];

		ins = " INSERT INTO te_representation (repres_id, layer_id, geom_type, geom_table, ";
		ins += " description, lower_x, lower_y, upper_x, upper_y, res_x, res_y, num_cols, ";
		ins += " num_rows) VALUES (";
		ins += Te2String(rep.id_);
		ins += ", " + Te2String(layerId);
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

		if(!execute(ins))
		{
			if(errorMessage_.empty())
				errorMessage_ = "Error inserting in the table te_representation!";   
			return false;
		}
	}
	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_representation!"; 
		return false;
	}
	return true;
}	


bool 
TeFireBird::insertLegend (TeLegendEntry* leg)
{
	try
	{
		leg->id(getSequenceValue("TE_LEGEND_TR"));
		string ins = "INSERT INTO te_legend (legend_id, theme_id, group_id, ";
		ins += " num_objs, lower_value, upper_value, label) VALUES ( ";
		ins += Te2String(leg->id());
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
	}
	catch(...)
	{
		errorMessage_ = "Error inserting in the table te_legend!"; 
		return false;
	}
		
	legendMap()[leg->id()] = leg;
	return insertVisual(leg);
}

bool 
TeFireBird::insertPolygonSet (const string& table, TePolygonSet &ps)
{
	
	for (unsigned int i = 0; i < ps.size(); i++ )
	{
		TePolygon poly = ps [i];
		if (!insertPolygon (table,poly))
			return false;
	}
	return true;
}


bool 
TeFireBird::insertPolygon (const string& table, TePolygon &poly)
{
	
	ADODB::_RecordsetPtr	recset_;
	double					extmax;
	unsigned int			k, ni, size;
	double*					points	= NULL;
	std::string				strSQL;


	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	
	recset_->CursorLocation=ADODB::adUseServer;
	
	strSQL="select first 1 * from " + table ;
	recset_->Open(_bstr_t(strSQL.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdText);
	
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


			string seqName=TeConvertToUpperCase(getNameSequence(table));
			string seq = "select gen_id(" + seqName + ",1) from rdb$generators where rdb$generator_name='" + seqName + "'";
		
			ring.geomId (getSequenceValue(seqName));
			
			if ( k==0)	parentId = ring.geomId();
					
			recset_->AddNew();
			recset_->Fields->GetItem("geom_id")->Value=(_variant_t) ((long)ring.geomId());
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (poly.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)ni);
			recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (b.lowerLeft().x());
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (b.lowerLeft().y());
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (b.upperRight().x());
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (b.upperRight().y());
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

			// Create and save the BLOB
			VARIANT varPoints;

			if(!LinearRingToVariant(varPoints, ring))
			{
				recset_->Close();
				return false;
			}

			recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);
			recset_->Update();
			SafeArrayDestroy (varPoints.parray);

			
		} //for
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

	recset_->Close();
	return true;
}

bool 
TeFireBird::insertLineSet(const string& table, TeLineSet &ls)
{		
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
		
	recset_->CursorLocation=ADODB::adUseServer;
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenForwardOnly, 
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);
	
	double	extmax;
	int		size;
	double	*points	= NULL;

	for ( unsigned int i = 0; i < ls.size(); i++ )
	{
		TeLine2D line = ls [i];
		try
		{
			string seqName=TeConvertToUpperCase(getNameSequence(table));
			line.geomId (getSequenceValue(seqName));
			
			size = line.size();
			TeBox b = line.box();
			extmax = MAX(b.width(),b.height());
			recset_->AddNew();
			recset_->Fields->GetItem("geom_id")->Value=(long) line.geomId();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (b.lowerLeft().x());
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (b.lowerLeft().y());
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (b.upperRight().x());
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (b.upperRight().y());
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

			// Create and save the BLOB
			VARIANT varPoints;

			if (!LineToVariant(varPoints, line))
			{
				recset_->Close();
				return false;
			}

			
			recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);
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
	}
	recset_->Close();
	return true;
}


bool 
TeFireBird::insertLine (const string& table, TeLine2D &line)
{
	ADODB::_RecordsetPtr	recset_;
	std::string				seqName;
	std::string				seq;
	std::string				strSQL;
	double					extmax;
	int						size;
	double*					points	= NULL;
	
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	recset_->CursorLocation=ADODB::adUseServer;


	strSQL="select first 1 * from " + table ;
	recset_->Open(_bstr_t(strSQL.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdText);
	try
	{
		seqName=TeConvertToUpperCase(getNameSequence(table));
		seq = "select gen_id(" + seqName + ",1) from rdb$generators where rdb$generator_name='" + seqName + "'";
		line.geomId (getSequenceValue(seqName));			

		size = line.size();
		TeBox b = line.box();
		extmax = MAX(b.width(),b.height());
		recset_->AddNew();
		recset_->Fields->GetItem("geom_id")->Value = (long) line.geomId();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
		recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (b.lowerLeft().x());
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (b.lowerLeft().y());
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (b.upperRight().x());
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (b.upperRight().y());
		recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);
		
		// Create and save the BLOB
		VARIANT varPoints;
		if (!LineToVariant(varPoints, line))
		{
			recset_->Close();
			return false;
		}
		recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);		
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
TeFireBird::insertPointSet (const string& table, TePointSet &ps)
{
	for (unsigned int i = 0; i < ps.size(); i++ )
	{
		TePoint point = ps [i];
		if (!insertPoint (table,point))
			return false;
	}
	return true;
}

bool 
TeFireBird::insertPoint (const string& table, TePoint &p)
{
	ADODB::_RecordsetPtr	recset_;
	std::string				seqName;
	
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	recset_->CursorLocation=ADODB::adUseServer;

	string strSQL;
	strSQL="select first 1 * from " + table ;
	recset_->Open(_bstr_t(strSQL.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdText);

	try
	{
		seqName=TeConvertToUpperCase(getNameSequence(table));
		p.geomId(getSequenceValue(seqName));
		recset_->AddNew();
		recset_->Fields->GetItem("geom_id")->Value=(_variant_t)((long)p.geomId());
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
		recset_->Fields->GetItem("x")->Value = (_variant_t) (p.elem().x());
		recset_->Fields->GetItem("y")->Value = (_variant_t) (p.elem().y());
		recset_->Update();	
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
TeFireBird::insertCellSet (const string& table, TeCellSet &cs)
{
	for (unsigned int i = 0; i < cs.size(); i++ )
	{
		TeCell cell = cs [i];
		if (!insertCell (table,cell))
			return false;
	}
	return true;
}


bool 
TeFireBird::insertCell (const string& table, TeCell &cell)
{
	TeBox b = cell.box();

	string seqName=TeConvertToUpperCase(getNameSequence(table));
	cell.geomId(getSequenceValue(seqName)); 
	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, col_number, row_number, lower_x, ";
	ins += " lower_y, upper_x, upper_y ) ";
	ins += " VALUES ( ";
	ins += Te2String(cell.geomId());
	ins += ", '" + escapeSequence(cell.objectId ()) + "'";
	ins += ",  " + Te2String(cell.column ());
	ins += ",  " + Te2String(cell.line ());
	ins += ",  " + Te2String(b.lowerLeft().x());
	ins += ",  " + Te2String(b.lowerLeft().y());
	ins += ",  " + Te2String(b.upperRight().x());
	ins += ",  " + Te2String(b.upperRight().y());
	ins += " )";
		
	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}
	return true;
}

bool 
TeFireBird::insertTextSet (const string& table, TeTextSet &ts)
{
	for (unsigned int i = 0; i < ts.size(); i++ )
	{
		TeText text = ts [i];
		if (!insertText (table,text))
			return false;
	}
	return true;
}

bool 
TeFireBird::insertText(const string& table, TeText &text)
{
	ADODB::_RecordsetPtr	recset_;
	TeFireBirdPortal		*ip=NULL;
	std::string				seqName;

	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	recset_->CursorLocation=ADODB::adUseServer;
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenForwardOnly, 
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);

	try
	{
		seqName=TeConvertToUpperCase(getNameSequence(table));
		text.geomId(getSequenceValue(seqName));
		recset_->AddNew();
		recset_->Fields->GetItem("geom_id")->Value=(_variant_t)((long)text.geomId());
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (text.objectId().c_str());
		recset_->Fields->GetItem("x")->Value = (_variant_t) (text.elem().x());
		recset_->Fields->GetItem("y")->Value = (_variant_t) (text.elem().y());
		recset_->Fields->GetItem("text_value")->Value = (_bstr_t) (text.textValue().c_str());
		recset_->Fields->GetItem("angle")->Value = (_variant_t) (text.angle());
		recset_->Fields->GetItem("height")->Value = (_variant_t) (text.height());
		recset_->Fields->GetItem("alignment_vert")->Value = (_variant_t) (text.alignmentVert());
		recset_->Fields->GetItem("alignment_horiz")->Value = (_variant_t) (text.alignmentHoriz());
		recset_->Update();
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
TeFireBird::insertArcSet (const string& table, TeArcSet &as)
{
	for (unsigned int i = 0; i < as.size(); i++ )
	{
		TeArc arc = as [i];
		if (!insertArc (table,arc))
			return false;
	}
	return true;
}

bool 
TeFireBird::insertArc (const string& table, TeArc &arc)
{
	arc.geomId(getSequenceValue(TeConvertToUpperCase(getNameSequence(table)))); 
	string ins = "INSERT INTO " + table + " (geom_id, ";
	ins += " object_id, from_node, to_node ) ";
	ins += " VALUES ( ";
	ins += Te2String(arc.geomId());
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
	return true;
}

bool 
TeFireBird::insertNodeSet (const string& table, TeNodeSet &ns)
{
	for (unsigned int i = 0; i < ns.size(); i++ )
	{
	     TeNode no = ns [i];
	     if (!insertNode (table,no))
		return false;
	}
	return true;
}

bool 
TeFireBird::insertNode (const string& table, TeNode &node)
{
	node.geomId(getSequenceValue(TeConvertToUpperCase(getNameSequence(table)))); 
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, x, y) ";
	ins += " VALUES ( ";
	ins += Te2String(node.geomId());
	ins += ", '" + escapeSequence(node.objectId()) + "'";
	ins += ",  " + Te2String(node.location().x());
	ins += ",  " + Te2String(node.location().y());
	ins += " ) ";

	if(!execute(ins))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}
	return true;
}

bool 
TeFireBird::insertRasterGeometry(const string& tableName, TeRasterParams& par, const string& objectId)
{
	std::string			objId; 
	int					geomId;


	if (tableName.empty())
		return false;
		
	if (objectId.empty())	objId = "O1";
	else					objId = objectId;

	geomId =getSequenceValue(TeConvertToUpperCase(getNameSequence(tableName)));
		
	TeBox box = par.boundingBox();

	string ins = "INSERT INTO " + tableName + " (geom_id, object_id, raster_table, lut_table, ";
	ins += "res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, ";
	ins += "lower_x, lower_y, upper_x, upper_y) ";
	ins += " VALUES ("+ Te2String(geomId) +", '" + objId + "', '" + par.fileName_+ "', '" +  par.lutName_ + "', ";
	ins += Te2String(par.resx_) + ", " + Te2String(par.resy_) + ", ";
	ins += Te2String(par.nBands()) + ", " + Te2String(par.ncols_) + ", " + Te2String(par.nlines_) + ", ";
	ins += Te2String(par.blockHeight_) + ", " + Te2String(par.blockWidth_) + ", ";
	ins += Te2String(box.x1_,8) +", " + Te2String(box.y1_,8) + ", ";
	ins += Te2String(box.x2_,8) +", " + Te2String(box.y2_,8) + ")";
	if (!this->execute(ins))
	{
		return false;
	}
	string 	metadataTableName = tableName+"_metadata";
	return insertRasterMetadata(metadataTableName, geomId,par);
}

bool 
TeFireBird::insertTable (TeTable &table)
{
	string tableName = table.name();
	int size = table.size();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	TeAttributeList::iterator itEnd = att.end();

	TeTableRow row;
	int i;
	unsigned int j;
	for ( i = 0; i < size; i++  )
	{
		row = table[i];
		it = att.begin();
		string q = 	"INSERT INTO "+tableName+" VALUES(";
		j = 1;
		int jj = 0;
		while ( it != itEnd )
		{
			if (j != 1)
				q += ", ";

  			string oracleFormat = "";
			string dateTime = "";
			if(((*it).rep_.type_==TeDATETIME) && (!row[jj].empty()))
			{
				TeTime t  (row[jj], (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_,
						   (*it).timeSeparator_, (*it).indicatorPM_);
				
				dateTime = t.getDateTime("MMsDDsYYYYsHHsmmsSS");
				//oracleFormat = " TO_DATE('" + dateTime + "', 'DD/MM/YYYY HH24:MI:SS')";
				oracleFormat="CAST ('" + dateTime + "' As Timestamp)";
			}
			
			switch ((*it).rep_.type_)
  			{
  				case TeSTRING:
					q += "'"+ escapeSequence(row[jj]) +"'";
  				break;
  				case TeREAL:
					q += row[jj]; 
  				break;
  				case TeINT:
					q += row[jj];
  				break;
				case TeDATETIME:
					q += oracleFormat;
  				break;
				case TeCHARACTER:
					q += "'" + escapeSequence(row[jj]) + "'";
  				break;
				case TeBLOB:
					q += " EMPTY_BLOB() ";
				break;
  				default:
					q += "'"+ escapeSequence(row[jj]) +"'";
  				break;
  			}
			++it;
			j++;
			jj++;
		}
		q += ")";
		
		if (!execute(q))
			continue;
	}
	return true;
}

bool 
TeFireBird::updateRepresentation (int layerId, TeRepresentation& rep)
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

	return this->execute(sql);
}



bool
TeFireBird::updateTable (TeTable &table)
{
	string tableName = table.name();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();

	TeTableRow row;
	unsigned int i, j;
	string uniqueName = table.uniqueName();
	string uniqueVal;
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
				oracleFormat="CAST ('" + dateTime + "' As Timestamp)";
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
		if (!execute(q))
			continue;
	}
	return true;
}

string 
TeFireBird::getSQLAutoNumber(const string& table)
{
	string aut = getNameSequence(table) +".NEXTVAL";
	return aut;
}


//----- TeOraclePortal methods ---

TeFireBirdPortal::TeFireBirdPortal ( TeDatabase*  pDatabase) 
{
	db_ = pDatabase;
	connection_ = ((TeFireBird*)pDatabase)->connection_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
}


TeFireBirdPortal::TeFireBirdPortal ()
{
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	curRow_ = 0;
}

TeFireBirdPortal::~TeFireBirdPortal ()
{
	freeResult();
}

bool 
TeFireBirdPortal::query (const string &qry, TeCursorLocation l, TeCursorType t, TeCursorEditType e )
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
		numRows_ =0;// recset_->GetRecordCount();
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
		HRESULT hr = ptrFields->get_Item(vCol, &ptrField);
		TeAttribute attribute;
		int nType = ptrField->Type;
		switch (nType)
		{
		case ADODB::adBoolean:
		case ADODB::adNumeric:
		case ADODB::adInteger:
		case ADODB::adSmallInt:
		case ADODB::adTinyInt:
		case ADODB::adUnsignedInt:
		case ADODB::adUnsignedSmallInt:
		case ADODB::adUnsignedTinyInt:
			attribute.rep_.type_ = TeINT;
			break;

		case ADODB::adSingle:
		case ADODB::adDouble:
			attribute.rep_.type_ = TeREAL;
			break;

		case ADODB::adDate:
		case ADODB::adDBDate:
		case ADODB::adDBTime:
		case ADODB::adDBTimeStamp:
			attribute.rep_.type_ = TeDATETIME;
			attribute.dateChronon_ = TeSECOND;
			attribute.dateTimeFormat_ = "DD/MM/YYYY HH24:MI:SS";
			break;

		case ADODB::adLongVarBinary:
		case ADODB::adVarBinary:
		case ADODB::adBinary:
			attribute.rep_.type_ = TeBLOB;
			break;

		case ADODB::adWChar:
		case ADODB::adVarChar:
		case ADODB::adVarWChar:
		case ADODB::adLongVarChar:
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
TeFireBirdPortal::getDate (const string& name)
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
		t = TeTime(string((char*)bvalue_), TeSECOND, "DD/MM/YYYY HH24:MI:SS"); 
		return t;
	}
	else
		return t;
}


TeTime
TeFireBirdPortal::getDate (int i)
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
		t = TeTime (string((char*)bvalue_), TeSECOND, "DD/MM/YYYY HH24:MI:SS"); 		
		return t;
	}
	else
		return t;
}


string
TeFireBirdPortal::getDateAsString(int i)
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
TeFireBirdPortal::getDateAsString(const string& s)
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
TeFireBird::createAutoIncrementTrigger(const string &tableName, const string &fieldName)
{
	string nameTri = getNameTrigger(tableName);
	string nameSeq = getNameSequence(tableName);



	string tri=" CREATE TRIGGER " + nameTri + " FOR " + tableName;
	tri+=" ACTIVE BEFORE INSERT POSITION 0 ";
	tri+=" AS ";
	tri+=" begin ";
	tri+=" new." + fieldName + "=gen_id(" + nameSeq + ",1);";
	tri+=" end ";
	
	if (!execute(tri))
	{	
		if(errorMessage_.empty())
			errorMessage_ = "Error creating trigger to table " + tableName + " !";
		return false;
	}

	return true;
}




bool 
TeFireBird::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	return(TeDatabase::getAttributeList (tableName, attList));
}



bool 
TeFireBird::updatePolygon (const string& table, TePolygon &p )
{
	double	extmax;
	unsigned int k, ni, size;
	double	*points	= NULL;

	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	try
	{
		TeBox b = p[0].box();
		for ( k = 0; k < p.size(); k++ )
		{
			TeLinearRing ring ( p[k] );
			string q ="SELECT * FROM " + table;

			q += " WHERE geom_id = " + Te2String(ring.geomId());

			if (!portal->query (q))
			{
				delete portal;
				return false;
			}

			if (!portal->fetchRow())
			{
				portal->freeResult();
				delete portal;
				return false;
			}
			size = ring.size();
			ni = 0;
			if (k==0)
			{
				extmax = MAX(b.width(),b.height());
				ni = p.size()-1;
			}
			else
				ni = 0;

			portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
			portal->recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			portal->recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)ni);
			portal->recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)k);
			portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (b.lowerLeft().x());
			portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (b.lowerLeft().y());
			portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (b.upperRight().x());
			portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (b.upperRight().y());
			portal->recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);
// Create and save the BLOB
			VARIANT varPoints;

			if(!LinearRingToVariant(varPoints, ring))
			{
				delete portal;
				return false;
			}

			portal->recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);
			portal->recset_->Update();
			SafeArrayDestroy (varPoints.parray);
			portal->recset_->Close();
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}



bool 
TeFireBird::updateLine (const string& table, TeLine2D &line)
{
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	string q ="SELECT * FROM " + table;
	q += " WHERE geom_id = " + Te2String(line.geomId());

	if (!portal->query (q))
	{
		delete portal;
		return false;
	}

	if (!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return false;
	}

	double	extmax;
	int		size;
	double	*points	= NULL;
	size = line.size();
	TeBox b = line.box();
	extmax = MAX(b.width(),b.height());

	try
	{
		portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
		portal->recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (b.lowerLeft().x());
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (b.lowerLeft().y());
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (b.upperRight().x());
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (b.upperRight().y());
		portal->recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

// Create and save the BLOB
		VARIANT varPoints;

		if (!LineToVariant(varPoints, line))
		{
			delete portal;
			return false;
		}

		portal->recset_->Fields->GetItem("spatial_data")->Value = (_variant_t) (varPoints);
		portal->recset_->Update();
		portal->recset_->Close();

		SafeArrayDestroy (varPoints.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}



bool 
TeFireBird::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, 
					     unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{

	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	bool update = false;
	string q ="SELECT * FROM " + table; 
	q += " WHERE block_id='" + blockId + "'";

	if (!portal->query (q))
	{
		delete portal;
		return false;
	}
		// check if this block is alread in the database
	if (portal->fetchRow())
		update = true;

	try
	{
		if (!update)
		{
			portal->freeResult();
			portal->recset_->Open(_bstr_t(table.c_str()),
				_variant_t((IDispatch*)connection_,true),
				ADODB::adOpenKeyset,
				ADODB::adLockOptimistic,
				ADODB::adCmdTable);
			portal->recset_->AddNew();
		}
		portal->recset_->Fields->GetItem("block_id")->Value = (_bstr_t) blockId.c_str();
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) ll.x();
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) ll.y();
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) ur.x();
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) ur.y();
		portal->recset_->Fields->GetItem("band_id")->Value = (long) band;
		portal->recset_->Fields->GetItem("resolution_factor")->Value = (long) res;
		portal->recset_->Fields->GetItem("subband")->Value = (long) subband;
		portal->recset_->Fields->GetItem("block_size")->Value = (long) size;

		VARIANT var;
		BYTE *pByte;

		// create a single dimensional byte array
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;	
		rgsabound[0].cElements = size;

		psa = SafeArrayCreate(VT_I1, 1, rgsabound);
			
		// set the data of the array with compressed data
		if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
			memcpy((LPVOID)pByte,(LPVOID)buf,size);
		SafeArrayUnaccessData(psa);

		var.vt = VT_ARRAY | VT_UI1;
		var.parray = psa;

		portal->recset_->Fields->GetItem("spatial_data")->Value = (_variant_t)(var);
		portal->recset_->Update();
		portal->recset_->Close();
		SafeArrayDestroy (var.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return false;
	}
	delete portal;
	return true;
}


bool 
TeFireBird::updateProjection (TeProjection *proj)
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
	sql += " datum='" + proj->datum().name() + "',";
	sql += " radius=" + Te2String(proj->datum().radius()) + ",";
	sql += " flattening=" + Te2String(proj->datum().flattening()) + ",";
	sql += " dx=" + Te2String(proj->datum().xShift()) + ",";
	sql += " dy=" + Te2String(proj->datum().yShift()) + ",";
	sql += " dz=" + Te2String(proj->datum().zShift()) ;
	sql += " WHERE projection_id = " + Te2String(proj->id());
	return this->execute(sql);
}



bool 
TeFireBird::insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size)
{
	if (whereClause.empty())
		return false;

	TeAdoPortal* portal = (TeAdoPortal*)getPortal();
	if (!portal)
		return false;

	string q = "SELECT * FROM "+ tableName +" WHERE " + whereClause;
	if ((!portal->query(q)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}

	try
	{
		BYTE *pByte;
		VARIANT var;
		
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;	
		rgsabound[0].cElements = size;

		// create a single dimensional byte array
		psa = SafeArrayCreate(VT_I1, 1, rgsabound);
			
		// set the data of the array with data in the edit box
		if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
			memcpy((LPVOID)pByte,(LPVOID)data, size * sizeof(unsigned char));
		SafeArrayUnaccessData(psa);
		var.vt = VT_ARRAY | VT_UI1;
		var.parray = psa;

		portal->recset_->Fields->Item[columnBlob.c_str()]->Value = (_variant_t)(var);
		portal->recset_->Update();
		portal->recset_->Close();
		SafeArrayDestroy (var.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error insert blob!";
		delete portal;
		return false;
	}

	delete portal;
	return true;
}
