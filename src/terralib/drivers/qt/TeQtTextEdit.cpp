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

#include <TeQtTextEdit.h>
#include "../../kernel/TeDefines.h"

TeQtTextEdit::TeQtTextEdit()
{
	canvas_ = 0;
	edit_ = false;
	mode_ = 0;
}

TeQtTextEdit::TeQtTextEdit(TeText& tx, TeVisual v, TeQtCanvas* canvas)
{
	edit_ = true;
	mode_ = 0;
	canvas_ = canvas;
	text_ = tx;
	visual_ = v;
	change();
}

TeQtTextEdit::~TeQtTextEdit ()
{
}

void 
TeQtTextEdit::change(QPoint p, bool apxAngle)
{
	double angle, dx, dy;

	if(mode_ == 1) // change angle from left
	{
		dx = -(p.x() - pCenter_.x());
		dy = p.y() - pCenter_.y();

		if(apxAngle)
		{
			double fdx = fabs(dx);
			double fdy = fabs(dy);
			if(fdx < fdy)
			{
				if(4*fdx < fdy)
					dx = 0.;
				else
				{
					fdx = fdy;
					if(dx >= 0)
						dx = fdx;
					else
						dx = -fdx;
				}
			}
			else
			{
				if(4*fdy < fdx)
					dy = 0.;
				else
				{
					fdy = fdx;
					if(dy >= 0)
						dy = fdy;
					else
						dy = -fdy;
				}
			}
		}
		if(dx == 0)
		{
			if(dy > 0)
				angle = 90;
			else
				angle = 270;
		}
		else
		{
			if(dx > 0)
				angle = atan(dy/dx) * 180. / TePI;
			else
				angle = atan(dy/dx) * 180. / TePI + 180;
		}
		text_.setAngle(angle);
	}
	else if(mode_ == 2) // change angle from right
	{
		dx = p.x() - pCenter_.x();
		dy = -(p.y() - pCenter_.y());

		if(apxAngle)
		{
			double fdx = fabs(dx);
			double fdy = fabs(dy);
			if(fdx < fdy)
			{
				if(4*fdx < fdy)
					dx = 0.;
				else
				{
					fdx = fdy;
					if(dx >= 0)
						dx = fdx;
					else
						dx = -fdx;
				}
			}
			else
			{
				if(4*fdy < fdx)
					dy = 0.;
				else
				{
					fdy = fdx;
					if(dy >= 0)
						dy = fdy;
					else
						dy = -fdy;
				}
			}
		}
		if(dx == 0)
		{
			if(dy > 0)
				angle = 90;
			else
				angle = 270;
		}
		else
		{
			if(dx > 0)
				angle = atan(dy/dx) * 180. / TePI;
			else
				angle = atan(dy/dx) * 180. / TePI + 180;
		}
		text_.setAngle(angle);
	}
	else if(mode_ == 3 || mode_ == 4) // change size
	{
		if(visual_.fixedSize() == false)
		{
			double dx = fabs(double(pCenter_.x() - p.x()));
			double dy = fabs(double(pCenter_.y() - p.y()));
			int h = apx(sqrt(dx*dx + dy*dy));

			double dh, hc, c = 1.;

			hc = h * c;
			dh = canvas_->mapVtoDW((int)hc);
			text_.setHeight(dh);
			QRect r = canvas_->textRect (text_, visual_);
			while(r.height()/2 <= h-1)
			{
				c *= 1.01;
				hc = h * c;
				dh = canvas_->mapVtoDW((int)hc);
				text_.setHeight(dh);
				r = canvas_->textRect (text_, visual_);
			}
		}
	}
	else if(mode_ == 5) // change position
	{
		TeCoord2D pt = canvas_->mapVtoDW(p);
		text_.add(pt);
	}
	change();
}

void 
TeQtTextEdit::init(TeText& tx, TeVisual v, TeQtCanvas* canvas)
{
	edit_ = true;
	mode_ = 0;
	canvas_ = canvas;
	text_ = tx;
	visual_ = v;
	change();
}

