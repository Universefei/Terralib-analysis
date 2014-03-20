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

#include <TeQtLegendItem.h>
#include <TeQtViewsListView.h>
#include "../../kernel/TeLegendEntry.h"
#include <qpixmap.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qfontmetrics.h>


//Legend item constructor
TeQtLegendItem::TeQtLegendItem(QListViewItem *parent,
				QString text, TeLegendEntry *legendEntry)
	: TeQtCheckListItem(parent, text), legendEntry_(legendEntry)
{
	type_ = LEGEND;
	setRenameEnabled(0,true);
	setSelected(false);
	setEnabled(true);

	order_ = parent->childCount();

//	if (legendEntry_ != 0)
//		createPixmap();
}


QPixmap TeQtLegendItem::createPixmap()
{
	int	ww = 21;
	int	pixh = 16;
	int	pixw = ww;

	QPixmap	pixmap(ww, pixh);
	pixmap.fill();

// TePOLYGONS = 1, TeLINES = 2, TePOINTS = 4, TeCELLS = 256
	bool hasPolygons = true;
	bool hasCells = true;
	bool hasLines = true;
	bool hasPoints = true;

	if(legendEntry_->to().find("mean = ") == string::npos)
	{
		TeGeomRepVisualMap& vm = legendEntry_->getVisualMap();
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
			drawPolygonRep(pixw, pixh, 0, &pixmap);
		else if ((hasPolygons || hasCells) && (hasLines == true) && (hasPoints == false))
		{
			pixmap.resize(2*ww, pixh);
			pixmap.fill();
			drawPolygonRep(pixw, pixh, 0, &pixmap);
			drawLineRep(ww, &pixmap);
		}
		else if ((hasPolygons || hasCells) && (hasLines == false) && (hasPoints == true))
		{
			pixmap.resize(2*ww, pixh);
			pixmap.fill();
			drawPolygonRep(pixw, pixh, 0, &pixmap);
			drawPointRep(pixw, pixh, ww, &pixmap);
		}
		else if ((hasPolygons || hasCells) && (hasLines == true) && (hasPoints == true))
		{
			pixmap.resize(3*ww, pixh);
			pixmap.fill();
			drawPolygonRep(pixw, pixh, 0, &pixmap);
			drawLineRep(ww, &pixmap);
			drawPointRep(pixw, pixh, 2*ww, &pixmap);
		}
		else if (hasPolygons == false && hasCells == false && hasLines == true && hasPoints == false)
			drawLineRep(0, &pixmap);
		else if (hasPolygons == false && hasCells == false && hasLines == false && hasPoints == true)
			drawPointRep(pixw, pixh, 0, &pixmap);
		else if (hasPolygons == false && hasCells == false && hasLines == true && hasPoints == true)
		{
			pixmap.resize(2*ww, pixh);
			pixmap.fill();
			drawLineRep(0, &pixmap);
			drawPointRep(pixw, pixh, ww, &pixmap);
		}
	}
//	setPixmap(0, pixmap);
	return pixmap;
}


void TeQtLegendItem::changeVisual(TeLegendEntry *legendEntry)
{
	legendEntry_ = legendEntry;
//	createPixmap();
}

void TeQtLegendItem::paintCell(QPainter* p, const QColorGroup& /* cg */, int /* column */, int width , int align)
{
//	TeQtCheckListItem::paintCell(p, cg, column, width, align);
	QPixmap pixmap = createPixmap();
	QRect r(QPoint(0, 0), QPoint(width - 1, height() - 1));
	p->fillRect(r, QBrush(Qt::white));

	p->drawPixmap(0, 0, pixmap);
	
	QRect r1(QPoint(pixmap.width(), 0), QPoint(width - 1, height() - 1));
	p->drawText(r1, align, text());

	QListView* lv = listView();
	QFontMetrics fm = lv->fontMetrics();
	int w1 = lv->treeStepSize() * ( depth() + ( lv->rootIsDecorated() ? 1 : 0) ) + lv->itemMargin();
	int w = pixmap.width() + fm.width(text()) + w1;
	lv->setColumnWidth(0, QMAX(lv->columnWidth(0), w));
}


void TeQtLegendItem::drawPolygonRep(int w, int h, int offset, QPixmap *pixmap)
{
	QPainter p(pixmap);
	QBrush	 brush;
	QColor	 cor;
	TeColor	 tcor;
	Qt::BrushStyle	style;

	TeVisual* visual = legendEntry_->visual(TePOLYGONS);
	int transp = 255 - (visual->transparency() * 255 / 100);
	tcor = visual->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	TeQtViewsListView* viewsListView = (TeQtViewsListView*) listView();
	map<TePolyBasicType, Qt::BrushStyle>& brushMap = viewsListView->getBrushStyleMap();

	style = brushMap[(TePolyBasicType)(legendEntry_->visual(TePOLYGONS)->style())];
	brush.setStyle(style);
	brush.setColor(cor);

	QRect trect(offset+1, 1, w-2, h-2);
	QRect rect(0, 0, offset+w-1, h-1);

	int width = rect.width();
	int height = rect.height();

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

	// Draw bitmap with 1-bits: drawing region
	QBrush bs(Qt::color1, style);
	QPen pen(Qt::color1, 1);
	maskPainter.setPen(pen);
 	maskPainter.fillRect(trect, bs);
	maskPainter.end();

	QRegion clipRegion(bm);
	p.setClipRegion(clipRegion);

	if(visual->transparency() == 0)
		p.fillRect(trect, brush);
	else
	{
		// set alpha buffer and color
		QImage img(rect.width(), rect.height(), 32);
		unsigned int val = (transp << 24) | (cor.red() << 16) | (cor.green() << 8) | cor.blue();
		img.fill(val);
		img.setAlphaBuffer(true);

		// plot transparency
		p.drawPixmap(trect.x(), trect.y(), img);
	}
	p.setClipping(false);

	Qt::PenStyle pstyle;
	uint		pwidth;

	tcor = visual->contourColor();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	pen.setColor(cor);

	map<TeLnBasicType, Qt::PenStyle>& penMap = viewsListView->getPenStyleMap();
	pstyle = penMap[(TeLnBasicType)(visual->contourStyle())];
	pen.setStyle(pstyle);

	pwidth = (Qt::PenStyle) visual->contourWidth();
	pen.setWidth (pwidth);

	p.setPen(pen);
	p.drawRect (offset+1+pwidth/2,1+pwidth/2,w-2-pwidth/2,h-2-pwidth/2);

	p.end();
}

