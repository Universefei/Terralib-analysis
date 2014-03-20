/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright ? 2001-2007 INPE and Tecgraf/PUC-Rio.
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

#include "TeQtViewsListView.h"
#include "TeQtCheckListItem.h"
#include "TeQtViewItem.h"
#include "TeQtThemeItem.h"
#include "TeAppTheme.h"
#include "TeWaitCursor.h"
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qdragobject.h>
#include <list>

TeQtViewsListView::TeQtViewsListView(QWidget *parent, const char *name)
		: QListView(parent, name)
{
	pressedThemeItem_ = 0;
	currentViewItem_ = 0;
	currentThemeItem_ = 0;
	refThemeItemForVis_ = 0;
	rightMouseWasClicked_ = false;
	checkItemStateChanged_ = false;
	leftButtonPressed_ = false;
	drag_ = false;

	setSelectionMode(QListView::Multi);
    setRootIsDecorated( TRUE );
//	setAcceptDrops(true);
	viewport()->setAcceptDrops(true);

	popupMenu_ = new QPopupMenu(this);

	//Mounting the brushStyleMap
	brushStyleMap_[TePolyTypeTransparent] = Qt::NoBrush;
	brushStyleMap_[TePolyTypeFill] = Qt::SolidPattern;
	brushStyleMap_[TePolyTypeHorizontal] = Qt::HorPattern;
	brushStyleMap_[TePolyTypeVertical] = Qt::VerPattern;
	brushStyleMap_[TePolyTypeFDiagonal] = Qt::FDiagPattern;
	brushStyleMap_[TePolyTypeBDiagonal] = Qt::BDiagPattern;
	brushStyleMap_[TePolyTypeCross] = Qt::CrossPattern;
	brushStyleMap_[TePolyTypeDiagonalCross] = Qt::DiagCrossPattern;

	//Mounting the penStyleMap
	penStyleMap_[TeLnTypeContinuous] = Qt::SolidLine;
	penStyleMap_[TeLnTypeDashed] = Qt::DashLine;
	penStyleMap_[TeLnTypeDotted] = Qt::DotLine;
	penStyleMap_[TeLnTypeDashDot] = Qt::DashDotLine;
	penStyleMap_[TeLnTypeDashDotDot] = Qt::DashDotDotLine;
	penStyleMap_[TeLnTypeNone] = Qt::NoPen;

	// rename action
	setDefaultRenameAction(QListView::Accept);

	connect(this,SIGNAL(clicked(QListViewItem*)),
			this,SLOT(clicked(QListViewItem*)));
	connect(this,SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&, int)),
		    this,SLOT(contextMenuRequested(QListViewItem*,const QPoint&, int)));
}


TeQtViewsListView::~TeQtViewsListView()
{
	if (popupMenu_)
		delete popupMenu_;
}


void TeQtViewsListView::selectItem(QListViewItem *item)
{
	checkItemStateChanged_ = false;
	rightMouseWasClicked_ = false;
	clicked(item);
}


void TeQtViewsListView::selectViewItem(TeView *view)
{
	if (view == 0)
		return;

	QListViewItem *child = firstChild();
    while (child)
	{
		TeQtViewItem *viewItem = (TeQtViewItem*)child;
		if (viewItem->getView() == view)
		{
			if (viewItem == currentViewItem_)
				break;
			else
			{
				unselectAllItems();
				child->setSelected(true);
				child->repaint();
				currentViewItem_ = viewItem;
				currentThemeItem_ = 0;
				emit itemChanged(child);
				return;
			}
		}
        child = child->nextSibling();
	}
}


TeQtViewItem* TeQtViewsListView::getViewItem(TeView *view)
{
	if (view == 0)
		return 0;

	QListViewItem *child = firstChild();
    while (child)
	{
		TeQtViewItem *viewItem = (TeQtViewItem*)child;
		if (viewItem->getView() == view)
			return viewItem;
		else
			child = child->nextSibling();
	}
	return 0;
}

TeQtViewItem* TeQtViewsListView::getViewItem(TeQtThemeItem *themeItem)
{
	if (themeItem == 0)
		return 0;

	TeQtViewItem *viewItem = (TeQtViewItem*)themeItem->parent();

	return viewItem;
}


