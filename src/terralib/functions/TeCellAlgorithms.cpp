/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.

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

#include "TeCellAlgorithms.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeDatabase.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeRepresentation.h"
#include "../kernel/TeGeneralizedProxMatrix.h"
#include "../kernel/TeOverlay.h"
#include "../kernel/TeQuerier.h"
#include "../kernel/TeQuerierParams.h"
#include "../kernel/TeSTEFunctionsDB.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeSTEvent.h"

using namespace TeOVERLAY;

// functions for working with the progress bar

void TeFillCellStepLoadingData()
{
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(2);
		TeProgress::instance()->setMessage("Step 1/3: Loading data");
		TeProgress::instance()->setProgress(1);
	}
	return;
}

void TeFillCellStepStartProcessing(int steps)
{
	if (TeProgress::instance())
	{
		TeProgress::instance()->reset();
		TeProgress::instance()->setTotalSteps(steps);
		TeProgress::instance()->setMessage("Step 2/3: Processing");
	}
}

bool TeFillCellStepNextStep(int step)
{
	if(TeProgress::instance())
	{
		if (TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			return false;
		}
		else
			TeProgress::instance()->setProgress(step);
	}
	return true;
}

void TeFillCellStepWriting()
{
	if (TeProgress::instance())
	{
		TeProgress::instance()->reset();
		TeProgress::instance()->setMessage("Step 3/3: Writing results");
	}
}

// TeBoxRasterIterator functions
TeBoxRasterIterator::TeBoxRasterIterator(TeRaster* raster_, TeBox box) 
{ 
	raster = raster_;
	TeCoord2D l = raster->coord2Index(box.lowerLeft());
	TeCoord2D r = raster->coord2Index(box.upperRight());

	xo = (int)floor(l.x()); if(xo < 0) xo = 0;
	yo = (int)floor(r.y()); if(yo < 0) yo = 0;
	xf = (int)floor(r.x()); if(xf >= raster->params().ncols_)   xf = raster->params().ncols_ - 1;
	yf = (int)floor(l.y()); if(yf >= raster->params().nlines_)  yf = raster->params().nlines_  - 1;

	// in the case where the cell does not have intersection with any pixel of the raster
	end_ = false;
	if(xo >= raster->params().ncols_)  end_ = true;
	if(yo >= raster->params().nlines_) end_ = true;
	if(xf <= 0)                        end_ = true;
	if(yf <= 0)                        end_ = true;
}


TeBoxRasterIterator&
TeBoxRasterIterator::operator++() 
{ 
	x++;
	if(x > xf)
	{
		x = xo;
		y++;
	}
	return *this;
}


TeBoxRasterIterator 
TeBoxRasterIterator::operator++(int) 
{
	x++;
	if(x >= xf)
	{
		x = xo;
		y++;
	}
	return *this;
}

double 
TeBoxRasterIterator::operator*() 
{ 
	double value;
	raster->getElement (x, y, value, 0);
	return value;
}

bool 
TeBoxRasterIterator::operator==(const TeBoxRasterIterator& rhs) const
{
	return (this->x == rhs.x && this->y == rhs.y);
}

bool 
TeBoxRasterIterator::operator!=(const TeBoxRasterIterator& rhs) const
{
    return (this->x != rhs.x || this->y != rhs.y);
}

bool 
TeBoxRasterIterator::getProperty(TeProperty& prop) 
{
	double value = 0.0;
    prop.attr_.rep_.type_ = TeREAL;
    prop.attr_.rep_.name_ = raster->params().fileName_;
    prop.value_ = Te2String (value);

	raster->getElement (x, y, value, 0);
	prop.value_ = Te2String(value);
    return true;
}

TeBoxRasterIterator 
TeBoxRasterIterator::begin()
{
	if(end_) 
		return end();
	x = xo;
	y = yo;
	return *this;
}

TeBoxRasterIterator 
TeBoxRasterIterator::end()
{
	y = yf+1;
	x = xo;
	return *this;
}

