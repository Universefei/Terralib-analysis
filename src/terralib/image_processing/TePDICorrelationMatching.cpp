#include "TePDICorrelationMatching.hpp"

#include "TePDIUtils.hpp"

#include "../kernel/TeThreadFunctor.h"
#include "../kernel/TeMutex.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeDefines.h"

#include <math.h>

void TePDICorrelationMatching::CorrThread::run()
{
  TEAGN_DEBUG_CONDITION( bigMatrix_.getMemPolicy() == ImgMatrixT::RAMMemPol,
    "invalid memory policy for use with threads" )
  TEAGN_DEBUG_CONDITION( smallMatrix_.getMemPolicy() == ImgMatrixT::RAMMemPol,
    "invalid memory policy for use with threads" )
    
  returnValue_ = false;
  
  const unsigned int small_mtx_lines = smallMatrix_.GetLines();
  const unsigned int small_mtx_cols = smallMatrix_.GetColumns();    
  const unsigned int big_mtx_lines_bound = bigMatrix_.GetLines() -
    small_mtx_lines;
  const unsigned int big_mtx_cols_bound = bigMatrix_.GetColumns() -
    small_mtx_cols;
  unsigned int big_mtx_line = 0;
  unsigned int big_mtx_col = 0;
  unsigned int small_mtx_line = 0;
  unsigned int small_mtx_col = 0; 
  double sbsum = 0;
  double ssum = 0;
  double bsum = 0;
  double spow2sum = 0;
  double bpow2sum = 0;
  const double totalElemensNmb = (double)( small_mtx_lines *
    small_mtx_cols );  
  double elemsnmb = 0; 
  double* big_matrix_line_ptr = 0;
  double* small_matrix_line_ptr = 0;  
  double auxVar1 = 0; 
  bool currLineBestCorValueFound = false;
  double curr_cor_value = 0;
  double currLineBestCorValue = 0;
  unsigned int currLineBestCorCol = 0;  
  
  for( big_mtx_line = 0 ; big_mtx_line < big_mtx_lines_bound ; 
    ++big_mtx_line )
  {
    // Can this line be processed ??
    
    mutex_.lock();
    
    if( big_mtx_line == nextLineToProcess_ )
    {
      ++nextLineToProcess_;
      mutex_.unLock();
      
      currLineBestCorValueFound = false;
      currLineBestCorValue = 0.0;
      currLineBestCorCol = 0;
    
      for( big_mtx_col = 0 ; big_mtx_col < big_mtx_cols_bound ; 
        ++big_mtx_col )
      {
        sbsum = 0;
        ssum = 0;
        bsum = 0;
        spow2sum = 0;
        bpow2sum = 0;
        elemsnmb = 0;
        
        for( small_mtx_line = 0 ; small_mtx_line < small_mtx_lines ;
          ++small_mtx_line )
        {
          big_matrix_line_ptr = bigMatrix_[ big_mtx_line + 
            small_mtx_line ];
          small_matrix_line_ptr = smallMatrix_[ small_mtx_line ];
          
          for( small_mtx_col = 0 ; small_mtx_col < small_mtx_cols ;
            ++small_mtx_col )
          {
            const double& smelem = small_matrix_line_ptr[ 
              small_mtx_col ];
            const double& bmelem = big_matrix_line_ptr[
              big_mtx_col + small_mtx_col ];
              
            if( ( smelem != dummyValue_ ) && ( bmelem != dummyValue_ ) )
            {
              sbsum += ( smelem * bmelem );
              ssum += smelem;
              bsum += bmelem;
              spow2sum += ( smelem * smelem );
              bpow2sum += ( bmelem * bmelem );
              ++elemsnmb;
            }
          }
        }
        
        if( elemsnmb )
        {
          auxVar1 = sqrt( ( elemsnmb * spow2sum ) - ( ssum * ssum ) ) * 
            sqrt( ( elemsnmb * bpow2sum ) - ( bsum * bsum ) );
            
          if( auxVar1 == 0.0 )
          {
            curr_cor_value = 0.0;
          }
          else
          {
            curr_cor_value = ( ( elemsnmb * sbsum ) - ( ssum * bsum ) ) /
              auxVar1;
            curr_cor_value *= ( elemsnmb / totalElemensNmb );
          }
          
          currLineBestCorValueFound = true;
          
          // Comparing the current correlation with
          // the current line best one
            
          if( ABS( curr_cor_value ) > ABS( currLineBestCorValue ) )
          {
            currLineBestCorValue = curr_cor_value;
            currLineBestCorCol = big_mtx_col;
          }          
        }
      }
      
      // Comparing the current line best correlation with
      // the current global best correlation
      
      if( currLineBestCorValueFound )
      {
        mutex_.lock();
        
        if( ( ! bestCorValueFound_ ) || 
          ( ABS( currLineBestCorValue ) > ABS( bestCorValue_ ) ) )
        {
          bestCorValueFound_ = true;
          bestCorValue_ = currLineBestCorValue;
          bestCorLine_ = big_mtx_line;
          bestCorCol_ = currLineBestCorCol;
        }      
        
        if( progress_.Increment() )
        {
          TEAGN_LOGMSG( "Canceled by the user" );
          abortFlag_ = true;
          mutex_.unLock();  
          return;
        }
        else
        {
          mutex_.unLock();  
        }
      }      
    }
    else
    {
      if( progress_.Increment() )
      {
        TEAGN_LOGMSG( "Canceled by the user" );
        abortFlag_ = true;
        mutex_.unLock();  
        return;
      }
      else
      {
        mutex_.unLock();  
      }
    }
  }
  
  returnValue_ = true;
}

