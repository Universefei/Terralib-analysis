#ifndef TEPROFILE1_H
#define TEPROFILE1_H

#include<vector>
#include "TeDefines.h"

class TeContourLineSet;
class TeLine2D;
class TePolygonSet;

using namespace std;

//! Class that calculates the terrain profile
class TL_DLL TeProfile1
{
public:
	//! Constructor
	TeProfile1(TeContourLineSet *parLineSet,TeLine2D *parLine);
	//! Destructor
	~TeProfile1();
	//! Method to calculate the terrain profile
	TeLine2D calculateProfile();
protected:
	TeContourLineSet *lineSet;
	TeLine2D *lineSight;
	vector<double> distances,altitudes;
};
#endif
