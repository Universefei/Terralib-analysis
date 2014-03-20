/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2004 INPE and Tecgraf/PUC-Rio.

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

#include "TePDIRaster2Vector.hpp"
#include "../kernel/TeAgnostic.h"
#include "TePDIUtils.hpp"

#include "../kernel/TeUtils.h"

#include <algorithm>
#include <string>
#include <vector>

#include <float.h>

//these are the 8 cardinals directions used to parcour an edge
#define NW 0
#define N  1
#define EN 2
#define E  3
#define SE 4
#define S  5
#define WS 6
#define W  7


TePDIRaster2Vector::TePDIRaster2Vector()
{
  raster_uses_dummy_ = false;
  dummy_value_ = 0;
  containerPolygons_.clear();
  rtreePolygons_=NULL;
  raster_channel_ = 0;
  
  /* Updating the directions  */
  
  directions_[ 0 ] = TeCoord2D( -1, -1 );  /* NW - 0 */
  directions_[ 1 ] = TeCoord2D(  0, -1 );  /* N  - 1 */
  directions_[ 2 ] = TeCoord2D(  1, -1 );  /* EN - 2 */
  directions_[ 3 ] = TeCoord2D(  1,  0 );  /* E  - 3 */
  directions_[ 4 ] = TeCoord2D(  1,  1 );  /* SE - 4 */
  directions_[ 5 ] = TeCoord2D(  0,  1 );  /* S  - 5 */
  directions_[ 6 ] = TeCoord2D( -1,  1 );  /* WS - 6 */
  directions_[ 7 ] = TeCoord2D( -1,  0 );  /* W  - 7 */  
}

TePDIRaster2Vector::~TePDIRaster2Vector()
{
  clear();
}

bool
TePDIRaster2Vector :: CheckParameters( 
  const TePDIParameters& parameters ) const {
  /* Checking for general required parameters */

  //parameter input_image
  TePDITypes::TePDIRasterPtrType inRaster;
  if( ! parameters.GetParameter( "rotulated_image", inRaster ) ) {

    TEAGN_LOGERR( "Missing parameter: rotulated_image" );
    return false;
  }
  if( ! inRaster.isActive() ) {

    TEAGN_LOGERR( "Invalid parameter: rotulated_image inactive" );
    return false;
  }
  if( inRaster->params().status_ == TeRasterParams::TeNotReady ) {

    TEAGN_LOGERR( "Invalid parameter: rotulated_image not ready" );
    return false;
  }
  
  /* Checking output_polsets */
  
  TEAGN_TRUE_OR_RETURN( 
    parameters.CheckParameter< TePDITypes::TePDIPolSetMapPtrType >( 
    "output_polsets" ), "Missing parameter: output_polsets" );
  
  TePDITypes::TePDIPolSetMapPtrType output_polsets;
  parameters.GetParameter( "output_polsets", output_polsets );
            
  TEAGN_TRUE_OR_RETURN( output_polsets.isActive(),
    "Invalid parameter output_polsets : Inactive polygon set pointer" );

  /* Checking channel */
  
  unsigned int channel = 0;
  
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "channel", channel ), 
    "Missing parameter: channel" );
    
  TEAGN_TRUE_OR_RETURN( ( channel < (unsigned int)inRaster->nBands() ),
    "Invalid parameter: channel" );
    
  /* checking rotulated_image data type */
  
  for( int band = 0 ; band < inRaster->params().nBands() ; ++band ) {
    TEAGN_TRUE_OR_RETURN( 
      ( inRaster->params().dataType_[ band ] != TeFLOAT ), 
      "Invalid rotulated_image floating point raster data type" );
    TEAGN_TRUE_OR_RETURN( 
      ( inRaster->params().dataType_[ band ] != TeDOUBLE ), 
      "Invalid rotulated_image floating point raster data type" );    
  }  
    
  return true;
}