TePDICorrelationMatching::TePDICorrelationMatching()
{
}


TePDICorrelationMatching::~TePDICorrelationMatching()
{
}


bool TePDICorrelationMatching::RunImplementation()
{
  /* Globals */
     
  const unsigned int procsNumber = TeGetPhysProcNumber();
  TeMutex glb_mem_lock;

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
  if( params_.CheckParameter< int >( "enable_multi_thread" ) &&
    ( procsNumber > 1 ) ) 
  {
    enable_multi_thread = true;
  }
  
  bool enable_threaded_raster_read = false;
  if( enable_multi_thread && params_.CheckParameter< int >( 
    "enable_threaded_raster_read" ) ) 
  {
    enable_threaded_raster_read = true;
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
  
  double* best_cor_value_ptr = 0;
  if( params_.CheckParameter< double* >( "best_cor_value_ptr" ) ) 
  {
    params_.GetParameter( "best_cor_value_ptr", best_cor_value_ptr );
  }
     
  double dummy_value = 0;
  if( params_.CheckParameter< double >( "dummy_value" ) ) 
  {
    params_.GetParameter( "dummy_value", dummy_value );
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
        return false;
      }
    } 
    else
    {
      if( ( resc_box2_ncols > resc_box1_ncols ) ||
        ( resc_box2_nlines > resc_box1_nlines ) )
      {
        return false;
      }    
    }
  }  
  
  /* Loading images */
  
  ImgMatrixT img1_matrix;
  ImgMatrixT img2_matrix;
  if( enable_multi_thread )
  {
    img1_matrix.Reset( ImgMatrixT::RAMMemPol );
    img2_matrix.Reset( ImgMatrixT::RAMMemPol );
  }
  else
  {
    img1_matrix.Reset( 0, 0, ImgMatrixT::AutoMemPol, 
      img1_matrix.getMaxTmpFileSize(), 25 );
    img2_matrix.Reset( 0, 0, ImgMatrixT::AutoMemPol, 
      img2_matrix.getMaxTmpFileSize(), 25 );      
  }
 
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
    tparams1.store( "dummy_value", dummy_value );
    
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
    tparams2.store( "dummy_value", dummy_value );
  
    if( enable_threaded_raster_read && ( procsNumber > 1 ) )
    {
      TeThreadFunctor::pointer aux_thread_ptr( new TeThreadFunctor );
      aux_thread_ptr->setStartFunctPtr( loadImage );
      aux_thread_ptr->setParameters( tparams1 );
      aux_thread_ptr->start();
   
      if( ! loadImage( tparams2 ) ) {
        aux_thread_ptr->waitToFinish();
        
        TEAGN_LOG_AND_RETURN( "Unable to load image 2" )
      } 
      else 
      {
        aux_thread_ptr->waitToFinish();
        
        TEAGN_TRUE_OR_RETURN( aux_thread_ptr->getReturnValue(),
          "Unable to load image 1" )
      }
    } 
    else 
    {
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

  /* Switching the big and small matrixes */
  
  ImgMatrixT* big_matrix_ptr = 0;
  ImgMatrixT* small_matrix_ptr = 0;
  bool mtx1IsTheSmallOne = true;
  
  {    
    const unsigned int m1size = img1_matrix.GetColumns() * 
      img1_matrix.GetLines();
    const unsigned int m2size = img2_matrix.GetColumns() * 
      img2_matrix.GetLines();      
      
    if( m1size < m2size )
    {
      mtx1IsTheSmallOne = true;
      big_matrix_ptr = &img2_matrix;
      small_matrix_ptr = &img1_matrix;
    } 
    else
    {
      mtx1IsTheSmallOne = false;
      big_matrix_ptr = &img1_matrix;
      small_matrix_ptr = &img2_matrix;
    }
  }
    
  /* Correlating areas */
  
  volatile bool best_cor_value_found = false;
  volatile double best_cor_value = 0.00;
  volatile unsigned int best_cor_bm_line = 0; /* UL point over big matrix */
  volatile unsigned int best_cor_bm_col = 0; /* UL point over big matrix */    
  
  if( enable_multi_thread )
  {
    std::vector< TeSharedPtr< CorrThread > > tVec;
    TeSharedPtr< CorrThread > tPtr;
    TePDIPIManager progress( "Looking for the best correlation", 
      procsNumber * ( big_matrix_ptr->GetLines() - small_matrix_ptr->GetLines() ), 
      progress_enabled_ );
    volatile bool abortFlag = false;
    volatile bool returnValue = true;
    volatile unsigned int nextLineToProcess = 0;    
    
    // Starting correlation threads
    
    unsigned int tVecIdx = 0 ;
    
    for( tVecIdx = 0 ; tVecIdx < procsNumber ; ++tVecIdx )
    {
      tPtr.reset( new CorrThread( progress, *big_matrix_ptr, *small_matrix_ptr,
        glb_mem_lock, nextLineToProcess, abortFlag, best_cor_value_found,
        best_cor_value, 
        best_cor_bm_line, best_cor_bm_col, dummy_value ) );
        
      tPtr->start();
        
      tVec.push_back( tPtr );
    }
    
    // Waiting each thread to finish
    
    for( tVecIdx = 0 ; tVecIdx < procsNumber ; ++tVecIdx )
    {
      tVec[ tVecIdx ]->waitToFinish();
      
      returnValue &= tVec[ tVecIdx ]->getReturnValue();
      
      if( ! returnValue )
      {
        abortFlag = true;
      }
    }
        
    TEAGN_TRUE_OR_RETURN( returnValue, "Correlation thread error" );
  }
  else
  {
    ImgMatrixT& big_matrix = *big_matrix_ptr;
    ImgMatrixT& small_matrix = *small_matrix_ptr;
      
    const unsigned int small_mtx_lines = small_matrix.GetLines();
    const unsigned int small_mtx_cols = small_matrix.GetColumns();    
    
    /* Iterating over each small matrix position over the bigger one */
    
    const unsigned int big_mtx_lines_bound = big_matrix.GetLines() -
      small_mtx_lines;
    const unsigned int big_mtx_cols_bound = big_matrix.GetColumns() -
      small_mtx_cols;
      
    unsigned int big_mtx_line = 0;
    unsigned int big_mtx_col = 0;
    unsigned int small_mtx_line = 0;
    unsigned int small_mtx_col = 0;

    double* big_matrix_line_ptr = 0;
    double* small_matrix_line_ptr = 0;
    double sbsum = 0;
    double ssum = 0;
    double bsum = 0;
    double spow2sum = 0;
    double bpow2sum = 0;
    const double totalElemensNmb = (double)( small_mtx_lines *
      small_mtx_cols );
    double elemsnmb = 0;
    double curr_cor_value = 0;
    double auxVar1 = 0;
        
    TePDIPIManager progress( "Looking for the best correlation", 
      big_mtx_lines_bound, progress_enabled_ );
        
    for( ; big_mtx_line < big_mtx_lines_bound ; ++big_mtx_line )
    {
      for( big_mtx_col = 0 ; big_mtx_col < big_mtx_cols_bound ; 
        ++big_mtx_col )
      {
        sbsum = 0;
        ssum = 0;
        bsum = 0;
        spow2sum = 0;
        bpow2sum = 0;
        elemsnmb = 0;
        
        for( small_mtx_line = 0 ; small_mtx_line < small_mtx_lines ;
          ++small_mtx_line )
        {
          big_matrix_line_ptr = big_matrix[ big_mtx_line + 
            small_mtx_line ];
          small_matrix_line_ptr = small_matrix[ small_mtx_line ];
          
          for( small_mtx_col = 0 ; small_mtx_col < small_mtx_cols ;
            ++small_mtx_col )
          {
            const double& smelem = small_matrix_line_ptr[ 
              small_mtx_col ];
            const double& bmelem = big_matrix_line_ptr[
              big_mtx_col + small_mtx_col ];
              
            if( ( smelem != dummy_value ) && ( bmelem != dummy_value ) )
            {
              sbsum += ( smelem * bmelem );
              ssum += smelem;
              bsum += bmelem;
              spow2sum += ( smelem * smelem );
              bpow2sum += ( bmelem * bmelem );
              ++elemsnmb;
            }
          }
        }
        
        if( elemsnmb > 0.0 )
        {
          auxVar1 = sqrt( ( elemsnmb * spow2sum ) - ( ssum * ssum ) ) * 
            sqrt( ( elemsnmb * bpow2sum ) - ( bsum * bsum ) );
            
          if( auxVar1 == 0.0 )
          {
            curr_cor_value = 0.0;
          }
          else
          {
            curr_cor_value = ( ( elemsnmb * sbsum ) - ( ssum * bsum ) ) /
              auxVar1;
            curr_cor_value *= ( elemsnmb / totalElemensNmb );  
          }
          
          best_cor_value_found = true;
          
          if( ABS( curr_cor_value ) > ABS( best_cor_value ) )
          {
            best_cor_value = curr_cor_value;
            best_cor_bm_line = big_mtx_line;
            best_cor_bm_col = big_mtx_col;
          }          
        }
      }
      
      TEAGN_FALSE_OR_RETURN( progress.Increment(),
        "Canceled by the user" );
    }
  }
  
  // Did we found the best correlation value ??
  
  if( ! best_cor_value_found ) return false;
  
  /* Copy the best correlation value to user output var */
  
  if( best_cor_value_ptr ) 
  {
    (*best_cor_value_ptr) = best_cor_value;
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
  
  if( mtx1IsTheSmallOne )
  {
    ulpoint_img1.setXY( 0 , 0 );
    llpoint_img1.setXY( 0, img1_matrix.GetLines() - 1 );
    urpoint_img1.setXY( img1_matrix.GetColumns() - 1 , 0 );
    lrpoint_img1.setXY( img1_matrix.GetColumns() - 1,
      img1_matrix.GetLines() - 1 );
      
    ulpoint_img2.setXY( best_cor_bm_col, best_cor_bm_line );
    llpoint_img2.setXY( 
      best_cor_bm_col,
      best_cor_bm_line + img1_matrix.GetLines() - 1 );
    urpoint_img2.setXY( 
      best_cor_bm_col + img1_matrix.GetColumns() - 1,
      best_cor_bm_line );
    lrpoint_img2.setXY( 
      best_cor_bm_col + img1_matrix.GetColumns() - 1,
      best_cor_bm_line + img1_matrix.GetLines() - 1 );
  } 
  else
  {
    ulpoint_img2.setXY( 0 , 0 );
    llpoint_img2.setXY( 0, img2_matrix.GetLines() - 1 );
    urpoint_img2.setXY( img2_matrix.GetColumns() - 1 , 0 );
    lrpoint_img2.setXY( img2_matrix.GetColumns() - 1,
      img2_matrix.GetLines() - 1 );
      
    ulpoint_img1.setXY( best_cor_bm_col, best_cor_bm_line );
    llpoint_img1.setXY( 
      best_cor_bm_col,
      best_cor_bm_line + img2_matrix.GetLines() - 1 );
    urpoint_img1.setXY( 
      best_cor_bm_col + img2_matrix.GetColumns() - 1,
      best_cor_bm_line );
    lrpoint_img1.setXY( 
      best_cor_bm_col + img2_matrix.GetColumns() - 1,
      best_cor_bm_line + img2_matrix.GetLines() - 1 );        
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
  
  out_tie_points_ptr->clear();
  
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
  
  return true;
}


bool TePDICorrelationMatching::CheckParameters( 
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
  
  return true;
}


void TePDICorrelationMatching::ResetState( const TePDIParameters& )
{
}


bool TePDICorrelationMatching::loadImage( const TeThreadParameters& params )
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
  
  double dummy_value = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "dummy_value", 
    dummy_value ), "Missing dummy_value parameter" )   
  
         
  /* Rescaling image */
  
  unsigned int nlines = (unsigned int) ceil( 
    ( (double)box_nlines ) * img_y_rescale_factor );
  unsigned int ncols = (unsigned int) ceil( 
    ( (double)box_ncols ) * img_x_rescale_factor );    
  
  glb_mem_lock.lock();
  
  if( ! img_matrix_ptr->Reset( nlines, ncols ) )
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
        input_channel ) ) 
      {
        img_matrix_ptr->operator()( curr_out_line, curr_out_col ) = value;
      } 
      else 
      {
        img_matrix_ptr->operator()( curr_out_line, curr_out_col ) = dummy_value;
      }
    }
    
    TEAGN_FALSE_OR_RETURN( progress.Increment(),
      "Canceled by the user" );
  }
        
  return true;
}


void TePDICorrelationMatching::matrix2Tiff( 
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


void TePDICorrelationMatching::raster2Tiff( 
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


bool TePDICorrelationMatching::checkTPs( 
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
            ( tpsvec[ idx1 ].pt1.y() != tpsvec[ idx2 ].pt1.y() ) || 
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


