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

#ifndef  __QTCANVAS_H
#define  __QTCANVAS_H

#include "../../kernel/TeDatabase.h"
#include "../../kernel/TeRaster.h"

#include "../../kernel/TeDefines.h"

#include <qpainter.h>
#include <qbrush.h> 
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qprinter.h>
#include <qscrollview.h>   
#include <qpopupmenu.h>

class TeQtProgress;

#include <string>
using namespace std;


class TL_DLL TeQtCanvas : public QScrollView
{
   Q_OBJECT

public:
    enum CursorMode { Pointer, Area, Pan, Hand, SizeVer, SizeBDiag,
		 UpArrow, Edit, Distance, Information, ZoomIn, ZoomOut };

	// Constructor
	TeQtCanvas(QWidget *parent=0, const char *name=0);

	// Destructor
	~TeQtCanvas();

	// Set the initial position for the area cursor
	void initCursorArea (QPoint p);

	// Set the cursor type
	void setMode (CursorMode);

	// Get the cursor type
	CursorMode getCursorMode() { return cursorMode_;}

	// Get the cursor box in world coordinates
	TeBox getCursorBox (){ return TeBox (xul_,ylr_,xlr_,yul_); }

	// Copy the pixmap contents to the viewport
	void copyPixmapToWindow();

	// Set the dimensions of the paint device, according the box 
	// given in world coordinates
	bool setWorld(TeBox b, int w = 0, int h = 0, QPaintDevice *pd = 0);

	// Set the dimensions of the paint device, creating or recreating the
	// pixmaps accordingly
	virtual bool setView(int w = 0, int h = 0, QPaintDevice* pd = 0);

	// Adjust the box in world coordinates if the extend flag is true or set
	// the canvas dimensions if the extend flag is false
	void setTransformation(double xmin, double xmax, double ymin, double ymax, bool extend = true);

	// Get the canvas box in world coordinates
	TeBox getWorld() {return wc_ ;}

	// Get the data box in world coordinates
	TeBox getDataWorld();

	// Clear the canvas viewport
	void clear();

	// Clear the viewport area corresponding to the given box 
	void clear(TeBox box);

	// Clear the viewport and all the pixmaps
	void clearAll();

	// Clear the area of the viewport and the area of all of the
	// pixmaps corresponding to the given box 
	void clearAll(TeBox box);

	// Get the position of the viewport relative to the total canvas area
	QPoint offset() {return QPoint(contentsX(), contentsY());}

	// Map the given point in data world coordinates to 
	// the corresponding point in the viewport
	QPoint mapDWtoV(const TeCoord2D& dw);

	// Map the given box in data world coordinates to 
	// the corresponding box in the viewport
	void mapDWtoV(TeBox& box);

	// Map the points of a line in data world coordinates to
	// the corresponding points in the viewport
	QPointArray mapDWtoV(const TeLine2D&);

	// Map the value in data world coordinates to 
	// the corresponding value in the viewport
	int mapDWtoV(double);

	// Map the value in pixels in the viewport to 
	// the corresponding value in the canvas world
	double mapVtoCW(int);

	// Map the point in the viewport to the 
	// corresponding point in the canvas world
	TeCoord2D mapVtoCW(const QPoint&);

	// Map the box in the viewport to the 
	// corresponding box in the canvas world
	void mapVtoCW(TeBox& box);

	// Map the value in pixels in the viewport to 
	// the corresponding value in the data world
	double mapVtoDW(int);

	// Map the point in the viewport to the 
	// corresponding point in the data world
	TeCoord2D mapVtoDW(const QPoint&);

	// Map the box in the canvas world coordinates to 
	// the corresponding box in the data world coordinates
	void mapCWtoDW(TeBox& box);

	// Map the box in the data world coordinates to 
	// the corresponding box in the canvas world coordinates
	void mapDWtoCW(TeBox& box);

	// Map the box in the canvas world coordinates to 
	// the corresponding box in the viewport
	void mapCWtoV(TeBox& box);

	// Map the point in the canvas world coordinates to 
	// the corresponding point in the viewport
	QPoint mapCWtoV(const TeCoord2D& c);
	 
// Pixel size in world coordinates
	double pixelSize () { return (xmax_-xmin_)/(double)width_; }

// Set the projection of the next incoming data
	void  setDataProjection ( TeProjection* proj );

// Set the projection
	void  setProjection ( TeProjection* proj);

// Retrieve the canvas projection
	TeProjection* projection() { return canvasProjection_; }

// Retrieve flag that compare canvas projection and data projection
	bool canvasAndDataProjectionEqual() {return canvasAndDataProjectionEqual_;};

// Plotting primitives

