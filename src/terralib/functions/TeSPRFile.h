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
/*! \file TeSPRFile.h
    \brief This file contains the support to manipulate geographical data in ASCII SPRING format
*/
#ifndef  __TERRALIB_INTERNAL_SPRFILE_H
#define  __TERRALIB_INTERNAL_SPRFILE_H

#include "../kernel/TeAsciiFile.h"
#include "../kernel/TeDataTypes.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeProjection.h"

#include "TeFunctionsDefines.h"

#include <string>
#include <stdio.h>
using namespace std;

//! Provides support for writing SPRING/ASCII files
class TLFUNCTIONS_DLL TeSPRFile : public TeAsciiFile
{
public:

	//! Contructors

	TeSPRFile(const string& fileName, const char* mode, char separator = ';');

	//! Destructor

	virtual ~TeSPRFile() {} //! base class

	//! LINES - write header
	void writeLinesHeader ();

	//! LINES 3D - write header
	void writeLine3DHeader()
	{ fprintf ( file_,"LINE3D\n"); }

	//! LINES - write Coord
	void writeCoord ( const double& x, const double& y)
{ 	fprintf ( file_," %.6f %.6f\n", x, y ); }

	//! LABELS - write Header
	void writeLabelHeader ( const int& option );

	//! LABELS - write Value
	void  writeCadastralLabel ( const string& geoid, const double& x, const double& y,
			const string& objectCategory = ""); 

	//! NETWORK - write labels
	void writeNetworkLabel ( const string& geoid, const double& x, const double& y,
					   const string& objectCategory, bool isL=true); 

	//! LABELS - write Theme
	void writeThematicLabel( const double& x, const double& y, const string& theme );

	//! POINTS - write a classified point
	void writeThematicPoint( const double& x, const double& y, const string& theme );

	//! POINTS - write Header
	void writePointsHeader();

	//! POINTS 3D - write Header
	void writePoint3DHeader()
	{ fprintf ( file_,"POINT3D\n"); }

	//! POINTS 3D - write Value
	void writePoint3D( const double& x, const double& y, const double& quote )
	{ fprintf ( file_," %.6f %.6f %.10E\n", x, y, quote ); }

	//! POINTS - write Value
	void writePoint ( TeCoord2D& p )
	{ this->writeCoord ( p.x(), p.y() ); }

	//! NETWORK - write Header
	void writeNetworkHeader();

	//! NETWORK - write arc header
	void writeArcHeader ( const string& geoid, const double& posImp = 0., 
		const double& negImp= 0., const double& demand = 0. ); 
				   
 	//! NETWORK - write Lines Header
	void writeNetworkLineHeader(int id )
	{ fprintf ( file_,"%d 1 1 0.0\n", id ); }

 	//! NETWORK - write Label Header
	void writeNetworkLabelHeader();
	
	//! TABLE - write Header
	void writeTableHeader( const int& dType );

	//! TABLE - write attribute information
	void writeAttributeInfo ( TeAttributeList&, const string&, const string& indexName = "" );

	//! TABLE - write text parameter
	void writeTextParameterInfo ( const string& name, int w = 0);
	
	//! TABLE - write real parameter
	void writeRealParameterInfo ( const string& );
	
	//! TABLE - write int parameter
	void writeIntParameterInfo  ( const string& );

	//! TABLE - write geoId
	void writeGeoId ( const string& geoid )
	{ fprintf ( file_, "%s%c%s", geoid.c_str(), separator_, geoid.c_str() ); }

	//! TABLE - write attribute name
	void writeValue ( const string& attrName, TeAttrDataType type );

	//! TABLE - write int value
	void writeValue ( const int& value )
	{ fprintf ( file_, "%c%d", separator_, value ); } 

	//! TABLE - write double value
	void writeValue ( const double& value )
	{ fprintf ( file_, "%c%.10E", separator_, (float)value ); }

	//! TABLE - write string 
	void writeValue ( const string& value )
	{ fprintf ( file_, "%c%s", separator_,value.c_str() ); }

	//! SAMPLE - write Header
	void writeSampleHeader();
	
	//! SAMPLE - write value
	void writeSample ( const string& index, const double& x, const double& y,
		const string& value);
	
	//! SAMPLE - write quote
	void writeSampleQuote( double quote )
	{ fprintf ( file_,"HEIGHT %.10E\n", quote ); }

	//! INFO - write projection
	void writeProjection ( TeProjection* );

	//! INFO - write Box
	void writeBox ( TeBox& );

	//! INFO - write InfoEND
	void writeInfoEnd ()
	{ fprintf ( file_, "%s\n", "INFO_END"); }

	//! GENERAL - write END
	void writeEnd ()
	{ fprintf ( file_, "%s \n", "END" ); }

	//! Sets the separator
	void setSeparator ( const char& c )
	{ separator_ = c ; }

	//! Returns the separator
	char separator ()
	{ return separator_; } 

//! No copy allowed

private:

	TeSPRFile(const TeSPRFile&);
	TeSPRFile& operator=(const TeSPRFile&){return *this;}
	char	separator_;
};

#endif

