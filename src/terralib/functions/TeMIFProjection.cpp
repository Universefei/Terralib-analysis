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

#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning(disable: 4786)
#endif

#include <map>
#include <string>
#include "TeMIFProjection.h"
#include "../kernel/TeDatum.h"
#include "../kernel/TeProjection.h"
#include "../kernel/TeException.h"
#include "../kernel/TeUtils.h"
#include <cstdlib>

using namespace std;
/*--- 
Projection Information

contains information about the parameters
needed by each projection

parameters are:

- Name
- Datum 
- Units 
- Origin Longitude,
- Origin Latitude,
- Standard Paralel 1
- Standard Paralel 2
- Scale Factor
- False Easting
- False Northing 
- Range

---*/

const int NUM_MIF_PROJ = 7;
const int NUM_MIF_DATUM = 19;
const int NUM_MIF_UNITS = 13;


// Name            Units  Long  Lat  Par1  Par2	Sca  Eas  Nor 
char* mifProjInfo[]= {
"LatLong",          "0",  "0",  "0", "0",  "0",	"0", "0", "0", 
"Albers",           "1",  "1",  "1", "1",  "1",	"0", "1", "1", 
"LambertConformal", "1",  "1",  "1", "1",  "1",	"0", "1", "1", 
"Mercator",         "1",  "1",  "0", "0",  "0",	"0", "0", "0", 
"Miller",           "1",  "1",  "0", "0",  "0",	"0", "0", "0", 
"UTM",              "1",  "1",  "1", "0",  "0",	"1", "1", "1", 
"Polyconic",        "1",  "1",  "1", "1",  "0",	"0", "1", "1"
};

char* mifProjCode[]= {
"1", "LatLong",
"3",  "LambertConformal",
"8",  "UTM",
"9",  "Albers",
"10", "Mercator", 
"11", "Miller",
"27", "Polyconic"
};

char* mifDatumCode[] = {
"0",   "Spherical",
"23",  "Astro-Chua",	
"24",  "CorregoAlegre",
"40",  "Indian",
"41",  "Indian",
"62",  "NAD27",
"63",  "NAD27",
"64",  "NAD27",
"65",  "NAD27",
"66",  "NAD27",
"67",  "NAD27",
"68",  "NAD27",
"69",  "NAD27",
"70",  "NAD27",
"71",  "NAD27",
"72",  "NAD27",
"74",  "NAD83",
"92",  "SAD69",
"104", "WGS84"
};

char* mifUnitsCode[] = {
"6",  "Centimeters",
"31", "Chains",
"3",  "Feet",
"2",  "Inches",
"1",  "Kilometers",
"30", "Links",
"7",  "Meters",
"0",  "Miles",
"5",  "Millimeters",
"9",  "Nautical Miles",
"32", "Rods",
"8",  "US Survey Feet",
"4",  "Yards"
};



// Definitions for types used
typedef map<int,string> NameMap;
typedef map<int, string> MIFCodeMap;

// Prototypes for aux functions

TeProjInfo TeMIFProjectionInfo  ( const string& projName  );
TeDatum TeMIFProjectionDatum    ( const int&  datCode );
string TeMIFProjectionName      ( const int&  prjCode );
string TeMIFProjectionUnits	( const int& unitsCode );