void 
TeQtTextEdit::change()
{
	QRect rect = canvas_->textRect(text_, visual_);

	pCenter_ = rect.center();

	pLeft_ = rect.center();
	pLeft_.setX(rect.left()+3);

	pRight_ = rect.center();
	pRight_.setX(rect.right()-3);

	pTop_ = rect.center();
	pTop_.setY(rect.top()+3);

	pBottom_ = rect.center();
	pBottom_.setY(rect.bottom()-3);

	double angle = text_.angle();
	if(angle == 0)
	{
//		canvas_->plotOnWindow();
//		canvas_->plotTextRects(text_, visual_);
//		canvas_->plotOnPixmap0();
		return;
	}

	int	nx, ny;
	double	d;

	d = fabs(double(pRight_.x() - pCenter_.x()));
	nx = pCenter_.x() + apx(cos(angle*TePI/180.) * d);
	ny = pCenter_.y() - apx(sin(angle*TePI/180.) * d);
	pRight_ = QPoint(nx, ny);

	d = fabs(double(pLeft_.x() - pCenter_.x()));
	nx = pCenter_.x() + apx(cos((180.+angle)*TePI/180.) * d);
	ny = pCenter_.y() - apx(sin((180.+angle)*TePI/180.) * d);
	pLeft_ = QPoint(nx, ny);

	d = fabs(double(pTop_.y() - pCenter_.y()));
	nx = pCenter_.x() + apx(cos((90.+angle)*TePI/180.) * d);
	ny = pCenter_.y() - apx(sin((90.+angle)*TePI/180.) * d);
	pTop_ = QPoint(nx, ny);

	d = fabs(double(pBottom_.y() - pCenter_.y()));
	nx = pCenter_.x() + apx(cos((270.+angle)*TePI/180.) * d);
	ny = pCenter_.y() - apx(sin((270.+angle)*TePI/180.) * d);
	pBottom_ = QPoint(nx, ny);

//	canvas_->plotTextRects(text_, visual_);
}

int
TeQtTextEdit::apx(double d)
{
	if(d >= 0)
		return (int)(d+.5);
	else
		return (int)(d-.5);
}

