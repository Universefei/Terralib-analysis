/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

#include "TeGeoProcessingFunctions.h"
#include "../kernel/TeQuerier.h"
#include "../kernel/TeQuerierParams.h"
#include "../kernel/TeOverlay.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeVectorRemap.h"
#include "../kernel/TeDatabase.h"
#include "TeCellAlgorithms.h"
#include "../kernel/TeAsciiFile.h"


//-------------------------------- Auxiliary functions
inline string getStringFromStatistic(TeStatisticType sType)
{
	string ss;

	if(sType == TeSUM)
		ss = "SUM";
	else if(sType == TeMAXVALUE)
		ss = "MAXVALUE";
	else if(sType == TeMINVALUE)
		ss = "MINVALUE";
	else if(sType == TeCOUNT)
		ss = "COUNT";
	else if(sType == TeVALIDCOUNT)
		ss = "VALIDCOUNT";
	else if(sType == TeSTANDARDDEVIATION)
		ss = "STANDARDDEVIATION";
	else if(sType == TeKERNEL)
		ss = "KERNEL";
	else if(sType == TeMEAN)
		ss = "MEAN";
	else if(sType == TeVARIANCE)
		ss = "VARIANCE";
	else if(sType == TeSKEWNESS)
		ss = "SKEWNESS";
	else if(sType == TeKURTOSIS)
		ss = "KURTOSIS";
	else if(sType == TeAMPLITUDE)
		ss = "AMPLITUDE";
	else if(sType == TeMEDIAN)
		ss = "MEDIAN";
	else if(sType == TeVARCOEFF)
		ss = "VARCOEFF";
	else if(sType == TeMODE)
		ss = "MODE";
	else if(sType == TeNOSTATISTIC)
		ss = "VALUE";
	return ss;
}


inline int countNumGeometries(TeTheme* theme, TeSelectedObjects selOb)
{
	TeDatabase* db = theme->layer()->database();

	string count;
	TeGeomRep gRep = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;

    if(gRep == TePOLYGONS)
	{
		string tableName = theme->layer()->tableName(TePOLYGONS);

		//Calculate the number of polygons to be drawn
		string selectPolyCount; 

		if(db->dbmsName()=="SqlServerAdo")
			selectPolyCount="SELECT COUNT(DISTINCT(parent_id)) FROM ";
		else if(db->dbmsName() != "OracleSpatial" && db->dbmsName() != "PostGIS")
			selectPolyCount = "SELECT DISTINCT parent_id FROM ";
		else
			selectPolyCount = "SELECT DISTINCT geom_id FROM ";

		selectPolyCount += tableName + ", " + theme->collectionTable() + " WHERE c_object_id = object_id";

		if(selOb == TeSelectedByPointing)
		{
			selectPolyCount += " AND (c_object_status = 1 OR c_object_status = 3)";

		}
		else if(selOb == TeSelectedByQuery)
		{
			selectPolyCount += " AND (c_object_status = 2 OR c_object_status = 3)";
		}
		
		if(db->dbmsName() =="SqlServerAdo")
			count = selectPolyCount;
		else if(db->dbmsName() == "PostgreSQL" || db->dbmsName() == "PostGIS")
			count = "SELECT COUNT(*) FROM (" + selectPolyCount + ") AS r";		
		else 
			count = "SELECT COUNT(*) FROM (" + selectPolyCount + ")";		
	}
	else if(gRep == TeLINES)
	{
		string tableName = theme->layer()->tableName(TeLINES);

		count = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		count += " WHERE object_id = c_object_id";

		if(selOb == TeSelectedByPointing)
		{
			count += " AND (c_object_status = 1 OR c_object_status = 3)";

		}
		else if(selOb == TeSelectedByQuery)
		{
			count += " AND (c_object_status = 2 OR c_object_status = 3)";
		}
	}
	else if(gRep == TePOINTS)
	{
		string tableName = theme->layer()->tableName(TePOINTS);

		count = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		count += " WHERE object_id = c_object_id";

		if(selOb == TeSelectedByPointing)
		{
			count += " AND (c_object_status = 1 OR c_object_status = 3)";

		}
		else if(selOb == TeSelectedByQuery)
		{
			count += " AND (c_object_status = 2 OR c_object_status = 3)";
		}
	}
	else if(gRep == TeCELLS)
	{
		string tableName = theme->layer()->tableName(TeCELLS);

		count = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		count += " WHERE object_id = c_object_id";

		if(selOb == TeSelectedByPointing)
		{
			count += " AND (c_object_status = 1 OR c_object_status = 3)";

		}
		else if(selOb == TeSelectedByQuery)
		{
			count += " AND (c_object_status = 2 OR c_object_status = 3)";
		}
	}
	else
		return -1;

	TeDatabasePortal* p = db->getPortal();

	if(p && p->query(count) && p->fetchRow())
	{
		int result = p->getInt(0);

		delete p;
		
		return result;

	}
	
	if(p)
		delete p;

	return -1;			
}

// objectid cell, <objectid polygon, itersection area>
inline void insertArea(const string& cellId, const string& polId, const double& area, map<string, map<string, double> >& cellMap)
{
	map<string, map<string, double> >::iterator it = cellMap.find(cellId);

	if(it != cellMap.end())
	{
// inserts into an existing entry
		map<string, double>::iterator itpol = it->second.find(polId);

		if(itpol != it->second.end())
		{
// polid already in index... add area
			itpol->second = itpol->second + area;
		}
		else
		{
// inserts polid and area
			(it->second)[polId] = area;
		}

	}
	else
	{
// inserts a new entry
		map<string, double> polMap;
		polMap.insert(pair<string, double>(polId, area));
		cellMap[cellId] = polMap;
	}
}

// objectid cell, <objectid polygon, itersection area>
inline string getMaxIntersectionArea(map<string, double>& polMap)
{
	string strMax;
	double maxArea = 0.0;

	map<string, double>::iterator it = polMap.begin();

	if(it != polMap.end())
	{
		strMax = it->first;
		maxArea = it->second;

		++it;

		while(it != polMap.end())
		{
			if(it->second > maxArea)
			{
				strMax = it->first;
				maxArea = it->second;
			}

			++it;
		}
	}

    return strMax;
}

