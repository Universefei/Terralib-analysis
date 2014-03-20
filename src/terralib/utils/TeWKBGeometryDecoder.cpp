#include "TeWKBGeometryDecoder.h"

//TerraLib include files
#include "../kernel/TeGeometry.h"
#include "../kernel/TeException.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

//STL include files
#include <cstring>

bool isLittleEndian()
{
	return !(((unsigned int) 1) == htonl((unsigned int) 1));
}

unsigned int SwapUInt(const unsigned int& uintVal)
{
	char uintIn[4], uintOut[4];

	unsigned int outVal;

	memcpy(uintIn,&uintVal,4);

	uintOut[0] = uintIn[3];
	uintOut[1] = uintIn[2];
	uintOut[2] = uintIn[1];
	uintOut[3] = uintIn[0];

	memcpy (&outVal,uintOut,4);

	return outVal;
}

void encodeRing(const TeLine2D& line, char*& wkbLine, unsigned int& size)
{
	unsigned int nPoints = line.size();
	size = sizeof(unsigned int) + (nPoints*2*sizeof(double));

	wkbLine = new char[size];
	char* l = wkbLine;

	//nPoints
	memcpy(l, &nPoints , sizeof(unsigned int));
	l += sizeof(unsigned int);

	for(unsigned int i = 0; i < nPoints ; ++i)
	{
		double x = line[i].x();
		double y = line[i].y();

		memcpy(l, &x, sizeof(double));
		l += sizeof(double);
		
		memcpy(l, &y, sizeof(double));
		l += sizeof(double);
	}
}

void decodeCoord(const char*& wkb, TeCoord2D& point, const char& byteOrder, unsigned int& readBytes)
{
	union
	{
		double dWord_;
		unsigned int aux_[2];
	} swapx1, swapy1;

	memcpy(&swapx1.dWord_, wkb, sizeof(double));
	memcpy(&swapy1.dWord_, wkb + sizeof(double), sizeof(double));

	readBytes = 2*(sizeof(double));
	wkb += readBytes; // x + y

	// 0 = Big Endian (wkbXDR)
	if((byteOrder == 0) && isLittleEndian())
	{
		union
		{
			double dWord_;
			unsigned int aux_[2];
		} swapx2, swapy2;

		swapx2.aux_[1] = ntohl(swapx1.aux_[0]);
		swapx2.aux_[0] = ntohl(swapx1.aux_[1]);

		swapy2.aux_[1] = ntohl(swapy1.aux_[0]);
		swapy2.aux_[0] = ntohl(swapy1.aux_[1]);	
		
		point.x(swapx2.dWord_);
		point.y(swapy2.dWord_);

		return;
	}
	else if((byteOrder == 0) && !isLittleEndian())
	{
		union
		{
			double dWord_;
			unsigned int aux_[2];
		} swapx2, swapy2;

		swapx2.aux_[1] = SwapUInt(swapx1.aux_[0]);
		swapx2.aux_[0] = SwapUInt(swapx1.aux_[1]);

		swapy2.aux_[1] = SwapUInt(swapy1.aux_[0]);
		swapy2.aux_[0] = SwapUInt(swapy1.aux_[1]);
		
		point.x(swapx2.dWord_);
		point.y(swapy2.dWord_);

		return ;
	}

	point.x(swapx1.dWord_);
	point.y(swapy1.dWord_);
}

void decodeRing(const char*& wkb, TeLine2D& line, const char& byteOrder, const int& lineSize, unsigned int& readBytes)
{
	const char* aux = wkb;
	unsigned int ringReadBytes = 0;
	
	for(int i = 0; i < lineSize; i++)
	{
		TeCoord2D point;
		decodeCoord(aux, point, byteOrder, ringReadBytes);
		line.add(point);
		readBytes += ringReadBytes;
	}
}

