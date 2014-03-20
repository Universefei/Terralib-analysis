#include "TePGUtils.h"
#include <stdexcept>

using namespace std;

TeCoord2D PgGeomPolygonCoordDecode(char *str, char* *s)
{

	char *cp;	// I use this pointer to get the pointer address
                // that strtod store in the second parameter.
                // It is used to know if the conversion of a string
                // to double ocurried normally

	if(!str)
		throw runtime_error("Invalid Coordinate!");

	// SKIP WHITE SPACES
	while(isspace((unsigned char)*str))
		++str;

	// SKIP THE FIRST '(' an go to number begin
	if(*str != '(')
			throw runtime_error("Invalid Coordinate!");

	++str;

	while(isspace((unsigned char)*str))
		++str;

	double x = strtod(str, &cp);

	if(cp <= str)								// If no conversion or there aren't more characters
		throw runtime_error("Invalid format!");	// Error, because the coord y will not be extracted
												// Otherwise, cp will point to the next caracter, that begin the list conversion of coord y

	// SKIP WHITE SPACES AFTER X COORDINATE AND JUMP ',' AND WHITE SPACES
	while(isspace((unsigned char) *cp))
		++cp;

	if(*cp != ',')
			throw runtime_error("Invalid Coordinate!");

	++cp;

	while(isspace((unsigned char) *cp))
		++cp;

	double y = strtod(cp, &str);
	
	if(str <= cp)
		throw runtime_error("Invalid format!");

	while(isspace((unsigned char) *str))
		++str;

	if(*str != ')')
			throw runtime_error("Invalid Coordinate!");

	++str;

	while(isspace((unsigned char) *str))
		++str;

	if(s != 0)
		*s = str;

	return TeCoord2D(x, y);
}

void Te2PgGeomPolygon(const TeLine2D& l, string& strPolygon)
{
	strPolygon = "(";

	unsigned int nVertex = l.size();
	for(unsigned int i = 0u; i < nVertex; ++i)
	{
		if(i != 0u)
			strPolygon += ",";

		 strPolygon += Te2String(l[i].x_, 15);
		 strPolygon += ",";
		 strPolygon += Te2String(l[i].y_, 15);
	}

	strPolygon += ")";

	return;
}

TeLine2D PgGeomPolygon2Te(char* strPolygon)
{
	char *s = strPolygon;

	while(isspace((unsigned char) *s))
		++s;

	if(*s != LDELIM_PGGEOMPOLYGON)
		throw runtime_error("Invalid line format!");

	++s;

	TeLine2D l;

	while(true)
	{
		TeCoord2D coord = PgGeomPolygonCoordDecode(s, &s);
		l.add(coord);	

		if(*s == PGGEOMPOLYGON_COORDS_DELIM)
			++s;
		else
			if(*s == RDELIM_PGGEOMPOLYGON)
			{
				++s;
				break;
			}
	}

    while(isspace((unsigned char) *s))
		++s;

	if((*s != '\0'))
		throw runtime_error("Invalid line format!");

	return l;
}





/*
 * Codificadores e decodificadores PostGIS
 *
 */

TeCoord2D PGCoord_decode(char *str, char* *s)
{
	char *cp;	// I use this pointer to get the pointer address
                // that strtod store in the second parameter.
                // It is used to know if the conversion of a string
                // to double ocurried normally

	if(!str)
		throw runtime_error("Invalid Coordinate!");

	while(isspace((unsigned char)*str))
		++str;	

	double x = strtod(str, &cp);

	if(cp <= str)								// If no conversion or there aren't more characters
		throw runtime_error("Invalid format!");	// Error, because the coord y will not be extracted
												// Otherwise, cp will point to the next caracter, that begin the list conversion of coord y

	while(isspace((unsigned char) *cp))
		++cp;	

	double y = strtod(cp, &str);
	
	if(str <= cp)
		throw runtime_error("Invalid format!");

	while(isspace((unsigned char) *str))
		++str;	

	if(s != 0)
		*s = str;

	return TeCoord2D(x, y);
}

string PGCoord_encode(const TeCoord2D& c)
{
	string str_p  = Te2String(c.x(), 15);
		   str_p += " ";
		   str_p += Te2String(c.y(), 15);

	return str_p;
}

