#include "TePDIMMIOMatching.hpp"

#include "TePDIUtils.hpp"
#include "TePDITypes.hpp"

#include "../kernel/TeThreadJobsManager.h"
#include "../kernel/TeFunctionCallThreadJob.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeGTFactory.h"
#include "../kernel/TeDefines.h"
#include "../kernel/TeMutex.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeRTree.h"

#include <math.h>
#include <limits.h>

#include <map>

#define BICUBIC_MODULE( x ) ( ( x < 0 ) ? ( -1 * x ) : x )
#define BICUBIC_K1( x , a ) ( ( ( a + 2 ) * x * x * x ) - \
  ( ( a + 3 ) * x * x ) + 1 )
#define BICUBIC_K2( x , a ) ( ( a * x * x * x ) - ( 5 * a * x * x ) + \
  ( 8 * a * x ) - ( 4 * a ) )
#define BICUBIC_RANGES(x,a) \
  ( ( ( 0 <= x ) && ( x <= 1 ) ) ? \
    BICUBIC_K1(x,a) \
  : ( ( ( 1 < x ) && ( x <= 2 ) ) ? \
      BICUBIC_K2(x,a) \
    : 0 ) )
#define BICUBIC_KERNEL( x , a ) BICUBIC_RANGES( BICUBIC_MODULE(x) , a )

TePDIMMIOMatching::TePDIMMIOMatching()
: moravecDownSampleFactor_( 1.5 ), moravecUpSampleFactor_( 0.5 ),
moravecRepeatabilityMinTolerance_( 3.0 )
{
}


TePDIMMIOMatching::~TePDIMMIOMatching()
{
}

bool TePDIMMIOMatching::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking input_image1_ptr */
    
  TePDITypes::TePDIRasterPtrType input_image1_ptr;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
    "input_image1_ptr", input_image1_ptr ),
    "Missing parameter: input_image1_ptr" );
  TEAGN_TRUE_OR_RETURN( input_image1_ptr.isActive(),
    "Invalid parameter: input_image1_ptr inactive" );
  TEAGN_TRUE_OR_RETURN( input_image1_ptr->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: input_image1_ptr not ready" );
  
  /* Checking matching method */
  
  FeatMatchingMethod matching_method = TePDIMMIOMatching::NormCrossCorrMethod;
  if( parameters.CheckParameter< FeatMatchingMethod >( 
    "matching_method" ) ) 
  {
    parameters.GetParameter( "matching_method", matching_method );
    
    TEAGN_TRUE_OR_RETURN( 
      ( matching_method == TePDIMMIOMatching::NormCrossCorrMethod ),
      "Invalid parameter : matching_method" );
  }     
    
  /* Checking input_channel1 */
    
  unsigned int input_channel1 = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
    "input_channel1", input_channel1 ),
    "Missing parameter: input_channel1" );
  TEAGN_TRUE_OR_RETURN( 
    ( ( (int)input_channel1 ) < 
    input_image1_ptr->params().nBands() ),
    "Invalid parameter: input_channel1" );
  
    
  /* Checking input_image1_ptr */
    
  TePDITypes::TePDIRasterPtrType input_image2_ptr;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
    "input_image2_ptr", input_image2_ptr ),
    "Missing parameter: input_image2_ptr" );
  TEAGN_TRUE_OR_RETURN( input_image2_ptr.isActive(),
    "Invalid parameter: input_image2_ptr inactive" );
  TEAGN_TRUE_OR_RETURN( input_image2_ptr->params().status_ != 
    TeRasterParams::TeNotReady, 
    "Invalid parameter: input_image2_ptr not ready" );    
    
  /* Checking input_channel2 */
    
  unsigned int input_channel2 = 0;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
    "input_channel2", input_channel2 ),
    "Missing parameter: input_channel2" );
  TEAGN_TRUE_OR_RETURN( 
    ( ( (int)input_channel2 ) < 
    input_image2_ptr->params().nBands() ),
    "Invalid parameter: input_channel2" );
    
  /* Checking out_tie_points_ptr */
    
  TeSharedPtr< TeCoordPairVect > out_tie_points_ptr;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
    "out_tie_points_ptr", out_tie_points_ptr ),
    "Missing parameter: out_tie_points_ptr" );
  TEAGN_TRUE_OR_RETURN( out_tie_points_ptr.isActive(),
    "Invalid parameter: out_tie_points_ptr inactive" );
    
  /* Checking gt_params */
    
  TeGTParams gt_params;
  if( parameters.CheckParameter< TeGTParams >( "gt_params" ) ) {
    parameters.GetParameter( "gt_params", gt_params );
    
    TEAGN_TRUE_OR_RETURN( ( TeGTFactory::instance().find( 
      gt_params.transformation_name_ ) != 
      TeGTFactory::instance().end() ),
      "Invalid parameter : gt_params" )
  }   
  
  /* Checking out_gt_params */
  
  TeGTParams::pointer out_gt_params_ptr;
  if( parameters.CheckParameter< TeGTParams::pointer >( 
    "out_gt_params_ptr" ) ) 
  {
    parameters.GetParameter( "out_gt_params_ptr", out_gt_params_ptr );
    
    TEAGN_TRUE_OR_RETURN( ( out_gt_params_ptr.isActive() ),
      "Invalid parameter : out_gt_params_ptr" )
  }      
    
  /* Checking input_box1 */
    
  TeBox input_box1;
  if( parameters.CheckParameter< TeBox >( "input_box1" ) ) {
    parameters.GetParameter( "input_box1", input_box1 );
    
    TEAGN_TRUE_OR_RETURN( ( input_box1.x1() >= 0 ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( ( input_box1.x2() >= 0 ),
      "Invalid parameter : input_box1" )      
    TEAGN_TRUE_OR_RETURN( ( input_box1.y1() >= 0 ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( ( input_box1.y2() >= 0 ),
      "Invalid parameter : input_box1" )         
      
    TEAGN_TRUE_OR_RETURN( 
      ( input_box1.x1() <= 
        ( input_image1_ptr->params().ncols_ - 1 ) ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_box1.x2() <= 
        ( input_image1_ptr->params().ncols_ - 1 ) ),
      "Invalid parameter : input_box1" )       
      
    TEAGN_TRUE_OR_RETURN( 
      ( input_box1.y1() <= 
        ( input_image1_ptr->params().nlines_ - 1 ) ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_box1.y2() <= 
        ( input_image1_ptr->params().nlines_ - 1 ) ),
      "Invalid parameter : input_box1" )        
      
    TEAGN_TRUE_OR_RETURN( ( input_box1.x1() < input_box1.x2() ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( ( input_box1.y1() > input_box1.y2() ),
      "Invalid parameter : input_box1" )                  
  }
    
  /* Checking input_box2 */
    
  TeBox input_box2;
  if( parameters.CheckParameter< TeBox >( "input_box2" ) ) {
    parameters.GetParameter( "input_box2", input_box2 );
    
    TEAGN_TRUE_OR_RETURN( ( input_box2.x1() >= 0 ),
      "Invalid parameter : input_box2" )
    TEAGN_TRUE_OR_RETURN( ( input_box2.x2() >= 0 ),
      "Invalid parameter : input_box2" )      
    TEAGN_TRUE_OR_RETURN( ( input_box2.y1() >= 0 ),
      "Invalid parameter : input_box2" )
    TEAGN_TRUE_OR_RETURN( ( input_box2.y2() >= 0 ),
      "Invalid parameter : input_box2" )         
      
    TEAGN_TRUE_OR_RETURN( 
      ( input_box2.x1() <= 
        ( input_image2_ptr->params().ncols_ - 1 ) ),
      "Invalid parameter : input_box2" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_box2.x2() <= 
        ( input_image2_ptr->params().ncols_ - 1 ) ),
      "Invalid parameter : input_box2" )       
      
    TEAGN_TRUE_OR_RETURN( 
      ( input_box2.y1() <= 
        ( input_image2_ptr->params().nlines_ - 1 ) ),
      "Invalid parameter : input_box2" )
    TEAGN_TRUE_OR_RETURN( 
      ( input_box2.y2() <= 
        ( input_image2_ptr->params().nlines_ - 1 ) ),
      "Invalid parameter : input_box2" )   
           
    TEAGN_TRUE_OR_RETURN( ( input_box2.x1() < input_box2.x2() ),
      "Invalid parameter : input_box1" )
    TEAGN_TRUE_OR_RETURN( ( input_box2.y1() > input_box2.y2() ),
      "Invalid parameter : input_box1" )               
  }    
    
  /* Checking max_tie_points */
  
  unsigned int max_tie_points = 0;
  if( parameters.CheckParameter< unsigned int >( "max_tie_points" ) ) {
    parameters.GetParameter( "max_tie_points", max_tie_points );
    
    TEAGN_TRUE_OR_RETURN( ( max_tie_points > 0 ),
      "Invalid parameter : max_tie_points" )    
  }
  
  /* Checking corr_window_width */
  
  unsigned int corr_window_width = 0;
  if( parameters.CheckParameter< unsigned int >( 
    "corr_window_width" ) ) {
    
    parameters.GetParameter( "corr_window_width", 
      corr_window_width );
      
    TEAGN_TRUE_OR_RETURN( ( ( corr_window_width % 2 ) > 0 ),
      "Invalid parameter : corr_window_width" )      
    
    TEAGN_TRUE_OR_RETURN( ( corr_window_width >= 13 ),
      "Invalid parameter : corr_window_width" )
  }    
  
  /* Checking moravec_window_width */
  
  unsigned int moravec_window_width = corr_window_width / 2;
  if( parameters.CheckParameter< unsigned int >( 
    "moravec_window_width" ) ) {
    
    parameters.GetParameter( "moravec_window_width", 
      moravec_window_width );
      
    TEAGN_TRUE_OR_RETURN( ( ( moravec_window_width % 2 ) > 0 ),
      "Invalid parameter : moravec_window_width" )      
    
    TEAGN_TRUE_OR_RETURN( ( moravec_window_width >= 11 ),
      "Invalid parameter : corr_window_width" )
  }   
  
  /* Checking pixel_x_relation */
  
  double pixel_x_relation = 1.0;
  if( parameters.CheckParameter< double >( "pixel_x_relation" ) ) {
    parameters.GetParameter( "pixel_x_relation", pixel_x_relation );
    
    TEAGN_TRUE_OR_RETURN( ( pixel_x_relation != 0.0 ),
      "Invalid parameter : pixel_x_relation" )
  }
  
  /* Checking pixel_y_relation */
  
  double pixel_y_relation = 1.0;
  if( parameters.CheckParameter< double >( "pixel_y_relation" ) ) {
    parameters.GetParameter( "pixel_y_relation", pixel_y_relation );
    
    TEAGN_TRUE_OR_RETURN( ( pixel_y_relation != 0.0 ),
      "Invalid parameter : pixel_y_relation" )
  }
  
  TeSharedPtr< std::vector< double > > out_tie_points_weights_ptr;
  if( parameters.CheckParameter< TeSharedPtr< std::vector< double > > >( 
    "out_tie_points_weights_ptr" ) ) {
    parameters.GetParameter( "out_tie_points_weights_ptr", 
      out_tie_points_weights_ptr );
    
    TEAGN_TRUE_OR_RETURN( out_tie_points_weights_ptr.isActive(),
      "Invalid parameter : out_tie_points_weights_ptr" )
  }
  
  /* Checking mask_image1_ptr */
  
  TePDITypes::TePDIRasterPtrType mask_image1_ptr;
  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "mask_image1_ptr" ) ) 
  {
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
      "mask_image1_ptr", mask_image1_ptr ),
      "Missing parameter: mask_image1_ptr" );
    TEAGN_TRUE_OR_RETURN( mask_image1_ptr.isActive(),
      "Invalid parameter: mask_image1_ptr inactive" );
    TEAGN_TRUE_OR_RETURN( mask_image1_ptr->params().status_ != 
      TeRasterParams::TeNotReady, 
      "Invalid parameter: mask_image1_ptr not ready" );  
    TEAGN_TRUE_OR_RETURN( mask_image1_ptr->params().nlines_ == 
      input_image1_ptr->params().nlines_,
      "Invalid parameter: mask_image1_ptr" );      
    TEAGN_TRUE_OR_RETURN( mask_image1_ptr->params().ncols_ == 
      input_image1_ptr->params().ncols_,
      "Invalid parameter: mask_image1_ptr" );        
  }
  
  /* Checking mask_image2_ptr */
  
  TePDITypes::TePDIRasterPtrType mask_image2_ptr;
  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "mask_image2_ptr" ) ) 
  {
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( 
      "mask_image2_ptr", mask_image2_ptr ),
      "Missing parameter: mask_image2_ptr" );
    TEAGN_TRUE_OR_RETURN( mask_image2_ptr.isActive(),
      "Invalid parameter: mask_image2_ptr inactive" );
    TEAGN_TRUE_OR_RETURN( mask_image2_ptr->params().status_ != 
      TeRasterParams::TeNotReady, 
      "Invalid parameter: mask_image2_ptr not ready" );  
    TEAGN_TRUE_OR_RETURN( mask_image2_ptr->params().nlines_ == 
      input_image2_ptr->params().nlines_,
      "Invalid parameter: mask_image2_ptr" );      
    TEAGN_TRUE_OR_RETURN( mask_image2_ptr->params().ncols_ == 
      input_image2_ptr->params().ncols_,
      "Invalid parameter: mask_image2_ptr" );        
  }  
  
  return true;
}

