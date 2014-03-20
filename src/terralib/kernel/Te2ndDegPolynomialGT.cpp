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

#include "Te2ndDegPolynomialGT.h"

#include "TeAgnostic.h"


Te2ndDegPolynomialGT::Te2ndDegPolynomialGT()
{
}


Te2ndDegPolynomialGT::~Te2ndDegPolynomialGT()
{
}


void Te2ndDegPolynomialGT::directMap( const TeGTParams& params, const TeCoord2D& pt1, 
  TeCoord2D& pt2 ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ), 
    "Transformation not defined" );
  
  const double& x = pt1.x();
  const double& y = pt1.y();

  pt2.x( 
      params.direct_parameters_( 0, 0 ) +
    ( params.direct_parameters_( 0, 1 ) * x ) +
    ( params.direct_parameters_( 0, 2 ) * y ) +
    ( params.direct_parameters_( 0, 3 ) * x * y ) +
    ( params.direct_parameters_( 0, 4 ) * x * x ) +
    ( params.direct_parameters_( 0, 5 ) * y * y ) );
  
  pt2.y( 
      params.direct_parameters_( 1, 0 ) +
    ( params.direct_parameters_( 1, 1 ) * x ) +
    ( params.direct_parameters_( 1, 2 ) * y ) +
    ( params.direct_parameters_( 1, 3 ) * x * y ) +
    ( params.direct_parameters_( 1, 4 ) * x * x ) +
    ( params.direct_parameters_( 1, 5 ) * y * y ) );
}


void Te2ndDegPolynomialGT::inverseMap( const TeGTParams& params, const TeCoord2D& pt2, 
  TeCoord2D& pt1 ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ), 
    "Transformation not defined" );
  
  const double& x = pt2.x();
  const double& y = pt2.y();

  pt1.x( 
      params.inverse_parameters_( 0, 0 ) +
    ( params.inverse_parameters_( 0, 1 ) * x ) +
    ( params.inverse_parameters_( 0, 2 ) * y ) +
    ( params.inverse_parameters_( 0, 3 ) * x * y ) +
    ( params.inverse_parameters_( 0, 4 ) * x * x ) +
    ( params.inverse_parameters_( 0, 5 ) * y * y ) );
  
  pt1.y( 
      params.inverse_parameters_( 1, 0 ) +
    ( params.inverse_parameters_( 1, 1 ) * x ) +
    ( params.inverse_parameters_( 1, 2 ) * y ) +
    ( params.inverse_parameters_( 1, 3 ) * x * y ) +
    ( params.inverse_parameters_( 1, 4 ) * x * x ) +
    ( params.inverse_parameters_( 1, 5 ) * y * y ) );
}


unsigned int Te2ndDegPolynomialGT::getMinRequiredTiePoints() const
{
  return 6;
}


