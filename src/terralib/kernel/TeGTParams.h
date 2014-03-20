/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeGTParams.h
    \brief This file contains the definition of parameters necessary to build a TeGeometricTransformation instance.
*/

#ifndef TEGTPARAMS_H
  #define TEGTPARAMS_H
  
  #include "TeMatrix.h"
  #include "TeCoord2D.h"
  #include "TeSharedPtr.h"
  #include "TeDefines.h"

  #include <string>

  /**
   * @brief Geometric transformation parameters.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup GeometricTransformationGroup
   */
  class TL_DLL TeGTParams
  {
    public :
    
      /**
       * @name General parameters.
       * 
       * @brief Used by all transformations..
       */      
    
      /** 
       * @typedef TeSharedPtr< TeGTParams > pointer
       * Type definition for a TeGTParams instance pointer. 
       */
      typedef TeSharedPtr< TeGTParams > pointer;    
      
      /**
       * @enum OutRemStrategy
       * Outliers remotion strategy.
       * @note When using an outliers remotion strategy only the general
       * parameters and the outliers remotion parameters will be considered.
       */ 
      enum OutRemStrategy {
        /**
         * @brief No outliers remotion applied.
         */
        NoOutRemotion,
        
        /**
         * @brief Exaustive outliers remotion (all
         * possible tie-points combinations will be
         * tested.
         */
        ExaustiveOutRemotion,
        
        /**
         * @brief Iteractive leave-worse-out) will remotion
         * be performed.
         * @note This method will use the optional parameter 
         * tiePointsWeights_ if it is available.
         */
        LWOutRemotion,
        
        /**
         * @brief Random Sample Consensus based outliers remotion
         * will be applied.
         * @note This method will use the optional parameter 
         * tiePointsWeights_ if it is available.
         * @note Reference: Martin A. Fischler and Robert C. Bolles,
         * Random Sample Consensus: A Paradigm for Model Fitting with 
         * Applications to Image Analysis and Automated Cartography,
         * Communications of the ACM  archive, Volume 24 ,  
         * Issue 6  (June 1981)
         */
        RANSACRemotion        
      };     
    
      /**
       * @brief Transformation name.
       *
       * @note Default value = affine.
       */
      std::string transformation_name_;
      
      /** 
       * @brief The outliers remotion strategy.  
       *
       * @note Default value = NoOutRemotion.    
       */
      OutRemStrategy out_rem_strat_;
    
      /**
       * @brief The current direct mapping transformation parameters.
       * @note Default value = An empty matrix.    
       */    
      TeMatrix direct_parameters_;
      
      /**
       * @brief The current inverse mapping transformation parameters.
       * @note Default value = An empty matrix.    
       */    
      TeMatrix inverse_parameters_;      

      /**
       * @brief The current transformation tie-points.
       * @note Default value = An empty vector.    
       */    
      std::vector< TeCoordPair > tiepoints_; 
      
      /**
       * @brief Enable/disable multi-thread processing (when
       * available (default=true).
       */    
      bool enable_multi_thread_;     
        
      /**
       * @name Outliers remotion strategy used parameters.
       * 
       * @brief Used when an outlier remotion strategy (other than
       * NoOutRemotion) is used.
       */        
      
      /**
       * @brief The maximum allowed direct mapping error.
       *
       * @note Default value = Positive infinite.    
       */         
      double max_dmap_error_;
      
      /**
       * @brief The maximum allowed inverse mapping error.
       *
       * @note Default value = Positive infinite.    
       */         
      double max_imap_error_;   

      /**
       * @brief The maximum allowed direct mapping mean square error.
       *
       * @note Default value = Positive infinite.    
       */         
      double max_dmap_rmse_;
      
      /**
       * @brief The maximum allowed inverse mapping mean square error.
       *
       * @note Default value = Positive infinite.    
       */         
      double max_imap_rmse_;    
           
      /**
       * @brief The current transformation tie-points weights.
       * @note Only positive values are allowed.
       */    
      std::vector< double > tiePointsWeights_;            
      
      /**
       * @name Projective transformation parameters.
       * @brief Projective transformation parameters.
       */             
      
      /**
       * @brief The weight matrix used by least square method.
       *
       * @note Default value = empty matrix.
       */         
      TeMatrix WMatrix_;
      
      /**
       * @brief The maximun number of iterations while processing.
       *
       * @note Default value = 20.
       */         
      unsigned int maxIters_;
      
      /**
       * @brief The tolerance used while executing an iterative processing.
       *
       * @note Default value = 0.0001.
       */         
      double tolerance_;
      
      /**
       * @brief Indicates whether use adaptive parameters or not.
       *
       * @note Default value = false.
       */         
      bool useAdaptiveParams_;    
      
      /**
       * @brief Adaptive parameter.
       *
       * @note Default value = 0.
       */       
      double deltaX_;
      
      /**
       * @brief Adaptive parameter.
       *
       * @note Default value = 0.
       */       
      double deltaY_;
      
      /**
       * @brief Adaptive parameter.
       *
       * @note Default value = 0.
       */       
      double deltaU_;
      
      /**
       * @brief Adaptive parameter.
       *
       * @note Default value = 0.
       */       
      double deltaV_;   
      
      /**
       * @name Class methods.
       * @brief Class methods.
       */          

      /**
       * @brief Default constructor
       */
      TeGTParams();
      
      /**
       * @brief Alternative constructor
       * @param external External reference.
       */
      TeGTParams( const TeGTParams& external );      
      
      /**
       * @brief Default Destructor
       */
      ~TeGTParams();
      
      /**
       * @brief operator= implementation.
       *
       * @param external External reference.
       * @return A const reference to the external object instance.
       */
      const TeGTParams& operator=( 
        const TeGTParams& external );      

      /**
       * @brief This is for TeFactory compatibility.
       */
      std::string decName() const;
      
      /**
       * @brief Reset to the default parameters.
       */
      void reset();      
      
      // Overload
      bool operator==( const TeGTParams& external ) const;
            
  };

#endif
