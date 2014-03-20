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

#include "TeQwtPlotZoomer.h"
#include "TeQwtPlot.h"
#include "../../kernel/TeUtils.h"
#include <qcursor.h>
#include <math.h>

TeQwtPlotZoomer::TeQwtPlotZoomer(QwtPlotCanvas *c)
	:QwtPlotZoomer(c)
{
}

TeQwtPlotZoomer::TeQwtPlotZoomer(int xAxis, int yAxis, QwtPlotCanvas *c)
	:QwtPlotZoomer(xAxis, yAxis, c)
{
}

TeQwtPlotZoomer::TeQwtPlotZoomer(int xAxis, int yAxis, int selectionFlags,
        DisplayMode trackerMode, QwtPlotCanvas *c)
    :QwtPlotZoomer(xAxis, yAxis, selectionFlags, trackerMode, c)
{
}

TeQwtPlotZoomer::~TeQwtPlotZoomer()
{
}
    
void TeQwtPlotZoomer::widgetKeyPressEvent(QKeyEvent *e)
{
	int key = e->key();
	uint i = zoomRectIndex();
	if(i > 0)
	{
		QwtDoubleRect zr = zoomStack()[0];
		QwtDoubleRect r = zoomStack()[i];
		QwtDoublePoint c = r.center();
		QwtDoublePoint dh(0., r.height()/10.);
		QwtDoublePoint dw(r.width()/10., 0.);

		if(key == Qt::Key_Left)
			c -= dw;
		else if(key == Qt::Key_Right)
			c += dw;
		else if(key == Qt::Key_Up) // inverted
			c += dh;
		else if(key == Qt::Key_Down) // inverted
			c -= dh;

		QwtDoubleRect rr = r;
		r.moveCenter(c);
		if(zr.contains(r) == false)
		{
			if(key == Qt::Key_Left)
				r.setRect(zr.left(), rr.top(), rr.width(), rr.height());
			else if(key == Qt::Key_Right)
				r.setRect(zr.right()-rr.width(), rr.top(), rr.width(), rr.height());
			else if(key == Qt::Key_Up) // inverted
				r.setRect(rr.left(), zr.bottom()-rr.height(), rr.width(), rr.height());
			else if(key == Qt::Key_Down) // inverted
				r.setRect(rr.left(), zr.top(), rr.width(), rr.height());
		}

		zoom(r);
		emit enableNextZoomSignal(false);
	}

	QwtPlotZoomer::widgetKeyPressEvent(e);
	if(key == Qt::Key_Escape)
		emit enablePreviousZoomSignal(false);
}

void TeQwtPlotZoomer::backward()
{
	uint i = zoomRectIndex();
	if (i > 0)
		zoom(-1);
}

void TeQwtPlotZoomer::forward()
{
	uint i = zoomRectIndex();
	if (i < zoomStack().size() - 1 )
		zoom(+1);
}


void TeQwtPlotZoomer::setEnabledSlot(bool b)
{
	QwtPlotZoomer::setEnabled(b);

	QCursor cursor;
	if (b)
		cursor.setShape(Qt::CrossCursor);
	else
		cursor.setShape(Qt::ArrowCursor);

	canvas()->setCursor(cursor);
}

void TeQwtPlotZoomer::widgetMouseReleaseEvent(QMouseEvent* e)
{
	QwtPlotZoomer::widgetMouseReleaseEvent(e);
	emit enableNextZoomSignal(false);

	uint i = zoomRectIndex();
	if(i == 1)
		emit enablePreviousZoomSignal(true);	
}

QwtDoubleRect TeQwtPlotZoomer::adjustRect(const QwtDoubleRect& r)
{
	QwtDoubleRect rec;
	double dx = r.width();
	double ldx = log10(dx);
	int idx = (int)ldx;
	double dy = r.height();
	double ldy = log10(dy);
	int idy = (int)ldy;
	
	if(ldx < 0)
		idx--;
	if(ldy < 0)
		idy--;

	double k = pow(10., double(idx));
	double v = r.left();
	int a = TeRound(v / k);
	double v2 = (double)a * k;
	if(v2 > v)
	{
		a = TeRound(v / k - .5);
		v2 = (double)a * k;
	}
	rec.setLeft(v2);

	v = r.right();
	a = TeRound(v / k);
	v2 = (double)a * k;
	if(v2 < v)
	{
		a = TeRound(v / k + .5);
		v2 = (double)a * k;
	}
	rec.setRight(v2);

	k = pow(10., double(idy));
	v = r.bottom();
	a = TeRound(v / k);
	v2 = (double)a * k;
	if(v2 < v)
	{
		a = TeRound(v / k + .5);
		v2 = (double)a * k;
	}
	rec.setBottom(v2);

	v = r.top();
	a = TeRound(v / k);
	v2 = (double)a * k;
	if(v2 > v)
	{
		a = TeRound(v / k - .5);
		v2 = (double)a * k;
	}
	rec.setTop(v2);

	return rec;
}




