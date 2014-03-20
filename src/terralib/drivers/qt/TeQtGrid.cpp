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
************************************************************************************/
#include "TeQtGrid.h"
#include "TeWaitCursor.h"
#include "TeApplicationUtils.h"
#include "../../kernel/TeExternalTheme.h"
#include <qtable.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <algorithm>

TeQtGrid::TeQtGrid(QWidget* parent, const char* name)
	: QTable(parent, name)
{
	autoPromote_ = true;
	gridX_ = -1;
	gridY_ = -1;
	iLine_ = -1;
	fLine_ = -1;
	iCol_ = -1;
	fCol_ = -1;
	gridWidth_  = -1;
	gridHeight_ = -1;
	portal_ = 0;
	theme_ = NULL;
	doRepaint_ = true;
	doRefresh_ = false;
	shiftKeyPressed_ = false;
	lastLinePressed_ = -1;
	verticalEdition_ = true;
	rowToEdit_ = -1;
	colToEdit_ = -1;
	editing_ = false;
	showGridInformation_ = true;
	columnDragEnabled_ = false;
	mousePressedOnHorizontalHeader_ = false;

	setNumCols(0);
	setNumRows(0);
	setSelectionMode(QTable::Multi);

	popupHorizHeader_ = new QPopupMenu;
	popupVertHeader_ = new QPopupMenu;
	popupViewport_ = new QPopupMenu;

	horizontalHeader()->setAcceptDrops(true);

	verticalHeader()->installEventFilter(this);
	horizontalHeader()->installEventFilter(this);

    connect(verticalHeader(), SIGNAL(released(int)),
	     this, SLOT(mouseReleasedOnVerticalHeaderSlot(int))); // NOTE: QT send the pressed position in your argument

	verticalHeader()->setResizeEnabled(false);
}


TeQtGrid::~TeQtGrid()
{
	if (portal_)
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
	}
	if (popupHorizHeader_)
		delete popupHorizHeader_;
	if (popupVertHeader_)
		delete popupVertHeader_;
	if (popupViewport_)
		delete popupViewport_;
}


void TeQtGrid::init (TeAppTheme *appTheme)
{
	int i, j, numRows, nCols, gs;
	int collectionNumCols = 0;
	char buf[20];
	string oid, uid;
	hide();
	clear();

	if ((appTheme == 0) || (appTheme->getTheme()->type() > 2))
		return;

	TeAttrTableVector attrTables; 
	
	TeTheme* theme = appTheme->getSourceTheme();
	if (theme == 0)
		return;
	theme->getAttTables(attrTables);
	if (attrTables.size() == 0)
		return;

	TeWaitCursor wait;

	theme_ = appTheme;
	CT_ = theme->collectionTable();
	CTE_ = theme->collectionAuxTable();
	db_ = theme->layer()->database();
	sqlJoin_ = theme->sqlGridJoin();

	// Mount the vector of indexes (indexName_) from the tables
	theme->getAttTables(tableVector_);
	for (i = 0; i < (int)tableVector_.size(); ++i)
	{
		TeTable t = tableVector_[i];
		if(t.tableType() == TeAttrStatic || t.tableType() == TeAttrEvent)
		{
			if(tableVector_[i].linkName().empty())
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].uniqueName());
			else
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].linkName());
		}
		else if(t.tableType() != TeAttrExternal)
		{
			if(tableVector_[i].uniqueName().empty() == false)
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].uniqueName());
			if(tableVector_[i].linkName().empty() == false)
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].linkName());
		}
		else
		{
			if(tableVector_[i].uniqueName().empty() == false)
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].uniqueName());
			if(tableVector_[i].linkName().empty() == false)
				indexVector_.push_back(tableVector_[i].name() + "." + tableVector_[i].linkName());
			if(tableVector_[i].relatedAttribute().empty() == false)
				indexVector_.push_back(tableVector_[i].relatedTableName() + "." + tableVector_[i].relatedAttribute());
		}
	}

	TeAttributeList collAttrList;
	db_->getAttributeList(CT_, collAttrList);
	collectionNumCols = collAttrList.size();

	if (portal_)
	{
		portal_->freeResult();
		delete portal_;
	}
	portal_= db_->getPortal();

	//read the objects and their legends
	//from the collection table
	orderBy_ = CTE_ + ".unique_id ";
	sql_ = sqlJoin_ + " ORDER BY " + orderBy_ + "ASC ";
	if (!portal_->query(sql_))
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
		wait.resetWaitCursor();
		return;
	}

	TeAttributeList extAttList;
	db_->getAttributeList(CTE_, extAttList);

	TeAttributeList& attrList = portal_->getAttributeList();
	TeAttributeList::iterator it = attrList.begin();
	int index = 0;
	while(it!=attrList.end())
	{
		if(TeConvertToUpperCase((*it).rep_.name_)== TeConvertToUpperCase(CTE_+".unique_id"))
			uniqueIdPos_ = index;
		else if (TeConvertToUpperCase((*it).rep_.name_)== "UNIQUE_ID")
			uniqueIdPos_ = index;
		else if(TeConvertToUpperCase((*it).rep_.name_)== TeConvertToUpperCase(CTE_+".grid_status"))
			gridStatusPos_ = index;
		else if(TeConvertToUpperCase((*it).rep_.name_)== "GRID_STATUS")
			gridStatusPos_ = index;
		++it;
		++index;
	}

	objectIdPos_ = attrList.size() - extAttList.size() - 6;
	resultIdPos_ = attrList.size() - extAttList.size() - 1;
	i = 0;
	int status;
	while (portal_->fetchRow())
	{
		uid = portal_->getData(uniqueIdPos_);
		oid = portal_->getData(objectIdPos_);

		//if(theme->getProductId() != TeEXTERNALTHEME)
		if(appTheme->getTheme()->getProductId() == TeTHEME)
		{
			gs = atoi(portal_->getData(gridStatusPos_));
			status = portal_->getInt(resultIdPos_);
		}
		else if(appTheme->getTheme()->getProductId() == TeEXTERNALTHEME)
		{
			std::map<std::string, int >::iterator it = theme_->getTheme()->getItemStatusMap().find(uid);
			if( it != theme_->getTheme()->getItemStatusMap().end())
				gs = it->second; 
			else
				gs = 0;

			//std::map<std::string, int>::iterator it2 = theme_->getTheme()->getObjLegendMap().find(oid);
			//if( it2 != theme_->getTheme()->getObjLegendMap().end())
			std::map<std::string, int>::iterator it2 = theme_->getTheme()->getObjStatusMap().find(oid);
			if( it2 != theme_->getTheme()->getObjStatusMap().end())
				status = it->second;
			else
				status = 0;
		}
	
		if(uid.empty())
		{
			unique2ObjectMap_[oid] = oid;
			ObjectInfo	info;
			info.status_ = status;
			objectMap_[oid] = info;
			portalRow2UniqueIdVector_.push_back(oid);
		}
		else
		{
			unique2ObjectMap_[uid] = oid;

			if(objectMap_.find(oid) == objectMap_.end())
			{
				ObjectInfo	info;
				info.uniqueMap_[uid] = gs;
				info.status_ = status;
				objectMap_[oid] = info;
			}
			else
			{
				objectMap_[oid].uniqueMap_[uid] = gs;

				if(objectMap_[oid].status_ == 0 && gs > 0)
					objectMap_[oid].status_ = gs;
				else if(objectMap_[oid].status_ == 1 && gs == 2)
					objectMap_[oid].status_ = 3;
				else if(objectMap_[oid].status_ == 2 && gs == 1)
					objectMap_[oid].status_ = 3;
			}
			portalRow2UniqueIdVector_.push_back(uid);
		}
		gridToPortalRowVector_.push_back(i);
		++i;
	}
	numRows = i;
	
	//get the column names

	nCols = theme->sqlAttList().size();
	vector<string> colNamesVector;
	for (j = 0; j < nCols; ++j)
		colNamesVector.push_back(theme->sqlAttList()[j].rep_.name_);

	doRepaint_ = false; // don�t repaint
	setNumRows(numRows);
	setNumCols(nCols);

	for (i = 0; i < nCols; ++i)
		horizontalHeader()->setLabel(i, colNamesVector[i].c_str());

	// Set the vector that will contain the order the
	// grid columns will be shown
	for (i = 0; i < nCols; i++)
	{
		gridToPortalColVector_.push_back(i);
		gridColVisVector_.push_back(true);
	}

	// Set the minimum width of the vertical header of the grid
	sprintf(buf, "%d", numRows);
	int minw = strlen(buf) + 1;
	if (minw < 3)
		minw = 3;
	verticalHeader()->setMinimumWidth(minw*9); // supposing char width of 9 pixels
	setLeftMargin(minw*9);


	doRepaint_ = true; // do repaint
	doRefresh_ = true; // do refresh

	refresh();
	int visibleCols = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=0; i<numCols(); i++)
	{
		if(visibleWidth() < columnPos(i) - scVal)
			break;
		visibleCols++;
	}

	for (i = 0; i < visibleCols; ++i)
		adjustColumn(i);
	refresh();
	emit gridChangedSignal();
	show();
	gridInformation();
}


