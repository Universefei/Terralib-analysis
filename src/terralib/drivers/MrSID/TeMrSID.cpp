/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2006 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
#include "TeMrSID.h"

#include <lt_fileSpec.h>
#include <MrSIDImageReader.h>
#include <lti_navigator.h>
#include <lti_sceneBuffer.h>
#include <lti_pixel.h>
#include <lti_utils.h>

#include <locale.h>
#include <math.h>
#include <stdio.h>

LT_USE_NAMESPACE(LizardTech)

// Static variables initialization:

bool TeMrSIDReader::mrsid_initialized = false;

/*
** ---------------------------------------------------------------
** Methods Implementation:
*/

TeMrSIDReader::TeMrSIDReader(const std::string& fname):
	fileName_(fname)
{
	LT_STATUS sts = LT_STS_Uninit;

	if(!mrsid_initialized)
	{
		setlocale(LC_ALL,"English");
		//ATENCAO - Verificar se isso funciona quando a configuracao do Windows for em Portugues.
		//    XTrans::initialize();      
		mrsid_initialized = true;
	}
	const LTFileSpec filespec(fname.c_str());
	sidImageFile_ = MrSIDImageReader::create();
	sts = sidImageFile_->initialize( filespec );
	if(!LT_SUCCESS(sts))
		throw "Erro lendo MrSID";
	sidNav_ = new LTINavigator(*sidImageFile_);
};

TeMrSIDReader::~TeMrSIDReader()
{
//	if(sidImageFile_) delete sidImageFile_;
	if(sidNav_) delete sidNav_;
}

int TeMrSIDReader::nBands() const
{
	return sidImageFile_->getNumBands();
}

unsigned int TeMrSIDReader::bitsPerPixel() const
{
	return sidImageFile_->getPixelProps ().getNumBytes() * 8;
}

TeMrSIDReader::ColorModel TeMrSIDReader::getColorModel()
{
	LTIColorSpace color_space = sidImageFile_->getColorSpace();

	if(color_space == LTI_COLORSPACE_RGB)
		return ColorModelRGB;
	else if(color_space == LTI_COLORSPACE_GRAYSCALE)
		return ColorModelGray;
	else
		return ColorModelUnknown;
}

unsigned int TeMrSIDReader::getWidth()
{
	return sidImageFile_->getWidth();
}

unsigned int TeMrSIDReader::getHeight()
{
	return sidImageFile_->getHeight();
}

bool TeMrSIDReader::hasWorldInfo()
{
	return !(sidImageFile_->isGeoCoordImplicit());
}

double TeMrSIDReader::originX()
{
	return sidImageFile_->getGeoCoord().getX(); 
}

double TeMrSIDReader::originY()
{
	return sidImageFile_->getGeoCoord().getY();
}

double TeMrSIDReader::resX()
{
  return sidImageFile_->getGeoCoord().getXRes();
}

double TeMrSIDReader::resY()
{
	return sidImageFile_->getGeoCoord().getYRes();
}

void TeMrSIDReader::getWorld(double& x0, double& y0, double& x1, double& y1)
{
	unsigned int w, h;
	getOrigin(x0, y0);
	getDimensions(w, h);
	sidImageFile_->getGeoCoord().pixelToGeo(w - 1, h - 1, 1.0, x1, y1);
}

void TeMrSIDReader::getNavigationParameters(std::vector<double>& nwf)
{
	const LTIGeoCoord& geo = sidImageFile_->getGeoCoord();
	double xUL, yUL, xRes, yRes, xRot, yRot;
	geo.get(xUL,yUL,xRes,yRes,xRot,yRot);
	nwf.clear();
	nwf.push_back(xRes);
	nwf.push_back(xRot);
	nwf.push_back(yRot);
	nwf.push_back(yRes);
	nwf.push_back(xUL);
	nwf.push_back(yUL);
	return;
}

