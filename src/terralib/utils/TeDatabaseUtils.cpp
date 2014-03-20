/************************************************************************************
TerraView - visualization and exploration of geographical databases using TerraLib.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.
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

#include "TeDatabaseUtils.h"
#include "../functions/TeLayerFunctions.h"
#include "../kernel/TeDBConnectionsPool.h"
#include "../kernel/TeFileTheme.h"

#include "../kernel/TeRasterTransform.h"

bool isTerralibModel(TeDatabase* db)
{
	if(db == NULL)
	{
		return false;
	}
	if(!db->tableExist("te_projection"))
		return false;
	if(!db->tableExist("te_layer"))
		return false;
	if(!db->tableExist("te_layer_table"))
		return false;
	if(!db->tableExist("te_tables_relation"))
		return false;
	if(!db->tableExist("te_representation"))
		return false;
	if(!db->tableExist("te_view"))
		return false;
	if(!db->tableExist("te_theme"))
		return false;
	if(!db->tableExist("te_grouping"))
		return false;
	if(!db->tableExist("te_theme_table"))
		return false;
	if(!db->tableExist("te_legend"))
		return false;
	if(!db->tableExist("te_visual"))
		return false;
	if(!db->tableExist("te_visual_raster"))
		return false;
	if(!db->tableExist("te_database"))
		return false;

	return true;
}


bool TeCopyDatabase(TeDatabase* dbFrom, TeDatabase* dbTo)
{
	// -- Step 1 : copies the external tables
	TeAttrTableVector externs;
	dbFrom->getAttrTables(externs,TeAttrExternal);
	if (!externs.empty())
	{
		TeDatabasePortal* portal = dbFrom->getPortal();
		if (!portal)
		{
			// log error here
			return false;
		}

		for (unsigned int nt=0; nt < externs.size(); nt++)
		{
			TeTable table = externs[nt];
			table.setId(-1);
			string sql = "select * from " + table.name();
			if (!portal->query(sql) || portal->numRows() == 0)
			{
				portal->freeResult();
				continue;
			}
			if (!dbTo->createTable(table.name(),table.attributeList()))
			{
				portal->freeResult();
				continue;
			}
			unsigned int nr=0;
			while (portal->fetchRow())
			{
				TeTableRow row;
				for(int i = 0; i < portal->numFields(); i++)
					row.push_back(portal->getData(i));
				table.add(row);
				nr++;
				if (nr % 200)
				{
					dbTo->insertTable(table);
					table.clear();
					nr = 0;
				}
			}
			if (table.size() >0)
				dbTo->insertTable(table);
				table.clear();	
			dbTo->insertTableInfo(-1,table);
			portal->freeResult();
		}
		delete portal;
	}

	// -- Step 2: Copies the layers
	if(!dbFrom->loadLayerSet())
	{
		// log error here
		return false;
	}

	TeLayerMap& fromLayerMap = dbFrom->layerMap();
	TeLayerMap::iterator itFrom = fromLayerMap.begin();

	while(itFrom != fromLayerMap.end())
	{
		TeLayer* fromLayer = (*itFrom).second;
		TeLayer* toLayer = new TeLayer(fromLayer->name(), dbTo, fromLayer->projection());
		map<string, string> tables;
		if (!TeCopyLayerToLayer(fromLayer, toLayer, &tables))
		{
			// log error here: layer couldn�t be copied
		}
		++itFrom;
	}
	// -- Set 3: Copies the views and themes
	if(!dbFrom->loadViewSet(dbFrom->user()))
	{
		// log error here
		return false;
	}

	if(!dbTo->loadLayerSet())
	{
		// log error here
		return false;
	}

	TeViewMap&	fromViewMap = dbFrom->viewMap();
	TeViewMap::iterator itvFrom = fromViewMap.begin();
	while (itvFrom != fromViewMap.end())
	{
		TeView* fromView = (*itvFrom).second;			
		TeView* toView = new TeView();

		TeProjection* toViewProjection = 0;
		if (fromView->projection())
			 toViewProjection = TeProjectionFactory::make(fromView->projection()->params());
		
		toView->projection(toViewProjection);
		toView->name(fromView->name());
		toView->user(dbTo->user());
		toView->isVisible(fromView->isVisible());

		TeBox b;
		toView->setCurrentBox(b);
		toView->setCurrentTheme(-1);

		if (!dbTo->insertView(toView))
		{
			// log error here
			++itvFrom;
			continue;
		}
		dbTo->insertView(toView);
		if(dbTo->projectMap().empty() == false)
		{
			TeProjectMap& pm = dbTo->projectMap();
			TeProject* project = pm.begin()->second;
			project->addView(toView->id());
		}
		dbTo->insertProjectViewRel(1, toView->id());

		TeLayerMap& toLayerMap = dbTo->layerMap();			
		vector<TeViewNode*>& themeVec = fromView->themes();
		for (unsigned int i = 0; i < themeVec.size(); ++i)
		{
			TeTheme* themeFrom = (TeTheme*) themeVec[i];
			string fromLayerName = themeFrom->layer()->name();
			TeLayer* toLayer = 0;
			TeLayerMap::iterator itTo = toLayerMap.begin();
			while(itTo != toLayerMap.end())
			{
				if(itTo->second->name() == fromLayerName)
				{
					toLayer = itTo->second;
					break;
				}
				++itTo;
			}

			if (!toLayer )
			{
				// log error here
				continue;
			}
			
			TeTheme* themeTo = new TeTheme(themeFrom->name(), toLayer);
			toView->add(themeTo);
						
			themeTo->outOfCollectionLegend(themeFrom->outOfCollectionLegend());
			themeTo->withoutDataConnectionLegend(themeFrom->withoutDataConnectionLegend ());
			themeTo->defaultLegend(themeFrom->defaultLegend());
			themeTo->pointingLegend(themeFrom->pointingLegend());
			themeTo->queryLegend(themeFrom->queryLegend());

			TeAttrTableVector tftablevec = themeFrom->attrTables();
			TeAttrTableVector tttablevec;

			for (unsigned int nt=0; nt<tftablevec.size(); nt++)
			{
				TeTable attTable(tftablevec[nt].name());
				dbTo->loadTableInfo(attTable); 
				tttablevec.push_back(attTable);
			}
			themeTo->setAttTables(tttablevec);
			themeTo->attributeRest(themeFrom->attributeRest());
			themeTo->temporalRest(themeFrom->temporalRest());
			themeTo->spatialRest(themeFrom->spatialRest());
			themeTo->visibleRep(themeFrom->visibleRep());
			if(!themeTo->save() || !themeTo->buildCollection())
			{
				// log error here
				continue;
			}
			themeTo->generateLabelPositions();
			
			if(themeFrom->grouping().groupMode_ != TeNoGrouping)
			{
				TeGrouping grouping;
				grouping = themeFrom->grouping();				
				themeTo->buildGrouping(grouping);
				TeLegendEntryVector& legends = themeFrom->legend();
				for (unsigned int nl=0; nl<legends.size(); nl++)
					themeTo->setGroupingVisual(nl+1,legends[nl].getVisualMap());
				if (!themeTo->saveGrouping())
				{
						// log error here
				}
			}
		} // end for each theme
		++itvFrom;
	} // end for each view
	return true;
}

//vector<string> getObjects(TeTheme* theme, int sel)
//{
//	vector<string> svec;
//	if(!theme)
//		return svec;
//
//	TeDatabase* db = 0;
//	if(theme->getProductId() == TeEXTERNALTHEME)
//		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
//	else if(theme->getProductId() == TeTHEME)
//		db = static_cast<TeTheme*>(theme)->layer()->database();
//
//	if(db == 0)
//		return svec;
//
//	string C = theme->collectionTable();
//	string CA = theme->collectionAuxTable();
//	string input;
//
//	if(sel == TeSelectedByPointing)
//		input += " WHERE " + C + ".c_object_status = 1 OR " + C + ".c_object_status = 3";
//	else if(sel == TeNotSelectedByPointing)
//		input += " WHERE " + C + ".c_object_status = 0 OR " + C + ".c_object_status = 2";
//	else if(sel == TeSelectedByQuery)
//		input += " WHERE " + C + ".c_object_status = 2 OR " + C + ".c_object_status = 3";
//	else if(sel == TeNotSelectedByQuery)
//		input += " WHERE " + C + ".c_object_status = 0 OR " + C + ".c_object_status = 1";
//	else if(sel == TeSelectedByPointingAndQuery)
//		input += " WHERE " + C + ".c_object_status = 3";
//	else if(sel == TeSelectedByPointingOrQuery)
//		input += " WHERE " + C + ".c_object_status <> 0";
//	else if(sel == TeGrouped)
//		input += " WHERE " + C + ".c_legend_id <> 0";
//	else if(sel == TeNotGrouped)
//		input += " WHERE " + C + ".c_legend_id = 0";
//
//	string query = "SELECT " + C + ".c_object_id FROM " + C + " LEFT JOIN " + CA;
//	query += " ON " + C + ".c_object_id = " + CA + ".object_id" + input;
//
//	TeDatabasePortal* portal = db->getPortal();
//	if (portal->query(query) == false)
//	{
//		delete portal;
//		return svec;
//	}
//	while (portal->fetchRow())
//		svec.push_back(portal->getData(0));
//
//	delete portal;
//	return svec;
//}
//
//vector<string> getItems(TeTheme* theme, int sel)
//{
//	vector<string> svec;
//	if(!theme)
//		return svec;
//
//	TeDatabase* db = 0;
//	if(theme->getProductId() == TeEXTERNALTHEME)
//		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
//	else if(theme->getProductId() == TeTHEME)
//		db = static_cast<TeTheme*>(theme)->layer()->database();
//
//	if(db == 0)
//		return svec;
//
//	string C = theme->collectionTable();
//	string CA = theme->collectionAuxTable();
//	string input;
//
//	if(sel == TeSelectedByPointing)
//		input += " WHERE " + CA + ".grid_status = 1 OR " + CA + ".grid_status = 3";
//	else if(sel == TeNotSelectedByPointing)
//		input += " WHERE " + CA + ".grid_status = 0 OR " + CA + ".grid_status = 2";
//	else if(sel == TeSelectedByQuery)
//		input += " WHERE " + CA + ".grid_status = 2 OR " + CA + ".grid_status = 3";
//	else if(sel == TeNotSelectedByQuery)
//		input += " WHERE " + CA + ".grid_status = 0 OR " + CA + ".grid_status = 1";
//	else if(sel == TeSelectedByPointingAndQuery)
//		input += " WHERE " + CA + ".grid_status = 3";
//	else if(sel == TeSelectedByPointingOrQuery)
//		input += " WHERE " + CA + ".grid_status <> 0";
//	else if(sel == TeGrouped)
//		input += " WHERE " + C + ".c_legend_id <> 0";
//	else if(sel == TeNotGrouped)
//		input += " WHERE " + C + ".c_legend_id = 0";
//
//	string query = "SELECT " + CA + ".unique_id FROM " + C + " LEFT JOIN " + CA;
//	query += " ON " + C + ".c_object_id = " + CA + ".object_id" + input;
//
//	TeDatabasePortal* portal = db->getPortal();
//	if (portal->query(query) == false)
//	{
//		delete portal;
//		return svec;
//	}
//	while (portal->fetchRow())
//		svec.push_back(portal->getData(0));
//
//	delete portal;
//	return svec;
//}
//
//vector<string> getObjects(TeTheme* theme, vector<string>& itens)
//{
//	vector<string> svec;
//	if(!theme)
//		return svec;
//
//	TeDatabase* db = 0;
//	if(theme->getProductId() == TeEXTERNALTHEME)
//		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
//	else if(theme->getProductId() == TeTHEME)
//		db = static_cast<TeTheme*>(theme)->layer()->database();
//
//	if(db == 0)
//		return svec;
//
//	TeDatabasePortal* portal = db->getPortal();
//	string C = theme->collectionTable();
//	string CA = theme->collectionAuxTable();
//
//	string query = "SELECT " + C + ".c_object_id FROM " + C + " LEFT JOIN " + CA;
//	query += " ON " + C + ".c_object_id = " + CA + ".object_id";
//	query += " WHERE " + CA + ".unique_id IN ";
//
//	set<string> idSet;
//  
//  vector< string >::iterator it_begin = itens.begin();
//  vector< string >::iterator it_end = itens.end();
//  
//	vector<string> inVec = generateInClauses(it_begin, it_end, db, false);
//	vector<string>::iterator it;
//	for(it=inVec.begin(); it!=inVec.end(); ++it)
//	{
//		if((*it).empty() == false)
//		{
//			string sel = query + *it;
//			if (portal->query(sel) == false)
//			{
//				delete portal;
//				return svec;
//			}
//			while (portal->fetchRow())
//				idSet.insert(portal->getData(0));
//			portal->freeResult();
//		}
//	}
//	delete portal;
//
//	set<string>::iterator sit;
//	for(sit=idSet.begin(); sit!=idSet.end(); ++sit)
//		svec.push_back(*sit);
//	return svec;
//}
//
//vector<string> getItems(TeTheme* theme, vector<string>& objects)
//{
//	vector<string> svec;
//	if(!theme)
//		return svec;
//
//	TeDatabase* db = 0;
//	if(theme->getProductId() == TeEXTERNALTHEME)
//		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
//	else if(theme->getProductId() == TeTHEME)
//		db = static_cast<TeTheme*>(theme)->layer()->database();
//
//	if(db == 0)
//		return svec;
//
//	TeDatabasePortal* portal = db->getPortal();
//	string C = theme->collectionTable();
//	string CA = theme->collectionAuxTable();
//
//	string query = "SELECT " + CA + ".unique_id FROM " + C + " LEFT JOIN " + CA;
//	query += " ON " + C + ".c_object_id = " + CA + ".object_id";
//	query += " WHERE " + C + ".c_object_id IN ";
//  
//  vector< string >::iterator it_begin = objects.begin();
//  vector< string >::iterator it_end = objects.end();  
//
//	vector<string> inVec = generateInClauses(it_begin, it_end, db);
//	vector<string>::iterator it;
//	for(it=inVec.begin(); it!=inVec.end(); ++it)
//	{
//		if((*it).empty() == false)
//		{
//			string sel = query + *it;
//			if (portal->query(sel) == false)
//			{
//				delete portal;
//				return svec;
//			}
//			while (portal->fetchRow())
//				svec.push_back(portal->getData(0));
//			portal->freeResult();
//		}
//	}
//	delete portal;
//	return svec;
//}
//
map<string, vector<string> > getObject2ItemsMap(TeTheme* theme, vector<string>& itens)
{
	map<string, vector<string> > outMap;
	if(!theme)
		return outMap;

	TeDatabase* db = 0;
	if(theme->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme)->getSourceDatabase();
	else if(theme->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme)->layer()->database();

	if(db == 0)
		return outMap;

	TeDatabasePortal* portal = db->getPortal();
	string C = theme->collectionTable();
	string CA = theme->collectionAuxTable();

	string query = "SELECT " + C + ".c_object_id, " + CA + ".unique_id FROM " + C + " LEFT JOIN " + CA;
	query += " ON " + C + ".c_object_id = " + CA + ".object_id";
	query += " WHERE " + CA + ".unique_id IN ";
  
  vector< string >::iterator it_begin = itens.begin();
  vector< string >::iterator it_end = itens.end();   

	vector<string> inVec = generateInClauses(it_begin, it_end, db, false);
	vector<string>::iterator it;
	for(it=inVec.begin(); it!=inVec.end(); ++it)
	{
		if((*it).empty() == false)
		{
			string sel = query + *it;
			if (portal->query(sel) == false)
			{
				delete portal;
				return outMap;
			}
			while (portal->fetchRow())
				outMap[portal->getData(0)].push_back(portal->getData(1));
			portal->freeResult();
		}
	}
	delete portal;

	return outMap;
}

vector<string>
generateItemsInClauseVec(TeTheme* theme, string& where)
{
	vector<string> inClauseVector;
	TeDatabase* db = 0;
	string	CT = theme->collectionTable();
	string	CA = theme->collectionAuxTable();
	string	from = " FROM " + CT + " LEFT JOIN " + CA + " ON " + CT + ".c_object_id = " + CA + ".object_id";

	if(theme->getProductId()==TeTHEME)
		db = theme->layer()->database();
	else if(theme->getProductId()==TeEXTERNALTHEME)
		db = ((TeExternalTheme*) theme)->getSourceDatabase();

	if(db == 0)
		return inClauseVector;

	TeDatabasePortal* portal = db->getPortal();
	string sel = "SELECT " + CT + ".c_object_id, " + CA + ".unique_id " + from;

	if(where.empty() == false)
		sel += " " + where;

	map<string, vector<string> > objMap;
	if(portal->query(sel))
	{
		while(portal->fetchRow())
			objMap[portal->getData(0)].push_back(portal->getData(1));
	}

	map<string, vector<string> >::iterator mit;
	vector<string>::iterator it;

	int i, chunkSize = 200;
	string inClause;
	
	i = 0;
	bool chunk = true;
	for(mit=objMap.begin(); mit!=objMap.end(); ++mit)
	{
		if (chunk == true)
		{
			chunk = false;
			if (!inClause.empty())
			{
				inClause[inClause.size() - 1] = ')';
				inClauseVector.push_back(inClause);
				inClause.clear();
			}
			inClause = "(";
		}
		for(it=mit->second.begin(); it!=mit->second.end(); ++it)
		{
			inClause += *it + ",";
			i++;
			if (i%chunkSize == 0)
				chunk = true;
		}
	}
	if (!inClause.empty())
	{
		inClause[inClause.size() - 1] = ')';
		inClauseVector.push_back(inClause);
	}
	return inClauseVector;
}

TeViewNode* findNode(const std::set<TeViewNode*>& orphanNodes, const int nodeId)
{
	TeViewNode* node = NULL;
	std::set<TeViewNode*>::const_iterator it;

	for(it = orphanNodes.begin(); it != orphanNodes.end(); ++it)
	{
		node = ((*it)->id() == nodeId) ? *it : 
				(*it)->type() == TeTREE ? 
				   ((TeViewTree*)(*it))->find(nodeId) :
				    NULL;

		if(node != NULL)
		{
			break;
		}
	}

	return node;
}

void updateNodesTrees(std::set<TeViewNode*>& orphanNodes, TeView* view)
{
	std::set<TeViewNode*>::iterator it;

	while(!orphanNodes.empty())
	{
		it = orphanNodes.begin();
		TeViewNode* node = *it;

		TeViewNode* group = findNode(orphanNodes, node->parentId());

		if(group == NULL)
		{
			group = view->root()->find(node->parentId());
		}

		if(group != NULL)
		{
			int pri = node->priority();
			group->add(node, false);
			node->priority(pri);
		}

		orphanNodes.erase(node);
	}
}

bool loadViewSetAndThemeGroups(TeDatabase* db, const std::string& userName)
{
	std::vector<TeAbstractTheme*> vecExternalThemes;
	std::set<TeViewNode*> orphanNodes;

	//clear view map
	TeViewMap::iterator it = db->viewMap().begin();
	while (it != db->viewMap().end())
	{
		if(it->second)
			delete it->second;
		++it;
	}
	db->viewMap().clear();

	//clear theme map
	TeThemeMap::iterator itTheme = db->themeMap().begin();
	while (itTheme != db->themeMap().end())
	{
		if(itTheme->second)
			delete itTheme->second;
		++itTheme;
	}

	//clear invalid theme map
	itTheme = db->invalidThemeMap().begin();
	while (itTheme != db->invalidThemeMap().end())
	{
		if(itTheme->second)
			delete itTheme->second;
		++itTheme;
	}
	db->invalidThemeMap().clear();
	db->themeMap().clear();
	db->legendMap().clear();
	
	//load view, projection, themes and grouping
	string sql = " SELECT ";
	sql += " te_view.*, "; //0-9  (10 columns)
	sql += " te_projection.*, "; //10 - 21 (12 columns)
	sql += " te_datum.radius, te_datum.flattening, te_datum.dx, te_datum.dy, te_datum.dz, "; //22 - 26 (5 columns)
	sql += " te_theme.*, "; // 27 - 46 (20 columns)
	sql += " te_grouping.*, "; //47 - 56 (10 columns)
	sql += " te_legend.*,  "; // 57 - 63    (7 columns)
	sql += " te_visual.*, "; //  64 - 87 (24 columns)
	sql += " te_visual_raster.* "; //  88 (7 columns)

	sql += " FROM (((((((te_view INNER JOIN te_projection ";
	sql += " ON te_view.projection_id = te_projection.projection_id) ";
	sql += " LEFT JOIN te_theme ON te_view.view_id = te_theme.view_id ) ";
	sql += " LEFT JOIN te_grouping ON te_theme.theme_id = te_grouping.theme_id) ";
	sql += " LEFT JOIN te_legend ON te_theme.theme_id = te_legend.theme_id) ";
	sql += " LEFT JOIN te_visual ON te_visual.legend_id = te_legend.legend_id) ";
	sql += " LEFT JOIN te_visual_raster ON te_theme.theme_id = te_visual_raster.theme_id) ";
	sql += " LEFT JOIN te_datum ON te_projection.datum = te_datum.name) ";
			
	sql += " WHERE ";
	if (!userName.empty())
		sql += " te_view.user_name = '" + userName + "'";
	else
		sql += " te_view.user_name = '" + db->user() + "'";
	sql += " ORDER BY te_view.view_id, te_theme.priority, te_theme.theme_id, te_legend.legend_id, ";
	sql += " te_legend.group_id, te_visual.geom_type, te_visual_raster.band_in  "; 

	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return false;
	
	if (!portal->query(sql))
	{	
		delete portal;
		return false;
	}
	
	int lastViewId = -1;
	TeView *view = 0;
	bool hasNewRow = portal->fetchRow(); //idicates that this portal was fetched 
	while(hasNewRow)
	{
		//load view and its projection 
		if(lastViewId!=atoi(portal->getData(0)))
		{
			//store the last view
			if(view)
			{
				updateNodesTrees(orphanNodes, view);
				db->viewMap()[view->id()] = view;
				view = 0;
			}
			TeProjection* proj = 0;
			if(!portal->getProjection(&proj, 10)) //load projection
			{
				delete portal;
				return false;
			}
			view = new TeView();
			if(!portal->getView(*view, 0)) //load view
			{
				delete portal;
				delete view;
				return false;
			}
			if (proj != 0)
				view->projection(proj);
			lastViewId = view->id();
		}  
		
		//make the rigth object
		string aux = portal->getData(27);
		if (aux.empty())
		{
			hasNewRow = portal->fetchRow();
			continue;
		}

		TeViewNodeType viewNodeType = (TeViewNodeType)portal->getInt(33);

		if(viewNodeType == TeTREE)
		{
			TeViewTree* vTree = new TeViewTree;
			vTree->id(portal->getInt(27));
			vTree->view(portal->getInt(29));
			vTree->name(portal->getData(30));
			int parentId = portal->getInt(31);
			vTree->priority(portal->getInt(32));
			
			if(vTree->id() == parentId)
			{
				view->add(vTree, false);
			}
			else
			{
				orphanNodes.insert(vTree);
			}

			vTree->parentId(parentId);
			
			hasNewRow = portal->fetchRow();
			continue;
		}
		else 
		{
			TeViewNode* viewNode = TeViewNodeFactory::make(viewNodeType);

			if(!viewNode)
			{
				int currentThemeId = portal->getInt(27);

				while((hasNewRow = portal->fetchRow()) && (portal->getInt(27) == currentThemeId))
					;

				continue;
			}

			if(!portal->getTheme(static_cast<TeAbstractTheme&>(*viewNode), 27))
			{
				delete viewNode;
				delete portal;
				return false;
			}

			if(viewNodeType == TeTHEME)
			{
				//load layer
				int id = static_cast<TeTheme*>(viewNode)->layerId();
				TeLayerMap::iterator it = db->layerMap().find(id);
				if (it == db->layerMap().end())
					db->loadLayerSet();
				
				static_cast<TeTheme*>(viewNode)->layer(db->layerMap()[id]);
			}
			
			TeAbstractTheme* theme = static_cast<TeAbstractTheme*>(viewNode);

			//load grouping 
			TeGrouping group;
			if(portal->getGrouping(group, 47))
				theme->grouping(group);

			//load all legends of this theme
			//and its visual
			bool hasLegsToThisTheme = true;
			while(hasLegsToThisTheme)
			{
				//legend
				TeLegendEntry legend;
				if(!portal->getLegend(legend, 57))
				{
					delete theme;
					delete view;
					delete portal;
					return false;
				}

				//visual
				TeRasterVisual* rasterVisual = theme->rasterVisual();
				if(rasterVisual == NULL)
				{
					rasterVisual = new TeRasterVisual();
				}
				bool hasVisualToThisLeg = true;
				bool hasRasterVisual = false;
				while(hasVisualToThisLeg)
				{
					TeVisual* visual = TeVisualFactory::make("tevisual");
					TeGeomRep geomRep;
					if(portal->getVisual(visual, geomRep, 64))
						legend.setVisual(visual, geomRep);
										
					if(rasterVisual != NULL && portal->getRasterVisual(*rasterVisual, 88))
						hasRasterVisual=true;
					
					hasNewRow = portal->fetchRow();
					if(!hasNewRow || portal->getInt(59)!= legend.group() || portal->getInt(57)!= legend.id())
						hasVisualToThisLeg = false;
				}

				//Set raster visual to this theme
				if(hasRasterVisual)
					theme->rasterVisual(rasterVisual);
				else
					delete rasterVisual;

				//Set legend to this theme
				theme->legend(legend); 
																
				//fill legend buffer
				if(legend.group() == -6)	
					db->legendMap()[legend.id()] = &theme->queryAndPointingLegend();
				else if(legend.group() == -5)	
					db->legendMap()[legend.id()] = &theme->queryLegend(); 
				else if (legend.group() == -4)
					db->legendMap()[legend.id()] = &theme->pointingLegend(); 
				else if (legend.group() == -3)
					db->legendMap()[legend.id()] = &theme->defaultLegend(); 
				else if (legend.group() == -2)
					db->legendMap()[legend.id()] = &theme->withoutDataConnectionLegend(); 
				else if (legend.group() == -1)
					db->legendMap()[legend.id()] = &theme->outOfCollectionLegend(); 
				else if (legend.group() == -10) //own legend
				{
					TeLegendEntry* legendTemp = new TeLegendEntry(legend);
					db->legendMap()[legend.id()] = legendTemp;
				}				
				
				if(!hasNewRow || portal->getInt(27)!= theme->id())
					hasLegsToThisTheme = false;
			}
				
			for (unsigned int i = 0; i < theme->legend().size(); ++i)
				db->legendMap()[theme->legend()[i].id()] = &theme->legend()[i];

			if(viewNode->type()==(int)TeTHEME)
			{
				//load theme table
				if(!db->loadThemeTable(static_cast<TeTheme*>(theme)))
				{
					delete portal;
					return false;
				}
			}
			
			if(viewNode->type() != (int)TeEXTERNALTHEME)
			{
				//load specific theme parameters
				if(!theme->loadMetadata(db))
				{
					db->invalidThemeMap()[viewNode->id()] = theme;
					continue;
				}
				db->themeMap()[viewNode->id()] = theme;
			}
			else
			{
				vecExternalThemes.push_back(theme);
			}
			
			int pri = theme->priority();

			//verifica se o tema pertence a algum agrupamento.
			if(theme->parentId() != 0 && theme->id() != theme->parentId())
			{
				orphanNodes.insert(theme);
			}
			else
			{
				view->add(theme, false);
			}

			theme->priority(pri);
		}
	}

	//store the last view
	if(view)
	{
		updateNodesTrees(orphanNodes, view);
		db->viewMap()[view->id()] = view;
		view = 0;
	}

	if(!vecExternalThemes.empty())
	{
		if(!db->loadExternalThemes(vecExternalThemes))
		{
			//todos se tornam invalidos
			for(unsigned int i = 0; i < vecExternalThemes.size(); ++i)
			{
				TeView* view = db->viewMap()[vecExternalThemes[i]->view()];

				db->invalidThemeMap()[vecExternalThemes[i]->id()] = vecExternalThemes[i];
				view->remove(vecExternalThemes[i]->id());
			}
			return true;
		}

		TeViewMap::iterator itView = db->viewMap().begin();
		while(itView != db->viewMap().end())
		{
			unsigned int i = 0;
			while(i < itView->second->themes().size())
			{
				unsigned int id = itView->second->themes()[i]->id();
				if(db->invalidThemeMap().find(id) != db->invalidThemeMap().end())
					itView->second->remove(id);
				else
					++i;
			}
			++itView;
		}
	}

	delete portal;
	return true;
}

bool TeCopyView(TeView* view, TeDatabase* inputDatabase, const std::string& newViewName, const std::string& newViewUser, const std::set<int>& setThemeIds, TeDatabase* outputDatabase, std::vector<bool>& vecResult)
{
	TeDatabase* dbIn = inputDatabase;
	TeDatabase* dbOut = outputDatabase;
	if(dbOut == NULL)
	{
		dbOut = dbIn;
	}

	/*if(!dbOut->beginTransaction())
	{
		return false;
	}*/

	TeProjection* proj = view->projection();
	TeProjection* newProjection = TeProjectionFactory::make(proj->params());

	TeView* newView = new TeView(newViewName, newViewUser);
	newView->setCurrentBox(view->getCurrentBox());
	newView->projection(newProjection);

	/*if(!dbOut->insertView(newView))
	{
		delete newView;
		dbOut->rollbackTransaction();
		return false;
	}*/	

	if(!dbOut->insertView(newView))
	{
		delete newView;
		return false;
	}

	for(unsigned int th = 0; th < view->size(); ++th)
	{
		TeViewNode* node = view->get(th);

		if(!setThemeIds.empty())
		{
			if(setThemeIds.find(node->id()) == setThemeIds.end())
			{
				continue;
			}
		}

		TeViewNode* viewNode = TeCopyViewNode(node, dbIn, dbOut, newView);
		bool result = false;
		if(viewNode != NULL)
		{
			result = true;
		}
		vecResult.push_back(result);
	}

	/*TeViewMap& viewMap = outputDatabase->viewMap();
	TeViewMap::iterator viewIt = viewMap.find(newView->id());
	if(viewIt != viewMap.end())
	{
		if(viewIt->second)
		{
			delete viewIt->second;
		}
	}
	viewMap.erase(viewIt);
	delete newView;*/

	/*if(!dbOut->commitTransaction())
	{
		dbOut->rollbackTransaction();
		return false;
	}*/

	return true;
}

