/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright ? 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeOCIOracle.h
    \brief This file contains two classes that represent a driver of interface between TerraLib and ORACLE DBMS, using OCI (Oracle Call Interface) library.
*/

#ifndef  __TERRALIB_INTERNAL_ORACLEOCI_H
#define  __TERRALIB_INTERNAL_ORACLEOCI_H

//#include "TeOCIConnect.h"
//#include "TeOCICursor.h"
#include <TeDatabase.h>
#include <TeDatabaseFactory.h>

#include <TeDatabaseIndex.h>

#include "TeOracleDefines.h"


class TeOCIConnect;
class TeOCICursor;
class TeOCIConnection;

#ifdef AFX_DLL
#define EXPORT_WIN __declspec( dllexport )
#else
#define EXPORT_WIN
#endif

/*! \class TeOCIOracle
	\brief A concrete implementation of a interface driver to the ORACLE DBMS

	This class contains attributes and methods to implement a driver of interface 
	between TerraLib and ORACLE DBMS using OCI (Oracle Call Interface) library.
	The class TeOCIOracle implements the abstract interface defined by the class
	TeDatabase.

	\sa 
	TeDatabase TeOCIConnect
*/
class TLORACLE_DLL TeOCIOracle : public TeDatabase
{

friend class TeOCIOraclePortal;

protected:

	TeOCIConnection*	connection_;	//!< pointer to a opened connection to ORACLE database server  
	long				sequenceCont_;	//!< sequential number used to improve the data insertion
	string				sequenceName_;  //!< sequence name used to improve the data insertion

	//! Creates a internal sequence structure in the Oracle DBMS 
	bool	createSequence(const string &seqName);

	//! Creates a internal trigger structure in the Oracle DBMS
	bool	createAutoIncrementTrigger(const string &tableName, const string &fieldName);
	
	//! Returns the sequence name created to a specific table 
	string	getNameSequence(const string &tableName);

	//! Returns the trigger name created to a specific table 
	string  getNameTrigger(const string &tableName);

public:
	
	//! Constructor
	TeOCIOracle();

	//! Destructor
	~TeOCIOracle();

	//! Escapes special characters in a string to be used in a SQL statement
	string  escapeSequence(const string& from); 

	//! Begins a transaction
	bool beginTransaction(); 

	//! Commits a transaction
	bool commitTransaction();

	//! Rollbacks a transaction
	bool rollbackTransaction();
	  
	//! Connects to a Oracle DBMS and creates the TerraLib conceptual data model
	virtual bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int& port=0, bool terralibModel=true, const std::string& characterSet = ""); 

	//! Connects to a Oracle DBMS
	virtual bool connect (const string& host, const string& user, const string& password, const string& database, int port = 0);
	
	//! Lists all tables in the database
	bool listTables(vector<string>& tableList);

	//! Closes the connection to a Oracle DBMS
	void  close();

	//! Verify if there is a specific table in the database
	bool tableExist(const string& table);

	//! Verify if there is a specific view in the database
	bool viewExist(const string& view);

	//! Verify if there is a specific column in a table 
	bool columnExist(const string& table, const string& column, TeAttribute& attr);

	//! Creates a new table
	virtual bool createTable (const string& table, TeAttributeList &attr);

	//! Adds a new column in a table
	bool addColumn (const string& table,TeAttributeRep &rep);

	//! Deletes a specific table
	bool deleteTable (const string& table);

	//! Executes a SQL statement that does not return rows (INSERT, UPDATE, CREATE, etc.)
	bool execute (const string &sql);
	
	//! Gets a cursor or record set using the opened connection 
	virtual TeDatabasePortal* getPortal();

	//! Verify if there is a relation associated to a specific table
	TeDBRelationType existRelation(const string& tableName, const string& relName);

	//! Creates a new relation between two tables
	bool createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion);
		
	//! Return a TeDatabaseIndex vector from a table
	virtual bool getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes);


	/** @name Inserts, updates and deletes geometries from the tables
	*/
	//@{ 	
	bool insertTableInfo (int layerId, TeTable &table, const string& user="");
	bool insertRelationInfo(const int tableId, const string& tField, const string& rName, const string& rField, int& relId);
	bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName);
	bool alterTable (const string& oldTableName, const string& newTablename);
	bool insertTable	(TeTable &table);
	bool updateTable	(TeTable &table);
	bool insertProjection (TeProjection *proj);
	bool insertRepresentation (int layerId, TeRepresentation& rep);
	bool insertLegend	(TeLegendEntry *legend);	
	bool insertView		(TeView *view);	
	bool insertViewTree (TeViewTree *tree);	
	bool insertTheme		(TeAbstractTheme *theme);
	bool insertThemeTable	(int themeId, int tableId, int relationId, int tableOrder);
	bool insertThemeGroup	(TeViewTree* tree);
	bool insertLayer	(TeLayer *layer);	
	bool insertProject	(TeProject *project);	
	bool deleteLayer	(int layerId);
	//@}

	//! Generates label positions to each object of a theme
	virtual bool generateLabelPositions	(TeTheme *theme, const std::string& objectId = ""); 

	/** @name Inserts, updates and deletes geometry tables
	*/
	//@{ 	
	bool getValueSequence(const string& table);
	virtual bool insertPolygonSet(const string& table, TePolygonSet &ps);
	virtual bool insertPolygon		(const string& table, TePolygon &p);	
	virtual bool updatePolygon		(const string& table, TePolygon &p);
	virtual bool insertLineSet(const string& table, TeLineSet &ls);
	virtual bool insertLine (const string& table, TeLine2D &l);	
	virtual bool updateLine		(const string& table, TeLine2D &l);
	virtual bool insertPoint	(const string& table, TePoint &p);	
	virtual bool insertText		(const string& table, TeText &t);
	virtual bool insertArc	(const string& table,TeArc &arc);
	virtual bool insertNode		(const string& table, TeNode &node);	
	virtual bool insertCell		(const string& table, TeCell &c);
	virtual bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf,unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);
	//@}

	/** @name Methods to deal with large binary objects (BLOB)
	*/
	//@{ 	 
	bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
	//@}

	/** @name Methods that return specific SQL statement according to each DBMS
	*/
	//@{ 
	string getSQLStatistics (TeGroupingAttr& attrs);
	string  getSQLAutoNumber(const string& table);
	string  getSQLTime(const TeTime& time) const;
	string toUpper(const string& value);
	//@}

	//! Concats values in a vector using unionString as the join between each value
	string concatValues(vector<string>& values, const string& unionString);

	//! Gets the list of attributes of a table
	virtual bool getAttributeList(const string& tableName,TeAttributeList& attList);
};


