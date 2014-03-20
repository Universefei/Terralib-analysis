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
#include "TePolygonSetProperties.h"

#include "TeGeometryAlgorithms.h"
#include "TeGeometry.h"
#include "TeAgnostic.h"
#include "TeRaster.h"
#include "TeSharedPtr.h"
#include "TeMatrix.h"

TePolygonSetProperties::TePolygonSetProperties( const TePolygonSet& polSet )
: polSet_( polSet )
{
}

TePolygonSetProperties::~TePolygonSetProperties()
{
}

double TePolygonSetProperties::getPolygonArea( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return TeGeometryArea( polSet_[ polIndex ] );
}

double TePolygonSetProperties::getPolygonBoxArea( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return TeGeometryArea( TeMakePolygon( polSet_[ polIndex ].box() ) );
}

double TePolygonSetProperties::getPolygonPerimeter( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  double perimeter = 0.0;
  TeLinearRing::iterator ring_it = polSet_[ polIndex ][ 0 ].begin();
  while( ring_it != ( polSet_[ polIndex ][ 0 ].end() - 1 ) )
  {
    perimeter += TeDistance( *ring_it, *(ring_it + 1) );
    ++ring_it;
  }
  
  return perimeter;
}

double TePolygonSetProperties::getPolygonFractalDimension( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  double area = getPolygonArea( polIndex );
  if ( area == 1.0 )
    area = 1.00000001;

  return 2 * log( 0.25 * getPolygonPerimeter( polIndex ) ) 
         / log ( area );
}

double TePolygonSetProperties::getPolygonPerimeterAreaRatio( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return getPolygonPerimeter( polIndex ) 
         / getPolygonArea( polIndex );
}

double TePolygonSetProperties::getPolygonCompacity( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return getPolygonPerimeterAreaRatio( polIndex ) 
         / sqrt( getPolygonArea( polIndex ) );
}

double TePolygonSetProperties::getPolygonShapeIndex( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return getPolygonPerimeter( polIndex ) 
         / ( 4 * sqrt( getPolygonArea( polIndex ) ) );
}

double TePolygonSetProperties::getPolygonDensity( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  return getPolygonArea( polIndex ) 
         / getPolygonRadius( polIndex );
}

double TePolygonSetProperties::getPolygonLength( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  TePolygon rotatedPolygon = rotatePolygon( polIndex, -getPolygonAngle( polIndex ) );
  return rotatedPolygon.box().height();
}

double TePolygonSetProperties::getPolygonWidth( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  TePolygon rotatedPolygon = rotatePolygon( polIndex, -getPolygonAngle( polIndex ) );
  return rotatedPolygon.box().width();
}

double TePolygonSetProperties::getPolygonCircle( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  double r = getPolygonRadius( polIndex );
  return getPolygonArea( polIndex ) 
         / ( TePI * r * r );
}

double TePolygonSetProperties::getPolygonContiguity( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  TeSharedPtr< TeRaster > raster( getRasterInPolygon( polIndex ) );
  if ( raster->params().nlines_ <= 0 || raster->params().nlines_ <= 0 )
    return 0.0;

  TeRaster::iteratorPoly it = raster->begin( polSet_[ polIndex ], TeBoxPixelIn, 0 );
  TeMatrix matrix_contig;
  matrix_contig.Init( 4 + raster->params().nlines_, 4 + raster->params().ncols_, 0.0 );
  while( it != raster->end( polSet_[ polIndex ], TeBoxPixelIn, 0 ) )
  {
    int x = it.currentColumn(),
        y = it.currentLine();
    matrix_contig( y + 2, x + 2 ) = 1.0;
    ++it;
  }

  TeMatrix kernel;
  kernel.Init(3, 3, 1.0);
  kernel(0, 1) = 2.0;  
  kernel(1, 0) = 2.0;  
  kernel(1, 2) = 2.0;  
  kernel(2, 1) = 2.0;
  double local_contiguity;
  double contiguity = 0.0;
  unsigned N = 0;

  for (int i = 1; i < matrix_contig.Nrow() - 1; i++)
    for (int j = 1; j < matrix_contig.Ncol() - 1; j++)
    {
      if ( matrix_contig(i, j) == 1.0 )
        N++;
      local_contiguity = 0.0;
      for (int ii = -1; ii < 2; ii++)
        for (int jj = -1; jj < 2; jj++)
          local_contiguity += matrix_contig(i + ii, j + jj) * kernel(ii + 1, jj + 1);
      contiguity += local_contiguity;
    }

  contiguity /= (double) N;
  contiguity -= 1.0;
  contiguity /= 12.0;

  return contiguity;
}

