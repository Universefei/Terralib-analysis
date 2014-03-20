#include "TeWKTGeometryDecoder.h"

//TerraLib include files
#include "../kernel/TeGeometry.h"
#include "../kernel/TeException.h"

string getWKTPoint(const TeCoord2D& pt)
{
	string wkt;

	wkt = Te2String(pt.x(), 10) + " " + Te2String(pt.y(), 10);

	return wkt;
}

string getWKTLine(const TeLine2D& line)
{
	string wkt;

	for(unsigned int i = 0; i < line.size(); i++)
	{
		if(i > 0)
			wkt += ",";

		wkt += getWKTPoint(line[i]);
	}

	return wkt;
}

string getOnlyGeometry(const string& wkt)
{
	int posI = wkt.find_first_of("("),
		posF = wkt.find_last_of(")");

	posI += 1;
	string res = wkt.substr(posI, posF - posI);

	return res;
}

string removeInvalidChars(const string& wkt)
{
	string res = wkt;

	int pos = res.find("(");

	if(pos == string::npos)
		pos = res.find(")");

	while(pos != string::npos)
	{
		res.erase(pos, 1);

		pos = res.find("(");

		if(pos == string::npos)
			pos = res.find(")");
	}

	return res;
}

TeCoord2D getCoord(const string& pt)
{
	TeCoord2D p;
	vector<string> ptS;
	TeSplitString(pt, " ", ptS);

	if(ptS.size() == 2)
		p.setXY(atof(ptS[0].c_str()), atof(ptS[1].c_str()));

	return p;
}

TeLine2D getLine(const string& line)
{
	TeLine2D l;
	vector<string> pts;
	vector<string>::iterator it;

	TeSplitString(line, ",", pts);

	for(it = pts.begin(); it != pts.end(); ++it)
		l.add(getCoord(*it));

	return l;
}

void TeWKTGeometryDecoder::encodePolygon(const TePolygon& polygon, string& wktPoly)
{
	if(polygon.size() > 0)
	{
		wktPoly = "POLYGON(";

		for(unsigned int i = 0; i < polygon.size(); i++)
		{
			if(i > 0)
				wktPoly += ",";

			wktPoly += "(" + getWKTLine(polygon[i]) + ")";
		}

		wktPoly += ")";
	}
}

void TeWKTGeometryDecoder::encodeLine(const TeLine2D& line, string& wktLine)
{
	if(line.size() >= 2)
	{
		wktLine = "LINESTRING(";
		wktLine += getWKTLine(line);
		wktLine += ")";
	}
}

void TeWKTGeometryDecoder::encodePoint(const TeCoord2D& point, string& wktPoint)
{
	wktPoint = "POINT(" + getWKTPoint(point) + ")"; 
}

void TeWKTGeometryDecoder::decodePolygon(const string& wktPoly, TePolygon& poly)
{
	string oGeom = getOnlyGeometry(wktPoly);
	vector<string> lines;
	vector<string>::iterator it;

	TeSplitString(oGeom, "),(", lines);

	for(it = lines.begin(); it != lines.end(); ++it)
	{
		string line = removeInvalidChars(*it);
		TeLine2D l = getLine(line);
		poly.add(l);
	}
}

void TeWKTGeometryDecoder::decodeLine(const string& wktLine, TeLine2D& line)
{
	string oGeom = getOnlyGeometry(wktLine);
	line = getLine(oGeom);
}

void TeWKTGeometryDecoder::decodePoint(const string& wktPoint, TeCoord2D& point)
{
	string oGeom = getOnlyGeometry(wktPoint);
	point = getCoord(oGeom);
}

