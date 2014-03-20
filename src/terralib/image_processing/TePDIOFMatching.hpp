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

#ifndef TEPDIOFMATCHING_HPP
  #define TEPDIOFMATCHING_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"
  
  #include "../kernel/TeThreadParameters.h"

  /**
   * @brief  Optical flow based image area matching.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIMatchingAlgorithms
   *
   * @note The required parameters are:
   *
   * @param input_image1_ptr (TePDITypes::TePDIRasterPtrType) - 
   * The input image 1.
   * @param input_channel1 ( unsigned int ) - Band to process 
   * from input_image1.
   *
   * @param input_image2_ptr (TePDITypes::TePDIRasterPtrType) - 
   * The input image 2.
   * @param input_channel2 ( unsigned int ) - Band to process 
   * from input_image2.
   *
   * @param out_tie_points_ptr ( TeSharedPtr< TeCoordPairVect > ) - The 
   * output tie- points 
   * where TeCoordPair.pt1 are input_image1 matricial
   * indexes and TeCoordPair.pt2 are input_image2 matricial
   * indexes.
   *
   * @note The Optional parameters are:
   *
   * @param gt_params ( TeGTParams ) - The geometric transformation
   * parameters to be used (if not supplied, the dafult TeGTParams
   * will be used - affine transformation, max direct mapping
   * error = 2 pixels, RANSACRemotion outliers remotion ).
   *
   * @param out_gt_params_ptr ( TeGTParams::pointer ) - The output 
   * parameters of internally generated geometric transformation 
   * (only valid when geometrical filtering is enabled).
   *
   * @param input_box1 ( TeBox ) - Box (image matrix coords) to process 
   * from input_image1 ( the entire image will be used if no box
   * was supplied )
   *
   * @param input_box2 ( TeBox ) - Box (image matrix coords) to process 
   * from input_image2 ( the entire image will be used if no box
   * was supplied )   
   *
   * @param enable_multi_thread (int) - If present (any value) 
   * a multi-threaded processing will be used; Some TeDecoders do
   * not support multi-thread so use it with cation
   * (default: multi-thread disabled).
   *
   * @param pixel_x_relation (double) - The pixel resolution relation 
   * pixel_x_relation = img1_pixel_res_x / img2_pixel_res_x (default=1.0);
   *
   * @param pixel_y_relation (double) - The pixel resolution relation 
   * pixel_y_relation = img1_pixel_res_y / img2_pixel_res_y (default=1.0);   
   *
   * @param max_tie_points (unsigned int) - The maximum number
   * of generated tie-points (default=529);   
   *
   * @param skip_geom_filter (int) - If present (any value) 
   * will disable the geometric filtering
   * ( default: geometric filtering enabled ). 
   *
   * @param corr_sens (double) - The correlation sensibility
   * (default: 0.5 range: (0,3] ).
   *
   * @param corr_window_width (unsigned int) - The correlation 
   * window width used to correlate points between 
   * images (Must be an odd number, minimum 3, default: 21).  
   *
   * @param max_size_opt (unsigned int) - The maximum image box allowed
   * size (size = pixels number = lines * columns); when the images pixels number
   * exceed this value a downsample will be performed for optimization
   * (default value=0 wich means optimization disabled).
   *
   * @param maximas_sens (double) - The maximas (candidate tie-points)
   * detection sensibility
   * (default: 0 range: [0,1] ).
   */
  class PDI_DLL TePDIOFMatching : public TePDIAlgorithm {
    public :

      /**
       * @brief Default Constructor.
       *
       */
      TePDIOFMatching();

      /**
       * @brief Default Destructor
       */
      ~TePDIOFMatching();

      /**
       * @brief Checks if the supplied parameters fits the requirements of each
       * PDI algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( const TePDIParameters& parameters ) const;

    protected :
    
      /**
       * @typedef TePDIMatrix< double > ImgMatrixT
       * @brief A type definition for a image matrix.
       */    
      typedef TePDIMatrix< double > ImgMatrixT;
    
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial 
       * state.
       */
      void ResetState( const TePDIParameters& params );
      
      /**
       * @brief A thread entry point to load raster data into
       * a simple matrix.
       * @param params The parameters needed for the thread 
       * execution.
       * @return true if OK, false on errors.
       */
      static bool loadImage( const TeThreadParameters& params );
      
      /**
       * @brief A thread entry point to generate the image features
       * and the maxima points.
       * @param params The parameters needed for the thread execution.
       * @return true if OK, false on errors.
       * @note The points inside img_maxima_points_ptr
       * will have img_matrix_ptr reference.
       */
      static bool generateFeatures( 
        const TeThreadParameters& params );      
      
      /**
       * @brief Apply a gaussian smoothing over the input_matrix.
       * @param input_matrix Input matrix.
       * @param output_matrix Output matrix.
       * @param iterations Iterations number.
       * @param progress_enabled Progress interface enabled.
       * @param glb_mem_lock a Reference to the global memory
       * allocation mutex.
       * @return true if OK, false on errors.
       */      
      static bool gaussianSmoothing( 
        const ImgMatrixT& input_matrix,
        ImgMatrixT& output_matrix, 
        unsigned int iterations,
        bool progress_enabled,
        TeMutex& glb_mem_lock );
        
      /**
       * @brief Generates a least squares surface from the input matrix.
       * @param input_matrix Input matrix.
       * @param ls_surface_matrix Output matrix containing the LS 
       * surface.
       * @param progress_enabled Progress interface enabled.
       * @param glb_mem_lock a Reference to the global memory
       * allocation mutex.       
       * @return true if OK, false on errors.
       */      
      static bool generateLSSurface( 
        const ImgMatrixT& input_matrix,
        ImgMatrixT& ls_surface_matrix, 
        bool progress_enabled,
        TeMutex& glb_mem_lock );
        
      /**
       * @brief Save the output matrix to a geotiff file.
       * @param input_matrix Input matrix.
       * @param out_file_name Output file name.
       * @param maxima_points The maxima points to be
       * filled with 255
       */      
      static void matrix2Tiff( 
        const ImgMatrixT& input_matrix,
        const std::string& out_file_name,
        const std::vector< TeCoord2D >& maxima_points );
        
      /**
       * @brief Save the output matrix to a geotiff file.
       * @param input_raster Input raster.
       * @param raster_channel Raster channel.
       * @param out_file_name Output file name.
       * @param maxima_points The maxima points to be drawed.
       * @param out_tie_points_ptr The tie-points to be drawed.
       * @param tie_points_space The tie-points space 
       * (pt1 or pt2).
       */      
      static void raster2Tiff( 
        const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
        unsigned int raster_channel,
        const std::string& out_file_name,
        const std::vector< TeCoord2D >& maxima_points,
        const TeSharedPtr< TeCoordPairVect >& out_tie_points_ptr,
        unsigned int tie_points_space );        
        
      /**
       * @brief Get image local maxima points.
       * @param input_matrix Input matrix.
       * @param maxima_points The generated points.
       * @param max_points The maximum number of generated points.
       * @param corr_window_width The correlation window width
       * (points outside the correlation window based valid image bounds
       * will not be generated ).
       * @param progress_enabled Progress interface enabled.
       * @param maximas_sens Maximas sensibility to blocks mean
       * ( block mean diff >= max blocks mean diff * maximas_sens )
       * @return true if OK, false on errors.
       */      
      static bool getMaximaPoints( 
        const ImgMatrixT& input_matrix,
        std::vector< TeCoord2D >& maxima_points,
        unsigned int max_points, unsigned int corr_window_width,
        bool progress_enabled,
        double maximas_sens ); 
        
      /**
       * @brief Generate the rotated normalized corelation 
       * windows matrix.
       * @param img_matrix Input image matrix.
       * @param corr_window_width The correlation windows width.
       * @param img_maxima_points The image maxima points
       * (these are the correlation matrix centers).
       * @param img_features_matrix The output image
       * features matrix ( each line is a stacked version
       * of one rotated correlation window ).
       * @param progress_enabled Progress interface enabled.
       * @param glb_mem_lock a Reference to the global memory
       * allocation mutex.        
       * @return true if OK, false on errors.
       */          
      static bool generateCorrWindows( 
        const ImgMatrixT& img_matrix,
        unsigned int corr_window_width,
        const std::vector< TeCoord2D >& img_maxima_points,
        ImgMatrixT& img_features_matrix,
        bool progress_enabled,
        TeMutex& glb_mem_lock );
        
      /**
       * @brief Save the generated features to tif files.
       * @param corr_window_width The correlation windows width.
       * @param img_maxima_points The image maxima points
       * (these are the correlation matrix centers).
       * @param img_features_matrix The output image
       * features matrix ( each line is a stacked version
       * of one rotated correlation window ).
       * @filenameaddon A string to be appended to the
       * file name of each feature file.
       * @return true if OK, false on errors.
       */          
      static void features2Tiff( 
        unsigned int corr_window_width,
        const std::vector< TeCoord2D >& img_maxima_points,
        ImgMatrixT& img_features_matrix,
        const std::string& filenameaddon );   
        
      /**
       * @brief A thread entry point to locate the
       * best features matching index on matrix 2 from
       * each feature on matrix 1.
       * @param params The parameters needed for the 
       * thread execution.
       * @return true if OK, false on errors.
       * 
       * @note The internal used parameters:
       * @param features2_indexes_ptr (std::vector< unsigned int >*)
       * A pointer the the empty vector where the generated
       * features matrix 2 indexes will be stored.
       * @param features2_indexes_lock_ptr (TeMutex*) A thread
       * lock object to access features2_indexes_ptr.
       * @param img1_features_matrix_ptr (ImgMatrixT*) A pointer
       * to the image 1 features matrix.
       * @param img2_features_matrix_ptr (ImgMatrixT*) A pointer
       * to the image 2 features matrix.
       * @param progress_enabled (bool) Progress enabled flag.
       * @param corr_sens (double) Correlation sensitivity.
       */
      static bool locateBestFeaturesMatching( 
        const TeThreadParameters& params );      

      /**
       * @brief Check maxima points for repeated values.
       * @param maximas Maxima points vector.
       * @return true if OK, false on errors.
       */
      bool checkMaximaPoints( const std::vector< TeCoord2D >& maximas )
        const;
        
      /**
       * @brief Check tie-points indexes for repeated values.
       * @param maximas Tie-points vector.
       * @return true if OK, false on errors.
       */
      bool checkTPs( 
        const TeCoordPairVect& tpsvec ) const;         
  };
  
/** @example TePDIOFMatching_test.cpp
 *    Shows how to use this class.
 */    

#endif
