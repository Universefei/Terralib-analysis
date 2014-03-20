/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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

#include <cstring>
#include <cstdlib>
#include <map>

using namespace std;

#include "TeDatum.h"
#include "TeException.h"
#include "TeUtils.h"

const int NUMDATUM = 11; // number of pre-defined datums
const int NUMSPH   = 10; // number of pre-defined ellipsoids

bool tokenizeWKT(char** wkt, vector<string>& tokens);

struct SphParam 
{
	double rd;
	double flt;
};

typedef map<string, SphParam> SphMap;

// Spheroid       Radius(Semi Major Axis)        Flattening
const char* ellipsoidList [] = 
{ 
"Airy",	       "6.3775634e+06", "3.340906e-03",	  
"Clarke-1866", "6.378206e+06",	"3.390060e-03",	 
"EarthRadius", "6.371000e+06",	"0.000000e+00",
"Everest",     "6.3772763e+06", "3.324468e-03",
"GRS80",       "6.378137e+06",  "3.352811e-03",
"Hayford", 	   "6.378388e+06",  "3.367003e-03", 	 
"UGGI67", 	   "6.378160e+06",	"3.352892e-03",	 
"WGS84",	   "6.378137e+06",	"3.352811e-03",
"WGS84/Spherical",	   "6.378137e+06",	"0.000000e+00",
"SIRGAS2000",	   "6.3781370e+06",	"3.35281068e-03"
};

// DatumName    Spheroid        Shift_X           Shift_Y           Shift_Z         (Shifts in relation to WSG84)
const char* datumList [] = 
{ 
"Aratu",           "Hayford",	      "-1.450000e+02",  "2.960000e+02",  "-1.470000e+02", "6208",
"Astro-Chua",      "Hayford",         "-1.438700e+02",  "2.433700e+02",  "-3.352000e+01", "6224",
"CorregoAlegre",   "Hayford",	      "-2.055700e+02",  "1.687700e+02",  "-4.120000e+00", "6225",
"Indian",          "Everest",	      "-6.687000e+01",  "4.370000e+00",  "-3.852000e+01", "6240",
"NAD27",           "Clarke-1866",     "-6.687000e+01",  "4.370000e+00",  "-3.852000e+01", "6267",
"NAD83",           "GRS80",	          "-6.687000e+01",  "4.370000e+00",  "-3.852000e+01", "6269",
"SAD69",           "UGGI67",          "-6.687000e+01",  "4.370000e+00",  "-3.852000e+01", "6618",
"SIRGAS2000",      "SIRGAS2000",      "0.000000e+00",   "0.000000e+00",  "0.000000e+00",  "6674",
"Spherical",       "EarthRadius",     "-6.687000e+01",  "4.370000e+00",  "-3.852000e+01", "-1",
"WGS84",           "WGS84",	          "0.000000e+00",   "0.000000e+00",  "0.000000e+00",  "6326",
"WGS84/Spherical", "EarthRadius", "0.000000e+00",   "0.000000e+00",  "0.000000e+00", "-1"
};

const char** TeGetDatumList()
{
	return datumList;
}

TeDatum::TeDatum(const TeDatum& other)
{
	name_ = other.name_;
	rd_ = other.rd_ ;
	flt_= other.flt_;
	dx_ = other.dx_;
	dy_= other.dy_;
	dz_ = other.dz_;
	epsgCode_ = other.epsgCode_;
}

TeDatum& TeDatum::operator=(const TeDatum& rhs)
{
	if ( this != &rhs )
	{
		name_ = rhs.name_;
		rd_ = rhs.rd_;
		flt_= rhs.flt_;
		dx_ = rhs.dx_;
		dy_= rhs.dy_;
		dz_ = rhs.dz_;
		epsgCode_ = rhs.epsgCode_;
	}
	return *this;
}