bool insertBoundaryCells(int newId, TeLayer* outPutLayer, map<string, map<string, double> >& cellMap, TeTheme* cellTheme, TeTheme* themeTrim = 0)
{
	int step = 0;
	TeTable& attTable = outPutLayer->attrTables()[0];
	TeCellSet outputCellSet;

	outputCellSet.resX(cellTheme->layer()->getRepresentation(TeCELLS)->resX_);
	outputCellSet.resY(cellTheme->layer()->getRepresentation(TeCELLS)->resY_);

	attTable.clear();

	if(TeProgress::instance())
	{
		string caption = "Overlay Intersection";
		string msg = "Saving Boundary. Please, wait!";
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(cellMap.size());
	}
	map<string, map<string, double> >::iterator it = cellMap.begin();

	while(it != cellMap.end())
	{
		++newId;
		step++;

		string newObjId = Te2String(newId);

// get cell data
		TeQuerierParams cellParams(true, true);
		cellParams.setParams(cellTheme, it->first);

		TeQuerier cellQuerier(cellParams);

		if(!cellQuerier.loadInstances())
			return  false;

		TeSTInstance cellInst;

		if(!cellQuerier.fetchInstance(cellInst))
			return false;
		
		TeCellSet cs;

		cellInst.getGeometry(cs);

		cs[0].objectId(newObjId);

		outputCellSet.add(cs[0]);		

// monta a linha com os valores das colunas da celula
		TeTableRow row;

		row.push_back(newObjId);

		vector<string>& attrs = cellInst.getProperties();

		for(int p = 0; p < (int)attrs.size(); ++p)
			row.push_back(attrs[p]);

		if(themeTrim)
		{
			string polId = getMaxIntersectionArea(it->second);

			if(polId.empty())
				return false;


			TeQuerierParams trimParams(false, true);
			trimParams.setParams(themeTrim, polId);

			TeQuerier trimQuerier(trimParams);

			if(!trimQuerier.loadInstances())
				return  false;

			TeSTInstance trimInst;

			if(!trimQuerier.fetchInstance(trimInst))
				return false;

			vector<string>& attrstrim = trimInst.getProperties();

			for(int p = 0; p < (int)attrstrim.size(); ++p)
				row.push_back(attrstrim[p]);
		}

		attTable.add(row);
// salva a celula e os atributos

		if((newId % 100) == 0)
		{
			outPutLayer->addCells(outputCellSet);
			outputCellSet.clear();

			outPutLayer->saveAttributeTable(attTable);
			attTable.clear();

			if(TeProgress::instance())
			{
				if(TeProgress::instance()->wasCancelled())
				{
					TeProgress::instance()->reset();
					return false;
				}
				TeProgress::instance()->setProgress(step);
			}
		}
       
		++it;
	}

	if(outputCellSet.size() > 0)
	{
        outPutLayer->addCells(outputCellSet);
		outputCellSet.clear();
	}


	if(attTable.size() > 0)
	{
        outPutLayer->saveAttributeTable(attTable);
		attTable.clear();
	}

	if(TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}



inline bool changeAttrList(TeAttributeList& attrList)
{
	bool change = false;
	
	for(int i=0; i<(int)attrList.size(); ++i) 
	{
		string& nameOr = attrList[i].rep_.name_;
	
		attrList[i].rep_.isAutoNumber_ = false;
		attrList[i].rep_.isPrimaryKey_ = false;
		
		//replace "." for "_"
		string::size_type f = nameOr.find(".");
		if(f != string::npos)
		{
			nameOr.replace(f, 1, "_");
			change = true;
		}

		string nameTemp = TeConvertToUpperCase(nameOr);

		int count = 1;
		int j = i+1;
		while(j<(int)attrList.size())
		{
			if((nameTemp == TeConvertToUpperCase(attrList[j].rep_.name_)) && (i!=j))
			{
				nameTemp = nameTemp +"_"+ Te2String(count);
				nameOr = nameOr +"_"+ Te2String(count);

				j = 0;
				change = true;
				++count;
			}
			++j;
		}
	}

	return change;
}


inline bool multiGeometryToLayer(TeMultiGeometry& mGeom, const string& newId, TeLayer* newLayer, TeProjection* proj=0, TeGeomRep geomRep=TeGEOMETRYNONE)
{
	bool flag = true;
	TeProjection* projLayer = newLayer->projection();
	
	if(mGeom.hasPolygons() && (geomRep==TeGEOMETRYNONE || geomRep==TePOLYGONS))  
	{
		TePolygonSet polSet, polSetTemp;
		mGeom.getGeometry (polSetTemp);

		if((proj) && (!((*proj) == (*projLayer))))
		{
			for(int i=0; i<(int)polSetTemp.size(); ++i)
			{
				TePolygon poly = polSetTemp[i];
				TePolygon pout;
				TeVectorRemap (poly, proj, pout, projLayer);
				pout.objectId(poly.objectId());
				polSet.add (pout);
			}
		}
		else
			polSet = polSetTemp;


		polSet.objectId(newId);
		flag = newLayer->addPolygons(polSet);
	} 
	else if (mGeom.hasLines() && (geomRep==TeGEOMETRYNONE || geomRep==TeLINES))
	{
		TeLineSet lineSet, lineSetTemp;
		mGeom.getGeometry (lineSetTemp);

		if((proj) && (!((*proj) == (*projLayer))))
		{
			for(int i=0; i<(int)lineSetTemp.size(); ++i)
			{
				TeLine2D line = lineSetTemp[i];
				TeLine2D lout;
				TeVectorRemap (line, proj, lout, projLayer);
				lout.objectId(line.objectId());
				lineSet.add (lout);
			}
		}
		else
			lineSet = lineSetTemp;

		lineSet.objectId (newId);
		flag = newLayer->addLines(lineSet);
	}
	else if (mGeom.hasPoints() && (geomRep==TeGEOMETRYNONE || geomRep==TePOINTS))
	{
		TePointSet pointSet, pointSetTemp;
		mGeom.getGeometry (pointSetTemp);

		if((proj) && (!((*proj) == (*projLayer))))
		{
			for(int i=0; i<(int)pointSetTemp.size(); ++i)
			{
				TePoint point = pointSetTemp[i];
				TePoint pout;
				TeVectorRemap (point, proj, pout, projLayer);
				pout.objectId(point.objectId());
				pointSet.add (pout);
			}
		}
		else
			pointSet = pointSetTemp;

		pointSet.objectId (newId);
		flag = newLayer->addPoints(pointSet);
	}
	else if (mGeom.hasCells() && (geomRep==TeGEOMETRYNONE || geomRep==TeCELLS))
	{
		TeCellSet cellSet, cellSetTemp;
		mGeom.getGeometry (cellSetTemp);
	
		if((proj) && (!((*proj) == (*projLayer))))
		{
			for(int i=0; i<(int)cellSetTemp.size(); ++i)
			{
				TeCell cell = cellSetTemp[i];
				TeCell cellout;
				TeVectorRemap (cell, proj, cellout, projLayer);
				cellout.objectId(cell.objectId());
				cellSet.add (cellout);
			}
		}
		else
			cellSet = cellSetTemp;

		cellSet.objectId (newId);
		flag = newLayer->addCells(cellSet);
	}
//	else 
//		return false;

	return flag;
}

//-------------------------------- Main functions

bool TeGeoOpAggregation(TeLayer* newLayer, TeTheme* theme, vector<string>& agregAttrVec, 
						TeGroupingAttr& attrMM, TeSelectedObjects selOb, TeAsciiFile* logFile)		
{
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	unsigned int i, j;
	int	offset, steps = 0;
	TeStatisticValMap stat;
	TeStatisticStringValMap sstat;
	vector<string> valVec;
	map<int, vector<string> > stringMap;
	map<int, vector<double> > doubleMap;
	string geoName, query;
	string layerName = newLayer->name();
	TeLayer* layer = theme->layer();
	TeDatabase* db = layer->database();
	string CT = theme->collectionTable();
	string OID = CT + ".c_object_id";

	TeGeomRep geomRepTheme = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;

	TeAttributeList aList = theme->sqlAttList();
	TeAttribute agregTeAttribute;
	agregTeAttribute.rep_.type_ = TeSTRING;
	agregTeAttribute.rep_.numChar_ = 200;
	agregTeAttribute.rep_.isPrimaryKey_ = false;
	agregTeAttribute.rep_.isAutoNumber_ = false;

	map<int, string> legendLabelMap;
	int legIndex = -1;

	map<int, int> attrSQLPositionMap;	// dado o id do vetor attrMM ele devolve a posicao no portal. 

/* acha o nome para a coluna (agregTeAttribute) que sempre eh acrescentada
   ao layer de saida: a coluna formada pelos atributos usados para agrupar
 */
	for(j=0; j<agregAttrVec.size(); ++j)
	{
		string name;
		if(agregAttrVec[j] == "CURRENT LEGEND")
		{
			agregAttrVec[j] = CT + ".c_legend_id";
			name = "agreg_leg_";
			legIndex = j;
		}
		else
		{	
			for(i=0; i<aList.size(); ++i)
			{
				name = aList[i].rep_.name_;
				if(name == agregAttrVec[j])
				{
					int f = name.find(".");
					if(f >= 0)
						name.replace(f, 1, "_");
					break;
				}
			}
		}
		if(j==0)
			agregTeAttribute.rep_.name_ += name;
		else
			agregTeAttribute.rep_.name_ += "_" + name;
	}

	if(agregTeAttribute.rep_.name_.size() > 30)
		agregTeAttribute.rep_.name_ = agregTeAttribute.rep_.name_.substr(0, 30);

/* o usuario quer agrupar pela legenda?
   - se sim, montamos o legendLabelMap
 */
	if(legIndex > -1)
	{
		TeLegendEntryVector& legVec = theme->legend();
		for(i=0; i<legVec.size(); ++i)
			legendLabelMap[legVec[i].id()] = legVec[i].label();
	}

/* montagem da lista de atributos do novo layer
 */
	TeAttributeList attList;

	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_";
	at.rep_.numChar_ = 255;
	at.rep_.isPrimaryKey_ = true;
	at.rep_.isAutoNumber_ = false;
	attList.push_back(at);

	attList.push_back(agregTeAttribute);
	
	at.rep_.type_ = TeINT;
	at.rep_.name_ = "agreg_count_";
	at.rep_.numChar_ = 0;
	at.rep_.isPrimaryKey_ = false;
	attList.push_back(at);	

	TeGroupingAttr::iterator it;
	string ss;
	for(it=attrMM.begin(); it!=attrMM.end(); ++it)
	{
		string s = it->first.name_;
		TeStatisticType sType = it->second;

		ss = getStringFromStatistic(sType);
		string sss = s + "_" + ss;

		at.rep_ = it->first;
		at.rep_.isPrimaryKey_ = false;

		if(((db->dbmsName()=="OracleAdo") || 
			(db->dbmsName()=="OracleSpatial") ) &&
			(sss.size() > 30))
		{
			int s = sss.size(); 
			at.rep_.name_ = sss.substr(s-30, s-1);
		}
		else
			at.rep_.name_ = sss;

		
		if(at.rep_.type_ == TeINT)
			at.rep_.type_ = TeREAL;
		attList.push_back(at);	
	}
	
/* faz as alteracaoes necessarias nos nomes de campos, para evitar duplicacao de nomes
   e cria a tabela de dados
 */
	changeAttrList(attList);
	TeTable attTable(newLayer->name(), attList, "object_id_", "object_id_");

	if(!newLayer->createAttributeTable(attTable))
        return false;

/* monta a query de selecao dos atributos nao espaciais
 */
	string queryAtr  = "SELECT " + OID;

	unsigned int lastAttrPos = 0;

	for(i=0; i<agregAttrVec.size(); ++i)
	{
		lastAttrPos = i;
		queryAtr += ", " + agregAttrVec[i];		
	}

	ss = "";

	for(j=0; j<attrMM.size(); ++j)
	{
		string s = attrMM[j].first.name_;

/* verifica se o atributo jah foi incluido no SELECT
 */
		bool existedName = false;
		
		for(i=0; i<agregAttrVec.size(); ++i)
		{
			if(s == agregAttrVec[i])
			{
				attrSQLPositionMap[j] = i;
				existedName=true;
				break;
			}
		}

		if((!existedName) && (s != ss))
		{
			queryAtr += ", " + s;
			++lastAttrPos;
			attrSQLPositionMap[j] = lastAttrPos;
		}

		ss = s;
	}

	queryAtr += theme->sqlFrom();

	if(selOb == TeSelectedByPointing)
		queryAtr += " WHERE (" + CT + ".c_object_status = 1 OR " + CT + ".c_object_status = 3)";
	else if(selOb == TeSelectedByQuery)
		queryAtr += " WHERE (" + CT + ".c_object_status = 2 OR " + CT + ".c_object_status = 3)";
	queryAtr += " ORDER BY ";
	for(i=0; i<agregAttrVec.size(); ++i)
	{
		queryAtr += agregAttrVec[i];
		if (i<agregAttrVec.size()-1)
			queryAtr += ", ";
	}

/* monta outra query para os atributos espaciais
 */
	if(geomRepTheme == TePOLYGONS)
		geoName = layer->tableName(TePOLYGONS);
	else if(geomRepTheme == TeLINES)
		geoName = layer->tableName(TeLINES);
	else if(geomRepTheme == TePOINTS)
		geoName = layer->tableName(TePOINTS);
	else
		geoName = layer->tableName(TeCELLS);

	TeAttributeList geoAtList;
	db->getAttributeList(geoName, geoAtList);
	offset = geoAtList.size();

	if((db->dbmsName()=="OracleAdo") || (db->dbmsName()=="OracleSpatial"))
		query = "SELECT " + geoName + ".*, ATR.* FROM " + geoName + ", (" + queryAtr + ") ATR ";
	else
		query = "SELECT " + geoName + ".*, ATR.* FROM " + geoName + ", (" + queryAtr + ") AS ATR ";

	string newOID = TeGetExtension(OID.c_str());

	if(newOID.empty())
		newOID = OID;

	query += "WHERE ATR." + newOID + " = " + geoName + ".object_id";
	query += " ORDER BY ";
	for(i=0; i<agregAttrVec.size(); ++i)
	{
		string newagre = TeGetExtension(agregAttrVec[i].c_str());

		if(newagre.empty())
			newagre = agregAttrVec[i];

		query += "ATR." + newagre;
		if(i<agregAttrVec.size()-1)
			query += ", ";
	}
	query += ", " + geoName + ".object_id";


	if(geomRepTheme == TePOLYGONS)
	{
		if(db->dbmsName() != "OracleSpatial" && db->dbmsName() != "PostGIS")
			query += ", " + geoName + ".parent_id, " + geoName + ".num_holes DESC";
	}

	TeDatabasePortal* portal = db->getPortal();

	int total;
	string qc = "SELECT COUNT(*) FROM " + CT;
	if(selOb == TeSelectedByPointing)
		qc += " WHERE " + CT + ".c_object_status = 1 OR " + CT + ".c_object_status = 3";
	else if(selOb == TeSelectedByQuery)
		qc += " WHERE " + CT + ".c_object_status = 2 OR " + CT + ".c_object_status = 3";
	if(portal->query(qc) && portal->fetchRow())
		total = atoi(portal->getData(0));

	portal->freeResult();
	if(portal->query(query) && portal->fetchRow())
	{
		TeAttributeList& portalAttList = portal->getAttributeList();

		if(geomRepTheme == TePOLYGONS)
			newLayer->addGeometry(TePOLYGONS);
		else if(geomRepTheme == TeLINES)
			newLayer->addGeometry(TeLINES);
		else if(geomRepTheme == TePOINTS)
			newLayer->addGeometry(TePOINTS);
		else
			newLayer->addGeometry(TeCELLS);

		bool flag = false;
		int count = 0;			
		int objectId = 1;
		string lastOid = portal->getData(offset);
		lastOid += "aaa";
		string lastVal;

		for(i=0; i<agregAttrVec.size(); ++i)
		{
			string s = portal->getData(agregAttrVec[i]);
			if(legIndex == (int)i)
				s = legendLabelMap[atoi(s.c_str())];
			lastVal += s;
			if(i<agregAttrVec.size() - 1)
				lastVal += "_";
		}
		
		TePolygonSet ps;
		TeMultiGeometry multGeom; 

		if(TeProgress::instance())
		{
			string caption = "Aggregation";
			TeProgress::instance()->setCaption(caption.c_str());
			string msg = "Executing aggregation. Please, wait!";
			TeProgress::instance()->setMessage(msg);
			TeProgress::instance()->setTotalSteps(total);
			t2 = clock();
			t0 = t1 = t2;
		}

		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));

		do
		{	
			string oid = portal->getData(offset);
			string currVal;
			for(i=0; i<agregAttrVec.size(); ++i)
			{
				string s = portal->getData(agregAttrVec[i]);
				if (legIndex == (int)i)
					s = legendLabelMap[atoi(s.c_str())];
				currVal += s;
				if (i<agregAttrVec.size() - 1)
					currVal += "_";
			}

			TePolygon p;
			TeLine2D  l;
			TePoint	  pt;
			TeCell    c;
			double	  dval;

			if(lastVal == currVal)
			{				
				if(oid != lastOid)
				{
					count++;
					//for(j=3, i=offset+(int)agregAttrVec.size()+1; i<(int)portalAttList.size(); ++i, ++j)
					//offset + 1 = atributos da tabela de geometria + c_object_id
					for(j=0, i=offset+1; i<portalAttList.size(); ++i, ++j)
					{
						string s = portal->getData(i);
						TeAttrDataType aType = portalAttList[i].rep_.type_;
						if(aType != TeREAL && aType != TeINT)
							stringMap[j].push_back(s);
						else
						{
							if(s.empty())
								dval = TeMAXFLOAT;
							else
								dval = atof(s.c_str());
							doubleMap[j].push_back(dval);
						}
					}
				}

				if(geomRepTheme == TePOLYGONS)
				{
					flag = portal->fetchGeometry(p);
					multGeom.addGeometry(p);					
				}
				else if(geomRepTheme == TeLINES)
				{
					flag = portal->fetchGeometry(l);
					multGeom.addGeometry(l);
				}
				else if(geomRepTheme == TePOINTS)
				{
					flag = portal->fetchGeometry(pt);
					multGeom.addGeometry(pt);
				}
				else
				{
					flag = portal->fetchGeometry(c);
					multGeom.addGeometry(c);
				}
			}
			else  
			{
				if(multGeom.hasPolygons())
				{
					TePolygonSet polSetValidHoles;
			
					if(!TeOVERLAY::TeValidPolygonHoles(multGeom.getPolygons(), polSetValidHoles))
					{
						if(TeProgress::instance())
							TeProgress::instance()->reset();

						delete portal;
						return false; 
					}

					TePolygonSet psetOut;

					if(!TeOVERLAY::TeUnion(polSetValidHoles, psetOut))
					{
						// when the operation returns false, write in the log a possible
						// inconsistency in the data
						if (logFile)
						{
							string mess = "Possivel inconsistencia na uniao dos objetos da classe: ";
							mess += lastVal;
							logFile->writeString(mess);
							logFile->writeNewLine();
						}
					}

					multGeom.setGeometry(psetOut);
				}

				if(!multiGeometryToLayer(multGeom, Te2String(objectId), newLayer))
				{
					if(TeProgress::instance())
						TeProgress::instance()->reset();

					delete portal;
					return false; 
				}


				std::map<int, TeStatisticValMap> statAux;
				std::map<int, TeStatisticStringValMap> sstatAux;

				valVec.push_back(Te2String(objectId));
				valVec.push_back(lastVal);
				valVec.push_back(Te2String(count));

				string currAttr = "";
				int attPos = 0;
				for(i=0; i<attrMM.size(); ++i)
				{
					if (currAttr != attrMM[i].first.name_)
					{
						stat.clear();
						sstat.clear();
						attPos = attrSQLPositionMap[i];		
						currAttr = attrMM[i].first.name_;
					}

					if(attrMM[i].first.type_ == TeREAL || attrMM[i].first.type_ == TeINT)
					{
						std::map<int, TeStatisticValMap>::iterator it = statAux.find(attPos);

						if(it != statAux.end())
						{
							double d = (it->second)[attrMM[i].second];
							valVec.push_back(Te2String(d, 16));
						}
						else
						{
							TeCalculateStatistics(doubleMap[attPos].begin(), doubleMap[attPos].end(), stat);
							statAux[attPos] = stat;
							double d = stat[attrMM[i].second];
							valVec.push_back(Te2String(d, 16));
						}
					}
					else
					{
						std::map<int, TeStatisticStringValMap>::iterator it = sstatAux.find(attPos);

						if(it != sstatAux.end())
						{
							string s = (it->second)[attrMM[i].second];
							valVec.push_back(s);
						}
						else
						{
							TeCalculateStatistics(stringMap[attPos].begin(), stringMap[attPos].end(), sstat);
							sstatAux[attPos] = sstat;
							string s = sstat[attrMM[i].second];
							valVec.push_back(s);
						}						
					}
				}

				TeTableRow row;
				for(i=0; i<valVec.size(); ++i)
					row.push_back(valVec[i]);
				attTable.add(row);

				valVec.clear();
				stringMap.clear();
				doubleMap.clear();
				count = 0;
				ps.clear();
				multGeom.clear();
				
				if(oid != lastOid)	// poderia ser igual???? Veja que aqui estamos no else do bloco oid == lastOid!!!!
				{
					count++;
					for(j=0, i=offset+1; i<portalAttList.size(); ++i, ++j)
					{
						string s = portal->getData(i);
						TeAttrDataType aType = portalAttList[i].rep_.type_;
						if(aType != TeREAL && aType != TeINT)
							stringMap[j].push_back(s);
						else
						{
							if(s.empty())
								dval = TeMAXFLOAT;
							else
								dval = atof(s.c_str());
							doubleMap[j].push_back(dval);
						}
					}
				}

				if(geomRepTheme == TePOLYGONS)
				{
					flag = portal->fetchGeometry(p);
					multGeom.addGeometry(p);					
				}
				else if(geomRepTheme == TeLINES)
				{
					flag = portal->fetchGeometry(l);
					multGeom.addGeometry(l);
				}
				else if(geomRepTheme == TePOINTS)
				{
					flag = portal->fetchGeometry(pt);
					multGeom.addGeometry(pt);
				}
				else
				{
					flag = portal->fetchGeometry(c);
					multGeom.addGeometry(c);
				}

				++objectId;
			}

			if((oid != lastOid) && TeProgress::instance())
			{
				steps++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					if(TeProgress::instance()->wasCancelled())
					{
						TeProgress::instance()->reset();
						delete portal;
						return false;
					}
					
					if((int)(t2-t0) > dt2)
						TeProgress::instance()->setProgress(steps);
				}
			}
			lastOid = oid;
			lastVal = currVal;

		}while(flag);
		if(TeProgress::instance())
			TeProgress::instance()->reset();

		if(!multGeom.empty())  
		{
			if(multGeom.hasPolygons())
			{
					TePolygonSet polSetValidHoles;
					
					if(!TeOVERLAY::TeValidPolygonHoles(multGeom.getPolygons(), polSetValidHoles))
					{
						if(TeProgress::instance())
							TeProgress::instance()->reset();

						delete portal;
						return false; 
					}

					TePolygonSet psetOut;

					if(!TeOVERLAY::TeUnion(polSetValidHoles, psetOut))
					{
						// when the operation returns false, write in the log a possible
						// inconsistency in the data
						if (logFile)
						{
							string mess = "Possivel inconsistencia na uniao dos objetos da classe: ";
							mess += lastVal;
							logFile->writeString(mess);
							logFile->writeNewLine();
						}
					}

					multGeom.setGeometry(psetOut);
			}

			if(!multiGeometryToLayer(multGeom, Te2String(objectId), newLayer))
			{
				if(TeProgress::instance())
					TeProgress::instance()->reset();

				delete portal;
				return false; 
			}

			std::map<int, TeStatisticValMap> statAux;
			std::map<int, TeStatisticStringValMap> sstatAux;

			valVec.push_back(Te2String(objectId));
			valVec.push_back(lastVal);
			valVec.push_back(Te2String(count));
			string currAttr = "";
			int attPos = 0;
			for(i = 0; i <attrMM.size(); ++i)
			{
				if (currAttr != attrMM[i].first.name_)
				{
					stat.clear();
					sstat.clear();
					attPos = attrSQLPositionMap[i];		
					currAttr = attrMM[i].first.name_;
				}
				if(attrMM[i].first.type_ == TeREAL || attrMM[i].first.type_ == TeINT)
				{
					std::map<int, TeStatisticValMap>::iterator it = statAux.find(attPos);

					if(it != statAux.end())
					{
						double d = (it->second)[attrMM[i].second];
						valVec.push_back(Te2String(d, 16));
					}
					else
					{
                        TeCalculateStatistics(doubleMap[attPos].begin(), doubleMap[attPos].end(), stat);
						statAux[attPos] = stat;
						double d = stat[attrMM[i].second];
						valVec.push_back(Te2String(d, 16));
					}
				}
				else
				{
					std::map<int, TeStatisticStringValMap>::iterator it = sstatAux.find(attPos);

					if(it != sstatAux.end())
					{
						string s = (it->second)[attrMM[i].second];
						valVec.push_back(s);
					}
					else
					{
                        TeCalculateStatistics(stringMap[attPos].begin(), stringMap[attPos].end(), sstat);
						sstatAux[attPos] = sstat;
						string s = sstat[attrMM[i].second];
						valVec.push_back(s);
					}						
				}
			}	

			TeTableRow row;
			for(i=0; i<valVec.size(); ++i)
				row.push_back(valVec[i]);
			attTable.add(row);

			valVec.clear();
			stringMap.clear();
			doubleMap.clear();
			count = 0;
			ps.clear();
			multGeom.clear();
		}			

		newLayer->saveAttributeTable(attTable);
	}
	else
	{
		delete portal;
		return false;
	}

	delete portal;
	return true;
}


