#include "TePDIOFMatching.hpp"

#include "TePDIUtils.hpp"

#include "../kernel/TeThreadFunctor.h"
#include "../kernel/TeMutex.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeGTFactory.h"
#include "../kernel/TeDefines.h"

#include <math.h>

#ifndef M_PI
  #define M_PI       3.14159265358979323846
#endif
#ifndef M_PI_2
  #define M_PI_2     1.57079632679489661923
#endif
#ifndef M_PI_4
  #define M_PI_4     0.785398163397448309616
#endif

TePDIOFMatching::TePDIOFMatching()
{
}


TePDIOFMatching::~TePDIOFMatching()
{
}


bool TePDIOFMatching::RunImplementation()
{
  /* Retriving Parameters */
  
  TePDITypes::TePDIRasterPtrType input_image1_ptr;
  params_.GetParameter( "input_image1_ptr", input_image1_ptr );  
  
  unsigned int input_channel1 = 0;
  params_.GetParameter( "input_channel1", input_channel1 );   
  
  TePDITypes::TePDIRasterPtrType input_image2_ptr;
  params_.GetParameter( "input_image2_ptr", input_image2_ptr );  
  
  unsigned int input_channel2 = 0;
  params_.GetParameter( "input_channel2", input_channel2 );   

  TeSharedPtr< TeCoordPairVect > out_tie_points_ptr;
  params_.GetParameter( "out_tie_points_ptr", out_tie_points_ptr );   
  
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
  
  bool enable_multi_thread = false;
  if( params_.CheckParameter< int >( "enable_multi_thread" ) ) {
    enable_multi_thread = true;
  }
  
  bool skip_geom_filter = false;
  if( params_.CheckParameter< int >( "skip_geom_filter" ) ) {
    skip_geom_filter = true;
  }  
  
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
  
  unsigned int max_tie_points = 529;
  if( params_.CheckParameter< unsigned int >( "max_tie_points" ) ) {
    params_.GetParameter( "max_tie_points", max_tie_points );
  }    
  
  double corr_sens = 0.5;
  if( params_.CheckParameter< double >( "corr_sens" ) ) {
    params_.GetParameter( "corr_sens", corr_sens );
  }    
  
  unsigned int corr_window_width = 21;
  if( params_.CheckParameter< unsigned int >( 
    "corr_window_width" ) ) {
    
    params_.GetParameter( "corr_window_width", corr_window_width );
  }     
  
  unsigned int max_size_opt = 0;
  if( params_.CheckParameter< unsigned int >( "max_size_opt" ) ) {
    params_.GetParameter( "max_size_opt", max_size_opt );
  }   
  
  double maximas_sens = 0;
  if( params_.CheckParameter< double >( "maximas_sens" ) ) {
    params_.GetParameter( "maximas_sens", maximas_sens );
  }      
  
  /* Calculating the number of allowed threads - this process
     not included into the count */
     
  unsigned int max_threads = 0;
  std::vector< TeThreadFunctor::pointer > threads_vector;
  
  if( enable_multi_thread ) {
    max_threads = TeGetPhysProcNumber() - 1;
    
    for( unsigned int index = 0 ; index < max_threads ; ++index ) {
      TeThreadFunctor::pointer aux_thread_ptr( new TeThreadFunctor );
    
      threads_vector.push_back( aux_thread_ptr );
    }
  }
  
  /* Calculating the box postion related values */
  
  const unsigned int orig_box1_x_off = (unsigned int)input_box1.lowerLeft().x();
  const unsigned int orig_box1_y_off = (unsigned int)input_box1.upperRight().y();
  const unsigned int orig_box1_nlines = 1 + (unsigned int)ABS( input_box1.height() );
  const unsigned int orig_box1_ncols = 1 + (unsigned int)ABS( input_box1.width() );
  
  const unsigned int orig_box2_x_off = (unsigned int)input_box2.lowerLeft().x();
  const unsigned int orig_box2_y_off = (unsigned int)input_box2.upperRight().y();
  const unsigned int orig_box2_nlines = 1 + (unsigned int)ABS( input_box2.height() );
  const unsigned int orig_box2_ncols = 1 + (unsigned int)ABS( input_box2.width() );
    
  /* Calculating the rescale factors 
     rescaled_image = original_image * rescale_factor */
  
  double img1_x_rescale_factor = 1.0;
  double img1_y_rescale_factor = 1.0;
  double img2_x_rescale_factor = 1.0;
  double img2_y_rescale_factor = 1.0;
  {
    double mean_pixel_relation = ( pixel_x_relation + pixel_y_relation ) /
      2.0;
      
    if( mean_pixel_relation > 1.0 ) {
      /* The image 1 has poor resolution - bigger pixel resolution values -
         and image 2 needs to be rescaled down */
      
      img2_x_rescale_factor = 1.0 / pixel_x_relation;
      img2_y_rescale_factor = 1.0 / pixel_y_relation;
    } else if( mean_pixel_relation < 1.0 ) {
      /* The image 2 has poor resolution - bigger pixel resolution values
        and image 1 needs to be rescaled down */
      
      img1_x_rescale_factor = pixel_x_relation;
      img1_y_rescale_factor = pixel_y_relation;
    }
  } 
  
  if( max_size_opt )
  {
    double resc_box1_size = 
      ( (double)orig_box1_nlines  ) * img1_y_rescale_factor *
      ( (double)orig_box1_ncols ) * img1_x_rescale_factor;
    double resc_box2_size = 
      ( (double)orig_box2_nlines  ) * img2_y_rescale_factor *
      ( (double)orig_box2_ncols ) * img2_x_rescale_factor;
    double max_size_opt_double = (double)max_size_opt;
    
    if( ( resc_box1_size > max_size_opt_double ) || 
      ( resc_box2_size > max_size_opt_double ) )
    {
      double opt_rescale_factor = 1.0;
      
      if( resc_box1_size > resc_box2_size )
      {
        opt_rescale_factor = sqrt( resc_box1_size / max_size_opt_double );
      }
      else
      {
        opt_rescale_factor = sqrt( resc_box2_size / max_size_opt_double );
      }
      
      img1_x_rescale_factor *= opt_rescale_factor;
      img1_y_rescale_factor *= opt_rescale_factor;
      img2_x_rescale_factor *= opt_rescale_factor;
      img2_y_rescale_factor *= opt_rescale_factor;
    }
  }
  
  /* Calculating the max tie-points to be generated for each image
     trying to keep the same tie-point density for both images */
     
  unsigned int img1_max_tps = 0;
  unsigned int img2_max_tps = 0;
  
  {
    double resc_box1_nlines =  ( (double)orig_box1_nlines  ) * 
      img1_y_rescale_factor;
    double resc_box1_ncols =  ( (double)orig_box1_ncols ) * 
      img1_x_rescale_factor;
    double resc_box2_nlines =  ( (double)orig_box2_nlines  ) * 
      img2_y_rescale_factor;
    double resc_box2_ncols =  ( (double)orig_box2_ncols ) * 
      img2_x_rescale_factor;
      
    double resc_box1_area = resc_box1_nlines * resc_box1_ncols;
    double resc_box2_area = resc_box2_nlines * resc_box2_ncols;
      
    if( resc_box1_area > resc_box2_area )
    {
      img2_max_tps = max_tie_points;
      
      img1_max_tps = (unsigned int)( 
        resc_box1_area / 
        ( resc_box2_area / ( (double)max_tie_points ) ) );
    }
    else
    {
      img1_max_tps = max_tie_points;
      
      img2_max_tps = (unsigned int)( 
        resc_box2_area / 
        ( resc_box1_area / ( (double)max_tie_points ) ) ); 
    }
  }
  
  /* Loading images */
  
  TeMutex glb_mem_lock; //global memory lock
  
  ImgMatrixT img1_matrix;
  ImgMatrixT img2_matrix;
 
  {
    TeThreadParameters tparams1;
    tparams1.store( "input_image_ptr", input_image1_ptr );
    tparams1.store( "img_matrix_ptr", &img1_matrix );
    tparams1.store( "img_x_rescale_factor", img1_x_rescale_factor );
    tparams1.store( "img_y_rescale_factor", img1_y_rescale_factor );
    tparams1.store( "box_x_off", orig_box1_x_off );
    tparams1.store( "box_y_off", orig_box1_y_off );
    tparams1.store( "box_nlines", orig_box1_nlines );
    tparams1.store( "box_ncols", orig_box1_ncols );
    tparams1.store( "input_channel", input_channel1 );
    tparams1.store( "progress_enabled", progress_enabled_ );
    tparams1.store( "glb_mem_lock_ptr", &glb_mem_lock );
    
    TeThreadParameters tparams2;
    tparams2.store( "input_image_ptr", input_image2_ptr );
    tparams2.store( "img_matrix_ptr", &img2_matrix );
    tparams2.store( "img_x_rescale_factor", img2_x_rescale_factor );
    tparams2.store( "img_y_rescale_factor", img2_y_rescale_factor );
    tparams2.store( "box_x_off", orig_box2_x_off );
    tparams2.store( "box_y_off", orig_box2_y_off );
    tparams2.store( "box_nlines", orig_box2_nlines );
    tparams2.store( "box_ncols", orig_box2_ncols );
    tparams2.store( "input_channel", input_channel2 );
    tparams2.store( "progress_enabled", progress_enabled_ );
    tparams2.store( "glb_mem_lock_ptr", &glb_mem_lock );
  
    if( max_threads > 0 ) {
      threads_vector[ 0 ]->setStartFunctPtr( loadImage );
      threads_vector[ 0 ]->setParameters( tparams1 );
      threads_vector[ 0 ]->start();
   
      if( ! loadImage( tparams2 ) ) {
        threads_vector[ 0 ]->waitToFinish();
        
        TEAGN_LOG_AND_RETURN( "Unable to load image 2" )
      } else {
        threads_vector[ 0 ]->waitToFinish();
        
        TEAGN_TRUE_OR_RETURN( threads_vector[ 0 ]->getReturnValue(),
          "Unable to load image 1" )
      }
    } else {
      TEAGN_TRUE_OR_RETURN( loadImage( tparams1 ),
        "Error loading image 1" )
      TEAGN_TRUE_OR_RETURN( loadImage( tparams2 ),
        "Error loading image 2" )
    }
  }
  
/* remove */        
//matrix2Tiff( img1_matrix, "img1_matrix.tif", 
//  std::vector< TeCoord2D >() );
//matrix2Tiff( img2_matrix, "img2_matrix.tif",
//  std::vector< TeCoord2D >() );
/* remove */  
  
  /* Generating the correlation windows and the maxima points */
  
  std::vector< TeCoord2D > img1_maxima_points;
  std::vector< TeCoord2D > img2_maxima_points;
  
  ImgMatrixT img1_features_matrix;
  img1_features_matrix.Reset( ImgMatrixT::RAMMemPol );
    /* each line is a stacked version
       of one rotated correlation 
       window */
       
  ImgMatrixT img2_features_matrix;
  img2_features_matrix.Reset( ImgMatrixT::RAMMemPol );
    /* each line is a stacked version
       of one rotated correlation 
       window */
  {
    TeThreadParameters tparams1;
    tparams1.store( "img_matrix_ptr", &img1_matrix );
    tparams1.store( "corr_window_width", corr_window_width );
    tparams1.store( "img_maxima_points_ptr", &img1_maxima_points );
    tparams1.store( "img_features_matrix_ptr", &img1_features_matrix );
    tparams1.store( "progress_enabled", progress_enabled_ );
    tparams1.store( "max_points", img1_max_tps );
    tparams1.store( "glb_mem_lock_ptr", &glb_mem_lock );
    tparams1.store( "maximas_sens", maximas_sens );
    
    TeThreadParameters tparams2;
    tparams2.store( "img_matrix_ptr", &img2_matrix );
    tparams2.store( "corr_window_width", corr_window_width );
    tparams2.store( "img_maxima_points_ptr", &img2_maxima_points );
    tparams2.store( "img_features_matrix_ptr", &img2_features_matrix );    
    tparams2.store( "progress_enabled", progress_enabled_ );
    tparams2.store( "max_points", img2_max_tps );
    tparams2.store( "glb_mem_lock_ptr", &glb_mem_lock );
    tparams2.store( "maximas_sens", maximas_sens );
    
    if( max_threads > 0 ) {
      threads_vector[ 0 ]->setStartFunctPtr( generateFeatures );
      threads_vector[ 0 ]->setParameters( tparams1 );
      threads_vector[ 0 ]->start();
    
      if( ! generateFeatures( tparams2 ) ) {
        threads_vector[ 0 ]->waitToFinish();
        
        TEAGN_LOG_AND_RETURN( "Unable to the correlation windows from image 2" )
      } else {
        threads_vector[ 0 ]->waitToFinish();
        
        TEAGN_TRUE_OR_RETURN( threads_vector[ 0 ]->getReturnValue(),
          "Unable to the correlation windows from image 1" )
      }      
    } else {
      TEAGN_TRUE_OR_RETURN( generateFeatures( tparams1 ),
        "Error generating the correlation windows from image 1" )
      TEAGN_TRUE_OR_RETURN( generateFeatures( tparams2 ),
        "Error generating the correlation windows from image 2" )        
    }
    
    if( ( img1_maxima_points.size() == 0 ) ||
      ( img2_maxima_points.size() == 0 ) )
    {
      /* No maximas found in one image */
      
      return true;
    }
    
/* remove */
//matrix2Tiff( img1_matrix, "img1_matrix_with_maxima_points.tif", 
//  img1_maxima_points );
//matrix2Tiff( img2_matrix, "img2_matrix_with_maxima_points.tif", 
//  img2_maxima_points );
/* remove */
    
    /* Bringing img1_maxima_points into input_image1_ptr 
       reference */
 
    std::vector< TeCoord2D >::iterator maximas_it = 
      img1_maxima_points.begin();
    std::vector< TeCoord2D >::iterator maximas_it_end = 
      img1_maxima_points.end();
      
    while( maximas_it != maximas_it_end ) {
      (*maximas_it) = 
        TeCoord2D( 
          ( (*maximas_it).x() / img1_x_rescale_factor ) +
            ( (double)orig_box1_x_off ),
          ( (*maximas_it).y() / img1_y_rescale_factor ) +
            ( (double)orig_box1_y_off )
        );
        
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->x() < input_image1_ptr->params().ncols_ ),
        "Invalid x coord generated" )
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->x() >= 0 ),
        "Invalid x coord generated" )        
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->y() < input_image1_ptr->params().nlines_ ),
        "Invalid y coord generated" )
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->y() >= 0 ),
        "Invalid y coord generated" )           
        
      ++maximas_it;
    }
    
    /* Bringing img2_maxima_points into input_image2_ptr 
       reference */    
    
    maximas_it = img2_maxima_points.begin();
    maximas_it_end = img2_maxima_points.end();
      
    while( maximas_it != maximas_it_end ) {
      (*maximas_it) = 
        TeCoord2D( 
          ( (*maximas_it).x() / img2_x_rescale_factor ) +
            ( (double)orig_box2_x_off ),
          ( (*maximas_it).y() / img2_y_rescale_factor ) +
            ( (double)orig_box2_y_off )
        );
      
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->x() < input_image2_ptr->params().ncols_ ),
        "Invalid x coord generated" )
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->x() >= 0 ),
        "Invalid x coord generated" )         
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->y() < input_image2_ptr->params().nlines_ ),
        "Invalid y coord generated" ) 
      TEAGN_DEBUG_CONDITION( 
        ( maximas_it->y() >= 0 ),
        "Invalid y coord generated" )                 
        
      ++maximas_it;
    }    
  }
  
  TEAGN_DEBUG_CONDITION( ( img1_maxima_points.size() ==
    img1_features_matrix.GetLines() ),
    "Features number mismatch" );
  TEAGN_DEBUG_CONDITION( ( img2_maxima_points.size() ==
    img2_features_matrix.GetLines() ),
    "Features number mismatch" ); 
    
  TEAGN_DEBUG_CONDITION( checkMaximaPoints( img1_maxima_points ),
    "Invalid img1 maximas" )
  TEAGN_DEBUG_CONDITION( checkMaximaPoints( img2_maxima_points ),
    "Invalid img2 maximas" )
    
  /* Free anused resources */
  
  img1_matrix.Reset();
  img2_matrix.Reset();