bool TePDIRaster2Vector::RunImplementation()
{
  clear();
  
  /* Extratcting parameters */
  
  params_.GetParameter( "rotulated_image", raster_ );
  params_.GetParameter( "channel", raster_channel_ );  
  
  unsigned long int max_pols = 0;
  if( params_.CheckParameter< unsigned long int >( "max_pols" ) ) {
    params_.GetParameter( "max_pols", max_pols );
  }
  
  /* Guessing dummy use */
        
  raster_uses_dummy_ = raster_->params().useDummy_;
  dummy_value_ = 0;
  if( raster_uses_dummy_ ) {
    dummy_value_ = raster_->params().dummy_[ 0 ];
  }

  //attributes of the raster
  
  resx_ = raster_->params().resx_ ;
  resy_ = raster_->params().resy_ ;

  const TeBox raster_bbox=raster_->params().boundingBox();
  nCols_  = raster_->params().ncols_;
  nLines_ = raster_->params().nlines_;

  //declarations
        
  double val;
  int i,j;

  /* Creating a new RTree */
  
  rtreePolygons_ = new TeSAM::TeRTree<unsigned int, 8, 4>(
    raster_->params().boundingBox());
    
  int countObjects = 0;
  vector<unsigned int> indexVec;

  StartProgInt( "Vectorizing...", nLines_ );
                
  // scanning rotulated image in row order to vectorize the cells
  
  TeCoord2D last_line_ll_point;
  TeCoord2D last_line_ll_point_indexed;
  TeCoord2D last_line_lr_point;
  TeCoord2D last_line_lr_point_indexed;  
  
  for ( j = 0 ; j < nLines_ ; j++) {
    TEAGN_FALSE_OR_RETURN( UpdateProgInt( j ), "Canceled by the user" );
    
    /* Cleaning the tileindexers that will not be used anymore */
    
    last_line_ll_point_indexed.setXY( 0, ( (double)j ) - 1.0 );
    last_line_lr_point_indexed.setXY( ( (double)nCols_ ) - 1, 
      ( (double)j ) - 1.0 );
    
    last_line_ll_point = raster_->index2Coord( 
      last_line_ll_point_indexed );
    last_line_lr_point = raster_->index2Coord( 
      last_line_lr_point_indexed );
            
    TeBox last_line_bbox( last_line_ll_point,
      last_line_lr_point );
      
    indexVec.clear();
    rtreePolygons_->search( last_line_bbox, indexVec ); 
    
    for( unsigned int indexVec_index = 0 ;
      indexVec_index < indexVec.size() ; ++indexVec_index ) {
      
      TePDIRaster2VectorPolStruct& curr_pol_struct =
        containerPolygons_[ indexVec[ indexVec_index ] ];
      
      if( raster_->coord2Index( 
        curr_pol_struct.poly_.box().lowerLeft() ).y() < j ) {
        
        curr_pol_struct.indexer_->clear();
      }
    }
      
    /* iterating through columns */
    
    for ( i = 0 ; i < nCols_ ; i++) {
      /* Virifying if the current point is already inside a polygon generated
         before */
         
      TeCoord2D coordMatrix(i, j);
      TeCoord2D coordWorld = raster_->index2Coord(coordMatrix);
      TeBox boxPoint(coordWorld, coordWorld);
      bool exist = false;  
      
      indexVec.clear();
      rtreePolygons_->search( boxPoint, indexVec );
      
      if( ! raster_->getElement(i,j,val,raster_channel_) ) {
        val = dummy_value_;
      }      
      
      if ( ! indexVec.empty() ) {
        unsigned int indexVec_index = 0;        

        while( indexVec_index < indexVec.size() ) {
          TEAGN_DEBUG_CONDITION( 
            ( indexVec[ indexVec_index ] < containerPolygons_.size() ),
            "Invalid cadidates vector index" );
    
          const TePDIRaster2VectorPolStruct& pol_str_ref = 
            containerPolygons_[ indexVec[ indexVec_index ] ];
      
          if ( val == pol_str_ref.value_ ) {
            if( TePDIUtils::TeRelation( coordWorld, 
              *( pol_str_ref.indexer_ ) ) == TeINSIDE ) {
                 
              exist = true;
              break;
            }        
          }
          
          ++indexVec_index;
        }                
      }

      if( ! exist ) {
         //we found a new class
         
        TeLine2D line; 
         
        if( detectEdge( i, j, line ) ) {
          try {
            TePolygon poly;
            poly.add( line );
            
            /* Verifying if the polygon is a hole of other one */
              
            unsigned int indexVec_index = 0;
            
            while( indexVec_index < indexVec.size() ) {
              TePDIRaster2VectorPolStruct& pol_struct_ref = 
                containerPolygons_[ indexVec[ indexVec_index ] ];
              TePDITileIndexer& indexer_ref = *( pol_struct_ref.indexer_ );
              TePolygon& pol_ref = pol_struct_ref.poly_;
            
              if( TePDIUtils::TeRelation( coordWorld, indexer_ref ) == 
                TeINSIDE ) {
              
                pol_ref.add( poly.first() );
                indexer_ref.addRing( pol_ref.size() - 1 );
                  
                break;
              }
              
              ++indexVec_index;
            }
            
            /* free indexVec */
            
            indexVec.clear();
            
            // inserting the new polygon into tree and container
            
            if( raster_uses_dummy_ && ( val == dummy_value_ ) ) {
              poly.objectId( "none" );
            } else {
              ++countObjects;
              poly.objectId( Te2String( countObjects ) );
            }
            
            TePDIRaster2VectorPolStruct dummy_ps;
            containerPolygons_.push_back( dummy_ps );
 
            containerPolygons_[ containerPolygons_.size() - 
              1 ].reset( poly, (int)val, 
              MAX( 
                  (
                    4.0 * poly.box().height() 
                    /
                    (
                      poly.box().width()
                      *
                      ( (double) poly[ 0 ].size() )
                    )
                  )
                  ,
                  raster_->params().resy_
                 )
            );                                  
      
            rtreePolygons_->insert( poly.box(), containerPolygons_.size() - 1 );  
          }
          catch(...) {
            TEAGN_LOGERR( "Error generating polygon - "
              "not enough memory" );
              
            /* no more polygons will be generated */    
            i = nCols_;
            j = nLines_;              
          }
        } else {
          /* no more polygons will be generated */
          i = nCols_;
          j = nLines_;
        }
        
        if( max_pols ) {
          if( countObjects >= ( (int)max_pols ) ) {
            /* no more polygons will be generated */
            i = nCols_;
            j = nLines_;          
          }
        }
      }
    }
  }
  
  /* Cleaning the tree */
  
  delete rtreePolygons_;
  rtreePolygons_ = 0;
        
  /* Generating output data */
  
  TePDITypes::TePDIPolSetMapPtrType output_polsets;
  params_.GetParameter( "output_polsets", output_polsets );
      
  output_polsets->clear();
  
  std::map< double, TePolygonSet >::iterator psm_it;
    
  std::vector<TePDIRaster2VectorPolStruct>::iterator cpit = 
    containerPolygons_.begin();
  std::vector<TePDIRaster2VectorPolStruct>::iterator cpit_end = 
    containerPolygons_.end();
  
  double value = 0;
    
  try {
    while( cpit != cpit_end ) {
      value = (double)cpit->value_;
    
      if( ( ! raster_uses_dummy_ ) || ( value != dummy_value_ ) ) {
        psm_it = output_polsets->find( value );
        
        if( psm_it == output_polsets->end() ) {
          TePolygonSet dummy_ps;
          dummy_ps.add( cpit->poly_ );
          
          (*output_polsets)[ value ] = dummy_ps;
        } else {
          psm_it->second.add( cpit->poly_ );
        }
      }
      
      cpit->clear();
      
      ++cpit;
    }
  }
  catch(...) {
    TEAGN_LOG_AND_THROW( 
      "Memory error - error generating output polygons sets map" )
  }
                
  return true;
}