bool TeGeoOpAdd(TeLayer* newLayer, TeTheme* theme, vector<TeTheme*> themeVec, 
				TeSelectedObjects selOb,TeAsciiFile* /* logFile */)
{
	
	bool insertAttr = false;
	
	// create attribute table from main theme
	TeAttributeList attrList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_"+ Te2String(newLayer->id());
	at.rep_.numChar_ = 255;
	at.rep_.isPrimaryKey_ = true;
	at.rep_.isAutoNumber_ = false;
	attrList.push_back(at);

	//main theme
	TeQuerierParams mainParams(true, true);
	mainParams.setParams(theme);
	mainParams.setSelecetObjs(selOb);
	TeQuerier* mainQuerier = new TeQuerier(mainParams);

	TeGeomRep themeGeomRep = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;

	if(!mainQuerier->loadInstances())
		return false;

	//get attribute list from querier
	TeAttributeList attribs = mainQuerier->getAttrList();
	TeAttributeList::iterator it = attribs.begin();
	while(it != attribs.end())
	{
		attrList.push_back (*it);
		++it;
	}

	//verify if there are duplicate names
	changeAttrList(attrList);
	attrList[0].rep_.isPrimaryKey_ = true;  //link attribute is primary key
		
	// --------- create table 
	TeTable attrTable (newLayer->name(), attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);

	if(!newLayer->createAttributeTable(attrTable))
        return false;

	int newId = 0;

	// ---------------------------------- Begin main theme
	TeSTInstance st;
	while(mainQuerier->fetchInstance(st))
	{
		TeTableRow row;
		vector<string>& vect = st.getProperties();
		row.push_back(Te2String(++newId));

		//insert os valores do tema principal
		for(int p=0; p<(int)vect.size();++p)
			row.push_back(vect[p]);

		attrTable.add(row);
				
		//insert in this layer only the visible geometries of the theme 
		if(!multiGeometryToLayer(st.geometries(), Te2String(newId), newLayer, 0, themeGeomRep))
			return false; 
		
		if((newId%100)==0)
		{
			//insert attributes
			newLayer->saveAttributeTable(attrTable); //save 100 records!!!
			if(attrTable.size()>0)
				insertAttr = true;
			attrTable.clear();
		}
	} 	//while fetchInstance no querier

	
	// save the last records 
	newLayer->saveAttributeTable(attrTable); 
	if(attrTable.size()>0)
		insertAttr = true;
	attrTable.clear();
	st.clear();
	delete mainQuerier;

	// ---------------------------------- End main theme

	for(int i=0; i<(int)themeVec.size(); ++i)
	{
		TeTheme* temp = themeVec[i];
		TeProjection* projTemp = temp->layer()->projection();
		
		TeQuerierParams params(true, true);

		params.setParams(temp);
		params.setSelecetObjs (selOb);
		TeQuerier* querier = new TeQuerier(params);

		TeGeomRep tempGeomRep = (TeGeomRep)(temp->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;

		if(!querier->loadInstances())
			return false;

		attribs.clear();
		attribs = querier->getAttrList();
		vector<int> indexVec;

		for(int k=0; k<(int)attrList.size(); ++k)
		{
			string name = attrList[k].rep_.name_;
			int ind = -1;
			
			for(int p=0; p<(int)attribs.size();++p)
			{
				if(TeConvertToUpperCase(name) == TeConvertToUpperCase(attribs[p].rep_.name_))
				{
					ind = p;
					break;
				}
			}

			indexVec.push_back(ind);
		}
					
		while(querier->fetchInstance(st))
		{
			TeTableRow row;
			vector<string>& vect = st.getProperties();
			row.push_back(Te2String(++newId));

			//insert the values when the names are equals
			for(int p=1; p<(int)attrList.size();++p)
			{
				int s = indexVec[p];
				if(s<0)
					row.push_back("");
				else
					row.push_back(vect[s]);
			}

			attrTable.add(row);
					
			if(!multiGeometryToLayer(st.geometries(), Te2String(newId), newLayer, projTemp, tempGeomRep))
				return false; 
										
			if((newId%100)==0)
			{
				//insert attributes
				newLayer->saveAttributeTable(attrTable); //save 100 records!!!
				if(attrTable.size()>0)
					insertAttr = true;
				attrTable.clear();
			}
		}  //while fetchInstance no querier
	
		newLayer->saveAttributeTable(attrTable); //save the last records!!!
		if(attrTable.size()>0)
			insertAttr = true;
		attrTable.clear();
		st.clear();
	}

	if(!newLayer->box().isValid())
		return false;

	if(!insertAttr)
		return false; 

	return true;
}

bool TeGeoOpOverlayIntersection(TeLayer* newLayer, TeTheme* theme, TeTheme* themeTrim, 
								TeSelectedObjects selOb, TeSelectedObjects selObTrim, 
								bool attrTrim, TeAsciiFile* logFile)
{
	if((!newLayer) || (!theme) || (!themeTrim))
		return false;

// check the theme with less polygons
	bool invert = false;

	TeGeomRep geomRepTheme = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;
	TeGeomRep geomRepThemeTrim = (TeGeomRep)(themeTrim->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;

	if((geomRepTheme == TePOLYGONS) && (geomRepThemeTrim==TePOLYGONS))
	{
		int themeCount = countNumGeometries(theme, selOb);
		int themeTrimCount = countNumGeometries(themeTrim, selObTrim);

		if(themeTrimCount > themeCount)
		{
			if(geomRepTheme == TePOLYGONS)
			{
				invert = true;

				TeTheme* aux = theme;
				theme = themeTrim;
				themeTrim = aux;
			}
		}
	}

	bool insertAttr = false;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
		
	// ----------------- begin create attribute table

	TeAttributeList attrList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_"+ Te2String(newLayer->id());
	at.rep_.numChar_ = 255;
	at.rep_.isPrimaryKey_ = true;
	at.rep_.isAutoNumber_ = false;
	attrList.push_back(at);

	TeAttrTableVector attrTablesAux;

	if(invert)
		themeTrim->getAttTables(attrTablesAux, TeAttrStatic);		
	else
		theme->getAttTables(attrTablesAux, TeAttrStatic);		

	for(unsigned int idx = 0; idx < attrTablesAux.size(); ++idx)
	{
		TeAttributeList& attlistAux = attrTablesAux[idx].attributeList();

		TeAttributeList::iterator it = attlistAux.begin();

		while(it != attlistAux.end())
		{
			TeAttribute at = *it;
			at.rep_.isPrimaryKey_ = false;

			attrList.push_back(at);
			++it;
		}
	}

	attrTablesAux.clear();

	if(attrTrim)
	{
		if(invert)
			theme->getAttTables(attrTablesAux, TeAttrStatic);	
		else
			themeTrim->getAttTables(attrTablesAux, TeAttrStatic);	

		for(unsigned int idx = 0; idx < attrTablesAux.size(); ++idx)
		{
			TeAttributeList& attlistAux = attrTablesAux[idx].attributeList();

			TeAttributeList::iterator it = attlistAux.begin();

			while(it != attlistAux.end())
			{
				TeAttribute at = *it;
				at.rep_.isPrimaryKey_ = false;

				attrList.push_back(at);
				++it;
			}
		}
	}	

	changeAttrList(attrList);

	attrList[0].rep_.isPrimaryKey_ = true;  //link attribute is primary key
	
	TeTable attrTable(newLayer->name(), attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);

	if(!newLayer->createAttributeTable(attrTable))
        return false;

// query geometries

	TeLayer* layer = theme->layer();
	TeLayer* layerTrim = themeTrim->layer();

	TeProjection* proj = layer->projection();
	TeProjection* projTrim = layerTrim->projection();

	//Querier Trim
	TeQuerierParams paramsTrim(true, true);
	paramsTrim.setParams(themeTrim);

	if(invert)
		paramsTrim.setSelecetObjs(selOb);
	else
		paramsTrim.setSelecetObjs(selObTrim);

	TeQuerier  querierTrim(paramsTrim); 

	if(!querierTrim.loadInstances())
		return  false;
	
	// ----------------- End create attribute table
	int numPolygonsTrim = querierTrim.numElemInstances();
	// set progress bar
	if(TeProgress::instance())
	{
		string caption = "Overlay Intersection";
		string msg = "Executing Overlay Intersection. Please, wait!";
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(numPolygonsTrim);
		t2 = clock();
		t0 = t1 = t2;
	}

	double tol = TeGetPrecision(proj);
	TePrecision::instance().setPrecision(tol);

	int steps = 0;
	TeSTInstance trim;
	int newId = 0;

// this map stores the id of boundary cells, that we need to decide atribute value
	map<string, map<string, double> > cellMap;

	while(querierTrim.fetchInstance(trim))
	{
		TePolygonSet polSet;
		if(!trim.getGeometry(polSet))
			continue;

		TeTableRow rowTrim;
		if(invert || attrTrim)
		{
			//fill attribute values from trim
			vector<string>& vecTrim = trim.getProperties();

			for(int p=0; p<(int)vecTrim.size();++p)
				rowTrim.push_back(vecTrim[p]);
		}

		TePolygonSet ps;
		TeLineSet	ls;
		TePointSet pts;
		TeCellSet cs;

		for(int i=0; i<(int)polSet.size(); ++i)
		{
			TePolygon polyTrim = polSet[i];
						
			if(!((*proj) == (*projTrim)))
			{
				TePolygon pout;
				TeVectorRemap (polyTrim, projTrim, pout, proj);
				polyTrim = pout;
				polyTrim.objectId(polSet[i].objectId());
			}

			// before intersection, valid polygon holes (union of holes)
			TePolygonSet polyTrimSet;
			if(!TeOVERLAY::TeValidPolygonHoles(polyTrim, polyTrimSet))
			{
				// when the operation returns false, write in the log a possible
				// inconsistency in the data
				if (logFile)
				{
					string mess = "Nao foi possivel realizar a consistencia dos aneis internos ";
					mess += polyTrim.objectId();
					mess += " do tema " + themeTrim->name() + ".";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}

				continue;
			}

			TeBox boxTrim = polyTrim.box();

			//Querier - theme that will be clip
			TeQuerierParams params(true, true);
			params.setParams(theme);

			if(invert)
				params.setSelecetObjs(selObTrim);
			else
				params.setSelecetObjs(selOb);

			params.setSpatialRest(boxTrim, TeINTERSECTS, geomRepTheme);

			TeQuerier querier(params);

			if(!querier.loadInstances())
				continue;
			
			TeSTInstance sti;
			while(querier.fetchInstance(sti))
			{
				TeTableRow row;
				vector<string>& vect = sti.getProperties();
				row.push_back("");

				if(invert)
				{
					for(int i=0; i<(int)rowTrim.size(); ++i)
						row.push_back(rowTrim[i]);

					if(attrTrim)
					{
						//insert values of the theme that is been clip 
						for(int p=0; p<(int)vect.size();++p)
							row.push_back(vect[p]);
					}
				}
				else
				{
					//insert values of the theme that is been clip 
					for(int p=0; p<(int)vect.size();++p)
						row.push_back(vect[p]);

					if(attrTrim)
					{
						for(int i=0; i<(int)rowTrim.size(); ++i)
							row.push_back(rowTrim[i]);
					}
				}	
				
				if(sti.hasPolygons() && (geomRepTheme==TePOLYGONS))
				{
					TePolygonSet polSetTheme;
					sti.getGeometry (polSetTheme);
					TePolygonSet polSetAux;
					TePolygonSet resultPS;

					for(int j=0; j<(int)polSetTheme.size(); ++j)
					{
						if(TeIntersects(polSetTheme[j].box(), polyTrim.box()))
							polSetAux.add(polSetTheme[j]);
					}

					// before intersection, valid polygon holes (union of holes)
					TePolygonSet polSetValidHoles;
					if(!TeOVERLAY::TeValidPolygonHoles(polSetAux, polSetValidHoles))
					{
						// when the operation returns false, write in the log a possible
						// inconsistency in the data
						if (logFile)
						{
							string mess = "Nao foi possivel realizar a consistencia dos aneis internos ";
							mess += polyTrim.objectId();
							mess += " do tema " + themeTrim->name() + " e ";
							mess += polSetTheme[0].objectId();
							mess += theme->name();
							logFile->writeString(mess);
							logFile->writeNewLine();
						}

						continue;
					}

					if(!TeOVERLAY::TeIntersection(polyTrimSet, polSetValidHoles, resultPS))
					{
						// when the operation returns false, write in the log a possible
						// inconsistency in the data
						if (logFile)
						{
							string mess = "Possivel inconsistencia na interseccao do objeto ";
							mess += polyTrim.objectId();
							mess += " do tema " + themeTrim->name() + " e ";
							mess += polSetTheme[0].objectId();
							mess += theme->name();
							logFile->writeString(mess);
							logFile->writeNewLine();
						}
					}

					if(resultPS.empty())
						continue;

					newId++;
					string strnewid = Te2String(newId);
					row[0] = strnewid;
					attrTable.add(row);

					for(int c=0; c<(int)resultPS.size(); ++c)
					{							
						resultPS[c].objectId(strnewid);
						ps.add(resultPS[c]);
					}
				}
				else if (sti.hasLines() && (geomRepTheme==TeLINES))
				{
					TeLineSet lineSet;
					sti.getGeometry (lineSet);

					for(int j=0; j<(int)lineSet.size(); ++j)
					{
						TeLineSet lsAux, resultLS;
						TeMultiGeometry resultMGEOM;
						lsAux.add(lineSet[j]);
						resultMGEOM = TeOVERLAY::TeIntersection(lsAux, polyTrimSet);

						resultMGEOM.getGeometry(resultLS);
						
						for(int c=0; c<(int)resultLS.size(); ++c)
						{
							newId++;
							TeLine2D laux = resultLS[c];
							laux.objectId(Te2String(newId));

							ls.add(laux);
							row[0] = Te2String(newId);
							attrTable.add(row);
						}
					}
				}
				else if (sti.hasPoints() && (geomRepTheme==TePOINTS))
				{
					TePointSet pointSet;
					sti.getGeometry (pointSet);

					for(int j=0; j<(int)pointSet.size(); ++j)
					{
						if(TeWithin(pointSet[j], polyTrim))
						{
							newId++;
							pointSet[j].objectId(Te2String(newId));
							pts.add(pointSet[j]);
							row[0] = Te2String(newId);
							attrTable.add(row);
						}
					}
				}
				else if (sti.hasCells() && (geomRepTheme==TeCELLS))
				{
					TeCellSet cellSet;
					sti.getGeometry (cellSet);					

					for(int j=0; j<(int)cellSet.size(); ++j)
					{
						TePolygon pCell = TeMakePolygon(cellSet[j].box());
						
						short rel = TeRelation(pCell, polyTrim);

						if((rel & TeWITHIN) || (rel & TeCOVEREDBY))
						{
							newId++;
							cellSet[j].objectId(Te2String(newId));
							cs.add(cellSet[j]);
							row[0] = Te2String(newId);
							attrTable.add(row);
						}
						else
						{							
							TePolygonSet pCells;
							pCells.add(pCell);
							TePolygonSet resultPS;

							if(!TeOVERLAY::TeIntersection(pCells, polyTrimSet, resultPS))
							{
								// when the operation returns false, write in the log a possible
								// inconsistency in the data
								if(logFile)
								{
									string mess = "Possivel inconsistencia na interseccao do objeto ";
									mess += polyTrim.objectId();
									mess += " do tema " + themeTrim->name() + " e ";
									mess += cellSet[j].objectId();
									mess += theme->name();
									logFile->writeString(mess);
									logFile->writeNewLine();
								}

								continue;
							}

							if(resultPS.empty())
								continue;

							double area = TeGeometryArea(resultPS);

							insertArea(cellSet[j].objectId(), polyTrim.objectId(), area, cellMap);
						}
					}

				}
			} //while fetchInstance 
		}//for each polygon of the Trim theme

		//inserir a geometria no banco
		if(geomRepTheme == TePOLYGONS && ps.size())
			newLayer->addPolygons(ps);
		else if(geomRepTheme == TeLINES && ls.size())
			newLayer->addLines(ls);
		else if(geomRepTheme == TePOINTS && pts.size())
			newLayer->addPoints(pts);
		else if(cs.size())
		{
			cs.resX(theme->layer()->getRepresentation(TeCELLS)->resX_);
			cs.resY(theme->layer()->getRepresentation(TeCELLS)->resY_);
			newLayer->addCells(cs);
		}

		trim.clear();

		steps++;
		t2 = clock();
		if ((TeProgress::instance()) && (int(t2-t1) > dt))
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(steps);
		}

		if((steps%100)==0)
		{
			newLayer->saveAttributeTable(attrTable); //save 100 records!!!
			if(attrTable.size()>0)
				insertAttr = true;
			attrTable.clear();
		}
	} //while fetch instance querier Trim

	newLayer->saveAttributeTable(attrTable); //save the last records!!!
	if(attrTable.size()>0)
		insertAttr = true;

	if(TeProgress::instance())
			TeProgress::instance()->reset();

	if(!cellMap.empty())
	{
		bool res = false;
		if(attrTrim)
			res = insertBoundaryCells(newId, newLayer, cellMap, theme, themeTrim);
		else
			res = insertBoundaryCells(newId, newLayer, cellMap, theme, 0);

		if(!res)
		{
			if(TeProgress::instance())
				TeProgress::instance()->reset();

			return false;
		}
	}

	if(!newLayer->box().isValid())
		return false;

	if(!insertAttr)
		return false; 

	return true;
}

bool TeGeoOpAssignDataLocationDistribute(TeTheme* themeMod, TeTheme* theme, const string& tableName, 
										 const int& spatialRelation, const vector<string>& attributes, TeAsciiFile* logFile)
{
	if((!themeMod) || (!themeMod->layer()) ||  (!theme))
		return false;

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;

	TeLayer* layerMod = themeMod->layer();	
	TeProjection* projMod = layerMod->projection();
	TeProjection* proj = theme->layer()->projection();
	
	TeGeomRep geomRepThemeMod = (TeGeomRep)(themeMod->visibleGeoRep() & ~TeTEXT);
	TeGeomRep geomRepTheme = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT);

	//Querier from theme 
	TeQuerierParams params(true, true);
	params.setParams(theme);
	if (!attributes.empty())
		params.setFillParams(true,false,attributes);
	TeQuerier  querier(params); 

	if(!querier.loadInstances())
		return  false;

	// ----------------- begin create attribute table

	TeAttributeList attrList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_";
	at.rep_.numChar_ = 255;
	at.rep_.isPrimaryKey_ = true;
	at.rep_.isAutoNumber_ = false;
	attrList.push_back(at);

	//get attribute list from querier
	TeAttributeList attribs = querier.getAttrList();
	TeAttributeList::iterator it = attribs.begin();
	while(it != attribs.end())
	{
		attrList.push_back (*it);
		++it;
	}
						
	changeAttrList(attrList);
	attrList[0].rep_.isPrimaryKey_ = true;  //link attribute is primary key
	
	TeTable attrTable (tableName, attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);

	if(!layerMod->createAttributeTable(attrTable))
        return false;

	//empty rows
	TeTable updateAttrTable (tableName, attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);
	
	// ----------------- End create attribute table
	vector<string> objIds;
	if( (geomRepTheme == TePOLYGONS) || 
		(geomRepTheme == TeCELLS)    ||
		((geomRepThemeMod == TeLINES) && (geomRepTheme == TeLINES))) 
	{
		int numElem = querier.numElemInstances();
		
		// set progress bar
		if(TeProgress::instance())
		{
			string caption = "Spatial Join";
			string msg = "Executing Spatial Join. Please, wait!";
			TeProgress::instance()->setCaption(caption.c_str());
			TeProgress::instance()->setMessage(msg);
			TeProgress::instance()->setTotalSteps(numElem);
			t2 = clock();
			t0 = t1 = t2;
		}
	
		int steps = 0;
		TeSTInstance sti;

		double tol = TeGetPrecision(projMod);
		TePrecision::instance().setPrecision(tol);	

		while(querier.fetchInstance(sti))
		{
			// Get attributes
			TeTableRow row;
			TeTableRow emptyRow;

			row.push_back("");
			emptyRow.push_back("");

			vector<string>& vec = sti.getProperties();
			for(int p=0; p<(int)vec.size();++p)
			{
				row.push_back(vec[p]);
				emptyRow.push_back ("");
			}
			
			//Get geometry
			TeMultiGeometry mGeom;
			sti.getGeometry(mGeom);

			vector<TeGeometry*> geoms;
			mGeom.getGeometry(geoms);

			for(int i=0; i<(int)geoms.size(); ++i)
			{
				if(!((*proj) == (*projMod)))
				{
					TeGeometry* gout; 
					if(geoms[i]->elemType()==TePOLYGONS && geomRepTheme==TePOLYGONS)
					{
						gout = new TePolygon;
						TeVectorRemap (*((TePolygon*)geoms[i]), proj, *((TePolygon*)gout), projMod);
					}
					else if (geoms[i]->elemType() == TeCELLS && geomRepTheme==TeCELLS)
					{
						gout = new TeCell;
						TeVectorRemap (*((TeCell*)geoms[i]), proj, *((TeCell*)gout), projMod);
					}
					else
					{
						gout = new TeLine2D;
						TeVectorRemap (*((TeLine2D*)geoms[i]), proj, *((TeLine2D*)gout), projMod);
					}
					gout->objectId(geoms[i]->objectId());
					delete geoms[i];
					geoms[i] = gout;
				}
				
				//querier that will be modified
				TeQuerierParams paramsMod(false, false);
				paramsMod.setParams(themeMod);
				paramsMod.setSpatialRest(geoms[i], spatialRelation, geomRepThemeMod);
				TeQuerier querierMod(paramsMod);

				if(!querierMod.loadInstances())
					continue;
				
				TeSTInstance stMod;
				while(querierMod.fetchInstance(stMod))
				{
					string obj = stMod.objectId();
					if( find(objIds.begin(), objIds.end(), obj) == objIds.end())
					{
						objIds.push_back (obj);
						row[0] = obj;
						attrTable.add(row);
					}
					else 
					{
						emptyRow[0] = obj;
						updateAttrTable.add(emptyRow);

						if (logFile)
						{
							std::string mess = "O ponto [" + obj + "] se relaciona com mais de um poligono.";
							logFile->writeString(mess);
							logFile->writeNewLine();
						}
					}
				}
				
				if(attrTable.size()>100)
				{
					layerMod->saveAttributeTable(attrTable); //save 100 records!!!
					layerMod->database()->updateTable(updateAttrTable);
					attrTable.clear();
					updateAttrTable.clear();
				}
			} // for
			
			for(int j=0; j<(int)geoms.size(); ++j)
				delete (geoms[j]);  //delete allocated geometries 

			steps++;
			t2 = clock();
			if ((TeProgress::instance()) && (int(t2-t1) > dt))
			{
				t1 = t2;
				if(TeProgress::instance()->wasCancelled())
				{
					TeProgress::instance()->reset();
					return false;
				}
				if((int)(t2-t0) > dt2)
					TeProgress::instance()->setProgress(steps);
			}

		} //while 
	} //if geometry type
	else 
	{
		return false;  //use nearest neighbor
	}
				
	if(TeProgress::instance())
		TeProgress::instance()->reset();

	layerMod->saveAttributeTable(attrTable); //save the last records!!!
	layerMod->database()->updateTable(updateAttrTable);
	
	return true;
}


