/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright 2001-2007 INPE and Tecgraf/PUC-Rio.

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

#ifndef  __TERRALIB_INTERNAL_SQLITE_H
#define  __TERRALIB_INTERNAL_SQLITE_H

#include "TeSQLiteDefines.h"
#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeDatabaseFactory.h"

class TeSQLitePortal;
struct sqlite3;
struct sqlite3_stmt;

//! SQLite database access class.
/*!
	This class contains the implementation of common methods for TerraLib access SQLite.   //
    \author Mario Rocco Pettinati <mario@funcate.org.br>    

 */	
class TLSQLITE_DLL TeSQLite : public TeDatabase
{
	friend class TeSQLitePortal;

	public:
		
		//! Constructor
		TeSQLite();

		//! Destructor
		virtual ~TeSQLite();

		/** @name Database
		*  Methods related to database and connection creation. 
		*  These methods return TRUE when the operation was successfull.
		*  Otherwise return FALSE and when possible an error message is captured.
		*/
		//@{	
		//! Creates a new database and open a connection to it
		virtual bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true, const std::string& characterSet = "");

		//! Opens a conection to a database server
		virtual bool connect(const string& host, const string& user, const string& password, const string& database, int port = -1);

		//! Closes the conection
		virtual void close(void);

		//! Verifies if a table exist in the database
		virtual bool tableExist(const string& table);

		//! Verifies if a table has a column
		virtual bool columnExist(const string& table, const string& column, TeAttribute& attr);

		//! Creates a table
		/*!
		  \param table table name
		  \param attr table list of attributes
		 */
		virtual bool createTable(const string& table, TeAttributeList &attr);

		//! Adds a column to a table
		/*!
		  \param table table name
		  \param rep representation of the column being created
		*/
		virtual bool addColumn (const string& table, TeAttributeRep &rep);
		
		//! Alter a property of a table 
		virtual bool alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName="");

		//! Alter the table name 
		virtual bool alterTable(const string& oldTableName, const string& newTablename);

		//! Return a TeDatabaseIndex vector from a table
		virtual bool getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes);


		//! Creates a reationship between two tables
		/*!
		  \param relName relationship name
		  \param table table that will receive the foreign key
		  \param fieldName column that will be foreign key
		  \param relatedTable table that exports the foreign key
		  \param relatedField field that will the exported foreign key
		  \param cascadeDeletion flag that indicates if the deletion should be propagated
		*/
		virtual bool createRelation (const string& relName, const string& table, const string& fieldName, 
							const string& relatedTable, const string& relatedField, bool cascadeDeletion);

		//! Checks if a relation exist
		/*!
		  \param tableName table where the relashionship exists
		  \param relName relationship name
		*/
		virtual TeDBRelationType existRelation(const string& tableName, const string& relName);

		//! Insert information about a project
		virtual bool insertProject(TeProject *project);

		//! Inserts information about a link to an external table
		/*
			\param tableId table identification (from te_layer_table)
			\param tField link column name of the static table
			\param eTable name of the external table
			\param eField name of the link column of the related table
			\param relId returns the identification of the relation
		*/
		virtual bool insertRelationInfo(const int tableId, const string& tField, const string& rTable, const string& rField, int& relId);

		//! Inserts information about an attribute table
		virtual bool insertTableInfo(int layerId, TeTable &table, const string& user="");

		//! Saves a large binary objects (BLOB) in a row table 
		virtual bool insertBlob(const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);		

		//@}

		/** @name Projection
		*  Accessing/Inserting/Modifying projection information into the database. 
		*/
		//@{ 	

		//! Insert information about a geographical projection
		virtual bool insertProjection (TeProjection *proj);
		//@}

		/** @name Layers
		*  Retrieving/Inserting/Modifying/Deleting layers in the database. 
		*/
		//@{ 
		//! Insert information about a layer
		virtual bool insertLayer(TeLayer *layer);
		//@}

		/** @name Representation
		*  Retrieving/Inserting/Modifying/Deleting representations in the database. 
		*/
		//@{ 
		//! Inserts information about a geometrical representation
		virtual bool insertRepresentation(int layerId, TeRepresentation& rep);
		//@}

		/** @name Views
		*  Retrieving/Inserting/Modifying/Deleting views in the database. 
		*/
		//@{ 
		//! Inserts a view
		virtual bool insertView (TeView *view);

		//! Recursive inserting of a view tree.
		virtual bool insertViewTree(TeViewTree *tree);
		//@}

		/** @name Themes
		*  Retrieving/Inserting/Modifying/Deleting themes and group of themes in the database. 
		*/
		//@{ 
		//! Inserts a group of themes in the database
		virtual bool insertThemeGroup(TeViewTree* tree);

		//! Generates the label position (x,y) to each object of a theme or of a particular object
		virtual bool generateLabelPositions(TeTheme *theme, const std::string& objectId = "");

		//! Inserts an abstract theme in the database.
		virtual bool insertTheme(TeAbstractTheme *theme);

		//! Inserts information about a table used by a theme 
		virtual bool insertThemeTable(int themeId, int tableId, int relationId, int tableOrder);
		//@}

		/** @name Legend
		*  Retrieving/Inserting/Modifying/Deleting legends in the database. 
		*/
		//@{ 
		//! Inserts legend in the database
		virtual bool insertLegend(TeLegendEntry *legend);
		//@}

		/** @name Polygon
		// Accessing/Inserting/Modifying polygon geometries into the database
		*/
		//@{ 
		//! Inserts a polygon in a geometry table
		virtual bool insertPolygon(const string& table, TePolygon &p);

		//! Updates a polygon in a geometry table
		virtual bool updatePolygon(const string& table, TePolygon &p);
		//@}

		/** @name Line
		*  Accessing/Inserting/Modifying Line geometries into the database. 
		*/
		//@{
		//! Inserts a line in a geometry table
		virtual bool insertLine(const string& table, TeLine2D &l);

		//! Updates a line in a geometry table
		virtual bool updateLine(const string& table, TeLine2D &l);
		//@}

		/** @name Point
		*  Accessing/Inserting/Modifying Point geometries into the database. 
		*/
		//@{
		virtual bool insertPoint(const string& table, TePoint &p);
		//@}

		/** @name Text
		*  Accessing/Inserting/Modifying Text geometries into the database. 
		*/
		//@{
		virtual bool insertText(const string& table, TeText &t);
		//@}

		/** @name Arc
		*  Accessing/Inserting/Modifying Arc geometries  into the database. 
		*/
		//@{
		//! Inserts an arc geometry in the database.
		virtual bool insertArc(const string& table,TeArc &arc);
		//@}

		/** @name Node
		*  Accessing/Inserting/Modifying Node geometries into the database. 
		*/
		//@{
		virtual bool insertNode(const string& table, TeNode &node);
		//@}

		/** @name Cell
		*  Accessing/Inserting/Modifying Cell geometries  into the database. 
		*/
		//@{
		virtual bool insertCell(const string& table, TeCell &c);
		//@}

		//! Insert a raster block into the database
		/*!
		  \param table table name
		  \param blockId block unique identifier
		  \param ll  block lower left coordinate
		  \param ur  block upper right coordinate
		  \param buf block binary data
		  \param size block size
		  \param band block band
		  \param res block resolution factor
		  \param subband sub band definitiion
		*/
		virtual	bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf, unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);

		/** @name Query
		*  Methods related to query the database
		*/
		//@{	
		//! Executes a SQL command that doesnt return a record set. Tipically a data definition comand
		virtual bool execute ( const string &sql);

		//! Begins a transaction
		bool beginTransaction(); 

		//! Commits a transaction
		bool commitTransaction();

		//! Rollbacks a transaction
		bool rollbackTransaction();

		//! Returns a database portal.
		/*!
			A database portal is used to submit queries to the database and to navigate over the
			resulting record set
		*/
		virtual TeDatabasePortal* getPortal ();

		//@}

		/** @name specifics SQLs
		*  return SQL strings 
		*/
		//@{ 
		
		//! Escape special characters in a string to be used in a SQL statement
		virtual string escapeSequence(const string& from);

		//! Gets the list of attributes of a table
		virtual bool getAttributeList(const string& tableName,TeAttributeList& attList);

		//! Concat values in a vector using unionString as the join between each value
		virtual string concatValues(vector<string>& values, const string& unionString);

		//! Returns the SQL function for upper case
		virtual string toUpper(const string& value);

		//@}

		//! Returns a valid SQL time string
		virtual string getSQLTime(const TeTime& time) const;

		//!< Returns a string SQL to be used in the ORDER BY clause when querying geometries.
		virtual std::string getSQLOrderBy(const TeGeomRep& rep) const;

		//! Returns the error message from the server
		virtual string errorMessage();

		int getLastInsertedSerial();


	protected:

		sqlite3*	_conn;
		int			_transactionCounter;		
};