TeLayer* 
TeCreateCells( const string& layerName, TeLayer* layerBase, 
			   double resX, double resY, TeBox& box, bool mask)
{
	if (!layerBase || layerName.empty())
		return 0;

	TeDatabase* db = layerBase->database();
	if (!db)
		return 0;

	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return 0;

	TeProjection* proj = layerBase->projection();

	string newLayerName = layerName;
	TeLayerMap& layerMap = db->layerMap();
	TeLayerMap::iterator it;
	bool flag = true;
	int n = 1;
	while (flag)
	{
		for (it = layerMap.begin(); it != layerMap.end(); ++it)
		{
			if (TeStringCompare(it->second->name(),newLayerName))
				break;
		}
		if (it == layerMap.end())
			flag = 0;
		else
			newLayerName = layerName + "_" +Te2String(n);
		n++;	
	}

	string polTableName;
	if (mask)
	{
		TeRepresentation* repp = layerBase->getRepresentation(TePOLYGONS);
		if (repp)
		{
			if (!box.isValid())
				box = adjustToCut(repp->box_,resX,resY);
			polTableName = repp->tableName_;
		}
		else
			mask = false;
	}
	else 
	{
		if (!box.isValid())
			box = adjustToCut(layerBase->box(),resX,resY);
	}

	TeBox newBox = adjustToCut(box,resX,resY);
	double x,y,x1,x2,y1,y2;
	x1 = newBox.x1_;
	y1 = newBox.y1_;
	x2 = newBox.x2_;
	y2 = newBox.y2_;

	int maxcols, maxlines;
	maxlines = TeRound((y2-y1)/resY);
	maxcols = TeRound((x2-x1)/resX);

	TeAttribute attribute;
	TeAttributeList attList;
	attribute.rep_.name_ = "object_id";
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.numChar_ = 48;
	attribute.rep_.isPrimaryKey_ = true;
	attList.push_back ( attribute );
	attribute.rep_.name_ = "Col";
	attribute.rep_.type_ = TeINT;
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back ( attribute );
	attribute.rep_.name_ = "Lin";
	attribute.rep_.type_ = TeINT;
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back ( attribute );	

	TeLayer* newLayer = new TeLayer(layerName,db,newBox,proj);
	if (!newLayer || newLayer->id() <= 0)
	{
		delete portal;
		return 0;
	}
	newLayer->addGeometry(TeCELLS);
	TeRepresentation* repp = newLayer->getRepresentation(TeCELLS);
	if (!repp)
	{
		db->deleteLayer(newLayer->id());
		delete portal;
		return 0;
	}
	repp->box_ = newBox;
	repp->resX_ = resX;
	repp->resY_ = resY;
	repp->nCols_ = maxcols;
	repp->nLins_ = maxlines;
	db->updateRepresentation(newLayer->id(),*repp);

	TeTable attTable (layerName,attList,"object_id","object_id",TeAttrStatic);
	
	newLayer->createAttributeTable(attTable);

	TeCellSet cells;
	cells.resX(resX);
	cells.resY(resY);

	int col,lin,ncels=0;
	bool status;
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(maxlines);

	TePolygon curPol;
	bool found = false;

	TePrecision::instance().setPrecision(TeGetPrecision(proj));
	
	y=y2;
	for (lin=0; lin<maxlines; ++lin) 
	{	
		double yu = y;
		y=y-resY;
		x=x1;

		for (col=0; col<maxcols; ++col)
		{
			TeBox box(x,y,x+resX,yu);
			found = false;
			// check if there should be used the polygon representation as a mask
			if (mask) 
			{
				TePolygon polBox = polygonFromBox(box);
				// check if the cell intersects the current polygon
				if (TeIntersects(polBox,curPol))
					found = true;
				else
				{
					if (db->spatialRelation(polTableName, TePOLYGONS, 
						(TePolygon*)&polBox, portal, TeINTERSECTS))
					{
						curPol.clear();
						portal->fetchGeometry(curPol);
						found = true;
					}
					portal->freeResult();
				}
			}

			if (!mask || found)
			{
				// build geometry
				TeCell cell(box,col,lin);
				char celId[32];
				sprintf(celId,"C%02dL%02d",col,lin);
				cell.objectId(string(celId));
				cells.add(cell);
				
				// build default attributes
				TeTableRow row;
				row.push_back(cell.objectId()); 
				row.push_back(Te2String(col)); 
				row.push_back(Te2String(lin)); 
				attTable.add(row);
				ncels++;
			}
			x=x+resX;
		}
		if (attTable.size() > 0)	// if there is some attributes in this line
		{
			status = newLayer->saveAttributeTable(attTable);
			attTable.clear();
			status = newLayer->addCells(cells);
			cells.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(lin);
			}			
		}
	}
	delete portal;
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	if (ncels > 0)
		return newLayer;
	else
	{
		db->deleteLayer(newLayer->id());
		return 0;
	}
}



	
TeLayer*
TeCreateCells(const string& layerName,TeTheme* theme, double resX, double resY, TeBox& box)
{
	TeLayer* inputLayer = theme->layer();
	TeRepresentation* pp = inputLayer->getRepresentation(TePOLYGONS);
	if (!pp)
		return 0;

	TeDatabase* db = inputLayer->database();
	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return 0;

	string newLayerName = layerName;
	TeLayerMap& layerMap = db->layerMap();
	TeLayerMap::iterator it;
	bool flag = true;
	int n = 1;
	while (flag)
	{
		for (it = layerMap.begin(); it != layerMap.end(); ++it)
		{
			if (TeStringCompare(it->second->name(),newLayerName))
				break;
		}
		if (it == layerMap.end())
			flag = 0;
		else
			newLayerName = layerName + "_" +Te2String(n);
		n++;	
	}

	if (!box.isValid())
		box = pp->box_;

	TeBox newBox = adjustToCut(box,resX,resY);
	double x,y,x1,x2,y1,y2;
	x1 = newBox.x1_;
	y1 = newBox.y1_;
	x2 = newBox.x2_;
	y2 = newBox.y2_;

	int maxcols, maxlines;
	maxlines = TeRound((y2-y1)/resY);
	maxcols = TeRound((x2-x1)/resX);

	TeAttribute attribute;
	TeAttributeList attList;
	attribute.rep_.name_ = "object_id";
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.isPrimaryKey_ = true;
	attribute.rep_.numChar_ = 48;
	attList.push_back ( attribute );

	attribute.rep_.name_ = "Col";
	attribute.rep_.type_ = TeINT;
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back ( attribute );

	attribute.rep_.name_ = "Lin";
	attribute.rep_.type_ = TeINT;
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back ( attribute );	

	TeLayer* newLayer = new TeLayer(layerName,db,newBox,inputLayer->projection());
	if (!newLayer || newLayer->id() <= 0)
	{
		return 0;
		delete portal;
	}
	newLayer->addGeometry(TeCELLS);
	TeRepresentation* repp = newLayer->getRepresentation(TeCELLS);
	if (!repp)
	{
		db->deleteLayer(newLayer->id());
		delete portal;
		return 0;
	}
	repp->box_ = newBox;
	repp->resX_ = resX;
	repp->resY_ = resY;
	repp->nCols_ = maxcols;
	repp->nLins_ = maxlines;
	db->updateRepresentation(newLayer->id(),*repp);

	TeTable attTable (layerName,attList,"object_id","object_id",TeAttrStatic);
	newLayer->createAttributeTable(attTable);

	string polTableName = inputLayer->tableName(TePOLYGONS);

	TeCellSet cells;
	cells.resX(resX);
	cells.resY(resY);

	TePolygon curPol;
	int col,lin;
	bool status;
	int ncels = 0;

	TePrecision::instance().setPrecision(TeGetPrecision(newLayer->projection()));

	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(maxlines);

	bool found;
	y=y2;
	for (lin=0; lin<maxlines; ++lin) 
	{	
		double yu = y;
		y=y-resY;
		x=x1;
		for (col=0; col<maxcols; ++col)
		{
			found = false;
			TeBox box(x,y,x+resX,yu);

			TePolygon polBox = polygonFromBox(box);
			// check if the cell intersects the current polygon
			if (TeIntersects(polBox,curPol))
				found = true;
			else
			{
				if (db->spatialRelation(polTableName, TePOLYGONS, 
					(TePolygon*)&polBox, portal, TeINTERSECTS, theme->collectionTable()))
				{
					curPol.clear();
					portal->fetchGeometry(curPol);
					found = true;
				}
				portal->freeResult();
			}
			if (found)
			{
				TeCell cell(box,col,lin);
				char celId[32];
				sprintf(celId,"C%02dL%02d",col,lin);
				cell.objectId(string(celId));
				cells.add(cell);

				TeTableRow row;
				row.push_back(cell.objectId()); 
				row.push_back(Te2String(col)); 
				row.push_back(Te2String(lin)); 
				attTable.add(row);
				++ncels;		
			}
			x=x+resX;
		}
		if (attTable.size() > 0)	// if there is some attributes in this line
		{
			status = newLayer->saveAttributeTable(attTable);
			attTable.clear();
			status = newLayer->addCells(cells);
			cells.clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				else
					TeProgress::instance()->setProgress(lin);
			}	
		}
	}
	delete portal;
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	if (ncels > 0)
		return newLayer;
	else
	{
		db->deleteLayer(newLayer->id());
		return 0;
	}
}


