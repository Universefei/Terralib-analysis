#include "TePDIMIMatching.hpp"

#include "TePDIUtils.hpp"
#include "TePDIMatrix.hpp"
#include "../kernel/TeThreadFunctor.h"
#include "../kernel/TeMutex.h"
#include "../kernel/TeCoord2D.h"

#include <math.h>
#include <limits.h>


TePDIMIMatching::TePDIMIMatching()
{
}


TePDIMIMatching::~TePDIMIMatching()
{
}


bool TePDIMIMatching::RunImplementation()
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
  
  double pixel_x_relation = 1.0;
  if( params_.CheckParameter< double >( "pixel_x_relation" ) ) {
    params_.GetParameter( "pixel_x_relation", pixel_x_relation );
  }
  
  double pixel_y_relation = 1.0;
  if( params_.CheckParameter< double >( "pixel_y_relation" ) ) {
    params_.GetParameter( "pixel_y_relation", pixel_y_relation );
  }  
  
  unsigned int max_size_opt = 0;
  if( params_.CheckParameter< unsigned int >( "max_size_opt" ) ) {
    params_.GetParameter( "max_size_opt", max_size_opt );
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
  
  /* Checking if the small matrix fits inside the bigger one */
  
  {
    unsigned int resc_box1_nlines = (unsigned int) ceil( 
      ( (double)orig_box1_nlines ) * img1_y_rescale_factor );
    unsigned int resc_box1_ncols = (unsigned int) ceil( 
      ( (double)orig_box1_ncols ) * img1_x_rescale_factor );     
    unsigned int resc_box2_nlines = (unsigned int) ceil( 
      ( (double)orig_box2_nlines ) * img2_y_rescale_factor );
    unsigned int resc_box2_ncols = (unsigned int) ceil( 
      ( (double)orig_box2_ncols ) * img2_x_rescale_factor );   
        
    const unsigned int m1size = resc_box1_nlines * 
      resc_box1_ncols;
    const unsigned int m2size = resc_box2_nlines * 
      resc_box2_ncols;      
      
    if( m1size < m2size )
    {
      if( ( resc_box1_ncols > resc_box2_ncols ) ||
        ( resc_box1_nlines > resc_box2_nlines ) )
      {
        return true;
      }
    } 
    else
    {
      if( ( resc_box2_ncols > resc_box1_ncols ) ||
        ( resc_box2_nlines > resc_box1_nlines ) )
      {
        return true;
      }    
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
  
  /* Correlating areas */
  
  out_tie_points_ptr->clear();
  
  {
    /* Switching the big and small matrixes */
    
    const unsigned int m1size = img1_matrix.GetColumns() * 
      img1_matrix.GetLines();
    const unsigned int m2size = img2_matrix.GetColumns() * 
      img2_matrix.GetLines();      
    
    ImgMatrixT* big_matrix_ptr = 0;
    ImgMatrixT* small_matrix_ptr = 0;
      
    if( m1size < m2size )
    {
      big_matrix_ptr = &img2_matrix;
      small_matrix_ptr = &img1_matrix;
    } 
    else
    {
      big_matrix_ptr = &img1_matrix;
      small_matrix_ptr = &img2_matrix;
    }
    
    ImgMatrixT& big_matrix = *big_matrix_ptr;
    ImgMatrixT& small_matrix = *small_matrix_ptr;
    
    const unsigned int small_mtx_lines = small_matrix.GetLines();
    const unsigned int small_mtx_cols = small_matrix.GetColumns();
    const unsigned int big_mtx_lines = big_matrix.GetLines();
    const unsigned int big_mtx_cols = big_matrix.GetColumns();    
    const double elements_nmb = (double)( small_mtx_cols *
      small_mtx_lines );    
    
    /* Generating small matrix histogram */
    
    std::vector< unsigned int > small_mtx_histo;
    long int small_mtx_min = LONG_MAX;
    long int small_mtx_max = (-1) * small_mtx_min;
    unsigned int small_mtx_histo_size = 0;
    {
      unsigned int line = 0;
      unsigned int col = 0;
      double* small_matrix_line_ptr = 0;
      long int elem = 0;
      
      for( ; line < small_mtx_lines ; ++line )
      {
        small_matrix_line_ptr = small_matrix[ line ];
        
        for( col = 0 ; col < small_mtx_cols ; ++col )
        {
          elem = (long int)small_matrix_line_ptr[ col ];
          
          if( elem > small_mtx_max )
          {
            small_mtx_max = elem;
          }
          
          if( elem < small_mtx_min )
          {
            small_mtx_min = elem;
          }
          
        }
      }
      
      if( small_mtx_max >= small_mtx_min )
      {
        small_mtx_histo_size = (unsigned int)(
          small_mtx_max - small_mtx_min + 1 );
          
        small_mtx_histo.resize( small_mtx_histo_size );
        
        for( line = 0 ; line < small_mtx_lines ; ++line )
        {
          small_matrix_line_ptr = small_matrix[ line ];
          
          for( col = 0 ; col < small_mtx_cols ; ++col )
          {
            elem = (long int)small_matrix_line_ptr[ col ];
            elem -= small_mtx_min;
            
            small_mtx_histo[ elem ] += 1;
          }
        }
      }      
    }
    
    /* Calc the small matrix entropy */
    
    double small_mtx_entropy = 0.0;
    const double log2 = log( (double)2.0 );
    
    {
      double prob = 0.0;
      const unsigned int small_mtx_histo_size = (unsigned int)
        small_mtx_histo.size();

      for( unsigned int hidx = 0 ; hidx < small_mtx_histo_size ; ++hidx )
      {
        prob = ( (double)(small_mtx_histo[ hidx ] ) ) / elements_nmb;
        
        if( prob > 0.0 ) {
          small_mtx_entropy += ( prob * ( log( prob ) / log2 ) );
        }        
      }
      
      small_mtx_entropy *= (-1.0);
    }

    /* Pre-initiating big matrix histogram */
    
    std::vector< unsigned int > big_mtx_histo;
    long int big_mtx_min = LONG_MAX;
    long int big_mtx_max = (-1) * big_mtx_min;
    unsigned int big_mtx_histo_size = 0;
    {
      unsigned int line = 0;
      unsigned int col = 0;
      double* big_matrix_line_ptr = 0;
      long int elem = 0;
      
      for( ; line < big_mtx_lines ; ++line )
      {
        big_matrix_line_ptr = big_matrix[ line ];
        
        for( col = 0 ; col < big_mtx_cols ; ++col )
        {
          elem = (long int)big_matrix_line_ptr[ col ];
          
          if( elem > big_mtx_max )
          {
            big_mtx_max = elem;
          }
          
          if( elem < big_mtx_min )
          {
            big_mtx_min = elem;
          }
          
        }
      }
      
      if( big_mtx_max >= big_mtx_min )
      {
        big_mtx_histo_size = (unsigned int)( 
          big_mtx_max - big_mtx_min + 1 );
          
        big_mtx_histo.resize( big_mtx_histo_size );
      }      
    }

    /* Pre-initiating joint histogram matrix */
    
    TePDIMatrix< unsigned int > joint_histo_mtx;
    TEAGN_TRUE_OR_RETURN( joint_histo_mtx.Reset( small_mtx_histo_size,
      big_mtx_histo_size, TePDIMatrix< unsigned int >::AutoMemPol ), 
      "Error allocating joint histogram" )
    
    /* Iterating over each small matrix position over the bigger one */
    
    const unsigned int big_mtx_lines_bound = big_matrix.GetLines() -
      small_mtx_lines;
    const unsigned int big_mtx_cols_bound = big_matrix.GetColumns() -
      small_mtx_cols;
      
    unsigned int big_mtx_line = 0;
    unsigned  int big_mtx_col = 0;
    unsigned int small_mtx_line = 0;
    unsigned int small_mtx_col = 0;
    
    double* big_matrix_line_ptr = 0;
    double* small_matrix_line_ptr = 0;
    
    double prob = 0.0;
    double bmtx_crop_entropy = 0;
    double curr_jentropy = 0;
    double curr_mi = 0;
    long int big_mtx_ele = 0;
    long int small_mtx_ele = 0;
    
    double best_mi = DBL_MAX * ( -1.0 );
    unsigned int best_mi_lin = 0;
    unsigned int best_mi_col = 0;
    
    unsigned int big_mtx_histo_idx = 0;
    unsigned int joint_histo_mtx_line = 0;
    unsigned int joint_histo_mtx_col = 0;
    unsigned int* joint_histo_line_ptr = 0;
    
    TePDIPIManager progress( "Looking for the best matching", 
      big_mtx_lines_bound, progress_enabled_ );
        
    for( ; big_mtx_line < big_mtx_lines_bound ; ++big_mtx_line )
    {
      for( big_mtx_col = 0 ; big_mtx_col < big_mtx_cols_bound ; 
        ++big_mtx_col )
      {
        /* Cleaning big_mtx_histo */
        
        for( big_mtx_histo_idx = 0 ; big_mtx_histo_idx < 
          big_mtx_histo_size ; ++big_mtx_histo_idx )
        {
          big_mtx_histo[ big_mtx_histo_idx ] = 0;
        }
        
        /* Cleaning joint_histo_mtx */
        
        for( joint_histo_mtx_line = 0; joint_histo_mtx_line <
          small_mtx_histo_size ; ++joint_histo_mtx_line )
        {
          joint_histo_line_ptr = joint_histo_mtx[ 
            joint_histo_mtx_line ];
            
          for( joint_histo_mtx_col = 0; joint_histo_mtx_col <
            big_mtx_histo_size ; ++joint_histo_mtx_col )
          {
            joint_histo_line_ptr[ joint_histo_mtx_col ] = 0;
          }
        }
          
        /* Generating histograms for the current position */
        
        for( small_mtx_line = 0 ; small_mtx_line < small_mtx_lines ;
          ++small_mtx_line )
        {
          big_matrix_line_ptr = big_matrix[ big_mtx_line + 
            small_mtx_line ];
          small_matrix_line_ptr = small_matrix[ small_mtx_line ];
          
          for( small_mtx_col = 0 ; small_mtx_col < small_mtx_cols ;
            ++small_mtx_col )
          {
            big_mtx_ele = (long int)big_matrix_line_ptr[ big_mtx_col + 
              small_mtx_col ];
            small_mtx_ele = (long int)small_matrix_line_ptr[ 
              small_mtx_col ];              
              
            big_mtx_histo_idx = big_mtx_ele - big_mtx_min;
            big_mtx_histo[ big_mtx_histo_idx ] += 1;
            
            joint_histo_mtx_line = small_mtx_ele - small_mtx_min;
            joint_histo_mtx( joint_histo_mtx_line, big_mtx_histo_idx ) 
              += 1;
          }
        }

        /* Calc the entropy for the current position */
        
        bmtx_crop_entropy = 0.0;
        
        for( big_mtx_histo_idx = 0 ; big_mtx_histo_idx < 
          big_mtx_histo_size ; ++big_mtx_histo_idx )
        {
          prob = ( (double)( big_mtx_histo[ big_mtx_histo_idx ] ) ) / 
            elements_nmb;
          
          if( prob > 0.0 ) {
            bmtx_crop_entropy += ( prob * ( log( prob ) / log2 ) );
          }        
        }        
        
        bmtx_crop_entropy *= (-1.0);
        
        /* Calc the joint entropy for the current position */
        
        curr_jentropy = 0.0;
        
        for( joint_histo_mtx_line = 0; joint_histo_mtx_line <
          small_mtx_histo_size ; ++joint_histo_mtx_line )
        {
          joint_histo_line_ptr = joint_histo_mtx[ 
            joint_histo_mtx_line ];
            
          for( joint_histo_mtx_col = 0; joint_histo_mtx_col <
            big_mtx_histo_size ; ++joint_histo_mtx_col )
          {
            prob = ( (double)( joint_histo_line_ptr[ 
              joint_histo_mtx_col ] ) ) / elements_nmb;
            
            if( prob > 0.0 ) {
              curr_jentropy += ( prob * ( log( prob ) / log2 ) );
            }               
          }
        }        
        
        curr_jentropy *= (-1.0);
        
        /* Is this the best position ??? */
        
        curr_mi = small_mtx_entropy + bmtx_crop_entropy -
          curr_jentropy;
        
        if( curr_mi > best_mi )
        {
          best_mi = curr_mi;
          best_mi_col = big_mtx_col;
          best_mi_lin = big_mtx_line;
        } 
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
    
    /* Copy the best MI to user output var */
    
    double* best_mi_ptr = 0;
    if( params_.CheckParameter< double* >( "best_mi_ptr" ) ) {
      params_.GetParameter( "best_mi_ptr", best_mi_ptr );
      (*best_mi_ptr) = best_mi;
    }       
    
    /* Generating tie-points */
    
    TeCoord2D ulpoint_img1;
    TeCoord2D urpoint_img1;
    TeCoord2D llpoint_img1;
    TeCoord2D lrpoint_img1;
    
    TeCoord2D ulpoint_img2;
    TeCoord2D urpoint_img2;
    TeCoord2D llpoint_img2;
    TeCoord2D lrpoint_img2;
    
    if( m1size < m2size )
    {
      ulpoint_img1.setXY( 0 , 0 );
      llpoint_img1.setXY( 0, img1_matrix.GetLines() - 1 );
      urpoint_img1.setXY( img1_matrix.GetColumns() - 1 , 0 );
      lrpoint_img1.setXY( img1_matrix.GetColumns() - 1,
        img1_matrix.GetLines() - 1 );
        
      ulpoint_img2.setXY( best_mi_col, best_mi_lin );
      llpoint_img2.setXY( 
        best_mi_col,
        best_mi_lin + img1_matrix.GetLines() - 1 );
      urpoint_img2.setXY( 
        best_mi_col + img1_matrix.GetColumns() - 1,
        best_mi_lin );
      lrpoint_img2.setXY( 
        best_mi_col + img1_matrix.GetColumns() - 1,
        best_mi_lin + img1_matrix.GetLines() - 1 );
    } 
    else
    {
      ulpoint_img2.setXY( 0 , 0 );
      llpoint_img2.setXY( 0, img2_matrix.GetLines() - 1 );
      urpoint_img2.setXY( img2_matrix.GetColumns() - 1 , 0 );
      lrpoint_img2.setXY( img2_matrix.GetColumns() - 1,
        img2_matrix.GetLines() - 1 );
        
      ulpoint_img1.setXY( best_mi_col, best_mi_lin );
      llpoint_img1.setXY( 
        best_mi_col,
        best_mi_lin + img2_matrix.GetLines() - 1 );
      urpoint_img1.setXY( 
        best_mi_col + img2_matrix.GetColumns() - 1,
        best_mi_lin );
      lrpoint_img1.setXY( 
        best_mi_col + img2_matrix.GetColumns() - 1,
        best_mi_lin + img2_matrix.GetLines() - 1 );        
    }
    
        
    /* Bringing points into input_image1_ptr 
       reference */
       
    ulpoint_img1.setXY( 
      ( ulpoint_img1.x() / img1_x_rescale_factor ) +
      ( (double)orig_box1_x_off ),
      ( ulpoint_img1.y() / img1_y_rescale_factor ) +
      ( (double)orig_box1_y_off ) );
    urpoint_img1.setXY( 
      ( urpoint_img1.x() / img1_x_rescale_factor ) +
      ( (double)orig_box1_x_off ),
      ( urpoint_img1.y() / img1_y_rescale_factor ) +
      ( (double)orig_box1_y_off ) );
    llpoint_img1.setXY( 
      ( llpoint_img1.x() / img1_x_rescale_factor ) +
      ( (double)orig_box1_x_off ),
      ( llpoint_img1.y() / img1_y_rescale_factor ) +
      ( (double)orig_box1_y_off ) );
    lrpoint_img1.setXY( 
      ( lrpoint_img1.x() / img1_x_rescale_factor ) +
      ( (double)orig_box1_x_off ),
      ( lrpoint_img1.y() / img1_y_rescale_factor ) +
      ( (double)orig_box1_y_off ) );                  

    /* Bringing points into input_image2_ptr 
       reference */
   
    ulpoint_img2.setXY( 
      ( ulpoint_img2.x() / img2_x_rescale_factor ) +
      ( (double)orig_box2_x_off ),
      ( ulpoint_img2.y() / img2_y_rescale_factor ) +
      ( (double)orig_box2_y_off ) );
    urpoint_img2.setXY( 
      ( urpoint_img2.x() / img2_x_rescale_factor ) +
      ( (double)orig_box2_x_off ),
      ( urpoint_img2.y() / img2_y_rescale_factor ) +
      ( (double)orig_box2_y_off ) );
    llpoint_img2.setXY( 
      ( llpoint_img2.x() / img2_x_rescale_factor ) +
      ( (double)orig_box2_x_off ),
      ( llpoint_img2.y() / img2_y_rescale_factor ) +
      ( (double)orig_box2_y_off ) );
    lrpoint_img2.setXY( 
      ( lrpoint_img2.x() / img2_x_rescale_factor ) +
      ( (double)orig_box2_x_off ),
      ( lrpoint_img2.y() / img2_y_rescale_factor ) +
      ( (double)orig_box2_y_off ) );  
         
    /* Generating output tie-points */
    
    out_tie_points_ptr->push_back( TeCoordPair( ulpoint_img1,
      ulpoint_img2 ) );
    out_tie_points_ptr->push_back( TeCoordPair( urpoint_img1,
      urpoint_img2 ) );
    out_tie_points_ptr->push_back( TeCoordPair( llpoint_img1,
      llpoint_img2 ) );
    out_tie_points_ptr->push_back( TeCoordPair( lrpoint_img1,
      lrpoint_img2 ) );
      
    TEAGN_DEBUG_CONDITION( checkTPs( *out_tie_points_ptr ),
      "Invalid tie-points" )                     
  }
  
  return true;
}


bool TePDIMIMatching::CheckParameters( 
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
  
  /* Checking img1 data type */
    
  TEAGN_TRUE_OR_RETURN( 
    ( ( input_image1_ptr->params().dataType_[ input_channel1 ] != 
    TeFLOAT ) && ( input_image1_ptr->params().dataType_[ 
    input_channel1 ] != TeDOUBLE ) ),
    "Invalid parameter: invalid input_image1 data type" );    
  
  /* Checking img2 data type */
    
  TEAGN_TRUE_OR_RETURN( 
    ( ( input_image2_ptr->params().dataType_[ input_channel2 ] != 
    TeFLOAT ) && ( input_image2_ptr->params().dataType_[ 
    input_channel2 ] != TeDOUBLE ) ),
    "Invalid parameter: invalid input_image2 data type" );    
  
  return true;
}


void TePDIMIMatching::ResetState( const TePDIParameters& )
{
}


bool TePDIMIMatching::loadImage( const TeThreadParameters& params )
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


void TePDIMIMatching::matrix2Tiff( 
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


void TePDIMIMatching::raster2Tiff( 
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


bool TePDIMIMatching::checkTPs( 
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
     
        

