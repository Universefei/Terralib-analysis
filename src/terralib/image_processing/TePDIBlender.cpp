#include "TePDIBlender.hpp"

#include "TePDIUtils.hpp"

#include "../kernel/TeOverlay.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeGTFactory.h"
#include "../kernel/TeAgnostic.h"

TePDIBlender::TePDIBlender()
: dummyValue_( 0.0 ),
  blendFunctPtr_( &TePDIBlender::noBlendMethodImp ), 
  blendMethod_( NoBlendMethod ),
  relation_( 0 )
{
}

TePDIBlender::~TePDIBlender()
{
  reset();
}

void TePDIBlender::reset()
{
  dummyValue_ = 0;
  blendFunctPtr_ = &TePDIBlender::noBlendMethodImp;
  blendMethod_ = NoBlendMethod;
  relation_ = 0;
  transPrt_.reset();
  raster1Ptr_.reset();
  raster1ChannelsVec_.clear();
  raster2Ptr_.reset();
  raster2ChannelsVec_.clear();
  interPolR1IdxRef_.clear();
  interLineSetInR1_.clear();
  interLineSetInR2_.clear();
  pixelOffsets1_.clear();
  pixelScales1_.clear();
  pixelOffsets2_.clear();
  pixelScales2_.clear();
}

