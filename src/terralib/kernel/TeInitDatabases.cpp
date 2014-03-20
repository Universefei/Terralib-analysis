/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2004 INPE and Tecgraf/PUC-Rio.

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
#include <TeInitDatabases.h>

#include <teConfig.h>

#ifdef PostGIS_FOUND
#include <TePostgreSQL.h>
#include <TePostGIS.h>
#endif //PostGIS_FOUND

#ifdef ORACLE_FOUND
#include <TeOracleSpatial.h>
#include <TeOCIOracle.h>
#endif //ORACLE_FOUND

#ifdef FBIRD_FOUND
#include <TeFirebird.h>
#endif //FBIRD_FOUND

#ifdef MySQL_FOUND
#include <TeMySQL.h>
#endif //MySQL_FOUND


#ifdef SQLite_ENABLED
#include <TeSQLite.h>
#endif //SQLite_ENABLED

void TeInitDatabases()
{
	static bool TeDatabasesFactoryInitalized__ = false;

	if (!TeDatabasesFactoryInitalized__)
	{
		TeDatabasesFactoryInitalized__ = true;

		

		#ifdef PostGIS_FOUND
		static TePostgreSQLFactory TePostgreSQLFactory_instance;
		static TePostGISFactory TePostGISFactory_instance;
		#endif //PostGIS_FOUND

		#ifdef ORACLE_FOUND
		static TeOracleSpatialFactory TeOracleSpatialFactory_instance;
		static TeOCIOracleFactory TeOCIOracleFactory_instance;
		#endif //ORACLE_FOUND

		#ifdef FBIRD_FOUND
		static TeFirebirdFactory TeFirebirdFactory_instance;
		#endif //FBIRD_FOUND

		#ifdef MySQL_FOUND
		static TeMySQLFactory TeMySQLFactory_instance;
		#endif //MySQL_FOUND

		
		#ifdef SQLite_ENABLED
		static TeSQLiteFactory TeSQLiteFactory_instance;
		#endif //SQLite_ENABLED
	}

	 
		
}
