/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TePostGIS.h
    \brief This file contains the especifics routines of PostgreSQL with PostGIS extension.
*/
#ifndef  __TERRALIB_INTERNAL_POSTGIS_H
#define  __TERRALIB_INTERNAL_POSTGIS_H

#include "TePostgreSQL.h"
#include "../../kernel/TeDatabaseFactory.h"

class TePostGISPortal;

//! PostgreSQL with PostGIS extension database access class.
/*!
	This class contains the implementation of common methods for TerraLib access PostgreSQL using a PostGIS extension.    
	From release 3.1 on, this driver makes use of RTree over GiST.
    So, to use it you will need install RTRee GiST support in your database.
	\note MAKE SURE you have UPGRADE your PostgreSQL database before use this driver.
    If you need to upgrade without use TerraView, please look at 
    TeUpdateDBVersion.cpp for the routines: updateDB302To310 and PostgreSQLUpdateDB302To310,
    for a tip on how to upgrade by yourself.
    \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>    
 */	
class TLPOSTGRESQL_DLL TePostGIS : public TePostgreSQL
{
	friend class TePostGISPortal;

	public:
		
		//! Constructor
		TePostGIS();

		//! Destructor
		virtual ~TePostGIS()
		{
		}

		//! Creates a new database based on "template1" and open a connection to the new one
		bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port = -1, bool terralibModel=true, const std::string& characterSet = "");

		//! Opens a conection to a database server
		bool connect(const string& host, const string& user, const string& password, const string& database, int port = -1);

		bool showDatabases(const string& host, const string& user, const string& password, vector<string>& dbNames, int port = -1);

		//! Returns a portal associated to this database
		TeDatabasePortal* getPortal();

		//! Creates a generic table
		bool createTable(const string& table, TeAttributeList &attr);

		//! Generate the label position (x,y) to each object of a theme
		bool generateLabelPositions(TeTheme *theme, const std::string& objectId = "");

		//! Returns all polygons in a table given a criteria expressed as an SQL where statement
		bool selectPolygonSet(const string& table, const string& criteria, TePolygonSet& ps);

		//! Returns all polygons that represents objects of a particular theme
		bool loadPolygonSet(TeTheme* theme, TePolygonSet& ps);

		//! Returns all polygons  that represents objects of a particular geoid
		bool loadPolygonSet(const string& table, const string& geoid, TePolygonSet& ps);

		//! Returns all polygons inside a given box
		bool loadPolygonSet(const string& table, TeBox& box, TePolygonSet& ps);

		//! Returns a database portal to iterate over the polygons that are inside a given box
		TeDatabasePortal* loadPolygonSet(const string& table, TeBox& box);

		//! Returns the first polygon that contais a given coordinate
	    bool locatePolygon(const string& table, TeCoord2D& pt, TePolygon& polygon, const double& tol = 0.0);		

		//! Returns the polygons that contains a give coordinate
		bool locatePolygonSet  (const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons);

		//! Inserts a polygon
		bool insertPolygon(const string& table, TePolygon& p);

		//! Updates a polygon
		bool updatePolygon(const string& table, TePolygon& p);

		bool loadLineSet(const string& table, const string& geoid, TeLineSet& ls);

		bool loadLineSet(const string& table, TeBox& box, TeLineSet& linSet);

		TeDatabasePortal* loadLineSet(const string& table, TeBox& box);

		//! Inserts a line
		bool insertLine(const string& table, TeLine2D& l);

		//! Updates a line
		bool updateLine(const string& table, TeLine2D& l);

		//! Locates a line
		bool locateLine(const string& table, TeCoord2D& pt, TeLine2D& line, const double& tol = 0.0);

		//! Inserts a point
		bool insertPoint(const string& table, TePoint& p);

		//! Updates a point
		bool updatePoint(const string& table, TePoint& p);

		//! Locates a point
		bool locatePoint(const string& table, TeCoord2D& pt, TePoint& point, const double& tol = 0.0);

		//! Inserts a node
		bool insertNode(const string& table, TeNode& node);

		//! Updates a node
		bool updateNode(const string& table, TeNode& node);

		//! Inserts a cell
		bool insertCell(const string& table, TeCell& c);

		//! Updates a cell
		bool updateCell(const string& table, TeCell& c);

		//! Locates a cell
		bool locateCell(const string& table, TeCoord2D& pt, TeCell& c, const double& tol = 0.0);

		//! Removes a geometry from the given tableName
		virtual bool removeGeometry(const string& tableName, const TeGeomRep& rep, const int& geomId);

		//! Creates a spatial index on column table
		bool createSpatialIndex(const string& table, const string& column, TeSpatialIndexType type = TeRTREE, short level = 0, short tile = 0);

		//! Return a string that describes a where clause to return the geometries inside the box
		string getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName);

		//!< Returns a string SQL to be used in the ORDER BY clause when querying geometries.
		virtual std::string getSQLOrderBy(const TeGeomRep& rep) const;

		//! Returns a string that describes a where clause to return the geometries of the table2 that are inside the geometries box of the table1
		string getSQLBoxWhere(const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1 = TePOLYGONS);

		//! Return a string SQL to be used in the clause SELECT to select the box (lower_x, lower_y, upper_x, upper_y)
		string getSQLBoxSelect(const string& tableName, TeGeomRep rep);

		//! Returns the box of a specific geometry (object_id)  
		bool getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom);

		//! Return the box of a select objects set 
		bool getMBRSelectedObjects(string geomTable, string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType, TeBox& bout, const double& tol = 0.0);

		//! Returns the name of the column that wiil be the spatially indexed, for a given type of geometry table 
		string getSpatialIdxColumn(TeGeomRep rep);

		//! Retrives the list of available character sets
		bool getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList);
};