bool TePDIMMIOMatching::RunImplementation()
{
  /* Retriving Parameters */
  
  FeatMatchingMethod matching_method = NormCrossCorrMethod;
  if( params_.CheckParameter< FeatMatchingMethod >( 
    "matching_method" ) ) 
  {
    params_.GetParameter( "matching_method", matching_method );
  }     
  
  TePDITypes::TePDIRasterPtrType input_image1_ptr;
  params_.GetParameter( "input_image1_ptr", input_image1_ptr ); 
  
  unsigned int input_channel1 = 0;
  params_.GetParameter( "input_channel1", input_channel1 );
  
  TePDITypes::TePDIRasterPtrType input_image2_ptr;
  params_.GetParameter( "input_image2_ptr", input_image2_ptr ); 
  
  unsigned int input_channel2 = 0;
  params_.GetParameter( "input_channel2", input_channel2 );  
    
  TeBox input_box1;
  if( params_.CheckParameter< TeBox >( "input_box1" ) ) {
    params_.GetParameter( "input_box1", input_box1 );
  } else {
    TeBox input_box1_proj = input_image1_ptr->params().box();
    
    TePDIUtils::MapCoords2RasterIndexes( input_box1_proj, input_image1_ptr,
      input_box1 );
  }
  
  TeBox input_box2;
  if( params_.CheckParameter< TeBox >( "input_box2" ) ) {
    params_.GetParameter( "input_box2", input_box2 );
  } else {
    TeBox input_box2_proj = input_image2_ptr->params().box();
    
    TePDIUtils::MapCoords2RasterIndexes( input_box2_proj, input_image2_ptr,
      input_box2 );
  }         
    
  TeSharedPtr< TeCoordPairVect > out_tie_points_ptr;
  params_.GetParameter( "out_tie_points_ptr", out_tie_points_ptr );
  
  double pixel_x_relation = 1.0;
  if( params_.CheckParameter< double >( "pixel_x_relation" ) ) {
    params_.GetParameter( "pixel_x_relation", pixel_x_relation );
  }
  
  double pixel_y_relation = 1.0;
  if( params_.CheckParameter< double >( "pixel_y_relation" ) ) {
    params_.GetParameter( "pixel_y_relation", pixel_y_relation );
  }    
  
  TeGTParams gt_params;
  gt_params.out_rem_strat_ = TeGTParams::RANSACRemotion;
  gt_params.max_dmap_error_ = 2.0;
  gt_params.max_imap_error_ = gt_params.max_dmap_error_ /
    MIN( pixel_x_relation, pixel_y_relation );
  gt_params.transformation_name_ = "affine";
  if( params_.CheckParameter< TeGTParams >( "gt_params" ) ) {
    params_.GetParameter( "gt_params", gt_params );
  }
  
  bool enable_multi_thread = false;
  if( params_.CheckParameter< int >( "enable_multi_thread" ) ) {
    enable_multi_thread = true;
  }
  
  bool enable_threaded_raster_read = false;
  if( params_.CheckParameter< int >( "enable_threaded_raster_read" ) ) {
    enable_threaded_raster_read = true;
  }  
  
  bool skip_geom_filter = false;
  if( params_.CheckParameter< int >( "skip_geom_filter" ) ) {
    skip_geom_filter = true;
  }  
  
  unsigned int max_tie_points = 1000;
  if( params_.CheckParameter< unsigned int >( "max_tie_points" ) ) {
    params_.GetParameter( "max_tie_points", max_tie_points );
  }    
  
  unsigned int corr_window_width = 21;
  if( params_.CheckParameter< unsigned int >( 
    "corr_window_width" ) ) {
    
    params_.GetParameter( "corr_window_width", corr_window_width );
  }    
  
  unsigned int moravec_window_width = 11;
  if( params_.CheckParameter< unsigned int >( "moravec_window_width" ) ) 
  {
    params_.GetParameter( "moravec_window_width", 
      moravec_window_width );
  }     
  
  TeSharedPtr< std::vector< double > > out_tie_points_weights_ptr;
  if( params_.CheckParameter< TeSharedPtr< std::vector< double > > >( 
    "out_tie_points_weights_ptr" ) ) 
  {
    params_.GetParameter( "out_tie_points_weights_ptr", 
      out_tie_points_weights_ptr );
  }
  else
  {
    out_tie_points_weights_ptr.reset( new std::vector< double > );
  } 
  
  bool enableMoravecDownsampledFilter = true;
  bool enableMoravecUpsampledFilter = false;
  if( params_.CheckParameter< int >( "disable_reap_filter" ) ) 
  {
    enableMoravecDownsampledFilter = false;
  }
  else
  {
    if( params_.CheckParameter< int >( "enable_upsampled_filter" ) ) {
      enableMoravecUpsampledFilter = true;
    } 
  }
    
  TePDITypes::TePDIRasterPtrType mask_image1_ptr;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "mask_image1_ptr" ) ) 
  {
    params_.GetParameter( "mask_image1_ptr", mask_image1_ptr );    
  }
  
  TePDITypes::TePDIRasterPtrType mask_image2_ptr;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "mask_image2_ptr" ) ) 
  {
    params_.GetParameter( "mask_image2_ptr", mask_image2_ptr );    
  }  
  
  bool enableSubImageSearch = true;
  if( params_.CheckParameter< int >( "disable_subimage_search" ) ) 
  {
    enableSubImageSearch = false;
  }    
  
  TePDITypes::TePDIRasterPtrType out_maximas_image1_ptr;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "out_maximas_image1_ptr" ) ) 
  {
    params_.GetParameter( "out_maximas_image1_ptr", out_maximas_image1_ptr );    
  }  
  
  TePDITypes::TePDIRasterPtrType out_maximas_image2_ptr;
  if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "out_maximas_image2_ptr" ) ) 
  {
    params_.GetParameter( "out_maximas_image2_ptr", out_maximas_image2_ptr );    
  }    
  
  // The original boxes postion over the original images
  
  const unsigned int orig_box1_x_off = (unsigned int)
    input_box1.lowerLeft().x();
  const unsigned int orig_box1_y_off = (unsigned int)
    input_box1.upperRight().y();
  const unsigned int orig_box1_nlines = 1 + (unsigned int)
    ABS( input_box1.height() );
  const unsigned int orig_box1_ncols = 1 + (unsigned int)
    ABS( input_box1.width() );
  
  const unsigned int orig_box2_x_off = (unsigned int)
    input_box2.lowerLeft().x();
  const unsigned int orig_box2_y_off = (unsigned int)
    input_box2.upperRight().y();
  const unsigned int orig_box2_nlines = 1 + (unsigned int)
    ABS( input_box2.height() );
  const unsigned int orig_box2_ncols = 1 + (unsigned int)
    ABS( input_box2.width() );  
  
  /* Calculating the rescale factors 
     factor = rescaled_orignal_image / original_image */
  
  double i1XRescFact = 1.0;
  double i1YRescFact = 1.0;
  double i2XRescFact = 1.0;
  double i2YRescFact = 1.0;
  {
    double mean_pixel_relation = ( pixel_x_relation + pixel_y_relation ) /
      2.0;
      
    if( mean_pixel_relation > 1.0 ) {
      /* The image 1 has poor resolution - bigger pixel resolution values -
         and image 2 needs to be rescaled down */
      
      i2XRescFact = 1.0 / pixel_x_relation;
      i2YRescFact = 1.0 / pixel_y_relation;
    } else if( mean_pixel_relation < 1.0 ) {
      /* The image 2 has poor resolution - bigger pixel resolution values
        and image 1 needs to be rescaled down */
      
      i1XRescFact = pixel_x_relation;
      i1YRescFact = pixel_y_relation;
    }
  }   
  
  /* Calculating the max tie-points to be generated for each image
     trying to keep the same tie-point density for both images */
     
  unsigned int img1MaxTps = max_tie_points;
  unsigned int img2MaxTps = max_tie_points;
  
  {
    double resc_box1_nlines =  ( (double)orig_box1_nlines  ) * 
      i1YRescFact;
    double resc_box1_ncols =  ( (double)orig_box1_ncols ) * 
      i1XRescFact;
    double resc_box2_nlines =  ( (double)orig_box2_nlines  ) * 
      i2YRescFact;
    double resc_box2_ncols =  ( (double)orig_box2_ncols ) * 
      i2XRescFact;
      
    double resc_box1_area = resc_box1_nlines * resc_box1_ncols;
    double resc_box2_area = resc_box2_nlines * resc_box2_ncols;
      
    if( resc_box1_area > resc_box2_area )
    {
      img1MaxTps = (unsigned int)( 
        resc_box1_area / 
        ( resc_box2_area / ( (double)max_tie_points ) ) );
    }
    else if( resc_box1_area < resc_box2_area )
    {
      img2MaxTps = (unsigned int)( 
        resc_box2_area / 
        ( resc_box1_area / ( (double)max_tie_points ) ) ); 
    }
  }
  
  // Moravec specific variables

  unsigned int moravecDSWindowSize = (unsigned int)( 
    moravec_window_width / moravecDownSampleFactor_ );
  if( ( moravecDSWindowSize % 2 ) == 0 ) ++moravecDSWindowSize;
  TEAGN_DEBUG_CONDITION( moravecDSWindowSize > 2, 
    "Invalid moravecDSWindowSize ")

  unsigned int moravecUSWindowSize = (unsigned int)( 
    moravec_window_width / moravecUpSampleFactor_ );
  if( ( moravecUSWindowSize % 2 ) == 0 ) ++moravecUSWindowSize;
  TEAGN_DEBUG_CONDITION( moravecUSWindowSize > 2, 
    "Invalid moravecUSWindowSize ")
       
  /* Calculating the number of allowed threads - this process
     not included into the count */
  
  if( enable_multi_thread )
  {
    jobsMan_.setAutoMaxSimulJobs();
  }
  else
  {
    jobsMan_.setMaxSimulJobs( 0 );
  }
  
 /* Loading images */
 
  TePDIPIManager progress( "Matching", 6, progress_enabled_ ); 
 
//jobsMan_.setMaxSimulJobs( 0 );   
//jobsMan_.setAutoMaxSimulJobs();
  
  TeSharedPtr< TePDIMtxDoubleAdptInt > img1_matrix_ptr;
  TeSharedPtr< TePDIMtxDoubleAdptInt > img1_matrix_ds_ptr; // Downsampled version
  TeSharedPtr< TePDIMtxDoubleAdptInt > img1_matrix_us_ptr; // Upsampled version  
  
  MaskMatrixT mask_matrix1;
  mask_matrix1.Reset( MaskMatrixT::AutoMemPol );

  {
    createImgMatrix( input_image1_ptr->params().dataType_[input_channel1],
      img1_matrix_ptr );
    if( enableMoravecDownsampledFilter )
    {
      createImgMatrix( input_image1_ptr->params().dataType_[input_channel1],
        img1_matrix_ds_ptr );
    }
    if( enableMoravecUpsampledFilter )
    {
      createImgMatrix( input_image1_ptr->params().dataType_[input_channel1],
        img1_matrix_us_ptr );
    }
  }
  
  TeSharedPtr< TePDIMtxDoubleAdptInt > img2_matrix_ptr;
  TeSharedPtr< TePDIMtxDoubleAdptInt > img2_matrix_ds_ptr; // Downsampled version
  TeSharedPtr< TePDIMtxDoubleAdptInt > img2_matrix_us_ptr; // Upsampled version
  
  MaskMatrixT mask_matrix2;
  mask_matrix2.Reset( MaskMatrixT::AutoMemPol );

  {
    createImgMatrix( input_image2_ptr->params().dataType_[input_channel2],
      img2_matrix_ptr );
    if( enableMoravecDownsampledFilter )
    {
      createImgMatrix( input_image2_ptr->params().dataType_[input_channel2],
        img2_matrix_ds_ptr );
    }
    if( enableMoravecUpsampledFilter )
    {
      createImgMatrix( input_image2_ptr->params().dataType_[input_channel2],
        img2_matrix_us_ptr );  
    }
  }
 
  {
    LoadImageParams img1pars;
    
    img1pars.progress_enabled_ = progress_enabled_;
    img1pars.glb_mem_lock_ptr_ = &globalMutex_;
    img1pars.moravecDownSampleFactor_ = moravecDownSampleFactor_;
    img1pars.moravecUpSampleFactor_ = moravecUpSampleFactor_;
    img1pars.returnValue_ = true;
    img1pars.generateDownsampledImage_ = enableMoravecDownsampledFilter;
    img1pars.generateUpsampledImage_ = enableMoravecUpsampledFilter;    
    
    img1pars.input_image_ptr_ = input_image1_ptr;
    img1pars.mask_image_ptr_ = mask_image1_ptr;
    img1pars.input_image_channel_ = input_channel1;
    img1pars.img_matrix_ptr_ = img1_matrix_ptr.nakedPointer();
    img1pars.img_matrix_ds_ptr_ = img1_matrix_ds_ptr.nakedPointer();
    img1pars.img_matrix_us_ptr_ = img1_matrix_us_ptr.nakedPointer();
    img1pars.mask_matrix_ptr_ = &mask_matrix1;
    img1pars.in_box_x_off_ = orig_box1_x_off;
    img1pars.in_box_y_off_ = orig_box1_y_off;
    img1pars.in_box_nlines_ = orig_box1_nlines;
    img1pars.in_box_ncols_ = orig_box1_ncols;
    img1pars.origImgXRescFact_ = i1XRescFact;
    img1pars.origImgYRescFact_ = i1YRescFact;
    
    LoadImageParams img2pars = img1pars;
    img2pars.input_image_ptr_ = input_image2_ptr;
    img2pars.mask_image_ptr_ = mask_image2_ptr;
    img2pars.input_image_channel_ = input_channel2;
    img2pars.img_matrix_ptr_ = img2_matrix_ptr.nakedPointer();
    img2pars.img_matrix_ds_ptr_ = img2_matrix_ds_ptr.nakedPointer();
    img2pars.img_matrix_us_ptr_ = img2_matrix_us_ptr.nakedPointer();
    img2pars.mask_matrix_ptr_ = &mask_matrix2;
    img2pars.in_box_x_off_ = orig_box2_x_off;
    img2pars.in_box_y_off_ = orig_box2_y_off;
    img2pars.in_box_nlines_ = orig_box2_nlines;
    img2pars.in_box_ncols_ = orig_box2_ncols;    
    img2pars.origImgXRescFact_ = i2XRescFact;
    img2pars.origImgYRescFact_ = i2YRescFact;
    
    if( enable_threaded_raster_read && enable_multi_thread )
    {
      TeFunctionCallThreadJob job1;
      job1.jobFunc_ = loadImage;
      job1.jobFuncParsPtr_ = (void*)&img1pars;
      jobsMan_.executeJob( job1 );
  
      TeFunctionCallThreadJob job2;
      job2.jobFunc_ = loadImage;
      job2.jobFuncParsPtr_ = (void*)&img2pars;
      jobsMan_.executeJob( job2 );
      
      jobsMan_.waitAllToFinish();
    }
    else
    {
      loadImage( (void*)&img1pars );
      loadImage( (void*)&img2pars );
    }
    
    TEAGN_TRUE_OR_RETURN( img1pars.returnValue_, "Error loading image" )
    TEAGN_TRUE_OR_RETURN( img2pars.returnValue_, "Error loading image" )
  }  