bool 
TeCellStatistics(TeTheme* themeOut, TeTable& newAttrTable, TeGroupingAttr& stat, 
				 TeTheme* themeIn, TeDatabase* db)
{

	TeGroupingAttr tempStat = stat;
	string sqlResut = db->getSQLStatistics (tempStat);

	bool dbStat = true;
	TeGroupingAttr::iterator it = tempStat.begin();
	while(it!= tempStat.end())
	{
		if(it->second != TeNOSTATISTIC)
		{
			dbStat = false;
			break;
		}
		++it;
	}
		
	if((!dbStat) || (sqlResut.empty()))
		return false; 

	TeDatabasePortal* portal = db->getPortal();

	// mount sql 
	//geom and collection tables from cell layer (cell)
	string tableGeomOut = themeOut->layer()->tableName (TeCELLS);
	string tableCollOut; 
	if(themeOut->hasRestriction())
		tableCollOut = themeOut->collectionTable();

	//geom and collection tables from in layer (point or cell)
	string tableGeomIn = themeIn->layer()->tableName (TePOINTS);
	TeGeomRep geomRepIn = TePOINTS;
	if(tableGeomIn.empty())
	{
		geomRepIn = TeCELLS;
		tableGeomIn = themeIn->layer()->tableName (TeCELLS);
	}

	string tableCollIn; 
	if(themeIn->hasRestriction())
		tableCollIn = themeIn->collectionTable();

	//sql from database
	string sql = " SELECT " + tableGeomOut + ".object_id,  ";
	sql += 	sqlResut;
	sql += " FROM " + tableGeomOut +","+ tableGeomIn;

	//for each point attr table 
	string sqlWhere ="";
	TeAttrTableVector vec = themeIn->attrTables();
	for(unsigned int index=0; index<vec.size(); ++index)
	{
		if( (vec[index].tableType() != TeAttrStatic) && 
			(vec[index].tableType() != TeAttrEvent)  &&
			(vec[index].tableType() != TeFixedGeomDynAttr)) //only to static attribute table
			continue;

		//from clause
		sql += ","+ vec[index].name();

		//where clause
		if(!sqlWhere.empty())
			sqlWhere += " AND ";
		sqlWhere += vec[index].name() +"."+ vec[index].linkName() +" = "+ tableGeomIn +".object_id ";
	}

	if(!tableCollIn.empty())
	{
		sql += ", "+ tableCollIn;
		if(!sqlWhere.empty())
			sqlWhere += " AND ";
	
		sqlWhere = tableGeomIn +".object_id = "+ tableCollIn +".c_object_id ";
	}
	if(!tableCollOut.empty())
	{
		sql += ", "+ tableCollOut;
		if(!sqlWhere.empty())
			sqlWhere += " AND ";
	
		sqlWhere += tableGeomOut +".object_id = "+ tableCollOut +".c_object_id ";
	}

	//box where
	if(!sqlWhere.empty())
		sqlWhere += " AND ";
	
	sqlWhere += db->getSQLBoxWhere (tableGeomOut, tableGeomIn, geomRepIn);

	sql += " WHERE "+ sqlWhere; 
	sql += " GROUP BY "+ tableGeomOut +".object_id";

	if(!portal->query(sql))
	{
		delete portal;
		return false;
	}
	
	//Keep statistics in the tableAttrCell table   
	if(!TeKeepStatistics(newAttrTable, portal))
	{
		delete portal;
		return false;
	}
	
	delete portal;
	return true;
}


bool	
TeKeepStatistics(TeTable& tableAttrCell, TeDatabasePortal* portal)
{
	
	TeAttributeList attr = portal->getAttributeList();

	//Mount the update sql from portal
	string insert = "INSERT INTO "+ tableAttrCell.name() +" VALUES ( ";
	while(portal->fetchRow())
	{
		string insert2 = "";
		for(unsigned int i=0; i<attr.size(); ++i)
		{
			if(i>0)
				insert2 += ",";
			
			string val = portal->getData(i);
			if (attr[i].rep_.type_ == TeSTRING) 
				insert2 += "'"+ val +"'";
			else
				insert2 += val;
		}

		string result = insert + insert2 +" )";
		
		if(!portal->getDatabase()->execute(result))
			return false;
	}
		
	return true;
}	


//////////////////////////////////////////////////////////////////////
//
//				Fill Cell Auxiliary Funcions
//
/////////////////////////////////////////////////////////////////////


bool TeFillCellInitLoad (TeLayer* cell_layer, const string& cell_tablename, TeCellSet& cells)
{
	if (!cell_layer) return false;

	// get cells
	if (!cell_layer->getCells (cells)) return false;

	// if dynamic table was not created, create it
	TeTable table;
	if (!cell_layer->getAttrTablesByName(cell_tablename, table, TeFixedGeomDynAttr))
		if (!TeCreateBasicDynamicCellTable (cell_layer, cell_tablename))
			return false;

	return true;
}


void TeFillCellInitSTO (const TeCell& cell, TePropertyVector& result, TeSTElementSet& cellObjSet, TeAttrTableType atttype, TeTimeInterval& t)
{
	TeSTInstance cellObj;
	cellObj.objectId (cell.objectId());
	string uniqueId = cell.objectId();
	cellObj.addUniqueId (uniqueId); 
	
	if(atttype != TeAttrStatic)
	{
		uniqueId += t.getInitialDate() + t.getInitialTime();
		uniqueId += t.getFinalDate()   + t.getFinalTime  ();

		cellObj.addUniqueId (uniqueId);
		cellObj.timeInterval (t);
	}

	TePropertyVector::iterator itProp = result.begin();
	while (itProp != result.end())
	{
		cellObj.addPropertyValue (itProp->value_);
		itProp++;
	}
	cellObjSet.insertSTInstance (cellObj);
	return;
}


