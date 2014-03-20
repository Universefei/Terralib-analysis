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

#include "TeLayerFunctions.h"
#include "../kernel/TeImportRaster.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeRaster.h"
#include "../kernel/TeExternalTheme.h"
#include "TeThemeFunctions.h"


void TeStartProcessing(int steps, const char* message)
{
	if (TeProgress::instance())
		{
			TeProgress::instance()->reset();
			TeProgress::instance()->setTotalSteps(steps);
			TeProgress::instance()->setMessage(message);
		}
}

bool TeNextStep(int step)
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

bool TeCopyThemeToLayer(TeTheme* inTheme, TeLayer* outLayer, 
							int selObj, map<string,string> *txtTable)
{
	if (!inTheme || !outLayer)
		return false;

	TeLayer* inLayer = inTheme->layer();
	// both projection should be valid
	// do not allow only one NoProjection 
	TeProjection* pfrom = TeProjectionFactory::make(inTheme->layer()->projection()->params());
	TeProjection* pto = TeProjectionFactory::make(outLayer->projection()->params());

	if (!(*pfrom == *pto))
	{
		if (pfrom->name() == "NoProjection" || pto->name() == "NoProjection")
		{
			delete pfrom;
			delete pto;
			return false;
		}
	}

	TeDatabasePortal* portal = inLayer->database()->getPortal();
	if (!portal)
		return false;

	string sql;
	TeGeomRep rep = static_cast<TeGeomRep>(inLayer->geomRep());
	if (rep & TeRASTER)
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTER);
		sql = "SELECT object_id FROM " + rep->tableName_;
		if (portal->query(sql))
		{
			string objid;
			TeRasterParams par;
			TeRaster* rst;
			while (portal->fetchRow())
			{
				objid = portal->getData(0);
				rst = inLayer->raster(objid);
				par = rst->params();
				if (!TeImportRaster(outLayer,rst,par.blockWidth_,par.blockHeight_,par.compression_[0],
									 objid,par.dummy_[0],par.useDummy_,par.tiling_type_))
				{
					delete pfrom;
					delete pto;
					delete portal;
					return false;
				}
			}
		}
		// build the spatial index in the raster table
		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeRASTER);
		outLayer->database()->insertMetadata(outLayer->tableName(TeRASTER), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeRASTER), spatialColumn, (TeSpatialIndexType)TeRTREE);
		portal->freeResult();
	}

	if (rep & TeRASTERFILE)
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTERFILE);
		if (rep)
		{
			string inRepTableName = rep->tableName_;
			rep->tableName_ = "RasterLayer" + Te2String(outLayer->id());
			if (outLayer->database()->insertRepresentation(outLayer->id(),*rep))
			{
				sql = "INSERT INTO " + rep->tableName_ + " (object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type) ";
				sql += " SELECT object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type FROM " + inRepTableName;
				outLayer->database()->execute(sql);
			}
		}
	}
	
	// check if there is any other vectorial representation
	if (!((rep & TePOLYGONS) || (rep & TeLINES) || 
		  (rep & TePOINTS)   || (rep & TeCELLS) ||
		  (rep & TeTEXT)))
	{
		delete pfrom;
		delete pto;
		delete portal;
		return true;
	}

	// this SQL selects instances of objects according to input, so it uses te_collection_aux
	string inputInst;
	// this SQL selects objects according to input, so it uses te_collection
	string inputObj; 
	if(selObj == 1) // queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 2";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 2";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 2) // pointed
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 1";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 1";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 3) // not queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status <> 2";
		inputObj += " AND " + inTheme->collectionTable() + ".c_object_status <> 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status <> 2";
		inputInst += " AND " + inTheme->collectionAuxTable() + ".grid_status <> 3)";
	}
	else if(selObj == 4) // not pointed
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status <> 1";
		inputObj += " AND " + inTheme->collectionTable() + ".c_object_status <> 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status <> 1";
		inputInst += " AND " + inTheme->collectionAuxTable() + ".grid_status <> 3)";
	}
	else if(selObj == 5) // pointed and queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	else if(selObj == 6) // pointed or queried
	{
		inputObj = "(" + inTheme->collectionTable() + ".c_object_status = 2";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 1";
		inputObj += " OR " + inTheme->collectionTable() + ".c_object_status = 3)";
		inputInst = "(" + inTheme->collectionAuxTable() + ".grid_status = 2";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 1";
		inputInst += " OR " + inTheme->collectionAuxTable() + ".grid_status = 3)";
	}
	// Initally test if there is any object selected
	sql = "SELECT c_object_id FROM " + inTheme->collectionTable();
	if (!inputObj.empty())
		sql += " WHERE " + inputObj;
	if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
	{
		delete portal;
		delete pfrom;
		delete pto;
		return false; // error!
	}
	if (!portal->fetchRow())
	{
		delete portal;
		delete pfrom;
		delete pto;
		return true; // there is no object selected
	}
	portal->freeResult();

	unsigned int n = 1;
	if (inLayer->hasGeometry(TePOINTS))
	{
		sql = "SELECT " + inLayer->tableName(TePOINTS) + ".* FROM ";
		sql += inLayer->tableName(TePOINTS) + ", " + inTheme->collectionTable();
		sql += " WHERE object_id = c_object_id";
		if (!inputObj.empty())
			sql += " AND " + inputObj;

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TePointSet>(portal,pfrom,outLayer);
		portal->freeResult(); 

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOINTS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOINTS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOINTS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	
	if (inLayer->hasGeometry(TeLINES))
	{
		sql = "SELECT " + inLayer->tableName(TeLINES) + ".* FROM ";
		sql += inLayer->tableName(TeLINES) + ", " + inTheme->collectionTable();
		sql += " WHERE object_id = c_object_id";
		if (!inputObj.empty())
			sql += " AND " + inputObj;
		sql += " ORDER BY ext_max DESC";

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TeLineSet>(portal,pfrom,outLayer);
		portal->freeResult(); 

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeLINES);
		outLayer->database()->insertMetadata(outLayer->tableName(TeLINES), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeLINES), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	if (inLayer->hasGeometry(TePOLYGONS))
	{
		sql = "SELECT " + inLayer->tableName(TePOLYGONS) + ".* FROM ";
		sql += inLayer->tableName(TePOLYGONS) + ", " + inTheme->collectionTable();
		sql += " WHERE object_id = c_object_id";
		if (!inputObj.empty())
			sql += " AND " + inputObj;

		/*if (inLayer->database()->dbmsName() != "OracleSpatial" && inLayer->database()->dbmsName() != "PostGIS")
			sql += " ORDER BY object_id ASC, parent_id ASC, num_holes DESC";
		else
			sql += " ORDER BY object_id ASC";*/
		sql += " ORDER BY " + inLayer->database()->getSQLOrderBy(TePOLYGONS);

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TePolygonSet>(portal,pfrom,outLayer);
		portal->freeResult(); 

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOLYGONS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOLYGONS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOLYGONS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}

	if (inLayer->hasGeometry(TeCELLS))
	{
		sql = "SELECT " + inLayer->tableName(TeCELLS) + ".* FROM ";
		sql += inLayer->tableName(TeCELLS) + ", " + inTheme->collectionTable();
		sql += " WHERE object_id = c_object_id";
		if (!inputObj.empty())
			sql += " AND " + inputObj;

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TeCellSet>(portal,pfrom,outLayer);
		portal->freeResult(); 

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeCELLS);
		outLayer->database()->insertMetadata(outLayer->tableName(TeCELLS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeCELLS), spatialColumn, (TeSpatialIndexType)TeRTREE);
		
	}

	if (inLayer->hasGeometry(TeTEXT))
	{
		unsigned int j;
		vector<string> textTables;
		vector<string> textDescr;
		string selrep = "SELECT geom_table, description FROM te_representation WHERE layer_id = " + Te2String(inLayer->id());
		selrep += " AND geom_type = " + Te2String(TeTEXT);
		if (portal->query(selrep))
		{
			while(portal->fetchRow())
			{
				textTables.push_back(portal->getData(0));
				textDescr.push_back(portal->getData(1));
			}
		}
		portal->freeResult();

		int k=1;
		for (j=0; j<textTables.size(); j++)
		{
			string newTextTable = textTables[j] + "_1";
			while (inLayer->database()->tableExist(newTextTable))
			{
				k++;
				newTextTable = textTables[j] + "_" + Te2String(k);
			}
			sql = "SELECT DISTINCT " + textTables[j] + ".* FROM ";
			sql += textTables[j] + ", " + inTheme->collectionTable();
			sql += " WHERE object_id = c_object_id OR object_id = 'TE_NONE_ID'";
			if(inputObj.empty() == false)
				sql += " AND " + inputObj;
			if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				delete portal;
				delete pfrom;
				delete pto;
				return false;
			}
			if (portal->fetchRow())
			{
				if (!outLayer->addGeometry(TeTEXT,newTextTable,textDescr[j]))
				{
					delete portal;
					delete pfrom;
					delete pto;
					return false;
				}
				TeCopyPortalGeometriesToLayer<TeTextSet>(portal,pfrom,outLayer,newTextTable);
				if (txtTable)
				{
					(*txtTable)[textTables[j]] = newTextTable;
				}
			}
			portal->freeResult();
		}
	}
	delete pfrom;
	delete pto;
	portal->freeResult();

	TeAttrTableVector attributeTables;
	inTheme->getAttTables(attributeTables);
	sql = "SELECT table_id FROM te_theme_table WHERE theme_id = ";
	bool hasExternal = false;
	sql += Te2String(inTheme->id()) + " AND relation_id > 0";
	if (portal->query(sql) && portal->fetchRow())
		hasExternal = true;
	portal->freeResult();

	for (unsigned int i=0; i<attributeTables.size(); i++)
	{
		TeTable table = attributeTables[i];
		if ((table.tableType() == TeAttrExternal) || table.attributeList().size() <= 0)
			continue;

		TeAttributeList attListIn = table.attributeList();
		table.setId(-1);
		n = 1;
		string tname = table.name();
		string tlinkqueattr = table.linkName();
		while (true)
		{
			if (!outLayer->database()->tableExist(tname))
				break;
			n++;
			tname = table.name() + "_" + Te2String(n);
		}
		string tfrom = table.name();
		table.name(tname);
		if (!outLayer->createAttributeTable(table))
		{
			delete portal;
			return false;
		}
		string fieldsIn;
		string fieldsOut;
		vector<string> attNamesIn;
		vector<string> attNamesOut;
		table.attributeNames(attNamesOut);
		for (n=0;n<attNamesOut.size()-1; n++)
		{
			fieldsOut += attNamesOut[n] + ",";
			fieldsIn += tfrom + "." + attListIn[n].rep_.name_ + ",";
		}
		fieldsOut += attNamesOut[n];
		fieldsIn += tfrom + "." + attListIn[n].rep_.name_;

		if (!hasExternal)	// table might have temporal instances 
		{
			sql = "SELECT " + fieldsIn + " ";
			sql += inTheme->sqlGridFrom();
			if (!inputInst.empty())
				sql += " WHERE " + inputInst;
		}
		else				// table doesn�t have temporal instances 
		{
			sql = "SELECT " + fieldsIn + " FROM " +  tfrom;
			sql += ", " + inTheme->collectionTable() + " WHERE ";
			sql += tfrom + "." + tlinkqueattr + "=" + inTheme->collectionTable() + ".c_object_id";
			if (!inputObj.empty())
				sql += " AND " + inputObj;
		}

		if (inLayer->database() == outLayer->database())
		{
			string popule = "INSERT INTO " + table.name() + " ( " + fieldsOut + ") " + sql;
			inLayer->database()->execute(popule);
		}
		else
		{
			if (portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				int nr = 0;
				int count = 0;
			
				TeStartProcessing((portal->numRows() * portal->numFields()),"saving table ...");


				while (portal->fetchRow())
				{
					TeTableRow row;
					for(int i = 0; i < portal->numFields(); i++)
					{
						row.push_back(portal->getData(i));
						count++;
						TeNextStep(count);
					}

					table.add(row);
					nr++;
					if (nr % 100 == 0)
					{
						if (!outLayer->saveAttributeTable(table))
						{
							delete portal;							
							return false;
						}
						table.clear();
						nr = 0;
					}
				}
				if (table.size() >0)
				{
					if (!outLayer->saveAttributeTable(table))
					{
						delete portal;							
						return false;
					}
				}
				table.clear();
			}
		}
		portal->freeResult();
	}
	delete portal;
	TeProgress::instance()->reset();
	return true;
}

