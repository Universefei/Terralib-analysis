CONFIG += dll 
TARGET = stat

include (../config.pri)

DEFINES += STAT_AS_DLL

INCLUDEPATH += $$TERRALIBPATH/src/terralib/stat

LIBS += -lterralib

SOURCES	+= $$TERRALIBPATH/src/terralib/stat/filaDouble.cpp \
	  $$TERRALIBPATH/src/terralib/stat/filaInt.cpp \
	  $$TERRALIBPATH/src/terralib/stat/filaR.cpp \
	  $$TERRALIBPATH/src/terralib/stat/heap.cpp \
	  $$TERRALIBPATH/src/terralib/stat/lista.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeBayesFunctions.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeKernelFunctions.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterArvore.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterFunctions.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterGrafo.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeStatDataStructures.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeSemivarModelFactory.cpp \
	  $$TERRALIBPATH/src/terralib/stat/TeMSVFactory.cpp 

HEADERS	+= $$TERRALIBPATH/src/terralib/stat/erro.h \
	  $$TERRALIBPATH/src/terralib/stat/filaDouble.h \
	  $$TERRALIBPATH/src/terralib/stat/filaInt.h \
	  $$TERRALIBPATH/src/terralib/stat/filaR.h \
	  $$TERRALIBPATH/src/terralib/stat/heap.h \
	  $$TERRALIBPATH/src/terralib/stat/lista.h \
	  $$TERRALIBPATH/src/terralib/stat/TeBayesFunctions.h \
	  $$TERRALIBPATH/src/terralib/stat/TeKernelFunctions.h \
	  $$TERRALIBPATH/src/terralib/stat/TeKernelParams.h \
	  $$TERRALIBPATH/src/terralib/stat/TeKMeansGrouping.h \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterArvore.h \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterFunctions.h \
	  $$TERRALIBPATH/src/terralib/stat/TeSkaterGrafo.h \
	  $$TERRALIBPATH/src/terralib/stat/TeSpatialStatistics.h \
	  $$TERRALIBPATH/src/terralib/stat/TeStatDataStructures.h \
	  $$TERRALIBPATH/src/terralib/stat/TeSemivarModelFactory.h \
	  $$TERRALIBPATH/src/terralib/stat/TeMSVFactory.h \
          $$TERRALIBPATH/src/terralib/stat/TeStatDefines.h 

