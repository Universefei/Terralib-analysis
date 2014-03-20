/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2004 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

/**
 * @file TePDIRaster2Vector.hpp
 *
 * @brief This file contains functions to support vectorizing of
 * raster data into a TerraLib database
 * the input raster is a 4-connected rotulated image :
 *               *each region is for 4-connected
 *               *each region has a unique identifier
 */

#ifndef  TEPDIRASTER2VECTOR_HPP
  #define  TEPDIRASTER2VECTOR_HPP

  #include "TePDIParameters.hpp"
  #include "TePDIAlgorithm.hpp"

  #include "TePDITileIndexer.hpp"
  
  #include "../kernel/TeRaster.h"
  #include "../kernel/TeLayer.h"
  #include "../kernel/TeRTree.h"


  /**
    *  @brief A polygon container node class. 
    */    
  class PDI_DLL TePDIRaster2VectorPolStruct
  {
    public:
    
      /**
      * @brief The value ( color ) related to a polygon.
      */
      int value_;

      /**
      * @brief The polygon tile indexer Y Axis resolution.
      */
      double tile_indexer_dy_;
      
      /**
      * @brief The polygon tile indexer pointer.
      */
      TePDITileIndexer* indexer_;    
      
      /**
      * @brief The stored polygon instance.
      */
      TePolygon poly_;
      
      /**
      * @brief Init internal variables.
      */    
      void init()
      {
        value_ = 0;
        tile_indexer_dy_ = 1.0;
        indexer_ = 0;
      };
      
      /**
      * @brief Init internal variables.
      */    
      void clear()
      {
        if( indexer_ ) {
          delete indexer_;
        }
        
        init();
      };    
      
      /**
      * @brief Default constructor.
      */       
      TePDIRaster2VectorPolStruct()
      {
        init();
      };
      
      /**
      * @brief Alternative constructor.
      * @param external External reference.
      */       
      TePDIRaster2VectorPolStruct( 
        const TePDIRaster2VectorPolStruct& external )
      {
        init();
        
        reset( external.poly_, external.value_, 
          external.tile_indexer_dy_ );
      }
      
      /**
      * @brief Alternative constructor.
      * @param p Polygon reference.
      * @param v V.
      * @param tile_indexer_dy Tile indexer dy.
      */       
      TePDIRaster2VectorPolStruct( const TePolygon& p, 
        const int& v, 
        const double& tile_indexer_dy )
      {
        init();
        
        reset( p, v, tile_indexer_dy );
      }
      
      /**
      * @brief Default destructor.
      */       
      ~TePDIRaster2VectorPolStruct()
      {
        clear();
      };      
      
      /**
      * @brief Operator= overload.
      * @param external External reference.
      * @return a reference to the external object.
      */         
      const TePDIRaster2VectorPolStruct& operator=( 
        const TePDIRaster2VectorPolStruct& external )
      {
        reset( external.poly_, external.value_, 
          external.tile_indexer_dy_ );
          
        return external;
      }
      
      /**
      * @brief Reset the current instance.
      * @param p Polygon reference.
      * @param v V.
      * @param tile_indexer_dy Tile indexer dy.
      */       
      void reset( const TePolygon& p, const int& v, 
        const double& tile_indexer_dy )
      {
        clear();
        
        poly_ = p;
        value_ = v;
        tile_indexer_dy_ = tile_indexer_dy;
        
        indexer_ = new TePDITileIndexer( poly_, 
          tile_indexer_dy_ );       
      }
  };

  /**
    * @brief Raster vectorizer.
    * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
    * @author Eric S. Abreu <eric@dpi.inpe.br>
    * @author Gilberto Ribeiro de Quiroz <gribeiro@dpi.inpe.br>
    * @author Nicolas Despres <nicolasdespres@wanadoo.fr>
    * @ingroup TePDIGeneralAlgoGroup
    *
    * @note The general required parameters by TePDIRaster2Vector :
    *
    * @param rotulated_image (TePDITypes::TePDIRasterPtrType),
    * The rotulated_image raster ( 1 band, not floating point, 4-connected 
    * - each region is for 4-connected ).
    * @param output_polsets ( TePDITypes::TePDIPolSetMapPtrType ) - 
    * The user supplied output polygon sets map where the generated polygons 
    * will be stored (each polygon set contains polygons related to the
    * same pixel value).
    * @param channel (unsigned int) - The raster channel to process.
    *
    * @note Optional parameters:
    *
    * @param max_pols (unsigned long int) - The maximum number of poligons
    * to generate (default:0 - no maximum defined).
    */
  class PDI_DLL TePDIRaster2Vector : public TePDIAlgorithm
  {
    public:
    /** @typedef Type definition for a instance pointer */
    typedef TeSharedPtr< TePDIRaster2Vector > pointer;
    /** @typedef Type definition for a const instance pointer */
    typedef const TeSharedPtr< TePDIRaster2Vector > const_pointer;

    /**
      * @brief Default Constructor.
      */
    TePDIRaster2Vector();

    /**
      * @brief Default destructor.
      */
    ~TePDIRaster2Vector();

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

    protected:

      /**
      * @brief Flag indication for dummy value use ( rotulated image ).
      */
      bool raster_uses_dummy_;
      
      /**
      * @brief The used dummy value.
      */
      double dummy_value_;
      
      /**
      * @brief Directions vector.
      */ 
      TeCoord2D directions_[8];  
      
      /** @brief pointer to the input image */
      TePDITypes::TePDIRasterPtrType raster_;

      /** @brief resolution x */
      double resx_;
      
      /** @brief resolution y */
      double resy_;
      
      /** @brief number of lines */
      long   nLines_;
      
      /** @brief number of cols */
      long   nCols_;

      /** @brief number of cols */
      unsigned int raster_channel_;
      
      /** 
      *  @brief A RTree instance pointer to optimize the searching of points
      *  inside already created polygons 
      */
      TeSAM::TeRTree<unsigned int, 8, 4>*    rtreePolygons_;
      
      /** @brief Vector of all polygons */
      vector<TePDIRaster2VectorPolStruct>  containerPolygons_;    
      
      /**
      * @brief Runs the current algorithm implementation.
      *
      * @return true if OK. false on error.
      */
      bool RunImplementation();

      /**
      * @brief Reset the internal state to the initial state.
      *
      * @param params The new parameters referente at initial state.
      */
      void ResetState( const TePDIParameters& params );

      /**
        * @brief Detects a edge of a cell in Raster.
        *
        * @param i abscissa( column ) of the upper-left point of the shape
        * @param j ordinate( line ) of the upper-left point of the shape
        * @param line 2D Line.
        * @return true if ok, otherwise false
        */
      bool detectEdge(long i, long j, TeLine2D &line );
        
      /**
      * @brief Tests if the current point is a edge start.
      * @param x X coord.
      * @param y Y coord.
      * @return true if the current point is a edge start, false if not.
      */
      bool startingEdgeTest( const int& x, const int& y );
      
      /**
      * @brief Clear all internally allocated resources.
      */
      void clear();    

  };


  /** @example TePDIRaster2Vector_test.cpp
  *    Shows how to use this class.
  */  
 
#endif
