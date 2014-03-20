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
/*! \file TeTin.h
    \brief This file provides support for a TIN structure
*/
#ifndef  __TERRALIB_INTERNAL_TIN_H
#define  __TERRALIB_INTERNAL_TIN_H

#include "TeGeometry.h"
#include "TeLayer.h"
#include "TeQuerier.h"
#include "TeDatabase.h"
#include "TeKdTree.h"
#include "TeRTree.h"

#include <list>
#include <set>

typedef TeSAM::TeKdTreeNode<TeCoord2D, unsigned int, unsigned int> TimKdTreeNode;

/** @} */ 
 
/** @defgroup MathConst Mathematical constants and macro definitions for use in Tin
  @{
 */
const double	TeTinBIGFLOAT = TeMAXFLOAT / 10.;		//!< Near Maximum float value - TeMAXFLOAT / 10.

//!  Tin Vertex types 
//enum TeTinVertexType 
//{ DELETEDVERTEX = -1, NORMALVERTEX = 0, ONCONTOURVERTEX = 1, ONBREAKLINEVERTEX = 0x10 };

enum TeTinVertexType 
{ DELETEDVERTEX = -1, NORMALVERTEX = 0, ONCONTOURVERTEX = 1, ONBOXVERTEX = 2, ONBREAKLINEVERTEX = 3,
  CONTOURFIRSTVERTEX = 4, CONTOURLASTVERTEX = 5, BREAKLINEFIRSTVERTEX = 6, BREAKLINELASTVERTEX = 7};
//{ DELETEDVERTEX = -1, NORMALVERTEX = 0, ONCONTOURVERTEX = 1, ONBOXVERTEX = 2, ONBREAKLINEVERTEX = 0x10 };

//!  A basic class for handling vertex of TIN
/*!
	\sa TeGeometry TeMeasure
*/
class TL_DLL TeTinVertex : public TeGeomSingle<TeCoord2D>, public TeMeasure
{
public:

	//! Empty constructor
	TeTinVertex() : TeMeasure ( TeMAXFLOAT ), oneEdge_ (-1) {}

	//! Constructor with parameters
	TeTinVertex( TeCoord2D xy, double value, TeTinVertexType type,
		long edge ) :
		TeGeomSingle<TeCoord2D> (xy), TeMeasure ( value ),
		 type_ (type), oneEdge_ ( edge )
		{ if ( ! xy.tooBig() ) TeGeometry::setBox(TeBox(xy,xy)); }

// -- Methods
//
	//! Returns TRUE if a TeTinVertex is equal to other
	bool operator== (const TeTinVertex& other) const
	{
		if((value_ == other.value_) && (type_ == other.type_) )
		{
			TeCoord2D p1 = location();
			TeCoord2D p2 = other.location();
			return p1==p2;
		}
		return false;
	}

	long oneEdge () 
	{ return oneEdge_; }

	void setOneEdge (long oneEdge) 
	{ oneEdge_ = oneEdge; }

	TeTinVertexType type () 
	{ return type_; }

	void setType (TeTinVertexType type) 
	{ type_ = type; }

	void value (double value) 
	{ value_ = value; }

	

	double value () 
	{ return value_; }

	
	TeGeomRep elemType() { return TeSAMPLES; }

private:

	TeTinVertexType type_;
	long         oneEdge_;
	
};

//!  A basic class for handling a std::set of TeTinVertex
class TL_DLL TeTinVertexSet : public TeGeomComposite <TeTinVertex>
{
public:

// -- Constructors

	TeTinVertexSet() {}

// -- Methods

	bool loadFromFile (std::string &fileName);

	TeGeomRep elemType() { return TeSAMPLES; }
};

//! Tin Edges type 
enum TeTinEdgeType 
{ DELETEDEDGE = -1, NORMALEDGE = 0, ONCONTOUREDGE = 1, ONBREAKLINEEDGE = 2 };