TeQtThemeItem* TeQtViewsListView::getThemeItem(TeAppTheme* appTheme)
{
	if (appTheme == 0)
		return 0;

	QListViewItem *item;
	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem* themeItem = (TeQtThemeItem*)item;
			if (themeItem->getAppTheme() == appTheme)
					return themeItem;
		}
		it++;
		item = it.current();
	}
	return 0;
}

TeQtThemeItem* TeQtViewsListView::getThemeItem(int themeId)
{
	QListViewItem *item;
	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem* themeItem = (TeQtThemeItem*)item;
			if (themeItem->getAppTheme()->getTheme()->id() == themeId)
					return themeItem;
		}
		it++;
		item = it.current();
	}
	return 0;
}


void TeQtViewsListView::removeViewItem(TeView *view)
{
	QListViewItem *item;

	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::VIEW)
		{
			TeQtViewItem* viewItem = (TeQtViewItem*)item;
			if (viewItem->getView() == view)
			{
				if (viewItem == currentViewItem_)
					currentViewItem_ = 0;
				removeItem(item);
				break;
			}
		}
		it++;
		item = it.current();
	}	
}


void TeQtViewsListView::removeThemeItem(TeAppTheme* appTheme)
{
	QListViewItem *item;

	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem* themeItem = (TeQtThemeItem*)item;
			if (themeItem->getAppTheme() == appTheme)
			{
				if (themeItem == currentThemeItem_)
					currentThemeItem_ = 0;
				removeItem(item);
				break;
			}
		}
		it++;
		item = it.current();
	}	
}

void TeQtViewsListView::removeThemeItem(int themeId)
{
	QListViewItem *item;

	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem* themeItem = (TeQtThemeItem*)item;
			if (themeItem->getAppTheme()->getTheme()->id() == themeId)
			{
				if (themeItem == currentThemeItem_)
					currentThemeItem_ = 0;
				removeItem(item);
				break;
			}
		}
		it++;
		item = it.current();
	}	
}

void TeQtViewsListView::removeItem(QListViewItem *item)
{
	if (item == 0)
		return;

	TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;

	if(checkListItem->getType() == TeQtCheckListItem::VIEW)
	{
		TeQtViewItem* viewItem = (TeQtViewItem*)item;

		vector<QListViewItem*> childrenVector;
		childrenVector = viewItem->getChildren();

		int i;
		TeQtThemeItem *themeItem;
		for (i = 0; i < (int)childrenVector.size(); ++i)
		{
			themeItem = (TeQtThemeItem*)childrenVector[i];
			delete themeItem;
		}

		if (currentViewItem_ == viewItem)
		{
			currentViewItem_ = 0;
			currentThemeItem_ = 0;
		}
		delete viewItem;
	}
	else if (checkListItem->getType() == TeQtCheckListItem::THEME)
	{
		TeQtThemeItem* themeItem = (TeQtThemeItem*)item;
		if (currentThemeItem_ == themeItem)
			currentThemeItem_ = 0;
		delete themeItem;
	}
	else
		delete item;
}


void TeQtViewsListView::checkItemStateChanged(QCheckListItem *checkListItem, bool visible)
{
	checkItemStateChanged_ = true;
	emit checkListItemVisChanged(checkListItem, visible);
}


void TeQtViewsListView::openViewItems()
{
	QListViewItem *child = firstChild();
	while (child)
	{
		setOpen(child, true);
		child = child->nextSibling();
	}
}


void TeQtViewsListView::reset()
{
	unselectAllItems();
	currentViewItem_ = 0;
	currentThemeItem_ = 0;
}


void TeQtViewsListView::setOn(QCheckListItem *item, bool visible)
{
	TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;
	if (checkListItem->getType() == TeQtCheckListItem::THEME)
	{
		TeQtThemeItem *themeItem = (TeQtThemeItem*)item;
		themeItem->setOn(visible);
		checkItemStateChanged_ = false;
		emit checkListItemVisChanged(checkListItem, visible);
	}
}