void TeMrSIDReader::getBoundingBox(double& xmin, double& ymin, double& xmax, double& ymax)
{
	double sx, sy;
	unsigned int w, h;
	double x0, y0, x1, y1;

	sx = resX();
	sy = resY();

	getOrigin(x0, y0);
	x0-= sx * 0.5;
	y0-= sy * 0.5;

	getDimensions(w, h);

	x1 = x0 + w * sx;
	y1 = y0 + h * sy;

	if(x0 < x1)
	{
		xmin = x0;
		xmax = x1;
	}
	else
	{
		xmin = x1;
		xmax = x0;
	}

	if(y0 < y1)
	{
		ymin = y0;
		ymax = y1;
	}
	else
	{
		ymin = y1;
		ymax = y0;
	}
}

void TeMrSIDReader::world2Pixel(double wx, double wy, int& ix, int& iy)
{
	double px, py;

	sidImageFile_->getGeoCoord().geoToPixel(wx, wy, 1.0, px, py);

	ix = (int)(px + 0.5);
	iy = (int)(py + 0.5);
}

void TeMrSIDReader::pixel2World(int ix, int iy, double& wx, double& wy)
{
	sidImageFile_->getGeoCoord().pixelToGeo(ix, iy, 1.0, wx, wy);
}


LTISceneBuffer* 
TeMrSIDReader::getImageBuffer(unsigned int width, unsigned int height, 
							  unsigned int nbands, unsigned char* data)
{
	if(data == NULL)
		return NULL;

	unsigned int size = width * height;
	void** bands_data = (void**)calloc(nbands, sizeof(void*));
	for(unsigned int i = 0;i < nbands;i++)
		bands_data[i] = data + i * size;
	return new LTISceneBuffer(sidImageFile_->getPixelProps(), width, height, bands_data);
}

int
TeMrSIDReader::bestResolution(double res)
{
	double magx, magy, mag;

	//compute Magnification
	magx = fabs(1/res);
	magy = fabs(1/res);

	if(magx > magy)
		mag = magx;
	else
		mag = magy;

	if(mag <= sidImageFile_->getMinMagnification())
		mag = sidImageFile_->getMinMagnification();
	else if(mag >= sidImageFile_->getMaxMagnification())
		mag = sidImageFile_->getMaxMagnification();
	else if(mag < 1)
	{
		int imag = (int)(1.0/mag) >> 1;
		int i;

		for(i = 1;i <= imag;i = i << 1);
		mag = 1.0/i;
	}
	else 
	{
		int imag = (int)ceil(mag);
		int i;
		for(i = 1;i < imag;i = i << 1);
			mag = i;
	}
	return LTIUtils::magToLevel(mag);
}

int 
TeMrSIDReader::getBestResolutionLevel(unsigned int w, unsigned int h, 
									  double x0, double y0, double x1, double y1)
{
	double magx, magy, mag;

	//compute Magnification
	magx = fabs((double)w/(x1 - x0) * resX());
	magy = fabs((double)h/(y1 - y0) * resY());

	if(magx > magy)
		mag = magx;
	else
		mag = magy;

	if(mag <= sidImageFile_->getMinMagnification())
		mag = sidImageFile_->getMinMagnification();
	else if(mag >= sidImageFile_->getMaxMagnification())
		mag = sidImageFile_->getMaxMagnification();
	else if(mag < 1)
	{
		int imag = (int)(1.0/mag) >> 1;
		int i;

		for(i = 1;i <= imag;i = i << 1);
		mag = 1.0/i;
	}
	else 
	{
		int imag = (int)ceil(mag);
		int i;
		for(i = 1;i < imag;i = i << 1);
			mag = i;
	}
	return LTIUtils::magToLevel(mag);
}

int 
TeMrSIDReader::getWorstResolutionLevel(unsigned int w, unsigned int h, 
									   double x0, double y0, double x1, double y1)
{
	double magx, magy, mag;

	//compute Magnification
	magx = fabs((double)w/(x1 - x0) * resX());
	magy = fabs((double)h/(y1 - y0) * resY());

	if(magx < magy)
		mag = magx;
	else
		mag = magy;

	if(mag <= sidImageFile_->getMinMagnification())
		mag = sidImageFile_->getMinMagnification();
	else if(mag >= sidImageFile_->getMaxMagnification())
		mag = sidImageFile_->getMaxMagnification();
	else if(mag < 1)
	{
		int imag = (int)ceil(1.0/mag);
		int i;
		for(i = 1;i < imag;i = i << 1);
			mag = 1.0/i;
	}
	else 
	{
		int imag = (int)mag >> 1;
		int i;
		for(i = 1;i <= imag;i = i << 1);
		mag = i;
	}
	return LTIUtils::magToLevel(mag);
}

