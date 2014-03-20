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
#include "TeKernelParams.h"
#include "TeQtTerraStat.h"
#include "TeKernelFunctions.h"
#include "TeStatDataStructures.h"
#include "TeQtDatabasesListView.h"
///#include "TeImportRaster.h"
#include "TeDecoderDatabase.h"
#include "TeQtViewsListView.h"
#include "TeQtGrid.h"
#include "TeSTEFunctionsDB.h"
#include "TeWaitCursor.h"
#include "TeColorUtils.h"
#include "TeDatabaseUtils.h"
#include "TeKernelParams.h"
#include "TeQtLayerItem.h"
#include "TeQtViewItem.h"
#include "TeQtThemeItem.h"
#include <qobject.h>
#include <qapplication.h>


string getKernelFunction (TeKernelFunctionType t)
{
	string result;

	switch (t)
	{
		case TeKQuartic:
      result = QObject::tr("Quartic").latin1();
			break;
		case TeKNormal:
			result = QObject::tr("Normal").latin1();
			break;
		case TeKUniform:
			result = QObject::tr("Uniform").latin1();
			break;
		case TeKTriangular:
			result = QObject::tr("Triangular").latin1();
			break;
		case TeKNegExponential:
			result = QObject::tr("NegExponential").latin1();
			break;
	}
	return result;
}

string getKernelComputeType (TeKernelComputeType t)
{
	string result;

	switch (t)
	{
		case TeKDensity:
			result = QObject::tr("Density").latin1();
			break;
		case TeKProbability:
			result = QObject::tr("Probability").latin1();
			break;
		case TeKMovingAverage:
			result = QObject::tr("Spatial Moving Average").latin1();
			break;
	}
	return result;
}

string getCombinationType (TeKernelCombinationType /* t */)
{
	string result;
	result = "DESCONHECIDO";
	return result;
}


/**
 * Retorna legenda para kernel
 **/
bool getKernelLegend(TeKernelParams& par, vector<string>& result)
{
	string legend = QObject::tr("Kernel parameters").latin1();
	result.push_back(legend); 
	string numb = "";
	
	legend = QObject::tr("Event theme").ascii()+ numb +" : "+ par.eventThemeName1_ ;
	result.push_back(legend);
	
	if(!par.intensityAttrName1_.empty())
	{
		legend = QObject::tr("Attribute name").ascii()+ numb +" : "+ par.intensityAttrTable1_+"."+ par.intensityAttrName1_;
		result.push_back(legend);
	}

  if (par.radiusValue1_ > 0) {
	  legend = QObject::tr("Radius value").ascii()+ numb +" : "+ Te2String(par.radiusValue1_);
  }
  else {
    legend = QObject::tr("Adaptive radius").ascii();
  }
  result.push_back(legend);

  legend = QObject::tr("Compute: ").ascii()+getKernelComputeType(par.computeType1_);
  result.push_back(legend);

  legend = QObject::tr("Function: ").ascii()+getKernelFunction(par.kernelFunction1_);
  result.push_back(legend);

  return true;
}


/**
 * Grava uma coluna na tabela de atributos associada a um STOSet.
 * Nome da coluna eh o mesmo nome da propriedade
 **/
bool insertPropertyColumn(TeSTElementSet& stoSet,
                          const string& tableName,
                          const string&  columnName) 
{
	
	
 	vector<int> index;
	int ind = stoSet.getAttributeIndex(columnName);
	if(ind<0)
	{
		ind = stoSet.getAttributeIndex(tableName+"."+columnName);
		if(ind>-1)
			index.push_back (ind);
	}
	else
		index.push_back (ind);
  
	if(!index.empty())
	{
		if (!TeUpdateDBFromSet (&stoSet, tableName, &index)) 
			return false;
	}
	else
	{
		if (!TeUpdateDBFromSet (&stoSet, tableName)) 
			return false;
	}
	return true;
}

/**
 * Grava uma coluna na tabela de atributos associada a um STOSet.
 * Nome da coluna eh o mesmo nome da propriedade
 **/
bool insertPropertyColumn(TeSTStatInstanceSet& stSet,
                          int idxDoubleProp,
                          TeTheme* theme,
                          const string& tableName,
                          const string& columnName) {

  TeProperty prop;

  prop.attr_.rep_.name_ = columnName;
  prop.attr_.rep_.type_ = TeREAL;
  double value;
  TePropertyVector pVect;
  pVect.push_back(prop);

  //Constroi um de outro
  TeSTElementSet rs(theme);
  rs.addProperty(prop.attr_);

  TeSTStatInstanceSet::iterator ito = stSet.begin();
  while (ito != stSet.end()) {

    //Cria uma nova propriedade no objeto
    (*ito).getDoubleProperty(idxDoubleProp, value);
    pVect[0].value_ = Te2String(value, STAT_PRECISION);
    (*ito).properties(pVect);
    rs.insertSTInstance(*ito);
    ++ito;
  }

  return insertPropertyColumn(rs, tableName, columnName) ;
}