void TeQtViewsListView::clicked(QListViewItem *item)
{
	if(item == 0)
		return;
	if(drag_ && pressedThemeItem_)
		return;

	unselectAllItems();

	TeQtCheckListItem *checkListItem = (TeQtCheckListItem*)item;

	if(	checkItemStateChanged_ && (checkListItem->getType() == TeQtCheckListItem::THEME))
	{
		checkItemStateChanged_ = false;
		TeQtThemeItem* themeItem = (TeQtThemeItem*)checkListItem;
		if(themeItem->isThemeItemInvalid())
			themeItem->setOn(false);

		if(currentThemeItem_)
		{
			currentThemeItem_->parent()->setSelected(true);
			currentThemeItem_->parent()->repaint();
			currentThemeItem_->setSelected(true);
			currentThemeItem_->repaint();
			return;
		}
	}

	if(checkListItem->getType() != TeQtCheckListItem::THEME)
	{
		if (rightMouseWasClicked_ == true || checkItemStateChanged_ == true ||
			drag_ == true || checkListItem->getType() == TeQtCheckListItem::LEGEND ||
			checkListItem->getType() == TeQtCheckListItem::CHART)
		{
			if (checkListItem->getType() == TeQtCheckListItem::LEGEND ||
				checkListItem->getType() == TeQtCheckListItem::CHART)
			{
				checkListItem->setSelected(false);
				checkListItem->repaint();
			}

			if (currentViewItem_ != 0)
			{
				currentViewItem_->setSelected(true);
				currentViewItem_->repaint();
			}

			if (currentThemeItem_ != 0)
			{
				currentThemeItem_->setSelected(true);
				currentThemeItem_->repaint();
			}
			rightMouseWasClicked_ = false;
			checkItemStateChanged_ = false;
			return;
		}
	}

	if (checkListItem->getType() == TeQtCheckListItem::VIEW)
	{
		item->setSelected(true);
		item->repaint();

		TeQtViewItem *viewItem = (TeQtViewItem*)item;
		if (viewItem != currentViewItem_)
		{
			currentViewItem_ = viewItem;
			currentThemeItem_ = 0;
			emit itemChanged(item);
		}
		else
		{
			if (currentThemeItem_ != 0)
			{
				currentThemeItem_->setSelected(true);
				currentThemeItem_->repaint();
			}
		}
	}
	else if(checkListItem->getType() == TeQtCheckListItem::THEME)
	{
		TeQtThemeItem* themeItem = (TeQtThemeItem*)checkListItem;
		if(themeItem->isThemeItemInvalid())
			return;
		item->parent()->setSelected(true);
		item->parent()->repaint();
		item->setSelected(true);
		item->repaint();

		TeQtViewItem *viewItem = (TeQtViewItem*)item->parent();
		if (currentViewItem_ != viewItem && rightMouseWasClicked_ == false)
		{
			currentViewItem_ = viewItem;
			emit itemChanged(item->parent());
		}

		themeItem = (TeQtThemeItem*)item;
		if (themeItem != currentThemeItem_ && rightMouseWasClicked_ == false)
		{
			currentThemeItem_ = themeItem;
			emit itemChanged(item);
		}
	}
	rightMouseWasClicked_ = false;
	checkItemStateChanged_ = false;
}


void TeQtViewsListView::contextMenuRequested(QListViewItem *item, const QPoint&, int)
{
	popupItem_ = (TeQtCheckListItem*)item;

	unselectAllItems();
	if (currentViewItem_)
	{
		currentViewItem_->setSelected(true);
		currentViewItem_->repaint();
	}

	if (currentThemeItem_)
	{
		currentThemeItem_->setSelected(true);
		currentThemeItem_->repaint();
	}

	emit popupSignal();	
}

