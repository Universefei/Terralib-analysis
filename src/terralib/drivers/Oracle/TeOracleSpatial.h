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
/*! \file TeOracleSpatial.h
	\brief This file contains two classes that implement a driver of interface between TerraLib and ORACLE SPATIAL DBMS, using OCI (Oracle Call Interface) library.
*/

#ifndef  __TERRALIB_INTERNAL_ORACLESPATIAL_H
#define  __TERRALIB_INTERNAL_ORACLESPATIAL_H

#include "TeOCIOracle.h"

#include <TeDatabaseFactory.h>

#include "TeOracleDefines.h"

#ifdef AFX_DLL
#define EXPORT_WIN __declspec( dllexport )
#else
#define EXPORT_WIN
#endif

/*! \enum TeSDOGType
	\brief Types of geometries in the ORACLE SPATIAL 
*/
enum TeSDOGType  
{ TeSDOUNKNOWN, TeSDOPOINT, TeSDOLINE, TeSDOPOLYGON,TeSDOCOLLECTION, 
  TeSDOMULTIPOINT, TeSDOMULTILINE, TeSDOMULTIPOLYGON};

/*! \class TeOracleSpatial
	\brief A concrete implementation of a interface driver to the ORACLE SPATIAL DBMS
	
	This class contains attributes and methods to implement a driver of interface 
	between TerraLib and ORACLE SPATIAL DBMS using OCI (Oracle Call Interface) library.
	
	\sa 
	TeOCIOracle TeOCIConnect
*/
class TLORACLE_DLL TeOracleSpatial : public TeOCIOracle
{

friend class TeOracleSpatialPortal;

private:

	//! Deletes the metadata information associated to a geometric table in the USER_SDO_GEOM_METADATA table  
	bool	deleteMetadata(const string &table, const string &column);
	
	//! Rebuilds a specific spatial index created to a geometric table
	bool	rebuildSpatialIndex(const string &table);

	//! Deletes a specific spatial index created to a geometric table
	bool	deleteSpatialIndex(const string &table);

public:
	
	//! Constructor
	TeOracleSpatial();

	//! Destructor
	~TeOracleSpatial()
	{}

	//! Connects to Oracle Spatial DBMS
	bool connect (const string& host, const string& user, const string& password, const string& database, int port = -1);

	//! Creates a new table
    bool createTable (const string& table, TeAttributeList &attr);

	//! Creates a spatial index to a specific geometry table
	bool createSpatialIndex(const string &table, const string &column,TeSpatialIndexType type= TeRTREE,short level=0,short tile=0);

	//! Inserts the metadata information associated to a specific geometric table in the the USER_SDO_GEOM_METADATA table
	bool insertMetadata(const string &table, const string &column, double tolx,double toly,TeBox &box,short srid=0);

	//! Returns the name of the column that will be spatially indexed, for a given type of geometry table 
	string getSpatialIdxColumn(TeGeomRep rep);
	
	//! Gets a cursor using the opened connection 
	TeDatabasePortal* getPortal ();

	//! Generates label positions to each object of a theme
	bool generateLabelPositions	(TeTheme *theme, const std::string& objectId = ""); 
	
	/** @name Inserts, updates and deletes geometries from the tables
	*/
	//@{ 
	bool insertPolygon		(const string& table, TePolygon &p);	
	bool updatePolygon		(const string& table, TePolygon &p);
	bool locatePolygon		(const string& table, TeCoord2D &pt, TePolygon &polygon, const double& tol = 0.0);
	bool selectPolygonSet	(const string& table, const string& criteria, TePolygonSet &ps); 
	bool loadPolygonSet		(const string& table, const string& geoid, TePolygonSet &ps);  
	bool loadPolygonSet		(const string& table, TeBox &box, TePolygonSet &ps); 
	bool loadPolygonSet		(TeTheme* theme, TePolygonSet &ps); 
	TeDatabasePortal*		loadPolygonSet(const string& table, TeBox &box); 
	bool allocateOrdinatesObject(TePolygon &poly, string& elInfo, TeOCICursor* cursor=0);
	
	bool insertLine		(const string& table, TeLine2D &l);		
	bool updateLine		(const string& table, TeLine2D &l);
	bool locateLine		(const string& table, TeCoord2D &pt, TeLine2D &line, const double& tol = 0.0);
	bool loadLineSet	(const string& table, const string& geoid, TeLineSet &ls); 
	bool loadLineSet	(const string& table, TeBox &box, TeLineSet &linSet); 
	TeDatabasePortal*   loadLineSet (const string& table, TeBox &box);
	bool allocateOrdinatesObject(TeLine2D &line, TeOCICursor* cursor=0);
			
