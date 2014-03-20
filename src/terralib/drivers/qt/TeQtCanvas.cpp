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

#include "TeQtCanvas.h"
#include "../../kernel/TeDecoderMemory.h"
#include "TeQtProgress.h"
#include "../../kernel/TeVectorRemap.h"
#include "../../kernel/TeGeometryAlgorithms.h"
#include "../../kernel/TeDefines.h"
#include "../../kernel/TeRasterRemap.h"
#include "TeDecoderQtImage.h"

#include <qpaintdevice.h> 
#include <qpaintdevicemetrics.h> 
#include <qapplication.h>

#include <string>	
#include <algorithm>	
#include <time.h>


TeQtCanvas::TeQtCanvas(QWidget *parent, const char *name)
			: QScrollView(parent,name,WNorthWestGravity)
{
	pixmap0_ = 0;
	pixmap1_ = 0;
	pixmap2_ = 0;
	backRaster_ = 0;
	printerFactor_ = 1.;
	numberOfPixmaps_ = 3;

	canvasProjection_ = 0;
	dataProjection_ = 0;
	pointStyle_ = 0;
	nodeStyle_ = 0;
	pointSize_ = 3;
	nodeSize_ = 4;
	db_ = 0;

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

	setVScrollBarMode(QScrollView::Auto);
	setHScrollBarMode(QScrollView::Auto);
    viewport()->setMouseTracking(true);
    viewport()->setFocusPolicy(StrongFocus);
    viewport()->setBackgroundColor(backgroundColor());
	setMargin(0);

	width_ = viewport()->width();
	height_ = viewport()->height();
	popupCanvas_ = new QPopupMenu(this);
	_penStyle=TeLnTypeContinuous;
}

TeQtCanvas::~TeQtCanvas()
{
	if (dataProjection_)
		delete dataProjection_;
	if (canvasProjection_)
		delete canvasProjection_;

	if (backRaster_)
		delete backRaster_;
	backRaster_ = 0;

	QPaintDevice* dev = painter_.device();
	if(dev)
		painter_.end();

	if(pixmap0_)
		delete pixmap0_;
	pixmap0_ = 0;

	if(pixmap1_)
		delete pixmap1_;
	pixmap1_ = 0;

	if(pixmap2_)
		delete pixmap2_;
	pixmap2_ = 0;
}

void TeQtCanvas::setDataProjection ( TeProjection* proj )
{	
	if(dataProjection_)
		delete dataProjection_;

	if (proj == 0)
		return;
	dataProjection_ = TeProjectionFactory::make(proj->params());

	canvasAndDataProjectionEqual_ = true;
	if (canvasProjection_ && dataProjection_ && !(*canvasProjection_== *dataProjection_))
	{
		canvasAndDataProjectionEqual_ = false;
		dataProjection_->setDestinationProjection(canvasProjection_);
		canvasProjection_->setDestinationProjection(dataProjection_);
	}
}

void TeQtCanvas::setProjection ( TeProjection* proj )
{	
	if (canvasProjection_)
		delete canvasProjection_;
	canvasProjection_ = TeProjectionFactory::make(proj->params()); 
	params_.projection(canvasProjection_);

	canvasAndDataProjectionEqual_ = true;
	if(canvasProjection_ && dataProjection_ && !(*canvasProjection_== *dataProjection_))
		canvasAndDataProjectionEqual_ = false;
}

void TeQtCanvas::plotOnWindow ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == viewport())
		return;
	if(dev)
		painter_.end();
	painter_.begin(viewport());
}

void TeQtCanvas::plotOnPixmap0 ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == pixmap0_)
		return;
	if(dev)
		painter_.end();
	painter_.begin(pixmap0_);
}

void TeQtCanvas::plotOnPixmap1 ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == pixmap1_)
		return;
	if(dev)
		painter_.end();
	painter_.begin(pixmap1_);
}

void TeQtCanvas::plotOnPixmap2 ()
{
	QPaintDevice* dev = painter_.device();
	if(dev == pixmap2_)
		return;
	if(dev)
		painter_.end();
	painter_.begin(pixmap2_);
}

bool TeQtCanvas::setWorld(TeBox b, int w, int h, QPaintDevice *pd)
{
	xmin_= b.x1();
	xmax_= b.x2();
	ymin_= b.y1();
	ymax_= b.y2();
	if(setView(w, h, pd) == false)
		return false;
	bool extend = pd ? false : true;
	setTransformation(xmin_, xmax_, ymin_, ymax_, extend);
	params_.nBands(3);
	double resx = pixelSize();
	double resy = pixelSize();
	TeBox box(getWorld());
	params_.lowerLeftResolutionSize(box.x1_+0.5*resx,box.y1_+0.5*resy,resx,resy,width_,height_);
	params_.projection();
	params_.mode_ = 'w';
	params_.decoderIdentifier_ = "";
	return true;
}

void TeQtCanvas::setTransformation(double xmin, double xmax, double ymin, double ymax, bool extend)
{
	if (painter_.device() == 0)
		return;
	
	xmin_= xmin;
	xmax_= xmax;
	ymin_= ymin;
	ymax_= ymax;

	double	dxw = xmax_ - xmin_;
	double	dyw = ymax_ - ymin_;
	double	dxv = width_;
	double	dyv = height_;
					
	double	fx = dxv/dxw;
	double	fy = dyv/dyw;

	if (fx > fy)
	{
		f_ = fy;
		if (extend)
		{
			dxw = width_/f_;
			xmax_ = xmin_ + dxw;
		}
		else
			width_ = (int)(f_*dxw + .5);
	}
	else
	{
		f_ = fx;
		if (extend)
		{
			dyw = height_/f_;
			ymax_ = ymin_ + dyw;
		}
		else
			height_ = (int)(f_*dyw + .5);
	}

	wc_ = TeBox (xmin_,ymin_,xmax_,ymax_);
	params_.boundingBoxResolution(xmin_,ymin_,xmax_,ymax_,params_.resx_,params_.resy_);

	QPaintDeviceMetrics devMetric(viewport());
	double wpixels = devMetric.width();
	double wMM = devMetric.widthMM();
	double wT = wMM;
	if(canvasProjection_)
	{
		string unit = TeConvertToUpperCase(canvasProjection_->units());
		if(unit == "METERS")
			wT = wMM / 1000.;
		else if(unit == "KILOMETERS")
			wT = wMM / 1000000.;
		else if(unit == "CENTIMETERS")
			wT = wMM / 100.;
		else if(unit == "MILLIMETERS")
			wT = wMM;
		else if(unit == "FEET")
			wT = wMM / (12. * 25.4);
		else if(unit == "INCHES")
			wT = wMM / 25.4;
		else if(unit == "DECIMALDEGREES")
			wT = wMM / 110000000.;
	}
	double wp = wT / wpixels;
	scaleApx_ = (1. / f_) / wp;
}

void TeQtCanvas::scaleApx(double scale)
{
	QPaintDeviceMetrics devMetric(viewport());
	double wMM = devMetric.widthMM();
	double wT = wMM;
	if(canvasProjection_)
	{
		string unit = TeConvertToUpperCase(canvasProjection_->units());
		if(unit == "METERS")
			wT = wMM / 1000.;
		else if(unit == "KILOMETERS")
			wT = wMM / 1000000.;
		else if(unit == "CENTIMETERS")
			wT = wMM / 100.;
		else if(unit == "MILLIMETERS")
			wT = wMM;
		else if(unit == "FEET")
			wT = wMM / (12. * 25.4);
		else if(unit == "INCHES")
			wT = wMM / 25.4;
		else if(unit == "DECIMALDEGREES")
			wT = wMM / 110000000.;
	}

	double ff = scale / scaleApx_;
	double xmin, ymin, xmax, ymax;
	if(ff < 1)
	{
		double dx = (wc_.width() - (wc_.width() * ff)) / 2.;
		double dy = (wc_.height() - (wc_.height() * ff)) / 2.;
		xmin = xmin_ + dx;
		ymin = ymin_ + dy;
		xmax = xmax_ - dx;
		ymax = ymax_ - dy;
	}
	else
	{
		double dx = ((wc_.width() * ff) - wc_.width()) / 2.;
		double dy = ((wc_.height() * ff) - wc_.height()) / 2.;
		xmin = xmin_ - dx;
		ymin = ymin_ - dy;
		xmax = xmax_ + dx;
		ymax = ymax_ + dy;
	}

	TeBox box(xmin, ymin, xmax, ymax);
	setWorld(box, contentsWidth(), contentsHeight());
}

double TeQtCanvas::mapVtoCW(int pixels)
{
	TeBox wbox = getDataWorld();
	TeCoord2D wpc(wbox.x1_ + wbox.width()/2., wbox.y1_ + wbox.height()/2.);
	QPoint pc = mapDWtoV(wpc);
	QPoint qp(pc.x()+pixels, pc.y());
	TeCoord2D wp = mapVtoCW(qp);
	wpc = mapVtoCW(pc);
	double d = fabs(wp.x() - wpc.x());
	return d;
}

double TeQtCanvas::mapVtoDW(int pixels)
{
	TeBox wbox = getDataWorld();
	TeCoord2D	wpc(wbox.x1_ + wbox.width()/2., wbox.y1_ + wbox.height()/2.);
	QPoint pc = mapDWtoV(wpc);
	QPoint qp(pc.x()+pixels, pc.y());
	TeCoord2D	wp = mapVtoDW(qp);
	wpc = mapVtoDW(pc);
	double d = fabs(wp.x() - wpc.x());
	return d;
}

TeCoord2D TeQtCanvas::mapVtoCW(const QPoint& p)
{
	TeCoord2D w((p.x()-x0_)/f_ + xmin_,
		(height_ - y0_ - p.y())/f_ + ymin_);
	return w;
}

TeCoord2D TeQtCanvas::mapVtoDW(const QPoint& v)
{
	TeCoord2D w((v.x()-x0_)/f_ + xmin_,
		(height_ - y0_ - v.y())/f_ + ymin_);

	if(canvasAndDataProjectionEqual_ == false)
	{
		canvasProjection_->setDestinationProjection(dataProjection_);
		dataProjection_->setDestinationProjection(canvasProjection_);
		w = canvasProjection_->PC2LL (w);
		w = dataProjection_->LL2PC (w);
	}
	return w;
}


TeBox TeQtCanvas::getDataWorld()
{
	if(canvasAndDataProjectionEqual_ == false)
	{
		TeBox b = TeRemapBox(wc_, canvasProjection_, dataProjection_);
		return b;
	}
	return wc_ ;
}

