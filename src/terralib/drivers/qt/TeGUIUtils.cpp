#include "TeGUIUtils.h"
#include "../../kernel/TeViewNode.h"
#include "../../kernel/TeView.h"
#include "../../kernel/TeDatabase.h"

bool fillThemeCombo(TeGeomRep tg, TeView* view, QComboBox* cBox, string selName, bool clear)
{
//	bool enableWindow = false;
	
	if (clear)
		cBox->clear();
	
	int idx = 0;
	int count = 0;

	vector<TeViewNode *>& themeVec = view->themes();
	for(unsigned int i=0; i<themeVec.size(); i++)
	{
		TeAbstractTheme* t = (TeAbstractTheme*)themeVec[i];
		if(t->type() < 2 && t->visibleGeoRep() & tg)
		{
			cBox->insertItem(t->name().c_str());
			if (!t->name().compare(selName)) 
			{
				idx = count;
			}
			count++;
		}
	}
  
	if (count > 0) 
	{
		cBox->setCurrentItem(idx);
	}
	
	return (count > 0);
}

bool fillThemeCombo(vector<TeGeomRep>& tg, TeView* view, QComboBox* cBox, string selName, bool clear)
{
	if(clear)
		cBox->clear();
	
	vector<TeViewNode *> themeVec = view->themes();
	if (clear)
		cBox->clear();
	
	int idx = 0;
	int count = 0;
	
	for(unsigned int i=0; i<themeVec.size(); i++)
	{
		TeAbstractTheme* t = (TeAbstractTheme*)themeVec[i];
		if (t->type() >= 2)
			continue;

		bool hasGeomRep = false;
		for(unsigned int j=0; j<tg.size(); ++j)
		{
			if(t->visibleGeoRep() & tg[j])
				hasGeomRep = true;
		}
		
		if(hasGeomRep)
		{
			cBox->insertItem(t->name().c_str());
			if (!t->name().compare(selName)) 
			{
				idx = count;
			}
			count++;
		}
	}
  
	if (count > 0) 
	{
		cBox->setCurrentItem(idx);
	}
	
	return (count > 0);
}

bool fillThemeCombo( TeView* view, QComboBox* cBox, string selName, bool clear)
{
	vector<TeViewNode *> themeVec = view->themes();
	
	if (clear)
		cBox->clear();
	
	int idx = 0;
	int count = 0;
	
	for(unsigned int i=0; i<themeVec.size(); i++)
	{
		TeAbstractTheme* t = (TeAbstractTheme*)themeVec[i];
		if (t->type() >= 2)
			continue;
		cBox->insertItem(t->name().c_str());
		if (!t->name().compare(selName)) 
		{
			idx = count;
		}
		count++;
	}
  
	if (count > 0) 
	{
		cBox->setCurrentItem(idx);
	}
	
	return (count > 0);
}


bool fillTableCombo(TeView* view,
                     QComboBox *cTableCombo, 
                     QComboBox *cThemeCombo,
                     string selName, bool clear, TeAttrTableType tableType)
{
	TeTable ttable;
	string tableName;
	string themeName =  string(cThemeCombo->currentText().ascii());
	TeTheme* currTheme = view->get(themeName);

	if (currTheme == NULL) 
		return false;

	TeAttrTableVector atvec;
	currTheme->getAttTables(atvec, tableType);
	
	if (atvec.size() == 0)
		return false;

	if (clear)
		cTableCombo->clear();
	int idx = 0;
	
	for(unsigned int i=0; i<atvec.size(); i++)
	{
		TeTable ttable = atvec[i];
		tableName = ttable.name();
		cTableCombo->insertItem(tableName.c_str());
		if(selName.compare(tableName)) 
		{
			idx = i;
		}
	}
	
	cTableCombo->setCurrentItem(idx);
	return true;
}


bool fillColumnCombo(TeAttrDataType aType,
                      TeTheme* theme,
                      QComboBox *cColumnCombo, 
                      QComboBox *cTableCombo,
                      string selName, bool clear) 
{

//	bool done = false;
  
	TeAttributeList columns;
	TeAttrTableVector atvec;
  theme->getAttTables(atvec);

	string tableName = string(cTableCombo->currentText().ascii());
	unsigned int t;

	for (t = 0; t < atvec.size(); t++) 
	{
		if (!tableName.compare(atvec[t].name()))
			break;
	}
	
	if (t == atvec.size())
		return false;

	TeTable ttable = atvec[t];
	columns = ttable.attributeList();
	
	if (clear)
		cColumnCombo->clear();
	
	int idx = 0;
	int count = 0;
	
	for(unsigned int i=0; i<columns.size(); i++)
	{
		int type = columns[i].rep_.type_;
		string colName = columns[i].rep_.name_;	
		if ((aType == TeUNKNOWN) || (type == aType)) 
		{
			cColumnCombo->insertItem(colName.c_str());
			if (!selName.compare(colName)) 
			{
				idx = count;
			}
			count++;
		}
	}
	
	if (count > 0) 
		cColumnCombo->setCurrentItem(idx);
	
	return count> 0;
}

bool fillColumnCombo(vector<TeAttrDataType> &aTypeVec,
                      TeTheme* theme,
                      QComboBox *cColumnCombo, 
                      QComboBox *cTableCombo,
                      string selName, bool clear)
{
	if(clear)
		cColumnCombo->clear();

	for(unsigned int i = 0; i < aTypeVec.size(); i++)
		fillColumnCombo(aTypeVec[i], theme, cColumnCombo, cTableCombo, selName, false);
	
	return true;
}

bool fillColumnCombo(vector<TeAttrDataType> &aTypeVec, TeTheme* theme,  QComboBox *cColumnCombo, bool clear)
{
	if(clear)
		cColumnCombo->clear();
	TeAttrTableVector attrTables = theme->attrTables();
	for(unsigned int i=0; i<attrTables.size(); ++i)
	{
		TeAttributeList attrList = attrTables[i].attributeList();
		for(unsigned int j=0; j<attrList.size(); ++j)
		{
			for(unsigned int type=0; type<aTypeVec.size(); ++type)
			{
				if(attrList[j].rep_.type_==aTypeVec[type])
				{
					string tableCol = attrTables[i].name()+"."+attrList[j].rep_.name_;
					cColumnCombo->insertItem(tableCol.c_str());
				}
			}
		}
	}
	return true;
}

bool fillLayerCombo(TeDatabase* db, TeGeomRep rep, QComboBox *lColumnCombo, const string& current, bool clear)
{
	int cur=0;
	if (clear)
		lColumnCombo->clear();
	else
		cur=lColumnCombo->count();

	TeLayerMap& layerMap = db->layerMap();
	TeLayerMap::iterator itlay = layerMap.begin();
	int i=0;
	while ( itlay != layerMap.end() )
	{
		if ((*itlay).second->geomRep() & rep)
		{
			lColumnCombo->insertItem((*itlay).second->name().c_str());
			if ((*itlay).second->name() == current)
				cur += i;
			++i;
		}
		++itlay;
	}
	lColumnCombo->setCurrentItem(cur);
	return i>0;
}