string TeDatum::getProj4Description()
{

	string datumProj4 = "";
	if (name_ == "Aratu")
		datumProj4 += " +ellps=intl";
	else if ((name_ == "Astro-Chua") || (name_ == "CorregoAlegre"))
	   datumProj4 = " +ellps=intl";
	else if (name_ == "Indian")
		datumProj4 = " +a=6377276.345 +b=6356075.41314024";
	else if (name_ == "NAD27")
		datumProj4 = " +ellps=clrk66";
	else if (name_ == "NAD83")
		datumProj4 = " +ellps=GRS80 +datum=NAD83";	
	else if (name_ == "SAD69")
		datumProj4 = " +ellps=aust_SA";	
	else if (name_ == "SIRGAS2000")
		datumProj4 = " +ellps=aust_SA";			
	if ((name_ == "Spherical") ||
	    (name_ == "WGS84/Spherical")) 
		datumProj4 += " +a=6378137 +b=6378137";
	else if (name_ == "WGS84")
		datumProj4 = " +ellps=GRS80";
		
	if (dx_ != 0.0 || dy_ || 0.0 && dz_ || 0.0)
		datumProj4 += " +towgs84=" + Te2String(dz_,4) + "," +  Te2String(dy_,4)+ "," +  Te2String(dz_,4);

	return datumProj4;
}

std::string TeDatum::getWKTDatumNameForGEOGCS()
{
	std::map<std::string, std::string> teDatumNameToWKT;
	teDatumNameToWKT["Aratu"] = "Aratu";
	teDatumNameToWKT["Astro-Chua"] = "Chua";
	teDatumNameToWKT["CorregoAlegre"] = "Corrego Alegre";
	teDatumNameToWKT["Indian"] = "Indian 1975";
	teDatumNameToWKT["NAD27"] = "NAD27";
	teDatumNameToWKT["NAD83"] = "GCS_North_American_1983";
	teDatumNameToWKT["SAD69"] = "SAD69";
	teDatumNameToWKT["SIRGAS2000"] = "SIRGAS 2000";	
	teDatumNameToWKT["Spherical"] = "Sphere";
	teDatumNameToWKT["WGS84"] = "WGS 84";

	std::string name = teDatumNameToWKT[name_];
	return name;
}