TePoint PGPoint_decode(char *str)
{
	if(!str)
		throw runtime_error("Invalid point format!");
		
	char *s;

	while(*str != ';')
		 ++str;

	if(*str == ';')
		++str;

	while(isspace((unsigned char) *str))
		 ++str;

	str+=6;	// Jump text "POINT(" and go to number.

	TeCoord2D c = PGCoord_decode(str, &s);

	if(*s != ')')
		throw logic_error("Invalid point format!");

	TePoint p;
	
	p.add(c);

	return p;
}

string PGPoint_encode(const TePoint& p)
{
	string result  = "POINT(";
		   result += PGCoord_encode(p.location());
		   result += ")";

	return result;
}

string PGNode_encode(const TeNode& p)
{
	string result  = "POINT(";
		   result += PGCoord_encode(p.location());
		   result += ")";

	return result;
}

TeLine2D PGLine_decode(char *str, char* *sd)
{
	char *s;

	s = str;

	while(isspace((unsigned char) *s))
		++s;

	if(*s != LDELIM_LINESTRING)
		throw runtime_error("Invalid line format!");

	++s;

	TeLine2D l;

	double	xmin, ymin, xmax, ymax;

	xmin = ymin = +TeMAXFLOAT;
	xmax = ymax = -TeMAXFLOAT;

	while(true)
	{
		TeCoord2D coord = PGCoord_decode(s, &s);
		l.add(coord);

		if(xmin > coord.x())
			xmin = coord.x();

		if(xmax < coord.x())
			xmax = coord.x();
		
		if(ymin > coord.y())
			ymin = coord.y();
		
		if(ymax < coord.y())
			ymax = coord.y();

		if(*s == COORDS_DELIM)
			++s;
		else
			if(*s == RDELIM_LINESTRING)
			{
				++s;
				break;
			}
	}

    while(isspace((unsigned char) *s))
		++s;

	if((*s != '\0') && (*s != RDELIM_LINESTRING) && (*s != LINES_DELIM))
		throw runtime_error("Invalid line format!");

	l.setBox(TeBox(xmin, ymin, xmax, ymax));

	*sd = s;

	return l;
}

TeLine2D PGLinestring_decode(char *str)
{
	if(!str)
		throw runtime_error("Invalid point format!");
		
	char *s;

	s = str;

	while(*s != ';')
		 ++s;

	if(*s == ';')
		++s;

	while(isspace((unsigned char) *s))
		 ++s;

	s += 10;	// Jump text "LINESTRING(" and go to number.

	return PGLine_decode(s, &s);
}

TePolygon PGPolygon_decode(char *str)
{
	if(!str)
		throw runtime_error("Invalid polygon format!");
		
	char *s;

	while(*str != ';')
		++str;

	if(*str == ';')
		++str;

	while(isspace((unsigned char) *str))
		 ++str;

	str += 8;	// Jump text "POLYGON(" and go to first line.


	s = str;

	TePolygon pol;
	while(true)
	{
		TeLine2D l = PGLine_decode(s, &s);
		TeLinearRing r(l);
		pol.add(r);

		if(*s == COORDS_DELIM)
			++s;
		else
			if(*s == RDELIM_LINESTRING)
			{
				++s;
				break;
			}
	}

	while(isspace((unsigned char) *s))
		++s;

	if(*s != '\0')
		throw runtime_error("Invalid polygon format!");

	pol.setBox(pol[0].box());

	return pol;
}

TeBox PGBox_decode(char *str)
{

	if(string(str).empty())
	{
		return TeBox();
	}

	char *s;

	s = str;

	while(isspace((unsigned char) *s))
	++s;

	s+=6;
	 
	TeCoord2D coord1 = PGCoord_decode(s, &s);

	while(*s != ',')
	++s;

	++s;
	 
	//if(*s != COORDS_DELIM)
	//	throw runtime_error("Invalid box format!");

	//++s;

	TeCoord2D coord2 = PGCoord_decode(s, &s);

	//if(*s != RDELIM_BOX)
	//	throw runtime_error("Invalid box format!");
	
	//++s;

	//while(isspace((unsigned char) *s))
	//	 ++s;

	//if(*s != '\0')
	//	 throw runtime_error("Invalid box format!");

	return TeBox(coord1, coord2);
}

string PGBox_encode(const TeBox& b)
{
	string str_b = "BOX3D(";
	       str_b += PGCoord_encode(b.upperRight());
           str_b += ",";
		   str_b += PGCoord_encode(b.lowerLeft());
           str_b += ")";

	return str_b;
}

