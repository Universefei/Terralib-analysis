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

#include <TeQtCheckListItem.h>
#include "../../kernel/TeUtils.h"
#include <vector>
#include <cstdlib>


int TeQtCheckListItem::compare (QListViewItem *i, int col, bool ascending) const
{
	int order1 = atoi(key(col, ascending).latin1());
	int order2 = atoi(i->key(col, ascending).latin1());

	if (order1 < order2)
		return -1;
	else if (order1 == order2)
		return 0;
	else 
		return 1;
}


QString TeQtCheckListItem::key ( int /* col */, bool /* ascending */ ) const
{
	return Te2String(order_).c_str();
}


vector<QListViewItem*> TeQtCheckListItem::getChildren()
{
	vector<QListViewItem*> childrenVector;

	QListViewItem *child = firstChild();
    while (child)
	{
		childrenVector.push_back(child);
        child = child->nextSibling();
	}
	return childrenVector;
}


void TeQtCheckListItem::unselectChildren()
{
	TeQtCheckListItem *item;
	int n = childCount();
	int i = 0;

	if (n == 0)
		return;

	QListViewItemIterator it(this);
	++it;
	item = (TeQtCheckListItem*)(it.current());
	while(item)
	{
		if (item->parent() == this)
		{
			item->setSelected(false);
			item->repaint();
			++i;
		}
		if (i == n)
			break;
		++it;
		item = (TeQtCheckListItem*)(it.current());
	}
}


bool TeQtCheckListItem::isChild(QListViewItem *item)
{
	if (item->parent() == this)
		return true;
	else
		return false;
}