bool TePDIBlender::init( const TePDITypes::TePDIRasterPtrType& raster1Ptr,
  const std::vector< unsigned int > raster1ChannelsVec,
  const TePDITypes::TePDIRasterPtrType& raster2Ptr,
  const std::vector< unsigned int > raster2ChannelsVec,
  BlendMethod blendMethod,
  TePDIInterpolator::InterpMethod interpMethod,
  const TeGTParams& transParams,
  const double& dummyValue,
  const std::vector< double >& pixelOffsets1,
  const std::vector< double >& pixelScales1,
  const std::vector< double >& pixelOffsets2,
  const std::vector< double >& pixelScales2,
  const TePolygon& raster1Pol, 
  const TePolygon& raster2Pol )
{
  // Reset this instance
  
  reset();
  
  // Updating global vars
  
  dummyValue_ = dummyValue;
  
  // updating input images parameters 
  {
    TEAGN_TRUE_OR_RETURN( raster1Ptr.isActive(), "Invalid pointer" );
    TEAGN_TRUE_OR_RETURN( raster2Ptr.isActive(), "Invalid pointer" );
    TEAGN_TRUE_OR_RETURN( raster1Ptr->params().status_ != 
      TeRasterParams::TeNotReady, "Invalid pointer" );
    TEAGN_TRUE_OR_RETURN( raster2Ptr->params().status_ != 
      TeRasterParams::TeNotReady, "Invalid pointer" );
      
    TEAGN_TRUE_OR_RETURN( raster1ChannelsVec.size(), 
      "Invalid raster channels vector size" )  
    TEAGN_TRUE_OR_RETURN( raster2ChannelsVec.size(), 
      "Invalid raster channels vector size" )  
    TEAGN_TRUE_OR_RETURN( raster1ChannelsVec.size() == raster2ChannelsVec.size(), 
      "Incompatible rasters channels vectors size" )  
    TEAGN_TRUE_OR_RETURN( raster1ChannelsVec.size() == pixelOffsets1.size(), 
      "Incompatible pixelOffsets1 size" )      
    TEAGN_TRUE_OR_RETURN( raster1ChannelsVec.size() == pixelScales1.size(), 
      "Incompatible pixelScales1 size" )         
    TEAGN_TRUE_OR_RETURN( raster2ChannelsVec.size() == pixelOffsets2.size(), 
      "Incompatible pixelOffsets2 size" )      
    TEAGN_TRUE_OR_RETURN( raster2ChannelsVec.size() == pixelScales2.size(), 
      "Incompatible pixelScales2 size" )          
      
    for( unsigned int cVecIdx1 = 0 ; cVecIdx1 < raster1ChannelsVec.size() ;
      ++cVecIdx1 )
    {
      TEAGN_TRUE_OR_RETURN( ((int)raster1ChannelsVec[ cVecIdx1 ]) < 
        raster1Ptr->params().nBands(), "Invalid band/channel" );
    }
    for( unsigned int cVecIdx2 = 0 ; cVecIdx2 < raster2ChannelsVec.size() ;
      ++cVecIdx2 )
    {
      TEAGN_TRUE_OR_RETURN( ((int)raster2ChannelsVec[ cVecIdx2 ]) < 
        raster2Ptr->params().nBands(), "Invalid band/channel" );
    }
    
    raster1Ptr_ = raster1Ptr;
    raster1ChannelsVec_ = raster1ChannelsVec;
    raster2Ptr_ = raster2Ptr;
    raster2ChannelsVec_ = raster2ChannelsVec;
    
    pixelOffsets1_ = pixelOffsets1;
    pixelScales1_ = pixelScales1;
    pixelOffsets2_ = pixelOffsets2;
    pixelScales2_ = pixelScales2;
  }
  
  // initiating the interpolators instances
  
  TEAGN_TRUE_OR_RETURN( interp1Instance_.reset( raster1Ptr_, interpMethod,
    dummyValue_ ), "Error initiating interpolator instance" );
    
  TEAGN_TRUE_OR_RETURN( interp2Instance_.reset( raster2Ptr_, interpMethod,
    dummyValue_ ), "Error initiating interpolator instance" );    
  
  // updating geometric transformation parameters
  
  transPrt_.reset( TeGTFactory::make( transParams ) );
  TEAGN_TRUE_OR_RETURN( transPrt_.isActive() , 
    "Invalid transformation parameters" );  
    
  if( ! transPrt_->reset( transParams ) )
  {
    transPrt_.reset();

    TEAGN_LOG_AND_RETURN( "Invalid transformation parameters" );
  }  
  
  // Updating data related to the intersection between the two images
  
  {
    // definint the raster 2 bounding box polygon - 
    // raster 2 indexed  reference
    TePolygon r2IdxPolBBoxR2Ref;    
  
    if( raster2Pol.size() )
    {
      r2IdxPolBBoxR2Ref = raster2Pol;
    }
    else
    {
      // raster 2 bounding box polygon - raster 2 projected  reference
      TePolygon r2PolBBoxR2Ref = polygonFromBox( 
        raster2Ptr_->params().boundingBox() );
        
      TePDIUtils::MapCoords2RasterIndexes( r2PolBBoxR2Ref, raster2Ptr_,
        r2IdxPolBBoxR2Ref );        
    }
      
    // raster 2 bounding box polygon - raster 1 indexed  reference
    TePolygon r2IdxPolBBoxR1Ref;
    {
      TeLine2D line;
      TeCoord2D pt1;
      TeCoord2D pt2;
      
      TEAGN_DEBUG_CONDITION( r2IdxPolBBoxR2Ref.size() == 1, 
        "Internal error" );
      
      for( unsigned int pointIdx = 0 ; pointIdx < r2IdxPolBBoxR2Ref[ 0 ].size() ;
        ++pointIdx )
      {
        pt2 = r2IdxPolBBoxR2Ref[ 0 ][ pointIdx ];
        
        transPrt_->inverseMap( pt2, pt1 );
        
        line.add( pt1 );
      }
      
      r2IdxPolBBoxR1Ref.add( TeLinearRing( line ) );
    }
    
    // defining the raster 1 bounding box polygon - raster 1 indexed  reference
    
    TePolygon r1IdxPolBBoxR1Ref;  
    
    if( raster1Pol.size() )
    {
      r1IdxPolBBoxR1Ref = raster1Pol;
    }
    else
    {
      // raster 1 bounding box polygon - raster 1 projected  reference
      TePolygon r1PolBBoxR1Ref = polygonFromBox( 
        raster1Ptr_->params().boundingBox() );    
        
      TePDIUtils::MapCoords2RasterIndexes( r1PolBBoxR1Ref, raster1Ptr_,
        r1IdxPolBBoxR1Ref );        
    };
      
    // Calculating the intersection parameters interPolR1IdxRef_
    // interLineSetInR1_ interLineSetInR2_
    
    // raster 2 bounding box polygon set - raster 1 indexed  reference
    TePolygonSet r2IdxPolBBoxPSR1Ref;
    r2IdxPolBBoxPSR1Ref.add( r2IdxPolBBoxR1Ref );
    
    // raster 1 bounding box polygon set - raster 1 indexed  reference
    TePolygonSet r1IdxPolBBoxPSR1Ref;
    r1IdxPolBBoxPSR1Ref.add( r1IdxPolBBoxR1Ref );  
         
    // Raster 1 line set - raster 1 indexed  reference
    TeLineSet r1IdxLineSetR1Ref;      
    TePDIUtils::makeSegmentSet( r1IdxPolBBoxR1Ref, r1IdxLineSetR1Ref );   
    
    // Raster 2 line set - raster 1 indexed  reference
    TeLineSet r2IdxLineSetR1Ref;
    TePDIUtils::makeSegmentSet( r2IdxPolBBoxR1Ref, r2IdxLineSetR1Ref );    
    
    relation_ = TeRelation( r1IdxPolBBoxR1Ref, r2IdxPolBBoxR1Ref );
    
    switch( relation_ )
    {
      case TeDISJOINT :
      case TeTOUCHES :
      {
        break;
      }
      case TeEQUALS :
      {
        break;
      }
      case TeCOVEREDBY :
      case TeWITHIN :
      {
        interPolR1IdxRef_ = r1IdxPolBBoxR1Ref;
        interLineSetInR2_ = r1IdxLineSetR1Ref;
        break;
      }
      case TeCONTAINS :
      case TeCOVERS :
      {
        interPolR1IdxRef_ = r2IdxPolBBoxR1Ref;
        interLineSetInR1_ = r2IdxLineSetR1Ref;
        break;
      }
      case TeOVERLAPS :
      {
        // Intersection polygon set - raster 1 indexed reference
        TePolygonSet interPsR1IdxRef;        
          
        if( TeOVERLAY::TeIntersection( r1IdxPolBBoxPSR1Ref,
          r2IdxPolBBoxPSR1Ref, interPsR1IdxRef ) )
        {
          if( interPsR1IdxRef.size() == 1 )
          {
            if( interPsR1IdxRef[ 0 ].size() == 1 )
            {
              interPolR1IdxRef_ = interPsR1IdxRef[ 0 ];
              
              TeMultiGeometry interR1MGeom = TeOVERLAY::TeIntersection( 
                r1IdxLineSetR1Ref, r2IdxPolBBoxPSR1Ref );
              
              TeMultiGeometry interR2MGeom = TeOVERLAY::TeIntersection( 
                r2IdxLineSetR1Ref, r1IdxPolBBoxPSR1Ref );        
                
              if( ( ! interR1MGeom.getGeometry( interLineSetInR2_ ) ) ||
                ( ! interR2MGeom.getGeometry( interLineSetInR1_ ) ) )
              {
                interLineSetInR1_.clear();
                interLineSetInR1_.clear();
              }                  
            }
          }
        }
              
        break;      
      }

      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid relation" );
      }
    }
  }
  
  // updating blending and interpolation parameters
  
  blendMethod_ = blendMethod;
  
  switch( blendMethod_ )
  {
    case NoBlendMethod :
    {
      blendFunctPtr_ = &TePDIBlender::noBlendMethodImp;
      
      break;
    }
    case MeanBlendMethod :
    {
      blendFunctPtr_ = &TePDIBlender::meanBlendMethodImp;
      
      break;
    }
    case EuclideanBlendMethod :
    {
      if( ( interLineSetInR1_.size() > 0 ) && 
        ( interLineSetInR2_.size() > 0 ) )
      {
        blendFunctPtr_ = &TePDIBlender::eucBlendMethodImp;
      }
      else
      {
        blendFunctPtr_ = &TePDIBlender::meanBlendMethodImp;
      }
      
      break;
    }
    default :
    {
      TEAGN_LOG_AND_THROW( "Invalid blending method" );
      
      break;
    }      
  }
       
  // So far... so good !

  return true;
}
      