	QPainter* getPainter () {return &painter_;}
	void plotOnWindow ();
	void plotOnPixmap0 ();
	void plotOnPixmap1 ();
	void plotOnPixmap2 ();
	virtual void copyPixmapToWindow(QPixmap*, int, int, int, int);
	virtual void copyPixmap0ToPrinter();
	virtual void copyPixmap0To(QPaintDevice*);
	virtual void copyPixmap0ToWindow();
	virtual void copyPixmap1ToWindow();
	virtual void copyPixmap0ToWindow(int, int, int, int);
	virtual void copyPixmap0ToPixmap1();
	virtual void copyPixmap0ToPixmap1(int, int, int, int);
	virtual void copyPixmap1ToPixmap0();
	virtual void copyPixmap1ToPixmap0(int, int, int, int);
	virtual void copyPixmap1ToPixmap2();
	virtual void copyPixmap1ToPixmap2(int, int, int, int);
	virtual void copyPixmap2ToPixmap1();
	virtual void copyPixmap2ToPixmap1(int, int, int, int);
	virtual void copyPanArea(int, int);

	void plotPoint (const TeCoord2D &p);
	void plotPoint (const TePoint &p){ plotPoint(p.location()); }

	void setPointColor (int r, int g, int b);
	void setPointStyle (int s, int w = 3);
	void setPointPixmap (char* p);

	void plotCell (TeCell &s, const bool& restoreBackground = false);

	QRect getLegendRect (QPoint p, const QPixmap* pix, string tx);
	void plotLegend (QPoint p, const QPixmap* pix, string tx);

	void plotPolygon (const TePolygon &p, const bool& restoreBackground = false);
	void setPolygonColor (int r, int g, int b);
	void setPolygonStyle (int s, int width = 1);
	void setPolygonLineColor (int r, int g, int b);
	void setPolygonLineStyle (int s, int width = 1);

	void plotLine (TeLine2D &l);
	void plotArrow(QPointArray a);
	void plotMiddleLineArrow(QPoint ptBegin, QPoint ptEnd);
	void plotMiddleLineArrow(const TeLine2D &lne, QPointArray a);
	void setLineColor (int r, int g, int b);
	void setLineStyle (int s, int w = 1);
	QPointArray	getArrow(QPoint ptBegin, QPoint ptEnd, double size);

	void plotNode (TeNode &n);
	void setNodeColor (int r, int g, int b);
	void setNodeStyle (int s, int w = 4);
	void setNodePixmap (char* p);

	void plotArc (TeArc &a);
	void setArcColor (int r, int g, int b);
	void setArcStyle ( int s, int w = 1 );

	void plotPie (double x, double y, double w, double h, double a, double alen);
	void setPieColor (int r, int g, int b);
	void plotRect (double x, double y, double w, double h, int transp=0, bool legend=false);
	void plotRect (QRect&);
	void setRectColor (int r, int g, int b);

	void plotText (const TeCoord2D &p, const string &str, double angle = 0., double alignh = 0., double alignv = 0.);
	void plotText (const TeText &tx, const TeVisual& visual);
	QRect textRect (const TeText &tx, TeVisual visual);
	void plotTextRects (TeText &tx, TeVisual visual);
	void setTextColor (int r, int g, int b);
	void setTextSize (int size);
	void setTextStyle (string& family, int size, bool bold, bool italic);
	void textExtent ( string &str, int &w, int &h, double angle = 0.);
	void plotXorPolyline (QPointArray& PA, bool cdev=true);
	void plotXorTextDistance(vector<QPointArray> xorPointArrayVec, double unitConv, string unit);

	void setDB (TeDatabase* db) { db_ = db; }
	void plotRaster (TeRaster* raster, TeRasterTransform* transf, TeQtProgress *progress);
	void clearRaster();
	void endPrinting();

	QPixmap* getPixmap0 () {return pixmap0_; }
	QPixmap* getPixmap1 () {return pixmap1_; }
	QPixmap* getPixmap2 () {return pixmap2_; }

	void plotGraphicScale(TeVisual& visual, double offsetX, double offsetY, double unitConv=1, const string& dunit="");

	void setClipRegion(int x, int y, int w, int h);
	void setClipRegion(QRegion region);
	void setClipping (bool enable);

	TeRasterParams& getParams()
	{	return params_; }

	double scaleApx() {return scaleApx_;}
	void scaleApx(double);