void getWKBHeader(const char*& wkb, char& byteOrder, unsigned int& wkbType, unsigned int& numGeometries)
{
	const int byteOrderPlusGeomType = sizeof(unsigned char) + sizeof(unsigned int);	

	memcpy(&byteOrder, wkb, sizeof(unsigned char));		
	memcpy(&wkbType, wkb + sizeof(unsigned char), sizeof(unsigned int));

	const char* aux = wkb;
	aux += byteOrderPlusGeomType;
	
	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	if(byteOrder == 0 && isLittleEndian())
	{
		wkbType = ntohl(wkbType);
	}
	else if(byteOrder == 1 && !isLittleEndian())
	{
		wkbType = SwapUInt(wkbType);
	}	

	numGeometries = 0;

	if(wkbType > 1 && wkbType <= 7)
	{
		memcpy(&numGeometries, aux, sizeof(unsigned int));
		aux += sizeof(unsigned int);

		// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
		if(byteOrder == 0 && isLittleEndian())
		{
			numGeometries = ntohl(numGeometries);	
		}
		else if(byteOrder == 1 && !isLittleEndian())
		{
			numGeometries = SwapUInt(numGeometries);
		}
	}
}

void TeWKBGeometryDecoder::encodePolygon(const TePolygon& polygon, char*& wkbPoly, unsigned int& size)
{
	char byteOrder;
	unsigned int wkbType = 3;//WKBPolygon
	unsigned int nRings = polygon.size();

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	byteOrder = (!isLittleEndian()) ? 0 : 1;
//	if(((unsigned int) 1) == htonl((unsigned int) 1))
//	{
//		byteOrder = 0;
//	}
//	else
//	{
//		byteOrder = 1;
//	}

	//cabecalho
	size = sizeof(char) + (2*sizeof(unsigned int));

	//rings
	for(unsigned int i = 0; i < nRings ; ++i)
	{
		size += sizeof(unsigned int) + (2 * sizeof(double) * polygon[i].size());
	}
	
	wkbPoly = new char[size];
	char* p = wkbPoly;

	//byteOrder	
	memcpy(p, &byteOrder , sizeof(char));
	p += sizeof(char);

	//WKBPolygon
	memcpy(p, &wkbType , sizeof(unsigned int));
	p += sizeof(unsigned int);

	//Rings
	memcpy(p, &nRings , sizeof(unsigned int));
	p += sizeof(unsigned int);

	TePolygon::iterator it;

	for(it = polygon.begin(); it != polygon.end(); ++it)
	{
		char* ring;
		unsigned int rSize;
		
		encodeRing((*it), ring, rSize);

		memcpy(p, ring, rSize);
		p += rSize; 

		delete [] ring;
	}
}

void TeWKBGeometryDecoder::encodeLine(const TeLine2D& line, char*& wkbLine, unsigned int& size)
{
	char byteOrder;
	unsigned int nPoints = line.size();
	unsigned int wkbType = 2;//WKBLineString

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	byteOrder = (!isLittleEndian()) ? 0 : 1;
//	if(((unsigned int) 1) == htonl((unsigned int) 1))
//	{
//		byteOrder = 0;
//	}
//	else
//	{
//		byteOrder = 1;
//	}

	size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int) + (2 * sizeof(double) * nPoints);

	wkbLine = new char[size];
	char* l = wkbLine;

	//byteOrder	
	memcpy(l, &byteOrder , sizeof(char));
	l += sizeof(char);

	//WKBLineString
	memcpy(l, &wkbType , sizeof(unsigned int));
	l += sizeof(unsigned int);

	char* ring;
	unsigned int lSize;
	encodeRing(line, ring, lSize);

	memcpy(l, ring, lSize);
	delete []ring;
}

void TeWKBGeometryDecoder::encodePoint(const TeCoord2D& point, char*& wkbPoint, unsigned int& size)
{
	char byteOrder;
	unsigned int wkbType = 1;//WKBPoint

	// 0 = Big Endian (wkbXDR) e 1 = Little Endian (wkbNDR)
	byteOrder = (!isLittleEndian()) ? 0 : 1;
//	if(((unsigned int) 1) == htonl((unsigned int) 1))
//	{
//		byteOrder = 0;
//	}
//	else
//	{
//		byteOrder = 1;
//	}

	//cabecalho
	size = sizeof(char) + sizeof(int) + (2 * sizeof(double));
	
	char* p = new char[size];
	wkbPoint = p;

	//byteOrder	
	memcpy(p, &byteOrder , sizeof(char));
	p += sizeof(char);

	//WKBPoint
	memcpy(p, &wkbType , sizeof(unsigned int));
	p += sizeof(unsigned int);

	double x = point.x();
	double y = point.y();

	memcpy(p, &x, sizeof(double));
	p += sizeof(double);
	memcpy(p, &y, sizeof(double));
	p += sizeof(double);
}

