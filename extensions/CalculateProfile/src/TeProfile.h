#ifndef TEPROFILE_H
#define TEPROFILE_H

#include<vector>
#include <tdkebdialogs.h>

class TeContourLineSet;
class TeLine2D;
class TePolygonSet;

using namespace std;

//! Class that calculates the terrain profile
class TDK_EBDIALOGS_API TeProfile
{
public:
	//! Constructor
	TeProfile(TeContourLineSet *parLineSet,TeLine2D *parLine);
	//! Destructor
	~TeProfile();
	//! Method to calculate the terrain profile
	TeLine2D calculateProfile();
protected:
	TeContourLineSet *lineSet;
	TeLine2D *lineSight;
	vector<double> distances,altitudes;
};
#endif
