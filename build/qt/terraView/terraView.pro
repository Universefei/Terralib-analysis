TARGET = terraView
CONFIG += dll
include (../config.pri)
CONFIG += qt

debug:CONFIG += console

TEMPLATE = app

win32 {
      win32-g++ {
		CONFIG -= dll
#		CONFIG += 
      }
	  else {
	    debug:QMAKE_LIBDIR += $$TERRALIBPATH/Debug/terraViewCore \
				$$TERRALIBPATH/Debug/te_dxf \
				$$TERRALIBPATH/Debug/te_shapelib \
				$$TERRALIBPATH/Debug/terralib 
	    release {
		   QMAKE_LIBDIR += $$TERRALIBPATH/Release/terraViewCore \
		   $$TERRALIBPATH/Release/te_dxf \
		   $$TERRALIBPATH/Release/te_shapelib \
		   $$TERRALIBPATH/Release/terralib 
 	    
	           CONFIG += windows
        }
      }

      DEFINES += UNICODE
}

unix:LIBS += -lclntsh -lnnz10  -licudata -licuuc -licui18n -lfbembed

LIBS += -ldxf -lshapelib -lterralibtiff -lterralibpdi -lstat -lqwt -lte_functions -lspl -lte_utils -lte_qwt -lte_spl -lte_mysql -lte_postgresql \
	-lte_oracle -lte_firebird -lte_dxf -lte_shapelib -lterralib -lterraViewCore -lte_apputils

INCLUDEPATH += $$TERRALIBPATH/terraView \
	      $$TERRALIBPATH/src/qwt/include \
	      $$TERRALIBPATH/src/terralib/drivers/qwt \
	      $$TERRALIBPATH/src/terralib/drivers/shapelib \
	      $$TERRALIBPATH/src/terralib/drivers/spl \
	      $$TERRALIBPATH/src/terralib/drivers/DXF \
	      $$TERRALIBPATH/src/terralib/kernel \
	      $$TERRALIBPATH/src/terralib/utils \
	      $$TERRALIBPATH/src/terralib/functions \
	      $$TERRALIBPATH/src/terralib/stat \
	      $$TERRALIBPATH/src/shapelib \
	      $$TERRALIBPATH/src/zlib \
	      $$TERRALIBPATH/src/libspl \
	      $$TERRALIBPATH/ui/terraView \
	      $$TERRALIBPATH/src/terralib/drivers/qt \
	      $$TERRALIBPATH/src/terralib/drivers/MySQL \
	      $$TERRALIBPATH/src/terralib/drivers/MySQL/include \ 
	      $$TERRALIBPATH/src/terralib/drivers/Oracle \
	      $$TERRALIBPATH/src/terralib/drivers/Oracle/OCI/include \
	      $$TERRALIBPATH/src/terralib/drivers/PostgreSQL \
	      $$TERRALIBPATH/src/terralib/drivers/PostgreSQL/includepg \
	      $$TERRALIBPATH/src/terralib/drivers/Firebird \
	      $$TERRALIBPATH/src/terralib/image_processing \
	      $$TERRALIBPATH/src/appUtils
#unix:INCLUDEPATH += ./linux/ui 

SOURCES += $$TERRALIBPATH/terraView/main.cpp 

trans.path = $$DESTDIR
trans.files = $$TERRALIBPATH/terraView/*.qm

INSTALLS += trans
