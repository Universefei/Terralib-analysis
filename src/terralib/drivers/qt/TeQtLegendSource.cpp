#include <TeQtLegendSource.h>
#include "../../kernel/TeLayer.h"
#include "../../kernel/TeTheme.h"
#include "../../utils/TeColorUtils.h"
#include "../../kernel/TeDataTypes.h"
#include "../../kernel/TeGroupingAlgorithms.h"

#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>


TeQtLegendSource::TeQtLegendSource() : nRows_(0), nCols_(0)
{
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
}


void TeQtLegendSource::copyAppThemeContents(const TeAppTheme& appTheme)
{
	appTheme_ = appTheme;  // Make a copy of the theme	
	db_ = appTheme_.getLocalDatabase();

	// Set the number of lines
	nRows_ = ((TeTheme*)appTheme_.getTheme())->legend().size();

	// Set the number of columns
	TeGroupingMode groupingMode = ((TeTheme*)appTheme_.getTheme())->grouping().groupMode_;
	if (groupingMode == TeUniqueValue)
		nCols_ = 4;
	else
		nCols_ = 5;
}

QVariant TeQtLegendSource::cell(int row, int col)
{
	QVariant v;
	QString qs;

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();

	if(legVec.size() == 0)
		return v;

	TeGrouping& groupingParams = ((TeTheme*)appTheme_.getTheme())->grouping();
	TeGroupingMode groupingMode = groupingParams.groupMode_;

	if (col == 0)   // Create a pixmap in this column
	{
		QPixmap p;
		p = createPixmap(row);
		return p;
	}

	if (col == 1)
	{
		qs = legVec[row].slice().from_.c_str();
		return qs;
	}

	if (col == 2)
	{
		if (groupingMode == TeUniqueValue)
			qs = legVec[row].label().c_str();
		else
			qs = legVec[row].to().c_str();
		return qs;
	}
	
	if (col == 3)
	{
		if (groupingMode == TeUniqueValue)
			qs = Te2String(legVec[row].slice().count_).c_str();
		else
			qs = legVec[row].label().c_str();
		return qs;
	}

	if (col == 4)
	{
		qs = Te2String(legVec[row].slice().count_).c_str();
		return qs;
	}

	return v;
}

QPixmap TeQtLegendSource::createPixmap(int row)
{	
	int		ww   = 21;		// individual width
	int		tw   = 0;		// total width
	int		pixh = 16;		
	int		pixw = ww;

	QPixmap	pixmap(ww, pixh);

	bool hasPolygons = true;
	bool hasLines = true;
	bool hasPoints = true;

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	TeGeomRepVisualMap& vm = legVec[row].getVisualMap();
	
	if (vm.find(TePOLYGONS) == vm.end() && vm.find(TeCELLS) == vm.end())
		hasPolygons = false;
	else 
		tw += ww;

	if (vm.find(TeLINES) == vm.end())
		hasLines = false;
	else
		tw += ww;

	if (vm.find(TePOINTS) == vm.end())
		hasPoints = false;
	else
		tw += ww;

	pixmap.resize(tw, pixh);
	pixmap.fill();

	int wi = 0;
	if (hasPolygons)
	{
		drawPolygonRep(pixw, pixh, wi, row, &pixmap);
		wi	+= ww;
	}
	if (hasLines)
	{
		drawLineRep(wi, row,  &pixmap);
		wi	+= ww;
	}
	if (hasPoints)
	{
		drawPointRep(pixw, pixh, wi, row, &pixmap);
		wi	+= ww;
	}
	return pixmap;
}

void TeQtLegendSource::drawPolygonRep(int w, int h, int offset, int row, QPixmap *pixmap)
{
	QPainter p(pixmap);
	QBrush	 brush;
	QColor	 cor;
	TeColor	 tcor;
	Qt::BrushStyle	style;

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	TeGeomRepVisualMap& vm = legVec[row].getVisualMap();

	int transp = 255 - (vm[TePOLYGONS]->transparency() * 255 / 100);
	tcor = vm[TePOLYGONS]->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);

	style = brushStyleMap_[(TePolyBasicType)(vm[TePOLYGONS]->style())];
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

	if(vm[TePOLYGONS]->transparency() == 0)
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

	tcor = vm[TePOLYGONS]->contourColor();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	pen.setColor(cor);

	pstyle = penStyleMap_[(TeLnBasicType)(vm[TePOLYGONS]->contourStyle())];
	pen.setStyle(pstyle);

	pwidth = (Qt::PenStyle) vm[TePOLYGONS]->contourWidth();
	pen.setWidth (pwidth);

	p.setPen(pen);
	p.drawRect (offset+1+pwidth/2,1+pwidth/2,w-2-pwidth/2,h-2-pwidth/2);

	p.end();
}