    bool insertPoint	(const string& table, TePoint &p);	
	bool updatePoint	(const string& table, TePoint &p);
	bool locatePoint	(const string& table, TeCoord2D &pt, TePoint &point, const double& tol = 0.0);
    
	bool insertText		(const string& table, TeText &t);	

	bool insertArc		(const string& table,TeArc &arc);
	bool insertNode		(const string& table, TeNode &node);	
	bool updateNode		(const string& table, TeNode &node);	

	bool insertCell		(const string& table, TeCell &c);
	bool updateCell		(const string& table, TeCell &c);
	bool locateCell		(const string& table, TeCoord2D &pt, TeCell &cell, const double& tol = 0.0);

	//! Removes a geometry from the given tableName
	virtual bool removeGeometry(const string& tableName, const TeGeomRep& rep, const int& geomId);
	
	bool insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, unsigned char *buf,unsigned long size, int band=0, unsigned int res=1, unsigned int subband=0);
	//@}

	/** @name Methods that return specific SQL statement according to each DBMS
	*/
	//@{ 
	
	//!< Returns a string SQL to be used in the ORDER BY clause when querying geometries.
	virtual std::string getSQLOrderBy(const TeGeomRep& rep) const;

	string getSQLBoxWhere (const TeBox& box, const TeGeomRep rep, const std::string& tableName);
	
	//! Return a string that describes a where clause in SQL to return the geometries inside the box
	virtual string getSQLBoxWhere(const TeBox& box, const TeGeomRep rep, const std::string& tableName, const std::string& columnName);

	string getSQLBoxWhere (const string& table1, const string& table2, TeGeomRep rep2, TeGeomRep rep1); 
	string getSQLBoxSelect (const string& tableName, TeGeomRep rep); 
	//@}
	
	bool getMBRSelectedObjects(string geomTable,string colGeom, string fromClause, string whereClause, string afterWhereClause, TeGeomRep repType,TeBox &bout, const double& tol = 0.0);
	bool getMBRGeom(string tableGeom, string object_id, TeBox& box, string colGeom);

	/** @name Spatial query
	*/
	//@{ 
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsIn, TeDatabasePortal *portal, int relate, const string& actCollTable="");
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsIn, const string& visGeomTable, TeGeomRep visRep, TeDatabasePortal *portal, int relate, const string& visCollTable=""); 
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeGeometry* geom, TeDatabasePortal *portal, int relate, const string& actCollTable=""); 
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsIn, TeKeys& actIdsOut, int relate, const string& actCollTable="");
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsIn, const string& visGeomTable, TeGeomRep visRep, TeKeys& visIdsOut, int relate, const string& visCollTable=""); 
	bool spatialRelation(const string& actGeomTable, TeGeomRep actRep, TeGeometry* geom, TeKeys& actIdsOut, int relate, const string& actCollTable=""); 
	//@}

	/** @name Metric functions
	*/
	//@{ 
	bool calculateArea(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsOut, double &area);
	bool calculateLength(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIdsIn, double &length);
	bool calculateDistance(const string& actGeomTable, TeGeomRep actRep, TeKeys& Ids, double& distance);
	bool calculateDistance(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, TeGeomRep visRep, const string& objId2, double& distance);
	//@}

	/** @name Functions that generate new geometry
	*/
	//@{  
	bool buffer(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIds, TePolygonSet& bufferSet, double dist);
	bool convexHull(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIds, TePolygonSet& convexHullSet);
	bool centroid(const string&  actGeomTable , TeGeomRep actRep, TePointSet& centroidSet, TeKeys actIds = vector<string>(), const string& actCollTable = "");
	//@}

	/** @name Functions that return the nearest neighbors 
	*/
	//@{  
	bool nearestNeighbors(const string& actGeomTable, const string& actCollTable, TeGeomRep actRep, const string& objId1, TeKeys& actIdsOut, int numRes=1);
	bool nearestNeighbors(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, const string& visCollTable, TeGeomRep visRep, TeKeys& visIdsOut, int numRes=1); 
	bool nearestNeighbors(const string& actGeomTable, const string& actCollTable, TeGeomRep actRep, const string& objId1, TeDatabasePortal* portal, int numRes=1);
	bool nearestNeighbors(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, const string& visCollTable, TeGeomRep visRep, TeDatabasePortal* portal, int numRes=1); 
	//@}

	/** @name Set functions
	*/
	//@{  
	bool geomIntersection(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIds, TeGeometryVect& geomVect);
	bool geomIntersection(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, TeGeomRep visRep, const string& objId2, TeGeometryVect& geomVect);
	bool geomDifference(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& objId2, TeGeometryVect& geomVect);
	bool geomDifference(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, TeGeomRep visRep, const string& objId2, TeGeometryVect& geomVect);
	bool geomUnion(const string& actGeomTable, TeGeomRep actRep, TeKeys& actIds, TeGeometryVect& geomVect);
	bool geomUnion(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, TeGeomRep visRep, const string& objId2, TeGeometryVect& geomVect);
	bool geomXOr(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& objId2, TeGeometryVect& geomVect);
	bool geomXOr(const string& actGeomTable, TeGeomRep actRep, const string& objId1, const string& visGeomTable, TeGeomRep visRep, const string& objId2, TeGeometryVect& geomVect);	
	//@}
};

