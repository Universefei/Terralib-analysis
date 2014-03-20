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
/*!
 \file TePostgreSQL.h
 \brief This file contains the especifics routines of PostgreSQL.
*/

/*
 * TODO: 01. Implementar o alterTable que nao pode simplesmente alterar o tipo da coluna
 *		 02. No metodo query do portal, determinar os campos que fazem parte da chave primaria
 *
 */

#ifndef  __TERRALIB_INTERNAL_POSTGRESQL_H
#define  __TERRALIB_INTERNAL_POSTGRESQL_H

#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeDatabaseFactory.h"
#include "TePostgreSQLDefines.h"

class TePostgreSQLPortal;
class TePGConnection;
class TePGRecordset;

struct pg_conn;

//! PostgreSQL database access class.
/*!
	This class contains the implementation of common methods for TerraLib access PostgreSQL.   //
	\note MAKE SURE you have UPGRADE your PostgreSQL database before use this driver.
    If you need to upgrade without use TerraView, please look at 
    TeUpdateDBVersion.cpp for the routines: updateDB302To310 and PostgreSQLUpdateDB302To310,
    for a tip on how to upgrade by yourself.
    \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>    

 */	
class TLPOSTGRESQL_DLL TePostgreSQL : public TeDatabase
{
	friend class TePostgreSQLPortal;

	public:
		
		//! Constructor
		TePostgreSQL();

		//! Destructor
		virtual ~TePostgreSQL();

         /** @name Connection
	    *  Methods related with connections.
	    */
	    //@{	

        /*! \brief Sets a connection that will be used by database. */
        virtual void setConnection(TeConnection* c);

        /*! \brief Gets the connection used by database. 
            \note The caller of this method will take the ownership of the returned pointer.
        */
        virtual TeConnection* getConnection();

        //@}

		//! Returns the error message from the server
		string errorMessage();