void TeQtGrid::initPortal()
{
	TeWaitCursor wait;

	int i, j;
	int nCols = numCols();

	doRepaint_ = false; // do not repaint

	for (j = 0; j < nCols; ++j)
		horizontalHeader()->removeLabel(j);

	TeAttributeList collAttrList;
	db_->getAttributeList(CT_, collAttrList);

	portal_= db_->getPortal();

	//read the objects and their legends
	//from the collection table
	if (!portal_->query(sql_))
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
		wait.resetWaitCursor();
		return;
	}

	TeAttrTableVector cTableVec = tableVector_;
	tableVector_.clear();
	
	TeTheme* theme = theme_->getSourceTheme();
	if (theme == 0)
		return;
	theme->getAttTables(tableVector_);

	TeAttributeList extAttList;
	db_->getAttributeList(CTE_, extAttList);

	TeAttributeList& attrList = portal_->getAttributeList();
	uniqueIdPos_ = attrList.size() - 2;
	objectIdPos_ = attrList.size() - extAttList.size() - 6;
	gridStatusPos_ = attrList.size() - 1;
	resultIdPos_ = attrList.size() - extAttList.size() - 1;
	
	//get the column names
	nCols = theme->sqlAttList().size();
	vector<string> colNamesVector;
	for (j = 0; j < nCols; ++j)
		colNamesVector.push_back(theme->sqlAttList()[j].rep_.name_);

	setNumCols(nCols);

	arrangeColumns(cTableVec);	
	for(i=0; (unsigned int)i<gridColVisVector_.size(); i++)
		showColumn(i);

	for (i = 0; i < nCols; ++i)
	{
		j = gridToPortalColVector_[i];
		horizontalHeader()->setLabel(i, colNamesVector[j].c_str());
	}
	doRepaint_ = true; // do repaint
	doRefresh_ = true; // do refresh

	for(i=0; (unsigned int)i<gridColVisVector_.size(); i++)
	{
		bool b = gridColVisVector_[i];
		if(b == false)
			hideColumn(i);
	}
	refresh();
	int visibleCols = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=0; i<numCols(); i++)
	{
		if(visibleWidth() < columnPos(i) - scVal)
			break;
		visibleCols++;
	}

	for (i = 0; i < visibleCols; ++i)
		adjustColumn(i);
	refresh();
	emit gridChangedSignal();
	show();
	gridInformation();
}

void TeQtGrid::updatePortalContents()
{
	TeWaitCursor wait;

	if (portal_ == 0)
	{
		wait.resetWaitCursor();
		return;
	}

	portal_->freeResult();

	if (!portal_->query(sql_))
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
	}
}

void TeQtGrid::clear()
{
	gridX_ = -1;
	gridY_ = -1;
	iLine_ = -1;
	fLine_ = -1;
	iCol_ = -1;
	fCol_ = -1;
	gridWidth_  = -1;
	gridHeight_ = -1;
	rowToEdit_ = -1;
	colToEdit_ = -1;

	shiftKeyPressed_ = false;
	lastLinePressed_ = -1;

	gridToPortalRowVector_.clear();
	gridToPortalColVector_.clear();
	objectMap_.clear();
	unique2ObjectMap_.clear();
	portalRow2UniqueIdVector_.clear();
	gridColVisVector_.clear();
	indexVector_.clear();
	tableVector_.clear();

	clearPortal();
//	if (portal_)
//	{
//		portal_->freeResult();
//		delete portal_;
//		portal_ = 0;
//	}
	setNumRows(0);
	setNumCols(0);
}

void TeQtGrid::clearPortal()
{
	if (portal_)
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
	}
}


bool TeQtGrid::eventFilter(QObject *o, QEvent *e)
{
	if (doRepaint_ == false) // do not repaint
		return true; // stop event
	static int stx, sty;

	if(o == horizontalHeader())
	{
		QMouseEvent *m = (QMouseEvent *)e;
		int gridCol = columnAt(m->x() + contentsX());
		if(portal_ && gridCol != -1)
		{
			TeTable table = findTable(gridCol);
			QString msg = tr("Table:") + " ";
			msg += table.name().c_str();
			if(table.tableType() == TeAttrStatic)
				msg += " - " + tr("Table type: TeAttrStatic");
			else if(table.tableType() == TeAttrExternal)
				msg += "  " + tr("Table type: TeAttrExternal");
			else if(table.tableType() == TeAttrEvent)
				msg += " - " + tr("Table type: TeAttrEvent");
			else if(table.tableType() == TeFixedGeomDynAttr)
				msg += " - " + tr("Table type: TeFixedGeomDynAttr");
			else if(table.tableType() == TeDynGeomDynAttr)
				msg += " - " + tr("Table type: TeDynGeomDynAttr");

			TeAttributeList& AL = portal_->getAttributeList();
			int pcol = gridToPortalColVector_[gridCol];
			if(AL.size() > (unsigned int)pcol)
			{
				msg += " - " + tr("Column:");
				msg += " ";
				msg += AL[pcol].rep_.name_.c_str();
				if(AL[pcol].rep_.type_ == TeSTRING)
				{
					msg += " - " + tr("Column type: TeSTRING");
					msg += " - " + tr("Num char:");
					msg += " ";
					msg += Te2String(AL[pcol].rep_.numChar_).c_str();
				}
				else if(AL[pcol].rep_.type_ == TeREAL)
					msg += "  " + tr("Column type: TeREAL");
				else if(AL[pcol].rep_.type_ == TeINT)
					msg += " - " + tr("Column type: TeINT");
				else if(AL[pcol].rep_.type_ == TeDATETIME)
					msg += " - " + tr("Column type: TeDATETIME");
				if(isIndex(pcol))
					msg += " - " + tr("Column is not editable");
				else
					msg += " - " + tr("Column is editable");
			}

			displayGridInformation(msg);
		}
		
		if (e->type() == QEvent::MouseButtonPress)
		{
			stx = sty = -1;
			if (m->button() == RightButton)
			{
				if(gridCol < 0)
					return false;

				int portalColToPopup = gridToPortalColVector_[gridCol];
				popupHorizHeader_->move(m->globalPos().x(), m->globalPos().y());
				emit horizHeaderSignal(portalColToPopup);
				return true;
			}
			if (m->button() == LeftButton)
			{
				stx = contentsX();
				sty = contentsY();
				if(sty != 0)
					mousePressedOnHorizontalHeader_ = true;
				int p = horizontalHeader()->sectionPos(gridCol);
				int ss = horizontalHeader()->sectionSize(gridCol);
				int x = m->x() + horizontalHeader()->offset();
				if(abs(x-p) <= 3 || abs(x-(p+ss)) <= 3)
					return false;
				columnDragEnabled_ = true;
				return false;
			}
		}
		else if(e->type() == QEvent::DragEnter) {
			QDragEnterEvent *dg = (QDragEnterEvent *)e;
			dg->accept(QTextDrag::canDecode(dg));
			return true;
		}
		else if(e->type() == QEvent::DragMove) {
			return true;
		}
		else if(e->type() == QEvent::MouseMove) {
			if (columnDragEnabled_)
			{
				columnDragEnabled_ = false;
				clearSelection(true);
				setSelectionMode(QTable::NoSelection);
				string t = "swap:" + Te2String(gridCol);
				QTextDrag* tDrag = new QTextDrag(t.c_str(), this, "gridswap");
				tDrag->drag();
			}
			return false;
		}
		else if(e->type() == QEvent::Drop) {
			QDragEnterEvent *dg = (QDragEnterEvent *)e;
			dg->accept(QTextDrag::canDecode(dg));
			QDropEvent *dp = (QDropEvent *)e;

			QString tx;
			string dragText;
			if(QTextDrag::decode(dp, tx))
				dragText = tx.latin1();

			string fName = dragText;
			int f = fName.find("-fieldName:");
			if(f >= 0)
			{
				fName = fName.substr(f+strlen("-fieldName:"));

				string table = dragText;
				table = table.substr(0, f);
				f = table.find("table:");
				if(f == 0)
					table = table.substr(strlen("table:"));

				int col = columnAt(dg->pos().x() + contentsX());
				string dropName;
				if (col >= 0)				
					dropName = horizontalHeader()->label(col).latin1();
				emit linkTable(table, fName, dropName);
			}
			else // swap column
			{
//				f = fName.find("swap:", string::npos);
				f = fName.find("swap:");
				if(f >= 0)
				{
					TeWaitCursor wait;
					string s = fName;
					s = s.substr(strlen("swap:"));
					int col1 = atoi(s.c_str());
					int col2 = columnAt(dg->pos().x() + contentsX());
					if(col1 < col2)
						for(int i=col1; i<col2; i++)
							swapColumns(i, i+1);
					else
						for(int i=col1; i>col2; i--)
							swapColumns(i, i-1);
					int w1 = columnWidth(col1);
					int w2 = columnWidth(col2);
					setColumnWidth(col1, w2);
					setColumnWidth(col2, w1);
					refresh();
					QPoint p = dg->pos();
					QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::LeftButton);
					QApplication::postEvent(horizontalHeader(), e);
				}
			}
			return true;
		}
		else if (e->type() == QEvent::MouseButtonRelease)
		{
			columnDragEnabled_ = false;
			setSelectionMode(QTable::Multi);
			if(stx > -1 && sty > -1)
				setContentsPos(stx, sty);
			return false;
		}
	}
	else if(o == verticalHeader())
	{
		if (e->type() == QEvent::MouseButtonDblClick)
		{
			QMouseEvent *m = (QMouseEvent *)e;
			if (m->button() == LeftButton)
			{
				int lin = rowAt(m->y() + contentsY());
				if (lin >= 0)
				{
					emit vertHeaderLeftDblClickSignal(lin);
				}
			}
		}
		else if (e->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent *m = (QMouseEvent *)e;
			if (m->button() == LeftButton)
			{
				stx = contentsX();
				sty = contentsY();
				return false;
			}
			else if (m->button() == RightButton)
			{
				int lin = rowAt(m->y() + contentsY());
				if (lin < 0)
					return false;

				popupVertHeader_->move(m->globalPos().x(), m->globalPos().y());
				emit vertHeaderSignal(lin);
				return true;
			}
		}
		else if (e->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent *m = (QMouseEvent *)e;
			if (m->button() == LeftButton)
			{
				if (m->state() & Qt::ShiftButton)
					shiftKeyPressed_ = true;
				else
					shiftKeyPressed_ = false;
				int lin = m->y();
				dragLineReleased_ = rowAt(lin + contentsY());
				setContentsPos(stx, sty);
			}
			return false;
		}
	}
	else if (o == viewport())
	{
		if (e->type() == QEvent::MouseButtonPress)
		{
			if(rowToEdit_ > -1 && colToEdit_ > -1)
				endEdit(rowToEdit_, colToEdit_, true, true);
			rowToEdit_ = -1;
			colToEdit_ = -1;
			editing_ = false;

			QMouseEvent *m = (QMouseEvent *)e;
			if (m->button() == RightButton)
			{
				popupViewport_->move(m->globalPos().x(), m->globalPos().y());
				emit viewportSignal();
				return true;
			}
		}
		else if (e->type() == QEvent::MouseButtonDblClick)
		{
			if(theme_->getTheme()->type() != TeTHEME)
				return true;

			QMouseEvent *m = (QMouseEvent *)e;
			if (m->button() == LeftButton)
			{
				int col = gridToPortalColVector_[currentColumn()];
				if(isIndex(col))
					return true;

				refresh();
				setEditMode(QTable::Editing, currentRow(), currentColumn());
				beginEdit(currEditRow(), currEditCol(), false);
				rowToEdit_ = currEditRow();
				colToEdit_ = currEditCol();
				return false;
			}
		}
		else if (e->type() == QEvent::Enter)
			gridInformation();
	}
	else
	{
		if (e->type() == QEvent::KeyPress)
		{
			if(rowToEdit_ == -1 || colToEdit_ == -1)
				return true;

			QKeyEvent *k = (QKeyEvent*)e;
			if (k->state() & Qt::ControlButton)
			{
				if(k->key() == Qt::Key_PageDown)
				{
					scrollToNextPointedObjectSlot();
					return true;
				}
				else if(k->key() == Qt::Key_PageUp)
				{
					scrollToPrevPointedObjectSlot();
					return true;
				}
			}
#ifdef WIN32
			else if (k->state() & Qt::AltButton)
#else
			else if (k->state() & (Qt::AltButton | Qt::ControlButton))
#endif
			{
				if(k->key() == Qt::Key_PageDown)
				{
					scrollToNextQueriedObjectSlot();
					return true;
				}
				else if(k->key() == Qt::Key_PageUp)
				{
					scrollToPrevQueriedObjectSlot();
					return true;
				}
			}
			else if (k->key() == Qt::Key_Return)
			{
				editing_ = true;
				endEdit(rowToEdit_, colToEdit_, true, true);
				goToEditNewCell();
				return true;
			}
		}
	}

	return QTable::eventFilter(o, e);
}


