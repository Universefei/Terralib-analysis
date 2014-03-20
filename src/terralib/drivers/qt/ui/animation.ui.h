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
#include <qdir.h>
#include <qvaluelist.h>
#include <qapplication.h>
//#include <terraViewBase.h>
#include <TeQtCanvas.h>
#include <TeQtAnimaThread.h>
#include <TeWaitCursor.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qdatetime.h>

void Animation::init()
{
	help_ = 0;
	canvas_ = 0;

	pauseButton->setEnabled(false);
	stopButton->setEnabled(false);
	playButton->setEnabled(false);
	playingScrollBar->setEnabled(false);
	velocitySlider->setEnabled(false);
	velocityTextLabel->setEnabled(false);
	frameNumbertextLabel->setEnabled(false);
	frameNumberLineEdit->setEnabled(false);
	loopCheckBox->setEnabled(false);
	velocitySlider->setMinValue(1);
	velocitySlider->setMaxValue(1000);
	velocitySlider->setValue(500);
}

void Animation::playButton_clicked()
{
	animaThread_.pause(false);
	animaThread_.pauseFrame(0);
	animaThread_.start();
	playButton->setEnabled(false);
	pauseButton->setEnabled(true);
	stopButton->setEnabled(true);
	playingScrollBar->setEnabled(true);
	directoryPushButton->setEnabled(false);
}


void Animation::pauseButton_clicked()
{
	directoryPushButton->setEnabled(false);
	bool p = !animaThread_.pause();
	animaThread_.pause(p);
	if(p == false)
		animaThread_.start();
}


void Animation::stopButton_clicked()
{
	animaThread_.stop();
	stopButton->setEnabled(false);
	pauseButton->setEnabled(false);
	playButton->setEnabled(true);
	playingScrollBar->setEnabled(false);
	directoryPushButton->setEnabled(true);
}


void Animation::velocitySlider_valueChanged( int v)
{
	string t = Te2String((double)v/10., 1);
	velocitylineEdit->setText(t.c_str());
}


void Animation::playingScrollBar_valueChanged( int f)
{
	if(animaThread_.pause())
		animaThread_.showFrame(f);
	else
		animaThread_.frame(f);
}


void Animation::closeEvent( QCloseEvent * )
{
	if(animaThread_.running())
		animaThread_.stop();
	hide();
}


void Animation::directoryPushButton_clicked()
{
	QString animaDir = QFileDialog::getExistingDirectory(
			QString::null,
			this,
			tr("Get Existing Directory"),
			tr("Select a directory"),
			true);
	setAnimaParams(animaDir, canvas_);
}


void Animation::setAnimaParams(QString& animaDir, TeQtCanvas* canvas)
{
	if (!animaDir.isEmpty())
	{
		TeWaitCursor wait;
		if(animaThread_.running())
			animaThread_.stop();

		QDir dir(animaDir);

		animaThread_.setAnimation(this, dir);	
		velocityTextLabel->setEnabled(true);
		velocitySlider->setEnabled(true);
		frameNumbertextLabel->setEnabled(true);
		loopCheckBox->setEnabled(true);

		animaThread_.canvas(canvas);	
		stopButton->setEnabled(false);
		pauseButton->setEnabled(false);
		playButton->setEnabled(true);
		playingScrollBar->setEnabled(false);
		directoryPushButton->setEnabled(false);

		QStringList fileList;
		fileList = dir.entryList(QDir::Files);
		if(fileList.count() == 0)
		{
			velocitySlider->setMaxValue(1000);
			velocitySlider->setValue(500);
			wait.resetWaitCursor();
			return;
		}
		QString s = *(fileList.at(0));
		QString ss = dir.absPath() + "/" + s;
		QPixmap pixmap(ss);
		QPixmap pix(pixmap.width(), pixmap.height());
		QTime time;
		time.start();
		pixmap.load(ss);
		bitBlt(&pix, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(), Qt::CopyROP);
		int ms = time.elapsed();
		time.restart();
		pixmap.load(ss);
		bitBlt(&pix, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(), Qt::CopyROP);
		ms += time.elapsed();
		ms /= 2;
		time.restart();
		pixmap.load(ss);
		bitBlt(&pix, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(), Qt::CopyROP);
		ms += time.elapsed();
		ms /= 2;

		int vmax = 20;
		if(ms)
			vmax = 10000/ms;
		velocitySlider->setMaxValue(vmax);
		velocitySlider->setValue(vmax/2);
	}
	else
	{
		QMessageBox::warning(this, tr("Warning"), tr("Select a directory!"));
		pauseButton->setEnabled(false);
		stopButton->setEnabled(false);
		playButton->setEnabled(false);
		playingScrollBar->setEnabled(false);
		velocitySlider->setEnabled(false);
		velocityTextLabel->setEnabled(false);
		frameNumbertextLabel->setEnabled(false);
		loopCheckBox->setEnabled(false);
	}
}

void Animation::velocitylineEdit_returnPressed()
{
	QString t = velocitylineEdit->text();
	double d = atof(t.latin1()) * 10.;
	int v = (int)d;
	velocitySlider->setValue(v);
}


void Animation::playDir( QString /* dir */)
{
	pauseButton->setEnabled(true);
	stopButton->setEnabled(true);
	playButton->setEnabled(true);
	playingScrollBar->setEnabled(true);
	velocitySlider->setEnabled(true);
	velocityTextLabel->setEnabled(true);
	frameNumbertextLabel->setEnabled(true);
	loopCheckBox->setEnabled(true);
	directoryPushButton->setEnabled(false);
	directoryPushButton_clicked();
	loopCheckBox->setChecked(true);
	playButton_clicked();
}

void Animation::setCanvas( TeQtCanvas* canvas )
{
	canvas_ = canvas;
}

void Animation::helpPushButton_clicked()
{
	if(help_)
		delete help_;

	help_ = new Help(this, "help", false);
	help_->init("animation.htm");
	if(help_->erro_ == false)
	{
		help_->show();
		help_->raise();
	}
	else
	{
		delete help_;
		help_ = 0;
	}
}