class TLSQLITE_DLL TeSQLitePortal : public TeDatabasePortal
{
	public:

		//! Constructor
		TeSQLitePortal();

		//! Constructor
		TeSQLitePortal(TeDatabase* pDatabase);
		
		//! Virtual Destructor
		virtual ~TeSQLitePortal();

		virtual bool query ( const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeUNIDIRECTIONAL, TeCursorEditType e = TeREADONLY, TeCursorDataType dt = TeTEXTCURSOR );

		//! Fetchs the next row in a record set that shouldve been previously opened
		virtual bool fetchRow ();

		//! Fetchs a particular row
		virtual bool fetchRow (int i);

		//! Frees the current record set
		virtual void freeResult ();

		//! Gets the value of the i-th attribute as a literal
		virtual char* getData (int i);

		//! Gets the value of a named attribute as a literal
		virtual char* getData (const string& s);

		//! Gets the value of the i-th attribute as an integer
		virtual int getInt (int i);

		//! Gets the value of a named attribute as an integer
		virtual int getInt (const string& s);

		//! Gets the value of a named attribute as a boolean
		virtual bool getBool (const string& s);

		//! Gets the value of the i-th attribute as a boolean
		virtual bool getBool (int i);

		//! Gets the value of the i-th attribute as a date 
		virtual TeTime getDate (int i);

