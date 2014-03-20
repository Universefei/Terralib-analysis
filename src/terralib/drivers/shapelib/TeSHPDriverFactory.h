
#ifndef _TESHPDRIVERFACTORY_H
#define _TESHPDRIVERFACTORY_H

#include "../../kernel/TeGDriverFactory.h"

#include "TeSHPDefines.h"

class TeGeoDataDriver;
struct TeGDriverParams;
class TeShapefileDriver;

class SHP_DLL TeSHPDriverFactory : public TeGDriverFactory 
{
  public:
    TeSHPDriverFactory();

    ~TeSHPDriverFactory();

  protected:

    //!  Builds a new product from a set of parameters (should be implemented by descendants)
    TeGeoDataDriver * build(TeGDriverParams * params);
    //!  Builds a new product without parameters (should be implemented by descendants)
    TeGeoDataDriver * build();

	void loadGeoFilesInfo();
};

static TeSHPDriverFactory fact;

#endif