void TePDIBlender::noBlendMethodImp( const double& line, const double& col,
  const unsigned int& rasterChannelsVecsIdx, double& value )
{
  TEAGN_DEBUG_CONDITION( transPrt_.isActive(), "Invalid transformation pointer" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster1ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster2ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets2_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales2_.size(),
    "Invalid raster channel index" );
              
  interp1Instance_.interpolate( line, col, raster1ChannelsVec_[ 
    rasterChannelsVecsIdx ], value);
    
  if( value == dummyValue_ )
  {
    noBlendMethodImp_coord1_.x( col );
    noBlendMethodImp_coord1_.y( line );
    
    transPrt_->directMap( noBlendMethodImp_coord1_, noBlendMethodImp_coord2_ );
    
    interp2Instance_.interpolate( noBlendMethodImp_coord2_.y(), 
      noBlendMethodImp_coord2_.x(), raster2ChannelsVec_[ rasterChannelsVecsIdx 
      ], value);
      
    if( value != dummyValue_ )
    {
      value = ( value * pixelScales2_[ rasterChannelsVecsIdx ] ) + 
        pixelOffsets2_[ rasterChannelsVecsIdx ];
    }
  }
  else
  {
    value = ( value * pixelScales1_[ rasterChannelsVecsIdx ] ) + 
      pixelOffsets1_[ rasterChannelsVecsIdx ];
  }
}
        
