/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2004 INPE and Tecgraf/PUC-Rio.

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

#include "TeAppTheme.h"
#include "../../kernel/TeExternalTheme.h"
#include "../../kernel/TeUtils.h"
#include "../../kernel/TeDatabase.h"
#include "TeApplicationUtils.h"
#include "../../utils/TeDatabaseUtils.h"


TeAppTheme::TeAppTheme(TeAbstractTheme* theme) :
		theme_(theme),
		outOfCollectionVisibility_(false),
		withoutDataConnectionVisibility_(false),
		refineAttributeWhere_(""),
		refineSpatialWhere_(""), 
		refineTemporalWhere_(""),
		groupColorDir_(TeColorAscSatEnd),
		groupObjects_(TeAll),
		pieDiameter_ (0.0),
		pieMinDiameter_(0.0),
		pieMaxDiameter_(0.0),
		barMinHeight_(0.0),
		barMaxHeight_(0.0),
		barWidth_(0.0),
		chartMaxVal_(-1),
		chartMinVal_(-1),
		chartType_(TeBarChart),
		chartObjects_(TeAll),
		chartFunction_("AVG"),
		keepDimension_(0),
		showCanvasLegend_(false),
		canvasLegWidth_(0.0),
		canvasLegHeight_(0.0), 
		editRep_ (TeGeomRep(0)),
		mix_color_(false),
		count_obj_(false), 
		textTable_("")
{
}

TeAppTheme::~TeAppTheme()
{	
}


TeAppTheme::TeAppTheme(const TeAppTheme& other)
{
	theme_ = (TeAbstractTheme*)TeViewNodeFactory::make(other.theme_->getProductId());
	if(theme_->getProductId()==TeTHEME)
        *(TeTheme*)theme_ = *(TeTheme*)other.theme_;
	else if(theme_->getProductId()==TeEXTERNALTHEME)
		*(TeExternalTheme*)theme_ = *(TeExternalTheme*)other.theme_;
	else
		*theme_ =  *other.theme_;

	chartSelected_ = other.chartSelected_;
	chartPoint_ = other.chartPoint_;
	tePoint_ = other.tePoint_;
	outOfCollectionVisibility_ = other.outOfCollectionVisibility_;
	withoutDataConnectionVisibility_ = other.withoutDataConnectionVisibility_;
	refineAttributeWhere_ = other.refineAttributeWhere_;
	refineSpatialWhere_ = other.refineSpatialWhere_; 
	refineTemporalWhere_ = other.refineTemporalWhere_;
	groupColor_ = other.groupColor_;
	groupColorDir_ = other.groupColorDir_;
	groupObjects_ = other.groupObjects_;
	pieDiameter_ = other.pieDiameter_;
	pieAttr_ = other.pieAttr_;
	pieMinDiameter_ = other.pieMinDiameter_;
	pieMaxDiameter_ = other.pieMaxDiameter_;
	barMinHeight_ = other.barMinHeight_;
	barMaxHeight_ = other.barMaxHeight_;
	barWidth_ = other.barWidth_;
	chartMaxVal_ = other.chartMaxVal_;
	chartMinVal_ = other.chartMinVal_;
	chartType_ = other.chartType_;
	chartObjects_ = other.chartObjects_;
	chartFunction_ = other.chartFunction_;
	keepDimension_ = other.keepDimension_;
	showCanvasLegend_ = other.showCanvasLegend_;
	canvasLegWidth_ = other.canvasLegWidth_;
	canvasLegHeight_ = other.canvasLegHeight_; 
	editRep_ = other.editRep_;
	mix_color_ = other.mix_color_;
	count_obj_ = other.count_obj_; 
	textTable_ = other.textTable_;
}

 TeAppTheme& TeAppTheme::operator =(const TeAppTheme& other)
{
	if (this != &other)
	{
		theme_ = (TeAbstractTheme*)TeViewNodeFactory::make(other.theme_->getProductId());
		if(theme_->getProductId()==TeTHEME)
            *(TeTheme*)theme_ = *(TeTheme*)other.theme_;
		else if(theme_->getProductId()==TeEXTERNALTHEME)
			*(TeExternalTheme*)theme_ = *(TeExternalTheme*)other.theme_;
		else
			*theme_ =  *other.theme_;
		
		chartSelected_ = other.chartSelected_;
		chartPoint_ = other.chartPoint_;
		tePoint_ = other.tePoint_;
		outOfCollectionVisibility_ = other.outOfCollectionVisibility_;
		withoutDataConnectionVisibility_ = other.withoutDataConnectionVisibility_;
		refineAttributeWhere_ = other.refineAttributeWhere_;
		refineSpatialWhere_ = other.refineSpatialWhere_; 
		refineTemporalWhere_ = other.refineTemporalWhere_;
		groupColor_ = other.groupColor_;
		groupColorDir_ = other.groupColorDir_;
		groupObjects_ = other.groupObjects_;
		pieDiameter_ = other.pieDiameter_;
		pieAttr_ = other.pieAttr_;
		pieMinDiameter_ = other.pieMinDiameter_;
		pieMaxDiameter_ = other.pieMaxDiameter_;
		barMinHeight_ = other.barMinHeight_;
		barMaxHeight_ = other.barMaxHeight_;
		barWidth_ = other.barWidth_;
		chartMaxVal_ = other.chartMaxVal_;
		chartMinVal_ = other.chartMinVal_;
		chartType_ = other.chartType_;
		chartObjects_ = other.chartObjects_;
		chartFunction_ = other.chartFunction_;
		keepDimension_ = other.keepDimension_;
		showCanvasLegend_ = other.showCanvasLegend_;
		canvasLegWidth_ = other.canvasLegWidth_;
		canvasLegHeight_ = other.canvasLegHeight_; 
		editRep_ = other.editRep_;
		mix_color_ = other.mix_color_;
		count_obj_ = other.count_obj_; 
		textTable_ = other.textTable_;
	}

	return *this;
}