void TeFillCellInitSTO (const TeCell& cell, vector<string>& result, TeSTElementSet& cellObjSet, TeAttrTableType atttype, TeTimeInterval& t)
{
	TeSTInstance cellObj;
	cellObj.objectId (cell.objectId());
	string uniqueId = cell.objectId();
	cellObj.addUniqueId (uniqueId); 

	if(atttype != TeAttrStatic)
	{
		uniqueId += t.getInitialDate() + t.getInitialTime();
		uniqueId += t.getFinalDate()   + t.getFinalTime  ();

		cellObj.addUniqueId (uniqueId);
		cellObj.timeInterval (t);
	}

	cellObj.setProperties(result);
	cellObjSet.insertSTInstance (cellObj);
	return;
}


bool
TeCreateBasicDynamicCellTable (TeLayer* cell_layer, const string cell_tablename)
{
	if (!cell_layer) return false;

	TeAttribute		attribute;
	TeAttributeList attList;
	TeAttributeList keyList;

	// attr_id
	attribute.rep_.name_ = "attr_id";
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.numChar_ = 48;
	attribute.rep_.isPrimaryKey_ = true;
	attList.push_back (attribute);
	keyList.push_back(attribute);

	// object_id
	attribute.rep_.name_ = "object_id";
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.isPrimaryKey_ = false;
	attribute.rep_.numChar_ = 48;
	attList.push_back ( attribute );
	
	//  initial_time
	attribute.rep_.name_ = "initial_time";
	attribute.rep_.type_ = TeDATETIME;   
	attribute.dateTimeFormat_ = "YYYYsMMsDDsHHsmmsSS";
	attribute.dateChronon_ = TeSECOND;
	attribute.rep_.numChar_ = 48;
	attList.push_back ( attribute );

	//  final_time
	attribute.rep_.name_ = "final_time";
 	attribute.rep_.type_ = TeDATETIME;  
	attribute.dateTimeFormat_ = "YYYYsMMsDDsHHsmmsSS";
	attribute.dateChronon_ = TeSECOND;
	attribute.rep_.numChar_ = 48;
	attList.push_back ( attribute );
		
	// Create table and initialize attributes
	TeTable cells_attTable (cell_tablename);       
	cells_attTable.setAttributeList(attList);
	cells_attTable.setTableType(TeFixedGeomDynAttr);
	cells_attTable.setLinkName("object_id");
	cells_attTable.setUniqueName("attr_id");
	cells_attTable.attInitialTime ("initial_time"); 
	cells_attTable.attFinalTime ("final_time"); 
	cells_attTable.attTimeUnit (TeSECOND); 
			
	if (!cell_layer->createAttributeTable(cells_attTable))
		return false;

	TeAttributeList attr; 
	cell_layer->database ()->getAttributeList (cell_tablename, attr);

	return true;
}

//////////////////////////////////////////////////////////////////////
//
//					Fill Cell Operations
//
/////////////////////////////////////////////////////////////////////

struct cellDist
{
	double distance;
	TeCellSet::iterator ref;
	bool done;
	bool valid;
};


vector<vector<cellDist> > BuildNeighbors(TeCellSet cells)
{
	vector<vector<cellDist> > result;
	int line = 0, column = 0;

	// look for the number of lines and columns of the set
	for(TeCellSet::iterator cell_it = cells.begin(); cell_it != cells.end(); cell_it++)
	{
		if( (*cell_it).column() > column) column = (*cell_it).column();
		if( (*cell_it).line  () > line)   line   = (*cell_it).line  ();
	} 
	
	line++;
	column++;

	// initialize the neighbors	
	result.resize(line);
	for(int i = 0; i < line; i++)
	{
		result[i].resize(column);
		for(int j = 0; j < column; j++)
		{
			result[i][j].valid    = false;
			result[i][j].done     = false;
		    result[i][j].distance = 0;
		}
	}

	// set the values for the ones that have a cell
	for(TeCellSet::iterator cell_it = cells.begin(); cell_it != cells.end(); cell_it++)
	{
		result[ (*cell_it).line() ][ (*cell_it).column() ].ref   = cell_it;
		result[ (*cell_it).line() ][ (*cell_it).column() ].valid = true;
	} 
	
	return result;
}

struct procDist
{
	int line;
	int column;
	TeBox box;
};


bool CalculateDistance(unsigned int line, unsigned int column, TeBox box, TeTheme* theme, double r, TeGeomRep rep, vector<vector<cellDist> >& Neighbors, list<procDist>& procList)
{
	if( Neighbors[line][column].done || !Neighbors[line][column].valid )
	{
		return false;
	}

	TeCellSet::iterator cell_it = Neighbors[line][column].ref; 
	
	TeQuerierParams params(true, false);
	params.setParams (theme);
    params.setSpatialRest(box, TeWITHIN, rep);

	TeQuerier querier(params);

    querier.loadInstances();
	int quant = querier.numElemInstances();
	TeCoord2D p1 = (*cell_it).box().center();

	double dist = TeMAXFLOAT;

	if(rep == TePOINTS)
	{
		for(int i = 0; i < quant; i++)
		{
		    TeSTInstance sti;
		    querier.fetchInstance(sti);
			TeMultiGeometry mg = sti.geometries();
	
			double x = TeDistance(p1, mg.getPoints()[0].location());
			if (dist > x) dist = x;
		}
	}
	if(rep == TeLINES)
	{
		for(int i = 0; i < quant; i++)
		{
		    TeSTInstance sti;
		    querier.fetchInstance(sti);
			TeMultiGeometry mg = sti.geometries();

			TeLineSet ls = mg.getLines();

    		int line_index;
		    TeCoord2D pinter;
			double x;
    		TeNearest (p1, ls, line_index, pinter, x);
            if (dist > x) dist = x;
		}
	}
	if(rep == TePOLYGONS)
	{
		for(int i = 0; i < quant; i++)
		{
		    TeSTInstance sti;
		    querier.fetchInstance(sti);
			TeMultiGeometry mg = sti.geometries();
			TePolygonSet ps = mg.getPolygons();

		    TePolygonSet box_ps, intersect;
		    TePolygon pol = polygonFromBox((*cell_it).box());
		    box_ps.add(pol);

		    TeOverlay(box_ps, ps, intersect, TeINTERSECTION);
			unsigned int kk = 0;
		    if(!intersect.empty()) { dist = 0; kk = ps.size(); }
		   
			for (; kk < ps.size(); kk++)
			{
				TePolygon p = ps[kk];
				for (unsigned int j = 0; j < p.size(); j++)
				{
					TeLinearRing l = p[j];
					for (unsigned int k = 0; k < l.size(); k++)
					{
						double d = 	TeDistance (p1, l[k]);
						if (dist > d) dist = d;
					}
				}
			}
		}
	}

	Neighbors[line][column].done     = true; 
	Neighbors[line][column].distance = dist; 

	TeBox searchbox(p1.x() - dist - 1.001*r, p1.y() - dist - 1.001*r,
                    p1.x() + dist + 1.001*r, p1.y() + dist + 1.001*r);

	procDist pd;
	pd.box = searchbox;
	if(line > 0)                         { pd.line = line-1; pd.column = column;   procList.push_back(pd); }
	if(column > 0)                       { pd.line = line;   pd.column = column-1; procList.push_back(pd); }
    if(line < Neighbors.size() - 1)      { pd.line = line+1; pd.column = column;   procList.push_back(pd); }
	if(column < Neighbors[0].size() - 1) { pd.line = line;   pd.column = column+1; procList.push_back(pd); }
	
	return true;
}