/* remove */
//raster2Tiff( input_image1_ptr, input_channel1,
//  "orig_img1_plus_all_maximas.tif",
//  img1_maxima_points,TeSharedPtr< TeCoordPairVect >(), 0 );
//raster2Tiff( input_image2_ptr, input_channel2,
//  "orig_img2_plus_all_maximas.tif",
//  img2_maxima_points, TeSharedPtr< TeCoordPairVect >(), 0 ); 
/* remove */
  
  
/* remove */
//features2Tiff( corr_window_width, img1_maxima_points, 
//  img1_features_matrix, "img1" );
//features2Tiff( corr_window_width, img2_maxima_points, 
//  img2_features_matrix, "img2" );
/* remove */   
  
  /* Matching the correlation windows */
  
  /* A vector with the matrix 2 features indexes with
    the best correlation value for each matrix 1 element 
    features2_indexes size = img1_features_matrix size */
  std::vector< unsigned int > features2_indexes;
  
  /* A vector with the matrix 1 features indexes with
    the best correlation value for each matrix 2 element 
    features1_indexes size = img2_features_matrix size */
  std::vector< unsigned int > features1_indexes;
      
  {
    /* lock objects for the two vectors */
    TeMutex globalMutex;
   
    TeThreadParameters tparams;
    tparams.store( "features2_indexes_ptr", &features2_indexes );
    tparams.store( "features1_indexes_ptr", &features1_indexes );
    tparams.store( "global_mutex_ptr", 
      &globalMutex );
    tparams.store( "img1_features_matrix_ptr", 
      &img1_features_matrix );
    tparams.store( "img2_features_matrix_ptr", 
      &img2_features_matrix );
    tparams.store( "progress_enabled", progress_enabled_ );
    tparams.store( "corr_sens", corr_sens );
    
    if( max_threads > 0 ) 
    {
      for( unsigned int thr_vec_ind = 0 ;
        thr_vec_ind < threads_vector.size() ; ++thr_vec_ind ) {
        
        threads_vector[ thr_vec_ind ]->setStartFunctPtr( 
          locateBestFeaturesMatching );
          
        threads_vector[ thr_vec_ind ]->setParameters( tparams );
        threads_vector[ thr_vec_ind ]->start();
      }
        
      if( ! locateBestFeaturesMatching( tparams ) ) {
        for( unsigned int thr_vec_ind = 0 ;
          thr_vec_ind < threads_vector.size() ; ++thr_vec_ind ) {
          
          threads_vector[ thr_vec_ind ]->waitToFinish();
        }
        
        TEAGN_LOG_AND_RETURN( 
          "Error locating the features matching for matrix 1" )
      } else {
        bool one_thread_returned_error = false;
        
        for( unsigned int thr_vec_ind = 0 ;
          thr_vec_ind < threads_vector.size() ; ++thr_vec_ind ) {
          
          threads_vector[ thr_vec_ind ]->waitToFinish();
          
          if( ! threads_vector[ thr_vec_ind ]->getReturnValue() ) {
            one_thread_returned_error = true;
          }
        }
        
        TEAGN_TRUE_OR_RETURN( ( ! one_thread_returned_error ),
          "Error locating the features matching" )
      }      
    } else {
      TEAGN_TRUE_OR_RETURN( locateBestFeaturesMatching( tparams ),
        "Error locating the features matching" );
    }
  }
  
  TEAGN_DEBUG_CONDITION( ( img1_maxima_points.size() ==
    features2_indexes.size() ),
    "Features number mismatch" );
  TEAGN_DEBUG_CONDITION( ( img2_maxima_points.size() ==
    features1_indexes.size() ),
    "Features number mismatch" );
    
  /* Free anused resources */
  
  img1_features_matrix.Reset();  
  img2_features_matrix.Reset();
    
  /* Generating tie-points */
  
  out_tie_points_ptr->clear();
   
  {
    unsigned int features2_indexes_size = features2_indexes.size();
    unsigned int features1_indexes_size = features1_indexes.size();
  
    unsigned int f1i_index = 0;//index over features1_indexes
    unsigned int f2i_index = 0;//index over features2_indexes
 
    /* features1_indexes pointed indexes */
    unsigned int f1_pointed_index = 0;
    /* features2_indexes pointed indexes */
    unsigned int f2_pointed_index = 0;
    
    /* Getting the matchings priorizing the pointings
       indicated by features2_indexes */
  
    for( f2i_index = 0 ; f2i_index < features2_indexes_size ;
      ++f2i_index ) {
      
      f2_pointed_index = features2_indexes[ f2i_index ];
      
      if( f2_pointed_index < features1_indexes_size ) {
        f1_pointed_index = features1_indexes[ f2_pointed_index ];
          
        if( f1_pointed_index == f2i_index ) {
         
          out_tie_points_ptr->push_back( TeCoordPair(
            img1_maxima_points[ f2i_index ],
            img2_maxima_points[ f2_pointed_index ] ) ); 
            
          /* Invalidating the pointings to avoid duplicated 
            matches */
            
          features2_indexes[ f2i_index ] = features1_indexes_size;
          features1_indexes[ f2_pointed_index ] =
            features2_indexes_size;
        }
      }
    }
    
    /* Getting the remaining matchings priorizing the pointings
       indicated by features1_indexes */
    
    for( f1i_index = 0 ; f1i_index < features1_indexes_size ;
      ++f1i_index ) {
      
      f1_pointed_index = features1_indexes[ f1i_index ];
      
      if( f1_pointed_index < features2_indexes_size ) {
        
        f2_pointed_index = features2_indexes[ f1_pointed_index ];
        
        if( f2_pointed_index == f1i_index ) {
         
          out_tie_points_ptr->push_back( TeCoordPair(
            img1_maxima_points[ f1_pointed_index ],
            img2_maxima_points[ f1i_index ] ) ); 
            
          /* Invalidating the pointings to avoid duplicated 
            matches */
            
          features2_indexes[ f1_pointed_index ] = features1_indexes_size;
          features1_indexes[ f1i_index ] = features2_indexes_size;            
        }
      }
    }    

  }
  
  TEAGN_DEBUG_CONDITION( checkTPs( *out_tie_points_ptr ),
    "Invalid tie-points" )
  
  /* Free anused resources */
  
  img1_maxima_points.clear();
  img2_maxima_points.clear();  
  
  features2_indexes.clear();
  features1_indexes.clear();
  