bool TeGeoOpOverlayUnion(TeLayer* newLayer, TeTheme* theme, TeTheme* themeOverlay, 
						 TeSelectedObjects selOb, TeSelectedObjects selObOverlay, TeAsciiFile* logFile)
{
///////////////////////////////////////////////////////////////////
return false;
// verifivar se vai trabalhar com celulas
// se vai, modificar a interface.....
///////////////////////////////////////////////////////////////////
	if((!newLayer) || (!theme) || (!themeOverlay))
		return false;

	bool insertAttr = false;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
		
	TeLayer* layer = theme->layer();
	TeLayer* layerOverlay = themeOverlay->layer();

	TeGeomRep geomRepTheme = (TeGeomRep)(theme->visibleGeoRep() & ~TeTEXT);
	
	TeProjection* proj = layer->projection();
	TeProjection* projOverlay = layerOverlay->projection();

	//Querier Overlay
	TeQuerierParams paramsOverlay(true, true);
	paramsOverlay.setParams(themeOverlay);
	paramsOverlay.setSelecetObjs(selObOverlay);
	TeQuerier  querierOverlay(paramsOverlay); 

	if(!querierOverlay.loadInstances())
		return  false;

	// ----------------- begin create attribute table

	TeAttributeList attrList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_"+ Te2String(newLayer->id());
	at.rep_.numChar_ = 255;
	at.rep_.isPrimaryKey_ = true;
	at.rep_.isAutoNumber_ = false;
	attrList.push_back(at);

	//theme
	TeQuerierParams paramsTemp(false, true);
	paramsTemp.setParams(theme);
	TeQuerier* querierTemp = new TeQuerier(paramsTemp);

	if(!querierTemp->loadInstances())
		return false;

	//get attribute list from querier
	TeAttributeList attribs = querierTemp->getAttrList();
	TeAttributeList::iterator it = attribs.begin();
	while(it != attribs.end())
	{
		attrList.push_back (*it);
		++it;
	}
						
	delete querierTemp;

	//overlay
	attribs.clear();
	attribs = querierOverlay.getAttrList(); 

	//fill attributes from Overlay theme 
	it = attribs.begin();
	while(it != attribs.end())
	{
		attrList.push_back (*it);
		++it;
	}

	changeAttrList(attrList);
	attrList[0].rep_.isPrimaryKey_ = true;  //link attribute is primary key
	
	TeTable attrTable (newLayer->name(), attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);

	if(!newLayer->createAttributeTable(attrTable))
        return false;
	
	// ----------------- End create attribute table
	int numPolygonsOverlay = querierOverlay.numElemInstances();
	// set progress bar
	if(TeProgress::instance())
	{
		string caption = "Overlay Union";
		string msg = "Executing Overlay Union. Please, wait!";
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(numPolygonsOverlay);
		t2 = clock();
		t0 = t1 = t2;
	}

	double tol = TeGetPrecision(layer->projection());
	TePrecision::instance().setPrecision(tol);	

	int steps = 0;
	TeSTInstance Overlay;
	int newId = 0;

	while(querierOverlay.fetchInstance(Overlay))
	{
		TePolygonSet polSet;
		if(!Overlay.getGeometry(polSet))
			continue;

		TeTableRow rowOverlay;
		//fill attribute values from Overlay
		vector<string>& vecOverlay = Overlay.getProperties();

		for(int p=0; p<(int)vecOverlay.size();++p)
			rowOverlay.push_back(vecOverlay[p]);

		TePolygonSet ps;
//		TeCellSet cs;

		for(int i=0; i<(int)polSet.size(); ++i)
		{
			TePolygon polyOverlay = polSet[i];
						
			if(!((*proj) == (*projOverlay)))
			{
				TePolygon pout;
				TeVectorRemap (polyOverlay, projOverlay, pout, proj);
				pout.objectId(polyOverlay.objectId());
				polyOverlay = pout;
			}

			TePolygonSet polyOverlaySet;
			polyOverlaySet.add(polyOverlay);

			TeBox boxOverlay = polyOverlay.box();

			//Querier - theme that will be clip
			TeQuerierParams params(true, true);
			params.setParams(theme);
			params.setSelecetObjs(selOb);
			params.setSpatialRest(boxOverlay, TeINTERSECTS, geomRepTheme);

			TeQuerier querier(params);

			if(!querier.loadInstances())
				continue;
			
			TeSTInstance sti;
			while(querier.fetchInstance(sti))
			{
				TeTableRow row;
				vector<string>& vect = sti.getProperties();
				row.push_back("");

				//insert values of the theme that is been clip 
				for(int p=0; p<(int)vect.size();++p)
					row.push_back(vect[p]);

				for(int i=0; i<(int)rowOverlay.size(); ++i)
					row.push_back(rowOverlay[i]);
				
				if(sti.hasPolygons() && (geomRepTheme==TePOLYGONS))
				{
					TePolygonSet polSet;
					sti.getGeometry (polSet);

					for(int j=0; j<(int)polSet.size(); ++j)
					{
						TePolygonSet psAux, resultPS;
						psAux.add(polSet[j]);
						if(!TeOVERLAY::TeIntersection(polyOverlaySet, psAux, resultPS))
						{
							// when the operation returns false, write in the log a possible
							// inconsistency in the data
							if (logFile)
							{
								string mess = "Possivel inconsistencia na interseccao do objeto ";
								mess += polyOverlay.objectId();
								mess += " e ";
								mess += polSet[j].objectId();
								logFile->writeString(mess);
								logFile->writeNewLine();
							}
						}
						
						for(int c=0; c<(int)resultPS.size(); ++c)
						{
							newId++;
							TePolygon paux = resultPS[c];
							paux.objectId(Te2String(newId));
							ps.add(paux);
							row[0] = Te2String(newId);
							attrTable.add(row);
						}
					}
				}
//				else if (sti.hasCells())
//				{
//					TeCellSet cellSet;
//					sti.getGeometry (cellSet);
//
//					for(int j=0; j<(int)cellSet.size(); ++j)
//					{
//						if(TeIntersects(cellSet[j], polyOverlay))
//						{
//							newId++;
//							cellSet[j].objectId(Te2String(newId));
//							cs.add(cellSet[j]);
//							row[0] = Te2String(newId);
//							attrTable.add(row);
//						}
//					}
//				}
			} //while fetchInstance 
		}//for each polygon of the Overlay theme

		//inserir a geometria no banco
		if(geomRepTheme==TePOLYGONS && ps.size())
			newLayer->addPolygons(ps);
//		else if(cs.size())
//			newLayer->addCells(cs);

		Overlay.clear();

		steps++;
		t2 = clock();
		if ((TeProgress::instance()) && (int(t2-t1) > dt))
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(steps);
		}

		if((steps%100)==0)
		{
			newLayer->saveAttributeTable(attrTable); //save 100 records!!!
			if(attrTable.size()>0)
				insertAttr = true;
			attrTable.clear();
		}
	} //while fetch instance querier Overlay

	if(TeProgress::instance())
			TeProgress::instance()->reset();

	newLayer->saveAttributeTable(attrTable); //save the last records!!!
	if(attrTable.size()>0)
		insertAttr = true;

	if(!newLayer->box().isValid())
		return false;

	if(!insertAttr)
		return false; 

	return true;
}