TeBox PGBoxRtree_decode(char *str)
{
	char *cp;	// I use this pointer to get the pointer address
                // that strtod store in the second parameter.
                // It is used to know if the conversion of a string
                // to double ocurried normally

	if(!str)
		throw runtime_error("Invalid Coordinate!");

	// SKIP WHITE SPACES
	while(isspace((unsigned char)*str))
		++str;

	// SKIP THE FIRST '(' an go to number begin
	if(*str != '(')
			throw runtime_error("Invalid Box!");

	++str;

	while(isspace((unsigned char)*str))
		++str;

	double x2 = strtod(str, &cp);

	if(cp <= str)								// If no conversion or there aren't more characters
		throw runtime_error("Invalid format!");	// Error, because the coord y will not be extracted
												// Otherwise, cp will point to the next caracter, that begin the list conversion of coord y

	// SKIP WHITE SPACES AFTER X1 COORDINATE AND JUMP ',' AND WHITE SPACES
	while(isspace((unsigned char) *cp))
		++cp;

	if(*cp != ',')
			throw runtime_error("Invalid Box!");

	++cp;

	while(isspace((unsigned char) *cp))
		++cp;

	double y2 = strtod(cp, &str);
	
	if(str <= cp)
		throw runtime_error("Invalid format!");

	// SKIP WHITE SPACES AFTER Y1 COORDINATE AND JUMP ',' AND WHITE SPACES
	while(isspace((unsigned char) *str))
		++str;

	if(*str != ')')
		throw runtime_error("Invalid Box!");

	++str;

	while(isspace((unsigned char) *str))
		++str;

	if(*str != ',')
		throw runtime_error("Invalid Box!");

	++str;

	while(isspace((unsigned char) *str))
		++str;

	if(*str != '(')
		throw runtime_error("Invalid Box!");

	++str;

	while(isspace((unsigned char) *str))
		++str;

	double x1 = strtod(str, &cp);
	
	if(cp <= str)
		throw runtime_error("Invalid format!");

	// SKIP WHITE SPACES AFTER x2 COORDINATE AND JUMP ',' AND WHITE SPACES
	while(isspace((unsigned char) *cp))
		++cp;

	if(*cp != ',')
			throw runtime_error("Invalid Box!");

	++cp;

	while(isspace((unsigned char) *cp))
		++cp;

	double y1 = strtod(cp, &str);
	
	if(str <= cp)
		throw runtime_error("Invalid format!");

	while(isspace((unsigned char) *str))
		++str;

	if(*str != ')')
		throw runtime_error("Invalid Box!");

	return TeBox(x1, y1, x2, y2);
}

string PGBoxRtree_encode(const TeBox& b)
{
	string str_b  = "(";
	       str_b += Te2String(b.upperRight().x(), 15);
		   str_b += ", ";
		   str_b += Te2String(b.upperRight().y(), 15);
		   str_b += ", ";
		   str_b += Te2String(b.lowerLeft().x(), 15);
		   str_b += ", ";
		   str_b += Te2String(b.lowerLeft().y(), 15);
		   str_b += ")";	       

	return str_b;
}



void PGConcatLineString(const TeLine2D& l, string& concatString)
{
	register unsigned int i = 0;
	register unsigned int nStep = l.size();

	concatString += "(";
	for(; i < nStep; ++i)
	{
		if(i != 0)
		{
			concatString += ", ";
		}		 

		concatString += PGCoord_encode(l[i]);
	}
	
	concatString += ")";

	return;
}

TeBox PGBoxFromPolygon(char *str)
{
	TePolygon pol = PGPolygon_decode(str);

	return pol.box();
}

string PGMakePolygon(const TeBox& box)
{
	string result  = "POLYGON((";
		   result += Te2String(box.x1(), 15);
	       result += " ";
	       result += Te2String(box.y1(), 15);
		   result += ", ";
		   result += Te2String(box.x1(), 15);
	       result += " ";
	       result += Te2String(box.y2(), 15);
		   result += ", ";
		   result += Te2String(box.x2(), 15);
	       result += " ";
	       result += Te2String(box.y2(), 15);
		   result += ", ";
		   result += Te2String(box.x2(), 15);
	       result += " ";
	       result += Te2String(box.y1(), 15);
		   result += ", ";
		   result += Te2String(box.x1(), 15);
	       result += " ";
	       result += Te2String(box.y1(), 15);
		   result += "))";

	return result;
}