bool TeFillCellDistanceOperation(TeTheme* theme,
								 TeGeomRep rep,
								 TeLayer* cell_layer,
								 const string& cell_tablename,
								 const string& output_columnName,
								 TeTimeInterval t)
{
	TeFillCellStepLoadingData();

	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	list<procDist> procList;
	vector<vector<cellDist> > Neighbors = BuildNeighbors(cells);

	TeFillCellStepStartProcessing(cells.size()*2);

	TePropertyVector result;
	TeCellSet::iterator cell_it = cells.begin();

	double res = cells.resX();
	if( res < cells.resY () ) res = cells.resY();
	
	procDist pd;

	pd.line   = (*cell_it).line();
	pd.column = (*cell_it).column();
	pd.box    = theme->getThemeBox();

	procList.push_back(pd);
	int count = 0; // counting the progress

	while(!procList.empty())
	{
		pd = procList.front();
		procList.pop_front();
		if(CalculateDistance(pd.line, pd.column, pd.box, theme, res, rep, Neighbors, procList))
			if(!TeFillCellStepNextStep(++count)) return false;
	}

	TeTable table(cell_tablename);
	cell_layer->database()->loadTableInfo(table);

	TeSTElementSet cellObjSet (cell_layer);
	//include the attribute description
	TeAttribute attr;
	attr.rep_.type_ = TeREAL;
	attr.rep_.decimals_ = 10;
	attr.rep_.name_ = output_columnName;
	cellObjSet.addProperty(attr);

	while (cell_it != cells.end())
	{
	    string value_ = Te2String(Neighbors[(*cell_it).line()][(*cell_it).column()].distance);
		vector<string> attrValues;
		attrValues.push_back(value_);
	    
		TeFillCellInitSTO ((*cell_it), attrValues, cellObjSet, table.tableType(), t);
		cell_it++;

		if(!TeFillCellStepNextStep(++count)) return false;
	}
	
	TeFillCellStepWriting();

	if (!TeUpdateDBFromSet(&cellObjSet, cell_tablename))
			return  false;

	return true;
}

#include <sstream>

bool TeFillCellSpatialOperation(TeDatabase* db,
								TeComputeSpatialStrategy* operation,
								const string& cell_layername,
								const string& cell_tablename,
								const string& output_columnName,
								TeTimeInterval t)
{
	if (!db) return false;

	TeFillCellStepLoadingData();

	// Load input layer
	TeLayer* cell_layer = new TeLayer (cell_layername);
	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	TeTable table(cell_tablename);
	db->loadTableInfo(table);

	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	TeSTElementSet cellObjSet (cell_layer);
	//adds the new attribute description 
	TeAttribute attr;
	attr.rep_.type_ = TeREAL;
	attr.rep_.decimals_ = 10;
	attr.rep_.name_ = output_columnName;

	cellObjSet.addProperty(attr);

	TeFillCellStepStartProcessing(cells.size());
	TeCellSet::iterator cell_it = cells.begin();

	int count = 0;
	while (cell_it != cells.end())
	{
	    string value_ = operation->compute ((*cell_it).box());
	    vector<string> attrValues;
		attrValues.push_back (value_);
	   
		TeFillCellInitSTO ((*cell_it), attrValues, cellObjSet, table.tableType(), t);
		cell_it++;

		if(!TeFillCellStepNextStep(++count)) return false;
	} 

	TeFillCellStepWriting();
	bool result = TeUpdateDBFromSet(&cellObjSet, cell_tablename);
	cell_layer->loadLayerTables();

	return result;
}