void TeQtGrid::mouseReleasedOnVerticalHeaderSlot(int lin)     
{
	TeWaitCursor wait;
	string uid, oid;
	int i, begin, end;
	set<string> drawSet;
	
	if (shiftKeyPressed_ == false) // single toggle
	{
		portal_->fetchRow(gridToPortalRowVector_[lin]);
		uid = portal_->getData(uniqueIdPos_);
		oid = portal_->getData(objectIdPos_);
		drawSet.insert(oid);
		ObjectInfo& info = objectMap_[oid];
		changeObjectStatus(info, uid, "pointing");
	}
	else	// set or reset grid lines (shift and pointing operation)
	{
		if (lastLinePressed_ != -1)
		{
			portal_->fetchRow(gridToPortalRowVector_[lastLinePressed_]);
			uid = portal_->getData(uniqueIdPos_);
			oid = portal_->getData(objectIdPos_);
			ObjectInfo& info = objectMap_[oid];
			int pointed = info.status_  & 0x1;
			if(uid.empty() == false)
				pointed = info.uniqueMap_[uid] & 0x1;

			if (lin > lastLinePressed_)
			{
				begin = lastLinePressed_ + 1;
				end = lin;
			}
			else if (lin < lastLinePressed_)
			{
				begin = lin;
				end = lastLinePressed_ - 1;
			}
			else
				begin = end = lin;

			for (i = begin ; i <= end; ++i)
			{
				portal_->fetchRow(gridToPortalRowVector_[i]);
				uid = portal_->getData(uniqueIdPos_);
				oid = portal_->getData(objectIdPos_);
				ObjectInfo& info = objectMap_[oid];
				changeObjectStatus(info, uid, "shiftAndPointing", pointed);
				drawSet.insert(oid);
			}
		}
	}

	if (lin != dragLineReleased_)
	{
		if (dragLineReleased_ == -1)
		{
			int ini, fim;
			for (ini = iLine_; ini <= fLine_; ini++)
				if (isRowSelected(ini, true))
					break;

			for (fim = fLine_; fim >= iLine_; fim--)
				if (isRowSelected(fim, true))
					break;

			if (fim > lin)
				dragLineReleased_ = fim;
			else
				dragLineReleased_ = ini;
		}
		portal_->fetchRow(gridToPortalRowVector_[lin]);
		uid = portal_->getData(uniqueIdPos_);
		oid = portal_->getData(objectIdPos_);
		ObjectInfo& info = objectMap_[oid];
		int pointed = info.status_  & 0x1;
		if(uid.empty() == false)
			pointed = info.uniqueMap_[uid] & 0x1;

		if (lin < dragLineReleased_)
		{
			begin = lin + 1;
			end = dragLineReleased_;
		}
		else
		{
			begin = dragLineReleased_;
			end = lin - 1;
		}

		for (i = begin ; i <= end; ++i)
		{
			portal_->fetchRow(gridToPortalRowVector_[i]);
			uid = portal_->getData(uniqueIdPos_);
			oid = portal_->getData(objectIdPos_);
			ObjectInfo& info = objectMap_[oid];
			changeObjectStatus(info, uid, "shiftAndPointing", pointed);
			drawSet.insert(oid);
		}
	}

	lastLinePressed_ = lin;
	clearSelection(true);

	refresh();
	if (saveObjectStatus(theme_, drawSet, objectMap_) == false)
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Fail to update the \"c_object_status\" and \"grid_status\" fields in the collection table!"));
		return;
	}
	emit putColorOnObject(theme_, drawSet);
	gridInformation();
}


