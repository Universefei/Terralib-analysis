/************************************************************************************
TerraView - visualization and exploration of geographical databases using TerraLib.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

#include <TeDecoderQtImage.h>

TeDecoderQtImage::TeDecoderQtImage ( const TeRasterParams& par )
{
	image_ = 0;
	transp_ = 255;
	params_ = par;
}

TeDecoderQtImage::~TeDecoderQtImage ()
{
	clear ();
	transp_ = 255;
}

void
TeDecoderQtImage::init()
{
	params_.status_= TeRasterParams::TeNotReady;
	if (params_.mode_ == 'c')	// creating a new file
	{
		clear ();
		image_ = new QImage (params_.ncols_,params_.nlines_,32);
		params_.status_ = TeRasterParams::TeReadyToWrite;
	}
	else if (params_.mode_ == 'w')
	{
		if (image_)
			params_.status_ = TeRasterParams::TeReadyToWrite;
	}
	else if (params_.mode_ == 'r')
	{
		if (image_)
			params_.status_ = TeRasterParams::TeReadyToRead;	
	}
	image_->fill(0);
}

bool
TeDecoderQtImage::clear()
{
	if (image_ != 0)
		delete image_;
	image_ = 0;
	return true;
}

bool 
TeDecoderQtImage::getElement (int col, int lin, double &val,int band)
{
	QRgb cell = image_->pixel(col, lin);
	if (band == 0)
	{
		val = (double) qRed(cell);
		return true;
	}
	if (band == 1)
	{
		val = (double) qGreen(cell);
		return true;
	}
	if (band == 2)
		val = (double) qBlue(cell);
	return true;
}

bool 
TeDecoderQtImage::setElement (int col, int lin, double val,int band )
{
	uchar* c = image_->scanLine(lin);
	int pixel = col << 2;
	c[pixel+2-band] = (uchar)val;
	c[pixel+3] = transp_;
	return true;
}

bool 
TeDecoderQtImage::setElementRGB(int col, int lin, double Rval, double Gval, double Bval, unsigned int /*transp*/)
{
	unsigned int val = (transp_ << 24) | 
		               static_cast<int>(Rval) << 16 | 
					   static_cast<int>(Gval) << 8  |
					   static_cast<int>(Bval);
	image_->setPixel(col,lin,val);
	return true;
}

bool 
TeDecoderQtImage::setAlphaBufferToTransparent()
{
	int	i, j, p;
	int width = image_->width();
	int height = image_->height();

	for(i=0; i<height; ++i)
	{
		uchar* c = image_->scanLine(i);
		for(j=0; j<width; ++j)
		{
			p = (j << 2) + 3;
			c[p] = 0;
		}
	}
	return true;
}
