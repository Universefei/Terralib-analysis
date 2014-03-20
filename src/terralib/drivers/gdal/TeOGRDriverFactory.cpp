#include "TeOGRDriverFactory.h"
#include "TeOGRDriver.h"

// TerraLib include files
#include "../../kernel/TeGeoDataDriver.h"

// OGR
#include <ogrsf_frmts.h>

TeOGRDriverFactory::TeOGRDriverFactory() :
TeGDriverFactory("OGR")
{}

TeOGRDriverFactory::~TeOGRDriverFactory() 
{}

TeGeoDataDriver * TeOGRDriverFactory::build(TeGDriverParams * params) 
{
	TeGeoDataDriver* driver = 0;

	if(params != 0 && !params->fileName_.empty())
		driver = new TeOGRDriver(params->fileName_, params->writing_);

	return driver;
}

TeGeoDataDriver* TeOGRDriverFactory::build() 
{
	return 0;
}

void TeOGRDriverFactory::loadGeoFilesInfo()
{
	OGRRegisterAll();
	int ndrivers = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	for(int i = 0; i < ndrivers; ++i)
	{
		// Gest OGR i-th drivers...
		OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i);
		// Builds the geo file info structure...
		TeGeoFileInfo info;
		info.setId("OGR");

		std::string name = driver->GetName();
		if(!isDriverFile(name, info)) // Is an OGR driver file?!
			continue;

		bool createSupport = driver->TestCapability("ODrCCreateDataSource"); // Has creation support?
		info.setCreateSupport(createSupport);

		_geoFilesInfo.push_back(info);
	}
}

// It's a test!
// ? OGR no answer... metadata is poor...
bool TeOGRDriverFactory::isDriverFile(const std::string& ogrDriver, TeGeoFileInfo& info)
{
	/*if(ogrDriver == "ESRI Shapefile")
	{
		info.setDescription("ESRI Shapefile");
		info.setExtension("shp");
		return true;
	}*/

	/*if(ogrDriver == "DXF")
	{
		info.setDescription("AutoCAD DXF");
		info.setExtension("dxf");
		return true;
	}*/

	if(ogrDriver == "BNA")
	{
		info.setDescription("Atlas BNA");
		info.setExtension("bna");
		return true;
	}

	if(ogrDriver == "DGN")
	{
		info.setDescription("Microstation DGN");
		info.setExtension("dgn");
		return true;
	}

	if(ogrDriver == "MapInfo File")
	{
		info.setDescription("MapInfo File");
		info.setExtension("mif");
		return true;
	}

	if(ogrDriver == "GML")
	{
		info.setDescription("Geography Markup Language");
		info.setExtension("gml");
		return true;
	}

	// continue...

	return false;
}
