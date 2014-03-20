#include <TeSFSTheme.h>

#include <TeDatabase.h>
#include <TeDBConnectionsPool.h>

TeSFSTheme::TeSFSTheme(const std::string& name, TeViewNode* parent, int view, int id)
	: TeAbstractTheme(name, parent, view, id, (TeViewNodeType)TeSFSTHEME),
	_projection(0),
	_localDB(0),
	_tableName(""),
	_columnName(""),
	_remoteDbId(-1)
{}

TeSFSTheme::~TeSFSTheme()
{}

void TeSFSTheme::setLocalDatabase(TeDatabase* database)
{
	_localDB = database;
}

void TeSFSTheme::setRemoteDatabaseId(const int& remoteDbId)
{
	_remoteDbId = remoteDbId;
}

void TeSFSTheme::setRemoteDatabase(TeDatabase* remoteDb)
{
	_remoteDb = remoteDb;
}

TeDatabase* TeSFSTheme::getLocalDatabase()
{
	return _localDB;
}

void TeSFSTheme::setThemeProjection(TeProjection* proj)
{ 
	if (_projection)
		delete _projection;
	_projection = proj; 
}

TeProjection* TeSFSTheme::getThemeProjection()
{
	return _projection;
}

void TeSFSTheme::getAttributeList(TeAttributeList& attrList)
{
	attrList = _attrList;
}

TeDatabasePortal* TeSFSTheme::getQueryPortal(const TeBox& box, int& spatialColumnIndex, TeGeomRep& geomRep)
{
	if(_originalAttrList.empty())
	{
		_remoteDb->getAttributeList(_tableName, _originalAttrList);
	}
	
	std::string columnList = "";

	std::string indexColumnName = "";
	for(unsigned int i = 0; i < _originalAttrList.size(); ++i)
	{
		if(_originalAttrList[i].rep_.name_ == _columnName)
		{
			continue;
		}

		if(_originalAttrList[i].rep_.isPrimaryKey_)
		{
			indexColumnName = _originalAttrList[i].rep_.name_;
		}

		columnList += "," + _originalAttrList[i].rep_.name_;
	}

	columnList =  ", " + _columnName + " as spatial_data" + columnList;

	if(indexColumnName.empty())
	{
		columnList = "1 as geom_id, 1 as object_id " + columnList;
	}
	else
	{
		columnList = "1 as geom_id, " + indexColumnName + " as object_id " + columnList;
	}
	
	TeDatabasePortal* portal = _remoteDb->getPortal();

	std::string sql = "SELECT " + columnList + " FROM " + _tableName;
	sql += " WHERE " + _remoteDb->getSQLBoxWhere(box, geomRep, _tableName, _columnName);

	if(portal->query(sql) == false)
	{
		delete portal;
		return 0;
	}

	_attrList = portal->getAttributeList();
	spatialColumnIndex = 2;
	geomRep = (TeGeomRep)this->visibleRep();

	if(portal->fetchRow() == false)
	{
		delete portal;
		return 0;
	}

	return portal;
}

void TeSFSTheme::setSFSTableInfo(const std::string& tableName, const std::string& columnName)
{
	_tableName = tableName;
	_columnName = columnName;
}

bool TeSFSTheme::save()
{
//insert theme in database 
	if(id()==0)
	{
		if(!_localDB->insertTheme(this)) //updateThemeTable
		{
			_localDB->deleteTheme(this->id());
			return false;
		}
	}
	else
	{
		if(!_localDB->updateTheme(this)) //updateThemeTable
		{
			_localDB->deleteTheme(this->id());
			return false;
		}
	}
	return true;
}

bool TeSFSTheme::buildGrouping(const TeGrouping& g, TeSelectedObjects selectedObjects, vector<double>* dValuesVec)
{
	return true;
}

bool TeSFSTheme::buildGrouping(const TeGrouping& g, TeChronon chr, vector<map<string, string> >& mapObjValVec)
{
	return true;
}

bool TeSFSTheme::saveGrouping(TeSelectedObjects selectedObjects)
{
	return true;
}

bool TeSFSTheme::deleteGrouping()
{
	return true;
}

void TeSFSTheme::setLegendsForObjects()
{
}

void TeSFSTheme::setOwnLegendsForObjects()
{
}

