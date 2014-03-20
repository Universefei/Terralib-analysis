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
#include <TeQtAnimaThread.h>
#include <TeQtCanvas.h>
#include <qpainter.h>
#include <qpaintdevice.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <animation.h>
#include <qdatetime.h>

void TeQtAnimaThread::run()
{
	bool init = false;
	stop_ = false;
	frame_ = pauseFrame_;
	frames_ = fileList_.count();
	animation_->playingScrollBar->setMinValue(0);
	animation_->playingScrollBar->setMaxValue(frames_-1);
	animation_->playingScrollBar->setValue(frame_);
	while(frame_ < frames_)
	{
		showFrame(frame_);
		if(pause_)
			frame_ = pauseFrame_;
		if(pause_ || stop_)
			break;

		int t = 10000 / animation_->velocitySlider->value() - 10000 / animation_->velocitySlider->maxValue();
		sleep(t);
		frame_++;
		if(frame_ == frames_)
		{
			if(animation_->loopCheckBox->isChecked())
			{
				sleep(t);
				sleep(t);
				frame_ = 0;
			}
			else
				init = true;
		}
	}
	if(init)
	{
		animation_->stopButton->setEnabled(false);
		animation_->pauseButton->setEnabled(false);
		animation_->playButton->setEnabled(true);
		animation_->playingScrollBar->setEnabled(false);
		showFrame(0);
	}
}

void TeQtAnimaThread::showFrame(int i)
{
	if((unsigned int)i >= fileList_.count())
		return;
	pauseFrame_ = i;
	QString s = *(fileList_.at(i));
	QString ss = dir_.absPath() + "/" + s;
	QPixmap pixmap(ss);
	QRect rect = pixmap.rect();
	QRect crect = canvas_->viewport()->rect();
	rect = crect.intersect(rect);
	QPaintDevice *device = canvas_->viewport();
	bitBlt(device, 0, 0, &pixmap, 0, 0, rect.width(), rect.height(), Qt::CopyROP);
	animation_->frameNumberLineEdit->setText(Te2String(pauseFrame_).c_str());
	animation_->playingScrollBar->setValue(pauseFrame_);
}

void TeQtAnimaThread::setAnimation(Animation* a, QDir d)
{
	animation_ = a;
	dir_ = d;
	fileList_ = dir_.entryList(QDir::Files);
}

void TeQtAnimaThread::stop()
{
	if(pause_)
		pause_=false;
	stop_ = true; wait();
}

void TeQtAnimaThread::sleep(int t)
{
	QTime time;
	time.start();

	int vel = animation_->velocitySlider->value();
	int frame = animation_->playingScrollBar->value();
	while(time.elapsed() < t)
	{
		if(pause_ || stop_ || vel != animation_->velocitySlider->value() || frame != animation_->playingScrollBar->value())
			break;
	}
}
