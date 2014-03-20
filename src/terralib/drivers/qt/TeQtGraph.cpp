/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.
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

#include "TeQtGraph.h"
#include "../../kernel/TeCoord2D.h"
#include <qpaintdevicemetrics.h> 
#include <qprinter.h>

TeQtGraphPlot::TeQtGraphPlot (QWidget *parent, const char * /* name */)
{
	QPaintDevice* dev = painter_.device();
	if(dev)
		painter_.end();

// Build a new pixmap
	widget_ = parent;
    QPaintDeviceMetrics pdm( widget_ );
	pixmap_ = new QPixmap (pdm.width(), pdm.height());
	pixmap_->fill(widget_->paletteBackgroundColor());

	if(dev)
		painter_.end();
	painter_.begin(pixmap_);

	pwidth_ = pdm.width();
	pheight_ = pdm.height();

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

	QString family;
#ifdef WIN32
	family = "Verdana";
#else
	family = "Sans";
#endif
	QFont font(family);
	font.setPointSize(10);
	font.setBold(true);
	painter_.setFont(font);
}

TeQtGraphPlot::~TeQtGraphPlot ()
{
	if(painter_.device())
		painter_.end();

	delete pixmap_;
}

void TeQtGraphPlot::resize (int w, int h)
{
	if(painter_.device())
		painter_.end();
	delete pixmap_;

	pwidth_ = w;
	pheight_ = h;
	pixmap_ = new QPixmap (w, h);
	pixmap_->fill(widget_->paletteBackgroundColor());
	painter_.begin(pixmap_);
}

void TeQtGraphPlot::plot ()
{
	plotOnPixmap();
	polygonBrush_ = QBrush(Qt::white);
	fillRectangle (0, 0, pwidth_, pheight_);
	plotCurves ();
	copyPixmapToWindow();
}

int TeQtGraphPlot::getWidth ()
{
    return pwidth_;
}

int	TeQtGraphPlot::getHeight ()
{
    return pheight_;
}

void TeQtGraphPlot::setLineAttributes(TeColor c, int /* s */, int w)
{
	QColor cor(c.red_, c.green_, c.blue_);
	linePen_.setColor (cor);
	linePen_.setStyle(penStyleMap_[(TeLnBasicType)TeLnTypeContinuous]);
	linePen_.setWidth(w);
}

void TeQtGraphPlot::setFillAreaColor (TeColor c)
{
	QColor cor(c.red_, c.green_, c.blue_);
	polygonBrush_.setColor (cor);
	polygonBrush_.setStyle(brushStyleMap_[(TePolyBasicType)TePolyTypeFill]);
}

void TeQtGraphPlot::setTextColor (TeColor c)
{
	QColor cor(c.red_, c.green_, c.blue_);
	textPen_.setColor (cor);
}

void TeQtGraphPlot::fillRectangle (int x, int y, int dx, int dy)
{
	painter_.fillRect (x,y,dx,dy,polygonBrush_);
}

void TeQtGraphPlot::drawRectangle (int x, int y, int dx, int dy)
{
	painter_.setPen(linePen_);
	painter_.moveTo(x,y);
	painter_.lineTo(x+dx,y);
	painter_.lineTo(x+dx,y+dy);
	painter_.lineTo(x,y+dy);
	painter_.lineTo(x,y);
}

void TeQtGraphPlot::drawLine (int sx, int sy, int dx, int dy)
{
	painter_.setPen(linePen_);
	painter_.drawLine (sx,sy,dx,dy);
}

void TeQtGraphPlot::drawText (int x, int y, char* t, double angle)
{
	painter_.setPen(textPen_);

	if(angle != 0)
	{
		painter_.save();
		painter_.translate(x, y);
		painter_.rotate(-angle);
		painter_.drawText(0, 0, t);
		painter_.restore();
	}
	else
		painter_.drawText(x, y, t);
}

void TeQtGraphPlot::fillArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c)
{
	QColor cor(c.red_, c.green_, c.blue_);
	painter_.setBrush(cor);
	painter_.drawPie (xc,yc,w,h,(int)(a1*16),(int)((a2-a1)*16));
}

void TeQtGraphPlot::drawArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c)
{
	QColor cor(c.red_, c.green_, c.blue_);
	painter_.setBrush(cor);
	painter_.drawArc (xc,yc,w,h,(int)(a1*16),(int)((a2-a1)*16));
}

void TeQtGraphPlot::copyPixmapToWindow ()
{
	if (pixmap_)
		bitBlt (widget_,0,0,pixmap_,0,0,pwidth_,pheight_,Qt::CopyROP,true);
}

void TeQtGraphPlot::copyPixmapToWindow(int ulx, int uly, int w, int h)
{
	if (pixmap_)
		bitBlt (widget_,ulx,uly,pixmap_,ulx,uly,w,h,Qt::CopyROP,true);
}

void TeQtGraphPlot::plotOnWindow ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == widget_)
		return;
	if(dev)
		painter_.end();
	painter_.begin(widget_);
}

void TeQtGraphPlot::plotOnPixmap ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == pixmap_)
		return;
	if(dev)
		painter_.end();
	painter_.begin(pixmap_);
}

TePlotWidget::TePlotWidget(QWidget *parent, const char *name):TeQtGraphPlot(parent,name)
{
	widget_ = parent;
}

void TePlotWidget::plot ()
{
	TeQtGraphPlot::plot ();
}


void TePlotWidget::erase( int order)
{
	kill (order);
	plot ();
}

void TeQtGraphPlot::plotRectangle (int x, int y, int dx, int dy)
{
	painter_.setPen(linePen_);
	painter_.moveTo(x,y);
	painter_.lineTo(x+dx,y);
	painter_.lineTo(x+dx,y+dy);
	painter_.lineTo(x,y+dy);
	painter_.lineTo(x,y);
}

void TeQtGraphPlot::value_to_pixel(double px, double py, int* x, int* y)
{
	TeGraphPlot::value_to_pixel(px, py, x, y);
}

void TeQtGraphPlot::copyPixmapToPrinter()
{
	if (pixmap_)
	{
		QPrinter printer;
		if (printer.setup())
		{
			QPainter p(&printer);

			QPaintDevice* dev = painter_.device();
			if(dev == pixmap_)
				painter_.end();

			p.drawPixmap(0, 0, *pixmap_);
			p.flush();
			p.end();

			if(dev && painter_.device() == 0)
				painter_.begin(dev);
		}
	}
}

void TeQtGraphPlot::textExtent (string &str, int &w, int &h, double /* angle */)
{
	QFont font = painter_.font();
	QFontMetrics fm(font);

	w = fm.width(str.c_str());
	h = fm.height();

	//?????????????????? fazer codigo......
}