/*!	\class TeOCIOraclePortal
	\brief A class that implements a concept of record set to a ORACLE DBMS

	This class is part of the driver of interface 
	between TerraLib and ORACLE DBMS. It was developed using OCI (Oracle Call Interface) library.
	The class TeOCIOraclePortal implements the abstract interface defined by the class
	TeDatabasePortal.

	\sa 
	TeDatabasePortal TeOCICursor
*/
class TLORACLE_DLL TeOCIOraclePortal : public TeDatabasePortal
{
protected:
	TeOCICursor		*cursor_;	//!< pointer to a record set  
	long			curRow_;	//!< current row index in the record set

    //! Verify if there is a connection associated to this cursor
	bool isConnected();		
    
	//! Verify if the current row is the last of the record set
	bool isEOF();

	//! Gets a linear ring from blob type
	bool getLinearRing (TeLine2D& line);
	//! Gets a linear ring that begins in the i-th portal position from blob type
	bool getLinearRing (TeLine2D& line, const unsigned int& i);

public:

	//! Constructor
	TeOCIOraclePortal(TeOCIOracle *pDatabase);
	
	//! Destructor
	virtual ~TeOCIOraclePortal();

	//! Returns a pointer to the record set
	TeOCICursor* getCursor() { return cursor_; }

    //! Moves to the first row in the record set
	bool moveFirst();

	//! Moves to the next row in the record set
    bool moveNext();

	//! Executes a SQL statement that return rows or records (SELECT ...)
	bool query ( const string &q,TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeUNIDIRECTIONAL, TeCursorEditType e = TeREADONLY, TeCursorDataType dt = TeTEXTCURSOR );

	//! Fetchs to next row
	bool fetchRow ();

	//! Fetchs to i-th row
	bool fetchRow (int i);

	//! Clear all memory located by the record set
	void freeResult();

	/** @name Methods that get values from record set
	*/
	//@{ 
	char* getData (int i);
	char* getData (const string& s);
	double getDouble (int i);
	double getDouble (const string& s);
	int	getInt (int i);
	int	getInt (const string& s);
	bool getBool (const string& s);
	bool getBool (int i);
	bool getBlob(const string& s, unsigned char* &data, long& size);
	TeTime getDate (int i);
	TeTime getDate (const string& s);
	string getDateAsString(int i);
	string getDateAsString(const string& s);
	//@}

	/** @name Methods that return a geometry from the record set current row and fetch to next row
	*/
	//@{ 
	virtual bool fetchGeometry (TePolygon& poly);
	virtual bool fetchGeometry (TeLine2D& line);
	virtual bool fetchGeometry (TePoint& p);
    virtual bool fetchGeometry (TeNode& n);
	virtual bool fetchGeometry (TePolygon& poly, const unsigned int& initIndex);
	virtual bool fetchGeometry (TeLine2D& line, const unsigned int& initIndex);
	virtual bool fetchGeometry (TePoint& p, const unsigned int& initIndex);
    virtual bool fetchGeometry (TeNode& n, const unsigned int& initIndex);
	//@}

	//! Gets a raster geometry block 
	bool getRasterBlock(unsigned long& size, unsigned char*);
};


/**
 * @brief This is the class for TeOCIOracle driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLORACLE_DLL TeOCIOracleFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeOCIOracleFactory() : TeDatabaseFactory( std::string( "OracleOCI" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeOCIOracleFactory() {};
      
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
      TeOCIOracle* instance_ptr = new TeOCIOracle();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeOCIOracleFactory TeOCIOracleFactory_instance;
}; 

#endif 