//!  A basic class for handling edges of a  TIN
/*!
	\sa TeGeometry
*/
class TL_DLL TeTinEdge : public TeGeometry
{
public:

// -- Constructors

	TeTinEdge() {}
	TeTinEdge(long from, long to, long left, long right,
		TeTinEdgeType type) :
	from_ (from), to_ (to), left_ (left), right_ (right), type_ (type)
	{}

// -- Methods

	//! Returns TRUE if a TeTinEdge is equal to other
	bool operator== (const TeTinEdge& other) const
	{
		if( (to_ == other.to_) && (from_ == other.from_) &&
			(left_ == other.left_) && (right_ == other.right_) &&
			(type_ == other.type_) )
			return true;

		return false;
	}

	long from () 
	{ return from_; }

	long to () 
	{ return to_; }

	void setVertices (long from, long to)
	{ 
		from_ = from;
		to_ = to;
	}

	void swapVertices ()
	{ 
		long aux = from_;
		from_ = to_;
		to_ = aux;
	}

	long left () 
	{ return left_; }

	long right ()
	{ return right_; }

	void setTriangles (long left, long right)
	{ 
		left_ = left;
		right_ = right;
	}

	void swapTriangles ()
	{ 
		long aux = left_;
		left_ = right_;
		right_ = aux;
	}
	
	void exchangeTriangle ( long t1, long t2 )
	{
		if (left_ == t1 )
			left_ = t2;
		else
			right_ = t2;
	}

	void exchangeVertex ( long v1, long v2 )
	{
		if (from_ == v1 )
			from_ = v2;
		else
			to_ = v2;
	}

	TeTinEdgeType type () 
	{ return type_; }

	void setType (TeTinEdgeType type) 
	{ type_ = type; }

	TeGeomRep elemType() { return TeSAMPLES; }

private:

	long	from_, to_;
	long	left_, right_;
	TeTinEdgeType type_;
};

//!  A basic class for handling a std::set of TeTinEdge
class TL_DLL TeTinEdgeSet : public TeGeomComposite<TeTinEdge>
{
public:

// -- Constructors

	TeTinEdgeSet()	{}

// -- Methods

	bool loadFromFile (std::string &fileName);

	TeGeomRep elemType() { return TeSAMPLES; }
};

//! TeTinTriangle: A class for handling a TIN triangle
class TL_DLL TeTinTriangle : public TeGeometry
{
public:

// -- Constructors

	TeTinTriangle()
	{
		triEdges_[0] = -1;
		triEdges_[1] = -1;
		triEdges_[2] = -1;
	}

	TeTinTriangle(long first, long second, long third) 
	{
		triEdges_[0] = first;
		triEdges_[1] = second;
		triEdges_[2] = third;
	}

// -- Methods

	//! Returns TRUE if a TeTinTriangle is equal to other
	bool operator== (const TeTinTriangle& other) const
	{
		if( (triEdges_[0] == other.triEdges_[0]) &&
			(triEdges_[1] == other.triEdges_[1]) &&
			(triEdges_[2] == other.triEdges_[2]) )
			return true;

		return false;
	}

	void setEdges (long first, long second, long third)
	{
		triEdges_[0] = first;
		triEdges_[1] = second;
		triEdges_[2] = third;
	}

	//! Return triangle edges ids
	/*!
      \param first  first edge
      \param second second edge
      \param third  third edge
	 */
	void edges (long& first, long& second, long& third)
	{
		first  = triEdges_[0];
		second = triEdges_[1];
		third  = triEdges_[2];
	}

	long edgeAt (short num)
	{
		if ((num > -1) && (num < 3))
			return triEdges_[num];
		else
			return -1;
	}

	TeGeomRep elemType() { return TeSAMPLES; }

private:

	long	triEdges_[3];
};

//! TeTinTriangle: A class for handling a std::set of TIN triangles
class TL_DLL TeTinTriangleSet : public TeGeomComposite<TeTinTriangle>
{
public:

// -- Constructors

	TeTinTriangleSet()	{}

// -- Methods