bool TeSFSTheme::locatePolygon(TeCoord2D &pt, TePolygon &polygon, const double& tol)
{
	return true;
}

bool TeSFSTheme::locatePolygonSet(TeCoord2D &pt, double tol, TePolygonSet &polygons)
{
	return true;
}

bool TeSFSTheme::locateLine(TeCoord2D &pt, TeLine2D &line, const double& tol)
{
	return true;
}

bool TeSFSTheme::locatePoint(TeCoord2D &pt, TePoint &point, const double& tol)
{
	return true;
}

bool TeSFSTheme::locateCell(TeCoord2D &pt, TeCell &c, const double& tol)
{
	return true;
}

set<string> TeSFSTheme::getObjects(TeSelectedObjects selectedObjects)
{
	return std::set<std::string>();
}


set<string> TeSFSTheme::getObjects(const vector<string>& itemVec)
{
	return std::set<std::string>();
}

vector<string> TeSFSTheme::getItemVector(TeSelectedObjects selectedObjects)
{
	return vector<string>();
}

vector<string> TeSFSTheme::getItemVector(const set<string>& oidSet)
{
	return vector<string>();
}

unsigned int TeSFSTheme::getNumberOfObjects()
{
	return 0;
}

bool TeSFSTheme::saveMetadata(TeDatabase* database)
{	
	int themeId = id();

	if(_projection == 0)
		_projection = new TeNoProjection();

	if(_projection->id() <= 0)
	{
		if(database->insertProjection(_projection) == false)
			return false;
	}

	TeDatabasePortal* portal = database->getPortal();
	if(!portal)
		return false;
	
	string sql = "SELECT * FROM te_sfs_theme WHERE theme_id = "+ Te2String(themeId);
	bool isUpdate = false;
	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}

	if(portal->fetchRow())
		isUpdate = true;
	
	delete portal;

	string strSQL;
	if(isUpdate)
	{
		strSQL  = "UPDATE te_sfs_theme SET ";
		strSQL += " database_id = " + Te2String(_remoteDbId);
		strSQL += ",table_name = '" + _tableName + "'";
		strSQL += ",column_name = '" + _columnName + "'";
		strSQL += ",geom_rep = " + Te2String(visibleRep_);
		strSQL += ",proj_id = " + Te2String(getThemeProjection()->id());
		strSQL += " WHERE theme_id = "+ Te2String(themeId);
	}
	else
	{
		strSQL  = "INSERT INTO te_sfs_theme (theme_id, database_id, table_name, column_name, geom_rep, proj_id)";
		strSQL += "VALUES (" + Te2String(themeId) + "," + Te2String(_remoteDbId) + ",'" + _tableName + "','" + _columnName+ "'," + Te2String(visibleRep_) + "," + Te2String(getThemeProjection()->id()) + ")";
	}

	if(!database->execute(strSQL))
		return false;

	return true;
}

bool TeSFSTheme::loadMetadata(TeDatabase* db)
{
	int remoteDBId = -1;
	int projId = -1;

	if(db == 0)
		return false;

	setLocalDatabase(db);

	if(!getRemoteThemeInfo(remoteDBId, _tableName, _columnName, visibleRep_, projId))
		return false;

	TeDatabase* remotedb = TeDBConnectionsPool::instance().getDatabase(remoteDBId);
	if(remotedb == NULL)
	{	
		std::string hostName, dbmsName, dbName, userName, password;
		int port = 0;
		if(!TeDBConnectionsPool::instance().getConnectionInfo(db, remoteDBId, hostName, dbmsName, dbName, userName, password, port))
		{
			return false;
		}

		remotedb = TeDBConnectionsPool::instance().getDatabase(dbmsName, dbName, hostName, userName, password, port);

		if(remotedb != NULL)
		{
			TeDBConnectionsPool::instance().saveExternalDBConnection(db, remotedb);
		}
	}
	
	if(!remotedb || !remotedb->isConnected())
		return false;

	_remoteDbId = remoteDBId;
	_remoteDb = remotedb;

	_projection = _localDB->loadProjection(projId);

	_originalAttrList.clear();
	remotedb->getAttributeList(_tableName, _originalAttrList);

	return true;
}