void TeQtGrid::putColorOnGrid(set<string> uidSet, set<string> oidSet, string operation, string /* attrQuery */)     
{
	//if(uidSet.empty() && oidSet.empty())
	//	return;

	int	x, y;
	string oid, uid;
	set<string> drawSet;
	set<string>::iterator it, nit;
	list<string>::iterator vit;
	map<string, int>::iterator uit;
	map<string, ObjectInfo>::iterator oit;
	set<string> auxSet;

	for (it = oidSet.begin(); it != oidSet.end(); ++it)
	{
		ObjectInfo& info = objectMap_[*it];
		if(info.uniqueMap_.size())
		{
			for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				uidSet.insert(uit->first);
		}
		else
			auxSet.insert(*it);
	}
	oidSet = auxSet;

	if (operation == "newQuery" || operation == "filterQuery" || operation == "addQuery")
	{
		//get the unique ids that were queried before this query
		list<string> oldUQueriedList, oldOQueriedList;
		for(oit =  objectMap_.begin(); oit != objectMap_.end(); oit++)
		{
			ObjectInfo& info = oit->second;
			if(info.uniqueMap_.size())
			{
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				{
					if(uit->second & 0x2)
						oldUQueriedList.push_back(uit->first);
				}
			}
			else
			{
				if(info.status_ & 0x2)
					oldOQueriedList.push_back(oit->first);
			}
		}

		if(operation == "newQuery")
		{
			// remove intersection (new and old set)
			vit = oldUQueriedList.begin();
			while(vit != oldUQueriedList.end())
			{
				if((nit = uidSet.find(*vit)) != uidSet.end())
				{
					vit = oldUQueriedList.erase(vit);
					uidSet.erase(nit);
				}
				else
					vit++;
			}
			vit = oldOQueriedList.begin();
			while(vit != oldOQueriedList.end())
			{
				if((nit = oidSet.find(*vit)) != oidSet.end())
				{
					vit = oldOQueriedList.erase(vit);
					oidSet.erase(nit);
				}
				else
					vit++;
			}

			// reset old queried
			for(vit = oldUQueriedList.begin(); vit != oldUQueriedList.end(); vit++)
			{
				oid = unique2ObjectMap_[*vit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*vit] &= 0x1;
				drawSet.insert(oid);
			}
			for(vit = oldOQueriedList.begin(); vit != oldOQueriedList.end(); vit++)
			{
				oid = *vit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ &= 0x1;
				drawSet.insert(oid);
			}

			// set new queried
			for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
			{
				oid = unique2ObjectMap_[*nit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*nit] |= 0x2;
				drawSet.insert(oid);
			}
			for(nit = oidSet.begin(); nit != oidSet.end(); nit++)
			{
				oid = *nit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ |= 0x2;
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					uit->second |= 0x2;
				drawSet.insert(oid);
			}
		}
		else if(operation == "filterQuery")
		{
			// remove intersection (old set)
			for (nit = uidSet.begin(); nit != uidSet.end(); nit++)
				if((vit = std::find(oldUQueriedList.begin(), oldUQueriedList.end(), *nit)) != oldUQueriedList.end())
					oldUQueriedList.erase(vit);
			for (nit = oidSet.begin(); nit != oidSet.end(); nit++)
				if((vit = std::find(oldOQueriedList.begin(), oldOQueriedList.end(), *nit)) != oldOQueriedList.end())
					oldOQueriedList.erase(vit);

			// reset old queried
			for(vit = oldUQueriedList.begin(); vit != oldUQueriedList.end(); vit++)
			{
				oid = unique2ObjectMap_[*vit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*vit] &= 0x1;
				drawSet.insert(oid);
			}
			for(vit = oldOQueriedList.begin(); vit != oldOQueriedList.end(); vit++)
			{
				oid = *vit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ &= 0x1;
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					uit->second &= 0x1;
				drawSet.insert(oid);
			}
		}
		else if(operation == "addQuery")
		{
			// remove intersection (new set)
			for (vit = oldUQueriedList.begin(); vit != oldUQueriedList.end(); vit++)
				if((nit = uidSet.find(*vit)) != uidSet.end())
					uidSet.erase(nit);
			for (vit = oldOQueriedList.begin(); vit != oldOQueriedList.end(); vit++)
				if((nit = oidSet.find(*vit)) != oidSet.end())
					oidSet.erase(nit);

			// set new queried
			for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
			{
				oid = unique2ObjectMap_[*nit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*nit] |= 0x2;
				drawSet.insert(oid);
			}
			for(nit = oidSet.begin(); nit != oidSet.end(); nit++)
			{
				oid = *nit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ |= 0x2;
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					uit->second |= 0x2;
				drawSet.insert(oid);
			}
		}
	}
	else if (operation == "newPointing" || operation == "togglePointing" || operation == "addPointing")
	{
		list<string> oldUPointedList, oldOPointedList;
		if(operation != "togglePointing")
		{
			//get the object ids that were pointed before this pointing
			for(oit =  objectMap_.begin(); oit != objectMap_.end(); oit++)
			{
				ObjectInfo& info = oit->second;
				if(info.uniqueMap_.size())
				{
					for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					{
						if(uit->second & 0x1)
							oldUPointedList.push_back(uit->first);
					}
				}
				else
				{
					if(info.status_ & 0x1)
						oldOPointedList.push_back(oit->first);
				}
			}
		}

		if(operation == "newPointing")
		{
			// remove intersection (new and old set)
			vit = oldUPointedList.begin();
			while(vit != oldUPointedList.end())
			{
				if((nit = uidSet.find(*vit)) != uidSet.end())
					vit = oldUPointedList.erase(vit);
				else
					vit++;
			}
			vit = oldOPointedList.begin();
			while(vit != oldOPointedList.end())
			{
				if((nit = oidSet.find(*vit)) != oidSet.end())
					vit = oldOPointedList.erase(vit);
				else
					vit++;
			}

			// reset old pointeds
			for(vit = oldUPointedList.begin(); vit != oldUPointedList.end(); vit++)
			{
				oid = unique2ObjectMap_[*vit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*vit] &= 0x2;
				drawSet.insert(oid);
			}
			for(vit = oldOPointedList.begin(); vit != oldOPointedList.end(); vit++)
			{
				oid = *vit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ &= 0x2;
				drawSet.insert(oid);
			}

			// set new pointed
			for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
			{
				oid = unique2ObjectMap_[*nit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*nit] |= 0x1;
				drawSet.insert(oid);
			}
			for(nit = oidSet.begin(); nit != oidSet.end(); nit++)
			{
				oid = *nit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ |= 0x1;
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					uit->second |= 0x1;
				drawSet.insert(oid);
			}
		}
		else if(operation == "togglePointing")
		{
			// toggle status (new pointed ids)
			nit = oidSet.begin();
			if(nit != oidSet.end())
			{
				oid = *nit;
				ObjectInfo& info = objectMap_[oid];
				int status = info.status_;

				if(status & 0x1) // reset new pointed
				{
					while(nit != oidSet.end())
					{
						oid = *nit;
						ObjectInfo& info = objectMap_[oid];
						info.status_ &= 0x2;
						for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
							uit->second &= 0x2;
						nit++;
						drawSet.insert(oid);
					}
				}
				else // set new pointed
				{
					while(nit != oidSet.end())
					{
						oid = *nit;
						ObjectInfo& info = objectMap_[oid];
						info.status_ |= 0x1;
						for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
							uit->second |= 0x1;
						nit++;
						drawSet.insert(oid);
					}
				}
			}

			nit = uidSet.begin();
			if(nit != uidSet.end())
			{
				oid = unique2ObjectMap_[*nit];
				ObjectInfo& info = objectMap_[oid];
				int status = info.status_;

				if(status & 0x1) // reset new pointed
				{
					for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
					{
						oid = unique2ObjectMap_[*nit];
						ObjectInfo& info = objectMap_[oid];
						info.uniqueMap_[*nit] &= 0x2;
						drawSet.insert(oid);
					}
				}
				else // set new pointed
				{
					for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
					{
						oid = unique2ObjectMap_[*nit];
						ObjectInfo& info = objectMap_[oid];
						info.uniqueMap_[*nit] |= 0x1;
						drawSet.insert(oid);
					}
				}
			}
		}
		else if(operation == "addPointing")
		{
			// remove intersection (new pointed ids)
 // not erase (not optimize) - graphic plot need selectd objects

			// set new pointeds
			for(nit = uidSet.begin(); nit != uidSet.end(); nit++)
			{
				oid = unique2ObjectMap_[*nit];
				ObjectInfo& info = objectMap_[oid];
				info.uniqueMap_[*nit] |= 0x1;
				drawSet.insert(oid);
			}
			for(nit = oidSet.begin(); nit != oidSet.end(); nit++)
			{
				oid = *nit;
				ObjectInfo& info = objectMap_[oid];
				info.status_ |= 0x1;
				for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
					uit->second |= 0x1;
				drawSet.insert(oid);
			}
		}
	}

	for(it = drawSet.begin(); it != drawSet.end(); it++)
	{
		oid = *it;
		ObjectInfo& info = objectMap_[oid];
		if(info.uniqueMap_.size())
		{
			set<int> statusSet;
			uit = info.uniqueMap_.begin();
			uid = uit->first;

			while(uit != info.uniqueMap_.end())
				statusSet.insert(uit++->second);
			if(statusSet.find(3) != statusSet.end())
				info.status_ = 3;
			else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) != statusSet.end())
				info.status_ = 3;
			else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) == statusSet.end())
				info.status_ = 2;
			else if(statusSet.find(2) == statusSet.end() && statusSet.find(1) != statusSet.end())
				info.status_ = 1;
			else
				info.status_ = 0;
		}
	}

	// If it is not a query operation (i.e., a pointing operation), scroll the grid if the
	// single object is not visible in the grid.
	if (operation == "togglePointing")
	{
		vector<string>::iterator itpos;
		if(uid.empty() == false)
			itpos = std::find(portalRow2UniqueIdVector_.begin(), portalRow2UniqueIdVector_.end(), uid);
		else
			itpos = std::find(portalRow2UniqueIdVector_.begin(), portalRow2UniqueIdVector_.end(), oid);
		int pos = 0;
		if(itpos != portalRow2UniqueIdVector_.end())
			pos = itpos - portalRow2UniqueIdVector_.begin();

		vector<int>::iterator itpos2;
		itpos2 = std::find(gridToPortalRowVector_.begin(),gridToPortalRowVector_.end(), pos);
		pos = itpos2 - gridToPortalRowVector_.begin();

		if (pos <= iLine_ || pos >= fLine_)
		{
			// if editing, end edit to enable vertical scrolling
			if(editing_ && (rowToEdit_ > -1 && colToEdit_ > -1))
			{
				endEdit(rowToEdit_, colToEdit_, true, true);
				rowToEdit_ = -1;
				colToEdit_ = -1;
				editing_ = false;
			}
			doRepaint_ = true;
			x = contentsX();
			y = rowPos(pos);
			setContentsPos(x, y);
		}
	}

	clearSelection(true);
	refresh();

	if (saveObjectStatus(theme_, drawSet, objectMap_) == false)
	{
		//QMessageBox::critical(this, tr("Error"),
		//	tr("Fail to update the \"c_object_status\" and \"grid_status\" fields in the collection table!"));
		//return;
	}

	emit putColorOnObject(theme_, drawSet);

	if(autoPromote_)
	{
		// if editing, end edit to enable vertical scrolling
		if(editing_ && (rowToEdit_ > -1 && colToEdit_ > -1))
		{
			endEdit(rowToEdit_, colToEdit_, true, true);
			rowToEdit_ = -1;
			colToEdit_ = -1;
			editing_ = false;
		}
		if (operation == "newPointing" || operation == "addPointing")
			promotePointedObjectsSlot();
		else if(operation == "newQuery" || operation == "addQuery" || operation == "filterQuery")
			promoteQueriedObjectsSlot();
	}

	gridInformation();
}


void TeQtGrid::drawContents (QPainter *qp, int clipx, int clipy, int clipw, int cliph)
{
	if(doRepaint_ == true)
	{
		update();
		QTable :: drawContents(qp, clipx, clipy, clipw, cliph);
	}
}