/*! \class TeOracleSpatialPortal
	\brief A class that implements a concept of record set to a ORACLE SPATIAL DBMS
	
	This class is part of the driver of interface 
	between TerraLib and ORACLE SPATIAL DBMS. 
	It was developed using OCI (Oracle Call Interface) library.
	
	\sa 
	TeOCIOraclePortal TeOCICursor
*/
class TLORACLE_DLL TeOracleSpatialPortal : public TeOCIOraclePortal
{

	friend class TeOracleSpatial;
	
protected:
	//! Gets the size of the element information array (SDO_ELEM_INFO type) of the record set current row 
	int			getDimArraySize();
	//! Gets the i-th element element information array (SDO_ELEM_INFO type) of the record set current row
	bool		getDimElement(int i,int &elem);
	//! Gets the number of ordinates of the coordinate array (SDO_ORDINATES type) of the record set current row
	int			numberOfOrdinates();
	//! Gets the i-th coordinate from the coordinates array (SDO_ORDINATES type) of the record set current row
	bool		getCoordinates(int i,TeCoord2D& coord);
	//! Gets the geometry type of the record set current row
	bool		getGeometryType(TeSDOGType& gType);
	//! Gets the spatial reference associated to the SDO_GEOMETRY type of the record set current row
	int			getSpatialReferenceId();
	//! Gets the point (x and y) associated to the SDO_POINT type of the record set current row
	bool		getPointXYZ (double& x,double& y);

public:

	//! Constructor
	TeOracleSpatialPortal(TeOracleSpatial* pDatabase) : TeOCIOraclePortal(pDatabase) 
	{}

	//! Destructor
	virtual ~TeOracleSpatialPortal()
	{}
    
	//! Executes a SQL statement that uses operators and functions of the ORACLE SPATIAL
	bool querySDO (const string &q);
	
	/** @name Methods that return the geometry of the record set current row and fetch to the next row
	*/
	//@{ 
	bool getGeometry (TeGeometry** geom, bool& result);
	bool fetchGeometry (TePolygon& poly);
	bool fetchGeometry (TePolygon& poly, const unsigned int& initIndex);
	bool fetchGeometry (TePolygonSet& polySet);
	bool fetchGeometry (TeLine2D& line);
	bool fetchGeometry (TeLine2D& line, const unsigned int& initIndex);
	bool fetchGeometry (TeLineSet& lineSet);
	bool fetchGeometry (TeNode& n);
	bool fetchGeometry (TeNode& n, const unsigned int& initIndex);
	bool fetchGeometry (TePoint& p);
	bool fetchGeometry (TePoint& p, const unsigned int& initIndex);
	bool fetchGeometry (TePointSet& pointSet);
	bool fetchGeometry (TeCell& cell);
	bool fetchGeometry (TeCell& cell, const unsigned int& initIndex);
	//@}
};


/**
 * @brief This is the class for TeOracleSpatial driver factory.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup DatabaseUtils
 */
class TLORACLE_DLL TeOracleSpatialFactory : public TeDatabaseFactory
{
  public :
      
    /**
     * Default constructor
     */
    TeOracleSpatialFactory() : TeDatabaseFactory( 
      std::string( "OracleSpatial" ) ) {};      
      
    /**
     * Default Destructor
     */
    ~TeOracleSpatialFactory() {};
      
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
      TeOracleSpatial* instance_ptr = new TeOracleSpatial();
      
      if( arg.host_ != "" ) {
        instance_ptr->connect( arg.host_, arg.user_, arg.password_,
          arg.database_, arg.port_ );
      }
  
      return (TeDatabase*)instance_ptr;
    }
};

namespace {
  static TeOracleSpatialFactory TeOracleSpatialFactory_instance;
}; 

#endif 