/***
 * Cria um STOSet a partir de um tema e um conjunto de atributos,
 * colocando todas as tabelas do TEMA
 ***/
bool createSTOSetFromTheme(TeTheme* theme, 
                              vector<string>& attributes,
                              TeSTElementSet* rs) 
{
	rs->setTheme(theme);
	return TeSTOSetBuildDB(rs, true, false, attributes);
}


/***
 * Cria um STOSet a partir de um tema e um conjunto de atributos,
 * colocando todas as tabelas do TEMA
 ***/
bool createSTOStatSetFromTheme(TeTheme* theme, 
                               vector<string>& attributes,
                               int additionalDouble,
                               TeSTStatInstanceSet* rs) {

  TeSTElementSet stoSet(theme);
   if (!TeSTOSetBuildDB(&stoSet, true, false, attributes))
     return false;
   rs->Copy(stoSet, attributes.size(), additionalDouble);
   return true;

}

TeKernelMethod::~TeKernelMethod()
{
	if (evSet1_)
		delete (TePointTInstanceSet *)evSet1_;
	
	if (evSet2_)
		delete (TePointTInstanceSet *)evSet2_;
	
	if(regSet_ && (isGridEv_ || isGridReg_))
	{
		delete (TeKernelGridSupport*)regSet_ ;
	}
	else if (regSet_)
	{
		delete (TeSTStatInstanceSet*)regSet_;
	}
	regSet_=0;
	evSet2_=0;
	evSet1_=0;
}

void TeKernelMethod::error(TeKernelErrorType type, string message, QWidget* parent) {

  QString title;
  switch (type) {
  case Uerror:
	  title = tr("Unexpected Error");
    break;
  case Error:
		title = tr("Error");
    break;
  case Warning:
		title = tr("Warning");
    break;
  }

  if (parent) {
	  QMessageBox::information(parent, title, tr(message.c_str()));
  }
  else {
    QMessageBox::information(0, (title), tr(message.c_str())); 
  }

}

/**
 * Defines name, theme pointer and id for a given theme 
 * id = 0 -- support theme
 * id = 1,2 -- first and second event theme
 **/
bool TeKernelMethod::setTheme(string name, int id, TeView* view) {
  if (id == 0) {
    params_.supportThemeName_ = name;
    regTheme_ = view->get(params_.supportThemeName_);
    if (evTheme1_ == NULL) {
      return false;
    }
	params_.supportThemeId_ = regTheme_->id(); 
  }
  else if (id == 1) {
    params_.eventThemeName1_ = name;

    evTheme1_ = view->get(params_.eventThemeName1_);
    if (evTheme1_ == NULL) {
      return false;
    }
    params_.eventThemeId1_ = evTheme1_->id();
    
  }
  else if (id == 2) {
    params_.eventThemeName2_ = name;

    evTheme2_ = view->get(params_.eventThemeName2_);
    if (evTheme2_ == NULL) {
      return false;
    }
    params_.eventThemeId2_ = evTheme2_->id();
    
  }
  return true;
}


enum TeGridType {GridFromSupport, GridFromEvent, GridFromBoth};


/**
 * Generates support set from parameters
 *   If grid, generate box
 *   If theme, generate column
 **/