		//! Gets the value of a named attribute as a date 
		virtual TeTime getDate (const string& s);

		//! Gets the of a date/time attribute as a string formatted as accepted in further SQL statements
		virtual string getDateAsString(int i);

		//! Gets the of a date/time attribute as a string formatted as accepted in further SQL statements
		virtual string getDateAsString(const string& s);

		//! Gets the value of a named BLOB attribute 
		virtual bool getBlob(const string& s, unsigned char* &data, long& size);

		virtual bool getRasterBlock(unsigned long& size, unsigned char* ptData);

		//! Gets the index of a named attribute
		int getColumnIndex (const string& s);

		/** @name Fetch Geometry
		  The following methods decodify geometries as stored 
		  in the database according to the data model proposed in TerraLib.
		  \param geom		the geometry that will be filled from portal
		  \param initIndex  the position index in the portal where begins the geometry information 
		  \return The fetchGeometry methods advance the portal to the next record and 
		  they return TRUE if there are more records to be read and FALSE otherwise.
		*/
		//@{ 	
		virtual	bool fetchGeometry(TePolygon& geom);
		virtual	bool fetchGeometry(TePolygon& geom, const unsigned int& initIndex);
		virtual	bool fetchGeometry(TeLine2D& geom);
		virtual	bool fetchGeometry(TeLine2D& geom, const unsigned int& initIndex);
		virtual	bool fetchGeometry(TeNode& geom);
		virtual	bool fetchGeometry(TeNode& geom, const unsigned int& initIndex);
		virtual	bool fetchGeometry(TePoint& geom);
		virtual	bool fetchGeometry(TePoint& geom, const unsigned int& initIndex);

		//@}

		virtual string errorMessage();

	protected:

		sqlite3*		_conn;
		sqlite3_stmt*	_recordSet;
		bool			_skipFirstFetch;
		bool			_firstFetchResult;
		int				_currentRow;
		std::string			_query;

		std::map<std::string, int>		_mapColumnNames;
};

/**
 * @brief This is the class for TeSQLite driver factory.
 * @author Mario Rocco Pettinati <mario@funcate.org.br>
 * @ingroup DatabaseUtils
 */
class TLSQLITE_DLL TeSQLiteFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeSQLiteFactory() : TeDatabaseFactory( 
      std::string( "SQLite" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeSQLiteFactory() {};
      
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
		TeSQLite* instance_ptr = new TeSQLite();
      
		//if( arg.host_ != "" )
		//{
			instance_ptr->connect( arg.host_, arg.user_, arg.password_, arg.database_, arg.port_ );
		//}
  
		return (TeDatabase*)instance_ptr;
    }
};

namespace 
{
	static TeSQLiteFactory TeSQLiteFactory_instance;
};

#endif //__TERRALIB_INTERNAL_SQLITE_H
