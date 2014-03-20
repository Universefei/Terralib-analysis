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

#ifndef TEQTTABLE_H
#define TEQTTABLE_H

#include "../../kernel/TeDefines.h"

#include <qtable.h>
#include <string>
using namespace std;

class TeDatabase;
class TeDatabasePortal;

class TL_DLL TeQtTable : public QTable
{
protected:
	TeDatabase			*db_;
	TeDatabasePortal	*portal_;
	int					gridX_, gridY_;
	int					iLine_, fLine_;
	int					iCol_, fCol_;
	int					gridWidth_, gridHeight_;
	bool				doRepaint_;

	virtual void drawContents (QPainter *qp, int clipx, int clipy, int clipw, int cliph);

	void update();

public:

    TeQtTable(QWidget *parent, QString name);

	virtual ~TeQtTable();

	virtual void endEdit(int row, int col, bool accept, bool replace)
	{
		QTable::endEdit(row, col, accept, replace);
	}

	void openTable(TeDatabase* db, string& table);

	void openSql(TeDatabase* db, string& sql);

	void freePortal();

	void deletePortal();

	TeDatabasePortal* getPortal() {return portal_;}
};

#endif