bool TeKernelMethod::generateSupportSet() {
 
  
  bool isGrid = isGridReg_ || isGridEv_;
  bool isRatio = (params_.eventThemeId2_ > 0);

  justOnePolygon_ = false;

  //support set is from type TeKernelGridSupport
  if (isGrid) 
  {

    //Computes grid boundary and defines if just one set.
    TeGridType t;
    TeBox      gridBound;

    if (isGridEv_)
    {
      if (isRatio)
        t = GridFromBoth;
      else
        t = GridFromEvent;
    }
    else
      t = GridFromSupport;
 
    if (db_ == NULL) {
      error(Uerror, tr("There is no database. Please, contact support!").latin1());
      return false;
    }

    TeBox bound1;
    TeBox bound2;
    
    switch(t) {
    case GridFromSupport:
      if (regTheme_ == NULL) {
		error(Uerror, tr("There is no support theme. Please, contact support!").latin1());
        return false;
      }
	  gridBound = regTheme_->getThemeBox();
      break;
    case GridFromEvent:
      if (evTheme1_ == NULL) {
		error(Uerror, tr("There is no event theme. Please, contact support!").latin1());
        return false;
      }
	  gridBound = evTheme1_->getThemeBox();
      break;
    case GridFromBoth:
      if ((evTheme1_ == NULL) || (evTheme2_ == NULL)) {
		error(Uerror, tr("There is no event theme. Please, contact support!").latin1());
        return false;
      }

      bound1 = evTheme1_->getThemeBox();
      bound2 = evTheme2_->getThemeBox();
    
      //Box is the rectangle that envolves both boxes
      gridBound.x1_ = (bound1.x1_ < bound2.x1_) ? bound1.x1_ : bound2.x1_;
      gridBound.y1_ = (bound1.y1_ < bound2.y1_) ? bound1.y1_ : bound2.y1_;
      gridBound.x2_ = (bound1.x2_ > bound2.x2_) ? bound1.x2_ : bound2.x2_;
      gridBound.y2_ = (bound1.y2_ < bound2.y2_) ? bound1.y2_ : bound2.y2_;

      break;

    default:
		error(Uerror, tr("Invalid type. Please, contact support!").latin1());
      return false;
      break;
    }
    
    
    if (!gridBound.isValid()) {
		error(Uerror, tr("Invalid grid box. Please, contact support!").latin1());
        return false;
    }


    //Verifies if it has just one polygon
    if (t == GridFromSupport) {

      TeDatabasePortal* portal = this->db_->getPortal();

      TeLayer* l = regTheme_->layer();

      if (l->hasGeometry(TePOLYGONS))
      {
        string count = "SELECT COUNT(*) FROM " + l->tableName(TePOLYGONS);
        count += ", " + regTheme_->collectionTable() + " WHERE ";
        count += l->tableName(TePOLYGONS) + ".object_id = ";
        count += regTheme_->collectionTable() + ".c_object_id";
        portal->freeResult();
        if(portal->query(count))
        {
          if(portal->fetchRow())
          {
            count = portal->getData(0);		
          }
          else {
 			error(Uerror, tr("There is no polygons. Please, contact support!").latin1());
          }
        }

        int nPol = atoi(count.c_str());
        this->justOnePolygon_ = (nPol == 1);
		delete portal;
      }
    }

    TeProjection* proj = db_->loadProjection(this->evTheme1_->layer()->projection()->id());

    if (proj == NULL) {
 			error(Uerror, tr("There is no projection. Please, contact support!").latin1());
       return false;
    }
	if (this->params_.eventThemeId2_ > 0) // if it is kernel ratio
		regSet_ = new TeKernelGridSupport(gridBound, gridCols_, proj, 2);   
	else
		regSet_ = new TeKernelGridSupport(gridBound, gridCols_, proj);   
  }
  //Gera grid irregular a partir de poligonos do layer
  else {
    
    //Define regioes a partir de tema -- aplicar restricao
    if (regTheme_ == NULL) {
 		error(Uerror, tr("There is no support theme. Please, contact support!").latin1());
      return false;
    }

	//Generates set from theme
    vector<string> empty;
    //TeSTStatInstanceSet* rs = new TeSTStatInstanceSet();
	regSet_ = new TeSTStatInstanceSet();
    createSTOStatSetFromTheme(regTheme_, empty, 2, (TeSTStatInstanceSet*)regSet_);
    resName_ = string(this->params_.supportThemeAttr_);
    //regSet_ = rs;
  }
  return true;
}


/**
 * Generates event set from theme. If there is an attribute, set is initialize with it.
 * Otherwise, just geometries are loaded. Geometries may be points or polygons
 **/
bool TeKernelMethod::generateEventSet(int which) {

  bool     isPoint;
  TeTheme* theme;
  TeLayer* layer;
  string   table;
  string   column;
    
  if (which == 1) {
    isPoint = this->isPoint1_;
    theme = this->evTheme1_;
    if (this->evTheme1_ == NULL) {
 		error(Uerror, tr("There is no event set for the kernel. Please, contact support!").latin1());
      return false;
    }
    layer = this->evTheme1_->layer();
    if (layer == NULL) {
 		error(Uerror, tr("There is no layer for the theme. Please, contact support!").latin1());
      return false;
    }
    table = this->params_.intensityAttrTable1_;
    column = this->params_.intensityAttrName1_;
  }
  else if (which == 2) {
    isPoint = this->isPoint2_;
    theme = this->evTheme2_;
    if (this->evTheme2_ == NULL) {
 		error(Uerror, tr("There is no event set for the kernel. Please, contact support!").latin1());
      return false;
    }
    layer = this->evTheme2_->layer();
    if (layer == NULL) {
 		error(Uerror, tr("There is no layer for the theme. Please, contact support!").latin1());
      return false;
    }
    table = this->params_.intensityAttrTable2_;
    column = this->params_.intensityAttrName2_;
  }
  else {
 		error(Uerror, tr("Invalid event set. Please, contact support!").latin1());
    return false;
  }
  
  TeWaitCursor wait;

  bool done;
  vector<string> vecAttr;
  if (column.compare("")) {
      vecAttr.push_back(table+"."+column);
  }
 if (isPoint) {

    //Generates set from theme
    TePointTInstanceSet* evs = new TePointTInstanceSet();
    if (vecAttr.size() == 0) {
      done =  createSTOStatSetFromTheme(theme, vecAttr, 1, evs);
      //Deve inicializar com 1 toda a propriedade
      TePointTInstanceSet::iterator itev = evs->begin();
      while (itev != evs->end()) {
        (*itev).setDoubleProperty(0, 1.0);
        ++itev;
      }
    }
    else {
      done = createSTOStatSetFromTheme(theme, vecAttr, 0, evs);
    }

    if (!done)
      return false;
    if (which == 1) {
      evSet1_ = evs;
    }
    else {
      evSet2_ = evs;
    }

  }
  else {

      //Generates set from theme
    TePointTInstanceSet* evs = new TePointTInstanceSet();
    if (vecAttr.size() == 0) {
      done =  createSTOStatSetFromTheme(theme, vecAttr, 1, evs);
      //Deve inicializar com 1 toda a propriedade
      TePointTInstanceSet::iterator itev = evs->begin();
      while (itev != evs->end()) {
        (*itev).setDoubleProperty(0, 1.0);
        ++itev;
      }
  }
  else {
      done = createSTOStatSetFromTheme(theme, vecAttr, 0, evs);
    }
    if (!done)
      return false;
    if (which == 1) {
      evSet1_ = evs;
    }
    else {
      evSet2_ = evs;
    }
  }
  return true;
}


