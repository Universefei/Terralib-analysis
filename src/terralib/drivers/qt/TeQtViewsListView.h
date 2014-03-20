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

#ifndef  __TERRALIB_INTERNAL_QTVIEWSLISTVIEW_H
#define  __TERRALIB_INTERNAL_QTVIEWSLISTVIEW_H

#include <qlistview.h>
#include "../../kernel/TeDataTypes.h"

#include "../../kernel/TeDefines.h"
class QPopupMenu;
class TeView;
class TeQtThemeItem;
class TeQtViewItem;
class TeQtCheckListItem;
class TeAppTheme;

class TL_DLL TeQtViewsListView : public QListView
{
	Q_OBJECT

public:

    TeQtViewsListView(QWidget *parent = 0, const char *name = 0);

	~TeQtViewsListView();

	TeQtViewItem* currentViewItem() 
	{ return currentViewItem_; }

	TeQtThemeItem* currentThemeItem() 
	{ return currentThemeItem_; }

	TeQtCheckListItem *popupItem()
		{ return popupItem_; }

    void setPopupItem(TeQtCheckListItem* item)
        { popupItem_ = item; }

	QPopupMenu* getPopupMenu()
		{ return popupMenu_; }

	void selectItem(QListViewItem *item);

	void selectViewItem(TeView *view);

	TeQtViewItem* getViewItem(TeView *view);

	TeQtViewItem* getViewItem(TeQtThemeItem* themeItem);

	TeQtThemeItem* getThemeItem(TeAppTheme* appTheme);

	TeQtThemeItem* getThemeItem(int themeId);

	void removeViewItem(TeView *view);

	void removeThemeItem(TeAppTheme* appTheme);

	void removeThemeItem(int themeId);

	void removeItem(QListViewItem *item);

	void checkItemStateChanged(QCheckListItem*, bool);

	void setOpenViewItem(QCheckListItem *checkListItem, bool visible)
		{ setOpen(checkListItem, visible); }

	void openViewItems();

	void reset();

	void setOn(QCheckListItem*, bool);

	vector<TeQtViewItem*> getViewItemVec();

	vector<TeQtThemeItem*> getThemeItemVec();

	map<TePolyBasicType, Qt::BrushStyle>& getBrushStyleMap() {return brushStyleMap_;}
	map<TeLnBasicType, Qt::PenStyle>& getPenStyleMap() {return penStyleMap_;}

signals:
	void itemChanged(QListViewItem*);
	void popupSignal();
	void checkListItemVisChanged(QCheckListItem*, bool);
	void dragDropItems(TeQtThemeItem*, TeQtViewItem*, TeQtViewItem*);

protected slots:
	void clicked(QListViewItem *item);
	void contextMenuRequested(QListViewItem*, const QPoint&, int);

protected:
	TeQtViewItem		*currentViewItem_;
	TeQtThemeItem		*currentThemeItem_;
	bool				rightMouseWasClicked_;
	bool				checkItemStateChanged_;
	bool				drag_;
	bool				themeOpen_;
	bool				leftButtonPressed_;
	QPoint				pressedPosition_;
	TeQtCheckListItem*	popupItem_;
	QPopupMenu*			popupMenu_;
	map<TePolyBasicType, Qt::BrushStyle>	brushStyleMap_;
	map<TeLnBasicType, Qt::PenStyle>		penStyleMap_;
	TeQtThemeItem		*pressedThemeItem_;
	TeQtCheckListItem	*refThemeItemForVis_;  //reference theme item for visibility of another one

	void contentsMousePressEvent(QMouseEvent *e);

	void contentsMouseReleaseEvent(QMouseEvent *e);

	void contentsMouseMoveEvent(QMouseEvent* e);

	void contentsDragMoveEvent(QDragMoveEvent *e);

	void contentsDropEvent(QDropEvent *e);

	void unselectAllItems();
};


#endif
