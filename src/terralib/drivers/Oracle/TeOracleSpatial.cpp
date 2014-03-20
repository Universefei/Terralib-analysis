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

#include "TeOracleSpatial.h"

#include <sys/stat.h>
#include "TeSpatialOperations.h"
#include "TeGeometryAlgorithms.h"

#include "TeOCICursor.h"
#include "TeOCIConnect.h"

string 
getOracleSpatialRelation(int relation)
{
	string spatialRel="";
	switch (relation)
	{
		case TeDISJOINT:
		case TeINTERSECTS:
			spatialRel = "ANYINTERACT";	
		break;

		case TeTOUCHES:
			spatialRel = "TOUCH";
		break;

		case TeOVERLAPS:
			spatialRel = "OVERLAPBDYINTERSECT";
		break;

		case TeCOVERS:
			spatialRel = "COVERS"; 
		break;

		case TeCOVEREDBY:
			spatialRel = "COVEREDBY";
		break;
		
		case TeCONTAINS:
			spatialRel = "CONTAINS";
		break;
		
		case TeWITHIN:
			spatialRel = "INSIDE";
		break;
		
		case TeEQUALS:
			spatialRel = "EQUAL";
		break;
		
		case TeCROSSES:
			spatialRel = "OVERLAPBDYDISJOINT";
		break;
		default:
			spatialRel = "";
		break;
	}
	
	return spatialRel;
}


TeOracleSpatial::TeOracleSpatial() : TeOCIOracle()  
{
	dbmsName_ = "OracleSpatial";
}


bool
TeOracleSpatial::createSpatialIndex(const string &table, const string &column,TeSpatialIndexType type, short level, short tile)
{
	TeOracleSpatialPortal  *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string name;
	if(table.size()>21)
		name = TeConvertToUpperCase(table.substr(0,20)) + "_IDX";
	else
		name = TeConvertToUpperCase(table) + "_IDX";
	
	string index = "SELECT * FROM USER_SDO_INDEX_INFO";
	index += " WHERE table_name = '" + TeConvertToUpperCase(table) + "'";
	if(!ocip->query(index))
	{
		delete ocip;
		return false;
	}
	
	if(ocip->fetchRow())
	{
		errorMessage_ = "Spatial Index table already exist!";
		delete ocip;
		return false;
	}
		
	delete ocip;

	index = " CREATE INDEX " + name;
	index += " ON " + table + "(" + column + ")";
	index += " INDEXTYPE IS MDSYS.SPATIAL_INDEX ";
	
	if(type == TeQUADTREE)
	{
		if(level==0)
			return false;

		index += " PARAMETERS ('";
		index += "SDO_LEVEL = " + Te2String(level);
		index += " SDO_NUMTILES = " + Te2String(tile) + "'";
	}

	if(!execute(index))
		return false;
	
	return true;
}

bool
TeOracleSpatial::insertMetadata(const string &table, const string &column, double tolx,double toly,TeBox &box,short /* srid */)
{	
	TeOracleSpatialPortal	*ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string check = "SELECT * FROM USER_SDO_GEOM_METADATA ";
	check += " WHERE TABLE_NAME = '" + TeConvertToUpperCase(table);
	check += "' AND COLUMN_NAME = '" + TeConvertToUpperCase(column) + "'";
	if(!ocip->query(check))
	{
		delete ocip;
		return false;
	}

	if(ocip->fetchRow())
	{
		delete ocip;
		return false;
	}
			
	delete ocip;
	
	double xmin = box.x1();
	double ymin = box.y1();
	double xmax = box.x2();
	double ymax = box.y2();

	string inser = "INSERT INTO USER_SDO_GEOM_METADATA VALUES ( ";
	inser += "'" + TeConvertToUpperCase(table) + "' ," ;
	inser += "'" + TeConvertToUpperCase(column) + "' ," ; 
	inser += " MDSYS.SDO_DIM_ARRAY(";
	inser += " MDSYS.SDO_DIM_ELEMENT('X',";
	inser += Te2String(xmin,15) + "," + Te2String(xmax,15) + "," + Te2String(tolx,15) + "), ";
	inser += " MDSYS.SDO_DIM_ELEMENT('Y',";
	inser += Te2String(ymin,15) + "," + Te2String(ymax,15) + "," + Te2String(toly,15) + ")), ";
	inser += " NULL ) ";

	if(!(execute(inser.c_str()))) 
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting in the table USER_SDO_GEOM_METADATA! "; 
		return false;
	}

	return true;
}

string 
TeOracleSpatial::getSpatialIdxColumn(TeGeomRep rep)
{
	if (rep == TeRASTER)
		return "block_box";

	return "spatial_data";
}


bool
TeOracleSpatial::connect(const string& host, const string& user, const string& password, const string& database, int port)
{
	if(connection_ == NULL)
	{
		connection_ = new TeOCIConnection();
	}
	else if (connection_->isConnected())
	{ 
		delete (connection_);
		connection_ = new TeOCIConnection();
	}

	isConnected_ = false;
	if (connection_->connect(host.c_str(),user.c_str(),password.c_str(), true))
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
		errorMessage_ = "Error connecting to database server: " + connection_->getErrorMessage();
		delete (connection_); //disconect
		connection_=NULL;
		return false;
	}
}

bool
TeOracleSpatial::createTable(const string& table, TeAttributeList &attr)
{
	short	cont=0;
	string pkeys ="";
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
				if((*it).rep_.numChar_ > 0)
				{
					tablec += (*it).rep_.name_ + " VARCHAR2(" + Te2String((*it).rep_.numChar_) + ")";
				}
				else
				{
					tablec += (*it).rep_.name_ + " VARCHAR2(4000)";
				}
				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT '" + (*it).rep_.defaultValue_ + "' ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;
			
			case TeREAL:
				if((*it).rep_.decimals_>0)
					tablec += (*it).rep_.name_ +" NUMBER(*,"+ Te2String((*it).rep_.decimals_) +") ";
				else
					tablec += (*it).rep_.name_ +" NUMBER(*,38) ";

				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;
			
			case TeINT:
			case TeUNSIGNEDINT:
				tablec += (*it).rep_.name_ + " NUMBER(32) ";
				if((*it).rep_.isAutoNumber_)
				{
					hasAutoNumber=true;
					fieldName=(*it).rep_.name_;
				}

				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";

			break;

			case TeDATETIME:
				tablec += (*it).rep_.name_ + " DATE ";
				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;

			case TeCHARACTER:
				tablec += (*it).rep_.name_ + " CHAR ";
				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;

			case TeBOOLEAN:
				tablec += (*it).rep_.name_ + " NUMBER(1) ";
				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;

			case TeBLOB:
				tablec += (*it).rep_.name_ + " BLOB ";
			break;

			case TePOINTTYPE:
			case TePOINTSETTYPE:
							tablec += "spatial_data	MDSYS.SDO_GEOMETRY ";
							++it;
							cont++;
							continue;

			case TeLINE2DTYPE:
			case TeLINESETTYPE:
							tablec += "spatial_data	MDSYS.SDO_GEOMETRY ";
							++it;
							cont++;
							continue;

			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
							tablec += "spatial_data	MDSYS.SDO_GEOMETRY ";
							++it;
							cont++;
							continue;

			case TeCELLTYPE:
			case TeCELLSETTYPE:
							tablec += "col_number	NUMBER(32) NOT NULL,";
							tablec += "row_number	NUMBER(32) NOT NULL,";
							tablec += "spatial_data	MDSYS.SDO_GEOMETRY ";

							++it;
							cont++;
							continue;					

			case TeRASTERTYPE:
							tablec += "band_id NUMBER(32) NOT NULL, ";
							tablec += "resolution_factor NUMBER(32), ";
							tablec += "subband NUMBER(32),";
							tablec += "block_box  MDSYS.SDO_GEOMETRY, ";
							tablec += "block_size NUMBER(32), ";
							tablec += "spatial_data BLOB ";
							
							++it;
							cont++;
							continue;

			case TeNODETYPE:
			case TeNODESETTYPE:
							tablec += "spatial_data	MDSYS.SDO_GEOMETRY ";

							++it;
							cont++;
							continue;

			case TeTEXTTYPE:
			case TeTEXTSETTYPE:

			default:
				tablec += (*it).rep_.name_ + " VARCHAR2(255) ";
				if(!((*it).rep_.defaultValue_.empty()))
					tablec += " DEFAULT " + (*it).rep_.defaultValue_ + " ";

				if(!((*it).rep_.null_))
					tablec += " NOT NULL ";
			break;
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

TeDatabasePortal*  
TeOracleSpatial::getPortal ()
{
	TeOracleSpatialPortal* ocip = new TeOracleSpatialPortal (this);
	return ocip;
}

std::string TeOracleSpatial::getSQLOrderBy(const TeGeomRep& rep) const
{
	std::string orderBy = "object_id ASC";
	return orderBy;
}

string 
TeOracleSpatial::getSQLBoxWhere (const TeBox& box, const TeGeomRep rep, const std::string& tableName)
{
	std::string columnName = "spatial_data";

	if(rep == TeRASTER)
		columnName = "block_box";

	return getSQLBoxWhere(box, rep, tableName, columnName);
}

string TeOracleSpatial::getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName, const std::string& columnName)
{
	std::string plainTableName = tableName;
	std::string owner = this->user();
	
	std::vector<std::string> vecTableName;
	TeSplitString(tableName, ".", vecTableName);
		
	if(vecTableName.size() == 2)
	{
		owner = vecTableName[0];
		plainTableName = vecTableName[1];
	}
	
	string wherebox;
	

	if(rep == TeTEXT)
	{
		wherebox = TeDatabase::getSQLBoxWhere (box, rep, tableName);
		return wherebox;
	}

	std::string strProj = "null";
	if(!tableName.empty())
	{
		strProj = "(select sdo_SRID from mdsys.sdo_geom_metadata_table where " + toUpper("sdo_table_name") + " = " + toUpper("'" + plainTableName + "'") + " AND upper(sdo_owner) = upper('" + owner + "'))";
	}

	wherebox = "mdsys.sdo_filter (" + columnName +",";
	wherebox += "mdsys.sdo_geometry(2003," + strProj + ",null,";
	wherebox += "mdsys.sdo_elem_info_array(1,1003,3),";
	wherebox += "mdsys.sdo_ordinate_array(";
	wherebox += Te2String(box.x1(),15) + ", " + Te2String(box.y1(),15);
	wherebox += ", " + Te2String(box.x2(),15) + ", " + Te2String(box.y2(),15) + ")),";
	wherebox += "'mask=anyinteract querytype=window') = 'TRUE'";

	return wherebox;
}