void TeWKBGeometryDecoder::decodePolygon(const char*& wkbPoly, TePolygon& poly, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;

	getWKBHeader(wkbPoly, byteOrder, wkbType, numGeometries);
	
	if(wkbType != 3)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	const char* aux = wkbPoly;

	//Size of header
	readBytes = sizeof(char) + (2*sizeof(unsigned int));
	aux += readBytes;
	unsigned int polyReadBytes = 0;

	for(unsigned int i = 0; i < numGeometries; i++)
	{
		unsigned int ringSize;
		memcpy(&ringSize, aux, sizeof(unsigned int));

		polyReadBytes += sizeof(unsigned int);
		aux += sizeof(unsigned int);

		TeLinearRing ring;
		unsigned int ringReadBytes = 0;
		decodeRing(aux, ring, byteOrder, ringSize, ringReadBytes);

		polyReadBytes += ringReadBytes;
		aux += ringReadBytes;
		
		poly.add(ring);
	}
	readBytes += polyReadBytes;
}

void TeWKBGeometryDecoder::decodeLine(const char*& wkbLine, TeLine2D& line, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numCoords;

	getWKBHeader(wkbLine, byteOrder, wkbType, numCoords);
	
	if(wkbType != 2)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	//Size of header
	const char* aux = wkbLine;
	readBytes = sizeof(char) + (2*sizeof(unsigned int));
	aux += readBytes;
	
	unsigned int lineReadBytes = 0;
	decodeRing(aux, line, byteOrder, numCoords, lineReadBytes);
	readBytes += lineReadBytes;
}

void TeWKBGeometryDecoder::decodePoint(const char*& wkbPoint, TeCoord2D& point, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;

	getWKBHeader(wkbPoint, byteOrder, wkbType, numGeometries);
	
	if(wkbType != 1)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	//Size of header
	const char* aux = wkbPoint;
	readBytes = sizeof(char) + sizeof(int);
	aux += readBytes;

	unsigned int ptReadBytes = 0;
	decodeCoord(aux, point, byteOrder, ptReadBytes);
	readBytes += ptReadBytes;
}

void TeWKBGeometryDecoder::decodePolygonSet(const char*& wkbMultiPolygon, TePolygonSet& pset, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;

	getWKBHeader(wkbMultiPolygon, byteOrder, wkbType, numGeometries);
	
	if(wkbType != 6)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	const char* aux = wkbMultiPolygon;

	//Size of header
	readBytes = sizeof(char) + (2*sizeof(unsigned int));
	aux += readBytes;

	for(unsigned int i = 0; i < numGeometries; i++)
	{
		unsigned int polyReadBytes = 0;
		TePolygon poly;
		decodePolygon(aux, poly, polyReadBytes);
		readBytes += polyReadBytes;
		aux += polyReadBytes;
		pset.add(poly);
	}
}

void TeWKBGeometryDecoder::decodeLineSet(const char*& wkbMultiLine, TeLineSet& lset, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;

	getWKBHeader(wkbMultiLine, byteOrder, wkbType, numGeometries);
	
	if(wkbType != 5)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	const char* aux = wkbMultiLine;

	//Size of header
	readBytes = sizeof(char) + (2*sizeof(unsigned int));
	aux += readBytes;

	for(unsigned int i = 0; i < numGeometries; i++)
	{
		unsigned int lineReadBytes = 0;
		TeLine2D l;
		decodeLine(aux, l, lineReadBytes);
		readBytes += lineReadBytes;
		aux += lineReadBytes;
		lset.add(l);
	}
}

void TeWKBGeometryDecoder::decodePointSet(const char*& wkbMultiPoint, TePointSet& ptset, unsigned int& readBytes)
{
	char byteOrder;
	unsigned int wkbType;
	unsigned int numGeometries;

	getWKBHeader(wkbMultiPoint, byteOrder, wkbType, numGeometries);
	
	if(wkbType != 4)
		throw TeException(UNKNOWN_ERROR_TYPE, "Binary data doesn't supported!");

	const char* aux = wkbMultiPoint;

	//Size of header
	readBytes = sizeof(char) + (2*sizeof(unsigned int));
	aux += readBytes;

	for(unsigned int i = 0; i < numGeometries; i++)
	{
		unsigned int ptReadBytes = 0;
		TeCoord2D c;
		decodePoint(aux, c, ptReadBytes);
		readBytes += ptReadBytes;
		aux += ptReadBytes;
		TePoint pt(c);
		ptset.add(pt);
	}
}