	bool loadFromFile (std::string &fileName);

	TeGeomRep elemType() { return TeSAMPLES; }
};

//! A class to handle a TIN
class TL_DLL TeTin
{
public:

// -- Constructors

	TeTin()
	{
		minedgesize_ = 0;
		fbvertex_ = 0;
		breakline_ = false;
		indexTriang_ = new TeSAM::TeRTree<long>(TeBox());
	}
	TeTin(TeBox & /* box */) 
	{
		minedgesize_ = 0;
		fbvertex_ = 0;
		breakline_ = false;
		indexTriang_ = new TeSAM::TeRTree<long>(TeBox());
	}

// -- Methods

	void clear()
	{ vertexSet_.clear(); triangleSet_.clear(); edgeSet_.clear();}

	TeBox box()
	{ return vertexSet_.box(); }

	bool createInitialTriangles(TeBox &box);
	bool insertLineSamples ( TeContourLine& line );
	bool loadFromFile(std::string &fileName);
	bool getLines(TeLineSet &lineSet);
	bool isEmpty ()
	{ return ( vertexSet_.empty() && triangleSet_.empty() &&
	           edgeSet_.empty() ); } 

	TeTinEdge& edge (long i)
	{ return edgeSet_[i]; }

	TeTinVertex& vertex (long i)
	{ return vertexSet_[i];	}

	TeTinTriangle& triangle (long i)
	{ return triangleSet_[i];	}

	typedef TeTinVertexSet::iterator TeTinVertexIterator;

	TeTinVertexIterator vertexBegin()
	{ return vertexSet_.begin(); }

	TeTinVertexIterator vertexEnd()
	{ return vertexSet_.end(); }

	typedef  TeTinEdgeSet::iterator TeTinEdgeIterator;

	TeTinEdgeIterator edgeBegin()
	{ return edgeSet_.begin(); }

	TeTinEdgeIterator edgeEnd()
	{ return edgeSet_.end(); }

	typedef  TeTinTriangleSet::iterator TeTinTriangleIterator;

	TeTinTriangleIterator triangleBegin()
	{ return triangleSet_.begin(); }

	TeTinTriangleIterator triangleEnd()
	{ return triangleSet_.end(); }

	bool triangleContainsPoint(TeTinTriangle& t, TeCoord2D& pt);
	bool triangleContainsPoint(long t, TeCoord2D& pt)
	{ return triangleContainsPoint(triangleSet_[t], pt); }

	long edgeWithVertexDiffFromEdge(long t, long v, long e);
	long oneTriangleWithVertex(long vertex);
	long oneEdgeWithVertex(long vertex);

	long edgeOppVertex(TeTinTriangle& triangle, long vertex);
	long edgeOppVertex(long triangle, long vertex)
	{ return edgeOppVertex(triangleSet_[triangle], vertex); }

	long vertexOppEdge(long triangle, long edge);
	bool vertexEdges(long vertex, std::list<long> &edges);
	bool vertexOppEdges(long vertex, std::list<long> &edges);
	bool vertexOppEdges(long v, std::list<long> &triset, std::list<long> &edges);

	bool vertexOppVertices(long vertex, std::list<long> &vertices);
	bool vertexSecondNeighVertices(long v, list<long> &vertices);
	bool vertexTriangles(long vertex, std::list<long> &triangles);
	bool triangle3Neighbors(long t, long& neighbor0, long& neighbor1, long& neighbor2);
	long triangleAt (TeCoord2D& pt);
	long triangleAt (TeBox boxPt);

	bool triangleVertices(TeTinTriangle& t, long& vertex0, long& vertex1, long& vertex2);
	bool triangleVertices(long t, long& vertex0, long& vertex1, long& vertex2)
	{ return this->triangleVertices(triangleSet_[t], vertex0, vertex1, vertex2); }

	long triangleVertexAt(long t, long vertex);

