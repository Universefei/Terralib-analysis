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

#ifndef TEPDIBATCHGEOMOSAIC_HPP
  #define TEPDIBATCHGEOMOSAIC_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @class TePDIBatchGeoMosaic
   * @brief Mosaic of many geo-referenced rasters.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIMosaicGroup
   *
   * @note The required parameters are:
   * @param input_rasters ( TePDITypes::TePDIRasterVectorType ) - 
   * Input rasters vector (i.e if using two bands from each
   * raster, the vector will follow the order [ raster0 raster0
   * raster1 raster1 ...... ] ).
   * @param bands ( std::vector< unsigned int > ) - The bands from each input_raster 
   * following the order [Raster0 Band 0 - Raster0 Band 1 - ....
   * RasterN Band 0 - RasterN Band 1 - .... - RasterN Band N ].
   * @param output_raster ( TePDITypes::TePDIRasterPtrType ) - Output raster.
   *
   * @note The optional parameters are:
   * @param blend_method ( TePDIBlender::BlendMethod ) - Blending method
   * (default: TePDIBlender::NoBlendMethod).     
   * @param interp_method ( TePDIInterpolator::InterpMethod ) - Interpolation
   * method (default TePDIInterpolator::NNMethod) .
   * @param dummy_value (double) - A dummy pixel value for use 
   * in pixels where no data is available.
   * @param auto_equalize ( int ) - If present ( any value )
   * auto-equalizing will be made ( using overlap area reference ).    
   *
   * @note The output_raster pixel resolution will follow the best pixel 
   * resolution found.
   * @note Please check each blending strategy for other specific required
   * parameters.
   *
   * @example TePDIMosaic_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIBatchGeoMosaic : public TePDIAlgorithm {
    public :
      
      TePDIBatchGeoMosaic();

      ~TePDIBatchGeoMosaic();
      
      //Overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;      

    protected :
    
      /** @brief Rasters list node type definition */    
      class RastersListNodeT
      {
        public :
        
          /** @brief Raster pointer */    
          TePDITypes::TePDIRasterPtrType rasterPtr_;
          
          /** @brief Polygon delimiting the valid raster data (under the 
          global reference projection) */    
          TePolygon dataPolygonRefProj_;
          
          /** @brief Polygon delimiting the valid raster data (under the 
          raster projection) */    
          TePolygon dataPolygonRaProj_;          
          
          /** Bands to use for this raster */
          std::vector< unsigned int > channelsVec_;  
          
          RastersListNodeT();
          
          ~RastersListNodeT();
          
          const RastersListNodeT& operator=( const RastersListNodeT& other );
      };
    
      /** @brief Rasters list type definition */    
      typedef std::list< RastersListNodeT > RastersListT;
    
      //Overloaded
      void ResetState( const TePDIParameters& params );    
     
      //Overloaded
      bool RunImplementation();     
      
      /**
       * @brief Find the channels for the raster with index inputRastersIdx.
       * @param inputRastersIdx Raster index inside input_rasters;
       * @param bands The user suplied bands vector.
       * @param input_rasters The user suplied input rasters pointers vector.
       * @param The channels to use for the required raster.
       * @return true if OK, false on errors.
       */          
      bool getRUsedChannels( 
        const unsigned int& inputRastersIdx,
        const std::vector< unsigned int >& bands,
        const TePDITypes::TePDIRasterVectorType& input_rasters,
        std::vector< unsigned int >& channels ) const;
        
      /**
       * @brief Initiate the rasters list.
       * @param bands The user suplied bands vector..
       * @param input_rasters The user suplied input rasters pointers vector.
       * @param referenceProjPtr Reference projection pointer.
       * @param rList The rasters list to initialize.
       * @return true if OK, false on errors.
       */       
      bool initRastersList( const std::vector< unsigned int >& bands,
        const TePDITypes::TePDIRasterVectorType& input_rasters,      
        TeProjection* referenceProjPtr,
        RastersListT& rList ) const;        
        
      /**
       * @brief Return the most frequent projection among the given rasters.
       * @param rastersVec The input rasters vector.
       * @return A pointer to the choosed projection or NULL if not found.
       * @note The returned projection should not be altered !!
       */       
      TeProjection* getFreqProjection( 
        const TePDITypes::TePDIRasterVectorType& rastersVec ) const;
        
      /**
       * @brief Extract the raster closer to given inputNode.
       * @param rList The rasters list. The input rasters vector.
       * @param inputNode Input node with the input raster.
       * @param outputNode The extracted node.
       * @return true if ok, false on errors.
       */         
      bool extractCloserRasterNode( RastersListT& rList,
        const RastersListNodeT& inputNode,
        RastersListNodeT& outputNode ) const;
        
      /**
       * @brief Extract the raster closer to given inputNode.
       * @param rList The rasters list. The input rasters vector.
       * @param outputNode1 The extracted node 1.
       * @param outputNode2 The extracted node 2.
       * @return true if ok, false on errors.
       */         
      bool extractClosestRastersNodes( RastersListT& rList,
        RastersListNodeT& outputNode1, RastersListNodeT& outputNode2 ) const;        

  };

#endif
