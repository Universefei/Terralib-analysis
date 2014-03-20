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

#include <TeQtThemeItem.h>
#include <TeQtViewsListView.h>
#include <TeAppTheme.h>
#include <TeQtLegendItem.h>
#include <TeQtChartItem.h>
#include "../../kernel/TeDatabase.h"
#include <qpixmap.h>
#include "../../../../app_icons/theme.xpm"
#include "../../../../app_icons/fileTheme.xpm"
#include "../../../../app_icons/externalTheme.xpm"
#include "../../../../app_icons/invalidExternalTheme.xpm"
#include <qimage.h>

//Database item constructor
TeQtThemeItem::TeQtThemeItem(QListViewItem *parent,
	QString text, TeAppTheme* appTheme)
	: TeQtCheckListItem(parent,text,CheckBox), appTheme_(appTheme)
{
	type_ = THEME;
	TeAbstractTheme* absTheme = appTheme_->getTheme();

	if (absTheme->type() == TeTHEME)
		setPixmap(0, QPixmap(theme_xpm));
	else if (absTheme->type() == TeFILETHEME)
		setPixmap(0, QPixmap(fileTheme_xpm));
	else if (absTheme->type() == TeEXTERNALTHEME)
		setPixmap(0, QPixmap(externalTheme_xpm));

	invalidThemeItem_ = false;
	setRenameEnabled(0,true);
	setSelected(false);
	setEnabled(true);

	order_ = absTheme->priority();
}


void TeQtThemeItem::setThemeItemAsInvalid()
{
	invalidThemeItem_ = true;
	TeAbstractTheme* absTheme = appTheme_->getTheme();
	if (absTheme->type() == TeEXTERNALTHEME)
		setPixmap(0, QPixmap(invalidExternalTheme_xpm));
}


void TeQtThemeItem::stateChange(bool visible)
{
	TeQtViewsListView *lView = (TeQtViewsListView*)listView();
	lView->checkItemStateChanged(this, visible);
}


void TeQtThemeItem::removeLegends()
{
	vector<QListViewItem*> legendVector = getChildren();
	unsigned int i;
	for (i = 0; i < legendVector.size(); ++i)
	{
		TeQtCheckListItem *checkItem = (TeQtCheckListItem*)legendVector[i];
		if (checkItem->getType() == TeQtCheckListItem::LEGEND ||
			checkItem->getType() == TeQtCheckListItem::LEGENDTITLE)
			delete checkItem;
	}
}


void TeQtThemeItem::removeCharts()
{
	vector<QListViewItem*> legendVector = getChildren();
	unsigned int i;
	for (i = 0; i < legendVector.size(); ++i)
	{
		TeQtCheckListItem *checkItem = (TeQtCheckListItem*)legendVector[i];
		if (checkItem->getType() == TeQtCheckListItem::CHART ||
			checkItem->getType() == TeQtCheckListItem::CHARTTITLE)
			delete checkItem;
	}
}


void TeQtThemeItem::getLegends(vector<const QPixmap*>& pixmapVector, vector<string>& labelVector, string& title)
{
	vector<QListViewItem*> legendVector = getChildren();
	unsigned int i;
	for (i = 0; i < legendVector.size(); ++i)
	{
		TeQtCheckListItem *checkItem = (TeQtCheckListItem*)legendVector[i];

		if (checkItem->getType() == TeQtCheckListItem::LEGENDTITLE)
		{
			title = checkItem->text().latin1();
		}
		else if (checkItem->getType() == TeQtCheckListItem::LEGEND)
		{
			TeQtLegendItem *legendItem = (TeQtLegendItem*)checkItem;
			pixmapVector.push_back(legendItem->pixmap(0));
			labelVector.push_back(legendItem->text().latin1());
		}
	}
}
				
TeQtCheckListItem* TeQtThemeItem::getLegendItem(TeLegendEntry* leg)
{
	vector<QListViewItem*> legendVector = getChildren();
	unsigned int i;
	for (i = 0; i < legendVector.size(); ++i)
	{
		TeQtCheckListItem *checkItem = (TeQtCheckListItem*)legendVector[i];

		if (checkItem->getType() == TeQtCheckListItem::LEGEND)
		{
			TeQtLegendItem *legendItem = (TeQtLegendItem*)checkItem;
			if(legendItem->legendEntry()->id() == leg->id())
				return legendItem;
		}
	}
	return (TeQtCheckListItem*)0;
}

void TeQtThemeItem::updateAlias()
{
	TeTheme* baseTheme = (TeTheme*)appTheme_->getTheme();
	if(baseTheme->type() != TeTHEME)
		return;

	TeDatabase* db = baseTheme->layer()->database();
	map<int, map<string, string> >& mapThemeAlias = db->mapThemeAlias();
	map<string, string>& mapAA = mapThemeAlias[baseTheme->id()];

	vector<QListViewItem*> itemVec = getChildren();

	unsigned int i;
	for (i = 0; i < itemVec.size(); ++i)
	{
		TeQtCheckListItem *checkItem = (TeQtCheckListItem*)itemVec[i];

		if (checkItem->getType() == TeQtCheckListItem::LEGENDTITLE)
		{
			string attr, nattr;
			if(baseTheme->grouping().groupMode_ != TeNoGrouping)
			{
				attr = baseTheme->grouping().groupAttribute_.name_;
				nattr = baseTheme->grouping().groupNormAttribute_;
			}

			if(nattr.empty() || nattr == "NONE")
			{
				map<string, string>::iterator it = mapAA.find(attr);
				if(it != mapAA.end())
				{
					string alias = it->second;
					if(alias.empty())
						checkItem->setText(0, attr.c_str());
					else
						checkItem->setText(0, alias.c_str());
				}
				else
					checkItem->setText(0, attr.c_str());
			}
			else
			{
				string alias = attr;
				map<string, string>::iterator it = mapAA.find(attr);
				if(it != mapAA.end())
					alias = it->second;

				it = mapAA.find(nattr);
				if(it != mapAA.end())
					alias += "/" + it->second;
				else
					alias += "/" + nattr;

				checkItem->setText(0, alias.c_str());
			}
		}
		else if (checkItem->getType() == TeQtCheckListItem::CHART)
		{
			string attr = ((TeQtChartItem*)checkItem)->name();
			map<string, string>::iterator it = mapAA.find(attr);
			if(it != mapAA.end())
			{
				string alias = it->second;
				checkItem->setText(0, alias.c_str());
			}
			else
				checkItem->setText(0, attr.c_str());
		}
	}
}

