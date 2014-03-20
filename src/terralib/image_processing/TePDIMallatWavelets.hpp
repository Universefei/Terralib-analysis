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

#ifndef TEPDIMALLATWAVELETS_HPP
  #define TEPDIMALLATWAVELETS_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"

  /**
   * @brief Mallat wavelts algorithm.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIWaveletsAlgorithms
   *
   * @note The general required parameters:
   *
   * @param filter_task ( std::string ) - Task to be 
   * accomplished - see below.
   * @param filters_scale ( double ) - The filters scale.
   * @param a_filter_l ( std::vector< double > ) - Analysis filter, low pass, non
   * normalized ( used in decomposition ).
   * @param a_filter_h ( std::vector< double > ) - Analysis filter, high pass, non
   * normalized ( used in decomposition ).
   * @param s_filter_l ( std::vector< double > ) - Synthesis filter, low pass, non
   * normalized ( used in recomposition ).
   * @param s_filter_h ( std::vector< double > ) - Synthesis filter, high pass, non
   * normalized ( used in recomposition ). 
   *
   * @note The input parameters for filter_task="SBExtract" are:
   * 
   * @param input_image ( TePDITypes::TePDIRasterPtrType ) - Input Image.
   * @param band ( int ) - Input band from input_image.   
   * @param sub_band ( TePDITypes::TePDIRasterPtrType ) - The extracted  
   * sub-band raster reference.
   * @param sub_band_index ( int ) - Sub-band index [ 0, 1, 2, 3 ].
   * @param levels ( int ) - The number of decomposition levels.
   *
   * @note The input parameters for filter_task="SBSwap" are:
   *
   * @param input_image ( TePDITypes::TePDIRasterPtrType ) - Input Image.
   * @param band ( int ) - Input band from input_image.   
   * @param output_image ( TePDITypes::TePDIRasterPtrType ) - Output Image.
   * @param sub_band ( TePDITypes::TePDIRasterPtrType ) - The new  
   * sub-band raster reference.
   * @param sub_band_index ( int ) - Sub-band index [ 0, 1, 2, 3 ].
   * @param levels ( int ) - The number of decomposition levels.
   *
   * @note The input parameters for filter_task="GetPyramid" are:
   * 
   * @param input_image ( TePDITypes::TePDIRasterPtrType ) - Input Image.
   * @param band ( int ) - Input band from input_image.   
   * @param sub_band ( TePDITypes::TePDIRasterPtrType ) - The extracted  
   * sub-band raster reference.
   * @param sub_band_index ( int ) - Sub-band index [ 0, 1, 2, 3 ].
   * @param levels ( int ) - The number of decomposition levels.
   * @param pyramid ( TePDITypes::TePDIRasterVectorPtrType ) - The generated
   * pyramid following the format :
   * [raster0][raster1][raster2][raster3]....[raster( levels*4 - 1 )]
   * where: raster0->sub-band00 of level1, raster1->sub-band01 of level1,
   * raster2->sub-band10 of level1, raster3->sub-band11 of level1,
   * raster4->sub-band00 of level2,  ....., raster(n-1)->sub-band11 of the
   * last level.
   *
   * @note The input parameters for filter_task="RecomposePyramid" are:
   * 
   * @param input_image ( TePDITypes::TePDIRasterPtrType - Optional ):
   * Reference Input Image.
   * @param band ( int - Optional ) - Input band from input_image.   
   * @param pyramid ( TePDITypes::TePDIRasterVectorPtrType ) - The input
   * pyramid following the format:
   * [raster0][raster1][raster2][raster3]....[raster( levels*4 - 1 )]
   * where: raster0->sub-band00 of level1, raster1->sub-band01 of level1,
   * raster2->sub-band10 of level1, raster3->sub-band11 of level1,
   * raster4->sub-band00 of level2,  ....., raster(n-1)->sub-band11 of the
   * last level.
   * @param output_image ( TePDITypes::TePDIRasterPtrType ) - Output Image.
   *
   * @note Each decomposed level will follow the structure ( matricial indexes ):
   * index 0 = [ 0, 0 ] = Low-Low filtered - upper-left .
   * index 1 = [ 0, 1 ] = Low-High filtered - upper-right.
   * index 2 = [ 1, 0 ] = High-Low filtered - bottom-left.
   * index 3 = [ 1, 1 ] = High-High filtered - bottom-right.
   *
   */
  class PDI_DLL TePDIMallatWavelets : public TePDIAlgorithm {
    public :

      /**
       * @brief Default Constructor.
       *
       */
      TePDIMallatWavelets();

      /**
       * @brief Default Destructor
       */
      ~TePDIMallatWavelets();

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

    protected :

      /**
       * @brief Histogram shift flag.
       *
       */
      bool shift_histogram_flag_;
      
      /**
       * @brief The input image mean ( used for histogram shift ).
       *
       */
      double input_image_mean_;
    
      /**
       * @brief Internal pyramidal structure of decomposed bands.
       *
       * @note The index 0 corresponds to the full original input image.
       */
      mutable std::vector< TePDIMatrix< TePDIMatrix< double > > > pyramid_;    
    
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial 
       * state.
       */
      void ResetState( const TePDIParameters& params );
      
      /**
       * @brief Do Wavelet decomposition using input_image and levels number 
       * parameters.
       *
       * @note No output image will be generated by this function. 
       * To decompose and generate output image use Apply()
       * ( using Apply() the generated image will be
       * saved inside the output_image parameter );
       *
       * @param params The current strategy parameters reference.
       * @return true if no erros ocurred.
       */
      bool Decompose( const TePDIParameters& params );
      
      /**
       * @brief Do Wavelet recomposition to output_image parameter and the current
       * internal state.
       *
       * @note If a input_image exists, it will be used to guess the
       * recomposed image dimensions.
       *
       * @param ref_level The reference level used for image reconstruction
       * ( starting from 1 ), this level will be used to reconstruct the
       * first LL sub-band used by the other levels.
       * @param params The current strategy parameters reference.
       * @return true if no erros ocurred.
       */
      bool Recompose( unsigned int ref_level, const TePDIParameters& params );

      /**
       * @brief Returns a copy of one genereted sub-band.
       *
       * @param level Sub-band decomposition level ( starting from 1 ).
       * @param subband Sub-band index [ 0 - top left, 1 - top right, 
       * 2 - bottom left, 3 - bottom right ].
       * @param sb_raster The raster where the sub-band data will be copied.
       * @return true if ok, false on errors.
       */
      bool GetSubBand( unsigned int level, 
        unsigned int subband, 
        const TePDITypes::TePDIRasterPtrType& sb_raster ) const;
        
      /**
       * @brief Change one genereted sub-band by another one.
       *
       * @note Both sub-bands must have the same number of lines and
       * columns.
       *
       * @param level Sub-band decomposition level ( starting from 1 ).
       * @param subband_index Old Sub-band index 
       * [ 0 - top left, 1 - top right, 2 - bottom left, 3 - bottom right ].
       * @param newsband The new sub-band data.
       * @return true if ok, false on errors.
       */        
      bool ChangeSubBand( unsigned int level, unsigned int subband_index,
        const TePDITypes::TePDIRasterPtrType& newsband );
        
        
      /**
       * @brief Push a new level at pyramid level's top.
       *
       * @param level Sub-band level ( starting from 1 ).
       * @param sb0 Sub-band 0 ( top left ) data. 
       * @param sb1 Sub-band 1 ( top right ) data. 
       * @param sb2 Sub-band 2 ( bottom left ) data. 
       * @param sb3 Sub-band 3 ( bottom right ) data. 
       * @return true if ok, false on errors.
       */           
      bool PushLevel( unsigned int level, 
        const TePDIMatrix< double >& sb0,
        const TePDIMatrix< double >& sb1,
        const TePDIMatrix< double >& sb2,
        const TePDIMatrix< double >& sb3 );      
      
      /**
       * @brief Retrive the analysis filters from the internal parameters.
       *
       * @param low_filter The low filter.
       * @param high_filter The high filter.
       * @param low_shift The low filter shift.
       * @param high_shift The high filter shift.
       * @param params The current strategy parameters reference.
       * @return true if ok, false on errors.
       */
      bool GetAnalysisFilters( std::vector< double >& low_filter,
        std::vector< double >& high_filter,
        int& low_shift,
        int& high_shift,
        const TePDIParameters& params ) const;
      
      /**
       * @brief Retrive the synthesis filters from the internal parameters.
       *
       * @param low_filter The low filter.
       * @param high_filter The high filter.
       * @param low_shift The low filter shift.
       * @param high_shift The high filter shift.
       * @param params The current strategy parameters reference.
       * @return true if ok, false on errors.
       */
      bool GetSynthesisFilters( std::vector< double >& low_filter,
        std::vector< double >& high_filter,
        int& low_shift,
        int& high_shift,
        const TePDIParameters& params ) const;        
        
      /**
       * @brief Generates one level of Wavelet decomposition, using the data from
       * the preceding level.
       *
       * @param level Decomposition level ( starting from 1 ).
       * @param params The current strategy parameters reference.
       * @return true if ok, false on errors.
       */
      bool GenerateLevel( unsigned int level, const TePDIParameters& params );
      
      /**
       * @brief Applies a vertical filter.
       *
       * @param inMatrix Input matrix.
       * @param filter Filter to apply.
       * @param filter_shift Filter shift.
       * @param outMatrix Output matrix.
       */
      void ApplyVerticalFilter( 
        const TePDIMatrix< double >& inMatrix,
        const std::vector< double >& filter,
        int filter_shift,
        TePDIMatrix< double >& outMatrix ) const;
        
      /**
       * @brief Applies a horizontal filter.
       *
       * @param inMatrix Input matrix.
       * @param filter Filter to apply.
       * @param filter_shift Filter shift.
       * @param outMatrix Output matrix.
       */
      void ApplyHorizontalFilter( 
        const TePDIMatrix< double >& inMatrix,
        const std::vector< double >& filter,
        int filter_shift, 
        TePDIMatrix< double >& outMatrix ) const;    
        
      /**
       * @brief Recursive build of a simperimposed level ( 
       * a level where the sub-band 0 is a
       * composition of all higher levels ).
       *
       * @param level Decomposition level ( starting from 1 ).
       * @param level_matrix The user pre-initiated resultant level matrix.
       * @param normalize If true, grayscale level normalization will be 
       * performed.
       * @param min_norm_level Minimal grayscale level used for 
       * grayscale range normalization.
       * @param max_norm_level Maximum grayscale level used for 
       * grayscale range normalization.
       * @return true if ok, false on error.
       */
      bool BuildSuperimposedLevel( unsigned int level,
        TePDIMatrix< double >& level_matrix, bool normalize = false,
        double min_norm_level = 0, double max_norm_level = 0 ) const;
      
      /**
       * @brief Build a agregated matrix using 4 sub-bands of identical dimensions.
       *
       * @param upper_left Uppper-left sub-band.
       * @param upper_right Uppper-right sub-band.
       * @param lower_left Lower-left sub-band.
       * @param lower_right Lower-right sub-band.
       * @param result The user pre-initiated result matrix where all data 
       * will be stored.
       * @return true if ok, false on error.
       */          
      bool AgregateSubBands(
        const TePDIMatrix< double >& upper_left,
        const TePDIMatrix< double >& upper_right,
        const TePDIMatrix< double >& lower_left,
        const TePDIMatrix< double >& lower_right,
        TePDIMatrix< double >& result ) const;
        
      /**
       * @brief Allocate space for a new level at pyrimid's top.
       * @param level Level number.
       * @param sblines Sub-bands lines number 
       * ( only required if level == 1 ).
       * @param sbcolumns Sub-bands columns number 
       * ( only required if level == 1 ).
       * @return true if ok, false on error.
       */      
      bool AllocateTopLevel( unsigned int level, unsigned int sblines = 0,
        unsigned int sbcolumns = 0 );
        
      /**
       * @brief Compute the analysis inversed filters reference shift ( offset ).
       *
       * @param a_lowfilter The analysis low filter reference.
       * @param a_highfilter The analysis high filter reference.
       * @param lowshift The computed shift for lowfilter.
       * @param highshift The computed shift for highfilter.
       * @return true if ok, false on errors.
       */      
      bool GetAFiltersShift( 
        const std::vector< double >& a_lowfilter,
        const std::vector< double >& a_highfilter,
        int& lowshift, int& highshift ) const;
        
      /**
       * @brief Compute the syntesis inversed filters reference shift ( offset ).
       *
       * @param s_lowfilter The syntesis low filter reference.
       * @param s_highfilter The syntesis high filter reference.
       * @param a_lowfilter The analysis low filter reference.
       * @param a_highfilter The analysis high filter reference.
       * @param lowshift The computed shift for lowfilter.
       * @param highshift The computed shift for highfilter.
       * @return true if ok, false on errors.
       */      
      bool GetSFiltersShift( 
        const std::vector< double >& a_lowfilter,
        const std::vector< double >& a_highfilter,
        const std::vector< double >& s_lowfilter,
        const std::vector< double >& s_highfilter,
        int& lowshift, int& highshift ) const;        

      /**
       * @brief Reconstruct the Low-low ( 0, 0 ) sub-band.
       *
       * @param sb00_in A reference to the input sub-band ( 0, 0 ).
       * @param sb01_in A reference to the input sub-band ( 0, 1 ).
       * @param sb10_in A reference to the input sub-band ( 1, 0 ).
       * @param sb11_in A reference to the input sub-band ( 1, 1 ).
       * @param s_low_filter Low Synthesis filter.
       * @param s_high_filter High Synthesis filter.
       * @param lowshift Low filter shift.
       * @param highshift High filter shift.
       * @param sb00_out A reference to the generated sub-band ( user 
       * initiated matrix ).
       * @return true if ok, false on errors.
       */                   
      bool ReconstructLL( 
        const TePDIMatrix< double >& sb00_in,
        const TePDIMatrix< double >& sb01_in,
        const TePDIMatrix< double >& sb10_in,
        const TePDIMatrix< double >& sb11_in,
        const std::vector< double >& s_low_filter,
        const std::vector< double >& s_high_filter,
        int lowshift, int highshift,
        TePDIMatrix< double >& sb00_out ) const;
        
      /**
       * @brief Up-samples a matrix.
       *
       * @param in Input matrix.
       * @param out A user initiated output matrix.
       * @return true if ok, false on errors.
       */        
      bool UpSampleMatrix( const TePDIMatrix< double >& in,
        TePDIMatrix< double >& out ) const;
        
      /**
       * @brief Matrix addition.
       *
       * @param in1 Input matrix 1.
       * @param in2 Input matrix 2.
       * @param res Result matrix.
       * @return true if ok, false on errors.
       */        
      bool AddMatrixes( const TePDIMatrix< double >& in1,
        const TePDIMatrix< double >& in2, TePDIMatrix< double >& res ) const;  
        
      /**
       * @brief Applies a reconstruction vertical filter.
       *
       * @param inMatrix A Input matrix filtered with ApplyHorizontalRFilter.
       * @param filter Filter to apply.
       * @param filter_shift Filter shift.
       * @param outMatrix Output matrix.
       * @return true if ok, false on errors.
       */
      bool ApplyVerticalRFilter( 
        const TePDIMatrix< double >& inMatrix,
        const std::vector< double >& filter,
        int filter_shift,
        TePDIMatrix< double >& outMatrix ) const;
        
      /**
       * @brief Applies a reconstruction horizontal filter.
       *
       * @param inMatrix An upsampled Input matrix.
       * @param filter Filter to apply.
       * @param filter_shift Filter shift.
       * @param outMatrix Output matrix.
       * @return true if ok, false on errors.
       */
      bool ApplyHorizontalRFilter( 
        const TePDIMatrix< double >& inMatrix,
        const std::vector< double >& filter,
        int filter_shift,
        TePDIMatrix< double >& outMatrix ) const;    
        
      /**
       * @brief Do matrix normalization using the supplied ranges.
       *
       * @param input The input matrix.
       * @param output The output matrix.
       * @param min The mininum reference value.
       * @param max The maxumum reference value.
       */      
      bool NormalizeMatrix( const TePDIMatrix< double >& input, 
        TePDIMatrix< double >& output, double min, 
        double max ) const;      
  };
  
/** @example TePDIWavelets_test.cpp
 *    Shows how to use this class.
 */    

#endif