bool TeFillCellCategoryCountPercentageRasterOperation(TeDatabase* db,
													  string raster_layername,
													  string cell_layername,
													  const string& cell_tablename,
													  const string attrName,
													  TeTimeInterval t)
{
	TeFillCellStepLoadingData();

	map<string, string> classesMap;
	vector< string > atts;
	string value;

	TeLayer* input_layer = new TeLayer (raster_layername);
	if (!db->loadLayer (input_layer))
	{
		 db->close();
		 return false;
	}

	atts.push_back(attrName);

	vector<double> values;
	bool found;
	unsigned int i;
	TeRaster* raster = input_layer->raster();
	TeRaster::iterator rasterItBegin = raster->begin();
	TeRaster::iterator rasterItEnd   = raster->end();

	TeTable table(cell_tablename);
	db->loadTableInfo(table);

	TeLayer* cell_layer = new TeLayer (cell_layername);
	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	// Initialize cell set
	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	// Initialize object set to store cell properties
	TeSTElementSet cellObjSet (cell_layer);

	// looking for the unique values in all the raster
	TeFillCellStepStartProcessing(raster->params().nlines_ * raster->params().ncols_ + cells.size());

	int count = 0;
	while(rasterItBegin != rasterItEnd)
	{
		found = false;
		for(i = 0; i < values.size(); ++i)
			if(values[i] == (*rasterItBegin)[0])
				found = true;
		if(!found) 
			values.push_back((*rasterItBegin)[0]);
		++rasterItBegin;

		if(!TeFillCellStepNextStep(++count)) return false;
	}

	// Process
	TePropertyVector result;
	map<double, int> counter;

	// set the propertyvector of the cell object set
	for(i = 0; i < values.size(); i++)
	{
		stringstream str;

		str << values[i];
        string value_ =  str.str();
        for (unsigned j = 0; j < value_.size(); j++)
            if(value_[j] == '.' || value_[j] == '+')
                value_[j] = '_';

        TeProperty prop;
        prop.attr_.rep_.name_ = attrName + value_;
        prop.attr_.rep_.type_ = TeREAL;
        prop.attr_.rep_.numChar_ = 48;
		prop.attr_.rep_.decimals_ = 10;

        cellObjSet.addProperty(prop.attr_);
	}

	TeCellSet::iterator cell_it = cells.begin();
	vector<string> vprop;

	while (cell_it != cells.end())
	{
		double total = 0.0;
		TePolygon p = polygonFromBox((*cell_it).box());
		TeRaster::iteratorPoly rasterItBegin(raster);
		rasterItBegin = raster->begin(p, TeBBoxPixelInters, 0);
		TeRaster::iteratorPoly rasterItEnd(raster);
		rasterItEnd = raster->end(p, TeBBoxPixelInters, 0);

		for(i = 0; i < values.size(); i++)
			counter[values[i]] = 0;

		while(rasterItBegin != rasterItEnd)
		{
			double pixel = (*rasterItBegin);
			counter[pixel]++;
			++rasterItBegin;
			total++;
		}		

		//add the attribute values in the object set
		vprop.clear();
		for(i = 0; i < values.size(); i++)
		{
			stringstream str;
			if (total > 0)
				str << counter[values[i]] / total;
			else
				str << "0.0";

			vprop.push_back(str.str());
		}

		TeFillCellInitSTO ((*cell_it), vprop, cellObjSet, table.tableType(), t);
		cell_it++;

		if(!TeFillCellStepNextStep(++count)) return false;
	} 

	TeFillCellStepWriting();
	// Update DB
	if (!TeUpdateDBFromSet (&cellObjSet, cell_tablename))
		return  false;
	return  true;
}


bool TeFillCellCategoryAreaPercentageOperation(TeTheme* theme,
											   const string attrName,
								 			   TeLayer* cell_layer,
											   const string& cell_tablename,
											   TeTimeInterval t)
{
	map<string, string> classesMap;
	vector< string > atts;
	TeQuerier* querier;
	TeSTInstance sti;

	atts.push_back(attrName);

 	TeQuerierParams params (false, atts);
	params.setParams (theme);

	querier = new TeQuerier(params);

	querier->loadInstances();

	while( querier->fetchInstance(sti) )
	{
		std::string value = sti.getPropertyVector()[0].value_;

		std::string replaceChars = TeReplaceSpecialChars(value);

		std::string removeChars = TeRemoveSpecialChars(replaceChars);

		classesMap[sti.getPropertyVector()[0].value_] = removeChars;

	}
	delete querier;

	return TeFillCellCategoryAreaPercentageOperation(theme, attrName, cell_layer, cell_tablename, classesMap, t);
}


bool TeFillCellCategoryAreaPercentageOperation(TeTheme* theme,
											   const string attrName,
								 			   TeLayer* cell_layer,
											   const string& cell_tablename,
											   map<string, string>& classesMap,
											   TeTimeInterval t)
{
	TeFillCellStepLoadingData();

	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	TeSTElementSet cellObjSet (cell_layer);

	TeTable table(cell_tablename);
	cell_layer->database()->loadTableInfo(table);

	TeCellSet::iterator cell_it = cells.begin();

	TeFillCellStepStartProcessing(cells.size());
	int count = 0;
	while (cell_it != cells.end())
	{
	    TeProperty  prop;

		// COMPUTE
    // convert input box to a polygonset
	    TePolygonSet box_ps;
	    TePolygon pol = polygonFromBox((*cell_it).box());
	    box_ps.add(pol);
	    map<string, double>  areaMap;
	    double partial_area = 0.0;

	    // Build stoset
	    TeSTElementSet stos (theme);
	    vector<string> attrNames;
	    attrNames.push_back (attrName); 
	    if (TeSTOSetBuildDB (&stos, true, false, attrNames))
        {   
        
            // initialize areMap
            map<string, string>::iterator itMap = classesMap.begin();
            while (itMap != classesMap.end())
            {
                areaMap[(*itMap).second] = 0;
                ++itMap;
            }
            partial_area = 0.0;
            // iterate over all objects that intercept the box
            TeSTElementSet::iterator it = stos.begin();
            while (it != stos.end())
            {
                string category;
                if ((*it).getPropertyValue (attrName, category))
                {   
                    TePolygonSet objGeom;  
                    if ((*it).getGeometry(objGeom))
                    {
                        TePolygonSet intersect;
                        TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
                        double intersectArea = TeGeometryArea (intersect);
                        partial_area += intersectArea;
                        areaMap[classesMap[category]] +=  intersectArea; // suppose objects are disjoint
                    }
                }
                ++it;
            }
        }

	    double areaTot = TeGeometryArea (box_ps);
	    double perc_total = 0.0;
	    double corrected_perc_total = 0.0;
	    vector<double> vec_percent;

		map <string, double>:: iterator areaIt = areaMap.begin();
	    while(areaIt != areaMap.end())  
	    {
	        double percent = 0.0;
	        if (areaTot != 0) percent = (*areaIt).second/areaTot;
	        perc_total += percent;
	        vec_percent.push_back (percent);
			
			//attribute list
			TeAttribute at;
			at.rep_.name_ = attrName + (*areaIt).first;
			at.rep_.type_ = TeREAL;
			at.rep_.decimals_ = 10;
			cellObjSet.addProperty(at);
	        ++areaIt;
	    }

		//fill the object set
		int i= 0;
	    areaIt = areaMap.begin();

		vector<string> values;

	    while(areaIt != areaMap.end())  
	    {
	        
			double percent = vec_percent[i];
	        corrected_perc_total += percent;
	        values.push_back(Te2String (percent));
	        ++areaIt;
	        i++;
	    }

		TeFillCellInitSTO ((*cell_it), values, cellObjSet, table.tableType(), t);

		cell_it++;
		if(!TeFillCellStepNextStep(++count)) return false;
	} 

	TeFillCellStepWriting();
	bool result = TeUpdateDBFromSet(&cellObjSet, cell_tablename);
	cell_layer->loadLayerTables();

	return result;
}