	bool triangle3DPoints(TeTinTriangle& t, TeCoord2D& pt0, TeCoord2D& pt1,
		TeCoord2D& pt2, double& val0, double& val1, double& val2);
	bool triangle3DPoints(long t, TeCoord2D& pt0, TeCoord2D& pt1, TeCoord2D& pt2,
		double& val0, double& val1, double& val2)
	{ return triangle3DPoints( triangleSet_[t], pt0, pt1, pt2, val0, val1, val2); }


	bool trianglePoints(TeTinTriangle& t, TeCoord2D& pt0, TeCoord2D& pt1, TeCoord2D& pt2);
	bool trianglePoints(long t, TeCoord2D& pt0, TeCoord2D& pt1, TeCoord2D& pt2)
	{ return trianglePoints( triangleSet_[t], pt0, pt1, pt2); }
	
	bool twoNewTriangles(long t, long v, std::set<long>& triangles);
	long duplicateTriangle(long t, long n, long v, std::set<long>& triangles);
	bool dupNeighTriangle (long t, long e, long v, std::set<long>& triangles);

	void testDelaunayForVertex(long v, std::set<long>& triangles);
	bool testDelaunayAt(long t, long v, std::set<long>& triangles);

	bool testDelaunay(long e);
	bool testDelaunay(long t, long v);
	bool testDelaunay(long t1, long t2, long t3);
	bool testDelaunay(long t1, long t2, long t3, long t4);

	bool createDelaunay();
	bool generateDelaunay(long nt, long ntbase, long& contr);

	void vertexExchange(long oldVertex, long newVertex);
	//! Method used to read the identification number of the opposite vertex of a given edge
	long oppositeVertex(long t, long e);

	//! Creates a triangulation using the Smaller Angle method
	bool createSmallerAngleTriangulation();
	//! Test the angle between two normals
	/*!
	  \param t	triangle identification
	  \param v	neighbor triangle
	  */
	bool testSmallerAngleAt(long t, long v);

	//! Test isolines
	bool createIsolinesTriangulation();

	bool swapEdges( long t, long tv, long ai );

	bool insertPoint(TeSample& xyz)
	{ return insertPoint ( xyz.location().x(),xyz.location().y(),xyz.value() ); }

	bool insertPoint(double x, double y, double value);

	bool edgesInterBySegment( TeCoord2D& pf, TeCoord2D& pn, std::list<long> &aiset, std::list<long> &viset );
	bool insertSegment( TeCoord2D& pf, TeCoord2D& pn );


	void convexize ();
	bool isEdgeTriangle (long t);

	int sizeTriangles() 
	{return triangleSet_.size();}

	int sizeVertex()
	{return vertexSet_.size();}

	TeGeomRep elemType() { return TeSAMPLES; }

	//! Creates TIN
	/*!
	  \param sampleSet	SampleSet
	  \param contourSet ContourLineSet
	  */
	bool createTIN(TeSampleSet& sampleSet, TeContourLineSet& contourSet);

	//! Creates TIN
	/*!
	  \param inputLayer_ptr layer
	  \param ps				PolygonSet
	  */
	bool createTIN(TeLayer* inputLayer_ptr, TePolygonSet& ps);

	//! Creates TIN from point's layer
	/*!
	  \param inputLayer_ptr layer
	  */
	bool createTIN(TeLayer* inputLayer_ptr);

	//! Creates a attributes table
	/*!
      \param attTable	attributes table
	  \param db			current database
	  \param layer		layer
	  */
    bool createAttrTable(TeTable& attTable, TeDatabase* db, TeLayer* layer);