string 
TeOracleSpatial::getSQLBoxWhere (const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1)
{
	string wherebox;
	string colname1, colname2; 
	colname1 = colname2 = "spatial_data";
	
	if(rep1 == TeRASTER) 
		colname1 = "block_box";

	if(rep2 == TeRASTER)
		colname2 = "block_box";
		
	wherebox =  "MDSYS.SDO_FILTER ("+ table1 +"."+ colname1 +",";
	wherebox += table2 +"."+ colname2 +", 'querytype = window') = 'TRUE'";  

	return wherebox;
}


string 
TeOracleSpatial::getSQLBoxSelect (const string& tableName, TeGeomRep rep)
{
	
	string select;
	string colname = "spatial_data";

	if(rep == TeRASTER)
		colname = "block_box";

	select =  tableName +".* , ";
	select += " SDO_GEOM.SDO_MIN_MBR_ORDINATE("+ tableName +"."+ colname +", 1) as lower_x,";
	select += " SDO_GEOM.SDO_MIN_MBR_ORDINATE("+ tableName +"."+ colname +", 2) as lower_y,";
	select += " SDO_GEOM.SDO_MAX_MBR_ORDINATE("+ tableName +"."+ colname +", 1) as upper_x,"; 
	select += " SDO_GEOM.SDO_MAX_MBR_ORDINATE("+ tableName +"."+ colname +", 2) as upper_y ";
	return select;
}

bool 
TeOracleSpatial::getMBRSelectedObjects(string /* geomTable */,string colGeom, string fromClause, string whereClause, string /* afterWhereClause */, TeGeomRep /* repType */,TeBox &bout, const double& tol)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	if (!portal)
		return false;

	string sql = "SELECT SDO_AGGR_MBR(" + colGeom + ") ";
	sql += " FROM " + fromClause;
	
	if(!whereClause.empty())
		sql += " WHERE " + whereClause;  
	
	if(!portal->query(sql)) 
	{
		delete portal;
		return false;
	}

	if(!portal->fetchRow())
	{
		delete portal;
		return false;
	}

	try
	{
		TeCoord2D coord1,coord2;
		portal->getCoordinates (1, coord1);
		portal->getCoordinates (2, coord2);
		TeBox b(coord1.x()-tol, coord1.y()-tol, coord2.x()+tol, coord2.y()+tol);
		bout = b;
	}
	
	catch(...)
	{
		delete portal;
		return false;
	}
	
	delete portal;
	return true;
}


bool 
TeOracleSpatial::getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	if (!portal)
		return false;

	string sql = "SELECT SDO_GEOM.SDO_MBR(" + tableGeom + "." + colGeom + ") ";
	sql += " FROM " + tableGeom;
	sql += " WHERE object_id = '" + object_id + "'";

	if((!portal->query(sql)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}
	
	TeCoord2D coord1,coord2;
	portal->getCoordinates (1, coord1);
	portal->getCoordinates (2, coord2);
	TeBox b(coord1.x(), coord1.y(), coord2.x(), coord2.y());
	box = b;
	
	delete portal;
	return true;
}


