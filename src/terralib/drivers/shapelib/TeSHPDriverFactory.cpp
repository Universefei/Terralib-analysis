#include "TeSHPDriverFactory.h"
#include "../../kernel/TeGeoDataDriver.h"
#include "TeDriverSHPDBF.h"

TeSHPDriverFactory::TeSHPDriverFactory() :
TeGDriverFactory("SHP")
{
  // Bouml preserved body begin 0005C282
  // Bouml preserved body end 0005C282
}

TeSHPDriverFactory::~TeSHPDriverFactory() 
{
  // Bouml preserved body begin 0005C302
  // Bouml preserved body end 0005C302
}

TeGeoDataDriver * TeSHPDriverFactory::build(TeGDriverParams * params) 
{
  // Bouml preserved body begin 0005C202
	TeShapefileDriver* driver = NULL;

	if(params != NULL && !params->fileName_.empty())
		driver = new TeShapefileDriver(params->fileName_);

	return driver;
  // Bouml preserved body end 0005C202
}

TeGeoDataDriver * TeSHPDriverFactory::build() 
{
  // Bouml preserved body begin 0005C382
	return NULL;
  // Bouml preserved body end 0005C382
}

void TeSHPDriverFactory::loadGeoFilesInfo()
{
	TeGeoFileInfo info;
	info.setId("SHP");
	info.setDescription("ESRI Shapefile");
	info.setExtension("shp");
	info.setCreateSupport(false);
	_geoFilesInfo.push_back(info);
}
