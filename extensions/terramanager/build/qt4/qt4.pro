# This file is for best compatibility with TerraOGC extension.
TERRALIBPATH = ../../../..
TEOGC = $${TERRALIBPATH}/../terraogc
TERRALIB_SRC_DIR = $${TERRALIBPATH}/src
LIBTERRAMANAGER_SRC_DIR = ../../src
TEOGCBUILD = $${TEOGC}/build/qt4

TARGET = terramanager

include ($${TEOGCBUILD}/config.pri)

DEFINES += TERRAMANAGERDLL

INCLUDEPATH += \
  ../../src \
  $${TERRALIB_SRC_DIR}/terralib/kernel \
  $${TERRALIB_SRC_DIR}/terralib/functions \
  $${TERRALIB_SRC_DIR}/terralib/utils \
  $${TERRALIB_SRC_DIR}/terralib/drivers/shapelib \
  $${TERRALIB_SRC_DIR}/terralib/stat \
  $${TERRALIB_SRC_DIR}/tiff \
  $${TERRALIB_SRC_DIR}/shapelib \
  $${TERRALIB_SRC_DIR}/zlib \
  $${TERRALIB_SRC_DIR}/terralib/drivers/libjpeg

LIBS -= -lterraogccommon

LIBS += -lte_utils -lte_shapelib

win32 {
    INCLUDEPATH += $(GD_DIR)/include
    win32-g++:LIBS += -L$(GD_DIR)/lib/mingw
    win32-msvc2003:QMAKE_LIBDIR += $(GD_DIR)/lib/msvc2003

    LIBS += -lbgd
}

contains( TE_DATABASE_OPTION , HAS_POSTGRESQL ) {
  DEFINES += HAS_POSTGRESQL

  INCLUDEPATH += \
	$$TERRALIB_SRC_DIR/terralib/drivers/PostgreSQL \
	$$TERRALIB_SRC_DIR/terralib/drivers/PostgreSQL/includepg

    LIBS += -lte_postgresql
}

contains( TE_DATABASE_OPTION , HAS_MYSQL ) {
  DEFINES += HAS_MYSQL

  INCLUDEPATH += \
    $$TERRALIB_SRC_DIR/mysql \
    $$TERRALIB_SRC_DIR/terralib/drivers/MySQL

    LIBS += -lte_mysql
}

contains( TE_DATABASE_OPTION , HAS_OCI ) {
  DEFINES += HAS_OCI

  INCLUDEPATH += \
	$$TERRALIB_SRC_DIR/terralib/drivers/OracleSpatial \
	$$TERRALIB_SRC_DIR/terralib/drivers/OracleSpatial/OCI/include

    LIBS += -lte_oracle
}

HEADERS = \
    ../../src/TeDecoderGDImage.h \
    ../../src/TeGDCanvas.h \
    ../../src/TeImageMapCanvas.h \
    ../../src/TeGDLegend.h \
    ../../src/TerraManagerException.h \
    ../../src/TerraManagerUtils.h \
    ../../src/TerraManagerConfig.h \
    ../../src/TerraManagerDatatypes.h \
    ../../src/TerraManager.h

SOURCES = \
    ../../src/TeDecoderGDImage.cpp \
    ../../src/TeGDCanvas.cpp \
    ../../src/TeImageMapCanvas.cpp \
    ../../src/TerraManager.cpp \
    ../../src/TerraManagerUtils.cpp \
    ../../src/TerraManagerException.cpp \
    ../../src/TeGDLegend.cpp