unsigned int TeAppTheme::getTableIndexFromField(string fieldName)
{
	unsigned int i, n, index;
	index = 0;

	TeTheme* theme = (TeTheme*)theme_;
	vector<TeTable>& tableVec = theme->attrTables();
	TeAttributeList attrList = theme->sqlAttList();

	n = tableVec[0].attributeList().size();
	for (i = 0; i < attrList.size(); ++i)
	{
		if (i > n - 1)
		{
			++index;
			n += tableVec[index].attributeList().size();
		}

		if (attrList[i].rep_.name_ == fieldName)
			break;
	}
	return index;
}



bool TeAppTheme::locateText (TeCoord2D &pt, TeText &text, const double& tol)
{
	TeTheme* theme = (TeTheme*)theme_;
	TeDatabasePortal* portal = theme->layer()->database()->getPortal();
	string table = textTable();

	TeBox box (pt.x()-tol,pt.y()-tol,pt.x()+tol,pt.y()+tol);
	string q = "SELECT " + table + ".* FROM " + table + ", " + theme->collectionTable();
	char buf [1024];
	sprintf (buf," WHERE c_object_id = object_id AND (x < %f AND x > %f AND y < %f AND y > %f)",
		box.x2(),box.x1(),box.y2(),box.y1());
	q += buf;
	if (!portal->query(q) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}
	// Look for all texts
	bool flag = true;
	TeTextSet ts;
	do
	{
		TeText p;
		flag = portal->fetchGeometry(p);
		ts.add ( p );
	} while (flag);

	delete portal;

	int k;
	if (TeNearest (pt, ts, k, tol))
	{
		text = ts[k];
		return true;
	}
	return false;
}

