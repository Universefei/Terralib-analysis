#include "TeDXFDriverFactory.h"
#include "TeDXFDriver.h"
#include "TeGeoDataDriver.h"

TeDXFDriverFactory::TeDXFDriverFactory() :
TeGDriverFactory("DXF")
{
}

TeDXFDriverFactory::~TeDXFDriverFactory() 
{
}

TeGeoDataDriver * TeDXFDriverFactory::build(TeGDriverParams * params) 
{
	TeGeoDataDriver* driver = NULL;

	if(params != NULL && !params->fileName_.empty())
		driver = new TeDXFDriver(params->fileName_);

	return driver;
}

TeGeoDataDriver* TeDXFDriverFactory::build() 
{
	return NULL;
}

void TeDXFDriverFactory::loadGeoFilesInfo()
{
	TeGeoFileInfo info;
	info.setId("DXF");
	info.setDescription("AutoCAD DXF");
	info.setExtension("dxf");
	info.setCreateSupport(false);
	_geoFilesInfo.push_back(info);
}