/* 
matrix2Tiff( *img1_matrix_ptr, "img1_matrix_ptr_loaded.tif" );
matrix2Tiff( *img1_matrix_ds_ptr, "img1_matrix_ds_ptr_loaded.tif" );
matrix2Tiff( *img1_matrix_us_ptr, "img1_matrix_us_ptr_loaded.tif" );
matrix2Tiff( *img2_matrix_ptr, "img2_matrix_ptr_loaded.tif" );
matrix2Tiff( *img2_matrix_ds_ptr, "img2_matrix_ds_ptr_loaded.tif" );
matrix2Tiff( *img2_matrix_us_ptr, "img2_matrix_us_ptr_loaded.tif" );
*/

  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
  
  // Extracting maximas from all images
  
//jobsMan_.setMaxSimulJobs( 0 );  
//jobsMan_.setAutoMaxSimulJobs();
  MaximasListT img1_maximas_list;
  MaximasListT img1ds_maximas_list;
  MaximasListT img1us_maximas_list;
  MaximasListT img2_maximas_list;
  MaximasListT img2ds_maximas_list;
  MaximasListT img2us_maximas_list;
  
  {
    // Creating job execution parameters 
    
    TeFunctionCallThreadJob dummyJob;
    dummyJob.jobFunc_ = extractLocalMaximas;
    
    ExtractLocalMaximasParams img1pars;
    img1pars.seachSubImagesNmb_ = ( enableSubImageSearch ? 9 : 1 );
    img1pars.returnValue_ = true;
    img1pars.progressEnabled_ = progress_enabled_;
    img1pars.glbMutexptr_ = &globalMutex_;
        
    img1pars.maximasNumber_ = img1MaxTps;    
    img1pars.imgMatrixPtr_ = img1_matrix_ptr.nakedPointer();
    img1pars.outMaximasListPtr_ = &img1_maximas_list;
    img1pars.moravecWindowSize_ = moravec_window_width;
    img1pars.maskMatrixPtr_ = ( mask_image1_ptr.isActive() ? (&mask_matrix1) :
      0 );
    
    ExtractLocalMaximasParams img1dspars = img1pars;
    img1dspars.imgMatrixPtr_ = img1_matrix_ds_ptr.nakedPointer();
    img1dspars.outMaximasListPtr_ = &img1ds_maximas_list;
    img1dspars.moravecWindowSize_ = moravecDSWindowSize;
    img1dspars.maskMatrixPtr_ = 0;

    ExtractLocalMaximasParams img1uspars = img1pars;
    img1uspars.imgMatrixPtr_ = img1_matrix_us_ptr.nakedPointer();
    img1uspars.outMaximasListPtr_ = &img1us_maximas_list;
    img1uspars.moravecWindowSize_ = moravecUSWindowSize;
    img1uspars.maskMatrixPtr_ = 0;
    
    ExtractLocalMaximasParams img2pars = img1pars;
    img2pars.maximasNumber_ = img2MaxTps;
    img2pars.imgMatrixPtr_ = img2_matrix_ptr.nakedPointer();
    img2pars.outMaximasListPtr_ = &img2_maximas_list;
    img2pars.maskMatrixPtr_ = ( mask_image2_ptr.isActive() ? (&mask_matrix2) :
      0 );
    
    ExtractLocalMaximasParams img2dspars = img2pars;
    img2dspars.imgMatrixPtr_ = img2_matrix_ds_ptr.nakedPointer();
    img2dspars.outMaximasListPtr_ = &img2ds_maximas_list;
    img2dspars.moravecWindowSize_ = moravecDSWindowSize;
    img2dspars.maskMatrixPtr_ = 0;

    ExtractLocalMaximasParams img2uspars = img2pars;
    img2uspars.imgMatrixPtr_ = img2_matrix_us_ptr.nakedPointer();
    img2uspars.outMaximasListPtr_ = &img2us_maximas_list;
    img2uspars.moravecWindowSize_ = moravecUSWindowSize;
    img2uspars.maskMatrixPtr_ = 0;
    
    // Starting jobs
    
    dummyJob.jobFuncParsPtr_ = (void*)&img1pars;
    jobsMan_.executeJob( dummyJob );     
    
    if( enableMoravecDownsampledFilter )
    {
      dummyJob.jobFuncParsPtr_ = (void*)&img1dspars;
      jobsMan_.executeJob( dummyJob ); 
    }
    
    if( enableMoravecUpsampledFilter )
    {
      dummyJob.jobFuncParsPtr_ = (void*)&img1uspars;
      jobsMan_.executeJob( dummyJob ); 
    }
  
    dummyJob.jobFuncParsPtr_ = (void*)&img2pars;
    jobsMan_.executeJob( dummyJob );     
    
    if( enableMoravecDownsampledFilter )
    {
      dummyJob.jobFuncParsPtr_ = (void*)&img2dspars;
      jobsMan_.executeJob( dummyJob );   
    }
    
    if( enableMoravecUpsampledFilter )
    {
      dummyJob.jobFuncParsPtr_ = (void*)&img2uspars;
      jobsMan_.executeJob( dummyJob );   
    }
    
    // Waiting all jobs to finish
    
    jobsMan_.waitAllToFinish();
    
    TEAGN_TRUE_OR_RETURN( img1pars.returnValue_, 
      "Error loocking for maximas" )
    TEAGN_TRUE_OR_RETURN( img1dspars.returnValue_,
      "Error loocking for maximas" )
    TEAGN_TRUE_OR_RETURN( img1uspars.returnValue_, 
      "Error loocking for maximas" )
    TEAGN_TRUE_OR_RETURN( img2pars.returnValue_, 
      "Error loocking for maximas" )
    TEAGN_TRUE_OR_RETURN( img2dspars.returnValue_, 
      "Error loocking for maximas" )
    TEAGN_TRUE_OR_RETURN( img2uspars.returnValue_, 
      "Error loocking for maximas" )
  }
  

//matrix2Tiff( *img1_matrix_ptr, "img1_matrix_ptr_max.tif",
//  img1_maximas_list );
//matrix2Tiff( *img1_matrix_ds_ptr, "img1_matrix_ds_ptr_max.tif",
//  img1ds_maximas_list );
//matrix2Tiff( *img1_matrix_us_ptr, "img1_matrix_us_ptr_max.tif",
//  img1us_maximas_list );
//matrix2Tiff( *img2_matrix_ptr, "img2_matrix_ptr_max.tif",
//  img2_maximas_list );
//matrix2Tiff( *img2_matrix_ds_ptr, "img2_matrix_ds_ptr_max.tif",
//  img2ds_maximas_list );
//matrix2Tiff( *img2_matrix_us_ptr, "img2_matrix_us_ptr_max.tif",
//  img2us_maximas_list );  

  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );

  // Applying moravec repeatability filter 
  
//jobsMan_.setMaxSimulJobs( 0 );  
//jobsMan_.setAutoMaxSimulJobs();
    
  if( enableMoravecDownsampledFilter )
  {
    MaximasListT auxList;
    
    moravecRepeatability( img1_maximas_list, img1ds_maximas_list,
      img1_matrix_ds_ptr->getNCols(),
      img1_matrix_ds_ptr->getNLines(), 
      moravecRepeatabilityMinTolerance_ / moravecDownSampleFactor_, 
      moravecDownSampleFactor_, auxList );
      
    img1_maximas_list = auxList;
  }
  
  if( enableMoravecUpsampledFilter )
  {
    MaximasListT auxList; 
    
    moravecRepeatability( img1_maximas_list, img1us_maximas_list,
      img1_matrix_us_ptr->getNCols(),
      img1_matrix_us_ptr->getNLines(), 
      moravecRepeatabilityMinTolerance_ / moravecUpSampleFactor_, 
      moravecUpSampleFactor_, auxList );  
        
    img1_maximas_list = auxList;
  }
  
  if( enableMoravecDownsampledFilter )
  {
    MaximasListT auxList;
    
    moravecRepeatability( img2_maximas_list, img2ds_maximas_list,
      img2_matrix_ds_ptr->getNCols(),
      img2_matrix_ds_ptr->getNLines(), 
      moravecRepeatabilityMinTolerance_ / moravecDownSampleFactor_, 
      moravecDownSampleFactor_, auxList );
      
    img2_maximas_list = auxList;
  }
    
  if( enableMoravecUpsampledFilter )
  {
    MaximasListT auxList;
    
    moravecRepeatability( img2_maximas_list, img2us_maximas_list,
      img2_matrix_us_ptr->getNCols(),
      img2_matrix_us_ptr->getNLines(), 
      moravecRepeatabilityMinTolerance_ / moravecUpSampleFactor_, 
      moravecUpSampleFactor_, auxList );
      
    img2_maximas_list = auxList;
  }

/*  
matrix2Tiff( *img1_matrix_ptr, "img1_matrix_ptr_max_repeat.tif",
  img1_maximas_list );
matrix2Tiff( *img2_matrix_ptr, "img2_matrix_ptr_max_repeat.tif",
  img2_maximas_list );
*/
  
  img1us_maximas_list.clear();
  img1ds_maximas_list.clear();  
  img2us_maximas_list.clear();
  img2ds_maximas_list.clear();
  
  img1_matrix_ds_ptr.reset();
  img1_matrix_us_ptr.reset();
  img2_matrix_ds_ptr.reset();
  img2_matrix_us_ptr.reset();  
  
  // filtering out points closer to images borders to avoid
  // correlation windows generation errors 

  
  {
    /* The radius of a feature window rotated by 90 degrees. 
     * over the input image */
    const unsigned int winRadius = (unsigned int)
      (
        ceil( 
          sqrt( 
            2 
            * 
            ( 
              ( (double)corr_window_width ) 
              *
              ( (double)corr_window_width )
            )
          ) / 2.0
        )
      );
    
    unsigned int img1FistValidLine = winRadius;
    unsigned int img1FistValidCol = winRadius;
    unsigned int img2FistValidLine = winRadius;
    unsigned int img2FistValidCol = winRadius;
    
    // Removing maximas outside the image 1 allowed bondaries
    
    {
      unsigned int img1LinesBound = img1_matrix_ptr->getNLines() - winRadius;
      unsigned int img1ColsBound = img1_matrix_ptr->getNCols() - winRadius;
    
      MaximasListT::iterator maxIt = img1_maximas_list.begin();
      MaximasListT::iterator maxItEnd = img1_maximas_list.end();
      MaximasListT::iterator maxAuxIt;
      
      while( maxIt != maxItEnd )
      {
        if( ( maxIt->x_ < img1FistValidCol ) ||
          ( maxIt->x_ >= img1ColsBound ) || 
          ( maxIt->y_ < img1FistValidLine ) ||
          ( maxIt->y_ >= img1LinesBound ) )
        {
          maxAuxIt = maxIt;
          ++maxIt;
          img1_maximas_list.erase( maxAuxIt );
        }
        else
        {
          ++maxIt;
        }
      }
    }
    
    // Removing maximas outside the image 2 allowed bondaries
    
    {
      unsigned int img2LinesBound = img2_matrix_ptr->getNLines() - winRadius;
      unsigned int img2ColsBound = img2_matrix_ptr->getNCols() - winRadius;
    
      MaximasListT::iterator maxIt = img2_maximas_list.begin();
      MaximasListT::iterator maxItEnd = img2_maximas_list.end();
      MaximasListT::iterator maxAuxIt;
      
      while( maxIt != maxItEnd )
      {
        if( ( maxIt->x_ < img2FistValidCol ) ||
          ( maxIt->x_ >= img2ColsBound ) || 
          ( maxIt->y_ < img2FistValidLine ) ||
          ( maxIt->y_ >= img2LinesBound ) )
        {
          maxAuxIt = maxIt;
          ++maxIt;
          img2_maximas_list.erase( maxAuxIt );
        }
        else
        {
          ++maxIt;
        }
      }
    }
  }
  
/*
matrix2Tiff( *img1_matrix_ptr, "img1_matrix_ptr_max_outremoved.tif",
  img1_maximas_list );
matrix2Tiff( *img2_matrix_ptr, "img2_matrix_ptr_max_outremoved.tif",
  img2_maximas_list );  
*/

  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
  
  // Generating the output maximas rasters
  
  if( out_maximas_image1_ptr.isActive() )
  {
    TEAGN_TRUE_OR_RETURN( generateMaximasRaster( *img1_matrix_ptr,
      img1_maximas_list, *out_maximas_image1_ptr ),
      "Error generating the maximas image 1" )
  }
  
  if( out_maximas_image2_ptr.isActive() )
  {
    TEAGN_TRUE_OR_RETURN( generateMaximasRaster( *img2_matrix_ptr,
      img2_maximas_list, *out_maximas_image2_ptr ),
      "Error generating the maximas image 2" )
  }   
  
  /* Generating features matrix */
  
  TePDIMatrix< double > img1featMtx;
  img1featMtx.Reset( TePDIMatrix< double >::RAMMemPol );
  
  TePDIMatrix< double > img2featMtx;  
  img2featMtx.Reset( TePDIMatrix< double >::RAMMemPol );
  
  {
    GenerateCorrelationFeaturesParams params1;
    
    switch( matching_method )
    {
      case NormCrossCorrMethod :
      {
        params1.normalizeWindows_ = true;
        break;
      }
      default :
      {
        TEAGN_LOG_AND_THROW( "Invalid correlation method" );
        break;
      }
    }
    
    params1.imgMatrixPtr_ = img1_matrix_ptr.nakedPointer();
    params1.inMaximasListPtr_ = &img1_maximas_list;
    params1.featMtxPtr_ = &img1featMtx;
    params1.returnValue_ = true;
    params1.glbMemLockPtr_ = &globalMutex_;
    params1.corrWindowsWidth_ = corr_window_width;
    params1.progressEnabled_ = progress_enabled_;
    
    GenerateCorrelationFeaturesParams params2 = params1;
    params2.imgMatrixPtr_ = img2_matrix_ptr.nakedPointer();
    params2.inMaximasListPtr_ = &img2_maximas_list;
    params2.featMtxPtr_ = &img2featMtx;
    
    // Starting jobs
    
    TeFunctionCallThreadJob dummyJob;
    dummyJob.jobFunc_ = generateCorrelationFeatures;
   
    // Starting jobs
    
    dummyJob.jobFuncParsPtr_ = (void*)&params1;
    jobsMan_.executeJob( dummyJob );     
    
    dummyJob.jobFuncParsPtr_ = (void*)&params2;
    jobsMan_.executeJob( dummyJob );       
    
    // Waiting all jobs to finish
    
    jobsMan_.waitAllToFinish();
    
    TEAGN_TRUE_OR_RETURN( params1.returnValue_, 
      "Error generating correlation features" )
    TEAGN_TRUE_OR_RETURN( params2.returnValue_,
      "Error generating correlation features" )    
  }
  
