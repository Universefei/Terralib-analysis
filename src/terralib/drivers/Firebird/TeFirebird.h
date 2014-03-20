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
/*! \file TeFirebird.h
    \brief This file contains the particularities of a Firebird driver
*/
#ifndef  __TERRALIB_INTERNAL_FIREBIRD_H
#define  __TERRALIB_INTERNAL_FIREBIRD_H

#ifdef WIN32
#include <winsock.h>
#endif

#include <TeTable.h>
#include <TeDatabase.h>
#include <TeDatabaseFactory.h>

#ifdef IBPP_WINDOWS
#include <windows.h>
#endif

#include <ibpp/core/ibpp.h>

#include "TeFirebirdDefines.h"
#include "TeFirebirdConnection.h"

#ifdef IBPP_UNIX
#include <unistd.h>
#define DeleteFile(x) unlink(x)
#define Sleep(x) usleep(1000 * x)
#endif

#ifdef HAS_HDRSTOP
#pragma hdrstop
#endif

//! A concrete implementation of a driver to the Firebird SGDB
class TLFIREBIRD_DLL TeFirebird : public TeDatabase
{
friend class TeFirebirdPortal;

public: 

	TeFirebird();

	virtual ~TeFirebird();

	IBPP::Database getIBPPFirebird();

	string  escapeSequence(const string& from);

	bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true, const std::string& characterSet = "");

	bool connect (const string& host, const string& user, const string& password, const string& database, int port=-1);

	bool showDatabases (const string& host, const string& user, const string& password, vector<string>& dbNames, int port=-1);

	void close();

	bool tableExist(const string& table);

	bool columnExist(const string& table, const string& column, TeAttribute& attr);

	bool addColumn (const string& table,TeAttributeRep &rep);

	bool createRelation (const string& /* relName */, const string& /* table */, const string& /* fieldName */,
						const string& /* relatedTable */, const string& /* relatedField */, bool /* cascadeDeletion */)
	{ return true; }

	TeDBRelationType existRelation(const string& /* tableName */, const string& /* relName */)

	{	return TeNoRelation; }

	bool execute ( const string &q);

	bool createTable(const string& table, TeAttributeList &attr);

	bool alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName = "");

	virtual TeDatabasePortal* getPortal ();

// specific TerraLib database access methods

	bool insertProjection (TeProjection *proj);
	bool updateProjection (TeProjection *proj);

	//! Check if a particular layer exists (passing its name)
	virtual bool layerExist	(string layerName);

	bool insertLayer(TeLayer* layer);
	bool updateLayer(TeLayer *layer);

	//! Insert information about a datum
	virtual bool insertDatum(const TeDatum& datum);

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
	bool updateTheme (TeAbstractTheme *theme);
	bool insertThemeTable (int themeId, int tableId, int relationId, int tableOrder);
	bool updateRepresentation (int layerId, TeRepresentation& rep);

	//! Return a TeDatabaseIndex vector from a table
	virtual bool getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes);

	/** @name Raster Tables
	*  Retrieving/Inserting/Modifying/Deleting raster representations in the database. 
	*/
	//@{ 
	//! Inserts information about a raster geometry associated to an object
	/*!
		\param tableName name of the table that stores the raster representation
		\param par raster parameters 
		\param objectId identification of the object associated to the raster geometry
	*/
	bool insertRasterGeometry(const string& tableName, TeRasterParams& par, const string& objectId = "");
	//@}

	//! Updates the information about the raster geometry associated to an object of a layer
	/*!
		\param layerId layer unique database identification
		\param par raster parameters 
		\param objectId identification of the object associated to the raster geometry
	*/
	virtual bool updateRasterRepresentation(int layerId, TeRasterParams& par, const string& objectId="");

	bool insertLegend (TeLegendEntry *legend);
	virtual bool insertPolygon	(const string& table, TePolygon &p);
	virtual bool updatePolygon	(const string& table, TePolygon &p);
	virtual bool insertLine (const string&  table, TeLine2D &l);
	virtual bool updateLine (const string& table, TeLine2D &l);
	bool insertPoint (const string& table, TePoint &p);
	bool insertText (const string& table, TeText &t);
	bool insertArc (const string& table,TeArc &arc);
	bool insertNode	(const string& table, TeNode &node);	
	bool insertCell	(const string& table, TeCell &c);

	bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf,unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);

	string getSQLTime(const TeTime& time) const;
	string getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, 
						   const string& initialTime, const string& finalTime);

	//! Return a string that describes a where clause in SQL to return the geometries inside the box
	virtual string getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName, const std::string& columnName);

	bool inClauseValues(const string& query, const string& attribute, vector<string>& inClauseVector);

	bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
	
	//! Concat values in a vector using unionString as the join between each value
	string concatValues(vector<string>& values, const string& unionString);

	bool updateLayerBox(TeLayer* layer);

	//! Returns the SQL function for upper case
	string toUpper(const string& value);

	string getConcatFieldsExpression(const vector<string>& fNamesVec);

	bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

	virtual std::string mapToFirebirdType(const TeAttribute& teRep) const;

	//! Implementation of the method that create a spatial index for a spatial table. It create separeted index for each column ascending or descending for best performance in Firebird db.
	virtual bool createSpatialIndex(const string& table, const string& columns, TeSpatialIndexType /*type*/ = TeRTREE,short /* level */ =0,short /* tile */ =0);

	bool createAutoIncrement(const std::string& table, const std::string& column);

	int getLastGeneratedAutoNumber(const std::string& table);

	//! Gets the list of attributes of a table
	virtual bool getAttributeList(const string& tableName,TeAttributeList& attList);

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

   bool deleteColumn(const string& table, const string& colName);

   //@}

