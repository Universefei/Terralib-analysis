#include "TeGeoDataDriver.h"

#include "TeDataTypes.h"
#include "TeAttribute.h"

TeGeoDataDriver::TeGeoDataDriver() 
	: isDataInited_(false)
	, isMetadataInited_(false)
	, numObjects_(0)
	, rep_(TeGEOMETRYNONE)
	, dataProjection_(0)
{
}

TeGeoDataDriver::~TeGeoDataDriver()
{
}

TeProjection* TeGeoDataDriver::getDataProjection()
{
	if(isMetadataInited() == false)
	{
		initMetadata();
	}
	return dataProjection_;
}

TeAttributeList& TeGeoDataDriver::getAttributesList() 
{
	if(isMetadataInited() == false)
	{
		initMetadata();
	}
	return attrList_;
}

int TeGeoDataDriver::getRepresentation() 
{
	if(isMetadataInited() == false)
	{
		initMetadata();
	}
	return rep_; 
}

TeBox TeGeoDataDriver::getBox() 
{
	if(isMetadataInited() == false)
	{
		initMetadata();
	}
	return box_;
}

TeGeoDataSet* TeGeoDataDriver::getData()
{
	if(isDataInited() == false)
	{
		if(initData() == false)
		{
			return 0;
		}
	}

	return getDataImpl();
}

TeGeoDataSet* TeGeoDataDriver::filter(const TeBox& box)
{
	if(isDataInited() == false)
	{
		if(initData() == false)
		{
			return 0;
		}
	}

	return filterImpl(box, "");
}

TeGeoDataSet* TeGeoDataDriver::filter(const std::string& attrRestriction)
{
	if(isDataInited() == false)
	{
		if(initData() == false)
		{
			return 0;
		}
	}

	return filterImpl(TeBox(), attrRestriction);
}

TeGeoDataSet* TeGeoDataDriver::filter(const TeBox& box, const std::string& attrRestriction)
{
	if(isDataInited() == false)
	{
		if(initData() == false)
		{
			return 0;
		}
	}

	return filterImpl(box, attrRestriction);
}

 bool TeGeoDataDriver::isDataInited() const
 {
	 return isDataInited_;
 }

 bool TeGeoDataDriver::isMetadataInited() const
 {
	 return isMetadataInited_;
 }