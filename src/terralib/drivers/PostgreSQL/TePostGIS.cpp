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

#include "TePostGIS.h"
#include "TePGUtils.h"
#include "TePGInterface.h"

#define BUFFSIZE                1024

TePostGIS::TePostGIS()
{
	dbmsName_ = "PostGIS";
}
 
bool TePostGIS::newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port, bool terralibModel, const std::string& characterSet)
{
	errorMessage_ = "";

	string createDB  = "CREATE DATABASE ";
	       createDB += "\"" + database + "\"";
		   createDB += " TEMPLATE = template_postgis";

   if(!characterSet.empty())
	{
		createDB += " ENCODING = '" + characterSet + "'";
	}

	if(!connect(host, user, password, "template_postgis", port))
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
		std::string errMessage = errorMessage();
		close();
		errorMessage_ = errMessage;
		return false;
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

bool TePostGIS::connect(const string& host, const string& user, const string& password, const string& database, int port)
{
	if(!realConnect(host, user, password, database, port))
		return false;

// see if PostGIS is present
	TePGRecordset rec;

	string sql = "SELECT postgis_version()";

	if(!rec.open(sql.c_str(), tepg_connection_) || rec.recordCount() <= 0)
	{
		rec.close();

		this->close();

		errorMessage_ = "Couldn't find PostGIS extension! You may use the PostgreSQL driver!";

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
		TePostgreSQL::gistBoxOps_ = rec.value(0);
		rec.close();

		return true;
	}

	rec.close();

	this->close();

	errorMessage_ = "GiST extension not found! Please, install R-Tree GiST in your database and try again!";

	return false;	
}

bool TePostGIS::showDatabases(const string& host, const string& user, const string& password, vector<string>& dbNames, int port)
{
	errorMessage_ = "";

	if(TePostgreSQL::realConnect(host, user, password, "template1", port))
	{
		string sql = "SELECT datname FROM pg_database WHERE datname NOT IN ('template0', 'template1') ORDER BY datname";

		TePostGISPortal p(this);

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

TeDatabasePortal* TePostGIS::getPortal()
{
	errorMessage_ = "";

	TeDatabasePortal *portal = new TePostGISPortal(this);

	return portal;
}

bool TePostGIS::createTable(const string& table, TeAttributeList &attr)
{
	errorMessage_ = "";

	bool first = true;

	TeAttributeList::iterator it = attr.begin();
	
	string createTable ="CREATE TABLE " + table +" (";
	
	string type;
	char	size[8];

	string pkeys;

	vector<string> addGeomColumnsSQL;
	string addGeomSQL = "";

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

			case TeCHARACTER:	type = "CHAR ";
								sprintf (size, "(%d)", (*it).rep_.numChar_);
								type += string (size);
								break;

			case TeBOOLEAN:		type = "BOOLEAN";
								break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
								addGeomSQL = "SELECT AddGeometryColumn('";
								addGeomSQL += TeConvertToLowerCase(table);
								addGeomSQL += "', 'spatial_data', ";
								addGeomSQL += Te2String(it->rep_.epsgCode_) + ", 'POINT', 2)";

								addGeomColumnsSQL.push_back(addGeomSQL);

								++it;
								continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:							
								addGeomSQL  = "SELECT AddGeometryColumn('";
								addGeomSQL += TeConvertToLowerCase(table);
								addGeomSQL += "', 'spatial_data',";
								addGeomSQL += Te2String(it->rep_.epsgCode_) + ", 'LINESTRING', 2)";

								addGeomColumnsSQL.push_back(addGeomSQL);

								++it;
								continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
								addGeomSQL  = "SELECT AddGeometryColumn('";
								addGeomSQL += TeConvertToLowerCase(table);
								addGeomSQL += "', 'spatial_data',";
								addGeomSQL += Te2String(it->rep_.epsgCode_) + ", 'POLYGON', 2)";

								addGeomColumnsSQL.push_back(addGeomSQL);

								++it;
								continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
								if(!first)
									createTable += ", ";
								else
									first = false;

								addGeomSQL  = "SELECT AddGeometryColumn('";
								addGeomSQL += TeConvertToLowerCase(table);
								addGeomSQL += "', 'spatial_data',";
								addGeomSQL += Te2String(it->rep_.epsgCode_) + ", 'POLYGON', 2)";

								addGeomColumnsSQL.push_back(addGeomSQL);

								createTable += " col_number   INTEGER      NOT NULL,";
								createTable += " row_number	INTEGER      NOT NULL ";
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
								addGeomSQL = "SELECT AddGeometryColumn('";
								addGeomSQL += TeConvertToLowerCase(table);
								addGeomSQL += "', 'spatial_data', ";
								addGeomSQL += Te2String(it->rep_.epsgCode_) + ", 'POINT', 2)";

								addGeomColumnsSQL.push_back(addGeomSQL);

								++it;
								continue;

			case TeTEXTTYPE:
			case TeTEXTSETTYPE:

			default:			type = "VARCHAR ";
								sprintf (size, "(%d)", (*it).rep_.numChar_);
								type += string (size);
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

	if(!execute(createTable))
		return false;

	for(unsigned int i = 0; i < addGeomColumnsSQL.size(); ++i)
	{
		if(!execute(addGeomColumnsSQL[i]))
			return false;
	}

	return true;
}

bool TePostGIS::generateLabelPositions(TeTheme *theme, const std::string& objectId)
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if(theme->layer()->hasGeometry(TeCELLS)    || 
	   theme->layer()->hasGeometry(TePOLYGONS) ||
	   theme->layer()->hasGeometry(TeLINES)    ||
	   theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TeCELLS);
		
		if(geomTable.empty())
		{
			geomTable = theme->layer()->tableName(TePOLYGONS);
			if(geomTable.empty())
			{
				geomTable = theme->layer()->tableName(TeLINES);

				if(geomTable.empty())
					geomTable = theme->layer()->tableName(TePOINTS);
			}
		}
		
		upd= " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(xmin(spatial_data::box3d) + (xmax(spatial_data::box3d)";
		upd += " -  xmin(spatial_data::box3d)) / 2.0) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(ymin(spatial_data::box3d) + (ymax(spatial_data::box3d)";
		upd += " - ymin(spatial_data::box3d)) / 2.0) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";

		upd += " WHERE (label_x IS NULL) OR (label_y IS NULL)";
	}	

	if (!objectId.empty())
		upd += " AND c_object_id='"+objectId+"'";
		
	return execute(upd);
}


bool TePostGIS::selectPolygonSet(const string& table, const string& criteria, TePolygonSet& ps)
{
	TeDatabasePortal *portal = this->getPortal();
	
	string sql ="SELECT * FROM " + table;
	
	if(!criteria.empty())
		sql += " WHERE " + criteria;
	
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
		ps.add(poly);
	}
	while(flag);

	delete portal;

	return true;
}

