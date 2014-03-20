#include "TeOGRDriver.h"
#include "TeOGRUtils.h"

// TerraLib include files
#include "../../kernel/TeProjection.h"
#include "../../kernel/TeSTElementSet.h"
#include "../../kernel/TeLayer.h"
#include "../../kernel/TeMultiGeometry.h"
#include "TeOGRDataSet.h"

// OGR
#include <ogrsf_frmts.h>

TeOGRDriver::TeOGRDriver(const std::string& fileName, const bool &writing, const std::string &OGRDriverName) :
TeGeoDataDriver(),
fileName_(fileName),
ogrLayer_(0)
{
	// Registers all format drivers built into OGR.
	OGRRegisterAll();
	ogrDS_=0;
	if(writing == false)
	{
		// Open OGR Datasource
		ogrDS_ = OGRSFDriverRegistrar::Open(fileName.c_str());
		/* TODO: The data set may has one or more layers. Is necessary a revision on TeGeoDataDriver interface?
			 For while, only first layer is considered by the driver. */
		if(ogrDS_ != 0)
		{
			int nLayers = ogrDS_->GetLayerCount();
			if(nLayers > 0)
			{
				ogrLayer_ = ogrDS_->GetLayer(0); // Gets the first layer
			}
		}

		if(ogrLayer_ == 0)
		{
			std::string errorMessage = CPLGetLastErrorMsg();
			return;
		}

		// Number of objects
		numObjects_ = ogrLayer_->GetFeatureCount();

		// Box
		OGREnvelope* env = new OGREnvelope();
		ogrLayer_->GetExtent(env);
		box_ = Convert2TerraLib(env);
		delete env;
	
		// Geometric representation
		rep_ = Convert2TerraLib(ogrLayer_->GetLayerDefn()->GetGeomType());

		attrList_ = Convert2TerraLib(ogrLayer_->GetLayerDefn());
	}
	else
	{
		OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(OGRDriverName.c_str());
		ogrDS_ = poDriver->CreateDataSource(fileName.c_str(), NULL );
	}

	if(ogrLayer_ == 0)
	{
		dataProjection_ = new TeNoProjection();
		return;
	}

	OGRSpatialReference* srs = ogrLayer_->GetSpatialRef();
	if(srs == 0)
	{
		dataProjection_ = new TeNoProjection();
		return;
	}

	if(srs->AutoIdentifyEPSG() == OGRERR_UNSUPPORTED_SRS)
	{
		dataProjection_ = new TeNoProjection();
		return;
	}

	dataProjection_ = TeProjectionFactory::make(atoi(srs->GetAuthorityCode(0)));
}

TeOGRDriver::~TeOGRDriver() 
{
	OGRDataSource::DestroyDataSource(ogrDS_);
	ogrDS_ = 0;
}

std::string TeOGRDriver::getFileName() 
{
	return fileName_;
}

bool TeOGRDriver::isDataAccessible() 
{
	return TeCheckFileExistence(fileName_) && (ogrDS_ != 0);
}

bool TeOGRDriver::initData()
{
	return true;
}

bool TeOGRDriver::initMetadata()
{
	return true;
}

void TeOGRDriver::getDriversNames(std::vector<std::string>& dnames)
{
	int ndrivers = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
	for(int i = 0; i < ndrivers; ++i)
	{
		OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriver(i);
		dnames.push_back(driver->GetName());
	}
}

bool TeOGRDriver::createAttributeTable(TeTable &table)
{
	if(ogrLayer_ == 0)
		return false;

	TeAttributeList::iterator it;
	TeAttributeList list = table.attributeList();
	for(it = list.begin(); it != list.end(); ++it)
	{
		OGRFieldDefn oField = Convert2OGR(*it);
		
		if((*it).rep_.type_ == TeSTRING)
			oField.SetWidth((*it).rep_.numChar_);
		
		if(ogrLayer_->CreateField(&oField) != OGRERR_NONE)
			return false;
	}
	return true;
}

bool TeOGRDriver::createLayer(TeLayer* layer, const int& rep)
{
	OGRwkbGeometryType	type;

	if(ogrDS_ == 0 || layer == 0)
		return false;

	if(layer->hasGeometry(TePOLYGONS) && rep)
		type = wkbPolygon;
	else if(layer->hasGeometry(TeLINES) && rep)
		type = wkbLineString;
	else if (layer->hasGeometry(TePOINTS) && rep)
		type=wkbPoint;

	if((ogrLayer_ = ogrDS_->CreateLayer(layer->name().c_str(), Convert2OGR(layer->projection()), type, NULL )) == 0) 
		return false;

	return (ogrLayer_ == 0) ? false : true;
}

bool TeOGRDriver::addElement(TeSTInstance* e)
{
	if(e == 0)
		return false;

	TePropertyVector properties = e->getPropertyVector();
	OGRFeature* poFeature = OGRFeature::CreateFeature(ogrLayer_->GetLayerDefn());
	
	TePropertyVector::iterator  itProp;
	for(itProp=properties.begin(); itProp != properties.end(); ++itProp)
	{
		const char* constName=(const char*)(*itProp).attr_.rep_.name_.c_str();
		poFeature->SetField(constName, (*itProp).value_.c_str());
	}
		
	TeMultiGeometry& geoms = e->geometries();
	TeGeometry* geom = 0;

	if(geoms.hasPoints())
		geom = (TeGeometry*)&geoms.getPoints()[0];
	else if(geoms.hasLines())
		geom = (TeGeometry*)&geoms.getLines()[0];
	else if(geoms.hasPolygons())
		geom = (TeGeometry*)&geoms.getPolygons()[0];

	OGRGeometry* ogrGeom = Convert2OGR(geom);
	poFeature->SetGeometry(ogrGeom);
	if(ogrLayer_->CreateFeature(poFeature) != OGRERR_NONE)
	{
		OGRFeature::DestroyFeature(poFeature);
		return false;
	}

	OGRFeature::DestroyFeature( poFeature );
	OGRGeometryFactory::destroyGeometry(ogrGeom);

	return true;
}

TeGeoDataSet* TeOGRDriver::getDataImpl()
{
	if(ogrLayer_ == 0)
	{
		return false;
	}

	ogrLayer_->SetSpatialFilter(0);
	ogrLayer_->SetAttributeFilter(0);
	ogrLayer_->ResetReading();
	
	TeOGRDataSet* dataSet = new TeOGRDataSet(ogrLayer_);
	return dataSet;
}

TeGeoDataSet* TeOGRDriver::filterImpl(const TeBox& box, const std::string& attrRestriction)
{
	if(ogrLayer_ == 0)
	{
		return false;
	}

	ogrLayer_->SetSpatialFilter(0);
	ogrLayer_->SetAttributeFilter(0);
	ogrLayer_->ResetReading();
	ogrLayer_->SetSpatialFilterRect(box.x1(), box.y1(), box.x2(), box.y2());

	TeOGRDataSet* dataSet = new TeOGRDataSet(ogrLayer_);
	return dataSet;
}
