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

#ifndef TEPDIHARALICK_HPP
  #define TEPDIHARALICK_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIParameters.hpp"
  #include "TePDITypes.hpp"
  
  #include <map>

  /**
   * @class TePDIHaralick
   * @brief Haralick texture features over raster regions.
   * @author Vanessa Oliveira de Souza <vanessa@dpi.inpe.br>
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note Reference 1 : Textural Features for Image Classification -
   * Robert M. Haralick, K. Shanmugam, Its'hak Dinstein.
   * @note Reference 2: Computer and Robot Vision - Robert M. Haralick - 
   * Addison-Wesley Publishing Company
   * @note Reference 3: Definiens Developer 7 Reference Book
   * @note Reference 4 : Remote Sensing - Models and Methods for Image
   * Processing 2nd Edition - Robert A. Schowengerdt - Academic Press.
   *
   * @note The general required parameters are:
   *
   * @param input_raster (TePDITypes::TePDIRasterPtrType) Input image -
   * no floating point images allowed.
   * @param direction ( TePDIHaralick::DirectionT ) - The direction
   * (or direction combination bit-field mask) to be used.
   * @param polygonset ( TePDITypes::TePDIPolygonSetPtrType ) - Restriction 
   * polygon set - The area where to do calcules over all rasters.
   *
   * @example TePDIHaralick_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIHaralick : public TePDIAlgorithm 
  {
    public :
      
      /** @enum DirectionT Concurrence matrix used directions */ 
      enum DirectionT 
      {
        /** @brief Invalid direction */
        NoDirection = 0,
        /** @brief North direction */
        North = 1,
        /** @brief NorthEast direction */
        NorthEast = 2,
        /** @brief East direction */
        East = 4,
        /** @brief SouthEast direction */
        SouthEast = 8,
        /** @brief South direction */
        South = 16,
        /** @brief SouthWest direction */
        SouthWest = 32,
        /** @brief West direction */
        West = 64,
        /** @brief NorthWest direction */
        NorthWest = 128
      };

      TePDIHaralick();

      /**
       * @brief Default Destructor
       */
      ~TePDIHaralick();

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
       * @brief GLCM Entropy of one polygon inside one band (Reference 1).
       *
       * @param band Raster band.
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @param entropyValue The generated value.
       * @return true if OK, false on errors.
       */
      bool getGLCMEntropy( unsigned int band, unsigned int pol_index,
        double& entropyValue );
        
       /**
        * @brief GLCM Energy (a.k.a GLCM angular second moment) (Reference 2).
        * The energy measures textural uniformity, that is, repetition of pairs of pixel in the image. 
        * High values of energy indicate that the distribution of gray level in region in the image 
        * has a constant distribution or a periodic form.
        *
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param energyValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMEnergy( unsigned int band, unsigned int pol_index,
        double& energyValue );
  
        /**
        * @brief GLCM Contrast of one polygon inside one band (Reference 4).
        * The contrast is a estimate of the local variations. 
        * Average of the squares of the differences between pixels.
        *
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param contrastValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMContrast( unsigned int band, unsigned int pol_index,
        double& contrastValue );
  
        /**
        * @brief GLCM Homogeneity (a.k.a. Inverse Difference Moment) of 
        * one polygon inside one band (Reference 1).
        * This measure of texture results in great values for regions with similar gray level. 
        * The contrast and the homogeneity are inversely correlacionad
        *
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param homogeneityValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMHomogeneity( unsigned int band, unsigned int pol_index,
        double& homogeneityValue );
  
        /**
        * @brief GLCM QuiSquare of one polygon inside one band.
        * This measure can be understood as a normalization of the energy for the linear dependence 
        * of the gray level tones in a region in the image. 
        *        
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param QuiSquareValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMQuiSquare( unsigned int band, unsigned int pol_index,
        double& QuiSquareValue );
      
        /**
        * @brief GLCM Mean of one polygon inside one band (Reference 3).
        * The GLCM mean is the average expressed in terms of the GLCM. 
        *        
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param meanValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMMean( unsigned int band, unsigned int pol_index,
        double& meanValue );        
      
        /**
        * @brief GLCM Dissimilarity of one polygon inside one band
        * (Reference 3).
        * Similar to contrast, but increases linearly. High if the local region 
        * has a high contrast.
        *        
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param dissimilarityValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMDissimilarity( unsigned int band, unsigned int pol_index,
        double& dissimilarityValue );      
      
       /**
        * @brief GLCM standard deviation (Reference 3).
        * @defails GLCM standard deviation uses the GLCM, therefore it deals 
        * specifically with the combinations of reference and neighbor pixels. 
        * Thus, it is not the same as the simple standard deviation of gray 
        * levels in the original image.       
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param ang2ndMomentValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMStdDev( unsigned int band, unsigned int pol_index,
        double& stdDevValue );    
           
       /**
        * @brief GLCM correlation (Reference 2).
        * @param band Raster band.
        * @param pol_index The polygon index inside the polygon set
        * ( if no polygon set is present, use de default zero value ).
        * @param correlationValue The generated value.
        * @return true if OK, false on errors.
        */
      bool getGLCMCorrelation( unsigned int band, unsigned int pol_index,
        double& correlationValue );             
  
    protected :
    
      /**
        * Concurrence matrix key type( A coord pair where x=first pixel value 
        * and y=second pixel value).
        */    
      typedef std::pair< double, double > GLCMMatrixKeyT;
    
      /** 
        * Concurrence matrix type. 
        * @param GLCMMatrixKeyT A coord pair where x=first pixel value and 
        * y=second pixel value.
        * @param double The pair frequency ( The frequency field must be 
        * double type due the matrix normalizing ).
        */
      typedef std::map< GLCMMatrixKeyT, double > GLCMMatrixT;
      
      /** 
        * Concurrence matrix cache key type.
        */
      typedef std::pair< unsigned int, unsigned int > GLCMMatrixCacheKeyT;
      
      /** Concurrence matrix cache node type. */
      class GLCMMatrixCacheNodeT
      {
        public :
          
          /** GLCM matrix (simetric, normalized between 0 and 1). */
          GLCMMatrixT matrix_;
          
          /** Original minimum value read from the input image.
          (default: DBL_MAX ) */
          double rasterMinGray_;
          
          /** Original maximum value read from the input image. 
          (default: -1.0 * DBL_MAX )*/
          double rasterMaxGray_;
                  
          /** The total number of co-ocurrences inserted into the GLCM matrix.
            (default:0)*/
          unsigned int coOcurrences_;                  
          
          GLCMMatrixCacheNodeT()
          : rasterMinGray_( DBL_MAX ), rasterMaxGray_( -1.0 * DBL_MAX ), 
          coOcurrences_( 0 )
          {};
          
          ~GLCMMatrixCacheNodeT() {};
      };
      
      /** 
        * Concurrence matrix cache type.
        */
      typedef std::map< GLCMMatrixCacheKeyT, GLCMMatrixCacheNodeT > 
        GLCMMatrixCacheT;    
 
      /** @brief Direction mask. */
      DirectionT direction_;
      
      /** @brief Concurrent matrix cache. */
      GLCMMatrixCacheT conc_matrix_cache_;
      
      /** @brief Input raster pointer. */
      TePDITypes::TePDIRasterPtrType input_raster_;
      
      /** @brief Polygon set pointer. */
      TePDITypes::TePDIPolygonSetPtrType polygonset_;
      
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
       * @brief Build a co-ocurrence matrix node using a supplied polygon.
       *
       * @param band Raster band.
       * @param pol_index The polygon index inside the polygon set
       * ( if no polygon set is present, use de default zero value ).
       * @return The generated matrix node pointer o NULL on errors.
       */      
      GLCMMatrixCacheNodeT const* getGLCMMatrixNodePtr( unsigned int band, 
        unsigned int pol_index );
      
      /** Clear all internal allocated resources. */
      void clear();
   
  };

#endif