double TePolygonSetProperties::getPolygonGyrationRadius( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  TeSharedPtr< TeRaster > raster( getRasterInPolygon( polIndex ) );
  TeRaster::iteratorPoly it = raster->begin( polSet_[ polIndex ], TeBoxPixelIn, 0 );
  vector<TeCoord2D> xyPositionsVector;
  while( it != raster->end( polSet_[ polIndex ], TeBoxPixelIn, 0 ) )
  {
    int x = it.currentColumn(),
        y = it.currentLine();
    TeCoord2D xy(x, y);
	  xy = raster->index2Coord( xy );
    xyPositionsVector.push_back(xy);
    ++it;
  }
  TEAGN_DEBUG_CONDITION( xyPositionsVector.size() > 0, "No data" )

  double sum = 0.0;
  TeCoord2D centroid = TeFindCentroid( polSet_[ polIndex ] );
  for ( unsigned i = 0; i < xyPositionsVector.size(); i++ )
    sum += TeDistance( centroid, xyPositionsVector[ i ] );

  return sum / xyPositionsVector.size();
}

double TePolygonSetProperties::getPolygonEllipticFit( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  vector<TeCoord2D> borderCoordinates = getBorderCoordinates( polIndex );
  double a = getPolygonArea( polIndex );
  double b = TeGeometryArea( createEllipse( fitEllipse( borderCoordinates ) ) );
  // is b nan?
  if ( b != b ) 
  {
    a = 1.0;
    b = 0.0;
  }

  return (a > b ? b / a : a / b);
}

double TePolygonSetProperties::getPolygonAngle( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  vector<TeCoord2D> borderCoordinates = getBorderCoordinates( polIndex );
  TePolygon ellipse = createEllipse( fitEllipse( borderCoordinates ) );
  double max_distance = 0.0;
  TeCoord2D p1;
  TeCoord2D p2;
  TeLinearRing::iterator ring_it = ellipse[ 0 ].begin();
  while( ring_it != ( ellipse[ 0 ].end() - 1 ) )
  {
    if ( TeDistance( *ring_it, *(ring_it + 1) ) > max_distance )
    {
      max_distance = TeDistance( *ring_it, *(ring_it + 1) );
      p1 = *ring_it;
      p2 = *(ring_it + 1);
    }
    ++ring_it;
  }
  double angle = atan( ( p2.y() - p1.y() ) / ( p2.x() - p1.x() ) );
  if ( angle < 0.0 )
    angle *= -1.0;
  // is angle nan?
  if ( angle != angle ) 
    angle = 0.0;

  return angle;
}

double TePolygonSetProperties::getPolygonRectangularFit( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  double rectangularFit = getPolygonArea( polIndex ) / 
                          getPolygonBoxArea( polIndex );
  TePolygon rotatedPolygon = rotatePolygon( polIndex, -getPolygonAngle( polIndex ) );
  double rotatedFit = getPolygonArea( polIndex ) /
                      TeGeometryArea( TeMakePolygon( rotatedPolygon.box() ) );
  if (rotatedFit > rectangularFit)
    rectangularFit = rotatedFit;

  return rectangularFit;
}

void TePolygonSetProperties::printProperties( int polIndex )
{
  unsigned start = 0;
  unsigned finish = polSet_.size();
  if ( polIndex != -1 )
  {
    start = polIndex;
    finish = polIndex + 1;
  }
  
  for (unsigned i = start; i < finish; i++)
  {
    cout << "Polygon " << i << endl;
    cout << "  getPolygonArea: " << getPolygonArea(i) << endl;
    cout << "  getPolygonAngle: " << getPolygonAngle(i) * 180 / TePI << endl;
    cout << "  getPolygonBoxArea: " << getPolygonBoxArea(i) << endl;
    cout << "  getPolygonCircle: " << getPolygonCircle(i) << endl;
    cout << "  getPolygonCompacity: " << getPolygonCompacity(i) << endl;
    cout << "  getPolygonContiguity: " << getPolygonContiguity(i) << endl;
    cout << "  getPolygonDensity: " << getPolygonDensity(i) << endl;
    cout << "  getPolygonEllipticFit: " << getPolygonEllipticFit(i) << endl;
    cout << "  getPolygonFractalDimension: " << getPolygonFractalDimension(i) << endl;
    cout << "  getPolygonGyrationRadius: " << getPolygonGyrationRadius(i) << endl;
    cout << "  getPolygonLength: " << getPolygonLength(i) << endl;
    cout << "  getPolygonPerimeter: " << getPolygonPerimeter(i) << endl;
    cout << "  getPolygonPerimeterAreaRatio: " << getPolygonPerimeterAreaRatio(i) << endl;
    cout << "  getPolygonRadius: " << getPolygonRadius(i) << endl;
    cout << "  getPolygonRectangularFit: " << getPolygonRectangularFit(i) << endl;
    cout << "  getPolygonShapeIndex: " << getPolygonShapeIndex(i) << endl;
    cout << "  getPolygonWidth: " << getPolygonWidth(i) << endl;
  }
}