void TePDIBlender::eucBlendMethodImp( const double& line, const double& col,
  const unsigned int& rasterChannelsVecsIdx, double& value )
{
  TEAGN_DEBUG_CONDITION( transPrt_.isActive(), "Invalid transformation pointer" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster1ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster2ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets2_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales2_.size(),
    "Invalid raster channel index" );   
  TEAGN_DEBUG_CONDITION( ( interLineSetInR1_.size() > 0 ) &&
    ( interLineSetInR2_.size() > 0 ), "Internal error" );
  
  interp1Instance_.interpolate( line, col, raster1ChannelsVec_[ 
    rasterChannelsVecsIdx ], eucBlendMethodImp_value1_);
    
  eucBlendMethodImp_coord1_.x( col );
  eucBlendMethodImp_coord1_.y( line ); 
     
  transPrt_->directMap( eucBlendMethodImp_coord1_, eucBlendMethodImp_coord2_ );
  
  interp2Instance_.interpolate( eucBlendMethodImp_coord2_.y(), 
    eucBlendMethodImp_coord2_.x(), raster2ChannelsVec_[ rasterChannelsVecsIdx 
    ], eucBlendMethodImp_value2_ );  
    
  if( eucBlendMethodImp_value1_ == dummyValue_ )
  {
    if( eucBlendMethodImp_value2_ == dummyValue_ )
    {
      value = eucBlendMethodImp_value2_;
    }
    else
    {
      value = ( eucBlendMethodImp_value2_ * pixelScales2_[ 
        rasterChannelsVecsIdx ] ) + pixelOffsets2_[ rasterChannelsVecsIdx ];
    }
  }
  else
  {
    eucBlendMethodImp_value1_ = ( eucBlendMethodImp_value1_ * 
      pixelScales1_[ rasterChannelsVecsIdx ] ) + pixelOffsets1_[
      rasterChannelsVecsIdx ];
    
    if( eucBlendMethodImp_value2_ == dummyValue_ )
    {
      value = eucBlendMethodImp_value1_;
    }
    else
    {
      eucBlendMethodImp_value2_ = ( eucBlendMethodImp_value2_ * 
        pixelScales2_[ rasterChannelsVecsIdx ] ) + 
        pixelOffsets2_[ rasterChannelsVecsIdx ];
        
      // Distance from the current point to raster 1 
      
      eucBlendMethodImp_minDistToR1_ = DBL_MAX;
      eucBlendMethodImp_curCoord_.x( col );
      eucBlendMethodImp_curCoord_.y( line );
      
      for( eucBlendMethodImp_iLSIdx_ = 0 ; eucBlendMethodImp_iLSIdx_ < 
        interLineSetInR1_.size() ; ++eucBlendMethodImp_iLSIdx_ )
      {
        TEAGN_DEBUG_CONDITION( interLineSetInR1_[ eucBlendMethodImp_iLSIdx_ 
          ].size() == 2, "Invalid line set size" );
          
        eucBlendMethodImp_currDist_ = TePerpendicularDistance( 
          interLineSetInR1_[ eucBlendMethodImp_iLSIdx_ ][ 0 ],
          interLineSetInR1_[ eucBlendMethodImp_iLSIdx_ ][ 1 ], eucBlendMethodImp_curCoord_, 
          eucBlendMethodImp_dummyCoord_ );
          
        if( eucBlendMethodImp_currDist_ < eucBlendMethodImp_minDistToR1_ ) 
          eucBlendMethodImp_minDistToR1_ = eucBlendMethodImp_currDist_;
      }
      
      // Distance from the current point to raster 2
      
      eucBlendMethodImp_minDistToR2_ = DBL_MAX;
      
      for( eucBlendMethodImp_iLSIdx_ = 0 ; eucBlendMethodImp_iLSIdx_ < 
        interLineSetInR2_.size() ; ++eucBlendMethodImp_iLSIdx_ )
      {
        TEAGN_DEBUG_CONDITION( interLineSetInR2_[ eucBlendMethodImp_iLSIdx_ 
          ].size() == 2, "Invalid line set size" );
          
        eucBlendMethodImp_currDist_ = TePerpendicularDistance( 
          interLineSetInR2_[ eucBlendMethodImp_iLSIdx_ ][ 0 ],
          interLineSetInR2_[ eucBlendMethodImp_iLSIdx_ ][ 1 ], eucBlendMethodImp_curCoord_, 
          eucBlendMethodImp_dummyCoord_ );
          
        if( eucBlendMethodImp_currDist_ < eucBlendMethodImp_minDistToR2_ ) 
          eucBlendMethodImp_minDistToR2_ = eucBlendMethodImp_currDist_;
      }  
        
      // Updating weights
      
      if( eucBlendMethodImp_minDistToR1_ == 0.0 )
      {
        eucBlendMethodImp_weight1_ = 1.0;
      }
      else
      {
        eucBlendMethodImp_weight1_ = 1.0 / eucBlendMethodImp_minDistToR1_;
      }
        
      if( eucBlendMethodImp_minDistToR2_ == 0.0 )
      {
        eucBlendMethodImp_weight2_ = 1.0;
      }
      else
      {
        eucBlendMethodImp_weight2_ = 1.0 / eucBlendMethodImp_minDistToR2_;
      } 
             
      TEAGN_DEBUG_CONDITION( ( ( eucBlendMethodImp_weight1_ > 0.0 ) ||
       ( eucBlendMethodImp_weight2_ > 0.0 ) ),
       "Invalid weights" );
    
      value = ( ( eucBlendMethodImp_value1_ * eucBlendMethodImp_weight1_ ) +
        ( eucBlendMethodImp_value2_ * eucBlendMethodImp_weight2_ ) ) /
        ( eucBlendMethodImp_weight1_ + eucBlendMethodImp_weight2_ );
    }
  }
}