bool TeGeoOpAssignByLocationCollect(TeTheme* restrTheme, TeTheme* srcTheme, 
									const string& newTableName, TeGroupingAttr& measuresColl, 
									const int& spatialRelation)
{
	if(!restrTheme || !restrTheme->layer() ||		// no spatial restriction theme
	   !srcTheme ||									// no source theme
	   newTableName.empty() ||						// no output table name 
	   measuresColl.empty())
	   return false;

	// verifies if the souce and restriction themes are compatible
	TeGeomRep restrRep = (TeGeomRep)(restrTheme->visibleGeoRep() & ~TeTEXT);
	TeGeomRep srcRep =  (TeGeomRep)(srcTheme->visibleGeoRep() & ~TeTEXT);
	TeRaster* rst = 0;
	if (srcRep == TeRASTER)
	{
		if (restrRep != TePOINTS)
			return false;
		rst = srcTheme->layer()->raster();
		if (!rst)
			return false;
	}
	else
	{
		if (restrRep != TePOLYGONS && restrRep != TeCELLS)	// no set of measures to collect
			return false;
	}

	TeProjection* restrProj = restrTheme->layer()->projection();
	TeProjection* srcProj	= srcTheme->layer()->projection();
	bool doRemap = !(*restrProj == *srcProj);

	// try to retrieve the geometries of the spatial restriction theme
	TeQuerierParams params(true, false);
	params.setParams(restrTheme);
	TeQuerier spatialRestrictions(params); 
	if(!spatialRestrictions.loadInstances())
		return false;

	// there is at least one restriction geometry, so create the output table
	// defines its attribute list

	TeAttributeList attrList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "object_id_";
	at.rep_.numChar_ = 255;
	attrList.push_back(at);
	vector<int>  countAttr;
	int attrNum = 1;
	vector<int> bandCollection;

	TeGroupingAttr::iterator it = measuresColl.begin();		// for each different measure
	while (it != measuresColl.end())						// create a new attribute
	{
		at.rep_ = it->first;
		at.rep_.isPrimaryKey_ = false;
		string nameTemp = it->first.name_ + "_" + getStringFromStatistic(it->second);
		if(((restrTheme->layer()->database()->dbmsName()=="OracleAdo") || 
			(restrTheme->layer()->database()->dbmsName()=="OracleSpatial") ) &&
			(nameTemp.size() > 30))
		{
			int s = nameTemp.size(); 
			at.rep_.name_ = nameTemp.substr(s-30, s-1);
		}
		else
			at.rep_.name_ = nameTemp;

		if (srcRep == TeRASTER)
		{
			int nn = at.rep_.name_.find("_");
			int b = atoi(at.rep_.name_.substr(4,nn-4).c_str());
			bandCollection.push_back(b);
			if (rst->params().dataType_[b]== TeFLOAT || 
				rst->params().dataType_[b]== TeDOUBLE)
				at.rep_.type_ = TeREAL;
			else
				at.rep_.type_ = TeINT;
			attrList.push_back(at);
			++it;
			continue;
		}
			
		if (it->second == TeCOUNT || it->second == TeVALIDCOUNT)
		{
			at.rep_.type_ = TeINT;
			countAttr.push_back (attrNum);
		}
		else if (at.rep_.type_ == TeINT)
			at.rep_.type_ = TeREAL;

		attrList.push_back(at);
		++it;
		++attrNum;
	}	

	changeAttrList(attrList);	// validates attribute list						
	attrList[0].rep_.isPrimaryKey_ = true;
	attrList[0].rep_.isAutoNumber_ = false;

	TeTable measuresTable (newTableName, attrList, attrList[0].rep_.name_, attrList[0].rep_.name_);
	if(!restrTheme->layer()->createAttributeTable(measuresTable)) // creates the new attribute table
        return false;

	//use function to point and cell
	TeDatabase* db = restrTheme->layer()->database();
	int spatialRes = TeOVERLAPS | TeWITHIN | TeCOVEREDBY | TeCROSSES;
	bool sqlGroup = false;
	
	if( (*restrProj==*srcProj) && 
		(((restrRep==TeCELLS) && (srcRep==TePOINTS)) ||
		((restrRep==TeCELLS) && (srcRep==TeCELLS) && ((spatialRelation==TeINTERSECTS) || 
		(spatialRelation==spatialRes)))))
	{
		if(TeCellStatistics(restrTheme, measuresTable, measuresColl, srcTheme, restrTheme->layer()->database()))
			sqlGroup = db->insertTableInfo(restrTheme->layer()->id(),measuresTable);
	}

	if(sqlGroup)
		return true;

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	if(TeProgress::instance())								// sets the progress bar
	{
		string caption = "Assign Data By Location";
		string msg = "Executing collecting. Please, wait!";
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(spatialRestrictions.numElemInstances());
		t2 = clock();
		t0 = t1 = t2;
	}

	double tol = TeGetPrecision(srcProj);
	TePrecision::instance().setPrecision(tol);	

	unsigned int nregions = 0;
	TeSTInstance restrInstance;

	// a special case is when the source theme has a raster representation
	if (srcRep == TeRASTER && restrRep == TePOINTS)
	{
		TePointSet restrPoints;
		unsigned int npt;
		unsigned int nbs;
		while (spatialRestrictions.fetchInstance(restrInstance)) 
		{
			restrPoints.clear();
			TeMultiGeometry mGeom;
			restrInstance.getGeometry(mGeom);
			restrPoints = mGeom.getPoints();
			if (doRemap)
			{
				TePointSet restrPointAux;
				TeVectorRemap(restrPoints,restrProj,restrPointAux,srcProj);
				restrPoints.clear();
				restrPoints = restrPointAux;
			}
			TeCoord2D lc;
			double val;
			string sval;

			TeTableRow row;
			int nrows = 0;
			for (npt=0; npt<restrPoints.size(); ++npt)
			{
				row.clear();
				row.push_back(restrInstance.objectId());	// identify the instance
				for (nbs=0; nbs<bandCollection.size();++nbs)
				{
					TeCoord2D loc = restrPoints[npt].location();
					lc = rst->params().coord2Index(loc);
					if (rst->getElement(TeRoundRasterIndex(lc.x()),TeRoundRasterIndex(lc.y()),val,nbs))
					{
						if (rst->params().dataType_[0] == TeUNSIGNEDCHAR ||
						    rst->params().dataType_[0] == TeCHAR)
							sval = Te2String(static_cast<int>(val));
						else
							sval = Te2String(val,5);
						row.push_back(sval);
					}	
					else
						row.push_back("");
				}
			}
			measuresTable.add(row);
			row.clear();
			nrows++;	
			if (measuresTable.size()>100)	// saves chunks of 100 records
			{
				restrTheme->layer()->saveAttributeTable(measuresTable); 
				measuresTable.clear();
			}
			++nregions;
			t2 = clock();
			if ( int(t2-t1) > dt && TeProgress::instance())
			{
				t1 = t2;
				if(TeProgress::instance()->wasCancelled())
				{
					TeProgress::instance()->reset();
					return false;
				}
				if((int)(t2-t0) > dt2)
					TeProgress::instance()->setProgress(nregions);
			}
		}
		if (measuresTable.size() > 0)	// saves remaining records
		{
			restrTheme->layer()->saveAttributeTable(measuresTable); 
			measuresTable.clear();
		}
		if (TeProgress::instance())
			TeProgress::instance()->reset();
		return true;
	}

	TePolygonSet restrPols;
	TeCellSet restrCells;
	while(spatialRestrictions.fetchInstance(restrInstance)) // collect data for each geometry
	{	
		// of the spatial restriction set
		// defines a querier to select measures from objects that
		// are delimited by the a geometry and a spatial restriction
		TeQuerierParams par2(false,measuresColl);
		par2.setParams(srcTheme);
		restrCells.clear();
		restrPols.clear();
		TeMultiGeometry mGeom;
		restrInstance.getGeometry(mGeom);
		// get the geometry of the instance
		if (restrRep == TeCELLS)						// restriction is given by a cell set geometry
		{
			mGeom.getGeometry(restrCells);
			if (doRemap)						// bring it to the source projection
			{									// if necessary
				TeCellSet restrCellsAux;
				TeVectorRemap(restrCells,restrProj,restrCellsAux,srcProj);
				restrCells.clear();
				restrCells = restrCellsAux;
			}
			par2.setSpatialRest(&restrCells,spatialRelation,srcRep);
		}
		else									// restriction is given by a cell set geometry
		{
			mGeom.getGeometry(restrPols);
			if (doRemap)						// bring it to the source projection
			{									// if necessary
				TePolygonSet restrPolsAux;
				TeVectorRemap(restrPols,restrProj,restrPolsAux,srcProj);
				restrPols.clear();
				restrPols = restrPolsAux;
			}
			par2.setSpatialRest(&restrPols,spatialRelation,srcRep);
		}
		// creates a querier to retrieve the desired measures of the objects inside the 
		// spatial restriction
		TeQuerier sourceMeasures(par2);

		// process each source within the spatial restriction geometry
		TeTableRow row;
		int nrows = 0;
		row.push_back(restrInstance.objectId());	// identify the instance
		
		if (sourceMeasures.loadInstances())
		{
			TeSTInstance sourceInstance;
			unsigned int p;								// get the calculated attributes
			sourceMeasures.fetchInstance(sourceInstance);
			vector<string>& vec = sourceInstance.getProperties();
			for(p=0; p<vec.size();++p)
				row.push_back(vec[p]);
		}
		else 
		{
			//insert null or zero attribute value
			for(int p=1; p<(int)attrList.size();++p)
			{
				if(find(countAttr.begin(), countAttr.end(), p) != countAttr.end())
					row.push_back(Te2String(0));
				else
					row.push_back("");
			}
		}
	
		measuresTable.add(row);
		row.clear();
		nrows++;
			
		if(measuresTable.size()>100)	// saves chunks of 100 records
		{
			restrTheme->layer()->saveAttributeTable(measuresTable); 
			measuresTable.clear();
		}

		++nregions;
		t2 = clock();
		if ( int(t2-t1) > dt && TeProgress::instance())
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(nregions);
		}
	}
	if (measuresTable.size() > 0)	// saves remaining records
	{
		restrTheme->layer()->saveAttributeTable(measuresTable); 
		measuresTable.clear();
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}