//! PostgreSQL with PostGIS extension data navigation functionalities.
/*!
	This class contains the implementation of common methods for TerraLib navigate throw data.    
 */	
class TLPOSTGRESQL_DLL TePostGISPortal : public TePostgreSQLPortal
{
	public:

		//! Constructor
		TePostGISPortal()
		{
		}

		//! Constructor
		TePostGISPortal(TeDatabase *pDatabase);
		
		//! Virtual Destructor
		virtual ~TePostGISPortal()
		{
		};	

		bool fetchGeometry(TePolygon& pol);
		bool fetchGeometry(TeLine2D& line);
		bool fetchGeometry(TeNode& n);
		bool fetchGeometry(TePoint& p);
		bool fetchGeometry(TeCell& cell);

		bool fetchGeometry(TePolygon& pol, const unsigned int& initIndex);
		bool fetchGeometry(TeLine2D& line, const unsigned int& initIndex);
		bool fetchGeometry(TeNode& n, const unsigned int& initIndex);
		bool fetchGeometry(TePoint& p, const unsigned int& initIndex);
		bool fetchGeometry(TeCell& cell, const unsigned int& initIndex);

	protected:

		TePointSet		_multiPointBuffer;
		TeLineSet		_multiLineBuffer;
		TePolygonSet	_multiPolygonBuffer;

};


/**
 * @brief This is the class for TePostGIS driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLPOSTGRESQL_DLL TePostGISFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TePostGISFactory() : TeDatabaseFactory( std::string( "PostGIS" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TePostGISFactory() {};
      
  protected :  
  
    /**
     * Implementation for the abstract TeFactory::build.
     *
     * @param arg A const reference to the parameters used by the
     * database.
     * @return A pointer to the new generated database instance.
     */
    TeDatabase* build( const TeDatabaseFactoryParams& arg )
    {
      TePostGIS* instance_ptr = new TePostGIS();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TePostGISFactory TePostGISFactory_instance;
};

#endif	// __TERRALIB_INTERNAL_POSTGIS_H

