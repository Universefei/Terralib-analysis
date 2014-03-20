#ifndef __TESFSTHEME_H
#define __TESFSTHEME_H

// TerraLib
#include "TeDefines.h"
#include "TeAbstractTheme.h"

// forward declarations
class TeDatabase;
class TeDatabasePortal;


class TL_DLL TeSFSTheme : public TeAbstractTheme
{

public:

	TeSFSTheme(const std::string& name = "", TeViewNode* parent = 0, int view = 0, int id = 0);

	virtual ~TeSFSTheme();

	void setLocalDatabase(TeDatabase* database);

	void setRemoteDatabaseId(const int& remoteDbId);

	void setRemoteDatabase(TeDatabase* remoteDb);

	TeDatabase* getLocalDatabase();

	void setThemeProjection(TeProjection* proj);

	void getAttributeList(TeAttributeList& attrList);

	//! Returns a pointer to a projection that is the spatial reference for the objects of this theme
	/*! Concrete classes should reimplement this method.*/
	virtual TeProjection* getThemeProjection();

	virtual TeDatabasePortal* getQueryPortal(const TeBox& box, int& spatialColumnIndex, TeGeomRep& geomRep);

	virtual void setSFSTableInfo(const std::string& tableName, const std::string& columnName);

	//! Save the theme parameters
	virtual bool save();

	//! Save the grouping parameters in memory when there is no chronon
	virtual bool buildGrouping(const TeGrouping& g, TeSelectedObjects selectedObjects = TeAll,
		               vector<double>* dValuesVec = 0);

	//! Save the grouping parameters in memory when there is chronon
	virtual bool buildGrouping(const TeGrouping& g, TeChronon chr, vector<map<string, string> >& mapObjValVec);
	
	
	//! Build the grouping and associate each object to its group  
	virtual bool saveGrouping(TeSelectedObjects selectedObjects = TeAll);

	//! Delete grouping
	virtual bool deleteGrouping(); 

	//! Set the legend id for each object of the theme 
	virtual void setLegendsForObjects();

	//! Set the own legend id for each object of the theme 
	virtual void setOwnLegendsForObjects();

	/** @name Locate geometries
	    Returns the geometry(ies) of the theme given coordinate
	*/
	//@{ 	
	virtual bool locatePolygon(TeCoord2D &pt, TePolygon &polygon, const double& tol = 0.0);
	virtual bool locatePolygonSet(TeCoord2D &pt, double tol, TePolygonSet &polygons);
	virtual bool locateLine(TeCoord2D &pt, TeLine2D &line, const double& tol = 0.0);
	virtual bool locatePoint(TeCoord2D &pt, TePoint &point, const double& tol = 0.0);
	virtual bool locateCell(TeCoord2D &pt, TeCell &c, const double& tol = 0.0);
	//@}

	//! Get the set of objects corresponding to the object selection criteria
	virtual set<string> getObjects(TeSelectedObjects selectedObjects = TeAll);

	//! Get the set of objects corresponding to the list of items
	virtual set<string> getObjects(const vector<string>& itemVec);

	//! Get the set of items corresponding to the object selection criteria
	virtual vector<string> getItemVector(TeSelectedObjects selectedObjects);

	//! Get the set of items corresponding to the set of objects
	virtual vector<string> getItemVector(const set<string>& oidSet);

	//! Get the number of objects acessible by this theme
	virtual unsigned int getNumberOfObjects(); 

	//! Save (insert or update) the theme metadata in the database
	virtual bool saveMetadata(TeDatabase*);

	//! Load the theme metadata from database
	virtual bool loadMetadata(TeDatabase*);

	static bool createSFSThemeTable(TeDatabase* sourceDB);

	bool getRemoteThemeInfo(int& databaseId, std::string& tableName, std::string& columnName, int& geomRep, int& projId);

protected:

	//! Erase the theme metadata in the database
	virtual bool eraseMetadata(TeDatabase*);
	
protected:

	std::string			_tableName;
	std::string			_columnName;
	TeDatabase*			_localDB;
	TeDatabase*			_remoteDb;
	int					_remoteDbId;
	TeProjection*		_projection;
	TeAttributeList		_attrList;
	TeAttributeList		_originalAttrList;

};

//!  This class implements a factory to create remote theme objects. 
class TL_DLL TeSFSThemeFactory : public TeViewNodeFactory
{

public:
	
	//! Constructor 
	TeSFSThemeFactory() : TeViewNodeFactory((int)TeSFSTHEME)
	{}

	//! Created theme objects 
	TeViewNode* build(TeViewNodeParams* params)
	{	
		TeViewNodeParams auxParams = *params;
		return new TeSFSTheme(auxParams.name_, auxParams.myParent_, auxParams.viewId_, auxParams.id_);
	}
	
	TeViewNode* build()
	{
		return new TeSFSTheme();
	}
};

namespace 
{
  static TeSFSThemeFactory sfsThemeFactory;
}; 

#endif //__TESFSTHEME_H
