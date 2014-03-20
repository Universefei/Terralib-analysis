
#include "TeComputeAttributeStrategies.h"
#include "TeSTElementSet.h"
#include "TeOverlay.h"
#include "TeGeometryAlgorithms.h"
#include "TeSTEFunctionsDB.h"
#include "TeRaster.h"
#include "TeQuerier.h"
#include "TeQuerierParams.h"

using namespace TeOVERLAY;

// For relative values to be weighted in space (e.g., population densitity, malaria per number of inhabitants, etc.)	
string TeAverageWeighByAreaStrategy::compute (TeBox box)  
{
	double weigh_val = 0.0;

	TeQuerier* querier = getWithinGeometry(box, attribute);

    int quant = querier->numElemInstances();			
	
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
	TePolygon pol = polygonFromBox(box);
	box_ps.add(pol);

	double areaBox = TeGeometryArea(box);
	if (areaBox <= 0.0) return Te2String(0);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();

		string val;
		sti.getPropertyValue(attribute, val);
		double num_val = atof(val.c_str());
	
		TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
		double intersectArea = TeGeometryArea (intersect);
		weigh_val += num_val*(intersectArea/areaBox); // suppose objects are disjoint
	}

	delete querier;
	return Te2String (weigh_val);
}

string TeCategoryMajorityStrategy::compute (TeBox box)  
{
	map<string, double>  areaMap;

	// initialize areMap
	map<string, string>::iterator itMap = classesMap.begin();
	while( itMap != classesMap.end() )
	{
		areaMap[(*itMap).second] = 0;
		++itMap;
	}

	TeQuerier* querier = getWithinGeometry(box, attribute);

    int quant = querier->numElemInstances();			
	
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
	TePolygon pol = polygonFromBox(box);
	box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();
	
		string category;
		sti.getPropertyValue(attribute, category);

		TePolygonSet intersect;
		TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
		double intersectArea = TeGeometryArea (intersect);
		areaMap[classesMap[category]] += intersectArea; // suppose objects are disjoint
	}

	delete querier;

	double max = 0.0;
	string value;

	map <string, double>:: iterator it = areaMap.begin();
	while(it != areaMap.end())  
	{
		if ((*it).second > max)
		{
			value = (*it).first;
			max   = (*it).second;
		}
		++it;		
	}

	return value;
}


string TeTotalAreaPercentageStrategy::compute (TeBox box)
{  
	double area = TeGeometryArea(box);
	double totArea = 0.0;

	if (area <= 0.0) return Te2String(0);

	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();			
	
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
	TePolygon pol = polygonFromBox(box);
	box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();
	
		TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
			
		double intersectArea = TeGeometryArea (intersect);
		totArea += intersectArea; // suppose objects are disjoint
	}
	
	double percentage = totArea/area;

	if (percentage > 0.999) percentage = 1;

	delete querier;
	return Te2String (percentage);
}


string TeCountObjectsStrategy::compute(TeBox box)
{
	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();
	delete querier;
	return Te2String (quant);
}

string TeCountLineObjectsStrategy::compute(TeBox box)
{
	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();
	int found = 0;
	
	TePolygonSet box_ps;
	TeLineSet intersect, objGeom;
	TeSTInstance sti;
	TeMultiGeometry mg;
    TePolygon pol = polygonFromBox(box);
    box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getLines();

        TeLineSet intersect;
        mg = TeOverlay(objGeom, box_ps, TeINTERSECTION);

        intersect = mg.getLines();
        if(!intersect.empty()) found++;
	}
	
	delete querier;
	return Te2String (found);
}


string TeCountPolygonalObjectsStrategy::compute(TeBox box)
{
	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();
	int found = 0;
	
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
    TePolygon pol = polygonFromBox(box);
    box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();
	
		TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
        if(!intersect.empty()) found++;
	}
	
	delete querier;
	return Te2String (found);
}


