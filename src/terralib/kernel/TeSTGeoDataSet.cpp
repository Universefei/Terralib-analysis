#include "TeSTGeoDataSet.h"

#include "TeSTElementSet.h"

TeSTGeoDataSet::TeSTGeoDataSet(TeSTElementSet* dataSet)
	: TeGeoDataSet()
	, dataSet_(dataSet)
{
	if(dataSet_ == 0)
	{
		size_ = 0;
		return;
	}
	size_ = dataSet_->numElements();
}

TeSTGeoDataSet::TeSTGeoDataSet(TeSTElementSet* dataSet, std::vector<int>& vecFilter)
	: TeGeoDataSet()
	, dataSet_(dataSet)
	, vecFilter_(vecFilter)
{
	if(dataSet_ == 0)
	{
		size_ = 0;
		return;
	}
	size_ = vecFilter.size();
}

TeSTGeoDataSet::~TeSTGeoDataSet()
{
}

bool TeSTGeoDataSet::getAttribute(const int& index, const int& col, std::string& value)
{
	if(dataSet_ == 0)
	{
		return false;
	}

	TeSTInstance* instance = dataSet_->getSTInstance(getRealIndex(index));
	if(instance == 0)
	{
		return false;
	}

	if(instance->getPropertyValue(value, col) == false)
	{
		return false;
	}

	return true;
}

bool TeSTGeoDataSet::getAttributes(const int& index, std::vector<std::string>& vecAtributes) 
{ 
	if(dataSet_ == 0)
	{
		return false;
	}

	TeSTInstance* instance = dataSet_->getSTInstance(getRealIndex(index));
	if(instance == 0)
	{
		return false;
	}

	vecAtributes.clear();
	for(unsigned int i = 0; i < instance->getProperties().size(); ++i)
	{
		vecAtributes.push_back(instance->getProperties()[i]);
	}

	return true; 
}

bool TeSTGeoDataSet::getGeometry(const int& index, TePointSet& ps)
{ 
	if(dataSet_ == 0)
	{
		return false;
	}

	TeSTInstance* instance = dataSet_->getSTInstance(getRealIndex(index));
	if(instance == 0)
	{
		return false;
	}

	TeMultiGeometry& multiGeom = instance->geometries();
	TePointSet& psCopy = multiGeom.getPoints();

	ps.copyElements(psCopy);

	return true;
}
	
bool TeSTGeoDataSet::getGeometry(const int& index, TeLineSet& ls)
{ 
	if(dataSet_ == 0)
	{
		return false;
	}

	TeSTInstance* instance = dataSet_->getSTInstance(getRealIndex(index));
	if(instance == 0)
	{
		return false;
	}

	TeMultiGeometry& multiGeom = instance->geometries();
	ls = multiGeom.getLines();

	return true;
}

bool TeSTGeoDataSet::getGeometry(const int& index, TePolygonSet& ps)
{ 
	if(dataSet_ == 0)
	{
		return false;
	}

	TeSTInstance* instance = dataSet_->getSTInstance(getRealIndex(index));
	if(instance == 0)
	{
		return false;
	}

	TeMultiGeometry& multiGeom = instance->geometries();

	TePolygonSet& psCopy = multiGeom.getPolygons();
	ps.copyElements(psCopy);

	return true;
}

int TeSTGeoDataSet::getRealIndex(const int& index)
{
	if(vecFilter_.empty())
	{
		return index;
	}

	return vecFilter_[index];
}