bool Te2ndDegPolynomialGT::computeParameters( TeGTParams& params ) const
{
    /* Reference: Remote Sensing - Models and Methods For Image Processing
                  Second Edition
                  Robert A. Schowengerdt
                  Academic Press
    */

  const unsigned int tiepointsSize = params.tiepoints_.size();
  TEAGN_DEBUG_CONDITION( ( tiepointsSize > 5 ),
    "Invalid tie-points size" )
  TEAGN_DEBUG_CONDITION( params.transformation_name_ == "2ndDegPolynomial",
    "Invalid transformation name" )     
    
  // Initiating matrixes
    
  TeMatrix W;
  TEAGN_TRUE_OR_THROW( W.Init( tiepointsSize, 6 ), "Matrix init error" );
  
  TeMatrix WI;
  TEAGN_TRUE_OR_THROW( WI.Init( tiepointsSize, 6 ), "Matrix init error" );  
  
  TeMatrix X;
  TEAGN_TRUE_OR_THROW( X.Init( tiepointsSize, 1 ), "Matrix init error" );
  
  TeMatrix XI;
  TEAGN_TRUE_OR_THROW( XI.Init( tiepointsSize, 1 ), "Matrix init error" );
  
  TeMatrix Y;
  TEAGN_TRUE_OR_THROW( Y.Init( tiepointsSize, 1 ), "Matrix init error" );
  
  TeMatrix YI;
  TEAGN_TRUE_OR_THROW( YI.Init( tiepointsSize, 1 ), "Matrix init error" );
  
  // Creating the equation system parameters
  
  double* linePtr = 0;
    
  for ( unsigned int tpIdx = 0 ; tpIdx < tiepointsSize ; ++tpIdx ) 
  {
    const TeCoordPair& tp = params.tiepoints_[ tpIdx ];
    
    linePtr = W[ tpIdx ];
    linePtr[ 0 ] = 1;
    linePtr[ 1 ] = tp.pt1.x();
    linePtr[ 2 ] = tp.pt1.y();
    linePtr[ 3 ] = tp.pt1.x() * tp.pt1.y();
    linePtr[ 4 ] = tp.pt1.x() * tp.pt1.x();
    linePtr[ 5 ] = tp.pt1.y() * tp.pt1.y();
    
    linePtr = WI[ tpIdx ];
    linePtr[ 0 ] = 1;
    linePtr[ 1 ] = tp.pt2.x();
    linePtr[ 2 ] = tp.pt2.y();
    linePtr[ 3 ] = tp.pt2.x() * tp.pt2.y();
    linePtr[ 4 ] = tp.pt2.x() * tp.pt2.x();
    linePtr[ 5 ] = tp.pt2.y() * tp.pt2.y();    
    
    X( tpIdx, 0 ) = tp.pt2.x();
    
    XI( tpIdx, 0 ) = tp.pt1.x();
    
    Y( tpIdx, 0 ) = tp.pt2.y();
    
    YI( tpIdx, 0 ) = tp.pt1.y();    
  }
  
  // Solving...
  
  TeMatrix PinvW;
  if( ! W.getPinv( PinvW ) ) return false;
  
  TeMatrix PinvWI;
  if( ! WI.getPinv( PinvWI ) ) return false;  
  
  TeMatrix A;
  TEAGN_TRUE_OR_THROW( TeMatrix::getProduct( PinvW, X, A ), "Internal error" );
  TEAGN_DEBUG_CONDITION( A.Nrow() == 6, "Invalid matrix" );
  TEAGN_DEBUG_CONDITION( A.Ncol() == 1, "Invalid matrix" );
    
  TeMatrix AI;
  TEAGN_TRUE_OR_THROW( TeMatrix::getProduct( PinvWI, XI, AI ), "Internal error" );
  TEAGN_DEBUG_CONDITION( AI.Nrow() == 6, "Invalid matrix" );
  TEAGN_DEBUG_CONDITION( AI.Ncol() == 1, "Invalid matrix" );  
        
  TeMatrix B;
  TEAGN_TRUE_OR_THROW( TeMatrix::getProduct( PinvW, Y, B ), "Internal error" );
  TEAGN_DEBUG_CONDITION( B.Nrow() == 6, "Invalid matrix" );
  TEAGN_DEBUG_CONDITION( B.Ncol() == 1, "Invalid matrix" );  
    
  TeMatrix BI;
  TEAGN_TRUE_OR_THROW( TeMatrix::getProduct( PinvWI, YI, BI ), "Internal error" );              
  TEAGN_DEBUG_CONDITION( BI.Nrow() == 6, "Invalid matrix" );
  TEAGN_DEBUG_CONDITION( BI.Ncol() == 1, "Invalid matrix" );  
  
  // Copying the parameters to output
  
  TEAGN_TRUE_OR_THROW( params.direct_parameters_.Init( 2, 6 ), 
    "Matrix init error" );
  params.direct_parameters_( 0, 0 ) = A( 0, 0 );
  params.direct_parameters_( 0, 1 ) = A( 1, 0 );
  params.direct_parameters_( 0, 2 ) = A( 2, 0 );
  params.direct_parameters_( 0, 3 ) = A( 3, 0 );
  params.direct_parameters_( 0, 4 ) = A( 4, 0 );
  params.direct_parameters_( 0, 5 ) = A( 5, 0 );
  params.direct_parameters_( 1, 0 ) = B( 0, 0 );
  params.direct_parameters_( 1, 1 ) = B( 1, 0 );
  params.direct_parameters_( 1, 2 ) = B( 2, 0 );
  params.direct_parameters_( 1, 3 ) = B( 3, 0 );
  params.direct_parameters_( 1, 4 ) = B( 4, 0 );
  params.direct_parameters_( 1, 5 ) = B( 5, 0 );  
  
  TEAGN_TRUE_OR_THROW( params.inverse_parameters_.Init( 2, 6 ), 
    "Matrix init error" );
  params.inverse_parameters_( 0, 0 ) = AI( 0, 0 );
  params.inverse_parameters_( 0, 1 ) = AI( 1, 0 );
  params.inverse_parameters_( 0, 2 ) = AI( 2, 0 );
  params.inverse_parameters_( 0, 3 ) = AI( 3, 0 );
  params.inverse_parameters_( 0, 4 ) = AI( 4, 0 );
  params.inverse_parameters_( 0, 5 ) = AI( 5, 0 );
  params.inverse_parameters_( 1, 0 ) = BI( 0, 0 );
  params.inverse_parameters_( 1, 1 ) = BI( 1, 0 );
  params.inverse_parameters_( 1, 2 ) = BI( 2, 0 );
  params.inverse_parameters_( 1, 3 ) = BI( 3, 0 );
  params.inverse_parameters_( 1, 4 ) = BI( 4, 0 );
  params.inverse_parameters_( 1, 5 ) = BI( 5, 0 );   

  return true;
}


bool Te2ndDegPolynomialGT::isTransDefined( const TeGTParams& params )  const
{
  if( ( params.transformation_name_ == "2ndDegPolynomial" ) &&
    ( params.direct_parameters_.Nrow() == 2 ) &&
    ( params.direct_parameters_.Ncol() == 6 ) &&
    ( params.inverse_parameters_.Nrow() == 2 ) &&
    ( params.inverse_parameters_.Ncol() == 6 ) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

