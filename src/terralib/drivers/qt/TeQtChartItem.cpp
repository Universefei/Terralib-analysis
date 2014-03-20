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

#include <qpixmap.h>
#include <qpainter.h>
#include <TeQtChartItem.h>
#include <TeQtViewsListView.h>


//Chart item constructor
TeQtChartItem::TeQtChartItem(QListViewItem *parent,
				QString text, TeColor color)
	: TeQtCheckListItem(parent, text), color_(color)
{
	type_ = CHART;
	name_ = text.latin1();
	setRenameEnabled(0,true);
	setSelected(false);
	setEnabled(true);

	order_ = parent->childCount();

	//create pixmap
	int	w = 21;
	int	h = 16;

	QPixmap pixmap(w, h);
	pixmap.resize(w, h);
	pixmap.fill();

	QPainter p(&pixmap);
	QBrush	 brush;
	QColor	 qColor;

	qColor.setRgb(color_.red_, color_.green_, color_.blue_);
	brush.setColor(qColor);
	brush.setStyle((Qt::BrushStyle)SolidPattern);

	p.fillRect (1, 1, w-2, h-2, brush);
	p.end();

	setPixmap(0, pixmap);
}

void TeQtChartItem::changeColor(TeColor color)
{
	color_ = color;

	int	w = 21;
	int	h = 16;

	QPixmap pixmap(w, h);
	pixmap.resize(w, h);
	pixmap.fill();

	QPainter p(&pixmap);
	QBrush	 brush;
	QColor	 qColor;

	qColor.setRgb(color_.red_, color_.green_, color_.blue_);
	brush.setColor(qColor);
	brush.setStyle((Qt::BrushStyle)SolidPattern);

	p.fillRect (1, 1, w-2, h-2, brush);
	p.end();

	setPixmap(0, pixmap);
	repaint();
}