TeVisual TeAppTheme::getTextVisual(const TeText &text)
{
	int	dot_height, rgb;
	string family;
	string table = textTable() + "_txvisual";
	bool fix_size = false, bold = false, italic = false;

	TeTheme* theme = (TeTheme*)theme_;
	TeDatabase* db = theme->layer()->database();

	string sel = "SELECT dot_height, fix_size, color, family, bold, italic FROM ";
	sel += table + " WHERE geom_id = " + Te2String(text.geomId());

	TeDatabasePortal* portal = db->getPortal();
	if(portal->query(sel))
	{
		if(portal->fetchRow())
		{
			dot_height = atoi(portal->getData(0));
			if(atoi(portal->getData(1)))
				fix_size = true;
			rgb = atoi(portal->getData(2));
			family = portal->getData(3);
			if(atoi(portal->getData(4)))
				bold = true;
			if(atoi(portal->getData(5)))
				italic = true;
		}
	}
	delete portal;
	TeColor	cor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
	TeVisual visual;
	visual.size(dot_height);
	visual.fixedSize(fix_size);
	visual.color(cor);
	visual.family(family);
	visual.bold(bold);
	visual.italic(italic);

	return visual;
}


string TeAppTheme::getLegendTitleAlias()
{
	string text;

	TeTheme* theme = (TeTheme*)theme_;

	TeLegendEntryVector& legendVector = theme->legend();
	if (legendVector.size() > 0)
	{
		TeDatabase* database = theme->layer()->database();
		TeAttributeRep rep; 
		string nattr;
		if(theme->grouping().groupMode_ != TeNoGrouping)
		{
			rep = theme->grouping().groupAttribute_; 
			nattr = theme->grouping().groupNormAttribute_;
		}
		text = rep.name_;
		map<int, map<string, string> >::iterator it = database->mapThemeAlias().find(theme->id());
		if(it != database->mapThemeAlias().end())
		{
			map<string, string>& m = it->second;
			map<string, string>::iterator tit = m.find(text);
			if(tit != m.end())
				text = tit->second;
			if(!(nattr.empty() || nattr == "NONE"))
			{
				tit = m.find(nattr);
        		if(tit != m.end()) {
        		  if (!tit->second.empty()) {
					text += "/" + tit->second;
          		  }
        		}
        		else  //Deixando ficar sem barra 
					text += "/" + nattr;
			}
		}
	}
	return text;
}

string TeAppTheme::	getPieBarAlias(string atr)
{
	string text;

	TeTheme* theme = (TeTheme*)theme_;

	if (chartAttributes_.size() > 0)
	{
		TeDatabase* database = theme->layer()->database();
		map<int, map<string, string> >::iterator it = database->mapThemeAlias().find(theme->id());
		if(it != database->mapThemeAlias().end())
		{
			map<string, string>& m = it->second;
			map<string, string>::iterator tit = m.find(atr);
			if(tit != m.end())
				text = tit->second;
		}
	}
	return text;
}

bool TeAppTheme::setAlias(string attr, string attAlias)
{
	TeTheme* theme = (TeTheme*)theme_;
	TeDatabase* db = theme->layer()->database();
	map<int, map<string, string> >& mapThemeAlias = db->mapThemeAlias();
	map<string, string>& mapAA = mapThemeAlias[theme->id()];
	map<string, string>::iterator it = mapAA.begin();

	while(it != mapAA.end())
	{
		if(it->first != attr && it->second == attAlias)
			return false;
		it++;
	}

	int f = attr.find("/");
	int af = attAlias.find("/");
	if(f >= 0 && af >= 0)
	{
		string nattr = attr;
		attr = attr.substr(0, f);
		nattr = nattr.substr(f+1);

		string alias = attAlias;
		string nalias = alias;
		alias = alias.substr(0, af);
		nalias = nalias.substr(af+1);

		mapAA[attr] = alias;
		mapAA[nattr] = nalias;
		updateAppTheme (db, this);
		return true;
	}
	else
	{
    	//Alterando para deixar ficar sem /
    	if(f >= 0) {
      		string nattr = attr;
      		nattr = nattr.substr(f+1);
      		mapAA[nattr] = "";
			attr = attr.substr(0, f);
    	}
		mapAA[attr] = attAlias;
		updateAppTheme (db, this);
		return true;
	}
}