/* remove */
//raster2Tiff( input_image1_ptr, input_channel1,
//  "orig_img1_plus_all_tp.tif",
//  std::vector< TeCoord2D >(), out_tie_points_ptr, 0 );
//raster2Tiff( input_image2_ptr, input_channel2,
//  "orig_img2_plus_all_tp.tif",
//  std::vector< TeCoord2D >(), out_tie_points_ptr, 1 );  
/* remove */     
  
  /* Doing geometrical filtering using user supplied 
     geometrical transformation parameters */
     
  if( ! skip_geom_filter ) {
    gt_params.tiepoints_ = (*out_tie_points_ptr);
       
    TeGeometricTransformation::pointer trans_ptr( 
      TeGTFactory::make( gt_params ) );
    if( trans_ptr->reset( gt_params ) ) {
      trans_ptr->getParameters( gt_params );
      
      /* updating the output tie points */
      
      (*out_tie_points_ptr) = gt_params.tiepoints_;
      
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
  
/* remove */
//raster2Tiff( input_image1_ptr, input_channel1,
//  "orig_img1_plus_filtered_tp.tif",
//  std::vector< TeCoord2D >(), out_tie_points_ptr, 0 );
//raster2Tiff( input_image2_ptr, input_channel2,
//  "orig_img2_plus_filtered_tp.tif",
//  std::vector< TeCoord2D >(), out_tie_points_ptr, 1 );  
/* remove */   
  
  return true;
}


bool TePDIOFMatching::CheckParameters( 
  const TePDIParameters& parameters ) const
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
  
  /* Checking max_tie_points */
  
  unsigned int max_tie_points = 0;
  if( parameters.CheckParameter< unsigned int >( "max_tie_points" ) ) {
    parameters.GetParameter( "max_tie_points", max_tie_points );
    
    TEAGN_TRUE_OR_RETURN( ( max_tie_points > 0 ),
      "Invalid parameter : max_tie_points" )    
  }
  
  /* Checking corr_sens */
  
  double corr_sens = 0;
  if( parameters.CheckParameter< double >( "corr_sens" ) ) {
    parameters.GetParameter( "corr_sens", corr_sens );
    
    TEAGN_TRUE_OR_RETURN( ( corr_sens > 0.0 ) &&
      ( corr_sens <= 3.0 ),
      "Invalid parameter : corr_sens" )
  }  
  
  /* Checking corr_window_width */
  
  unsigned int corr_window_width = 0;
  if( parameters.CheckParameter< unsigned int >( 
    "corr_window_width" ) ) {
    
    parameters.GetParameter( "corr_window_width", 
      corr_window_width );
      
    TEAGN_TRUE_OR_RETURN( ( ( corr_window_width % 2 ) > 0 ),
      "Invalid parameter : corr_window_width" )      
    
    TEAGN_TRUE_OR_RETURN( ( corr_window_width >= 3 ),
      "Invalid parameter : corr_window_width" )
  }    
  
  /* Checking maximas_sens */
  
  double maximas_sens = 0;
  if( parameters.CheckParameter< double >( "maximas_sens" ) ) {
    parameters.GetParameter( "maximas_sens", maximas_sens );
    
    TEAGN_TRUE_OR_RETURN( ( maximas_sens >= 0.0 ) &&
      ( maximas_sens <= 1.0 ),
      "Invalid parameter : maximas_sens" )
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
  
  return true;
}


void TePDIOFMatching::ResetState( const TePDIParameters& )
{
}


bool TePDIOFMatching::loadImage( const TeThreadParameters& params )
{
  /* Retriving parameters */
  
  TePDITypes::TePDIRasterPtrType input_image_ptr;
  TEAGN_TRUE_OR_THROW( params.retrive( "input_image_ptr", input_image_ptr ),
    "Missing thread parameter" )
    
  ImgMatrixT* img_matrix_ptr;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_matrix_ptr", img_matrix_ptr ),
    "Missing thread parameter" )
    
  double img_x_rescale_factor = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_x_rescale_factor", 
    img_x_rescale_factor ), "Missing thread parameter" )
    
  double img_y_rescale_factor = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_y_rescale_factor", 
    img_y_rescale_factor ), "Missing thread parameter" )    
    
  unsigned int input_channel = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "input_channel", 
    input_channel ), "Missing thread parameter" )  
    
  unsigned int box_x_off = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "box_x_off", 
    box_x_off ), "Missing thread parameter" )  

  unsigned int box_y_off = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "box_y_off", 
    box_y_off ), "Missing thread parameter" )      
    
  unsigned int box_nlines = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "box_nlines", 
    box_nlines ), "Missing thread parameter" )      
    
  unsigned int box_ncols = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "box_ncols", 
    box_ncols ), "Missing thread parameter" )   
    
  bool progress_enabled = false;   
  TEAGN_TRUE_OR_THROW( params.retrive( "progress_enabled", 
    progress_enabled ), "Missing thread parameter" ) 
    
  TeMutex* glb_mem_lock_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "glb_mem_lock_ptr", 
    glb_mem_lock_ptr ), "Missing thread parameter" )   
  TeMutex& glb_mem_lock = *glb_mem_lock_ptr;
         
  /* Rescaling image */
  
  unsigned int nlines = (unsigned int) ceil( 
    ( (double)box_nlines ) * img_y_rescale_factor );
  unsigned int ncols = (unsigned int) ceil( 
    ( (double)box_ncols ) * img_x_rescale_factor );    
  
  glb_mem_lock.lock();
  
  if( ! img_matrix_ptr->Reset( nlines, ncols, 
    TePDIMatrix< double >::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Unable to allocate space for the loaded image" )
    glb_mem_lock.unLock();
    return false;
  }
    
  glb_mem_lock.unLock();
    
  unsigned int curr_out_line = 0;
  unsigned int curr_out_col = 0;
  unsigned int curr_input_line = 0;
  unsigned int curr_input_col = 0;
  double value = 0;
  
  TePDIPIManager progress( "Rescaling image", nlines, progress_enabled );
  
  for( curr_out_line = 0 ; curr_out_line < nlines ; 
    ++curr_out_line ) {
    
    curr_input_line = 
      TeRound( 
        ( 
          ( (double)curr_out_line ) / img_y_rescale_factor
        ) 
        +
        ( (double) box_y_off )
      );
    
    for( curr_out_col = 0 ; curr_out_col < ncols ; 
      ++curr_out_col ) {
      
      curr_input_col = 
        TeRound( 
          ( 
            ( (double)curr_out_col ) / img_x_rescale_factor
          ) 
          +
          ( (double) box_x_off )
        );        
      
      if( input_image_ptr->getElement( curr_input_col, curr_input_line, value, 
        input_channel ) ) {
        
        img_matrix_ptr->operator()( curr_out_line, curr_out_col ) = value;
      } else {
        img_matrix_ptr->operator()( curr_out_line, curr_out_col ) = 0;
      }
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(),
      "Canceled by the user" );
  }
        
  return true;
}


