
#ifndef _TEDXFDRIVERFACTORY_H
#define _TEDXFDRIVERFACTORY_H

#include "tedxfdefines.h"
#include "TeGDriverFactory.h"

class TeDXFDriver;
class TeGeoDataDriver;
struct TeGDriverParams;

class TEDXF_DLL TeDXFDriverFactory : public TeGDriverFactory 
{
  public:
    TeDXFDriverFactory();

    ~TeDXFDriverFactory();


  protected:
    //!  Builds a new product from a set of parameters (should be implemented by descendants)
    TeGeoDataDriver * build(TeGDriverParams * params); 

    //!  Builds a new product without parameters (should be implemented by descendants)
    TeGeoDataDriver* build();

	void loadGeoFilesInfo();
};

#endif