/*  
features2Tiff( corr_window_width, img1_maximas_list, img1featMtx, 
  "img1feat" );
features2Tiff( corr_window_width, img2_maximas_list, img2featMtx, 
  "img2feat" ); 
*/

  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
  
  // Generating tie-points (images boxes reference)
  
  TEAGN_TRUE_OR_RETURN( matchFeatures( matching_method, img1featMtx,
    img1_maximas_list, img2featMtx, img2_maximas_list, 
    *out_tie_points_ptr, *out_tie_points_weights_ptr
    ), "Error generating tie-points" );
  
/*  
matrix2Tiff( *img1_matrix_ptr, "img1_matrix_ptr_tps.tif",
  *out_tie_points_ptr, true );
matrix2Tiff( *img2_matrix_ptr, "img2_matrix_ptr_tps.tif",
  *out_tie_points_ptr, false );      
*/  

  // Clean anused resources
  
  img1_matrix_ptr.reset();
  img2_matrix_ptr.reset();
  
  img1featMtx.Reset( 0, 0 );
  img2featMtx.Reset( 0, 0 );
  
  img1_matrix_ds_ptr.reset();
  img1_matrix_us_ptr.reset();  
    
  // Bring tie-points to original images reference
    
  {
    for( unsigned int tpsVecIdx = 0 ; tpsVecIdx < 
      out_tie_points_ptr->size() ; ++tpsVecIdx )
    {
      TeCoordPair& coordPair = out_tie_points_ptr->operator[]( tpsVecIdx );
      
      coordPair.pt1.x( ( coordPair.pt1.x() / i1XRescFact ) + (double)orig_box1_x_off );
      coordPair.pt1.y( ( coordPair.pt1.y() / i1YRescFact ) + (double)orig_box1_y_off );
      
      coordPair.pt2.x( ( coordPair.pt2.x() / i2XRescFact ) + (double)orig_box2_x_off );
      coordPair.pt2.y( ( coordPair.pt2.y() / i2YRescFact ) + (double)orig_box2_y_off );
    }
  }
  
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
  
  /* Doing geometrical filtering using user supplied 
     geometrical transformation parameters */
     
  if( ! skip_geom_filter ) {
    gt_params.tiepoints_ = (*out_tie_points_ptr);
    gt_params.tiePointsWeights_ = (*out_tie_points_weights_ptr);
       
    TeGeometricTransformation::pointer trans_ptr( 
      TeGTFactory::make( gt_params ) );
    if( trans_ptr->reset( gt_params ) ) {
      trans_ptr->getParameters( gt_params );
      
      /* updating the output tie points */
      
      (*out_tie_points_ptr) = gt_params.tiepoints_;
      
      // Updating the output tie points weights
      
      (*out_tie_points_weights_ptr) = gt_params.tiePointsWeights_;
      
      /* updating the output gt_params, if available */
      
      TeGTParams::pointer out_gt_params_ptr;
      if( params_.CheckParameter< TeGTParams::pointer >( 
        "out_gt_params_ptr" ) ) 
      {
        params_.GetParameter( "out_gt_params_ptr", out_gt_params_ptr );
        
        *out_gt_params_ptr = gt_params;
      }       
    } else {
      out_tie_points_ptr->clear();
    }
  }
  
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by user" );
  
  // So far.... so good !!
  
  return true;
}


void TePDIMMIOMatching::ResetState( const TePDIParameters& )
{
}


void TePDIMMIOMatching::loadImage( void * paramsPtr )
{
 /* Retriving parameters */
  
  LoadImageParams& params = *( (LoadImageParams*) paramsPtr );
  
  params.returnValue_ = false;
  
  // Loading original image box
  
  const unsigned int origBoxRescNLines = (unsigned int)(
    params.in_box_nlines_ * params.origImgYRescFact_ );
  const unsigned int origBoxRescNCols = (unsigned int)(
    params.in_box_ncols_ * params.origImgXRescFact_ );
  
  {
    // Allocating resources 
    
    params.glb_mem_lock_ptr_->lock();
    
    if( ! params.img_matrix_ptr_->reset( origBoxRescNLines, 
      origBoxRescNCols ) )
    {
      TEAGN_LOGERR( "Unable to allocate space for the loaded image" )
      params.glb_mem_lock_ptr_->unLock();
      return;
    }
    
    bool useMaskImage = false;
    if( params.mask_image_ptr_.isActive() )
    {
      useMaskImage = true;
      
      TEAGN_DEBUG_CONDITION( params.mask_matrix_ptr_, 
        "Invalid mask_matrix_ptr_" );
      TEAGN_DEBUG_CONDITION( params.mask_image_ptr_->params().nlines_ ==
        params.input_image_ptr_->params().nlines_, "invalid mask raster lines" )
      TEAGN_DEBUG_CONDITION( params.mask_image_ptr_->params().ncols_ ==
        params.input_image_ptr_->params().ncols_, "invalid mask raster cols" )
        
      if( ! params.mask_matrix_ptr_->Reset( origBoxRescNLines, origBoxRescNCols ) )
      {
        TEAGN_LOGERR( "Unable to allocate space for the loaded mask image" )
        params.glb_mem_lock_ptr_->unLock();
        return;
      }
    }    
      
    params.glb_mem_lock_ptr_->unLock();
    
    // Resampling image and mask

    unsigned int curr_out_line = 0;
    unsigned int curr_out_col = 0;
    unsigned int curr_input_line = 0;
    unsigned int curr_input_col = 0;
    double imagePixelValue = 0;
    double maskPixelValue = 0;
    TeRaster& inputRasterRef = (*params.input_image_ptr_);
    
    TePDIPIManager progress( "Loading image data", params.in_box_nlines_, 
       params.progress_enabled_ );
    
    for( curr_out_line = 0 ; curr_out_line < origBoxRescNLines ; 
      ++curr_out_line ) 
    {
      curr_input_line = (unsigned int)( ( ( (double)curr_out_line ) / 
        params.origImgYRescFact_ ) + ( (double) params.in_box_y_off_ ) );      
      
      for( curr_out_col = 0 ; curr_out_col < origBoxRescNCols ; 
        ++curr_out_col ) {
        
        curr_input_col = (unsigned int)( ( ( (double)curr_out_col ) / 
            params.origImgXRescFact_ ) + ( (double) params.in_box_x_off_ ) );        
        
        inputRasterRef.getElement( curr_input_col, curr_input_line,
          imagePixelValue, params.input_image_channel_ );
        
        params.img_matrix_ptr_->setValue( curr_out_line, curr_out_col, 
          imagePixelValue );
          
        if( useMaskImage )
        {
          params.mask_image_ptr_->getElement( curr_input_col, curr_input_line,
            maskPixelValue, 0 );          
            
          params.mask_matrix_ptr_->operator[]( curr_out_line )[ curr_out_col ] = 
            (unsigned char)maskPixelValue;
        }
      }
      
      if( progress.Increment() )
      {
        TEAGN_LOGMSG( "Canceled by the user" );
        
        return;
      }
    }
  }
  
  // Retriving raster channel max and min channel allowed values
  
  if( params.generateDownsampledImage_ || params.generateUpsampledImage_ )
  {
    double rasterChnMin = 0;
    double rasterChnMax = 0;  
    if( ! TePDIUtils::TeGetRasterMinMaxBounds( 
      params.input_image_ptr_,
      params.input_image_channel_, rasterChnMin, rasterChnMax ) )
    {
       TEAGN_LOGERR( "Error getting channels bounds" );  
       return;
    }
    
    // Loading downsampled image box
    
    if( params.generateDownsampledImage_ )
    {
      const unsigned int out_box_nlines = (unsigned int)
        ( ((double)origBoxRescNLines) / params.moravecDownSampleFactor_ );
      const unsigned int out_box_ncols = (unsigned int)
        ( ((double)origBoxRescNCols) / params.moravecDownSampleFactor_ );
      
      if( ! bicubicResampleMatrix( *( params.img_matrix_ptr_ ), out_box_nlines,
        out_box_ncols, params.progress_enabled_, *(params.glb_mem_lock_ptr_),
        rasterChnMin, rasterChnMax, *( params.img_matrix_ds_ptr_ ) ) )
      {
        TEAGN_LOGERR( "Error generating downsampled image" );
        return;
      }
    }
    
    // Loading upsampled image box
    
    if( params.generateUpsampledImage_ )
    {
      // Calculating sizes for resampled boxes
      
      const unsigned int out_box_nlines = (unsigned int)
        ( ((double)origBoxRescNLines) / params.moravecUpSampleFactor_ );
      const unsigned int out_box_ncols = (unsigned int)
        ( ((double)origBoxRescNCols) / params.moravecUpSampleFactor_ );
      
      if( ! bicubicResampleMatrix( *( params.img_matrix_ptr_ ), out_box_nlines,
        out_box_ncols, params.progress_enabled_, *(params.glb_mem_lock_ptr_),
        rasterChnMin, rasterChnMax, *( params.img_matrix_us_ptr_ ) ) )
      {
        TEAGN_LOGERR( "Error generating downsampled image" );
        return;
      }
    } 
  }   
  
  params.returnValue_ = true;
}

