//STL include files
#include <string>

//TerraLib include files
#include "TeDatabase.h"
#include "TeDatabaseFactory.h"
#include "TeDBConnectionsPool.h"

TeDBConnectionsPool::~TeDBConnectionsPool()
{
	clear();
}

TeDatabase* TeDBConnectionsPool::getDatabase(std::string& dbmsName, 
											 std::string& dbName, 
											 std::string& host, std::string& user, 
											 std::string& passwd, int& portNumber)
{
	verifyDbParams(dbmsName, dbName, host, user, passwd, portNumber);

	TeDatabaseFactoryParams params = asFactoryParams(dbmsName, dbName, host, user, passwd, portNumber);

    TeDatabase* db = NULL;

	std::map<std::string, int>::iterator conIt = _dbConns.find(getDbKey(params));

	db = (conIt == _dbConns.end()) ?
		TeDatabaseFactory::make(params) :
		getDatabase(conIt->second); 

    return db;
}

TeDatabase* TeDBConnectionsPool::getDatabase(const int& dbIdx)
{
    TeDatabase* db = NULL;

    std::map<int, TeDatabase*>::iterator dbIt = _dbs.find(dbIdx);

    if(dbIt != _dbs.end())
    {
        db = dbIt->second;
    }

	/*if((db != NULL) && (!db->isConnected()))
	{
		if(!db->connect(db->host(), db->user(), db->password(), db->databaseName(), 
			db->portNumber()))
		{
			removeDatabase(dbIdx);
			db = NULL;
		}
	}*/

    return db;
}

int TeDBConnectionsPool::getDatabaseIdx(TeDatabase* db)
{
    int res = -1;

	std::map<std::string, int>::iterator conIt = _dbConns.find(
		getDbKey(asFactoryParams(db->dbmsName(), db->databaseName(), db->host(), db->user(),
		db->password(), db->portNumber())));

    if(conIt != _dbConns.end())
    {
        res = conIt->second;
    }

    return res;
}

void TeDBConnectionsPool::removeDatabase(const int& dbId)
{
    std::map<int, TeDatabase*>::iterator dbIt = _dbs.find(dbId);

    if(dbIt == _dbs.end())
    {
        throw;
    }

    TeDatabase* db = dbIt->second;

    std::string dbKey = getDbKey(asFactoryParams(db->dbmsName(), db->databaseName(), db->host(), 
        db->user(), db->password(), db->portNumber()));

    std::map<std::string, int>::iterator conIt = _dbConns.find(dbKey);

    if(conIt == _dbConns.end())
    {
        throw;
    }

    _dbs.erase(dbIt);
    _dbConns.erase(conIt);

    if(db->isConnected())
    {
        db->clear();
        db->close();
    }

    delete db;
}
//
void TeDBConnectionsPool::clear()
{
	std::map<int, TeDatabase*>::iterator it;

	for(it = _dbs.begin(); it != _dbs.end(); ++it)
	{
        if(it->second->isConnected())
        {
            it->second->clear();
            it->second->close();
        }
        
        delete it->second;
	}

	_dbs.clear();
	_dbConns.clear();
}