void TeQtGrid::update()
{
	if(portal_ == 0)
		return;
	int ie = currEditRow();
	int je = currEditCol();
	QString vale;
	if(ie >= 0 && je >= 0)
	{
		setCellContentFromEditor(ie, je);
		vale = text(ie, je);
		if(vale.isEmpty())
		{
			int pos = gridToPortalRowVector_[ie];
			if(portal_->fetchRow(pos))
			{
				string s = portal_->getData(gridToPortalColVector_[je]);
				if(s.empty() == false)
					vale = s.c_str();
			}
		}
	}

	int	i, j, pos;

	int gridY = contentsY();
	int gridX = contentsX();

	if(mousePressedOnHorizontalHeader_ && gridY == 0)
	{
		setContentsPos(gridX_, gridY_);
		gridY = gridY_;
		mousePressedOnHorizontalHeader_ = false;
	}

	int gridHeight = visibleHeight(); 
	int gridWidth = visibleWidth(); 

	if (doRefresh_ == false && 
		gridX_ == gridX && gridY_ == gridY &&
		gridHeight_ == gridHeight && 
		gridWidth_ == gridWidth)
		return;

	gridX_ = gridX;
	gridY_ = gridY;
	gridHeight_ = gridHeight;
	gridWidth_ = gridWidth;
		
// clear cells
	doRepaint_ = false; // do not repaint
	if (iLine_ >= 0 && iCol_ >= 0)
	{
		for (i = iLine_; i <= fLine_; ++i)
		{
			for (j = iCol_; j <= fCol_; ++j)
			{
				if(item(i, j))
					clearCell(i, j);
			}
		}
	}

	iLine_ = rowAt(gridY_);
	iCol_  = columnAt(gridX_);
	visibleLines_ = rowAt(gridHeight_);
	if (visibleLines_ == -1)
		visibleLines_ = numRows();

	visibleCols_ = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=iCol_; i<numCols(); i++)
	{
		if(gridWidth_ < columnPos(i) - scVal)
			break;
		visibleCols_++;
	}

	if (iLine_ < 0 || iCol_ < 0)
	{
		gridX_ = gridY_ = iLine_ = fLine_ = iCol_ = fCol_ = -1;
		doRepaint_ = true;
		return;
	}

	fLine_ = iLine_ + visibleLines_;
	if (fLine_ > numRows() - 1)
		fLine_ = numRows() - 1;
	fCol_ = iCol_ + visibleCols_ - 1;

	int status;
	string st, oid, uid;
	QColor qc;
	TeColor color;
	for (i = iLine_; i <= fLine_ ; ++i)
	{
		pos = gridToPortalRowVector_[i];
		if (portal_->fetchRow(pos) == false)
			return;

		oid = portal_->getData(objectIdPos_);
		uid = portal_->getData(uniqueIdPos_);

		ObjectInfo& info = objectMap_[oid];
		if(uid.empty() == false)
			status = info.uniqueMap_[uid];
		else
			status = info.status_;

		if (status == 1)
			color = (theme_->getTheme())->pointingLegend().visual(TePOLYGONS)->color();
		else if (status == 2)
			color = (theme_->getTheme())->queryLegend().visual(TePOLYGONS)->color();
		else if (status == 3)
			color = (theme_->getTheme())->queryAndPointingLegend().visual(TePOLYGONS)->color();
		qc = QColor(color.red_,color.green_,color.blue_);

		for (j = iCol_; j <= fCol_; ++j)
		{
			if (gridColVisVector_[j] == true)
			{
				st = portal_->getData(gridToPortalColVector_[j]);
				if(status != 0)
				{
					TeQtGridItem *ti;
					if(ie == i && je == j)
						ti = new TeQtGridItem( this, QTableItem::OnTyping, vale);
					else
						ti = new TeQtGridItem( this, QTableItem::OnTyping, st.c_str());

					ti->setColor(qc);
					setItem(i, j, ti );
				}
				else
				{
					if(ie == i && je == j)
						setText(i, j, vale);
					else
						setText(i, j, st.c_str());
				}
			}

			if(i == fLine_ && j == fCol_- 1)
			{
				doRepaint_ = true; // do repaint
				doRefresh_ = true;
				updateCell(i, j);
			}
		}
	}

	doRepaint_ = true; // do repaint
	doRefresh_ = false;

	if(ie >= 0 && je >= 0)
		endEdit(ie, je, true, true);

	if(rowToEdit_ > -1 && colToEdit_ > -1 && editing_)
	{
		setEditMode(QTable::Editing, rowToEdit_, colToEdit_);
		beginEdit(rowToEdit_, colToEdit_, false);
	}
}


void TeQtGrid::adjustColumns()
{
	int	i;
	for (i = 0; i < numCols(); ++i)
		adjustColumn(i);
	refresh();
}


void TeQtGrid::swapColumns(int col1, int col2)
{
	int	nc = numCols();
	vector<int>	cols;
	int	n;

	if (col1 == -1 || col2 == -1)
	{
		for(n = 0; n < nc; n++)
			if (isColumnSelected(n, true) == true)
				cols.push_back(n);
		if(cols.size() != 2)
		{
			QMessageBox::warning(this, tr("Warning"),
				tr("Select the two grid columns to be exchanged!"));
			return;
		}
		col1 = cols[0];
		col2 = cols[1];
	}

	int w1 = columnWidth(col1);
	int w2 = columnWidth(col2);

	int c1 = gridToPortalColVector_[col1];
	int c2 = gridToPortalColVector_[col2];

	gridToPortalColVector_[col1] = c2;
	gridToPortalColVector_[col2] = c1;

	QTable::swapColumns(col1, col2);
	for (n=0; n<nc; n++)
	{
		string st = portal_->getAttribute(gridToPortalColVector_[n]).rep_.name_;
		horizontalHeader()->setLabel(n, st.c_str());
	}

	setColumnWidth(col1, w2);
	setColumnWidth(col2, w1);

	refresh();
}


void TeQtGrid::setVisColumn(bool vis, int col)
{
	unsigned int nc = numCols();
	unsigned int n;
	vector<int>	cols;

	if (vis == false)
	{
		if (col == -1)
		{
			for(n=0; n<nc; n++)
				if (isColumnSelected(n, true) == true)
					cols.push_back(n);
			if(cols.size() == 0)
			{
				QMessageBox::warning(this, tr("Warning"),
					tr("Select the grid column(s) to be hidden!"));
				return;
			}
		}
		else
			cols.push_back(col);

		for(n=0; n<cols.size(); n++)
		{
			showColumn(cols[n]); // it is a BUG in the QT 3.2.0 
			hideColumn(cols[n]);
			gridColVisVector_[cols[n]] = false;
		}
	}
	else
	{
		for(n=0; n<nc; n++)
		{
			if(gridColVisVector_[n] == false)
			{
				gridColVisVector_[n] = true;
				showColumn(n);
				cols.push_back(n);
			}
		}
	}
	refresh();
}

void TeQtGrid::refresh()
{
	doRefresh_ = true;
	repaint();
	update();
}


void TeQtGrid::promotePointedObjectsSlot()
{
	TeWaitCursor	wait;
	gridToPortalRowVector_.clear();
	string	oid, uid;
	TeDatabasePortal* portal = db_->getPortal();
	vector<int> notByPointingVector;

	unsigned int i=0;
    int gs;
	if (portal->query(sql_))
	{
		while (portal->fetchRow())
		{
			if(theme_->getTheme()->getProductId() == TeTHEME)
			{
				gs = atoi(portal->getData(gridStatusPos_));
				if(gs &= 0x1)
					gridToPortalRowVector_.push_back(i);
				else
					notByPointingVector.push_back(i);
			}
			else if(theme_->getTheme()->getProductId() == TeEXTERNALTHEME)
			{
				uid = portal->getData(uniqueIdPos_);
				if(uid.empty() == false)
				{
					std::map<std::string, int >::iterator it = theme_->getTheme()->getItemStatusMap().find(uid);
					if( it != theme_->getTheme()->getItemStatusMap().end())
					{
						if(it->second & 1)
							gridToPortalRowVector_.push_back(i);
						else
							notByPointingVector.push_back(i);
					}
					else
					{
						notByPointingVector.push_back(i);
					}
				}
				else
				{
					oid = portal->getData(objectIdPos_);
					std::map<std::string, int >::iterator it = theme_->getTheme()->getObjStatusMap().find(oid);
					if( it != theme_->getTheme()->getObjStatusMap().end())
					{
						if(it->second & 1)
							gridToPortalRowVector_.push_back(i);
						else
							notByPointingVector.push_back(i);
					}
					else
					{
						notByPointingVector.push_back(i);
					}
				}
			}		
			++i;
		}
	}
	delete portal;

	for (i = 0; i < notByPointingVector.size(); ++i)
		gridToPortalRowVector_.push_back(notByPointingVector[i]);
	
	doRepaint_ = true;

	int x = contentsX();
	int y = rowPos(0);
	setContentsPos(x, y);

	refresh();
}


void TeQtGrid::promoteQueriedObjectsSlot()
{
	TeWaitCursor	wait;
	gridToPortalRowVector_.clear();
	string	oid, uid, val;
	TeDatabasePortal* portal = db_->getPortal();
	vector<int> notByQueryVector;

	unsigned int i=0;
    int gs;
	if (portal->query(sql_))
	{
		while (portal->fetchRow())
		{
			if(theme_->getTheme()->getProductId() == TeTHEME)
			{
				val =  portal->getData(gridStatusPos_);
				if(val.empty())
					gs = atoi(portal->getData(resultIdPos_));
				else
					gs = atoi(val.c_str());

				if(gs &= 0x2)
					gridToPortalRowVector_.push_back(i);
				else
					notByQueryVector.push_back(i);
			}
			else if(theme_->getTheme()->getProductId() == TeEXTERNALTHEME)
			{
				uid = portal->getData(uniqueIdPos_);
				if(uid.empty() == false)
				{
					std::map<std::string, int >::iterator it = theme_->getTheme()->getItemStatusMap().find(uid);
					if( it != theme_->getTheme()->getItemStatusMap().end())
					{
						if(it->second & 2)
							gridToPortalRowVector_.push_back(i);
						else
							notByQueryVector.push_back(i);
					}
					else
					{
						notByQueryVector.push_back(i);
					}
				}
				else
				{
					oid = portal->getData(objectIdPos_);
					std::map<std::string, int >::iterator it = theme_->getTheme()->getObjStatusMap().find(oid);
					if( it != theme_->getTheme()->getObjStatusMap().end())
					{
						if(it->second & 2)
							gridToPortalRowVector_.push_back(i);
						else
							notByQueryVector.push_back(i);
					}
					else
					{
						notByQueryVector.push_back(i);
					}
				}
			}		
			++i;
		}
	}
	delete portal;

	for (i = 0; i < notByQueryVector.size(); ++i)
		gridToPortalRowVector_.push_back(notByQueryVector[i]);
	
	doRepaint_ = true;

	int x = contentsX();
	int y = rowPos(0);
	setContentsPos(x, y);

	refresh();
}


void TeQtGrid::sortAscOrderSlot()    
{
	vector<int>	v;

	sort(v, "ASC");
}


void TeQtGrid::sortDescOrderSlot()    
{
	vector<int>	v;

	sort(v, "DESC");
}