int
TeQtTextEdit::located(QPoint& p)
{
	QRect rLeft(0, 0, 6, 6), rRight(0, 0, 6, 6), rTop(0, 0, 6, 6), rBottom(0, 0, 6, 6), rCenter(0, 0, 6, 6);
	rLeft.moveCenter(pLeft_);
	rRight.moveCenter(pRight_);
	rTop.moveCenter(pTop_);
	rBottom.moveCenter(pBottom_);
	rCenter.moveCenter(pCenter_);

	mode_ = 0;
	double d, m = TeMAXFLOAT;

	if(rLeft.contains(p))
	{
		QPoint a(rLeft.left() + rLeft.width()/2, rLeft.left() + rLeft.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 1;
	}
	if(rRight.contains(p))
	{
		QPoint a(rRight.left() + rRight.width()/2, rRight.left() + rRight.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 2;
	}
	if(rTop.contains(p))
	{
		QPoint a(rTop.left() + rTop.width()/2, rTop.left() + rTop.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 3;
	}
	if(rBottom.contains(p))
	{
		QPoint a(rBottom.left() + rBottom.width()/2, rBottom.left() + rBottom.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 4;
	}
	if(rCenter.contains(p))
	{
		QPoint a(rCenter.left() + rCenter.width()/2, rCenter.left() + rCenter.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		mode_ = 5;
	}

	if(mode_ == 1)
		pLeft_ = p;
	else if(mode_ == 2)
		pRight_ = p;
	else if(mode_ == 3)
		pTop_ = p;
	else if(mode_ == 4)
		pBottom_ = p;
	else if(mode_ == 5)
		pCenter_ = p;

	return mode_;
}

bool
TeQtTextEdit::isHotPoint(QPoint& p)
{
	QRect rLeft(0, 0, 6, 6), rRight(0, 0, 6, 6), rTop(0, 0, 6, 6), rBottom(0, 0, 6, 6), rCenter(0, 0, 6, 6);
	rLeft.moveCenter(pLeft_);
	rRight.moveCenter(pRight_);
	rTop.moveCenter(pTop_);
	rBottom.moveCenter(pBottom_);
	rCenter.moveCenter(pCenter_);

	mode_ = 0;
	double d, m = TeMAXFLOAT;

	if(rLeft.contains(p))
	{
		QPoint a(rLeft.left() + rLeft.width()/2, rLeft.left() + rLeft.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 1;
	}
	if(rRight.contains(p))
	{
		QPoint a(rRight.left() + rRight.width()/2, rRight.left() + rRight.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 2;
	}
	if(rTop.contains(p))
	{
		QPoint a(rTop.left() + rTop.width()/2, rTop.left() + rTop.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 3;
	}
	if(rBottom.contains(p))
	{
		QPoint a(rBottom.left() + rBottom.width()/2, rBottom.left() + rBottom.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		if(m == d)
			mode_ = 4;
	}
	if(rCenter.contains(p))
	{
		QPoint a(rCenter.left() + rCenter.width()/2, rCenter.left() + rCenter.height()/2);
		d = (p.x()-a.x())*(p.x()-a.x()) + (p.y()-a.y())*(p.y()-a.y());
		m = MIN(m, d);
		mode_ = 5;
	}

	if(mode_ > 0)
		return true;
	return false;
}

void
TeQtTextEdit::endEdit()
{
	edit_ = false;
	mode_ = 0;
}

int
TeQtTextEdit::mode()
{
	return mode_;
}

void
TeQtTextEdit::mode(int mode)
{
	mode_ = mode;
}

bool
TeQtTextEdit::edit()
{
	return edit_;
}

TeText
TeQtTextEdit::text()
{
	return text_;
}

void
TeQtTextEdit::text(TeText t)
{
	text_ = t;
}

void
TeQtTextEdit::height(double h)
{
	text_.setHeight(h);
}

void
TeQtTextEdit::angle(double a)
{
	text_.setAngle(a);
}

void
TeQtTextEdit::addPosition(TeCoord2D p)
{
	TeCoord2D tp = text_.location();
	tp += p;
	text_.add(tp);
}

TeQtCanvas*
TeQtTextEdit::canvas()
{
	return canvas_;
}

QRect
TeQtTextEdit::getRect()
{
	QRect rect = canvas_->textRect(text_, visual_);
	int	x1 = rect.left();
	int	y1 = rect.top();
	int	x2 = rect.right();
	int	y2 = rect.bottom();

	double angle = text_.angle() * -1;
	if(!(angle == 0 || angle == 90 || angle == 180 || angle == 270))
	{
		double d, dx, dy, alfa, beta;
		QPoint c = rect.center();
		QPoint p, p1, p2, p3, p4;

		p = QPoint(x1, y1);
		dx = p.x() - c.x();
		dy = p.y() - c.y();
		alfa = atan(dy/dx) * 180. / TePI;
		if(dx<0)
			alfa = 180. + alfa;
		else if(dx>=0 && dy<0)
			alfa = 360. + alfa;
		beta = alfa + angle;
		d = sqrt(double((p.x()-c.x()) * (p.x()-c.x()) + (p.y()-c.y()) * (p.y()-c.y())));
		p1.setX(c.x() + TeRound(cos(beta*TePI/180.) * d));
		p1.setY(c.y() + TeRound(sin(beta*TePI/180.) * d));

		p = QPoint(x1, y2);
		dx = p.x() - c.x();
		dy = p.y() - c.y();
		alfa = atan(dy/dx) * 180. / TePI;
		if(dx<0)
			alfa = 180. + alfa;
		else if(dx>=0 && dy<0)
			alfa = 360. + alfa;
		beta = alfa + angle;
		d = sqrt(double((p.x()-c.x()) * (p.x()-c.x()) + (p.y()-c.y()) * (p.y()-c.y())));
		p2.setX(c.x() + TeRound(cos(beta*TePI/180.) * d));
		p2.setY(c.y() + TeRound(sin(beta*TePI/180.) * d));

		p = QPoint(x2, y1);
		dx = p.x() - c.x();
		dy = p.y() - c.y();
		alfa = atan(dy/dx) * 180. / TePI;
		if(dx<0)
			alfa = 180. + alfa;
		else if(dx>=0 && dy<0)
			alfa = 360. + alfa;
		beta = alfa + angle;
		d = sqrt(double((p.x()-c.x()) * (p.x()-c.x()) + (p.y()-c.y()) * (p.y()-c.y())));
		p3.setX(c.x() + TeRound(cos(beta*TePI/180.) * d));
		p3.setY(c.y() + TeRound(sin(beta*TePI/180.) * d));

		p = QPoint(x2, y2);
		dx = p.x() - c.x();
		dy = p.y() - c.y();
		alfa = atan(dy/dx) * 180. / TePI;
		if(dx<0)
			alfa = 180. + alfa;
		else if(dx>=0 && dy<0)
			alfa = 360. + alfa;
		beta = alfa + angle;
		d = sqrt(double((p.x()-c.x()) * (p.x()-c.x()) + (p.y()-c.y()) * (p.y()-c.y())));
		p4.setX(c.x() + TeRound(cos(beta*TePI/180.) * d));
		p4.setY(c.y() + TeRound(sin(beta*TePI/180.) * d));

		x1 = MIN(p1.x(), p2.x());
		x1 = MIN(x1, p3.x());
		x1 = MIN(x1, p4.x());
		y1 = MIN(p1.y(), p2.y());
		y1 = MIN(y1, p3.y());
		y1 = MIN(y1, p4.y());
		x2 = MAX(p1.x(), p2.x());
		x2 = MAX(x2, p3.x());
		x2 = MAX(x2, p4.x());
		y2 = MAX(p1.y(), p2.y());
		y2 = MAX(y2, p3.y());
		y2 = MAX(y2, p4.y());

		rect.setLeft(x1);
		rect.setTop(y1);
		rect.setRight(x2);
		rect.setBottom(y2);
	}
	return rect;
//	QRect rect;
//	int	x1, y1, x2, y2;

//	x1 = pLeft_.x();
//	x1 = MIN(x1, pRight_.x());
//	x1 = MIN(x1, pTop_.x());
//	x1 = MIN(x1, pBottom_.x());

//	y1 = pLeft_.y();
//	y1 = MIN(y1, pRight_.y());
//	y1 = MIN(y1, pTop_.y());
//	y1 = MIN(y1, pBottom_.y());

//	x2 = pLeft_.x();
//	x2 = MAX(x2, pRight_.x());
//	x2 = MAX(x2, pTop_.x());
//	x2 = MAX(x2, pBottom_.x());

//	y2 = pLeft_.y();
//	y2 = MAX(y2, pRight_.y());
//	y2 = MAX(y2, pTop_.y());
//	y2 = MAX(y2, pBottom_.y());

//	if(text_.angle() == 0 || text_.angle() == 90 || text_.angle() == 180 || text_.angle() == 270)
//		rect = QRect(x1-3, y1-3, x2-x1+7, y2-y1+7);
//	else
//	{
//		int h = y2 - y1;
//		int w = x2 - x1;
//		int	d = h;
//		if(w < h)
//			d = w;
//		x1 -= d;
//		y1 -= d;
//		x2 += d;
//		y2 += d;
//		rect = QRect(x1, y1, x2-x1, y2-y1);
//	}
//	return rect;
}

QRect
TeQtTextEdit::getHRect()
{
	return canvas_->textRect(text_, visual_);
}

TeQtMultiTextEdit::TeQtMultiTextEdit()
{
	current_ = -1;
	selMode_ = false;
}

TeQtMultiTextEdit::~TeQtMultiTextEdit()
{
}

int
TeQtMultiTextEdit::size()
{
	return textVec_.size();
}

void
TeQtMultiTextEdit::push(TeQtTextEdit t)
{
	int geomId = t.text().geomId();
	if(geomIdSet_.find(geomId) == geomIdSet_.end())
	{
		geomIdSet_.insert(geomId);
		textVec_.push_back(t);
		current_ = textVec_.size() - 1;
	}
}

void
TeQtMultiTextEdit::change()
{
	if(current_ == -1 || textVec_.empty())
		return;

//	canvas->plotOnWindow();
	unsigned int i;
	for(i=0; i<textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		t.change();
	}
}

void
TeQtMultiTextEdit::change(QPoint p, bool apxAngle)
{
	if(current_ == -1 || textVec_.empty())
		return;

	TeQtTextEdit& t = textVec_[current_];
	TeCoord2D pa = t.text().location();
	t.change(p, apxAngle);
	TeCoord2D pb = t.text().location();
	TeCoord2D poffset(pb.x()-pa.x(), pb.y()-pa.y());

	if(t.mode()==1 || t.mode()==2)
		angle(t.text().angle());
	else if(t.mode()==3 || t.mode()==4)
		height(t.text().height());
	else if(t.mode() == 5)
		addPosition(poffset);
	change();
}

int 
TeQtMultiTextEdit::located(QPoint& p)
{
	unsigned int i;
	int mode = 0;
	for(i=0; i<textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		mode = t.located(p);
		if(mode)
		{
			current_ = i;
			break;
		}
	}
	return mode;
}

bool
TeQtMultiTextEdit::isHotPoint(QPoint& p)
{
	if(textVec_.size() == 0)
		return false;

	unsigned int i;
	for(i=0; i<textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		if(t.isHotPoint(p))
		{
			current_ = i;
			break;
		}
	}
	if(i < textVec_.size())
		return true;
	return false;
}

void
TeQtMultiTextEdit::endEdit()
{
	unsigned int i;
	for(i=0; i<textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		t.endEdit();
	}
	textVec_.clear();
	geomIdSet_.clear();
}

int
TeQtMultiTextEdit::mode()
{
	if(current_ >= 0 && textVec_.empty() == false)
		return textVec_[current_].mode();
	return 0;
}

void
TeQtMultiTextEdit::mode(int m)
{
	if(current_ >= 0 && textVec_.empty() == false)
		textVec_[current_].mode(m);
}

bool
TeQtMultiTextEdit::selMode()
{
	return selMode_;
}

void
TeQtMultiTextEdit::selMode(bool m)
{
	selMode_ = m;
}

bool
TeQtMultiTextEdit::edit()
{
	if(current_ >= 0 && textVec_.empty() == false)
		return textVec_[current_].edit();
	return false;
}

TeText
TeQtMultiTextEdit::text(int i)
{
	TeText t;
	if(i<(int)textVec_.size())
		return textVec_[i].text();
	else
		return t;
}

TeVisual
TeQtMultiTextEdit::visual(int i)
{
	TeVisual v;
	if(i<(int)textVec_.size())
		return textVec_[i].visual();
	else
		return v;
}

void
TeQtMultiTextEdit::height(double h)
{
	int i;
	for(i=0; i<(int)textVec_.size(); i++)
	{
		if(i == current_)
			continue;
		TeQtTextEdit& t = textVec_[i];
		t.height(h);
	}
}

void
TeQtMultiTextEdit::angle(double a)
{
	int i;
	for(i=0; i<(int)textVec_.size(); i++)
	{
		if(i == current_)
			continue;
		TeQtTextEdit& t = textVec_[i];
		t.angle(a);
	}
}

void
TeQtMultiTextEdit::addPosition(TeCoord2D p)
{
	int i;
	for(i=0; i<(int)textVec_.size(); i++)
	{
		if(i == current_)
			continue;
		TeQtTextEdit& t = textVec_[i];
		t.addPosition(p);
	}
}

QRect
TeQtMultiTextEdit::getRect()
{
	QRect	rect;
	int i;
	for(i=0; i<(int)textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		rect = rect | t.getRect();
	}
	return rect;
}

int
TeQtMultiTextEdit::getTextIndex(QPoint p)
{
	int i;
	for(i=0; i<(int)textVec_.size(); i++)
	{
		TeQtTextEdit& t = textVec_[i];
		QRect rect = t.getHRect();
		TeText tx = t.text();
		double angle = tx.angle() * -1;

		if(angle == 0)
		{
			if(rect.contains(p, true))
				return i;
		}
		else
		{
			double d, dx, dy, alfa, beta;
			QPoint c = rect.center();

			dx = p.x() - c.x();
			dy = p.y() - c.y();
			alfa = atan(dy/dx) * 180. / TePI;
			if(dx<0)
				alfa = 180. + alfa;
			else if(dx>=0 && dy<0)
				alfa = 360. + alfa;
			beta = alfa - angle;
			d = sqrt(double((p.x()-c.x()) * (p.x()-c.x()) + (p.y()-c.y()) * (p.y()-c.y())));
			int x = c.x() + TeRound(cos(beta*TePI/180.) * d);
			int y = c.y() + TeRound(sin(beta*TePI/180.) * d);

			QPoint pp(x, y);
			if(rect.contains(pp))
				return i;
		}
	}
	return -1;
}

void
TeQtMultiTextEdit::updateText(int index, string t)
{
	TeText tx = textVec_[index].text();
	tx.setTextValue(t);
	textVec_[index].text(tx);
	textVec_[index].change();
}

void
TeQtMultiTextEdit::visual(const TeVisual& visual)
{
	unsigned int i;
	for(i=0; i<textVec_.size(); i++)
		textVec_[i].visual(visual);
}
