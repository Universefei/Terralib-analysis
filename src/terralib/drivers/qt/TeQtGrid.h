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

#ifndef  __TERRALIB_INTERNAL_QTGRID_H
#define  __TERRALIB_INTERNAL_QTGRID_H

#include <qwidget.h>
#include <qtable.h>
#include <qcolor.h> 
#include <qlabel.h>
#include <qpopupmenu.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "../../kernel/TeDatabase.h"
#include "TeAppTheme.h"
#include "TeApplicationUtils.h"

#include "../../kernel/TeDefines.h"

using namespace std;

class TL_DLL TeQtGrid : public QTable
{
	Q_OBJECT

protected:
	TeDatabase				*db_;
	TeAppTheme		*theme_;
	TeDatabasePortal		*portal_;
	string					sqlJoin_;
	string					orderBy_;
	string					sql_;
	string					CT_;
	string					CTE_;
	vector<string>			indexVector_;
	vector<TeTable>			tableVector_;
	bool					editing_;
	bool					doRefresh_;
	bool					doRepaint_;
	bool					shiftKeyPressed_;
	bool					showGridInformation_;
	int						uniqueIdPos_;
	int						objectIdPos_;
	int						resultIdPos_;
	int						gridStatusPos_;
	int						gridX_, gridY_;
	int						iLine_, fLine_;
	int						iCol_, fCol_;
	int						gridWidth_, gridHeight_;
	int						visibleLines_, visibleCols_;
	long					lastLinePressed_;
	long					dragLineReleased_;
	map<string, ObjectInfo>	objectMap_;
	map<string, string>		unique2ObjectMap_;
	vector<int>				gridToPortalRowVector_;
	vector<int>				gridToPortalColVector_;
	vector<bool>			gridColVisVector_;
	vector<string>			portalRow2UniqueIdVector_;
	int						rowToEdit_;
	int						colToEdit_;
	bool					verticalEdition_;
	QPopupMenu				*popupHorizHeader_;
	QPopupMenu				*popupVertHeader_;
	QPopupMenu				*popupViewport_;
	bool					columnDragEnabled_;
	bool					mousePressedOnHorizontalHeader_;

	virtual bool eventFilter(QObject*, QEvent*);

	virtual void drawContents (QPainter *qp, int clipx, int clipy,
							   int clipw, int cliph);

	//void sort(vector<int> cols, string order);

	bool writeCell();

	void editNextRow();

	void editNextColumn();
	
	void update();

	void goToEditNewCell();

	virtual void endEdit(int row, int col, bool accept, bool replace);

	void arrangeColumns(TeAttrTableVector cTableVec);
	
public:

	bool autoPromote_;

	TeQtGrid (QWidget* parent = 0, const char* name = 0);

	~TeQtGrid();

	void init(TeAppTheme *theme);

	void initPortal();

	void updatePortalContents();

	QPopupMenu* popupHorizHeader()
		{ return popupHorizHeader_; }

	QPopupMenu* popupVertHeader()
		{ return popupVertHeader_; }

	QPopupMenu* popupViewport()
		{ return popupViewport_; }

	TeDatabasePortal* getPortal()
		{ return portal_; }
	TeAppTheme* getTheme() 
    {  return theme_;} 


	bool isIndex(int col);

	bool isDateTimeRegistered(int col);

	bool isSortBy(int col);

	bool isExternalTable(int col);

	TeTable& findTable(int col);

	TeTable& findTable(string colName);

	void adjustColumns();

	void swapColumns(int col1 = -1, int col2 = -1);

	int	getColumn(int col) {return gridToPortalColVector_[col];}

	void setVisColumn (bool vis, int col=-1);

	bool getVisColumn (int col);

	void putColorOnGrid(set<string> uniqueIdSet, set<string> objectIdSet, string operation, string attrQuery = "");

	void setVerticalEdition(bool flag);

	void refresh();

	void clear();

	void clearPortal();

	string	getObject(int row);

	bool isVerticalEdition() { return verticalEdition_; }

	bool showGridInformation() { return showGridInformation_;}

	void showGridInformation(bool b) { showGridInformation_ = b;}

	void gridInformation ();

	void insertObjectIntoCollection(string newId, string newTId = "");

	void goToLastLine();

	void goToLastColumn();

	bool hasPointedLine();

	void deletePointedLines();

	vector<int> getSelectedColumns();

	void arrangeGridToPortalRowVector(int col);

public slots:
	void mouseReleasedOnVerticalHeaderSlot(int lin);

	void promotePointedObjectsSlot();

	void promoteQueriedObjectsSlot();

	void sortAscOrderSlot();

	void sortDescOrderSlot();

	void sort(vector<int> cols, string order);

	void removeQueryColorSlot();

	void removePointingColorSlot();

	void scrollToPrevPointedObjectSlot();

	void scrollToNextPointedObjectSlot();

	void scrollToPrevQueriedObjectSlot();

	void scrollToNextQueriedObjectSlot();

	void invertSelectionSlot();

protected slots:
	void columnWidthChanged(int col);

signals:
	void putColorOnObject(TeAppTheme*, set<string>&);
	void horizHeaderSignal(int);
	void vertHeaderSignal(int);
	void viewportSignal();
	void vertHeaderLeftDblClickSignal(int);
	void linkTable(string, string, string);
	void gridChangedSignal();
	void displayGridInformation(QString);
	void clearGridInformation();
};




class TL_DLL TeQtGridItem : public QTableItem
{
	QColor color_;

public:
    TeQtGridItem(QTable *t, EditType et, const QString &txt)
		: QTableItem( t, et, txt )
	{}

	void setColor(QColor& c)
	{ color_.setRgb(c.red(), c.green(), c.blue()); }

    void paint(QPainter *p, const QColorGroup &cg,
		       const QRect &cr, bool selected );
};

#endif