	//! Creates polygons with points of triangles
	/*!
      \param ps			PolygonSet
      \param attTable	attributes table
      \param t			triangle's number
	  \param pt0		triangle's point 0
	  \param pt1		triangle's point 1
	  \param pt2		triangle's point 2
	  \param val0		point's 0 value
	  \param val1		point's 1 value
	  \param val2		point's 2 value
	 */
    bool createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeCoord2D& pt0,
		               TeCoord2D& pt1, TeCoord2D& pt2, double& val0, double& val1,
					   double& val2);

	//! Creates polygons with points of triangles
	/*!
      \param ps			PolygonSet
      \param attTable	attributes table
      \param t			triangle's number
	  \param pt0		triangle's point 0
	  \param pt1		triangle's point 1
	  \param pt2		triangle's point 2
	  \param val0		point's 0 value
	  \param val1		point's 1 value
	  \param val2		point's 2 value
	  \param slope		slope's value of triangle
	 */
    bool createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeCoord2D& pt0,
		               TeCoord2D& pt1, TeCoord2D& pt2, double& val0, double& val1,
					   double& val2, double& slope, double& aspect);

	bool createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeTinVertex vert0,
		               TeTinVertex vert1, TeTinVertex vert2, double& slope, double& aspect);

    //! Add polygons in the layer and save attributes table in the database
	/*!
      \param db			current database
	  \param layer		layer
	  \param ps			PolygonSet
	  \param attTable	attributes table
	  */
    bool saveTIN(TeDatabase* db, TeLayer* layer, TePolygonSet& ps,
		         TeTable& attTable);

    //! Add points in the layer and save attributes table in the database
	/*!
      \param db			current database
	  \param layer		layer
	  \param ps			PointSet
	  \param attTable	attributes table
	  */
	bool saveVertexTIN(TeDatabase* db, TeLayer* layer, TePointSet& ps,
			   TeTable& attTable);

	//! Saves TIN in the database
	/*!
      \param db				current database
	  \param outLayerName	layer name that will be created
	  \param tablename		attributes table name that will be created
	  */
	bool saveTINdb(TeDatabase* db, string& outLayerName, string& tablename);

	//! Saves TIN in the database
	/*!
      \param db				current database
	  \param outLayerName	layer name that will be created
	  \param tablename		attributes table name that will be created
	  */
	TeLayer* saveTINdb(TeDatabase* db, TeProjection* proj, string& outLayerName, string& tablename);

	//! Saves vertex of TIN in the database
	/*!
      \param db				current database
	  \param outLayerName	layer name that will be created
	  \param tablename		attributes table name that will be created
	  */
	bool saveVertexTINdb(TeDatabase* db, string& outLayerName, string& tablename);
	
	//!  Deletes vertex ONBOXVERTEX and re-triangular TIN
	bool borderUp();

	//! Saves TIN in SPRING file format
	/*!
	  \param name File's name
	  */
	void writeTriangleEdgesSPRFile (std::string& name, TeRaster* raster);

	//! Calculates the area of triangle
	/*!
	  \param t		TeTinTriangle object
	  \param area	area of triangle
	  */
	void areaTriangle (TeTinTriangle& t, float& area); //necessita testar o metodo de calculo de area do TeGeometryAlgorithms

	//! Calculates the slope of triangle
	/*!
	  \param t			TeTinTriangle object
	  \param slope		slope of triangle
	  \param slopetype	type of slope (g = degres or r = radian)  
	  */
	void slopeTriangle (TeTinTriangle& t, double& slope, char slopetype); 

	//! Calculates the aspect of triangle
	/*!
	  \param t		TeTinTriangle object
	  \param aspect aspect of triangle
	  */
	void aspectTriangle (TeTinTriangle& t, double& aspect);

	//! Method that calculates gradient values for a TIN
	/*!
	  \param grid		pointer to output grid
	  \param gradtype	type of gradient (slope or aspect)
	  \param slopetype	type os slope (degree or percent)
	  */
	bool calculateGradient(TeRaster*& grid, char gradtype, char slopetype);

	//! Creates a PointSet object with vertex of TIN
	/*!
	  \param ps			PointSet object
	  \param atttable	attributes table
	  \param layer		layer
	  \param db			current database
	  */
	bool createPointSet(TePointSet& ps, TeTable& atttable, TeLayer* layer, TeDatabase* db);

	//! Creates a sample object with vertex of TIN
	/*!
	  \param sample SampleSet object
	  */
	bool createSampleSet(TeSampleSet& sample);
	
	//! Calculates the gradient of triangle
	/*!
	  \param t			TeTinTriangle object
	  \param gradtype	type of gradient (s = slope or a = aspect)
	  \param slopetype	type of slope (g = degres or r = radian)
	  */
	double triangleGradient(TeTinTriangle& t, char gradtype, char slopetype);

	//! Calculates the normal
	/*!
	  \param t			TeTinTriangle object
	  \param nvector	normal vector
	  */
	bool normalTriangle(TeTinTriangle& t, std::vector<double>& nvector); // calculo da normal (adaptado do SPRING)
	bool normalTriangle(std::vector<TeTinVertex> vert, double* nvector);

	//! Function that normalize a vector by its size
	/*!
	  \param nvector	pointer to the vector coordinates x, y and z
	  */
	void normalizeVector(std::vector<double>& nvector);
	void normalizeVector(double* nvector);

	//! Calculates the lines and the columns intercepted by a triangle (Used to image)
	/*!
	  \param raster	pointer to a raster
	  \param t		current triangle
	  \param fline	first line of the raster
	  \param lline	last line of the raster
	  \param fcol	first column of the raster
	  \param lcol	last column of the raster
	  */
	bool defineInterLinesColumns(TeRaster* raster, TeTinTriangle t, int& fline, int& lline, int& fcol, int& lcol);

	//! Fills the grid locations, inside a triangle, with a zvalue linearly evaluated (Used to image)
	/*!
	  \param raster	pointer to a raster
	  \param band	current band
	  \param t		current triangle
	  \param fline	first line of the raster
	  \param lline	last line of the raster
	  \param fcol	first column of the raster
	  \param lcol	last column of the raster
	  */
	void linearInterpolation(TeRaster* raster, int band, TeTinTriangle t, int fline, int lline, int fcol, int lcol);

	//! Fills the grid locations, inside triangles, with a zvalue linearly evaluated (Used to grid)
	/*!
	  \param grid	pointer to output grid
	  */
	bool linearInterpolation(TeRaster*& grid);

	/*! Fills the grid locations with a zvalue evaluated by a quintic polynomium. (Used to grid)
	  \param grid			pointer to output grid
	  \param interpolator	interpolator
	  */
	//Testado apenas para grade.
	bool quinticInterpolation(TeRaster*& grid, int interpolator);

	//! This function interpolates the average weighted by the inverse of the distance
	/*!
	  \param pt point coordinates
	  \param z	point's value
	  */
	bool interpolationPoint(TeCoord2D& pt, double& z);

	//! Calculates the slope and orientation on the point
	/*!
	  \param pt	point coordinates
	  \param s	slope on the point
	  \param a	orientation on the point
	  */
	bool slopeaspectPoint(TeCoord2D& pt,double& s,double& a);

	bool vertexOncontour(long& v);

	//! Set triangulation tolerances
	/*!
	  \param minedgesize	triangulation edges minimum size
	  \param isoTol			isolines tolerance
	  \param ptsdist		distance of isolines points
	  \param breaklineTol	breaklines tolerance
	  */
	void setTolerances(double minedgesize = 0, double isoTol = -1, double ptsdist = -1, double breaklineTol = -1);

	//! Calculates the distance between a point and a segment 
	/*!
	  \param fseg	segment first point
	  \param lseg	segment last point
	  \param pt		considered point
	  \param pti	intersection point
	  \return the distance between the point and the segment
	  */
	double pointToSegmentDistance(TeCoord2D fseg, TeCoord2D lseg, TeCoord2D pt, TeCoord2D& pti);

	//! Insert breaklines in the triangulation
	/*!
	  \param breaklineSet	break lines
	  */
	bool insertBreaklines(TeLineSet& breaklineSet, double scale);

	//! Method that perform a copy of breaklines
	/*!
	  \param breaklines	break lines
	  */
	void copyBreaklines(TeLineSet& breaklines);

	//! Insert break vertex in the triangulation
	/*!
	  \param breaklines	break lines
	  */
	bool insertBreaklinesPoints(TeLineSet breaklines, double scale);

	//! Method that search the closest points of a specific vertex
	/*!
	  \param v			vertex identification
	  \param clstVert	vector of closest vertex
	  */
	bool vertexClosestPoints(long v, std::vector<long>& clstVert);

	//! Method that searches the break vertex closest points
	/*!
	  \param v			vertex identification
	  \param rclstVert	vector of right vertex
	  \param lclstVert	vector of left vertex
	  */
	bool breakVertexClosestPoints(long v, std::vector<long>& rclstVert, std::vector<long>& lclstVert);

	//! Calculate the first and second derivatives in the vertex of a given triangle
	void vertexDerivatives();

	//! Calculate first derivative on triangles
	void triangleFirstDeriv();

	//! Calculate first derivative on vertex
	void vertexFirstDeriv();

	//! Calculate second derivative on triangles
	void triangleSecondDeriv();

	//! Calculate second derivative on vertex
	void vertexSecondDeriv();

	//! Calculate the first derivative in the vertex of a given break triangle
	void breakVertexFirstDeriv();

	//! Calculate the second derivative in the vertex of a given break triangle
	void breakTriangleSecondDeriv();

	//! Calculates the second derivative at all triangulation break vertex
	void breakVertexSecondDeriv();

	//! Calculates the first derivative in a given vertex
	/*!
	  \param v			vertex identification
	  \param clstVertex	vector of a list of vertex
	  \return deriv		first derivative in x and y directions
	  */
	void calcVertexFirstDeriv(long v, std::vector<long> clstVertex, TeTinVertex& deriv);

	//! Calculates the second derivative in a given vertex
	/*!
	  \param v			vertex identification
	  \param clstVertex	vector of a list of vertex
	  \return deriv		second derivative in x and y direction
	  */
	void calcVertexSecondDeriv(long v, std::vector<long> clstVertex, TeTinVertex& deriv);

	//! Calculates the second derivatives in a vertex of a given triangle
	/*!
	  \param v			vertex identification
	  \param triangles	list of triangles identification
	  \return fderiv	second derivative in x and y direction
	  */
	void calcTriangleSecondDeriv(long v, std::list<long> triangles, TeTinVertexSet& fderiv);

	//! Search the point that intersects two triangles containing points pf and pn
	/*!
	  \param pf		first point
	  \param pn		last point
	  \param pts	list of points
	  */
	bool findInterPoints(TeCoord2D& pf, TeCoord2D& pn, TeTinVertexSet& pts);

	//! Finds the triangle list sharing a given node
	/*!
	  \param v		vertex identification number
	  \param rtri	list with the triangles at right side
	  \param ltri	list with the triangles at left side
	 */
	void vertexTriangles(long v, std::list<long>& rtri, std::list<long>& ltri);

	//! Method that evaluate Z values for pt1 and pt2 using the Akima polynomium fitted in a triangle
	/*!
	  \param t	triangle identification number
	  \param v1
	  \param v2
	  */
	void calcZvalueAkima(long t, TeTinVertex& v1, TeTinVertex& v2);

	void defineAkimaCoeficients(long t, double* coef);

	//! Define the coefficients of the Akima polynomium fitted in a given triangle
	/*!
	  \param t		triangle
	  \param coef	pointer to a double vector containing the polynomium coefficients
	 */
	void defineAkimaCoeficients(long t, TeTinVertexSet vertSet, double* coef);

	//! Method that check the topology in a triangulation
	bool checkTopology();

	//! Method that check the edges in a triangulation
	/*!
	  \param t	triangle
	 */
	void checkEdges(long t);

	//! Order edges
	void orderEdges();

	//! Finds a edge given two vertex identification
	/*!
	  \param v1	first vertex identification number
	  \param v2	second vertex identification number
	  */
	long findEdge(long v1, long v2);

	//! Method that checks if a vertex is on the isoline segment
	bool onIsolineSegment(long e, TeTinVertex& vert);

	//! Method that checks if a point pt is on a segment
	bool onSegment(TeCoord2D pt, TeCoord2D fseg, TeCoord2D lseg, double tol);

	//! Method that veryfies if the Isoline is a segment
	bool isIsolineSegment(long e);

	//! Method that veryfies if the Breakline is a segment
	bool isBreaklineSegment(long e);

	void writeTriangleSPRFile(std::string nameVert, std::string nameEdge, std::string nameTriang);

	//! Load triangles from an input layer with TePOLYGONS representation.
	/*!
	  \param layer	input theme
	  \param db		database
	  */
	bool loadTIN(TeLayer* layer, TeDatabase* db);

	/*! Method that filters the points that are closer considering a given tolerance
	  */
	void point3dListFilter(TeTinVertexSet vertSet, double tol);

	//! Method that performs a least square fitting in a set of points
	/*!
	  \param vectx	point coordinates
	  \param vecty	point coordinates
	  \param np		number of points
	  \param degree	degree of the polynomium used in the fitting
	  \param coef	pointer to the polynomium coefficients
	  */
	bool leastSquareFitting(double* vectx, double* vecty, int np, int degree, double* coef);

	//! Method used to solve a system of linear equation throught the Gauss elimination method.
	/*!
	  \param nvars	number of variables
	  \param neqs		number of equations
	  \param matrix	array with variables
	  */
	bool gaussElimination(int nvars, int neqs, double** matrix);

	//! Method that veryfies if the neighbour is on an Isoline or on a Breakline
	bool isNeighborOnIsoOrBreakline(long t, long nviz);

	//! Method to generate contour from a TIN
	void generateContour(double contour, TeContourLine*& ptline);

	//! Method used to define the intersections between an isoline and a triangle
	bool defineInterTriangle(TeContourLine*& ptline, TeTinVertex* vert);

	//! Method used to define the intersection with an edge of a triangle
	/*!
	  \param ptline	contour line
	  \param fpt	first point of the edge
	  \param spt	second point of the edge
	  */
	bool defineInterEdge(TeContourLine*& ptline, TeTinVertex fpt, TeTinVertex spt);

	long onSameSide(TeCoord2D pt1, TeCoord2D pt2, TeCoord2D fseg, TeCoord2D lseg);

	//! Returns the error message
	string errorMessage ()
	{ return errorMessage_; }