string TeDatum::getWKTDescription()
{
	std::map<std::string, std::string> teDatumNameToWKT;
	teDatumNameToWKT["Aratu"] = "Aratu";
	teDatumNameToWKT["Astro-Chua"] = "Chua";
	teDatumNameToWKT["CorregoAlegre"] = "Corrego_Alegre";
	teDatumNameToWKT["Indian"] = "Indian_1975";
	teDatumNameToWKT["NAD27"] = "North_American_Datum_1927";
	teDatumNameToWKT["NAD83"] = "North_American_Datum_1983";
	teDatumNameToWKT["SAD69"] = "South_American_Datum_1969";
	teDatumNameToWKT["SIRGAS2000"] = "Sistema_de_Referencia_Geocentrico_para_America_del_Sur_2000";	
	teDatumNameToWKT["Spherical"] = "Sphere";
	teDatumNameToWKT["WGS84"] = "WGS_1984";
	
	std::string name = teDatumNameToWKT[name_];
	if(name.empty())
		name = name_;
	string datumWKT = "DATUM[\"" + name;  
    datumWKT += "\",SPHEROID[\"";

	string sph_epsgcode = "";
	string dt_epsgcode = "";
	string spheroidName = "";

	if (name_ == "Aratu")
	{
		dt_epsgcode = "6208";
		sph_epsgcode = "7022";
		spheroidName = "International 1924";
	}
	else if (name_ == "Astro-Chua") 
	{
		dt_epsgcode = "6224";
		sph_epsgcode = "7022";
		spheroidName = "International 1924";
	}
	else if (name_ == "CorregoAlegre")
	{
		dt_epsgcode = "6225";
		sph_epsgcode = "7022";
		spheroidName = "International 1924";
	}
	else if (name_ == "Indian")
	{
		dt_epsgcode = "6240";
		sph_epsgcode = "7022";
		spheroidName = "International 1924";
	}
	else if (name_ == "NAD27")
	{
		dt_epsgcode = "6267";
		sph_epsgcode = "7008";
		spheroidName = "Clarke 1866";
	}
	else if (name_ == "NAD83")
	{
		dt_epsgcode = "6269";
		sph_epsgcode = "7019";
		spheroidName = "GRS 1980";
	}
	else if (name_ == "SAD69")
	{
		dt_epsgcode = "6618";
		sph_epsgcode = "7050";
		spheroidName = "GRS 1967 (SAD69)";
	}
	else if (name_ == "SIRGAS2000")
	{
		dt_epsgcode = "6674";
		sph_epsgcode = "7019";
		spheroidName = "GRS 1980";
	}
	else if ((name_ == "Spherical") ||
	         (name_ == "WGS84/Spherical")) 
	{
		sph_epsgcode = "";
		dt_epsgcode = "";
		spheroidName = "Sphere";
	}
	else if (name_ == "WGS84")
	{
		dt_epsgcode = "6326";
		sph_epsgcode = "7030";
		spheroidName = "WGS 84";
	}	
	datumWKT += spheroidName;
	datumWKT += "\",";
  datumWKT += Te2String(this->radius(),2);
  datumWKT += ",";
  datumWKT += Te2String(1.0/this->flattening(),9);

	
	if (!sph_epsgcode.empty())
		datumWKT +=",AUTHORITY[\"EPSG\",\"" + sph_epsgcode + "\"]";
	datumWKT += "]";
	
	if (dx_ != 0.0 || dy_ != 0.0 || dz_ != 0.0)
		datumWKT += ",TOWGS84[" + Te2String(dx_,4) + "," + Te2String(dy_,4) + "," + Te2String(dz_,4) + ",0.0,0.0,0.0,0.0]";
		
	if (!dt_epsgcode.empty())
		datumWKT +=",AUTHORITY[\"EPSG\",\"" + dt_epsgcode + "\"]";
	datumWKT += "]";	
	
	return datumWKT;
}

// Static datum map
TeDatumMap TeDatumFactory::_datumMap;

TeDatum TeDatumFactory::make(const string& name)
{
	loadDatumMap();

	TeDatumMap::iterator it = _datumMap.find(name);
	if (it != _datumMap.end()) // found it
		return (*it).second;

	return TeDatum ("Spherical");
}

TeDatum TeDatumFactory::makeFromEPSGCode(const std::string epsgCode)
{
	string dname;
	if (epsgCode == "6208")
		dname = "Aratu";
	else if (epsgCode == "6224")
		dname = "Astro-Chua";
	if (epsgCode == "6225")
		dname = "CorregoAlegre";
	else if (epsgCode == "6240")
		dname = "Indian";
	else if (epsgCode == "6267")
		dname = "NAD27";
	else if (epsgCode == "6169")
		dname = "NAD83";
	else if (epsgCode == "6618")
		dname = "SAD69";
	else if (epsgCode == "6674")
		dname = "SIRGAS2000";		
	else if (epsgCode == "6326")
		dname = "WGS84";
	else 
		dname = "Spherical";
	TeDatum dat = TeDatumFactory::make(dname);
	dat.epsgCode_ = atoi(epsgCode.c_str());
	return dat;
}

