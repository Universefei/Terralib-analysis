/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
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


#include "TeSTElementSet.h"
#include "TeQtMethods.h"
#include "TeSkaterFunctions.h"
#include "TeQtTerraStat.h"
#include "TeQtGrid.h"
#include "TeSTEFunctionsDB.h"
#include "TeDatabaseUtils.h"
#include <qobject.h>
#include <qapplication.h>

/////////////////////////////////////////////////////////////////////////////////////


bool TeSkaterMethod::apply(vector<double>& het) 
{
  if (this->hasPop_) 
  {
    this->attrVector_.push_back(this->attrPop_);
  }

  //add the aggregate attribute in the attribute vetor
  if(!aggrAttr_.empty())
	  attrVector_.push_back(aggrAttr_);
    
  TeSTElementSet stoSet(regTheme_);

	if(TeProgress::instance())
	{
		string caption = tr("Skater function").latin1();
		TeProgress::instance()->setCaption(caption.c_str());
	}

  //verify if the theme has external table
  TeAttrTableVector tables;
  if(regTheme_->getAttTables(tables, TeAttrExternal))
  {
	  TeGroupingAttr grouping;
	  for(unsigned int j=0; j<attrVector_.size(); ++j)
	  { 
		  	TeAttributeRep attrRep(attrVector_[j]);
			pair<TeAttributeRep, TeStatisticType> attr(attrRep, TeMEAN);
			grouping.push_back(attr);
	  }

	  if (!TeSTOSetBuildDB(&stoSet, grouping, true)) 
	  {
			QMessageBox::critical(NULL, tr("Error"), 
				tr("Unexpected: Regions and attributes could not be loaded!"));
			return false;
	  }
  }
  else
  {
	  if (!TeSTOSetBuildDB(&stoSet, true, false, attrVector_)) 
	  {
			QMessageBox::critical(NULL, tr("Error"), 
				tr("Unexpected: Regions and attributes could not be loaded!"));
			return false;
	  }
  }

	//containers to store information when the data will be aggregate by an attribute
	map<string, TeSTElementSet* >	aggrAttrToElemSet;
  
	//get the distinct values of the attribute value
	if(!aggrAttr_.empty())
	{
		TeSTElementSet::iterator it = stoSet.begin();
		int index = stoSet.getAttributeIndex(aggrAttr_);
		if(index<0)
		{
			QMessageBox::critical(NULL, tr("Error"), 
				tr("It is no possible aggregate objects by attribute!"));
			return false;
		}

		map<string, TeSTElementSet* >::iterator it2;
		while(it!=stoSet.end())
		{
			//get the aggregate attribute
			string val;
			it->getPropertyValue(val, index);
			it2 = aggrAttrToElemSet.find(val);
			if( it2 == aggrAttrToElemSet.end()) 
			{
				TeSTElementSet* elemSet = new TeSTElementSet(regTheme_);
				TeAttributeList fromAttList = stoSet.getAttributeList();
				TeAttributeList toAttList;
				for(unsigned int i=0; i< fromAttList.size(); ++i)
				{
					if((int)i == index)
						continue;
					toAttList.push_back (fromAttList[i]);
				}

				elemSet->setAttributeList(toAttList);
				it->removePropertyValue(index);
				elemSet->insertSTInstance(*it);
				aggrAttrToElemSet[val] = elemSet;				
			}
			else
			{
				it->removePropertyValue(index);
				it2->second->insertSTInstance(*it);
			}

			++it;
		}
	}
	
	double tol = TeGetPrecision(regTheme_->layer()->projection());
	TePrecision::instance().setPrecision(tol);

	//load prox matrix or construct one
	TeGeneralizedProxMatrix<TeSTElementSet>*			proxMat=0;
	TeProxMatrixConstructionStrategy<TeSTElementSet>*	constStrategy=0;

	if(TeProgress::instance())
	{
		string msg = tr("Loading Proximity Matrix. Please, wait!").latin1();
		TeProgress::instance()->setMessage(msg);
	}
	//-------------- Load prox matrix or create new
	if(!loadDefaultGPM(regTheme_->layer()->database(), regTheme_->id(), proxMat))
	{
		TeGeomRep geomRep = TeGEOMETRYNONE;
		if(regTheme_->layer()->hasGeometry (TePOLYGONS))
			geomRep = TePOLYGONS;
		else if (regTheme_->layer()->hasGeometry (TeCELLS))
			geomRep = TeCELLS;

		if(geomRep==TeGEOMETRYNONE)
		{
			QString msg = tr("The theme must have a geometric representation of polygons") + "\n";
			msg += tr("or cells to create contiguity matrix!");
			QMessageBox::warning(this, tr("Warning"), msg);
			map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
			while(itSTElemSets != aggrAttrToElemSet.end())
			{

				if(itSTElemSets->second)
					delete itSTElemSets->second;
				++itSTElemSets;
			}

			aggrAttrToElemSet.clear();
			return false;
		}
		
		if(TeProgress::instance())
		{
			string msg = tr("Building the Proximity Matrix. Please, wait!").latin1();
			TeProgress::instance()->setMessage(msg);
		}
		
		//create matrix and save in the database
		constStrategy = new TeProxMatrixLocalAdjacencyStrategy (&stoSet, geomRep); 
		TeProxMatrixNoWeightsStrategy sw_no;
		TeProxMatrixNoSlicingStrategy ss_no;
		proxMat = new TeGeneralizedProxMatrix<TeSTElementSet>(constStrategy, &sw_no, &ss_no);
		
		if (!proxMat->constructMatrix())
		{
			QMessageBox::critical(this, tr("Error"), tr("Fail building the proximity matrix!"));
			if(proxMat)
				delete proxMat;
			map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
			while(itSTElemSets != aggrAttrToElemSet.end())
			{

				if(itSTElemSets->second)
					delete itSTElemSets->second;
				++itSTElemSets;
			}
			aggrAttrToElemSet.clear();
			return false;
		}

		//save in the database
		bool savedMatrix = true;
		if(!regTheme_->layer()->database()->tableExist("te_gpm"))
		{
			if(!createGPMMetadataTable(regTheme_->layer()->database()))
			{
				QMessageBox::warning(this, tr("Error"), 
					tr("It was not possible to save proximity matrix in database!"));
				savedMatrix = false;
			}
		}

		string tableName = "te_neighbourhood_"+ Te2String(regTheme_->id());
		string tempTable = tableName;
		int count = 1;
		while(savedMatrix && regTheme_->layer()->database()->tableExist(tempTable))
		{
			tempTable = tableName+"_"+Te2String(count);
			++count;
		}

		if(savedMatrix && (!createNeighbourhoodTable(regTheme_->layer()->database(), tempTable)))
		{
			QMessageBox::warning(this, tr("Error"), 
					tr("It was not possible to save proximity matrix in database!"));
				savedMatrix = false;
		}

		proxMat->neighbourhood_table_=tempTable;
		//load object ids
		vector<string> objIds;
		TeSTElementSet::iterator itAux = stoSet.begin();
		while(itAux!=stoSet.end())
		{
			objIds.push_back(itAux->objectId());
			++itAux;
		}

		if(savedMatrix && (!insertGPM(regTheme_->layer()->database(), proxMat, objIds)))
		{
			QMessageBox::warning(this, tr("Error"), 
					tr("It was not possible to save proximity matrix in database!"));
				savedMatrix = false;
		}
	}

	//if the data must be aggregated by an attribute 
	int numGroups = this->nClusters_;
	if(!aggrAttr_.empty())
	{
		int step = 0;
		if(TeProgress::instance())
		{
			TeProgress::instance()->reset();
			string msg = tr("Calculating skater... Please, wait!").latin1();
			TeProgress::instance()->setMessage(msg);
			TeProgress::instance()->setTotalSteps(aggrAttrToElemSet.size());
		}
		
		map<string, TeSTElementSet* >::iterator it = aggrAttrToElemSet.begin();
		while(it != aggrAttrToElemSet.end())
		{
			this->nClusters_ = numGroups;
			TeSTElementSet* elSet = it->second;
			TeGeneralizedProxMatrix<TeSTElementSet>	tempGPM;
			TeSTElementSet::iterator itSet = elSet->begin();
			string v = it->first;
			
			//! Build the prox matrix only for the objects in each element set
			bool objectWithoutNeig = false;
			while(itSet != elSet->end())
			{
				TeNeighboursMap neigs = proxMat->getMapNeighbours(itSet->objectId());
				TeNeighboursMap::iterator itN = neigs.begin();
				while(itN != neigs.end())
				{
					string obj1 = itN->first;
					string obj2 = itSet->objectId();
					if(elSet->hasElement(itN->first))
						tempGPM.connectObjects(itSet->objectId(), itN->first, itN->second);
					++itN;
				}
				
				if((tempGPM.getNeighbours(itSet->objectId()).size())<1)
				{
					objectWithoutNeig = true;
					itSet = elSet->end();
					continue;
				}

				++itSet;
			}
			
			int numObjs = tempGPM.numberOfObjects(); 
			if(objectWithoutNeig || numObjs<2)
			{
				++it;
				continue;
			}

			if(numObjs < this->nClusters_)
				this->nClusters_ = numObjs;

			//! Skater function
			if (!TeSkaterFunction(this->hasPop_, this->type_, this->nClusters_, this->minPop_, *elSet, &tempGPM, this->resName_, het) ) 
			{
				QMessageBox::critical(NULL, tr("Error"), tr("Choose a connected graph!"));
				if(proxMat)
					delete proxMat;
			
				map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
				while(itSTElemSets != aggrAttrToElemSet.end())
				{

					if(itSTElemSets->second)
						delete itSTElemSets->second;
					++itSTElemSets;
				}
				aggrAttrToElemSet.clear();
				return false;
			}

			if (!insertPropertyColumn(*elSet, tableName_, resName_) ) 
			{
				QMessageBox::critical(NULL, tr("Error"), tr("Error storing results. Check result column name!"));
				if(proxMat)
					delete proxMat;

				map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
				while(itSTElemSets != aggrAttrToElemSet.end())
				{

					if(itSTElemSets->second)
						delete itSTElemSets->second;
					++itSTElemSets;
				}
				aggrAttrToElemSet.clear();
				return false;
			}
			
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
				{
					TeProgress::instance()->reset();
					if(proxMat)
						delete proxMat;

					map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
					while(itSTElemSets != aggrAttrToElemSet.end())
					{

						if(itSTElemSets->second)
							delete itSTElemSets->second;
						++itSTElemSets;
					}
					aggrAttrToElemSet.clear();
					return false;
				}
				else
					TeProgress::instance()->setProgress(step);
			}	
			++step;
			++it;
		}
		if (TeProgress::instance())
			TeProgress::instance()->reset();
	}
	else
	{
		if (!TeSkaterFunction(this->hasPop_, this->type_, this->nClusters_, this->minPop_, stoSet, proxMat, this->resName_, het) ) 
		{
			QMessageBox::critical(NULL, tr("Error"), tr("Choose a connected graph!"));
			if(proxMat)
				delete proxMat;
			map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
			while(itSTElemSets != aggrAttrToElemSet.end())
			{

				if(itSTElemSets->second)
					delete itSTElemSets->second;
				++itSTElemSets;
			}
			aggrAttrToElemSet.clear();
			return false;
		}

		double h;
		for(unsigned int i = 0; i < het.size(); i++) 
		{
			h = het[i];
		}

		if (!insertPropertyColumn(stoSet, tableName_, resName_) ) 
		{
			QMessageBox::critical(NULL, tr("Error"), tr("Error storing results. Check result column name!"));
			if(proxMat)
				delete proxMat;
			map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
			while(itSTElemSets != aggrAttrToElemSet.end())
			{

				if(itSTElemSets->second)
					delete itSTElemSets->second;
				++itSTElemSets;
			}
			aggrAttrToElemSet.clear();
			return false;
		}
	}

  // ---------------------------------------------- load updated table from database 
	// Update all the themes that uses this table
	TeViewMap& viewMap = regTheme_->layer()->database()->viewMap();
	TeViewMap::iterator it;
	set<TeLayer*> layerSet;
	for (it = viewMap.begin(); it != viewMap.end(); ++it)
	{
		TeView *view = it->second;
		vector<TeViewNode*>& themesVector = view->themes();
		for (unsigned int i = 0; i < themesVector.size(); ++i)
		{
			TeTheme* theme = (TeTheme*)themesVector[i];
			if (theme->type() != TeTHEME)
				continue;
			if (theme->isThemeTable(tableName_) == true)
			{
				theme->loadThemeTables();
				layerSet.insert(theme->layer());
			}
		}
	}

	// Update the layer tables affected
	set<TeLayer*>::iterator setIt;
	for (setIt = layerSet.begin(); setIt != layerSet.end(); ++setIt)
		(*setIt)->loadLayerTables();

	//delete pointers
	map<string, TeSTElementSet* >::iterator itSTElemSets = aggrAttrToElemSet.begin();
	while(itSTElemSets != aggrAttrToElemSet.end())
	{

		if(itSTElemSets->second)
			delete itSTElemSets->second;
		++itSTElemSets;
	}
	aggrAttrToElemSet.clear();
	if(proxMat)
		delete proxMat;
	return true;
}