bool TeFillCellNonSpatialOperation (TeTheme* theme,
									TeGeomRep rep,
									const string& input_attrname,
									TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
									TeLayer* cell_layer,
									const string& cell_tablename, 
									const string& output_columnName,
									TeTimeInterval t)
{
	TeFillCellStepLoadingData();

	if (!theme)      return false;
	if (!cell_layer) return false;
	if (!operation)  return false;

	string s = theme->name();
	// Initialize cell set
	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	TeTable table(cell_tablename);
	cell_layer->database()->loadTableInfo(table);

	// Initialize object set to store cell properties
	TeSTElementSet cellObjSet (cell_layer);

	// Process
	TePropertyVector result;
	TeCellSet::iterator cell_it = cells.begin();
	bool firstIteration = true;
	vector<string> attrNames;
	attrNames.push_back (input_attrname);	
	
	TeSTElementSet stos (theme);
	if (!TeSTOSetBuildDB (&stos, true, false, attrNames))
		return false;
    
	TeFillCellStepStartProcessing(cells.size());
	int count = 0;	
	while (cell_it != cells.end())
	{
		vector<TeSTInstance*> boxResult;
		stos.search((*cell_it).box(), boxResult);

		TeSTElementSet elemSetTemp(theme, stos.getAttributeList());
		for(unsigned int i=0; i<boxResult.size(); ++i)
		{
            bool intersects = true;
			if(rep==TePOLYGONS) 
			{
				intersects = false;
				TePolygonSet	pSet;
				boxResult[i]->getGeometry(pSet);
				for(unsigned int j=0; j<pSet.size(); ++j)
				{
					if(TeIntersects(pSet[j],TeMakePolygon((*cell_it).box())))
					{
						intersects = true;
						j = pSet.size();
					}
				}
			}
			else if(rep==TeLINES) 
			{
				intersects = false;
				TeLineSet		lSet;
				boxResult[i]->getGeometry(lSet);
				for(unsigned int j=0; j<lSet.size(); ++j)
				{
					if(TeIntersects(lSet[j],TeMakePolygon((*cell_it).box())))
					{
						intersects = true;
						j = lSet.size();
					}
				}
			}
						
			if(intersects)
				elemSetTemp.insertSTInstance(*(boxResult[i]));
		}
		
		result = operation->compute (elemSetTemp.begin(0), elemSetTemp.end(0), output_columnName);
		
		// if property not found, stos iterator will return zero to operation. 
		// To check this, have to be less general and use getProperty inside the compute method.		
		if(firstIteration)
		{
			//add the attribute definitions in the object set, only once
			TePropertyVector::iterator it = result.begin();
			while(it!=result.end())
			{
                cellObjSet.addProperty(it->attr_);
				++it;
			}
		}

		//add the attribute values in the object set
		TePropertyVector::iterator it = result.begin();
		vector<string> values;
		while(it!=result.end())
		{
			values.push_back(it->value_);
			++it;
		}

		TeFillCellInitSTO ((*cell_it), values, cellObjSet, table.tableType(), t);
		cell_it++;
		firstIteration = false;

		if(!TeFillCellStepNextStep(++count)) return false;
	} 

	TeFillCellStepWriting();
	// Update DB
	if (!TeUpdateDBFromSet (&cellObjSet, cell_tablename))
		return  false;
	return  true;
}

bool TeFillCellNonSpatialOperation (TeDatabase* db,
										const string& input_layername, 
										TeGeomRep rep,
										const string& input_tablename, 
										const string& input_attrname,
										TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
										const string& cell_layername, 
										const string& cell_tablename, 
										const string& output_columnName,
										TeTimeInterval t)
{
	if (!db) return false;

	// Load input layers
	TeLayer* input_layer = new TeLayer (input_layername);
	if (!db->loadLayer (input_layer))
	{
		 db->close();
		 return false;
	}

	// Load output cells layer with geometry previously created
	TeLayer* cell_layer = new TeLayer (cell_layername);
	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	// Initialize theme
	TeTheme* theme = new TeTheme ("", input_layer);
	vector<string> attrTableNames;
	attrTableNames.push_back (input_tablename);
	TeAttrTableVector atts;
	if (!input_layer->getAttrTablesByName(attrTableNames, atts)) return false;
	theme->setAttTables (atts);

	bool ret = TeFillCellNonSpatialOperation (theme,
											  rep,
											  input_attrname,
											  operation,
											  cell_layer,
											  cell_tablename, 
											  output_columnName,
											  t);

	return ret;
}


bool TeFillCellNonSpatialRasterOperation (TeDatabase* db,
										  const string& input_raster_layername, 
										  TeComputeAttrStrategy<TeBoxRasterIterator>* operation,
										  const string& cell_layername, 
										  const string& cell_tablename, 
										  const string& output_columnName,
										  TeTimeInterval t)
{
	TeFillCellStepLoadingData();

	if (!db) return false;

	// Load input layers
	TeLayer* input_layer = new TeLayer (input_raster_layername);
	if (!db->loadLayer (input_layer))
	{
		 db->close();
		 return false;
	}

	TeTable table(cell_tablename);
	db->loadTableInfo(table);

	TeRaster* raster = input_layer->raster();
	if (!raster) return false;

	// Load output cells layer with geometry previously created
	TeLayer* cell_layer = new TeLayer (cell_layername);
	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	// Initialize cell set
	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	// Initialize object set to store cell properties
	TeSTElementSet cellObjSet (cell_layer);
	//include the attribute description
	TeAttribute attr;
	attr.rep_.type_ = TeREAL;
	attr.rep_.name_ = output_columnName;
	attr.rep_.decimals_ = 10;
	cellObjSet.addProperty(attr);

	TeFillCellStepStartProcessing(cells.size());

	// Process
	TePropertyVector result;
	TeCellSet::iterator cell_it = cells.begin();

	int count = 0;
	while (cell_it != cells.end())
	{
		TeBoxRasterIterator rasterItBegin(raster, (*cell_it).box());
		rasterItBegin.begin();
		TeBoxRasterIterator rasterItEnd(raster, (*cell_it).box());
		rasterItEnd.end();
		result = operation->compute (rasterItBegin, rasterItEnd, output_columnName);										
		TeFillCellInitSTO ((*cell_it), result, cellObjSet, table.tableType(), t);
		cell_it++;

		if(!TeFillCellStepNextStep(++count)) return false;
	} 

	TeFillCellStepWriting();
	// Update DB
	if (!TeUpdateDBFromSet (&cellObjSet, cell_tablename))
		return  false;
	return  true;
}