bool 
TeOracleSpatial::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, 
								   unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{
	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) this->getPortal();
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

	string sdo_geom = " MDSYS.SDO_GEOMETRY(2003, NULL, NULL";
	sdo_geom += ", MDSYS.SDO_ELEM_INFO_ARRAY( 1, 1003, 3 )";
	sdo_geom += ", MDSYS.SDO_ORDINATE_ARRAY( " ;
	sdo_geom += Te2String(ll.x(),15);
	sdo_geom += ", " + Te2String(ll.y(),15);
	sdo_geom += ", " + Te2String(ur.x(),15);
	sdo_geom += ", " + Te2String(ur.y(),15);
	sdo_geom += ")) ";

	try
	{
		if (!update)
		{
			q = "INSERT INTO "+ table +" (block_id, band_id, subband, ";
			q += " resolution_factor, block_box, block_size, spatial_data) VALUES ( ";
			q += "'" + blockId + "'";
			q += ", " + Te2String(band);
			q += ", " + Te2String(subband);
			q += ", " + Te2String(res);
			q += ", " + sdo_geom;
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
TeOracleSpatial::allocateOrdinatesObject(TePolygon &poly, string& elInfo, TeOCICursor* cursor_)
{
	int		totalsize, ni, size;
	double	xult, yult;
	short	orient;

	ni = poly.size () - 1;

	xult = -9999.99;
	yult = -9999.99;

	totalsize = 0;

	try
	{
		//OCI: create the ordinates array
		if(!cursor_)
			connection_->allocateObjectOrdinates();
		
		for (int k = 0; k <= ni; k++ )
		{
			TeLinearRing ring ( poly[k] );
			totalsize += ring.size();
			size = ring.size();
			orient = TeOrientation(ring);
			
			if (k==0)  //external polygon: UNCLOCKWISE
			{
				elInfo = "1, 1003, 1";
				if(orient == TeCOUNTERCLOCKWISE)   
				{
					for (int i=0;i<size;i++)
					{
						if(xult != ring[i].x() || yult != ring[i].y())
						{
							if(cursor_)
							{
								cursor_->appendOrdinates(ring[i].x());
								cursor_->appendOrdinates (ring[i].y());	
							}
							else
							{
								connection_->appendOrdinates(ring[i].x());
								connection_->appendOrdinates (ring[i].y());	
							}
						
							xult = ring[i].x();
							yult = ring[i].y();
						}
					}
				}
				//keep UNCLOCKWISE ring 
				else
				{
					for (int i=0;i<size;i++)
					{
						if(xult != ring[size-1-i].x() || yult != ring[size-1-i].y())
						{
							if(cursor_)
							{
								cursor_->appendOrdinates(ring[size-1-i].x());
								cursor_->appendOrdinates (ring[size-1-i].y());
							}
							else
							{
								connection_->appendOrdinates(ring[size-1-i].x());
								connection_->appendOrdinates (ring[size-1-i].y());
							}
							
							xult = ring[size-1-i].x();
							yult = ring[size-1-i].y();
						}
					}
				}
			}

			else  //internal polygon: CLOCKWISE
			{
				int pos = ((totalsize - size) * 2) + 1; 
				elInfo += ", " + Te2String(pos) + ", 2003, 1";
						
				if(orient == TeCLOCKWISE)   
				{
					for (int i=0;i<size;i++)
					{
						if(xult != ring[i].x() || yult != ring[i].y())
						{
							if(cursor_)
							{
								cursor_->appendOrdinates(ring[i].x());
								cursor_->appendOrdinates (ring[i].y());
							}
							else
							{
								connection_->appendOrdinates(ring[i].x());
								connection_->appendOrdinates (ring[i].y());
							}

							xult = ring[i].x();
							yult = ring[i].y();
						}
					}
				}
				//keep CLOCKWISE ring
				else
				{
					for (int i=0;i<size;i++)
					{
						if(xult != ring[size-1-i].x() || yult != ring[size-1-i].y())
						{
							connection_->appendOrdinates(ring[size-1-i].x());
							connection_->appendOrdinates (ring[size-1-i].y());
							
							xult = ring[size-1-i].x();
							yult = ring[size-1-i].y();
						}
					}
				}
			}
		}//for all rings
	}
	catch(...)
	{
		return false;
	}
	
	return true;	
}

bool 
TeOracleSpatial::allocateOrdinatesObject(TeLine2D &line, TeOCICursor* cursor_)
{
	int size = line.size();
	double	xult, yult;
	xult = -9999.99;
	yult = -9999.99;

	try
	{
		//OCI: create the ordinates array
		if(!cursor_)
			connection_->allocateObjectOrdinates ();

		for (int i=0;i<size;i++)
		{		
			if(xult != line[i].x() || yult != line[i].y())
			{
				if(cursor_)
				{
					cursor_->appendOrdinates(line[i].x());
					cursor_->appendOrdinates(line[i].y());
				}
				else
				{
					connection_->appendOrdinates(line[i].x());
					connection_->appendOrdinates(line[i].y());
				}
				xult = line[i].x();
				yult = line[i].y();
			}
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}


bool 
TeOracleSpatial::insertPolygon (const string& table, TePolygon &poly)
{
	string	elinfo;
	
	if(!allocateOrdinatesObject(poly, elinfo))
		return false;
	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, spatial_data) VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(poly.objectId()) + "'";
	ins += ", MDSYS.SDO_GEOMETRY(2003, NULL, NULL";
	ins += ", MDSYS.SDO_ELEM_INFO_ARRAY( " + elinfo + " )";
	ins += ", :ordinates_) ";
	ins += " )";

	if(!connection_->executeSDOSTM(ins))
	{
		errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}
	return true;
}

bool 
TeOracleSpatial::updatePolygon (const string& table, TePolygon &poly)
{
	if(!tableExist(table))
	{	
		errorMessage_ = "Table not exist!";
		return false;
	}
	
	string elinfo;

	if(!allocateOrdinatesObject(poly, elinfo))
		return false;
		
	string sql;
	sql =  "UPDATE " + table + " SET ";
	sql += ", object_id = '" + poly.objectId() + "'";
	sql += ", spatial_data = ";
	sql += " MDSYS.SDO_GEOMETRY(2003, NULL, NULL";
	sql += ", MDSYS.SDO_ELEM_INFO_ARRAY( " + elinfo + " )";
	sql += ", :ordinates_) ";
	sql += " WHERE geom_id = " + poly.geomId();

	//OCI
	if(!connection_->executeSDOSTM(sql))
	{
		errorMessage_ = "Error updating in the table " + table + "!"; 
		return false;
	}
	
	return true;
}

bool 
TeOracleSpatial::selectPolygonSet (const string& table, const string& criteria, TePolygonSet &ps)
{
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string sql ="SELECT * FROM " + table;
	if (!criteria.empty())
		sql += " WHERE " + criteria;
	sql += " ORDER BY object_id ASC ";
	 
	if (!ocip->query(sql) || !ocip->fetchRow())
	{
		delete ocip;
		return false;
	}
	
	bool flag = true;
	do
	{
		TePolygon poly;
		flag = ocip->fetchGeometry(poly);
		ps.add(poly);
	}
	while (flag);

	delete ocip;
	return true;
}

bool 
TeOracleSpatial::loadPolygonSet (const string& table, const string& geoid, TePolygonSet &ps)
{
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string q ="SELECT * FROM " + table;

	if (geoid != "")
		q += " WHERE object_id = '" + geoid +"'";
	
	if (!ocip->query(q) || !ocip->fetchRow())
	{	
		delete ocip;
		return false;
	}

	bool flag = true;
	do
	{
		TePolygon poly;
		flag = ocip->fetchGeometry(poly);
		ps.add(poly);
	}
	while (flag);
	delete ocip;
	return true;
}


bool 
TeOracleSpatial::loadPolygonSet (const string& table, TeBox &box, TePolygonSet &ps)
{
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string q = "SELECT * FROM " + table;
	q += this->getSQLBoxWhere (box, TePOLYGONS, table);
	q += " ORDER BY object_id ";

	if (!ocip->query(q) || !ocip->fetchRow())
	{	
		delete ocip;
		return false;
	}
	bool flag = true;
	do
	{
		TePolygon poly;
		flag = ocip->fetchGeometry(poly);
		ps.add(poly);
	}
	while (flag);
	delete ocip;
	return true;
}

bool 
TeOracleSpatial::loadPolygonSet(TeTheme* theme, TePolygonSet &ps)
{
	string collTable = theme->collectionTable();
	if (collTable.empty())
		return false;

	TeLayer* themeLayer = theme->layer();
	if (!themeLayer->hasGeometry(TePOLYGONS))
		return false;
	
	string polygonTable = themeLayer->tableName(TePOLYGONS);
	if (polygonTable.empty())
		return false;

	string sql = "SELECT * FROM (" + polygonTable + " RIGHT JOIN " + collTable;
	sql += " ON " + polygonTable + ".object_id = " + collTable + ".c_object_id)";
	
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	if (!ocip->query(sql) || !ocip->fetchRow())
	{
		delete ocip;
		return false;
	}

	bool flag = true;
	do
	{
		TePolygon poly;
		flag = ocip->fetchGeometry(poly);
		ps.add ( poly );
	}
	while (flag);		
	delete ocip;
	return true;
}

TeDatabasePortal* 
TeOracleSpatial::loadPolygonSet(const string& table, TeBox &box)
{
	TeOracleSpatialPortal *portal = (TeOracleSpatialPortal*)this->getPortal();
	if (!portal)
		return 0;

	string q;
	q = "SELECT * FROM " + table + " WHERE ";
	q += this->getSQLBoxWhere (box, TePOLYGONS, table);
	q += " ORDER BY object_id ";

	if (!portal->query(q) || !portal->fetchRow())
	{	
		delete portal;
		return 0;
	}
	else 
		return portal;
}


//Spatial query
//retornam um portal
bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, TeDatabasePortal *portal, int relate, const string& actCollTable)
{
	string Ids = getStringIds(actIdsIn);
	string actGeomColl = "spatial_data";
	
	string spatialRel = getOracleSpatialRelation(relate);
	
	//Montar a sql para passar para o Oracle
	string sql = "SELECT geomTable1.* ";
	sql += " FROM "+ actGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!actCollTable.empty())
	{
		sql += ", "+ actCollTable +" collTable ";
		sql += " WHERE geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
	
	sql += " geomTable2.object_id IN (" + Ids + ") AND ";

	if(relate==TeEQUALS)
		sql += " geomTable1.object_id NOT IN (" + Ids + ") AND ";  
	
	if(relate==TeDISJOINT)
		sql += " NOT ";  // NOT ANYINTERACT

	sql += " SDO_RELATE(geomTable1."+ actGeomColl +", geomTable2."+ actGeomColl +", 'mask= "; 
	sql += spatialRel + " querytype=WINDOW') = 'TRUE'";

	portal->freeResult();
	if(!((TeOracleSpatialPortal*)portal)->querySDO (sql)) 
		return false;

	return (portal->fetchRow());
}

bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, const string& visGeomTable, TeGeomRep /* visRep */, TeDatabasePortal *portal, int relate, const string& visCollTable)
{
	string Ids = getStringIds(actIdsIn);
	string spatialRel = getOracleSpatialRelation(relate);
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";
	
	//Montar a sql para passar para o Oracle
	string sql = "SELECT geomTable1.* ";
	sql += " FROM "+ visGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!visCollTable.empty())
	{
		sql += ", "+ visCollTable +" collTable";
		sql += " WHERE geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
	
	sql += " geomTable2.object_id IN (" + Ids + ") AND ";

	if(relate==TeDISJOINT)
		sql += " NOT ";  // NOT ANYINTERACT

	sql += " SDO_RELATE(geomTable1."+ visGeomColl +", geomTable2."+ actGeomColl +", 'mask= "; 
	sql += spatialRel + " querytype=WINDOW') = 'TRUE'";

	portal->freeResult();
	if(!((TeOracleSpatialPortal*)portal)->querySDO(sql))
		return false;
	
	return (portal->fetchRow());
		
}

bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep /* actRep */, TeGeometry* geom, TeDatabasePortal *portal, int relate, const string& actCollTable) 
{
	portal->freeResult();
	string elinfo, sdo;
	
	TeOCICursor	*cursor_ = ((TeOracleSpatialPortal*)portal)->getCursor();	
	string spatialRel = getOracleSpatialRelation(relate);
	string actGeomColl = "spatial_data";

	if(geom->elemType()==TePOLYGONS)
	{
		TePolygon poly, *pPoly;
		pPoly = new TePolygon();
		pPoly = (TePolygon*)geom;
		poly = *pPoly;

		if(!allocateOrdinatesObject(poly, elinfo, cursor_))
		{
			delete cursor_;
			return false;
		}

		sdo = " MDSYS.SDO_GEOMETRY(2003, NULL, NULL, ";
		sdo += " MDSYS.SDO_ELEM_INFO_ARRAY(" + elinfo + "), ";
		sdo += " :ordinates_)";

		//delete pPoly;  //delete tambem o geom, talvez deixar para aplicacao
	}
	
	else if (geom->elemType()==TeLINES)
	{
		TeLine2D line, *pLine;
		pLine = new TeLine2D();
		pLine = (TeLine2D*)geom;
		line = *pLine;

		if(!allocateOrdinatesObject(line, cursor_))
		{
			delete cursor_;
			return false;
		}

		elinfo = "1, 2, 1";

		sdo = " MDSYS.SDO_GEOMETRY(2002, NULL, NULL, ";
		sdo += " MDSYS.SDO_ELEM_INFO_ARRAY(" + elinfo + "), ";
		sdo += " :ordinates_)";

		//delete pLine;  //delete tambem o geom
	}
	
	else if (geom->elemType()==TePOINTS)
	{
		TePoint point, *pPoint;
		pPoint = new TePoint();
		pPoint = (TePoint*)geom;
		point = *pPoint;
		
		sdo = " MDSYS.SDO_GEOMETRY(2001, NULL, ";
		sdo += " MDSYS.SDO_POINT_TYPE( ";
		sdo += Te2String(point.location().x(),15);
		sdo += ", " + Te2String(point.location().y(),15);
		sdo += ", NULL )";
		sdo += ", NULL, NULL))";
		
		//delete pPoint;
	}

	else if (geom->elemType()==TeCELLS)
	{
		TeCell cell, *pCell;
		pCell = new TeCell();
		pCell = (TeCell*)geom;
		cell = *pCell;

		TeBox b = cell.box();
		
		sdo = " MDSYS.SDO_GEOMETRY(2003, NULL, NULL ";
		sdo += ", MDSYS.SDO_ELEM_INFO_ARRAY( 1, 1003, 3 )";
		sdo += ", MDSYS.SDO_ORDINATE_ARRAY( " ;
		sdo += Te2String(b.lowerLeft().x(),15);
		sdo += ", " + Te2String(b.lowerLeft().y(),15);
		sdo += ", " + Te2String(b.upperRight().x(),15);
		sdo += ", " + Te2String(b.upperRight().y(),15);
		sdo += ")) ";

		//delete pCell;
	}
	
	//Montar a sql para passar para o Oracle
	string sql = "SELECT geomTable.* ";
	sql += " FROM " + actGeomTable + " geomTable ";

	if(!actCollTable.empty())
	{
		sql += ", "+ actCollTable +" collTable ";
		sql += " WHERE geomTable.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
	
	
	if(relate==TeDISJOINT)
		sql += " NOT ";  // NOT ANYINTERACT

	sql += " MDSYS.SDO_RELATE(geomTable."+ actGeomColl +", "+ sdo +", 'mask= "; 
	sql += spatialRel + " querytype=WINDOW') = 'TRUE'";

	if(!((TeOracleSpatialPortal*)portal)->querySDO(sql))
		return false;

	return (portal->fetchRow());
}


//retornam um vetor de object_ids resultantes da consulta
bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, TeKeys& actIdsOut, int relate, const string& actCollTable)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) getPortal(); 
		
	string Ids = getStringIds(actIdsIn);
	string spatialRel = getOracleSpatialRelation(relate);
	string actGeomColl = "spatial_data";

	//Montar a sql para passar para o Oracle
	string sql = "SELECT geomTable1.object_id ";
	sql += " FROM "+ actGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!actCollTable.empty())
	{
		sql += ", "+ actCollTable +" collTable ";
		sql += " WHERE geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
	
	sql += " geomTable2.object_id IN (" + Ids + ") AND ";

	if(relate==TeEQUALS)
		sql += " geomTable1.object_id NOT IN (" + Ids + ") AND ";  
	
	if(relate==TeDISJOINT)
		sql += " NOT ";  // NOT ANYINTERACT

	sql += " SDO_RELATE(geomTable1."+ actGeomColl +", geomTable2."+ actGeomColl +", 'mask= "; 
	sql += spatialRel + " querytype=WINDOW') = 'TRUE'";

	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}
	
	actIdsOut.clear();
	while(portal->fetchRow())
	{
		string objId = portal->getData (0);
		actIdsOut.push_back(objId);
	}

	sort(actIdsOut.begin(), actIdsOut.end());
	unique(actIdsOut.begin(), actIdsOut.end());

	delete portal;
	return true;
}


bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, const string& visGeomTable, TeGeomRep /* visRep */, TeKeys& visIdsOut, int relate, const string& visCollTable)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) getPortal(); 
	
	string Ids = getStringIds(actIdsIn);
	string spatialRel = getOracleSpatialRelation(relate);
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";
	
	//Montar a sql para passar para o Oracle
	string sql = "SELECT geomTable1.object_id ";
	sql += " FROM "+ visGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!visCollTable.empty())
	{
		sql += ", "+ visCollTable +" collTable";
		sql += " WHERE geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
	
	sql += " geomTable2.object_id IN (" + Ids + ") AND ";

	if(relate==TeDISJOINT)
		sql += " NOT ";  // NOT ANYINTERACT

	sql += " SDO_RELATE(geomTable1."+ visGeomColl +", geomTable2."+ actGeomColl +", 'mask= "; 
	sql += spatialRel + " querytype=WINDOW') = 'TRUE'";

	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}
	
	visIdsOut.clear();
	while(portal->fetchRow())
	{
		string objId = portal->getData (0);
		visIdsOut.push_back(objId);
	}

	sort(visIdsOut.begin(), visIdsOut.end());
	unique(visIdsOut.begin(), visIdsOut.end());

	delete portal;
	return true;
}


bool 
TeOracleSpatial::spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeGeometry* geom, TeKeys& actIdsOut, int relate, const string& actCollTable)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) getPortal(); 
		
	if(!spatialRelation(actGeomTable, actRep, geom, portal, relate, actCollTable))
	{
		delete portal;
		return false;
	}
		
	actIdsOut.clear();
	do
	{
		string objId = portal->getData ("object_id");
		actIdsOut.push_back(objId);
	}while(portal->fetchRow());

	sort(actIdsOut.begin(), actIdsOut.end());
	unique(actIdsOut.begin(), actIdsOut.end());

	delete portal;
	return true;
}

// metric functions
bool
TeOracleSpatial::calculateArea(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, double &area)
{
	string Ids = getStringIds(actIdsIn);
	string actGeomColl = "spatial_data";
	
	string sql = "SELECT SUM(SDO_GEOM.SDO_AREA(g."+ actGeomColl +", m.diminfo))";
	sql += " FROM "+ actGeomTable +" g, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	area = portal->getDouble(0);
	delete portal;
	return true;
}

bool
TeOracleSpatial::calculateLength(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIdsIn, double &length)
{
	string Ids = getStringIds(actIdsIn);
	string actGeomColl = "spatial_data";
	
	string sql = "SELECT SUM(SDO_GEOM.SDO_LENGTH(g."+ actGeomColl +", m.diminfo))";
	sql += " FROM "+ actGeomTable +" g, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	length = portal->getDouble(0);
	delete portal;
	return true;
}


//distancia entre objetos de um mesma tabela
bool
TeOracleSpatial::calculateDistance(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& Ids, double& distance)
{
	string Id1 = Ids[0];
	string Id2 = Ids[1];
	string actGeomColl = "spatial_data";

	string sql = "SELECT MIN(SDO_GEOM.SDO_DISTANCE(g1."+ actGeomColl +", m.diminfo, ";
	sql += " g2."+ actGeomColl +", m.diminfo))";
	sql += " FROM "+ actGeomTable +" g1,"+ actGeomTable +" g2, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ Id1 +"'"; 
	sql += " AND g2.object_id = '"+ Id2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	distance = portal->getDouble(0);
	delete portal;
	return true;
}


//distancia entre objetos de duas tabelas distintas
bool
TeOracleSpatial::calculateDistance(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, TeGeomRep /* visRep */, const string& objId2, double& distance)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";
	
	string sql = "SELECT MIN(SDO_GEOM.SDO_DISTANCE(g1."+ actGeomColl +", m1.diminfo, ";
	sql += " g2."+ visGeomColl +", m2.diminfo))";
	sql += " FROM "+ actGeomTable +" g1,"+ visGeomTable +" g2, ";
	sql += " USER_SDO_GEOM_METADATA m1, USER_SDO_GEOM_METADATA m2 ";
	sql += " WHERE m1.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m1.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND m2.table_name = '"+ TeConvertToUpperCase(visGeomTable) +"'";
	sql += " AND m2.column_name = '"+ TeConvertToUpperCase(visGeomColl) +"'";
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	distance = portal->getDouble(0);
	delete portal;
	return true;
}



// functions that return a new geometry

//Euclidean distance value: dist
bool
TeOracleSpatial::buffer(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIds, TePolygonSet& bufferSet, double dist)
{
	string Ids = getStringIds(actIds);
	string actGeomColl = "spatial_data";

	string sql = "SELECT g.geom_id, g.object_id,";
	sql += " SDO_GEOM.SDO_BUFFER(g."+ actGeomColl +", m.diminfo, "+ Te2String(dist, 15) +")";
	sql += " FROM "+ actGeomTable +" g, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		TePolygonSet polySet;
		flag = portal->fetchGeometry(polySet);
		//teste c/ buffer com filhos
		for(unsigned int i=0; i<polySet.size(); i++)
			bufferSet.add(polySet[i]);

	}while(flag);

	delete portal;
	return true;
}

bool 
TeOracleSpatial::convexHull(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIds, TePolygonSet& convexHullSet)
{
	string Ids = getStringIds(actIds);
	string actGeomColl = "spatial_data";
	
	string sql = "SELECT g.geom_id, g.object_id,";
	sql += " SDO_GEOM.SDO_CONVEXHULL(g."+ actGeomColl +", m.diminfo )";
	sql += " FROM "+ actGeomTable +" g, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}
	
	bool flag = true;
	do 
	{
		TePolygon poly;
		flag = portal->fetchGeometry(poly);
		convexHullSet.add(poly);
	}while(flag);

	delete portal;
	return true;
}

bool 
TeOracleSpatial::centroid(const string&  actGeomTable , TeGeomRep /* actRep */, TePointSet& centroidSet, TeKeys actIds, const string& /* actCollTable */)
{
	string Ids = getStringIds(actIds);
	string actGeomColl = "spatial_data";
	
	string sql = "SELECT g.geom_id, g.object_id,";
	sql += " SDO_GEOM.SDO_CENTROID(g."+ actGeomColl +", m.diminfo )";
	sql += " FROM "+ actGeomTable +" g, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	
	//if empty it calculates the centroids to all geometries 
	if(!Ids.empty())
		sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		TePoint point;
		flag = portal->fetchGeometry(point);
		centroidSet.add(point);
	}while(flag);

	delete portal;
	return true;
	
}

bool 
TeOracleSpatial::nearestNeighbors(const string& actGeomTable, const string& actCollTable, TeGeomRep actRep, const string& objId1, TeKeys& actIdsOut, int numRes)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) getPortal(); 
	
	if(!nearestNeighbors(actGeomTable, actCollTable, actRep, objId1, portal, numRes))
	{
		delete portal;
		return false;
	}
		
	actIdsOut.clear();
	while(portal->fetchRow())
	{
		string objId = portal->getData ("object_id");
		actIdsOut.push_back(objId);
	}

	sort(actIdsOut.begin(), actIdsOut.end());
	unique(actIdsOut.begin(), actIdsOut.end());

	delete portal;
	return true;
}


bool 
TeOracleSpatial::nearestNeighbors(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, const string& visCollTable, TeGeomRep visRep, TeKeys& visIdsOut, int numRes)
{
	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*) getPortal(); 
	
	if(!nearestNeighbors(actGeomTable, actRep, objId1, visGeomTable, visCollTable, visRep, portal, numRes))
	{
		delete portal;
		return false;
	}
		
	visIdsOut.clear();
	while(portal->fetchRow())
	{
		string objId = portal->getData ("object_id");
		visIdsOut.push_back(objId);
	}

	sort(visIdsOut.begin(), visIdsOut.end());
	unique(visIdsOut.begin(), visIdsOut.end());

	delete portal;
	return true;
}

bool 
TeOracleSpatial::nearestNeighbors(const string& actGeomTable, const string& actCollTable, TeGeomRep /* actRep */, const string& objId1, TeDatabasePortal* portal, int numRes)
{
	string actGeomColl = "spatial_data";

	//select the spatial index
	string index = " SELECT INDEX_NAME FROM USER_SDO_INDEX_INFO";
	index += " WHERE TABLE_NAME = '"+ TeConvertToUpperCase(actGeomTable) +"'"; 

	portal->freeResult();
	if(!portal->query(index) || !portal->fetchRow())
        return false;

	string indexName = string(portal->getData(0));
	string perf = "/*+ INDEX("+ TeConvertToUpperCase(actGeomTable) +" "+ indexName +") */ ";
	
	string sql = "SELECT "+ perf +"  geomTable1.* ";
	sql += " FROM "+ actGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!actCollTable.empty())
	{
		sql += ", "+ actCollTable +" collTable ";
		sql += " WHERE ";
		sql += " geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
		
	sql += " SDO_NN(geomTable1."+ actGeomColl +", geomTable2."+ actGeomColl +", 'sdo_batch_size=10')='TRUE' AND ";
	sql += " geomTable2.object_id = '" + objId1 + "' AND ";
	sql += " geomTable1.object_id <> '"+ objId1 +"' AND ";
	sql += " ROWNUM <= "+ Te2String(numRes);

	portal->freeResult();
	if(!((TeOracleSpatialPortal*)portal)->querySDO(sql))
		return false;
	
	return true;
}
	
