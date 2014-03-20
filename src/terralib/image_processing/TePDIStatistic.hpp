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

#ifndef TEPDISTATISTIC_HPP
  #define TEPDISTATISTIC_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "TePDIJointHistogram.hpp"
  #include "TePDIHistogram.hpp"
  #include "../kernel/TeSharedPtr.h"
  #include "TePDITypes.hpp"

  #include "../kernel/TeMatrix.h"
  #include "../kernel/TeGeometry.h"

  #include <map>

  /**
   * @brief This is the class for statistic calcules over raster data.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters are:
   *
   * @param rasters ( TePDITypes::TePDIRasterVectorType ) - A vector of all used 
   * rasters ( NOTE: When more then one band is used from the same raster,
   * this raster must appear more then one time ).
   * @param bands ( std::vector< int > ) - A vector of all used bands
   * for each raster inside the reasters vector above respectively.
   *
   * @note The following parameters are optional, and will be used if
   * present.
   *
   * @param polygonset ( TePDITypes::TePDIPolygonSetPtrType ) - Restriction 
   * polygon set - The area where to do calcules over all rasters - If
   * not present the entire raster area will be used.   
   * @param histograms ( std::vector< TePDIHistogram::pointer > ) - 
   * A vector of all raster histogram references ( internal histogram 
   * generation will be disabled - This parameter cannot be used 
   * when the parameter polygonset is present );
   * @param iterator_strat ( TeStrategicIterator ) - The iteration strategy
   * used when analysing the raster elements ( default value:
   * TeBoxPixelIn - Only pixels with center inside the polygonset will be
   * considered ).
   *
   * @note For the methods using two or more channels(bands):
   * No spatial overlay is checked. Images with the same
   * number of lines and columns are required.
   */
  class PDI_DLL TePDIStatistic : public TePDIAlgorithm {
    public :
    
      /** @typedef TeSharedPtr< TePDIStatistic > pointer 
          Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIStatistic > pointer;
      
      /** @typedef const TeSharedPtr< TePDIStatistic > const_pointer 
          Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIStatistic > const_pointer;    

      /**
       * @brief Default Constructor.
       *
       */
      TePDIStatistic();

      /**
       * @brief Default Destructor
       */
      ~TePDIStatistic();

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
      
      /**
       * @brief Generates the histogram for a given raster/band.
       * 
       * @note The internal histogram generation will use 256 levels
       * for interpolated histograms when generated from floating point
       * data type rasters.
       * @param raster_index Index for the raster ( using the
       * supplied parameters ).
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The histogram pointer.
       */
      const TePDIHistogram& getHistogram( unsigned int raster_index, 
        unsigned int pol_index = 0 );      
      
      /**
       * @brief Generates the joint histogram for a given raster/band pair.
       * 
       * @param raster1_index Index for the first raster.
       * @param raster2_index Index for the second raster.
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The histogram reference.
       */
      const TePDIJointHistogram& getJointHistogram( 
        unsigned int raster1_index, unsigned int raster2_index,
        unsigned int pol_index = 0 );

      /**
       * @brief Sum of all pixels values.
       *
       * @param raster_index Index for the raster ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The sum value or DBL_MAX when an error occurs.
       */
      double getSum( unsigned int raster_index, unsigned int pol_index = 0 );
      
      /**
       * @brief The cubic sum of pixels values.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters ).
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The pixel level sum or DBL_MAX when an error occurs.
       */
      double getSum3( unsigned int raster_index, unsigned int pol_index = 0 );

      /**
       * @brief The quartic sum of pixels values.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters ).
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The pixel level sum or DBL_MAX when an error occurs.
       */
      double getSum4( unsigned int raster_index, unsigned int pol_index = 0 );      
      
      /**
       * @brief Mean pixels value for input_image1.
       *
       * @param raster_index Index for the raster ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The mean value or DBL_MAX when an error occurs.
       */
      double getMean( unsigned int raster_index, unsigned int pol_index = 0 );      

      /**
       * @brief Covariance between two bands.
       *
       * @param raster1_index Index for the first used raster / band ( using the
       * supplied parameters )
       * @param raster2_index Index for the second used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The covariance value or DBL_MAX when an error occurs.
       */
      double getCovariance( unsigned int raster1_index,  
        unsigned int raster2_index, unsigned int pol_index = 0 );

      /**
       * @brief Variance of one band data.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The variance value or DBL_MAX when an error occurs.
       */
      double getVariance( unsigned int raster_index, 
        unsigned int pol_index = 0 );
      
      /**
       * @brief Standard deviation of one band data.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The Standard deviation value or DBL_MAX when an 
       * error occurs.
       */
      double getStdDev( unsigned int raster_index,
        unsigned int pol_index = 0 );      
                
      /**
       * @brief Entropy of one band data.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The entropy value or DBL_MAX when an 
       * error occurs.
       */
      double getEntropy( unsigned int raster_index,
        unsigned int pol_index = 0 );
      
      /**
       * @brief The minimum pixel value of one band data.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The minimum pixel value value or DBL_MAX when an 
       * error occurs.
       */
      double getMin( unsigned int raster_index, unsigned int pol_index = 0 );      

      /**
       * @brief The maximum pixel value of one band data.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The maximum pixel value value or DBL_MAX when an 
       * error occurs.
       */
      double getMax( unsigned int raster_index, unsigned int pol_index = 0 );      

      /**
       * @brief The highest frequency pixel value of one band data (mode).
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The highest frequency pixel value of one band data (mode)
       * or DBL_MAX when an error occurs.
       */
      double getMode( unsigned int raster_index, unsigned int pol_index = 0 );      
                  
      /**
       * @brief Correlation between two bands.
       *
       * @param raster1_index Index for the first used raster / band ( using the
       * supplied parameters )
       * @param raster2_index Index for the second used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The correlation value or DBL_MAX when an error occurs.
       */
      double getCorrelation( unsigned int raster1_index,  
        unsigned int raster2_index, unsigned int pol_index = 0 );

      /**
       * @brief The Percentile value of one band data.
       *
       * @param sample_index Sample index ( 0 -> 100 );
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The highest frequency pixel value of one band data (mode)
       * or DBL_MAX when an error occurs.
       */
      double getPercentile( double sample_index, unsigned int raster_index, 
        unsigned int pol_index = 0 );

      /**
       * @brief Mean matrix calcule.
       *
       * @note Lines = 1, Columns = total bands/rasters number.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The mean matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;
       */
      TeMatrix getMeanMatrix( unsigned int pol_index = 0 );

      /**
       * @brief Central moments matrix.
       *
       * @note Lines = 3, Columns = total bands/rasters number.
       *
       * @param raster_index Index for the used raster / band ( using the
       * supplied parameters )
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The central moments matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;       
       */
      TeMatrix getCMMatrix( unsigned int pol_index = 0 );

      /**
       * @brief Assimetry coefficients matrix.
       *
       * @note Lines = 1, Columns = total bands/rasters number.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The Assimetry coefficients matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;          
       */
      TeMatrix getAssimetryMatrix( unsigned int pol_index = 0 );

      /**
       * @brief Kurtosis coefficients matrix.
       *
       * @note Lines = 1, Columns = total bands/rasters number.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The Kurtosis coefficients matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;               
       */
      TeMatrix getKurtosisMatrix( unsigned int pol_index = 0 );

      /**
       * @brief Variation coefficients matrix.
       *
       * @note Lines = 1, Columns = total bands/rasters number.
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).*
       * @return The Variation coefficients matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;                  
       */
      TeMatrix getVarCoefMatrix( unsigned int pol_index = 0 );
      
      /**
       * @brief Covariance matrix.
       *
       * @note Lines = total bands/rasters number, 
       *       Columns = total bands/rasters number.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).*
       * @return The Covariance matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;         
       */
      TeMatrix getCovMatrix( unsigned int pol_index = 0 );      
      
      /**
       * @brief Correlation matrix.
       *
       * @note Lines = total bands/rasters number, 
       *       Columns = total bands/rasters number.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).*
       * @return The Correlation matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;        
       */
      TeMatrix getCorMatrix( unsigned int pol_index = 0 );        
      
      /**
       * @brief Variance matrix.
       *
       * @note Lines = total bands/rasters number, 
       *       Columns = 1.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).*
       * @return The Variance matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;        
       */
      TeMatrix getVarMatrix( unsigned int pol_index = 0 );       
      
      /**
       * @brief Standard deviation matrix.
       *
       * @note Lines = total bands/rasters number, 
       *       Columns = 1.
       *
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).*
       * @return The Standard deviation matrix.
       * @note Those matrix elements where an error ocurred are
       * marked with value DBL_MAX;        
       */
      TeMatrix getStdDevMatrix( unsigned int pol_index = 0 );          

    protected :
    
      /**
       * @typedef std::pair< unsigned int, unsigned int  > SingleRasterCachesKeyT
       * A type definition for a cache related to a single raster.
       * @param "unsigned int" Raster index.
       * @param "unsigned int" Polygon index.       
       */    
      typedef std::pair< unsigned int, unsigned int  > SingleRasterCachesKeyT;
      
      /**
       * @typedef std::pair< unsigned int, SingleRasterCachesKeyT  > CoupleRasterCachesKeyT
       * A type definition for a cache related to a couple of rasters.
       * @param "unsigned int" Raster1 index.
       * @param "SingleRasterCachesKeyT" Raster2 index / polygon index.       
       */    
      typedef std::pair< unsigned int, SingleRasterCachesKeyT  > 
        CoupleRasterCachesKeyT;      
        
      /** Histogram cache type definition */
      typedef std::map< SingleRasterCachesKeyT, TePDIHistogram* > 
        HistoCacheT;
        
      /** Joint histogram cache type definition */  
      typedef std::map< CoupleRasterCachesKeyT, TePDIJointHistogram* >
        JHistoCacheT;
    
      /**
       * @brief The current restriction polygon set.
       */
      TePDITypes::TePDIPolygonSetPtrType polygonset_;
      
      /**
       * @brief The current rasters.
       */
      TePDITypes::TePDIRasterVectorType rasters_;
      
      /**
       * @brief The current rasters bands.
       */
      std::vector< int > bands_;

      /**
       * @brief Joint histograms cache.
       */
      JHistoCacheT jHistoCache_;
        
      /**
       * @brief A cache of level ordered histograms to avoid rebuilding
       * histograms all the times.
       */
      HistoCacheT histo_cache_;
      
      /**
       * @brief The current iterator strategy.
       */      
      TeStrategicIterator iterator_strat_;
      
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );
        
      /**
       * @brief Sum of products between all pixels from different bands.
       *
       * @param raster1_index Index for the first used raster / band ( 
       * using the supplied parameters ).
       * @param raster2_index Index for the second used raster / band ( 
       * using the supplied parameters ).
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The Sum of products or DBL_MAX when an 
       * error occurs.
       */
      double getSumPB1B2( unsigned int raster1_index,
        unsigned int raster2_index, unsigned int pol_index );

      //! Clears all internal allocated structures.
      void clear();       
   
  };
  
/** @example TePDIStatistic_test.cpp
 *    Shows how to use this class.
 */    

#endif //TEPDISTATISTIC_HPP