bool TePDIOFMatching::generateFeatures( 
  const TeThreadParameters& params )
{
  /* Retriving parameters */
  
  ImgMatrixT* img_matrix_ptr;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_matrix_ptr", img_matrix_ptr ),
    "Missing thread parameter" )
    
  unsigned int corr_window_width = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "corr_window_width", corr_window_width ),
    "Missing thread parameter" )
  TEAGN_DEBUG_CONDITION( ( corr_window_width >= 3 ),
    "Invalid correlation window width" )    
  TEAGN_DEBUG_CONDITION( ( ( corr_window_width % 2 ) != 0 ),
    "Invalid correlation window width" )
    
  std::vector< TeCoord2D >* img_maxima_points_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_maxima_points_ptr", 
    img_maxima_points_ptr ), "Missing thread parameter" )
    
  ImgMatrixT* img_features_matrix_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img_features_matrix_ptr", 
    img_features_matrix_ptr ), "Missing thread parameter" )
    
  bool progress_enabled = false;   
  TEAGN_TRUE_OR_THROW( params.retrive( "progress_enabled", 
    progress_enabled ), "Missing thread parameter" )     
    
  unsigned int max_points = 0;   
  TEAGN_TRUE_OR_THROW( params.retrive( "max_points", 
    max_points ), "Missing thread parameter" )  
    
  TeMutex* glb_mem_lock_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "glb_mem_lock_ptr", 
    glb_mem_lock_ptr ), "Missing thread parameter" )   
  TeMutex& glb_mem_lock = *glb_mem_lock_ptr;   
  
  double maximas_sens = 0;   
  TEAGN_TRUE_OR_THROW( params.retrive( "maximas_sens", 
    maximas_sens ), "Missing thread parameter" )         
    
  /* Filtering image */
  
  ImgMatrixT filtered_img_matrix;
  TEAGN_TRUE_OR_RETURN( gaussianSmoothing( *img_matrix_ptr, 
    filtered_img_matrix, 1, progress_enabled, glb_mem_lock ),
    "Error filtering image" )
    
  /* Get the least squares surface */
  
  ImgMatrixT ls_surface_matrix;
  TEAGN_TRUE_OR_RETURN( generateLSSurface( 
    filtered_img_matrix, 
    ls_surface_matrix, progress_enabled, glb_mem_lock ),
    "Error getting the least squares surface" )   
    
/* remove */    
//matrix2Tiff( ls_surface_matrix, "ls_surface_matrix.tif",
//  std::vector< TeCoord2D >() );     
/* remove */    
    
  /* Free unused resources */
  
  filtered_img_matrix.Reset();

  /* Get the maxima points */
  
  TEAGN_TRUE_OR_RETURN( getMaximaPoints( ls_surface_matrix,
    *img_maxima_points_ptr, max_points, corr_window_width, 
    progress_enabled, maximas_sens ),
    "Error generating maxima points" );
  TEAGN_DEBUG_CONDITION( ( img_maxima_points_ptr->size() <=
    max_points ), "Invalid number of generated maxima points" )

/* remove */    
//matrix2Tiff( *img_matrix_ptr, "croped_img_plus_maximas.tif",
//  *img_maxima_points_ptr );     
/* remove */
    
  /* Generating the rotated correlation windows matrix */
  
  if( img_maxima_points_ptr->size() > 0 ) {
//    TEAGN_TRUE_OR_RETURN( img_features_matrix_ptr->Reset( 
//      corr_window_width * corr_window_width, 
//      img_maxima_points_ptr->size(), ImgMatrixT::RAMMemPol ),
//      "Error allocating image features matrix" )  
      
    TEAGN_TRUE_OR_RETURN( generateCorrWindows( *img_matrix_ptr,
      corr_window_width, *img_maxima_points_ptr, 
      *img_features_matrix_ptr, progress_enabled, glb_mem_lock ),
      "Error generating the correlation windows" )
  } else {
    img_features_matrix_ptr->Reset();
  }
    
  return true;
}


bool TePDIOFMatching::gaussianSmoothing( 
  const ImgMatrixT& input_matrix,
  ImgMatrixT& output_matrix, 
  unsigned int iterations,
  bool progress_enabled,
  TeMutex& glb_mem_lock )
{        
  TEAGN_TRUE_OR_THROW( ( iterations > 0 ), "Invalid iterations" )
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetColumns() > 2 ),
    "Invalid columns number" )
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetLines() > 2 ),
    "Invalid lines number" )   
    
  unsigned int nlines = input_matrix.GetLines();
  unsigned int ncols = input_matrix.GetColumns();
  unsigned int last_line_index = nlines - 1;
  unsigned int last_col_index = ncols - 1;  
  unsigned int curr_line = 0;
  unsigned int curr_col = 0;   
    
  glb_mem_lock.lock();  
    
  if( !output_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" )
    glb_mem_lock.unLock();
    return false;
  }
    
  glb_mem_lock.unLock();
  
  /* Fill borders with zero */
  
  for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
    output_matrix( curr_line, 0 ) = 0.0;
    output_matrix( curr_line, last_col_index ) = 0.0;
  }
  
  for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
    output_matrix( 0, curr_col ) = 0.0;
    output_matrix( last_line_index, curr_col ) = 0.0;
  }  
  
  /* Smoothing */
  
  TePDIPIManager progress( "Filtering image", nlines - 2, progress_enabled );
  
  for( curr_line = 1 ; curr_line < last_line_index ; ++curr_line ) {
    for( curr_col = 1 ; curr_col < last_col_index ; ++curr_col ) {
      output_matrix( curr_line, curr_col ) = 
        ( 
          input_matrix( curr_line - 1, curr_col ) +
          ( 4.0 * input_matrix( curr_line, curr_col ) ) +
          input_matrix( curr_line + 1, curr_col ) +
          input_matrix( curr_line, curr_col - 1 ) +
          input_matrix( curr_line, curr_col + 1 )        
        ) / 8.0;
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(),
      "Canceled by the user" );
  }
  
  return true;
}


