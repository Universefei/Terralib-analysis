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

#include <qdir.h>
#include <TeWaitCursor.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qtextcodec.h>

void Help::init( QString helpFile )
{
	std::string		strLocale;
	std::string		strPath;

	erro_ = false;
	QDir dir;

	strLocale=QTextCodec::locale();
	if(strLocale.find("br") != std::string::npos)			strPath="pt_br";
	else if(strLocale.find("pt") != std::string::npos)		strPath="pt_br";
	else if(strLocale.find("es") != std::string::npos)		strPath="es";
	else if(strLocale.find("en") != std::string::npos)		strPath="en_us";

	QString absFile = qApp->applicationDirPath() + "/doc/" + strPath.c_str() + "/";
	absFile += helpFile;
	std::string path = absFile.latin1();
	QFileInfo info(absFile);
	bool exist = info.exists();
	bool isReadable = info.isReadable();
	if(exist==false || isReadable==false)
	{
		if(dir.cdUp())
		{
			if(dir.cdUp())
			{
				absFile = dir.absPath() + "/doc/" + strPath.c_str() + "/";
				absFile += helpFile;
				std::string path = absFile.latin1();
				QFileInfo info(absFile);
				exist = info.exists();
				isReadable = info.isReadable();
				if(exist==false || isReadable==false)
					erro_ = true;;
			}
			else
				erro_ = true;;
		}
		else
			erro_ = true;;
	}

	if(erro_)
	{
		QMessageBox::information(this, this->caption(), tr("The help of this interface is not available yet!")); 
		return;
	}

    connect(homePushButton, SIGNAL(clicked()), textBrowser, SLOT(home()));
    connect(backwardPushButton, SIGNAL(clicked()), this, SLOT(backward()));
    connect(forwardPushButton, SIGNAL(clicked()), this, SLOT(forward()));
    connect(textBrowser, SIGNAL(linkClicked(const QString&)), this, SLOT(linkClicked(const QString&)));
    connect(textBrowser, SIGNAL(anchorClicked(const QString&, const QString&)), this, SLOT(anchorClicked(const QString&, const QString&)));

	textBrowser->setSource(absFile);
	erro_ = false;
}


void Help::reject()
{
	hide();
}


void Help::printPushButton_clicked()
{
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage(true);
    printer.setOptionEnabled(QPrinter::PrintSelection, false);
    printer.setOptionEnabled(QPrinter::PrintPageRange, false);

    if ( printer.setup( this ) )
	{
        QPainter p( &printer );
        if( !p.isActive() ) // starting printing failed
            return;
        QPaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
        QRect body( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
        QSimpleRichText richText( textBrowser->text(),
                                  textBrowser->font(),
                                  textBrowser->context(),
                                  textBrowser->styleSheet(),
                                  textBrowser->mimeSourceFactory(),
                                  body.height() );
        richText.setWidth( &p, body.width() );
        QRect view( body );

		int page = 1;
		do
		{
			richText.draw( &p, body.left(), body.top(), view, colorGroup() );
			view.moveBy( 0, body.height() );
			p.translate( 0 , -body.height() );
			p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
						view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
			if ( view.top()  >= richText.height() )
				break;
			printer.newPage();
			page++;
		} while (TRUE);
	}
}


void Help::fontSizeComboBox_activated( const QString& /* size */ )
{
//	QFont font = textBrowser->font();
//	font.setPointSize(size.toInt());
//	font.setBold(boldCheckBox->isChecked());
//	textBrowser->setFont(font);
//	textBrowser->update();
}


void Help::boldCheckBox_clicked()
{
//	fontSizeComboBox_activated(fontSizeComboBox->currentText());
}


void Help::backward()
{
	if(paragVec_.size() == 0)
	{
		textBrowser->home();
		return;
	}

	textBrowser->backward();

	int para = paragVec_[paragVec_.size()-1];
	paragVec_.pop_back();
	int index = indexVec_[indexVec_.size()-1];
	indexVec_.pop_back();

	textBrowser->setCursorPosition(para, index);
	textBrowser->ensureCursorVisible();
}


void Help::forward()
{
	int para, index;

	textBrowser->getCursorPosition(&para, &index);

	paragVec_.push_back(para);
	indexVec_.push_back(index);

	textBrowser->forward();
}


void Help::linkClicked(const QString & /* link */ )
{
	int para, index;

	textBrowser->getCursorPosition(&para, &index);

	paragVec_.push_back(para);
	indexVec_.push_back(index);
}


void Help::anchorClicked(const QString & /* link */, const QString & /* anchor */ )
{
}


void Help::scrollToAnchor( const QString &name)
{
	textBrowser->scrollToAnchor(name);
}