void TeQtGrid::sort(vector<int> cols, string order)
{
	unsigned int nc = numCols();
	unsigned int n;
	string uid, oid;

	if (portal_ == 0)
		return;


	if (cols.size() == 0)
	{
		for(n = 0; n < nc; ++n)
			if (isColumnSelected(n, true) == true)
				cols.push_back(n);
	}

	if (cols.size() == 0)
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Select the grid column(s) to be sorted!"));
		return;
	}

	orderBy_.clear();
	for (n=0; n<cols.size(); n++)
		orderBy_ += (horizontalHeader()->label(cols[n]) + " ").latin1() + order + ",";
	orderBy_ = orderBy_.substr(0, orderBy_.size()-1);
	doRepaint_ = true;

	portal_->freeResult();
	sql_ = sqlJoin_ + " ORDER BY " + orderBy_;
	if (portal_->query(sql_) == false)
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
		return;
	}

	gridToPortalRowVector_.clear();
	portalRow2UniqueIdVector_.clear();
	int i = 0;
	while(portal_->fetchRow())
	{
		uid = portal_->getData(uniqueIdPos_);
		if(uid.empty() == false)
			portalRow2UniqueIdVector_.push_back(uid);
		else
		{
			oid = portal_->getData(objectIdPos_);
			portalRow2UniqueIdVector_.push_back(oid);
		}
		gridToPortalRowVector_.push_back(i);
		++i;
	}

	refresh();
}


TeTable& TeQtGrid::findTable(int col) // grid column
{
	unsigned int i;
    int nCols;
	static	TeTable t;

	int portalCol = gridToPortalColVector_[col];
	TeAttributeList attrList;
	
	nCols = 0;
	for (i = 0; i < tableVector_.size(); ++i)
	{
		attrList = tableVector_[i].attributeList();
		nCols += attrList.size();
		if (portalCol < nCols)
			break;				
	}
	if(i == tableVector_.size())
		return t;
	return tableVector_[i];	
}

TeTable& TeQtGrid::findTable(string  colName)
{
	int	i;
	for(i=0; i<numCols(); i++)
	{
		string cname = horizontalHeader()->label(i).latin1();
		if(cname == colName)
			break;
	}
	return findTable(i);
}


void TeQtGrid::setVerticalEdition(bool flag)
{
	verticalEdition_ = flag;				
}

bool TeQtGrid::writeCell()
{
	string	val;
	QString qs;
	unsigned int i;

	if(rowToEdit_ == -1 || colToEdit_ == -1)
		return false;

	int portalRowToEdit = gridToPortalRowVector_[rowToEdit_];
	int portalColToEdit = gridToPortalColVector_[colToEdit_];
	endEdit(rowToEdit_, colToEdit_, true, false);

	int col = gridToPortalColVector_[colToEdit_];
	if(isIndex(col))
		return false;

	string colName = horizontalHeader()->label(colToEdit_).latin1();
	qs = text(rowToEdit_, colToEdit_);
//	if(qs.isEmpty() && db_->dbmsName() == "Ado")
//		return false;
	if(qs.isEmpty() == false)
		val = qs.latin1();

	TeAttributeList&	attrList = portal_->getAttributeList();
	TeAttrDataType type = attrList[portalColToEdit].rep_.type_;

	if (portal_->fetchRow(portalRowToEdit) == false)
		return false;

	TeWaitCursor wait;
	TeTable& tableToEdit = findTable(colToEdit_);
	string tableName = tableToEdit.name();
	string linkName;
	if(tableToEdit.tableType() == TeAttrStatic || tableToEdit.tableType() == TeAttrEvent)
		linkName = tableToEdit.linkName();
	else
		linkName = tableToEdit.uniqueName();

	i = linkName.find(".");
	if(i == string::npos)
		linkName = tableName + "." + linkName;

	string id = portal_->getData(linkName);

	string qString = "UPDATE " + tableName;
	if(val.empty())
	{
		qString += " SET " + colName + " = null";
		qString += " WHERE " + linkName + " = '" + id + "'";
	}
	else
	{
		if (type == TeSTRING || type == TeDATETIME)
		{
			qString += " SET " + colName + " = '" + db_->escapeSequence(val);
			qString += "' WHERE " + linkName + " = '" + id + "'";
		}
		else
		{
			qString += " SET " + colName + " = " + val;
			qString += " WHERE " + linkName + " = '" + id + "'";
		}
	}

	if (db_->execute(qString) == false)
		return false;

	if(theme_ != NULL && theme_->getTheme() != NULL && (theme_->getTheme()->type() == TeTHEME || theme_->getTheme()->type() == TeEXTERNALTHEME))
	{
		TeTheme* teTheme = dynamic_cast<TeTheme*>(theme_->getTheme());
		TeLayer* layer = teTheme->layer();
		if(layer != NULL)
		{
			layer->updateLayerEditionTime();
		}
	}

	if (db_->dbmsName() != "Ado")
		arrangeGridToPortalRowVector(col);

	return true;
}


void TeQtGrid::editNextRow()
{
	if (rowToEdit_ < numRows()-1)
		++rowToEdit_;
	else
	{
		rowToEdit_ = 0;
		editNextColumn();
	}
}


void TeQtGrid::editNextColumn()
{
	bool editavel = false;
	while(editavel == false)
	{
		if (colToEdit_ == numCols()-1)
		{
			colToEdit_ = 0;
			if (rowToEdit_ == numRows()-1)
				rowToEdit_ = 0;
			else
				++rowToEdit_;
		}
		else
			++colToEdit_;

		int col = gridToPortalColVector_[colToEdit_];
		if(isIndex(col))
			editavel = false;
		else
			editavel = true;
	}
}

void TeQtGrid::goToEditNewCell()
{
	int	x = contentsX();
	int y = contentsY();

	if(verticalEdition_ == true)
		editNextRow();
	else
		editNextColumn();

	if(rowToEdit_ >= fLine_)
		y = rowPos(iLine_+1);
	if(colToEdit_ >= fCol_)
		x = columnPos(iCol_+1);

	setContentsPos(x, y);
	refresh();

	int i = currEditRow();
	int j = currEditCol();
	if(i != rowToEdit_ || j != colToEdit_)
	{
		if(i > 0 && j > 0)
			endEdit(i, j, true, true);
		setEditMode(QTable::Editing, rowToEdit_, colToEdit_);
		beginEdit(rowToEdit_, colToEdit_, false);
	}
}

void TeQtGrid::removeQueryColorSlot()
{
	TeWaitCursor wait;

	set<string> drawSet;
	map<string, int>::iterator uit;

	TeDatabase* db = theme_->getLocalDatabase();
	
	map<string, ObjectInfo>::iterator oit;
	for(oit =  objectMap_.begin(); oit != objectMap_.end(); oit++)
	{
		ObjectInfo& info = oit->second;
		if(info.status_ & 0x2)
		{
			for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				uit->second &= 0x1;
			info.status_ &= 0x1;
			drawSet.insert(oit->first);
		}
	}

	clearSelection(true);
	refresh();
	string s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionTable();
	s += " SET c_object_status = 0 WHERE c_object_status = 2";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionTable();
	s += " SET c_object_status = 1 WHERE c_object_status = 3";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionAuxTable();
	s += " SET grid_status = 0 WHERE grid_status = 2";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionAuxTable();
	s += " SET grid_status = 1 WHERE grid_status = 3";
	db->execute(s);

	theme_->getTheme()->removeQueryColor();
	emit putColorOnObject(theme_, drawSet);
}


void TeQtGrid::removePointingColorSlot()
{
	TeWaitCursor wait;

	set<string> drawSet;
	map<string, int>::iterator uit;

	map<string, ObjectInfo>::iterator oit;
	for(oit =  objectMap_.begin(); oit != objectMap_.end(); oit++)
	{
		ObjectInfo& info = oit->second;
		if(info.status_ & 0x1)
		{
			for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				uit->second &= 0x2;
			info.status_ &= 0x2;
			drawSet.insert(oit->first);
		}
	}

	clearSelection(true);
	refresh();

	TeDatabase* db = theme_->getLocalDatabase();

	string s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionTable();
	s += " SET c_object_status = 0 WHERE c_object_status = 1";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionTable();
	s += " SET c_object_status = 2 WHERE c_object_status = 3";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionAuxTable();
	s += " SET grid_status = 0 WHERE grid_status = 1";
	db->execute(s);

	s = "UPDATE " + ((TeTheme*)theme_->getTheme())->collectionAuxTable();
	s += " SET grid_status = 2 WHERE grid_status = 3";
	db->execute(s);

	theme_->getTheme()->removePointingColor();
	emit putColorOnObject(theme_, drawSet);
}


void TeQtGrid::scrollToNextPointedObjectSlot()
{
	TeWaitCursor wait;

	int	rowpos, portalpos, nrows = numRows();
	int	x, y;
	string oid, uid;

	x = contentsX();
	y = contentsY();
	rowpos = rowAt(y);

	if(rowpos == numRows() - 1)
	{
		wait.resetWaitCursor();
		return;
	}
	rowpos++;

	while(rowpos < nrows)
	{
		portalpos = gridToPortalRowVector_[rowpos];
		portal_->fetchRow(portalpos);

		oid = portal_->getData(objectIdPos_);
		uid = portal_->getData(uniqueIdPos_);

		ObjectInfo& info = objectMap_[oid];
		int status = info.status_;
		if(uid.empty() == false)
			status = info.uniqueMap_[uid];


		if (status & 0x1)
			break;
		rowpos++;
	}


	if (rowpos == nrows)
	{
		wait.resetWaitCursor();
		return;
	}

	y = rowPos(rowpos);
	setContentsPos (x, y);

	clearSelection(true);
	refresh();
}


