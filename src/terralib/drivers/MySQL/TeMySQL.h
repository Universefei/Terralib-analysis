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
/*! \file TeMySQL.h
    \brief This file contains the particularities of a MySQL driver
*/
#ifndef  __TERRALIB_INTERNAL_MYSQL_H
#define  __TERRALIB_INTERNAL_MYSQL_H

#include "TeMySQLDefines.h"

#include "../../kernel/TeTable.h"
#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeDatabaseFactory.h"

#include "../../kernel/TeDatabaseIndex.h"

#ifdef WIN32
#include <winsock.h>
#endif

//#include <mysql.h>

//Forward declarations
struct st_mysql;
struct st_mysql_res;
//typedef char** MYSQL;

/******************************************************************************
 *                                                                            *
 *                                 TeMySQL
 *                                                                            *
 *****************************************************************************/

//! A concrete implementation of a driver to the MySQL SGDB
class TLMYSQL_DLL TeMySQL : public TeDatabase
{
    friend class TeMySQLPortal;

public: 

	TeMySQL();

	~TeMySQL();

/*---------------------------------------------------------------------------*/
/// Database connection related.

	st_mysql*	getMySQL () 
    {	return mysql_;	}

	string  escapeSequence(const string& from);

	bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true, const std::string& characterSet = "");

	bool connect (const string& host, const string& user, const string& password, const string& database, int port=-1);

	bool showDatabases (const string& host, const string& user, const string& password, vector<string>& dbNames, int port=-1);

	void close();

/*---------------------------------------------------------------------------*/
/// Table manipulation related.

    unsigned int maxFieldsPerRelation() { return 4096; }

	//! Returns a lista of databases
	virtual bool listDatabases(std::vector<std::string> & databaseList);

	//! Drop a database
	virtual bool dropDatabase(const std::string & databaseName);

	bool tableExist(const string& table);

	bool columnExist(const string& table, const string& column, TeAttribute& attr);

	bool addColumn (const string& table,TeAttributeRep &rep);

	//! Delete the index from tables in the conceptual model
	virtual bool deleteIndex(const string& tableName, const string& indexName);

	//! Return a TeDatabaseIndex vector from a table
	virtual bool getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes);

	//! Alter a property of a table 
	virtual bool alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName="");

	bool defineIntegrity(void);

	bool createRelation (const string& /* relName */, const string& /* table */, const string& /* fieldName */,
                         const string& /* relatedTable */, const string& /* relatedField */, bool /* cascadeDeletion */)
	{ return true; }

	TeDBRelationType existRelation(const string& /* tableName */, const string& /* relName */)

	{	return TeNoRelation; }

	bool execute ( const string &q);

	bool createTable(const string& table, TeAttributeList &attr);

	TeDatabasePortal* getPortal ();

    string	errorMessage ();
    //    {	return string(mysql_error(mysql_));	}

    int	errorNum ();
    //    {	return mysql_errno(mysql_);	}
	
/*---------------------------------------------------------------------------*/
/// specific TerraLib database access methods

	bool insertProjection (TeProjection *proj);
	bool insertLayer(TeLayer* layer);
	bool insertProject(TeProject* project);
	bool insertRepresentation(int layerId, TeRepresentation& rep);	
	bool insertTable(TeTable &table);
	bool updateTable	(TeTable &table);
	bool insertTableInfo (int layerId, TeTable &table, const string& user="");
	bool insertRelationInfo(const int tableId, const string& tField,
						    const string& eTable, const string& eField, int& relId);
	bool insertView (TeView *view);
	bool insertViewTree (TeViewTree *tree);	
	bool insertTheme(TeAbstractTheme *theme);
	bool insertThemeGroup (TeViewTree* tree);
	bool generateLabelPositions (TeTheme *theme, const std::string& objectId = "");
	bool insertThemeTable (int themeId, int tableId, int relationId, int tableOrder);
	bool insertLegend (TeLegendEntry *legend);

/*---------------------------------------------------------------------------*/
/// Geometry of features.

	bool insertPolygon	(const string& table, TePolygon &p);
	bool insertPolygonSet(const string& table, TePolygonSet &ps);
	bool updatePolygon	(const string& table, TePolygon &p);
	bool insertLineSet	(const string& table, TeLineSet &ls);	
	bool insertLine (const string&  table, TeLine2D &l);
	bool updateLine (const string& table, TeLine2D &l);
	bool insertPoint (const string& table, TePoint &p);
	bool insertText (const string& table, TeText &t);
	bool insertArc (const string& table,TeArc &arc);
	bool insertNode	(const string& table, TeNode &node);	
	bool insertCell	(const string& table, TeCell &c);
	int  insertId();
	

	bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf,unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);

	string getSQLTime(const TeTime& time) const;
	string getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, 
                               const string& initialTime, const string& finalTime);

	bool inClauseValues(const string& query, const string& attribute, vector<string>& inClauseVector);

	bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
	
	//! Concat values in a vector using unionString as the join between each value
	string concatValues(vector<string>& values, const string& unionString);

	//! Returns the SQL function for upper case
	string toUpper(const string& value);

	string getConcatFieldsExpression(const vector<string>& fNamesVec);