void TeQtLegendSource::drawLineRep(int offset, int row, QPixmap *pixmap)
{
	QPainter	p(pixmap);
	QPen		pen;
	QColor		cor;
	TeColor		tcor;
	Qt::PenStyle style;
	uint		width;

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	TeGeomRepVisualMap& vm = legVec[row].getVisualMap();

	tcor = vm[TeLINES]->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	pen.setColor(cor);

	style = penStyleMap_[(TeLnBasicType)(vm[TeLINES]->style())];
	pen.setStyle(style);

	width = (Qt::PenStyle) vm[TeLINES]->width();
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


void TeQtLegendSource::drawPointRep(int pw, int ph, int offset, int row, QPixmap *pixmap)
{
	QPainter	painter(pixmap);
	QColor		cor;
	TeColor		tcor;

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	TeGeomRepVisualMap& vm = legVec[row].getVisualMap();

	tcor = vm[TePOINTS]->color();
	cor.setRgb(tcor.red_, tcor.green_, tcor.blue_);
	int s = vm[TePOINTS]->style();
	int	w = vm[TePOINTS]->size();
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


void TeQtLegendSource::setCell(int row, int col, const QVariant &v)
{
	// col = 0 (Color)
	// col = 1 (From) mode != UniqueValue; (Value) mode = UniqueValue
	// col = 2 (To) mode != UniqueValue; (Label) mode = UniqueValue
	// col = 3 (Label) mode != UniqueValue; (Count) mode = UniqueValue
	// col = 4 (Count) mode != UniqueValue 

	if (col == 0)
	{
		emit dataChanged();
		return;
	}

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();

	TeGrouping& groupingParams = ((TeTheme*)appTheme_.getTheme())->grouping();
	TeGroupingMode groupingMode = groupingParams.groupMode_;

	if (groupingMode == TeUniqueValue)
	{
		if (legVec[row].label() == "Missing Data" && col == 2)
			return;
	}
	else
	{
		if (legVec[row].from() == "Missing Data" && col == 1 ||
			legVec[row].label() == "Missing Data" && col == 3 ||
			legVec[row].to() == "" && col == 2)
			return;
	}

	if (groupingMode == TeStdDeviation)
	{
		if (legVec[row].from().find("mean =") != string::npos  && col == 1 ||
			legVec[row].label().find("mean =") != string::npos  && col == 3 ||
			legVec[row].to() == "" && col == 2)
			return;
	}

	// Check if the value changed is numeric
	if(groupingMode != TeUniqueValue && (col == 1 || col == 2))
	{
		bool ok;
		QString qs = v.toString();
		qs.toDouble(&ok);
		if(ok == false)
			return;
	}

	QString qs = v.toString();
	string newValue = qs.latin1();

	// Check if the label was the value that was edited,
	// in this case set it to the new value
	if((groupingMode != TeUniqueValue && col == 3) ||
		(groupingMode == TeUniqueValue && col == 2))
	{
		legVec[row].label(newValue);
		return;
	}

	// The column edited is the "from" or "to" column for
	// a grouping mode that is different of the "Unique Value" mode
	vector<TeLegendEntry> tempLegVec = legVec;
	dValuesVec_.clear();
	((TeTheme*)appTheme_.getTheme())->buildGrouping(((TeTheme*)appTheme_.getTheme())->grouping(), TeAll, &dValuesVec_);
	legVec = tempLegVec;
	
	if (col == 1)
		legVec[row].from(newValue);
	else if (col == 2)
		legVec[row].to(newValue);

	// Make the label value to be the same as (from ~ to)
	if (col == 1 || col == 2)
	{
		string oldLabel = legVec[row].label();
		string newLabel = legVec[row].slice().from_;
		newLabel += " ~ ";
		newLabel += legVec[row].slice().to_;
		if ((newLabel != oldLabel) && (oldLabel.find(" ~ ") != string::npos))
			legVec[row].label(newLabel);
	}

	// Recalculate the new count value
	vector<TeSlice> sliceVec;
	TeSlice slice(legVec[row].from(), legVec[row].to());
	sliceVec.push_back(slice);
	TeElemCountingBySlice(dValuesVec_.begin(), dValuesVec_.end(), sliceVec);
	legVec[row].count(sliceVec[0].count_);

	emit dataChanged();
}
    
bool TeQtLegendSource::generateLegends(const TeGrouping& groupingParams, double minValue, double maxValue)
{
	((TeTheme*)appTheme_.getTheme())->grouping(groupingParams);

	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();

	legVec.clear();	
	dValuesVec_.clear();
	mapObjValVec_.clear();

	bool ret;
	TeChronon chronon = groupingParams.groupChronon_;
	if (chronon == TeNOCHRONON)
		ret = ((TeTheme*)appTheme_.getTheme())->buildGrouping(groupingParams, TeAll, &dValuesVec_);
	else
		ret = ((TeTheme*)appTheme_.getTheme())->buildGrouping(groupingParams, chronon, mapObjValVec_);

	if (ret == false)
	{
		errorMessage_ = tr("Fail to generate the legends");
		nRows_ = 0;
		return false;
	}

	// Set the number of rows
	nRows_ = legVec.size();

	// Set the number of columns
	TeGroupingMode groupingMode = groupingParams.groupMode_;
	if (groupingMode == TeUniqueValue)
		nCols_ = 4;
	else
		nCols_ = 5;

	// Change the legends if the minValue and maxValue are 
	// different of zero

	if (groupingMode == TeEqualSteps && (minValue == 0. && maxValue == 0.) == false)
	{
		double slice = (maxValue - minValue)/double(groupingParams.groupNumSlices_);
		int i;
		for (i = 0; i < groupingParams.groupNumSlices_; ++i)
		{
			string from = Te2String(minValue + double(i)*slice, groupingParams.groupPrecision_);
			legVec[i].from(from);
			string to = Te2String(minValue + double(i+1)*slice, groupingParams.groupPrecision_);
			legVec[i].to(to);
			string label = legVec[i].from() + " ~ " + legVec[i].to();
			legVec[i].label(label);
		}

		// Recalculate the new count value
		vector<TeSlice> sliceVec;
		for (i = 0; i < groupingParams.groupNumSlices_; ++i)
		{
			TeSlice slice(legVec[i].from(), legVec[i].to());
			sliceVec.push_back(slice);
		}
		TeElemCountingBySlice(dValuesVec_.begin(), dValuesVec_.end(), sliceVec);
		for (unsigned int j = 0; j < sliceVec.size(); ++j)
			legVec[j].count(sliceVec[j].count_);
	}

	return true;
}

void TeQtLegendSource::putColorOnLegend(vector<ColorBar>& iColorVec, vector<ColorBar>& iColorBVec)
{
	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	if(legVec.size() == 0)
		return;

	unsigned int i;
	bool hasMissingData = false;

	// Update the grouping colors of the theme
	string groupingColors = getColors(iColorVec, iColorBVec, ((TeTheme*)appTheme_.getTheme())->grouping().groupMode_);
	appTheme_.groupColor(groupingColors);

//	vector<string> colorNameVec;
//	getColorNameVector(groupingColors, colorNameVec);
	vector<TeColor> colorVec;

	int nColors = (int)legVec.size();
	string s = legVec[legVec.size()-1].from();
	if(s == "Missing Data")
	{
		hasMissingData = true;
		nColors--;
	}

	TeGrouping& groupingParams = ((TeTheme*)appTheme_.getTheme())->grouping();
	if (groupingParams.groupMode_ != TeStdDeviation)
	{
//		getColors(colorNameVec, nColors, colorVec);
		colorVec = getColors(iColorVec, nColors);
//		if (colorNameVec.size() == 1 && invertColor == true)
//		{
//			vector<TeColor> cVec;
//			for (int k = colorVec.size() - 1; k >= 0; --k)
//				cVec.push_back(colorVec[k]);
//			colorVec = cVec;
//		}
	}
	else
	{
		int leftColors = 0, rightColors = 0;

		for(i = 0; i < legVec.size(); ++i)
		{
			string vv = legVec[i].label();
			if (legVec[i].label().find("mean = ") != string::npos)
				break;
		}
		leftColors = i;
		rightColors = legVec.size() - i - 1;
		if(hasMissingData)
			rightColors--;

		vector<TeColor> leftColorVec, rightColorVec;
//		vector<string> leftColorNameVec, rightColorNameVec;
//		if ((colorNameVec.size() % 2) == 0)
//		{
//			unsigned int a = colorNameVec.size()/2;
//			for (i = 0; i < a; ++i)
//				leftColorNameVec.push_back(colorNameVec[i]);
//			for (; i < colorNameVec.size(); ++i)
//				rightColorNameVec.push_back(colorNameVec[i]);
//		}
//		else
//		{
//			unsigned int a = colorNameVec.size()/2 + 1;
//			for (i = 0; i < a; ++i)
//				leftColorNameVec.push_back(colorNameVec[i]);
//			for (i = a-1; i < colorNameVec.size(); ++i)
//				rightColorNameVec.push_back(colorNameVec[i]);
//		}
//
//		getColors(leftColorNameVec, leftColors, leftColorVec);
//		getColors(rightColorNameVec, rightColors, rightColorVec);
		leftColorVec = getColors(iColorVec, leftColors);
		rightColorVec = getColors(iColorBVec, rightColors);

//		if (leftColorNameVec.size() == 1 && invertColor == true)
//		{
//			vector<TeColor> cVec;
//			for (int k = leftColorVec.size() - 1; k >= 0; --k)
//				cVec.push_back(leftColorVec[k]);
//			leftColorVec = cVec;
//		}
//
//		if (rightColorNameVec.size() == 1 && invertColor == false)
//		{
//			vector<TeColor> cVec;
//			for (int k = rightColorVec.size() - 1; k >= 0; --k)
//				cVec.push_back(rightColorVec[k]);
//			rightColorVec = cVec;
//		}

		for (i = 0; i < leftColorVec.size(); ++i)
			colorVec.push_back(leftColorVec[i]);
		colorVec.push_back(leftColorVec[i-1]);
		for (i = 0; i < rightColorVec.size(); ++i)
			colorVec.push_back(rightColorVec[i]);
	}

	if(hasMissingData)
	{
		TeColor	cor(255, 255, 255);
		colorVec.push_back(cor);
	}

	int visRep = appTheme_.getTheme()->visibleRep();
	for(i = 0; i < legVec.size(); i++)
	{
		string vv = legVec[i].label();
		if (groupingParams.groupMode_ == TeStdDeviation &&
			legVec[i].label().find("mean = ") != string::npos)
			continue;

		TeColor	cor = colorVec[i];
		if (visRep & TePOINTS)
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TePOINTS);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TePOINTS); 
		}

		if (visRep & TeLINES)
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TeLINES);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TeLINES); 
		}

		if ((visRep & TePOLYGONS) || (visRep & TeCELLS))
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TePOLYGONS);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TePOLYGONS); 
		}
	}
}

