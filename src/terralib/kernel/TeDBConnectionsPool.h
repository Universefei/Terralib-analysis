#ifndef TDK_DB_CONNECTIONSPOOL_H
#define TDK_DB_CONNECTIONSPOOL_H

//STL include files
#include <iostream>
#include <map>
#include <string>
#include <vector>

//TerraLib include files
#include "TeSingleton.h"
#include "TeDatabaseFactoryParams.h"
#include "TeException.h"

//foward declarations
class TeDatabase;

class TL_DLL TeDBConnectionsPool : public TeSingleton<TeDBConnectionsPool>
{
public:

	~TeDBConnectionsPool();

	TeDatabase* getDatabase(std::string& dbmsName, std::string& dbName, 
                            std::string& host, std::string& user, 
                            std::string& passwd, int& portNumber);

public:

    TeDatabase* getDatabase(const int& dbIdx);

    int getDatabaseIdx(TeDatabase* db);

	void clear();

    void saveExternalDBConnection(TeDatabase* mainDb, TeDatabase* externalDb);

	void loadExternalDBConnections(TeDatabase* mainDb);

	void deleteExternalDBConnection(TeDatabase* mainDb, const int& externalDbId);

	std::vector<int> getConnectionsIdVector();

    static void createDBConnectionTable(TeDatabase* db);

	bool getConnectionInfo(TeDatabase* mainDb, 
									const int& connId, 
									std::string& hostName, 
									std::string& dbmsName, 
									std::string& dbName, 
									std::string& userName, 
									std::string& password, 
									int& port);

private:

	void insertDatabase(TeDatabase* db, const int& dbId);

	void removeDatabase(const int& dbId);

	std::string getDbKey(const TeDatabaseFactoryParams& dbParams) const;

	void verifyDbParams(std::string& dbmsName, std::string& dbName, 
								  std::string& host, std::string& user, 
								  std::string& passwd, int& portNumber);

	TeDatabaseFactoryParams asFactoryParams(const std::string& dbmsName, 
											const std::string& dbName, 
											const std::string& host, const std::string& user, 
											const std::string& passwd, const int& portNumber);

    std::map<std::string, int> _dbConns;
    std::map<int, TeDatabase*> _dbs;
};

#endif //TDK_DB_CONNECTIONSPOOL_H
