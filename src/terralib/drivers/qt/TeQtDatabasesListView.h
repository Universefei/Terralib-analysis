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

#ifndef  __TERRALIB_INTERNAL_QTDATABASESLISTVIEW_H
#define  __TERRALIB_INTERNAL_QTDATABASESLISTVIEW_H


class QPopupMenu;
class TeQtCheckListItem;
class TeQtDatabaseItem;
class TeQtLayerItem;
class TeLayer;

#include "../../kernel/TeDefines.h"

#include <qlistview.h>
#include <vector>


class TL_DLL TeQtDatabasesListView : public QListView
{
	Q_OBJECT

public:
    TeQtDatabasesListView(QWidget *parent = 0, const char *name = 0);

	~TeQtDatabasesListView();

	TeQtDatabaseItem* currentDatabaseItem() 
		{ return currentDatabaseItem_; }

	TeQtLayerItem* currentLayerItem() 
		{ return currentLayerItem_; }

	TeQtCheckListItem *popupItem()
		{ return popupItem_; }

    void setPopupItem(TeQtCheckListItem* item)
        { popupItem_ = item; }

	QPopupMenu* getPopupMenu()
		{ return popupMenu_; }

	void selectItem(QListViewItem *item);

	void selectLayerItem(TeLayer *layer);

	void removeItem(QListViewItem *item);

	std::vector<TeQtDatabaseItem*>  getDatabaseItemVec();


signals:
	void itemChanged(QListViewItem*);
	void popupSignal();
	
	/** @brief This signal will be emitted after a database item remotion 
	  * @note Signals will be emitted for each children layer item removed */
	void databaseItemRemoved();
	
	/** @brief This signal will be emitted after a layer item remotion */
	void layerItemRemoved();

protected slots:
	virtual void clicked(QListViewItem *item);
	void contextMenuRequested(QListViewItem*, const QPoint&, int);

protected:
	TeQtDatabaseItem	*currentDatabaseItem_;
	TeQtLayerItem		*currentLayerItem_;
	bool				rightMouseWasClicked_;
	TeQtCheckListItem	*popupItem_;
	QPopupMenu			*popupMenu_;

	void contentsMousePressEvent(QMouseEvent *e);

	void unselectAllItems();
};


#endif