void TeQtGrid::scrollToPrevPointedObjectSlot()
{
	TeWaitCursor wait;

	int	rowpos, portalpos;
	int	x, y;
	string oid, uid;

	x = contentsX();
	y = contentsY();
	rowpos = rowAt(y);

	if(rowpos == 0)
	{
		wait.resetWaitCursor();
		return;
	}
	rowpos--;

	while(rowpos >= 0)
	{
		portalpos = gridToPortalRowVector_[rowpos];
		portal_->fetchRow(portalpos);

		oid = portal_->getData(objectIdPos_);
		uid = portal_->getData(uniqueIdPos_);

		ObjectInfo& info = objectMap_[oid];
		int status = info.status_;
		if(uid.empty() == false)
			status = info.uniqueMap_[uid];

		if (status & 0x1)
			break;
		rowpos--;
	}


	if (rowpos < 0)
	{
		wait.resetWaitCursor();
		return;
	}

	y = rowPos(rowpos);
	setContentsPos (x, y);

	clearSelection(true);
	refresh();
}


void TeQtGrid::scrollToNextQueriedObjectSlot()
{
	TeWaitCursor wait;

	int	rowpos, portalpos, nrows = numRows();
	int	x, y;
	string oid, uid;

	x = contentsX();
	y = contentsY();
	rowpos = rowAt(y);

	if(rowpos == numRows() - 1)
	{
		wait.resetWaitCursor();
		return;
	}
	rowpos++;

	while(rowpos < nrows)
	{
		portalpos = gridToPortalRowVector_[rowpos];
		portal_->fetchRow(portalpos);

		oid = portal_->getData(objectIdPos_);
		uid = portal_->getData(uniqueIdPos_);

		ObjectInfo& info = objectMap_[oid];
		int status = info.status_;
		if(uid.empty() == false)
			status = info.uniqueMap_[uid];

		if (status & 0x2)
			break;
		rowpos++;
	}


	if (rowpos == nrows)
	{
		wait.resetWaitCursor();
		return;
	}

	y = rowPos(rowpos);
	setContentsPos (x, y);

	clearSelection(true);
	refresh();
}


void TeQtGrid::scrollToPrevQueriedObjectSlot()
{
	TeWaitCursor wait;

	int	rowpos, portalpos;
	int	x, y;
	string oid, uid;

	x = contentsX();
	y = contentsY();
	rowpos = rowAt(y);

	if(rowpos == 0)
	{
		wait.resetWaitCursor();
		return;
	}
	rowpos--;

	while(rowpos >= 0)
	{
		portalpos = gridToPortalRowVector_[rowpos];
		portal_->fetchRow(portalpos);

		oid = portal_->getData(objectIdPos_);
		uid = portal_->getData(uniqueIdPos_);

		ObjectInfo& info = objectMap_[oid];
		int status = info.status_;
		if(uid.empty() == false)
			status = info.uniqueMap_[uid];

		if (status & 0x2)
			break;
		rowpos--;
	}


	if (rowpos < 0)
	{
		wait.resetWaitCursor();
		return;
	}

	y = rowPos(rowpos);
	setContentsPos (x, y);

	clearSelection(true);
	refresh();
}

void TeQtGrid::invertSelectionSlot()
{
	TeWaitCursor wait;

	set<string> drawSet;
	map<string, int>::iterator uit;
	string oid, uid;

	map<string, ObjectInfo>::iterator oit;
	for(oit =  objectMap_.begin(); oit != objectMap_.end(); oit++)
	{
		ObjectInfo& info = oit->second;
		if(info.status_ & 0x2)	//queried object -> sets it to selected
		{
			for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				uit->second &= 0x1;
			info.status_ &= 0x1;
			for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
				uit->second |= 0x1;
			info.status_ |= 0x1;
		}
		for(uit = info.uniqueMap_.begin(); uit != info.uniqueMap_.end(); uit++)
			uit->second ^= 0x1;
		info.status_ ^= 0x1;			 
		drawSet.insert(oit->first);
	}
	refresh();
	if (saveObjectStatus(theme_, drawSet, objectMap_) == false)
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Fail to update the \"c_legend_result\" and \"grid_status\" fields in the collection table!"));
		return;
	}
	emit putColorOnObject(theme_, drawSet);
}

bool TeQtGrid::getVisColumn(int col)
{
	if(col < (int)gridColVisVector_.size())
		return gridColVisVector_[col];
	return false;
}


bool TeQtGrid::isIndex(int col) // portal column
{
	unsigned int i;
	string cname = portal_->getAttribute(col).rep_.name_;

	if(cname.find(".") == string::npos)
	{
		TeAttributeList attrList;		
		int nCols = 0;
		for (i = 0; i < tableVector_.size(); ++i)
		{
			attrList = tableVector_[i].attributeList();
			nCols += attrList.size();
			if (col < nCols)
				break;				
		}
		
		cname.insert(0, tableVector_[i].name() + ".");
//		if(i>0)
//			cname.insert(0, tableVector_[(i-1)].name() + ".");
	}

	cname = TeConvertToUpperCase(cname);
	for(i=0; i<indexVector_.size(); i++)
	{
		string s = TeConvertToUpperCase(indexVector_[i]);
		if(s.find(cname) != string::npos)
			return true;
	}
	return false;
}

bool TeQtGrid::isDateTimeRegistered(int col) // portal column
{
	string cname = portal_->getAttribute(col).rep_.name_;
	TeTable& t = findTable(cname);
	string name = cname;
	int f = name.find(".");
	if(f >= 0)
		name = name.substr(f+1);
	if(t.attInitialTime() == name || t.attFinalTime() == name)
		return true;
	return false;
}

bool TeQtGrid::isSortBy(int col) // portal column
{
	string cname = portal_->getAttribute(col).rep_.name_;
	cname += " ";

//	if(orderBy_.find(cname, string::npos) == string::npos)
	if(orderBy_.find(cname) == string::npos)
		return false;
	return true;
}

bool TeQtGrid::isExternalTable(int col) // portal column
{
	unsigned int c;
	for(c=0; c<gridToPortalColVector_.size(); c++)
	{
		if(gridToPortalColVector_[c] == col)
			break;
	}

	TeTable& T = findTable(c);
	if(T.tableType() == TeAttrExternal)
		return true;
	else
		return false;
}

string TeQtGrid::getObject(int row)
{
	int portalpos = gridToPortalRowVector_[row];
	portal_->fetchRow(portalpos);

	string oid = portal_->getData(objectIdPos_);
	return oid;
}

void TeQtGrid::gridInformation()
{
	if(showGridInformation_ == false)
		return;

	int orows = 0, opointed = 0, oqueried = 0, opqs = 0;
	int pointed = 0, queried = 0, pqs = 0;
	int rows = numRows();
	map<string, ObjectInfo>::iterator it = objectMap_.begin();

	while (it != objectMap_.end())
	{
		orows++;
		ObjectInfo& info = it->second;
		int leg_res = info.status_;
		if(leg_res == 1)
			opointed++;
		else if(leg_res == 2)
			oqueried++;
		else if(leg_res == 3)
		{
			opointed++;
			oqueried++;
			opqs++;
		}

		map<string, int> uMap = info.uniqueMap_;
		if(uMap.size())
		{
			map<string, int>::iterator uit = uMap.begin();
			while(uit != uMap.end())
			{
				int gStatus = uit->second;
				if(gStatus == 1)
					pointed++;
				else if(gStatus == 2)
					queried++;
				else if(gStatus == 3)
				{
					pointed++;
					queried++;
					pqs++;
				}
				uit++;
			}
		}
		it++;
	}

	string sorows = Te2String(orows);
	string sopointed = Te2String(opointed);
	string soqueried = Te2String(oqueried);
	string sopqs = Te2String(opqs);
	string srows = Te2String(rows);
	string spointed = Te2String(pointed);
	string squeried = Te2String(queried);
	string spqs = Te2String(pqs);

	if(orows != rows || opointed != pointed || oqueried != queried || opqs != pqs)
	{
		sorows += "/" + srows;
		sopointed += "/" + spointed;
		soqueried += "/" + squeried;
		sopqs += "/" + spqs;
	}

	QString msg = tr("Number of Rows:") + " " + sorows.c_str() + ", " + tr("Pointed:") + " " + sopointed.c_str();
	msg += ", " + tr("Queried:") + " " + soqueried.c_str() + ", " + tr("Pointed and Queried:") + " " + sopqs.c_str();
	emit displayGridInformation(msg);
}

void TeQtGrid::endEdit(int row, int col, bool accept, bool replace)
{
	if(accept && replace)
		writeCell();
	else
		QTable::endEdit(row, col, accept, replace);
}


void TeQtGrid::columnWidthChanged(int col)
{
	if(doRepaint_)
		refresh();
	QTable::columnWidthChanged(col);
}

void TeQtGrid::insertObjectIntoCollection(string newId, string newTId)
{
	doRepaint_ = false;
	ObjectInfo	info;

	string oid = newId;
	string uid = newId;
	if(newTId.empty() == false)
	{
		uid += newTId;
		info.uniqueMap_[uid] = 1;
		unique2ObjectMap_[uid] = oid;
	}
	else
	{
		info.uniqueMap_[oid] = 1;
		unique2ObjectMap_[oid] = oid;
	}

	info.status_ = 1;
	objectMap_[oid] = info;

	portalRow2UniqueIdVector_.push_back(uid);

	int numRows = portalRow2UniqueIdVector_.size();
	gridToPortalRowVector_.push_back(numRows - 1);
	doRepaint_ = true;
	setNumRows(numRows);
}