bool TeKernelMethod::call() 
{
  bool isGrid = this->isGridEv_ || this->isGridReg_;
  bool isRatio = this->params_.eventThemeName2_.compare("");

//  bool justOnePolygon = false;

  double totalArea;
  if (isGrid)  {
    totalArea = ((TeKernelGridSupport *)regSet_)->totalArea();
  }
  else {
    //SUBSTITUIR
	  totalArea = TeGeometryArea(regTheme_->getThemeBox());
  }

  int  totalRegions;
  if (isGrid) {
      totalRegions = ((TeKernelGridSupport *)regSet_)->numObjects();
  }
  else {
      totalRegions = ((TeSTStatInstanceSet *)regSet_)->numObjects();
  }

  if (TeProgress::instance())
  {
		TeProgress::instance()->setTotalSteps(totalRegions);
  }

  //Kernel de razao
  if (isRatio) {
   
   
    TeStatKernelRatio kerRatio;
    kerRatio.numReg_ = totalRegions;
    kerRatio.kComb_ = params_.combinationType_;
    kerRatio.ktype_ = params_.computeType1_;
    kerRatio.kfunc1_ = params_.kernelFunction1_;
    kerRatio.radius1_ = params_.radiusValue1_;
    kerRatio.kfunc2_ = params_.kernelFunction2_;
    kerRatio.radius2_ = params_.radiusValue2_;
    kerRatio.totalArea_ = totalArea;


    //Colocar janela com parametros para o usuario confirmar

    if (isGrid) {
      //Verifica se o iterador eh para um unico poligono
      if (justOnePolygon_) {
        TeDatabasePortal* portal = db_->getPortal();

        string count = "SELECT c_object_id FROM " + regTheme_->layer()->tableName(TePOLYGONS)+",";
        count += regTheme_->collectionTable() + " WHERE ";
        count += regTheme_->layer()->tableName(TePOLYGONS) + ".object_id = ";
        count += regTheme_->collectionTable() + ".c_object_id";
        portal->freeResult();
        if(portal->query(count))
        {
          if(portal->fetchRow())
          {
            count = portal->getData(0);		
          }
          else {
 			      error(Uerror, tr("There is no polygons. Please, contact support!").latin1());
            return false;
          }
        }
		    delete portal;
        TePolygonSet polygon;
        if (!regTheme_->layer()->loadGeometrySet(count, polygon)) {
 		    	error(Uerror, tr("There is no polygon boundary. Please, contact support!").latin1());
          return false;
        }
        boundary_ = polygon.first();
        if (boundary_.empty()) {
 		    	error(Uerror, tr("There is no polygon boundary. Please, contact support!").latin1());
          return false;
        }
        return kerRatio.apply(*((TePointTInstanceSet *)evSet1_),
            *((TePointTInstanceSet *)evSet2_),
            (*((TeKernelGridSupport *)regSet_)).begin(boundary_), 
            (*((TeKernelGridSupport *)regSet_)).end(boundary_));
      }
      else {
        return kerRatio.apply(*((TePointTInstanceSet *)evSet1_),
            *((TePointTInstanceSet *)evSet2_),
            (*((TeKernelGridSupport *)regSet_)).begin(), 
            (*((TeKernelGridSupport *)regSet_)).end());
      }
    }
    else {
       return kerRatio.apply(*((TePointTInstanceSet *)evSet1_), 
         *((TePointTInstanceSet *)evSet2_),
          (*((TeSTStatInstanceSet *)regSet_)).begin(), 
          (*((TeSTStatInstanceSet *)regSet_)).end());
    }
  }  
  //Kernel normal com um unico conjunto
  else {
   
    TeStatKernel kernel;
    kernel.radius_ = params_.radiusValue1_;
    kernel.kfunc_ =  params_.kernelFunction1_;
    kernel.ktype_ = params_.computeType1_;
    kernel.totalArea_ = totalArea;
    kernel.numReg_ = totalRegions;

    //Colocar janela com parametros para o usuario confirmar

    if (isGrid) {
      //Verifica se o iterador eh para um unico poligono
      if (justOnePolygon_) {
        TeDatabasePortal* portal = db_->getPortal();

        string count = "SELECT c_object_id FROM " + regTheme_->layer()->tableName(TePOLYGONS)+",";
        count += regTheme_->collectionTable() + " WHERE ";
        count += regTheme_->layer()->tableName(TePOLYGONS) + ".object_id = ";
        count += regTheme_->collectionTable() + ".c_object_id";
        portal->freeResult();
        if(portal->query(count))
        {
          if(portal->fetchRow())
          {
            count = portal->getData(0);		
          }
          else {
 			      error(Uerror, tr("There is no polygons. Please, contact support!").latin1());
            return false;
          }
        }
		    delete portal;
        TePolygonSet polygon;
        if (!regTheme_->layer()->loadGeometrySet(count, polygon)) {
 		    	error(Uerror, tr("There is no polygon boundary. Please, contact support!").latin1());
          return false;
        }
        boundary_ = polygon.first();
        if (boundary_.empty()) {
 		    	error(Uerror, tr("There is no polygon boundary. Please, contact support!").latin1());
          return false;
        }
        return kernel.apply(*((TePointTInstanceSet *)evSet1_),
            (*((TeKernelGridSupport *)regSet_)).begin(boundary_), 
            (*((TeKernelGridSupport *)regSet_)).end(boundary_));
      }
      else {
        return kernel.apply(*((TePointTInstanceSet *)evSet1_),
            (*((TeKernelGridSupport *)regSet_)).begin(), 
            (*((TeKernelGridSupport *)regSet_)).end());
      }
    }
    else {
       return kernel.apply(*((TePointTInstanceSet *)evSet1_), 
          (*((TeSTStatInstanceSet *)regSet_)).begin(), 
          (*((TeSTStatInstanceSet *)regSet_)).end());
    }
  }  
  return true;
}

 

