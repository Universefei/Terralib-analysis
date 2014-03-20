/** \file TerraManagerDatatypes.h
  * \brief This file contains datatypes used by TerraManager.
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  */

#ifndef  __TERRAMANAGER_INTERNAL_TERRAMANAGERDATATYPES_H
#define  __TERRAMANAGER_INTERNAL_TERRAMANAGERDATATYPES_H

// TerraLib include files
#include <TeVisual.h>

// STL include files
#include <vector>

namespace TeMANAGER
{

/** \enum TeDBMSType
  * \brief Types for Database Management Systems connections.
  */
enum TeDBMSType
{
	TeMySQLDB = 1,			/**< For MySQL connection. */
	TePostgreSQLDB = 2,		/**< For PostgreSQL without PostGIS extension support. */
	TePostGISDB = 3,		/**< For POstgreSQL with PostGIS exetnsion support. */
	TeADODB = 4,			/**< For Microsoft Access. */
	TeADOOracleDB = 5,		/**< For Oracle connection using Microsoft ADO support (only in Windows). */
	TeADOSqlServerDB = 6,	/**< For SQL Server (only in Windows). */
	TeOracleSpatialDB = 7,	/**< For Oracle Spatial using OCI support. */
	TeOracleOCIDB = 8		/**< For Oracle without spatial support using OCI. */
};

struct TeCartogramParameters
{
    std::vector<TeColor> colors_;
	int minSize_;
	int maxSize_;
    int width_;
	int type_;
};

} // end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_TERRAMANAGERDATATYPES_H


