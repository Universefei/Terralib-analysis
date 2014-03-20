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
/*! \file TeAdoSqlServer.h
    \brief This file contains the particularities of a ADO SQL Server driver
*/
#ifndef  __TERRALIB_INTERNAL_ADOSQLSERVER_H
#define  __TERRALIB_INTERNAL_ADOSQLSERVER_H

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
#import <msadox.dll>
#import <oledb32.dll> rename_namespace("OLEDB") 
#else
#error ONLY FOR WINDOWS OPERATIONAL SYSTEM
#endif

#include <TeDatabase.h>
#include <TeAdoDB.h>


#ifdef AFX_DLL
#define EXPORT_WIN __declspec( dllexport )
#else
#define EXPORT_WIN
#endif

//! A driver class to an SQL Server database accessible using ADO library
class  TLADO_DLL  TeSqlServer : public TeAdo
{
friend class TeSqlServerPortal;

public: 

	TeSqlServer();
	~TeSqlServer();

  unsigned int maxFieldsPerRelation() { return 1000; }
	// Methods to create and connect to a ADO database
		virtual bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true, const std::string& characterSet = "");
		virtual bool connect (const string& host, const string& user, const string& password, const string& database, int port = -1);

	// Methods to manipulate a table
		virtual bool tableExist(const string& table);
     	virtual bool columnExist(const string& table, const string& column, TeAttribute& attr);
		virtual bool getAttributeList(const string& tableName,TeAttributeList& attList);
     	virtual bool createTable(const string& table, TeAttributeList &attr);
		virtual bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName="");
		virtual bool alterTable(const string& oldTableName, const string& newTablename);
			
     	virtual bool addColumn(const string& table,TeAttributeRep &rep);

		//! Deletes a column to a table
		/*!
			\param table table name
			\param colName name of the column being deleted
		*/
		virtual bool deleteColumn (const string& table, const string& colName);
		virtual bool deleteTable (const string& table);
     	virtual bool createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion);

		virtual bool deleteLayer(int layerId);
		virtual bool deleteTheme(int themeId);
		virtual bool deleteView (int viewId);

		virtual TeDBRelationType existRelation(const string& tableName, const string& relName);

		//! Returns a lista of databases
		virtual bool listDatabases(std::vector<std::string> & /*databaseList*/);
		//! Drop a database
		virtual bool dropDatabase(const std::string & /*databaseName*/);

	
		// Insert an attribute table
		virtual bool insertTable(TeTable &table);
     	virtual bool insertTheme (TeAbstractTheme *theme);     	
		virtual bool generateLabelPositions (TeTheme *theme, const std::string& objectId = "");
    
		virtual bool insertPolygonSet (const string& table, TePolygonSet &ps);	
     	virtual bool insertPolygon (const string& table, TePolygon &p);

	    virtual bool insertLineSet(const string& table, TeLineSet &ls); 
		virtual bool insertLine(const string& table, TeLine2D &l);
	
	    virtual bool insertPointSet(const string& table, TePointSet &ps); 
		virtual bool insertPoint(const string& table, TePoint &p);

		virtual bool insertTextSet(const string& table, TeTextSet &ts);	
		virtual bool insertText(const string& table, TeText &t);


     	virtual TeDatabasePortal* getPortal ();
     	virtual string	getNameTrigger(const string &tableName); 

		virtual string getSQLTime(const TeTime& time) const; 

		virtual string toUpper(const string& value);
};

//! A portal to access a ADO database
class  TLADO_DLL  TeSqlServerPortal : public TeAdoPortal
{

public :

	// Constructor / Destructor
	TeSqlServerPortal ();
	TeSqlServerPortal (TeDatabase* pDatabase);
	~TeSqlServerPortal ();

	TeTime getDate (int i);
	TeTime getDate (const string& s);

	string getDateAsString(int i);
	string getDateAsString(const string& s);
};


/**
 * @brief This is the class for TeSqlServer driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLADO_DLL TeSqlServerFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeSqlServerFactory() : TeDatabaseFactory( 
      std::string( "SqlServerAdo" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeSqlServerFactory() {};
      
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
      TeSqlServer* instance_ptr = new TeSqlServer();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeSqlServerFactory TeSqlServerFactory_instance;
};


#endif