bool 
TeOracleSpatial::nearestNeighbors(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, const string& visCollTable, TeGeomRep /* visRep */, TeDatabasePortal* portal, int numRes)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";

	//select the spatial index
	string index = " SELECT INDEX_NAME FROM USER_SDO_INDEX_INFO";
	index += " WHERE TABLE_NAME = '"+ TeConvertToUpperCase(visGeomTable) +"'"; 

	portal->freeResult();
	if(!portal->query(index) || !portal->fetchRow())
		return false;

	string indexName = string(portal->getData(0));
	string perf = "/*+ INDEX("+ TeConvertToUpperCase(visGeomTable) +" "+ indexName +") */ ";
	string nres = " ROWNUM <= "+ numRes;
	
	string sql = "SELECT "+ perf +"  geomTable1.* ";
	sql += " FROM "+ visGeomTable +" geomTable1,";
	sql += actGeomTable + " geomTable2 ";
	
	if(!visCollTable.empty())
	{
		sql += ", "+ visCollTable +" collTable";
		sql += " WHERE ";
		sql += " geomTable1.object_id = collTable.c_object_id AND ";
	}
	else
		sql += " WHERE ";
		
	sql += " SDO_NN(geomTable1."+ actGeomColl +", geomTable2."+ actGeomColl;
	sql += ", 'sdo_batch_size=10') = 'TRUE' AND ";
	sql += " geomTable2.object_id = '" + objId1 + "' AND ";
	sql += " ROWNUM <= "+ Te2String(numRes);
	
	portal->freeResult();
	if(!((TeOracleSpatialPortal*)portal)->querySDO(sql))
		return false;
	
	return true;
}


//Intersection entre dois objetos geograficos de uma mesma tabela


bool 
TeOracleSpatial::geomIntersection(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIds, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string Ids = getStringIds(actIds);

	string sql = "SELECT SDO_GEOM.SDO_INTERSECTION(g1."+ actGeomColl +", m.diminfo, ";
	sql += " g2."+ actGeomColl +", m.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ actGeomTable +" g2, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if((!portal->query(sql))||(!portal->fetchRow()))
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}


bool 
TeOracleSpatial::geomIntersection(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, TeGeomRep /* visRep */, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";
	
	string sql = "SELECT SDO_GEOM.SDO_INTERSECTION(g1."+ actGeomColl +", m1.diminfo, ";
	sql += " g2."+ visGeomColl +", m2.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ visGeomTable +" g2, ";
	sql += " USER_SDO_GEOM_METADATA m1, USER_SDO_GEOM_METADATA m2 ";
	sql += " WHERE m1.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m1.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND m2.table_name = '"+ TeConvertToUpperCase(visGeomTable) +"'";
	sql += " AND m2.column_name = '"+ TeConvertToUpperCase(visGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false; 
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}


bool 
TeOracleSpatial::geomDifference(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, TeGeomRep /* visRep */, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";

	string sql = "SELECT SDO_GEOM.SDO_DIFFERENCE(g1."+ actGeomColl +", m1.diminfo, ";
	sql += " g2."+ visGeomColl +", m2.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ visGeomTable +" g2, ";
	sql += " USER_SDO_GEOM_METADATA m1, USER_SDO_GEOM_METADATA m2 ";
	sql += " WHERE m1.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m1.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND m2.table_name = '"+ TeConvertToUpperCase(visGeomTable) +"'";
	sql += " AND m2.column_name = '"+ TeConvertToUpperCase(visGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}

bool 
TeOracleSpatial::geomDifference(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";

	string sql = "SELECT SDO_GEOM.SDO_DIFFERENCE(g1."+ actGeomColl +", m.diminfo, ";
	sql += " g2."+ actGeomColl +", m.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ actGeomTable +" g2, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}

bool 
TeOracleSpatial::geomXOr(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";

	string sql = "SELECT SDO_GEOM.SDO_XOR(g1."+ actGeomColl +", m.diminfo, ";
	sql += " g2."+ actGeomColl +", m.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ actGeomTable +" g2, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);

	delete portal;
	return true;
}


bool 
TeOracleSpatial::geomXOr(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, TeGeomRep /* visRep */, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";

	string sql = "SELECT SDO_GEOM.SDO_XOR(g1."+ actGeomColl +", m1.diminfo, ";
	sql += " g2."+ visGeomColl +", m2.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ visGeomTable +" g2, ";
	sql += " USER_SDO_GEOM_METADATA m1, USER_SDO_GEOM_METADATA m2 ";
	sql += " WHERE m1.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m1.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND m2.table_name = '"+ TeConvertToUpperCase(visGeomTable) +"'";
	sql += " AND m2.column_name = '"+ TeConvertToUpperCase(visGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}


bool 
TeOracleSpatial::geomUnion(const string& actGeomTable, TeGeomRep /* actRep */, TeKeys& actIds, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string Ids = getStringIds(actIds);

	string sql = "SELECT SDO_GEOM.SDO_UNION(g1."+ actGeomColl +", m.diminfo, ";
	sql += " g2."+ actGeomColl +", m.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ actGeomTable +" g2, USER_SDO_GEOM_METADATA m";
	sql += " WHERE m.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND object_id IN ("+ Ids +")";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
			
	delete portal;
	return true;
}


bool 
TeOracleSpatial::geomUnion(const string& actGeomTable, TeGeomRep /* actRep */, const string& objId1, const string& visGeomTable, TeGeomRep /* visRep */, const string& objId2, TeGeometryVect& geomVect)
{
	string actGeomColl = "spatial_data";
	string visGeomColl = "spatial_data";

	string sql = "SELECT SDO_GEOM.SDO_UNION(g1."+ actGeomColl +", m1.diminfo, ";
	sql += " g2."+ visGeomColl +", m2.diminfo)";
	sql += " FROM "+ actGeomTable +" g1,"+ visGeomTable +" g2, ";
	sql += " USER_SDO_GEOM_METADATA m1, USER_SDO_GEOM_METADATA m2 ";
	sql += " WHERE m1.table_name = '"+ TeConvertToUpperCase(actGeomTable) +"'";
	sql += " AND m1.column_name = '"+ TeConvertToUpperCase(actGeomColl) +"'"; 
	sql += " AND m2.table_name = '"+ TeConvertToUpperCase(visGeomTable) +"'";
	sql += " AND m2.column_name = '"+ TeConvertToUpperCase(visGeomColl) +"'"; 
	sql += " AND g1.object_id = '"+ objId1 +"'"; 
	sql += " AND g2.object_id = '"+ objId2 +"'";

	TeOracleSpatialPortal* portal = (TeOracleSpatialPortal*)getPortal();
	
	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	bool flag = true;
	do 
	{
		bool result=false;
		TeGeometry* geom = new TeGeometry(); 
		TeGeometry* geom2 = geom;
		flag = portal->getGeometry(&geom, result);
		if(result)
			geomVect.push_back (geom);
		delete geom2;
	}while(flag);
		
	delete portal;
	return true;
}


//End Spatial Query

bool
TeOracleSpatial::insertLine (const string& table, TeLine2D &line)
{
	
	string	elinfo = "1, 2, 1";
	
	if(!allocateOrdinatesObject(line))
		return false;

	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, spatial_data) VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(line.objectId()) + "'";
	ins += ", MDSYS.SDO_GEOMETRY(2002, NULL, NULL";
	ins += ", MDSYS.SDO_ELEM_INFO_ARRAY( " + elinfo + " )";
	ins += ", :ordinates_) ";
	ins += " )";

	//OCI
	if(!connection_->executeSDOSTM(ins))
	{
		errorMessage_ = "Error inserting in the table " + table + "!"; 
		return false;
	}

	return true;
}


bool 
TeOracleSpatial::updateLine(const string& table, TeLine2D &line)
{
	string	elinfo = "1, 2, 1";
	
	if(!allocateOrdinatesObject(line))
		return false;
	
	string sql;
	sql =  "UPDATE " + table + " SET ";
	sql += "  object_id= '" + line.objectId() + "'";
	sql += ", spatial_data = ";
	sql += " MDSYS.SDO_GEOMETRY(2002, NULL, NULL";
	sql += ", MDSYS.SDO_ELEM_INFO_ARRAY( " + elinfo + " )";
	sql += ", :ordinates_) ";
	sql += " WHERE geom_id = " +  line.geomId ();

	//OCI
	if(!connection_->executeSDOSTM(sql))
	{
		errorMessage_ = "Error updating in the table " + table + "!"; 
		return false;
	}
	return true;
}


bool 
TeOracleSpatial::loadLineSet (const string& table, const string& geoid, TeLineSet &ls)
{
	
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string q ="SELECT * FROM " + table;
	if (geoid != "")
		q += " WHERE object_id = '" + geoid +"'";
	
	if (!ocip->query(q) || !ocip->fetchRow())
	{	
		delete ocip;
		return false;
	}

	bool flag = true;
	do 
	{
		TeLine2D line;
		flag = ocip->fetchGeometry(line);
		ls.add ( line );
	}while(flag);

	delete ocip;
	return true;
}

bool 
TeOracleSpatial::loadLineSet (const string& table, TeBox &bb, TeLineSet &linSet)
{
	TeOracleSpatialPortal *portal = (TeOracleSpatialPortal*)getPortal();
	if (!portal)
		return false;

	string q;
	q = "SELECT * FROM " + table + " WHERE ";
	q += this->getSQLBoxWhere (bb, TeLINES, table);
	q += " ORDER BY object_id";

	if (!portal->query(q) || !portal->fetchRow())
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

TeDatabasePortal* 
TeOracleSpatial::loadLineSet (const string& table, TeBox &box)
{
	TeOracleSpatialPortal *portal = (TeOracleSpatialPortal*) getPortal();
	if (!portal)
		return 0;

	string q;
	q = "SELECT * FROM " + table + " WHERE ";
	q += this->getSQLBoxWhere (box, TeLINES, table);
	q += " ORDER BY object_id";

	if (!portal->query(q) || !portal->fetchRow())
	{	
		delete portal;
		return 0;
	}
	return portal;
}

bool 
TeOracleSpatial::insertPoint(const string& table, TePoint &point)
{
	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, spatial_data) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(point.objectId()) + "'";
	ins += ", MDSYS.SDO_GEOMETRY(2001, NULL, ";
	ins += "MDSYS.SDO_POINT_TYPE( ";
	ins += Te2String(point.location().x(),15);
	ins += ", " + Te2String(point.location().y(),15);
	ins += ", NULL )";
	ins += ", NULL, NULL)";
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
TeOracleSpatial::updatePoint (const string& table, TePoint &point)
{
	string sql;
	sql =  "UPDATE " + table + " SET ";
	sql += "object_id = '" + point.objectId() + "'";
	sql += ", spatial_data = ";
	sql += " MDSYS.SDO_GEOMETRY(2001, NULL";
	sql += ", MDSYS.SDO_POINT_TYPE( ";
	sql += Te2String(point.location ().x(),15);
	sql += ", " + Te2String(point.location ().y(),15);
	sql += ", NULL )";
	sql += ", NULL, NULL)";
	sql += " WHERE geom_id = " + Te2String(point.geomId());

	if(!execute(sql))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error updating in the table " + table + "!"; 
		return false;
	}

	return true;
}
	
bool 
TeOracleSpatial::insertText	(const string& table, TeText &text)
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
	return true;
}


