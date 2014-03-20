/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeAdoFireBird.h
    This file particularities of a ADO FireBird driver
*/
#ifndef  __TERRALIB_INTERNAL_ADOFIREBIRD_H
#define  __TERRALIB_INTERNAL_ADOFIREBIRD_H

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
//! A driver class to an FireBird database accessible using ADO library
/*!
  \sa
  TeDatabase
*/
class  EXPORT_WIN  TeFireBird : public TeAdo
{
friend class TeFireBirdPortal;

protected :

	virtual unsigned int getSequenceValue(const std::string &seqName);


public: 

	TeFireBird();
	~TeFireBird();

	// Methods to create and connect to a ADO database
		bool newDatabase(const string& database, const string& user, const string& password, const string& host, const int &port=0, bool terralibModel=true);
		bool connect (const string& host, const string& user, const string& password, const string& database, int port = 0);

  unsigned int maxFieldsPerRelation() { return 0; }

	// Methods to manipulate a table
		bool tableExist(const string& table);
     	bool columnExist(const string& table, const string& column, TeAttribute& attr);
		bool createTable(const string& table, TeAttributeList &attr);
			
     	bool addColumn(const string& table,TeAttributeRep &rep);
		bool deleteTable (const string& table);
     	bool createRelation (const string& name, const string& table, const string& fieldName, const string& relatedTable, const string& relatedField, bool cascadeDeletion);

		bool deleteLayer(int layerId);

		TeDBRelationType existRelation(const string& tableName, const string& relName);

		//bool listTables(vector<string>& tableList);
		//bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName="");
		//bool alterTable (const string& oldTableName, const string& newTableName);


		// Method to create a symbol table
		//bool createSymbolTable(void);//******************

	// Insert/Update methods

	// Insert an attribute table
     	bool insertTable(TeTable &table);
		bool insertRelationInfo(const int tableId, const string& tField, const string& rName, const string& rField, int& relId);
		bool updateTable (TeTable &table);
     	bool insertTableInfo (int layerId, TeTable &table, const string& user="");

		bool updateRepresentation (int layerId, TeRepresentation& rep);
		bool updateProjection (TeProjection *proj);
		bool insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size);
		


     	bool insertLayer(TeLayer *layer);
     	bool insertRepresentation (int layerId, TeRepresentation& rep);
     	bool insertView (TeView *view);
		bool insertViewTree (TeViewTree *tree);
     	bool insertTheme (TeAbstractTheme *theme);     	
		bool insertThemeTable (int themeId, int tableId, int relationId, int tableOrder);
		bool insertThemeGroup(TeViewTree* tree);
		bool generateLabelPositions (TeTheme *theme);
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

		bool insertRasterGeometry(const string& tableName, TeRasterParams& par, const string& objectId);

     	bool insertCellSet (const string& table, TeCellSet &cs);
		bool insertCell	(const string& table, TeCell &c);

		bool getAttributeList(const string& tableName,TeAttributeList& attList);


     	TeDatabasePortal* getPortal ();

     //	bool createVisualTable ();
	//	bool createVisualRasterTable();

		
		string	getNameTrigger(const string &tableName);

		bool	createSequence(const string &seqName);
		bool	createAutoIncrementTrigger(const string &tableName, const string &fieldName);
		string	getNameSequence(const string &tableName);

		string  getSQLAutoNumber(const string& table);
		bool	updatePolygon (const string& table, TePolygon &p );
		bool	updateLine (const string& table, TeLine2D &line);
		bool	insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur,unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband);


};

//! A portal to access a ADO database
class  EXPORT_WIN  TeFireBirdPortal : public TeAdoPortal
{

public :

	// Constructor / Destructor
	TeFireBirdPortal ();
	TeFireBirdPortal (TeDatabase* pDatabase);
	~TeFireBirdPortal ();

	bool query ( const string &qry, TeCursorLocation l = TeSERVERSIDE, TeCursorType t = TeBIDIRECTIONAL, TeCursorEditType e = TeREADWRITE);
	TeTime getDate (int i);
	TeTime getDate (const string& s);

	string getDateAsString(int i);
	string getDateAsString(const string& s);
	
};

/**
 * @brief This is the class for TeFireBird driver factory.
 * @author Diego Moreira Exército Brasileiro
 * @ingroup DatabaseUtils
 */
class TeFireBirdFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeFireBirdFactory() : TeDatabaseFactory( std::string( "FireBirdAdo" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeFireBirdFactory() {};
      
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
      TeFireBird* instance_ptr = new TeFireBird();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeFireBirdFactory TeFireBirdFactory_instance;
};
#endif