bool TePDIOFMatching::generateLSSurface( 
  const ImgMatrixT& input_matrix,
  ImgMatrixT& ls_surface_matrix, 
  bool progress_enabled,
  TeMutex& glb_mem_lock )
{        
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetColumns() > 2 ),
    "Invalid columns number" )
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetLines() > 2 ),
    "Invalid lines number" )   
    
  const unsigned int nlines = input_matrix.GetLines();
  const unsigned int ncols = input_matrix.GetColumns();
  
  TePDIPIManager progress( "Creating LS surface",
    ( nlines - 2 ) + ( 9 * ( nlines - 2 ) ) + nlines, 
    progress_enabled );

  /* Creating gx and gy */
  
  glb_mem_lock.lock();
  
  ImgMatrixT gx_matrix;
  if( ! gx_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" )
    glb_mem_lock.unLock();
    return false;
  }
  
  ImgMatrixT gy_matrix;
  if( ! gy_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" )
    glb_mem_lock.unLock();
    return false;
  }
    
  glb_mem_lock.unLock();
  
  {
    unsigned int curr_line = 0;
    unsigned int curr_col = 0; 
    const unsigned int last_line_index = nlines - 1;
    const unsigned int last_col_index = ncols - 1;      
    
    /* Border fill */
    
    for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
      gx_matrix( curr_line, 0 ) = 0.0;
      gx_matrix( curr_line, last_col_index ) = 0.0;
      
      gy_matrix( curr_line, 0 ) = 0.0;
      gy_matrix( curr_line, last_col_index ) = 0.0;      
    }
    for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
      gx_matrix( 0, curr_col ) = 0.0;
      gx_matrix( last_line_index, curr_col ) = 0.0;
      
      gy_matrix( 0, curr_col ) = 0.0;
      gy_matrix( last_line_index, curr_col ) = 0.0;
    }  
    
    /* Calculating values */
    
    for( curr_line = 1 ; curr_line < last_line_index ; ++curr_line ) { 
      for( curr_col = 1 ; curr_col < last_col_index ; ++curr_col ) { 
        gx_matrix( curr_line, curr_col ) =
          ( input_matrix( curr_line + 1, curr_col ) -
            input_matrix( curr_line - 1, curr_col ) 
          ) / 2.0;
          
        gy_matrix( curr_line, curr_col ) =
          ( input_matrix( curr_line, curr_col + 1 ) -
            input_matrix( curr_line, curr_col - 1 ) 
          ) / 2.0;          
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
  }  
  
  /* Creating gxx and gxy and gyy */
  
  glb_mem_lock.lock();
  
  ImgMatrixT gxx_matrix;
  if( ! gxx_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" )
    glb_mem_lock.unLock();
    return false;
  }
  
  ImgMatrixT gxy_matrix;
  if( ! gxy_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" )
    glb_mem_lock.unLock();
    return false;
  }
  
  ImgMatrixT gyy_matrix;
  if( ! gyy_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
  {
    TEAGN_LOGERR( "Error reseting matrix" ) 
    glb_mem_lock.unLock();
    return false;
  }
    
  glb_mem_lock.unLock();
     
  {
    /* zero fill */
    
    {
      unsigned int curr_line = 0;
      unsigned int curr_col = 0; 
      
      for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
        for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
          gxx_matrix( curr_line, curr_col ) = 0.0;
          gxy_matrix( curr_line, curr_col ) = 0.0;
          gyy_matrix( curr_line, curr_col ) = 0.0;
        }
      }
    }
    
    /* Allocating p matrix */
    
    glb_mem_lock.lock();
    
    ImgMatrixT p_matrix;
    if( ! p_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
    {
      TEAGN_LOGERR( "Error reseting matrix" ) 
      glb_mem_lock.unLock();
      return false;
    }
      
    glb_mem_lock.unLock();       
    
    /* iterating over the optical flow displacement */
    
    int curr_line_offset = 0;
    int curr_col_offset = 0;
    unsigned int curr_line = 0;
    unsigned int curr_col = 0;
    unsigned int curr_line_bound = 0;
    unsigned int curr_col_bound = 0;    
    
    for( curr_line_offset = -1 ; curr_line_offset < 2 ; 
      ++curr_line_offset ) { 
      
      for( curr_col_offset = -1 ; curr_col_offset < 2 ; 
        ++curr_col_offset ) { 
        
        /* fill p matrix with ones */
      
        for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
          for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
            p_matrix( curr_line, curr_col ) = 1.0;
          }
        }
        
        /* updating p matrix */
        
        curr_line_bound = nlines - 1;
        curr_col_bound = ncols - 1;
      
        for( curr_line = 1 ; curr_line < curr_line_bound ; ++curr_line ) {
          for( curr_col = 1 ; curr_col < curr_col_bound ; ++curr_col ) {
            p_matrix( curr_line, curr_col ) = 
              ( gx_matrix( curr_line, curr_col ) * gx_matrix( curr_line + 
                curr_line_offset, curr_col + curr_col_offset ) ) +
              ( gy_matrix( curr_line, curr_col ) * gy_matrix( curr_line +
                curr_line_offset, curr_col + curr_col_offset ) );
              
          }
        }
        
        for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
          for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
            if( p_matrix( curr_line, curr_col ) < 0.0 ) {
              p_matrix( curr_line, curr_col ) = 0.0;
            }
          }
        }   
        
        /* Generating gxx, gxy and gyy */
        
        curr_line_bound = nlines - 1;
        curr_col_bound = ncols - 1;
      
        for( curr_line = 1 ; curr_line < curr_line_bound ; ++curr_line ) {
          for( curr_col = 1 ; curr_col < curr_col_bound ; ++curr_col ) {
            gxx_matrix( curr_line, curr_col ) = 
              gxx_matrix( curr_line, curr_col ) +
              ( p_matrix( curr_line, curr_col ) * gx_matrix( curr_line + 
              curr_line_offset, curr_col + curr_col_offset ) *
              gx_matrix( curr_line + curr_line_offset, curr_col + 
              curr_col_offset ) ) ;
              
            gxy_matrix( curr_line, curr_col ) = 
              gxy_matrix( curr_line, curr_col ) +
              ( p_matrix( curr_line, curr_col ) * gx_matrix( curr_line + 
              curr_line_offset, curr_col + curr_col_offset ) *
              gy_matrix( curr_line + curr_line_offset, curr_col + 
              curr_col_offset ) ) ;              
              
            gyy_matrix( curr_line, curr_col ) = 
              gyy_matrix( curr_line, curr_col ) +
              ( p_matrix( curr_line, curr_col ) * gy_matrix( curr_line + 
              curr_line_offset, curr_col + curr_col_offset ) *
              gy_matrix( curr_line + curr_line_offset, curr_col + 
              curr_col_offset ) ) ;              
          }
          
          TEAGN_FALSE_OR_RETURN( progress.Increment(),
            "Canceled by the user" );
        }        
      }
    }
  }    
  
  /* Creating the ls surface matrix */
  {
    glb_mem_lock.lock();
    
    if( ! ls_surface_matrix.Reset( nlines, ncols, ImgMatrixT::AutoMemPol ) )
    {
      TEAGN_LOGERR( "Error reseting matrix" )
      glb_mem_lock.unLock();
      return false;
    }
      
    glb_mem_lock.unLock();
      
    unsigned int curr_line = 0;
    unsigned int curr_col = 0;          
    double div_value = 0;
  
    for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
      for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
        div_value = sqrt( ( gxx_matrix( curr_line, curr_col ) *
          gxx_matrix( curr_line, curr_col ) ) + ( gyy_matrix( curr_line,
          curr_col ) * gyy_matrix( curr_line, curr_col ) ) +
          ( 2.0 * gxy_matrix( curr_line, curr_col ) * 
          gxy_matrix( curr_line, curr_col ) ) )
          *
          sqrt( gxx_matrix( curr_line, curr_col ) +
            gyy_matrix( curr_line, curr_col ) );
        
        if( div_value == 0.0 ) {
          ls_surface_matrix( curr_line, curr_col ) = 
            ( -1.0 ) * DBL_MAX;
        } else {
          ls_surface_matrix( curr_line, curr_col ) =
            ABS( ( gxx_matrix( curr_line, curr_col ) * gyy_matrix( 
            curr_line, curr_col ) ) - ( gxy_matrix( curr_line,
            curr_col ) * gxy_matrix( curr_line, curr_col ) ) ) / 
            div_value;
        }
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }  
  }
  
  return true;
}


void TePDIOFMatching::matrix2Tiff( 
  const ImgMatrixT& input_matrix,
  const std::string& out_file_name,
  const std::vector< TeCoord2D >& maxima_points )
{   
  TEAGN_TRUE_OR_THROW( ( ! out_file_name.empty() ), "Invalid file name" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.GetLines() > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( input_matrix.GetColumns() > 0 ), 
    "Invalid matrix cols" )
    
  /* Finding min and max */
  const double dbl_max = DBL_MAX;
  const double dbl_min = (-1.0) * dbl_max;
  double mtx_min = dbl_max;
  double mtx_max = dbl_min;
  double mtx_diff = 1.0;
  
  {
    for( unsigned int line = 0 ; 
      line < input_matrix.GetLines() ; ++line ) 
    {
      for( unsigned int col = 0 ; 
        col < input_matrix.GetColumns() ; 
        ++col ) 
      {
        if( ( input_matrix( line, col ) > dbl_min ) &&
          ( input_matrix( line, col ) < dbl_max ) )
        {
          if( input_matrix( line, col ) < mtx_min )
          {
            mtx_min = input_matrix( line, col );
          } 
          
          if( input_matrix( line, col ) > mtx_max )
          {
            mtx_max = input_matrix( line, col );
          }
        }
      }  
    }
    
    if( mtx_max != mtx_min )
    {
      mtx_diff = mtx_max - mtx_min;
    } 
  }
    
  TeRasterParams params;
  params.setNLinesNColumns( input_matrix.GetLines(),
    input_matrix.GetColumns() );
  params.nBands( 1 );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  params.nBands( 1 );
  params.decoderIdentifier_ = "TIF";
  params.mode_ = 'c';
  params.fileName_ = out_file_name;
  
  TeRaster out_raster( params );
  TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
  
  for( unsigned int line = 0 ; 
    line < input_matrix.GetLines() ; ++line ) {
    for( unsigned int col = 0 ; 
      col < input_matrix.GetColumns() ; 
      ++col ) {

      TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
        ( ( input_matrix( line, col ) - mtx_min ) / mtx_diff ) * 255.0, 0 ),
        "Error writing raster" )
    }  
  }
  
  /* Draw maxima points */
  
  std::vector< TeCoord2D >::const_iterator maximas_it = 
    maxima_points.begin();
  std::vector< TeCoord2D >::const_iterator maximas_it_end = 
    maxima_points.end();
    
  while( maximas_it != maximas_it_end ) {
    int x = TeRound( maximas_it->x() );
    TEAGN_TRUE_OR_THROW( ( x < (int)input_matrix.GetColumns() ),
      "Invalid maxima column" )
    TEAGN_TRUE_OR_THROW( ( x >= 0 ),
      "Invalid maxima column" )      
    
    int y = TeRound( maximas_it->y() );
    TEAGN_TRUE_OR_THROW( ( y < (int)input_matrix.GetLines() ),
      "Invalid maxima line" )
    TEAGN_TRUE_OR_THROW( ( y >= 0 ),
      "Invalid maxima line" )      
    
    TEAGN_TRUE_OR_THROW( out_raster.setElement( x, y, 
      255.0, 0 ),
      "Error writing raster" )
      
    ++maximas_it;
  }  
}