TeQuerier* TeComputeSpatialStrategy::getWithinGeometry(TeBox box, string attribute)
{
	vector<string> attrNames;
	
	if(attribute != "") attrNames.push_back (attribute);
    TeQuerierParams params(true, attrNames);

    params.setParams (theme);
	params.setSpatialRest(box, TeWITHIN, rep);
    TeQuerier* querier = new TeQuerier(params);
	querier->loadInstances();
	return querier;
}

string TeLineLengthStrategy::compute (TeBox box)
{
	double length = 0;

	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();			
	
	TePolygonSet box_ps;
	TeLineSet intersect, objGeom;
	TeSTInstance sti;
	TePolygon pol = polygonFromBox(box);
	box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getLines();
        TeMultiGeometry mg;
					
		mg = TeOverlay(objGeom, box_ps, TeINTERSECTION);
				
		intersect = mg.getLines();
		for(TeLineSet::iterator lineIt = intersect.begin(); lineIt != intersect.end(); lineIt++)
		{
			length += TeLength (*lineIt);
		}
	}

	delete querier;
	return Te2String (length);
}

// For absolute values to be partinioned in space (e.g., population, number of malaria cases, etc.)	
string TeSumWeighByAreaStrategy::compute (TeBox box)
{
	double weigh_val = 0.0;
	double area = TeGeometryArea(box);

	if (area <= 0.0) return Te2String(0);

	TeQuerier* querier = getWithinGeometry(box, attribute);
    int quant = querier->numElemInstances();			
	
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
	TePolygon pol = polygonFromBox(box);
	box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();
	
		string val;
		sti.getPropertyValue(attribute, val);
		double num_val = atof(val.c_str());

		double geomArea = TeGeometryArea (objGeom);
		if (geomArea > 0.0)
		{
			TePolygonSet intersect;
			TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
			double intersectArea = TeGeometryArea (intersect);
			weigh_val += num_val*(intersectArea/geomArea); 
		}
	}

	delete querier;
	return Te2String (weigh_val);
}

TeMinimumDistanceStrategy::TeMinimumDistanceStrategy(TeTheme* theme_, TeGeomRep rep_, double distError_) 
: TeComputeSpatialStrategy(theme_, rep_) 
{ 
	maxDist = distError_; tree = NULL; 
}

void TeMinimumDistanceStrategy::CreatePoints(TeCoord2D p1, TeCoord2D p2)
{
	double dist = TeDistance(p1, p2);
	if(dist > maxDist)
	{
		TeCoord2D p3;

		TeGetMiddlePoint (p1, p2, p3);

		dataSet.push_back(pair<TeCoord2D, TePoint>(p3, p3));
		
		CreatePoints(p1, p3);
		CreatePoints(p2, p3);
	}
}

TeMinimumDistanceLinesStrategy::TeMinimumDistanceLinesStrategy(TeTheme* theme_, double dist_)
	:TeMinimumDistanceStrategy(theme_, TeLINES, dist_)
{
	vector<string> attrNames;
    TeQuerierParams params(true, attrNames);

    params.setParams (theme);
    TeQuerier* querier = new TeQuerier(params);
	querier->loadInstances();
    int quant = querier->numElemInstances();

    TeSTInstance sti;
	TeLineSet ls;

    for(int c = 0; c < quant; c++)
    {
        querier->fetchInstance (sti);
        ls = sti.geometries().getLines();

		for(unsigned i = 0; i < ls.size(); i++)
		{
			dataSet.push_back(pair<TeCoord2D, TePoint>(ls[i][0], ls[i][0]));
			for(unsigned j = 1; j < ls[i].size(); j++)
			{
				dataSet.push_back(pair<TeCoord2D, TePoint>(ls[i][j], ls[i][j]));
	
				TeCoord2D p1 = ls[i][j-1];
				TeCoord2D p2 = ls[i][j];
				
				CreatePoints(p1, p2);
			}
		}
	}

	tree = new KDTREE(theme_->getThemeBox(), 5); // bucketSize == 5
	tree->build(dataSet);
	delete querier;
}

