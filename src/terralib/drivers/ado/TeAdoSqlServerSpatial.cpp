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
#include <TeAdoSqlServerSpatial.h>
#include <TeWKBGeometryDecoder.h>
#include <sys/stat.h>

#include <TeUtils.h>

#define CHUNKSIZE	240

typedef map<int,TeNode> TeNodeMap;


inline void TESTHR( HRESULT hr )
{
	if( FAILED(hr) ) _com_issue_error( hr );
}

TeSqlServerSpatial::TeSqlServerSpatial()  
{
    HRESULT hr = CoInitialize(0);
	dbmsName_ = "SqlServerAdoSpatial";
    if(FAILED(hr))
    {
        cout << "Can't start COM!? " << endl;
    }
}

TeSqlServerSpatial::~TeSqlServerSpatial()
{
}


bool 
TeSqlServerSpatial::connect (const string& host, const string& user, const string& password, const string& database, int /* port */)
{
	
	std::string  connectionString;
	
	connectionString = "Provider=SQLNCLI10.1;Integrated Security='';Persist Security Info=False;User ID=";
	connectionString+=user + ";Initial Catalog=" + database + ";Data Source=" + host + ";Initial File Name='';Server SPN=''";

	connectionString = "Provider=SQLOLEDB.1;Password=" + password + ";Persist Security Info=True;User ID=" + user + ";Initial Catalog=" + database + ";Data Source=" + host;

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
TeSqlServerSpatial::getMBRSelectedObjects(string /* geomTable */,string /* colGeom */, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType,TeBox &bout, const double& tol)
{
	string	fields;
	string	query;
	bool	status = false;

	TeBox	box;
	bout = box;

	TeDatabasePortal* portal = this->getPortal();
	fields = " MIN(spatial_data.MakeValid().STEnvelope().STPointN(1).STX) as X1,";
	fields+= "MIN(spatial_data.MakeValid().STEnvelope().STPointN(1).STY) as Y1,";
	fields+= "MAX(spatial_data.MakeValid().STEnvelope().STPointN(3).STX) as X2,";
	fields+= "MAX(spatial_data.MakeValid().STEnvelope().STPointN(3).STY) as Y2 ";
	query =  " SELECT " + fields;
	query += " FROM " + fromClause; 
	if (!whereClause.empty())			query += " WHERE " + whereClause;
	if (!afterWhereClause.empty())		query += afterWhereClause;
	if (portal->query (query))
			{
				bool b = portal->fetchRow();
				while(b)
				{
					string vxmin = portal->getData(0);
					string vymin = portal->getData(1);
					string vxmax = portal->getData(2);
					string vymax = portal->getData(3);
					if(vxmin.empty() || vymin.empty() || vxmax.empty() || vymax.empty())
					{
						b = portal->fetchRow();
						continue;
					}
					double xmin = atof(vxmin.c_str());
					double ymin = atof(vymin.c_str());
					double xmax = atof(vxmax.c_str());
					double ymax = atof(vymax.c_str());
					TeBox	ibox(xmin, ymin, xmax, ymax);
					updateBox (bout, ibox);
					b = portal->fetchRow();
					status = true;
				}
			}
	delete portal;
	return status;
}


string
TeSqlServerSpatial::getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& )
{
	std::string whereBox;

	whereBox=" spatial_data.STIsValid() = 1 and spatial_data.STIntersects (geometry::STGeomFromText('POLYGON((";
	whereBox+=Te2String(box.x1());
	whereBox+=" ";
	whereBox+=Te2String(box.y1());
	whereBox+=",";
	whereBox+=Te2String(box.x2());
	whereBox+=" ";
	whereBox+=Te2String(box.y1());
	whereBox+=",";
	whereBox+=Te2String(box.x2());
	whereBox+=" ";
	whereBox+=Te2String(box.y2());
	whereBox+=",";
	whereBox+=Te2String(box.x1());
	whereBox+=" ";
	whereBox+=Te2String(box.y2());
	whereBox+=",";
	whereBox+=Te2String(box.x1());
	whereBox+=" ";
	whereBox+=Te2String(box.y1());
	whereBox+="))',0))=1 ";
	return whereBox;
}

