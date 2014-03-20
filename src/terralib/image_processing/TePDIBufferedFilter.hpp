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

#ifndef TEPDIBUFFEREDFILTER_HPP
  #define TEPDIBUFFEREDFILTER_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "TePDITypes.hpp"
  #include "TePDIFilterMask.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @brief This is the base class for buffer based filters.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIFiltersGroup
   */
  class PDI_DLL TePDIBufferedFilter : public TePDIAlgorithm {
    public :
      /** @typedef Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIBufferedFilter > pointer;
      /** @typedef Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIBufferedFilter > const_pointer;

      /**
       * @brief Default Destructor
       */
      virtual ~TePDIBufferedFilter();

    protected :
      /**
       * @brief Convolution Buffer.
       */
      double** conv_buf_;

      /**
       * @brief Convolution Buffer lines.
       */
      unsigned int conv_buf_lines_;

      /**
       * @brief Convolution Buffer columns.
       */
      unsigned int conv_buf_columns_;

      /**
       * @brief Temporary representation of filter masks.
       */
      double** temp_maskmatrix_;

      /**
       * @brief Number of lines of the temporary representation of filter masks.
       */
      unsigned int temp_maskmatrix_lines_;

      /**
       * @brief Number of columns of the temporary representation of filter masks.
       */
      unsigned int temp_maskmatrix_columns_;

      /**
       * @brief Default Constructor.
       *
       */
      TePDIBufferedFilter();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      virtual void ResetState( const TePDIParameters& params );

      /**
       * @brief Initiates the convolution buffer.
       */
      void init_conv_buf();

      /**
       * @brief Initiates the internal filter mask matrix representation.
       */
      void init_maskmatrix();

      /**
       * @brief Resets the convolution buffer.
       *
       * @param lines Convolution buffer lines.
       * @param columns Convolution buffer columns.
       */
      void reset_conv_buf( unsigned int lines, unsigned int columns );

      /**
       * @brief Resets the internal filter mask matrix representation..
       *
       * @param mask The new filter Mask.
       */
      void reset_maskmatrix( TePDIFilterMask::pointer& mask );

      /**
       * @brief Updates the convolution buffer with a new raster line.
       *
       * @param inRaster Input Raster.
       * @param line Raster line.
       * @param band Raster band.
       */
      void up_conv_buf( TePDITypes::TePDIRasterPtrType& inRaster, unsigned int line,
        unsigned int band );

      /**
       * @brief Generates statistics of a suplied window inside the convolution buffer.
       *
       * @param start_line Convolution buffer start line.
       * @param start_column Convolution buffer start column.
       * @param width Window width.
       * @param height Window height.
       * @param mean Mean.
       * @param variance variance.
       */
      void conv_buf_estatistics( unsigned int start_line, unsigned int start_column,
        unsigned int width, unsigned int height, double& mean, double& variance );

      /**
       * @brief Rools up the convolution buffer count lines.
       *
       * @param count Count times to rool convolution buffer.
       */
      void conv_buf_roolup( unsigned int count );

  };

#endif //TEPDIBUFFEREDFILTER_HPP