bool TePostGIS::loadPolygonSet(TeTheme* theme, TePolygonSet& ps)
{
	string collTable = theme->collectionTable();
	
	if(collTable.empty())
		return false;

	TeLayer* themeLayer = theme->layer();

	if(!themeLayer->hasGeometry(TePOLYGONS))
		return false;
	
	string polygonTable = themeLayer->tableName(TePOLYGONS);

	if(polygonTable.empty())
		return false;

	string sql  = "SELECT * FROM (" + polygonTable + " RIGHT JOIN " + collTable;
	       sql += " ON " + polygonTable + ".object_id = " + collTable + ".c_object_id)";
	
	TeDatabasePortal *portal = this->getPortal();
	
	if(!portal)
		return false;

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
		ps.add ( poly );
	}
	while(flag);		
	
	delete portal;
	
	return true;
}

bool TePostGIS::loadPolygonSet(const string& table, const string& geoid, TePolygonSet& ps)
{
	TeDatabasePortal *portal = this->getPortal();
	
	string q ="SELECT * FROM " + table;

	if (!geoid.empty())
		q += " WHERE object_id = '" + geoid +"'";
	
	if (!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
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

bool TePostGIS::loadPolygonSet(const string& table, TeBox& box, TePolygonSet& ps)
{
	TeDatabasePortal *portal = this->getPortal();

	if(!portal)
		return false;

	string q = "SELECT * FROM " + table + " WHERE ";
	       q += this->getSQLBoxWhere (box, TePOLYGONS, table);

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

TeDatabasePortal* TePostGIS::loadPolygonSet(const string& table, TeBox& box)
{
	TeDatabasePortal *portal = this->getPortal();
	
	if(!portal)
		return 0;

	string q = "SELECT * FROM " + table + " WHERE ";
	       q += this->getSQLBoxWhere(box, TePOLYGONS, table);

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return 0;
	}
	else 
		return portal;
}

bool TePostGIS::locatePolygon(const string& table, TeCoord2D& pt, TePolygon& polygon, const double& tol)
{
	TeDatabasePortal *portal = this->getPortal();

	if (!portal)
		return false;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);

//sometimes, not only the name of the table but a complete join is given in the 'table'	argument. Tries to break the string if there is an space
	std::string tableName = table;
	
	size_t found = tableName.find(" ");
	if(found != tableName.npos)
	{
		std::vector<std::string> vecValues;
		TeSplitString(tableName, " ", vecValues);

		tableName = vecValues[0];
	}

	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += getSQLBoxWhere(box, TePOLYGONS, tableName);
	
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

bool TePostGIS::locatePolygonSet(const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons)
{
	TeDatabasePortal *portal = this->getPortal();
	
	if (!portal)
		return false;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);


	string sql  = "SELECT * FROM ";
	       sql += table;
		   sql += " WHERE ";
		   sql += getSQLBoxWhere(box, TePOLYGONS, table);

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

bool TePostGIS::insertPolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	unsigned int sizeBinaryPolygon = 0;
	char* binaryPolygon = TePolygonToWKBPolygon(p, sizeBinaryPolygon);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPolygon;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPolygon;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, spatial_data)";//::geometry
			command += " VALUES ($1, $2::geometry)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPolygon;
			
	if(result == 1)
	{
		string sql = "SELECT currval('" + table + "_geom_id_seq')";
		
		TePGRecordset rec;
		rec.open(sql.c_str(), tepg_connection_);
		if(rec.recordCount() > 0)
		{
			p.geomId(rec.getInt(0));
			for(register unsigned int i = 0; i < p.size(); ++i)
			{
				p[i].geomId(rec.getInt(0));
			}
		}

		rec.close();
	}
	else
		return false;

	return true;
}