TeViewNode* TeCopyViewNode(TeViewNode* node, TeDatabase* inputDatabase, TeDatabase* outputDatabase, TeView* newView)
{
	if(node == NULL)
	{
		return NULL;
	}

	if(!outputDatabase->beginTransaction())
	{
		return NULL;
	}

	TeViewNode* newViewNode = NULL;
	if (node->type() == TeTREE)
	{
		TeViewTree* tree = dynamic_cast<TeViewTree*>(node);
		if(tree == NULL)
		{
			outputDatabase->rollbackTransaction();
			return NULL;
		}

		TeViewTree* newTree = new TeViewTree(tree->viewNodeParams());
		newTree->id(0);

		if(!outputDatabase->insertViewTree(newTree))
		{
			outputDatabase->rollbackTransaction();
			delete newTree;
			return NULL;
		}

		std::vector<TeViewNode*>& vecNodes = tree->nodes();
		for(unsigned int i = 0; i < vecNodes.size(); ++i)
		{
			int parentId = vecNodes[i]->parentId();
			vecNodes[i]->parentId(newTree->id());

			TeViewNode* newChild = TeCopyViewNode(vecNodes[i], inputDatabase, outputDatabase, newView);
			
			vecNodes[i]->parentId(parentId);

			if(newChild == NULL)
			{
				outputDatabase->rollbackTransaction();
				delete newTree;
				return NULL;
			}
			newTree->add(newChild);
		}

		newViewNode = newTree;
	}
	else
	{
		TeAbstractTheme* absTheme = dynamic_cast<TeAbstractTheme*>(node);
//realiza a copia do tema
		TeAbstractTheme* newAbsTheme = absTheme->copyTo(outputDatabase, newView);
		if(newAbsTheme == 0)
		{
			outputDatabase->rollbackTransaction();
			return NULL;
		}

		newViewNode = newAbsTheme;

		if(newAbsTheme->type() == TeTHEME)
		{
			TeTheme* theme = dynamic_cast<TeTheme*>(node);
			TeTheme* newTheme = dynamic_cast<TeTheme*>(newAbsTheme);
			if(newTheme != NULL)
			{
				TeAttrTableVector tableVectorInExternal;
				inputDatabase->getAttrTables(tableVectorInExternal, TeAttrExternal);

				for(unsigned int i = 0; i < tableVectorInExternal.size(); ++i)
				{
					if(!outputDatabase->tableExist(tableVectorInExternal[i].name()))
					{
						if(!outputDatabase->createTable(tableVectorInExternal[i].name(), tableVectorInExternal[i].attributeList()))
						{
							outputDatabase->rollbackTransaction();
							delete newViewNode;
							return NULL;
						}
						TeTable& table = tableVectorInExternal[i];
						table.setId(-1);
						if(!outputDatabase->insertTableInfo(-1, table))
						{
							outputDatabase->rollbackTransaction();
							delete newViewNode;
							return NULL;
						}

						if(!TeCopyTable(inputDatabase, tableVectorInExternal[i].name(), outputDatabase, tableVectorInExternal[i].name()))
						{
							outputDatabase->rollbackTransaction();
							delete newViewNode;
							return NULL;
						}
					}
				}

				TeAttrTableVector& tableVectorIn = theme->attrTables();
				TeAttrTableVector tableVectorOut;

				for(unsigned int i = 0; i < tableVectorIn.size(); ++i)
				{
					TeTable attTable(tableVectorIn[i].name());
					outputDatabase->loadTableInfo(attTable); 
					if(tableVectorIn[i].tableType() == TeAttrExternal)
					{
						// Gets the related table id from output database
						std::string relatedTableName = tableVectorIn[i].relatedTableName();
						TeTable relatedTable(relatedTableName);
						outputDatabase->loadTableInfo(relatedTable);							
						// Updates the external table
						attTable.setTableType(TeAttrExternal, relatedTable.id(), tableVectorIn[i].relatedAttribute());
						attTable.relatedTableName(relatedTableName);
						attTable.setLinkName(tableVectorIn[i].linkName());
					}
					tableVectorOut.push_back(attTable);
				}
				newTheme->setAttTables(tableVectorOut);
			}				
		}		
	}

	if(newViewNode != NULL)
	{
		if(!outputDatabase->commitTransaction())
		{
			outputDatabase->rollbackTransaction();
		}
	}
	else
	{
		outputDatabase->rollbackTransaction();		
	}

	return newViewNode;
}

bool TeCopyTable(TeDatabase* inputDatabase, const std::string& inputTable, TeDatabase* outputDatabase, const std::string& outputTable)
{
	if(!inputDatabase->tableExist(inputTable) || !outputDatabase->tableExist(outputTable))
	{
		return false;
	}

	TeDatabasePortal* portal = inputDatabase->getPortal();
	std::string sql = "SELECT * FROM " + inputTable;
	if(!portal->query(sql))
	{
		return false;
	}
	
	TeAttributeList& attrList = portal->getAttributeList();

	unsigned int numColumns = attrList.size();

	TeTable table(outputTable);
	table.setAttributeList(attrList);

	while(portal->fetchRow())
	{
		TeTableRow row;
		for(unsigned int i = 0; i < numColumns; ++i)
		{
			row.push_back(portal->getData(i));
		}
		table.add(row);

		if(table.size() % 100 == 0)
		{
			if(!outputDatabase->insertTable(table))
			{
				delete portal;
				return false;
			}
			table.clear();
		}	
	}

	if(table.size() > 0)
	{
		if(!outputDatabase->insertTable(table))
		{
			delete portal;
			return false;
		}
	}
	table.clear();

	delete portal;

	return true;
}