std::string TeSqlServerSpatial::getSQLOrderBy(const TeGeomRep& rep) const
{
	std::string orderBy = "object_id DESC";
	return orderBy;
}


bool 
TeSqlServerSpatial::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	TeSqlServerSpatialPortal *sqlp = (TeSqlServerSpatialPortal*)getPortal();
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
		else if (dataType == 240)
		{
			attr.rep_.type_ = TeOBJECT;
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
TeSqlServerSpatial::createTable(const string& table, TeAttributeList &attr)
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
				if ( (*it).rep_.numChar_ > 0 && (*it).rep_.numChar_ < 256)
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
			case TeLINE2DTYPE:
			case TeLINESETTYPE:
			case TePOLYGONTYPE:
			case TePOLYGONSETTYPE:
			case TeCELLTYPE:
			case TeCELLSETTYPE:
				tablec += " spatial_data geometry";
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
TeSqlServerSpatial::alterTable (const string& table, TeAttributeRep &rep, const string& oldColName)
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





TeDatabasePortal*  
TeSqlServerSpatial::getPortal ()
{
	TeSqlServerSpatialPortal* portal = new TeSqlServerSpatialPortal (this);
	return portal;
}




bool 
TeSqlServerSpatial::generateLabelPositions (TeTheme *theme, const std::string& objectId )
{
	string	geomTable, upd;
	string	collTable = theme->collectionTable();
	
	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if(theme->layer()->hasGeometry(TePOLYGONS))		geomTable = theme->layer()->tableName(TePOLYGONS);
	else if(theme->layer()->hasGeometry(TeLINES))	geomTable = theme->layer()->tableName(TeLINES);
	else if(theme->layer()->hasGeometry(TePOINTS))	geomTable = theme->layer()->tableName(TePOINTS);
	else if(theme->layer()->hasGeometry(TeCELLS))	geomTable = theme->layer()->tableName(TeCELLS);

	upd="update " + collTable +" set label_x=(select MAX(spatial_data.MakeValid().STCentroid ( ).STX) from ";
	upd+=geomTable + " where spatial_data.STIsValid()=1 and object_id=c_object_id), ";
	upd+=" label_y=(select MAX(spatial_data.MakeValid().STCentroid ( ).STY) from ";
	upd+=geomTable + " where spatial_data.STIsValid()=1 and object_id=c_object_id) ";
	upd += " WHERE label_x IS NULL OR label_y IS NULL";

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



std::string TeSqlServerSpatial::STGeomFromText(const TePolygon &polygon)
{
	std::string					sql;
	TeLine2D					lnePolygon;
	TeLine2D::iterator			it;

	lnePolygon.copyElements(polygon[0]);

	sql=" geometry::STGeomFromText('POLYGON ((";
	for(it=lnePolygon.begin();it!=lnePolygon.end();it++)
	{
		if(it!=lnePolygon.begin()) sql+=",";
		sql+=Te2String((*it).x());
		sql+=" ";
		sql+=Te2String((*it).y());
	}
	sql+="))',0)";
	return sql;
}

std::string TeSqlServerSpatial::STGeomFromWkb(const TePolygon &polygon)
{
	std::string					sql;
	TeLine2D					lnePolygon;
	TeLine2D::iterator			it;
	unsigned int				size;
	char						*wkbPol;

	

	TeWKBGeometryDecoder::encodePolygon(polygon,wkbPol,size);

	sql=" geometry::STGeomFromWKB(";
	sql+=wkbPol;
	sql+="),0)";
	return sql;
}


bool
TeSqlServerSpatial::createSpatialIndex(const string& table, const string& columns, TeSpatialIndexType /*type*/, short /*level*/, short /*tile*/)
{
	std::string			SQL;
	double				x1=0,y1=0;
	double				x2=0,y2=0;
	TeDatabasePortal	*dbPortal=0;

	SQL=" SELECT MIN(spatial_data.MakeValid().STEnvelope().STPointN(1).STX) as X1,";
	SQL+=" MIN(spatial_data.MakeValid().STEnvelope().STPointN(1).STY) as Y1,";
	SQL+=" MAX(spatial_data.MakeValid().STEnvelope().STPointN(3).STX) as X2,";
	SQL+=" MAX(spatial_data.MakeValid().STEnvelope().STPointN(3).STY) as Y2 ";
	SQL+=" FROM " + table;

	dbPortal= this->getPortal();
	if(dbPortal)
	{
		if( dbPortal->query(SQL) && dbPortal->fetchRow())
		{
			x1=dbPortal->getDouble("X1");
			y1=dbPortal->getDouble("Y1");
			x2=dbPortal->getDouble("X2");
			y2=dbPortal->getDouble("Y2");
		}
		delete dbPortal;
	}

	SQL="CREATE SPATIAL INDEX SP_IDX_SPATIAL_" + table;
	SQL+=" ON " + table + "(spatial_data)";
	SQL+=" USING GEOMETRY_GRID WITH (";
	SQL+=" BOUNDING_BOX = ( xmin=" + Te2String(x1);
	SQL+=", ymin=" + Te2String(y1);
	SQL+=", xmax=" + Te2String(x2);
	SQL+=", ymax=" + Te2String(y2);
	SQL+="), GRIDS = (MEDIUM, MEDIUM, MEDIUM, MEDIUM),";
	SQL+=" CELLS_PER_OBJECT = 64,  PAD_INDEX  = ON );";
	return this->execute(SQL);
}


bool 
TeSqlServerSpatial::PutBinaryIntoVariant(VARIANT &ovData, BYTE * pBuf,unsigned long cBufLen)
{
	bool fRetVal = false;
	
	VariantInit(&ovData); 

	ovData.vt = VT_ARRAY | VT_UI1;
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].cElements = cBufLen;
	rgsabound[0].lLbound = 0;

	ovData.parray = SafeArrayCreate(VT_UI1,1,rgsabound);
	if(ovData.parray != NULL)
		{
			void * pArrayData = NULL;
			SafeArrayAccessData(ovData.parray,&pArrayData);
			memcpy(pArrayData, pBuf, cBufLen);
			SafeArrayUnaccessData(ovData.parray);
			fRetVal = true;
		}
	return fRetVal;
}

bool 
TeSqlServerSpatial::insertPolygonSet (const string& table, TePolygonSet &ps)
{
	string			sql;
	unsigned int	i;
	unsigned int	size=0;
	int				geom_id;
	char			*wkb;

	ADODB::_RecordsetPtr recset;
	recset.CreateInstance(__uuidof(ADODB::Recordset));

try
{
	if (ps.empty())	return true;
	for(i=0;i<ps.size(); i++)
	{
		sql=" insert into " + table;
		sql+=" (object_id,spatial_data) values('";
		sql+=ps[i].objectId();
		sql+="',geometry::STPolyFromWKB(?,0))";

		
		TeWKBGeometryDecoder::encodePolygon(ps[i],wkb,size);
		VARIANT varWKB;
		
		if(PutBinaryIntoVariant(varWKB,(BYTE*)wkb,size))
		{
			ADODB::_CommandPtr		comm;
			ADODB::_ParameterPtr	param;

			comm.CreateInstance(__uuidof(ADODB::Command)); 
			comm->ActiveConnection = connection_;
			comm->CommandType=ADODB::adCmdText;
			comm->CommandText=sql.c_str();

			param = comm->CreateParameter(_bstr_t(L""),ADODB::adVarBinary,ADODB::adParamInput,-1);
			param->Value=varWKB;
			comm->Parameters->Append(param);
			comm->Execute(0,0,ADODB::adCmdText);
			SafeArrayDestroy (varWKB.parray);
			delete wkb;
		}
		sql="select @@IDENTITY as id from " + table;
		recset->Open(_bstr_t(sql.c_str()), _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenForwardOnly,ADODB::adLockReadOnly,ADODB::adCmdText);
		geom_id=recset->GetCollect("id").intVal;
		ps[i].geomId(geom_id);
		recset->Close();
	}
}catch(_com_error &e)
{
		errorMessage_ = e.Description();
		if(recset->State == 1)	recset->Close();
		return false;
}
catch(...)
{
	errorMessage_ = "Oppps !";
	recset->Close();
	return false;
}
	return true;
}


bool 
TeSqlServerSpatial::insertPolygon (const string& table, TePolygon &pol)
{
	TePolygonSet	pls;
	pls.add(pol);
	return this->insertPolygonSet(table,pls);
}


std::string TeSqlServerSpatial::STGeomFromText(const TeLine2D &line)
{
	std::string					sql;
	TeLine2D::iterator			it;


	sql=" geometry::STGeomFromText('LINESTRING (";
	for(it=line.begin();it!=line.end();it++)
	{
		if(it!=line.begin()) sql+=",";
		sql+=Te2String((*it).x());
		sql+=" ";
		sql+=Te2String((*it).y());
	}
	sql+=")',0)";
	return sql;
}

//std::string TeSqlServerSpatial::STGeomFromWkb(const TePolygon &polygon)
//{
//
//
//}

bool
TeSqlServerSpatial::insertLineSet(const string& table, TeLineSet &ls)
{
	string			sql;
	unsigned int	i;
	unsigned int	size=0;
	int				geom_id;
	char			*wkb;

	ADODB::_RecordsetPtr recset;
	recset.CreateInstance(__uuidof(ADODB::Recordset));

try
{
	if (ls.empty())	return true;
	for(i=0;i<ls.size(); i++)
	{
		sql=" insert into " + table;
		sql+=" (object_id,spatial_data) values('";
		sql+=ls[i].objectId();
		sql+="',geometry::STLineFromWKB(?,0))";

		
		TeWKBGeometryDecoder::encodeLine(ls[i],wkb,size);
		VARIANT varWKB;
		
		if(PutBinaryIntoVariant(varWKB,(BYTE*)wkb,size))
		{
			ADODB::_CommandPtr		comm;
			ADODB::_ParameterPtr	param;

			comm.CreateInstance(__uuidof(ADODB::Command)); 
			comm->ActiveConnection = connection_;
			comm->CommandType=ADODB::adCmdText;
			comm->CommandText=sql.c_str();

			param = comm->CreateParameter(_bstr_t(L""),ADODB::adVarBinary,ADODB::adParamInput,-1);
			param->Value=varWKB;
			comm->Parameters->Append(param);
			comm->Execute(0,0,ADODB::adCmdText);
			SafeArrayDestroy (varWKB.parray);
			delete wkb;
		}
		sql="select @@IDENTITY as id from " + table;
		recset->Open(_bstr_t(sql.c_str()), _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenForwardOnly,ADODB::adLockReadOnly,ADODB::adCmdText);
		geom_id=recset->GetCollect("id").intVal;
		ls[i].geomId(geom_id);
		recset->Close();
	}
}catch(_com_error &e)
{
		errorMessage_ = e.Description();
		if(recset->State == 1)	recset->Close();
		return false;
}
catch(...)
{
	errorMessage_ = "Oppps !";
	recset->Close();
	return false;
}
	return true;
}

bool
TeSqlServerSpatial::insertLine(const string& table, TeLine2D &l)
{
    TeLineSet	lns;
	lns.add(l);
	return this->insertLineSet(table,lns);
}

std::string TeSqlServerSpatial::STGeomFromText(const TePoint &point)
{
	std::string					sql;
	sql=" geometry::STGeomFromText('POINT (";
	sql+=Te2String(point.location().x());
	sql+=" ";
	sql+=Te2String(point.location().y());
	sql+=")',0)";
	return sql;
}

bool
TeSqlServerSpatial::insertPointSet(const string& table, TePointSet &ps)
{
	string			sql;
	unsigned int	i;
	unsigned int	size=0;
	int				geom_id;
	char			*wkb;

	ADODB::_RecordsetPtr recset;
	recset.CreateInstance(__uuidof(ADODB::Recordset));

try
{
	if (ps.empty())	return true;
	for(i=0;i<ps.size(); i++)
	{
		sql=" insert into " + table;
		sql+=" (object_id,spatial_data) values('";
		sql+=ps[i].objectId();
		sql+="',geometry::STPointFromWKB(?,0))";

		
		TeWKBGeometryDecoder::encodePoint(ps[i].location(),wkb,size);
		VARIANT varWKB;
		
		if(PutBinaryIntoVariant(varWKB,(BYTE*)wkb,size))
		{
			ADODB::_CommandPtr		comm;
			ADODB::_ParameterPtr	param;

			comm.CreateInstance(__uuidof(ADODB::Command)); 
			comm->ActiveConnection = connection_;
			comm->CommandType=ADODB::adCmdText;
			comm->CommandText=sql.c_str();

			param = comm->CreateParameter(_bstr_t(L""),ADODB::adVarBinary,ADODB::adParamInput,-1);
			param->Value=varWKB;
			comm->Parameters->Append(param);
			comm->Execute(0,0,ADODB::adCmdText);
			SafeArrayDestroy (varWKB.parray);
			delete wkb;
		}
		sql="select @@IDENTITY as id from " + table;
		recset->Open(_bstr_t(sql.c_str()), _variant_t((IDispatch*)connection_, true),
            ADODB::adOpenForwardOnly,ADODB::adLockReadOnly,ADODB::adCmdText);
		geom_id=recset->GetCollect("id").intVal;
		ps[i].geomId(geom_id);
		recset->Close();
	}
}catch(_com_error &e)
{
		errorMessage_ = e.Description();
		if(recset->State == 1)	recset->Close();
		return false;
}
catch(...)
{
	errorMessage_ = "Oppps !";
	recset->Close();
	return false;
}
	return true;
}

bool
TeSqlServerSpatial::insertPoint(const string& table, TePoint &p)
{
   
    TePointSet	pts;
	pts.add(p);
	return this->insertPointSet(table,pts);
}


bool 
TeSqlServerSpatial::createTextGeometry(const string& table)
{
	if(table.empty())
		return false;

	TeAttributeList attList;

	{TeAttribute attGeomId;
	attGeomId.rep_.name_ = "geom_id";
	attGeomId.rep_.type_ = TeUNSIGNEDINT;
	attGeomId.rep_.isAutoNumber_ = true;
	attGeomId.rep_.isPrimaryKey_ = true;
	attGeomId.rep_.null_ = false;
	attList.push_back(attGeomId);}

	{TeAttribute attObjId;
	attObjId.rep_.name_ = "object_id";
	attObjId.rep_.type_ = TeSTRING;
	attObjId.rep_.numChar_ = 255;
	attObjId.rep_.null_ = false;
	attList.push_back(attObjId);}


	{TeAttribute attSpatial;
	attSpatial.rep_.name_ = "spatial_data";
	attSpatial.rep_.type_ = TePOINTTYPE;
	attList.push_back(attSpatial);}

	{TeAttribute attTextValue;
	attTextValue.rep_.name_ = "text_value";
	attTextValue.rep_.type_ = TeSTRING;
	attTextValue.rep_.numChar_ = 255;
	attList.push_back(attTextValue);}

	{TeAttribute attAngle;
	attAngle.rep_.name_ = "angle";
	attAngle.rep_.type_ = TeREAL;
	attAngle.rep_.decimals_ = 15;
	attAngle.rep_.defaultValue_ = "0.0";
	attList.push_back(attAngle);}

	{TeAttribute attHeight;
	attHeight.rep_.name_ = "height";
	attHeight.rep_.type_ = TeREAL;
	attHeight.rep_.decimals_ = 15;
	attHeight.rep_.defaultValue_ = "0.0";
	attList.push_back(attHeight);}

	{TeAttribute attAlignVert;
	attAlignVert.rep_.name_ = "alignment_vert";
	attAlignVert.rep_.type_ = TeREAL;
	attAlignVert.rep_.decimals_ = 15;
	attList.push_back(attAlignVert);}

	{TeAttribute attAlignHoriz;
	attAlignHoriz.rep_.name_ = "alignment_horiz";
	attAlignHoriz.rep_.type_ = TeREAL;
	attAlignHoriz.rep_.decimals_ = 15;
	attList.push_back(attAlignHoriz);}

	if(!createTable(table, attList))
		return false;

	string idxName = "te_idx_"  + table + "_obj";

	if(!createIndex(table, idxName, "object_id"))
		return false;

	idxName = "te_idx_"  + table + "_pos";

	return createIndex(table, idxName, "x, y");
}

bool
TeSqlServerSpatial::insertTextSet(const string& table, TeTextSet &ts)
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
TeSqlServerSpatial::insertText(const string& table, TeText &t)
{
    TeTextSet	txs;
	txs.add(t);
	return this->insertTextSet(table,txs);
}

bool 
TeSqlServerSpatial::locatePoint (const string& table, TeCoord2D &pt, TePoint &point, const double& tol)
{
	TePointSet				ps;
	int						k;
	TeDatabasePortal		*portal = this->getPortal();
	bool					flag;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	string q ="SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data FROM " + table;
	q+=" where ";
	q+=this->getSQLBoxWhere(box,TePOINTS,table);
	if (!portal->query(q) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}
	
	do 
	{
		TePoint point;
		flag = portal->fetchGeometry (point);
		ps.add ( point );
	}while (flag);
	delete portal;
	if (TeNearest (pt, ps, k, tol))
	{
		point = ps[k];
		return true;
	}
	return false;
}


bool 
TeSqlServerSpatial::locatePolygon (const string& table, TeCoord2D &pt, TePolygon &polygon, const double& tol)
{
	bool					flag;
	TeDatabasePortal		*portal = this->getPortal();

	if (!portal)			return false;

	std::string q;
	q="select geom_id, object_id, spatial_data.STAsBinary() as spatial_data from ";
	q+=table;
	q+=" where spatial_data.STContains(geometry::STGeomFromText('POINT(";
	q+=Te2String(pt.x());
	q+=" ";
	q+=Te2String(pt.y());
	q+=")',0))=1";
	
	if (!portal->query(q) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}
	
	do
	{
		flag = portal->fetchGeometry(polygon);
	}while (flag);
	delete portal;
	return true;
}

bool 
TeSqlServerSpatial::locatePolygonSet (const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons)
{

	
	bool					flag;
	TeDatabasePortal		*portal = this->getPortal();

	if (!portal)			return false;

	std::string q;
	q="select geom_id, object_id, spatial_data.STAsBinary() as spatial_data from ";
	q+=table;
	q+=" where spatial_data.STContains(geometry::STGeomFromText('POINT(";
	q+=Te2String(pt.x());
	q+=" ";
	q+=Te2String(pt.y());
	q+=")',0))=1";
	
	if (!portal->query(q) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}
	
	do
	{
		TePolygon pol;
		flag = portal->fetchGeometry(pol);
		polygons.add(pol);
	}while (flag);
	delete portal;
	return !polygons.empty();
}


bool 
TeSqlServerSpatial::locateLine (const string& table, TeCoord2D &pt, TeLine2D &line, const double& tol)
{
	TeLineSet				ls;
	int						k;
	bool					flag;
	TeDatabasePortal		*portal = this->getPortal();
	TeCoord2D				paux;

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	string q ="SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data FROM " + table;
	q+=" where ";
	q+=this->getSQLBoxWhere(box,TeLINES,table);

	if (!portal->query(q) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	do 
	{
		TeLine2D l;
		flag = portal->fetchGeometry( l );
		ls.add ( l );
	} while (flag);
	delete portal;
	if (TeNearest (pt, ls, k, paux, tol))
	{
		line = ls[k];
		return true;
	}
	return false;
}

bool 
TeSqlServerSpatial::locateLineSet (const string& table, TeCoord2D &pt, TeLineSet &ls, const double& tol)
{
	TeDatabasePortal		*portal = this->getPortal();
	TeBox					box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	string					q ="SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data FROM " + table;

	q+=" where ";
	q+=this->getSQLBoxWhere(box,TeLINES,table);

	if (!portal->query(q) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	// Get all lines
	bool flag = true;
	do 
	{
		TeLine2D l;
		flag = portal->fetchGeometry( l );
		ls.add(l);
	} while (flag);

	delete portal;
	return !ls.empty();
}

//----- TeSqlServerSpatialPortal methods ---

TeSqlServerSpatialPortal::TeSqlServerSpatialPortal ( TeDatabase*  pDatabase) 
{
	db_ = pDatabase;
	connection_ = ((TeSqlServerSpatial*)pDatabase)->connection_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
}


TeSqlServerSpatialPortal::TeSqlServerSpatialPortal ()
{
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	curRow_ = 0;
}

TeSqlServerSpatialPortal::~TeSqlServerSpatialPortal ()
{
	freeResult();
}

TeTime
TeSqlServerSpatialPortal::getDate (const string& name)
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
TeSqlServerSpatialPortal::getDate (int i)
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
TeSqlServerSpatialPortal::getDateAsString(int i)
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
TeSqlServerSpatialPortal::getDateAsString(const string& s)
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
TeSqlServerSpatialPortal::fetchGeometry(TePolygon& pol)
{
	int				geom_id;
	std::string		object_id;
	unsigned int	readBytes;	
	unsigned char	*wkb=0;
	const char		*wkb2Decoder;
	long			size;

	try 
	{
		this->getBlob("spatial_data",wkb,size);
		if(wkb !=0)
		{
			wkb2Decoder=(const char*)wkb;
			TeWKBGeometryDecoder::decodePolygon(wkb2Decoder,pol,readBytes);
			geom_id=this->getInt("geom_id");
			object_id=this->getData("object_id");

			pol.geomId(geom_id);
			pol.objectId(object_id);
			delete wkb;
		}
		fetchRow(); 
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	if(recset_->IsEOF)	return false;
	return true;
}


bool 
TeSqlServerSpatialPortal::fetchGeometry(TeLine2D& line)
{
	int				geom_id;
	std::string		object_id;
	unsigned int	readBytes;	
	unsigned char	*wkb=0;
	const char		*wkb2Decoder;
	long			size;

	try 
	{
		this->getBlob("spatial_data",wkb,size);
		if(wkb !=0)
		{
			wkb2Decoder=(const char*)wkb;
			TeWKBGeometryDecoder::decodeLine(wkb2Decoder,line,readBytes);
			geom_id=this->getInt("geom_id");
			object_id=this->getData("object_id");

			line.geomId(geom_id);
			line.objectId(object_id);
			delete wkb;
		}
		fetchRow(); 
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	if(recset_->IsEOF)	return false;
	return true;
}

bool 
TeSqlServerSpatialPortal::fetchGeometry(TePoint& p)
{
	TeCoord2D		coord;
	unsigned int	readBytes;	
	unsigned char	*wkb=0;
	const char		*wkb2Decoder;
	long			size;

	try {
			p.geomId( atoi(getData("geom_id")));
			p.objectId( string(getData("object_id")));
			this->getBlob("spatial_data",wkb,size);
			if(wkb !=0)
			{
				wkb2Decoder=(const char*)wkb;
				TeWKBGeometryDecoder::decodePoint(wkb2Decoder,coord,readBytes);
				delete wkb;
				p.add(coord);
		}
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeSqlServerSpatialPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	TeCoord2D		coord;
	unsigned int	readBytes;	
	unsigned char	*wkb=0;
	const char		*wkb2Decoder;
	long			size;

	try 
	{
		p.geomId( atoi(getData(initIndex)));
		p.objectId( string(getData(initIndex+1)));
		this->getBlob("spatial_data",wkb,size);
		if(wkb !=0)
			{
				wkb2Decoder=(const char*)wkb;
				TeWKBGeometryDecoder::decodePoint(wkb2Decoder,coord,readBytes);
				delete wkb;
				p.add(coord);
			}
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}


int getLastGeomId ( TeSqlServerSpatial* db, const string& table )
{
	TeSqlServerSpatialPortal *sqlp = NULL;
	try
	{
		int lastGeomId = 0;

		if (db == NULL)
			return lastGeomId;
		//gets the last geom id
	
		sqlp = (TeSqlServerSpatialPortal*)db->getPortal();
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