protected :
	IBPP::Database firebird_;
	IBPP::Database firebird_ref_;
	IBPP::Transaction transaction_;

	//! Create the indexes for the tables in the conceptual model using Descending order
	bool createDescIndex(const string& tableName, const string& indexName, const string& columnsName);
};

//! A concrete implementation of a portal to a Firebird database
class TLFIREBIRD_DLL TeFirebirdPortal  : public TeDatabasePortal
{
private:

public :

// Constructor / Destructor

	TeFirebirdPortal (TeFirebird* firebird);

	TeFirebirdPortal (const TeFirebirdPortal& p);

	virtual ~TeFirebirdPortal ();

// Basic database methods

	bool query (const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeUNIDIRECTIONAL, TeCursorEditType e = TeREADONLY, TeCursorDataType dt = TeTEXTCURSOR );

	bool fetchRow ();

	bool fetchRow (int i);

	void freeResult ();

// specific SQL SELECT command methods

	char*		getData (int i);
	char*		getData (const string& s);
	double		getDouble (int i);
	double		getDouble (const string& s);
	int			getInt (int i);
	int			getInt (const string& s);
	bool		getBool (const string& s);
	bool		getBool (int i);

	virtual bool fetchGeometry(TePolygon& pol);
	virtual bool fetchGeometry(TeLine2D& line);
	bool fetchGeometry(TeNode& n);
	bool fetchGeometry(TePoint& p);
	virtual bool fetchGeometry(TePolygon& pol, const unsigned int& initIndex);
	virtual bool fetchGeometry(TeLine2D& line, const unsigned int& initIndex);
	bool fetchGeometry(TeNode& n, const unsigned int& initIndex);
	bool fetchGeometry(TePoint& p, const unsigned int& initIndex);
	
	bool setData (int /* field */, string /* val */)
	{return true; }

	bool getBlob (const int& column, unsigned char* &data, unsigned long& size);
	bool getBlob (const string& s, unsigned char* &data, long& size);
	bool getRasterBlock(unsigned long& size, unsigned char* ptData);

	TeTime getDate (int i);
	TeTime getDate (const string& s);

	string getDateAsString(int i);
	string getDateAsString(const string& s);

protected:
	virtual TeLinearRing getLinearRing (int &ni);

	IBPP::Database firebird_;
	IBPP::Statement result_;
	IBPP::Transaction transaction_;
	std::string data_,
				query_;
	int currRow_;
};


/**
 * @brief This is the class for TeFirebird driver factory.
 * @author Frederico Augusto Bed� Teot�nio<frederico.bede@funcate.org.br>
 * @ingroup DatabaseUtils
 */
class TLFIREBIRD_DLL TeFirebirdFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeFirebirdFactory() : TeDatabaseFactory( std::string( "Firebird" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeFirebirdFactory() {};
      
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
      TeFirebird* instance_ptr = new TeFirebird();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeFirebirdFactory TeFirebirdFactory_instance;
}; 

#endif

