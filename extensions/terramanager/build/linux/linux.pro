# This file is for best compatibility with TerraOGC extension.
# To other TerraManager compilation projects see ../build

TEMPLATE = lib
CONFIG = warn_on rtti exceptions dll

contains( TE_PROJECT_TYPE, DEBUG ) {
  CONFIG += debug
  DEFINES += DEBUG
} else {
  CONFIG += release
  DEFINES += NDEBUG
}

# Destination
DESTDIR = ../../lib
TARGET = terramanager
OBJECTS_DIR = .obj

# Files section
INCLUDEPATH = \
  ../../src \
  ../../../../src/terralib/kernel \
  ../../../../src/terralib/functions \
  ../../../../src/terralib/utils \
  ../../../../src/terralib/drivers/shapelib \
  ../../../../src/terralib/stat \
  ../../../../src/tiff \
  ../../../../src/shapelib \
  ../../../../src/zlib \
  ../../../../src/terralib/drivers/libjpeg

HEADERS = \
	../../src/TeDecoderGDImage.h \
	../../src/TeGDCanvas.h \
	../../src/TeGDLegend.h \
	../../src/TeImageMapCanvas.h \
	../../src/TerraManagerConfig.h \
	../../src/TerraManagerDatatypes.h \
	../../src/TerraManagerException.h \
	../../src/TerraManager.h \
	../../src/TerraManagerUtils.h

SOURCES = \
	../../src/TeDecoderGDImage.cpp \
	../../src/TeGDCanvas.cpp \
	../../src/TeGDLegend.cpp \
	../../src/TeImageMapCanvas.cpp \
	../../src/TerraManager.cpp \
	../../src/TerraManagerException.cpp \
	../../src/TerraManagerUtils.cpp \
	../../../../src/terralib/utils/TeColorUtils.cpp \
	../../../../src/terralib/drivers/shapelib/TeExportSHP.cpp

# These 2 last included cpp files avoid 2 links in TerraOGC apps.

# Libs section
debug:LIBS += -L../../../../Debug/linux-g++
release:LIBS += -L../../../../Release/linux-g++

# Database section
contains( TE_DATABASE_OPTION , HAS_POSTGRESQL ) {
  DEFINES += HAS_POSTGRESQL

  INCLUDEPATH += \
	../../../../src/terralib/drivers/PostgreSQL \
	../../../../src/terralib/drivers/PostgreSQL/includepg
}

contains( TE_DATABASE_OPTION , HAS_MYSQL ) {
  DEFINES += HAS_MYSQL

  INCLUDEPATH += \
	../../../../src/mysql \
	../../../../src/terralib/drivers/MySQL
}

contains( TE_DATABASE_OPTION , HAS_OCI ) {
  DEFINES += HAS_OCI

  INCLUDEPATH += \
	../../../../src/terralib/drivers/Oracle \
	../../../../src/terralib/drivers/Oracle/OCI/include
}