void
TePDIRaster2Vector::ResetState( const TePDIParameters& /* params */ )
{
  clear();       
}


bool TePDIRaster2Vector::detectEdge(long i, long j, TeLine2D &line )
{
  TEAGN_DEBUG_RETURN( startingEdgeTest( i, j ), 
    "Starting edge detection error at x=" + Te2String( i ) + " line=" +
    Te2String( j ) )
    
  line.clear();
    
  try {
    /* Current polygon pixel values */
    
    double pol_pixels_value = 0;
    
    if( ! raster_->getElement(i, j, pol_pixels_value, raster_channel_ ) ) {
      pol_pixels_value = dummy_value_;
    }  
    
    /* Generating chaing code by following the polygon borders */
    
    short curr_dir = E;
    short new_dir = E;
    short new_test_start_dir = E;
    short curr_pixel_corner = NW;
    
    short pinit_leaving_dir = W;
    bool pinit_leaving_dir_set = false;
    
    TeCoord2D curr_chain_p = TeCoord2D( 0, 0 );

    int curr_x_index = i;
    int curr_y_index = j;
    int next_x_index = i;
    int next_y_index = j;
    
    double curr_pixel_value = 0;
    
    bool look_for_next_pixel = true;
    bool next_pixel_found = false;
    bool must_add_curr_chain_p = false;
    
    line.add( curr_chain_p );
    
    while( look_for_next_pixel ) {
      /* Finding the next direction */
          
      new_test_start_dir = ( curr_dir + 6 ) % 8;
      new_dir = new_test_start_dir;
      next_pixel_found = false;
          
      do {
        next_x_index = curr_x_index + (int)( directions_[new_dir].x() );
        next_y_index = curr_y_index + (int)( directions_[new_dir].y() );
              
        if( ( next_x_index > (-1) ) && ( next_y_index > (-1) ) && 
          ( next_x_index < nCols_ ) && ( next_y_index < nLines_ ) ) {
                
          if( ! raster_->getElement( next_x_index, next_y_index, 
            curr_pixel_value, raster_channel_ ) ) {
                          
            curr_pixel_value = dummy_value_;  
          }
                          
          if ( curr_pixel_value == pol_pixels_value ){
            next_pixel_found = true;
            break;
          }
        }
              
        new_dir = ( new_dir + 2 ) % 8;
      } while( new_dir != new_test_start_dir );
      
      /* Generating the polygon line following the current state */
      
        if( ! next_pixel_found ) {
          // there is only one point inside the polygon
          // that is the last one
          
          curr_chain_p += ( directions_[ E ] );
          line.add( curr_chain_p );
          
          curr_chain_p += ( directions_[ S ] );
          line.add( curr_chain_p );
          
          curr_chain_p += ( directions_[ W ] );
          line.add( curr_chain_p );
          
          curr_chain_p += ( directions_[ N ] );
          line.add( curr_chain_p );                        
                        
          look_for_next_pixel = false;
        } else if( ( curr_x_index == i ) && ( curr_y_index == j ) &&
          ( pinit_leaving_dir == new_dir ) ) {
        
          /* We are back to the fist pixel again
            ( going to the same direction ) */
          
          switch( curr_pixel_corner ) {
            case EN :
            {
              curr_chain_p += ( directions_[ S ] );
              line.add( curr_chain_p );
          
              curr_chain_p += ( directions_[ W ] );
              line.add( curr_chain_p );
          
              curr_chain_p += ( directions_[ N ] );
              line.add( curr_chain_p );
              
              break;
            }
            case SE :
            { 
              curr_chain_p += ( directions_[ W ] );
              line.add( curr_chain_p );
          
              curr_chain_p += ( directions_[ N ] );
              line.add( curr_chain_p );
              
              break;
            }
            case WS :
            {
              curr_chain_p += ( directions_[ N ] );
              line.add( curr_chain_p );
                        
              break;
            }
            default :
            {
              TEAGN_LOG_AND_RETURN( "Invalid pixel corner=" + 
                Te2String( curr_pixel_corner) + " curr_dir=" + 
                Te2String( curr_dir ) );
              break;
            }
          }      
         
          look_for_next_pixel = false;   
        } else {
          /* Updating current chain point and current pixel corner */
          
          if( curr_dir == new_dir ) {
            curr_chain_p += ( directions_[ curr_dir ] );
            
            must_add_curr_chain_p = true;
          } else {
            if( must_add_curr_chain_p ) {
              line.add( curr_chain_p );
              
              must_add_curr_chain_p = false;
            }
          
            switch( new_dir ) {
              case E:
              {
                switch( curr_pixel_corner ) {
                  case NW :
                  {
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );
                          
                    break;
                  }
                  case EN :
                  {
                    curr_pixel_corner = NW;
                    break;
                  }
                  case SE :
                  { 
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );                                                      
                    
                    curr_pixel_corner = NW;
                    break;
                  }
                  case WS :
                  {
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );                    
                    
                    curr_pixel_corner = NW;
                    break;
                  }
                  default :
                  {
                    TEAGN_LOG_AND_THROW( "Invalid pixel corner" )
                    break;
                  }
                }
                
                break;
              }
              case S:
              {
                switch( curr_pixel_corner ) {
                  case NW :
                  {
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );  
                                    
                    curr_pixel_corner = EN;  
                    break;
                  }
                  case EN :
                  {
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );
                                    
                    break;
                  }
                  case SE :
                  {
                    curr_pixel_corner = EN;
                    break;
                  }
                  case WS :
                  {
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );                 
                  
                    curr_pixel_corner = EN;
                    break;
                  }
                  default :
                  {
                    TEAGN_LOG_AND_THROW( "Invalid pixel corner" )
                    break;
                  }
                }
                
                break;
              }    
              case W:
              {
                switch( curr_pixel_corner ) {
                  case NW :
                  {
                    curr_chain_p += ( directions_[ E ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p ); 
                                    
                    curr_pixel_corner = SE;  
                    break;
                  }
                  case EN :
                  {
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p ); 
                                    
                    curr_pixel_corner = SE;
                    break;
                  }
                  case SE :
                  {
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p ); 
                                    
                    break;
                  }
                  case WS :
                  {
                    curr_pixel_corner = SE;
                    break;
                  }
                  default :
                  {
                    TEAGN_LOG_AND_THROW( "Invalid pixel corner" )
                    break;
                  }
                }
                
                break;
              }
              case N:
              {
                switch( curr_pixel_corner ) {
                  case NW :
                  {
                    curr_pixel_corner = WS;  
                    break;
                  }
                  case EN :
                  {
                    curr_chain_p += ( directions_[ S ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );
                                    
                    curr_pixel_corner = WS;  
                    break;
                  }
                  case SE :
                  {
                    curr_chain_p += ( directions_[ W ] );
                    line.add( curr_chain_p );
                    
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );
                                    
                    curr_pixel_corner = WS;  
                    break;
                  }
                  case WS :
                  {
                    curr_chain_p += ( directions_[ N ] );
                    line.add( curr_chain_p );                

                    break;
                  }
                  default :
                  {
                    TEAGN_LOG_AND_THROW( "Invalid pixel corner" )
                    break;
                  }
                }
              
                break;
              }
              default :
              {
                TEAGN_LOG_AND_THROW( "Invalid new_dir" )
                break;
              }
            }
          }
            
          /* Updating the leaving direction from the initial point */
          
          if( ( ! pinit_leaving_dir_set ) && ( curr_x_index == i ) && 
              ( curr_y_index == j ) ) {
                
            pinit_leaving_dir_set = true;
            pinit_leaving_dir = new_dir;
          }           
            
          /* updating the current state */
          
          curr_x_index = next_x_index;
          curr_y_index = next_y_index;  
          
          curr_dir = new_dir;         
        }
    }
      
    /* Changing lines coords to world coords */
    
    const TeCoord2D chain_init_p = raster_->index2Coord( TeCoord2D( 
      ( (double)i ) - 0.5, ( (double)j ) - 0.5 ) );  
      
    TeLine2D::iterator it = line.begin();
    TeLine2D::iterator it_end = line.end();
    
    double min_x = DBL_MAX;
    double min_y = DBL_MAX;
    double max_x = ( -1.0 ) * DBL_MAX;
    double max_y = ( -1.0 ) * DBL_MAX;
    
    while( it != it_end ) {
      it->x_ = chain_init_p.x_ + ( it->x_ * resx_ );
      it->y_ = chain_init_p.y_ - ( it->y_ * resy_ );
      
      if( it->x_ < min_x ) min_x = it->x_;
      if( it->y_ < min_y ) min_y = it->y_;

      if( it->x_ > max_x ) max_x = it->x_;
      if( it->y_ > max_y ) max_y = it->y_;
      
      ++it;
    }
    
    /* Updating the line box */
    
    TeBox newbox( min_x, min_y, max_x, max_y );
    line.setBox( newbox );
    
    /* Closing the line, if necessary */
    
    if( line.size() > 1 ) {
      if( ( line[ 0 ].x() != line[ line.size() - 1 ].x() ) ||
        ( line[ 0 ].y() != line[ line.size() - 1 ].y() ) ) {
        
        line.add( line[ 0 ] );
      }
    }  
  }
  catch(...) {
    line.clear();
    
    TEAGN_LOG_AND_RETURN( "Unable to detect edge - "
      "not enough memory" )
  }

  return true;
}


