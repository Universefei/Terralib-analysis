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
    \brief This file contains the particularities of a ADO SQL Server Spatial driver
*/
#ifndef  __TERRALIB_INTERNAL_ADOSQLSERVER_SPATIAL_H
#define  __TERRALIB_INTERNAL_ADOSQLSERVER_SPATIAL_H

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
#include <TeAdoSqlServer.h>


#ifdef AFX_DLL
#define EXPORT_WIN __declspec( dllexport )
#else
#define EXPORT_WIN
#endif

//! A driver class to an SQL Server Spatial database accessible using ADO library
class  TLADO_DLL  TeSqlServerSpatial : public TeSqlServer
{
friend class TeSqlServerSpatialPortal;

protected:

	virtual std::string STGeomFromWkb(const TePolygon &polygon);

	//! \brief STGeomFromText
	/*! Method to convert a terralib polygon to wbt
		\param		polygon		terralib polygon
		\return		returns the sql in wbt format
	*/
	virtual std::string STGeomFromText(const TePolygon &polygon);

	//! \brief STGeomFromText
	/*! Method to convert a terralib line to wbt
		\param		line		terralib line
		\return		returns the sql in wbt format
	*/
	virtual std::string STGeomFromText(const TeLine2D &line);

	//! \brief STGeomFromText
	/*! Method to convert a terralib point to wbt
		\param		point		terralib point
		\return		returns the sql in wbt format
	*/
	virtual std::string STGeomFromText(const TePoint &point);

	//! \brief PutBinaryIntoVariant
	/*! Method to transform the binary data to variant
		\param	ovData		variant
		\param	pBuf		binary data
		\param	cBufLen		buf length
	*/
	virtual bool PutBinaryIntoVariant(VARIANT &ovData, BYTE * pBuf,unsigned long cBufLen);

public: 

	//! \brief Empty Constructor
	TeSqlServerSpatial();

	//! \brief Destructor
	~TeSqlServerSpatial();

	virtual bool connect (const string& host, const string& user, const string& password, const string& database,  int port = -1);

	
	// Methods to manipulate a table

	//! \brief columnExist
	/*! Method to check if column exists
		\param	table		table name
		\param	column		column name
		\param	attr		terralib attribute
		\return	returns true whether column exist
	*/
    bool columnExist(const string& table, const string& column, TeAttribute& attr);
    
	//! \brief createTable
	/*! Method to create a table
		\param	table		table name
		\param	attr		terralib attribute
		\return	returns true whether create table with sucess
	*/
	bool createTable(const string& table, TeAttributeList &attr);
	
	//! \brief alterTable
	/*! Method to alter table structure
		\param	table		table name
		\param	rep			Terralib Attribute representation
		\param	oldColName	old column name
		\return	returns true whether sucess
	*/
	bool alterTable (const string& table, TeAttributeRep &rep, const string& oldColName="");
			
	// Insert an attribute table
	//! \brief generateLabelPositions
	/*! Method to generate labels positions to terralib theme
		when created
		\param	theme		terralib theme
		\param	objectid	object identify
		\return returns true whether sucess
	*/
	bool generateLabelPositions (TeTheme *theme, const std::string& objectId = "");
    
	//! \brief insertPolygonSet
	/*! Method to insert the polygon list to spatial database
		\param	table		table name
		\param	ps			terralib polygon list
		\return	returns true whether sucess
	*/
	bool insertPolygonSet (const string& table, TePolygonSet &ps);	

	//! \brief insertPolygon
	/*! Method to insert the polygon to spatial database
		\param	table		table name
		\param	p			terralib polygon 
		\return	returns true whether sucess
	*/
    bool insertPolygon (const string& table, TePolygon &p);

	//! \brief insertLineSet
	/*! Method to insert the terralib line list to spatial database
		\param	table		table name
		\param	ls			terralib line list
		\return	returns true whether sucess
	*/
	bool insertLineSet(const string& table, TeLineSet &ls); 

