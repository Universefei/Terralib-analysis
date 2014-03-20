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

#ifndef TEPDISAMCLASSIFIER_HPP
  #define TEPDISAMCLASSIFIER_HPP

  #include "TePDIAlgorithm.hpp"
  #include "TePDIMatrix.hpp"
  
  #include "../kernel/TeSharedPtr.h"
  #include "../kernel/TeThread.h"
  #include "../kernel/TeMutex.h"
  
  #include <vector>

  /**
   * @class TePDISAMClassifier
   * @brief Spectral Angle Mapper classification algorithm.
   * @details This algorithm maps the spectral similarity of input raster
   * to the given reference spectra wich can be either laboratory of field spectra.
   * This method assumes that the data have been reduced to apparent reflectance 
   * with all dark current and path radiance biases removed.
   * Reference: The spectral image processing system (SIPS)- interactive 
   * visualization and analysis of imaging spectrometer data. Kruse, F. A.; 
   * Lefkoff, A. B.; Boardman, J. W.; Heidebrecht, K. B.; Shapiro, A. T.; 
   * Barloon, P. J.; Goetz, A. F. H. The earth and space science information 
   * system (ESSIS). AIP Conference Proceedings, Volume 283, pp. 192-201 (1993).
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIClassificationAlgorithms
   *
   * @note The required parameters are:
   *
   * @param input_raster (TePDITypes::TePDIRasterPtrType) - Input raster.
   * @param bands ( std::vector< unsigned int > ) - Bands to process from the 
   * input raster.
   * @param spectral_samples ( TePDISAMClassifier::SpectralSamplesVecPtrT ) -
   * Reference spectral samples of all required classes.
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - Output raster.
   *
   * @note The optional parameters are:
   *
   * @param enable_multi_thread (int) - If present (any value) multi-threaded
   * processing will be used (default:multi-threaded processing disabled).
   * @example TePDIClassification_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDISAMClassifier : public TePDIAlgorithm
  {
  
    public :
    
      /**
       * @class ClassSpectralSamples
       * @brief Class spectral samples.
       */    
      class PDI_DLL ClassSpectralSamples
      {
        public :
        
          /**
           * @brief Class ID. (this ID will be used when generating the output
           * result raster ).
           * @details Only positive values are valid (the default value is zero 
           * wich means invalid class).
           */            
          unsigned int classId_;
          
          /**
           * @brief Maximum angular distance.
           * @details This is the maximum acceptable angle (radians) between 
           * one pixel spectra and the reference spectra (pixels with angular 
           * distance higher than this value will not be classifyed as 
           * belonging to this class (default:0.1 radians).
           */            
          double maxAngularDist_;          
          
          /**
           * @brief Class samples (on sample per line ).
           * @note The number of columns must match the number of bands
           * to process from the input raster.
           */          
          TePDIMatrix< double > samplesMatrix_;
        
          ClassSpectralSamples();
          
          ~ClassSpectralSamples();
      };
      
      /**
       * @brief Samples vector.
       */
      typedef std::vector< ClassSpectralSamples > SpectralSamplesVectorT;
        
      /**
       * @brief Samples vector pointer.
       */        
      typedef TeSharedPtr< SpectralSamplesVectorT > SpectralSamplesVecPtrT;
  
      TePDISAMClassifier();
  
      ~TePDISAMClassifier();
  
      // Overloaded
      bool CheckParameters(const TePDIParameters& parameters) const;
  
    protected :
    
      /**
       * @class ClassReferenceData
       * @brief Class classification reference data.
       */    
      class PDI_DLL ClassReferenceData
      {
        public :
        
          /**
           * @brief Class ID. (this ID will be used when generating the output
           * result raster ).
           * @details Only positive values are valid (the default value is zero 
           * wich means invalid class).
           */            
          unsigned int classId_;
          
          /**
           * @brief Maximum angular distance.
           * @details This is the maximum acceptable angle (radians) between 
           * one pixel spectra and a reference spectra (pixels with angular 
           * distance higher than this value will not be classifyed as 
           * belonging to this class (default:0.1 radians).
           */            
          double maxAngularDist_;          
          
          /**
           * @brief The means vector for the class.
           */          
          std::vector< double > samplesMeansVec_;
        
          ClassReferenceData();
          
          ~ClassReferenceData();
      };    
    
      /**
       * @class SegThread
       * @brief Segmentation thread class
       */    
      class PDI_DLL SegThread : public TeThread
      {
        public :
        
          SegThread( bool progEnabled, TeRaster& inRaster, 
            const std::vector< unsigned int >& inRasterBands,
            const std::vector< ClassReferenceData >& refDataVector,
            TeMutex& inMutex, TeMutex& outMutex, 
            std::vector< bool >& linesProcStatusVec, volatile bool& abortFlag,
            TeRaster& outRaster );
          
          ~SegThread();
          
          inline bool getReturnValue() const
          {
            return returnValue_;
          };
          
          //overload
          void run();          
          
        protected :
        
          /** Progress enabled/disabled flat. */        
          bool progEnabled_;
          
          /** The return value for this thread instance execution
          ( true if ok, false on errors). */
          volatile bool returnValue_;
          
          /** Input raster to process. */          
          TeRaster& inRaster_;
          
          /** Input raster bands to process. */          
          const std::vector< unsigned int >& inRasterBands_;
          
          /** The reference classes data vector. */          
          const std::vector< ClassReferenceData >& refDataVector_;
          
          /** The mutex to use when reading input data. */          
          TeMutex& inMutex_;
          
          /** The mutex to use when writing output data. */          
          TeMutex& outMutex_;
          
          /** The processing status vector of each line. */          
          std::vector< bool >& linesProcStatusVec_;
          
          /** The abort flag to be checked/set when an error occurs. */          
          volatile bool& abortFlag_;
          
          /** The output raster where to save the result. */          
          TeRaster& outRaster_;
      };      
    
      //Overloaded
      void ResetState( const TePDIParameters& );
    
      //Overloaded
      bool RunImplementation();
      
      /**
       * @brief Calc the classes reference data required by the
       * classification process.
       * @param spectralSamples - Input spectral samples.
       * @param refDataVector - The generated output reference data vector.
       */      
      bool calcClassRefData( const SpectralSamplesVectorT& spectralSamples,
        std::vector< ClassReferenceData >& refDataVector ) const;
  };

#endif