void TePDIMMIOMatching::createImgMatrix( TeDataType dataType, 
  TeSharedPtr< TePDIMtxDoubleAdptInt >& matrixPtr )
{
  switch( dataType )
  {
    case TeDOUBLE :
    {
      TePDIMtxDoubleAdpt< double >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< double >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< double >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );
      
      break;
    }
    case TeFLOAT :
    {
      TePDIMtxDoubleAdpt< float >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< float >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< float >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    case TeLONG :
    {
      TePDIMtxDoubleAdpt< long int >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< long int >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< long int >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
        
      break;
    }
    case TeUNSIGNEDLONG :
    {
      TePDIMtxDoubleAdpt< unsigned long int >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< unsigned long int >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< unsigned long int >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
        
      break;
    }
    case TeINTEGER :
    {
      TePDIMtxDoubleAdpt< int >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< int >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< int >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
        
      break;
    }
    case TeSHORT :
    {
      TePDIMtxDoubleAdpt< short int >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< short int >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< short int >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    case TeUNSIGNEDSHORT :
    {
      TePDIMtxDoubleAdpt< unsigned short int >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< unsigned short int >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< unsigned short int >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    case TeCHAR :
    {
      TePDIMtxDoubleAdpt< char >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< char >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< char >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    case TeUNSIGNEDCHAR :
    {
      TePDIMtxDoubleAdpt< unsigned char >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< unsigned char >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< unsigned char >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    case TeBIT :
    {
      TePDIMtxDoubleAdpt< unsigned char >* newMtxPtr =  
        new TePDIMtxDoubleAdpt< unsigned char >;
      newMtxPtr->Reset( TePDIMtxDoubleAdpt< unsigned char >::AutoMemPol );
      matrixPtr.reset( newMtxPtr );        
        
      break;
    }
    default :
    {
      TEAGN_LOG_AND_THROW( "Invalid data type ");
      break;
    }
  }
}

void TePDIMMIOMatching::extractLocalMaximas( void * paramsPtr )
{
 /* Retriving parameters */
  
  ExtractLocalMaximasParams params = *( (ExtractLocalMaximasParams*) 
    paramsPtr );
  TEAGN_DEBUG_CONDITION( ( params.moravecWindowSize_ % 2 ),
    "Invalid moravecWindowSize" );    
  TEAGN_DEBUG_CONDITION( params.seachSubImagesNmb_ > 0,
    "Invalid params.seachSubImagesNmb_" );
    
  ( (ExtractLocalMaximasParams*) paramsPtr )->returnValue_ = false;
  
  if( ( params.imgMatrixPtr_->getNCols() > params.moravecWindowSize_ ) &&
    ( params.imgMatrixPtr_->getNLines() > params.moravecWindowSize_ ) )
  {
    /* Defining sub-windows */
    
    unsigned int sqrtSI = 0;
    if( ( params.imgMatrixPtr_->getNCols() <= ( sqrtSI * 
      params.moravecWindowSize_ ) ) || ( params.imgMatrixPtr_->getNLines() <=
      ( sqrtSI * params.moravecWindowSize_ ) ) ||
      ( params.maximasNumber_ < params.seachSubImagesNmb_ ) )
    {
      sqrtSI = 1;
    }
    else
    {
      sqrtSI = (unsigned int)ceil( sqrt( 
        (double)params.seachSubImagesNmb_ ) );
      if( sqrtSI == 0 ) sqrtSI = 1; 
    }

    const unsigned int subWindowMaxWidth = (unsigned int)
      ( params.imgMatrixPtr_->getNCols() / sqrtSI );
    const unsigned int subWindowMaxHeight = (unsigned int)
      ( params.imgMatrixPtr_->getNLines() / sqrtSI );  
        
    // Allocating buffers
    
    TePDIMatrix< double > dirVarMatrix;
    if( ! dirVarMatrix.Reset( subWindowMaxHeight, subWindowMaxWidth,
      TePDIMatrix< double >::RAMMemPol ) )
    {
      TEAGN_LOGERR( "Buffer reset error" );
      return;
    }         
       
    // Looking for maximas on each sub-image
      
    unsigned int windColBound = 0;
    unsigned int windLinBound = 0;
    unsigned int windWidth = 0;
    unsigned int windHeight = 0;
      
    TePDIPIManager progress( "Finding local maximas", sqrtSI * sqrtSI, 
      params.progressEnabled_ );
      
    for( unsigned int line = 0 ; line < params.imgMatrixPtr_->getNLines() ;
      line += subWindowMaxHeight )
    {
      windLinBound = MIN( line + subWindowMaxHeight, 
        params.imgMatrixPtr_->getNLines() );
      windHeight = windLinBound - line;
      
      for( unsigned int col = 0 ; col < params.imgMatrixPtr_->getNCols() ; 
        col += subWindowMaxWidth ) 
      {
        if( windHeight > params.moravecWindowSize_ )
        {
          windColBound = MIN( col + subWindowMaxWidth, 
            params.imgMatrixPtr_->getNCols() );
          windWidth = windColBound - col;
          
          if( windWidth > params.moravecWindowSize_ )
          {
            if( ! extractLocalMaximas( params.imgMatrixPtr_, 
              params.maskMatrixPtr_, col, line, 
              windWidth, windHeight, dirVarMatrix, 
              params.moravecWindowSize_, 
              params.maximasNumber_ / params.seachSubImagesNmb_,
              *params.outMaximasListPtr_, progress ) )
            {
              return;
            } 
            
//matrix2Tiff( *params.imgMatrixPtr_, "subimagesmaximas.tif", *params.outMaximasListPtr_ );              
          }
        }
        
        if( progress.Increment() )
        {
          TEAGN_LOGWARN( "Canceled by the user" );
          return;
        }
      }
    }
  }
  
  // So far... so good !
  
  ( (ExtractLocalMaximasParams*) paramsPtr )->returnValue_ = true;
}

bool TePDIMMIOMatching::extractLocalMaximas( 
  TePDIMtxDoubleAdptInt const* imgMatrixPtr,
  MaskMatrixT const* maskMatrixPtr,
  const unsigned int& xStart, const unsigned int& yStart, 
  const unsigned int& width,
  const unsigned int& height, TePDIMatrix< double >& bufferMatrix, 
  const unsigned int& moravecWindowSize, 
  const unsigned int& maximasNumber, MaximasListT& outMaximasList, 
  TePDIPIManager& progress )
{
  TEAGN_DEBUG_CONDITION( imgMatrixPtr, "Invalid pointer" )
  TEAGN_DEBUG_CONDITION( xStart < imgMatrixPtr->getNCols() - moravecWindowSize,
    "Invalid xStart" )
  TEAGN_DEBUG_CONDITION( yStart < imgMatrixPtr->getNLines() - moravecWindowSize,
    "Invalid yStart" )
  TEAGN_DEBUG_CONDITION( width >= moravecWindowSize, "Invalid width" )
  TEAGN_DEBUG_CONDITION( height >= moravecWindowSize, "Invalid height" )
  TEAGN_DEBUG_CONDITION( bufferMatrix.GetColumns() >= width, "Invalid buffer" );
  TEAGN_DEBUG_CONDITION( bufferMatrix.GetLines() >= height, "Invalid buffer" );
  
  // Globals
  
  const int windRad = (unsigned int)( moravecWindowSize / 2 );   
  
  {
    // directional variance calcule
    
    unsigned int bufferLin = 0;
    unsigned int bufferCol = 0;
    const unsigned int bufferLinBound = height - moravecWindowSize + 1;
    const unsigned int bufferColBound = width - moravecWindowSize + 1;
    unsigned int imgLine = 0;
    unsigned int imgCol = 0;
    unsigned int offset = 0;
    const TePDIMtxDoubleAdptInt& imgMatrix = *imgMatrixPtr;
    double neightValue = 0;
    double diffValue = 0;  
    double horVar = 0;
    double verVar = 0;
    double diagVar = 0;
    double adiagVar = 0;
    double centerValue = 0;
    unsigned int horImgLine = 0;
    unsigned int horImgCol = 0;
    unsigned int verImgLine = 0;
    unsigned int verImgCol = 0;
    unsigned int aDiagImgLine = 0;
    
    for( bufferLin = 0 ; bufferLin < bufferLinBound ; ++bufferLin )
    {
      imgLine = bufferLin + yStart;
      
      for( bufferCol = 0 ; bufferCol < bufferColBound ; ++bufferCol )
      {
        imgCol = bufferCol + xStart;
        
        imgMatrix.getValue( imgLine + windRad, imgCol + windRad, centerValue );
        
        horVar = verVar = diagVar = adiagVar = 0;
        
        horImgLine = imgLine + windRad;
        horImgCol = imgCol;
        
        verImgLine = imgLine;
        verImgCol = imgCol + windRad;
        
        aDiagImgLine = imgLine + moravecWindowSize - 1;
        
        for( offset = moravecWindowSize ; offset ; --offset )
        {
          imgMatrix.getValue( horImgLine, horImgCol, neightValue );
          diffValue = neightValue - centerValue;
          horVar += ( diffValue * diffValue );
          
          imgMatrix.getValue( verImgLine, verImgCol, neightValue );
          diffValue = neightValue - centerValue;
          verVar += ( diffValue * diffValue );
  
          imgMatrix.getValue( verImgLine, horImgCol, neightValue );
          diffValue = neightValue - centerValue;
          diagVar += ( diffValue * diffValue );
        
          imgMatrix.getValue( aDiagImgLine, horImgCol, neightValue );
          diffValue = neightValue - centerValue;
          adiagVar += ( diffValue * diffValue );
          
          ++horImgCol;
          ++verImgLine;
          --aDiagImgLine;
        }
        
        bufferMatrix( bufferLin + windRad, bufferCol + windRad ) = 
          MIN( horVar, MIN( verVar, MIN( diagVar, adiagVar ) ) );
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    }
  }
  
//doublesMatrix2Tiff( bufferMatrix, "bufferMatrix.tif" );
  
  // Loocking for local maximas
  
  MaximasMapT localMaximasMap;
  
  {
    const unsigned int bufferLinBound = height - windRad - moravecWindowSize + 1;
    const unsigned int bufferColBound = width - windRad - moravecWindowSize + 1;  
    unsigned int bufferLin = 0;
    unsigned int bufferCol = 0;
    double centerValue = 0;
    double*  bufferLinPtr = 0;   
    unsigned int bufferLin2 = 0;
    unsigned int bufferCol2 = 0;
    unsigned int bufferColBound2 = 0;
    unsigned int bufferLinBound2 = 0;
    bool currentIsMaxima = false;
    std::pair< double, MaximasMapNodeT > auxPair;
    
    for( bufferLin = windRad ; bufferLin < bufferLinBound ; ++bufferLin )
    {
      for( bufferCol = windRad ; bufferCol < bufferColBound ; ++bufferCol )
      {
        centerValue = bufferMatrix( bufferLin + windRad, bufferCol + windRad );
        
        bufferLinBound2 = bufferLin + moravecWindowSize;
        bufferColBound2 = bufferCol + moravecWindowSize;
        
        // This is a maxima point compared with all neighboors ??
        
        currentIsMaxima = true;
        
        for( bufferLin2 = bufferLin  ; bufferLin2 < bufferLinBound2 ;
          ++bufferLin2 )
        {
          bufferLinPtr = bufferMatrix[ bufferLin2 ];
          
          for( bufferCol2 = bufferCol ; bufferCol2 < bufferColBound2 ;
            ++bufferCol2 )
          {
            if( bufferLinPtr[ bufferCol2 ] > centerValue )
            {
              currentIsMaxima = false;
              bufferLin2 = bufferLinBound2;
              break;
            }
            else if( maskMatrixPtr && ( maskMatrixPtr->operator[]( bufferLin2 + 
              yStart )[ bufferCol2 + xStart ] == 0 ) )
            {
              currentIsMaxima = false;
              bufferLin2 = bufferLinBound2;
              break;
            }              
          }
        }
        
        if( currentIsMaxima )
        {
          auxPair.first = centerValue;
          auxPair.second.first = bufferCol + xStart + windRad;
          auxPair.second.second = bufferLin + yStart + windRad;
    
          localMaximasMap.insert( auxPair );
          
          if( localMaximasMap.size() > maximasNumber )
          {
            localMaximasMap.erase( localMaximasMap.begin() );
          }
        }
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    }
  }
    
  // filtering the best maximas
  
  unsigned int currMaximasCount = 0;
  MaximasMapT::reverse_iterator it = localMaximasMap.rbegin();
  const MaximasMapT::reverse_iterator itEnd = localMaximasMap.rend();
  MaximasListNodeT auxNode;
  
  while( ( currMaximasCount < maximasNumber ) && ( it != itEnd ) )
  {
    auxNode.dirVariance_ = it->first;
    auxNode.x_ = it->second.first;
    auxNode.y_ = it->second.second;
    
    outMaximasList.push_back( auxNode );
  
    ++currMaximasCount;
    ++it;
  }
  
  return true;
}

void TePDIMMIOMatching::moravecRepeatability( const MaximasListT& inputList,
  const MaximasListT& constraintList, 
  unsigned int constraintImageWidth,
  unsigned int constraintImageHeight,  
  double moravecRepeatabilityMinTolerance, 
  double moravecReSampleFactor, 
  MaximasListT& outputList )
{
  TEAGN_DEBUG_CONDITION( moravecRepeatabilityMinTolerance > 0,
    "Invalid moravecRepeatabilityMinTolerance ")
  TEAGN_DEBUG_CONDITION( moravecReSampleFactor > 0,
    "Invalid moravecReSampleFactor ")
      
  outputList.clear();
  
  // Criating an RTree for seach optmization
  
  const TeBox myTreeBBox( -1.0, -1.0, (double)(constraintImageWidth),
    (double)(constraintImageHeight) );  
  
  TeSAM::TeRTree<MaximasListNodeT const*, 8, 4> myTree( myTreeBBox );
  
  {
    MaximasListT::const_iterator cListIt = constraintList.begin();
    MaximasListT::const_iterator cListItEnd = constraintList.end();
    TeBox auxBox;
    
    while( cListIt != cListItEnd )
    {
      auxBox.x1_ = (double)(cListIt->x_) - moravecRepeatabilityMinTolerance;
      auxBox.y1_ = (double)(cListIt->y_) - moravecRepeatabilityMinTolerance;
      
      auxBox.x2_ = (double)(cListIt->x_) + moravecRepeatabilityMinTolerance;
      auxBox.y2_ = (double)(cListIt->y_) + moravecRepeatabilityMinTolerance;
      
      myTree.insert( auxBox, &(*cListIt) );
          
      ++cListIt;
    }
  }
  
  // Iterating over input list
  
  MaximasListT::const_iterator iListIt = inputList.begin();
  MaximasListT::const_iterator iListItEnd = inputList.end();
  std::vector< MaximasListNodeT const* > resultVec;
  TeBox auxBox;
    
  while( iListIt != iListItEnd )
  {
    const MaximasListNodeT& iListEle = *iListIt;
    
    auxBox.x1_ = auxBox.x2_ = iListEle.x_ / moravecReSampleFactor;
    auxBox.y1_ = auxBox.y2_ = iListEle.y_ / moravecReSampleFactor;
    
    resultVec.clear();
    
    if( myTree.search( auxBox, resultVec ) )
    {
      outputList.push_back( iListEle );  
    }
    
    ++iListIt;  
  }
  
  return;
}

void TePDIMMIOMatching::generateCorrelationFeatures( void * paramsPtr )
{
  /* Retriving parameters */
  
  GenerateCorrelationFeaturesParams& params = 
    *( (GenerateCorrelationFeaturesParams*) paramsPtr );
  
  const TePDIMtxDoubleAdptInt& img_matrix = *(params.imgMatrixPtr_);
  
  TePDIMatrix< double >& img_features_matrix = *( params.featMtxPtr_);
  
  params.returnValue_ = false;
    
  if( params.inMaximasListPtr_->size() > 0 ) {
    /* Allocating output matrix */
       
    params.glbMemLockPtr_->lock();
    
    if( ! params.featMtxPtr_->Reset( params.inMaximasListPtr_->size(),
      params.corrWindowsWidth_ * params.corrWindowsWidth_ ) )
    {
      TEAGN_LOGERR( "Error allocating image features matrix" )
      params.glbMemLockPtr_->unLock();
      return;
    }
      
    params.glbMemLockPtr_->unLock();
      
    /* variables related to the current window over the hole image */
    unsigned int curr_window_x_start = 0;
    unsigned int curr_window_y_start = 0;
    unsigned int curr_window_x_center = 0;
    unsigned int curr_window_y_center = 0;
    unsigned int curr_window_x_bound = 0;
    unsigned int curr_window_y_bound = 0;
    
    /*used on the rotation calcule */

    const unsigned int wind_radius = params.corrWindowsWidth_ / 2;
    // output window radius
    const double wind_radius_double = (double)wind_radius;

    const unsigned int img_features_matrix_cols = 
      params.featMtxPtr_->GetColumns();
    unsigned int curr_x = 0;
    unsigned int curr_y = 0;
    double curr_x_minus_radius = 0;
    double curr_y_minus_radius = 0;
    unsigned int curr_offset = 0;
    double int_x_dir = 0;
    double int_y_dir = 0;
    double int_norm = 0;
    double rotated_curr_x = 0;/* rotaded coord - window ref */
    double rotated_curr_y = 0;/* rotaded coord - window ref */
    
     /* the found rotation parameters */
    double rot_cos = 0;
    double rot_sin = 0;
    
    /* the coords rotated but in the hole image reference */
    int rotated_curr_x_img = 0;
    int rotated_curr_y_img = 0;
    
    /* current window mean and standart deviation */
    double curr_wind_mean = 0.0;
    double curr_wind_stddev = 0.0;
    double curr_wind_stddev_aux = 0.0;
    
    // used on intensity vector calcule
    double imgMatrixValue1 = 0;
    double imgMatrixValue2 = 0;
    
    TePDIPIManager progress( "Generating correlation windows",
      params.inMaximasListPtr_->size(), params.progressEnabled_ );
    
    MaximasListT::const_iterator maximIt = params.inMaximasListPtr_->begin();
    MaximasListT::const_iterator maximItEnd = params.inMaximasListPtr_->end();
    unsigned int maximasListIndex = 0;
      
    while( maximIt != maximItEnd ) {
      
      /* Calculating the current window position */
     
      curr_window_x_center = maximIt->x_;
      curr_window_y_center = maximIt->y_;
      curr_window_x_start = curr_window_x_center - wind_radius;
      curr_window_y_start = curr_window_y_center - wind_radius;
      curr_window_x_bound = curr_window_x_start + params.corrWindowsWidth_;
      curr_window_y_bound = curr_window_y_start + params.corrWindowsWidth_;
        
      /* Estimating the intensity vector X direction */
      
      for( curr_offset = 0 ; curr_offset < wind_radius ;
        ++curr_offset ) {      

        for( curr_y = curr_window_y_start ; curr_y < curr_window_y_bound ; 
          ++curr_y ) 
        {
          img_matrix.getValue( curr_y, curr_window_x_bound - 1 - curr_offset, 
             imgMatrixValue1 );
          img_matrix.getValue( curr_y, curr_window_x_start + curr_offset, 
             imgMatrixValue2 );
        
          int_x_dir += imgMatrixValue1 - imgMatrixValue2;
        }
      }
      
      int_x_dir /= ( 2.0 * ( (double) wind_radius ) );
      
      /* Estimating the intensity vector y direction */
      
      for( curr_offset = 0 ; curr_offset < wind_radius ;
        ++curr_offset ) {      

        for( curr_x = curr_window_x_start ; 
          curr_x < curr_window_x_bound ;
          ++curr_x ) 
        {
          img_matrix.getValue( curr_window_y_start + curr_offset, curr_x , 
            imgMatrixValue1 );
          img_matrix.getValue( curr_window_y_bound - 1 - curr_offset, 
            curr_x, imgMatrixValue2 );          
        
          int_y_dir += imgMatrixValue1 - imgMatrixValue2;
        }
      }      
      
      int_y_dir /= ( 2.0 * ( (double) wind_radius ) );
      
      /* Calculating the rotation parameters - 
         counterclockwise rotation 
         
         | u |    |cos  -sin|   |X|
         | v | == |sin   cos| x |Y|
      */
      int_norm = sqrt( ( int_x_dir * int_x_dir ) + 
        ( int_y_dir * int_y_dir ) );
      
      if( int_norm != 0.0 ) {
        rot_cos = int_x_dir / int_norm;
        rot_sin = int_y_dir / int_norm;
      } else {
        /* No rotation */
        rot_cos = 1.0;
        rot_sin = 0.0;
      }
      
      /* Generating the rotated window data and inserting it into 
         the img_features_matrix by setting the intensity vector
         to the direction (1,0) by a clockwise rotation
         using the inverse matrix 
      
         | u |    |cos   sin|   |X|
         | v | == |-sin  cos| x |Y|
      */
         
      for( curr_y = 0 ; curr_y < params.corrWindowsWidth_ ; ++curr_y ) 
      {
        for( curr_x = 0 ; curr_x < params.corrWindowsWidth_ ; ++curr_x ) 
        {
          /* briging the window to the coord system center */
          
          curr_x_minus_radius = ((double)curr_x) - 
            wind_radius_double;
          curr_y_minus_radius = ((double)curr_y) - 
            wind_radius_double;
          
          /* rotating the centered window */
          
          rotated_curr_x = 
            ( ( rot_cos * curr_x_minus_radius ) + 
            ( rot_sin * curr_y_minus_radius ) );
          
          rotated_curr_y =
            ( ( -1.0 * rot_sin * curr_x_minus_radius ) + 
            ( rot_cos * curr_y_minus_radius ) );
            
          /* bringing the window back to its original
             location with the correct new scale */ 
            
          rotated_curr_x += wind_radius_double;
          rotated_curr_y += wind_radius_double;
          
          /* copy the new rotated window to the output vector */
            
          rotated_curr_x_img = curr_window_x_start +
            TeRound( rotated_curr_x );
          rotated_curr_y_img = curr_window_y_start +
            TeRound( rotated_curr_y );                        
           
          if( ( rotated_curr_x_img > 0 ) &&  
            ( rotated_curr_x_img < (int)img_matrix.getNCols() ) &&
            ( rotated_curr_y_img > 0 ) &&
            ( rotated_curr_y_img < (int)img_matrix.getNLines() ) )
          {
            img_matrix.getValue( rotated_curr_y_img, rotated_curr_x_img, 
              imgMatrixValue1 ); 
          }
          else
          {
            TEAGN_LOGERR( "Invalid matrix position -> rotated_curr_y_img=" +
              Te2String( rotated_curr_y_img ) + " img_matrix.getNLines()=" +
              Te2String( img_matrix.getNLines() ) + " Processing point [" + Te2String( curr_window_x_center, 1 ) +
              "," + Te2String( curr_window_y_center, 1 ) + "] ");
          
            imgMatrixValue1 = 0;
          }
            
          img_features_matrix( maximasListIndex, ( curr_y * 
            params.corrWindowsWidth_ ) + curr_x ) = imgMatrixValue1;

        }
      }
      
      /* Normalizing the generated window by subtracting its mean
         and dividing by its standard deviation */      
      
      if( params.normalizeWindows_ )
      {
        curr_wind_mean = 0.0;
        
        for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
          ++curr_x ) {
          
          curr_wind_mean += img_features_matrix( maximasListIndex,
            curr_x );
        }
        
        curr_wind_mean /= ( (double)img_features_matrix_cols  );
        
        curr_wind_stddev = 0.0;  
        
        for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
          ++curr_x ) {
          
          curr_wind_stddev_aux = img_features_matrix( 
            maximasListIndex, curr_x ) - curr_wind_mean;
            
          curr_wind_stddev += ( curr_wind_stddev_aux *
            curr_wind_stddev_aux );
        }      
        
        curr_wind_stddev /= ( (double)img_features_matrix_cols  );
        curr_wind_stddev = sqrt( curr_wind_stddev );
        
        if( curr_wind_stddev == 0.0 ) {
          for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
            ++curr_x ) {
            
            double& curr_value = img_features_matrix( 
              maximasListIndex, curr_x );
            
            curr_value -= curr_wind_mean;
          } 
        } else {
          for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
            ++curr_x ) {
            
            double& curr_value = img_features_matrix( 
              maximasListIndex, curr_x );
            
            curr_value -= curr_wind_mean;
            curr_value /= curr_wind_stddev;
          }
        }
      }
      
      /* Finished !! */
     
      if( progress.Increment() )
      {
        TEAGN_LOGERR( "Canceled by the user" );
        return;
      }
      
      ++maximasListIndex;
      ++maximIt;
    }
  }    
  
  params.returnValue_ = true;
}

bool TePDIMMIOMatching::matchFeatures( FeatMatchingMethod matching_method,
  const TePDIMatrix< double >& img1featMtx,
  const MaximasListT& img1_maximas_list,
  const TePDIMatrix< double >& img2featMtx,
  const MaximasListT& img2_maximas_list,
  TeCoordPairVect& tiePointsVec,
  std::vector< double >& tiePointsWeights )
{
  if( ( img1featMtx.GetLines() == 0 ) || ( img2featMtx.GetLines() == 0 ) )
  {
    return true;
  }
  
  TEAGN_DEBUG_CONDITION( ( img1featMtx.GetLines() == 
    img1_maximas_list.size() ), "Size mismatch" )
  TEAGN_DEBUG_CONDITION( ( img2featMtx.GetLines() == 
    img2_maximas_list.size() ), "Size mismatch" )    
  TEAGN_DEBUG_CONDITION( ( img1featMtx.GetColumns() == 
    img2featMtx.GetColumns() ), "Size mismatch" )
    
  tiePointsVec.clear();
  tiePointsWeights.clear();
  
  // Generating the matching matrix
  // For each matching method the matching matrix will have 
  // different values range.
  
  TePDIMatrix<double> matchMatrix;
  TEAGN_TRUE_OR_RETURN( matchMatrix.Reset( img1_maximas_list.size(),
    img2_maximas_list.size(), TePDIMatrix<double>::RAMMemPol ),
    "Error allocating memory" );  
  
  const unsigned int mMCols = matchMatrix.GetColumns();
  const unsigned int mMLines = matchMatrix.GetLines();  
  
  {
    const double negativeInfinite = DBL_MAX * (-1.0);
    double* linePtr = 0;
    unsigned int col = 0;
    
    for( unsigned int line =0 ; line < mMLines ; ++line )
    {
      linePtr = matchMatrix[ line ];
      
      for( col =0 ; col < mMCols ; ++col )
        linePtr[ col ] = negativeInfinite;
    }
  }
  
  const unsigned long int maxJobs = jobsMan_.getMaxSimulJobs() ?
      jobsMan_.getMaxSimulJobs() : 1;
      
  double matchingMatrixMin = DBL_MAX;
  double matchingMatrixMax = (-1.0) * DBL_MAX;   
  
  switch( matching_method )
  {
    case NormCrossCorrMethod :
    {
      // Spawning jobs
      
      std::vector< CalcCCorrelationMtxParams > jobsParamsVec;  
      
      CalcCCorrelationMtxParams auxPars;      
      auxPars.img1FeatMtxPtr_ = &img1featMtx;
      auxPars.img2FeatMtxPtr_ = &img2featMtx;
      auxPars.matchMatrixMinPtr_ = &matchingMatrixMin;
      auxPars.matchMatrixMaxPtr_ = &matchingMatrixMax;
      auxPars.matchMatrixPtr_ = &matchMatrix;
      auxPars.progressEnabled_ = progress_enabled_;
      auxPars.glbMutexptr_ = &globalMutex_;
      auxPars.returnValue_ = false;
      
      TeFunctionCallThreadJob dummyJob;
      dummyJob.jobFunc_ = calcCCorrelationMtx;

      unsigned int jobidx = 0;
      jobsParamsVec.resize( maxJobs );
      
      for( jobidx = 0 ; jobidx < maxJobs ; ++jobidx )
      {
        jobsParamsVec[ jobidx ] = auxPars;
        dummyJob.jobFuncParsPtr_ = &( jobsParamsVec[ jobidx ] );
        
        jobsMan_.executeJob( dummyJob );
      }
      
      // Waiting jobs to finish
      
      jobsMan_.waitAllToFinish();
      
      for( jobidx = 0 ; jobidx < maxJobs ; ++jobidx )
      {
        TEAGN_TRUE_OR_RETURN( jobsParamsVec[ jobidx ].returnValue_, 
          "Error calculating euclidian distance" );
      }      
      
      break;
    }
    default :
    {
      TEAGN_LOG_AND_THROW( "Invalid method")
      break;
    }
  }
  
  // Finding best matchings
  
  // Feature 1 desires to match with the feature 2 with the index
  // indicated by this vector
  std::vector< unsigned int> feat1Tofeat2( matchMatrix.GetLines(),
    matchMatrix.GetColumns() );  
  
  // Feature 2 desires to match with the feature 1 with the index
  // indicated by this vector
  std::vector< unsigned int> feat2Tofeat1( matchMatrix.GetColumns(),
    matchMatrix.GetLines() );     
  
  switch( matching_method )
  {
    case NormCrossCorrMethod :
    {
     // maximum values found on each column
      std::vector< double > colsMaxsVec( matchMatrix.GetColumns(), 
        (-1.0) * DBL_MAX );
      
      // maximum values found on each line
      std::vector< double > linesMaxsVec( matchMatrix.GetLines(), 
        (-1.0) * DBL_MAX );
      
      double const* linePtr = 0;
      unsigned int mMLine =0;
      unsigned int mMCol =0;
      
      TePDIPIManager progress( "Matching features",
        mMLines, progress_enabled_ );       
      
      for( mMLine =0 ; mMLine < mMLines ; ++mMLine )
      {
        linePtr = matchMatrix[ mMLine ];
        
        for( mMCol =0 ; mMCol < mMCols ; ++mMCol )
        {
          const double& value = linePtr[ mMCol ];
          
          if( value > linesMaxsVec[ mMLine ] )
          {
            linesMaxsVec[ mMLine ] = value;
            
            if( value > 0.0 )
            {
              feat1Tofeat2[ mMLine ] = mMCol;
            }
          }
          
          if( value > colsMaxsVec[ mMCol ] )
          {
            colsMaxsVec[ mMCol ] = value;
            
            if( value > 0.0 )
            {
              feat2Tofeat1[ mMCol ] = mMLine;
            }
          }
        }
        
        if( progress.Increment() )
        {
          TEAGN_LOGERR( "Canceled by the user" );
          return false;
        }           
      }
      
      break;
    }
    default :
    {
      TEAGN_LOG_AND_THROW( "Invalid method")
      break;
    }
  }
  
  // Calculating the normalization parameters
  
  double list1DirVarianceMin = DBL_MAX;
  double list1DirVarianceMax = (-1.0) * list1DirVarianceMin;
  double list1DirVarianceRange = 0;
  
  double list2DirVarianceMin = DBL_MAX;
  double list2DirVarianceMax = (-1.0) * list2DirVarianceMin;  
  double list2DirVarianceRange = 0;
  
  TEAGN_DEBUG_CONDITION( matchingMatrixMax >= matchingMatrixMin,
    "Invalid maching matrix limits" );  
  const double matchingMatrixRange = matchingMatrixMax - matchingMatrixMin;
  
  {
    // Image 1 maximas variance range 
    
    MaximasListT::const_iterator mListIt = img1_maximas_list.begin();
    MaximasListT::const_iterator mListItEnd = img1_maximas_list.end();
    
    while( mListIt != mListItEnd )
    {
      if( mListIt->dirVariance_ < list1DirVarianceMin )
      {
        list1DirVarianceMin = mListIt->dirVariance_;
      }
      if( mListIt->dirVariance_ > list1DirVarianceMax )
      {
        list1DirVarianceMax = mListIt->dirVariance_;
      }
          
      ++mListIt;
    }
    
    list1DirVarianceRange = list1DirVarianceMax - list1DirVarianceMin;
    
    // Image 2 maximas variance range 
    
    mListIt = img2_maximas_list.begin();
    mListItEnd = img2_maximas_list.end();
    
    while( mListIt != mListItEnd )
    {
      if( mListIt->dirVariance_ < list2DirVarianceMin )
      {
        list2DirVarianceMin = mListIt->dirVariance_;
      }
      if( mListIt->dirVariance_ > list2DirVarianceMax )
      {
        list2DirVarianceMax = mListIt->dirVariance_;
      }
          
      ++mListIt;
    }
    
    list2DirVarianceRange = list2DirVarianceMax - list2DirVarianceMin;
  }
  
  // Generating tie-points
  
  {
    MaximasListT::const_iterator mList1It = img1_maximas_list.begin();
    MaximasListT::const_iterator mList2It;
    TeCoordPair dummyCPair;
    unsigned int desiredFeature2Idx = 0;
    unsigned int desiredFeature1Idx = 0;
    const unsigned int feat2Tofeat1Size = (unsigned int)
      feat2Tofeat1.size();
    double tpWeight = 0;
    
    for( unsigned int feat1Tofeat2_idx = 0 ; feat1Tofeat2_idx < 
      feat1Tofeat2.size() ; ++feat1Tofeat2_idx )
    {
      desiredFeature2Idx = feat1Tofeat2[ feat1Tofeat2_idx ];
      
      // feature1 wants to match with the feature 2 with index
      // feat1Tofeat2[ feat1Tofeat2_idx ]
      if( desiredFeature2Idx < feat2Tofeat1Size )
      {
        // does feature 2 wants to match back ??
        
        desiredFeature1Idx = feat2Tofeat1[ desiredFeature2Idx ];
        
        if( desiredFeature1Idx == feat1Tofeat2_idx )
        { // Great !!! we hava a match
        
          // move an iterator to the correct maximas list 2 position
          
          mList2It = img2_maximas_list.begin();
          for( unsigned int maxList2dx = 0 ; maxList2dx < 
            desiredFeature2Idx ; ++maxList2dx )
          {
            ++mList2It;
          }
          
          // Generating the new tie-point
          
          dummyCPair.pt1.x( mList1It->x_ );
          dummyCPair.pt1.y( mList1It->y_ );
          dummyCPair.pt2.x( mList2It->x_ );
          dummyCPair.pt2.y( mList2It->y_ );
          
          tiePointsVec.push_back( dummyCPair );
          
          // updating the new tie-point weight
          
          TEAGN_DEBUG_CONDITION( mList1It->dirVariance_ >= 0,
            "Negative directional variance found" )
          TEAGN_DEBUG_CONDITION( mList2It->dirVariance_ >= 0,
            "Negative directional variance found" )
            
          tpWeight = 1.0;
          
          if( ( list1DirVarianceRange != 0.0 ) &&
            ( list2DirVarianceRange != 0.0 ) && ( matchingMatrixRange != 0.0 ) )
          {
            tpWeight = ( ( ( mList1It->dirVariance_ - 
              list1DirVarianceMin ) / list1DirVarianceRange ) +
              ( ( mList2It->dirVariance_ - list2DirVarianceMin ) / 
              list2DirVarianceRange ) ) / 2.00;
          
            switch( matching_method )
            {
              case NormCrossCorrMethod :
              {
                tpWeight *= 
                ( ( matchMatrix[ desiredFeature1Idx ][ desiredFeature2Idx ] 
                  - matchingMatrixMin ) / matchingMatrixRange  );
                                
                break;
              }            
              default :
              {
                TEAGN_LOG_AND_THROW( "Invalid method")
                break;    
              }            
            }
          }
          
          TEAGN_DEBUG_CONDITION( ( ( tpWeight >= 0.0 ) && 
            ( tpWeight <= 1.0 ) ), "invalid tpWeight=" + Te2String( tpWeight, 
            2 ) );
          tiePointsWeights.push_back( tpWeight );          
        }
      }
      
      ++mList1It;
    }
  }
  
  return true;
}

void TePDIMMIOMatching::calcCCorrelationMtx( void * paramsPtr )
{
  CalcCCorrelationMtxParams& params = 
    *((CalcCCorrelationMtxParams*)paramsPtr );
  
  params.returnValue_ = false;
  
  TEAGN_DEBUG_CONDITION( params.matchMatrixPtr_->GetLines() ==
    params.img1FeatMtxPtr_->GetLines(), "Size mismatch" )
  TEAGN_DEBUG_CONDITION( params.matchMatrixPtr_->GetColumns() ==
    params.img2FeatMtxPtr_->GetLines(), "Size mismatch" )
  TEAGN_DEBUG_CONDITION( params.img1FeatMtxPtr_->GetColumns() ==
    params.img2FeatMtxPtr_->GetColumns(), "Size mismatch" )    
  TEAGN_DEBUG_CONDITION( params.img1FeatMtxPtr_->getMemPolicy() ==
    TePDIMatrix< double >::RAMMemPol, "Invalid metrix policy" );
  TEAGN_DEBUG_CONDITION( params.img2FeatMtxPtr_->getMemPolicy() ==
    TePDIMatrix< double >::RAMMemPol, "Invalid metrix policy" );   
  TEAGN_DEBUG_CONDITION( params.matchMatrixPtr_->getMemPolicy() ==
    TePDIMatrix< double >::RAMMemPol, "Invalid metrix policy" );      
  TEAGN_DEBUG_CONDITION( params.matchMatrixMinPtr_, "Invalid pointer" );
  TEAGN_DEBUG_CONDITION( params.matchMatrixMaxPtr_, "Invalid pointer" );            
    
  TePDIMatrix< double >& matchMatrix = *params.matchMatrixPtr_;
  const TePDIMatrix< double >& img1FeatMtx = *(params.img1FeatMtxPtr_);
  const TePDIMatrix< double >& img2FeatMtx = *(params.img2FeatMtxPtr_);
  const unsigned int matchMatrixLines = matchMatrix.GetLines();
  const unsigned int matchMatrixCols = matchMatrix.GetColumns();
  double& matchMatrixMin = *params.matchMatrixMinPtr_;
  double& matchMatrixMax = *params.matchMatrixMaxPtr_;
  unsigned int notProcLine = 0;
  unsigned int notProcCol = 0;
  const double negativeInfinite = DBL_MAX * (-1.0);
  double const* feat1Ptr = 0;
  double const* feat2Ptr = 0;
  double sumAA = 0;
  double sumBB = 0;
  double ccorrelation = 0;
  double cc_norm = 0;
  unsigned int featCol = 0;
  const unsigned int featSize = img1FeatMtx.GetColumns();
  double* mMLinePtr = 0;
  double lineMinValue = 0;
  double lineMaxValue = 0;
  
  // Loocking for a not processed line
  // Each line will be processed by each thread
  
  TePDIPIManager progress( "Calculating correlation matrix",
    matchMatrixLines, params.progressEnabled_ );  
  
  for( notProcLine = 0 ; notProcLine < matchMatrixLines ; ++notProcLine )
  {
    params.glbMutexptr_->lock();
    
    mMLinePtr = matchMatrix[ notProcLine ];
    
    if( mMLinePtr[ 0 ] == negativeInfinite )
    {
      // mark line as under processing 
      mMLinePtr[ 0 ] = 0;  
      
      params.glbMutexptr_->unLock();
      
      lineMinValue = DBL_MAX;
      lineMaxValue = -1.00 * DBL_MAX;
      
      for( notProcCol = 0 ; notProcCol < matchMatrixCols ; ++notProcCol )
      {      
        params.glbMutexptr_->lock();
        
        feat1Ptr = img1FeatMtx[ notProcLine ];
        feat2Ptr = img2FeatMtx[ notProcCol ];        
        
        params.glbMutexptr_->unLock();
        
        sumAA = 0;
        sumBB = 0;   
        
        for( featCol = 0 ; featCol < featSize ; ++featCol )
        {
          sumAA += feat1Ptr[ featCol ] * feat1Ptr[ featCol ];
          sumBB += feat2Ptr[ featCol ] * feat2Ptr[ featCol ];
        }
        
        cc_norm = sqrt( sumAA * sumBB );
        
        if( cc_norm == 0.0 )
        {
          ccorrelation = -1.00;
          mMLinePtr[ notProcCol ] = -1.00 ;
        }
        else
        {
          ccorrelation = 0;
          
          for( featCol = 0 ; featCol < featSize ; ++featCol )
          {
            ccorrelation += ( feat1Ptr[ featCol ] * feat2Ptr[ featCol ] ) / 
              cc_norm;
          }
            
          mMLinePtr[ notProcCol ] = ccorrelation;            
        }
        
        if( ccorrelation < lineMinValue )
        {
          lineMinValue = ccorrelation;
        }  
        if( ccorrelation > lineMaxValue )
        {
          lineMaxValue = ccorrelation;
        }
      }
      
      // Updating the global match matrix min and max variables using
      // the current line min and max
      
      TEAGN_DEBUG_CONDITION( lineMinValue <= lineMaxValue, "invalid values" )
      
      params.glbMutexptr_->lock();
      
      if( lineMinValue < matchMatrixMin )
      {
        matchMatrixMin = lineMinValue;
      }
      if( lineMaxValue > matchMatrixMax )
      {
        matchMatrixMax = lineMaxValue;
      }        
      
      params.glbMutexptr_->unLock();      
    }
    else
    {
      params.glbMutexptr_->unLock();
    }
   
    if( progress.Increment() )
    {
      TEAGN_LOGERR( "Canceled by the user" );
      return;
    }
  }
  
  params.returnValue_ = true;
}

void TePDIMMIOMatching::matrix2Tiff( 
  const TePDIMtxDoubleAdptInt& input_matrix,
  const std::string& out_file_name,
  const MaximasListT& maxima_points )
{   
  TEAGN_TRUE_OR_THROW( ( ! out_file_name.empty() ), "Invalid file name" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.getNLines() > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.getNCols() > 0 ), 
    "Invalid matrix cols" )
    
  double value = 0;
    
  TeRasterParams params;
  params.setNLinesNColumns( input_matrix.getNLines(),
    input_matrix.getNCols() );
  params.nBands( 1 );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  params.nBands( 1 );
  params.decoderIdentifier_ = "TIF";
  params.mode_ = 'c';
  params.fileName_ = out_file_name;
  
  TeRaster out_raster( params );
  TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
  
  for( unsigned int line = 0 ; 
    line < input_matrix.getNLines() ; ++line ) {
    for( unsigned int col = 0 ; 
      col < input_matrix.getNCols() ; 
      ++col ) 
    {
      input_matrix.getValue( line, col, value );
      TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
        MIN( value, 254.0 ), 0 ),
        "Error writing raster" )
    }  
  }
  
  /* Draw maxima points */
  
  MaximasListT::const_iterator maximas_it = 
    maxima_points.begin();
  MaximasListT::const_iterator maximas_it_end = 
    maxima_points.end();
    
  while( maximas_it != maximas_it_end ) {
    const unsigned int& x = maximas_it->x_;
    
    TEAGN_TRUE_OR_THROW( ( ((int)x) < (int)input_matrix.getNCols() ),
      "Invalid maxima column" )
    const unsigned int& y = maximas_it->y_;
    TEAGN_TRUE_OR_THROW( ( ((int)y) < (int)input_matrix.getNLines() ),
      "Invalid maxima line" )
    
    TEAGN_TRUE_OR_THROW( out_raster.setElement( x, y, 
      255.0, 0 ),
      "Error writing raster" )
      
    ++maximas_it;
  }  
}

