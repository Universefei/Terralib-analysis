/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeAdoDB.h
    \brief This file contains the particularities of a ADO driver
*/
#ifndef  __TERRALIB_INTERNAL_ADODB_H
#define  __TERRALIB_INTERNAL_ADODB_H

#include <TeDatabaseFactory.h>

#ifdef WIN32
#pragma warning ( disable: 4192 )
#pragma warning ( disable: 4146 )
#include <atldbcli.h>
#include <iostream>
#include <comdef.h>

/*
 *   Lets use the import directive to create some smart pointers for us
 */
#import "msado26.tlb"  rename("EOF", "IsEOF") rename("BOF", "IsBOF")
#import "msadox.dll"
#import "oledb32.dll" rename_namespace("OLEDB")
#else
#error ONLY FOR WINDOWS OPERATIONAL SYSTEM
#endif
#include <TeDatabase.h>


#ifdef AFX_DLL
#define EXPORT_WIN __declspec( dllexport )
#else
#define EXPORT_WIN
#endif

#include "TeAdoDefines.h"

//! A driver class to a database accessible using ADO library
class  TLADO_DLL  TeAdo : public TeDatabase
{
friend class TeAdoPortal;

public: 
	TeAdo();
	~TeAdo();

	// Methods to create and connect to a ADO database
	virtual bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true, const std::string& characterSet = "");
	virtual bool connect (const string& host, const string& user, const string& password, const string& database, int port = -1);
	//! Connects using a stored connection string
	virtual bool connect (const string& fullDatabaseName = "");
//	bool connect ();
	virtual void connection (ADODB::_ConnectionPtr conn);
	string connectionString() { return connectionString_; }

	// Close connection
	void close() ;

  unsigned int maxFieldsPerRelation() { return 255; }
	// Submit a SQL statement
	bool execute ( const string &sql);
	string  escapeSequence(const string& from);

	// Methods to manipulate a table
	bool tableExist(const string& table);
	bool listTables(vector<string>& tableList);
	bool columnExist(const string& table, const string& column,TeAttribute& attr);
	bool allowEmptyString(const string& tableName, const string& column);
	bool getAttributeList(const string& tableName,TeAttributeList& attList);
	
	virtual bool createTable(const string& table, TeAttributeList &attr);
	virtual bool addColumn(const string& table,TeAttributeRep &rep);
	virtual bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName="");
	virtual bool alterTable (const string& oldTableName, const string& newTableName);
	
	virtual bool deleteColumn (const string& table, const string& colName);
	virtual bool createRelation (const string& relationName, 
					   const string& foreignTbl, const string& FTKey, 
					   const string& relatedTbl, const string& RTKey, 
					   bool cascadeDeletion);

	TeDBRelationType existRelation(const string& tableName, const string& relName);

	// Insert/Update methods
	bool insertProject(TeProject *project);

	// Insert an attribute table
	bool insertTable(TeTable &table);
	bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
		
	bool insertTableInfo (int layerId, TeTable &table, const string& user="");
	bool insertRelationInfo(const int tableId, const string& tField,
						    const string& eTable, const string& eField, int& relId);
	bool insertLayer(TeLayer *layer);
	bool insertRepresentation (int layerId, TeRepresentation& rep);
	bool insertView (TeView *view);
	virtual bool insertTheme (TeAbstractTheme *theme);
	virtual bool insertThemeGroup (TeViewTree* tree);
	bool insertThemeTable	(int themeId, int tableId, int relationId, int tableOrder);
	
	bool insertViewTree (TeViewTree *tree);
	bool insertLegend (TeLegendEntry *legend);
	bool insertProjection (TeProjection *proj);	

	virtual bool insertPolygonSet (const string& table, TePolygonSet &ps);	
	virtual bool insertPolygon (const string& table, TePolygon &p);
	bool updatePolygon (const string& table, TePolygon &p);

	bool insertLineSet (const string& table, TeLineSet &ls);
	bool insertLine (const string& table, TeLine2D &l);
	bool updateLine (const string& table, TeLine2D &l);

	bool insertPointSet (const string& table, TePointSet &ps);
	bool insertPoint (const string& table, TePoint &p);
	bool updatePoint (const string& table, TePoint &p);

	bool insertTextSet (const string& table, TeTextSet &ts);	
	bool insertText (const string& table, TeText &t);

	bool insertArcSet (const string& table, TeArcSet &as);
	bool insertArc (const string& table,TeArc &arc);

	bool insertNodeSet (const string& table, TeNodeSet &ns);	
	bool insertNode	(const string& table, TeNode &node);	
	bool updateNode	(const string& table, TeNode &node);	

	bool insertCellSet (const string& table, TeCellSet &cs);
	bool insertCell	(const string& table, TeCell &c);

	bool ByteArrayToVariant(VARIANT &varArray, unsigned char* bytes, int nbytes);
	bool LineToVariant(VARIANT &varArray, TeLine2D& line);
	bool LinearRingToVariant(VARIANT &varArray, TeLinearRing& ring);
	bool PointsToVariant(VARIANT &varArray, double* points, int npoints);

	TeDatabasePortal* getPortal ();

	bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf,unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);

	//! Gets the date and time following the format MMsDDsYYYYsHHsMMsSS to use with Access
	string getSQLTime(const TeTime& time) const;
	string getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, 
						   const string& initialTime, const string& finalTime);
	
	//! Concat values in a vector using unionString as the join between each value
	string concatValues(vector<string>& values, const string& unionString);

	//! Returns the SQL function for upper case
	string toUpper(const string& value);

	//informations about the system datetime format
	string			systemDateTimeFormat_;  //!< system format for date and time 
	string			systemIndAM_;			//!< AM indicator for a 12 hour clock
	string			systemIndPM_;			//!< PM indicator for a 12 hour clock
	string			systemDateSep_;			//!< date separator
	string			systemTimeSep_;			//!< time separator

	bool updateBBox(const string& tableName, const string& idName, int idValue, const TeBox& box);
	bool updateLayerBox(TeLayer* layer);
	bool updateLayer(TeLayer* layer);

	static string systemDateTimeFormat(string& indAM, string& indPM, string& sepD, string& sepT);

	/** @name Transaction control methods 
	  */
	//@{ 
	//! Begins a transaction
	virtual bool beginTransaction(); 

	//! Commits a transaction
	virtual bool commitTransaction();

	//! Rollbacks a transaction
	virtual bool rollbackTransaction();
   //@}