void TeQtLegendSource::putColorOnLegend(string& groupingColors, bool invertColor)
{
	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	if(legVec.size() == 0)
		return;

	unsigned int i;
	bool hasMissingData = false;

	// Update the grouping colors of the theme
	appTheme_.groupColor(groupingColors);

	vector<string> colorNameVec;
	getColorNameVector(groupingColors, colorNameVec);
	vector<TeColor> colorVec;

	int nColors = (int)legVec.size();
	string s = legVec[legVec.size()-1].from();
	if(s == "Missing Data")
	{
		hasMissingData = true;
		nColors--;
	}

	TeGrouping& groupingParams = ((TeTheme*)appTheme_.getTheme())->grouping();
	if (groupingParams.groupMode_ != TeStdDeviation)
	{
		getColors(colorNameVec, nColors, colorVec);
		if (colorNameVec.size() == 1 && invertColor == true)
		{
			vector<TeColor> cVec;
			for (int k = colorVec.size() - 1; k >= 0; --k)
				cVec.push_back(colorVec[k]);
			colorVec = cVec;
		}
	}
	else
	{
		int leftColors = 0, rightColors = 0;

		for(i = 0; i < legVec.size(); ++i)
		{
			string vv = legVec[i].label();
			if (legVec[i].label().find("mean = ") != string::npos)
				break;
		}
		leftColors = i;
		rightColors = legVec.size() - i - 1;
		if(hasMissingData)
			rightColors--;

		vector<TeColor> leftColorVec, rightColorVec;
		vector<string> leftColorNameVec, rightColorNameVec;
		if ((colorNameVec.size() % 2) == 0)
		{
			unsigned int a = colorNameVec.size()/2;
			for (i = 0; i < a; ++i)
				leftColorNameVec.push_back(colorNameVec[i]);
			for (; i < colorNameVec.size(); ++i)
				rightColorNameVec.push_back(colorNameVec[i]);
		}
		else
		{
			unsigned int a = colorNameVec.size()/2 + 1;
			for (i = 0; i < a; ++i)
				leftColorNameVec.push_back(colorNameVec[i]);
			for (i = a-1; i < colorNameVec.size(); ++i)
				rightColorNameVec.push_back(colorNameVec[i]);
		}

		getColors(leftColorNameVec, leftColors, leftColorVec);
		getColors(rightColorNameVec, rightColors, rightColorVec);

		if (leftColorNameVec.size() == 1 && invertColor == true)
		{
			vector<TeColor> cVec;
			for (int k = leftColorVec.size() - 1; k >= 0; --k)
				cVec.push_back(leftColorVec[k]);
			leftColorVec = cVec;
		}

		if (rightColorNameVec.size() == 1 && invertColor == false)
		{
			vector<TeColor> cVec;
			for (int k = rightColorVec.size() - 1; k >= 0; --k)
				cVec.push_back(rightColorVec[k]);
			rightColorVec = cVec;
		}

		for (i = 0; i < leftColorVec.size(); ++i)
			colorVec.push_back(leftColorVec[i]);
		colorVec.push_back(leftColorVec[i-1]);
		for (i = 0; i < rightColorVec.size(); ++i)
			colorVec.push_back(rightColorVec[i]);
	}

	if(hasMissingData)
	{
		TeColor	cor(255, 255, 255);
		colorVec.push_back(cor);
	}

	for(i = 0; i < legVec.size(); i++)
	{
		string vv = legVec[i].label();
		if (groupingParams.groupMode_ == TeStdDeviation &&
			legVec[i].label().find("mean = ") != string::npos)
			continue;

		TeColor	cor = colorVec[i];
		if(((TeTheme*)appTheme_.getTheme())->visibleRep() & TePOINTS)
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TePOINTS);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TePOINTS); 
		}
		if(((TeTheme*)appTheme_.getTheme())->visibleRep() & TeLINES)
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TeLINES);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TeLINES); 
		}
		if((((TeTheme*)appTheme_.getTheme())->visibleRep() & TePOLYGONS) || (((TeTheme*)appTheme_.getTheme())->visibleRep() & TeCELLS))
		{
			TeVisual* v = ((TeTheme*)appTheme_.getTheme())->defaultLegend().visual(TePOLYGONS);	
			v->color(cor);
			legVec[i].setVisual(v->copy(), TePOLYGONS); 
		}
	}
}