void TePDIMMIOMatching::matrix2Tiff( 
  const TePDIMtxDoubleAdptInt& input_matrix,
  const std::string& out_file_name,
  const TeCoordPairVect& tiepoints,
  bool usePt1 )
{   
  TEAGN_TRUE_OR_THROW( ( ! out_file_name.empty() ), "Invalid file name" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.getNLines() > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.getNCols() > 0 ), 
    "Invalid matrix cols" )
    
  double value = 0;
    
  TeRasterParams params;
  params.setNLinesNColumns( input_matrix.getNLines(),
    input_matrix.getNCols() );
  params.nBands( 1 );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  params.nBands( 1 );
  params.decoderIdentifier_ = "TIF";
  params.mode_ = 'c';
  params.fileName_ = out_file_name;
  
  TeRaster out_raster( params );
  TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
  
  for( unsigned int line = 0 ; 
    line < input_matrix.getNLines() ; ++line ) {
    for( unsigned int col = 0 ; 
      col < input_matrix.getNCols() ; 
      ++col ) 
    {
      input_matrix.getValue( line, col, value );
      TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
        MIN( value, 254.0 ), 0 ),
        "Error writing raster" )
    }  
  }
  
  /* Draw maxima points */
  
  TeCoordPairVect::const_iterator it = 
    tiepoints.begin();
  TeCoordPairVect::const_iterator it_end = 
    tiepoints.end();
  
  int tpx = 0;
  int tpy = 0;
    
  while( it != it_end ) {
    if( usePt1 )
    {
      tpx = (int)it->pt1.x();
      tpy = (int)it->pt1.y();
    }
    else
    {
      tpx = (int)it->pt2.x();
      tpy = (int)it->pt2.y();
    }
    
    TEAGN_TRUE_OR_THROW( ( tpx < (int)input_matrix.getNCols() ),
      "Invalid maxima column" )
    TEAGN_TRUE_OR_THROW( ( tpx >= 0 ),
      "Invalid maxima column" )
    TEAGN_TRUE_OR_THROW( ( tpy < (int)input_matrix.getNLines() ),
      "Invalid maxima line" )
    TEAGN_TRUE_OR_THROW( ( tpy >= 0 ),
      "Invalid maxima line" )           
    
    TEAGN_TRUE_OR_THROW( out_raster.setElement( tpx, tpy, 
      255.0, 0 ),
      "Error writing raster" )
      
    ++it;
  }  
}