protected :
	ADODB::_ConnectionPtr connection_;
	string connectionString_;
};

//! A portal to access a ADO database
class  TLADO_DLL  TeAdoPortal : public TeDatabasePortal
{
protected:
	
	_bstr_t					bvalue_;
	string					error_message_;
	long					error_num_;
	long					curRow_;
	
	TeLinearRing			getLinearRing();

public :
// Constructor / Destructor

	TeAdoPortal ();
	TeAdoPortal (TeDatabase* pDatabase);
	~TeAdoPortal ();

    // Queries
	bool isConnected ();
    bool isEOF();

    // Move operations
    bool moveFirst();
    bool moveNext();

	ADODB::_ConnectionPtr	connection_;
	ADODB::_RecordsetPtr	recset_;


// Basic database methods
	bool query ( const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeBIDIRECTIONAL, TeCursorEditType e = TeREADWRITE, TeCursorDataType dt = TeTEXTCURSOR );
	bool fetchRow ();
	bool fetchRow (int i);
	void freeResult ();
	
// specific TerraLib database access methods
	bool fetchGeometry(TePolygon& pol);
	bool fetchGeometry(TePolygon& pol, const unsigned int& initIndex);
	bool fetchGeometry(TeLine2D& line);
	bool fetchGeometry(TeLine2D& line, const unsigned int& initIndex);
	bool fetchGeometry(TeNode& n);
	bool fetchGeometry(TeNode& n, const unsigned int& initIndex );
	bool fetchGeometry(TePoint& p);
	bool fetchGeometry(TePoint& p, const unsigned int& initIndex);

	char* getData (int i);
	char* getData (const string& s);
	double  getDouble (int i);
	double  getDouble (const string& s);
	int	getInt (int i);
	int	getInt (const string& s);
	bool  getBool (const string& s);
	bool  getBool (int i);
	bool  getBlob (const string& s, unsigned char* &data, long& size);
	bool getBlob (int i, unsigned char* &data, long& size);

	bool BlobToLine(TeLine2D& line);
	bool getRasterBlock(unsigned long& size, unsigned char* ptData);

	TeTime getDate (int i);
	TeTime getDate (const string& s);

	//! Returns 
	string getDateAsString(int i);
	string getDateAsString(const string& s);

};

/**
 * @brief This is the class for TeAdo driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLADO_DLL TeAdoFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeAdoFactory() : TeDatabaseFactory( std::string( "Ado" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeAdoFactory() {};
      
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
      TeAdo* instance_ptr = new TeAdo();
      
      if( arg.database_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeAdoFactory TeAdoFactory_instance;
}; 

#endif
