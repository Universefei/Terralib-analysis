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

/** \file  TeOGRUtils.h
    \brief Utility functions for OGR support.
    \author Douglas Uba <douglas@dpi.inpe.br>
   */

#ifndef _TEOGRUTILS_H
#define _TEOGRUTILS_H

// Defines
#include "TeGDALDefines.h"

// TerraLib include files
#include "../../kernel/TeAttribute.h"
#include "../../kernel/TeBox.h"
#include "../../kernel/TeSTInstance.h"

// OGR include files
#include <ogr_core.h>

// SLT
#include <vector>

// forward declarations
class OGRGeometry;
class OGREnvelope;
class OGRSpatialReference;
class OGRFeatureDefn;
class OGRFieldDefn;
class OGRFeature;

/*!
    \brief It converts the OGR Geometry to TerraLib Geometry.
    \param ogrGeom A valid OGR Geometry.
    \return A vector of TerraLib Geometries.
    \note It uses the WKB to create the TerraLib Geometry.
*/
TEGDAL_DLL std::vector<TeGeometry*> Convert2TerraLib(OGRGeometry* ogrGeom);

/*!
    \brief It converts the TerraLib Geometry to OGR Geometry.
    \param geom A valid TerraLib Geometry.
    \return An OGR geometry.
    \note It uses the WKB to create the OGR Geometry.
*/
TEGDAL_DLL OGRGeometry* Convert2OGR(TeGeometry* geom);

/*!
	\brief It converts the OGR Envelope to TerraLib Box.
	\param env A valid OGR Envelope.
	\return A valid TerraLib Box.
*/
TEGDAL_DLL TeBox Convert2TerraLib(OGREnvelope* env);

/*!
	\brief It converts the OGR Feature Definition to TerraLib Attribute List.
    \param featDef A valid OGR Feature Definition.
    \return A TerraLib Attribute List.
    \note It uses the method that converts a OGR Field Definition to TerraLib Attribute.
*/
TEGDAL_DLL TeAttributeList Convert2TerraLib(OGRFeatureDefn* featDef);

/*!
	\brief It converts the OGR Field Definition to TerraLib Attribute.
	\param fieldDef A valid OGR Field Definition.
	\return A TerraLib Attribute.
*/
TEGDAL_DLL TeAttribute Convert2TerraLib(OGRFieldDefn* fieldDef);

/*!
	\brief It converts the TerraLib Attribute to OGR Field Definition.
	\param attr A valid TerraLib Attribute.
	\return An OGR Field Definition.
*/
TEGDAL_DLL OGRFieldDefn Convert2OGR(const TeAttribute& attr);

/*!
	\brief It converts a OGR Feature to TerraLib STInstance object.
    \param feat A valid OGR Feature.
    \return A TerraLib STInstance object.
*/
TEGDAL_DLL TeSTInstance Convert2TerraLib(OGRFeature* feat);

/*!
	\brief It converts the OGR Geometry Type element to TerraLib Geometric Representation
	\param geomType A valid OGR Geometry Type element.
	\return TerraLib Geometric composite.
*/
TEGDAL_DLL int Convert2TerraLib(OGRwkbGeometryType geomType);

/*!
	\brief It converts the TerraLib Projection to OGR Spatial Reference System.
	\param proj A valid TerraLib Projection.
	\return OGR Spatial Reference System.
*/
TEGDAL_DLL OGRSpatialReference* Convert2OGR(TeProjection* proj);

#endif // _TEOGRUTILS_H
