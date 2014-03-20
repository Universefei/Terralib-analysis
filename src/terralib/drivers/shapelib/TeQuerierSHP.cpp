
#include "TeQuerierSHP.h"
#include "TeSTEFunctionsSHP.h"
#include "TeSTInstance.h"

bool 
TeQuerierSHP::loadInstances(TeTSEntry* ) 
{ 
	// Read some information about the shapefile
	string filePrefix = TeGetName(params_->fileName().c_str());
	string shpfileName = filePrefix + ".shp";

	hSHP = SHPOpen( shpfileName.c_str(), "rb" );
	
	if( hSHP == 0 )
		return false;

	double 	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
	
	TeBox b(adfMinBound[0], adfMinBound[1], adfMaxBound[0], adfMaxBound[1]);
	params_->box(b);

	string dbffileName = filePrefix + ".dbf";
    hDBF = DBFOpen( dbffileName.c_str(), "rb" );
    if( hDBF == 0  || DBFGetFieldCount(hDBF) == 0)
		return false;
    
	//load attribute list
	TeReadDBFAttributeList(shpfileName, (*attrList_));

	TeAttributeRep repobjid;
	repobjid.name_ = "object_id";
	repobjid.numChar_ = 16;

	TeAttribute attobjid;
	attobjid.rep_ = repobjid;

	//insert objectId attribute in attribute list
	attrList_->push_back(attobjid); 
    curEntity = 0;
	return true;
}
		
bool 
TeQuerierSHP::fetchInstance(TeSTInstance& stoi ) 
{
	if(curEntity>(nEntities-1))
		return false;

	//clear stoi
	stoi.clear();

	TeAttributeRep repobjid;
	repobjid.name_ = "object_id";
	repobjid.numChar_ = 16;
	TeAttribute attobjid;
	attobjid.rep_ = repobjid;
	
	TeProperty propobjid;
	propobjid.attr_ = attobjid;
	
	SHPObject* psShape;
	
	string objectid = Te2String(curEntity);
	stoi.objectId(objectid);
	propobjid.value_ = objectid;
		
	TePropertyVector prop;
	psShape = SHPReadObject(hSHP,curEntity);
		
	if (TeDecodeShape(psShape,stoi.geometries(),objectid))
	{
		int numCols = attrList_->size()-1; //the last column is the object id
		for(int n=0;n<numCols;n++)
		{
			string value = DBFReadStringAttribute(hDBF,curEntity,n);
			TeProperty p;
			p.attr_ = (*attrList_)[n];
			p.value_ = value;
			prop.push_back(p);
		}
		prop.push_back(propobjid); // add the object id
		stoi.properties(prop);
		stoi.theme(0);
	}
	SHPDestroyObject(psShape);
	++curEntity;
	return true; 
} 

bool 
TeQuerierSHP::loadGeometries(TeMultiGeometry&, unsigned int&) 
{ 
	return false; 
}

bool 
TeQuerierSHP::loadGeometries(TeMultiGeometry&) 
{ 
	return false; 
}



