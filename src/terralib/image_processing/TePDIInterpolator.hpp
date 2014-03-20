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

#ifndef TEPDIINTERPOLATOR_HPP
  #define TEPDIINTERPOLATOR_HPP

  #include "TePDIDefines.hpp"
  #include "../kernel/TeSharedPtr.h"
  #include "../kernel/TeRaster.h"
  
  /**
   * @class TePDIInterpolator
   * @brief This is the class for pixel interpolation from an input raster.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIInterpolator {
    public :
      /** @typedef TeSharedPtr< TePDIInterpolator > pointer 
          Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIInterpolator > pointer;
      /** @typedef const TeSharedPtr< TePDIInterpolator > const_pointer 
          Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIInterpolator > const_pointer;
      
      /**
       * @enum Allowed interpolation methods.
       */      
      enum InterpMethod {
        /** Near neighborhood interpolation method. */
        NNMethod = 1,
        /** Bilinear interpolation method. */
        BilinearMethod = 2,
        /** Bicubic interpolation method. */
        BicubicMethod = 3      
      };

      /**
       * @brief Default Constructor.
       *
       */
      TePDIInterpolator();

      /**
       * @brief Default Destructor
       */
      ~TePDIInterpolator();
      
      /**
       * @brief Reset this interpolator instance.
       *
       * @param input_raster Input raster shared pointer.
       * @param method Interpolation method (For pallete based rasters only
       * the NNMethod can be used).
       * @param dummy_value A pixel dummy value to use when the input_raster
       * do not have dummy_values.
       * @return true if OK, false on errors.
       *
       */
      bool reset( const TeSharedPtr< TeRaster >& input_raster,
        InterpMethod method, double dummy_value );
        
      /**
       * @brief Interpolate a pixel value.
       *
       * @param line Line.
       * @param col Column.
       * @param band Band.
       * @param value Interpolated value.
       * @note The caller of this method must be aware that the returned 
       * interpolated value may be outside the original input raster valid 
       * values range.       
       *
       */
      inline void interpolate( const double& line, const double& col, 
        const unsigned int& band, double& value )
      {
        TEAGN_DEBUG_CONDITION( interp_funct_ptr_, 
          "Invalid interpolation function pointer" );
     
        ( this->*( interp_funct_ptr_ ) )( line, col, band, value );
      };        

    protected :
    
      /**
       * @brief Type definition for the a interpolation function pointer.
       *
       * @param line Line.
       * @param col Column.
       * @param band Band.
       * @param value Interpolated value.
       */      
      typedef void (TePDIInterpolator::*InterpFunctPtr)( const double& line, 
        const double& col, const unsigned int& band, double& value );

      /**
       * @brief Input raster naked pointer (default:0).
       */
      TeRasterParams* input_raster_params_nptr_;
      
      /**
       * @brief The current interpolation function pointer (default:0).
       */      
      InterpFunctPtr interp_funct_ptr_;
     
      /**
       * @brief The bicubic interpolation lines bound (not included,
       default:0).
       */      
      double bicubic_lines_bound_;      

      /**
       * @brief The bicubic interpolation columns bound (not included,
       default:0).
       */      
      double bicubic_columns_bound_;        
      
      /**
       * @brief The bicubic kernel parameters (default:-1.0).
       */      
      double bicubic_kernel_parameter_;        
            
      /**
       * @brief The last valid line index ( included, default:0 ).
       */         
      double last_valid_line_;

      /**
       * @brief The last valid column index ( included, default:0 ).
       */         
      double last_valid_col_;

      /**
       * @brief The current raster dummy use status (default:false).
       */      
      bool raster_uses_dummy_;
      
      /**
       * @brief The current raster bands number (default:0).
       */      
      unsigned int nbands_;      
      
      /**
       * @brief The current raster bands number (default:0).
       */      
      double dummy_value_;       
            
      /**
       * @brief Input raster shared pointer copy.
       */
      TeSharedPtr< TeRaster > input_raster_ptr_;      
            
      /**
       * @brief Input raster shared naked pointer (default:0).
       */
      TeRaster* input_raster_nptr_;  

      /**
       * @brief Input raster maximum allowed channels values
       * vector (default:0).
       */
      double* input_raster_max_chan_val_vec_;  

      /**
       * @brief Input raster minimum allowed channels values
       * vector (default:0).
       */
      double* input_raster_min_chan_val_vec_; 
      
      /** @brief Internal bilinear function used variable. */      
      double bilinear_line_min_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_line_max_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_col_min_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_col_max_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_diff_line_min_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_diff_line_max_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_diff_col_min_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_diff_col_max_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_dist1_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_dist2_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_dist3_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_dist4_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_weigh1_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_weigh2_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_weigh3_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_weigh4_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_elem1_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_elem2_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_elem3_;
      /** @brief Internal bilinear function used variable. */      
      double bilinear_elem4_;
      
      /** @brief Internal bicubic function used variable. */      
      unsigned int bicubic_grid_raster_line_;
      /** @brief Internal bicubic function used variable. */      
      unsigned int bicubic_grid_raster_col_;
      /** @brief Internal bicubic function used variable. */      
      unsigned int bicubic_buffer_line_;
      /** @brief Internal bicubic function used variable. */      
      unsigned int bicubic_buffer_col_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_buffer_[4][4];
      /** @brief Internal bicubic function used variable. */      
      double bicubic_offset_x_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_offset_y_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_h_weights_[4];
      /** @brief Internal bicubic function used variable. */      
      double bicubic_v_weights_[4];
      /** @brief Internal bicubic function used variable. */      
      double bicubic_h_weights_sum_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_v_weights_sum_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_int_line_accum_;
      /** @brief Internal bicubic function used variable. */      
      double bicubic_int_lines_values_[4];

      /**
       * @brief Set default internal state.
       */
      void init();

      /**
       * @brief Clear all allocated resources and
       * rollback to the initial state.
       */
      void clear();
      
      // just like the method interpolate above
      void nNInterpolation( const double& line, const double& col,
        const unsigned int& band, double& value );
        
      // just like the method interpolate above
      void bilinearInterpolation( const double& line, const double& col, 
        const unsigned int& band, double& value );
        
      // just like the method interpolate above
      void bicubicInterpolation( const double& line, const double& col, 
        const unsigned int& band, double& value );    

  };
  
/** @example TePDIInterpolator_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDIFILTERMASK_HPP