/**
 * Apply kernel method based on parameters definition
 *   Create event(s) set
 *   Create support set
 *   Execute suitable kernel method (basic, adaptive, ratio)
 *   Returns support set region
 **/
bool TeKernelMethod::apply(void*& supportSet, QWidget* parent) 
{
	precision_ = -1;

	//Verifies if there is an event theme
	if (this->evTheme1_ == NULL) {
		error(Uerror, tr("There is no event set for the kernel. Please, contact support!").latin1());
		return false;
	}

	//Verifies if this theme has objects without geometries
	TeGeomRep rep1 = TePOINTS;
	if(!isPoint1_)
		rep1 = TePOLYGONS;
	if(this->evTheme1_->hasObjectsWithoutGeometries(rep1))
	{
		string errorMessage = tr("The theme ").latin1();
		errorMessage += " - "+ this->evTheme1_->name() +" - "; 
		errorMessage += tr("\n has occurrences/events without location. ").latin1();
		errorMessage += tr("\n These occurrences should be removed to generate a valid kernel map.").latin1();  
		errorMessage += tr("\n Would you like to remove them now? ").latin1();
		
		int response = QMessageBox::question(this, tr("Question"), QString(errorMessage.c_str()), tr("Yes"), tr("No")); 
		if(response == 0)
		{
			//remove objetcs without geometries
			if(!this->evTheme1_->removeObjectsWithoutGeometries(rep1))
			{
				QMessageBox::information(this, tr("Information"), 
					tr("Occurrences/events without location could not be removed.\nTry to generate the theme again."));
				return false;
			}
			
			QMessageBox::information(this, tr("Information"), 
					tr("Occurrences/events without location were removed from the theme."));
		}
		else
		{
			return false;
		}
	}

	//Verifies if all themes are in the same projection
	TeProjection* ev1 = this->evTheme1_->getThemeProjection();
	if (this->regTheme_ != NULL) 
	{
		TeProjection* reg = regTheme_->getThemeProjection();
		if (!(*reg == *ev1)) {
			error(Warning,tr("All of the themes should be in the same projection!").latin1(),parent);
			return false;
		}
	}

	if (this->evTheme2_ != NULL) 
	{
		TeProjection* ev2 = this->evTheme2_->getThemeProjection();
		if (!(*ev1 == *ev2)) {
			error(Warning,tr("All of the themes should be in the same projection!").latin1(),parent);
			return false;
		}

		//Verifies if this theme has objects without geometries
		TeGeomRep rep2 = TePOINTS;
		if(!isPoint2_)
			rep2 = TePOLYGONS;
		if(this->evTheme2_->hasObjectsWithoutGeometries(rep2))
		{
			string errorMessage = tr("The theme ").latin1();
			errorMessage += " - "+ this->evTheme1_->name() +" - "; 
			errorMessage += tr("\n has occurrences/events without location. ").latin1();
			errorMessage += tr("\n These occurrences should be removed to generate a valid kernel map.").latin1();  
			errorMessage += tr("\n Would you like to remove them now? ").latin1();
			
			int response = QMessageBox::question(this, tr("Question"), QString(errorMessage.c_str()), tr("Yes"), tr("No")); 
			if(response == 0)
			{
				//remove objetcs without geometries
				if(!this->evTheme2_->removeObjectsWithoutGeometries(rep2))
				{
					QMessageBox::information(this, tr("Information"), 
					tr("Occurrences/events without location could not be removed.\nTry to generate the theme again."));
					return false;
				}
				QMessageBox::information(this, tr("Information"), 
					tr("Occurrences/events without location were removed from the theme."));
			}
		}
	}

	if (TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(4);
		TeProgress::instance()->setMessage(tr("Generating the support set...").latin1());
		TeProgress::instance()->setProgress(1);
	}
	
	// Generates support set
	if (!this->generateSupportSet() ) {
		if (TeProgress::instance())
			TeProgress::instance()->reset();
		error(Error, tr("It was not possible to create the support set!").latin1(), parent);
		return false;
	}
	if (TeProgress::instance()) {
		if (TeProgress::instance()->wasCancelled() ) {
			TeProgress::instance()->reset();
			return false;
		}
		else 
		{
			TeProgress::instance()->setMessage(tr("Generating the event set...").latin1());
			TeProgress::instance()->setProgress(2);
		}
	}

	// Generates first event set
	if (!this->generateEventSet(1)) {
		if (TeProgress::instance())
			TeProgress::instance()->reset();
		error(Error, tr("It was not possible to create the event set!").latin1(),parent);
		return false;
	}

	if (TeProgress::instance()) {
		if (TeProgress::instance()->wasCancelled()) {
			TeProgress::instance()->reset();
			return false;
		}
		else
			TeProgress::instance()->setTotalSteps(4);
	}


	// Generates second event set
	if (this->evTheme2_ != NULL) {
		if (TeProgress::instance())
		{
			TeProgress::instance()->setTotalSteps(5);
			TeProgress::instance()->setMessage(tr("Generating second event set...").latin1());
			TeProgress::instance()->setProgress(3);
		}
		if (!this->generateEventSet(2)) {
			if (TeProgress::instance())
				TeProgress::instance()->reset();
			error(Error, tr("It was not possible to create the event set 2!").latin1(),parent);
			return false;
		}
	}

	if (TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(5);
		TeProgress::instance()->setMessage(tr("Calculating Kernel Map...").latin1());
		TeProgress::instance()->setProgress(4);
	}

	// Calls kernel functions
	bool okKernel = this->call();
	if (okKernel) 
		supportSet = this->regSet_;

  //Deallocate temporary values
   if (this->evSet1_)
  {
      delete (TePointTInstanceSet *)this->evSet1_;
	  evSet1_=0;
  }

  if (this->evSet2_)
  {
     delete (TePointTInstanceSet *)this->evSet2_;
	 evSet2_=0;
  }
//    delete this->evSet2_;

	return okKernel;
}