void TePDIOFMatching::raster2Tiff( 
  const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
  unsigned int raster_channel,
  const std::string& out_file_name,
  const std::vector< TeCoord2D >& maxima_points,
  const TeSharedPtr< TeCoordPairVect >& out_tie_points_ptr,
  unsigned int tie_points_space )
{   
  TEAGN_TRUE_OR_THROW( ( ! out_file_name.empty() ), 
    "Invalid file name" )
  TEAGN_TRUE_OR_THROW( ( input_raster_ptr->params().nlines_ > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_THROW( ( input_raster_ptr->params().ncols_ > 0 ), 
    "Invalid matrix cols" )
    
  TeRasterParams params;
  params.setNLinesNColumns( input_raster_ptr->params().nlines_,
    input_raster_ptr->params().ncols_ );
  params.nBands( 1 );
  params.setDataType( TeUNSIGNEDCHAR, -1 );
  params.nBands( 1 );
  params.decoderIdentifier_ = "TIF";
  params.mode_ = 'c';
  params.fileName_ = out_file_name;
  
  TeRaster out_raster( params );
  TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
  double value = 0;
  
  for( int line = 0 ; 
    line < input_raster_ptr->params().nlines_ ; ++line ) {
    for( int col = 0 ; 
      col < input_raster_ptr->params().ncols_ ; 
      ++col ) {
      
      input_raster_ptr->getElement( col, line, value, 
        raster_channel );
      

      TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
        value, 0 ),
        "Error writing raster" )
    }  
  }
  
  /* Draw maxima points */
  
  std::vector< TeCoord2D >::const_iterator maximas_it = 
    maxima_points.begin();
  std::vector< TeCoord2D >::const_iterator maximas_it_end = 
    maxima_points.end();
    
  while( maximas_it != maximas_it_end ) {
    int x = TeRound( maximas_it->x() );
    TEAGN_TRUE_OR_THROW( ( x < input_raster_ptr->params().ncols_ ),
      "Invalid maxima column" )
    TEAGN_TRUE_OR_THROW( ( x >= 0 ),
      "Invalid maxima column" )      
    
    int y = TeRound( maximas_it->y() );
    TEAGN_TRUE_OR_THROW( ( y < input_raster_ptr->params().nlines_ ),
      "Invalid maxima line" )
    TEAGN_TRUE_OR_THROW( ( y >= 0 ),
      "Invalid maxima line" )      
    
    TEAGN_TRUE_OR_THROW( out_raster.setElement( x, y, 
      255.0, 0 ),
      "Error writing raster" )
      
    ++maximas_it;
  }
  
  /* Draw tie-points */
  
  if( out_tie_points_ptr.isActive() ) {
    TeCoordPairVect::iterator it = out_tie_points_ptr->begin();
    TeCoordPairVect::iterator it_end = out_tie_points_ptr->end();
    
    while( it != it_end ) {
      int x = 0;
      int y = 0;
      
      if( tie_points_space == 0 ) {
         x = TeRound( it->pt1.x() );
         y = TeRound( it->pt1.y() );
      } else {
         x = TeRound( it->pt2.x() );
         y = TeRound( it->pt2.y() );
      }
      
      TEAGN_TRUE_OR_THROW( ( x < input_raster_ptr->params().ncols_ ),
        "Invalid maxima column" )
      TEAGN_TRUE_OR_THROW( ( x >= 0 ),
        "Invalid maxima column" )      
      TEAGN_TRUE_OR_THROW( ( y < input_raster_ptr->params().nlines_ ),
        "Invalid maxima line" )
      TEAGN_TRUE_OR_THROW( ( y >= 0 ),
        "Invalid maxima line" )      
      
      TEAGN_TRUE_OR_THROW( out_raster.setElement( x, y, 
        255.0, 0 ),
        "Error writing raster" )    
    
      ++it;
    }
  
  }
}


bool TePDIOFMatching::getMaximaPoints( 
  const ImgMatrixT& input_matrix,
  std::vector< TeCoord2D >& maxima_points,
  unsigned int max_points, unsigned int corr_window_width,
  bool progress_enabled, double maximas_sens )
{
  maxima_points.clear();
  
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetLines() > 0 ), 
    "Invalid matrix lines" )
  TEAGN_TRUE_OR_RETURN( ( input_matrix.GetColumns() > 0 ), 
    "Invalid matrix cols" )
  TEAGN_DEBUG_CONDITION( ( max_points > 0 ), 
    "Invalid number of maximum maxima points" )
  TEAGN_DEBUG_CONDITION( ( corr_window_width >= 3 ), 
    "Invalid correlation window width" )
  TEAGN_DEBUG_CONDITION( 
    ( maximas_sens >= 0.0 ) && ( maximas_sens <= 10.0 ), 
    "Invalid maximas_sens" )    
    
  /* The radius of a windows rotated by 90 degrees. */
  const unsigned int rotated_window_radius = (unsigned int)
    (
      ceil( 
        sqrt( 
          2 * 
          ( ( (double)corr_window_width ) * 
            ( (double)corr_window_width ) 
          )
        ) / 2.0
      )
    );
  const unsigned int rotated_window_diam = 1 + ( 2 *
    rotated_window_radius );
    
  /* Checking if the current rotated correlation window width 
     fits inside the current image */
     
  const unsigned int total_lines = input_matrix.GetLines();
  const unsigned int total_cols = input_matrix.GetColumns();

  if( total_lines <= rotated_window_diam ) {
    return true;
  }
  if( total_cols <= rotated_window_diam ) {
    return true;
  }    
  
  const unsigned int max_blks_per_line = total_cols - 
    rotated_window_diam;
  const unsigned int max_blks_per_col = total_lines - 
    rotated_window_diam;   
  
  /* Calc block dimentions */
  
  unsigned int blocks_per_line_col = 
    MIN( (unsigned int)floor( sqrt( (double)max_points ) ), 
      MIN( max_blks_per_line, max_blks_per_col ) );
  
  if( blocks_per_line_col == 0 )
  {
    return true;
  }
  
  unsigned int block_width = (unsigned int)
    ceil(
      ((double)( total_cols - rotated_window_diam ) ) 
      / 
      ((double) blocks_per_line_col )
    );
  unsigned int block_height = (unsigned int)
    ceil(
      ((double)( total_lines - rotated_window_diam ) ) 
      / 
      ((double) blocks_per_line_col )
    );
    
  TEAGN_DEBUG_CONDITION( ( block_width > 0 ), 
    "Invalid block width" )
  TEAGN_DEBUG_CONDITION( ( block_height > 0 ), 
    "Invalid block height" )
  
  /* Looking maximas */
  
  const double dbl_max = DBL_MAX;
  const double dbl_min = dbl_max * (-1.0);  
  
  std::vector< TeCoord2D > internal_maxima_points;
  internal_maxima_points.reserve( blocks_per_line_col * 
    blocks_per_line_col );
  
  std::vector< double > maxima_points_blkmean_diffs;
  maxima_points_blkmean_diffs.reserve( blocks_per_line_col * 
    blocks_per_line_col );
  double maxima_points_blkmean_diffs_max = dbl_min;
  double maxima_points_blkmean_diffs_min = dbl_max;
    
  unsigned int curr_block_start_line = 0;
  unsigned int curr_block_start_col = 0;
  unsigned int curr_block_line_bound = 0;
  unsigned int curr_block_col_bound = 0;
  unsigned int curr_line = 0;
  unsigned int curr_col = 0;
  
  unsigned int block_maxima_col = 0;
  unsigned int block_maxima_line = 0;  
  double block_maxima_val = 0.0;
  
  unsigned int curr_horiz_block = 0;
  unsigned int curr_vert_block = 0;
  const unsigned int cols_bound = total_cols - 
   rotated_window_radius;
  const unsigned int lines_bound = total_lines - 
   rotated_window_radius;
   
  double blk_mean = 0.0;
  double blk_mean_diff = 0.0;   
  
  TePDIPIManager progress( "Finding maxima points",
    blocks_per_line_col, progress_enabled );  
    
  for( curr_vert_block = 0 ; curr_vert_block < blocks_per_line_col ;
    ++curr_vert_block ) {
    
    curr_block_start_line = rotated_window_radius +
      ( curr_vert_block * block_height );
    curr_block_line_bound = MIN( curr_block_start_line + block_height,
      lines_bound );
    
    for( curr_horiz_block = 0 ; curr_horiz_block < blocks_per_line_col ;
      ++curr_horiz_block ) {
      
      curr_block_start_col = rotated_window_radius +
        ( curr_horiz_block * block_width );
      curr_block_col_bound = MIN( curr_block_start_col + block_width,
        cols_bound );  
      
      block_maxima_val = dbl_min;
      blk_mean = 0.0;
      blk_mean_diff = 0.0;
      
      for( curr_line = curr_block_start_line ;
        curr_line < curr_block_line_bound ; ++curr_line ) {
        for( curr_col = curr_block_start_col ;
          curr_col < curr_block_col_bound ; ++curr_col ) {
          
          const double& curr_value = input_matrix( curr_line, 
            curr_col );
            
          if( ( curr_value > dbl_min ) && ( curr_value < dbl_max ) )
          {
            blk_mean += curr_value;
            
            if( curr_value >= block_maxima_val ) {
             
              block_maxima_val = curr_value;
              block_maxima_col = curr_col;
              block_maxima_line = curr_line;
            }            
          }
        }
      }
      
      if( block_maxima_val != dbl_min )
      {
        blk_mean /= (double)( 
          ( curr_block_line_bound - curr_block_start_line ) *
          ( curr_block_col_bound - curr_block_start_col ) );
        
        blk_mean_diff = block_maxima_val - blk_mean;
          
        maxima_points_blkmean_diffs.push_back( blk_mean_diff );
          
        if( maxima_points_blkmean_diffs_max < blk_mean_diff )
        {
          maxima_points_blkmean_diffs_max = blk_mean_diff;
        }
        
        if( maxima_points_blkmean_diffs_min > blk_mean_diff )
        {
          maxima_points_blkmean_diffs_min = blk_mean_diff;
        }        
        
        internal_maxima_points.push_back( TeCoord2D( block_maxima_col, 
          block_maxima_line ) );
      }
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(),
      "Canceled by the user" );
  }
  
  /* Post-selecting maxima points by the difference to blocks mean */
  
  double mean_diff_lim = ( ( maxima_points_blkmean_diffs_max -
    maxima_points_blkmean_diffs_min ) * maximas_sens ) +
    maxima_points_blkmean_diffs_min;
    
  maxima_points.reserve( blocks_per_line_col * 
    blocks_per_line_col );
        
  for( unsigned int imp_idx = 0 ; imp_idx < internal_maxima_points.size() ;
    ++imp_idx )
  {
    if( maxima_points_blkmean_diffs[ imp_idx ] >= mean_diff_lim )
    {
      maxima_points.push_back( internal_maxima_points[ imp_idx ] );
    }
  }
  
  return true;
}