void TePDIMMIOMatching::doublesMatrix2Tiff( 
  const TePDIMatrix< double >& input_matrix,
  const std::string& out_file_name,
  const MaximasListT& maxima_points )
{   
  TEAGN_TRUE_OR_THROW( ( ! out_file_name.empty() ), "Invalid file name" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.GetLines() > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.GetColumns() > 0 ), 
    "Invalid matrix cols" )
    
  // Guessing matrix range
  
  unsigned int line = 0;
  unsigned int col = 0;
  double input_matrix_min = DBL_MAX;
  double input_matrix_max = -1.0 * DBL_MAX;
  
  for( line = 0 ; line < input_matrix.GetLines() ; ++line ) 
  {
    for( col = 0 ; col < input_matrix.GetColumns() ; ++col ) 
    {
      if( input_matrix( line, col ) < input_matrix_min )
      {
        input_matrix_min = input_matrix( line, col );
      }
      if( input_matrix( line, col ) > input_matrix_max )
      {
        input_matrix_max = input_matrix( line, col );
      }      
    }  
  }  
  
  double input_matrix_range = input_matrix_max - input_matrix_min;
  
  // Generate output
    
  TeRasterParams params;
  params.setNLinesNColumns( input_matrix.GetLines(),
    input_matrix.GetColumns() );
  params.nBands( 1 );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  params.decoderIdentifier_ = "TIF";
  params.mode_ = 'c';
  params.fileName_ = out_file_name;
  
  TeRaster out_raster( params );
  TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
  
  for( line = 0 ; line < input_matrix.GetLines() ; ++line ) 
  {
    for( col = 0 ; col < input_matrix.GetColumns() ; ++col ) 
    {
      TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
        255.0 * ( ( input_matrix( line, col ) - input_matrix_min ) / 
        input_matrix_range ), 0 ),
        "Error writing raster" )
    }  
  }
  
  /* Draw maxima points */
  
  MaximasListT::const_iterator maximas_it = 
    maxima_points.begin();
  MaximasListT::const_iterator maximas_it_end = 
    maxima_points.end();
    
  while( maximas_it != maximas_it_end ) {
    const unsigned int& x = maximas_it->x_;
    
    TEAGN_TRUE_OR_THROW( ( ((int)x) < (int)input_matrix.GetColumns() ),
      "Invalid maxima column" )
    const unsigned int& y = maximas_it->y_;
    TEAGN_TRUE_OR_THROW( ( ((int)y) < (int)input_matrix.GetLines() ),
      "Invalid maxima line" )
    
    TEAGN_TRUE_OR_THROW( out_raster.setElement( x, y, 
      255.0, 0 ),
      "Error writing raster" )
      
    ++maximas_it;
  }  
}

