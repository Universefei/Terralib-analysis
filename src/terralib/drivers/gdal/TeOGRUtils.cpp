// OGR Driver include files
#include "TeOGRUtils.h"

// TerraLib include files
#include "../../kernel/TeGeometry.h"
#include "../../kernel/TeUtils.h"
#include "../../utils/TeWKBGeometryDecoder.h"

// OGR include files
#include <ogrsf_frmts.h>
#include <ogr_spatialref.h>

std::vector<TeGeometry*> Convert2TerraLib(OGRGeometry* ogrGeom)
{
	ogrGeom->flattenTo2D();
	int wkbSize = ogrGeom->WkbSize();
	unsigned char* wkbArray = new unsigned char[wkbSize];
	ogrGeom->exportToWkb(wkbNDR, wkbArray);
	
	const char* wkb = (const char*)wkbArray;
	
	unsigned int readBytes;
	std::vector<TeGeometry*> geoms;
	OGRwkbGeometryType geomType = ogrGeom->getGeometryType();
	std::string gname = ogrGeom->getGeometryName();
	switch(geomType)
	{
		case wkbPolygon:
		{
			TePolygon* p = new TePolygon();
			TeWKBGeometryDecoder::decodePolygon(wkb, *p, readBytes);
			geoms.push_back(p);
			break;
		}

		case wkbLineString:
		{
			TeLine2D* l = new TeLine2D();
			TeWKBGeometryDecoder::decodeLine(wkb, *l, readBytes);
			geoms.push_back(l);
			break;
		}

		case wkbPoint:
		{
			TeCoord2D c;
			TeWKBGeometryDecoder::decodePoint(wkb, c, readBytes);
			TePoint* pt = new TePoint(c);
			geoms.push_back(pt);
			break;
		}

		case wkbMultiPolygon:
		{
			TePolygonSet pset;
			TeWKBGeometryDecoder::decodePolygonSet(wkb, pset, readBytes);
			for(unsigned int i = 0; i < pset.size(); i++)
				geoms.push_back(new TePolygon(pset[i]));
			break;
		}

		case wkbMultiLineString:
		{
			TeLineSet lset;
			TeWKBGeometryDecoder::decodeLineSet(wkb, lset, readBytes);
			for(unsigned int i = 0; i < lset.size(); i++)
				geoms.push_back(new TeLine2D(lset[i]));
			break;
		}

		case wkbMultiPoint:
		{
			TePointSet ptset;
			TeWKBGeometryDecoder::decodePointSet(wkb, ptset, readBytes);
			for(unsigned int i = 0; i < ptset.size(); i++)
				geoms.push_back(new TePoint(ptset[i]));
			break;
		}
	}

	delete [] wkbArray;

	return geoms;
}

OGRGeometry* Convert2OGR(TeGeometry* geom)
{
	unsigned int size;
	char* wkb = 0;
	OGRGeometry* ogrGeom = 0;
	switch(geom->elemType())
	{
		case TePOINTS:
		{
			TePoint* tePoint = dynamic_cast<TePoint*>(geom);
			ogrGeom = OGRGeometryFactory::createGeometry(wkbPoint);
			((OGRPoint*)ogrGeom)->setX(tePoint->location().x());
			((OGRPoint*)ogrGeom)->setY(tePoint->location().y());
			break;
		}
		
		case TeLINES:
		{
			TeLine2D* teLine = dynamic_cast<TeLine2D*>(geom);
			TeWKBGeometryDecoder::encodeLine(*teLine, wkb, size);
			if(wkb != 0)
			{
				ogrGeom = OGRGeometryFactory::createGeometry(wkbLineString);
				ogrGeom->importFromWkb((unsigned char*)wkb, size);
				delete wkb;
			}
			break;
		}
		case TePOLYGONS:
		{
			TePolygon* tePolygon = dynamic_cast<TePolygon*>(geom);
			TeWKBGeometryDecoder::encodePolygon(*tePolygon, wkb, size);
			if(wkb != 0)
			{
				ogrGeom = OGRGeometryFactory::createGeometry(wkbPolygon);
				ogrGeom->importFromWkb((unsigned char*)wkb, size);
				delete wkb;
			}
			break;
		}

		default:
			return 0;
	}

	return ogrGeom;
}

TeBox Convert2TerraLib(OGREnvelope* env)
{
	return TeBox(env->MinX, env->MinY, env->MaxX, env->MaxY);
}