bool TeKernelMethod::getPrecision(int& prec) {

  //Precisao ja foi calculada
  if (precision_ >= 0) {
    prec= precision_;
    return true;
  }

  bool isGrid = isGridReg_ || isGridEv_;
  if (isGrid) {
    if (justOnePolygon_) {
      if (this->getPrecision((*((TeKernelGridSupport *)regSet_)).begin(boundary_), 
        (*((TeKernelGridSupport *)regSet_)).end(boundary_))) {
        prec = precision_;
        return true;
      }
      else
        return false;
    }
    else {
      if (this->getPrecision((*((TeKernelGridSupport *)regSet_)).begin(), 
        (*((TeKernelGridSupport *)regSet_)).end())) {
        prec = precision_;
        return true;
      }
      else
        return false;
    }
  }
  if (this->getPrecision((*((TeSTStatInstanceSet *)regSet_)).begin(), 
    (*((TeSTStatInstanceSet *)regSet_)).end())) {
    prec = precision_;
    return true;
  }
  else {
    return false;
  }

}

bool TeKernelMethod::setGridLayer(string name) {

  this->resName_ = name;
  this->gridLayer_ = NULL;

  // Check whether there is a layer with this name in the database
  TeLayerMap& layerMap = db_->layerMap();
  TeLayerMap::iterator it;
  for (it = layerMap.begin(); it != layerMap.end(); ++it)
  {
    if (it->second->name() == name)
    {
      this->gridLayer_ = it->second;
      this->params_.generatedLayerId_ = it->second->id();
      break;
    }
  }

  //Vai criar uma nova -- verifica se nome esta ok
  if (!gridLayer_)
  {
//    size_t pos = name.find("-",string::npos,1);
    size_t pos = name.find("-", 0, 1);
   if (pos != string::npos)
    {
		QMessageBox::warning( NULL, tr("Error"), 
			tr("The layer name cannot have the \"-\" character!"));
      return false;
    }
    char ch = name.at(0);
    if (isdigit(ch))
    {
		QMessageBox::information( NULL, tr("Error"),
			tr("The layer name cannot have numerical characters!"));
      return false;
    }
  }

  return true;
}


