#include "TeSTGeoDataDriver.h"

#include "TeSTGeoDataSet.h"
#include "TeSTElementSet.h"

TeSTGeoDataDriver::TeSTGeoDataDriver()
	: dataSet_(0)
{
}

TeSTGeoDataDriver::~TeSTGeoDataDriver()
{
}

TeGeoDataSet* TeSTGeoDataDriver::getDataImpl()
{
	if(dataSet_ == 0)
	{
		return 0;
	}

	TeSTGeoDataSet* dataSet = new TeSTGeoDataSet(dataSet_);
	return dataSet;
}

TeGeoDataSet* TeSTGeoDataDriver::filterImpl(const TeBox& box, const std::string& attrRestriction)
{
	if(dataSet_ == 0)
	{
		return 0;
	}

	if(box.isValid() && attrRestriction.empty())
	{
		std::vector<int> vecFilter;
		dataSet_->search(box, vecFilter);

		TeSTGeoDataSet* dataSet = new TeSTGeoDataSet(dataSet_, vecFilter);
		return dataSet;
	}

	return 0;
}