TeMinimumDistancePointsStrategy::TeMinimumDistancePointsStrategy(TeTheme* theme_)
	:TeMinimumDistanceStrategy(theme_, TePOINTS, 0)
{
	vector<string> attrNames;
    TeQuerierParams params(true, attrNames);

    params.setParams (theme);
    TeQuerier* querier = new TeQuerier(params);
	querier->loadInstances();
    int quant = querier->numElemInstances();

    TeSTInstance sti;
	TePointSet ps;

    for(int c = 0; c < quant; c++)
    {
        querier->fetchInstance (sti);
        ps = sti.geometries().getPoints();

		for(unsigned i = 0; i < ps.size(); i++)
		{
			dataSet.push_back(pair<TeCoord2D, TePoint>(ps[i].location(), ps[i]));
		}
	}

	tree = new KDTREE(theme_->getThemeBox(), 5); // bucketSize == 5
	tree->build(dataSet);
	delete querier;
}

TeMinimumDistancePolygonsStrategy::TeMinimumDistancePolygonsStrategy(TeTheme* theme_, double dist_)
    :TeMinimumDistanceStrategy(theme_, TePOLYGONS, dist_)
{
	vector<string> attrNames;
    TeQuerierParams params(true, attrNames);

    params.setParams (theme);
    TeQuerier* querier = new TeQuerier(params);
	querier->loadInstances();
    int quant = querier->numElemInstances();

    TeSTInstance sti;
	TePolygonSet ps;

    for(int c = 0; c < quant; c++)
    {
        querier->fetchInstance (sti);
        ps = sti.geometries().getPolygons();

		for(unsigned i = 0; i < ps.size(); i++)
		{
			for(unsigned j = 0; j < ps[i].size(); j++)
			{
				dataSet.push_back(pair<TeCoord2D, TePoint>(ps[i][j][0], ps[i][j][0]));
	
	            for(unsigned k = 1; k < ps[i][j].size(); k++)
	            {
					dataSet.push_back(pair<TeCoord2D, TePoint>(ps[i][j][k], ps[i][j][k]));
					
					TeCoord2D p1 = ps[i][j][k-1];
					TeCoord2D p2 = ps[i][j][k];
					CreatePoints(p1, p2);
				}
							
				TeCoord2D p1 = ps[i][j][0];
				TeCoord2D p2 = ps[i][j][ps[i][j].size()-1];
				CreatePoints(p1, p2);
			}
		}
	}

	tree = new KDTREE(theme_->getThemeBox(), 5); // bucketSize == 5
	tree->build(dataSet);
	delete querier;
}

string TeMinimumDistanceStrategy::compute (TeBox box)
{
	vector<TePoint> report;
	vector<double> sqrDists;
	TeCoord2D c(TeMAXFLOAT, TeMAXFLOAT);
	TeCoord2D key = box.center();
	TePoint item(c);

	report.push_back(item);

	tree->nearestNeighborSearch(key, report, sqrDists, 1);

	double dist = sqrt(sqrDists[0]);
	return Te2String (dist);
}

string TeMinimumDistancePolygonsStrategy::compute (TeBox box)
{
	TeQuerier* querier = getWithinGeometry(box);
    int quant = querier->numElemInstances();			
		
	TePolygonSet box_ps, intersect, objGeom;
	TeSTInstance sti;
    TePolygon pol = polygonFromBox(box);
    box_ps.add(pol);

	for(int i = 0; i < quant; i++)
	{
		querier->fetchInstance (sti);
		objGeom = sti.geometries().getPolygons();
	
		TeOverlay(box_ps, objGeom, intersect, TeINTERSECTION);
        if(!intersect.empty()) return Te2String (0);
	}
	
	delete querier;

	return TeMinimumDistanceStrategy::compute(box);
}