TeDatum TeDatumFactory::makeFromWKT(const std::string wktDatumDescription)
{
	TeDatum dat;
	vector<string> tokens;
	char* wktchar = new char[wktDatumDescription.size()+1];
	strcpy(wktchar,wktDatumDescription.c_str());
	if (!tokenizeWKT((char **) &wktchar, tokens))
  {
    delete wktchar;
		return dat;
  }

	unsigned int i =0;
	while (i<tokens.size())
  {
    string tok = tokens[i];
    if (tok == "DATUM")
      break;
		++i;
  }

	if (i== tokens.size())
		return dat;
	++i;

	map<string,string> ewktToTeDatum;
	ewktToTeDatum["D_Chua"] = "Astro-Chua";
	ewktToTeDatum["D_Corrego_Alegre"] = "CorregoAlegre";
	ewktToTeDatum["D_Indian_1975"] = "Indian";
	ewktToTeDatum["D_North_American_1927"] = "NAD27";
	ewktToTeDatum["D_North_American_1983"] = "NAD83";
	ewktToTeDatum["D_South_American_1969"] = "SAD69";
	ewktToTeDatum["D_WGS_1984"] = "WGS84";
	ewktToTeDatum["D_Aratu"] = "Aratu";
	ewktToTeDatum["D_Sistema_de_Referencia_Geocentrico_para_America_del_Sur_2000"] = "SIRGAS2000";		
	ewktToTeDatum["D_SIRGAS_2000"] = "SIRGAS2000";		
	ewktToTeDatum["D_SIRGAS2000"] = "SIRGAS2000";
	ewktToTeDatum["D_SIRGAS"] = "SIRGAS2000";
	
	// lets try the ESRI's WKT format 
	TeTrim(tokens[i]);
	map<string,string>::iterator it = ewktToTeDatum.find(tokens[i]);
	if (it != ewktToTeDatum.end())
	{
		dat = TeDatumFactory::make(it->second);
		return dat;
	}

	map<string,string> wktToTeDatum;
	wktToTeDatum["Aratu"] = "Aratu";
	wktToTeDatum["Chua"] = "Astro-Chua";
	wktToTeDatum["Corrego_Alegre"] = "CorregoAlegre";
	wktToTeDatum["Indian_1975"] = "Indian";
	wktToTeDatum["North_American_Datum_1927"] = "NAD27";
	wktToTeDatum["North_American_Datum_1983"] = "NAD83";
	wktToTeDatum["Sistema_de_Referencia_Geocentrico_para_America_del_Sur_2000"] = "SIRGAS2000";
	wktToTeDatum["South_American_Datum_1969"] = "SAD69";
	wktToTeDatum["Unknown"] = "Spherical";
	wktToTeDatum["WGS_1984"] = "WGS84";


	// lets try OGC's SFS WKT format 
	it = wktToTeDatum.find(tokens[i]);
	if (it != wktToTeDatum.end())
	{
		dat = TeDatumFactory::make(it->second);
		return dat;
	}

	unsigned int is = i;
	//// lets try EPSG authority codes 
	//unsigned int is = i;
	//while (i<tokens.size() && tokens[i] != "AUTHORITY")
	//	++i;
	//if (i== tokens.size())
	//	return dat;
	//++i;
	//if (tokens[i]=="EPSG")
	//{
	//	++i;
	//	dat = TeDatumFactory::makeFromEPSGCode(tokens[i]);
	//	return dat;
	//}

	// so let's try according to the parameters
	i = is;	
	dat.name(tokens[i]);
	while (i<tokens.size() && tokens[i] != "SPHEROID")
		++i;
	if (i== tokens.size())
		return dat;
	++i; ++i;
	dat.radius(atof(tokens[i].c_str()));
	++i;
	dat.flattening(1/atof(tokens[i].c_str()));
	++i;
	if (tokens[i] == "TOWGS84" && i<tokens.size())
	{
		++i;
		double dx = (atof(tokens[i].c_str()));
		++i;
		double dy = (atof(tokens[i].c_str()));
		++i;
		double dz = (atof(tokens[i].c_str()));
		++i;
		++i;
		++i;
		++i;
		dat.dx_ = dx;
		dat.dy_ = dy;
		dat.dz_ = dz;
	}
	return dat;
}