	double printerFactor() {return printerFactor_;}
	void widthRef2PrinterFactor(int w) {widthRef2PrinterFactor_ = w;}
	void plotLegend (const QPoint& p, TeLegendEntry* leg, const string& tx);
	void plotLegend (const QPoint& p, const TeColor& cor, const string& tx);
	bool locateGraphicScale(const QPoint& p);
	QRect& graphicScaleRect() { return graphicScaleRect_;}
	void numberOfPixmaps(int n) {numberOfPixmaps_ = n;}
	int numberOfPixmaps() {return numberOfPixmaps_;}
	QPopupMenu* popupCanvas() { return popupCanvas_; }


signals:
	void mousePressed (TeCoord2D&, int, QPoint&);
	void mouseDoublePressed (TeCoord2D&, int, QPoint&);
	void mouseRightButtonPressed (TeCoord2D&, int, QPoint&);
	void mouseMoved (TeCoord2D&, int, QPoint&);
	void mouseReleased (TeCoord2D&, int, QPoint&);
	void mouseLeave ();
	void paintEvent ();
	void popupCanvasSignal(QMouseEvent*);
	void keyPressed(QKeyEvent*);
	void keyReleased(QKeyEvent*);
	void zoomArea();

protected:

	TeDatabase*	db_;

	QPainter	painter_;
	QImage		qimage_;
	QPopupMenu* popupCanvas_;
	QPixmap*	pixmap0_;
	QPixmap*	pixmap1_;
	QPixmap*	pixmap2_;
	int			numberOfPixmaps_;

	TeProjection*	canvasProjection_;
	TeProjection*	dataProjection_;

	TeRaster*		backRaster_;
	TeRasterParams	params_;

	double	printerFactor_;
	int		widthRef2PrinterFactor_;
	QRect	graphicScaleRect_;
	bool	canvasAndDataProjectionEqual_;
	
	// Sizes and transformations
	TeBox	wc_;
	double 	xmin_;
	double 	xmax_;
	double 	ymin_;
	double 	ymax_;	
	int		x0_;
	int		y0_;
	int		width_;
	int		height_;
	double	f_;
	double	scaleApx_;
	int		scale_;
	int		lx1_;
	int		ly1_;
	int		lx2_;
	int		ly2_;

// Area Cursor section
	CursorMode cursorMode_;
	bool	down_;
	double 	xul_; // cursor world coordinates
	double 	xlr_;
	double 	yul_;
	double 	ylr_;
	int 	ixul_; // cursor screen coordinates
	int 	ixlr_;
	int 	iyul_;
	int 	iylr_;

// Pie
	QBrush		pieBrush_;
	QColor		pieColor_;
	QBrush		rectBrush_;
	QColor		rectColor_;
	
// Polygon
	int			polygonTransparency_;
	QBrush		polygonBrush_;
	QPen		polygonPen_;
	QColor		polygonColor_;
	map<TePolyBasicType, Qt::BrushStyle> brushStyleMap_;
	
// Line
	QPen		linePen_;
	QColor		lineColor_;
	map<TeLnBasicType, Qt::PenStyle> penStyleMap_;
	TeLnBasicType	_penStyle;
	
// Point
	int			pointStyle_;
	int			pointSize_;
	QPen		pointPen_;
	QColor		pointColor_;
	QPixmap*	pointPixmap_;

// Arc
	QPen		arcPen_;
	QColor		arcColor_;
	
// Node
	int			nodeStyle_;
	int			nodeSize_;
	QPen		nodePen_;
	QColor		nodeColor_;
	QPixmap*	nodePixmap_;

// Text
	QPen		textPen_;
	QColor		textColor_;
	QFont		textFont_;
	int			textSize_;

// Draw Representations
	void drawPolygonRep (QPoint p, int w, int h, TeVisual& v);
	void drawLineRep (QPoint p, TeVisual& v);
	void drawPointRep (QPoint p, TeVisual& v);
	void plotMark(QPoint &p, int s, int w);

// Mouse events
	void contentsMousePressEvent( QMouseEvent* );
	void contentsMouseMoveEvent ( QMouseEvent* );
	void contentsMouseReleaseEvent ( QMouseEvent* );
	void contentsMouseDoubleClickEvent ( QMouseEvent* );
	void contentsContextMenuEvent( QContextMenuEvent* );

// Leave Event
	void leaveEvent(QEvent*);

// Key events
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);

// Other events
	void viewportPaintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent*);

// Correct scrolling when there are scrollbars
	void correctScrolling(QPoint &p);

// Build Raster
	bool buildRaster();
};

#endif