void TeAppTheme::removeAlias(string attribute)
{
	string attr = attribute;
	string nattr = attr;
	TeTheme* theme = (TeTheme*)theme_;
	TeDatabase* db = theme->layer()->database();
	map<int, map<string, string> >& mapThemeAlias = db->mapThemeAlias();
	map<string, string>& mapAA = mapThemeAlias[theme->id()];

	int f = attr.find("/");
	if(f >= 0)
	{
		attr = attr.substr(0, f);
		nattr = nattr.substr(f+1);
	}
	map<string, string>::iterator it = mapAA.find(attr);
	if(it != mapAA.end())
	{
		mapAA.erase(it);
		if(mapAA.size() == 0)
		{
			map<int, map<string, string> >::iterator tit = mapThemeAlias.find(theme->id());
			if(tit != mapThemeAlias.end())
				mapThemeAlias.erase(tit);
		}
	}
	if(nattr != attr)
	{
		map<string, string>::iterator it = mapAA.find(nattr);
		if(it != mapAA.end())
		{
			mapAA.erase(it);
			if(mapAA.size() == 0)
			{
				map<int, map<string, string> >::iterator tit = mapThemeAlias.find(theme->id());
				if(tit != mapThemeAlias.end())
					mapThemeAlias.erase(tit);
			}
		}
	}

	updateAppTheme (db, this);
}

void TeAppTheme::removeAllAlias()
{
	TeTheme* theme = (TeTheme*)theme_;
	TeDatabase* db = theme->layer()->database();
	map<int, map<string, string> >& mapThemeAlias = db->mapThemeAlias();
	map<int, map<string, string> >::iterator it = mapThemeAlias.find(theme->id());
	if(it != mapThemeAlias.end())
		mapThemeAlias.erase(it);
	updateAppTheme (db, this);
}

void TeAppTheme::concatTableName(string& attr)
{
	string tableName;

	if(attr.find(".") == attr.npos)
	{
		TeTheme* theme = (TeTheme*)theme_;
		vector<TeTable>& tableVec = theme->attrTables();
		TeAttrTableVector::iterator it = tableVec.begin();

		while(it != tableVec.end())
		{
			TeTable	t = *it;
			TeAttributeList AL = t.attributeList();
			TeAttributeList::iterator iit = AL.begin();

			while(iit != AL.end())
			{
				if((*iit).rep_.name_ == attr)
				{
					tableName = (*it).name();
					break;
				}
				iit++;
			}

			if(tableName.empty() == false)
			{
				tableName += ".";
				attr.insert(0, tableName);
				break;
			}
			it++;
		}
	}
}

int TeAppTheme::numObjects() 
{
  return theme_->getNumberOfObjects();
}

TeBox& TeAppTheme::boundingBox()
{
   return theme_->getThemeBox();
}

TeTheme*  
TeAppTheme::getSourceTheme()
{
	TeTheme* t = 0;
	if(!theme_)
		return t;
	if(theme_->getProductId() == TeEXTERNALTHEME)
		t = static_cast<TeExternalTheme*>(theme_)->getRemoteTheme();
	else if(theme_->getProductId() == TeTHEME)
		t = (TeTheme*) theme_;
	return t;
}

TeDatabase*		
TeAppTheme::getLocalDatabase()
{
	TeDatabase* db = 0;
	if(!theme_)
		return db;
	if(theme_->getProductId() == TeEXTERNALTHEME)
		db = static_cast<TeExternalTheme*>(theme_)->getSourceDatabase();
	else if(theme_->getProductId() == TeTHEME)
		db = static_cast<TeTheme*>(theme_)->layer()->database();
	return db;
}
	

string			
TeAppTheme::getLocalCollectionTable()
{
	string colTable = "";
	if(!theme_)
		return colTable;
	if(theme_->getProductId() == TeEXTERNALTHEME || theme_->getProductId() == TeTHEME)
		colTable = static_cast<TeTheme*>(theme_)->collectionTable();
	return colTable;
}