// private functions
double TePolygonSetProperties::getPolygonRadius( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )
  
  double max_radius = -1;
  double radius;
  TeCoord2D centroid = TeFindCentroid( polSet_[ polIndex ] );

  TePolygon::iterator polygon_it = polSet_[ polIndex ].begin();
  while( polygon_it != polSet_[ polIndex ].end() )
  {
    TeLinearRing::iterator ring_it = (*polygon_it).begin();
    while( ring_it != (*polygon_it).end() )
    {
      radius = TeDistance( *ring_it, centroid );
      if ( radius > max_radius )
        max_radius = radius;
      ++ring_it;
    }
    ++polygon_it;
  }

  return max_radius;
}

vector<TeCoord2D> TePolygonSetProperties::getBorderCoordinates( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  vector<TeCoord2D> borderCoordinates;
  TeLinearRing::iterator ring_it = polSet_[ polIndex ][ 0 ].begin();

  TeCoord2D a;
  TeCoord2D b;
  while( ring_it != polSet_[ polIndex ][ 0 ].end() - 1 )
  {
    borderCoordinates.push_back( *ring_it );

    // add intermediate coordinate
    a = *ring_it;
    b = *(ring_it + 1);
    TeCoord2D semiCoordinate( ( a.x() + b.x() ) / 2, ( a.y() + b.y() ) / 2 );
    borderCoordinates.push_back( semiCoordinate );

    ++ring_it;
  }

  return borderCoordinates;
}

double getMean( vector<TeCoord2D> xyPositions, char coord )
{
  if ( xyPositions.size() == 0 )
    return 0.0;

  double sum = 0.0;
  if ( coord == 'X' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      sum += xyPositions[i].x();
  }
  else if ( coord == 'Y' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      sum += xyPositions[i].y();
  }

  return sum / xyPositions.size();
}

double getMax( vector<TeCoord2D> xyPositions, char coord )
{
  double max = 0.0;
  if ( coord == 'X' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      if ( xyPositions[i].x() > max )
        max = xyPositions[i].x();
  } 
  else if ( coord == 'Y' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      if ( xyPositions[i].y() > max )
        max = xyPositions[i].y();
  } 

  return max;
}

double getMin( vector<TeCoord2D> xyPositions, char coord )
{
  double min = 99999999.9;
  if ( coord == 'X' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      if ( xyPositions[i].x() < min )
        min = xyPositions[i].x();
  } 
  else if ( coord == 'Y' )
  {
    for ( unsigned i = 0; i < xyPositions.size(); i++)
      if ( xyPositions[i].y() < min )
        min = xyPositions[i].y();
  } 

  return min;
}