bool TePDIRaster2Vector::startingEdgeTest( const int& x, 
  const int& y )
{
  int nlines = raster_->params().nlines_;
  int ncols = raster_->params().ncols_;

  TEAGN_TRUE_OR_RETURN( ( y < nlines ), 
    "Invalid col [" + Te2String( x ) + "]" );
  TEAGN_TRUE_OR_RETURN( ( x < ncols ), 
    "Invalid line [" + Te2String( y ) + "]" );
    
  if( ( x == 0 ) || ( y == 0 ) || ( x == ( ncols - 1 ) ) || 
    ( y == ( nlines - 1 ) ) ) {
    
    return true;
  } else {
    double test_val = 0;
    double current_val = 0;  
  
    if( ! raster_->getElement(x, y, current_val, raster_channel_ ) ) {
      TEAGN_TRUE_OR_RETURN( raster_uses_dummy_, "Raster read error" );
      current_val = dummy_value_;
    }
    
    if( ! raster_->getElement(x - 1, y, test_val, raster_channel_ ) ) {
      TEAGN_TRUE_OR_RETURN( raster_uses_dummy_, "Raster read error" );
      test_val = dummy_value_;
    }  
  
    TEAGN_TRUE_OR_RETURN( ( current_val != test_val ), 
      "The given point isn't a starting edge [x=" + Te2String( x ) +
      " y=" + Te2String( y ) + "]" );
      
    return true;
  }
}


void TePDIRaster2Vector::clear()
{
  raster_.reset();
  
  if( rtreePolygons_ ) {
    delete rtreePolygons_;
    rtreePolygons_ = 0;
  }
  
  containerPolygons_.clear();
}