TeProjection*
TeMIFProjectionFactory::make ( vector<string>& argList )
{
	// Create a TerraLib projection from MIF information]
	// INPUT - a list of parameters

	int argInd = 0;

	TeProjectionParams params;
	params.hemisphere = TeSOUTH_HEM;

	// Step 1 - read the projection name
	
	int projCode = atoi( argList[argInd++].c_str() );

	params.name = TeMIFProjectionName ( projCode );

	// Step 2 - read the datum

	int datumCode = atoi( argList[argInd++].c_str() );
	TeProjInfo pjInfo;
	try 
	{    
		params.datum = TeMIFProjectionDatum ( datumCode );
		// Step 3 - Build a map of Projection info
		pjInfo = TeMIFProjectionInfo ( params.name );
	}
	catch (...)
	{
		TeProjection* nproj = new TeNoProjection();
		return nproj;
	}

	// Step 4 - Read the appropriate parameters

	if ( pjInfo.hasUnits ) 
	{
		string unitsName = argList[argInd++];
		string unitsName2 = TeRemoveSpecialChars(unitsName);
		if (unitsName2 == "m" || unitsName2 == "M")
			unitsName2 = "Meters";
		params.units = unitsName2;
	}
	else
		params.units = "DecimalDegrees"; 

	string val; 
	if ( pjInfo.hasLon0 )
	{
		val = argList[argInd++].c_str();
		params.lon0 = atof(val.c_str())*TeCDR;
	}

	if ( pjInfo.hasLat0 )
	{
		val = argList[argInd++].c_str();
		params.lat0 = atof(val.c_str())*TeCDR;
	}

	if ( pjInfo.hasStlat1 )
	{
		val = argList[argInd++].c_str();
		params.stlat1 = atof(val.c_str())*TeCDR;
	}

	if ( pjInfo.hasStlat2 )
	{
		val = argList[argInd++].c_str();
		params.stlat2 = atof(val.c_str())*TeCDR;
	}

	if ( pjInfo.hasScale )
	{
		val = argList[argInd++].c_str();
		params.scale = atof(val.c_str());
	}

	if ( pjInfo.hasOffx )
	{
		val = argList[argInd++].c_str();
		params.offx = atof(val.c_str());
	}

	if ( pjInfo.hasOffy )
	{
		val = argList[argInd++].c_str();
		params.offy = atof(val.c_str());
	}

	// Step 4 - create the projection

	return TeProjectionFactory::make ( params );
}


TeProjInfo
TeMIFProjectionInfo ( const string& projName  )
{

	TeProjInfoMap pjMap;
	TeProjInfo pjInfo;

	int k = 0;

	for ( int i = 0; i < NUM_MIF_PROJ; i++ )
	{
		string name = mifProjInfo [k++];
			
	    pjInfo.hasUnits  = atoi ( mifProjInfo [k++] );
	    pjInfo.hasLon0   = atoi ( mifProjInfo [k++] );
	    pjInfo.hasLat0   = atoi ( mifProjInfo [k++] );
	    pjInfo.hasStlat1 = atoi ( mifProjInfo [k++] );
		pjInfo.hasStlat2 = atoi ( mifProjInfo [k++] );
		pjInfo.hasScale  = atoi ( mifProjInfo [k++] );
		pjInfo.hasOffx   = atoi ( mifProjInfo [k++] );
	    pjInfo.hasOffy   = atoi ( mifProjInfo [k++] );

		pjMap [ name ] = pjInfo;
	}


	TeProjInfoMap::iterator it = pjMap.find ( projName );

	if ( it == pjMap.end() )
		throw TeException ( PROJECTION_NOT_AVAILABLE );

return (*it).second;
}

TeDatum
TeMIFProjectionDatum ( const int& datCode )
{

	MIFCodeMap datumCode;

	int k= 0;

	for ( int i = 0; i < NUM_MIF_DATUM; i++ )
	{
		int code    = atoi ( mifDatumCode [k++] );
		string name = mifDatumCode [k++];

		datumCode [ code ] = name;
	}

	MIFCodeMap::iterator it = datumCode.find ( datCode );

	if ( it == datumCode.end() )
		throw TeException ( DATUM_ERROR );			
		
    string datName = (*it).second;

	return TeDatumFactory::make ( datName );

}

string
TeMIFProjectionName ( const int& prjCode )
{

	MIFCodeMap projCode;
	
	int k= 0;

	for ( int i = 0; i < NUM_MIF_PROJ; i++ )
	{
		int code    = atoi ( mifProjCode [k++] );
		string name = mifProjCode [k++];

		projCode [ code ] = name;
	}

	MIFCodeMap::iterator it = projCode.find ( prjCode );

	if ( it == projCode.end() )
		throw TeException ( PROJECTION_NOT_AVAILABLE );

return (*it).second;

}

string 
TeMIFProjectionUnits ( const int& code )
{
	MIFCodeMap unitsCode;
	
	int k= 0;

	for ( int i = 0; i < NUM_MIF_UNITS; i++ )
	{
		int code    = atoi ( mifUnitsCode [k++] );
		string name = mifUnitsCode [k++];

		unitsCode [ code ] = name;
	}

	MIFCodeMap::iterator it = unitsCode.find ( code );

	if ( it == unitsCode.end() )
		return "Meters";

return (*it).second;

}
