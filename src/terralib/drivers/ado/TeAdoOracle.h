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
/*! \file TeAdoOracle.h
    \brief This file contains the particularities of a ADO Oracle driver
*/
#ifndef  __TERRALIB_INTERNAL_ADOORACLE_H
#define  __TERRALIB_INTERNAL_ADOORACLE_H

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

//! A driver class to an Oracle database accessible using ADO library
class  TLADO_DLL  TeOracle : public TeAdo
{
friend class TeOraclePortal;

public: 

	TeOracle();
	~TeOracle();

  unsigned int maxFieldsPerRelation() { return 1000; }

	// Methods to create and connect to a ADO database
		bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=-1, bool terralibModel=true);
		bool connect (const string& host, const string& user, const string& password, const string& database, int port = -1);

	// Methods to manipulate a table
		bool tableExist(const string& table);
		bool listTables(vector<string>& tableList);
     	bool columnExist(const string& table, const string& column, TeAttribute& attr);
		bool createTable(const string& table, TeAttributeList &attr);
			
     	bool addColumn(const string& table,TeAttributeRep &rep);
		bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName="");
		bool alterTable (const string& oldTableName, const string& newTableName);
		bool deleteTable (const string& table);
     	bool createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion);

		bool deleteLayer(int layerId);

		TeDBRelationType existRelation(const string& tableName, const string& relName);

// Insert/Update methods

	// Insert an attribute table
        bool insertTableInfo (int layerId, TeTable &table, const string& user="");
		bool insertRelationInfo(const int tableId, const string& tField, const string& rName, const string& rField, int& relId);

     	bool insertLayer(TeLayer *layer);
		virtual bool updateBBox	(const string& tableName, const string& idName, int idValue, const TeBox& box); /*****/
		virtual bool updateLayerBox	(TeLayer* layer); 
		virtual bool updateLayer	(TeLayer* layer); 

     	bool insertRepresentation (int layerId, TeRepresentation& rep);
     	bool insertView (TeView *view);
		bool insertViewTree (TeViewTree *tree);
     	bool insertTheme (TeAbstractTheme *theme);     	
		bool insertThemeTable (int themeId, int tableId, int relationId, int tableOrder);
		bool insertThemeGroup(TeViewTree* tree);
		bool generateLabelPositions (TeTheme *theme, const std::string& objectId = "");
     	bool insertLegend (TeLegendEntry *legend);
     	bool insertProjection (TeProjection *proj);	
	
		bool insertPolygonSet (const string& table, TePolygonSet &ps);	
     	bool insertPolygon (const string& table, TePolygon &p);

     	bool insertLineSet (const string& table, TeLineSet &ls);
     	bool insertLine (const string& table, TeLine2D &l);

     	bool insertPointSet (const string& table, TePointSet &ps);
     	bool insertPoint (const string& table, TePoint &p);

     	bool insertTextSet (const string& table, TeTextSet &ts);	
     	bool insertText (const string& table, TeText &t);

     	bool insertArcSet (const string& table, TeArcSet &as);
     	bool insertArc (const string& table,TeArc &arc);

     	bool insertNodeSet (const string& table, TeNodeSet &ns);	
     	bool insertNode	(const string& table, TeNode &node);	

     	bool insertCellSet (const string& table, TeCellSet &cs);
		bool insertCell	(const string& table, TeCell &c);

     	TeDatabasePortal* getPortal ();

     	bool createAutoIncrementTrigger(const string &tableName, const string &fieldName);
		string	getNameTrigger(const string &tableName);

		bool	createSequence(const string &seqName);
		string	getNameSequence(const string &tableName);

		string  getSQLAutoNumber(const string& table);

		bool getAttributeList(const string& tableName,TeAttributeList& attList);

		string getSQLTime(const TeTime& time) const;

		bool insertProject(TeProject* project);
};

//! A portal to access a ADO database
class  TLADO_DLL  TeOraclePortal : public TeAdoPortal
{

public :

	// Constructor / Destructor
	TeOraclePortal ();
	TeOraclePortal (TeDatabase* pDatabase);
	~TeOraclePortal ();

	bool query ( const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeBIDIRECTIONAL, TeCursorEditType e = TeREADWRITE, TeCursorDataType dt = TeTEXTCURSOR );

	TeTime getDate (int i);
	TeTime getDate (const string& s);

	string getDateAsString(int i);
	string getDateAsString(const string& s);
};


/**
 * @brief This is the class for TeOracle driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLADO_DLL TeOracleFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeOracleFactory() : TeDatabaseFactory( std::string( "OracleAdo" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeOracleFactory() {};
      
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
      TeOracle* instance_ptr = new TeOracle();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeOracleFactory TeOracleFactory_instance;
};


#endif