bool TeKernelMethod::createLayer(TeRaster* raster,
                                 TerraViewBase* mWindow)
{
 
  TeQtDatabasesListView* dbView = mWindow->getDatabasesListView();
  if (!isNewLayer())  {

    //Recupera item associado ao layer para poder remover 
    TeQtLayerItem* lItPrev = dbView->currentLayerItem();
    dbView->selectLayerItem(gridLayer_);
    TeQtLayerItem* layerItem = dbView->currentLayerItem();
    if (lItPrev != layerItem) {
      mWindow->removeItem((TeQtCheckListItem*)layerItem);
      if (lItPrev != NULL)
        dbView->selectLayerItem(lItPrev->getLayer());
    }
    else {
      mWindow->removeItem(layerItem);
    }
  }
  TeLayer* l = new TeLayer(resName_, db_, raster->projection());
  int bw, bh;
  if (raster->params().ncols_ > 512)
  {
	  bw = 256;
	  bh = 256;
  }
  else
  {
	  bw = raster->params().ncols_;
	  bh = raster->params().nlines_;
  }

  	// create a raster geometry in a TerraLib database
	TeRasterParams pDB = raster->params();
	pDB.fileName_ = "rasterkernel" + Te2String(l->id()); // the name of the raster table that will contain the blob's
	pDB.nBands(1);
	pDB.setDataType(TeDOUBLE);
	pDB.setDummy(raster->params().dummy_[0]);
	pDB.mode_ = 'c';
	// parameters specific of the database decoder
	pDB.decoderIdentifier_ = "DB";			// a database decoder 
	pDB.database_ = db_;					// pointer to the database
	pDB.tiling_type_ = TeRasterParams::TeNoExpansible;      
	pDB.blockHeight_ = bh;					// we are storing the entire raster in one block
	pDB.blockWidth_ = bw;
	pDB.setPhotometric(TeRasterParams::TeMultiBand);	// the photometric interpretation of the raster
	TeRaster rstDb(pDB);
	rstDb.init();
	if (rstDb.params().status_ == TeRasterParams::TeReadyToWrite)
	{
		double val;
		int col, lin;
		for (lin=0;lin<rstDb.params().nlines_;++lin)
			for (col=0;col<rstDb.params().ncols_;++col)
			{
				raster->getElement(col,lin,val,0);
				rstDb.setElement(col,lin,val,0);
			}
	}
    l->addRasterGeometry(&rstDb,"O1");	
	rstDb.clear();
	l->raster(0);
 
  params_.generatedLayerId_ = l->id();
  
  TeQtDatabaseItem *dbItem = dbView->currentDatabaseItem();
  
  TeQtLayerItem *layerItem = 
    new TeQtLayerItem((QListViewItem*)dbItem,l->name().c_str(), l);
  layerItem->setEnabled(true);
  if (!dbView->isOpen(layerItem->parent()))
				dbView->setOpen(layerItem->parent(),true);
  mWindow->checkWidgetEnabling();

  //Verifica se existe tema com mesmo nome e remove
	TeQtViewItem* curViewItem = mWindow->getViewsListView()->currentViewItem();
	vector<TeQtThemeItem*> themeItemVec = curViewItem->getThemeItemVec();
	unsigned int i;
	for (i = 0; i < themeItemVec.size(); ++i)
	{
		TeAppTheme* appTheme = themeItemVec[i]->getAppTheme();
		TeTheme* theme = (TeTheme*)appTheme->getTheme();
		if (theme && !resName_.compare(theme->name()))
		{
			TeQtViewsListView* viewList = mWindow->getViewsListView();
			TeQtThemeItem* themeItem = viewList->getThemeItem(appTheme);
			if (themeItem != NULL)
				mWindow->removeItem((TeQtCheckListItem*)themeItem);
		}
	}
	TeAttrTableVector ATV;
  if (! mWindow->createTheme(resName_, mWindow->currentView(), l, "", ATV))
    return false;


  //Recupera apontador para o tema criado
	themeItemVec = curViewItem->getThemeItemVec();
	for (i = 0; i < themeItemVec.size(); i++)
	{
		TeAppTheme* appTheme = themeItemVec[i]->getAppTheme();
		TeTheme* theme = (TeTheme*)appTheme->getTheme();
		if (!resName_.compare(theme->name()))
		{
			this->rasterTheme_ = theme;
		}
	}


  if (this->rasterTheme_ == NULL) {
      QMessageBox::information( NULL, tr("Error"),
		  tr("There is no theme with the raster representation!"));
      return false;
  }
  return true;
}

