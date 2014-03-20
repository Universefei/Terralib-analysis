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

#include <TeQtViewItem.h>
#include <TeQtThemeItem.h>
#include <qpixmap.h>
#include "../../../../app_icons/view.xpm"


//Database item constructor
TeQtViewItem::TeQtViewItem(QListView *parent,
	QString text, TeView *view)
	: TeQtCheckListItem(parent,text), view_(view)
{
	type_ = VIEW;
//	QPixmap p("images/view.bmp");
	QPixmap p((const char**)view_xpm);
	setPixmap(0,p);
	setRenameEnabled(0,true);
	setSelected(false);
	setEnabled(true);
	setDragEnabled(false);
}

void	
TeQtViewItem::resetAll()
{
	vector<QListViewItem*> childrenVector = this->getChildren();
	for (int i = 0; i < (int)childrenVector.size(); ++i)
		((TeQtThemeItem*)childrenVector[i])->setOn(false);
}

vector<TeQtThemeItem*> TeQtViewItem::getThemeItemVec()
{
	vector<TeQtThemeItem*> themeItemVec;

	TeQtCheckListItem*	checkListItem;
	TeQtThemeItem*	themeItem;

	QListViewItemIterator it(this);
	++it; // Lauro
	while(it.current())
	{
		checkListItem = (TeQtCheckListItem*)it.current();
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			themeItem = (TeQtThemeItem*)it.current();
			themeItemVec.push_back(themeItem);
		}
		else if(checkListItem->getType() == TeQtCheckListItem::VIEW) // Lauro
			break; // Lauro

		++it;
	}

	return themeItemVec;
}