bool 
TeOracleSpatial::insertArc (const string& table, TeArc &arc)
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

	return true;
}

bool 
TeOracleSpatial::insertNode (const string& table, TeNode &node)
{	
	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, spatial_data) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(node.objectId()) + "'";
	ins += ", MDSYS.SDO_GEOMETRY(2001, NULL, ";
	ins += "MDSYS.SDO_POINT_TYPE( ";
	ins += Te2String(node.location().x(),15);
	ins += ", " + Te2String(node.location ().y(),15);
	ins += ", NULL )";
	ins += ", NULL, NULL)";
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
TeOracleSpatial::updateNode (const string& table, TeNode &node)
{	
	string sql;
	sql =  "UPDATE " + table + " SET ";
	sql += " object_id = '" + node.objectId() + "'";
	sql += ", spatial_data = ";
	sql += " MDSYS.SDO_GEOMETRY(2001, NULL";
	sql += ", MDSYS.SDO_POINT_TYPE( ";
	sql += Te2String(node.location ().x(),15);
	sql += ", " + Te2String(node.location ().y(),15);
	sql += ", NULL )";
	sql += ", NULL, NULL)";
	sql += " WHERE geom_id = " + Te2String(node.geomId());
	
	if(!execute(sql))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error updating in the table " + table + "!"; 
		return false;
	}
	return true;
}

bool 
TeOracleSpatial::insertCell (const string& table, TeCell &cell )
{
	
	TeBox b = cell.box();

	string ins = "INSERT INTO " + table + " ( ";
	ins += " geom_id, object_id, col_number, row_number, spatial_data) ";
	ins += " VALUES ( ";
	ins += getNameSequence(table) +".NEXTVAL";
	ins += ", '" + escapeSequence(cell.objectId ()) + "'";
	ins += ",  " + Te2String(cell.column ());
	ins += ",  " + Te2String(cell.line ());
	ins += ", MDSYS.SDO_GEOMETRY(2003, NULL, NULL";
	ins += ", MDSYS.SDO_ELEM_INFO_ARRAY( 1, 1003, 3 )";
	ins += ", MDSYS.SDO_ORDINATE_ARRAY( " ;
	ins += Te2String(b.lowerLeft().x(), 15);
	ins += ", " + Te2String(b.lowerLeft().y(),15);
	ins += ", " + Te2String(b.upperRight().x(),15);
	ins += ", " + Te2String(b.upperRight().y(),15);
	ins += ")) ";
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
TeOracleSpatial::updateCell(const string& table, TeCell &cell)
{
	TeBox b = cell.box ();
	
	string sql;
	sql =  "UPDATE " + table + " SET ";
	sql += " object_id= '" + cell.objectId() + "'";
	sql += " col_number= " + Te2String(cell.column ());
	sql += " row_number= " + Te2String(cell.line ());
	sql += " spatial_data= ";
	sql += " MDSYS.SDO_GEOMETRY(2003, NULL, NULL";
	sql += ", MDSYS.SDO_ELEM_INFO_ARRAY( 1, 1003, 3 )";
	sql += ", MDSYS.SDO_ORDINATE_ARRAY( " ;
	sql += Te2String(b.lowerLeft().x(), 15);
	sql += ", " + Te2String(b.lowerLeft().y(),15);
	sql += ", " + Te2String(b.upperRight().x(),15);
	sql += ", " + Te2String(b.upperRight().y(),15);
	sql += ")) ";
	sql += " WHERE geom_id = " +  cell.geomId ();
	
	if(!execute(sql))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error updating in the table " + table + "!"; 
		return false;
	}
	
	return true;
}

bool
TeOracleSpatial::deleteMetadata(const string &table, const string &column)
{
	string del = "DELETE FROM USER_SDO_GEOM_METADATA ";
	del += " WHERE TABLE_NAME = '" + TeConvertToUpperCase(table);
	del += "' AND COLUMN_NAME = '" + TeConvertToUpperCase(column) + "'";
	if(!(execute(del.c_str ())))
		return false;
	return true;
}


bool
TeOracleSpatial::rebuildSpatialIndex(const string &table)
{
	TeOracleSpatialPortal *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string tabIndex;

	string ind = "SELECT index_name FROM USER_SDO_INDEX_INFO";
	ind += " WHERE table_name = '" + TeConvertToUpperCase(table) + "'";
	if (!ocip->query(ind))
	{
		delete ocip;
		return false;
	}
	
	if(!ocip->fetchRow())
	{
		delete ocip;
		return false;
	}

	tabIndex = string(ocip->getData(0));
	delete ocip;
		
	string reb = "ALTER INDEX ";
	reb += tabIndex + " REBUILD";
	if(!execute(reb))
		return false;

	return true;
}

bool
TeOracleSpatial::deleteSpatialIndex(const string &table)
{
	TeOracleSpatialPortal	*ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string tabIndex;

	string ind = "SELECT index_name FROM USER_SDO_INDEX_INFO";
	ind += " WHERE table_name = '"+ TeConvertToUpperCase(table) +"'";
	if (!ocip->query(ind))
	{
		delete ocip;
		return false;
	}

	if(!ocip->fetchRow())
	{
		delete ocip;
		return false;
	}

	tabIndex = string(ocip->getData(0));
	ocip->freeResult();
	delete ocip;

	string drop = "DROP INDEX "+ tabIndex;
	if (!(execute(drop.c_str ())))
		return false;
	return true;
}

bool 
TeOracleSpatial::generateLabelPositions	(TeTheme *theme, const std::string& objectId)
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
		
		upd= " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(SDO_GEOM.SDO_MIN_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 1) + (SDO_GEOM.SDO_MAX_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 1) -  SDO_GEOM.SDO_MIN_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 1))/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(SDO_GEOM.SDO_MIN_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 2) + (SDO_GEOM.SDO_MAX_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 2) -  SDO_GEOM.SDO_MIN_MBR_ORDINATE( ";
		upd += geomTable + ".spatial_data, 2))/2) ";
		upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";

		upd += " WHERE label_x IS NULL OR label_y IS NULL";

	}
	
	else if (theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);
		
		upd= " UPDATE "+ collTable +" SET ";
		upd += " label_x = (SELECT MAX(p.spatial_data.SDO_POINT.X) ";
		upd += " FROM " + geomTable + " p WHERE object_id = c_object_id), ";
		
		upd += " label_y = (SELECT MAX(p.spatial_data.SDO_POINT.Y) ";
		upd += " FROM " + geomTable + " p WHERE object_id = c_object_id) ";
		upd += " WHERE label_x IS NULL OR label_y IS NULL";
	}

	if (!objectId.empty())
		upd += " AND c_object_id='"+objectId+"'";
		
	return execute(upd);
}

bool 
TeOracleSpatial::locatePolygon (const string& table, TeCoord2D &pt, TePolygon& polygon, const double& /* tol */)
{
	TeOracleSpatialPortal  *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string q = "SELECT * FROM " + table;
	q += " WHERE MDSYS.SDO_RELATE (spatial_data,";
	q += "MDSYS.SDO_GEOMETRY(2001,NULL, MDSYS.SDO_POINT_TYPE(";
	q += Te2String(pt.x(),15) + ", " + Te2String(pt.y(), 15);
	q += ", NULL), NULL, NULL), ";
	q += " 'mask=contains querytype = window') = 'TRUE'";

	if (!ocip->query(q) || !ocip->fetchRow())
	{	
		delete ocip;
		return false;
	}

	ocip->fetchGeometry(polygon);
	delete ocip;
	return true;
}

bool 
TeOracleSpatial::locateLine (const string& table, TeCoord2D &pt, TeLine2D& line, const double& tol)
{

	TeOracleSpatialPortal	*ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	
	string sdoGeom = "mdsys.sdo_geometry(2003,null,null,";
	sdoGeom += "mdsys.sdo_elem_info_array(1,1003,3),";
	sdoGeom += "mdsys.sdo_ordinate_array(";
	sdoGeom += Te2String(box.x1(),15) + ", " + Te2String(box.y1(),15);
	sdoGeom += ", " + Te2String(box.x2(),15) + ", " + Te2String(box.y2(),15) + "))";
		
	string q = "SELECT * FROM " + table;
	q += " WHERE MDSYS.SDO_RELATE (spatial_data,";
	q += sdoGeom +","; 
	q += "'mask=anyinteract querytype = window') = 'TRUE'";

	if (!ocip->query(q) || !ocip->fetchRow())
	{
		delete ocip;
		return false;
	}

	ocip->fetchGeometry(line);
	delete ocip;
	return true;
}

