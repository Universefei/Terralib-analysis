#include "TePDIFilterMask.hpp"

#include "../kernel/TeAgnostic.h"


TePDIFilterMask::TePDIFilterMask()
{
  reset( 0, 0 );
}


TePDIFilterMask::TePDIFilterMask( unsigned int width, double gain )
{
  reset( width, gain );
}


TePDIFilterMask::~TePDIFilterMask()
{
}


void TePDIFilterMask::reset( unsigned int width, double gain )
{
  TEAGN_TRUE_OR_THROW( ( width % 2 != 0 ),
    "Invalid width suplied, must be an odd number." );
  TEAGN_TRUE_OR_THROW( width > 2,
    "Invalid width suplied, must be greater than 2" );

  weights_gain_ = gain;
  weights_sum_ = 0;
  internal_matrix_.Init( width, width, 0. );
}


unsigned int TePDIFilterMask::columns() const
{
  return (unsigned int) internal_matrix_.Ncol();
}


unsigned int TePDIFilterMask::lines() const
{
  return (unsigned int) internal_matrix_.Nrow();
}


void TePDIFilterMask::set( unsigned int line, unsigned int column,
  double value )
{
  TEAGN_TRUE_OR_THROW( (int)line < internal_matrix_.Nrow(),
    "Invalid line number" );
  TEAGN_TRUE_OR_THROW( (int)column < internal_matrix_.Ncol(),
    "Invalid columnLine number" );

  weights_sum_ -= internal_matrix_( line, column );
  internal_matrix_( line, column ) = value;
  weights_sum_ += value;
}


double TePDIFilterMask::get( unsigned int line, unsigned int column ) const
{
  TEAGN_TRUE_OR_THROW( (int)line < internal_matrix_.Nrow(),
    "Invalid line number" );
  TEAGN_TRUE_OR_THROW( (int)column < internal_matrix_.Ncol(),
    "Invalid columnLine number" );

  return internal_matrix_( line, column ) + weights_gain_;
}

double TePDIFilterMask::getSum()
{
  return weights_sum_;
}


void TePDIFilterMask::operator=( const TePDIFilterMask& external )
{
  internal_matrix_ = external.internal_matrix_;
  weights_sum_ = external.weights_sum_;
  weights_gain_ = external.weights_gain_;
}


void TePDIFilterMask::normalize()
{
  if( weights_sum_ != 1. ) {

    TEAGN_TRUE_OR_THROW( weights_sum_ != 0.,
      "Trying to normalize a zero weights sum mask" );

    for( int line = 0 ; line < internal_matrix_.Nrow() ; ++line ) {
      for( int column = 0 ; column < internal_matrix_.Ncol() ;
           ++column ) {

        internal_matrix_( line, column ) /= weights_sum_;
      }
    }

    weights_sum_ = 1.;
  }
}


double** TePDIFilterMask::getWeightsMatrix() const
{
  TEAGN_TRUE_OR_THROW( internal_matrix_.Nrow() != 0,
    "Cannot create a Weights Matrix from an empty mask." );
  TEAGN_TRUE_OR_THROW( internal_matrix_.Ncol() != 0,
    "Cannot create a Weights Matrix from an empty mask." );

  double** outmatrix = new double*[ internal_matrix_.Nrow() ];
  TEAGN_TRUE_OR_THROW( outmatrix != 0,
    "Memory allocation error" );

  for( int line = 0 ; line < internal_matrix_.Nrow() ; ++line ) {
    outmatrix[ line ] = new double[ internal_matrix_.Ncol() ];

    TEAGN_TRUE_OR_THROW( outmatrix[ line ] != 0,
      "Memory allocation error" );
  }

  for( int linecounter = 0 ; linecounter < internal_matrix_.Nrow() ; ++linecounter ) {
    for( int column = 0 ; column < internal_matrix_.Ncol() ;
      ++column ) {
      outmatrix[ linecounter ][ column ] = get( linecounter, column );
    }
  }

  return outmatrix;
}


void TePDIFilterMask::deleteWeightsMatrix( double** matrix,
  unsigned int lines )
{
  if( matrix != 0 ) {
    for( unsigned int line = 0 ; line < lines ; ++line ) {
      if( matrix[ line ] != 0 ) {
        delete[] matrix[ line ];
      }
    }

    delete[] matrix;
  }
}


bool TePDIFilterMask::isMorfMask()
{
  double value;

  bool zero_matrix = true;

  if( weights_gain_ != 0 ) {
    return false;
  }

  for( int line = 0 ; line < internal_matrix_.Nrow() ; ++line ) {
    for( int column = 0 ; column < internal_matrix_.Ncol() ;
         ++column ) {

      value = internal_matrix_( line, column );

      if( ( value != 1. ) && ( value != 0 ) ) {
        return false;
      }

      if( zero_matrix ) {
        if( value != 0 ) {
          zero_matrix = false;
        }
      }
    }
  }

  if( zero_matrix ) {
    return false;
  }

  return true;
}