void TeQtLegendSource::getColorNameVector(string& groupingColors, vector<string>& colorNameVec)
{
	if(groupingColors.empty())
		groupingColors = tr("R").latin1();

	QString s = groupingColors.c_str();
	QStringList ss = QStringList::split("-",s,true);

	for(unsigned int i = 0; i < ss.size(); i++)
	{
		QString a = ss[i];
		if(tr("R") == a)
			colorNameVec.push_back("RED");
		else if(tr("G") == a)
			colorNameVec.push_back("GREEN");
		else if(tr("B") == a)
			colorNameVec.push_back("BLUE");
		else if(tr("Cy") == a)
			colorNameVec.push_back("CYAN");
		else if(tr("Or") == a)
			colorNameVec.push_back("ORANGE");
		else if(tr("Mg") == a)
			colorNameVec.push_back("MAGENTA");
		else if(tr("Y") == a)
			colorNameVec.push_back("YELLOW");
		else
			colorNameVec.push_back("GRAY");
	}
}

void TeQtLegendSource::setLegends(const vector<TeLegendEntry>& inLegVec)
{
	vector<TeLegendEntry>& legVec = ((TeTheme*)appTheme_.getTheme())->legend();
	legVec.clear();
	legVec = inLegVec;
	nRows_ = legVec.size();
}

void TeQtLegendSource::setGroupingParams(const TeGrouping& gParams)
{
	TeGrouping& groupingParams = ((TeTheme*)appTheme_.getTheme())->grouping();
	groupingParams = gParams;

	// Set the number of columns
	TeGroupingMode groupingMode = groupingParams.groupMode_;
	if (groupingMode == TeUniqueValue)
		nCols_ = 4;
	else
		nCols_ = 5;
}




