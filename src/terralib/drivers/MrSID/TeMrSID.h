/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright ¨ 2001-2006 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeMrSID.h
    This file contains functions that provide a wrapper around the SDK provided by LizardTech to decode MrSID images.
*/
#ifndef  __TERRALIB_INTERNAL_MRSID_H
#define  __TERRALIB_INTERNAL_MRSID_H

//Internal classes from LizardTech namespace
namespace LizardTech
{
class MrSIDImageReader;
class LTINavigator;
class LTISceneBuffer;
}

#include <string>
#include <vector>


//! A reader to a MrSID image
/*!
	This class uses the SDK provided by LizardTech to built a MrSID image reader support.
	This class is used by TeDecoderMrSID and shouldn't be used directly.
	\sa TeDecoderMrSID
*/
class TeMrSIDReader
{
	static bool mrsid_initialized; // indicates if MrSID has already been initialized
	LizardTech::LTINavigator*		sidNav_;
	LizardTech::MrSIDImageReader*	sidImageFile_;
	std::string						fileName_;

	//!Returns an ImageBuffer for loading an image from MrSIDNavigator.
	/**
		\param size(in) size of the image buffer
		\param nbands(in) number of output bands
		\param bands(in) output bands vector indicating the desired output bands and its order
		\param data(out) bands image buffer vector
	*/
	LizardTech::LTISceneBuffer*		getImageBuffer(unsigned int width, unsigned int height,
												unsigned int nbands, unsigned char* data);
public:
	//! Constructor
	TeMrSIDReader(const std::string& fname);

	//! Destructor
	~TeMrSIDReader();

	//! Returns the  image filename associated to this reader
	std::string fileName() const
	{	return fileName_; }

	//! Color models from MrSID encoding system
	typedef enum { 
		ColorModelUnknown,  //!< unknown color model
		ColorModelGray,     //!< gray scale, composed by one 8 bit channel 
		ColorModelMap,      //!< indexed , composed by one 8 bit channel and a palette
		ColorModelRGB,      //!< composed by three 8 bit channels
		ColorModelRGBA      //!< composed by four 8 bit channels
	} ColorModel;

	//! Returns the color model of the image
	ColorModel getColorModel();

	//! Returns the number of bands in the image   
	int nBands() const;  

	//! Returns the number of bits per pixel of the image   
	unsigned int bitsPerPixel() const; 

	//! Returns the width of the image in pixels  
	unsigned int getWidth();

	//! Returns the height of the image in pixels 
	unsigned int getHeight();

	//! Returns the dimensions of the image
	void getDimensions(unsigned int& width, unsigned int& height);

	//! Checks if the imagem has world navigation information
	bool hasWorldInfo();

	//! Gets the upper-left X position
	double originX();

 	//! Gets the upper-left Y position
	double originY();

	//! Gets the (X,Y) coordinate of the upper-left position
	void getOrigin(double& x, double& y);

	//! Returns the X resolution
	double resX();

	//! Returns the Y resolution
	double resY();

	//! Returns the X and Y resolutions
	void getResolution(double& rx, double& ry);

	//! Gets the box of the image 
	void getWorld(double& x0, double& y0, double& x1, double& y1);

	//! Gets the bounding box of the image 
	void getBoundingBox(double& xmin, double& ymin, double& xmax, double& ymax);

	//! Gets all the coordinate information
	void getNavigationParameters(std::vector<double>& nwf);

	//! Converts a geo coordinate to pixel coordinate 
	void world2Pixel(double wx, double wy, int& ix, int& iy);
	
	//! Converts a pixel coordinate to geo coordinate 
	void pixel2World(int ix, int iy, double& wx, double& wy);

	/** Returns the level that corresponds to a resolution >= than 
		the resolution defined by a given box an a dimension
    */
	int getBestResolutionLevel(unsigned int w, unsigned int h, 
							   double x0, double y0, double x1, double y1);

	/** 
		Returns the level that corresponds to a resolution <= than 
		the resolution defined by a given box an a dimension
    */
	int getWorstResolutionLevel(unsigned int w, unsigned int h, 
								double x0, double y0, double x1, double y1);

	/** 
		Returns the level that corresponds to the closest resolution to 
		the resolution defined by a given box an a dimension
    */
	int getProxResolutionLevel(unsigned int w, unsigned int h, 
								double x0, double y0, double x1, double y1);

	/** 
		Returns the level that corresponds to the closest resolution to 
		the resolution given
    */
	int bestResolution(double res);

	//! Selects an area in a given level of the image
	bool selectArea(int lev, double& x0, double& y0, double& x1, double& y1);

	//! Returns the width in pixels, of the area selected
	int getSelectedAreaWidth();

	//! Returns the height in pixels, of the area selected
	int getSelectedAreaHeight();

	//! Returns the selected area in a buffer of char
	bool getSelectedAreaData(unsigned char*& data);

	//! Returns true, and the values, if there is a specification for a no data value
	bool getNoDataPixel(std::vector<double>& nodata);

	//! Returns the number of levels in the image
	int getNumLevels();

	//! Zoom the image to a given level
	bool zoomTo(int level);

	//! Returns true, and the values, if there is a specification for a max value
	bool getMaxValues(std::vector<double>& maxvalues);

	//! Returns true, and the values, if there is a specification for a min value
	bool getMinValues(std::vector<double>& minvalues);

};

//! Inline Methods
inline void TeMrSIDReader::getDimensions(unsigned int& w, unsigned int& h)
{
  w = getWidth();
  h = getHeight();
}

inline void TeMrSIDReader::getResolution(double& rx, double& ry)
{
  rx = resX();
  ry = resY();
}

inline void TeMrSIDReader::getOrigin(double& x, double& y)
{
  x = originX();
  y = originY();
}
#endif