void TeQtLegendItem::drawLineRep(int offset, QPixmap *pixmap)
{
	QPainter	p(pixmap);
	QPen		pen;
	QColor		cor;
	TeColor		tcor;
	Qt::PenStyle style;
	uint		width;

	TeVisual* visual = legendEntry_->visual(TeLINES);

	tcor = visual->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	pen.setColor(cor);

	TeQtViewsListView* viewsListView = (TeQtViewsListView*) listView();
	map<TeLnBasicType, Qt::PenStyle>& penMap = viewsListView->getPenStyleMap();

	style = penMap[(TeLnBasicType)(legendEntry_->visual(TeLINES)->style())];
	pen.setStyle(style);

	width = (Qt::PenStyle) legendEntry_->visual(TeLINES)->width();
	pen.setWidth (width);

	p.setPen(pen);
	p.moveTo(offset+1, 5);
	p.lineTo(offset+3, 5);
	p.lineTo(offset+6, 6);
	p.lineTo(offset+8, 8);
	p.lineTo(offset+10, 9);
	p.lineTo(offset+14, 10);
	p.lineTo(offset+16, 10);
	p.lineTo(offset+18, 9);
	p.lineTo(offset+19, 9);
	p.moveTo(offset+2, 13);
	p.lineTo(offset+3, 12);
	p.lineTo(offset+5, 11);
	p.lineTo(offset+6, 11);
	p.lineTo(offset+9, 10);
	p.lineTo(offset+10, 9);
	p.end();
}


void TeQtLegendItem::drawPointRep( int pw, int ph, int offset, QPixmap *pixmap)
{
	QPainter	painter(pixmap);
	QColor		cor;
	TeColor		tcor;

	TeVisual* visual = legendEntry_->visual(TePOINTS);
	tcor = visual->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	int s = visual->style();

	int	w = visual->size();
	QPoint	p;
	p.setX(offset+pw/2);
	p.setY(ph/2-1);

	painter.setPen(cor);
	if (s == TePtTypePlus)
	{
		painter.drawLine (p.x()-w/2,p.y(),p.x()+w/2,p.y());
		painter.drawLine (p.x(),p.y()-w/2,p.x(),p.y()+w/2);
	}
	else if (s == TePtTypeStar)
	{
		painter.save ();
		painter.translate (p.x(),p.y());
		painter.drawLine (0,-w/2,0,w/2);
		painter.rotate (45);
		painter.drawLine (0,-w/2,0,w/2);
		painter.rotate (-90);
		painter.drawLine (0,-w/2,0,w/2);
		painter.restore ();
	}
	else if (s == TePtTypeCircle)
	{
		painter.setBrush(cor);
		painter.drawChord (p.x()-w/2,p.y()-w/2,w,w,0,360*16);
	}
	else if (s == TePtTypeX)
	{
		painter.drawLine (p.x()-w/2,p.y()-w/2,p.x()+w/2,p.y()+w/2);
		painter.drawLine (p.x()-w/2,p.y()+w/2,p.x()+w/2,p.y()-w/2);
	}
	else if (s == TePtTypeBox)
	{
		painter.fillRect (p.x()-w/2,p.y()-w/2,w,w,cor);
	}
	else if (s == TePtTypeDiamond)
	{
		QPointArray pa(5);
		pa.setPoint(0, p.x()-w/2, p.y());
		pa.setPoint(1, p.x(), p.y()-w/2);
		pa.setPoint(2, p.x()+w/2, p.y());
		pa.setPoint(3, p.x(), p.y()+w/2);
 		pa.setPoint(4, p.x()-w/2, p.y());
		painter.setBrush(cor);
		painter.drawPolygon(pa);
	}
	else if (s == TePtTypeHollowCircle)
	{
		painter.drawArc (p.x()-w/2,p.y()-w/2,w,w,0,360*16);
	}
	else if (s == TePtTypeHollowBox)
	{
		painter.setBrush(Qt::NoBrush);
		painter.drawRect (p.x()-w/2,p.y()-w/2,w,w);
	}
	else if (s == TePtTypeHollowDiamond)
	{
		painter.drawLine (p.x()-w/2,p.y(),p.x(),p.y()-w/2);
		painter.drawLine (p.x(),p.y()-w/2,p.x()+w/2,p.y());
		painter.drawLine (p.x()+w/2,p.y(),p.x(),p.y()+w/2);
		painter.drawLine (p.x(),p.y()+w/2,p.x()-w/2,p.y());
	}
	painter.end();
}