int 
TeMrSIDReader::getProxResolutionLevel(unsigned int w, unsigned int h, 
                                      double x0, double y0, double x1, double y1)
{
	double magx, magy, mag;

	//compute Magnification
	magx = fabs((double)w/(x1 - x0) * resX());
	magy = fabs((double)h/(y1 - y0) * resY());

	if(magx > magy)
		mag = magx;
	else
		mag = magy;

	if(mag <= sidImageFile_->getMinMagnification())
		mag = sidImageFile_->getMinMagnification();
	else if(mag >= sidImageFile_->getMaxMagnification())
		mag = sidImageFile_->getMaxMagnification();
	else if(mag < 1)
	{
		//    int imag = (int)(1.0/mag) >> 1;
		int imag = (int)(1.0/mag + 0.5);
		int i;
		for(i = 1;i < imag;i = i << 1);
			mag = 1.0/i;
	}
	else 
	{
		int imag = (int)ceil(mag);
		int i;
		for(i = 1;i < imag;i = i << 1);
			mag = i;
	}
	return LTIUtils::magToLevel(mag);
}

bool 
TeMrSIDReader::selectArea(int lev, double& x0, double& y0, double& x1, double& y1)
{ 
	double mag;
	LT_STATUS sts = LT_STS_Uninit;
	mag = LTIUtils::levelToMag(lev);

	if(mag < sidImageFile_->getMinMagnification())
		mag = sidImageFile_->getMinMagnification();
	else if(mag > sidImageFile_->getMaxMagnification())
		mag = sidImageFile_->getMaxMagnification();

	sts = sidNav_->setSceneAsGeoULLR(x0, y1, x1, y0, mag);
	if(!LT_SUCCESS(sts))
		return false;

	if(!sidNav_->clipToImage())
		return false;

	sidNav_->roundScene();
	const LTIGeoCoord& geo_coord = sidImageFile_->getGeoCoord();
	geo_coord.pixelToGeo(sidNav_->getUpperLeftCol() - 0.5, sidNav_->getUpperLeftRow() - 0.5, 
						 mag, x0, y1);
	geo_coord.pixelToGeo(sidNav_->getLowerRightCol() - 0.5, sidNav_->getLowerRightRow() - 0.5, 
						 mag, x1, y0);
	return sidNav_->isSceneValid();
}

int 
TeMrSIDReader::getSelectedAreaWidth()
{
	return sidNav_->getNumCols();
}

int 
TeMrSIDReader::getSelectedAreaHeight()
{
	return sidNav_->getNumRows();
}

/* Loads the image selected by the navigator rectangle into memory.
   Expects that the pointer to unsiged char was already allocated */
bool 
TeMrSIDReader::getSelectedAreaData(unsigned char*& data)
{
	if(!sidImageFile_)
		return false;
	if(!sidNav_)
		return false;
	if(!data)
		return false;

	// generate a memory buffer
	unsigned int size = sidNav_->getNumCols() * sidNav_->getNumRows();
	void** bands_data = (void**)calloc(nBands(), sizeof(void*));
	for(int i=0;i<nBands();++i)
		bands_data[i] = data + i * size;
	LTISceneBuffer* sidBuffer = new LTISceneBuffer(sidImageFile_->getPixelProps(), sidNav_->getNumCols(), 
		                           sidNav_->getNumRows(), bands_data);
	if(sidBuffer == NULL)
		return false;

	LT_STATUS sts = LT_STS_Uninit;
	sts = sidImageFile_->read(sidNav_->getScene(), *sidBuffer);

	if(!LT_SUCCESS(sts))
		return false;
	delete sidBuffer;
	return true;
}