vector<double> TePolygonSetProperties::fitEllipse( vector<TeCoord2D> xyPositions )
{
  double meanX = getMean( xyPositions, 'X' );
  double meanY = getMean( xyPositions, 'Y' );
  double maxX = getMax( xyPositions, 'X' );
  double maxY = getMax( xyPositions, 'Y' );
  double minX = getMin( xyPositions, 'X' );
  double minY = getMin( xyPositions, 'Y' );
  double sx = (maxX - minX) / 2;
  double sy = (maxY - minY) / 2;

  vector<double> x;
  vector<double> y;

  // normalizing data
  for ( unsigned i = 0; i < xyPositions.size(); i++ )
  {
    x.push_back( ( xyPositions[ i ].x() - meanX ) / sx );
    y.push_back( ( xyPositions[ i ].y() - meanY ) / sy );
  }
   
  // creating design matrix
  TeMatrix D;
  TeMatrix Dt;
  D.Init( x.size(), 6, 0.0 );
  for ( unsigned i = 0; i < xyPositions.size(); i++ )
  {
    D( i, 0 ) = x[ i ] * x[ i ];
    D( i, 1 ) = x[ i ] * y[ i ];
    D( i, 2 ) = y[ i ] * y[ i ];
    D( i, 3 ) = x[ i ];
    D( i, 4 ) = y[ i ];
    D( i, 5 ) = 1.0;
  }
  D.Transpose( Dt );

  // creating scatter matrix
  TeMatrix S;
  S = Dt * D;

  // creating constant matrix
  TeMatrix C;
  C.Init( 6, 6, 0.0 );
  C( 2, 0 ) = -2.0;
  C( 1, 1 ) = 1.0;
  C( 0, 2 ) = -2.0;

  TeMatrix iS;
  S.Inverse( iS );

  TeMatrix productiSC;
  productiSC = iS * C;

  // extract eigenvalues and eigenvectors
  double evec[100];
  for ( int i = 0; i < productiSC.Ncol(); i++ )
    evec[ i ] = 0.0;
  findEigenVectors( productiSC, evec );

  vector<double> tempA;
  for ( int i = 0; i < productiSC.Ncol(); i++ )
    tempA.push_back( evec[i] );

  // normalizing resultant values
  vector<double> A(6);
  A[ 0 ] = tempA[ 0 ] * sy * sy;
  A[ 1 ] = tempA[ 1 ] * sx * sy;
  A[ 2 ] = tempA[ 2 ] * sx * sx;
  A[ 3 ] = - 2 * tempA[ 0 ] * sy * sy * meanX 
           - tempA[ 1 ] * sx * sy * meanY 
           + tempA[ 3 ] * sx * sy * sy;
  A[ 4 ] = - tempA[ 1 ] * sx * sy * meanX
           - 2 * tempA[ 2 ] * sx * sx * meanY
           + tempA[ 4 ] * sx * sx * sy;
  A[ 5 ] = tempA[ 0 ] * sy * sy * meanX * meanX
           + tempA[ 1 ] * sx * sy * meanX * meanY
           + tempA[ 2 ] * sx * sx * meanY * meanY
           - tempA[ 3 ] * sx * sy * sy * meanX
           - tempA[ 4 ] * sx * sx * sy * meanY
           + tempA[ 5 ] * sx * sx * sy * sy;

  return A;
}

vector<double> solveEllipse( vector<double> A )
{
  double theta = atan2( A[ 1 ], A[ 0 ] - A[ 2 ] ) / 2;
  double ct = cos( theta );
  double st = sin( theta );
  double ap = A[ 0 ] * ct * ct 
              + A[ 1 ] * ct * st 
              + A[ 2 ] * st * st;
  double cp = A[ 0 ] * st * st
              - A[ 1 ] * ct * st
              + A[ 2 ] * ct * ct;

  TeMatrix T;
  T.Init( 2, 2, 0.0 );
  T( 0, 0 ) = A[ 0 ];
  T( 1, 0 ) = A[ 1 ] / 2;
  T( 0, 1 ) = A[ 1 ] / 2;
  T( 1, 1 ) = A[ 2 ];
  TeMatrix Tt2;
  Tt2 = 2 * T;
  TeMatrix Tt2i;
  Tt2.Inverse( Tt2i );
  TeMatrix productA;
  productA.Init( 2, 1, 0.0 );
  productA( 0, 0 ) = A[ 3 ];
  productA( 1, 0 ) = A[ 4 ];
  TeMatrix t;
  t = -Tt2i * productA;
  double cx = t( 0, 0 );
  double cy = t( 1, 0 );
  TeMatrix tt;
  t.Transpose(tt);
  
  TeMatrix product;
  product = tt * T * t;
  double val = product( 0, 0 );
  double scale = 1 / ( val - A[ 5 ] );

  double r1 = 1 / sqrt( scale * ap );
  double r2 = 1 / sqrt( scale * cp );

  vector<double> v;
  v.push_back( r1 );
  v.push_back( r2 );
  v.push_back( cx );
  v.push_back( cy );
  v.push_back( theta );

  return v;
}

TePolygon TePolygonSetProperties::createEllipse( vector<double> A )
{
  vector<double> v = solveEllipse( A );
  unsigned N = 30;
  double dx = 2 * TePI / N;
  double theta = v[ 4 ];

  TeMatrix R;
  R.Init( 2, 2, 0.0 );    
  R( 0, 0 ) = cos( theta );
  R( 1, 0 ) = sin( theta );
  R( 0, 1 ) = -sin( theta );
  R( 1, 1 ) = cos( theta );

  vector<TeCoord2D> XY;
  TeLine2D polygonLines;
  for ( unsigned i = 0; i < N; i++ )
  {
    double angle = i * dx;
    TeMatrix matrixXY;
    matrixXY.Init( 2, 1, 0.0 );
    matrixXY( 0, 0 ) = v[ 0 ] * cos( angle );
    matrixXY( 1, 0 ) = v[ 1 ] * sin( angle );
    TeMatrix d1;
    d1 = R * matrixXY;
    TeCoord2D tempXY( d1( 0, 0 ) + v[ 2 ], d1( 1, 0 ) + v[ 3 ] );
    polygonLines.add( tempXY );
  }

  polygonLines.add( polygonLines[ 0 ] );
  TeLinearRing polygonRing( polygonLines );
  TePolygon polygon;
  polygon.add( polygonRing );
  
  return polygon;
}

