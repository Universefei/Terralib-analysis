#include "TePDIMatrixFunctions.hpp"
#include "../kernel/TeAgnostic.h"

#include "math.h"

namespace TePDIMatrixFunctions {

  double GetCorrelation( const TePDIMatrix< double >& matrix1,
    const TePDIMatrix< double >& matrix2 )
  {
    const unsigned int lines_bound = 
      MIN( matrix1.GetLines(), matrix2.GetLines() );
    const unsigned int cols_bound = 
      MIN( matrix1.GetColumns(), matrix2.GetColumns() );
      
    if( ( lines_bound == 0 ) || ( cols_bound == 0 ) ) {
      return 0;
    } else {
      unsigned int line = 0;
      unsigned int col = 0;
      
      double M1XM2 = 0;
      double M1XM1 = 0;
      double M2XM2 = 0;
      double M1SUM = 0;
      double M2SUM = 0;
      
      double element1 = 0;
      double element2 = 0;
      
      for( line = 0; line < lines_bound ; ++line ) {
        for( col = 0 ; col < cols_bound ; ++col ) {
          element1 = matrix1( line, col );
          element2 = matrix2( line, col );
        
          M1XM2 += element1 * element2;
          M1XM1 += element1 * element1;
          M2XM2 += element2 * element2;
          M1SUM += element1;
          M2SUM += element2;
        }
      }
      
      const double elements = ( double )( lines_bound * cols_bound );
      double mean1 = M1SUM / elements;
      double mean2 = M2SUM / elements;
      
      double var1 = sqrt( ( M1XM1 / elements ) - ( mean1 * mean1 ) );
      double var2 = sqrt( ( M2XM2 / elements ) - ( mean2 * mean2 ) );
      
      if( ( var1 != 0 ) && ( var2 != 0 ) ) {
        return ( ( ( M1XM2 / elements ) - ( mean1 * mean2 ) ) /
          ( var1 * var2 ) );
      } else {
        return 0;
      }
    }
  }
  
  
  bool Abs( const TePDIMatrix< double >& inMatrix, 
    TePDIMatrix< double >& absMatrix )
  {
    const unsigned int lines_bound = inMatrix.GetLines();
    const unsigned int cols_bound = inMatrix.GetColumns();
    unsigned int line = 0;
    unsigned int col = 0;
    
    TEAGN_TRUE_OR_RETURN( absMatrix.Reset( lines_bound, cols_bound ),
      "Unable to reset output matrix" );
    
    for( line = 0 ; line < lines_bound ; ++line ) {
      for( col = 0 ; col < cols_bound ; ++col ) {
        absMatrix( line, col ) = ABS( inMatrix( line, col ) );
      }
    }
    
    return true;
  }

  
  bool WhereIsGreater( const TePDIMatrix< double >& inMatrix1,
    const TePDIMatrix< double >& inMatrix2,
    TePDIMatrix< double >& gtMatrix )
  {
    const unsigned int m1lines = inMatrix1.GetLines();
    const unsigned int m2lines = inMatrix2.GetLines();
    const unsigned int m1cols = inMatrix1.GetColumns();
    const unsigned int m2cols = inMatrix2.GetColumns();
    
    unsigned int line = 0;
    unsigned int col = 0;
    
    TEAGN_TRUE_OR_RETURN( gtMatrix.Reset( m1lines, m1cols ),
      "Unable to reset output matrix" );
    
    for( line = 0 ; line < m1lines ; ++line ) {
      for( col = 0 ; col < m1cols ; ++col ) {
        if( ( line < m2lines ) && ( col < m2cols ) ) {
          if( inMatrix1( line, col ) > inMatrix2( line, col ) ) {
            gtMatrix( line, col ) = 1.;
          } else {
            gtMatrix( line, col ) = 0;
          }
        } else {
          gtMatrix( line, col ) = 0;
        }
      }
    }
    
    return true;  
  }
  
  
  bool Negate( const TePDIMatrix< double >& inMatrix,
    TePDIMatrix< double >& negMatrix )
  {
    const unsigned int lines_bound = inMatrix.GetLines();
    const unsigned int cols_bound = inMatrix.GetColumns();
    unsigned int line = 0;
    unsigned int col = 0;
    
    TEAGN_TRUE_OR_RETURN( negMatrix.Reset( lines_bound, cols_bound ),
      "Unable to reset output matrix" );
    
    for( line = 0 ; line < lines_bound ; ++line ) {
      for( col = 0 ; col < cols_bound ; ++col ) {
        if( inMatrix( line, col ) == 0 ) {
          negMatrix( line, col ) = 1.0;
        } else {
          negMatrix( line, col ) = 0.0;
        }
      }
    }
    
    return true;
  }  

}

