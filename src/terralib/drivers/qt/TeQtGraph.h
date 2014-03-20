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

#ifndef  __TERRALIB_INTERNAL_QTGRAPH_H
#define  __TERRALIB_INTERNAL_QTGRAPH_H

#include <qwidget.h> 
#include <qpainter.h>
#include <qpixmap.h>
#include "../../kernel/TeGraph.h"
//#include <>

class TL_DLL TeQtGraphPlot : public TeGraphPlot
{
public :
	TeQtGraphPlot (QWidget *parent=0, const char *name=0);
	~TeQtGraphPlot ();
	virtual void plot ();
	void plotRectangle (int x, int y, int dx, int dy);
	void value_to_pixel(double px, double py, int* x, int* y);
	int	pwidth_;
	int	pheight_;
	int	 getWidth ();
	int	 getHeight ();
	QPixmap* getPixmap() {return pixmap_;}
	QWidget* widget_;
	void plotOnWindow();
	void plotOnPixmap();
	void copyPixmapToWindow();
	void copyPixmapToWindow(int, int, int, int);
	void copyPixmapToPrinter();
   	void setLineAttributes (TeColor c, int , int );
	void setFillAreaColor (TeColor c);
	void setTextColor (TeColor c);
	void fillRectangle (int x, int y, int dx, int dy);
	void drawRectangle (int x, int y, int dx, int dy);
	void drawLine (int sx, int sy, int dx, int dy);
	void drawText (int x, int y, char*t, double angle = 0);
	void fillArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c);
	void drawArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c);
	void textExtent ( string &str, int &w, int &h, double angle = 0.);
	void resize(int, int);

private :
	QPainter	painter_;
	QPixmap*	pixmap_;
	QPen		linePen_;
	QColor		lineColor_;
	QBrush		polygonBrush_;
	QColor		polygonColor_;
	QPen		textPen_;
	map<TePolyBasicType, Qt::BrushStyle> brushStyleMap_;
	map<TeLnBasicType, Qt::PenStyle> penStyleMap_;
};

class TL_DLL TePlotWidget : public TeQtGraphPlot {
public:

// Initialization
	TePlotWidget(QWidget *parent=0, const char *name=0);
	~TePlotWidget() {}
	void erase (int order = 0);
	void plot ();
};

#endif
