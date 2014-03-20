/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/

#ifndef TEPDIMMIOMATCHING_HPP
  #define TEPDIMMIOMATCHING_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMtxDoubleAdpt.hpp"

  #include "../kernel/TeThreadJobsManager.h"
  #include "../kernel/TeMutex.h"

  #include <list>

  /*!
      \class TePDIMMIOMatching
      \brief Modified Moravec Interest Operator based image area matching.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \ingroup PDIMatchingAlgorithms
     
      \note The required parameters are:
     
      \param input_image1_ptr (TePDITypes::TePDIRasterPtrType) - 
      The input image 1.
      \param input_channel1 ( unsigned int ) - Band to process 
      from input_image1.
     
      \param input_image2_ptr (TePDITypes::TePDIRasterPtrType) - 
      The input image 2.
      \param input_channel2 ( unsigned int ) - Band to process 
      from input_image2.
     
      \param out_tie_points_ptr ( TeSharedPtr< TeCoordPairVect > ) - The 
      output tie-points where TeCoordPair.pt1 are input_image1 matricial
      indexes and TeCoordPair.pt2 are input_image2 matricial
      indexes.
     
      \note The Optional parameters are:
      
      \param matching_method (TePDIMMIOMatching::FeatMatchingMethod) Features 
      matching method (default: TePDIMMIOMatching::NormCrossCorrMethod).    
     
      \param pixel_x_relation (double) - The pixel resolution relation 
      pixel_x_relation = img1_pixel_res_x / img2_pixel_res_x (default=1.0).
     
      \param pixel_y_relation (double) - The pixel resolution relation 
      pixel_y_relation = img1_pixel_res_y / img2_pixel_res_y (default=1.0).   
      
      \param gt_params ( TeGTParams ) - The geometric transformation
      parameters to be used (if not supplied, the dafult TeGTParams
      will be used - affine transformation, max direct mapping
      error = 2 pixels, RANSACRemotion outliers remotion ).
     
      \param out_gt_params_ptr ( TeGTParams::pointer ) - The output 
      parameters of internally generated geometric transformation 
      (only valid when geometrical filtering is enabled).
     
      \param input_box1 ( TeBox ) - Box (image matrix coords) to process 
      from input_image1 ( the entire image will be used if no box
      was supplied ).
     
      \param input_box2 ( TeBox ) - Box (image matrix coords) to process 
      from input_image2 ( the entire image will be used if no box
      was supplied ).
     
      \param enable_multi_thread (int) - If present (any value) 
      a multi-threaded processing will be used.
      (default: multi-thread disabled).
      
      \param enable_threaded_raster_read (int) - If present (any value) 
      a multi-threaded input rasters data reading will be performed
      if enable_multi_thread is also enabled; 
      Some TeDecoders do not support multi-thread so use it with caution
      (default: multi-threaded raster read disabled).      
     
      \param max_tie_points (unsigned int) - The maximum number
      of generated tie-points (default=1000).
     
      \param skip_geom_filter (int) - If present (any value) 
      will disable the geometric filtering
      ( default: geometric filtering enabled ). 
  
      \param enable_upsampled_filter (int) - If present (any value) 
      will enable the upsampled image Moravec repeatability filter
      ( default: upsampled filtering disabled - only the downsampled
      image filter will be applied ).  
      
      \param disable_reap_filter (int) - If present (any value) 
      will disable the image Moravec repeatability filters (upsample
      and downsample) ( default: filtering enabled - filter will be 
      applied to avoid creation of tie-points over noisy areas).        
     
      \param corr_window_width (unsigned int) - The correlation 
      window width used to correlate points between 
      images (Must be an odd number, minimum 13, default: 21).  
  
      \param moravec_window_width (unsigned int) - The Moravec
      window width used to locate canditate tie-points
      (Must be an odd number, minimum 11, default: 11 ).  
      
      \param out_tie_points_weights_ptr ( TeSharedPtr< std::vector< double > > ) - The 
      output tie- points weights vector (normalized between 0 and 1).
      
      \param mask_image1_ptr (TePDITypes::TePDIRasterPtrType) - Mask image 1
      pointer (tie-points falling inside mask image areas marked with
      zeroes will not be generated; this image must have the same number
      of lines/columns as input_image1).
      
      \param mask_image2_ptr (TePDITypes::TePDIRasterPtrType) - Mask image 2
      pointer (tie-points falling inside mask image areas marked with
      zeroes will not be generated; this image must have the same number
      of lines/columns as input_image1).     
       
      \param disable_subimage_search (int) - If present (any value) 
      the sub-image search will not enabled (the sub-image search
      improves the tie-point distribution homogeneity - 
      default:sub-image search enabled).
      
      \param out_maximas_image1_ptr (TePDITypes::TePDIRasterPtrType) - This
      parameters is for visualization and test purposes and if this
      parameter is present and the pointer is valid (pointing to a
      valid raster instance) the output maximas image (frim input
      image 1) will be generated an copied to the pointed raster (this raster
      will be initiated to 3 RGB bands where the maximas points are marked
      with value R255G0B0).
      
      \param out_maximas_image2_ptr - The same as the out_maximas_image1_ptr
      parameter but for the second input image.   
      
      \example TePDIMMIOMatching_test.cpp Shows how to use this class.
  */
  class PDI_DLL TePDIMMIOMatching : public TePDIAlgorithm {
    public :
      
      /*! Features matching methods. */
      enum FeatMatchingMethod
      {
        /*! Features normalized cross-correlation method */
        NormCrossCorrMethod
      };

      TePDIMMIOMatching();

      ~TePDIMMIOMatching();

      // Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;

    protected :
    
      /*! Mask matrix type.
       */
      typedef TePDIMatrix< unsigned char > MaskMatrixT;     
      
      /*! Maximas list node type */            
      struct MaximasListNodeT
      {
        /*! Maxima point X coord */
        unsigned int x_; 
        /*! Maxima point Y coord */
        unsigned int y_;
        /*! Maxima point X diretional variance (always positive value)*/
        double dirVariance_;
      };
      
      /*! Maximas list type 
       */
      typedef std::list< MaximasListNodeT > MaximasListT;
      
      /*! Ordered maximas map node type 
         \param unsigned int (first) Column - X.
         \param unsigned int (second) Line - Y.
       */
      typedef std::pair< unsigned int, unsigned int > MaximasMapNodeT;      
      
      /*! Ordered maximas map type.
       */
      typedef std::multimap< double, MaximasMapNodeT > MaximasMapT;      
      
      /*! The parameters required to run the method loadImage.
       */      
      class PDI_DLL LoadImageParams
      {
        public :
        
          /*! A pointer to the input image raster.*/
          TePDITypes::TePDIRasterPtrType input_image_ptr_;
          
          /*! A pointer to the input mask image raster or an inactive
          pointer when no mask image is used.*/
          TePDITypes::TePDIRasterPtrType mask_image_ptr_;
          
          unsigned int input_image_channel_;
          
          TePDIMtxDoubleAdptInt* img_matrix_ptr_;
          
          TePDIMtxDoubleAdptInt* img_matrix_ds_ptr_;
          
          TePDIMtxDoubleAdptInt* img_matrix_us_ptr_;
          
          /*! A pointer to a non-initialized output mask matrix.*/          
          MaskMatrixT* mask_matrix_ptr_;
          
          /*! Input box X axis offset (default:0)*/
          unsigned int in_box_x_off_;
          
          /*! Input box Y axis offset (default:0)*/
          unsigned int in_box_y_off_;
          
          /*! Input box number of lines (default:0)*/
          unsigned int in_box_nlines_;
          
          /*! Input box number of columns (default:0)*/
          unsigned int in_box_ncols_;
          
          /*! If true the progress interface will be used (default:false)*/
          bool progress_enabled_;
          
          /*! A pointer to the global mutex instance (default:0)*/
          TeMutex* glb_mem_lock_ptr_;
          
          /*! factor = rescaled_orignal_image / original_image */
          double origImgXRescFact_;
          
          /*! factor = rescaled_orignal_image / original_image */
          double origImgYRescFact_;
          
          /*! factor = rescaled_original_img / ds_image */
          double moravecDownSampleFactor_;
          
          /*! factor = rescaled_original_img / us_image */
          double moravecUpSampleFactor_;
          
          /*! Processing return value, true if OK (default:false) */
          volatile bool returnValue_;
          
          /*! If true, the upsampled image will be generated (default:false) */
          bool generateUpsampledImage_;
          
          /*! If true, the downsampled image will be generated (default:false) */
          bool generateDownsampledImage_;
          
          LoadImageParams()
          : input_image_channel_( 0 ), img_matrix_ptr_( 0 ),
            img_matrix_ds_ptr_( 0 ), img_matrix_us_ptr_( 0 ),
            mask_matrix_ptr_( 0 ), in_box_x_off_( 0 ),
            in_box_y_off_( 0 ), in_box_nlines_( 0 ),
            in_box_ncols_( 0 ), progress_enabled_( false ),
            glb_mem_lock_ptr_( 0 ),
            origImgXRescFact_( 1 ),
            origImgYRescFact_( 1 ), moravecDownSampleFactor_( 1 ),
            moravecUpSampleFactor_( 1 ), returnValue_( false ),
            generateUpsampledImage_( false ), generateDownsampledImage_( false )
          {};
          
          ~LoadImageParams() {};
      };
      
      class PDI_DLL ExtractLocalMaximasParams 
      {
        public :
        
          /*! Input image matrix */
          TePDIMtxDoubleAdptInt const* imgMatrixPtr_;
          
          /*! The output list where the found maximas index coords will be 
              appended */
          MaximasListT* outMaximasListPtr_;
          
          unsigned int moravecWindowSize_;
          
          unsigned int maximasNumber_;
          
          /*! Number of sub-images to seach (power of 2). */
          unsigned seachSubImagesNmb_; // Number of sub-images to seach.
          
          //Job return value.
          volatile bool returnValue_;
          
          /*! Is progress interface enabled ?? */
          bool progressEnabled_;
          
          /*! Global mutex pointer */
          TeMutex* glbMutexptr_;  
          
          /*! A pointer to a mask matrix or null if mask is not used.*/          
          MaskMatrixT const* maskMatrixPtr_;          
             
          ExtractLocalMaximasParams()
          : imgMatrixPtr_( 0 ), outMaximasListPtr_( 0 ),
            moravecWindowSize_( 0 ), maximasNumber_( 0 ),
            seachSubImagesNmb_( 0 ), returnValue_( false ),
            progressEnabled_( false ), glbMutexptr_( 0 ),
            maskMatrixPtr_( 0 )
          {};
          
          ~ExtractLocalMaximasParams() {};
      };
      
      struct GenerateCorrelationFeaturesParams
      {
        /*! Input image matrix */
        TePDIMtxDoubleAdptInt const* imgMatrixPtr_;
        
        /*! Input maximas list */
        MaximasListT const* inMaximasListPtr_;
        
        /*! Output features matrix pointer */
        TePDIMatrix< double >* featMtxPtr_;
        
        /*! Execution return value (true if OK, false on errors).*/
        volatile bool returnValue_;
        
        /* A mutex object pointer to sync large memory allocations */
        TeMutex* glbMemLockPtr_; // To sync large memory allocations
        
        /*! The generated correlation windows width.*/
        unsigned int corrWindowsWidth_;
        
        /*! Is progress interface enabled ?? */
        bool progressEnabled_;
        
        /*! If true, the generated windows will be normalized between -1 
         * and + 1. 
         */
        bool normalizeWindows_;
      };
      
      class CalcCCorrelationMtxParams
      {
        public :
          /*! Image 1 features matrix pointer (default:0). */
          TePDIMatrix< double > const* img1FeatMtxPtr_;
          
          /*! Image 2 features matrix pointer. */
          TePDIMatrix< double > const* img2FeatMtxPtr_;
  
          /*! A pointer to an output matching matrix pr�-initialized with 
          negative infinite values DBL_MAX * -1 (default:0)*/
          TePDIMatrix<double>* matchMatrixPtr_;
          
          /*! A pointer to pre-initialized double variable (DBL_MAX) to store
          the mininum value written to the output match matrix (default:0) */
          double* matchMatrixMinPtr_;
          
          /*! A pointer to pre-initialized double variable (-1.0 * DBL_MAX) 
          to store the maximum value written to the output match matrix (default:0) */
          double* matchMatrixMaxPtr_;          
    
          /*! Is progress interface enabled ?? (default:false)*/
          bool progressEnabled_;
          
          /*! Global mutex pointer (default:0)*/
          TeMutex* glbMutexptr_;
    
          /*! Execution return value (true if OK, false on errors - default:
          false).*/
          volatile bool returnValue_;
          
          CalcCCorrelationMtxParams()
          : img1FeatMtxPtr_( 0 ), img2FeatMtxPtr_( 0 ), matchMatrixPtr_( 0 ), 
          matchMatrixMinPtr_( 0 ),
          matchMatrixMaxPtr_( 0 ),
          progressEnabled_( false ), glbMutexptr_( 0 ), returnValue_( false )
          {};
          
          ~CalcCCorrelationMtxParams() {};
      };      
      
      /*! Moravec downsample factor = original_image / downsampled_image */
      const double moravecDownSampleFactor_;
      
      /*! Moravec upsample factor = original_image / upsampled_image */
      const double moravecUpSampleFactor_;
      
      /*! Moravec repeatability tolerance */
      const double moravecRepeatabilityMinTolerance_;      
      
      /*! Internal threaded jobs manager. */
      TeThreadJobsManager jobsMan_;
      
      /*! A global syncronization mutex. */
      TeMutex globalMutex_;
      
      // Overloaded
      bool RunImplementation();

      // Overloaded
      void ResetState( const TePDIParameters& params );
      
      /**
        \brief A job function load raster data into
        a simple matrix.
        \param paramsPtr The parameters needed for the thread 
        execution (a pointer to a LoadImageParams struct).
       */
      static void loadImage( void * paramsPtr ); 
      
      /*! Instantiate an image matrix capable of store pixel data
         of the supplied type.
        \param dataType Pixel type.
        \param matrixPtr A pointer for the new created matrix.
       */
      static void createImgMatrix( TeDataType dataType, 
        TeSharedPtr< TePDIMtxDoubleAdptInt >& matrixPtr );
     
      /*!
        \brief Extract local interest maxima points using a Modified
        Moravec Interest Operator aproach.
        \param paramsPtr The parameters needed for the thread 
        execution (a pointer to a ExtractLocalMaximasParams struct).
       */
      static void extractLocalMaximas( void * paramsPtr );
      
      /*!
        \brief Extract local interest maxima points from an sub-image
        sub-region using a Modified Moravec Interest Operator aproach.
        \param imgMatrixPtr Input image matrix pointer.
        \param maskMatrixPtr Input mask matrix pointer (or null if mask
        is not used).
        \param xStart Image sub-region initial X coordinate. 
        \param yStart Image sub-region initial y coordinate.
        \param width Image sub-region width.
        \param height Image sub-region height.
        \param bufferMatrix Pre-initiated buffer with at least width x height.
        \param moravecWindowSize Moravec window size
        \param maximasNumber The number of maximas to append to outMaximasMap.
        \param outMaximasList The found maximas will be appended to this container.
        \param progres Progress interface manager reference.
        \param return true if OK, false on errors.
       */      
      static bool extractLocalMaximas( 
        TePDIMtxDoubleAdptInt const* imgMatrixPtr,
        MaskMatrixT const* maskMatrixPtr,
        const unsigned int& xStart, const unsigned int& yStart, 
        const unsigned int& width, const unsigned int& height, 
        TePDIMatrix< double >& bufferMatrix,
        const unsigned int& moravecWindowSize, 
        const unsigned int& maximasNumber, 
        MaximasListT& outMaximasList, 
        TePDIPIManager& progress );         
      
      /*! 
        \brief Applies a Moravec repeatability constraint over an image
        maximas list.
        \param inputList Original image maximas list.
        \param constraintList Constraint list.
        \param constraintImageWidth Constraint image width.
        \param constraintImageHeight Constraint image height.
        \param moravecRepeatabilityMinTolerance Error tolerance (pixels)
        - constraint image reference.
        \param moravecReSampleFactor Moravec resample factor 
        (factor = inputList_scal / constraintList_scale).
        \param outputList Output maximas list from input list.
       */
      void moravecRepeatability( const MaximasListT& inputList,
        const MaximasListT& constraintList, 
        unsigned int constraintImageWidth,
        unsigned int constraintImageHeight,
        double moravecRepeatabilityMinTolerance, 
        double moravecReSampleFactor, 
        MaximasListT& outputList );      
      
      /*!
        \brief Generate correlation features.
        \param paramsPtr The parameters needed for the thread 
        execution (a pointer to a ExtractLocalMaximasParams struct).
       */
      static void generateCorrelationFeatures( void * paramsPtr );    
      
      /*! 
        \brief Features matching and tie-points generation.
        \param matching_method Features matching method.
        \param img1featMtx Image 1 features matrix.
        \param img1_maximas_list Image 1 maximas list.
        \param img2featMtx Image 2 features matrix.
        \param img2_maximas_list Image 2 maximas list.
        \param tiePointsVec Output tie-points vector.
        \param tiePointsWeights Output tie-points weights vector
        ( normalized between DBL_MIN and 1 ).
        \return true if OK, false on errors.
       */      
      bool matchFeatures( FeatMatchingMethod matching_method,
        const TePDIMatrix< double >& img1featMtx,
        const MaximasListT& img1_maximas_list,
        const TePDIMatrix< double >& img2featMtx,
        const MaximasListT& img2_maximas_list,
        TeCoordPairVect& tiePointsVec,
        std::vector< double >& tiePointsWeights );
      
      /*!
        \brief cross-correlation matrix calcule.
        \param paramsPtr The parameters needed for the thread 
        execution (a pointer to a CalcCCorrelationMtxParams struct).
       */
      static void calcCCorrelationMtx( void * paramsPtr );      
      
      /*!
        \brief Save the output matrix to a geotiff file.
        \param input_matrix Input matrix.
        \param out_file_name Output file name.
        \param maxima_points The maxima points to be
        filled with 255
       */      
      static void matrix2Tiff( 
        const TePDIMtxDoubleAdptInt& input_matrix,
        const std::string& out_file_name,
        const MaximasListT& maxima_points = MaximasListT() );
      
      /*!
        \brief Save the output matrix to a geotiff file.
        \param input_matrix Input matrix.
        \param out_file_name Output file name.
        \param tiepoints The tie oints to be
        filled with 255
        \param usePt1 Use pt1 from tiepoints, otherwise use pt1
       */      
      static void matrix2Tiff( 
        const TePDIMtxDoubleAdptInt& input_matrix,
        const std::string& out_file_name,
        const TeCoordPairVect& tiepoints,
        bool usePt1 );    
          
      /*!
        \brief Save the output matrix to a geotiff file.
        \param input_matrix Input matrix.
        \param out_file_name Output file name.
        \param maxima_points The maxima points to be
        filled with 255
       */      
      static void doublesMatrix2Tiff( 
        const TePDIMatrix< double >& input_matrix,
        const std::string& out_file_name,
        const MaximasListT& maxima_points = MaximasListT() );          
      
      /*!
        \brief Save the generated features to tif files.
        \param corr_window_width The correlation windows width.
        \param img_maxima_points The image maxima points
        (these are the correlation matrix centers).
        \param img_features_matrix The output image
        features matrix ( each line is a stacked version
        of one rotated correlation window ).
        \filenameaddon A string to be appended to the
        file name of each feature file.
        \return true if OK, false on errors.
       */          
      static void features2Tiff( 
        unsigned int corr_window_width,
        const MaximasListT& img_maxima_points,
        TePDIMatrix< double >& img_features_matrix,
        const std::string& filenameaddon );      

      /*!
        \brief Input matrix bicubic resample.
        \param inputMatrix Input matrix.
        \param outLines Output number of lines.
        \param outCols Output number of columns.
        \param progressEnabled Progress enabled flag.
        \param globalMutex Global mutex.
        \param outMinValue The allowed output min value.
        \param outMaxValue The allowed output max value.
        \param outputMatrix Output matrix.
        \return true if OK, false on errors.
       */      
      static bool bicubicResampleMatrix( const TePDIMtxDoubleAdptInt& inputMatrix,
        unsigned int outLines, unsigned int outCols, bool progressEnabled,
        TeMutex& globalMutex, double outMinValue, double outMaxValue,
        TePDIMtxDoubleAdptInt& outputMatrix );
      
      /*!
        \brief Generate the output maximas image ( 3 RGB bands where the 
        maximas points are marked with value R255G0B0).
        \param inputMatrix Input image data matrix.
        \param maximaPoints Input maximas point list.
        \param outRaster The ouptput raster.
        \return true if ok, false on errors.
       */      
      static bool generateMaximasRaster( 
        const TePDIMtxDoubleAdptInt& inputMatrix,
        const MaximasListT& maximaPoints,
        TeRaster& outRaster );       
  };


#endif