bool TeKernelMethod::createColumn(TeAppTheme* currTheme,
                         TeQtGrid* grid) 
{

	grid->clear();
	if(!insertPropertyColumn(*((TeSTStatInstanceSet *)this->regSet_),
      0, regTheme_, params_.supportThemeTable_, params_.supportThemeAttr_))
	  return false;

	 // ---------------------------------------------- load updated table from database 
	// Update all the themes that uses this table

	TeViewMap& viewMap = this->db_->viewMap();
	TeViewMap::iterator it;
	set<TeLayer*> layerSet;
	for (it = viewMap.begin(); it != viewMap.end(); ++it)
	{
		TeView *view = it->second;
		vector<TeViewNode*>& themesVector = view->themes();
		for (unsigned int i = 0; i < themesVector.size(); ++i)
		{
			if(themesVector[i]->type() == TeTHEME)
			{
				TeTheme* theme = (TeTheme*)themesVector[i];
				if (theme->isThemeTable(params_.supportThemeTable_) == true)
				{
					theme->loadThemeTables();
					layerSet.insert(theme->layer());
				}
			}
		}
	}

	// Update the layer tables affected
	set<TeLayer*>::iterator setIt;
	for (setIt = layerSet.begin(); setIt != layerSet.end(); ++setIt)
		(*setIt)->loadLayerTables();

	// ---------------------------------------------- grid 
	if(currTheme!= NULL)
	{
		grid->init(currTheme);
		grid->refresh();
		grid->goToLastColumn();
	}

	return true;
}

//Alterando pois soh precisa internamente
bool TeKernelMethod::createKernelLegends(int nColors, bool /* color */, TeAppTheme* appTheme)
{
  	int precision;
	if (!getPrecision(precision)) 
	{
		QMessageBox::critical(NULL, tr("Error"), 
		tr("Unexpected error in the evaluation of the kernel precision!"));
		return false;
	}
	
	precision=15; //teste
  
	// recupera o tema sobre o qual foi calculado kernel
	TeTheme* theme = 0;
	if (this->rasterTheme_ != NULL)
		theme = this->rasterTheme_;
	else
		theme = this->regTheme_;

	TeAttributeRep rep;				  
	rep.type_ = TeREAL;

	TeGrouping groupKernel;
	groupKernel.groupNumSlices_ = nColors;
	groupKernel.groupPrecision_ = precision;

	if (theme->layer()->geomRep() & TeRASTER)    // suporte foi grade
	{
		groupKernel.groupMode_ = TeRasterSlicing;
		rep.name_ = "0";
	}
	else										// suporte foi regioes
	{
		groupKernel.groupMode_ = TeEqualSteps;
		rep.name_ = this->params_.supportThemeTable_ + "." + this->params_.supportThemeAttr_;
	}
	groupKernel.groupAttribute_ = rep;

	if (!theme->buildGrouping(groupKernel))
		return false;
  
	vector<string> colorRamp;
	colorRamp.push_back("CYAN");
	colorRamp.push_back("GREEN");
	colorRamp.push_back("YELLOW");
	colorRamp.push_back("ORANGE");
	colorRamp.push_back("RED");

	// cria rampa de cores default para dados de kernel e define visual de cada fatia
	vector<TeColor> colorGrad;
	getColors(colorRamp,10, colorGrad);
	for (int g=0; g<theme->grouping().groupNumSlices_; g++)
	{
		TeVisual visual(TePOLYGONS);            
		visual.color(colorGrad[g]);
		if((theme->layer()->geomRep() & TePOLYGONS) || 
		   (theme->layer()->geomRep() & TeCELLS) || 
		   (theme->layer()->geomRep() & TeRASTER))
		{
			visual.transparency(theme->defaultLegend().visual(TePOLYGONS)->transparency());
			visual.contourStyle(theme->defaultLegend().visual(TePOLYGONS)->contourStyle());
			visual.contourWidth(theme->defaultLegend().visual(TePOLYGONS)->contourWidth());
			visual.contourColor(theme->defaultLegend().visual(TePOLYGONS)->contourColor());	
			theme->setGroupingVisual(g+1,visual.copy(),TePOLYGONS);
		}
		if(theme->layer()->geomRep() & TeLINES)
		{
			visual.style(theme->defaultLegend().visual(TeLINES)->style());
			visual.width(theme->defaultLegend().visual(TeLINES)->width());
			theme->setGroupingVisual(g+1,visual.copy(),TeLINES);
		}
		else if(theme->layer()->geomRep() & TePOINTS)
		{
			visual.style(theme->defaultLegend().visual(TePOINTS)->style());
			visual.size(theme->defaultLegend().visual(TePOINTS)->size());
			theme->setGroupingVisual(g+1,visual.copy(),TePOINTS);
		}
	}
	colorGrad.clear();

	// acrescenta algumas informacoes relativas ao theme application

	std::string scores("Cy-G-Y-Or-R");
	vector<ColorBar> bg =  getColorBarVector(scores, true);
 	string groupingColors = getColors(bg, bg,TeEqualSteps);
	appTheme->groupColor(groupingColors);
	appTheme->countObj(false);
	appTheme->mixColor(true);
	appTheme->groupColorDir(TeColorAscSatBegin);
	if (!theme->saveGrouping())
		return false;
	if (!(theme->layer()->geomRep() & TeRASTER)  && !theme->saveLegendInCollection())
		return false;
	
	theme->visibleRep(theme->visibleRep() | 0x40000000);
	updateAppTheme(this->db_,appTheme);

	if (this->rasterTheme_ != NULL)
		appTheme->setAlias("0",tr("Kernel Values").latin1());
	return true;
}