		//! Creates a new database based on "template1" and open a connection to the new one
		virtual bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port = -1, bool terralibModel=true, const std::string& characterSet = "");

		//! Opens a conection to a database server
		virtual bool connect(const string& host, const string& user, const string& password, const string& database, int port = -1);

		virtual bool connect(pg_conn* conn);

		//! Show the server databases (only for MySQL, Oracle and PostgreSQL)
		virtual bool showDatabases(const string& host, const string& user, const string& password, vector<string>& dbNames, int port = -1);

		//! Closes the conection
		void close(void);

		bool listTables(vector<string>& tableList);

		//! Returns a lista of databases
		virtual bool listDatabases(std::vector<std::string> &databaseList);

		//! Drop a database
		virtual bool dropDatabase(const std::string &databaseName);

		//! Verifies if a table exist
		bool tableExist(const string& table);

		//! Deletes a table. Do not allow the deletion of model tables
		virtual bool deleteTable (const string& table);

		//! Verifies if a table has a column
	    bool columnExist(const string& table, const string& column, TeAttribute& attr);

		//! Creates a generic table
		virtual bool createTable(const string& table, TeAttributeList &attr);

		//! Adds an attribute field
		virtual bool addColumn(const string& table, TeAttributeRep &rep);

		//! Creates a reationship between two tables
		bool createRelation(const string& relName, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion);

		//! Checks if a relation exist
		TeDBRelationType existRelation(const string& tableName, const string& relName);

		//! Executes a SQL command that doesnt return a record set tipically a data definition comand
		bool execute(const string &sql);

		//! Returns a portal associated to this database
		virtual TeDatabasePortal* getPortal();

		//! Saves a table and its contents in the database
		bool insertTable(TeTable &table);

		//! Updates a table and its contents in the database
		bool updateTable	(TeTable &table);
		
		//! Inserts information about a link to an external table
		bool insertRelationInfo(const int tableId, const string& tField, const string& rTable, const string& rField, int& relId);

		//! Insert information about a table related to a layer
	    bool insertTableInfo(int layerId, TeTable& table, const string& user = "");

		//! Alter a property of a table 
		bool alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName="");

		//! Alter a table name
		bool alterTable(const string& oldTableName, const string& newTableName);

	    //! Saves a large binary objects (BLOB) in a row table 
	   	bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
		
		//! Insert information about a geographical projection
		bool insertProjection(TeProjection *proj);

		//! Insert information about a layer
		bool insertLayer(TeLayer *layer);

		//! Insert information about a project
		bool insertProject(TeProject *proj);

		//! Insert information about a geometrical representation
		bool insertRepresentation(int layerId, TeRepresentation& rep);

		//! Inserts information about a view
		bool insertView(TeView *view);

		//! Inserts view tree information
		bool insertViewTree(TeViewTree *tree);

		//! Inserts a theme group
		bool insertThemeGroup(TeViewTree* tree);

		//! Inserts theme information
		bool insertTheme(TeAbstractTheme *theme);

		//! Inserts information about a table used by a theme 
		bool insertThemeTable(int themeId, int tableId, int relationId, int tableOrder);

		//! Generate the label position (x,y) to each object of a theme
		virtual bool generateLabelPositions(TeTheme *theme, const std::string& objectId = "");

		//! Inserts legend information
		bool insertLegend(TeLegendEntry *legend);		

		//! Returns all polygons inside a given box
		virtual bool loadPolygonSet(const string& table, TeBox& box, TePolygonSet& ps);

		//! Returns a database portal to iterate over the polygons that are inside a given box
		virtual TeDatabasePortal* loadPolygonSet(const string& table, TeBox& box);

		//! Returns the first polygon that contais a given coordinate
	    virtual bool locatePolygon(const string& table, TeCoord2D& pt, TePolygon& polygon, const double& tol = 0.0);		

		//! Returns the polygons that contains a give coordinate
		virtual bool locatePolygonSet  (const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons);

		//! Inserts a polygon
		virtual bool insertPolygon(const string& table, TePolygon& p);

		//! Updates a polygon
		virtual bool updatePolygon(const string& table, TePolygon& p);

		virtual bool loadLineSet(const string& table, TeBox& box, TeLineSet& linSet);

		virtual TeDatabasePortal* loadLineSet(const string& table, TeBox& box);

		//! Inserts a line
		virtual bool insertLine(const string& table, TeLine2D& l);

		//! Updates a line
		virtual bool updateLine(const string& table, TeLine2D& l);

		//! Locates a line
		virtual bool locateLine(const string& table, TeCoord2D& pt, TeLine2D& line, const double& tol = 0.0);

		//! Inserts a point
		virtual bool insertPoint(const string& table, TePoint& p);

		virtual bool updatePoint(const string& table, TePoint& p);

		//! Locates a point
		virtual bool locatePoint(const string& table, TeCoord2D& pt, TePoint& point, const double& tol = 0.0);

		//! Inserts text
		bool insertText(const string& table, TeText& t);

		//! Inserts an arc
		bool insertArc(const string& table,TeArc& arc);

		//! Inserts a node
		virtual bool insertNode(const string& table, TeNode& node);
		virtual bool updateNode(const string& table, TeNode &node);

		//! Inserts a cell
		virtual bool insertCell(const string& table, TeCell& c);
		virtual bool updateCell(const string& table, TeCell &c);

		// Locates a cell
		virtual bool locateCell(const string& table, TeCoord2D& pt, TeCell& c, const double& tol = 0.0);

		//! Insert a raster block into the database
		virtual bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char* buf,unsigned long size, int band = 0, unsigned int res = 1, unsigned int subband = 0);

		//! Return a string SQL to calculate the statistics to each cell through database functions 
		string getSQLStatistics(TeGroupingAttr& attrs);

		//! Return a string SQL to temporal where 
		string getSQLTemporalWhere(TeTimeInterval& timeInterval, TeTemporalRelation timeOperator, const string& initialTime, const string& finalTime);

		//! Return a string SQL to temporal where
		string getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, const string& initialTime, const string& finalTime);

		//! Return the database function to generate autonumber values
		string getAutoNumberSQL(const string& table);

		//! Decodes a time structure into a time string that PostgreSQL can understand
		string getSQLTime(const TeTime& t) const;

		//! Concat values in a vector using unionString as the join between each value
	    string concatValues(vector<string>& values, const string& unionString);

	    //! Returns the SQL function for upper case
	    string toUpper(const string& value);

		//! Returns the SQL function for substring that starts from left to right with informed length.
        string leftString(const string& name, const int& length);

		//! Creates a spatial index on column table
		virtual bool createSpatialIndex(const string& table, const string& column, TeSpatialIndexType type = TeRTREE, short level = 0, short tile = 0);

		//! Return a string that describes a where clause to return the geometries inside the box
		virtual string getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName);

		//! Returns a string that describes a where clause to return the geometries of the table2 that are inside the geometries box of the table1
		virtual string getSQLBoxWhere(const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1 = TePOLYGONS);

		//! Return a string SQL to be used in the clause SELECT to select the box (lower_x, lower_y, upper_x, upper_y)
		virtual string getSQLBoxSelect(const string& tableName, TeGeomRep rep);

		//! Returns the box of a specific geometry (object_id)  
		virtual bool getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom);

		//! Return the box of a select objects set 
		virtual bool getMBRSelectedObjects(string geomTable, string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType, TeBox& bout, const double& tol = 0.0);

		//! Returns the name of the column that wiil be the spatially indexed, for a given type of geometry table 
		virtual string getSpatialIdxColumn(TeGeomRep rep);

		//! Begins a transaction
		bool beginTransaction(); 

		//! Commits a transaction
		bool commitTransaction();

		//! Rollbacks a transaction
		bool rollbackTransaction();

		//! Gets the client encoding
		virtual std::string getClientEncoding();

		//! Sets the client encoding
		virtual bool setClientEncoding(const std::string& characterSet);

		//! Retrives the list of available character sets
		virtual bool getEncodingList(std::vector<std::string>& vecEncodingList);
		 
		//! Retrives the list of available character sets
		virtual bool getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList);

		//! Return a TeDatabaseIndex vector from a table
		virtual bool getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes);

		//! Gets the list of attributes of a table
		virtual bool getAttributeList(const string& tableName,TeAttributeList& attList);

	protected:

		//! Returns a string with insert of a raster block
		virtual void getInsertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, const int& band, const unsigned int& res, const unsigned int& subband, char* buf, const unsigned long& size, string& sql);

		//! Returns a string with update of a raster block
		virtual void getUpdateRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, const int& band, const unsigned int& res, const unsigned int& subband, char* buf, const unsigned long& size, string& sql);
		
		//! Escape special characters in a string to be used in a SQL statement
		string escapeSequence(const string& from);
		

		//! Opens a conection to a database server
		bool realConnect(const string& host, const string& user, const string& password, const string& database, int port = -1);

		//! Connection to a PostgreSQL server
		TePGConnection*     tepg_connection_;
        TePGConnection*     tepg_connection_ref;

		//! Choose GIST OPERATOR beteween PostgreSQL 8.0 and 8.1
        std::string         gistBoxOps_;

		//! Keeps the number of opened transactions
		int                 transactionCounter_;

};