/*---------------------------------------------------------------------------*/
/// transaction related.

	bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

/*---------------------------------------------------------------------------*/
/// encoding related.

	//! Gets the client encoding
	virtual std::string getClientEncoding();

	//! Sets the client encoding
	virtual bool setClientEncoding(const std::string& characterSet);

	//! Retrives the list of available character sets
	virtual bool getEncodingList(std::vector<std::string>& vecEncodingList);

	//! Retrives the list of available character sets
	virtual bool getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList);

private :

    st_mysql*	mysql_;
    char*       bufferBlob_;	 // internal buffer used to write blob data
    long    	bufferBlobSize_; // size of the data already allocated

};

/******************************************************************************
 *                                                                            *
 *                           TeMySQLPortal
 *                                                                            *
 *****************************************************************************/

//! A concrete implementation of a portal to a MySQL database
class TLMYSQL_DLL TeMySQLPortal  : public TeDatabasePortal
{
private:

	st_mysql			*mysql_;
	st_mysql_res*		result_;
    char**		row_;

public :

    // Constructor / Destructor

    TeMySQLPortal ( TeMySQL *m);

    TeMySQLPortal ( const TeMySQLPortal& p);

    ~TeMySQLPortal ();

    // Basic database methods

    int  insertId();

    bool query (const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeUNIDIRECTIONAL, TeCursorEditType e = TeREADONLY, TeCursorDataType dt = TeTEXTCURSOR );

    bool fetchRow ();

    bool fetchRow (int i);

    void freeResult ();

    string	errorMessage ();

    //{ return string(mysql_error(mysql_));} //: last error message()

    int	errorNum ();

    //{ return mysql_errno(mysql_);}

    // specific SQL SELECT command methods

    char*		getData (int i);
    char*		getData (const string& s);
    double		getDouble (int i);
    double		getDouble (const string& s);
    int			getInt (int i);
    int			getInt (const string& s);
    bool		getBool (const string& s);
    bool		getBool (int i);

    bool fetchGeometry(TePolygon& pol);
    bool fetchGeometry(TeLine2D& line);
    bool fetchGeometry(TeNode& n);
    bool fetchGeometry(TePoint& p);
    bool fetchGeometry(TePolygon& pol, const unsigned int& initIndex);
    bool fetchGeometry(TeLine2D& line, const unsigned int& initIndex);
    bool fetchGeometry(TeNode& n, const unsigned int& initIndex);
    bool fetchGeometry(TePoint& p, const unsigned int& initIndex);

    bool setData (int /* field */, string /* val */)
    {return true; }
    st_mysql_res* result()
    {return result_; }

    bool getBlob (const string& s, unsigned char* &data, long& size);
    bool getRasterBlock(unsigned long& size, unsigned char* ptData);

    TeTime getDate (int i);
    TeTime getDate (const string& s);

    string getDateAsString(int i);
    string getDateAsString(const string& s);

protected:
    TeLinearRing getLinearRing (int &ni);
};

/******************************************************************************
 *                                                                            *
 *                              TeMySQLFactory
 *                                                                            *
 *****************************************************************************/

/**
 * @brief This is the class for TeMySQL driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLMYSQL_DLL TeMySQLFactory : public TeDatabaseFactory
{
public :

    /**
     * Default constructor
     */
    TeMySQLFactory() : TeDatabaseFactory( std::string( "MySQL" ) ) {};

    /**
     * Default Destructor
     */
    ~TeMySQLFactory() {};

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
      TeMySQL* instance_ptr = new TeMySQL();
      
      if( arg.host_ != "" ) {
          instance_ptr->connect( arg.host_, arg.user_, arg.password_,
                                 arg.database_, arg.port_ );
      }

      return (TeDatabase*)instance_ptr;
  }
};

/*---------------------------------------------------------------------------*/
/*-----  --------------------------------------------------------------*/
namespace {
    static TeMySQLFactory TeMySQLFactory_instance;
}; 

#endif

