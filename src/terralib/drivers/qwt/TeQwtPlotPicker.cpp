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

#include "TeQwtPlotPicker.h"
#include <qcursor.h>

TeQwtPlotPicker::TeQwtPlotPicker(QwtPlotCanvas *c)
	: QwtPlotPicker(c)
{
	rect_ = QRect(0, 0, 10, 10);
	restoreArea_ = false;
	show_ = false;
	zoomActive_ = false;
	mode_ = scatter;
	histoCursorH_ = 5;
	QPen pen(Qt::red);
	painter_.begin(c);
	QRect r = c->frameRect();
	painter_.setPen(pen);
	c->setCursor(QCursor(Qt::ArrowCursor));
	setTrackerMode(QwtPlotPicker::AlwaysOn);
}

TeQwtPlotPicker::TeQwtPlotPicker(int xAxis, int yAxis, QwtPlotCanvas *c)
	: QwtPlotPicker(xAxis, yAxis, c)
{
	rect_ = QRect(0, 0, 10, 10);
	restoreArea_ = false;
	show_ = false;
	zoomActive_ = false;
	mode_ = scatter;
	histoCursorH_ = 5;
	QPen pen(Qt::red);
	painter_.begin(c);
	QRect r = c->frameRect();
	painter_.setPen(pen);
	c->setCursor(QCursor(Qt::ArrowCursor));
	setTrackerMode(QwtPlotPicker::AlwaysOn);
}

TeQwtPlotPicker::TeQwtPlotPicker(int xAxis, int yAxis, int selectionFlags, DisplayMode trackerMode, QwtPlotCanvas *c)
	: QwtPlotPicker(xAxis, yAxis, selectionFlags, NoRubberBand, trackerMode, c)
{
	rect_ = QRect(0, 0, 10, 10);
	restoreArea_ = false;
	show_ = false;
	zoomActive_ = false;
	mode_ = scatter;
	histoCursorH_ = 5;
	QPen pen(Qt::red);
	painter_.begin(c);
	QRect r = c->frameRect();
	painter_.setPen(pen);
	c->setCursor(QCursor(Qt::ArrowCursor));
}

void TeQwtPlotPicker::widgetMousePressEvent(QMouseEvent *e)
{
	if(show_ == false)
		return;

	p_ = e->pos();

	QwtDoubleRect dr = invTransform(rect_);

	int buttom = e->button();
	int state = e->state();
		
	emit mousePressedSignal(buttom, state, dr);
}

void TeQwtPlotPicker::widgetMouseReleaseEvent(QMouseEvent *e)
{
	if(show_ == false)
		return;

	p_ = e->pos();
}

void TeQwtPlotPicker::widgetMouseMoveEvent(QMouseEvent *e)
{
	const int cursor = canvas()->cursor().shape();

	if(zoomActive_)
	{
		if(cursor != (int)Qt::CrossCursor)
			canvas()->setCursor(QCursor(Qt::CrossCursor));
	}
	else
	{
		if(cursor != (int)Qt::ArrowCursor)
			canvas()->setCursor(QCursor(Qt::ArrowCursor));
	}

	if(show_ == false)
		return;

	int d = histoCursorH_;

	QPixmap *pixmap = canvas()->paintCache();

	if(restoreArea_ && pixmap)
	{
		if(mode_ == scatter)
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height(), Qt::CopyROP);
		else
		{
			int y = rect_.center().y();
			rect_.setRect(rect_.left(), y-d, rect_.width(), d+d);
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height()+1, Qt::CopyROP);
		}
	}

//#ifdef WIN32
	if(e->state() == Qt::AltButton)
//#else
//	if(e->state() == (Qt::AltButton | Qt::ControlButton))
//#endif
	{
		if(p_ != e->pos())
		{
			int dx = e->pos().x() - p_.x();
			int dy = e->pos().y() - p_.y();
			rect_.setWidth(rect_.width() + dx);
			if(rect_.width() < 3)
				rect_.setWidth(3);

			rect_.setHeight(rect_.height() + dy);
			if(rect_.height() < 3)
				rect_.setHeight(3);

			int y = rect_.center().y();
			if(mode_ == histog)
				rect_.setRect(rect_.left(), y-d, rect_.width(), d+d);
		}
	}

	p_ = e->pos();
	if(mode_ == histog)
		rect_.setRect(rect_.left(), p_.y()-d, rect_.width(), d+d);

	rect_.moveCenter(p_);
	QRect rc = canvas()->rect();
	rc.setRect(2, 2, rc.right()-3, rc.bottom()-3);
	QRect rec = rc.intersect(rect_);
	painter_.setClipping(false);
	if(mode_ == scatter)
		painter_.drawRect(rec);
	else
	{
		int xi = rec.left();
		int xf = rec.right();
		int cy = rec.center().y();
		painter_.drawLine(xi, cy, xf, cy);
		painter_.drawLine(xi, cy-d, xi, cy+d);
		painter_.drawLine(xf, cy-d, xf, cy+d);		
	}
	restoreArea_ = true;
}

void TeQwtPlotPicker::show(bool b)
{
	int d = histoCursorH_;

	QPixmap *pixmap = canvas()->paintCache();
	if(show_ && restoreArea_ && pixmap)
	{
		if(mode_ == scatter)
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height(), Qt::CopyROP);
		else
		{
			int y = rect_.center().y();
			rect_.setRect(rect_.left(), y-d, rect_.width(), d+d);
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height()+1, Qt::CopyROP);
		}
	}

	restoreArea_ = false;
	show_ = b;

	if(b)
		canvas()->setCursor(QCursor(Qt::ArrowCursor));
	else
		canvas()->setCursor(QCursor(Qt::crossCursor));
	setEnabled(b);
}

void TeQwtPlotPicker::mode(int m)
{
	int d = histoCursorH_;

	QPixmap *pixmap = canvas()->paintCache();
	if(restoreArea_ && show_ && pixmap)
	{
		if(mode_ == scatter)
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height(), Qt::CopyROP);
		else
		{
			int y = rect_.center().y();
			rect_.setRect(rect_.left(), y-d, rect_.width(), d+d);
			bitBlt(canvas(), rect_.left(), rect_.top(), pixmap, rect_.left()-2, rect_.top()-2, rect_.width(), rect_.height()+1, Qt::CopyROP);
		}
	}

	restoreArea_ = false;
	mode_ = m;
}