bool 
TeMrSIDReader::getMinValues(std::vector<double>& minvalues)
{
	const LTIPixel minv = sidImageFile_->getMinDynamicRange(); 
	LTIDataType dt = minv.getDataType();
	if (dt == LTI_DATATYPE_INVALID)
		return false;
	
	minvalues.clear();
	int i;
	switch (dt)
	{
	case LTI_DATATYPE_UINT8:
		for (i=0; i<minv.getNumBands(); ++i)
			minvalues.push_back(minv.getSampleValueUint8(i));
		break;
	case LTI_DATATYPE_SINT8:
		for (i=0; i<minv.getNumBands(); ++i)
			minvalues.push_back(minv.getSampleValueSint8(i));
		break;
	case LTI_DATATYPE_UINT16:
		for (i=0; i<minv.getNumBands(); ++i)
			minvalues.push_back(minv.getSampleValueUint16(i));
		break;
	case LTI_DATATYPE_SINT16:
		for (i=0; i<minv.getNumBands(); ++i)
			minvalues.push_back(minv.getSampleValueSint16(i));
		break;
	default:
		for (i=0; i<minv.getNumBands(); ++i)
			minvalues.push_back(minv.getSampleValueFloat32(i));
	}
	return true;	
}

bool 
TeMrSIDReader::getMaxValues(std::vector<double>& maxvalues)
{
	const LTIPixel maxv = sidImageFile_->getMaxDynamicRange(); 
	LTIDataType dt = maxv.getDataType();
	if (dt == LTI_DATATYPE_INVALID)
		return false;
	
	maxvalues.clear();
	int i;
	switch (dt)
	{
	case LTI_DATATYPE_UINT8:
		for (i=0; i<maxv.getNumBands(); ++i)
			maxvalues.push_back(maxv.getSampleValueUint8(i));
		break;
	case LTI_DATATYPE_SINT8:
		for (i=0; i<maxv.getNumBands(); ++i)
			maxvalues.push_back(maxv.getSampleValueSint8(i));
		break;
	case LTI_DATATYPE_UINT16:
		for (i=0; i<maxv.getNumBands(); ++i)
			maxvalues.push_back(maxv.getSampleValueUint16(i));
		break;
	case LTI_DATATYPE_SINT16:
		for (i=0; i<maxv.getNumBands(); ++i)
			maxvalues.push_back(maxv.getSampleValueSint16(i));
		break;
	default:
		for (i=0; i<maxv.getNumBands(); ++i)
			maxvalues.push_back(maxv.getSampleValueFloat32(i));
	}
	return true;
}
//! Returns true if there is a specification for a no data value
bool 
TeMrSIDReader::getNoDataPixel(std::vector<double>& nodata)
{
	const LTIPixel* nd = sidImageFile_->getNoDataPixel();
    	if(nd == NULL)
		return false;
	LTIDataType dt = nd->getDataType();
	if (dt == LTI_DATATYPE_INVALID)
		return false;

	nodata.clear();
	int i;
	switch (dt)
	{
	case LTI_DATATYPE_UINT8:
		for (i=0; i<nd->getNumBands(); ++i)
			nodata.push_back(nd->getSampleValueUint8(i));
		break;
	case LTI_DATATYPE_SINT8:
		for (i=0; i<nd->getNumBands(); ++i)
			nodata.push_back(nd->getSampleValueSint8(i));
		break;
	case LTI_DATATYPE_UINT16:
		for (i=0; i<nd->getNumBands(); ++i)
			nodata.push_back(nd->getSampleValueUint16(i));
		break;
	case LTI_DATATYPE_SINT16:
		for (i=0; i<nd->getNumBands(); ++i)
			nodata.push_back(nd->getSampleValueSint16(i));
		break;
	default:
		for (i=0; i<nd->getNumBands(); ++i)
			nodata.push_back(nd->getSampleValueFloat32(i));
	}
	return true;
}

int 
TeMrSIDReader::getNumLevels()
{
	return sidImageFile_->getNumLevels();
}

bool 
TeMrSIDReader::zoomTo(int level)
{
	double mag = LTIUtils::levelToMag(level);
	return LT_SUCCESS(sidNav_->zoomTo(mag,LTINavigator::STYLE_STRICT));
}

