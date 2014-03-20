/************************************************************************************
TerraCrime - visualization and exploration of geographical databases
using TerraLib.
Copyright © 2003-2004 INPE and LESTE/UFMG.
This file is part of TerraCrime. TerraCrime is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraCrime.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

#include <qapplication.h>
#include <terraStat.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qsettings.h>
#include <qdir.h>


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QTranslator translator( 0 );
  	QString qs = QTextCodec::locale();
    translator.load( QString("terraStat_") + QTextCodec::locale(), "." );
    a.installTranslator( &translator );

	  QSettings settings;
	  settings.beginGroup("/TerraCrime");
	  settings.writeEntry("/version", "2.0 Beta");
  	settings.writeEntry("/installDirectory", QDir::currentDirPath().latin1());
  	settings.endGroup();

    TerraStat *w = new TerraStat();
  	a.setMainWidget(w);
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