TePolygon TePolygonSetProperties::rotatePolygon( unsigned int polIndex, double angle )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  TePolygon rotatedPolygon;
  TeLinearRing rotatedRing;
  TeCoord2D centroid = TeFindCentroid( polSet_[ polIndex ] );
  unsigned actualCoordinate = 0;
  TeCoord2D firstCoordinate;

  TePolygon::iterator it = polSet_[ polIndex ].begin();
  while( it != polSet_[ polIndex ].end() )
  {
    TeLinearRing::iterator it_ring = (*it).begin();

    while( it_ring != (*it).end() )
    {
      TeCoord2D coord_r( -centroid.x(), -centroid.y() );
      coord_r += *it_ring;
      double r = sqrt( coord_r.x() * coord_r.x() + coord_r.y() * coord_r.y() );
      double theta = atan( coord_r.y() / coord_r.x() );
      if (coord_r.x() < 0)
        theta += TePI;
      TeCoord2D newCoordinate( centroid.x() + r * cos( theta + angle ), 
                               centroid.y() + r * sin( theta + angle ) );

      rotatedRing.add(newCoordinate);
      if (actualCoordinate == 0)
        firstCoordinate = newCoordinate;
      ++actualCoordinate;
      ++it_ring;
    }
    ++it;
  }
  rotatedRing.add( firstCoordinate );
  rotatedPolygon.add( rotatedRing );

  return rotatedPolygon;
}

TeRaster* TePolygonSetProperties::getRasterInPolygon( unsigned int polIndex )
{
  TEAGN_DEBUG_CONDITION( polIndex < polSet_.size(), "Invalid index" )

  double width = polSet_[ polIndex ].box().x2() - polSet_[ polIndex ].box().x1();
  double height = polSet_[ polIndex ].box().y2() - polSet_[ polIndex ].box().y1();
  double resolution_x = width / 9.5;
  double resolution_y = height / 9.5;

  TeRasterParams r_params;
  r_params.nBands( 1 );
  r_params.mode_ = 'c';
  r_params.decoderIdentifier_ = "SMARTMEM";
  r_params.setDataType( TeUNSIGNEDCHAR );
  r_params.boundingBoxResolution( polSet_[ polIndex ].box().x1(), polSet_[ polIndex ].box().y1(), 
        polSet_[ polIndex ].box().x2(), polSet_[ polIndex ].box().y2(), resolution_x, resolution_y );

  TeRaster* raster = new TeRaster( r_params );
  TEAGN_TRUE_OR_THROW ( raster->init( ), "Raster init failed" )

  return raster;
}

bool TePolygonSetProperties::findEigenVectors( TeMatrix input_matrix, double e_vec[] )
{
  double *e_vec_aux	= NULL, 
	     sum = 0.0; 
  int dim = input_matrix.Nrow();
  TeMatrix aux1, aux2, mt;
  int i,j,k;       
	
  if( dim <= 0 )
    return false;
  if( aux1.Init( dim, (double) 1 ) == false )
    return false;
  if( aux2.Init( dim, (double) 1 ) == false )
    return false;
  e_vec_aux = new double[dim];
  if( e_vec_aux == NULL )
    return false;
  for ( k = 0; k < dim; k++ )
  {
    e_vec_aux[ k ] = 0.0;
    e_vec[ k ] = 0.0;
  }
  aux1 = input_matrix;
  aux2 = input_matrix;
  unsigned max_iterations = 0;

  for (;;)
  {
    mt = aux1 * aux2;

    for ( i = 0; i < dim; i++ )
	  {
      for ( j = 0; j < dim; j++ )
        e_vec[ i ] = e_vec[ i ] + mt( i, j );
	    sum = sum + e_vec[i];
	  }

	  for ( j = 0 ; j < dim; j++ )
	    e_vec[ j ] = e_vec[ j ] / sum;

    for ( j = 0; j < dim; j++ )
	  {
	    if ( fabs( e_vec_aux[ j ] - e_vec[ j ] ) < 0.001 ) 
	    {
        delete []e_vec_aux;	
		    return true;
	    }
	    e_vec_aux[ j ] = e_vec[ j ];
	  }

	  aux1 = mt;
	  aux2 = mt;

	  // avoid infinite loop
	  if ( ++max_iterations > 10000 )
	    break;
  }
  delete []e_vec_aux;	
  return true;
}