TeAttributeList Convert2TerraLib(OGRFeatureDefn* featDef)
{
	TeAttributeList attrList;
	int nFields = featDef->GetFieldCount();
	for(int i = 0; i < nFields; i++)
	{
		OGRFieldDefn* fieldDef = featDef->GetFieldDefn(i); 
		attrList.push_back(Convert2TerraLib(fieldDef));
	}
	return attrList;
}

TeAttribute Convert2TerraLib(OGRFieldDefn* fieldDef)
{
	TeAttribute attr;
	attr.rep_.name_ = fieldDef->GetNameRef();
	switch(fieldDef->GetType())
	{
		case OFTInteger:
			attr.rep_.type_ = TeINT;
		break;

		case OFTIntegerList:
			attr.rep_.type_ = TeUNKNOWN; // ?
		break;

		case OFTString:
			attr.rep_.type_ = TeSTRING;
			attr.rep_.numChar_ = fieldDef->GetWidth();
		break;

		case OFTStringList:
			attr.rep_.type_ = TeUNKNOWN; // ?
		break;

		case OFTReal:
			attr.rep_.type_ = TeREAL;
		break;

		case OFTRealList:
			attr.rep_.type_ = TeUNKNOWN; // ?
		break;

		case OFTBinary:
			attr.rep_.type_ = TeBLOB;
		break;

		case OFTDate:
		case OFTTime:
		case OFTDateTime:
			attr.rep_.type_ = TeDATETIME;
		break;

		default:
			attr.rep_.type_ = TeUNKNOWN; // ?
	}

	return attr;
}

OGRFieldDefn Convert2OGR(const TeAttribute& attr)
{
	OGRFieldType ogrType;
	switch(attr.rep_.type_)
	{
		case TeSTRING:
			ogrType = OFTString;
		break;
		
		case TeREAL:
			ogrType = OFTReal;
		break;
		
		case TeUNSIGNEDINT:
		case TeBOOLEAN:
		case TeINT:
			 ogrType = OFTInteger;
		break;
		
		case TeDATETIME:
			ogrType = OFTDateTime;
		break;
		
		case TeBLOB:
			ogrType = OFTBinary;
		break;

		default:
			ogrType = OFTString;
	}

	return OGRFieldDefn(attr.rep_.name_.c_str(), ogrType);
}

TeSTInstance Convert2TerraLib(OGRFeature* feat)
{
	TeSTInstance obj;
	obj.setObjectId(Te2String(feat->GetFID()));
	obj.addPropertyValue(Te2String(feat->GetFID()));

	int nFields = feat->GetFieldCount();
	for(int i = 0; i < nFields; i++)
		obj.addPropertyValue(feat->GetFieldAsString(i));

	OGRGeometry* ogrGeom = feat->GetGeometryRef();
	if(ogrGeom == 0)
		return obj;

	std::vector<TeGeometry*> geoms = Convert2TerraLib(ogrGeom);
	for(unsigned int i = 0; i < geoms.size(); i++)
	{
		geoms[i]->objectId(obj.getObjectId());
		switch(geoms[i]->elemType())
		{
			case TePOLYGONS:
				obj.addGeometry(*(static_cast<TePolygon*>(geoms[i])));
			break;

			case TeLINES:
				obj.addGeometry(*(static_cast<TeLine2D*>(geoms[i])));
			break;

			case TePOINTS:
				obj.addGeometry(*(static_cast<TePoint*>(geoms[i])));
			break;
		}
	}

	for(unsigned int i = 0; i < geoms.size(); i++)
		delete geoms[i];

	return obj;
}

int Convert2TerraLib(OGRwkbGeometryType geomType)
{
	switch(geomType)
	{
		case wkbPolygon:
		case wkbPolygon25D:
		case wkbMultiPolygon:
		case wkbMultiPolygon25D:
			return TePOLYGONS;

		case wkbLineString:
		case wkbLineString25D:
		case wkbMultiLineString:
		case wkbMultiLineString25D:
			return TeLINES;

		case wkbPoint:
		case wkbPoint25D:
		case wkbMultiPoint:
		case wkbMultiPoint25D:
			return TePOINTS;

		case wkbUnknown:
			return TePOLYGONS | TeLINES | TePOINTS;

		default:
			return TeGEOMETRYNONE;
	}
}

OGRSpatialReference* Convert2OGR(TeProjection* proj)
{
	std::string strProjection = TeGetSProjFromTeProjection(proj);
	OGRSpatialReference* osrs = new OGRSpatialReference();
	osrs->importFromProj4(strProjection.c_str());
	return osrs;
}
