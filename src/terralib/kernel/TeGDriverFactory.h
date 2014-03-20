#ifndef _TEGDRIVERFACTORY_H
#define _TEGDRIVERFACTORY_H

// TerraLib
#include "TeAbstractFactory.h"
#include "TeGeoDataDriver.h"
#include "TeGeoFileInfo.h"

// STL
#include <string>
#include <vector>

using namespace std;

struct TL_DLL TeGDriverParams 
{
    TeGDriverParams(const string & fileType, const string & fileName);

    string getProductId() const; //!< Used by the factory.
    string fileType_;			 //!< Defines the type of file to be used. It identifies the factory of TeGeodataDriver objects.
    string fileName_;			 //!< Complete name of the file.
    bool writing_;

  private:

    //No copy allowed.
    TeGDriverParams(const TeGDriverParams & source);

    //No copy allowed.
    TeGDriverParams & operator=(const TeGDriverParams & source);
};

class TL_DLL TeGDriverFactory : public TeAbstractFactory<TeGeoDataDriver, TeGDriverParams, string> 
{
public:

	TeGDriverFactory(const string& facName);
	virtual ~TeGDriverFactory();

	static const std::vector<TeGeoFileInfo>& GetGeoFilesInfo();

	static std::string FindFactory(const std::string& fileName);

protected:
	
	virtual void loadGeoFilesInfo() = 0;

	virtual bool acceptFile(const std::string& fileName);

protected:

	static std::vector<TeGeoFileInfo> _geoFilesInfo; //!< Set of geo files info.
};

#endif // _TEGDRIVERFACTORY_H
