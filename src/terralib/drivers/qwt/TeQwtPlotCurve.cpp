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

#include "TeQwtPlotCurve.h"
#include <qwt_scale_map.h>
#include <qwt_painter.h>
#include <qpainter.h>


void TeQwtPlotCurve::drawCurve(QPainter *p, int style,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const
{
	if(style == UserCurve)
	{
		int y0 = yMap.transform(baseline());
		QRect rect;
		int w = xMap.transform(x(1)) - xMap.transform(x(0));
		int w2 = w/2;

		for (int i = from; i <= to; i++)
		{
			const int xi = xMap.transform(x(i));
			const int yi = yMap.transform(y(i));
			if(y0 < yi)
				rect = QRect(xi-w2, y0, w, yi-y0);
			else
				rect = QRect(xi-w2, yi, w, y0-yi);
			p->fillRect(rect, brush());
			p->drawRect(rect);
		}
	}
	else
		QwtPlotCurve::drawCurve(p, style, xMap, yMap, from, to);
}
