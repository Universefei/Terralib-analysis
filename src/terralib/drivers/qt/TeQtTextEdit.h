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

#ifndef  __TERRALIB_INTERNAL_QTTEXTEDIT_H
#define  __TERRALIB_INTERNAL_QTTEXTEDIT_H

#include "../../kernel/TeCoord2D.h"
#include "TeQtCanvas.h"
#include "../../kernel/TeTheme.h"

#include "../../kernel/TeDefines.h"

#include <qpoint.h>
#include <set>

class TL_DLL TeQtTextEdit
{
	QPoint				pLeft_;
	QPoint				pRight_;
	QPoint				pTop_;
	QPoint				pBottom_;
	QPoint				pCenter_;
	bool				edit_;
	int					mode_;
	TeQtCanvas*			canvas_;
	TeTheme*			theme_;
	TeVisual			visual_;
	TeText				text_;

public:

    TeQtTextEdit();
    TeQtTextEdit(TeText& tx, TeVisual v, TeQtCanvas* canvas);

	~TeQtTextEdit ();

	void init(TeText& tx, TeVisual v, TeQtCanvas* canvas);

	void change();

	void change(QPoint p, bool apxAngle=false);

	int	apx(double d);

	int located(QPoint& p);

	bool isHotPoint(QPoint& p);

	void endEdit();

	int mode();

	void mode(int m);

	bool edit();

	TeText text();

	void text(TeText t);

	void height(double h);

	void angle(double a);

	void addPosition(TeCoord2D p);

	TeQtCanvas*	canvas();

	TeVisual	visual() {return visual_;}

	void	visual(const TeVisual& visual) {visual_ = visual;}

	QRect	getRect();

	QRect	getHRect();
};


class TL_DLL TeQtMultiTextEdit
{
	vector<TeQtTextEdit> textVec_;
	set<int>			 geomIdSet_;
	int					 current_;
	bool				 selMode_;

public:

    TeQtMultiTextEdit();
	~TeQtMultiTextEdit();

	int size();

	void push(TeQtTextEdit t);

	void change();

	void change(QPoint p, bool apxAngle=false);

	int located(QPoint& p);

	bool isHotPoint(QPoint& p);

	void endEdit();

	int mode();

	void mode(int m);

	bool selMode();

	void selMode(bool m);

	bool edit();

	TeText text(int i);

	TeVisual visual(int i);

	void visual(const TeVisual&);

	void height(double h);

	void angle(double a);

	void addPosition(TeCoord2D p);

	void clear() {textVec_.clear(); geomIdSet_.clear();}

	QRect	getRect();

	int getTextIndex(QPoint p);

	void updateText(int index, string t);
};
#endif
