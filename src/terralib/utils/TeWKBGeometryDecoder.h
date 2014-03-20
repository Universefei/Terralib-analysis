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

#ifndef  __TERRALIB_INTERNAL_WKBGEOMETRYDECODER_H
#define  __TERRALIB_INTERNAL_WKBGEOMETRYDECODER_H

#include "TeUtilsDefines.h"

//Forward declarations
class TePolygon;
class TeLine2D;
class TeCoord2D;
class TePolygonSet;
class TeLineSet;
class TePointSet;

/** @namespace TeWKBGeometryDecoder
 *
 * @brief Contains decoding/encoding functions for TerraLib geometries into WKB-like geometries. 
 *
 * WKB format is used to struct information about geomtries in well-know manner. It is an interchange format.
 * For more information about WBK format see 
 * <A HREF="">WKB representation.</A>.
 */
namespace TeWKBGeometryDecoder
{
    /** @name Encoding Methods
		* Methods used to encode TeGeometry objects into pointers to char* in WKB format.
		*/
	//@{

	/** @brief Encodes a TePolygon into a WKB. 
	 *	@param polygon Polygon to be encoded. [in]
	 *	@param wkbPoly WKB genenerated by the polygon. [out]
	 *	@param size The size of WKB in byte. [out]
	 */
	TLUTILS_DLL void encodePolygon(const TePolygon& polygon, char*& wkbPoly, unsigned int& size);

	/** @brief Encodes a TeLine2D into a WKB. 
	 *	@param line Polygon to be encoded. [in]
	 *	@param wkbLIne WKB genenerated by the line. [out]
	 *	@param size The size of WKB in byte. [out]
	 */
	TLUTILS_DLL void encodeLine(const TeLine2D& line, char*& wkbLine, unsigned int& size);

	/** @brief Encodes a TeCoord2D into a WKB. 
	 *	@param polygon Polygon to be encoded. [in]
	 *	@param wkbPoly WKB genenerated by the polygon. [out]
	 *	@param size The size of WKB in byte. [out]
	 */
	TLUTILS_DLL void encodePoint(const TeCoord2D& point, char*& wkbPoint, unsigned int& size);
	//@}

    /** @name Decoding Methods
		* Methods used to decode pointers to char* in WKB format into code TeGeometry objects.
		*/
	//@{		

	/** @brief Decodes a WKB into a TePolygon object.
	 *	@param wkbPoly WKB to be decoded. [in]
	 *	@param poly TePolygon decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodePolygon(const char*& wkbPoly, TePolygon& poly, unsigned int& readBytes);

	/** @brief Decodes a WKB into a TeLine2D object.
	 *	@param wkbLine WKB to be decoded. [in]
	 *	@param line TeLine2D decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodeLine(const char*& wkbLine, TeLine2D& line, unsigned int& readBytes);

	/** @brief Decodes a WKB into a TeCoord2D object.
	 *	@param wkbPoint WKB to be decoded. [in]
	 *	@param point TeCoord2D decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodePoint(const char*& wkbPoint, TeCoord2D& point, unsigned int& readBytes);

	/** @brief Decodes a WKB into a TePolygonSet object.
	 *	@param wkbMultiPolygon WKB to be decoded. [in]
	 *	@param pset TePolygonSet decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodePolygonSet(const char*& wkbMultiPolygon, TePolygonSet& pset, unsigned int& readBytes);

	/** @brief Decodes a WKB into a TeLineSet object.
	 *	@param wkbMultiLine WKB to be decoded. [in]
	 *	@param lset TeLineSet decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodeLineSet(const char*& wkbMultiLine, TeLineSet& lset, unsigned int& readBytes);

	/** @brief Decodes a WKB into a TePointSet object.
	 *	@param wkbMultiPoint WKB to be decoded. [in]
	 *	@param ptset TePointSet decoded. [out]
	 *  @param readBytes number of bytes that were read. [out]
	 */
	TLUTILS_DLL void decodePointSet(const char*& wkbMultiPoint, TePointSet& ptset, unsigned int& readBytes);

	//@}
};

#endif  //__TERRALIB_INTERNAL_WKBGEOMETRYENCODER_H
