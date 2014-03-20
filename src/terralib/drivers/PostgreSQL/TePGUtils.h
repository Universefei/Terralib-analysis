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
/*! \file TePGUtils.h
    \brief This file contains utilities functions to read/write geometry fields (PostGIS and PostgreSQL).
	\author Gilberto Ribeiro de Queiroz - gribeiro@dpi.inpe.br
 */

#ifndef  __TERRALIB_INTERNAL_TePOSTGRESQLUTILS_H
#define  __TERRALIB_INTERNAL_TePOSTGRESQLUTILS_H

#include "../../kernel/TeGeometry.h"

#include "TePostgreSQLDefines.h"

#define COORDS_DELIM		','

#define COORD_DELIM			' '

#define LINES_DELIM		','

#define LDELIM_POINT		'('
#define RDELIM_POINT		')'

#define LDELIM_LINESTRING	'('
#define RDELIM_LINESTRING	')'

#define LDELIM_POLYGON		'('
#define RDELIM_POLYGON		')'

#define LDELIM_BOX			'('
#define RDELIM_BOX			')'

// DELIMITADORES PARA TIPOS GEOMETRICOS DO POSTGRESQL
#define LDELIM_PGGEOMPOLYGON       '('
#define RDELIM_PGGEOMPOLYGON       ')'

#define PGGEOMPOLYGON_COORDS_DELIM ','

/*
 * Codificadores e decodificadores de tipos geometricos do PostgreSQL.
 * Essas funcoes sao para o driver sem extensao espacial.
 *
 */

//! Decodifica uma coordenada no formato poligono dos tipos geometricos do PostgreSQL: (x1, y1)
TLPOSTGRESQL_DLL TeCoord2D PgGeomPolygonCoordDecode(char *str, char* *s);

//! Transaforma a linha l em um poligono texto strPolygon
TLPOSTGRESQL_DLL void Te2PgGeomPolygon(const TeLine2D& l, string& strPolygon);

//! Transaforma um poligono texto strPolygon em uma linha
TLPOSTGRESQL_DLL TeLine2D PgGeomPolygon2Te(char* strPolygon);

/*
 * Codificadores e decodificadores PostGIS - Texto
 * 
 */
TLPOSTGRESQL_DLL TeCoord2D PGCoord_decode(char *str, char* *s);

TLPOSTGRESQL_DLL string PGCoord_encode(const TeCoord2D& c);

TLPOSTGRESQL_DLL TePoint PGPoint_decode(char *str);

TLPOSTGRESQL_DLL string PGPoint_encode(const TePoint& p);

TLPOSTGRESQL_DLL string PGNode_encode(const TeNode& p);

TLPOSTGRESQL_DLL TeLine2D PGLine_decode(char *str, char* *sd);

TLPOSTGRESQL_DLL TeLine2D PGLinestring_decode(char *str);

TLPOSTGRESQL_DLL TePolygon PGPolygon_decode(char *str);

TLPOSTGRESQL_DLL TeBox PGBox_decode(char *str);

TLPOSTGRESQL_DLL string PGBox_encode(const TeBox& b);

TLPOSTGRESQL_DLL TeBox PGBoxRtree_decode(char *str);

TLPOSTGRESQL_DLL string PGBoxRtree_encode(const TeBox& b);

TLPOSTGRESQL_DLL void PGConcatLineString(const TeLine2D& l, string& concatString);

TLPOSTGRESQL_DLL TeBox PGBoxFromPolygon(char *str);

TLPOSTGRESQL_DLL string PGMakePolygon(const TeBox& box);


#endif	// __TERRALIB_INTERNAL_TePOSTGRESQLUTILS_H

