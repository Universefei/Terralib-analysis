/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeDecoderQtImage.h
    \brief This file contais the definitions of a decoder to a QTImage
*/
#ifndef __TERRALIB_INTERNAL_DECODERQTIMAGE_H
#define __TERRALIB_INTERNAL_DECODERQTIMAGE_H

#include "../../kernel/TeDecoder.h"
#include <qimage.h>

#include "../../kernel/TeDefines.h"

//! Implements a decoder to access a Qt image as a TeRaster
class TL_DLL TeDecoderQtImage : public TeDecoder
{
public:
	TeDecoderQtImage ( const TeRasterParams& );
	~TeDecoderQtImage ();

	bool setElement (int col, int lin, double val, int band = 0);
	bool getElement (int col, int lin, double &val, int band = 0);

	void init	();
	bool clear	();
	QImage*	getImage () { return image_; }

	bool setElementRGB(int col, int lin, double Rval, double Gval, double Bval, unsigned int /*transp*/ = 255);

	bool setAlphaBufferToTransparent();

	void setTransparency(int t)
	{ transp_ = t; }

	unsigned int getTransparency()
	{	return transp_; }

private:

	QImage*	image_;
	unsigned int transp_;
};

//! Implements a factory to build TeQtDecoders
class TL_DLL TeDecoderQtImageFactory : public TeDecoderFactory
{
public:

	TeDecoderQtImageFactory(const string& name) : TeDecoderFactory(name) {}

	virtual TeDecoder* build (const TeRasterParams& arg)
	{  return new TeDecoderQtImage(arg); }
};

#endif