bool TeSFSTheme::eraseMetadata(TeDatabase* database)
{
	if (!database)
		return false;

	int proid = -1;
	string strSQL = "SELECT proj_id FROM te_sfs_theme WHERE theme_id = "+ Te2String(this->id());
	TeDatabasePortal* portal = database->getPortal();
	if (portal->query(strSQL) && portal->fetchRow())
		proid = portal->getInt(0);
	delete portal;

	strSQL  = " DELETE FROM te_sfs_theme ";
    strSQL += " WHERE theme_id = "+ Te2String(this->id());
	
	if (!database->execute(strSQL))
		return false;

	if (proid > 0)
	{
		if(database->deleteProjection(proid) == false)
		{
			return false;
		}
	}
	return true;
}

bool TeSFSTheme::createSFSThemeTable(TeDatabase* sourceDB)
{
	if(!sourceDB)
		return false;

	if(sourceDB->tableExist("te_sfs_theme"))
		return true;

	TeAttributeList attList;

	TeAttribute att1;
	att1.rep_.name_ = "theme_id";
	att1.rep_.isAutoNumber_ = false;
	att1.rep_.isPrimaryKey_ = true;
	att1.rep_.null_ = false;
	att1.rep_.type_ = TeINT;
	att1.rep_.numChar_ = 0;
	attList.push_back(att1);

	TeAttribute att2;
	att2.rep_.name_ = "database_id";
	att2.rep_.isAutoNumber_ = false;
	att2.rep_.isPrimaryKey_ = false;
	att2.rep_.null_ = false;
	att2.rep_.type_ = TeINT;
	att2.rep_.numChar_ = 0;
	attList.push_back(att2);

	TeAttribute att3;
	att3.rep_.name_ = "table_name";
	att3.rep_.isAutoNumber_ = false;
	att3.rep_.isPrimaryKey_ = false;
	att3.rep_.null_ = false;
	att3.rep_.type_ = TeSTRING;
	att3.rep_.numChar_ = 255;
	attList.push_back(att3);

	TeAttribute att4;
	att4.rep_.name_ = "column_name";
	att4.rep_.isAutoNumber_ = false;
	att4.rep_.isPrimaryKey_ = false;
	att4.rep_.null_ = false;
	att4.rep_.type_ = TeSTRING;
	att4.rep_.numChar_ = 255;
	attList.push_back(att4);

	TeAttribute att5;
	att5.rep_.name_ = "geom_rep";
	att5.rep_.isAutoNumber_ = false;
	att5.rep_.isPrimaryKey_ = false;
	att5.rep_.null_ = false;
	att5.rep_.type_ = TeINT;
	attList.push_back(att5);

	TeAttribute att6;
	att6.rep_.name_ = "proj_id";
	att6.rep_.isAutoNumber_ = false;
	att6.rep_.isPrimaryKey_ = false;
	att6.rep_.null_ = false;
	att6.rep_.type_ = TeINT;
	attList.push_back(att6);

	if(!sourceDB->createTable("te_sfs_theme", attList))
		return false;

	if(!sourceDB->createRelation("fk_sfs_theme_id", "te_sfs_theme", "theme_id", "te_theme", "theme_id", true))
		return false;
	
	if(!sourceDB->createRelation("fk_sfs_theme_proj_id", "te_sfs_theme", "proj_id", "te_projection", "projection_id", false))
		return false;

	return true;
}

bool TeSFSTheme::getRemoteThemeInfo(int& databaseId, std::string& tableName, std::string& columnName, int& geomRep, int& projId)
{
	if(!_localDB)
		return false;

	try
	{
		TeDatabasePortal* dbPortal = _localDB->getPortal();

		if(!dbPortal)
			return false;

		std::string strSQL = "SELECT * FROM te_sfs_theme ";
		strSQL += "WHERE theme_id = " + Te2String(id());

 	    if(!dbPortal->query(strSQL) || !dbPortal->fetchRow())
		{
			dbPortal->freeResult();
			delete dbPortal;
			return false;
		}

		databaseId = dbPortal->getInt("database_id");
		tableName = dbPortal->getData("table_name");
		columnName = dbPortal->getData("column_name");
		geomRep = dbPortal->getInt("geom_rep");
		projId = dbPortal->getInt("proj_id");

		delete dbPortal;
	}
	catch(TeException&)
	{
	}
	
	return true;
}