bool TeCopyLayerToLayer(TeLayer* inLayer, TeLayer* outLayer, map<string,string> *txtTable)
{
	if (!inLayer || !outLayer)
		return false;

	// both projection should be valid
	// do not allow only one NoProjection 
	TeProjection* pfrom = TeProjectionFactory::make(inLayer->projection()->params());
	TeProjection* pto   = TeProjectionFactory::make(outLayer->projection()->params());
	if (!(*pfrom == *pto))
	{
		if (pfrom->name() == "NoProjection" || pto->name() == "NoProjection")
		{
			delete pfrom;
			delete pto;
			return false;
		}
	}

	TeDatabasePortal* portal = inLayer->database()->getPortal();
	if (!portal)
	{
		delete pfrom;
		delete pto;
		return false;
	}

	string sql;
	bool res;
	if (inLayer->hasGeometry(TeRASTER))
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTER);
		sql = "SELECT object_id FROM " + rep->tableName_;
		if (portal->query(sql))
		{
			string objid;
			TeRasterParams par;
			TeRaster* rst;
			while (portal->fetchRow())
			{
				objid = portal->getData(0);
				rst = inLayer->raster(objid);
				par = rst->params();
				res = TeImportRaster(outLayer,rst,par.blockWidth_,par.blockHeight_,par.compression_[0],
									 objid,par.dummy_[0],par.useDummy_,par.tiling_type_);
			}
		}
		portal->freeResult();
	}

	if (inLayer->hasGeometry(TeRASTERFILE)) // TODO : check the case when the databases are in different DBMS
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTERFILE);
		if (rep)
		{
			string inRepTableName = rep->tableName_;
			rep->tableName_ = "RasterLayer" + Te2String(outLayer->id());
			if (outLayer->database()->insertRepresentation(outLayer->id(),*rep))
			{
				sql = "INSERT INTO " + rep->tableName_ + " (object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type) ";
				sql += " SELECT object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type FROM " + inRepTableName;
				outLayer->database()->execute(sql);
			}
		}
	}

	unsigned int	n = 1;
	if (inLayer->hasGeometry(TePOINTS))
	{
		sql = "SELECT " + inLayer->tableName(TePOINTS) + ".* FROM ";
		sql += inLayer->tableName(TePOINTS);
		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
				TeCopyPortalGeometriesToLayer<TePointSet>(portal,pfrom,outLayer);
		portal->freeResult();

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOINTS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOINTS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOINTS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	
	if (inLayer->hasGeometry(TeLINES))
	{
		sql = "SELECT " + inLayer->tableName(TeLINES) + ".* FROM ";
		sql += inLayer->tableName(TeLINES);
		sql += " ORDER BY ext_max DESC";

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TeLineSet>(portal,pfrom,outLayer);
		portal->freeResult();
	
		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeLINES);
		outLayer->database()->insertMetadata(outLayer->tableName(TeLINES), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeLINES), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	if (inLayer->hasGeometry(TePOLYGONS))
	{
		sql = "SELECT " + inLayer->tableName(TePOLYGONS) + ".* FROM ";
		sql += inLayer->tableName(TePOLYGONS);
		/*if (inLayer->database()->dbmsName() != "OracleSpatial" && inLayer->database()->dbmsName() != "PostGIS")
			sql += " ORDER BY object_id ASC, parent_id ASC, num_holes DESC";
		else
			sql += " ORDER BY object_id ASC";*/
		sql += " ORDER BY " + inLayer->database()->getSQLOrderBy(TePOLYGONS);

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TePolygonSet>(portal,pfrom,outLayer);
		portal->freeResult();

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOLYGONS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOLYGONS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOLYGONS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}

	if (inLayer->hasGeometry(TeCELLS))
	{
		sql = "SELECT " + inLayer->tableName(TeCELLS) + ".* FROM ";
		sql += inLayer->tableName(TeCELLS) ;

		if (!portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			delete portal;
			delete pfrom;
			delete pto;
			return false;
		}
		if (portal->fetchRow())
			TeCopyPortalGeometriesToLayer<TeCellSet>(portal,pfrom,outLayer);
		portal->freeResult();

		TeRepresentation* repIn = inLayer->getRepresentation(TeCELLS);
		if(repIn != 0)
		{
			TeRepresentation* repOut = outLayer->getRepresentation(TeCELLS);
			if(repOut != 0)
			{
				repOut->resX_ = repIn->resX_;
				repOut->resY_ = repIn->resY_;

				outLayer->database()->updateRepresentation(outLayer->id(), *repOut);
			}
		}

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeCELLS);
		outLayer->database()->insertMetadata(outLayer->tableName(TeCELLS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeCELLS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	if (inLayer->hasGeometry(TeTEXT))
	{
		unsigned int j;
		vector<string> textTables;
		vector<string> textDescr;
		string selrep = "SELECT geom_table, description FROM te_representation WHERE layer_id = " + Te2String(inLayer->id());
		selrep += " AND geom_type = " + Te2String(TeTEXT);
		if (portal->query(selrep, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
		{
			while(portal->fetchRow())
			{
				textTables.push_back(portal->getData(0));
				textDescr.push_back(portal->getData(1));
			}
		}
		portal->freeResult();

		int k=1;
		for (j=0; j<textTables.size(); j++)
		{
			string newTextTable = textTables[j] + "_1";
			while (inLayer->database()->tableExist(newTextTable))
			{
				k++;
				newTextTable = textTables[j] + "_" + Te2String(k);
			}
			sql = "SELECT " + textTables[j] + ".* FROM ";
			sql += textTables[j] ;
			if (!portal->query(sql))
			{
				delete portal;
				delete pfrom;
				delete pto;
				return false;
			}
			
			if (portal->fetchRow())
			{
				if (!outLayer->addGeometry(TeTEXT,newTextTable,textDescr[j]))
				{
					delete portal;
					delete pfrom;
					delete pto;
					return false;
				}
				TeCopyPortalGeometriesToLayer<TeTextSet>(portal,pfrom,outLayer,newTextTable);
				if (txtTable)
				{
					(*txtTable)[textTables[j]] = newTextTable;
				}
			}
			portal->freeResult();
		}
	}
	delete pfrom;
	delete pto;
	portal->freeResult();

	vector<TeTable> attributeTables;
	inLayer->getAttrTables(attributeTables);
	for (unsigned int i=0; i<attributeTables.size(); i++)
	{
		TeTable table = attributeTables[i];
		TeTable tableSource=attributeTables[i];
		if (table.attributeList().size() <= 0)
			continue;

		TeAttributeList attListIn = table.attributeList();
		table.setId(-1);

		string tname = outLayer->name();
		if(outLayer->database()->tableExist(tname))
		{
			n = 1;
			tname = table.name();
			while (true)
			{
				if (!outLayer->database()->tableExist(tname))
					break;
				n++;
				tname = table.name() + "_" + Te2String(n);
			}	
		}		
		
		string tfrom = table.name();
		table.name(tname);

	

		if (outLayer->createAttributeTable(table))
		{
			if (inLayer->database() == outLayer->database())
			{
				string fieldsIn;
				string fieldsOut;
				vector<string> attNamesIn;
				vector<string> attNamesOut;
				table.attributeNames(attNamesOut);
				for (n=0;n<attNamesOut.size()-1; n++)
				{
					fieldsOut += attNamesOut[n] + ",";
					fieldsIn += attListIn[n].rep_.name_ + ",";
				}
				fieldsOut += attNamesOut[n];
				fieldsIn += attListIn[n].rep_.name_;

				sql = "SELECT " + fieldsIn + " FROM " + tfrom;
				string popule = "INSERT INTO " + table.name() + " ( " + fieldsOut + ") " + sql;
				inLayer->database()->execute(popule);
			}
			else
			{
				portal->freeResult();

				std::string columnNames = "";
				for (unsigned int i = 0; i < tableSource.attributeList().size(); ++i)
				{
					if(!columnNames.empty())	columnNames += ",";
					columnNames += tableSource.attributeList()[i].rep_.name_;
				}
				sql = "SELECT " + columnNames + " FROM " + tfrom;
				if (portal->query(sql, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
				{
					int nr = 0;
					while (portal->fetchRow())
					{
						TeTableRow row;
						for(int i = 0; i < portal->numFields(); i++)
						{
							if (table.attributeList()[i].rep_.type_== TeDATETIME)
							{
								TeTime t = portal->getDate(i);
								string temp = t.getDateTime(table.attributeList()[i].dateTimeFormat_,table.attributeList()[i].dateSeparator_, table.attributeList()[i].timeSeparator_);
								row.push_back(temp);
							}
							else
								row.push_back(portal->getData(i));
						}
						table.add(row);
						nr++;
						if (nr % 100 == 0)
						{
							outLayer->saveAttributeTable(table);
							table.clear();
							nr = 0;
						}
					}
					if (table.size() >0)
						outLayer->saveAttributeTable(table);
					table.clear();
				}
			}
			portal->freeResult();
		}
	}	
	delete portal;
	return true;
}

bool TeCopyExternThemeToLayer(TeTheme* inTheme, TeLayer* outLayer, 
							int selObj, map<string,string>*)
{
	if (!inTheme || !outLayer)
		return false;

	TeLayer* inLayer = ((TeExternalTheme*)(inTheme))->getRemoteTheme()->layer();
	// both projection should be valid
	// do not allow only one NoProjection 
	TeProjection* pfrom = TeProjectionFactory::make(inLayer->projection()->params());
	TeProjection* pto = TeProjectionFactory::make(outLayer->projection()->params());

	if (!(*pfrom == *pto))
	{
		if (pfrom->name() == "NoProjection" || pto->name() == "NoProjection")
		{
			delete pfrom;
			delete pto;
			return false;
		}
	}

	TeDatabasePortal* inPortal = inLayer->database()->getPortal();
	if (!inPortal)
		return false;

	string sql;
	TeGeomRep rep = static_cast<TeGeomRep>(inLayer->geomRep());
	if (rep & TeRASTER)
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTER);
		sql = "SELECT object_id FROM " + rep->tableName_;
		if (inPortal->query(sql))
		{
			string objid;
			TeRasterParams par;
			TeRaster* rst;
			while (inPortal->fetchRow())
			{
				objid = inPortal->getData(0);
				rst = inLayer->raster(objid);
				par = rst->params();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				if (!TeImportRaster(outLayer,rst,par.blockWidth_,par.blockHeight_,par.compression_[0],
									 objid,par.dummy_[0],par.useDummy_,par.tiling_type_))
				{
					delete pfrom;
					delete pto;
					delete inPortal;
					return false;
				}
			}
		}
		// build the spatial index in the raster table
		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeRASTER);
		outLayer->database()->insertMetadata(outLayer->tableName(TeRASTER), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeRASTER), spatialColumn, (TeSpatialIndexType)TeRTREE);
		inPortal->freeResult();
	}

	if (rep & TeRASTERFILE)
	{
		TeRepresentation* rep = inLayer->getRepresentation(TeRASTERFILE);
		if (rep)
		{
			string inRepTableName = rep->tableName_;
			rep->tableName_ = "RasterLayer" + Te2String(outLayer->id());
			if (outLayer->database()->insertRepresentation(outLayer->id(),*rep))
			{
				sql = "INSERT INTO " + rep->tableName_ + " (object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type) ";
				sql += " SELECT object_id, raster_table, lut_table, res_x, res_y, num_bands, num_cols, num_rows, block_height, block_width, lower_x, lower_y, upper_x, upper_y, tiling_type FROM " + inRepTableName;
				outLayer->database()->execute(sql);
			}
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// check if there is any other vectorial representation
	if (!((rep & TePOLYGONS) || (rep & TeLINES) || 
		  (rep & TePOINTS)   || (rep & TeCELLS) ||
		  (rep & TeTEXT)))
	{
		delete pfrom;
		delete pto;
		delete inPortal;
		return true;
	}

	// this SQL selects instances of objects according to input, so it uses te_collection_aux
	string inputInst;
	// this SQL selects objects according to input, so it uses te_collection
	string inputObj; 
	vector<string> objectVec;
	vector<string> itemVec;
	map<string, int>::iterator it;
	string extCollTable = ((TeExternalTheme*)(inTheme))->getRemoteTheme()->collectionTable();
	string not_string;

	if(selObj == 0) // all collection
	{
		objectVec = getObjects(inTheme, TeAll);
		itemVec = getItems(inTheme, TeAll);
	}
	else if(selObj == 1) // queried
	{
		objectVec = getObjects(inTheme, TeSelectedByQuery);
		itemVec = getItems(inTheme, TeSelectedByQuery);
	}
	else if(selObj == 2) // pointed
	{
		objectVec = getObjects(inTheme, TeSelectedByPointing);
		itemVec = getItems(inTheme, TeSelectedByPointing);
	}
	else if(selObj == 3) // not queried
	{
		objectVec = getObjects(inTheme, TeNotSelectedByQuery);
		itemVec = getItems(inTheme, TeNotSelectedByQuery);
	}
	else if(selObj == 4) // not pointed
	{
		objectVec = getObjects(inTheme, TeNotSelectedByPointing);
		itemVec = getItems(inTheme, TeNotSelectedByPointing);
	}
	else if(selObj == 5) // pointed and queried
	{
		objectVec = getObjects(inTheme, TeSelectedByPointingAndQuery);
		itemVec = getItems(inTheme, TeSelectedByPointingAndQuery);
	}
	else if(selObj == 6) // pointed or queried
	{
		objectVec = getObjects(inTheme, TeSelectedByPointingOrQuery);
		itemVec = getItems(inTheme, TeSelectedByPointingOrQuery);
	}
	if (objectVec.empty())
	{
		delete inPortal;
		delete pfrom;
		delete pto;
		return true; // there is no object selected
	}
  
  std::vector< std::string >::iterator it_begin = objectVec.begin();
  std::vector< std::string >::iterator it_end = objectVec.end();

	vector<string> svec = generateInClauses(it_begin, it_end, inLayer->database());
	unsigned int n = 1;
	vector<string>::iterator sit;
	inPortal->freeResult();

	if (inLayer->hasGeometry(TePOINTS))
	{
		sql = "SELECT " + inLayer->tableName(TePOINTS) + ".* FROM ";
		sql += inLayer->tableName(TePOINTS);
		for(sit=svec.begin(); sit!=svec.end(); ++sit)
		{
			string s = sql + " WHERE object_id " + not_string + " IN " + *sit;
		
			if (!inPortal->query(s, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				delete inPortal;
				delete pfrom;
				delete pto;
				return false;
			}
			if (inPortal->fetchRow())
				TeCopyPortalGeometriesToLayer<TePointSet>(inPortal,pfrom,outLayer);
			inPortal->freeResult(); 
		}

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOINTS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOINTS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOINTS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	
	if (inLayer->hasGeometry(TeLINES))
	{
		sql = "SELECT " + inLayer->tableName(TeLINES) + ".* FROM ";
		sql += inLayer->tableName(TeLINES);
		for(sit=svec.begin(); sit!=svec.end(); ++sit)
		{
			string s = sql + " WHERE object_id IN " + *sit;
			s += " ORDER BY ext_max DESC";

			if (!inPortal->query(s, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				delete inPortal;
				delete pfrom;
				delete pto;
				return false;
			}
			if (inPortal->fetchRow())
				TeCopyPortalGeometriesToLayer<TeLineSet>(inPortal,pfrom,outLayer);
			inPortal->freeResult(); 
		}

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeLINES);
		outLayer->database()->insertMetadata(outLayer->tableName(TeLINES), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeLINES), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}
	if (inLayer->hasGeometry(TePOLYGONS))
	{
		sql = "SELECT " + inLayer->tableName(TePOLYGONS) + ".* FROM ";
		sql += inLayer->tableName(TePOLYGONS);
		for(sit=svec.begin(); sit!=svec.end(); ++sit)
		{
			string s = sql + " WHERE object_id IN " + *sit;
			/*if (inLayer->database()->dbmsName() != "OracleSpatial" && inLayer->database()->dbmsName() != "PostGIS")
				s += " ORDER BY object_id ASC, parent_id ASC, num_holes DESC";
			else
				s += " ORDER BY object_id ASC"; */
			s += " ORDER BY " + inLayer->database()->getSQLOrderBy(TePOLYGONS);

			if (!inPortal->query(s, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				delete inPortal;
				delete pfrom;
				delete pto;
				return false;
			}
			if (inPortal->fetchRow())
				TeCopyPortalGeometriesToLayer<TePolygonSet>(inPortal,pfrom,outLayer);
			inPortal->freeResult(); 
		}

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TePOLYGONS);
		outLayer->database()->insertMetadata(outLayer->tableName(TePOLYGONS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TePOLYGONS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}

	if (inLayer->hasGeometry(TeCELLS))
	{
		sql = "SELECT " + inLayer->tableName(TeCELLS) + ".* FROM ";
		sql += inLayer->tableName(TeCELLS);
		for(sit=svec.begin(); sit!=svec.end(); ++sit)
		{
			string s = sql + " WHERE object_id IN " + *sit;

			if (!inPortal->query(s, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				delete inPortal;
				delete pfrom;
				delete pto;
				return false;
			}
			if (inPortal->fetchRow())
				TeCopyPortalGeometriesToLayer<TeCellSet>(inPortal,pfrom,outLayer);
			inPortal->freeResult(); 
		}

		std::string spatialColumn = outLayer->database()->getSpatialIdxColumn(TeCELLS);
		outLayer->database()->insertMetadata(outLayer->tableName(TeCELLS), spatialColumn, 0.0005,0.0005,outLayer->box());
		outLayer->database()->createSpatialIndex(outLayer->tableName(TeCELLS), spatialColumn, (TeSpatialIndexType)TeRTREE);
	}

	inPortal->freeResult();

	TeAttrTableVector attributeTables;
	((TeExternalTheme*)(inTheme))->getRemoteTheme()->getAttTables(attributeTables);
	sql = "SELECT table_id FROM te_theme_table WHERE theme_id = ";
	bool hasExternal = false;
	sql += Te2String(((TeExternalTheme*)(inTheme))->getRemoteTheme()->id()) + " AND relation_id > 0";
	if (inPortal->query(sql) && inPortal->fetchRow())
		hasExternal = true;

	for (unsigned int i=0; i<attributeTables.size(); i++)
	{
		TeTable table = attributeTables[i];
		if ((table.tableType() == TeAttrExternal) || table.attributeList().size() <= 0)
			continue;

		TeAttributeList attListIn = table.attributeList();
		table.setId(-1);
		n = 1;
		string tname = table.name();
		string tlinkqueattr = table.linkName();
		while (true)
		{
			if (!outLayer->database()->tableExist(tname))
				break;
			n++;
			tname = table.name() + "_" + Te2String(n);
		}
		string tfrom = table.name();
		table.name(tname);
		if (!outLayer->createAttributeTable(table))
		{
			delete inPortal;
			return false;
		}
		string fieldsIn;
		string fieldsOut;
		vector<string> attNamesIn;
		vector<string> attNamesOut;
		table.attributeNames(attNamesOut);
		for (n=0;n<attNamesOut.size()-1; n++)
		{
			fieldsOut += attNamesOut[n] + ",";
			fieldsIn += tfrom + "." + attListIn[n].rep_.name_ + ",";
		}
		fieldsOut += attNamesOut[n];
		fieldsIn += tfrom + "." + attListIn[n].rep_.name_;

		if (hasExternal)	// table might have temporal instances 
		{
      std::vector< std::string >::iterator it_begin = itemVec.begin();
      std::vector< std::string >::iterator it_end = itemVec.end();    
    
			svec = generateInClauses(it_begin, it_end, inLayer->database(), false);

			sql = "SELECT " + fieldsIn;
			sql += ((TeExternalTheme*)(inTheme))->getRemoteTheme()->sqlGridFrom();
			sql += " WHERE unique_id IN ";
		}
		else				// table doesn�t have temporal instances 
		{
			sql = "SELECT " + fieldsIn;
			sql += ((TeExternalTheme*)(inTheme))->getRemoteTheme()->sqlFrom();
			sql += " WHERE " + tlinkqueattr + " IN " ;
		}

		for(sit=svec.begin(); sit!=svec.end(); ++sit)
		{
			string s = sql + *sit;

			inPortal->freeResult();
			if (inPortal->query(s, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR))
			{
				int nr = 0;
				while (inPortal->fetchRow())
				{
					TeTableRow row;
					for(int i = 0; i < inPortal->numFields(); i++)
						row.push_back(inPortal->getData(i));
					table.add(row);
					nr++;
					if (nr % 100 == 0)
					{
						if (!outLayer->saveAttributeTable(table))
						{
							delete inPortal;							
							return false;
						}
						table.clear();
						nr = 0;
					}
				}
				if (table.size() >0)
				{
					if (!outLayer->saveAttributeTable(table))
					{
						delete inPortal;							
						return false;
					}
				}
				table.clear();
			}
		}
		inPortal->freeResult();
	}
	delete inPortal;
	return true;
}