bool TeFillCellAggregateOperation ( TeDatabase* db,
									const string& input_layername, 
									const string& input_tablename, 
									TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
									vector<string>& attrNames,
									const string& cell_layername, 
									const string& cell_tablename, 
									TeTimeInterval t)
{
	if (!db) return false;

	// Load input layers
	TeLayer* input_layer = new TeLayer (input_layername);
	if (!db->loadLayer (input_layer))
	{
		 db->close();
		 return false;
	}

	// Load output cells layer with geometry previously created
	TeLayer* cell_layer = new TeLayer (cell_layername);

	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	// Initialize cell set
	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	// Initialize object set to store cell properties
	TeSTElementSet cellObjSet (cell_layer);

	// Initialize theme
	TeTheme* theme = new TeTheme ("", input_layer);
	vector<string> attrTableNames;
	attrTableNames.push_back (input_tablename);
	TeAttrTableVector atts;
	if (!input_layer->getAttrTablesByName(attrTableNames, atts)) return false;
	theme->setAttTables (atts);


	// Process
	TeCellSet::iterator cell_it = cells.begin();
	while (cell_it != cells.end())
	{
		TeSTInstance cellObj;
		cellObj.objectId ((*cell_it).objectId());
		string uniqueId = (*cell_it).objectId();
		cellObj.addUniqueId (uniqueId); 
		uniqueId += t.getInitialDate() + t.getInitialTime();
		uniqueId += t.getFinalDate() + t.getFinalTime() ;
		cellObj.addUniqueId (uniqueId);
		cellObj.timeInterval (t);
		
		// Set restrictions on a theme and create stoset
		theme->setSpatialRest((*cell_it).box(), TeCELLS); //????? TeWITHIN 	
		TeSTElementSet stos (theme);
		TeSTOSetBuildDB (&stos, true, false, attrNames);
			
		// Create stoset based on input layer
		TePropertyVector result;
		vector<string>::iterator attIt = attrNames.begin();
		while (attIt != attrNames.end())
		{
			result = operation->compute (stos.begin(*attIt), stos.end(*attIt), (*attIt));
            // if property not found, stos iterator will return zero to operation. 
            // To check this, have to be less general and use getProperty inside the compute method.		
			TePropertyVector::iterator itProp = result.begin();
			while (itProp != result.end())
			{
				cellObj.addProperty (*itProp);
				itProp++;
			}
			attIt++;
		}
		cellObjSet.insertSTInstance (cellObj);

		cell_it++;
	}

	// Update DB
	if (!TeUpdateDBFromSet (&cellObjSet, cell_tablename)) return  false;

	return  true;
}


bool TeFillCellConnectionOperation (	TeDatabase* db,
										TeSTElementSet objects,
										TeGeomRep rep,
										TeGraphNetwork* net,
										const string& /*input_attrName*/,

										TeTimeInterval t,
										const string& cell_layername, 
										const string& cell_tablename, 
										const string& /*output_columnName*/,
										
								
										TeComputeAttrStrategy<TeSTElementSet::propertyIterator>* operation,
										double /*local_distance_factor*/,
										double /*net_distance_factor*/,
										double /*net_conn_factor*/,
										double /*mult_factor*/)
{

	if (!operation) return false;
	if (!net) return false;
	if (!db) return false;

	// Load output cells layer with geometry previously created

	TeLayer* cell_layer = new TeLayer (cell_layername);

	if (!db->loadLayer (cell_layer))
	{
		 db->close();
		 return false;
	}

	TeTable table(cell_tablename);
	db->loadTableInfo(table);

	// Initialize cell set
	TeCellSet cells;
	if (!TeFillCellInitLoad (cell_layer, cell_tablename, cells)) return false;

	// Initialize object set to store cell properties
	TeSTElementSet cellObjSet (cell_layer);

	// Process
	TePropertyVector result;
	TeCellSet::iterator cell_it = cells.begin();
	while (cell_it != cells.end())
	{
		TeSTInstance cellObj;
		cellObj.objectId ((*cell_it).objectId());
		string uniqueId = (*cell_it).objectId();
		cellObj.addUniqueId (uniqueId);
		uniqueId += t.getInitialDate() + t.getInitialTime();
		uniqueId += t.getFinalDate() + t.getFinalTime() ;
		cellObj.addUniqueId (uniqueId);
		cellObj.timeInterval (t);	
		cellObjSet.insertSTInstance (cellObj);
		cell_it++;
	} 

	TeProxMatrixOpenNetworkStrategy2    sc_net (&cellObjSet, TeCELLS, &objects, rep, 0, TeMAXFLOAT, TeMAXFLOAT, net);
 //	TeProxMatrixConnectionStrenghtStrategy sw(4, 0.0, false);

/* TODO: recolocar!!!
	TeProxMatrixInverseDistanceStrategy sw (local_distance_factor, net_distance_factor, net_conn_factor, mult_factor, false);
	TeProxMatrixNoSlicingStrategy ss_no;
	TeGeneralizedProxMatrix<TeSTElementSet> mat (&sc_net, &sw, &ss_no);

	// initialize object set to store cell properties
	TeSTElementSet cellObjSet2 (cell_layer);
	TeCellSet::iterator cell_it2 = cells.begin();
	while (cell_it2 != cells.end())
	{
		TeSTElementSet neigh = mat.getSTENeighbours((*cell_it2).objectId());	

		result = operation->compute (neigh.begin(input_attrName), neigh.end(input_attrName), output_columnName); 	// if property not found, stos iterator will return zero to operation. To check this, have to be less general and use getProperty inside the compute method.		
		TeFillCellInitSTO ((*cell_it), propvec, cellObjSet2, table.tableType(), t);
		cell_it2++;
	} 


// Update DB
	if (!TeUpdateDBFromSet (&cellObjSet2, cell_tablename))
			return  false;
*/
	return  true;
}


