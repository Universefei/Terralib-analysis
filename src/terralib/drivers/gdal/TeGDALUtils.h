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

/** \file  TeGDALUtils.h
    \brief Utility functions for GDAL support.
    \author Douglas Uba <douglas@dpi.inpe.br>
   */

#ifndef _TEGDALUTILS_H
#define _TEGDALUTILS_H

// Defines
#include "TeGDALDefines.h"

// TerraLib include files
#include "../../kernel/TeDataTypes.h"
#include "../../kernel/TeRasterParams.h"

// GDAL
#include <gdal_priv.h>
#include <ogr_spatialref.h>

// forward declarations
class GDALDataset;
class GDALRasterBand;

/*!
    \brief It converts the GDAL data set to TerraLib Raster Parameters set.
    \param  ds[in] A valid GDAL data set.
	\params params[out] TerraLib Raster Parameters that will be filled.
*/
TEGDAL_DLL void Convert2TerraLib(GDALDataset* ds, TeRasterParams& params);

/*!
    \brief It converts a GDAL raster band definition to TerraLib Raster Parameters set.
    \param  rasterBand[in] A valid GDAL raster band.
	\params params[out] TerraLib Raster Parameters that will be filled.
	\params i[in] The index of band.
*/
TEGDAL_DLL void Convert2TerraLib(GDALRasterBand* rasterBand, TeRasterParams& params, const int& i);

/*!
    \brief It converts the GDAL Color Interpretation type to TerraLib Raster Photometric Interpretation.
    \param  colorInterpretation A valid GDAL color interpretation.
	\return TerraLib Raster Photometric Interpretation.
*/
TEGDAL_DLL TeRasterParams::TeRasterPhotometricInterpretation Convert2TerraLib(GDALColorInterp colorInterpretation);

/*!
    \brief It reads the lut form a GDAL raster band and converts it to TerraLib.
    \param  rasterBand[in] A valid GDAL raster band with color table associated.
	\return params[out] TerraLib Raster Parameters that will be filled.
*/
TEGDAL_DLL void ReadLUT(GDALRasterBand* rasterBand, TeRasterParams& params);

/*!
    \brief It converts the GDAL data type to TerraLib Data Type.
    \param  type A valid GDAL data type.
	\return TerraLib Data Type enum element.
*/
TEGDAL_DLL TeDataType Convert2TerraLib(GDALDataType type);

/*!
    \brief It converts the TerraLib data type to GDAL Data Type.
    \param  type A valid TerraLib data type.
	\return GDAL Data Type enum element.
*/
TEGDAL_DLL GDALDataType Convert2GDAL(TeDataType type);

/*!
    \brief It converts the GDAL affine transformation coefficients to TerraLib Raster Parameters set.
    \param	geoTransformParams[in] A valid GDAL affine transformation coefficients.
	\params params[out] TerraLib Raster Parameters that will be filled.
	\note This method extracts the raster resolution (x,y) and builds the correct box based on affine transformation coefficients.
	\note geoTransformParams is an array of double with size = 6, where:
											geoTransformParams[0] - top left x 
											geoTransformParams[1] - w-e pixel resolution
											geoTransformParams[2] - rotation, 0 if image is "north up"
											geoTransformParams[3] - top left y
											geoTransformParams[4] - rotation, 0 if image is "north up"
											geoTransformParams[5] - n-s pixel resolution
*/
TEGDAL_DLL void Convert2TerraLib(double* geoTransformParams, TeRasterParams& params);

/*!
    \brief It converts the TerraLib Raster Parameters set to  GDAL affine transformation coefficients.
    \param	geoTransformParams[out] A valid GDAL affine transformation coefficients.
	\params params[in] TerraLib Raster Parameters that will be filled.
	\note This method extracts the raster resolution (x,y) and builds the correct box based on affine transformation coefficients.
	\note geoTransformParams is an array of double with size = 6, where:
											geoTransformParams[0] - top left x 
											geoTransformParams[1] - w-e pixel resolution
											geoTransformParams[2] - rotation, 0 if image is "north up"
											geoTransformParams[3] - top left y
											geoTransformParams[4] - rotation, 0 if image is "north up"
											geoTransformParams[5] - n-s pixel resolution
*/
TEGDAL_DLL void Convert2GDAL(double* geoTransformParams, const TeRasterParams& params);

/*!
    \brief Creates a raster data using GDAL.
    \param params TerraLib Raster Parameters that will be used.
    \return A pointer to a GDALDataset if it succeeds or a NULL pointer otherwise. Caller is responsible for kill it!
*/
TEGDAL_DLL GDALDataset* CreateGDALRaster(TeRasterParams& params);

#endif // _TEGDALUTILS_H