//! PostgreSQL data navigation functionalities.
/*!
	This class contains the implementation of common methods for TerraLib navigate throw data.    
 */	
class TLPOSTGRESQL_DLL TePostgreSQLPortal : public TeDatabasePortal
{
	public:

		//! Constructor
		TePostgreSQLPortal();

		//! Constructor
		TePostgreSQLPortal(TeDatabase *pDatabase);
		
		//! Virtual Destructor
		virtual ~TePostgreSQLPortal();

		//! Executes a SQL query that opens a record set
		virtual bool query(const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeUNIDIRECTIONAL, TeCursorEditType e = TeREADONLY, TeCursorDataType dt = TeTEXTCURSOR);

		//! Fetchs the next row in a record set that shouldve been previously opened
		bool fetchRow();

		//! Fetchs a particular row
		bool fetchRow(int i);

		//! Frees the current record set
		void freeResult();

		//! Gets the last error message
		string errorMessage();

		//! Gets the value of the i-th attribute as a literal
		char* getData(int i);

		//! Gets the value of a named attribute as a literal
		char* getData(const string& s);

		//! Gets the value of the i-th attribute a double
		double getDouble(int i);

		//! Gets the value of a named attribute as a double
		double getDouble(const string& s);

		//! Gets the value of the i-th attribute as an integer
		int	getInt(int i);

		//! Gets the value of a named attribute as an integer
		int	getInt(const string& s);

		//! Gets the value of a named attribute as a boolean
		bool getBool(const string& s);

		//! Gets the value of the i-th attribute as a boolean
		bool getBool(int i);

		//! Gets the value of the i-th attribute as a date 
	    TeTime getDate(int i);

	    //! Gets the value of a named attribute as a date 
	    TeTime getDate(const string& s);

		//! Gets the of a date/time attribute as a string formatted as accepted in further SQL statements
		string getDateAsString(int i);

		//! Gets the of a date/time attribute as a string formatted as accepted in further SQL statements
		string getDateAsString(const string& s);

		//! Gets the value of a named BLOB attribute 
		bool getBlob(const string& s, unsigned char*& data, long& size);

		virtual bool getRasterBlock(unsigned long& size, unsigned char* ptData);

		virtual bool fetchGeometry(TePolygon& pol);
		virtual bool fetchGeometry(TeLine2D& line);
		virtual bool fetchGeometry(TeNode& n);
		virtual bool fetchGeometry(TePoint& p);
		virtual	bool fetchGeometry (TeCell& cell);
		virtual bool fetchGeometry(TePolygon& pol, const unsigned int& initIndex);
		virtual bool fetchGeometry(TeLine2D& line, const unsigned int& initIndex);
		virtual bool fetchGeometry(TeNode& n, const unsigned int& initIndex);
		virtual bool fetchGeometry(TePoint& p, const unsigned int& initIndex);
		virtual	bool fetchGeometry (TeCell& cell, const unsigned int& initIndex);


	protected:

		//! Return a linear ring and the number of holes in case of external ring
		TeLinearRing getLinearRing(int& numberOfHoles);

		//! Return a linear ring that begins in the portal i-th position and the number of holes in case of external ring
		TeLinearRing getLinearRing(int& numberOfHoles, const unsigned int& i);

		//! Escape special characters in a string to be used in a SQL statement
		string escapeSequence(const string& from);

		// Protected data members
		TePGConnection *con_;
		TePGRecordset* tepg_recordset_;

		long curRow_;		

};


/**
 * @brief This is the class for TePostgreSQL driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLPOSTGRESQL_DLL TePostgreSQLFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TePostgreSQLFactory() : TeDatabaseFactory( 
      std::string( "PostgreSQL" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TePostgreSQLFactory() {};
      
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
      TePostgreSQL* instance_ptr = new TePostgreSQL();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TePostgreSQLFactory TePostgreSQLFactory_instance;
}; 

#endif	// __TERRALIB_INTERNAL_TePOSTGRESQL_H

/*
 *  Updates:
 *      - 2007/03/26: Mario Rocco Added new method - string leftString(const string& name, const int& length);
 */