void TeQtGrid::deletePointedLines()
{
	doRepaint_ = false;
	unsigned int	i, j;
	string oid, uid;
	vector<string> oidVec;
	map<string, vector<string> > uidMap;

	map<string, ObjectInfo>::iterator it = objectMap_.begin();
	while(it != objectMap_.end())
	{
		ObjectInfo	info = (*it).second;
		if(info.status_ == 1 || info.status_ == 3)
		{
			oid = (*it).first;
			uid.clear();
			map<string, int>::iterator uit = info.uniqueMap_.begin();
			vector<string> uidVec;
			while(uit != info.uniqueMap_.end())
			{
				if((*uit).second == 1 || (*uit).second == 3)
				{
					uid = (*uit).first;
					uidVec.push_back(uid);
				}
				uit++;
			}

			if(uidVec.size() == 0)
			{
				uidVec.push_back(oid);
			}
			uidMap[oid] = uidVec;
			oidVec.push_back(oid);
		}
		it++;
	}

	vector<string> iVec;
	for(i=0; i<oidVec.size(); i++)
	{
		oid = oidVec[i];
		ObjectInfo&	info = objectMap_[oid];
		vector<string> uidVec = uidMap[oid];

		for(j=0; j<uidVec.size(); j++)
		{
			string s = uidVec[j];
			iVec.push_back(s);
			info.uniqueMap_.erase(s);
			unique2ObjectMap_.erase(s);
		}
		if(info.uniqueMap_.size() == 0)
			objectMap_.erase(oid);
	}

	vector<int> jVec;
	vector<string> copy1(portalRow2UniqueIdVector_.begin(), portalRow2UniqueIdVector_.end());
	portalRow2UniqueIdVector_.clear();
	for(i=0; i<copy1.size(); i++)
	{
		string ps = copy1[i];
		for(j=0; j<iVec.size(); j++)
		{
			string s = iVec[j];
			if(ps == s)
			{
				jVec.push_back(i);
				break;
			}
		}
		if(j == iVec.size())
			portalRow2UniqueIdVector_.push_back(ps);
	}

	vector<int> copy2(gridToPortalRowVector_.begin(), gridToPortalRowVector_.end());
	gridToPortalRowVector_.clear();
	for(i=0; i<copy2.size(); i++)
	{
		int ps = copy2[i];
		for(j=0; j<jVec.size(); j++)
		{
			int s = jVec[j];
			if(ps == s)
				break;
		}
		if(j == jVec.size())
			gridToPortalRowVector_.push_back(ps);
	}

	for(i=jVec.size()-1; (int)i>=0; i--)
	{
		int s = jVec[i];
		for(j=0; j<gridToPortalRowVector_.size(); j++)
		{
			int ss = gridToPortalRowVector_[j];
			if(ss >= s)
				gridToPortalRowVector_[j] = ss - 1;
		}
	}

	int numRows = portalRow2UniqueIdVector_.size();
	doRepaint_ = true;
	setNumRows(numRows);
}

bool TeQtGrid::hasPointedLine()
{
	map<string, ObjectInfo>::iterator it = objectMap_.begin();

	while(it != objectMap_.end())
	{
		ObjectInfo	info = (*it).second;
		if(info.status_ == 1 || info.status_ == 3)
			return true;
		it++;
	}

	return false;
}

void TeQtGrid::goToLastLine()
{
	int nRows = numRows();
	int iniRow = nRows - visibleLines_;

	doRepaint_ = true;
	int x = columnAt(gridX_);
	int y = rowPos(iniRow);
	setContentsPos(x, y);
	refresh();
}

void TeQtGrid::goToLastColumn()
{
	int x = contentsWidth() - visibleWidth();

	doRepaint_ = true;
	int y = contentsY();
	setContentsPos(x, y);
	refresh();
}

vector<int> TeQtGrid::getSelectedColumns()
{
	int i, j;
	vector<int> colVec;

	for (i = 0; i < numCols(); i++)
	{
		if(gridColVisVector_[i])
		{
			if (isColumnSelected(i, true) == true)
			{
				j = gridToPortalColVector_[i];
				colVec.push_back(j);
			}
		}
	}
	return colVec;
}

void TeQtGrid::arrangeColumns(TeAttrTableVector cTableVec)
{
	int	i, j, k, m;
	TeAttributeList CAL;
	TeAttributeList	AL;
	vector<int> cVec;

	for(i=m=0; (unsigned int)i<cTableVec.size(); i++)
	{
		TeTable ct = cTableVec[i];
		CAL = ct.attributeList();

		TeTable t = tableVector_[i];
		AL = t.attributeList();

		if(AL.size() > CAL.size())
		{
			for(j=0; (unsigned int)j<AL.size(); j++, m++)
			{
				TeAttribute a = AL[j];
				for(k=0; (unsigned int)k<CAL.size(); k++)
				{
					TeAttribute ca = CAL[k];
					if(TeConvertToUpperCase(a.rep_.name_) == TeConvertToUpperCase(ca.rep_.name_))
						break;
				}
				if((unsigned int)k == CAL.size())
					cVec.push_back(m);
			}
		}
		else if(AL.size() < CAL.size())
		{
			for(j=0; (unsigned int)j<CAL.size(); j++, m++)
			{
				TeAttribute a = CAL[j];
				for(k=0; (unsigned int)k<AL.size(); k++)
				{
					TeAttribute ca = AL[k];
					if(TeConvertToUpperCase(a.rep_.name_) == TeConvertToUpperCase(ca.rep_.name_))
						break;
				}
				if((unsigned int)k == AL.size())
					cVec.push_back(m);
			}
		}
		else
			m += (int)AL.size();
	}

//	TeAttributeList& attrList = portal_->getAttributeList();
	int nCols = numCols();
	int nOldCols = gridColVisVector_.size();
	vector<bool> cVisVec(gridColVisVector_.begin(), gridColVisVector_.end());
	vector<int> cSwapVec(gridToPortalColVector_.begin(), gridToPortalColVector_.end());
	gridColVisVector_.clear();
	gridToPortalColVector_.clear();

	bool added = false;
	if(nCols > nOldCols)
		added = true;

	std::sort(cVec.begin(), cVec.end());
	if(cVec.size())
	{
		if(added)
		{
			for(i=nOldCols; i<nCols; i++)
				cVisVec.push_back(true);
			gridColVisVector_ = vector<bool>(cVisVec.begin(), cVisVec.end());

			for(i=0; (unsigned int)i<cVec.size(); i++)
			{
				k = cVec[i];
				for(j=0; (unsigned int)j<cSwapVec.size(); j++)
				{
					int v = cSwapVec[j];
					if(v >= k)
						cSwapVec[j] = v + 1;
				}
				cSwapVec.push_back(k);
			}
			gridToPortalColVector_ = vector<int>(cSwapVec.begin(), cSwapVec.end());
		}
		else
		{
			for(i=j=0; (unsigned int)i<cVec.size(); i++)
			{
				k = cVec[i];
				while(j < k)
				{
					bool b = cVisVec[j];
					gridColVisVector_.push_back(b);
					j++;
				}
				j = k + 1;
			}
			while((unsigned int)j < cVisVec.size())
			{
				bool b = cVisVec[j];
				gridColVisVector_.push_back(b);
				j++;
			}

			for(i=0; (unsigned int)i<cVec.size(); i++)
			{
				k = cVec[i];
				for(j=0; (unsigned int)j<cSwapVec.size(); j++)
				{
					int kk = cSwapVec[j];
					if(kk != k)
						gridToPortalColVector_.push_back(kk);
				}
				cSwapVec.clear();
				cSwapVec = vector<int>(gridToPortalColVector_.begin(), gridToPortalColVector_.end());
				gridToPortalColVector_.clear();
			}

			for(i = (int)cVec.size() - 1; i>=0; i--)
			{
				k = cVec[i];
				for(j=0; (unsigned int)j<cSwapVec.size(); j++)
				{
					int v = cSwapVec[j];
					if(v >= k)
						cSwapVec[j] = v - 1;
				}
			}
			gridToPortalColVector_ = vector<int>(cSwapVec.begin(), cSwapVec.end());
		}
	}
	else
	{
		// Set the vector that will contain the order the
		// grid columns will be shown
		gridToPortalColVector_.clear();
		gridColVisVector_.clear();
		for (i = 0; i < nCols; i++)
		{
			gridToPortalColVector_.push_back(i);
			gridColVisVector_.push_back(true);
		}
	}
}

void TeQtGrid::arrangeGridToPortalRowVector(int col)
{
	int i, p, pp;
	string id, s, ss;

	portal_->freeResult();
	if (!portal_->query(sql_))
	{
		portal_->freeResult();
		delete portal_;
		portal_ = 0;
		return;
	}

	if(isSortBy(col) == false)
		return;

	vector<int> cGridToPortal(gridToPortalRowVector_.begin(), gridToPortalRowVector_.end());
	vector<string> cPortalToUnique(portalRow2UniqueIdVector_.begin(), portalRow2UniqueIdVector_.end());
	gridToPortalRowVector_.clear();
	portalRow2UniqueIdVector_.clear();

	while (portal_->fetchRow())
	{
		id = portal_->getData(uniqueIdPos_);
		if(id.empty())
			id = portal_->getData(objectIdPos_);
		portalRow2UniqueIdVector_.push_back(id);
	}

	vector<string>::iterator it;
	for(i=0; (unsigned int)i<cGridToPortal.size(); i++)
	{
		p = cGridToPortal[i];
		s = cPortalToUnique[p];
		ss = portalRow2UniqueIdVector_[p];
		if(s == ss)
			gridToPortalRowVector_.push_back(p);
		else
		{
			it = std::find(portalRow2UniqueIdVector_.begin(), portalRow2UniqueIdVector_.end(), s);
			if(it != portalRow2UniqueIdVector_.end())
				pp = it - portalRow2UniqueIdVector_.begin();

			gridToPortalRowVector_.push_back(pp);
		}
	}
}

void TeQtGridItem::paint(QPainter* p, const QColorGroup& cg,
						 const QRect& cr, bool selected)
{
    QColorGroup g(cg);
	g.setColor( QColorGroup::Base, color_);
	QTableItem :: paint(p, g, cr, selected);
}





