// TerraManager include files
#include "TerraManagerUtils.h"
#include "TerraManagerException.h"

// TerraLib include files
#include <TeDatabase.h>

#ifdef HAS_ACCESS
#include <TeAdoDB.h>
#endif

#ifdef HAS_ADO_ORACLE
#include <TeAdoOracle.h>
#endif

#ifdef HAS_ADO_SQL_SERVER
#include <TeAdoSqlServer.h>
#endif

#ifdef HAS_OCI
#include <TeOracleSpatial.h>
#include <TeOCIOracle.h>
#endif

#ifdef HAS_MYSQL
#include <TeMySQL.h>
#endif

#ifdef HAS_POSTGRESQL
#include <TePostgreSQL.h>
#include <TePostGIS.h>
#endif

TeDatabase* TeMANAGER::TeMakeConnection(const TeDBMSType& dbType, const std::string& host,
										const std::string& user, const std::string& password,
										const std::string& database, const int& port)
{
	bool res = false;
	TeDatabase* db = 0;
	std::string errorMessage;

	try
	{
		switch(dbType)
		{
#ifdef HAS_ACCESS
			case TeADODB :
#ifdef WIN32
						db = new TeAdo();

						if(host.empty())
							res = db->connect(database);
						else
                            res = db->connect(host);						
		
						if(res)
							return db;

						errorMessage  = "Couldn't connect to the database.";		
						errorMessage += db->errorMessage();

						
#else
						errorMessage = "ADO can't be used.";
#endif
						break;
#endif
		
#ifdef HAS_MYSQL	
			case TeMySQLDB :
						db = new TeMySQL();
						res = true;
						break;
#endif
		
#ifdef HAS_POSTGRESQL	
			case TePostgreSQLDB :
						db = new TePostgreSQL();
						res = true;
						break;
						
			case TePostGISDB :
						db = new TePostGIS();
						res = true;
						break;
#endif
	
#ifdef HAS_ADO_ORACLE				
			case TeADOOracleDB :
#ifdef WIN32
						db = new TeOracle();
						res = true;
#else
						errorMessage = "Oracle ADO can't be used.";
#endif
						break;
#endif
		
#ifdef HAS_ADO_SQL_SERVER	
			case TeADOSqlServerDB :
#ifdef WIN32
							db = new TeSqlServer();
							res = true;
#else
							errorMessage = "SQL Server ADO can't be used.";
#endif
							break;
#endif
		
#ifdef HAS_OCI	
			case TeOracleSpatialDB :
							db = new TeOracleSpatial();
							res = true;
							break;
			
			case TeOracleOCIDB :
							db = new TeOCIOracle();
							res = true;
							break;
#endif
			
			default :
					errorMessage = "You should choose a compatible database driver.";					
		}
	}
	catch(...)
	{
		errorMessage = "Couldn't create a database instance.";
		throw TerraManagerException(errorMessage, "TerraManagerException");
	}

	if(!res)
		throw TerraManagerException(errorMessage, "TerraManagerException");

	try
	{
		res = db->connect(host, user, password, database, port);
	}
	catch(...)
	{
		errorMessage = "Error while opening database connection.";
		throw TerraManagerException(errorMessage, "TerraManagerException");
	}

	if(res)
		return db;

	errorMessage  = "Couldn't connect to the database.";
	errorMessage += db->errorMessage();
	throw TerraManagerException(errorMessage, "TerraManagerException");
}

unsigned int TeMANAGER::TeInterpolatePoint(const TeLine2D& l, const double& location, TeCoord2D& cout)
{
	if(location < 0.0 || location > 1.0 )
		throw TerraManagerException("You must specify a value between 0.0 and 1.0 in order to interpolate.", "TerraManager-Interpolate");

	const unsigned int nsegs = l.size() - 1;

	if(location == 0.0)
	{
		cout = l[0];
		return 0;
	}

	if(location == 1.0)
	{
		cout = l[nsegs];
		return nsegs - 1;
	}

	if(nsegs == 1)
	{
		double segLen = TeDistance(l[0], l[1]);
		cout.x_ = l[0].x_ + ((l[1].x_ - l[0].x_) * location);
		cout.y_ = l[0].y_ + ((l[1].y_ - l[0].y_) * location);
		return 0;
	}

/* more than 1 segment ... we need a loop! */

	double lineLen = TeLength(l);
	double totalLen = 0.0;
	double targetLen = lineLen * location;

	for(unsigned int i = 0; i < nsegs; ++i)
	{
		TeCoord2D& c1 = l[i];
		TeCoord2D& c2 = l[i + 1];

		double segLen = TeDistance(c1, c2);

		if(targetLen < (totalLen + segLen))
		{
			double relativePosition = (targetLen - totalLen) / segLen; /* (targetLenInSeg) / segLen; */
			cout.x_ = c1.x_ + ((c2.x_ - c1.x_) * relativePosition);
			cout.y_ = c1.y_ + ((c2.y_ - c1.y_) * relativePosition);
			return i;
		}

		totalLen += segLen;
	}

/* if there is a roundoff error ...*/
	cout = l[nsegs];
	return nsegs - 1;
}

namespace TeMANAGER
{
const double halfBufferPI   = 1.57079632679489661923;
};

void TeMANAGER::TeGetAngle(const TeCoord2D& first, const TeCoord2D& last, double& ang)
{
	if(first.x_ == last.x_)
	{
		ang = halfBufferPI;
		return;
	}

	if(first.y_ == last.y_)
	{
		ang = 0.0;
		return;
	}

	double dx  = (first.x_ - last.x_);
	double dy  = (first.y_ - last.y_);

	ang  = atan(dy / dx);

	return;
}