QPoint TeQtCanvas::mapDWtoV(const TeCoord2D& dw)
{
	TeCoord2D w = dw;
	if(canvasAndDataProjectionEqual_ == false)
	{
		canvasProjection_->setDestinationProjection(dataProjection_);
		dataProjection_->setDestinationProjection(canvasProjection_);
		w = dataProjection_->PC2LL (w);
		w = canvasProjection_->LL2PC (w);
	}

	QPoint v((int)((w.x() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((w.y() - ymin_)*f_ + 0.5));
	correctScrolling (v);
	return v;
}

QPointArray TeQtCanvas::mapDWtoV(const TeLine2D& line)
{
 	int i;
	int np = line.size();
 	QPointArray parray(np);
	TeCoord2D p;

	if(canvasAndDataProjectionEqual_ == false)
	{
		canvasProjection_->setDestinationProjection(dataProjection_);
		dataProjection_->setDestinationProjection(canvasProjection_);
 		for (i = 0 ; i < np; ++i)
 		{
			p = line[i];
			p = dataProjection_->PC2LL(p);
			p = canvasProjection_->LL2PC(p);
			QPoint v((int)((p.x() - xmin_)*f_ + 0.5)+x0_,
				height_ - y0_ - (int)((p.y() - ymin_)*f_ + 0.5));
			correctScrolling (v);
 			parray.setPoint(i, v);
 		}
	}
	else
	{
 		for (i = 0 ; i < np; i++)
 		{
			p = line[i];
			QPoint v((int)((p.x() - xmin_)*f_ + 0.5)+x0_,
				height_ - y0_ - (int)((p.y() - ymin_)*f_ + 0.5));
			correctScrolling (v);
 			parray.setPoint(i, v);
 		}
	}
	return parray;
}

void TeQtCanvas::mapDWtoV(TeBox& w)
{
	TeCoord2D ll, ur;
	if(canvasAndDataProjectionEqual_ == false)
		w = TeRemapBox (w, dataProjection_, canvasProjection_);

	ll = w.lowerLeft();
	ur = w.upperRight();

	QPoint p1((int)((ll.x() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((ll.y() - ymin_)*f_ + 0.5));
	correctScrolling (p1);

	QPoint p2((int)((ur.x() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((ur.y() - ymin_)*f_ + 0.5));
	correctScrolling (p2);

	w.x1_ = p1.x();
	w.y1_ = p2.y();
	w.x2_ = p2.x();
	w.y2_ = p1.y();

	//TeCoord2D ll, lr, ur, ul;
	//if(canvasAndDataProjectionEqual_ == false)
	//	TeBox TeRemapBox ( TeBox& box, TeProjection* projFrom, TeProjection* projTo)

	//{
	//	canvasProjection_->setDestinationProjection(dataProjection_);
	//	dataProjection_->setDestinationProjection(canvasProjection_);
	//	ll = dataProjection_->PC2LL (w.lowerLeft());
	//	lr.x(w.upperRight().x());
	//	lr.y(w.lowerLeft().y());
	//	lr = dataProjection_->PC2LL (lr);
	//	ur = dataProjection_->PC2LL (w.upperRight());
	//	ul.x(w.lowerLeft().x());
	//	ul.y(w.upperRight().y());
	//	ul = dataProjection_->PC2LL (ul);

	//	ll.x(std::min(ll.x(),ul.x()));
	//	ll.y(std::min(ll.y(),lr.y()));
	//	lr.x(std::max(lr.x(),ur.x()));
	//	lr.y(std::min(ll.y(),lr.y()));
	//	ur.x(std::max(ur.x(),lr.x()));
	//	ur.y(std::max(ul.y(),ur.y()));
	//	ul.x(std::min(ll.x(),ul.x()));
	//	ul.y(std::max(ul.y(),ur.y()));

	//	ll = canvasProjection_->LL2PC (ll);
	//	lr = canvasProjection_->LL2PC (lr);
	//	ur = canvasProjection_->LL2PC (ur);
	//	ul = canvasProjection_->LL2PC (ul);
	//	ll.x(std::min(ll.x(),ul.x()));
	//	ll.y(std::min(ll.y(),lr.y()));
	//	ur.x(std::max(ur.x(),lr.x()));
	//	ur.y(std::max(ul.y(),ur.y()));
	//}

	//QPoint p1((int)((ll.x() - xmin_)*f_ + 0.5)+x0_,
	//	height_ - y0_ - (int)((ll.y() - ymin_)*f_ + 0.5));
	//correctScrolling (p1);

	//QPoint p2((int)((ur.x() - xmin_)*f_ + 0.5)+x0_,
	//	height_ - y0_ - (int)((ur.y() - ymin_)*f_ + 0.5));
	//correctScrolling (p2);

	//w.x1_ = p1.x();
	//w.y1_ = p2.y();
	//w.x2_ = p2.x();
	//w.y2_ = p1.y();
}

void TeQtCanvas::mapCWtoDW(TeBox& box)
{
	TeCoord2D ll, ur;
	if(canvasAndDataProjectionEqual_ == false)
		box = TeRemapBox (box, canvasProjection_, dataProjection_);

	//TeCoord2D boxll = box.lowerLeft();
	//TeCoord2D boxur = box.upperRight();

	//if(canvasAndDataProjectionEqual_ == false)
	//{
	//	canvasProjection_->setDestinationProjection(dataProjection_);
	//	boxll = canvasProjection_->PC2LL(boxll);
	//	boxll = dataProjection_->LL2PC(boxll);
	//	boxur = canvasProjection_->PC2LL(boxur);
	//	boxur = dataProjection_->LL2PC(boxur);
	//}

	//box = TeBox(boxll, boxur);
}

void TeQtCanvas::mapDWtoCW(TeBox& box)
{
	TeCoord2D ll, ur;
	if(canvasAndDataProjectionEqual_ == false)
		box = TeRemapBox (box, dataProjection_, canvasProjection_);

	//TeCoord2D boxll = box.lowerLeft();
	//TeCoord2D boxur = box.upperRight();

	//if(canvasAndDataProjectionEqual_ == false)
	//{
	//	dataProjection_->setDestinationProjection(canvasProjection_);
	//	boxll = dataProjection_->PC2LL(boxll);
	//	boxll = canvasProjection_->LL2PC(boxll);
	//	boxur = dataProjection_->PC2LL(boxur);
	//	boxur = canvasProjection_->LL2PC(boxur);
	//}

	//box = TeBox(boxll, boxur);
}

void TeQtCanvas::mapCWtoV(TeBox& box)
{
	QPoint p1((int)((box.x1() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((box.y1() - ymin_)*f_ + 0.5));
	correctScrolling (p1);

	QPoint p2((int)((box.x2() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((box.y2() - ymin_)*f_ + 0.5));
	correctScrolling (p2);

// swap y value
	box.x1_ = p1.x();
	box.y1_ = p2.y();
	box.x2_ = p2.x();
	box.y2_ = p1.y();
}

void TeQtCanvas::mapVtoCW(TeBox& box)
{
	// swap y value
	TeCoord2D w1((box.x1()-x0_)/f_ + xmin_,
		(height_ - y0_ - box.y2())/f_ + ymin_);

	TeCoord2D w2((box.x2()-x0_)/f_ + xmin_,
		(height_ - y0_ - box.y1())/f_ + ymin_);

	box = TeBox(w1, w2);
}

QPoint TeQtCanvas::mapCWtoV(const TeCoord2D& c)
{
	QPoint p((int)((c.x() - xmin_)*f_ + 0.5)+x0_,
		height_ - y0_ - (int)((c.y() - ymin_)*f_ + 0.5));
	correctScrolling (p);

	return p;
}

void TeQtCanvas::setPolygonColor (int r, int g, int b)
{
	QRgb cor = qRgba(r, g, b, 50);
	polygonColor_.setRgb(cor);
	polygonBrush_.setColor (polygonColor_);
}

void TeQtCanvas::setPolygonStyle (int s, int t)
{
	polygonBrush_.setStyle(brushStyleMap_[(TePolyBasicType)s]);
	polygonTransparency_ = t;
}

void TeQtCanvas::setPolygonLineColor (int r, int g, int b)
{
	QColor cor(r, g, b);
	polygonPen_.setColor (cor);
}

void TeQtCanvas::setPolygonLineStyle (int s, int w)
{
	polygonPen_.setStyle(penStyleMap_[(TeLnBasicType)s]);
	polygonPen_.setWidth(w);
}

void TeQtCanvas::setLineColor (int r, int g, int b)
{
	lineColor_.setRgb(r,g,b);
	linePen_.setColor (lineColor_);
}

void TeQtCanvas::setLineStyle (int s, int w)
{
	_penStyle=(TeLnBasicType)s;
	if((_penStyle == TeLnTypeArrow) || (_penStyle == TeLnTypeMiddleArrow) )		linePen_.setStyle(Qt::SolidLine);
	else																		linePen_.setStyle(penStyleMap_[(TeLnBasicType)s]);
	linePen_.setWidth(w);
}

void TeQtCanvas::setTextColor (int r, int g, int b)
{
	textColor_.setRgb(r,g,b);
	textPen_.setColor (textColor_);
}

void TeQtCanvas::setTextStyle (string& family, int size, bool bold, bool italic )
{
	textFont_.setFamily (family.c_str());
	textFont_.setBold (bold);
	textFont_.setItalic (italic);
	if(size <= 0)
		textSize_ = 1;
	else
		textSize_ = size;

	if(pixmap0_ && pixmap1_ && (pixmap0_->rect() != pixmap1_->rect()))
		textSize_ = (int)((double)textSize_ * printerFactor_ + .5);

	textFont_.setPointSize (textSize_);
}

void TeQtCanvas::setTextSize (int size)
{
	if(size <= 0)
		textSize_ = 1;
	else
		textSize_ = size;

	if(pixmap0_ && pixmap1_ && (pixmap0_->rect() != pixmap1_->rect()))
		textSize_ = (int)((double)textSize_ * printerFactor_ + .5);

	textFont_.setPointSize (textSize_);
}


void TeQtCanvas::plotText (const TeCoord2D &pt, const string &str, double angle, double /*alignh*/, double /*alignv*/)
{
	painter_.setPen(textPen_);
	painter_.setFont(textFont_);
	QPoint p = mapDWtoV (pt);
	if (angle != 0.)
	{
		painter_.save ();
		painter_.translate (p.x(),p.y());
		painter_.rotate (-angle);
		painter_.drawText (0,-4,QString(str.c_str()));
		painter_.restore ();
	}
	else
		painter_.drawText (p,QString(str.c_str()));
}

void TeQtCanvas::plotXorTextDistance(vector<QPointArray> xorPointArrayVec, double unitConv, string unit)
{
	if(xorPointArrayVec.empty())
		return;

	QPoint a = xorPointArrayVec[0].point(0);
	QPoint b = xorPointArrayVec[0].point(1);
	TeCoord2D ppa = mapVtoCW(a);
	TeCoord2D ppb = mapVtoCW(b);
	double distance = sqrt((ppa.x()-ppb.x()) * (ppa.x()-ppb.x()) + (ppa.y()-ppb.y()) * (ppa.y()-ppb.y()));
	string s;
	if(unit == "Decimal Degrees")
			s = Te2String(distance/unitConv,6) + "(" + unit + ")";
	else
			s = Te2String(distance/unitConv,1) + "(" + unit + ")";

	double dy = (double)(b.y() - a.y());
	double dx = (double)(b.x() - a.x());
	distance = sqrt(dx * dx + dy * dy);
	double txHeight = -7.;
	double dd = distance / 2.;
	double alfa = atan(dy/dx);
	if(dx<0 && dy<0)
		alfa = alfa - TePI;
	else if(dy>0 && dx<0)
		alfa = TePI + alfa;
	double beta = atan(txHeight/dd);
	double c = dd * cos(beta);

	double gama = alfa + beta;
	if((dx<0 && dy<0) || (dy>0 && dx<0))
		gama = alfa - beta;

	double x = c * cos(gama);
	x += a.x();
	double y = c * sin(gama);
	y += a.y();
	QPoint p((int)x, (int)y);
	alfa = -alfa * 180. / TePI;
	if((dx<0 && dy<0) || (dy>0 && dx<0))
		alfa = 180. + alfa;
			
	QPen pen(QColor(239, 111, 63));
	QFont font("Helvetica", 12);
	font.setStyleStrategy(QFont::PreferAntialias);
	font.setBold(true);
	QFontMetrics fm(font);
	QRect rect = fm.boundingRect(s.c_str());
	int xp = - rect.width()/2;
	int yp = - rect.height()/2;

	plotOnPixmap0();
	painter_.setRasterOp (Qt::XorROP);
	painter_.setPen(pen);
	painter_.setFont(font);
	if (alfa != 0.)
	{
		painter_.save ();
		painter_.translate (p.x(),p.y());
		painter_.rotate (-alfa);
		painter_.drawText(xp, yp, rect.width(), rect.height(), Qt::AlignLeft|Qt::DontClip, s.c_str());
		painter_.restore ();
	}
	else
	{
		xp = p.x() - rect.width()/2;
		yp = p.y() - rect.height()/2;
		painter_.drawText((int)x, (int)y, rect.width(), rect.height(), Qt::AlignLeft|Qt::DontClip, s.c_str());
	}

	plotOnWindow();
	painter_.setRasterOp (Qt::XorROP);
	painter_.setPen(pen);
	painter_.setFont(font);
	painter_.save ();
	painter_.translate (-offset().x(), -offset().y());
	if (alfa != 0.)
	{
		painter_.translate (p.x(),p.y());
		painter_.rotate (-alfa);
		painter_.drawText(xp, yp, rect.width(), rect.height(), Qt::AlignLeft|Qt::DontClip, s.c_str());
	}
	else
	{
		xp = p.x() - rect.width()/2;
		yp = p.y() - rect.height()/2;
		painter_.drawText(xp, yp, rect.width(), rect.height(), Qt::AlignLeft|Qt::DontClip, s.c_str());
	}
	painter_.restore ();
}

void TeQtCanvas::plotText (const TeText& tx, const TeVisual& visual)
{
	if(tx.textValue().empty())
		return;

	int	x, y;

	TeCoord2D pt = tx.location();
	QPoint p = mapDWtoV (pt);
	double angle = tx.angle();

	QRect brect = textRect(tx, visual);
	painter_.setFont(textFont_);
	painter_.setPen(textPen_);
	painter_.setRasterOp (Qt::CopyROP);
	string st = tx.textValue();

	if (angle != 0.)
	{
		x = - brect.width()/2;
		y = - brect.height()/2;
		painter_.save ();
		painter_.translate (p.x(), p.y());
		painter_.rotate (-angle);
		painter_.drawText(x, y, brect.width(), brect.height(), Qt::AlignLeft|Qt::DontClip, st.c_str());
		painter_.restore ();
	}
	else
	{
		x = p.x() - brect.width()/2;
		y = p.y() - brect.height()/2;
		painter_.drawText(x, y, brect.width(), brect.height(), Qt::AlignLeft|Qt::DontClip, st.c_str());
	}
}

QRect TeQtCanvas::textRect(const TeText& tx, TeVisual visual)
{
	QRect rect;
	if(tx.textValue().empty())
		return rect;

	int size;
	int fixedSize = visual.fixedSize(); // font size is fixed
	if(fixedSize == false && tx.height() > 0.)
	{
		TeBox wbox = getDataWorld();
		TeCoord2D p1(wbox.x1_, wbox.y1_);
		TeCoord2D p2(double(p1.x() + tx.height()), double(p1.y() + tx.height()));
		TeBox box(p1, p2);
		mapDWtoV(box);
		size = int(box.height());
		size = (int)((double)size / printerFactor_ + .5);
	}
	else
		size = visual.size(); // font size
	if(size == 0)
		size = 1;

	setTextSize(size);
	textFont_.setFamily (visual.family().c_str());

	TeCoord2D pt = tx.location();
	QPoint p = mapDWtoV (pt);

	string st = tx.textValue();
	QFontMetrics fm(textFont_);
	rect = fm.boundingRect(st.c_str());
	QPoint cc = rect.center();
	QPoint tr = p;
	tr = tr - cc;
	rect.setRight(rect.right()+tr.x());
	rect.setLeft(rect.left()+tr.x());
	rect.setTop(rect.top()+tr.y());
	rect.setBottom(rect.bottom()+tr.y());
	return rect;
}

void TeQtCanvas::plotXorPolyline (QPointArray& PA, bool cdev)
{
	QPoint o = offset();
	QPen pen(QColor("green"));

	plotOnPixmap0();
	painter_.save ();
	if(cdev == false)
		painter_.translate(o.x(), o.y());
	painter_.setRasterOp (Qt::XorROP);
	painter_.setPen(pen);
	painter_.drawPolyline(PA);
	painter_.restore ();

	plotOnWindow(); // do offset translation on window
	painter_.save ();
	if(cdev)
		painter_.translate(-o.x(), -o.y());
	painter_.setRasterOp (Qt::XorROP);
	painter_.setPen(pen);
	painter_.drawPolyline(PA);
	painter_.restore ();

	plotOnPixmap0();
}

void TeQtCanvas::plotTextRects (TeText& tx, TeVisual visual)
{
	if(tx.textValue().empty())
		return;

	QPoint o = offset();
	QRect rect, l, r, t, b, c;

	TeCoord2D pt = tx.location();
	plotOnPixmap0();
	QPoint p = mapDWtoV (pt);
	double angle = tx.angle();

	rect = textRect(tx, visual);
	if(angle != 0.)
	{
		rect.setTop(rect.top()-p.y());
		rect.setBottom(rect.bottom()-p.y());
		rect.setLeft(rect.left()-p.x());
		rect.setRight(rect.right()-p.x());
	}

	l = rect;
	l.setTop(rect.top()+rect.height()/2-3);
	l.setBottom(rect.top()+rect.height()/2+3);
	l.setRight(rect.left() + 6);

	r = rect;
	r.setTop(l.top());
	r.setBottom(l.bottom());
	r.setLeft(rect.right() - 6);

	t = rect;
	t.setLeft(rect.left()+rect.width()/2-3);
	t.setRight(rect.left()+rect.width()/2+3);
	t.setBottom(rect.top() + 6);

	b = rect;
	b.setLeft(t.left());
	b.setRight(t.right());
	b.setTop(rect.bottom() - 6);

	c.setLeft(rect.left()+rect.width()/2-3);
	c.setRight(c.left() + 6);
	c.setTop(rect.top()+rect.height()/2-3);
	c.setBottom(c.top() + 6);

	QPointArray parL(4);
	parL.setPoint(0, l.bottomLeft());
	parL.setPoint(1, l.bottomRight());
	parL.setPoint(2, l.topRight());
	parL.setPoint(3, l.topLeft());

	QPointArray parR(4);
	parR.setPoint(0, r.bottomRight());
	parR.setPoint(1, r.bottomLeft());
	parR.setPoint(2, r.topLeft());
	parR.setPoint(3, r.topRight());

	QPointArray parT(4);
	parT.setPoint(0, t.topLeft());
	parT.setPoint(1, t.bottomLeft());
	parT.setPoint(2, t.bottomRight());
	parT.setPoint(3, t.topRight());

	QPointArray parB(4);
	parB.setPoint(0, b.bottomLeft());
	parB.setPoint(1, b.topLeft());
	parB.setPoint(2, b.topRight());
	parB.setPoint(3, b.bottomRight());

	QBrush	brush(QColor("green"));

	if (angle != 0.)
	{
		plotOnPixmap0();
		painter_.save ();
		painter_.setRasterOp (Qt::XorROP);
		painter_.setBrush(Qt::NoBrush);
		QPen pen(QColor("green"));
		painter_.setPen(pen);
		painter_.translate (p.x(), p.y());
		painter_.rotate (-angle);
		painter_.drawRect(rect);
		painter_.drawPolyline(parL);
		painter_.drawPolyline(parR);
		painter_.drawPolyline(parT);
		painter_.drawPolyline(parB);
		painter_.fillRect(c, brush);
		painter_.restore ();

		plotOnWindow(); // do offset translation on window
		painter_.save ();
		painter_.setRasterOp (Qt::XorROP);
		painter_.setBrush(Qt::NoBrush);
		pen = QPen(QColor("green"));
		painter_.setPen(pen);
		painter_.translate (p.x()-o.x(), p.y()-o.y());
		painter_.rotate (-angle);
		painter_.drawRect(rect);
		painter_.drawPolyline(parL);
		painter_.drawPolyline(parR);
		painter_.drawPolyline(parT);
		painter_.drawPolyline(parB);
		painter_.fillRect(c, brush);
		painter_.restore ();
	}
	else
	{
		plotOnPixmap0();
		painter_.save ();
		painter_.setRasterOp (Qt::XorROP);
		painter_.setBrush(Qt::NoBrush);
		QPen pen(QColor("green"));
		painter_.setPen(pen);
		painter_.drawRect(rect);
		painter_.drawPolyline(parL);
		painter_.drawPolyline(parR);
		painter_.drawPolyline(parT);
		painter_.drawPolyline(parB);
		painter_.fillRect(c, brush);
		painter_.restore ();

		plotOnWindow(); // do offset translation on window
		painter_.save ();
		painter_.setRasterOp (Qt::XorROP);
		painter_.setBrush(Qt::NoBrush);
		pen = QPen(QColor("green"));
		painter_.setPen(pen);
		painter_.translate (-o.x(), -o.y());
		painter_.drawRect(rect);
		painter_.drawPolyline(parL);
		painter_.drawPolyline(parR);
		painter_.drawPolyline(parT);
		painter_.drawPolyline(parB);
		painter_.fillRect(c, brush);
		painter_.restore ();
	}
	plotOnPixmap0();
}

void TeQtCanvas::setArcColor (int r, int g, int b)
{
	arcColor_.setRgb(r,g,b);
}

void TeQtCanvas::setArcStyle (int s, int w )
{
	arcPen_.setStyle ((Qt::PenStyle)s);
	arcPen_.setWidth(w);
}

void TeQtCanvas::setPointColor (int r, int g, int b)
{
	pointColor_.setRgb(r,g,b);
}

void TeQtCanvas::setPointStyle (int s, int w)
{
	pointStyle_ = (TePtBasicType) s;
	pointSize_ = w;
}

void TeQtCanvas::plotPoint (const TeCoord2D &pt)
{	
	pointPen_.setColor (pointColor_);
	painter_.setPen(pointPen_);

	QPoint p = mapDWtoV (pt);

	plotMark(p,pointStyle_, pointSize_);
}

void TeQtCanvas::setNodeColor (int r, int g, int b)
{
	nodeColor_.setRgb(r,g,b);
}

void TeQtCanvas::setNodeStyle (int s, int w)
{
	nodeStyle_ = s;
	nodeSize_ = w;
}

void TeQtCanvas::plotNode (TeNode &pt)
{	
	nodePen_.setColor (nodeColor_);
	painter_.setPen(nodePen_);

	QPoint p = mapDWtoV (pt.location());
	plotMark(p,nodeStyle_, nodeSize_);
}


void TeQtCanvas::plotMark(QPoint &p, int s, int w)
{
	painter_.setPen(pointColor_);
	if (s == TePtTypePlus)
	{
		painter_.drawLine (p.x()-w/2,p.y(),p.x()+w/2,p.y());
		painter_.drawLine (p.x(),p.y()-w/2,p.x(),p.y()+w/2);
	}
	else if (s == TePtTypeStar)
	{
		painter_.save ();
		painter_.translate (p.x(),p.y());
		painter_.drawLine (0,-w/2,0,w/2);
		painter_.rotate (45);
		painter_.drawLine (0,-w/2,0,w/2);
		painter_.rotate (-90);
		painter_.drawLine (0,-w/2,0,w/2);
		painter_.restore ();
	}
	else if (s == TePtTypeCircle)
	{
		painter_.setBrush(pointColor_);
		painter_.drawChord (p.x()-w/2,p.y()-w/2,w,w,0,360*16);
	}
	else if (s == TePtTypeX)
	{
		painter_.drawLine (p.x()-w/2,p.y()-w/2,p.x()+w/2,p.y()+w/2);
		painter_.drawLine (p.x()-w/2,p.y()+w/2,p.x()+w/2,p.y()-w/2);
	}
	else if (s == TePtTypeBox)
	{
		painter_.fillRect (p.x()-w/2,p.y()-w/2,w,w,pointColor_);
	}
	else if (s == TePtTypeDiamond)
	{
		QPointArray pa(5);
		pa.setPoint(0, p.x()-w/2, p.y());
		pa.setPoint(1, p.x(), p.y()-w/2);
		pa.setPoint(2, p.x()+w/2, p.y());
		pa.setPoint(3, p.x(), p.y()+w/2);
 		pa.setPoint(4, p.x()-w/2, p.y());
		painter_.setBrush(pointColor_);
		painter_.drawPolygon(pa);
	}
	else if (s == TePtTypeHollowCircle)
	{
		painter_.drawArc (p.x()-w/2,p.y()-w/2,w,w,0,360*16);
	}
	else if (s == TePtTypeHollowBox)
	{
		painter_.setBrush(Qt::NoBrush);
		painter_.drawRect (p.x()-w/2,p.y()-w/2,w,w);
	}
	else if (s == TePtTypeHollowDiamond)
	{
		painter_.drawLine (p.x()-w/2,p.y(),p.x(),p.y()-w/2);
		painter_.drawLine (p.x(),p.y()-w/2,p.x()+w/2,p.y());
		painter_.drawLine (p.x()+w/2,p.y(),p.x(),p.y()+w/2);
		painter_.drawLine (p.x(),p.y()+w/2,p.x()-w/2,p.y());
	}
}


void TeQtCanvas::plotCell (TeCell &cell, const bool& restoreBackground)
{
	int w, h;
	TeBox b = getDataWorld ();

	if (!TeIntersects (b, cell.box ()))
		return;

	QPoint pfrom, pto;

	pfrom = mapDWtoV (cell.box().lowerLeft());
	pto   = mapDWtoV (cell.box().upperRight());

	painter_.setBrush(polygonBrush_);
	painter_.setPen (polygonPen_);

	if(!restoreBackground && 
		(polygonTransparency_ == 0 || 
		polygonTransparency_ == 100 && polygonBrush_.style() == Qt::NoBrush))
	{
		if(polygonTransparency_ == 100 && polygonBrush_.style() != Qt::NoBrush)
			painter_.setBrush(Qt::NoBrush);

		if((w = pto.x() - pfrom.x()) <= 1)
			w = 2;
		if((h = pfrom.y() - pto.y()) <= 1)
			h = 2;

		painter_.drawRect( pfrom.x(), pto.y(), w+1, h+1);
	}
	else if(polygonBrush_.style() == Qt::HorPattern ||
	polygonBrush_.style() == Qt::VerPattern ||
	polygonBrush_.style() == Qt::CrossPattern ||
	polygonBrush_.style() == Qt::BDiagPattern ||
	polygonBrush_.style() == Qt::FDiagPattern ||
	polygonBrush_.style() == Qt::DiagCrossPattern)
	{
		if((w = pto.x() - pfrom.x()) <= 1)
			w = 2;
		if((h = pfrom.y() - pto.y()) <= 1)
			h = 2;

		painter_.drawRect( pfrom.x(), pto.y(), w+1, h+1);
	}
	else
	{
		QRect devRect;
		if(painter_.device() == viewport())
			devRect = viewport()->rect();
		else
			devRect = pixmap0_->rect();

		TeBox box = cell.box();
		mapDWtoV(box); // data coordinate to viewport coordinate
		if((w = (int)box.width()) <= 3)
			w = 4;
		if((h = (int)box.height()) <= 3)
			h = 4;	

		QRect polyRect((int)box.x1_, (int)box.y1_, w+1, h+1);
		QRect interRect = devRect & polyRect;
		QPoint pOffset = interRect.topLeft();

		int width = interRect.width();
		int height = interRect.height();

		int r = width%8;
		if(r)
			width += (8-r);
		r = height%8;
		if(r)
			height += (8-r);

		QBitmap	bm;
		bm.resize(width, height);
		//Fill bitmap with 0-bits: clipping region
		bm.fill(Qt::color0);
		QPainter maskPainter(&bm);

		// Draw cell with 1-bits: drawing region
		QBrush bs(Qt::color1);
		maskPainter.setBrush(bs);
		QPen pen(Qt::color1, polygonPen_.width());
		maskPainter.setPen(pen);
		maskPainter.translate(-pOffset.x(), -pOffset.y());
 		maskPainter.drawRect( interRect );
		maskPainter.end();

		QRegion clipRegion(bm);
		clipRegion.translate(pOffset.x(), pOffset.y());

		int transp = 255 - (polygonTransparency_ * 255 / 100);
		painter_.setClipRegion(clipRegion);
		if(restoreBackground) // restore background from pixmap2_
		{
			painter_.drawPixmap(pOffset.x(), pOffset.y(), *pixmap2_, pOffset.x(),
				pOffset.y(), interRect.width(), interRect.height());
		}

		// set alpha buffer and color
		QImage img(interRect.width(), interRect.height(), 32);
		unsigned int val = (transp << 24) | (polygonColor_.red() << 16) | (polygonColor_.green() << 8) | polygonColor_.blue();
		img.fill(val);
		img.setAlphaBuffer(true);

		// plot transparency
		painter_.drawPixmap(pOffset.x(), pOffset.y(), img);


		// plot contours
		painter_.setClipping(false);
		painter_.setBrush(Qt::NoBrush);
		painter_.drawRect( interRect );
	}
}
//void TeQtCanvas::plotCell (TeCell &cell, const bool& restoreBackground)
//{
//	int w, h;
//	TeBox b = getDataWorld ();
//
//	if (!TeIntersects (b, cell.box ()))
//		return;
//
//	QPoint pfrom, pto;
//
//	pfrom = mapDWtoV (cell.box().lowerLeft());
//	pto   = mapDWtoV (cell.box().upperRight());
//
//	painter_.setBrush(polygonBrush_);
//	painter_.setPen (polygonPen_);
//
//	if((polygonTransparency_ == 0 && polygonBrush_.style() != Qt::NoBrush) || // 100% opaque or
//		(polygonTransparency_ == 100 || polygonBrush_.style() == Qt::NoBrush)) // 100% transparent
//	{
//		if(polygonTransparency_ == 100 && polygonBrush_.style() != Qt::NoBrush)
//			painter_.setBrush(Qt::NoBrush);
//
//		if((w = pto.x() - pfrom.x()) <= 1)
//			w = 2;
//		if((h = pfrom.y() - pto.y()) <= 1)
//			h = 2;
//
//		//painter_.drawRect( pfrom.x()+1, pto.y()+1, w, h);
//		painter_.drawRect( pfrom.x(), pto.y(), w+1, h+1);
//	}
//	else
//	{
//		QRect devRect;
//		if(painter_.device() == viewport())
//			devRect = viewport()->rect();
//		else
//			devRect = pixmap0_->rect();
//
//		TeBox box = cell.box();
//		mapDWtoV(box); // data coordinate to viewport coordinate
//		if((w = (int)box.width()) <= 3)
//			w = 4;
//		if((h = (int)box.height()) <= 3)
//			h = 4;	
//
//		QRect polyRect((int)box.x1_, (int)box.y1_, w+1, h+1);
//		QRect interRect = devRect & polyRect;
//		QPoint pOffset = interRect.topLeft();
//
//		int width = interRect.width();
//		int height = interRect.height();
//
//		int r = width%8;
//		if(r)
//			width += (8-r);
//		r = height%8;
//		if(r)
//			height += (8-r);
//
//		QBitmap	bm;
//		bm.resize(width, height);
//		//Fill bitmap with 0-bits: clipping region
//		bm.fill(Qt::color0);
//		QPainter maskPainter(&bm);
//
//		// Draw cell with 1-bits: drawing region
//		QBrush bs(Qt::color1, polygonBrush_.style());
//		maskPainter.setBrush(bs);
//		QPen pen(Qt::color1, polygonPen_.width());
//		maskPainter.setPen(pen);
//		maskPainter.translate(-pOffset.x(), -pOffset.y());
// 		maskPainter.drawRect( interRect );
//		maskPainter.end();
//
//		QRegion clipRegion(bm);
//		clipRegion.translate(pOffset.x(), pOffset.y());
//
//		int transp = 255 - (polygonTransparency_ * 255 / 100);
//		painter_.setClipRegion(clipRegion);
//		// restore background from pixmap2_
//		painter_.drawPixmap(pOffset.x(), pOffset.y(), *pixmap2_, pOffset.x(),
//			pOffset.y(), interRect.width(), interRect.height());
//
//		// set alpha buffer and color
//		QImage img(interRect.width(), interRect.height(), 32);
//		unsigned int val = (transp << 24) | (polygonColor_.red() << 16) | (polygonColor_.green() << 8) | polygonColor_.blue();
//		img.fill(val);
//		img.setAlphaBuffer(true);
//
//		// plot transparency
//		painter_.drawPixmap(pOffset.x(), pOffset.y(), img);
//
//
//		// plot contours
//		painter_.setClipping(false);
//		painter_.setBrush(Qt::NoBrush);
//		painter_.drawRect( interRect );
//	}
//}
void TeQtCanvas::plotPolygon (const TePolygon &poly, const bool& restoreBackground)
{
	if(canvasAndDataProjectionEqual_)
	{
		if (!TeIntersects (wc_, poly.box ()))
			return;
	}
	else
	{
		TeBox canvasBox = TeRemapBoxPC2Geodetic(wc_, canvasProjection_);
		TeBox pBox = TeRemapBoxPC2Geodetic(poly.box(), dataProjection_);

		if (!TeIntersects(canvasBox, pBox))
			return;
	}

	int		i, k;
 	QPoint	p;
	painter_.setBrush(polygonBrush_);
	painter_.setPen (polygonPen_);

	if(!restoreBackground && poly.size() == 1 && // no holes
		(polygonTransparency_ == 0 || 
		polygonBrush_.style() == Qt::NoBrush))
		//polygonTransparency_ == 100 && polygonBrush_.style() == Qt::NoBrush))
	{
		TeLinearRing ring = poly[0];
 		QPointArray parray = mapDWtoV(ring);
		painter_.drawPolygon( parray );
	}
	else if(polygonBrush_.style() == Qt::HorPattern ||
	polygonBrush_.style() == Qt::VerPattern ||
	polygonBrush_.style() == Qt::CrossPattern ||
	polygonBrush_.style() == Qt::BDiagPattern ||
	polygonBrush_.style() == Qt::FDiagPattern ||
	polygonBrush_.style() == Qt::DiagCrossPattern)
	{
		TeLinearRing ring = poly[0];
 		QPointArray parray = mapDWtoV(ring);
		painter_.drawPolygon( parray );
	}
	else
	{
		QRect devRect;
		if(painter_.device() == viewport())
			devRect = viewport()->rect();
		else
			devRect = pixmap0_->rect();

		TeBox box = poly.box();
		mapDWtoV(box); // data coordinate to viewport coordinate
		if(box.width() <= 0 || box.height() <= 0)
			return;
		int w = (int)box.width();
		int h = (int)box.height();
		if(w < 4)
			w = 4;
		if(h < 4)
			h = 4;

		QRect polyRect((int)box.x1_, (int)box.y1_, w, h);
		if(polyRect.intersects(devRect) == false)
			return;
		QRect interRect = devRect & polyRect;

		if(painter_.hasClipping())
		{
			QRegion interRegion = QRegion(interRect);
			QRegion pclip = painter_.clipRegion();
			interRegion = interRegion.intersect(pclip);
			interRect = interRegion.boundingRect();
		}

		QPoint pOffset = interRect.topLeft();
		int width = interRect.width();
		int height = interRect.height();

		int r = width%8;
		if(r)
			width += (8-r);
		r = height%8;
		if(r)
			height += (8-r);

		if(width == 0)
			width = 8;
		if(height == 0)
			height = 8;

		QBitmap	bm;
		bm.resize(width, height);
		//Fill bitmap with 0-bits: clipping region
		bm.fill(Qt::color0);
		QPainter maskPainter(&bm);

		TeLinearRing ring = poly[0];
 		QPointArray parray = mapDWtoV(ring);

		// Draw polygon with 1-bits: drawing region
		QBrush bs(Qt::color1);
		maskPainter.setBrush(bs);
		QPen pen(Qt::color1, polygonPen_.width());
		maskPainter.setPen(pen);
		maskPainter.translate(-pOffset.x(), -pOffset.y());
 		maskPainter.drawPolygon( parray );

  		vector<QPointArray> holeVec;
		if(poly.size() > 1)
		{
			// Draw holes with 0-bits: clipping region
			maskPainter.setBrush(Qt::color0);
			pen.setColor(Qt::color0);
			maskPainter.setPen(pen);
			for ( k = 1; k < (int)poly.size(); k++ )
  			{
  				ring = poly[k];
  				QPointArray hole = mapDWtoV(ring);
				holeVec.push_back(hole);
  				maskPainter.drawPolygon( hole );
  			}
		}
		maskPainter.end();

		QRegion clipRegion(bm);
		clipRegion.translate(pOffset.x(), pOffset.y());

		painter_.setClipRegion(clipRegion);

		if(restoreBackground && pixmap0_->rect() == pixmap2_->rect())
		{
			// restore background from pixmap2_
			painter_.drawPixmap(pOffset.x(), pOffset.y(), *pixmap2_, pOffset.x(),
				pOffset.y(), interRect.width(), interRect.height());
		}

		if(polygonBrush_.style() != Qt::CustomPattern)
		{
			if(polygonBrush_.style() == Qt::SolidPattern)
			{
				int transp = 255 - (polygonTransparency_ * 255 / 100);

				// set alpha buffer and color
				QImage img(interRect.width(), interRect.height(), 32);
				unsigned int val = (transp << 24) | (polygonColor_.red() << 16) | (polygonColor_.green() << 8) | polygonColor_.blue();
				img.fill(val);
				img.setAlphaBuffer(true);

				// plot transparency
				painter_.drawPixmap(pOffset.x(), pOffset.y(), img);
			}
		}
		//else // polygonBrush_.style() == Qt::CustomPattern
		//{
		//	QImage imag("c:\\terra2\\aplicativos\\ui\\images\\visual.jpg");

		//	if(imag.depth() != 32)
		//		imag = imag.convertDepth(32);
		//	int i, j, k;
		//	unsigned char* u = imag.bits();
		//	j = imag.width() * imag.height();
		//	for(i=0; i<j; ++i)
		//	{
		//		k = i * 4;
		//		// if pattern color is white, transparency is total
		//		if(u[k] == 255 && u[k+1] == 255 && u[k+2] == 255)
		//			u[k+3] = 0;
		//		else
		//		{
		//			// if polygonColor_ is black, do not change the color
		//			if(polygonColor_.blue() != 0 || polygonColor_.green() != 0 || polygonColor_.red() != 0)
		//			{
		//				int peso = 1;
		//				u[k] = (u[k] + peso * polygonColor_.blue()) / (1+peso);
		//				u[k+1] = (u[k+1] + peso * polygonColor_.green()) / (1+peso);
		//				u[k+2] = (u[k+2] + peso * polygonColor_.red()) / (1+peso);
		//			}
		//			u[k+3] = transp;
		//		}
		//	}
		//	imag.setAlphaBuffer(true);

		//	int xp = pOffset.x() / imag.width() * imag.width();
		//	int yp = pOffset.y() / imag.height() * imag.height();

		//	for(j=0; j<interRect.height()+imag.height(); j+=imag.height())
		//	{
		//		for(i=0; i<interRect.width()+imag.width(); i+=imag.width())
		//			painter_.drawPixmap(xp+i, yp+j, imag);
		//	}
		//}

		// plot contours
		painter_.setClipping(false);
		painter_.drawPolyline( parray );
		for(i = 0; i< (int)holeVec.size(); i++)
			painter_.drawPolyline( holeVec[i] );
	}
}

//void TeQtCanvas::plotPolygon (const TePolygon &poly, const bool& restoreBackground)
//{
//	if(canvasAndDataProjectionEqual_)
//	{
//		if (!TeIntersects (wc_, poly.box ()))
//			return;
//	}
//	else
//	{
//		TeBox canvasBox = TeRemapBoxPC2Geodetic(wc_, canvasProjection_);
//		TeBox pBox = TeRemapBoxPC2Geodetic(poly.box(), dataProjection_);
//
//		if (!TeIntersects(canvasBox, pBox))
//			return;
//	}
//
//	int		i, k;
// 	QPoint	p;
//	painter_.setBrush(polygonBrush_);
//	painter_.setPen (polygonPen_);
//
//	if(polygonTransparency_==100 || polygonBrush_.style() == Qt::NoBrush) // contour
//	{
//		TeLinearRing ring = poly[0];
// 		QPointArray parray = mapDWtoV(ring);
//		painter_.drawPolyline( parray );
//
//		for ( k = 1; k < (int)(poly.size()); k++ )
//  		{
//  			ring = poly[k];
//  			QPointArray hole = mapDWtoV(ring);
//  			painter_.drawPolyline( hole );
//  		}
//	}
//	else if(!restoreBackground && poly.size() == 1 && // no holes
//		polygonTransparency_ == 0  && // and is opaque
//		!(polygonBrush_.style() == Qt::NoBrush || // and not is NOBrush or Pattern
//		polygonBrush_.style() == Qt::CustomPattern))
//	{
//		TeLinearRing ring = poly[0];
// 		QPointArray parray = mapDWtoV(ring);
//		painter_.drawPolygon( parray );
//	}
//	else
//	{
//		QRect devRect;
//		if(painter_.device() == viewport())
//			devRect = viewport()->rect();
//		else
//			devRect = pixmap0_->rect();
//
//		TeBox box = poly.box();
//		mapDWtoV(box); // data coordinate to viewport coordinate
//		if(box.width() <= 0 || box.height() <= 0)
//			return;
//		int w = (int)box.width();
//		int h = (int)box.height();
//		if(w < 4)
//			w = 4;
//		if(h < 4)
//			h = 4;
//
//		QRect polyRect((int)box.x1_, (int)box.y1_, w, h);
//		if(polyRect.intersects(devRect) == false)
//			return;
//		QRect interRect = devRect & polyRect;
//
//		if(painter_.hasClipping())
//		{
//			QRegion interRegion = QRegion(interRect);
//			QRegion pclip = painter_.clipRegion();
//			interRegion = interRegion.intersect(pclip);
//			interRect = interRegion.boundingRect();
//		}
//
//		QPoint pOffset = interRect.topLeft();
//		int width = interRect.width();
//		int height = interRect.height();
//
//		int r = width%8;
//		if(r)
//			width += (8-r);
//		r = height%8;
//		if(r)
//			height += (8-r);
//
//		if(width == 0)
//			width = 8;
//		if(height == 0)
//			height = 8;
//
//		QBitmap	bm;
//		bm.resize(width, height);
//		//Fill bitmap with 0-bits: clipping region
//		bm.fill(Qt::color0);
//		QPainter maskPainter(&bm);
//
//		TeLinearRing ring = poly[0];
// 		QPointArray parray = mapDWtoV(ring);
//
//		// Draw polygon with 1-bits: drawing region
//		QBrush bs(Qt::color1);
//		maskPainter.setBrush(bs);
//		QPen pen(Qt::color1, polygonPen_.width());
//		maskPainter.setPen(pen);
//		maskPainter.translate(-pOffset.x(), -pOffset.y());
// 		maskPainter.drawPolygon( parray );
//
//  		vector<QPointArray> holeVec;
//		if(poly.size() > 1)
//		{
//			// Draw holes with 0-bits: clipping region
//			maskPainter.setBrush(Qt::color0);
//			pen.setColor(Qt::color0);
//			maskPainter.setPen(pen);
//			for ( k = 1; k < (int)poly.size(); k++ )
//  			{
//  				ring = poly[k];
//  				QPointArray hole = mapDWtoV(ring);
//				holeVec.push_back(hole);
//  				maskPainter.drawPolygon( hole );
//  			}
//		}
//		maskPainter.end();
//
//		QRegion clipRegion(bm);
//		clipRegion.translate(pOffset.x(), pOffset.y());
//
//		painter_.setClipRegion(clipRegion);
//
//		if(restoreBackground)
//		{
//			if(pixmap0_->rect() == pixmap2_->rect())
//			{
//				// restore background from pixmap2_
//				painter_.drawPixmap(pOffset.x(), pOffset.y(), *pixmap2_, pOffset.x(),
//					pOffset.y(), interRect.width(), interRect.height());
//			}
//		}
//
//		if((polygonTransparency_ == 0 && polygonBrush_.style() == Qt::SolidPattern) || !(polygonBrush_.style() == Qt::SolidPattern || polygonBrush_.style() == Qt::CustomPattern))
//		{
//			painter_.drawPolygon( parray );
//		}
//		else
//		{
//			int transp = 255 - (polygonTransparency_ * 255 / 100);
//
//			if(polygonBrush_.style() == Qt::SolidPattern)
//			{
//				// set alpha buffer and color
//				QImage img(interRect.width(), interRect.height(), 32);
//				unsigned int val = (transp << 24) | (polygonColor_.red() << 16) | (polygonColor_.green() << 8) | polygonColor_.blue();
//				img.fill(val);
//				img.setAlphaBuffer(true);
//
//				// plot transparency
//				painter_.drawPixmap(pOffset.x(), pOffset.y(), img);
//			}
//			else
//			{
//				painter_.drawPolygon( parray );
//			}
//			//else // polygonBrush_.style() == Qt::CustomPattern
//			//{
//			//	QImage imag("c:\\terra2\\aplicativos\\ui\\images\\visual.jpg");
//	
//			//	if(imag.depth() != 32)
//			//		imag = imag.convertDepth(32);
//			//	int i, j, k;
//			//	unsigned char* u = imag.bits();
//			//	j = imag.width() * imag.height();
//			//	for(i=0; i<j; ++i)
//			//	{
//			//		k = i * 4;
//			//		// if pattern color is white, transparency is total
//			//		if(u[k] == 255 && u[k+1] == 255 && u[k+2] == 255)
//			//			u[k+3] = 0;
//			//		else
//			//		{
//			//			// if polygonColor_ is black, do not change the color
//			//			if(polygonColor_.blue() != 0 || polygonColor_.green() != 0 || polygonColor_.red() != 0)
//			//			{
//			//				int peso = 1;
//			//				u[k] = (u[k] + peso * polygonColor_.blue()) / (1+peso);
//			//				u[k+1] = (u[k+1] + peso * polygonColor_.green()) / (1+peso);
//			//				u[k+2] = (u[k+2] + peso * polygonColor_.red()) / (1+peso);
//			//			}
//			//			u[k+3] = transp;
//			//		}
//			//	}
//			//	imag.setAlphaBuffer(true);
//	
//			//	int xp = pOffset.x() / imag.width() * imag.width();
//			//	int yp = pOffset.y() / imag.height() * imag.height();
//	
//			//	for(j=0; j<interRect.height()+imag.height(); j+=imag.height())
//			//	{
//			//		for(i=0; i<interRect.width()+imag.width(); i+=imag.width())
//			//			painter_.drawPixmap(xp+i, yp+j, imag);
//			//	}
//			//}
//		}
//
//		// plot contours
//		painter_.setClipping(false);
//		painter_.drawPolyline( parray );
//		for(i = 0; i< (int)holeVec.size(); i++)
//			painter_.drawPolyline( holeVec[i] );
//	}
//}

QRect TeQtCanvas::getLegendRect (QPoint p, const QPixmap* pix, string tx)
{
	QRect rect;
	TeVisual visual;
	TeColor cor(0, 0, 0);
	visual.size(8);
	visual.bold(true);
	visual.color(cor);
	visual.fixedSize(true);
	setTextColor (cor.red_, cor.green_, cor.blue_);
	string fam(visual.family());
	setTextStyle (fam, visual.size(), visual.bold(), visual.italic());

	if(pix)
	{
		int x = (int)(p.x()+pix->width()+3.*printerFactor_);
		int y = (int)(p.y()+13.*printerFactor_);
		QFontMetrics fm(textFont_);
		rect = fm.boundingRect(tx.c_str());
		rect.setRight(rect.right() + x);
		rect.setLeft(rect.left() + x);
		rect.setTop(rect.top() + y);
		rect.setBottom(rect.bottom() + y);
		QRect prect = pix->rect();
		prect.setRight(prect.right() + p.x());
		prect.setLeft(prect.left() + p.x());
		prect.setTop(prect.top() + p.y());
		prect.setBottom(prect.bottom() + p.y());
		rect |= prect;
	}
	else
	{
		int x = (int)(p.x()+1.*printerFactor_);
		int y = (int)(p.y()+13.*printerFactor_);
		QFontMetrics fm(textFont_);
		rect = fm.boundingRect(tx.c_str());
		rect = fm.boundingRect(tx.c_str());
		rect.setRight(rect.right() + x);
		rect.setLeft(rect.left() + x);
		rect.setTop(rect.top() + y);
		rect.setBottom(rect.bottom() + y);
	}
	return rect;
}

void TeQtCanvas::plotLegend (QPoint p, const QPixmap* pix, string tx)
{
	TeVisual visual;
	TeColor cor(0, 0, 0);
	visual.fixedSize(true);
	visual.size(8);
	visual.bold(true);
	visual.color(cor);
	setTextColor (cor.red_, cor.green_, cor.blue_);
	string fam = visual.family();
	setTextStyle (fam, visual.size(), visual.bold(), visual.italic());
	painter_.setFont(textFont_);
	painter_.setPen(textPen_);

	if(pix)
	{
		int x = (int)(p.x()+pix->width()+3.*printerFactor_);
		int y = (int)(p.y()+13.*printerFactor_);
		bitBlt (painter_.device(), p.x(), p.y(), pix, 0, 0, pix->width(), pix->height(), Qt::CopyROP, true);
		painter_.drawText(x, y, tx.c_str());
	}
	else
	{
		int x = (int)(p.x()+1.*printerFactor_);
		int y = (int)(p.y()+13.*printerFactor_);
		painter_.drawText(x, y, tx.c_str());
	}
}

bool TeQtCanvas::locateGraphicScale(const QPoint& p)
{
	return graphicScaleRect_.contains(p);
}

void TeQtCanvas::plotLegend (const QPoint& p, const TeColor& cor, const string& tx)
{
	TeVisual  v;
	TeColor c = cor;
	v.color(c);
	c.init(0, 0, 0);
	v.contourColor(c);
	int	ww = (int)(21. * printerFactor_);
	int	hh = (int)(16. * printerFactor_);

	drawPolygonRep(p, ww, hh, v);

	QPoint pos = p;
	pos.setX(pos.x() + ww);
	pos.setX((int)(pos.x() + 3.*printerFactor_));
	pos.setY((int)(pos.y()+13.*printerFactor_));
	painter_.setPen(textPen_);
	painter_.drawText(pos, tx.c_str());
}

void TeQtCanvas::plotLegend (const QPoint& p, TeLegendEntry* leg, const string& tx)
{
	TeVisual visual;
	TeColor cor(0, 0, 0);
	visual.fixedSize(true);
	visual.size(8);
	visual.bold(true);
	visual.color(cor);
	setTextColor (cor.red_, cor.green_, cor.blue_);
	string fam = visual.family();
	setTextStyle (fam, visual.size(), visual.bold(), visual.italic());
	painter_.setFont(textFont_);
	painter_.setPen(textPen_);
	QPoint pos = p;

	int	ww = (int)(21. * printerFactor_);
	int	hh = (int)(16. * printerFactor_);

	if(leg == 0)
	{
		pos.setX(pos.x() + ww);
		pos.setX((int)(pos.x() + 3.*printerFactor_));
		pos.setY((int)(p.y()+13.*printerFactor_));
		painter_.drawText(pos.x(), pos.y(), tx.c_str());
		return;
	}

	// TePOLYGONS = 1, TeLINES = 2, TePOINTS = 4, TeCELLS = 256
	bool hasPolygons = true;
	bool hasCells = true;
	bool hasLines = true;
	bool hasPoints = true;

	string s = leg->to();
	if(s.find("mean = ") == string::npos)
	{
		TeGeomRepVisualMap& vm = leg->getVisualMap();
		if(vm.find(TePOLYGONS) == vm.end())
		{
			hasPolygons = false;
			hasCells = false;
		}
		if(vm.find(TeLINES) == vm.end())
			hasLines = false;
		if(vm.find(TePOINTS) == vm.end())
			hasPoints = false;

		if ((hasPolygons || hasCells) && (hasLines == false) && (hasPoints == false))
			drawPolygonRep(pos, ww, hh, *(vm[TePOLYGONS]));
		else if ((hasPolygons || hasCells) && (hasLines == true) && (hasPoints == false))
		{
			drawPolygonRep(pos, ww, hh, *(vm[TePOLYGONS]));
			pos.setX(pos.x() + ww);
			drawLineRep(pos, *(vm[TeLINES]));
		}
		else if ((hasPolygons || hasCells) && (hasLines == false) && (hasPoints == true))
		{
			drawPolygonRep(pos, ww, hh, *(vm[TePOLYGONS]));
			pos.setX(pos.x() + ww);
			drawPointRep(pos, *(vm[TePOINTS]));
		}
		else if ((hasPolygons || hasCells) && (hasLines == true) && (hasPoints == true))
		{
			drawPolygonRep(pos, ww, hh, *(vm[TePOLYGONS]));
			pos.setX(pos.x() + ww);
			drawLineRep(pos, *(vm[TeLINES]));
			pos.setX(pos.x() + ww);
			drawPointRep(pos, *(vm[TePOINTS]));
		}
		else if (hasPolygons == false && hasCells == false && hasLines == true && hasPoints == false)
			drawLineRep(pos, *(vm[TeLINES]));
		else if (hasPolygons == false && hasCells == false && hasLines == false && hasPoints == true)
			drawPointRep(pos, *(vm[TePOINTS]));
		else if (hasPolygons == false && hasCells == false && hasLines == true && hasPoints == true)
		{
			drawLineRep(pos, *(vm[TeLINES]));
			pos.setX(pos.x() + ww);
			drawPointRep(pos, *(vm[TePOINTS]));
		}
	}

	pos.setX(pos.x() + ww);
	pos.setX((int)(pos.x() + 3.*printerFactor_));
	pos.setY((int)(p.y()+13.*printerFactor_));
	painter_.setPen(textPen_);
	painter_.drawText(pos, tx.c_str());
}

void TeQtCanvas::drawPolygonRep (QPoint p, int w, int h, TeVisual& v)
{
	QBrush brush;
	brush.setStyle(brushStyleMap_[(TePolyBasicType)v.style()]);
	QColor bcor(v.color().red_, v.color().green_, v.color().blue_);
	brush.setColor(bcor);
	painter_.setBrush(brush);

	QPen pen;
	pen.setStyle(penStyleMap_[(TeLnBasicType)v.contourStyle()]);
	QColor pcor(v.contourColor().red_, v.contourColor().green_, v.contourColor().blue_);
	pen.setColor(pcor);
	int pwidth = (Qt::PenStyle) v.contourWidth();
	pen.setWidth (pwidth);
	painter_.setPen(pen);

	QRect rect(p.x(), (int)(p.y()+2.*printerFactor_), w, (int)(h-2.*printerFactor_));


	// set alpha buffer and color
    int transp = v.transparency();
	if( (transp !=0) && ((TePolyBasicType)v.style() == TePolyTypeFill) )
	{
		transp = (int)(2.55 * (double)(100 - transp));
		QImage img(rect.width(), rect.height(), 32);
		unsigned int val = (transp << 24) | (bcor.red() << 16) | (bcor.green() << 8) | bcor.blue();
		img.fill(val);
		img.setAlphaBuffer(true);
		// plot transparency
		painter_.drawPixmap(rect.x(), rect.y(), img);
	}
	else	painter_.fillRect(rect,brush);
	painter_.setBrush(Qt::NoBrush);
	painter_.drawRect(rect);
}

void TeQtCanvas::drawLineRep (QPoint p, TeVisual& v)
{
	QPen		pen;
	QColor		cor;
	TeColor		tcor;
	uint		width;

	tcor = v.color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	pen.setColor(cor);
	pen.setStyle(penStyleMap_[(TeLnBasicType)v.style()]);
	width = v.width();
	pen.setWidth (width);
	painter_.setPen(pen);

	painter_.moveTo((int)(p.x()+1.*printerFactor_), (int)(p.y()+5.*printerFactor_));
	painter_.lineTo((int)(p.x()+3.*printerFactor_), (int)(p.y()+5.*printerFactor_));
	painter_.lineTo((int)(p.x()+6.*printerFactor_), (int)(p.y()+6.*printerFactor_));
	painter_.lineTo((int)(p.x()+8.*printerFactor_), (int)(p.y()+8.*printerFactor_));
	painter_.lineTo((int)(p.x()+10.*printerFactor_), (int)(p.y()+9.*printerFactor_));
	painter_.lineTo((int)(p.x()+14.*printerFactor_), (int)(p.y()+10.*printerFactor_));
	painter_.lineTo((int)(p.x()+16.*printerFactor_), (int)(p.y()+10.*printerFactor_));
	painter_.lineTo((int)(p.x()+18.*printerFactor_), (int)(p.y()+9.*printerFactor_));
	painter_.lineTo((int)(p.x()+19.*printerFactor_), (int)(p.y()+9.*printerFactor_));
	painter_.moveTo((int)(p.x()+2.*printerFactor_), (int)(p.y()+13.*printerFactor_));
	painter_.lineTo((int)(p.x()+3.*printerFactor_), (int)(p.y()+12.*printerFactor_));
	painter_.lineTo((int)(p.x()+5.*printerFactor_), (int)(p.y()+11.*printerFactor_));
	painter_.lineTo((int)(p.x()+6.*printerFactor_), (int)(p.y()+11.*printerFactor_));
	painter_.lineTo((int)(p.x()+9.*printerFactor_), (int)(p.y()+10.*printerFactor_));
	painter_.lineTo((int)(p.x()+10.*printerFactor_), (int)(p.y()+9.*printerFactor_));
}

void TeQtCanvas::drawPointRep (QPoint pp, TeVisual& v)
{
	QColor		cor;
	TeColor		tcor;

	tcor = v.color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	int s = v.style();
	double	w = (double)v.size() * printerFactor_;

	QPoint p = pp;
	QPoint offset((int)(10.*printerFactor_), (int)(7.*printerFactor_));
	p += offset;

	painter_.setPen(cor);
	if (s == TePtTypePlus)
	{
		painter_.drawLine ((int)(p.x()-w/2),p.y(),(int)(p.x()+w/2.),p.y());
		painter_.drawLine (p.x(),(int)(p.y()-w/2.),p.x(),(int)(p.y()+w/2.));
	}
	else if (s == TePtTypeStar)
	{
		painter_.save ();
		painter_.translate (p.x(),p.y());
		painter_.drawLine (0,(int)(-w/2.),0,(int)(w/2.));
		painter_.rotate (45);
		painter_.drawLine (0,(int)(-w/2.),0,(int)(w/2.));
		painter_.rotate (-90);
		painter_.drawLine (0,(int)(-w/2.),0,(int)(w/2.));
		painter_.restore ();
	}
	else if (s == TePtTypeCircle)
	{
		painter_.setBrush(cor);
		painter_.drawChord ((int)(p.x()-w/2.),(int)(p.y()-w/2.),(int)w,(int)w,0,360*16);
	}
	else if (s == TePtTypeX)
	{
		painter_.drawLine ((int)(p.x()-w/2.),(int)(p.y()-w/2.),(int)(p.x()+w/2.),(int)(p.y()+w/2.));
		painter_.drawLine ((int)(p.x()-w/2.),(int)(p.y()+w/2.),(int)(p.x()+w/2.),(int)(p.y()-w/2.));
	}
	else if (s == TePtTypeBox)
	{
		painter_.fillRect ((int)(p.x()-w/2.),(int)(p.y()-w/2.),(int)w,(int)w,cor);
	}
	else if (s == TePtTypeDiamond)
	{
		QPointArray pa(5);
		pa.setPoint(0, (int)(p.x()-w/2.), p.y());
		pa.setPoint(1, p.x(), (int)(p.y()-w/2.));
		pa.setPoint(2, (int)(p.x()+w/2.), p.y());
		pa.setPoint(3, p.x(), (int)(p.y()+w/2.));
 		pa.setPoint(4, (int)(p.x()-w/2.), p.y());
		painter_.setBrush(cor);
		painter_.drawPolygon(pa);
	}
	else if (s == TePtTypeHollowCircle)
	{
		painter_.drawArc ((int)(p.x()-w/2.),(int)(p.y()-w/2.),(int)w,(int)w,0,360*16);
	}
	else if (s == TePtTypeHollowBox)
	{
		painter_.setBrush(Qt::NoBrush);
		painter_.drawRect ((int)(p.x()-w/2.),(int)(p.y()-w/2.),(int)w,(int)w);
	}
	else if (s == TePtTypeHollowDiamond)
	{
		painter_.drawLine ((int)(p.x()-w/2.),p.y(),p.x(),(int)(p.y()-w/2.));
		painter_.drawLine (p.x(),(int)(p.y()-w/2.),(int)(p.x()+w/2.),p.y());
		painter_.drawLine ((int)(p.x()+w/2.),p.y(),p.x(),(int)(p.y()+w/2.));
		painter_.drawLine (p.x(),(int)(p.y()+w/2.),(int)(p.x()-w/2.),p.y());
	}
}

void TeQtCanvas::plotLine (TeLine2D &line)
{
	TeBox b = getDataWorld ();
	if (!TeIntersects (b, line.box ()))
		return;
	
	linePen_.setColor (lineColor_);
	painter_.setPen(linePen_);

	QPointArray a = mapDWtoV(line);
	painter_.drawPolyline( a );
	if(_penStyle == TeLnTypeArrow)
	{
		plotArrow(a);
	}else if(_penStyle == TeLnTypeMiddleArrow)
	{
		plotMiddleLineArrow(line,a);
	}
}

void TeQtCanvas::plotArrow(QPointArray a)
{
	QPoint first=a.point(0);
	QPoint init=a.point(a.size() - 2);
	QPoint end=a.point(a.size() - 1);

	if(TeDistance(TeCoord2D(first.x(),first.y()),TeCoord2D(end.x(),end.y()))>10)	
		{
			QPointArray a = getArrow(init,end,10);
			painter_.setBrush(Qt::SolidPattern);
			painter_.setBrush(lineColor_);
			painter_.drawPolygon( a );
		}
}

void TeQtCanvas::plotMiddleLineArrow(const TeLine2D &lne, QPointArray a)
{
		int seg;
		TeCoord2D	center;
		TeCoord2D	previousCoord;
		QPoint		first=a.point(0);
		QPoint		end=a.point(a.size() - 1);
		TeLine2D	ln;

		ln.copyElements(lne);
		TeFindCentroid(lne,center);
		TeLocateLineSegment(center,ln,seg,1.0);
		previousCoord=lne[seg];
		QPoint pCenter=mapDWtoV(center);
		QPoint pPrevious=mapDWtoV(previousCoord);
		if(TeDistance(TeCoord2D(first.x(),first.y()),TeCoord2D(end.x(),end.y()))>20)	
		{
			plotMiddleLineArrow(pPrevious,pCenter);
		}
}

void TeQtCanvas::plotMiddleLineArrow(QPoint ptBegin, QPoint ptEnd)
{
	QPointArray a = getArrow(ptBegin,ptEnd,10);
	painter_.setBrush(Qt::SolidPattern);
	painter_.setBrush(lineColor_);
	painter_.drawPolygon( a );
}


QPointArray	TeQtCanvas::getArrow(QPoint ptBegin, QPoint ptEnd, double size)
{
  QPointArray	points(4);
  double	slopy , cosy , siny;
	
  
  slopy = atan2((double)( ptBegin.y() - ptEnd.y() ),(double)( ptBegin.x() - ptEnd.x() ) );
  cosy = cos( slopy );
  siny = sin( slopy ); 

  points[0]=QPoint(ptEnd.x(),ptEnd.y());
  points[1]=QPoint(ptEnd.x() - int(-size*cosy-(size/2.0*siny)),ptEnd.y() - int(-size*siny+(size/2.0*cosy )));
  points[2]=QPoint(ptEnd.x() - int(-size*cosy+(size/2.0*siny)),ptEnd.y() + int(size/2.0*cosy+size*siny));
  points[3]=QPoint(ptEnd.x(),ptEnd.y());
  return points;
}

void TeQtCanvas::plotArc (TeArc &arc)
{
	TeBox b = getDataWorld ();
	if (!TeIntersects (b, arc.box ()))
		return;
	
	arcPen_.setColor (arcColor_);
	painter_.setPen(arcPen_);

	QPoint pfrom, pto;

	pfrom = mapDWtoV (arc.fromNode().location());
	pto   = mapDWtoV (arc.toNode().location());

	painter_.drawLine( pfrom, pto );

	double ang = atan2 (double(pfrom.y()-pto.y()), double (pfrom.x()-pto.x()));
	QPoint pm((int)((pto.x()+pfrom.x())/2.+0.5),(int)((pto.y()+pfrom.y())/2.+0.5));
	
	double ang1 = ang + 25.*TeCDR;
	double ang2 = ang - 25.*TeCDR;
	QPoint	p1((int)(10.*cos(ang1)+0.5),(int)(10.*sin(ang1)+.5)),
			p2((int)(10.*cos(ang2)+0.5),(int)(10.*sin(ang2)+.5));
	p1 += pm;
	p2 += pm;
	painter_.drawLine( pm, p1 );
	painter_.drawLine( pm, p2 );
	painter_.drawLine( p1, p2 );

}

void TeQtCanvas::plotPie (double x, double y, double w, double h, double a, double alen)
{
	int	dx, dy, dw, dh, da, df;

	pieBrush_.setColor (pieColor_);
	painter_.setBrush(pieBrush_);
	painter_.setPen(linePen_);

	TeCoord2D p(x, y);
	QPoint	qp = mapDWtoV(p);
	dx = qp.x();
	dy = qp.y();

	TeCoord2D pt(x+w, y+h);
	QPoint	qpt = mapDWtoV(pt);
	dw = abs(qpt.x() - dx);
	dh = abs(qpt.y() - dy);

	da = (int)(a * 16);
	df = (int)(alen * 16);
	painter_.drawPie(dx, dy-dh, dw, dh, da, df);
}

void TeQtCanvas::setPieColor (int r, int g, int b)
{
	pieColor_.setRgb(r, g, b);
	pieBrush_.setStyle(Qt::SolidPattern);
}

void TeQtCanvas::plotRect (QRect& rect)
{
	rectBrush_.setColor (rectColor_);
	painter_.setBrush(Qt::NoBrush);
	painter_.setPen(linePen_);
	painter_.drawRect(rect);
}

void TeQtCanvas::plotRect (double x, double y, double w, double h, int transp, bool legend)
{
	if(pixmap1_ == 0)
		return;

	int	dx, dy, dw, dh;

	TeCoord2D p(x, y);
	QPoint	qp;
	if(legend)
		qp = QPoint((int)p.x(), (int)p.y());
	else
		qp = mapDWtoV(p);
	dx = qp.x();
	dy = qp.y();

	TeCoord2D pt(x+w, y+h);
	QPoint	qpt;
	if(legend)
	{
		qpt = QPoint((int)pt.x(), (int)pt.y());
		dw = abs(qpt.x() - dx);
		dh = abs(qpt.y() - dy);
	}
	else
	{	
//		qpt = mapDWtoV(pt);
		TeBox b = wc_;
		mapCWtoDW(b);
		TeCoord2D center = b.center();
		TeCoord2D cc = center;
		cc.x_ += w;
		dw = abs(mapDWtoV(center).x() - mapDWtoV(cc).x());
		cc = center;
		cc.y_ += h;
		dh = abs(mapDWtoV(center).y() - mapDWtoV(cc).y());
	}

	if(transp == 0)
	{
		rectBrush_.setColor (rectColor_);
		painter_.setBrush(rectBrush_);
		painter_.setPen(linePen_);
		painter_.drawRect(dx, dy-dh, dw, dh);
	}
	else
	{
		double alpha = transp / 100.;
		double beta = 1 - alpha;
		double red = rectColor_.red() * beta;
		double green = rectColor_.green() * beta;
		double blue = rectColor_.blue() * beta;

		int r = dw%8;
		if(r)
			dw += (8-r);
		r = dh%8;
		if(r)
			dh += (8-r);

		QImage ima = pixmap0_->convertToImage();
		QImage imap(dw, dh, 32);
		int i, j;
		int x = qp.x();
		int	y = qp.y() - dh;
		for(i=x; i-x<dw && i<ima.width(); i++)
		{
			if(i < 0)
				continue;
			for(j=y; j-y<dh && j<ima.height(); j++)
			{
				if(j < 0)
					continue;
				QRgb v = ima.pixel(i, j);
				int r = (int)((double)qRed(v) * alpha + red);
				int g = (int)((double)qGreen(v) * alpha + green);
				int b = (int)((double)qBlue(v) * alpha + blue);
				QRgb t = qRgb(r, g, b);
				imap.setPixel(i-x, j-y, t);
			}
		}

		QPixmap pm(dw, dh);
		pm.convertFromImage(imap);
		QRect rec = pm.rect();
		rec &= painter_.viewport();
		bitBlt (painter_.device(), x, y, &pm, 0, 0, rec.width(), rec.height(), Qt::CopyROP, true);
	}
}

void TeQtCanvas::setRectColor (int r, int g, int b)
{
	rectColor_.setRgb(r, g, b);
	rectBrush_.setStyle(Qt::SolidPattern);
}

static inline int blendComponent( int back, int fore, int falpha )
{
	int balpha = 255 - falpha;
    int a = falpha + balpha -(falpha*balpha)/255;
    return (falpha*fore + balpha*back -(balpha*falpha*fore)/255)/a;  
}

int d2dms(double v)
{
    double av;
    int d, m, s;
    av = fabs(v);
    d = (int)av;
    av = av - d;
    av = av * 60;
    m = (int)av;
    av = av - m;
    av = av * 60;
    s = (int)av;
    int gms = d * 10000 + m * 100 + s ;
	return gms;
}

double dms2d (int vs)
{
	int v = abs(vs);
	int dd = v/10000;
	int dm = (v-dd*10000)/100;
	int ds = v - dd*10000 - dm*100;
	double d = dd + dm/60. + ds/3600.;
	if (vs < 0)
		d = -d;
	return d;
}

string buildMosaicName (int x, int y, int delta)
{
	int		lad,lam,las,lod,lom,los,dd,dm,ds;
	char	hemis,hemiss;
	int		lx1,ly1;

	hemis = 'S';
	if (y>=0)hemis = 'N';
	ly1 = abs(y);
	lad = ly1/3600;
	lam = (ly1-lad*3600)/60;
	las = ly1 - lad*3600 - lam*60;

	hemiss = 'O';
	if (x>=0)hemis = 'E';
	lx1 = abs(x);
	lod = lx1/3600;
	lom = (lx1-lod*3600)/60;
	los = lx1 - lod*3600 - lom*60;

	dd = delta/10000;
	dm = (delta-dd*10000)/100;
	ds = delta - dd*10000 - dm*100;

	char name[128];
	sprintf(name,"%c%02d%02d%02d%c%02d%02d%02d%02d%02d%02d",
		hemis,lad,lam,las,hemiss,lod,lom,los,dd,dm,ds);
	return string(name);
}

void TeQtCanvas::copyPixmap0ToPrinter()
{
	if (pixmap0_)
	{
		QPrinter printer;
		if (printer.setup())
		{
			QPainter p(&printer);

			QPaintDevice* dev = painter_.device();
			if(dev == pixmap0_)
				painter_.end();

			p.drawPixmap(0, 0, *pixmap0_);
			p.flush();
			p.end();

			if(dev && painter_.device() == 0)
				painter_.begin(dev);
		}
	}
}

void TeQtCanvas::copyPixmapToWindow(QPixmap* p, int ulx, int uly, int w, int h)
{
	bitBlt (viewport(),ulx,uly,p,ulx,uly,w,h,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap0To(QPaintDevice* dev)
{
	if (pixmap0_)
	{
		bitBlt (dev,0,0,pixmap0_,0,0,width_,height_,Qt::CopyROP,true);
	}
}

void TeQtCanvas::copyPixmap0ToWindow()
{
	if (pixmap0_)
	{
		bitBlt (viewport(),0,0,pixmap0_,contentsX(),contentsY(),viewport()->rect().width(),viewport()->rect().height(),Qt::CopyROP,true);
	}
}

void TeQtCanvas::copyPixmap1ToWindow()
{
	if (pixmap1_)
	{
		bitBlt (viewport(),0,0,pixmap1_,contentsX(),contentsY(),viewport()->rect().width(),viewport()->rect().height(),Qt::CopyROP,true);
	}
}

void TeQtCanvas::copyPixmap0ToWindow(int ulx, int uly, int w, int h)
{
	if (pixmap0_)
	{
		bitBlt (viewport(),ulx,uly,pixmap0_,ulx,uly,w,h,Qt::CopyROP,true);
	}
}

void TeQtCanvas::copyPanArea(int x, int y)
{
	if (pixmap0_)
	{
		QPaintDevice* dev = painter_.device();
		if(dev)
			painter_.end();
		painter_.begin(viewport());
		QRect a, b;
		int vw = viewport()->width();
		int vh = viewport()->height();

		int xi = horizontalScrollBar()->value();
		int yi = verticalScrollBar()->value();
		bitBlt (viewport(), 0, 0, pixmap1_, xi+x, yi+y, vw, vh, Qt::CopyROP, true);

		if((xi + x + vw) > contentsWidth())
		{
			int ww =  (xi + x + vw) - contentsWidth();
			int xii = vw - ww;
			QRect a(xii, 0, vw - xii, vh);
			painter_.fillRect(a, painter_.backgroundColor());
		}
		if((xi + x) < 0)
		{
			QRect a(0, 0, -(xi + x), vh);
			painter_.fillRect(a, painter_.backgroundColor());
		}
		if((yi + y + vh) > contentsHeight())
		{
			int hh =  (yi + y + vh) - contentsHeight();
			int yii = vh - hh;
			QRect a(0, yii, vw, vh - yii);
			painter_.fillRect(a, painter_.backgroundColor());
		}
		if((yi + y) < 0)
		{
			QRect a(0, 0, vw, -(yi + y));
			painter_.fillRect(a, painter_.backgroundColor());
		}

		painter_.end();
		if(dev)
			painter_.begin(dev);
	}
}

void TeQtCanvas::copyPixmap0ToPixmap1()
{
	if (pixmap1_ && pixmap0_)
		bitBlt (pixmap1_,0,0,pixmap0_,0,0,width_,height_,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap0ToPixmap1(int ulx, int uly, int w, int h)
{
	if (pixmap1_ && pixmap0_)
		bitBlt (pixmap1_,ulx,uly,pixmap0_,ulx,uly,w,h,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap1ToPixmap0()
{
	if (pixmap1_ && pixmap0_)
		bitBlt (pixmap0_,0,0,pixmap1_,0,0,width_,height_,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap1ToPixmap0(int ulx, int uly, int w, int h)
{
	if (pixmap1_ && pixmap0_)
		bitBlt (pixmap0_,ulx,uly,pixmap1_,ulx,uly,w,h,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap1ToPixmap2()
{
	if (pixmap1_ && pixmap2_)
	{
		bitBlt (pixmap2_,0,0,pixmap1_,0,0,width_,height_,Qt::CopyROP,true);
		qimage_.reset();
		qimage_ = pixmap2_->convertToImage();
	}
}

void TeQtCanvas::copyPixmap1ToPixmap2(int ulx, int uly, int w, int h)
{
	if (pixmap1_ && pixmap2_)
	{
		bitBlt (pixmap2_,ulx,uly,pixmap1_,ulx,uly,w,h,Qt::CopyROP,true);
		qimage_.reset();
		qimage_ = pixmap2_->convertToImage();
	}
}

void TeQtCanvas::copyPixmap2ToPixmap1()
{
	if (pixmap1_ && pixmap2_)
		bitBlt (pixmap1_,0,0,pixmap2_,0,0,width_,height_,Qt::CopyROP,true);
}

void TeQtCanvas::copyPixmap2ToPixmap1(int ulx, int uly, int w, int h)
{
	if (pixmap1_ && pixmap2_)
		bitBlt (pixmap1_,ulx,uly,pixmap2_,ulx,uly,w,h,Qt::CopyROP,true);
}

void TeQtCanvas::plotRaster(TeRaster* raster, TeRasterTransform* transf, TeQtProgress *progress)
{
	int dt = CLOCKS_PER_SEC/2;
	int dt2 = CLOCKS_PER_SEC; //* .000001;
	clock_t	t0, t1, t2;
	params_.fileName_ = raster->params().fileName_;

	if (buildRaster ())
	{
		// Calculate the box of input image that intersects the box of the canvas
		TeBox bboxBackRaster = backRaster_->params().boundingBox();		
		TeBox bboxSearched = TeRemapBox(bboxBackRaster, backRaster_->projection(), raster->projection());
		TeBox bboxIntersection;
		if (!TeIntersection (raster->params().boundingBox(),bboxSearched,bboxIntersection))
			return ;			// no intersection 

		// fill QImage with zero (100% of transparency)
		TeDecoderQtImage* decqt = reinterpret_cast<TeDecoderQtImage*>(backRaster_->decoder());
		decqt->getImage()->setAlphaBuffer(false);
		decqt->setAlphaBufferToTransparent();

		// Create a remapping tool to back raster
		TeRasterRemap remap;
		if (transf)
		{
			remap.setTransformer(transf);
//			unsigned int tt = transf->getTransparency();
			decqt->setTransparency(transf->getTransparency());
		}
		remap.setOutput(backRaster_);
		TeBox b = raster->params().boundingBox();
		remap.setROI(b);

		// Calculates best resolution level to display the input image on this canvas
		//int res = raster->decoder()->bestResolution(params_.resx_/raster->params().resx_);
		int res = raster->decoder()->bestResolution(bboxBackRaster, backRaster_->params().ncols_, backRaster_->params().nlines_, backRaster_->params().projection());
		

		// Check if raster blocks in best level of resolution that intersects the canvas box
    
		TeRasterParams parBlock;	
		if( /*raster->projection()->operator==( *backRaster_->projection() )
      && */raster->selectBlocks(bboxIntersection,res,parBlock))       
		{
			if (progress)
				progress->setTotalSteps(raster->numberOfSelectedBlocks());
			t2 = clock();
			t0 = t1 = t2;
			
			// Process each block as an independent raster decoded in memory
			TeRaster* block = new TeRaster;
			remap.setInput(block);

			TeDecoderMemory* decMem = new TeDecoderMemory(parBlock);
			decMem->init();
			// Portal of raster block selection behaves as portal of geometries
			// use the "bool flag - do - while" scheme
			int numBlockProcessed=0;
			bool flag = true;
			do
			{
				flag = raster->fetchRasterBlock(decMem);
				block->setDecoder(decMem);
				remap.apply();
				numBlockProcessed++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					if((int)(t2-t0) > dt2)	
					{
						if (progress)
						{
							if (progress->wasCancelled())
								break;
							else
								progress->setProgress(numBlockProcessed);
						}
					}
				}
			} while (flag);
			if (progress)
				progress->reset();
			decMem->clear();
			delete block;
			raster->clearBlockSelection();
		}
		else		// no blocks found try to remap the whole raster
		{
			remap.setInput(raster);
			remap.apply(true);	
		}
		decqt->getImage()->setAlphaBuffer(true);
		painter_.drawPixmap(0, 0, *(decqt->getImage()));
	}
}

bool TeQtCanvas::buildRaster ()
{
	if (!backRaster_)
	{
		backRaster_ = new TeRaster();
		params_.mode_ = 'c';
		params_.useDummy_ = true;
		params_.setDummy(255);
		TeDecoderQtImage *dec = new TeDecoderQtImage(params_);
		backRaster_->setDecoder (dec);
		backRaster_->init();
	}
	return true;
}

void TeQtCanvas::endPrinting()
{
	QPaintDevice* dev = painter_.device();
	if(dev)
		painter_.end();
	if(pixmap0_)
		painter_.begin(pixmap0_);
}

void TeQtCanvas::plotGraphicScale(TeVisual& visual, double offsetX, double offsetY, double unitConv, const string& dunit)
{
	TeBox	box = getWorld();
	if(box.isValid() == false)
		return;

	string unit = dunit;
	if (dunit.empty())
		unit = canvasProjection_->units();

	double	w = box.width();
	double www = w / unitConv;
	double	dx = www / 9.;
	long	idx = (long)dx;
	//long	idx = TeRound(dx);
	double	f;
	double	fa = 1.;
	int		fw, fh;

	int	size = visual.size();
	if(pixmap0_ && pixmap1_ && (pixmap0_->rect() != pixmap1_->rect()))
		size = (int)((double)size * printerFactor_ + .5);
	//	size = TeRound((double)size * printerFactor_);

	int conta = 1000;
	if(idx > 0)
	{
		while(--conta)
		{
			dx /= 10.;
			fa *= 10.;
			idx = (long)dx;
	//		idx = TeRound(dx);
			if(idx == 0)
			{
				idx = (long)(dx * 10.);
				f = idx * fa / 10.;
	//			idx = TeRound(dx * 10.);
	//			f = TeRound((double)idx * fa / 10.);
				break;
			}
		}
	}
	else
	{
		while(--conta)
		{
			dx *= 10.;
			fa /= 10.;
			idx = (long)dx;
	//		idx = TeRound(dx);
			if(idx > 0)
			{
				f = idx * fa;
	//			f = TeRound((double)idx * fa);
				break;
			}
		}
	}
	if(conta == 0)
		return;

	double fff = f * unitConv;
	double space = fff * 3.;
	double hini = box.x1_ + (w - space) / 2.;

	TeColor	cor = visual.color();
	string	family = visual.family();
	bool	bold = visual.bold();
	bool	italic = visual.italic();
	QFont font(family.c_str(), size);
	font.setBold (bold);
	font.setItalic (italic);
	painter_.setFont(font);

	QFontMetrics fm(font);
	QRect rect;
	rect = fm.boundingRect(unit.c_str());
	fh = rect.height();
	double vini = box.y1_ + mapVtoCW(fh+6);

	TeCoord2D wp(hini, vini);
	int x = TeRound((wp.x() - xmin_)*f_) + x0_;
	int y = height_ - y0_ - TeRound((wp.y() - ymin_)*f_);
	QPoint p1(x, y);
	correctScrolling (p1);

	wp.x(wp.x() + fff);
	x = TeRound((wp.x() - xmin_)*f_) + x0_;
	y = height_ - y0_ - TeRound((wp.y() - ymin_)*f_);
	QPoint p2(x, y);
	correctScrolling (p2);

	wp.x(wp.x() + fff);
	x = TeRound((wp.x() - xmin_)*f_) + x0_;
	y = height_ - y0_ - TeRound((wp.y() - ymin_)*f_);
	QPoint p3(x, y);
	correctScrolling (p3);

	wp.x(wp.x() + fff);
	x = TeRound((wp.x() - xmin_)*f_) + x0_;
	y = height_ - y0_ - TeRound((wp.y() - ymin_)*f_);
	QPoint p4(x, y);
	correctScrolling (p4);

	rect = fm.boundingRect("0");
	fw = rect.width();
	QPoint pp(p1);
	bool setRect = false;
	if(graphicScaleRect_.isValid())
	{
		x = TeRound((double)graphicScaleRect_.left() * printerFactor_ + (double)fw/2.);
		y = TeRound((double)graphicScaleRect_.top() * printerFactor_ + (double)rect.height() + 8. * printerFactor_);
		pp.setX(x);
		pp.setY(y);
	}
	else
	{
		setRect = true;
		offsetX = 0.;
		offsetY = 0.;
	}

	x = TeRound((double)pixmap0_->width() * offsetX);
	y = TeRound((double)pixmap0_->height() * offsetY);
	if(x!=0 || y!=0)
	{
		graphicScaleRect_.setLeft(graphicScaleRect_.left() + x);
		graphicScaleRect_.setRight(graphicScaleRect_.right() + x);
		graphicScaleRect_.setTop(graphicScaleRect_.top() + y);
		graphicScaleRect_.setBottom(graphicScaleRect_.bottom() + y);
	}

	QPoint offset(x, y);
	pp += offset;

	QPoint interval = p2 - p1;

	p1 = pp;
	p2 = p1 + interval;
	p3 = p2 + interval;
	p4 = p3 + interval;
	if(setRect)
	{
		x = TeRound(((double)p1.x() - (double)fw/2.) / printerFactor_);
		graphicScaleRect_.setLeft(x);
	}

	QPen pen(QColor(cor.red_, cor.green_, cor.blue_));
	painter_.setPen(pen);

	if(bold == false)
	{
		QPoint pa = p2;
		QPoint pb = p3;
		painter_.drawLine(pa, pb);

		pa = p1;
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);

		pa = p2; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);

		pa = p3; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);

		pa = p4; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);

		pa = p1;
		pb = p4;
		y = TeRound((double)pa.y() - 1. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() - 1. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		y = TeRound((double)pa.y() + 2. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 2. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
	}
	else
	{
		QPoint pa = p2;
		QPoint pb = p3;
		painter_.drawLine(pa, pb);
		y = TeRound((double)pa.y() - 1. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() - 1. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);

		pa = p1;
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		x = TeRound((double)pa.x() - 1. * printerFactor_);
		pa.setY(x);
		x = TeRound((double)pb.x() - 1. * printerFactor_);
		pb.setY(x);
		painter_.drawLine(pa, pb);

		pa = p2; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		x = TeRound((double)pa.x() - 1. * printerFactor_);
		pa.setY(x);
		x = TeRound((double)pb.x() - 1. * printerFactor_);
		pb.setY(x);
		painter_.drawLine(pa, pb);

		pa = p3; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		x = TeRound((double)pa.x() - 1. * printerFactor_);
		pa.setY(x);
		x = TeRound((double)pb.x() - 1. * printerFactor_);
		pb.setY(x);
		painter_.drawLine(pa, pb);

		pa = p4; 
		pb = pa;
		y = TeRound((double)pa.y() - 5. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 4. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		x = TeRound((double)pa.x() - 1. * printerFactor_);
		pa.setY(x);
		x = TeRound((double)pb.x() - 1. * printerFactor_);
		pb.setY(x);
		painter_.drawLine(pa, pb);

		pa = p1;
		pb = p4;
		y = TeRound((double)pa.y() - 2. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() - 2. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
		y = TeRound((double)pa.y() + 3. * printerFactor_);
		pa.setY(y);
		y = TeRound((double)pb.y() + 3. * printerFactor_);
		pb.setY(y);
		painter_.drawLine(pa, pb);
	}

	x = TeRound((double)p1.x()-(double)fw/2.);
	y = TeRound((double)p1.y()-8.*printerFactor_);
	painter_.drawText(x, y, "0");
	if(setRect)
	{
		y = TeRound(((double)y - (double)rect.height()) / printerFactor_);
		graphicScaleRect_.setTop(y);
	}

	char buf[50];
	long n = (long)f;

	if(n > 0)
	{
		sprintf(buf, "%ld", n);
		QString s = buf;
		rect = fm.boundingRect(s);
		int fw1 = rect.width();

		sprintf(buf, "%ld", n*2);
		s = buf;
		rect = fm.boundingRect(s);
		int fw2 = rect.width();

		sprintf(buf, "%ld", n*3);
		s = buf;
		rect = fm.boundingRect(s);
		int fw3 = rect.width();

		if((fw1 + fw2)/2 < p3.x()-p2.x() &&
			(fw2 + fw3)/2 < p3.x()-p2.x())
		{
			sprintf(buf, "%ld", n);
			QString s = buf;
			rect = fm.boundingRect(s);
			fw = rect.width();
			x = TeRound((double)p2.x()-(double)fw/2.);
			y = TeRound((double)p2.y()-8.*printerFactor_);
			painter_.drawText(x, y, s);

			sprintf(buf, "%ld", n*2);
			s = buf;
			rect = fm.boundingRect(s);
			fw = rect.width();
			x = TeRound((double)p3.x()-(double)fw/2.);
			y = TeRound((double)p3.y()-8.*printerFactor_);
			painter_.drawText(x, y, s);
		}

		sprintf(buf, "%ld", n*3);
		s = buf;
		rect = fm.boundingRect(s);
		fw = rect.width();
		x = TeRound((double)p4.x()-(double)fw/2.);
		y = TeRound((double)p4.y()-8.*printerFactor_);
		painter_.drawText(x, y, s);
		if(setRect)
		{
			x = TeRound(((double)x + (double)fw) / printerFactor_);
			graphicScaleRect_.setRight(x);
		}
	}
	else
	{
		int nn=0;
		int fn = (int)fa;
		while(fn == 0)
		{
			fa *= 10.;
			fn = TeRound(fa);
			nn++;
		}
		sprintf (buf, "%.*f", nn, f);
		QString s = buf;
		rect = fm.boundingRect(s);
		int fw1 = rect.width();

		sprintf (buf, "%.*f", nn, f*2.);
		s = buf;
		rect = fm.boundingRect(s);
		int fw2 = rect.width();

		sprintf (buf, "%.*f", nn, f*3.);
		s = buf;
		rect = fm.boundingRect(s);
		int fw3 = rect.width();

		if((fw1 + fw2)/2 < p3.x()-p2.x() &&
			(fw2 + fw3)/2 < p3.x()-p2.x())
		{
			sprintf (buf, "%.*f", nn, f);
			QString s = buf;
			rect = fm.boundingRect(s);
			fw = rect.width();
			x = TeRound((double)p2.x()-(double)fw/2.);
			y = TeRound((double)p2.y()-8.*printerFactor_);
			painter_.drawText(x, y, s);

			sprintf (buf, "%.*f", nn, f*2.);
			s = buf;
			rect = fm.boundingRect(s);
			fw = rect.width();
			x = TeRound((double)p3.x()-(double)fw/2.);
			y = TeRound((double)p3.y()-8.*printerFactor_);
			painter_.drawText(x, y, s);
		}

		sprintf (buf, "%.*f", nn, f*3.);
		s = buf;
		rect = fm.boundingRect(s);
		fw = rect.width();
		x = TeRound((double)p4.x()-(double)fw/2.);
		y = TeRound((double)p4.y()-8.*printerFactor_);
		painter_.drawText(x, y, s);
		x = TeRound((double)x + printerFactor_);
		graphicScaleRect_.setRight(x);
	}

	int sw = p4.x() - p1.x();
	rect = fm.boundingRect(unit.c_str());
	fw = rect.width();
	fh = rect.height();
	int suini = TeRound(((double)(sw - fw)) / 2. + (double)p1.x());
	y = TeRound((double)p1.y()+(double)fh+2.*printerFactor_);
	painter_.drawText (suini, y, unit.c_str());
	if(setRect)
	{
		y = TeRound((double)y / printerFactor_);
		graphicScaleRect_.setBottom(y);
	}
}

void TeQtCanvas::setClipRegion(int x, int y, int w, int h)
{
	QRegion region(x, y, w, h);
	if(painter_.device())
		painter_.setClipRegion(region);
}

void TeQtCanvas::setClipRegion(QRegion region)
{
	if(painter_.device())
		painter_.setClipRegion(region);
}

void TeQtCanvas::setClipping(bool enable)
{
	if(painter_.device())
		painter_.setClipping(enable);
}

void TeQtCanvas::clearRaster()
{
	if (backRaster_)
		delete backRaster_;
	backRaster_ = 0;
}


void TeQtCanvas::setMode (CursorMode m)
{
	cursorMode_ = m;
	down_ = false;
}

void TeQtCanvas::clear()
{
	int ww, hh;
	ww = viewport()->width();
	hh = viewport()->height();

	// Clear the window
	QPaintDevice* dev = painter_.device();
	if(dev)
		painter_.end();
	painter_.begin(viewport());
	painter_.eraseRect(0, 0, ww, hh);
	painter_.end();
	if(dev)
		painter_.begin(dev);
}

void TeQtCanvas::clear(TeBox box)
{
	mapCWtoV(box);

	// Clear the window
	plotOnWindow();
	painter_.eraseRect((int)box.x1(), (int)box.y1(), (int)box.width(), (int)box.height());
	plotOnPixmap0();
}

void TeQtCanvas::clearAll()
{
	// Clear the viewport
	clear();

	// Clear the pixmaps
	if(pixmap0_)
		pixmap0_->fill(paletteBackgroundColor());
	if(pixmap1_)
		pixmap1_->fill(paletteBackgroundColor());
	if(pixmap2_)
		pixmap2_->fill(paletteBackgroundColor());

	if (backRaster_)
		delete backRaster_;
	backRaster_ = 0;
}

void TeQtCanvas::clearAll(TeBox box)
{
	// Clear the viewport area corresponding to the given box
	clear(box);

	// Clear the pixmap
	mapCWtoV(box);

	QPaintDevice* dev = painter_.device();
	if(dev)
		painter_.end();

	if(pixmap0_)
	{
		painter_.begin(pixmap0_);
		painter_.fillRect((int)box.x1(), (int)box.y1(), (int)box.width(), (int)box.height(), backgroundColor());
		painter_.end();
	}
	if(pixmap1_)
	{
		painter_.begin(pixmap1_);
		painter_.fillRect((int)box.x1(), (int)box.y1(), (int)box.width(), (int)box.height(), backgroundColor());
		painter_.end();
	}
	if(pixmap2_)
	{
		painter_.begin(pixmap2_);
		painter_.fillRect((int)box.x1(), (int)box.y1(), (int)box.width(), (int)box.height(), backgroundColor());
		painter_.end();
	}

	if(dev)
		painter_.begin(dev);
}


bool TeQtCanvas::setView(int w, int h, QPaintDevice *pd)
{
    int dpix, dpiy, ncolors, wmm, hmm, depth, pdepth;
	int ww = w, hh = h;

	x0_ = y0_ = 0;

	if (pd == 0)
	{
		if (ww == 0)
			ww = viewport()->width();
		if (hh == 0)
			hh = viewport()->height();
		resizeContents (ww,hh);

		QPaintDevice* dev = painter_.device();
		if(dev)
			painter_.end();

		// Build new pixmaps if window has been resized
		if ((pixmap0_ == 0 || width_ != ww || height_ != hh) ||
			(pixmap0_ && (pixmap0_->width() != ww || pixmap0_->height() != hh)))
		{
			if (pixmap0_)
				delete pixmap0_;
			if(numberOfPixmaps_ > 0)
				pixmap0_ = new QPixmap (ww,hh);
			else
				pixmap0_ = new QPixmap (0,0);

			if (pixmap1_)
				delete pixmap1_;
			if(numberOfPixmaps_ > 1)
				pixmap1_ = new QPixmap (ww,hh);
			else
				pixmap1_ = new QPixmap (0,0);

			if (pixmap2_)
				delete pixmap2_;
			if(numberOfPixmaps_ > 2)
				pixmap2_ = new QPixmap (ww,hh);
			else
				pixmap2_ = new QPixmap (0,0);

			pdepth = pixmap0_->depth ();
		}
		painter_.begin(pixmap0_);
		printerFactor_ = 1.;
	}
	else
	{
		QPaintDeviceMetrics pdm(pd);
		if (ww == 0)
			ww = pdm.width();
		if (hh == 0)
			hh = pdm.height ();

		int pw = pixmap0_->rect().width();
		int ph = pixmap0_->rect().height();
		if((double)ww/(double)pw < (double)hh/(double)ph)
			hh = (int)((double)ph * (double)ww/(double)pw);
		else
			ww = (int)((double)pw * (double)hh/(double)ph);

		dpix = pdm.logicalDpiX();
		dpiy = pdm.logicalDpiY();
		ncolors = pdm.numColors();
		wmm = pdm.widthMM ();
		hmm = pdm.heightMM ();
		depth = pdm.depth ();
		QPaintDevice* dev = painter_.device();
		if(dev)
			painter_.end();

		if (pixmap0_)
			delete pixmap0_;
		if(numberOfPixmaps_ > 0)
			pixmap0_ = new QPixmap (ww,hh);
		else
			pixmap0_ = new QPixmap (0,0);

		// plot is made on pixmap0 because printer don�t have transparency
		painter_.begin(pixmap0_);
		painter_.setClipRect( x0_, y0_, ww, hh );

		printerFactor_ = (double)ww / (double)widthRef2PrinterFactor_;

		if (pixmap1_)
			delete pixmap1_;
		pixmap1_ = new QPixmap (0,0);

		if (pixmap2_)
			delete pixmap2_;
		pixmap2_ = new QPixmap (0,0);
	}

	if(numberOfPixmaps_ > 0 && (pixmap0_->width() == 0 || pixmap0_->height() == 0))
		return false;
	else if(numberOfPixmaps_ > 1 && (pixmap1_->width() == 0 || pixmap1_->height() == 0))
		return false;
	else if(numberOfPixmaps_ > 2 && (pixmap2_->width() == 0 || pixmap2_->height() == 0))
		return false;

	width_ = ww;
	height_ = hh;
	params_.ncols_ = width_;
	params_.nlines_ = height_;

	down_ = false;
	xul_ = xmin_;
	yul_ = ymax_;
	xlr_ = xmax_;
	ylr_ = ymin_;

	if (pd == 0)
		clearAll();
	return true;
}

void TeQtCanvas::correctScrolling (QPoint &p)
{
	if(painter_.device() == viewport())
	{
		QPoint o (contentsX (), contentsY ());
		p -= o;
	}
}

void TeQtCanvas::initCursorArea(QPoint p)
{
	if (!pixmap0_)
		return;

	down_ = true;
	TeCoord2D pw = mapVtoCW(p);
	xul_ = pw.x();
	yul_ = pw.y();
	ixul_ = ixlr_ = p.x();
	iyul_ = iylr_ = p.y();
}

void TeQtCanvas::contentsMousePressEvent( QMouseEvent* e)
{
	if (!pixmap0_)
		return;

	QPoint m = e->pos();
	TeCoord2D p = mapVtoCW(m);

	if(e->button() == LeftButton)
	{
		if (cursorMode_ == Area || cursorMode_ == Edit)
		{
			plotOnWindow();
			down_ = true;
			xul_ = p.x();
			yul_ = p.y();
			ixul_ = ixlr_ = e->pos().x();
			iyul_ = iylr_ = e->pos().y();
			plotOnPixmap0();
			emit mouseMoved (p, e->state(), m);
		}
		else
			emit mousePressed (p, e->state(), m);
	}
	else if(e->button() == RightButton)
		emit mouseRightButtonPressed (p, e->state(), m);
}

void TeQtCanvas::contentsMouseMoveEvent ( QMouseEvent* e)
{
	if (!pixmap0_)
		return;
	plotOnWindow();
	QPoint m = e->pos();
	TeCoord2D p = mapVtoCW(m);
	if (down_)
	{
		QPoint o = offset();
		painter_.setPen (green);
		painter_.setRasterOp (Qt::XorROP);
		painter_.drawLine(ixul_-o.x(),iyul_-o.y(),ixlr_-o.x()-1,iyul_-o.y());            
		painter_.drawLine(ixlr_-o.x(),iyul_-o.y(),ixlr_-o.x(),iylr_-o.y() -1);            
		painter_.drawLine(ixlr_-o.x(),iylr_-o.y(),ixul_-o.x()-1,iylr_-o.y() );            
		painter_.drawLine(ixul_-o.x(),iylr_-o.y(),ixul_-o.x(),iyul_-o.y()-1 );            
		ixlr_ = e->pos().x();
		iylr_ = e->pos().y();
		painter_.drawLine(ixul_-o.x(),iyul_-o.y(),ixlr_-o.x()-1,iyul_-o.y());            
		painter_.drawLine(ixlr_-o.x(),iyul_-o.y(),ixlr_-o.x(),iylr_ -o.y()-1);            
		painter_.drawLine(ixlr_-o.x(),iylr_-o.y(),ixul_-o.x()-1,iylr_-o.y() );            
		painter_.drawLine(ixul_-o.x(),iylr_-o.y(),ixul_-o.x(),iyul_-o.y()-1 );            
		painter_.setRasterOp (Qt::CopyROP);
	}
	plotOnPixmap0();
	emit mouseMoved (p, e->state(), m);
}

void TeQtCanvas::contentsMouseReleaseEvent( QMouseEvent* e)
{
	if (!pixmap0_)
		return;
	QPoint m = e->pos();
	TeCoord2D p = mapVtoCW(m);
	if (down_)
	{
		ixlr_ = e->pos().x();
		iylr_ = e->pos().y();
		xlr_ = p.x();
		ylr_ = p.y();
		if(xul_ > xlr_)
		{
			double a = xlr_;
			xlr_ = xul_;
			xul_ = a;
		}
		if(ylr_ > yul_)
		{
			double a = ylr_;
			ylr_ = yul_;
			yul_ = a;
		}
		down_ = false;

		emit zoomArea();
	}
	emit mouseReleased (p, e->state(), m);
}

void TeQtCanvas::contentsMouseDoubleClickEvent ( QMouseEvent* e)
{
	if (!pixmap0_)
		return;

	QPoint m = e->pos();
	TeCoord2D p = mapVtoCW(m);

	if(e->button() == LeftButton)
		emit mouseDoublePressed (p, e->state(), m);
}


void TeQtCanvas::contentsContextMenuEvent( QContextMenuEvent* e)
{
	QMouseEvent *m = (QMouseEvent *)e;
	emit popupCanvasSignal(m);
}


void TeQtCanvas::leaveEvent ( QEvent * )
{
	emit mouseLeave();
}

void TeQtCanvas::keyPressEvent (QKeyEvent* e)
{
	emit keyPressed(e);
}

void TeQtCanvas::keyReleaseEvent (QKeyEvent* e)
{
	emit keyReleased(e);
}

void TeQtCanvas::viewportPaintEvent (QPaintEvent*)
{
	copyPixmapToWindow ();
	emit paintEvent ();
}

void TeQtCanvas::resizeEvent ( QResizeEvent * e)
{
	QScrollView::resizeEvent(e);
}

void TeQtCanvas::copyPixmapToWindow()
{
	if (pixmap0_)
	{
		int cx = contentsX();
		int cy = contentsY();
		bitBlt (viewport(),0,0,pixmap0_,cx,cy,viewport()->rect().width(),viewport()->rect().height(),CopyROP,true);
	}
}

int TeQtCanvas::mapDWtoV(double a)
{
	TeBox dbox = getDataWorld();
	double dwidth = dbox.width();
	int	pwidth = viewport()->width();

	int d = (int)((double)pwidth * a / (double)dwidth);
	return d;
}