bool TePostGIS::updatePolygon(const string& table, TePolygon &p)
{
	errorMessage_ = "";

	unsigned int sizeBinaryPolygon = 0;
	char* binaryPolygon = TePolygonToWKBPolygon(p, sizeBinaryPolygon);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPolygon;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPolygon;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	
	string command = "UPDATE " + table + " SET";
	command += " object_id = $1";
	command += ", spatial_data = $2::geometry";
	command += " WHERE geom_id = ";
    command += Te2String(p.geomId());
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPolygon;

	return (result == 1);
}

bool TePostGIS::loadLineSet(const string& table, const string& geoid, TeLineSet& ls)
{
	TeDatabasePortal *portal = this->getPortal();

	string q ="SELECT * FROM " + table;

	if(!geoid.empty())
		q += " WHERE object_id = '" + geoid +"'";

	if(!portal->query(q, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}

	bool flag = true;

	do 
	{
		TeLine2D line;
		flag = portal->fetchGeometry(line);
		ls.add (line);
	}while(flag);

	delete portal;

	return true;
}

bool TePostGIS::loadLineSet(const string& table, TeBox& box, TeLineSet& linSet)
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

TeDatabasePortal* TePostGIS::loadLineSet(const string& table, TeBox& box)
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

bool TePostGIS::insertLine(const string& table, TeLine2D& l)
{
	errorMessage_ = "";

	unsigned int sizeBinaryRing = 0;
	char* binaryRing = TeLine2DToWKBLineString(l, sizeBinaryRing);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(l.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryRing;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryRing;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, spatial_data)";
			command += " VALUES ($1, $2::geometry)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryRing;

	if(result == 1)
	{
		string sql = "SELECT currval('" + table + "_geom_id_seq')";

		TePGRecordset rec;
		rec.open(sql.c_str(), tepg_connection_);

		if(rec.recordCount() > 0)
			l.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;

	return true;
}

bool TePostGIS::updateLine(const string& table, TeLine2D& l)
{
	errorMessage_ = "";

		unsigned int sizeBinaryRing = 0;
	char* binaryRing = TeLine2DToWKBLineString(l, sizeBinaryRing);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(l.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryRing;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryRing;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;

	string command  = "UPDATE " + table + " SET";
		   command += " object_id = $1";
		   command += ", spatial_data = $2::geometry";
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

	return (result == 1);
}

bool TePostGIS::locateLine(const string& table, TeCoord2D &pt, TeLine2D &line, const double& tol)
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

bool TePostGIS::insertPoint(const string& table, TePoint &p)
{
	errorMessage_ = "";

	unsigned int sizeBinaryPoint = 2 * sizeof(double);
	char* binaryPoint = TePointToWKBPoint(p, sizeBinaryPoint);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPoint;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPoint;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, spatial_data)";
			command += " VALUES ($1, $2::geometry)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPoint;

	if(result == 1)
	{
		string sql = "SELECT currval('" + table + "_geom_id_seq')";
		TePGRecordset rec;
		rec.open(sql.c_str(), tepg_connection_);
		if(rec.recordCount() > 0)
			p.geomId(rec.getInt(0));

		rec.close();
	}
	else
		return false;
	
	return true;
}

bool TePostGIS::updatePoint(const string& table, TePoint &p)
{
	errorMessage_ = "";

	unsigned int sizeBinaryPoint = 2 * sizeof(double);
	char* binaryPoint = TePointToWKBPoint(p, sizeBinaryPoint);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(p.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPoint;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPoint;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	
	string command  = "UPDATE " + table + " SET";
			command += "  object_id = $1";
			command += ", spatial_data = $2::geometry";
			command += " WHERE geom_id = ";
			command += Te2String(p.geomId());
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPoint;
	
	return (result == 1);
}

bool TePostGIS::locatePoint(const string& table, TeCoord2D& pt, TePoint& point, const double& tol)
{
     TeDatabasePortal* portal = this->getPortal();

     TeBox bbox(pt.x() - tol, pt.y() - tol, pt.x() + tol, pt.y() + tol);

//sometimes, not only the name of the table but a complete join is given in the 'table' argument. Tries to break the string if there is an space
     std::string tableName = table;

     size_t found = tableName.find(" ");
     if(found != tableName.npos)
     {
         std::vector<std::string> vecValues;
         TeSplitString(tableName, " ", vecValues);

         tableName = vecValues[0];
     }

     string sql  = "SELECT * FROM ";
            sql += table;
            sql += " WHERE ";
            sql += this->getSQLBoxWhere(bbox, TePOINTS, tableName);

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

bool TePostGIS::insertNode(const string& table, TeNode& node)
{
	errorMessage_ = "";

	TePoint p;
	TeCoord2D nodeCoord(node.location().x(), node.location().y());
	p.add(nodeCoord);

	unsigned int sizeBinaryPoint = 2 * sizeof(double);
	char* binaryPoint = TePointToWKBPoint(p, sizeBinaryPoint);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(node.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPoint;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPoint;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, spatial_data)";
			command += " VALUES ($1, $2::geometry)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPoint;

	if(result != 1)
		return false;

	string sql = "SELECT currval('" + table + "_geom_id_seq')";
	TePGRecordset rec;
	rec.open(sql.c_str(), tepg_connection_);
	if(rec.recordCount() > 0)
		node.geomId(rec.getInt(0));

	rec.close();

	return true;
}

bool TePostGIS::updateNode(const string& table, TeNode& node)
{
	errorMessage_ = "";

	TePoint p;
	TeCoord2D nodeCoord(node.location().x(), node.location().y());
	p.add(nodeCoord);

	unsigned int sizeBinaryPoint = 2 * sizeof(double);
	char* binaryPoint = TePointToWKBPoint(p, sizeBinaryPoint);

	int nParams = 2;
	Oid paramTypes[2];
	const char *paramValues[2];
	int paramLengths[2];
	int paramFormats[2];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;

	string strOid = escapeSequence(node.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPoint;

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPoint;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	
	string  command  = "UPDATE " + table + " SET";
		    command += " object_id = $1";
			command += ", spatial_data = $2::geometry";
			command += " WHERE geom_id = ";
			command += Te2String(node.geomId());
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPoint;

	return (result == 1);
}

bool TePostGIS::insertCell(const string& table, TeCell &c)
{
	errorMessage_ = "";

	TePolygon p;
	TeLinearRing ring;
	ring.add(TeCoord2D(c.box().x1(), c.box().y1()));
	ring.add(TeCoord2D(c.box().x2(), c.box().y1()));
	ring.add(TeCoord2D(c.box().x2(), c.box().y2()));
	ring.add(TeCoord2D(c.box().x1(), c.box().y2()));
	ring.add(TeCoord2D(c.box().x1(), c.box().y1()));

	p.add(ring);
	p.srid(c.srid());

	unsigned int sizeBinaryPolygon = 0;
	char* binaryPolygon = TePolygonToWKBPolygon(p, sizeBinaryPolygon);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;
	paramTypes[2] = PG_INT4_TYPE;
	paramTypes[3] = PG_INT4_TYPE;

	string strOid = escapeSequence(c.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPolygon;

	string strCol = Te2String(c.column());
	paramValues[2] = strCol.c_str();		

	string strRow = Te2String(c.line());
	paramValues[3] = strRow.c_str();	

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPolygon;
	paramLengths[2] = 0;
	paramLengths[3] = 0;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 0;
	paramFormats[3] = 0;

	string command = "INSERT INTO ";
			command += table;
			command += " (object_id, spatial_data, col_number, row_number)";
			command += " VALUES ($1, $2::geometry, $3, $4)";
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPolygon;

	if(result != 1)
		return false;
		

	string sql = "SELECT currval('" + table + "_geom_id_seq')";
	TePGRecordset rec;
	rec.open(sql.c_str(), tepg_connection_);
	if(rec.recordCount() > 0)
		c.geomId(rec.getInt(0));
	
	rec.close();	

	return true;
}

bool TePostGIS::updateCell(const string& table, TeCell &c)
{
	errorMessage_ = "";

	TePolygon p;
	TeLinearRing ring;
	ring.add(TeCoord2D(c.box().x1(), c.box().y1()));
	ring.add(TeCoord2D(c.box().x2(), c.box().y1()));
	ring.add(TeCoord2D(c.box().x2(), c.box().y2()));
	ring.add(TeCoord2D(c.box().x1(), c.box().y2()));
	ring.add(TeCoord2D(c.box().x1(), c.box().y1()));

	p.add(ring);

	unsigned int sizeBinaryPolygon = 0;
	char* binaryPolygon = TePolygonToWKBPolygon(p, sizeBinaryPolygon);

	int nParams = 4;
	Oid paramTypes[4];
	const char *paramValues[4];
	int paramLengths[4];
	int paramFormats[4];
	
	//tipos dos parametros
	paramTypes[0] = PG_VARCHAR_TYPE;
	paramTypes[1] = 0;
	paramTypes[2] = PG_INT4_TYPE;
	paramTypes[3] = PG_INT4_TYPE;

	string strOid = escapeSequence(c.objectId());
	paramValues[0] = strOid.c_str();
	paramValues[1] = binaryPolygon;

	string strCol = Te2String(c.column());
	paramValues[2] = strCol.c_str();		

	string strRow = Te2String(c.line());
	paramValues[3] = strRow.c_str();	

	paramLengths[0] = 0;
	paramLengths[1] = sizeBinaryPolygon;
	paramLengths[2] = 0;
	paramLengths[3] = 0;
	
	paramFormats[0] = 0;
	paramFormats[1] = 1;
	paramFormats[2] = 0;
	paramFormats[3] = 0;

	string command  = "UPDATE " + table + " SET";
			command += " object_id = $1";
			command += ", spatial_data = $2::geometry";
			command += ", col_number = $3";
			command += ", row_number = $4";
			command += " WHERE geom_id = ";
			command += Te2String(c.geomId());
	
	int result = tepg_connection_->exec_cmd_params(command.c_str(),
                    nParams,
                    paramTypes,
                    paramValues,
                    paramLengths,
                    paramFormats,
                    0);

	delete [] binaryPolygon;

	return (result == 1);
}

bool TePostGIS::locateCell(const string& table, TeCoord2D& pt, TeCell& c, const double& tol)
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

bool TePostGIS::removeGeometry(const string& tableName, const TeGeomRep& rep, const int& geomId)
{
	if(tableName.empty())
	{
		return false;
	}

	std::string remove = "DELETE FROM " + tableName;
	remove += " WHERE geom_id = " + geomId;

	return this->execute(remove);
}

bool TePostGIS::createSpatialIndex(const string& table, const string& column, TeSpatialIndexType /*type*/, short /*level*/, short /*tile*/)
{
// we need to find te type we are indexing: a box from PostgreSQL or a geometry from PostGIS.
	string sql = "SELECT " + column + " FROM " + table + " WHERE 1 = 2";

	TePGRecordset rec;

	if(!rec.open(sql, (TePostgreSQL::tepg_connection_)))
	{
		rec.close();
		return false;
	}

	string create = "";

	if(rec.fieldType(0) == PG_BOX_TYPE)
	{
		create = "CREATE INDEX sp_idx_" + table + " ON " + table + " USING GIST (" + column + " " + TePostgreSQL::gistBoxOps_ + ")";
	}
	else
	{
		create = "CREATE INDEX sp_idx_" + table + " ON " + table + " USING GIST (" + column + " GIST_GEOMETRY_OPS)";
	}

	rec.close();	

	return this->execute(create);
}

string TePostGIS::getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName)
{
	if(rep == TeTEXT)
		return TeDatabase::getSQLBoxWhere(box, rep, tableName);

	string colname = "spatial_data";
	
	if(rep & TeRASTER)
		return TePostgreSQL::getSQLBoxWhere(box, rep, tableName);

//changed for compatibility with postGIS EWKB data. This modification allows query for data which has a projection different from -1 associated. Compatibility with -1 projection data is kept.
	std::string strProj = "";
	if(!tableName.empty())
	{
		TeDatabasePortal* portal = getPortal();

		std::string sql = "select SRID from geometry_columns where " + toUpper("f_table_name") + " = " + toUpper("'" + tableName + "'");
		if(portal->query(sql) && portal->fetchRow())
		{
			strProj = portal->getData(0);		
			if(strProj.empty() || strProj == "-1")
			{
				strProj = "";
			}
		}
		else
		{
			TeLayerMap& layerMap = this->layerMap();
			TeLayerMap::iterator it = layerMap.begin();
			while(it != layerMap.end())
			{
				if(it->second->tableName(rep) == tableName)
				{
					strProj = Te2String(it->second->projection()->epsgCode());
					break;
				}
				++it;
			}
		}

		delete portal;
	}

	std::string whereBox = "";
 	if(tableName.empty() == false && strProj.empty() == false)
	{
		whereBox = "(SetSRID('" + PGBox_encode(box) + "'::box3d, " + strProj + ") && " + colname + ")";
	}
	else
	{
		whereBox = "('" + PGBox_encode(box) + "'::box3d && " + colname + ")";
	}

	return whereBox;
}

std::string TePostGIS::getSQLOrderBy(const TeGeomRep& rep) const
{
	std::string orderBy = "object_id ASC";
	return orderBy;
}

string TePostGIS::getSQLBoxWhere(const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1)
{
	if((rep1 == TeTEXT) || (rep2 == TeTEXT))
	{
		return TeDatabase::getSQLBoxWhere(table1, table2, rep2, rep1);
	}

	string col1name = "spatial_data";
	string col2name = "spatial_data";;

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

string TePostGIS::getSQLBoxSelect (const string& tableName, TeGeomRep rep)
{
	if(rep == TeTEXT)
		return TeDatabase::getSQLBoxSelect(tableName, rep);

	string colname = "spatial_data";

	if(rep & TeRASTER)
		return TePostgreSQL::getSQLBoxSelect(tableName, rep);

	//string select  = tableName +".* , ";
	string select  = "";
			select += "xmin("  + tableName + "." + colname + ") as lower_x, ";
			select += "ymin("  + tableName + "." + colname + ") as lower_y, ";
			select += "xmax("  + tableName + "." + colname + ") as upper_x, ";
			select += "ymax("  + tableName + "." + colname + ") as upper_y ";

	return select;
}

bool TePostGIS::getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom)
{
// we need to find the type we are computing bounding box: a box from PostgreSQL or a geometry from PostGIS.
	string sql = "SELECT " + colGeom + " FROM " + tableGeom + " WHERE 1 = 2";

	TePGRecordset rec;

	if(!rec.open(sql, (TePostgreSQL::tepg_connection_)))
	{
		rec.close();
		return false;
	}

	if(rec.fieldType(0) == PG_BOX_TYPE)	// use the TePostgreSQL method
	{
		rec.close();

		return TePostgreSQL::getMBRGeom(tableGeom, object_id, box, colGeom);
	}

	rec.close();

// if we are here, so it is a PostGIS geometry: in this case we can use the function extent

	TeDatabasePortal* portal = getPortal();
	if(!portal)
		return false;

	string sel = "SELECT extent(" +  colGeom + ") FROM " + tableGeom;
	sel += " WHERE object_id = '" + object_id + "'";

	if(!portal->query(sel)|| !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	box = PGBox_decode(portal->getData(0));

	delete portal;
	return true;
}

bool TePostGIS::getMBRSelectedObjects(string geomTable, string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType, TeBox& bout, const double& tol)
{
	if(repType == TeTEXT)
		return TeDatabase::getMBRSelectedObjects(geomTable, colGeom, fromClause, whereClause, afterWhereClause, repType, bout, tol);

	if(repType == TeRASTER)
		return TePostgreSQL::getMBRSelectedObjects(geomTable, colGeom, fromClause, whereClause, afterWhereClause, repType, bout, tol);

	TeDatabasePortal* portal = getPortal();
	
	if (!portal)
		return false;

	string	fields = "extent(" + geomTable + "." + colGeom + ")::BOX3D";
	string	query  = " SELECT " + fields;
			query += " FROM " + fromClause; 

	if(!whereClause.empty())
		query += " WHERE " + whereClause;		

	if(portal->query(query) && portal->fetchRow())


	{
		TeBox aux(PGBox_decode(portal->getData(0)));
		bout = aux;

		delete portal;
		return true;
	}	

	delete portal;	

	return false;
}

string TePostGIS::getSpatialIdxColumn(TeGeomRep rep)
{
	if (rep == TeRASTER)
		return "block_box";

	return "spatial_data";
}


bool TePostGIS::getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList)
{ 
	if(!connect(host, user, password, "template_postgis", port))
	{
		close();
		return false;
	}

	bool returnValue = TePostgreSQL::getEncodingList(vecEncodingList);

	close();

	return returnValue; 
}


TePostGISPortal::TePostGISPortal(TeDatabase *pDatabase)
{
	db_ = pDatabase;
	con_ = ((static_cast<TePostGIS*>(pDatabase))->tepg_connection_);
}

bool TePostGISPortal::fetchGeometry(TePolygon& pol)
{
	errorMessage_ = "";

	if(_multiPolygonBuffer.empty() == false)
	{
		pol = *_multiPolygonBuffer.begin();
		_multiPolygonBuffer.erase(_multiPolygonBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt("geom_id");
	std::string objectId = tepg_recordset_->getData("object_id");
		
	TePolygonSet ps;
	tepg_recordset_->getPGISMultiPolygon("spatial_data", ps);
	

	for(unsigned int i = 0; i < ps.size(); ++i)
	{
		ps[i].geomId(geomId);
		ps[i].objectId(objectId);
		if(i == 0)
		{
			pol = ps[i];
		}
		else
		{
			_multiPolygonBuffer.add(ps[i]);
		}		
	}

	if(_multiPolygonBuffer.empty())
	{
		return fetchRow();
	}
	
	return true;
}

bool TePostGISPortal::fetchGeometry(TePolygon& pol, const unsigned int& initIndex)
{
	errorMessage_ = "";

	if(_multiPolygonBuffer.empty() == false)
	{
		pol = *_multiPolygonBuffer.begin();
		_multiPolygonBuffer.erase(_multiPolygonBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt(initIndex);
	std::string objectId = tepg_recordset_->getData(initIndex+1);
		
	TePolygonSet ps;
	tepg_recordset_->getPGISMultiPolygon(initIndex+2, ps);
	

	for(unsigned int i = 0; i < ps.size(); ++i)
	{
		ps[i].geomId(geomId);
		ps[i].objectId(objectId);
		if(i == 0)
		{
			pol = ps[i];
		}
		else
		{
			_multiPolygonBuffer.add(ps[i]);
		}		
	}

	if(_multiPolygonBuffer.empty())
	{
		return fetchRow();
	}
	
	return true;
}

bool TePostGISPortal::fetchGeometry(TeLine2D& line)
{
	errorMessage_ = "";

	if(_multiLineBuffer.empty() == false)
	{
		line = *_multiLineBuffer.begin();
		_multiLineBuffer.erase(_multiLineBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt("geom_id");
	std::string objectId = tepg_recordset_->getData("object_id");
		
	TeLineSet ls;
	tepg_recordset_->getPGISMultiLine("spatial_data", ls);
	

	for(unsigned int i = 0; i < ls.size(); ++i)
	{
		ls[i].geomId(geomId);
		ls[i].objectId(objectId);
		if(i == 0)
		{
			line = ls[i];
		}
		else
		{
			_multiLineBuffer.add(ls[i]);
		}		
	}

	if(_multiLineBuffer.empty())
	{
		return fetchRow();
	}

	return true;
}

bool TePostGISPortal::fetchGeometry(TeLine2D& line, const unsigned int& initIndex)
{
	errorMessage_ = "";

	if(_multiLineBuffer.empty() == false)
	{
		line = *_multiLineBuffer.begin();
		_multiLineBuffer.erase(_multiLineBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt(initIndex);
	std::string objectId = tepg_recordset_->getData(initIndex+1);
		
	TeLineSet ls;
	tepg_recordset_->getPGISMultiLine(initIndex+2, ls);
	

	for(unsigned int i = 0; i < ls.size(); ++i)
	{
		ls[i].geomId(geomId);
		ls[i].objectId(objectId);
		if(i == 0)
		{
			line = ls[i];
		}
		else
		{
			_multiLineBuffer.add(ls[i]);
		}		
	}

	if(_multiLineBuffer.empty())
	{
		return fetchRow();
	}

	return true;
}

bool TePostGISPortal::fetchGeometry(TeNode& n)
{
	errorMessage_ = "";

	TePoint p;
	tepg_recordset_->getPGISPoint("spatial_data", p);
	n.add(p.location());
	n.geomId(tepg_recordset_->getInt("geom_id"));
	n.objectId(tepg_recordset_->getData("object_id"));

	return fetchRow();
}

bool TePostGISPortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	errorMessage_ = "";

	TePoint p;
	tepg_recordset_->getPGISPoint(initIndex+2, p);
	n.add(p.location());
	n.geomId(tepg_recordset_->getInt(initIndex));
	n.objectId(tepg_recordset_->getData(initIndex+1));

	return fetchRow();
}

bool TePostGISPortal::fetchGeometry(TePoint& p)
{
	errorMessage_ = "";

	if(_multiPointBuffer.empty() == false)
	{
		p = *_multiPointBuffer.begin();
		_multiPointBuffer.erase(_multiPointBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt("geom_id");
	std::string objectId = tepg_recordset_->getData("object_id");
		
	TePointSet ps;
	tepg_recordset_->getPGISMultiPoint("spatial_data", ps);

	for(unsigned int i = 0; i < ps.size(); ++i)
	{
		ps[i].geomId(geomId);
		ps[i].objectId(objectId);
		if(i == 0)
		{
			p = ps[i];
		}
		else
		{
			_multiPointBuffer.add(ps[i]);
		}		
	}

	if(_multiPointBuffer.empty())
	{
		return fetchRow();
	}

	return true;
}

bool TePostGISPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	errorMessage_ = "";

	if(_multiPointBuffer.empty() == false)
	{
		p = *_multiPointBuffer.begin();
		_multiPointBuffer.erase(_multiPointBuffer.begin());
		return true;
	}

	int geomId = tepg_recordset_->getInt(initIndex);
	std::string objectId = tepg_recordset_->getData(initIndex+1);
		
	TePointSet ps;
	tepg_recordset_->getPGISMultiPoint(initIndex+2, ps);

	for(unsigned int i = 0; i < ps.size(); ++i)
	{
		ps[i].geomId(geomId);
		ps[i].objectId(objectId);
		if(i == 0)
		{
			p = ps[i];
		}
		else
		{
			_multiPointBuffer.add(ps[i]);
		}		
	}

	if(_multiPointBuffer.empty())
	{
		return fetchRow();
	}

	return true;
}

bool TePostGISPortal::fetchGeometry(TeCell& cell)
{
	errorMessage_ = "";

	TePolygon pol;
	tepg_recordset_->getPGISPolygon("spatial_data", pol);

	cell.geomId(tepg_recordset_->getInt("geom_id"));
	cell.objectId(tepg_recordset_->getData("object_id"));
	cell.setBox (pol.box());
	cell.column(tepg_recordset_->getInt("col_number"));
	cell.line(tepg_recordset_->getInt("row_number"));

	return fetchRow();
}

bool TePostGISPortal::fetchGeometry(TeCell& cell, const unsigned int& initIndex)
{
	errorMessage_ = "";

	TePolygon pol;
	tepg_recordset_->getPGISPolygon(initIndex+2, pol);

	cell.geomId(tepg_recordset_->getInt(initIndex));
	cell.objectId(tepg_recordset_->getData(initIndex+1));
	cell.setBox (pol.box());
	cell.column(tepg_recordset_->getInt(initIndex+3));
	cell.line(tepg_recordset_->getInt(initIndex+4));

	return fetchRow();
}

