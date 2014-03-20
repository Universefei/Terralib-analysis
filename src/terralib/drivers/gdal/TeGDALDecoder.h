/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.

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

/** \file  TeGDALDecoder.h
    \brief This file contains a concrete raster decoder based on GDAL Library.
    \author Douglas Uba <douglas@dpi.inpe.br>
   */

#ifndef _TEGDALDRIVER_H
#define _TEGDALDRIVER_H

// Defines
#include "TeGDALDefines.h"

// TerraLib include files
#include "../../kernel/TeDecoderVirtualMemory.h"

// STL include files
#include <vector>
#include <string>

// forward declarations;
class GDALDataset;

//! Simple class to represent the GDAL driver metadada.
class TEGDAL_DLL TeGDALDriverInfo
{

public:

	TeGDALDriverInfo();
	~TeGDALDriverInfo();

	const std::string& getId() const;
	const std::string& getFullName() const;
	const std::string& getExtension() const;
	bool hasCreateSupport() const;

	void setId(const std::string& id);
	void setFullName(const std::string& n);
	void setExtension(const std::string& e);
	void setCreateSupport(bool b);

	void print() const;

	std::string getFilter() const;

private:

	std::string _id;            //!< The GDAL internal driver identify. (ex.: AAIGrid)
	std::string _fullName;      //!< The driver full name. (ex.: Arc/Info ASCII Grid)
	std::string _extension;     //!< The extension file associated with the driver. (empty allowed)
	bool _hasCreateSupport;     //!< Indicates if the driver has creation support.
};

class TEGDAL_DLL TeGDALDecoder : public TeDecoderVirtualMemory 
{
	
public:
  
	TeGDALDecoder();

	TeGDALDecoder(const TeRasterParams& par);

	~TeGDALDecoder();

	//! Initializes the internal structures
	void init();

	//! Releases the internal structures
	bool clear();

	//! Writes an element
	bool setElement(int col, int lin, double val, int band = 0);

	//! Reads an element
	//bool getElement(int col, int lin, double &val, int band = 0);

	//! Initialize GDAL support
	static void initialize();
	
	/*!
		\brief It returns the GDAL driver name associated to the given file extension.
		\param extension The raster file extension.
		\return The GDAL driver name, its identifier if succeeds and a empty string otherwise.
	*/
	static std::string getGDALDriverName(const std::string& extension);

	static const std::vector<TeGDALDriverInfo>& getGDALDrivers();

	static std::string getSupportedFilesFilter();

protected:

	virtual bool getRasterBlock(const TeBlockIndex& index, void* buf);
	virtual bool putRasterBlock(const TeBlockIndex& index, void* buf, long bsize);

protected:
	
	//!< A pointer to GDAL Data Source.
	GDALDataset* _gdalDS;

	//!< Set of GDAL drivers information
	static std::vector<TeGDALDriverInfo> _gdalDrivers;

private:
  
    TeGDALDecoder(const TeGDALDecoder& source);

    TeGDALDecoder& operator=(const TeGDALDecoder& source);
};

//! Implements a factory to build decoders based on GDAL library
class TEGDAL_DLL TeGDALDecoderFactory : public TeDecoderFactory
{
public:

	//! Constructor for the factory
	TeGDALDecoderFactory(const string& name);

	//! Built the object
	TeDecoder* build (const TeRasterParams& arg)
	{  return new TeGDALDecoder(arg); }
};

#endif // _TEGDALDRIVER_H