bool 
TeOracleSpatial::locatePoint (const string& table, TeCoord2D &pt, TePoint& point, const double& tol)
{
	TeOracleSpatialPortal	*ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	
	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	
	string sdoGeom = "mdsys.sdo_geometry(2003,null,null,";
	sdoGeom += "mdsys.sdo_elem_info_array(1,1003,3),";
	sdoGeom += "mdsys.sdo_ordinate_array(";
	sdoGeom += Te2String(box.x1(),15) + ", " + Te2String(box.y1(),15);
	sdoGeom += ", " + Te2String(box.x2(),15) + ", " + Te2String(box.y2(),15) + "))";
		
	string q = "SELECT * FROM " + table;
	q += " WHERE MDSYS.SDO_RELATE (spatial_data,";
	q += sdoGeom +","; 
	q += "'mask=anyinteract querytype = window') = 'TRUE'";
	
	if (!ocip->query(q) || !ocip->fetchRow())
	{
		delete ocip;
		return false;
	}

	ocip->fetchGeometry(point);
	delete ocip;
	return true;
}

bool 
TeOracleSpatial::locateCell(const string& table, TeCoord2D &pt, TeCell& cell, const double& /* tol */)
{
	TeOracleSpatialPortal  *ocip = (TeOracleSpatialPortal*)getPortal();
	if(!ocip)
		return false;

	string q = "SELECT * FROM " + table;
	q += " WHERE MDSYS.SDO_RELATE (spatial_data,";
	q += "MDSYS.SDO_GEOMETRY(2001,NULL,MDSYS.SDO_POINT_TYPE(";
	q += Te2String(pt.x(),15) + ", " + Te2String(pt.y(), 15);
	q += ", NULL), NULL, NULL), ";
	q += " 'mask=contains querytype = window') = 'TRUE'";

	if (!ocip->query(q) || !ocip->fetchRow())
	{	
		delete ocip;
		return false;
	}
	
	ocip->fetchGeometry(cell);
	delete ocip;
	return true;
}

bool TeOracleSpatial::removeGeometry(const string& tableName, const TeGeomRep& rep, const int& geomId)
{
	if(tableName.empty())
	{
		return false;
	}

	std::string remove = "DELETE FROM " + tableName;
	remove += " WHERE geom_id = " + geomId;

	return this->execute(remove);
}

//---------------- TeOracleSpatialPortal

int 
TeOracleSpatialPortal::getDimArraySize()
{
	if(!cursor_)
		return 0;

	return(cursor_->getDimArraySize());
}

bool 
TeOracleSpatialPortal::getDimElement(int i,int &elem)
{
	if(!cursor_)
		return false;

	return (cursor_->getDimElement(i,elem));
}

int 
TeOracleSpatialPortal::numberOfOrdinates()
{
	if(!cursor_)
		return 0;

	return(cursor_->getNumberOrdinates());
}

bool
TeOracleSpatialPortal::getCoordinates(int i,TeCoord2D& coord)
{
	if(!cursor_)
		return false;

	return (cursor_->getCoordinates((i*2)-1,coord));
}

bool
TeOracleSpatialPortal::getGeometryType(TeSDOGType& gType)
{
	if(!cursor_)
		return false;

	int type = cursor_->getGeometryType(); 
	
	switch(type)
	{ 
		case 2000:
			gType=TeSDOUNKNOWN;
			break;

		case 2001:
			gType=TeSDOPOINT;
			break;

		case 2002:
			gType=TeSDOLINE;
			break;

		case 2003:
			gType=TeSDOPOLYGON;
			break;

		case 2004:
			gType=TeSDOCOLLECTION;
			break;

		case 2005:
			gType=TeSDOMULTIPOINT;
			break;

		case 2006:
			gType=TeSDOMULTILINE;
			break;
		
		case 2007:
			gType=TeSDOMULTIPOLYGON;
			break;
		default:
			return false;
	};
	return true;
}

int
TeOracleSpatialPortal::getSpatialReferenceId()
{
	if(!cursor_)
		return -1;

	return (cursor_->getSpatialReferenceId());
}

bool
TeOracleSpatialPortal::getPointXYZ (double& x,double& y)
{
	if(!cursor_)
		return false;

	return (cursor_->getXYZcoord(x,y));
}

bool TeOracleSpatialPortal::querySDO (const string &q)  
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

	if (!cursor_->querySDO(q))
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
		
		attribute.rep_.name_ = cursor_->colName(i); 
		attribute.rep_.numChar_ = cursor_->colSize(i);
		attList_.push_back ( attribute );
	}

	curRow_=-1;
	return true;
}

