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

#include <qapplication.h>
#include <qmessagebox.h>

#include <terraView.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qsettings.h>
#include <qdir.h>
#include <display.h>

#include <TeErrorLog.h>
#include <TeSHPDriverFactory.h>
#include <TeDXFDriverFactory.h>
#include <TeOGRDriverFactory.h>
#include <TeAgnostic.h>

#include <stdlib.h>

#include <string>

static TeSHPDriverFactory shpFactory;
static TeDXFDriverFactory dxfFactory;
static TeOGRDriverFactory gmlFactory;

int main( int argc, char ** argv )
{

  #ifndef TEAGN_DEBUG_MODE
    try
    {
  #endif
  
      // Initiating QT application global instance
  
      QApplication a( argc, argv );
      
      // Creating the user config dir
      
      const std::string configDir = std::string( QDir::homeDirPath().ascii() )
        + QDir::separator() + "terraView" + QDir::separator();
        
      QDir dummyQdir;
        
      if( ! dummyQdir.exists( configDir.c_str(), true ) ) 
      {
        if( ! dummyQdir.mkdir( configDir.c_str(), true ) ) 
        {
          QMessageBox::critical(0, "TerraView", 
            "Unable to create application settings directory" );
        }
      }        
      
      // Initiating the error log instance
      
      const std::string logFileBaseName = configDir + QDir::separator() + 
        "terraView";      
      
      if( dummyQdir.exists( configDir.c_str(), true ) ) 
      {
        TeErrorLog::instance().startSession( logFileBaseName + "ExecLog.txt" );
      }
              
      // Redirecting the stdout and stderr do disk files
      
      if( dummyQdir.exists( configDir.c_str(), true ) ) 
      {
        freopen( ( logFileBaseName + "StdOutLog.txt" ).c_str(),
          "w", stdout );
        freopen( ( logFileBaseName + "StdErrLog.txt" ).c_str(),
          "w", stderr );
      }      
    
      // Fix for GeoTIFF CSV files
      char* getEnvReturnPtr = getenv( "GEOTIFF_CSV" );
      if( getEnvReturnPtr == 0 )
      {
        #ifdef WIN32
          std::string geoTiffCSVPath = ( "GEOTIFF_CSV=" + a.applicationDirPath() + "\\csv" ).ascii();
          putenv( geoTiffCSVPath.c_str() );
        #else
          std::string geoTiffCSVPath = ( a.applicationDirPath() + "/csv" ).ascii();
          setenv( "GEOTIFF_CSV", geoTiffCSVPath.c_str(), 1 );
        #endif
      }

	        QTranslator translator( 0 );
      QString qs = QTextCodec::locale();
      translator.load( QString("qt_") + QTextCodec::locale(), qApp->applicationDirPath() + "\\translation");
      a.installTranslator( &translator );
      QTranslator translator2( 0 );
      translator2.load( QString("terraView_") + QTextCodec::locale(), qApp->applicationDirPath() + "\\translation");
      a.installTranslator( &translator2 );
    
      QSettings settings;
      settings.beginGroup("/TerraView");
      QString dir = qApp->applicationDirPath();
      settings.writeEntry("/version", "3.2.0");
      settings.writeEntry("/installDirectory", dir.latin1());
      settings.endGroup();
    
      TerraView *w = new TerraView();
      w->setIcon(QPixmap::fromMimeSource("terralib.bmp"));
      a.setMainWidget(w);
      a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
      w->setQApplicationPointer( &a );
      w->loadPluginsSlot();
    
      #ifdef WIN32
        w->checkParamsPassing();
        w->showMaximized();
        w->getDisplayWindow()->showMaximized();
        w->show();
      #else
        w->show();
        w->showMaximized();
        w->getDisplayWindow()->showMaximized();
      #endif
    
      if( a.exec() != 0 )
      {
        TEAGN_LOGERR( "Invalid return from QApplication instance");
      }
  
  #ifndef TEAGN_DEBUG_MODE
    }
    catch( const TeException& e ) 
    {
      TEAGN_LOGERR( e.message() );
      
      TeErrorLog::instance().endSession();
      
      return EXIT_FAILURE;  
    }
    catch(...) 
    {
      TEAGN_LOGERR( "Execution ended abnormally");
      
      TeErrorLog::instance().endSession();
      
      return EXIT_FAILURE;
    }
  #endif  
  
  TEAGN_LOGMSG( "Execution ended");
  
  TeErrorLog::instance().endSession();

  return EXIT_SUCCESS;
}
