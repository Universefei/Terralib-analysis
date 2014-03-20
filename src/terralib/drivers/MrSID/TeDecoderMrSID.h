/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeDecoderMrSID.h
    This file deals with decoding of images in MrSID Format
*/
#ifndef  __TERRALIB_INTERNAL_DECODERMRSID_H
#define  __TERRALIB_INTERNAL_DECODERMRSID_H

#include "../../kernel/TeDecoder.h"
#include "TeMrSID.h"

//! The TeMrSID decoder is a driver to decode images on the MrSID format.
/*	
	This class depends on the SDK provided by LizardTech, available at the
	website <www.lizardtech.com.br>. You should include this file in your
	final application and link it against the SDK libraries. \par
	This decoder provides only decoding capacities. It is not possible
	to codifiy images in MrSID format. \par
	The default file extension associated to this decoder is ".sid".
*/
class TeDecoderMrSID : public TeDecoder
{
public:

	//! Constructor from parameters
	TeDecoderMrSID(const TeRasterParams& par);

	//! Constructor from file name
	TeDecoderMrSID(const std::string& fname);

	//! Destructor
	~TeDecoderMrSID();

	//! Initalizes internal structures to make decoder able to read MrSID images
	void init();

	//! Releases internal structures and invalidates MrSID decoder
	bool clear();

	//! Provided for compatibility reasons only, this decoder can not encode data
	bool setElement(int /*col*/, int /*lin*/, double /*val*/, int /*band=0*/) 
	{ return false; }

	//! Gets the value of an specific element (col, lin, band) of a raster data
	/*!
		\param col pixel column identifier
		\param lin pixel line identifier
		\param val pixel value being retrieved
		\param band pixel band identifier
	*/
	bool getElement(int /*col*/, int /*lin*/, double& /*val*/, int /*band=0*/)
	{ return false; }

	//! Returns the best resolution level in a MrSID image to cover a paricular area
	/* 
		\param bb bounding box geographical coordinates of the area to be covered
		\param ncols number of columns of the area
		\param nlines number of lines of the area
		\param proj projection of the geographical coordinates
		\returns A resolution level available in the MrSID files
		\note Resolution level in decoder MrSid is a level in a multi-resolution pyramid
		stored in the format. Original resolution has value 0 and following higher
		levels assume integer values (1,2,3,4...)
	*/	
	int bestResolution(TeBox& bb, int ncols, int nlines, TeProjection* proj);

	//! Selects a portion of a MrSID image in a certain resolution level 
	/*
		\param bb bounding box (in geographical coordinates) of that area to be selected 
		\param the resolution level desired
		\param parBlock returns the parameters of the portion selected
		\returns TRUE if could select the portion and FALSE otherwise
		\note MrSID navigator will be bounded to the given resolution level
	*/
	bool selectBlocks(TeBox& bbox, int resLevel, TeRasterParams& parBlock);

	//! Returns the contents of a portion of image previously selected
	/*
		\param memDec a pointer to a decoder to a raster in memory. This decoder
		should be previously initialized
	*/
	bool getSelectedRasterBlock(TeDecoderMemory* memDec);

	//! Returns the number of blocks selected by the latest selection
	int numberOfSelectedBlocks() 
	{	return nSelectedBlocks_; }

	//! Reset block selection. Returns navigator to the original resolution level
	void clearBlockSelection();

	//! Returns the raster resolution level available that is more similiar to a given desired resolution
	int bestResolution(double res);


private:
	// Methods
	TeDecoderMrSID();  
	TeProjection* readProjection();
	void readMetadataInformation();


	// Members
	TeMrSIDReader* mrsid_;
	int nSelectedBlocks_;
};

//! Implements a factory to build MrSID decoders
class TeDecoderMrSIDFactory : public TeDecoderFactory
{
public:
	//! Factory constructor
	TeDecoderMrSIDFactory(const string& name);

	//! Builds the object
	TeDecoder* build (const TeRasterParams& arg)
	{  return new TeDecoderMrSID(arg); }
};

//! Creates a static factory to build MrSID decoders 
namespace 
{
	//! A static factory to build MrSID decoders 
	static TeDecoderMrSIDFactory theDecoderMrSIDFactory("MrSID");
};
#endif
