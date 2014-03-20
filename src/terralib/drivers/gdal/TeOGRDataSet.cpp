#include "TeOGRDataSet.h"

#include "TeSTInstance.h"
#include <TeOGRUtils.h>

// OGR
#include <ogrsf_frmts.h>
#include "ogr_core.h"

TeOGRDataSet::TeOGRDataSet(OGRLayer* ogrLayer)
	: TeGeoDataSet()
	, ogrLayer_(ogrLayer)
	, currentIndex_(-1)
{
	if(ogrLayer_ == 0)
	{
		return;
	}

	size_ = ogrLayer_->GetFeatureCount(true);
	moveTo(0);

}

TeOGRDataSet::~TeOGRDataSet()
{
}

bool TeOGRDataSet::getAttribute(const int& index, const int& col, std::string& value)
{
	if(moveTo(index) == false)
	{
		return false;
	}

	if(currentInstance_.getPropertyValue(value, col) == false)
	{
		return false;
	}

	return true;
}
	
bool TeOGRDataSet::getAttributes(const int& index, std::vector<std::string>& vecAtributes)
{
	if(moveTo(index) == false)
	{
		return false;
	}

	vecAtributes = currentInstance_.getProperties();

	return true;
}

bool TeOGRDataSet::getGeometry(const int& index, TePointSet& ps)
{
	if(moveTo(index) == false)
	{
		return false;
	}

	ps = currentInstance_.getPoints();

	return true;
}
	
bool TeOGRDataSet::getGeometry(const int& index, TeLineSet& ls)
{
	if(moveTo(index) == false)
	{
		return false;
	}

	ls = currentInstance_.getLines();

	return true;
}

bool TeOGRDataSet::getGeometry(const int& index, TePolygonSet& ps)
{
	if(moveTo(index) == false)
	{
		return false;
	}

	ps = currentInstance_.getPolygons();

	return true;
}

bool TeOGRDataSet::moveTo(const int& index)
{
	if(index == currentIndex_)
	{
		return true;
	}

	if(index >= (int)size())
	{
		return false;
	}

	currentInstance_.clear();

	if(index == 0)
	{
		ogrLayer_->ResetReading();
		currentIndex_ = -1;
	}	

	if(index == (currentIndex_ + 1))
	{
		OGRFeature* feature = ogrLayer_->GetNextFeature();
		if(feature != 0)
		{
			currentInstance_ = Convert2TerraLib(feature);
			OGRFeature::DestroyFeature(feature);
		}
	}
	else
	{
		ogrLayer_->SetNextByIndex(index-1);
			
		OGRFeature* feature = ogrLayer_->GetNextFeature();
		if(feature != 0)
		{
			currentInstance_ = Convert2TerraLib(feature);
			OGRFeature::DestroyFeature(feature);
		}
	}
	currentIndex_ = index;

	return true;
}
