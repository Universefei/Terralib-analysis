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

#ifndef TEPDIJOINTHISTOGRAM_HPP
  #define TEPDIJOINTHISTOGRAM_HPP

  #include "TePDIHistogram.hpp"
  #include "TePDITypes.hpp"

  #include <map>
  
  class TePDIHistogram;
  
  /**
   * @class TePDIJointHistogram
   * @brief Raster joint histogram.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIJointHistogram
  {
    public :
      
      //! Joint histogram container type definition.
      typedef std::map< std::pair< double, double >, unsigned int > ConteinerT;
      
      //! Iterator type definition.
      typedef ConteinerT::iterator iterator;
      
      //! Const-Iterator type definition.
      typedef ConteinerT::const_iterator const_iterator;
        
      TePDIJointHistogram();
      
      TePDIJointHistogram( const TePDIJointHistogram& external );      

       ~TePDIJointHistogram();
      
      const TePDIJointHistogram& operator=( const TePDIJointHistogram& external );      
      
      /**
       * @brief Enable / Disable the progress interface.
       *
       * @param enabled Flag to enable ( true - default ) or disable ( false ).
       */      
      void toggleProgress( bool enabled );

      /**
       * @brief Update the current histogram reading data from the supplied
       * rasters.
       *
       * @param input1Raster Input raster.
       * @param input1RasterChn Input raster channel/band number.
       * @param input2Raster Input raster.
       * @param input2RasterChn Input raster channel/band number.
       * @param iterStrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn).
       * @param levels The number of histogram levels, and if set to zero,
       * the number of levels will be automaticlly found by using
       * level step with value one.
       * @param restricPolSet The restriction polygon set.
       * @return true if OK, false on errors.
       * @note Pallete based rasters not supported.
       * @note Only rasters with the same pixel resolution must be used.
       * @note Floating point rasters will generate an aproximated histogram
           with the number of gray levels configured by the parameter levels.
       */
      bool update( TeRaster& inputRaster1, 
        unsigned int inputRasterChn1, TeRaster& inputRaster2, 
        unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
        unsigned int levels,
        const TePolygonSet& restricPolSet = TePolygonSet() );
      
      /**
       * @brief Clear the current histogram contents.
       */      
      void clear();      
      
      //!  Return a reference to raster 1 histogram.
      /*!
        \return A reference to raster 1 histogram.
      */
      const TePDIHistogram& getRaster1Hist() const;
      
      //!  Return a reference to raster 2 histogram.
      /*!
        \return A reference to raster 2 histogram.
      */
      const TePDIHistogram& getRaster2Hist() const;
      
      //! Returns an iterator pointing to the beginning of the joint histogram.
      const_iterator begin() const;
      
      //! Returns an iterator pointing to the end of the joint histogram.
      const_iterator end() const;
      
      //! Returns the histogram size.
      unsigned int size() const;

      //! Generate a inversed joint histogram from the current one.
      /*!
          \param external A external output histogram.
          \note The inversed histogram has swapped raster1 and raster2 
          values.
      */
      void getInverseJHist( TePDIJointHistogram& external );
      
      //! Returns the sum of all pixel velues frequencies.
      /*! \return The sum of all pixel velues frequencies. */
      unsigned long int getFreqSum() const;     
       
      /**
       * @brief Ouput histogram to string.
       */      
      std::string toString() const;       
      
    protected :
      
      //! Progress enabled flag.
      bool progressEnabled_;

      //! Internal container.
      ConteinerT jHistContainer_;
      
      //! Raster1 histogram.
      TePDIHistogram r1Histogram_;
      
      //! Raster2 histogram.
      TePDIHistogram r2Histogram_;

      // Internal used variables to store temporary histogram buffers.
	    unsigned int** init_jhist_;
      unsigned int init_jhist_lines_;
      unsigned int init_jhist_cols_;
	    unsigned int* init_r1hist_;
	    unsigned int* init_r2hist_;
      
      // Just like the Update method, but for floating point rasters.
      bool updateFloat( TeRaster& inputRaster1, 
          unsigned int inputRasterChn1, TeRaster& inputRaster2, 
          unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
          unsigned int levels, const TePolygonSet& restricPolSet );
      
      // Just like the Update method, but for integer rasters.
      bool updateInteger( TeRaster& inputRaster1, 
          unsigned int inputRasterChn1, TeRaster& inputRaster2, 
          unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
          const TePolygonSet& restricPolSet );
      
      // Just like the Update method, but for 8 bits rasters.
      bool update8Bits( TeRaster& inputRaster1, 
          unsigned int inputRasterChn1, TeRaster& inputRaster2, 
          unsigned int inputRasterChn2, TeStrategicIterator iterStrat, 
          const TePolygonSet& restricPolSet ); 
      
      /**
       * @brief Calculate the number of progress steps when using
       * a polygon iterator over a raster for the supplied 
       * polygon set.
       *
       * @param polset The polygon set.
       * @param resy Y axis resolution.
       */        
      unsigned long int getProgressSteps( 
        const TePolygonSet& polset, double resy );     

      //! Clear the internal temporary histogram buffers ( init_jhist_, init_jhist_lines_, init_jhist_cols_, init_r1hist_, init_r2hist_.
      void clearTempHistBuffers();
    
  };
  
/** @example TePDIJointHistogram_test.cpp
 *    Shows how to use this class.
 */    

#endif