bool TePDIOFMatching::generateCorrWindows( 
  const ImgMatrixT& img_matrix,
  unsigned int corr_window_width,
  const std::vector< TeCoord2D >& img_maxima_points,
  ImgMatrixT& img_features_matrix,
  bool progress_enabled,
  TeMutex& glb_mem_lock )
{
  TEAGN_TRUE_OR_THROW( img_matrix.GetColumns() *
    img_matrix.GetLines() > 0,
    "Invalid ls matrix" )
  TEAGN_TRUE_OR_THROW( ( ( corr_window_width % 2 ) > 0 ),
    "Invalid corr_window_width" )    
  TEAGN_TRUE_OR_THROW( ( corr_window_width >= 3 ),
    "Invalid corr_window_width" )      
    
  if( img_maxima_points.size() == 0 ) {
    img_features_matrix.Reset();
  } else {
  /* Generating the image features */

    /* each line is a stacked version of one rotated correlation 
       window */
       
    glb_mem_lock.lock();
    
    if( ! img_features_matrix.Reset( img_maxima_points.size(),
      corr_window_width * corr_window_width,
      ImgMatrixT::RAMMemPol ) )
    {
      TEAGN_LOGERR( "Error allocating image features matrix" )
      glb_mem_lock.unLock();
      return false;
    }
      
    glb_mem_lock.unLock();
      
    /* variables related to the current window over the hole image */
    unsigned int curr_window_x_start = 0;
    unsigned int curr_window_y_start = 0;
    unsigned int curr_window_x_center = 0;
    unsigned int curr_window_y_center = 0;
    unsigned int curr_window_x_bound = 0;
    unsigned int curr_window_y_bound = 0;
    
    /*used on the rotation calcule */
    const unsigned int wind_radius = corr_window_width / 2;
    const double wind_radius_double = (double)wind_radius;
    const unsigned int img_features_matrix_cols = 
      img_features_matrix.GetColumns();
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
    
    TePDIPIManager progress( "Generating correlation windows",
      img_maxima_points.size(), progress_enabled );     
      
    for( unsigned int img_maximas_index = 0 ; 
      img_maximas_index < img_maxima_points.size() ; 
      ++img_maximas_index ) {
      
      /* Calculating the current window position */
     
      curr_window_x_center = 
        (unsigned int)img_maxima_points[ img_maximas_index ].x();
      curr_window_y_center = 
        (unsigned int)img_maxima_points[ img_maximas_index ].y();
      curr_window_x_start = curr_window_x_center - wind_radius;
      curr_window_y_start = curr_window_y_center - wind_radius;
      curr_window_x_bound = curr_window_x_start + 
        corr_window_width;
      curr_window_y_bound = curr_window_y_start + 
        corr_window_width;
        
      /* Estimating the intensity vector X direction */
      
      for( curr_offset = 0 ; curr_offset < wind_radius ;
        ++curr_offset ) {      

        for( curr_y = curr_window_y_start ; 
          curr_y < curr_window_y_bound ;
          ++curr_y ) {
        
          int_x_dir += 
            img_matrix( curr_y, curr_window_x_bound - 1 - 
              curr_offset ) - 
            img_matrix( curr_y, 
              curr_window_x_start + curr_offset );
        }
      }
      
      int_x_dir /= ( 2.0 * ( (double) wind_radius ) );
      
      /* Estimating the intensity vector y direction */
      
      for( curr_offset = 0 ; curr_offset < wind_radius ;
        ++curr_offset ) {      

        for( curr_x = curr_window_x_start ; 
          curr_x < curr_window_x_bound ;
          ++curr_x ) {
        
          int_y_dir += 
            img_matrix( curr_window_y_start + curr_offset,
              curr_x ) - 
            img_matrix( curr_window_y_bound - 1 -
              curr_offset, curr_x );
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
      
      
/* remove */
//rot_cos = cos( M_PI_4 );
//rot_sin = sin( M_PI_4 );
//if( ( curr_window_x_center = 366 ) &&
//  ( curr_window_y_center == 641 ) ) {
//  double angle = atan( rot_sin / rot_cos );
//  angle = angle + 1.0;
//}
//double angle = atan( rot_sin / rot_cos );
/* remove */      
      
      
      
      /* Generating the rotated window data and inserting it into 
         the img_features_matrix by setting the intensity vector
         to the direction (1,0) by a clockwise rotation
         using the inverse matrix 
      
         | u |    |cos   sin|   |X|
         | v | == |-sin  cos| x |Y|
      */
         
      for( curr_y = 0 ; curr_y < corr_window_width ; ++curr_y ) {
        for( curr_x = 0 ; curr_x < corr_window_width ; ++curr_x ) {
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
             centered location */ 
            
          rotated_curr_x += wind_radius_double;
          rotated_curr_y += wind_radius_double;
          
          /* copy the new rotated window to the output vector */
            
          rotated_curr_x_img = curr_window_x_start +
            TeRound( rotated_curr_x );
          rotated_curr_y_img = curr_window_y_start +
            TeRound( rotated_curr_y );                        
           
          TEAGN_DEBUG_CONDITION( ( rotated_curr_x_img > 0 ), "" )
          TEAGN_DEBUG_CONDITION( ( rotated_curr_x_img < 
            (int)img_matrix.GetColumns() ), "" )
          TEAGN_DEBUG_CONDITION( ( rotated_curr_y_img > 0 ), "" )
          TEAGN_DEBUG_CONDITION( ( rotated_curr_y_img < 
            (int)img_matrix.GetLines() ), "" )
            
          img_features_matrix( img_maximas_index, ( curr_y * 
            corr_window_width ) + curr_x ) =
            img_matrix( rotated_curr_y_img, rotated_curr_x_img );

        }
      }
      
      
/* remove - skipping normalizing */
//continue;
/* remove */      
      
      
      /* Normalizing the generated window by subtracting its mean
         and dividing by its standard deviation */
      
      curr_wind_mean = 0.0;
      
      for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
        ++curr_x ) {
        
        curr_wind_mean += img_features_matrix( img_maximas_index,
          curr_x );
      }
      
      curr_wind_mean /= ( (double)img_features_matrix_cols  );
      
      curr_wind_stddev = 0.0;  
      
      for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
        ++curr_x ) {
        
        curr_wind_stddev_aux = img_features_matrix( 
          img_maximas_index, curr_x ) - curr_wind_mean;
          
        curr_wind_stddev += ( curr_wind_stddev_aux *
          curr_wind_stddev_aux );
      }      
      
      curr_wind_stddev /= ( (double)img_features_matrix_cols  );
      curr_wind_stddev = sqrt( curr_wind_stddev );
      
      if( curr_wind_stddev == 0.0 ) {
        for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
          ++curr_x ) {
          
          double& curr_value = img_features_matrix( 
            img_maximas_index, curr_x );
          
          curr_value -= curr_wind_mean;
        } 
      } else {
        for( curr_x = 0 ; curr_x < img_features_matrix_cols ; 
          ++curr_x ) {
          
          double& curr_value = img_features_matrix( 
            img_maximas_index, curr_x );
          
          curr_value -= curr_wind_mean;
          curr_value /= curr_wind_stddev;
        }
      }
      
      /* Finished !! */
     
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
  }    
  
  return true;
}


void TePDIOFMatching::features2Tiff( 
  unsigned int corr_window_width,
  const std::vector< TeCoord2D >& img_maxima_points,
  ImgMatrixT& img_features_matrix,
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

  for( unsigned int curr_wind_index = 0 ; 
    curr_wind_index < img_features_matrix.GetLines() ;
    ++curr_wind_index ) {
    
    TeRasterParams params;
    params.setNLinesNColumns( corr_window_width,
      corr_window_width );
    params.nBands( 1 );
    params.setDataType( TeDOUBLE, -1 );
    params.nBands( 1 );
    params.decoderIdentifier_ = "TIF";
    params.mode_ = 'c';
    params.fileName_ = Te2String( img_maxima_points[ 
      curr_wind_index ].x(), 0 ) + "_" + Te2String( 
      img_maxima_points[ curr_wind_index ].y(), 0 ) + 
      "_" + filenameaddon + ".tif";
    
    TeRaster out_raster( params );
    TEAGN_TRUE_OR_THROW( out_raster.init(), "Error init raster" );
    
    for( unsigned int line = 0 ; line < corr_window_width ; ++line ) {
      for( unsigned int col = 0 ; col < corr_window_width ; ++col ) {

        TEAGN_TRUE_OR_THROW( out_raster.setElement( col, line, 
          img_features_matrix( curr_wind_index, ( line * 
          corr_window_width ) + col ), 0 ), "Error writing raster" )
      }  
    }    
  }
}


