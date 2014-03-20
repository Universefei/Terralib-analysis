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
#include "TeQtTable.h"
#include <qmessagebox.h>
#include <qapplication.h>
#include "../../kernel/TeDatabase.h"

TeQtTable::TeQtTable(QWidget* parent, QString name)
	: QTable(parent, name)
{
	gridX_ = -1;
	gridY_ = -1;
	iLine_ = -1;
	fLine_ = -1;
	iCol_ = -1;
	fCol_ = -1;
	gridWidth_  = -1;
	gridHeight_ = -1;
	portal_ = 0;
	db_ = 0;
	doRepaint_ = true;

	setNumCols(0);
	setNumRows(0);
	setSelectionMode(QTable::NoSelection);
}

TeQtTable::~TeQtTable()
{
	if (portal_)
	{
		delete portal_;
		portal_ = 0;
	}
}


void TeQtTable::openTable (TeDatabase* db, string& table)
{
	int i, numRows, nCols;

	doRepaint_ = false; // don�t repaint

	if (table.empty() || db == 0)
		return;

	db_ = db;

	if (portal_)
		delete portal_;
	portal_= db_->getPortal();

	string s = "SELECT COUNT(*) FROM " + table;
	if (portal_->query(s) && portal_->fetchRow())
		numRows = atoi(portal_->getData(0));

	s = "SELECT * FROM " + table;
	portal_->freeResult();
	if (!portal_->query(s))
	{
		delete portal_;
		portal_ = 0;
		return;
	}

	TeAttributeList& attrList = portal_->getAttributeList();
	
	//get the column names
	nCols = attrList.size();
	setNumRows(numRows);
	setNumCols(nCols);

	for (i = 0; i < nCols; ++i)
		horizontalHeader()->setLabel(i, attrList[i].rep_.name_.c_str());

	// Set the minimum width of the vertical header of the grid
	char buf[10];
	sprintf(buf, "%d", numRows);
	int minw = strlen(buf) + 1;
	if (minw < 3)
		minw = 3;
	verticalHeader()->setMinimumWidth(minw*9); // supposing char width of 9 pixels
	setLeftMargin(minw*9);


	doRepaint_ = true; // do repaint

	int visibleCols = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=0; i<numCols(); i++)
	{
		if(visibleWidth() < columnPos(i) - scVal)
			break;
		visibleCols++;
	}

	repaint();
}


void TeQtTable::openSql(TeDatabase* db, string& sql)
{
	int i, numRows, nCols;

	doRepaint_ = false; // don�t repaint

	if (sql.empty() || db == 0)
		return;

	db_ = db;

	if (portal_)
		delete portal_;
	portal_= db_->getPortal();

	string s = sql;

	if (!portal_->query(s))
	{
		delete portal_;
		portal_ = 0;
		return;
	}

	TeAttributeList& attrList = portal_->getAttributeList();
	numRows = portal_->numRows();

	if(numRows <= 0)
	{
		numRows = 0;
		while (portal_->fetchRow())
			++numRows;
	}
	
	//get the column names
	nCols = attrList.size();
	setNumRows(numRows);
	setNumCols(nCols);

	for (i = 0; i < nCols; ++i)
		horizontalHeader()->setLabel(i, attrList[i].rep_.name_.c_str());

	// Set the minimum width of the vertical header of the grid
	char buf[10];
	sprintf(buf, "%d", numRows);
	int minw = strlen(buf) + 1;
	if (minw < 3)
		minw = 3;
	verticalHeader()->setMinimumWidth(minw*9); // supposing char width of 9 pixels
	setLeftMargin(minw*9);


	doRepaint_ = true; // do repaint

	int visibleCols = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=0; i<numCols(); i++)
	{
		if(visibleWidth() < columnPos(i) - scVal)
			break;
		visibleCols++;
	}

	repaint();
}

void TeQtTable::freePortal()
{
	gridX_ = -1;
	gridY_ = -1;
	iLine_ = -1;
	fLine_ = -1;
	iCol_ = -1;
	fCol_ = -1;
	gridWidth_  = -1;
	gridHeight_ = -1;
	portal_ = 0;
	db_ = 0;
	doRepaint_ = true;

	if (portal_)
	{
		delete portal_;
		portal_ = 0;
	}

	setNumRows(0);
	setNumCols(0);
}

void TeQtTable::deletePortal()
{
	if (portal_)
		delete portal_;
	portal_ = 0;
}

void TeQtTable::drawContents (QPainter *qp, int clipx, int clipy, int clipw, int cliph)
{
	if(doRepaint_ == false)
		return;

	update();
	QTable :: drawContents(qp, clipx, clipy, clipw, cliph);
}

void TeQtTable::update ()
{
	if(portal_ == 0)
		return;

	int	i, j;

	int gridY = contentsY();
	int gridX = contentsX();

	int gridHeight = visibleHeight(); 
	int gridWidth = visibleWidth(); 

	if (gridX_ == gridX && gridY_ == gridY &&
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
	int visibleLines = rowAt(gridHeight_);
	if (visibleLines == -1)
		visibleLines = numRows();

	int visibleCols = 0;
	int scVal = horizontalScrollBar()->value();
	for(i=iCol_; i<numCols(); i++)
	{
		if(gridWidth_ < columnPos(i) - scVal)
			break;
		visibleCols++;
	}

	if (iLine_ < 0 || iCol_ < 0)
	{
		gridX_ = gridY_ = iLine_ = fLine_ = iCol_ = fCol_ = -1;
		doRepaint_ = true;
		return;
	}

	fLine_ = iLine_ + visibleLines;
	if (fLine_ > numRows() - 1)
		fLine_ = numRows() - 1;
	fCol_ = iCol_ + visibleCols - 1;

	for (i = iLine_; i <= fLine_ ; ++i)
	{
		if (!portal_->fetchRow(i))
			return;

		string s;
		for (j = iCol_; j <= fCol_; ++j)
		{
			s = portal_->getData(j);
			setText(i, j, s.c_str());

			if(i == fLine_ && j == fCol_- 1)
			{
				doRepaint_ = true; // do repaint
				updateCell(i, j);
			}
		}
	}

	doRepaint_ = true; // do repaint
}





