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

#ifndef  __TERRALIB_INTERNAL_QTCHECKLISTITEM_H
#define  __TERRALIB_INTERNAL_QTCHECKLISTITEM_H

#include <qlistview.h>
#include <vector>
#include <qpainter.h>
#include <qpalette.h>

#include <../../kernel/TeDefines.h>

using namespace std;


class TL_DLL TeQtCheckListItem : public QCheckListItem
{

public:
	enum ItemType {DATABASE, INFOLAYER, VIEW, THEME, LEGENDTITLE, LEGEND, CHARTTITLE, CHART};

    TeQtCheckListItem(QListView *parent, QString text, QCheckListItem::Type ctype = QCheckListItem::Controller)
		: QCheckListItem(parent, text, ctype) {}
 
	TeQtCheckListItem(QListViewItem *parent, QString text, QCheckListItem::Type ctype = QCheckListItem::Controller)
		: QCheckListItem(parent, text, ctype) {}

	TeQtCheckListItem(QCheckListItem *parent, QString text, QCheckListItem::Type ctype = QCheckListItem::Controller)
	: QCheckListItem(parent, text, ctype) {}
	
	virtual ~TeQtCheckListItem() {}

	ItemType getType() {return type_;}
	
	int order()
		{return order_;}

	void order(int order)
		{order_ = order;}

	virtual int compare(QListViewItem * i, int col, bool ascending) const;

	virtual QString key(int column, bool ascending) const;

	vector<QListViewItem*> getChildren();

	void unselectChildren();

	bool isChild(QListViewItem *item);

	void cancelRename(int col)
		{ QListViewItem::cancelRename(col); }

	virtual void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align)
		{ QCheckListItem::paintCell(p, cg, column, width, align); }


protected:
	ItemType type_;
	int order_;
};

#endif
