#include "TeProfile1.h"
#include "TeGeometry.h"
#include "TeIntersector.h"
#include "TeOverlay.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include<map>

using namespace TeINTERSECTOR2;
using namespace std;
typedef std::map<double,double> TerrainProfile;

TeProfile1::TeProfile1(TeContourLineSet *parLineSet,TeLine2D *parLine)
{
	lineSet = parLineSet;
	lineSight = parLine;
}

TeLine2D TeProfile1::calculateProfile()
{
	//Variables used in the method
	TerrainProfile coordinates;
	TeLine2D temp;
	TeVectorBoundaryIP *result = new  TeVectorBoundaryIP[lineSet->size()];
	TeContourLineSet::iterator lineSet_Iter;
	double distance = 0;
	
	//Iterator that pass through ContourlineSet to calculate the intersections
	int iteration = 0;
	for(lineSet_Iter = lineSet->begin();lineSet_Iter != lineSet->end();lineSet_Iter++)
		if(TeINTERSECTOR2::TeSafeIntersections(*lineSet_Iter,*lineSight,result[iteration]))
		{
			for(unsigned int i = 0; i < result[iteration].size(); i++)
				altitudes.push_back(lineSet_Iter->value());
			iteration++;
		}
	iteration = 0;

	/*For to pass through result and add the intersections distances 
	to lineSight first point in a vector (profile X values)
	*/
	TeCoord2D first = TeCoord2D();
	TeCoord2D last = TeCoord2D();
  TeCoord2D pout;
	for(unsigned int j = 0; j < lineSet->size() ; j++)
	{
		for(unsigned int i = 0; i < result[j].size(); i++)
		{
			double dist = 0.;
			for(unsigned int k = 0; k < lineSight->size()-1; k++)
			{
				first = TeCoord2D((*lineSight)[k].x(), (*lineSight)[k].y());
				last = TeCoord2D((*lineSight)[k+1].x(), (*lineSight)[k+1].y());
				TeCoord2D pin(result[j].at(i).coords_.at(0).x(), result[j].at(i).coords_.at(0).y());
				if(TePerpendicularDistance(first, last, pin, pout) < 0.001)
				{
					distance = dist + TeDistance(pin, first);
					distances.push_back(distance);
				}
				else
				{
					dist+= TeDistance(first, last);
				}
			}
		}
	}		

	delete [] result;

	if(altitudes.size()>0 && distances.size()>0)
	{
		//For to sort profile's coordinates vector
		for(unsigned int i = 0; i < distances.size(); i++)
			coordinates.insert( TerrainProfile::value_type(distances.at(i),altitudes.at(i)));

		//Inserting profile's coordinates in a TeLine2D
		for( TerrainProfile::const_iterator iterator = coordinates.begin(); iterator != coordinates.end(); iterator++)
			temp.add(TeCoord2D(iterator->first-coordinates.begin()->first,iterator->second));
	}

	return temp;
}

TeProfile1::~TeProfile1()
{

}