	//! \brief insertLine
	/*! Method to insert the terralib line to spatial database
		\param	table		table name
		\param	l			terralib line
		\return	returns true whether sucess
	*/
	bool insertLine(const string& table, TeLine2D &l);
	
	//! \brief insertPointSet
	/*! Method to insert the terralib point list
		\param	table		table name
		\param	ps			terralib point list
		\return	returns true whether sucess
	*/
	bool insertPointSet(const string& table, TePointSet &ps); 

	//! \brief insertPoint
	/*! Method to insert the terralib point to database
		\param	table		table name
		\param	p			terralib point
		\return	returns true whether sucess
	*/
	bool insertPoint(const string& table, TePoint &p);

	//! \brief insertTextSet
	/*! Method to insert the terralib text list to database
		\param	table		table name
		\param	ts			terralib text list
		\return	returns true whether sucess
	*/
	bool insertTextSet(const string& table, TeTextSet &ts);	

	//! \brief insertText
	/*! Method to insert terralib text to database
		\param	table		table name
		\param	t			terralib text
		\return	returns true whether sucess
	*/
	bool insertText(const string& table, TeText &t);

	//! \brief createTextGeometry
	/*! Method to create a terralib text geometry table
		\param	table		table name
		\return returns true whether sucess
	*/
	virtual bool createTextGeometry(const string& table);
	
	//! \brief getSQLBoxWhere
	/*! Method to return the espatial sql box where
		\param	box			terralib bounding box
		\param	rep			representation
		\return		returns the spatial sql box where string
	*/
	virtual string  getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& );
	
	//! \brief getSQLOrderBy
	/*! Method to return the SQL Order By
		\param	rep		terralib representation
		\return	returns the sql string
	*/
	virtual std::string getSQLOrderBy(const TeGeomRep& rep) const;

	//! \brief getPortal
	/*! Method to return the terralib database portal
		\return	returns the terralib database portal
	*/
    TeDatabasePortal* getPortal ();

	//! \brief createSpatialIndex
	/*! Create a SQL Server Spatial Index
	*/
	bool	createSpatialIndex(const string& table, const string& columns, TeSpatialIndexType type, short level, short tile);

	//! Return the box of a specific geometry (object_id)  
	virtual bool	getMBRSelectedObjects(string geomTable,string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType,TeBox &bout, const double& tol);

	virtual bool locatePoint (const string& table, TeCoord2D &pt, TePoint &point, const double& tol);

	virtual bool locatePolygon (const string& table, TeCoord2D &pt, TePolygon &polygon, const double& tol);

	virtual bool locatePolygonSet (const string& table, TeCoord2D &pt, double tol, TePolygonSet &polygons);

	virtual bool locateLine		(const string& table, TeCoord2D &pt, TeLine2D &line, const double& tol = 0.0);

	virtual bool locateLineSet	(const string& table, TeCoord2D &pt, TeLineSet & ls, const double& tol = 0.0);
     	
};

//! A portal to access a ADO database
class  TLADO_DLL  TeSqlServerSpatialPortal : public TeAdoPortal
{

public :

	// Constructor / Destructor
	TeSqlServerSpatialPortal ();
	TeSqlServerSpatialPortal (TeDatabase* pDatabase);
	~TeSqlServerSpatialPortal ();

	TeTime getDate (int i);
	TeTime getDate (const string& s);

	string getDateAsString(int i);
	string getDateAsString(const string& s);
	bool	fetchGeometry(TePolygon& pol);
	bool	fetchGeometry(TeLine2D& line);
	bool	fetchGeometry(TePoint& p);
	bool	fetchGeometry(TePoint& p, const unsigned int& initIndex);
};


/**
 * @brief This is the class for TeSqlServerSpatial driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLADO_DLL TeSqlServerSpatialFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeSqlServerSpatialFactory() : TeDatabaseFactory( 
      std::string( "SqlServerAdoSpatial" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeSqlServerSpatialFactory() {};
      
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
      TeSqlServerSpatial* instance_ptr = new TeSqlServerSpatial();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeSqlServerSpatialFactory TeSqlServerSpatialFactory_instance;
};


#endif
