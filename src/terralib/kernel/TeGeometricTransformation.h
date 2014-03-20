/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeGeometricTransformation.h
   \brief Geometric trasformations base class.
*/

#ifndef TEGEOMETRICTRANSFORMATION_H
  #define TEGEOMETRICTRANSFORMATION_H
  
  #include "TeDefines.h"
  #include "TeCoord2D.h"
  #include "TeMatrix.h"
  #include "TeGTParams.h"
  #include "TeSharedPtr.h"
  #include "TeAgnostic.h"
  #include "TeThreadParameters.h"
  
  #include <vector>
  #include <list>
  
  /**
   * @brief This is the base class to deal with a geometric trasformation
   * direct and inverse mapping the tie-points TeCoordPair::pt1 space into 
   * TeCoordPair::pt2 space.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup GeometricTransformationGroup
   */
  class TL_DLL TeGeometricTransformation {
    private :
    
      /**
       * @brief The current internal parameters.
       */
      TeGTParams internal_params_;
          
    protected :
    
      /**
       * @brief Verifies if the supplied parameters already
       * has a defined transformation.
       * @param params Transformation parameters.
       * @return true if a transformation is already defined, 
       * false otherwise.
       */       
      virtual bool isTransDefined( const TeGTParams& params ) 
        const = 0;  
            
    public :

      /** 
       * @typedef TeSharedPtr< TeGeometricTransformation > pointer
       * Type definition for a TeGeometricTransformation instance pointer. 
       */
      typedef TeSharedPtr< TeGeometricTransformation > pointer;
      
      /**
       * @brief Default Destructor
       */
      virtual ~TeGeometricTransformation();
      
      /**
       * @brief Direct mapping ( from pt1 space into pt2 space ).
       *
       * @param pt1 pt1 coordinate.
       * @param pt2 pt2 coordinate.
       */      
      inline void directMap( const TeCoord2D& pt1, TeCoord2D& pt2 ) const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
        
        directMap( internal_params_, pt1, pt2 );
      };
      
      /**
       * @brief Inverse mapping ( from pt2 space into pt1 space ).
       *
       * @param pt2 pt2 coordinate.
       * @param pt1 pt1 coordinate.
       */      
      inline void inverseMap( const TeCoord2D& pt2, 
        TeCoord2D& pt1 ) const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )         
      
        inverseMap( internal_params_, pt2, pt1 );
      };
      
      /**
       * @brief Reset the current transformation following the new supplied 
       * parameters.
       *
       * @param newparams The new parameters.
       * @return true if OK, false on errors.
       */      
      bool reset( const TeGTParams& newparams );
      
      /**
       * @brief Returns a copy of the current internal transformation parameters.
       *
       * @param params The internal parameters copy.
       */        
      void getParameters( TeGTParams& params ) const;
      
      /**
       * @brief Returns a reference of the current internal transformation parameters.
       *
       * @return A reference of the current internal transformation parameters..
       */        
      const TeGTParams& getParameters() const;      
      
      /**
       * @brief Calculates the current transformation maximum direct mapping 
       * error ( from pt1 space into pt2 space ).
       *
       * @return The current maximum direct mapping error.
       */        
      inline double getDirectMappingError() const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
        
        return getDirectMappingError( internal_params_ );
      };
      
      /**
       * @brief Calculates the current transformation maximum inverse mapping 
       * error ( from pt2 space into pt1 space ).
       *
       * @return The current maximum inverse mapping error.
       */        
      inline double getInverseMappingError() const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
        
        return getInverseMappingError( internal_params_ );
      };     
      
      /**
       * @brief Calculates root mean square direct mapping error 
       * ( from pt1 space into pt2 space ).
       *
       * @return The root mean square error.
       */        
      inline double getDMapRMSE() const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
        
        return getDMapRMSE( internal_params_ );
      };        
      
      /**
       * @brief Calculates root mean square inverse mapping error
       * ( from pt2 space into pt1 space ).
       *
       * @return The root mean square error.
       */        
      inline double getIMapRMSE() const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
        
        return getIMapRMSE( internal_params_ );
      };
      
      /**
       * @brief Calculates the direct mapping error for the supplied tie-point
       * ( from pt1 space into pt2 space ).
       *
       * @param tie_point The tie-point.
       * @return The current direct mapping error.
       */        
      inline double getDirectMappingError( const TeCoordPair& tie_point ) const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
      
        return getDirectMappingError( tie_point, internal_params_ );      
      };
      
      /**
       * @brief Calculates the inverse mapping error for the supplied tie-point
       * ( from pt2 space into pt1 space ).
       *
       * @param tie_point The tie-point.
       * @return The current inverse mapping error.
       */        
      inline double getInverseMappingError( const TeCoordPair& tie_point ) const
      {
        TEAGN_DEBUG_CONDITION( isTransDefined( internal_params_ ),
          "Transformation not defined" )
      
        return getInverseMappingError( tie_point, internal_params_ );      
      };      
      
      /**
       * @brief Returns the minimum number of required tie-points for the current
       * transformation.
       *
       * @return The minimum number of required tie-points for the current
       * transformation.
       */       
      virtual unsigned int getMinRequiredTiePoints() const = 0;      
      
      /**
       * @brief Returns a default object.
       *
       * @return A default object.
       */
      static TeGeometricTransformation* DefaultObject( 
        const TeGTParams& );

    protected :
    
      /**
       * @brief Default Constructor.
       */
      TeGeometricTransformation();
      
      /**
       * @brief Direct mapping ( from pt1 space into pt2 space ).
       *
       * @param params Transformation parameters.
       * @param pt1 pt1 coordinate.
       * @param pt2 pt2 coordinate.
       */      
      virtual void directMap( const TeGTParams& params,
        const TeCoord2D& pt1, TeCoord2D& pt2 ) const = 0;
      
      /**
       * @brief Inverse mapping ( from pt2 space into pt1 space ).
       *
       * @param params Transformation parameters.
       * @param pt2 pt2 coordinate.
       * @param pt1 pt1 coordinate.
       */      
      virtual void inverseMap( const TeGTParams& params,
        const TeCoord2D& pt2, 
        TeCoord2D& pt1 ) const = 0;      
      
      /**
       * @brief Calculate the transformation parameters following the
       * new supplied tie-points.
       *
       * @param params Transformation parameters.
       * @return true if OK, false on errors.
       */       
      virtual bool computeParameters( TeGTParams& params ) 
        const = 0;
        
      /**
       * @brief Calculates maximum direct mapping error for the supplied 
       * parameters ( from pt1 space into pt2 space ).
       *
       * @param params Transformation parameters.       
       * @return The maximum direct mapping error for the supplied parameters.
       */        
      double getDirectMappingError( const TeGTParams& params ) const;        
      
      /**
       * @brief Calculates maximum inverse mapping error for the supplied 
       * parameters ( from pt2 space into pt1 space ).
       *
       * @param params Transformation parameters.       
       * @return The maximum inverse mapping error for the supplied parameters.
       */        
      double getInverseMappingError( const TeGTParams& params ) const;      
      
      /**
       * @brief Calculates root mean square direct mapping error for the supplied 
       * parameters ( from pt1 space into pt2 space ).
       *
       * @param params Transformation parameters.       
       * @return The root mean square error.
       */        
      double getDMapRMSE( const TeGTParams& params ) const;        
      
      /**
       * @brief Calculates root mean square inverse mapping error for the supplied 
       * parameters ( from pt2 space into pt1 space ).
       *
       * @param params Transformation parameters.       
       * @return The root mean square error.
       */        
      double getIMapRMSE( const TeGTParams& params ) const;       
        
      /**
       * @brief Calculates the direct mapping error for the supplied tie-point.
       * ( from pt1 space into pt2 space ).
       *
       * @param tie_point The tie-point.
       * @param params Transformation parameters.        
       * @return The current maximum direct mapping error.
       */        
      double getDirectMappingError( const TeCoordPair& tie_point,
        const TeGTParams& params ) const;

      /**
       * @brief Calculates the inverse mapping error for the supplied tie-point.
       * ( from pt2 space into pt1 space ).
       *
       * @param tie_point The tie-point.
       * @param params Transformation parameters.        
       * @return The current maximum inverse mapping error.
       */        
      double getInverseMappingError( const TeCoordPair& tie_point,
        const TeGTParams& params ) const;
                
    private :
    
      /**
       * @brief RANSAC iterations counter type.
       */        
      typedef unsigned int RansacItCounterT;
    
      /**
       * @brief Excluded tie-points data.
       */    
      class ExcTPDataNode
      {
        public :
          TeCoordPair tp_;
          /*! Tie point original weight (from user parameters) */
          double tp_weight_;
          
          double dmap_error_;
          double imap_error_;
          
          /*! Tie point error */
          double tp_error_;   
          
          ExcTPDataNode()
          {
            tp_weight_ = dmap_error_ = imap_error_ = tp_error_ = 0.0;
          }     
          
          ExcTPDataNode( const ExcTPDataNode& other )
          {
            tp_ = other.tp_;
            tp_weight_ = other.tp_weight_;
            dmap_error_ = other.dmap_error_;
            imap_error_ = other.imap_error_;
            tp_error_ = other.tp_error_;
          }
          
          const ExcTPDataNode& operator=( const ExcTPDataNode& other )
          {
            tp_ = other.tp_;
            tp_weight_ = other.tp_weight_;
            dmap_error_ = other.dmap_error_;
            imap_error_ = other.imap_error_;
            tp_error_ = other.tp_error_;
            
            return other;
          }
          
          bool operator< ( const ExcTPDataNode& other ) const
          {
            return ( ( tp_error_ < other.tp_error_ ) ? true : false );
          }
      };
      
      /**
       * @brief Tie-points data.
       */    
      class TPDataNode
      {
        public :
          unsigned int tpindex_;
          double dmap_error_;
          double imap_error_;
          
          /*! Tie point error */
          double tp_error_;
          
          TPDataNode()
          {
            dmap_error_ = imap_error_ = tp_error_ = 0.0;
            tpindex_ = 0;
          }     
          
          TPDataNode( const TPDataNode& other )
          {
            tpindex_ = other.tpindex_;
            dmap_error_ = other.dmap_error_;
            imap_error_ = other.imap_error_;
            tp_error_ = other.tp_error_;
          }
          
          const TPDataNode& operator=( const TPDataNode& other )
          {
            tpindex_ = other.tpindex_;
            dmap_error_ = other.dmap_error_;
            imap_error_ = other.imap_error_;
            tp_error_ = other.tp_error_;
            
            return other;
          }
                    
          bool operator< ( const TPDataNode& other ) const
          {
            return ( ( tp_error_ < other.tp_error_ ) ? true : false );
          }
      };      
    
      /**
       * @brief Operator= overload.
       *
       * @param external External instance reference.
       *
       * @return The external instance reference.
       */        
      const TeGeometricTransformation& operator=( 
        const TeGeometricTransformation& ) { return *this; };
        
      /**
       * @brief Recombine a seed vector without repetition (the number of combined 
       * elements on each iteration will follow the vectors size.
       *
       * @param seed Seed vector.
       * @param elements_nmb Number of elements to be permutated
       * @param seedpos Location inside the seed vector where to begin
       * the permutation.
       * @return TRUE if a permutation was made, FALSE if no more
       * permutation are left to made.
       *
       * @note All initial fields must be set to value 0 for the initial seed.
      */
      static bool recombineSeed( std::vector<unsigned int>& seed, 
        const unsigned int& seedpos, const unsigned int& elements_nmb );        
        
      /**
       * @brief Multi-thread exaustive outliers remotion 
       * strategy (All tie-points
       * combinations will be tested).
       * @param params The current transformation parameters.
       * @param threads_nmb Threads number (not including the
       * curent process).
       * @return TRUE if OK, false on errors.
      */        
      bool exaustiveOutRemotion( TeGTParams& params,
        unsigned int threads_nmb );      
        
      /**
       * @brief Exaustive outliers remotion thread entry. 
       * @param params Thread parameters.
       * @return TRUE if OK, false on errors.
      */        
      static bool eORThreadEntry( const TeThreadParameters& 
        params );          
      
      /**
       * @brief Leave-worse-out outliers remotion strategy 
       * (On each iteration the worse tie-point will be
       * removed).
       * @param params The current transformation parameters.
       * @return TRUE if OK, false on errors.
      */        
      bool lWOutRemotion( TeGTParams& params );   
      
      /**
       * @brief Update the excluded tie-points errors following
       * the supplied transformation parameters.
       * @param params The current transformation parameters.
       * @param useTPWeights Use the tie-points error vector.
       * @param exc_tp_list The excluded tie-points list (the first element
       * has the lower normalized error.
      */       
      void updateExcTPErrList( const TeGTParams& params,
        bool useTPWeights,
        std::list< ExcTPDataNode >& exc_tp_list ) const;
        
      /**
       * @brief Recalc the tie-points errors vector
       * the supplied transformation parameters.
       * @param params The current transformation parameters.
       * @param useTPWeights Use the tie-points error vector.
       * @param errvec The tie-points vector (the first element
       * has the lower normalized error).
      */       
      void updateTPErrVec( const TeGTParams& params,
        bool useTPWeights,
        std::vector< TPDataNode >& errvec ) const;   
             
      /**
       * @brief Random Sample Consensus based outliers remotion.
       * @param inputParams The current transformation parameters.
       * @param outputParams The output transformation parameters.
       * @return TRUE if OK, false on errors.
      */                 
      bool ransacRemotion( const TeGTParams& inputParams,
        TeGTParams& outputParams ) const;
      
             
      /**
       * @brief Normalize the tie-points weights in such a way
       * that the sum of all of then is 1.
       * @param tpWeights The tie-points weights.
       * @return true if OK, false on errors.
      */                 
      bool normalizeTPWeights( std::vector< double >& tpWeights ) const;
      
      double getPt1ConvexHullArea( const TeCoordPairVect& tiePoints ) const;

  };
  
#endif