void TeQtViewsListView::contentsMousePressEvent(QMouseEvent *e)
{
	rightMouseWasClicked_ = false;;
	if (e->button() == RightButton)
	{
		rightMouseWasClicked_ = true;
		popupMenu_->move(e->globalPos().x(), e->globalPos().y());
	}
	else if (e->button() == LeftButton)
	{
		QPoint p( contentsToViewport( e->pos() ) );
		pressedPosition_ = p;
		TeQtCheckListItem *item = (TeQtCheckListItem*)itemAt(p);
		if (item != 0)
		{
			if(item->getType() == TeQtCheckListItem::THEME)
			{
				// don?t drag, if the user clicked into the root decoration of the item
				int x = p.x();
				int len = treeStepSize() * (item->depth() + (rootIsDecorated() ? 1 : 0)) + itemMargin();
				if (x > len)
//				if (p.x() > treeStepSize() * (item->depth() + (rootIsDecorated() ? 1 : 0)) + itemMargin())
				{
					pressedThemeItem_ = (TeQtThemeItem*)item;
					themeOpen_ = isOpen(item);
				}
	
				// Item was pressed inside the visibility box
				if (x > len && x < len + 10)
				{
					if (e->state() & Qt::ShiftButton)
					{
						if (refThemeItemForVis_ == 0)
							refThemeItemForVis_ = (TeQtThemeItem*)item;
						else
						{
							TeQtThemeItem *themeItem = (TeQtThemeItem*)item;
							if ((themeItem == refThemeItemForVis_) ||
								(themeItem->parent() != refThemeItemForVis_->parent()))
								return;

							list<TeQtCheckListItem*> themeItemList;
							list<TeQtCheckListItem*>::iterator it;
							// find if the item pressed is below the reference item
							TeQtCheckListItem *itemBelow = (TeQtCheckListItem*)refThemeItemForVis_->itemBelow();
							while((itemBelow != 0) && (itemBelow->getType() == TeQtCheckListItem::THEME))
							{
								themeItemList.push_back(itemBelow);
								if (itemBelow == themeItem)
								{
									for (it = themeItemList.begin(); it != themeItemList.end(); ++it)
									{
										TeQtCheckListItem *thItem = *it;
										bool visible = refThemeItemForVis_->isOn();
										setOn(thItem, visible);
									}
									refThemeItemForVis_ = 0;
									return;
								}
								itemBelow = (TeQtCheckListItem*)itemBelow->itemBelow();
							}

							// find if the item pressed is above the reference item
							themeItemList.clear();
							TeQtCheckListItem *itemAbove = (TeQtCheckListItem*)refThemeItemForVis_->itemAbove();
							while((itemAbove != 0) && (itemAbove->getType() == TeQtCheckListItem::THEME))
							{
								themeItemList.push_back(itemAbove);
								if (itemAbove == themeItem)
								{
									for (it = themeItemList.begin(); it != themeItemList.end(); ++it)
									{
										TeQtCheckListItem* thItem = *it;
										bool visible = refThemeItemForVis_->isOn();
										setOn(thItem, visible);
									}
									refThemeItemForVis_ = 0;
									return;
								}
								itemAbove = (TeQtCheckListItem*)itemAbove->itemAbove();
							}
						}
					}
					else
						refThemeItemForVis_ = 0;
				}
			}
			else
			{
				item->setSelected(false);
				item->repaint();
				pressedThemeItem_ = 0;
			}
			leftButtonPressed_ = true;
		}
	}

	QListView::contentsMousePressEvent(e);
}


void TeQtViewsListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
	leftButtonPressed_ = false;
	QListView::contentsMouseReleaseEvent(e);

	if(currentThemeItem_)
	{
		unselectAllItems();
		currentThemeItem_->parent()->setSelected(true);
		currentThemeItem_->parent()->repaint();
		currentThemeItem_->setSelected(true);
		repaint();
	}
	drag_ = false;
}


void TeQtViewsListView::unselectAllItems()
{
	QListViewItem *item;

	QListViewItemIterator it(this);
	item = it.current();
	while(item)
	{
		setSelected(item,false);
		item->repaint();
		it++;
		item = it.current();
	}
}


void TeQtViewsListView::contentsMouseMoveEvent(QMouseEvent* e)
{
	QPoint p( contentsToViewport( e->pos() ) );
    if (leftButtonPressed_)
	{
		QPoint pdif = pressedPosition_ - p;
		if(abs(pdif.x()) > 2 || abs(pdif.y()) > 2)
		{
			drag_ = true;
			leftButtonPressed_ = false;
			if(pressedThemeItem_)
			{
				TeQtCheckListItem *item = (TeQtCheckListItem*)itemAt(p);
				if (item && item->getType() == TeQtCheckListItem::THEME)
				{
	//				clicked(item);
					QUriDrag* ud = new QUriDrag(viewport());
					ud->drag();
				}
			}
		}
    }
	if(drag_ && pressedThemeItem_)
		pressedThemeItem_->setOpen(themeOpen_);
}


void TeQtViewsListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
	if(pressedThemeItem_)
		pressedThemeItem_->setOpen(themeOpen_);

	if(e->source() != viewport())
	{
		e->ignore();
		e->acceptAction(false);
		return;
	}

    QPoint p = contentsToViewport(((QDragMoveEvent*)e)->pos());
    TeQtCheckListItem *item = (TeQtCheckListItem*)itemAt(p);
    if (item)
	{
		if (p.x() < treeStepSize() * (item->depth() + (rootIsDecorated() ? 1 : 0)) + itemMargin())
		{
			e->ignore();
			e->acceptAction(false);
		}
		else
		{
			if(item->getType() == TeQtCheckListItem::THEME || item->getType() == TeQtCheckListItem::VIEW)
			{
				e->accept();
				e->acceptAction();
			}
			else
			{
				e->ignore();
				e->acceptAction(false);
				if(currentThemeItem_)
				{
					unselectAllItems();
					currentThemeItem_->parent()->setSelected(true);
					currentThemeItem_->parent()->repaint();
					currentThemeItem_->setSelected(true);
					repaint();
				}
			}
		}
    }
	else
	{
		e->ignore();
		e->acceptAction(false);
	}
}


void TeQtViewsListView::contentsDropEvent(QDropEvent *e)
{
	if(e->source() != viewport())
	{
		e->ignore();
		e->acceptAction(false);
		pressedThemeItem_ = 0;
		if(currentThemeItem_)
		{
			unselectAllItems();
			currentThemeItem_->parent()->setSelected(true);
			currentThemeItem_->parent()->repaint();
			currentThemeItem_->setSelected(true);
			repaint();
		}
		return;
	}

	int itemPriority;
	TeQtThemeItem *themeItem;
	bool viewChanged = false;

    QPoint p = contentsToViewport(((QDragMoveEvent*)e)->pos());
	TeQtCheckListItem *dropItem = (TeQtCheckListItem*)itemAt(p);

	if (dropItem == 0)
	{
		e->ignore();
		e->acceptAction(false);
		pressedThemeItem_ = 0;
		if(currentThemeItem_)
		{
			unselectAllItems();
			currentThemeItem_->parent()->setSelected(true);
			currentThemeItem_->parent()->repaint();
			currentThemeItem_->setSelected(true);
			repaint();
		}
		return;
	}

	if (dropItem == pressedThemeItem_)
	{
		e->ignore();
		e->acceptAction(false);
		pressedThemeItem_ = 0;
		if(currentThemeItem_)
		{
			unselectAllItems();
			currentThemeItem_->parent()->setSelected(true);
			currentThemeItem_->parent()->repaint();
			currentThemeItem_->setSelected(true);
			repaint();
		}
		return;
	}

	e->accept();
	e->acceptAction();

	//Set the view item of the pressed item and the drop item
	TeQtViewItem *pressedViewItem = (TeQtViewItem*)pressedThemeItem_->parent();
	TeQtViewItem *dropViewItem;
	if(dropItem->getType() == TeQtCheckListItem::THEME)
		dropViewItem = (TeQtViewItem*)dropItem->parent();
	else
		dropViewItem = (TeQtViewItem*)dropItem;

	//set a flag to indicate whether there was a view change
	if (pressedViewItem != dropViewItem)
		viewChanged = true;

	//update the priorities and move the items accordingly
	if (viewChanged)
	{
		vector<QListViewItem*> themeItemVector = dropViewItem->getChildren();
		for (unsigned int i = 0; i < themeItemVector.size(); ++i)
		{
			themeItem = (TeQtThemeItem*)themeItemVector[i];
			if(pressedThemeItem_->text(0) == themeItem->text(0))
			{
				emit dragDropItems(0, 0, 0);
				pressedThemeItem_ = 0;
				return;
			}
		}

		if (dropItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem *dropThemeItem = (TeQtThemeItem*)dropItem;
			itemPriority = dropThemeItem->order();
			pressedThemeItem_->order(itemPriority);
			themeItem = dropThemeItem;
		}
		else
		{
			itemPriority = 0;
			pressedThemeItem_->order(itemPriority);
			themeItem = (TeQtThemeItem*)dropViewItem->firstChild();
		}

		while (themeItem)
		{
			themeItem->order(++itemPriority);
			themeItem = (TeQtThemeItem*)themeItem->nextSibling();
		}
		pressedViewItem->takeItem(pressedThemeItem_);
		dropViewItem->insertItem(pressedThemeItem_);
	}
	else
	{
		if (dropItem->getType() == TeQtCheckListItem::THEME)
		{
			TeQtThemeItem *dropThemeItem = (TeQtThemeItem*)dropItem;
			int pressedItemPriority = pressedThemeItem_->order();
			int droppedItemPriority = dropThemeItem->order();
			if (pressedItemPriority < droppedItemPriority)
			{
				itemPriority = pressedItemPriority;
				pressedThemeItem_->order(droppedItemPriority);
				themeItem = (TeQtThemeItem*)pressedThemeItem_->nextSibling();
				while (themeItem != dropThemeItem)
				{
					themeItem->order(itemPriority++);
					themeItem = (TeQtThemeItem*)themeItem->nextSibling();
				}
				dropThemeItem->order(itemPriority);
			}
			else
			{
				itemPriority = droppedItemPriority;
				pressedThemeItem_->order(itemPriority);
				themeItem = (TeQtThemeItem*)dropThemeItem;

				while (themeItem != pressedThemeItem_)
				{
					themeItem->order(++itemPriority);
					themeItem = (TeQtThemeItem*)themeItem->nextSibling();
				}
			}
			pressedThemeItem_->moveItem(dropThemeItem);
		}
		else
		{
			itemPriority = 0;
			pressedThemeItem_->order(itemPriority);
			themeItem = (TeQtThemeItem*)dropItem->firstChild();
			while (themeItem != pressedThemeItem_)
			{
				themeItem->order(++itemPriority);
				themeItem = (TeQtThemeItem*)themeItem->nextSibling();
			}
			pressedViewItem->takeItem(pressedThemeItem_);
			pressedViewItem->insertItem(pressedThemeItem_);
			dropViewItem->setOpen(true);
		}
	}

	if (pressedViewItem != dropViewItem)
	{
		// the drag and drop theme items belong to different views
		if (pressedThemeItem_ == currentThemeItem_)
		{
			currentViewItem_ = dropViewItem;
			clicked(pressedThemeItem_);
		}
	}

//	emit dragDropItems(pressedViewItem, dropViewItem);
	emit dragDropItems(pressedThemeItem_, pressedViewItem, dropViewItem);
	pressedThemeItem_->setOpen(themeOpen_);

	pressedThemeItem_ = 0;

	if(currentThemeItem_)
	{
		unselectAllItems();
		currentThemeItem_->parent()->setSelected(true);
		currentThemeItem_->parent()->repaint();
		currentThemeItem_->setSelected(true);
		repaint();
	}
}

vector<TeQtViewItem*> TeQtViewsListView::getViewItemVec()
{
	vector<TeQtViewItem*> viewItemVec;

	TeQtCheckListItem*	checkListItem;
	TeQtViewItem*		viewItem;

	QListViewItemIterator it(this);
	while(it.current())
	{
		checkListItem = (TeQtCheckListItem*)it.current();
		if (checkListItem->getType() == TeQtCheckListItem::VIEW)
		{
			viewItem = (TeQtViewItem*)it.current();
			viewItemVec.push_back(viewItem);
		}
		++it;
	}

	return viewItemVec;
}

vector<TeQtThemeItem*> TeQtViewsListView::getThemeItemVec()
{
	vector<TeQtThemeItem*> themeItemVec;

	TeQtCheckListItem*	checkListItem;
	TeQtThemeItem*		themeItem;

	QListViewItemIterator it(this);
	while(it.current())
	{
		checkListItem = (TeQtCheckListItem*)it.current();
		if (checkListItem->getType() == TeQtCheckListItem::THEME)
		{
			themeItem = (TeQtThemeItem*)it.current();
			themeItemVec.push_back(themeItem);
		}
		++it;
	}

	return themeItemVec;
}