void TePDIMMIOMatching::features2Tiff( 
  unsigned int corr_window_width,
  const MaximasListT& img_maxima_points,
  TePDIMatrix< double >& img_features_matrix,
  const std::string& filenameaddon )
{
  TEAGN_TRUE_OR_THROW( ( img_features_matrix.GetLines() ==
    img_maxima_points.size() ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( img_features_matrix.GetColumns() ==
    ( corr_window_width * corr_window_width ) ), 
    "Invalid matrix columns" )
  TEAGN_TRUE_OR_THROW( ( corr_window_width > 0 ),
    "Invalid corr_window_width" )   
    
  MaximasListT::const_iterator maxIt = img_maxima_points.begin();

  for( unsigned int curr_wind_index = 0 ; 
    curr_wind_index < img_features_matrix.GetLines() ;
    ++curr_wind_index ) {
    
    // finding feature min and max
    
    double featMin = DBL_MAX;
    double featMax = -1.0 * featMin;
    
    for( unsigned int fcol = 0 ; fcol < img_features_matrix.GetColumns();
      ++fcol )
    {
      if( img_features_matrix( curr_wind_index, fcol ) < featMin )
      {
        featMin = img_features_matrix( curr_wind_index, fcol );
      }
      if( img_features_matrix( curr_wind_index, fcol ) > featMax )
      {
        featMax = img_features_matrix( curr_wind_index, fcol );
      }
    }
    
    double featRange = ( featMin != featMax ) ? ( featMax - featMin ) : 1.0;
    double featMultFac = 255.0 / featRange;
    
    // saving output
    
    TeRasterParams params;
    params.setNLinesNColumns( corr_window_width,
      corr_window_width );
    params.nBands( 1 );
    params.setDataType( TeUNSIGNEDCHAR, -1 );
    params.nBands( 1 );
    params.decoderIdentifier_ = "TIF";
    params.mode_ = 'c';
    params.fileName_ = filenameaddon + "_" + Te2String( maxIt->x_, 0 ) + 
      "_" + Te2String( maxIt->y_, 0 ) + ".tif";
    
    TeRaster out_raster( params );
    TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
    
    for( unsigned int line = 0 ; line < corr_window_width ; ++line ) {
      for( unsigned int col = 0 ; col < corr_window_width ; ++col ) {

        TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
          ( img_features_matrix( curr_wind_index, ( line * 
          corr_window_width ) + col ) - featMin ) * featMultFac, 0 ), 
          "Error writing raster" )
      }  
    }
    
    ++maxIt;
  }
}

bool TePDIMMIOMatching::bicubicResampleMatrix( const TePDIMtxDoubleAdptInt& inputMatrix,
  unsigned int outLines, unsigned int outCols, bool progressEnabled,
  TeMutex& globalMutex, double outMinValue, double outMaxValue,
  TePDIMtxDoubleAdptInt& outputMatrix )
{
  globalMutex.lock();
  
  if( ! outputMatrix.reset( outLines, outCols ) )
  {
    globalMutex.unLock();
    
    TEAGN_LOGERR( "Output matrix reset error" );
  }
  
  globalMutex.unLock();
  
  const unsigned int inLines = inputMatrix.getNLines();
  const unsigned int inCols = inputMatrix.getNCols();
  const int lastInLineIdx = inLines - 1;
  const int lastInColIdx = inCols - 1;
  const double yResFact = ((double)outLines) / ((double)inLines);
  const double xResFact = ((double)outCols) / ((double)inCols);
  const double bicubic_columns_bound = ((double)inCols) - 2;
  const double bicubic_lines_bound = ((double)inLines) - 2;
  const double bicubic_kernel_parameter = -1.0;
  
  double inputLine = 0;
  double inputCol = 0;
  unsigned int outLine = 0;
  unsigned int outCol = 0;
  double value = 0;
  unsigned int  bicubic_grid_input_line = 0;
  unsigned int  bicubic_grid_input_col = 0;
  unsigned int  bicubic_buffer_line = 0;
  unsigned int  bicubic_buffer_col = 0;  
  double bicubic_offset_x;
  double bicubic_offset_y;
  double bicubic_h_weights[4];
  double bicubic_v_weights[4];  
  double bicubic_h_weights_sum = 0;
  double bicubic_v_weights_sum = 0;
  double bicubic_int_line_accum = 0;
  double bicubic_int_lines_values[4];
  int correctedInputLine = 0;
  int correctedInputCol = 0;
  
  TePDIPIManager progress( "Resampling", outLines, 
     progressEnabled );  
  
  for( outLine = 0 ; outLine < outLines ; ++outLine )
  {
    inputLine =  ((double)outLine) / yResFact;
    
    for( outCol = 0 ; outCol < outCols ; ++outCol )
    {
      inputCol =  ((double)outCol) / xResFact;
      
      if( ( inputCol < 1.0 ) || ( inputLine < 1.0 ) || ( inputCol >= 
        bicubic_columns_bound ) || ( inputLine >= bicubic_lines_bound ) ) 
      {
        /* Near neighborhood interpolation will be used */
        
        correctedInputLine = (int)TeRound( inputLine );
        correctedInputLine = MAX( 0, correctedInputLine );
        correctedInputLine = MIN( lastInLineIdx, correctedInputLine );
        
        correctedInputCol = (int)TeRound( inputCol );
        correctedInputCol = MAX( 0, correctedInputCol );
        correctedInputCol = MIN( lastInColIdx, correctedInputCol );        
        
        inputMatrix.getValue( correctedInputLine, correctedInputCol, value );
        outputMatrix.setValue( outLine , outCol, value );
      } else {
        bicubic_grid_input_line = ( (unsigned int)floor( inputLine ) ) - 1;
        bicubic_grid_input_col = ( (unsigned int)floor( inputCol ) ) - 1;
        
        /* Bicubic weights calcule for the required position */
        
        bicubic_offset_x = inputCol - (double)( bicubic_grid_input_col + 1 );
        bicubic_offset_y = inputLine - (double)( bicubic_grid_input_line + 1 );    
        
        bicubic_h_weights[0] = BICUBIC_KERNEL( 1.0 + bicubic_offset_x, 
          bicubic_kernel_parameter );
        bicubic_h_weights[1] = BICUBIC_KERNEL( bicubic_offset_x, 
          bicubic_kernel_parameter );
        bicubic_h_weights[2] = BICUBIC_KERNEL( 1.0 - bicubic_offset_x, 
          bicubic_kernel_parameter );
        bicubic_h_weights[3] = BICUBIC_KERNEL( 2.0 - bicubic_offset_x, 
          bicubic_kernel_parameter );
          
        bicubic_v_weights[0] = BICUBIC_KERNEL( 1.0 + bicubic_offset_y, 
          bicubic_kernel_parameter );
        bicubic_v_weights[1] = BICUBIC_KERNEL( bicubic_offset_y, 
          bicubic_kernel_parameter );
        bicubic_v_weights[2] = BICUBIC_KERNEL( 1.0 - bicubic_offset_y, 
          bicubic_kernel_parameter );
        bicubic_v_weights[3] = BICUBIC_KERNEL( 2.0 - bicubic_offset_y, 
          bicubic_kernel_parameter );
          
        bicubic_h_weights_sum = bicubic_h_weights[0] + bicubic_h_weights[1] +
          bicubic_h_weights[2] + bicubic_h_weights[3];
        bicubic_v_weights_sum = bicubic_v_weights[0] + bicubic_v_weights[1] +
          bicubic_v_weights[2] + bicubic_v_weights[3];
        
        /* interpolating the value */
        
        for( bicubic_buffer_line = 0 ; bicubic_buffer_line < 4 ; 
          ++bicubic_buffer_line) {
          
          bicubic_int_line_accum = 0;
          
          for( bicubic_buffer_col = 0 ; bicubic_buffer_col < 4 ; 
            ++bicubic_buffer_col ) 
          {
            inputMatrix.getValue( bicubic_grid_input_line + bicubic_buffer_line, 
                bicubic_grid_input_col + bicubic_buffer_col,
                value );            
            
            bicubic_int_line_accum += value * 
              bicubic_h_weights[ bicubic_buffer_col ];
          }
          
          bicubic_int_lines_values[ bicubic_buffer_line ] = 
            bicubic_int_line_accum / bicubic_h_weights_sum;
        }
        
        value = bicubic_int_lines_values[ 0 ] * bicubic_v_weights[ 0 ] +
          bicubic_int_lines_values[ 1 ] * bicubic_v_weights[ 1 ] +
          bicubic_int_lines_values[ 2 ] * bicubic_v_weights[ 2 ] +
          bicubic_int_lines_values[ 3 ] * bicubic_v_weights[ 3 ];
        value = value / bicubic_v_weights_sum;
        
        if( value > outMaxValue )
        {
          outputMatrix.setValue( outLine , outCol, outMaxValue );
        }
        else if( value < outMinValue )
        {
          outputMatrix.setValue( outLine , outCol, outMinValue );
        }
        else
        {  
          outputMatrix.setValue( outLine , outCol, value );
        }
      }
    }
    
    if( progress.Increment() )
    {
      TEAGN_LOGMSG( "Canceled by the user" );
      return false;
    }
  }
 
  return true;
}

bool TePDIMMIOMatching::generateMaximasRaster( 
  const TePDIMtxDoubleAdptInt& inputMatrix,
  const MaximasListT& maximaPoints,
  TeRaster& outRaster )
{
  // initiating the output raster
  
  const unsigned int nLines = inputMatrix.getNLines();
  const unsigned int nCols = inputMatrix.getNCols();
      
  TeRasterParams params = outRaster.params();
  params.setNLinesNColumns( nLines, nCols );
  params.nBands( 3 );
  params.setPhotometric( TeRasterParams::TeRGB );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  TEAGN_TRUE_OR_THROW( outRaster.init( params ), "Error init raster" );
  
  // copying data
  
  double value = 0;
  
  for( unsigned int line = 0 ; line < nLines ; ++line ) 
  {
    for( unsigned int col = 0 ; col < nCols ; ++col ) 
    {
      inputMatrix.getValue( line, col, value );
      
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( col, line, 
        MIN( value, 254.0 ), 0 ),
        "Error writing raster" )
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( col, line, 
        MIN( value, 254.0 ), 1 ),
        "Error writing raster" )
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( col, line, 
        MIN( value, 254.0 ), 2 ),
        "Error writing raster" )                
    }  
  }
  
  /* Draw maxima points */
  
  MaximasListT::const_iterator maximas_it = 
    maximaPoints.begin();
  MaximasListT::const_iterator maximas_it_end = 
    maximaPoints.end();
  unsigned int xValue = 0;
  unsigned int yValue = 0;
    
  while( maximas_it != maximas_it_end ) 
  {
    xValue = maximas_it->x_;
    yValue = maximas_it->y_;
    
    if( ( xValue < nCols ) && ( yValue < nLines ) )
    {
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( xValue, yValue, 
        255.0, 0 ),
        "Error writing raster" )
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( xValue, yValue, 
        0.0, 1 ),
        "Error writing raster" )        
      TEAGN_TRUE_OR_RETURN( outRaster.setElement( xValue, yValue, 
        0.0, 2 ),
        "Error writing raster" )        
    }
      
    ++maximas_it;
  }  

  return true;
}
