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

#ifndef TEPDIFILTERMASK_HPP
  #define TEPDIFILTERMASK_HPP

 #include "TePDIDefines.hpp"
 #include "../kernel/TeSharedPtr.h"

  #include "../kernel/TeMatrix.h"

  /**
   * @class TePDIFilterMask
   * @brief This is the class for image filtering mask.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIFilterMask {
    public :
      /** @typedef TeSharedPtr< TePDIFilterMask > pointer
          Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIFilterMask > pointer;
      /** @typedef const TeSharedPtr< TePDIFilterMask > const_pointer 
          Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIFilterMask > const_pointer;

      /**
       * @brief Default Constructor.
       *
       */
      TePDIFilterMask();

      /**
       * @brief Alternative Constructor.
       *
       * @param width Mask width ( odd values ).
       * @param gain Mask gain.
       */
      TePDIFilterMask( unsigned int width, double gain = 0 );

      /**
       * @brief Default Destructor
       */
      ~TePDIFilterMask();

      /**
       * @brief Resets the current mask.
       *
       * @param width Mask width.
       * @param gain Mask gain.
       */
      void reset( unsigned int width, double gain = 0 );

      /**
       * @brief The current mask coluns number.
       *
       * @return Mask columns number.
       */
      unsigned int columns() const;

      /**
       * @brief The current mask lines number.
       *
       * @return Mask columns number.
       */
      unsigned int lines() const;

      /**
       * @brief Assingns a new value to the element.
       *
       * @param line Element line.
       * @param column Element column.
       * @param value Element velue.
       */
      void set( unsigned int line, unsigned int column, double value );

      /**
       * @brief Returns the required element value.
       *
       * @param line Element line.
       * @param column Element column.
       * @return The element value.
       */
      double get( unsigned int line, unsigned int column ) const;

      /**
       * @brief Returns the sum of the weights ( withour gain ).
       *
       * @return The sum of the weights ( withour gain ).
       */
      double getSum();

      /**
       * @brief Operator = overload.
       *
       * @param external External Mask reference.
       */
	  void operator=( const TePDIFilterMask& external );

      /**
       * @brief Normalizes the current mask.
       */
      void normalize();

      /**
       * @brief Return a simple matrix[line][column] with the current weights.
       *
       * @note No automatic deletion will be performed for the returned
       * matrix.
       *
       * @return The weights matrix ( based on gain ).
       */
      double** getWeightsMatrix() const;

      /**
       * @brief Frees the memory of a weights matrix.
       *
       * @param matrix The weights matrix.
       * @param lines The lines number.
       */
      static void deleteWeightsMatrix( double** matrix, unsigned int lines );


      /**
       * @brief Verify if the mask is a morfological filter mask.
       *
       * @return true if the mask is a morfological filter mask, false if not.
       */
      bool isMorfMask();

      /**
       * @brief Creates an dummy average mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_Dummy();
      
      /**
       * @brief Creates an 3x3 average mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_Avg3x3( bool no_norm = false );

      /**
       * @brief Creates an 5x5 average mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_Avg5x5( bool no_norm = false );

      /**
       * @brief Creates an 7x7 average mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_Avg7x7( bool no_norm = false );

      /**
       * @brief Creates an 3x3 South mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_S();

      /**
       * @brief Creates an 3x3 North mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_N();

      /**
       * @brief Creates an 3x3 East mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_E();

      /**
       * @brief Creates an 3x3 West mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_W();

      /**
       * @brief Creates an 3x3 North-West mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_NW();

      /**
       * @brief Creates an 3x3 South-West mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_SW();

      /**
       * @brief Creates an 3x3 North-East mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_NE();

      /**
       * @brief Creates an 3x3 South-East mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_SE();

      /**
       * @brief Creates an non directional low frequencies pass filter mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_NDLow();

      /**
       * @brief Creates an non directional medium frequencies pass filter mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_NDMed();

      /**
       * @brief Creates an non directional high frequencies pass filter mask.
       *
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_NDHigh();

      /**
       * @brief Creates an filter mask for TM imagens enhancement.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_TMEnh( bool no_norm = false );

      /**
       * @brief Creates an morfological (D) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfD();

      /**
       * @brief Creates an morfological (E) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfE();

      /**
       * @brief Creates an morfological (M) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfM();

      /**
       * @brief Creates an morfological (M-) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfM_();

      /**
       * @brief Creates an morfological (M+) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfMPlus();

      /**
       * @brief Creates an morfological (Mtot) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfMtot();

      /**
       * @brief Creates an morfological (MX) filter mask.
       *
       * @param no_norm if true, no normalization will be performed.
       * @return The created mask.
       */
      static TePDIFilterMask::pointer create_MorfMX();

    protected :

      /**
       * @brief Internal weights matrix reference
       */
      mutable TeMatrix internal_matrix_;

      /**
       * @brief Internal matrix weights sum
       */
      double weights_sum_;

      /**
       * @brief Internal matrix weights gain
       */
      double weights_gain_;
  };
  
/** @example TePDIFilterMask_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDIFILTERMASK_HPP