void TeDatumFactory::loadDatumMap()
{
	if(!_datumMap.empty())
		return;

	// Step 1 - Read the initial Spheroids - Put in a Map
	SphMap sphMap;
	SphParam sphPar;
	int i, k = 0;
	for(i = 0; i < NUMSPH; ++i)
	{
		string name = ellipsoidList[k++];
		sphPar.rd = atof (ellipsoidList [k++]);
		sphPar.flt = atof (ellipsoidList [k++]);
		sphMap[name] = sphPar;
	}
	
	// Step 2 - Read the initial Datum List - Put in a static map
	k = 0;
	for(i = 0; i < NUMDATUM; ++i)
	{
		std::string name = datumList[k++];
			
		std::string sphname = datumList[k++];

		double dx = atof(datumList [k++]);
		double dy = atof(datumList [k++]);
		double dz = atof(datumList [k++]);
		int epsgCode = atoi(datumList [k++]);

		SphMap::iterator it = sphMap.find (sphname);
		if(it == sphMap.end())
			throw TeException (DATUM_ERROR);
		else
			sphPar = (*it).second;

		TeDatum d(name, sphPar.rd, sphPar.flt, dx, dy, dz);
		d.epsgCode_ = epsgCode;
		_datumMap[name] = d;
	}
}

TeDatum TeDatumFactory::makeFromProj4(const string& proj4)
{
	unsigned int pos;
    string attribute;
    string value;
	double dx=0, dy=0, dz=0;

	string elpsName="";

	TeDatum dat;
	dat.name("UserDefined");
	for(unsigned int i = 0; i < proj4.size(); i++)
    {
        for( pos = i; i != proj4.size() && proj4[i] != '=';i++);

        attribute = proj4.substr(pos+1, i-pos-1);

        for(pos = i; i != proj4.size() && proj4[i] != ' ';i++);

        if(proj4[i] == ' ') value = proj4.substr(pos+1, i-pos-1);
        else                value = proj4.substr(pos+1, i-pos);
		
		if (attribute == "datum")
			return TeDatumFactory::make(value);
				
		double b;
		// Datum parameters
        if(attribute == "R")	// radius for a spherical ellipsoid
		{
			dat = TeDatum("Spherical",atof(value.c_str()));
			return dat;
		}
		else if(attribute == "a")   // Earth equatorial radius
			dat.radius(atof(value.c_str()));
		else if (attribute == "b")
			b = atof(value.c_str());
	    else if(attribute == "f")	// Earth flattening
			dat.flattening(atof(value.c_str()));
	    else if(attribute == "rf")	// Earth  reverse flattening (f = 1/rf)
			dat.flattening(1/atof(value.c_str()));
		else if (attribute == "es") // eccentricity squared (f=1-(1-es)^0.5) 
			dat.flattening(1-sqrt(1-atof(value.c_str())));
		else if (attribute == "e")	// eccentricity (f=1-(1-es^2)^0.5)
			dat.flattening(1-sqrt(1-pow(atof(value.c_str()),2)));
		else if (attribute == "ellps")
			elpsName = value;
		else if (attribute == "towgs84")
		{
			vector<string> deltas;
			TeSplitString(value, ",", deltas);
			
			if (deltas.size()>=3)
			{
				dx = atof(deltas[0].c_str());
				dy = atof(deltas[1].c_str());
				dz = atof(deltas[2].c_str());
			}
		}
	}
	
	if ( (dx!=0.0 && dy!= 0 && dz != 0.0) && elpsName == "aust_SA")
		return TeDatumFactory::make("SAD69");
		
	if ( (dx==0.0 && dy== 0 && dz == 0.0) && elpsName == "GRS80")
		return TeDatumFactory::make("SIRGAS2000");
	
	dat.name(elpsName);
	dat.dx_ = dx;
	dat.dy_ = dy;
	dat.dz_ = dz;
    return dat;
}

TeDatumMap& TeDatumFactory::getDatumMap()
{
	loadDatumMap();
	return _datumMap;
}

bool findDatum(double semiMajor, double flatenning, TeDatum& datum)
{
	TeDatumMap& datumMap = TeDatumFactory::getDatumMap();

	TeDatumMap::iterator it = datumMap.begin();
	while(it != datumMap.end())
	{
		TeDatum dt = (*it).second;
		if(TeCompareDouble(dt.radius(), semiMajor, 6) &&
		   TeCompareDouble(dt.flattening(), flatenning, 6))
		{
			datum = dt;
			return true;
		}
		++it;
	}
	return false;
}