TePDIFilterMask::pointer TePDIFilterMask::create_Dummy()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  for( unsigned int line = 0 ; line < 3 ; ++line ) {
    for( unsigned int column = 0 ; column < 3 ; ++column ) {
      outmask->set( line, column, 0 );
    }
  }
  outmask->set( 1, 1, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_Avg3x3( bool no_norm )
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  for( unsigned int line = 0 ; line < 3 ; ++line ) {
    for( unsigned int column = 0 ; column < 3 ; ++column ) {
      outmask->set( line, column, 1. );
    }
  }

  if( ! no_norm ) {
    outmask->normalize();
  }

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_Avg5x5( bool no_norm )
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 5, 0 ) );

  for( unsigned int line = 0 ; line < 5 ; ++line ) {
    for( unsigned int column = 0 ; column < 5 ; ++column ) {
      outmask->set( line, column, 1. );
    }
  }

  if( ! no_norm ) {
    outmask->normalize();
  }

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_Avg7x7( bool no_norm )
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 7, 0 ) );

  for( unsigned int line = 0 ; line < 7 ; ++line ) {
    for( unsigned int column = 0 ; column < 7 ; ++column ) {
      outmask->set( line, column, 1. );
    }
  }

  if( ! no_norm ) {
    outmask->normalize();
  }

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_S()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, -1. );
  outmask->set( 0, 1, -1. );
  outmask->set( 0, 2, -1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_N()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, -1. );
  outmask->set( 2, 1, -1. );
  outmask->set( 2, 2, -1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_E()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, -1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, -1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, -1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_W()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, -1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, -1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, -1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_NW()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, -1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, -1. );
  outmask->set( 2, 2, -1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_SW()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, -1. );
  outmask->set( 0, 2, -1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, -1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_NE()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, -1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, -1. );
  outmask->set( 2, 1, -1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_SE()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, -1. );
  outmask->set( 0, 1, -1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, -1. );
  outmask->set( 1, 1, -2. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_NDLow()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 0 );
  outmask->set( 0, 1, -1. );
  outmask->set( 0, 2, 0 );

  outmask->set( 1, 0, -1. );
  outmask->set( 1, 1, 4. );
  outmask->set( 1, 2, -1. );

  outmask->set( 2, 0, 0 );
  outmask->set( 2, 1, -1. );
  outmask->set( 2, 2, 0 );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_NDMed()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, -1. );
  outmask->set( 0, 1, -1. );
  outmask->set( 0, 2, -1. );

  outmask->set( 1, 0, -1. );
  outmask->set( 1, 1, 8. );
  outmask->set( 1, 2, -1. );

  outmask->set( 2, 0, -1. );
  outmask->set( 2, 1, -1. );
  outmask->set( 2, 2, -1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_NDHigh()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, -2. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, -2. );
  outmask->set( 1, 1, 4. );
  outmask->set( 1, 2, -2. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, -2. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_TMEnh( bool no_norm )
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 5, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, -3. );
  outmask->set( 0, 2, -3. );
  outmask->set( 0, 3, 1. );
  outmask->set( 0, 4, 0 );

  outmask->set( 1, 0, -3. );
  outmask->set( 1, 1, 7. );
  outmask->set( 1, 2, 7. );
  outmask->set( 1, 3, -3. );
  outmask->set( 1, 4, 0 );

  outmask->set( 2, 0, -3. );
  outmask->set( 2, 1, 7. );
  outmask->set( 2, 2, 7. );
  outmask->set( 2, 3, -3. );
  outmask->set( 2, 4, 0 );

  outmask->set( 3, 0, 1. );
  outmask->set( 3, 1, -3. );
  outmask->set( 3, 2, -3. );
  outmask->set( 3, 3, 1. );
  outmask->set( 3, 4, 0 );

  outmask->set( 4, 0, 0 );
  outmask->set( 4, 1, 0 );
  outmask->set( 4, 2, 0 );
  outmask->set( 4, 3, 0 );
  outmask->set( 4, 4, 0 );

  if( ! no_norm ) {
    outmask->normalize();
  }

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfD()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 0 );
  outmask->set( 0, 1, 0 );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, 0 );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 0 );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 0 );
  outmask->set( 2, 2, 0 );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfE()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 0 );
  outmask->set( 0, 2, 0 );

  outmask->set( 1, 0, 0 );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 0 );

  outmask->set( 2, 0, 0 );
  outmask->set( 2, 1, 0 );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfM()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 0 );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 0 );

  outmask->set( 1, 0, 0 );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 0 );

  outmask->set( 2, 0, 0 );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 0 );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfM_()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 0 );
  outmask->set( 0, 1, 0 );
  outmask->set( 0, 2, 0 );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, 0 );
  outmask->set( 2, 1, 0 );
  outmask->set( 2, 2, 0 );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfMPlus()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 0 );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 0 );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, 0 );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 0 );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfMtot()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 1. );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, 1. );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 1. );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 1. );
  outmask->set( 2, 2, 1. );

  return outmask;
}


TePDIFilterMask::pointer TePDIFilterMask::create_MorfMX()
{
  TePDIFilterMask::pointer outmask( new TePDIFilterMask( 3, 0 ) );

  outmask->set( 0, 0, 1. );
  outmask->set( 0, 1, 0 );
  outmask->set( 0, 2, 1. );

  outmask->set( 1, 0, 0 );
  outmask->set( 1, 1, 1. );
  outmask->set( 1, 2, 0 );

  outmask->set( 2, 0, 1. );
  outmask->set( 2, 1, 0 );
  outmask->set( 2, 2, 1. );

  return outmask;
}



