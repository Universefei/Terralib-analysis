/*********************************
 * FUNCATE - GIS development team
 *
 * TerraLib INPE
 *********************************/
/** @file TeDXFDriver.h
 *  @brief This files contains a definition for a driver for DXF files. 
 * The driver is used to read from a file.
 *  @version 1.0
 *  @author Frederico Augusto Bedê
 *  @date 11/2009
 */
#ifndef _TEDXFDRIVER_H
#define _TEDXFDRIVER_H

#include "tedxfdefines.h"
#include "TeSTGeoDataDriver.h"
#include <string>
using namespace std;
#include "TeBox.h"
#include "TeDataTypes.h"
#include "TeAttribute.h"

class TeProjection;
class TeSTElementSet;
class TeDXFLayer;
class TeDXFVisitor;

/** @class TeDXFDriver
 *  @brief Concrete implementation of a driver for DXF files. 
 */
class TEDXF_DLL TeDXFDriver : public TeSTGeoDataDriver 
{
 public:
	/** @brief */
    TeDXFDriver(const string & fileName);

	/** @brief */
    virtual ~TeDXFDriver();

	//! Returns the complete access path associated to this driver
	virtual std::string getFileName();

	//! Returns true or false whether the data file is accessible
	virtual bool isDataAccessible();

	//!< Initializes the access to the data
	virtual bool initData();

	//!< Initializes the access to the data
	virtual bool initMetadata();
	
  protected:
    string fileName_; //!< Complete file name, including teh complete path and its extension.    


  private:
    /** @brief No copy allowed.    
         */
        TeDXFDriver(const TeDXFDriver & source);

    /** @brief No copy allowed.    
         */
        TeDXFDriver & operator=(const TeDXFDriver & source);

};
#endif