bool
TeOracleSpatialPortal::fetchGeometry (TePolygon& poly)
{
	int			elem,elemnext,i,k,elemType, sdoInterp;

	try
	{
		int ndim = getDimArraySize();

		if(ndim==0)
			return false;

		vector<TeCoord2D> coordinates;
		if(!cursor_->getCoordinates (coordinates))
			return false;

		//number of the oordinates in the SDO_GEOMETRY
		int noords = numberOfOrdinates();
	
		//Indicates the type of the SDO_GEOMETRY
		int sdoEType;  
		getDimElement(2,sdoEType);
	
		int geomId = atoi(getData("geom_id"));
		string objId = string(getData("object_id"));
		
		// 1005: compound polygon: Compound polygon with some vertices connected by straight
		//line segments and some by circular arcs.
		if((sdoEType==1005) || (sdoEType==2005))  
		{					   
			TeLine2D	lin;   
			//nelem: number of elemnts
			int nelem; 
			getDimElement(3,nelem);  
			
			//posinit is initial position in the vector 
			int posinit=0; //1;
			//ipoxmax: second element position in the ordinates array
			int iposmax=7;			
			
			int posmax;
			bool thisElemArc = false; 

			//keep the last point of the element
			//TePoint lastPoint;
			
			//for each element
			for(elem=1; elem<=nelem; elem++)  
			{
				//to catch the last position of the element: iposmax
				if(elem==nelem)
					posmax = noords+1;
				else
					getDimElement(iposmax, posmax);

				//verify if the element is a arc 
				getDimElement((iposmax-1), elemType);
				if(elemType == 2)
					thisElemArc = true;   
				else 
					thisElemArc = false;

				//ptSet: ordinates of the element
				TePointSet ptSet;

				//to catch all coords of the element
				int pos=0;
									
				for(pos=posinit; pos<((posmax-1)/2);++pos)
				{
					TePoint pt(coordinates[pos]);
					ptSet.add(pt);
				}

				if(thisElemArc) 
				{
					//pegar o proximo
					if(elem!=nelem)
					{
						TePoint pt(coordinates[pos]);
						ptSet.add(pt);
					}

					int size = ptSet.size();
					for(int s=1; (s+1)<size; s=s+2)
					{
						TeLine2D arc;
						TeGenerateArc (ptSet[s-1], ptSet[s], ptSet[s+1], arc, 10);

						int sz = arc.size();
						for(int i=0; i<sz; i++)
							lin.add(arc[i]);
					}
				}

				else //line segment
				{
					int size = ptSet.size();
					for(int s=0; s<size; s++)
						lin.add(ptSet[s].location());
				}

				iposmax+=3;
				posinit=pos;  //skip first coordinate: equal last point of previous element 
			
			} //for each element

			TeLinearRing rg(lin);
			rg.objectId(objId);
			rg.geomId(geomId);
			poly.add(rg);
		}

		else if((sdoEType==1003)||(sdoEType==2003))  //no complex
		{
			for(i=1;i<=ndim;i+=3)
			{
				TeLine2D	line;
				getDimElement(i,elem);	// oordinate position
				if((i+3) > ndim)
					elemnext = noords+1;
				else
					getDimElement(i+3,elemnext);

				getDimElement(i+2,sdoInterp);	// sdo interpretation 
				// sdoInterp = 3: Rectangle type
				// sdoInterp = 2: Polygon made up of a connected sequence of circular arcs 

				if(sdoInterp==2)
				{
					//para gerar o arco
					TePoint pt1;
					TePoint pt2;
					TePoint pt3;
										
					int cont=0;
					int k = elem/2;
					while (k<(elemnext/2))
					{
						for(int i=0; i<3; i++)
						{
							TeCoord2D pt = coordinates[k];
						
							if(i==0)
							{
								if(!cont)
									pt1.add(pt);
								else
								{
									pt1.add(pt3.location());
									pt2.add(pt);
									++i;
								}
							}
							else if (i==1)
								pt2.add(pt);
							else if (i==2)
								pt3.add(pt);

							++cont;
							++k;
						}

						TeLine2D arc;
						TeGenerateArc (pt1, pt2, pt3, arc, 20);
						
						int s = arc.size();
						for(int j=0; j<s; j++)
							line.add(arc[j]);
					}
				}
				else
				{
					// ler os pontos
					for(k=(elem/2);k<(elemnext/2);k++)
					{
						TeCoord2D pt = coordinates[k];
						line.add(pt);
					}

					if(sdoInterp==3) // rectangle
					{
						double xmin,ymin, xmax, ymax;
						xmin = line.box().x1();
						ymin = line.box().y1();
						xmax = line.box().x2();
						ymax = line.box().y2();

						line.clear();
						TeCoord2D pt1(xmin,ymin);
						line.add(pt1);
						TeCoord2D pt2(xmin,ymax);
						line.add(pt2);
						TeCoord2D pt3(xmax,ymax);
						line.add(pt3);
						TeCoord2D pt4(xmax,ymin);
						line.add(pt4);
						line.add(pt1);
					}
				}

				TeLinearRing ring(line);
				ring.objectId (objId);
				ring.geomId(geomId);
				poly.add(ring);
			}
		}
		poly.objectId(objId);
		poly.geomId(geomId);
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool
TeOracleSpatialPortal::fetchGeometry (TePolygon& poly, const unsigned int&)
{
	return this->fetchGeometry(poly);
}


bool
TeOracleSpatialPortal::getGeometry (TeGeometry** geom, bool& result)
{
	TeSDOGType gType;
	bool flag=true;	
	
	try
	{
		this->getGeometryType(gType);
			
		if(gType==TeSDOPOLYGON)
		{
			TePolygon   poly, *pol;
			flag = this->fetchGeometry(poly); 
			pol = new TePolygon();
			*pol = poly;
			*geom = pol;
			result = true;
			return flag;
		}
		else if(gType==TeSDOLINE)
		{
			TeLine2D	line, *lin;
			flag = this->fetchGeometry(line); 
			lin = new TeLine2D();
			*lin = line;
			*geom = lin;
			result = true;
			return flag;
		}
		else if(gType==TeSDOPOINT)
		{
			TePoint		point, *pnt;
			flag = this->fetchGeometry(point); 
			pnt = new TePoint();
			*pnt = point;
			*geom = pnt;
			result = true;
			return flag;
		}

		else if(gType==TeSDOMULTIPOLYGON)
		{
			TePolygonSet   polySet, *polSet;
			flag = this->fetchGeometry(polySet); 
			polSet = new TePolygonSet();
			*polSet = polySet;
			*geom = polSet;
			result = true;
			return flag;
		}
		else if(gType==TeSDOMULTILINE)
		{
			TeLineSet	lineSet, *linSet;
			flag = this->fetchGeometry(lineSet); 
			linSet = new TeLineSet();
			*linSet = lineSet;
			*geom = linSet;
			result = true;
			return flag;
		}
		else if(gType==TeSDOMULTIPOINT)
		{
			TePointSet		pointSet, *pntSet;
			flag = this->fetchGeometry(pointSet); 
			pntSet = new TePointSet();
			*pntSet = pointSet;
			*geom = pntSet;
			result = true;
			return flag;
		}
		
	}
	catch(...)
	{
		result = false;
		return false;
	}
		
	result = false;
	return (this->fetchRow());
}


bool
TeOracleSpatialPortal::fetchGeometry (TePolygonSet& polySet)
{
	int					elem,elemnext,i,k,sdoInterp;
	vector<TeCoord2D>	coordinates;
	
	try
	{
		int ndim = getDimArraySize();
		if(ndim==0)
			return false;
		
		TePolygonSet polyHoles;
		TeSDOGType gType;
		
		getGeometryType(gType);

		if(gType==TeSDOPOLYGON)  
		{
			TePolygon poly;
			bool res = fetchGeometry(poly);
			polySet.add(poly);
			return res;
		}

		if(!cursor_->getCoordinates(coordinates))
			return false;

		int noords = numberOfOrdinates();
	
		int geomId = atoi(getData("geom_id"));
		string objId = string(getData("object_id"));
		
		bool hasHole = false;

		for(i=1;i<=ndim;i+=3)
		{
			int			eType;
			TeLine2D	line;
			TePolygon	poly;
			getDimElement(i,elem);	// oordinate position
			if((i+3) > ndim)
				elemnext = noords+1;
			else
				getDimElement(i+3,elemnext);

			getDimElement(i+1, eType);		// sdo_etype do proximo elemento
			getDimElement(i+2,sdoInterp);	// sdo_interpretation
			
			if(eType==2003)
				poly = polyHoles.last();

			// ler os pontos
			for(k=(elem/2);k<(elemnext/2);k++)
			{
				TeCoord2D pt(coordinates[k]);
				line.add(pt);
			}

			if(sdoInterp == 3) // rectangle
			{
				
				double xmin,ymin, xmax, ymax;
				xmin = line.box().x1();
				ymin = line.box().y1();
				xmax = line.box().x2();
				ymax = line.box().y2();

				line.clear();
				TeCoord2D pt1(xmin,ymin);
				line.add(pt1);
				TeCoord2D pt2(xmin,ymax);
				line.add(pt2);
				TeCoord2D pt3(xmax,ymax);
				line.add(pt3);
				TeCoord2D pt4(xmax,ymin);
				line.add(pt4);
				line.add(pt1);
			}
			TeLinearRing ring(line);
			ring.objectId (objId);
			ring.geomId(geomId);
			
			poly.add(ring);
	
			//verificar se o proximo eh hole
			hasHole=false;
			if(i+4<ndim)
			{
				getDimElement(i+4, eType);		// sdo_etype do proximo elemento
				if(eType == 2003)
					hasHole = true;
			}
			poly.objectId(objId);
			poly.geomId(geomId);
			
			if(!hasHole)
				polySet.add(poly);
			else	
				polyHoles.add(poly);
		} //for

		polySet.objectId(objId);
		polySet.geomId(geomId);

		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry (TeLine2D& line)
{
	int					i;
	vector<TeCoord2D>	coordinates;

	try
	{
		if(!cursor_->getCoordinates(coordinates))
			return false;
		
		int noords = numberOfOrdinates();
		if(noords==0)
			return false;

		for(i=0;i<noords/2;i++)
			line.add(coordinates[i]);
			
		line.geomId (atoi(getData("geom_id")));
		line.objectId(string(getData("object_id")));
		return (this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry (TeLine2D& line, const unsigned int& initIndex)
{
	int					i;
	vector<TeCoord2D>	coordinates;

	try
	{
		if(!cursor_->getCoordinates(coordinates))
			return false;
		
		int noords = numberOfOrdinates();
		if(noords==0)
			return false;

		for(i=0;i<noords/2;i++)
			line.add(coordinates[i]);
			
		line.geomId (atoi(getData(initIndex)));
		line.objectId(string(getData(initIndex+1)));
		return (this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}


bool 
TeOracleSpatialPortal::fetchGeometry (TeLineSet& lineSet)
{
	vector<TeCoord2D>	coordinates;
	int					elem,elemnext,i,k;

	try
	{
		int ndim = getDimArraySize();
		if(ndim==0)
			return false;
		
		int noords = numberOfOrdinates();
		int geomId = atoi(getData("geom_id"));
		string objId = string(getData("object_id"));
		
		TeSDOGType gType;
		getGeometryType(gType);

		if(gType==TeSDOLINE)  
		{
			TeLine2D line;
			fetchGeometry(line);
			lineSet.add(line);
			return true;
		}

		if(!cursor_->getCoordinates(coordinates))
			return false;

		for(i=1;i<=ndim;i+=3)
		{
			TeLine2D	line;
			getDimElement(i,elem);		// oordinate position
			if((i+3) > ndim)
				elemnext = noords+1;
			else
				getDimElement(i+3,elemnext);
			
			// ler os pontos
			for(k=(elem/2);k<(elemnext/2);k++)
			{
				TeCoord2D pt(coordinates[k]);
				line.add(pt);
			}
			line.objectId (objId);
			line.geomId(geomId);
			lineSet.add(line);
		}

		lineSet.objectId(objId);
		lineSet.geomId(geomId);
		
		return (this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry(TePoint& p)
{
	double		x,y;
	
	try
	{
		if(!getPointXYZ(x,y))
			return false;		//point in SDO_POINT

		TeCoord2D c(x,y);
		p.add(c);
		p.objectId(string(getData("object_id")));
		p.geomId(atoi(getData("geom_id")));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	double		x,y;
	
	try
	{
		if(!getPointXYZ(x,y))
			return false;		//point in SDO_POINT

		TeCoord2D c(x,y);
		p.add(c);
		p.objectId(string(getData(initIndex+1)));
		p.geomId(atoi(getData(initIndex)));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry(TePointSet& pointSet)
{
	double		x,y;
	vector<TeCoord2D> coordinates;

	x=y=-99999.;
	try
	{
		int noords = numberOfOrdinates();
		if(noords==0)
			return false;

		TeSDOGType gType;
		getGeometryType(gType);

		if(gType==TeSDOPOINT)  
		{
			TePoint point;
			fetchGeometry(point);
			pointSet.add(point);
			return true;
		}

		if(!cursor_->getCoordinates(coordinates))
			return false;

		int geomId = atoi(getData("geom_id"));
		string objId = string(getData("object_id"));
		
		for(int i=1;i<=noords/2;i++)
		{
			TePoint pt(coordinates[i-1]);
			pt.geomId (geomId);
			pt.objectId (objId);
			pointSet.add(pt);
		}
		
		pointSet.objectId(objId);
		pointSet.geomId(geomId);
		
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry(TeNode& n)
{
	double	x,y;
	x=y=-99999.;
	try
	{
		if(!getPointXYZ(x,y))
			return false;

		TeCoord2D point(x,y);
		n.add(point);
		n.geomId(atoi(getData("geom_id")));
		n.objectId(string(getData("object_id")));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool 
TeOracleSpatialPortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	double	x,y;
	x=y=-99999.;
	try
	{
		if(!getPointXYZ(x,y))
			return false;

		TeCoord2D point(x,y);
		n.add(point);
		n.geomId(atoi(getData(initIndex)));
		n.objectId(string(getData(initIndex+1)));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool	
TeOracleSpatialPortal::fetchGeometry (TeCell& cell)
{
	TeCoord2D coord1,coord2;
	try
	{
		getCoordinates (1, coord1);
		getCoordinates (2, coord2);
		TeBox b(coord1.x(), coord1.y(), coord2.x(), coord2.y());
		
		cell.setBox (b);
		cell.geomId(atoi(getData("geom_id")));
		cell.objectId (string(getData("object_id")));
		cell.column(atoi(getData("col_number")));
		cell.line(atoi(getData("row_number")));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}

bool	
TeOracleSpatialPortal::fetchGeometry (TeCell& cell, const unsigned int& initIndex)
{
	TeCoord2D coord1,coord2;
	try
	{
		getCoordinates (1, coord1);
		getCoordinates (2, coord2);
		TeBox b(coord1.x(), coord1.y(), coord2.x(), coord2.y());
		
		cell.setBox (b);
		cell.geomId(atoi(getData(initIndex)));
		cell.objectId (string(getData(initIndex+1)));
		cell.column(atoi(getData(initIndex+2)));
		cell.line(atoi(getData(initIndex+3)));
		return(this->fetchRow());
	}
	catch(...)
	{
		errorMessage_ = cursor_->getErrorMessage();
		return false;
	}
}



