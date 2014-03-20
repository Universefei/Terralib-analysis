#include "TeDXFDriver.h"
#include "TeProjection.h"
#include "TeSTElementSet.h"
#include "tedxflayer.h"
#include "tedxfvisitor.h"

TeDXFDriver::TeDXFDriver(const string & fileName) 
	: TeSTGeoDataDriver()
	, fileName_(fileName)
{
}

TeDXFDriver::~TeDXFDriver() 
{
}

std::string TeDXFDriver::getFileName() 
{
	return fileName_;
}

bool TeDXFDriver::isDataAccessible() 
{
	return TeCheckFileExistence(fileName_);
}

bool TeDXFDriver::initData()
{

	if(isDataInited() == true)
	{
		return true;
	}

	isDataInited_ = true;

	if(dataSet_ == 0)
	{
		dataSet_ = new TeSTElementSet(getBox(), getAttributesList());
	}

	TeDXFVisitor* visitor = new TeDXFVisitor(fileName_);
	vector<TeDXFLayer*> layers = visitor->getLayers();
	vector<TeDXFLayer*>::iterator it;

	delete visitor;
	rep_ =0;
	int objectIdSeq=0;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		TeDXFLayer* layer = *it;

		if(!layer->getPoints().empty())
		{
			TePointSet pts = layer->getPoints();
			TePointSet::iterator pIt;
			
			for(pIt = pts.begin(); pIt != pts.end(); ++pIt)
			{
				objectIdSeq++;
				TeSTInstance obj;
				obj.addGeometry(*pIt);
				obj.objectId(Te2String(objectIdSeq));
				dataSet_->insertSTInstance(obj);
			}
			updateBox(box_, pts.box());
			numObjects_ += pts.size();
			rep_ += TePOINTS;
		}
		if(!layer->getLines().empty())
		{
			TeLineSet ls = layer->getLines();
			TeLineSet::iterator lIt;
			
			for(lIt = ls.begin(); lIt != ls.end(); ++lIt)
			{
				objectIdSeq++;
				TeSTInstance obj;
				obj.addGeometry(*lIt);
				obj.objectId(Te2String(objectIdSeq));
				dataSet_->insertSTInstance(obj);
			}
			updateBox(box_, ls.box());
			numObjects_ += ls.size();
			rep_ += TeLINES;
		}
		if(!layer->getPolygons().empty())
		{
			TePolygonSet pts = layer->getPolygons();
			TePolygonSet ::iterator pIt;
			
			for(pIt = pts.begin(); pIt != pts.end(); ++pIt)
			{
				objectIdSeq++;
				TeSTInstance obj;
				obj.addGeometry(*pIt);
				obj.objectId(Te2String(objectIdSeq));
				dataSet_->insertSTInstance(obj);
			}
			updateBox(box_, pts.box());
			numObjects_ += pts.size();
			rep_ += TePOLYGONS;
		}
		delete layer;
	}
	isMetadataInited_ = true;
	return true;
}
bool TeDXFDriver::initMetadata()
{
	return true;
}