void TeDBConnectionsPool::saveExternalDBConnection(TeDatabase* mainDb, 
												   TeDatabase* externalDb)
{
    std::string host = (externalDb->host().empty()) ? "host_name IS NULL" : 
                                                      "host_name = '" + externalDb->host() + "'";
    std::string user = (externalDb->user().empty()) ? "user_name IS NULL" : 
                                                      "user_name = '" + externalDb->user() + "'";
    std::string passwd = (externalDb->password().empty()) ? "user_password IS NULL" : 
                                                        "user_password = '" + externalDb->password() + "'";

    std::string selClause = "SELECT * FROM te_database_connection ";
    std::string whereClause = "WHERE dbms_name = '" + 
        externalDb->dbmsName() + "' AND " + host + " AND database_name = '" +
        externalDb->databaseName() + "' AND " + user + " AND " + passwd + " AND port_number = " + 
        Te2String(externalDb->portNumber());

	TeDatabasePortal* portal = mainDb->getPortal();

	if(!portal)
    {
        std::string errorMsg = "Fail getting database portal.";

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

	if(!portal->query(selClause + whereClause))
	{
		portal->freeResult();
		delete portal;

        std::string errorMsg = "Fail on external connections SQL.";

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

	if(portal->fetchRow())
    {
        if(getDatabaseIdx(externalDb) == -1)
        {
            int id = portal->getInt("connection_id");
            insertDatabase(externalDb, id);
        }

	    portal->freeResult();
	    delete portal;
        return;
    }

	portal->freeResult();

	std::string strSQL = "";

    strSQL  = "INSERT INTO te_database_connection(dbms_name, host_name, database_name, ";
    strSQL += "user_name, user_password, port_number) ";
    strSQL += "VALUES ('" + externalDb->dbmsName();
    strSQL += "', ";
    strSQL += externalDb->host().empty() ? "NULL" : "'" + externalDb->host() + "'";
    strSQL += ", '" + externalDb->databaseName();
    strSQL += "', ";
    strSQL += externalDb->user().empty() ? "NULL" : "'" + externalDb->user() + "'"; 
    strSQL += ", ";
    strSQL += externalDb->password().empty() ? "NULL" : "'" + externalDb->password() + "'";
    strSQL += ", " + Te2String(externalDb->portNumber());
    strSQL += ")";

    if(!mainDb->execute(strSQL))
    {
        std::string errorMsg = "Fail on inserting external connection on database connections table.";
		delete portal;

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

	selClause = "SELECT connection_id FROM te_database_connection ";

	if(!portal->query(selClause + whereClause))
	{
		portal->freeResult();
		delete portal;

        std::string errorMsg = "Fail on external connections SQL.";

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

	if(!portal->fetchRow())
    {
        std::string errorMsg = "Fail on saving external database connection.";        

		portal->freeResult();
	    delete portal;

		throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }

	int dbId = portal->getInt(0);

	portal->freeResult();
    delete portal;

    insertDatabase(externalDb, dbId);
}

void TeDBConnectionsPool::loadExternalDBConnections(TeDatabase* mainDb)
{
	if(!mainDb->tableExist("te_database_connection"))
	{
		return;
	}

	TeDatabasePortal* portal = mainDb->getPortal();

	if(portal == NULL)
	{
        std::string errorMsg = "Fail getting database portal.";

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
	}

	std::string sql = "SELECT * FROM te_database_connection ORDER BY connection_id";

	if(!portal->query(sql))
	{
		portal->freeResult();
		delete portal;

        return;
	}

    bool flag = portal->fetchRow();
    
    while(flag)
    {
	    std::string host(portal->getData("host_name")),
				    dbName(portal->getData("database_name")),
				    user(portal->getData("user_name")),
				    dbmsName(portal->getData("dbms_name")),
				    passwd(portal->getData("user_password"));
	    int portNumber = portal->getInt("port_number");
        int dbId = portal->getInt("connection_id");

		TeDatabase* database = getDatabase(dbmsName, dbName, host, user, passwd, portNumber);
		if(database != NULL)
		{
			insertDatabase(database, dbId);
		}

        flag = portal->fetchRow();
    }

	if(portal != NULL)
	{
		portal->freeResult();
		delete portal;
	}
}

std::vector<int> TeDBConnectionsPool::getConnectionsIdVector()
{
	std::vector<int> dbIds;
	std::map<std::string, int>::iterator it;
	
	for(it = _dbConns.begin(); it != _dbConns.end(); ++it)
	{
        int dbId = it->second;
		dbIds.push_back(dbId);
	}

	return dbIds;
}

void TeDBConnectionsPool::createDBConnectionTable(TeDatabase* db)
{
	if(db->tableExist("te_database_connection"))
    {
        return;
    }

	TeAttributeList attList;

	TeAttribute att1;
	att1.rep_.name_ = "connection_id";
	att1.rep_.isAutoNumber_ = true;
	att1.rep_.isPrimaryKey_ = true;
	att1.rep_.null_ = false;
	att1.rep_.type_ = TeINT;
	att1.rep_.numChar_ = 0;
	attList.push_back(att1);

	TeAttribute att2;
	att2.rep_.name_ = "dbms_name";
	att2.rep_.isAutoNumber_ = false;
	att2.rep_.isPrimaryKey_ = false;
	att2.rep_.null_ = true;
	att2.rep_.type_ = TeSTRING;
	att2.rep_.numChar_ = 255;
	attList.push_back(att2);

	TeAttribute att3;
	att3.rep_.name_ = "host_name";
	att3.rep_.isAutoNumber_ = false;
	att3.rep_.isPrimaryKey_ = false;
	att3.rep_.null_ = true;
	att3.rep_.type_ = TeSTRING;
	att3.rep_.numChar_ = 255;
	attList.push_back(att3);

	TeAttribute att4;
	att4.rep_.name_ = "database_name";
	att4.rep_.isAutoNumber_ = false;
	att4.rep_.isPrimaryKey_ = false;
	att4.rep_.null_ = true;
	att4.rep_.type_ = TeSTRING;
	att4.rep_.numChar_ = 255;
	attList.push_back(att4);

	TeAttribute att5;
	att5.rep_.name_ = "user_name";
	att5.rep_.isAutoNumber_ = false;
	att5.rep_.isPrimaryKey_ = false;
	att5.rep_.null_ = true;
	att5.rep_.type_ = TeSTRING;
	att5.rep_.numChar_ = 255;
	attList.push_back(att5);

	TeAttribute att6;
	att6.rep_.name_ = "user_password";
	att6.rep_.isAutoNumber_ = false;
	att6.rep_.isPrimaryKey_ = false;
	att6.rep_.null_ = true;
	att6.rep_.type_ = TeSTRING;
	att6.rep_.numChar_ = 255;
	attList.push_back(att6);

	TeAttribute att7;
	att7.rep_.name_ = "port_number";
	att7.rep_.isAutoNumber_ = false;
	att7.rep_.isPrimaryKey_ = false;
	att7.rep_.null_ = true;
	att7.rep_.type_ = TeINT;
	att7.rep_.numChar_ = 0;
	attList.push_back(att7);

	if(!db->createTable("te_database_connection", attList))
    {
        std::string errorMsg = "Fail creating te_dabase_connection table.";
        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
    }
}

void TeDBConnectionsPool::deleteExternalDBConnection(TeDatabase* mainDb, const int& externalDbId)
{
    std::string strSQL = "DELETE FROM te_database_connection WHERE connection_id = " + 
        Te2String(externalDbId);

    if(!mainDb->execute(strSQL))
    {
        throw;
    }

    removeDatabase(externalDbId);
}

void TeDBConnectionsPool::insertDatabase(TeDatabase* db, const int& dbId)
{
    if(_dbs.find(dbId) != _dbs.end())
    {
        return;
    }

	TeDatabaseFactoryParams params;
	params.database_ = db->databaseName();
	params.dbms_name_ = db->dbmsName();
	params.host_ = db->host();
	params.user_ = db->user();
	params.password_ = db->password();
	params.port_ = db->portNumber();
	
	_dbConns[getDbKey(params)] = dbId;
	_dbs[dbId] = db;
}

std::string TeDBConnectionsPool::getDbKey(const TeDatabaseFactoryParams& dbParams) const
{
	std::string dbKey = dbParams.dbms_name_,
                pv(";");

	dbKey += pv + dbParams.database_ + pv + dbParams.host_ + pv + dbParams.user_ + pv + 
		dbParams.password_ + pv + Te2String(dbParams.port_);

    return dbKey;
}

void TeDBConnectionsPool::verifyDbParams(std::string& dbmsName, std::string& /*dbName*/, 
								  std::string& host, std::string& user, 
								  std::string& passwd, int& portNumber)
{
    if(dbmsName == "Access" || dbmsName == "Ado")
	{
		dbmsName = "Ado";
        host = "";
        user = "";
        passwd = "";
        portNumber = -1;
	}
    else if(dbmsName == "SQL Server")
    {
        dbmsName = "SqlServerAdo";
    }else if(dbmsName == "SQL Server Spatial")
	{
		dbmsName="SqlServerAdoSpatial";
	}
	else if(dbmsName == "Oracle")
	{
		dbmsName = "OracleAdo";
	}
	else if(dbmsName == "OracleOCI")
	{
		dbmsName = "OracleOCI";
	}
    else 
	{
		if(host.empty())
		{
			host = "localhost";
		}
		
		if(dbmsName == "MySQL")
		{
			portNumber = 0;
		}
	}
}

TeDatabaseFactoryParams TeDBConnectionsPool::asFactoryParams(const std::string& dbmsName, 
															 const std::string& dbName, 
															 const std::string& host, 
															 const std::string& user, 
															 const std::string& passwd, 
															 const int& portNumber)
{
	TeDatabaseFactoryParams params;

	params.dbms_name_ = dbmsName;
	params.database_ = dbName;
	params.host_ = host;
	params.user_ = user;
	params.password_ = passwd;
	params.port_ = portNumber;

	return params;
}

bool TeDBConnectionsPool::getConnectionInfo(TeDatabase* mainDb, const int& connId, 
											std::string& hostName, std::string& dbmsName, std::string& dbName, 
											std::string& userName, std::string& password, int& port)
{
	if(!mainDb->tableExist("te_database_connection"))
	{
		return false;
	}
	
	TeDatabasePortal* portal = mainDb->getPortal();

	if(portal == NULL)
	{
        std::string errorMsg = "Fail getting database portal.";

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
	}

	std::string sql = "SELECT * FROM te_database_connection WHERE connection_id = " + Te2String(connId);

	if(!portal->query(sql))
	{
		portal->freeResult();
		delete portal;

		std::string errorMsg = "Fail querying data - " + mainDb->errorMessage();

        throw TeException(UNKNOWN_ERROR_TYPE, errorMsg, false);
	}
    
	bool found = false;
	if(portal->fetchRow())
    {
		found = true;

	    hostName = portal->getData("host_name");
		dbName = portal->getData("database_name");
		userName = portal->getData("user_name");
		dbmsName = portal->getData("dbms_name");
		password = portal->getData("user_password");
	    port = portal->getInt("port_number");
    }

	portal->freeResult();
	delete portal;
	portal = NULL;

	return found;
}
