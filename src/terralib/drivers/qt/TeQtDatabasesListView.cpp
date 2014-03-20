/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.
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

#include <TeQtDatabasesListView.h>
#include <TeQtCheckListItem.h>
#include <TeQtDatabaseItem.h>
#include <TeQtLayerItem.h>
#include <qpopupmenu.h>


TeQtDatabasesListView::TeQtDatabasesListView(QWidget *parent, const char *name)
		: QListView(parent, name)
{
	currentDatabaseItem_ = 0;
	currentLayerItem_ = 0;
	popupItem_ = 0;
	popupMenu_ = 0;
	rightMouseWasClicked_ = false;

	setSelectionMode(QListView::Multi);
    setRootIsDecorated( TRUE );

	popupMenu_ = new QPopupMenu(this);

	// rename action
	setDefaultRenameAction(QListView::Accept);

	connect(this,SIGNAL(clicked(QListViewItem*)),
			this,SLOT(clicked(QListViewItem*)));
	connect(this,SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&, int)),
		    this,SLOT(contextMenuRequested(QListViewItem*,const QPoint&, int)));
}


TeQtDatabasesListView::~TeQtDatabasesListView()
{
	if (popupMenu_)
		delete popupMenu_;
}


void TeQtDatabasesListView::selectItem(QListViewItem *item)
{
	rightMouseWasClicked_ = false;
	clicked(item);
}


void TeQtDatabasesListView::selectLayerItem(TeLayer *layer)
{
	if (layer == 0)
		currentLayerItem_ = 0;
		//return;

	QListViewItem		*item;
	TeQtCheckListItem	*checkListItem;
	TeQtLayerItem		*layerItem;
	TeQtDatabaseItem	*dbItem;

	unselectAllItems();
	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::INFOLAYER)
		{
			layerItem = (TeQtLayerItem*)item;
			if (layerItem->getLayer() == layer)
			{
				if (layerItem == currentLayerItem_)
					break;
				else
				{
					item->setSelected(true);
					item->repaint();
					currentLayerItem_ = layerItem;

					dbItem = (TeQtDatabaseItem*)item->parent();
					dbItem->setSelected(true);
					dbItem->repaint();
					currentDatabaseItem_ = dbItem;
					emit itemChanged(item);
					return;
				}
			}
		}
		it++;
		item = it.current();
	}
}


void TeQtDatabasesListView::removeItem(QListViewItem *item)
{
	if (item == 0)
		return;

	TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;

	if(checkListItem->getType() == TeQtCheckListItem::DATABASE)
	{
		TeQtDatabaseItem* dbItem = (TeQtDatabaseItem*)item;

		vector<QListViewItem*> childrenVector;
		childrenVector = dbItem->getChildren();

		unsigned int i;
		TeQtLayerItem *layerItem;
		for (i = 0; i < childrenVector.size(); ++i)
		{
			layerItem = (TeQtLayerItem*)childrenVector[i];
			delete layerItem;
			
			emit( layerItemRemoved() );
		}

		if (currentDatabaseItem_ == dbItem)
		{
			currentDatabaseItem_ = 0;
			currentLayerItem_ = 0;
		}
		delete dbItem;
		
		emit( databaseItemRemoved() );
	}
	else
	{
		// item is a layer
		TeQtLayerItem *layerItem = (TeQtLayerItem*)item;
		if (currentLayerItem_ == layerItem)
			currentLayerItem_ = 0;
		delete layerItem;
		
		emit( layerItemRemoved() );
	}
}


void TeQtDatabasesListView::clicked(QListViewItem *item)
{
	unselectAllItems();
	if (item == 0 || rightMouseWasClicked_ == true)
	{
		if (currentDatabaseItem_ != 0)
		{
			currentDatabaseItem_->setSelected(true);
			currentDatabaseItem_->repaint();
		}

		if (currentLayerItem_ != 0)
		{
			currentLayerItem_->setSelected(true);
			currentLayerItem_->repaint();
		}

		rightMouseWasClicked_ = false;
		return;
	}

	TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
	if (checkListItem->getType() == TeQtCheckListItem::DATABASE)
	{
		item->setSelected(true);
		item->repaint();

		TeQtDatabaseItem* dbItem = (TeQtDatabaseItem*)item;
		if (dbItem != currentDatabaseItem_)
		{
			currentDatabaseItem_ = dbItem;
			currentLayerItem_ = 0;
			emit itemChanged(item);
		}
		else
		{
			if (currentLayerItem_ != 0)
			{
				currentLayerItem_->setSelected(true);
				currentLayerItem_->repaint();
			}
		}
	}
	else if(checkListItem->getType() == TeQtCheckListItem::INFOLAYER)
	{
		TeQtDatabaseItem *dbItem = (TeQtDatabaseItem*)item->parent();
		if (currentDatabaseItem_ != dbItem)
		{
			currentDatabaseItem_ = dbItem;
			emit itemChanged(item->parent());
		}

		if (currentLayerItem_ != 0)
		{
			currentLayerItem_->setSelected(true);
			currentLayerItem_->repaint();
		}
	}
}


void TeQtDatabasesListView::contextMenuRequested(QListViewItem *item, const QPoint&, int)
{
	popupItem_ = (TeQtCheckListItem*)item;

	unselectAllItems();
	if (currentDatabaseItem_)
	{
		currentDatabaseItem_->setSelected(true);
		currentDatabaseItem_->repaint();
	}

	if (currentLayerItem_)
	{
		currentLayerItem_->setSelected(true);
		currentLayerItem_->repaint();
	}

	emit popupSignal();
}


void TeQtDatabasesListView::contentsMousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton)
	{
		rightMouseWasClicked_ = true;
		popupMenu_->move(e->globalPos().x(), e->globalPos().y());
	}
	QListView::contentsMousePressEvent(e);
}


void TeQtDatabasesListView::unselectAllItems()
{
	QListViewItem *item;

	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		item->setSelected(false);
		item->repaint();
		it++;
		item = it.current();
	}
}

vector<TeQtDatabaseItem*>  TeQtDatabasesListView::getDatabaseItemVec()
{
	vector<TeQtDatabaseItem*> databaseItemVec;

	TeQtCheckListItem*	checkListItem;
	TeQtDatabaseItem*	dbItem;

	QListViewItemIterator it(this);
	while(it.current())
	{
		checkListItem = (TeQtCheckListItem*)it.current();
		if (checkListItem->getType() == TeQtCheckListItem::DATABASE)
		{
			dbItem = (TeQtDatabaseItem*)it.current();
			databaseItemVec.push_back(dbItem);
		}
		++it;
	}

	return databaseItemVec;
}






