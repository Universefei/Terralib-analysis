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

#ifndef TEPDIHISTOGRAM_HPP
  #define TEPDIHISTOGRAM_HPP

  #include "TePDITypes.hpp"
  
  #include "../kernel/TeSharedPtr.h"
  #include "../kernel/TeGeometry.h"

  #include <map>

  /**
   * @brief Base Histogram class (from std::map ).
   * @ingroup PDIAux
   */  
  class PDI_DLL TePDIHMapSpec : public std::map< double, unsigned int > {};
  
  /**
   * @class TePDIHistogram
   * @brief This class deals with histograms and related tasks.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIHistogram : public TePDIHMapSpec {
    protected :
    
     /**
      * @brief Base Histogram class (from std::map ).
      * @ingroup PDIAux
      */     
      class PDI_DLL TePolygonSetPointer : public TeSharedPtr< TePolygonSet> {};
      
    public :
      /** @typedef TeSharedPtr< TePDIHistogram > pointer
       * Type definition for a instance pointer. */
      typedef TeSharedPtr< TePDIHistogram > pointer;
      /** @typedef const TeSharedPtr< TePDIHistogram > const_pointer
       * Type definition for a const instance pointer. */
      typedef const TeSharedPtr< TePDIHistogram > const_pointer;
  
      //! const_iterator type definition
      typedef TePDIHMapSpec::const_iterator const_iterator;      

      //! iterator type definition
      typedef TePDIHMapSpec::iterator iterator;      

      /**
       * @brief Default Constructor
       *
       */
      TePDIHistogram();
      
      /**
       * @brief Alternative constructor.
       *
       * @param external External histogram reference.
       *
       */
      TePDIHistogram( const TePDIHistogram& external );      
      
      /**
       * @brief Alternative constructor from std::map.
       *
       * @param external External map reference.
       *
       */
      TePDIHistogram( const std::map< double, unsigned int >& external );

      /**
       * @brief Default Destructor
       */
      ~TePDIHistogram();

      /**
       * @brief Generates a level-ordered histogram ( fixed step ).
       *
       * @note If the input raster has float data, the histogram will be
       * interpolated.
       *
       * @param in_raster Input raster.
       * @param band Band number.
       * @param levels The number of interpolated levels, and if set to zero,
       * the number of levels will be automaticlly found by using
       * level step with value one.
       * @param iterstrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn"
       * @param polsetptr The restriction polygon set.
       * @return true if the histogram has been generated, false on error.
       */
      bool reset(
        const TePDITypes::TePDIRasterPtrType& in_raster,
        unsigned int band, unsigned int levels,
        TeStrategicIterator iterstrat,
        const TeSharedPtr< TePolygonSet>& polsetptr = TePolygonSetPointer() );
        
      /**
       * @brief Histogram discrete verification.
       *
       * @return true if the histogram has only discrete levels.
       */
      bool IsDiscrete() const;
      
      /**
       * @brief Histogram fixed step verification.
       *
       * @return true if the histogram has fixed step.
       */
      bool hasFixedStep() const;      

      /**
       * @brief Round the histogram levels to the near integer generating a
       * descrete ( fixed step ) histogram.
       *
       * @return True if the process was concluded successfully, false if not.
       */
      bool Discretize();

      /**
       * @brief Histogram minimum level.
       *
       * @return The histogram minimum level.
       */
      double GetMinLevel() const;

      /**
       * @brief Histogram maximum level.
       *
       * @return The histogram maximum level.
       */
      double GetMaxLevel() const;

      /**
       * @brief Histogram minimum count (frequency) value for all levels.
       *
       * @return The histogram minimum count value.
       */
      unsigned int GetMinCount() const;

      /**
       * @brief Histogram maximum count (frequency) value for all levels.
       *
       * @return The histogram maximum count value.
       */
      unsigned int GetMaxCount() const;
      
      /**
       * @brief Histogram total count ( the sum of all frequencies ).
       *
       * @return The histogram total count
       */
      unsigned int getTotalCount() const;

      /**
       * @brief std::map::clear() overload.
       *
       */
      void clear();
      
      /**
       * @brief operator= overload.
       *
       */
      const TePDIHistogram& operator=( const TePDIHistogram& external );      
      
      /**
       * @brief operator= overload for simple std:maps.
       *
       */
      const TePDIHistogram& operator=( 
        const std::map< double, unsigned int >& external );
      
      /**
       * @brief Enable / Disable the progress interface.
       *
       * @param enabled Flag to enable ( true - default ) or disable ( false ).
       */      
      void ToggleProgressInt( bool enabled );
      
       // operator== overload.
      bool operator==( const TePDIHistogram& external ) const;
      
      /**
       * @brief Returns the histogram size.
       * @return Returns the histogram size.
       */
      unsigned int size() const;      
      
      /**
       * @brief Ouput histogram to string.
       */      
      std::string toString() const;

    protected :

      /**
       * @brief Progress interface enabled status.
       */
      bool progress_int_enabled_;
      
      /**
       * @brief Reset the internal variables with default values.
       */
      void init();      

      /**
       * @brief Generates a level-ordered histogram ( fixed step ) from
       * not floating point data type rasters.
       *
       * @note No trows generated.
       *
       * @param in_raster Input raster.
       * @param band Band number.
       * @param iterstrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn"       
       * @param polsetptr The restriction polygon set.
       * @return true if the histogram has been generated, false on error.
       */
      bool SetH(
        const TePDITypes::TePDIRasterPtrType& in_raster,
        unsigned int band,
        TeStrategicIterator iterstrat,
        const TeSharedPtr< TePolygonSet>& polsetptr );      
      
      /**
       * @brief Generates a level-ordered histogram ( fixed step )for an 
       * palette based raster.
       *
       * @note No trows generated.
       *
       * @param in_raster Input raster.
       * @param band Band number.
       * @param iterstrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn"        
       * @param polsetptr The restriction polygon set.
       * @return true if the histogram has been generated, false on error.
       */
      bool SetPaletteBasedH(
        const TePDITypes::TePDIRasterPtrType& in_raster,
        unsigned int band,
        TeStrategicIterator iterstrat,
        const TeSharedPtr< TePolygonSet>& polsetptr );
        
      /**
       * @brief Generates a level-ordered histogram ( fixed step )for an 8 
       * bit raster.
       *
       * @note No trows generated.
       *
       * @param in_raster Input raster.
       * @param band Band number.
       * @param iterstrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn"         
       * @param polsetptr The restriction polygon set.
       * @return true if the histogram has been generated, false on error.
       */
      bool Set8BitH(
        const TePDITypes::TePDIRasterPtrType& in_raster,
        unsigned int band,
        TeStrategicIterator iterstrat,
        const TeSharedPtr< TePolygonSet>& polsetptr );        
        
      /**
       * @brief Generates a level-ordered histogram (fixed step) for an 
       * 16 bit raster.
       *
       * @note No trows generated.
       *
       * @param in_raster Input raster.
       * @param band Band number.
       * @param iterstrat The raster iteration strategy (if you
       * do not know what this is use TeBoxPixelIn"         
       * @param polsetptr The restriction polygon set.
       * @return true if the histogram has been generated, false on error.
       */
      bool Set16BitH(
        const TePDITypes::TePDIRasterPtrType& in_raster,
        unsigned int band,
        TeStrategicIterator iterstrat,
        const TeSharedPtr< TePolygonSet>& polsetptr );
        
      /**
       * @brief Calculate the number of progress steps when using
       * a polygon iterator over a raster for the supplied 
       * polygon set.
       *
       * @param polsetptr The polygon set pointer.
       * @param resy Y axis resolution.
       */        
      unsigned long int getProgressSteps( 
        const TeSharedPtr< TePolygonSet>& polsetptr, double resy );
    
  };
  
/** @example TePDIHistogram_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDIHISTOGRAM_HPP