bool TePDIOFMatching::locateBestFeaturesMatching( 
  const TeThreadParameters& params )
{
  /* Retriving parameters */
  
  std::vector< unsigned int >* features2_indexes_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "features2_indexes_ptr", 
    features2_indexes_ptr ),
    "Missing thread parameter" )
  std::vector< unsigned int >& features2_indexes = *features2_indexes_ptr;    

  std::vector< unsigned int >* features1_indexes_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "features1_indexes_ptr", 
    features1_indexes_ptr ),
    "Missing thread parameter" )
  std::vector< unsigned int >& features1_indexes = *features1_indexes_ptr;
  
  TeMutex* globalMutexPtr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "global_mutex_ptr", 
    globalMutexPtr ),
    "Missing thread parameter" )
  TeMutex& globalMutex = *globalMutexPtr;  

  ImgMatrixT* img1_features_matrix_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img1_features_matrix_ptr", 
    img1_features_matrix_ptr ),
    "Missing thread parameter" )
  ImgMatrixT& img1_features_matrix =
    *img1_features_matrix_ptr;
    
  ImgMatrixT* img2_features_matrix_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "img2_features_matrix_ptr", 
    img2_features_matrix_ptr ),
    "Missing thread parameter" )
  ImgMatrixT& img2_features_matrix =
    *img2_features_matrix_ptr;
    
  bool progress_enabled = false;   
  TEAGN_TRUE_OR_THROW( params.retrive( "progress_enabled", 
    progress_enabled ), "Missing thread parameter" )     
    
  double corr_sens = 0;   
  TEAGN_TRUE_OR_THROW( params.retrive( "corr_sens", 
    corr_sens ), "Missing thread parameter" )  
    
  TEAGN_DEBUG_CONDITION( ( img2_features_matrix.GetColumns() ==
    img1_features_matrix.GetColumns() ),
     "Size columns mismatch" )        
    
  /* Init the output featers2_indexes vector once */
  
  globalMutex.lock();
  
  if( features2_indexes.size() == 0 )
  {
    for( unsigned int features2_indexes_index = 0 ; 
      features2_indexes_index < img1_features_matrix.GetLines() ;
      ++features2_indexes_index ) {
      
      /* fills with an matrix lines number indicating
         no processing done yet */
      features2_indexes.push_back( 
        img2_features_matrix.GetLines() );
    }
  }  
  
  /* Init the output featers2_indexes vector once */
  
  if( features1_indexes.size() == 0 )
  {
    for( unsigned int features1_indexes_index = 0 ; 
      features1_indexes_index < img2_features_matrix.GetLines() ;
      ++features1_indexes_index ) {
      
      /* fills with an matrix lines number indicating
         no processing done yet */
      features1_indexes.push_back( 
        img1_features_matrix.GetLines() );
    }
  }  
  
  globalMutex.unLock();  
    
  /* Iterating over img1_features_matrix */
  
  const unsigned int img1_features_matrix_lines = 
    img1_features_matrix.GetLines();
  const unsigned int img2_features_matrix_lines = 
    img2_features_matrix.GetLines();  
  
  TePDIPIManager progress( "Matching features", 
    img1_features_matrix_lines + img2_features_matrix_lines, 
    progress_enabled );
        
  const unsigned int cols_nmb = img2_features_matrix.GetColumns();
  const double max_allowed_diff = ((double)cols_nmb) / corr_sens;
            
  {
    unsigned int mat1_line = 0;
    unsigned int mat2_line = 0;
    double min_diff_value = 0;
    unsigned int min_diff_index = 0;
    unsigned int curr_col = 0;
    double curr_diff_value = 0;
    double curr_elem_diff = 0;
    double* img1FMtxLinePtr = 0;
    double* img2FMtxLinePtr = 0;
      
    for( mat1_line = 0 ; 
      mat1_line < img1_features_matrix_lines ; ++mat1_line ) {
      
      /* Looking for a non-processed feature */
      
      globalMutex.lock();
      
      if( features2_indexes[ mat1_line ] == img2_features_matrix_lines ) 
      {
        /* Tell to other threads that the current correlation
          is being calculated */
        
        features2_indexes[ mat1_line ] = img2_features_matrix_lines + 1;
        
        img1FMtxLinePtr = img1_features_matrix[ mat1_line ];
        
        globalMutex.unLock();
        
        /* Looking for the best correlation with matrix 2 */
        
        min_diff_value = DBL_MAX;
        
        for( mat2_line = 0 ; mat2_line < img2_features_matrix_lines ;
          ++mat2_line ) 
        {
          globalMutex.lock();
          
          img2FMtxLinePtr = img2_features_matrix[ mat2_line ]; 
           
          globalMutex.unLock();
           
          curr_diff_value = 0.0;
          
          for( curr_col = 0 ; curr_col < cols_nmb ; ++curr_col ) 
          {
            curr_elem_diff = img1FMtxLinePtr[ curr_col ] - 
              img2FMtxLinePtr[ curr_col ];
              
            curr_elem_diff *= curr_elem_diff;
              
            curr_diff_value += curr_elem_diff;
          }
          
          curr_diff_value = sqrt( curr_diff_value );
          
          if( curr_diff_value < min_diff_value ) {
            min_diff_value = curr_diff_value;
            min_diff_index = mat2_line;
          }
        }
        
        if( min_diff_value <= max_allowed_diff ) {
          globalMutex.lock();
          
          features2_indexes[ mat1_line ] = min_diff_index;
          
          globalMutex.unLock();
        }
      } else {
        globalMutex.unLock();
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
  }
  
  /* Iterating over img2_features_matrix */
  
  {
    unsigned int mat1_line = 0;
    unsigned int mat2_line = 0;
    double min_diff_value = 0;
    unsigned int min_diff_index = 0;
    unsigned int curr_col = 0;
    double curr_diff_value = 0;
    double curr_elem_diff = 0;
    double* img1FMtxLinePtr = 0;
    double* img2FMtxLinePtr = 0;    
      
    for( mat2_line = 0 ; 
      mat2_line < img2_features_matrix_lines ; ++mat2_line ) {
      
      /* Looking for a non-processed feature */
      
      globalMutex.lock();
      
      if( features1_indexes[ mat2_line ] == img1_features_matrix_lines ) 
      {
        /* Tell to other threads that the current correlation
          is being calculated */
        
        features1_indexes[ mat2_line ] = img1_features_matrix_lines + 1;
        
        img2FMtxLinePtr = img2_features_matrix[ mat2_line ];
        
        globalMutex.unLock();
        
        /* Looking for the best correlation with matrix 1 */
        
        min_diff_value = DBL_MAX;
        
        for( mat1_line = 0 ; mat1_line < img1_features_matrix_lines ;
          ++mat1_line ) 
        {
          globalMutex.lock();
            
          img1FMtxLinePtr = img1_features_matrix[ mat1_line ];  
            
          globalMutex.unLock();
            
          curr_diff_value = 0.0;
          
          for( curr_col = 0 ; curr_col < cols_nmb ; ++curr_col ) 
          {
            curr_elem_diff = img2FMtxLinePtr[ curr_col ] - 
              img1FMtxLinePtr[ curr_col ];
              
            curr_elem_diff *= curr_elem_diff;
            
            curr_diff_value += curr_elem_diff;
          }
          
          curr_diff_value = sqrt( curr_diff_value );
          
          if( curr_diff_value < min_diff_value ) {
            min_diff_value = curr_diff_value;
            min_diff_index = mat1_line;
          }
        }
        
        if( min_diff_value <= max_allowed_diff ) {
          globalMutex.lock();
          
          features1_indexes[ mat2_line ] = min_diff_index;
          
          globalMutex.unLock();
        }
      } else {
        globalMutex.unLock();
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
  }
  
  return true;  
}


bool TePDIOFMatching::checkMaximaPoints( 
  const std::vector< TeCoord2D >&maximas ) const
{
  for( unsigned int idx1 = 0 ; idx1 < maximas.size() ; ++idx1 )
  { 
    for( unsigned int idx2 = idx1 + 1 ; idx2 < maximas.size() ; ++idx2 )
    {
      TEAGN_TRUE_OR_RETURN( (
        ( maximas[ idx1 ].x() != maximas[ idx2 ].x() ) ||
        ( maximas[ idx1 ].y() != maximas[ idx2 ].y() ) ),
        "Invalid maxima points" +
        std::string( " index1=" ) + Te2String( idx1 ) +
        " [" + Te2String( maximas[ idx1 ].x(), 9 ) + "," +
        Te2String( maximas[ idx1 ].y(), 9 ) + "]" +
        std::string( " index2=" ) + Te2String( idx2 ) +
        " [" + Te2String( maximas[ idx2 ].x(), 9 ) + "," +
        Te2String( maximas[ idx2 ].y(), 9 ) + "]" )
    }
  }
  
  return true;
}


bool TePDIOFMatching::checkTPs( 
  const TeCoordPairVect& tpsvec ) const
{
  for( unsigned int idx1 = 0 ; idx1 < tpsvec.size() ; ++idx1 )
  { 
    for( unsigned int idx2 = idx1 + 1 ; idx2 < tpsvec.size() ; 
      ++idx2 )
    {
      TEAGN_TRUE_OR_RETURN( 
        (
          (
            ( tpsvec[ idx1 ].pt1.x() != tpsvec[ idx2 ].pt1.x() ) ||
            ( tpsvec[ idx1 ].pt1.y() != tpsvec[ idx2 ].pt1.y() ) 
          ) &&
          (
            ( tpsvec[ idx1 ].pt2.x() != tpsvec[ idx2 ].pt2.x() ) ||
            ( tpsvec[ idx1 ].pt2.y() != tpsvec[ idx2 ].pt2.y() ) 
          )
        ),
        "Invalid tie-point" +
        std::string( " index1=" ) + Te2String( idx1 ) +
        " [" + Te2String( tpsvec[ idx1 ].pt1.x(), 9 ) + "," +
        Te2String( tpsvec[ idx1 ].pt1.y(), 9 ) + " <-> " +
        Te2String( tpsvec[ idx1 ].pt2.x(), 9 ) + "," +
        Te2String( tpsvec[ idx1 ].pt2.y(), 9 ) + "]" +
        std::string( " index2=" ) + Te2String( idx2 ) +
        " [" + Te2String( tpsvec[ idx2 ].pt1.x(), 9 ) + "," +
        Te2String( tpsvec[ idx2 ].pt1.y(), 9 ) + " <-> " +
        Te2String( tpsvec[ idx2 ].pt2.x(), 9 ) + "," +
        Te2String( tpsvec[ idx2 ].pt2.y(), 9 ) + "]" );
    }
  }
  
  return true;
}


