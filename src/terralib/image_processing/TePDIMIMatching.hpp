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

#ifndef TEPDIMIMATCHING_HPP
  #define TEPDIMIMATCHING_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"
  
  #include "../kernel/TeThreadParameters.h"
  
  #include <map>

  /**
   * @brief Mutual Information image area matching ( only  
   * offset distortion is supported ).
   * @note Better used for searching a small image inside a big image.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @author Etore Marcari Junior <etore@dpi.inpe.br>
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
   * @param max_size_opt (unsigned int) - The maximum image box allowed
   * size (size = pixels number = lines * columns); when the images pixels number
   * exceed this value a downsample will be performed for optimization
   * (default value=0 wich means optimization disabled).
   *
   * @param best_mi_ptr (double*) - A pointer to an output double 
   * variable where to store the best found MI value.
   */
  class PDI_DLL TePDIMIMatching : public TePDIAlgorithm {
    public :

      /**
       * @brief Default Constructor.
       *
       */
      TePDIMIMatching();

      /**
       * @brief Default Destructor
       */
      ~TePDIMIMatching();

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
       * @brief Check tie-points indexes for repeated values.
       * @param maximas Tie-points vector.
       * @return true if OK, false on errors.
       */
      bool checkTPs( 
        const TeCoordPairVect& tpsvec ) const;         
  };
  
/** @example TePDIMIMatching_test.cpp
 *    Shows how to use this class.
 */    

#endif
