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

#ifndef TEQWTPLOTPICKER_H
#define TEQWTPLOTPICKER_H

#include "../../kernel/TeDefines.h"

#include <qwt_plot_picker.h>
#include <qpainter.h>
#include <qpixmap.h>

class TL_DLL TeQwtPlotPicker:public QwtPlotPicker
{
    Q_OBJECT

public:

	enum CursorMode
    {
        scatter,
        histog
	};

	TeQwtPlotPicker(QwtPlotCanvas *c);
	TeQwtPlotPicker(int xAxis, int yAxis, QwtPlotCanvas *c);
	TeQwtPlotPicker(int xAxis, int yAxis, int selectionFlags, DisplayMode trackerMode, QwtPlotCanvas *c);

	virtual ~TeQwtPlotPicker() {}

	void setRect(const QRect& r) {rect_ = r;}
	QRect& getRect() {return rect_;}
	void setPen(QPen p) {painter_.setPen(p);}
	void show(bool b);
	void mode(int m);
	void zoomActive(bool b) {zoomActive_ = b;}

protected:

    void widgetMousePressEvent(QMouseEvent *);
    void widgetMouseReleaseEvent(QMouseEvent *);
    void widgetMouseMoveEvent(QMouseEvent *); 

	QRect rect_;
    QPoint p_;
	QPainter painter_;
	bool restoreArea_;
	bool show_;
	bool zoomActive_;
	int mode_;
	int histoCursorH_;

signals:
	void mousePressedSignal(int, int, QwtDoubleRect);
};
#endif