void TePDIBlender::meanBlendMethodImp( const double& line, const double& col,
  const unsigned int& rasterChannelsVecsIdx, double& value )
{
  TEAGN_DEBUG_CONDITION( transPrt_.isActive(), "Invalid transformation pointer" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster1ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < raster2ChannelsVec_.size(),
    "Invalid raster channels vector index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales1_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelOffsets2_.size(),
    "Invalid raster channel index" );
  TEAGN_DEBUG_CONDITION( rasterChannelsVecsIdx < pixelScales2_.size(),
    "Invalid raster channel index" );   
  
  interp1Instance_.interpolate( line, col, raster1ChannelsVec_[ 
    rasterChannelsVecsIdx ], meanBlendMethodImp_value1_);
    
  meanBlendMethodImp_coord1_.x( col );
  meanBlendMethodImp_coord1_.y( line ); 
     
  transPrt_->directMap( meanBlendMethodImp_coord1_, meanBlendMethodImp_coord2_ );
  
  interp2Instance_.interpolate( meanBlendMethodImp_coord2_.y(), 
    meanBlendMethodImp_coord2_.x(), raster2ChannelsVec_[ rasterChannelsVecsIdx 
    ], meanBlendMethodImp_value2_ );  
    
  if( meanBlendMethodImp_value1_ == dummyValue_ )
  {
    if( meanBlendMethodImp_value2_ == dummyValue_ )
    {
      value = meanBlendMethodImp_value2_;
    }
    else
    {
      value = ( meanBlendMethodImp_value2_ * pixelScales2_[ 
        rasterChannelsVecsIdx ] ) + pixelOffsets2_[ rasterChannelsVecsIdx ];
    }
  }
  else
  {
    meanBlendMethodImp_value1_ = ( meanBlendMethodImp_value1_ * 
      pixelScales1_[ rasterChannelsVecsIdx ] ) + pixelOffsets1_[
      rasterChannelsVecsIdx ];
    
    if( meanBlendMethodImp_value2_ == dummyValue_ )
    {
      value = meanBlendMethodImp_value1_;
    }
    else
    {
      meanBlendMethodImp_value2_ = ( meanBlendMethodImp_value2_ * 
        pixelScales2_[ rasterChannelsVecsIdx ] ) + 
        pixelOffsets2_[ rasterChannelsVecsIdx ];
        
      value = ( meanBlendMethodImp_value1_ + meanBlendMethodImp_value2_ ) / 2.0;
    }
  }
}