bool TeGeoOpOverlayDifference(TeLayer* newLayer, TeTheme* theme1, 
							  TeTheme* theme2, TeSelectedObjects selOb1, 
							  TeSelectedObjects selOb2, TeAsciiFile* logFile)
{
	if((!newLayer) || (!theme1) || (!theme2))
		return false;

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
		
	TeLayer* layer1 = theme1->layer();
	TeLayer* layer2 = theme2->layer();

	unsigned int t;
	vector<string> linkNames;
	TeAttrTableVector vetTables;
	if (layer1->getAttrTables(vetTables,TeAttrStatic))
	{
		for (t=0; t<vetTables.size(); ++t)
			linkNames.push_back(TeConvertToUpperCase(vetTables[t].linkName()));
	}
	vetTables.clear();
	if (layer1->getAttrTables(vetTables,TeAttrEvent))
	{
		for (t=0; t<vetTables.size(); ++t)
			linkNames.push_back(TeConvertToUpperCase(vetTables[t].linkName()));
	}

	TeProjection* proj1 = layer1->projection();
	TeProjection* proj2 = layer2->projection();

	// querier to get the objects from theme 1
	TeQuerierParams params1(true, true);
	params1.setParams(theme1);
	params1.setSelecetObjs(selOb1);
	TeQuerier  querier1(params1); 

	if(!querier1.loadInstances())
		return false;				// there aren't objects on theme 1

	// create the new attribute table with the attributes from theme 1 
	// preserving the same link attribute
	TeAttributeList attrList;
	int npk=-1, aux1=0;
	TeAttributeList theme1AttrList = querier1.getAttrList();
	TeAttributeList::iterator it = theme1AttrList.begin();
	while (it != theme1AttrList.end())
	{
		attrList.push_back(*it);
		vector<string>::iterator it2 = find(linkNames.begin(), linkNames.end(), TeConvertToUpperCase((*it).rep_.name_));
		if (it2 != linkNames.end())
			npk = aux1;
		++it;
		++aux1;
	}

	// change attribute names if necessary and create a new attribute table
	changeAttrList(attrList);
	attrList[npk].rep_.isPrimaryKey_ = true;
	TeTable attrTable (newLayer->name(), attrList, attrList[npk].rep_.name_, attrList[npk].rep_.name_);
	if(!newLayer->createAttributeTable(attrTable))
       return false;

	// ----- 

	int numPolygons1 = querier1.numElemInstances();
	if(TeProgress::instance())
	{
		string caption = "Overlay Difference";
		string msg = "Executing Overlay Difference. Please, wait!";
		TeProgress::instance()->reset();
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(numPolygons1);
		t2 = clock();
		t0 = t1 = t2;
	}

	double tol = TeGetPrecision(proj1);
	TePrecision::instance().setPrecision(tol);

	int steps = 0;
	TeSTInstance elem1;
	while(querier1.fetchInstance(elem1))	// for each object of theme 1
	{
		TePolygonSet polSet1Aux;
		if(!elem1.getGeometry(polSet1Aux))
			continue;
		
		TePolygonSet polSet1;
		if(!TeOVERLAY::TeValidPolygonHoles(polSet1Aux, polSet1))
			return false;

		TePolygonSet resultAux;

		// querier to the theme 2
		TeGeomRep gRepTheme2 = (TeGeomRep)(theme2->visibleGeoRep() & ~TeTEXT); // TeTEXT not is considered;
		TeQuerierParams params2(true,false);
		params2.setParams(theme2);
		params2.setSelecetObjs(selOb2);
		params2.setSpatialRest(polSet1.box(), TeINTERSECTS, gRepTheme2);

		TeQuerier querier2(params2);
		if(!querier2.loadInstances())
		{
			// there is no intersection between this object and the objects from theme 2
			// so include the entire geometry of the object in the result
			for (unsigned int i=0; i<polSet1.size(); ++i)
				resultAux.add(polSet1[i]);
		}
		else
		{
			unsigned int m;
			TePolygonSet polSet2;
			TeSTInstance sti2;
			vector<string> oids;
			// build a polygon set with the representation of all objetcs from theme 2
			// that intercepts current object of theme 1
			while(querier2.fetchInstance(sti2))  
			{
				if(sti2.hasPolygons())
				{
					TePolygonSet polSetAux2;
					sti2.getGeometry (polSetAux2);
					if (!((*proj1) == (*proj2)))
					{
						TePolygonSet polySet2Out;
						TeVectorRemap (polSetAux2, proj2, polySet2Out, proj1);
						for (m=0; m<polySet2Out.size(); m++)
							polSet2.add(polySet2Out[m]);
					}
					else
					{
						for (m=0; m<polSetAux2.size(); m++)
							polSet2.add(polSetAux2[m]);
					}
					// keep track of the object ids for logging
					oids.push_back(sti2.objectId());
				}
			}// for each intersection object of theme 2 

			// before union, valid polygon holes (union of holes)
			TePolygonSet polSetAux;
			if(!TeOVERLAY::TeValidPolygonHoles(polSet2, polSetAux))
				return false;

			// Make union of all objects from theme 2 that intersects objects from theme 1
			TePolygonSet bluePols;

			if(!TeOVERLAY::TeUnion(polSetAux, bluePols))
			{
				if(logFile)
				{
					string mess = "Possivel inconsistencia na uniao dos obejtos do tema 2 que interceptam o objeto ";
					mess += polSet1[0].objectId();
					mess += " do tema " + theme1->name();
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
			}

			// try to recover the difference between the geometry of the current object
			// from theme 1 and the geometries of the objects from theme 2 that intercept
			// current object from theme 1
			if(!TeOVERLAY::TeDifference(polSet1, bluePols, resultAux))
			{
				// when the operation returns false, writes in the error log file a possible
				// inconsistency in the data
				if (logFile)
				{
					string mess = "Possivel inconsistencia na diferenca do objeto ";
					mess += polSet1[0].objectId();
					mess += " do tema " + theme1->name() + " e ";
					if (!oids.empty())
						mess += oids[0];
					for (m=1;m<oids.size();++m)
						mess += ", " + oids[m];
					mess += " do tema " + theme2->name();
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
			} 
		}
		// save partial results
		if (!resultAux.empty())
		{
			TeTableRow row;
			vector<string>& vecOverlay = elem1.getProperties();

			for (unsigned int p=0; p<vecOverlay.size();++p)
				row.push_back(vecOverlay[p]);
			attrTable.add(row);
			row.clear();
			for (unsigned int l=0; l <resultAux.size(); l++)
				resultAux[l].objectId(vecOverlay[npk]);
			newLayer->addPolygons(resultAux);
		}
		steps++;
		t2 = clock();
		if ((TeProgress::instance()) && (int(t2-t1) > dt))
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(steps);
		}
		if((steps%100)==0)
		{
			newLayer->saveAttributeTable(attrTable); 
			attrTable.clear();
		}
	} // for each object of theme 1

	if(TeProgress::instance())
		TeProgress::instance()->reset();

	if(attrTable.size()>0)
		newLayer->saveAttributeTable(attrTable); 

	if(!newLayer->box().isValid())
		return false;
	return true;
}

bool TeGeoOpBuffer(TeLayer* newLayer, TeTheme* themeIn, const TeSelectedObjects& selOb, const TeBUFFERREGION::TeBufferType& bufferType, const double& bufferDistance, 
				   const unsigned int& bufferLevels, const unsigned int& numPoints, const bool& doUnion, const string& tableName, const string& attrName,
				   TeAsciiFile* logFile, const bool& copyInputColumns)
{
	if(!newLayer || !themeIn)
		return false;
	
	vector<string> attrs;
	bool	distBasedOnAttr = false;
	double	dist = bufferDistance;

	TeLayer* inputLayer = themeIn->layer();
	TeTable& inputTable = inputLayer->attrTables()[0];
	TeAttributeList& inputAttrList = inputTable.attributeList();

//Verify if the distance is based on an attribute	
	if(!tableName.empty() && !attrName.empty())
	{
		attrs.push_back (tableName+"."+attrName);
		distBasedOnAttr = true;
	}
	if(copyInputColumns && !doUnion)
	{
		for(unsigned int i = 0; i < inputAttrList.size(); ++i)
		{
			attrs.push_back(inputAttrList[i].rep_.name_);
		}
	}
	
	TePrecision::instance().setPrecision(TeGetPrecision(newLayer->projection()) / 10.0);
		
	//define a list of attributes
	TeAttributeList attList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 16;
	at.rep_.name_ = "object_id_";
	at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);

	at.rep_.type_ = TeINT;       
	at.rep_.name_ = "bufferLevel";
	at.rep_.isPrimaryKey_ = false;
	attList.push_back(at);  

	if(!distBasedOnAttr || !doUnion)
	{
		at.rep_.type_ = TeREAL;       
		at.rep_.name_ = "bufferDistance";
		at.rep_.isPrimaryKey_ = false;
		attList.push_back(at);  
	}

	std::set<unsigned int> setIgnoreColumns;
	if(copyInputColumns && !doUnion)
	{
		for(unsigned int i = 0; i < inputAttrList.size(); ++i)
		{
			if(inputAttrList[i].rep_.name_ == "object_id_")
			{
				setIgnoreColumns.insert(i);
				continue;
			}
			if(inputAttrList[i].rep_.name_ == "bufferLevel")
			{
				setIgnoreColumns.insert(i);
				continue;
			}
			if(!distBasedOnAttr || !doUnion)
			{
				if(inputAttrList[i].rep_.name_ == "bufferDistance")
				{
					setIgnoreColumns.insert(i);
					continue;
				}
			}
			attList.push_back(inputAttrList[i]);
		}
	}

	TeTable attTable(newLayer->name()+"_Attr", attList, "object_id_", "object_id_");	

	//create attribute table 
	if (!newLayer->createAttributeTable(attTable))
	{
		// when the operation returns false, write in the log a possible
		// inconsistency in the data
		if (logFile)
		{
			string mess = "Error creating attribute table! ";
			logFile->writeString(mess);
			logFile->writeNewLine();
		}
		return false;
	}

	//insert a polygon representation in the new layer 		
	if(!newLayer->addGeometry(TePOLYGONS, newLayer->name()+"_pols"))
	{
		// when the operation returns false, write in the log a possible
		// inconsistency in the data
		if (logFile)
		{
			string mess = "Error adding polygon table! ";
			logFile->writeString(mess);
			logFile->writeNewLine();
		}
		return false;
	}
   		
	//querier to load all objects from database	
	TeQuerierParams params(true,attrs);
	params.setParams(themeIn);
	params.setSelecetObjs(selOb);
	TeQuerier querier(params);

	if(!querier.loadInstances())
	{
		// when the operation returns false, write in the log a possible
		// inconsistency in the data
		if (logFile)
		{
			string mess = "Error loading objects from database! ";
			logFile->writeString(mess);
			logFile->writeNewLine();
		}
		return false;
	}

	//progress bar
	int total = querier.numElemInstances();
	if(TeProgress::instance())
	{
		string caption = "Buffer operation";
		string msg;
		if(doUnion)
			msg = "Executing buffer operation. Please, wait!";
		else
			msg = "Executing buffer operation and saving the \n results in the database. Please, wait!";
		TeProgress::instance()->setCaption(caption.c_str());
		TeProgress::instance()->setMessage(msg);
		TeProgress::instance()->setTotalSteps(total);
	}
	
	vector<TePolygonSet>	result;
	for(unsigned int j=0; j<bufferLevels; ++j)
		result.push_back (TePolygonSet());

	TeTableRow		row;
	TeSTInstance	sti;
	int count = 0;
	while(querier.fetchInstance(sti))
	{
		TePolygonSet	polSetIn;
		TeLineSet		lineSetIn;
		TePointSet		pointSetIn;
		TeCellSet		cellSetIn;
		vector<TePolygonSet>	bufferResult;
		
		//progress bar
		if(TeProgress::instance())
		{
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			TeProgress::instance()->setProgress(count);
		}

		//get the distance
		if(distBasedOnAttr)
		{
			dist = sti[0];
			if(dist==0) //if there is no distance to this instance, continue
			{
				++count;
				continue;
			}
		}
		
		bool bufferOperation = false;
		if(sti.hasPolygons())
		{
			sti.getGeometry(polSetIn);
			// generate the buffers of each polygon of the set (polSetIn) and do the union
			// of the same distance buffers for each object
			bufferOperation = TeBUFFERREGION::TeBufferRegion(polSetIn, dist, numPoints, bufferLevels, bufferType, bufferResult);
		}
		else if(sti.hasLines())
		{
			sti.getGeometry(lineSetIn);
			// generate the buffers of each line of the set (lineSetIn) and do the union
			// of the same distance buffers 
			bufferOperation = TeBUFFERREGION::TeBufferRegion(lineSetIn, dist, numPoints, bufferLevels, bufferResult);
		}
		else if(sti.hasPoints())
		{
			sti.getGeometry(pointSetIn);
			// generate the buffers of each point of the set (pointSetIn) and do the union
			// of the same distance buffers 
			bufferOperation = TeBUFFERREGION::TeBufferRegion(pointSetIn, dist, numPoints, bufferLevels, bufferResult);
		}
		else if(sti.hasCells())
		{
			sti.getGeometry(cellSetIn);
			// generate the buffers of each cell of the set (cellSetIn) and do the union
			// of the same distance buffers 
			bufferOperation = TeBUFFERREGION::TeBufferRegion(cellSetIn, dist, numPoints, bufferLevels, bufferType, bufferResult);
		}
		else
		{
			if(TeProgress::instance())
				TeProgress::instance()->reset();
			return false;
		}
		
		if(!bufferOperation)
		{
			if (logFile)
			{	
				string mess = "Error in the buffer operation over object_id:  ";
				mess += sti.objectId();
				logFile->writeString(mess);
				logFile->writeNewLine();
			}
			++count;
			continue;
		}
		
		//Stores the buffer to do the union of each level 
		if(doUnion) 
		{
			for(unsigned int level=0; level<bufferResult.size(); ++level)
			{
				for(unsigned int pol=0; pol<bufferResult[level].size(); ++pol)
					result[level].add(bufferResult[level][pol]);
			}
		}
		else
		{
			for(unsigned int level=0; level<bufferResult.size(); ++level)
			{
				string slevel = Te2String(level+1);
				bufferResult[level].objectId(sti.objectId()+"_"+slevel);
				//attribute
				row.clear();
				row.push_back(sti.objectId()+"_"+slevel);
				row.push_back(slevel);
				row.push_back(Te2String(dist*(level+1)));

				if(copyInputColumns == true)
				{
					for(unsigned int i = 0; i < inputAttrList.size(); ++i)
					{
						unsigned int indexToUse = i;
						if(setIgnoreColumns.find(indexToUse) != setIgnoreColumns.end())
						{
							continue;
						}

						if(distBasedOnAttr)
						{
							++indexToUse;
						}

						std::string stiValue = "";
						sti.getPropertyValue(stiValue, indexToUse);

						row.push_back(stiValue);
					}
				}
				attTable.add(row);


				//geometry
				for(unsigned int pol=0; pol<bufferResult[level].size(); ++pol)
					result[level].add(bufferResult[level][pol]);
			}

			if(count%100==0)
			{
				//store buffer attribute and geometries
				//saves attributes
				if (!newLayer->saveAttributeTable(attTable))
				{
					if(logFile)
					{
						string mess = "Error saving attributes in the attribute table! ";
						logFile->writeString(mess);
						logFile->writeNewLine();
					}
					if(TeProgress::instance())
						TeProgress::instance()->reset();
					return false;
				}
				attTable.clear();
				
				//saves geometries
				for(unsigned int level=0; level<bufferResult.size(); ++level)
				{
					if(!newLayer->addPolygons(result[level]))
					{
						if(logFile)
						{
							string mess = "Error saving buffers in the geometric table! ";
							logFile->writeString(mess);
							logFile->writeNewLine();
						}
						if(TeProgress::instance())
						{
							TeProgress::instance()->reset();
						}
						return false;
					}
					result[level].clear();
				}
				
			}
		}
		++count;
	}

	if(doUnion)
	{
		//progress bar
		if(TeProgress::instance())
		{
			string caption = "Dissolve operation";
			TeProgress::instance()->setCaption(caption.c_str());
			string msg = "Executing dissolve operation and saving the \n results in the database. Please, wait!";
			TeProgress::instance()->setMessage(msg);
			TeProgress::instance()->setTotalSteps((int)bufferLevels);
		}
			
		//do the union for each level
		int objId = 0;
		TePolygonSet prevLevelPolSet;
		for(unsigned int level=0; level<result.size(); ++level)
		{
			TePolygonSet curLevelPolSet;
			if(!TeOVERLAY::TePairUnion(result[level], curLevelPolSet))
			{
				if(logFile)
				{
					string mess = "Error in the union of the buffers of the level ";
					mess += Te2String(level+1) +" !";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
				if(TeProgress::instance())
				{
					TeProgress::instance()->reset();
				}
				return false;
			}

			if(level>0)
			{
				TePolygonSet auxPolSet;
				//Difference between current level union and union of previous levels 
				if(!TeOVERLAY::TeDifference (curLevelPolSet, prevLevelPolSet, auxPolSet))
				{
					if(logFile)
					{
						string mess = "Error in the difference between buffers of the level ";
						mess += Te2String(level+1) +" and buffers of the level ";
						mess += Te2String(level) +" !";
						logFile->writeString(mess);
						logFile->writeNewLine();
					}
					if(TeProgress::instance())
					{
						TeProgress::instance()->reset();
					}
					return false;
				}

				curLevelPolSet.clear();
				for(unsigned int j=0; j< auxPolSet.size(); ++j)
                    curLevelPolSet.add(auxPolSet[j]);

				//Do the union between current level and previous levels
				auxPolSet.clear();
				if((level+1)<result.size())
				{
					if(!TeOVERLAY::TeUnion (curLevelPolSet, prevLevelPolSet, auxPolSet))
					{
						if(logFile)
						{
							string mess = "Error in the union between buffers of the level ";
							mess += Te2String(level+1) +" and buffers of the level ";
							mess += Te2String(level) +" !";
							logFile->writeString(mess);
							logFile->writeNewLine();
						}

						if(TeProgress::instance())
						{
							TeProgress::instance()->reset();
						}

						return false;
					}

					prevLevelPolSet.clear();
					for(unsigned int j=0; j< auxPolSet.size(); ++j)
						prevLevelPolSet.add(auxPolSet[j]);
				}
			}
			else
			{
				for(unsigned int j=0; j< curLevelPolSet.size(); ++j)
                    prevLevelPolSet.add(curLevelPolSet[j]);
			}

			for(unsigned int k=0; k<curLevelPolSet.size(); ++k)
			{
				string objectId = Te2String(objId);
				++objId;
				curLevelPolSet[k].objectId(objectId);
				//attribute
				row.clear();
				row.push_back(objectId);
				row.push_back(Te2String(level+1));
				if(!distBasedOnAttr)
					row.push_back(Te2String(dist*(level+1)));
				attTable.add(row);
			}
			
			//saves attributes
			if (!newLayer->saveAttributeTable(attTable))
			{
				if(logFile)
				{
					string mess = "Error saving attributes in the attribute table! ";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
				if(TeProgress::instance())
					TeProgress::instance()->reset();
				return false;
			}
			attTable.clear();
				
			//saves geometries
			if(!newLayer->addPolygons(curLevelPolSet))
			{
				if(logFile)
				{
					string mess = "Error saving buffers in the geometric table! ";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
				if(TeProgress::instance())
					TeProgress::instance()->reset();
				return false;
			}

			//progress bar
			if(TeProgress::instance())
			{
				if(TeProgress::instance()->wasCancelled())
				{
					TeProgress::instance()->reset();
					return false;
				}
				TeProgress::instance()->setProgress(level+1);
			}
		} //for
	}
	else
	{
		//saves attributes
		if (!newLayer->saveAttributeTable(attTable))
		{
			if(logFile)
			{
				string mess = "Error saving attributes in the attribute table! ";
				logFile->writeString(mess);
				logFile->writeNewLine();
			}
			if(TeProgress::instance())
				TeProgress::instance()->reset();
			return false;
		}
		
		//saves geometries
		for(unsigned int level=0; level<result.size(); ++level)
		{
			if(!newLayer->addPolygons(result[level]))
			{
				if(logFile)
				{
					string mess = "Error saving buffers in the geometric table! ";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
				if(TeProgress::instance())
					TeProgress::instance()->reset();
				return false;
			}
			result[level].clear();
		}
	}
	if(TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

bool 
TeGeoOpOverlayIntersection(TeLayer* newLayer, TeTheme* theme, TeTheme* themeTrim, TeSelectedObjects selObTrim, double backValue, TeAsciiFile* logFile)
{
	// check if the input parameters are valid
	if((!newLayer) || (!theme) || (!themeTrim))
		return false;

	// check if the theme to be clipped has a raster geometry valid
	TeRaster* rst = theme->layer()->raster();
	if (!rst) 
		return false;

	//Querier Trim
	TeQuerierParams paramsTrim(true, true);
	paramsTrim.setParams(themeTrim);
	paramsTrim.setSelecetObjs(selObTrim);
	TeQuerier  querierTrim(paramsTrim); 
	if(!querierTrim.loadInstances())
		return  false;
	
	TeSTInstance trim;
	TePolygonSet ps;
	while(querierTrim.fetchInstance(trim))
	{
		TePolygonSet polSet;
		if(!trim.getGeometry(polSet))
			continue;
		for(int i=0; i<(int)polSet.size(); ++i)
		{
			TePolygon polyTrim = polSet[i];
			// before intersection, valid polygon holes (union of holes)
			TePolygonSet polyTrimSet;
			if(!TeOVERLAY::TeValidPolygonHoles(polyTrim, polyTrimSet))
			{
				// when the operation returns false, write in the log a possible
				// inconsistency in the data
				if (logFile)
				{
					string mess = "Nao foi possivel realizar a consistencia dos aneis internos ";
					mess += polyTrim.objectId();
					mess += " do tema " + themeTrim->name() + ".";
					logFile->writeString(mess);
					logFile->writeNewLine();
				}
				continue;
			}
			ps.add(polSet[i]);
		}//for each polygon of the Trim theme
	}

	// there are no valid polygons to be used as mask to clip raster representation
	if (ps.empty())
		return false;

	// finds a valid name for the table that contains the raster geometry
	string rasterTableName = "RasterLayer" + Te2String(newLayer->id())+"_R_O1";
	rasterTableName = newLayer->database()->getNewTableName(rasterTableName);

	// tries to execute the clipping
	TeRaster* rstClip = TeRasterClipping(rst, ps, themeTrim->layer()->projection(), rasterTableName, backValue, "DB");
	newLayer->addRasterGeometry(rstClip,"O1");
	delete rstClip;
    return true;
}

bool TeGeoOpNearestNeighbour(TeTheme* inputTheme, TeSelectedObjects iselObj, 
							 TeTheme* neighTheme, TeSelectedObjects nnselObj, 
							 const string& distColName, const string& NNIDColName)
{
	if (!inputTheme || !neighTheme)
		return false;

	// retrieve the the centroids of all objects candidates to be the nearest neighbour
	TeQuerierParams qPar(true, false);
	qPar.setParams(neighTheme);
	qPar.setSelecetObjs(iselObj);
	TeQuerier* querier = new TeQuerier(qPar);
	if (!querier->loadInstances())
	{
		delete querier;
		return false;
	}

	TeProjection* pInput = inputTheme->layer()->projection();
	TeProjection* pNeigh = neighTheme->layer()->projection();

	bool doReproj = false;
	if (!(*pInput == *pNeigh))
	{
		pNeigh->setDestinationProjection(pInput);
		doReproj = true;
	}
		
	string newDistCol = distColName;
	if (newDistCol.empty())
		newDistCol = "DIST";

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
		t2 = clock();
		t0 = t1 = t2;
	}
	unsigned int step=0;
	std::vector<std::pair<TeCoord2D, TePoint> > dataSet;
	TeSTInstance st;
	TeMultiGeometry multigeo;
	string lastOID;
	while(querier->fetchInstance(st))
	{
		TeCoord2D cc;
		st.centroid(cc);
		if (doReproj)
		{
			TeCoord2D ll = pNeigh->PC2LL(cc);
			cc = pInput->LL2PC(cc);
		}
		TePoint pt(cc);
		pt.objectId(st.objectId());
		dataSet.push_back(std::pair<TeCoord2D, TePoint>(cc, pt)); 
		t2 = clock();
		if (((int(t2-t1) > dt) && TeProgress::instance()))
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(step);
		}
		++step;
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();

	step = 0;
	TeBox candExt = querier->getBox();
	delete querier;

// create a spatial index of the centroids of the candidates
	typedef TeSAM::TeAdaptativeKdTreeNode<TeCoord2D, std::vector<TePoint>, TePoint> KDNODE;
	typedef TeSAM::TeAdaptativeKdTree<KDNODE> KDTREE;

	unsigned int bucketSize = 3 * 1; //
	KDTREE tree(candExt, bucketSize); 
	tree.build(dataSet); 
	dataSet.clear(); 

//	verifies how to save the results
	bool updateNNId = false;
	TeAttribute attr;	
	TeAttrTableVector attTables;
	inputTheme->getAttTables(attTables, TeAttrStatic);

	// always save the distance column
	string tableName = "";
	string dcolName = distColName;
	unsigned int pos = distColName.find('.');
	if (pos != std::string::npos)
	{
		tableName = distColName.substr(0,pos);
		unsigned int nchars = distColName.size()-1;
		dcolName = distColName.substr(pos+1,nchars);
	}

	string objectIdCol;
	if (tableName.empty())
	{
		tableName = attTables[0].name();
		objectIdCol = attTables[0].linkName();
	}
	else
	{
		TeAttrTableVector::iterator it = attTables.begin();
		while (it != attTables.end())
		{
			if ((*it).name() == tableName)
			{
				objectIdCol = (*it).linkName();
				break;
			}
			++it;
		}
	}

	if (inputTheme->layer()->database()->columnExist(tableName,dcolName,attr))
	{
		if (attr.rep_.type_!= TeREAL)
		{
			inputTheme->layer()->database()->deleteColumn(tableName,dcolName);
			attr.rep_.type_ = TeREAL;
			if (!inputTheme->layer()->database()->addColumn(tableName,attr.rep_))
			{
				// can not create the distance column
				delete querier;
				return false;
			}
		}
	}
	else
	{
		attr.rep_.type_ = TeREAL;
		attr.rep_.name_ = dcolName;
		if (!inputTheme->layer()->database()->addColumn(tableName,attr.rep_))
		{
			// can not create the distance column
			delete querier;
			return false;
		}
	}

	// save nn id only when requested
	string nnidcol;
	if (!NNIDColName.empty())
	{
		updateNNId = true;
		nnidcol = NNIDColName;
		pos = NNIDColName.find('.');
		if (pos != std::string::npos)
			nnidcol = NNIDColName.substr(pos+1,NNIDColName.size());

		if (inputTheme->layer()->database()->columnExist(tableName,nnidcol,attr))
		{
			if (attr.rep_.type_!= TeSTRING)
			{
				inputTheme->layer()->database()->deleteColumn(tableName,nnidcol);
				attr.rep_.type_ = TeSTRING;
				attr.rep_.numChar_ = 100;
				if (!inputTheme->layer()->database()->addColumn(tableName,attr.rep_))
					updateNNId = false;
			}
		}
		else
		{
			attr.rep_.type_ = TeSTRING;
			attr.rep_.name_ = nnidcol;
			attr.rep_.numChar_ = 100;
			if (!inputTheme->layer()->database()->addColumn(tableName,attr.rep_))
				updateNNId = false;
		}
	}

// retrieve the objects to find the nearest neighbour 
	qPar.setParams(inputTheme);
	qPar.setSelecetObjs(nnselObj);
	querier = new TeQuerier(qPar);
	if (!querier->loadInstances())
	{
		delete querier;
		return false;
	}

	std::vector<TePoint> report;
	report.push_back(TePoint(TeMAXFLOAT, TeMAXFLOAT));
	std::vector<double> sqrDists;
	multigeo.clear();
	lastOID = "";
	string sql;

	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(querier->numElemInstances());
		t2 = clock();
		t0 = t1 = t2;
	}

	while(querier->fetchInstance(st))
	{
		TeCoord2D cc;
		st.centroid(cc);
		tree.nearestNeighborSearch(cc, report, sqrDists, 1);
		if (updateNNId)
			sql = "UPDATE " + tableName + " SET " + nnidcol + "='" + report[0].objectId() + "', " + dcolName + "=" + Te2String(sqrt(sqrDists[0]),4);
		else
			sql = "UPDATE " + tableName + " SET " + dcolName + "=" + Te2String(sqrt(sqrDists[0]),4);
		sql += " WHERE " + objectIdCol + "='" + st.objectId().c_str() + "'";
		if (!inputTheme->layer()->database()->execute(sql))
		{
			delete querier;
			return false;
		}
		if (((t2-t1) > dt) && (TeProgress::instance()))
		{
			t1 = t2;
			if(TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			if ((t2-t0) > dt2)
				TeProgress::instance()->setProgress(step);
		}
		++step;
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();
	delete querier;
	return true;
}

