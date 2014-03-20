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
 
#ifdef WIN32 
#pragma warning ( disable: 4786 ) 
#endif

#include "TeSPRFile.h" 
#include "../kernel/TeAssertions.h" 
#include "../kernel/TeDefines.h"
#include "../kernel/TeProjection.h"
#include "../kernel/TeDatum.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeDataTypes.h"

#include <cstdlib>
  
TeSPRFile::TeSPRFile ( const string& name, const char* mode, char separator): 	
	TeAsciiFile ( name, mode  ),
	separator_(separator)
{}


void
TeSPRFile::writeLabelHeader ( const int& option )	 
{
	if (option == TeSPRNetwork) 
		fprintf(file_,"%s\n", "NETWORK_OBJECTS" );
	else 
		fprintf(file_,"%s\n", "POINTS" ); 	
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s %c \n", "SEPARATOR ",  separator_  );
	fprintf(file_,"%s\n", "// SPRING Interchange File" );
	
	if (option == TeSPRThematic)
		fprintf(file_,"%s \n", "// Format coordx ; coordy ; classe");	
	else 
		fprintf(file_,"%s \n", "// Format coordx ; coordy  ; label ; name ; category_obj");	
	fprintf(file_,"%s\n", "INFO_END"); 
}

 
void  
TeSPRFile::writeCadastralLabel ( const string& geoid, const double& x, const double& y,
					   const string& objectCategory) 
{
	if (geoid != "")
	{
		fprintf ( file_, "%.6f %c %.6f %c %s %c %s %c %s \n", 
		(float)x, separator_, (float)y, separator_, geoid.c_str(), separator_, geoid.c_str(), separator_, objectCategory.c_str() );  
	}
}

void 
TeSPRFile::writeThematicLabel( const double& x, const double& y, const string& theme )
{
	if ( theme != "")
		fprintf ( file_, "%.6f %c %.6f %c %s \n", 
		(float)x, separator_, (float)y, separator_, theme.c_str() );  
}

void  
TeSPRFile::writeNetworkLabel ( const string& geoid, const double& x, const double& y,
					   const string& objectCategory, bool isL) 
{
	if (geoid != "")
	{
		if (isL)
			fprintf ( file_, "%.6f %c %.6f %c %s %c %s %c %s %c LINES\n", 
			(float)x, separator_, (float)y, separator_, geoid.c_str(), separator_, geoid.c_str(), separator_, objectCategory.c_str(), separator_ );  
		else
			fprintf ( file_, "%.6f %c %.6f %c %s %c %s %c %s %c NODES\n", 
			(float)x, separator_, (float)y, separator_, geoid.c_str(), separator_, geoid.c_str(), separator_, objectCategory.c_str(), separator_ );  
	}

}


void 
TeSPRFile::writeThematicPoint( const double& x, const double& y, const string& theme )
{
	if ( theme != "")
		fprintf ( file_, "%.6f %c %.6f %c %s \n", 
		(float)x, separator_, (float)y, separator_, theme.c_str() );  
	else
		fprintf ( file_, "%.6f %c %.6f %c Nenhuma \n", 
		(float)x, separator_, (float)y, separator_ );  

}


void 
TeSPRFile::writeProjection( TeProjection* proj ) 
{
	if (proj)
	{
		fprintf ( file_, "PROJECTION %s/%s\n", proj->name().c_str(),proj->datum().name().c_str());
	}
}

void 
TeSPRFile::writeBox( TeBox& box ) 
{
	TeCoord2D pt1 = box.lowerLeft();
	TeCoord2D pt2 = box.upperRight();

	fprintf ( file_, "%s %.6f %.6f %.6f %.6f \n", "BOX", pt1.x(), pt1.y(), pt2.x(),  pt2.y());
	fprintf ( file_, "%s\n", "// " );
}
  
//===================================================
//
//  Write lines header information
//
//===================================================

void
TeSPRFile::writeLinesHeader ()  
{
	fprintf(file_,"%s\n", "LINES" );
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s\n", "// SPRING Interchange File" );
}



//===================================================
//
//  TeSPRPoints- Write points header information
//
//===================================================

void
TeSPRFile::writePointsHeader ()  
{
	fprintf(file_,"%s\n", "POINT2D" );
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s %c \n", "SEPARATOR ",  separator_  );
	fprintf(file_,"%s\n", "// SPRING Interchange File" );
}

