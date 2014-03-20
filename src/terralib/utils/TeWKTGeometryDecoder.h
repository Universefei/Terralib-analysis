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

#ifndef  __TERRALIB_INTERNAL_WKTGEOMETRYDECODER_H
#define  __TERRALIB_INTERNAL_WKTGEOMETRYDECODER_H

#include "TeUtilsDefines.h"

//STL include files
#include <string>
using namespace std;

//Forward declarations
class TePolygon;
class TeLine2D;
class TeCoord2D;

/** @namespace TeWKTGeometryDecoder
 *
 * @brief Contains decoding/encoding functions for TerraLib geometries into WKT-like geometries. 
 *
 * WKT format is used to struct information about geometries in well-know manner. It is an interchange format.
 * For more information about WKT format see 
 * <A HREF="">WKT representation.</A>.
 */
namespace TeWKTGeometryDecoder
{
    /** @name Encoding Methods
		* Methods used to encode TeGeometry objects into strings in WKT format.
		*/
	//@{		

	/** @brief Encodes a TePolygon into a WKT. 
	 *	@param polygon Polygon to be encoded. 
	 *	@param[out] wktPoly WKT genenerated by the polygon.
	 */
	TLUTILS_DLL void encodePolygon(const TePolygon& polygon, string& wktPoly);

	/** @brief Encodes a TeLine2D into a WKT. 
	 *	@param line Polygon to be encoded.
	 *	@param wktLine[out] WKT genenerated by the line.
	 */
	TLUTILS_DLL void encodeLine(const TeLine2D& line, string& wktLine);

	/** @brief Encodes a TeCoord2D into a WKT. 
	 *	@param polygon Polygon to be encoded.
	 *	@param[out] wktPoint WKT genenerated by the point.
	 */
	TLUTILS_DLL void encodePoint(const TeCoord2D& point, string& wktPoint);
	//@}

    /** @name Decoding Methods
		* Methods used to decode strings in WKT format into code TeGeometry objects.
		*/
	//@{		

	/** @brief Decodes a WKT into a TePolygon object.
	 *	@param wktPoly WKT to be decoded. 
	 *	@param[out] poly TePolygon decoded.
	 */
	TLUTILS_DLL void decodePolygon(const string& wktPoly, TePolygon& poly);

	/** @brief Decodes a WKT into a TeLine2D object.
	 *	@param wktLine WKT to be decoded.
	 *	@param[out] line TeLine2D decoded.
	 */
	TLUTILS_DLL void decodeLine(const string& wktLine, TeLine2D& line);

	/** @brief Decodes a WKT into a TeCoord2D object.
	 *	@param wktPoint WKT to be decoded.
	 *	@param[out] point TeCoord2D decoded.
	 */
	TLUTILS_DLL void decodePoint(const string& wktPoint, TeCoord2D& point);
	//@}
};

#endif  //__TERRALIB_INTERNAL_WKTGEOMETRYENCODER_H