private:

	TeTinVertexSet		vertexSet_;		//! Triangulation vertices vector
	TeTinTriangleSet	triangleSet_;	//! Triangulation triangles vector
	TeTinEdgeSet		edgeSet_;		//! Triangulation edges vector

	TeTinVertexSet		vfderivSet_;	//! Vertex first derivatives vector
	TeTinVertexSet		vsderivSet_;	//! Vertex second derivatives vector
	TeTinVertexSet		tfderivSet_;	//! Triangles first derivatives vector
	TeTinVertexSet		tsderivSet_;	//! Triangles second derivatives vector
	TeTinVertexSet		vbrfderivSet_;	//! Right side nodes first derivatives vector
	TeTinVertexSet		vblfderivSet_;	//! Left side nodes first derivatives vector
	TeTinVertexSet		vbrsderivSet_;	//! Right side nodes second derivatives vector
	TeTinVertexSet		vblsderivSet_;	//! Left side nodes second derivatives vector

	TeTinVertexType		verttype_;		//! Vertex type

	long				fbvertex_;		//! First break vertex number
	bool				breakline_;

	double				isoTol_;		//! Isolines tolerance
	double				ptsdist_;		//! Distance between points of isoline
	double				breaklineTol_;	//! Breaklines tolerance
	double				minedgesize_;	//! Edge minimun size

	TeSAM::TeRTree<long>* indexTriang_;

	std::string			errorMessage_;
};

#endif