//===================================================
//
//  TeSPRTable - Write TABLE information
//
//===================================================

void
TeSPRFile::writeTableHeader (const int& /* dType */)  
{
	fprintf(file_,"%s\n", "TABLE" );
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s %c\n", "SEPARATOR ", separator_ );
	fprintf(file_,"%s\n", "UNFORMATED ");	
 } 
 
void 
TeSPRFile::writeAttributeInfo( TeAttributeList& attList, const string& objCategory,
							   const string& indexName ) 
{
	fprintf (file_, "%s %s \n", "CATEGORY_OBJ", objCategory.c_str() ); 	
	fprintf (file_, "%s%c%s%c%s%c%s \n", "T_KEY", separator_, "TEXT", separator_, "32", separator_, "0"); 	

	TeAttributeList::iterator it = attList.begin();
	for ( ;it != attList.end();++it )
	{
		string name = (*it).rep_.name_;
		if ( TeConvertToUpperCase(name) == TeConvertToUpperCase(indexName) ) continue; // skip the index name
		switch ((*it).rep_.type_)
		{
			case TeSTRING:
				this->writeTextParameterInfo(name, (*it).rep_.numChar_);
				break;
			case TeREAL:
				this->writeRealParameterInfo(name);
				break;
			case TeINT:
				this->writeIntParameterInfo(name);
				break;
			case TeCHARACTER:
				this->writeTextParameterInfo(name,1);
				break;
			case TeDATETIME:
				this->writeTextParameterInfo(name,25);
            default:
                break;
		}
	}  
} 
void
TeSPRFile::writeTextParameterInfo ( const string& name, int w )
{

	char sprName [TeNAME_LENGTH];

	int width = w;
	if ( w == 0 )
		width = name.size();

	TeConvertToUpperCase ( name, sprName );

	fprintf ( file_, "%s%c%s%c%d%c%d \n", sprName, separator_, "TEXT", separator_, width, separator_, 0 );
}

void
TeSPRFile::writeRealParameterInfo ( const string& name )
{
	char sprName [TeNAME_LENGTH];

	TeConvertToUpperCase ( name, sprName );

	fprintf ( file_, "%s%c%s%c%d%c%d \n", sprName, separator_, "REAL", separator_, 17, separator_,8 );
}

void
TeSPRFile::writeIntParameterInfo ( const string& name )
{
	char sprName [TeNAME_LENGTH];

	TeConvertToUpperCase ( name, sprName );

	fprintf ( file_, "%s%c%s%c%d%c%d \n", sprName, separator_, "INTEGER", separator_, 10, separator_, 0 );
}

void
TeSPRFile::writeValue ( const string& value, TeAttrDataType type )
{
	switch ( type )
	{
		case TeINT :
			writeValue ( atoi ( value.c_str())  ); 
			break;
		case TeREAL :
			writeValue ( atof ( value.c_str())  ); 
			break;
		default :
			writeValue ( value );
	}

}


//===================================================
//
//  Write SAMPLE information
//
//===================================================

void
TeSPRFile::writeSampleHeader ()
{
	fprintf(file_,"%s\n", "SAMPLE" );
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s %c \n", "SEPARATOR ",  separator_  );
	fprintf(file_,"%s\n", "// SPRING Interchange File" );
}


void
TeSPRFile::writeSample( const string& /* geoid */, const double& x, 
 						  const double& y, const string& value) 
{
	fprintf ( file_, "%s \n", "POINT3D" );
	fprintf ( file_, "%.6f %c %.6f %c %s \n", x, separator_, y, separator_, value.c_str() ); 
	fprintf ( file_, "%s \n", "END" );
}

//===================================================
//
//  Write NETWORK information
//
//===================================================


void 
TeSPRFile::writeNetworkHeader()
{
	fprintf(file_,"%s\n", "NETWORK" );
	fprintf(file_,"%s\n", "INFO");
	fprintf(file_,"%s\n", "// SPRING Interchange File" );
}


void
TeSPRFile::writeArcHeader ( const string& geoid, const double& posImp, 
		const double& negImp, const double& demand)
{
	fprintf ( file_, "%s %.6f %.6f %.6f \n", geoid.c_str(), posImp, negImp, demand ); 
}


