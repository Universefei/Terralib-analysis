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

#ifndef TEPDIEMCLAS_HPP
  #define TEPDIEMCLAS_HPP

#include "TePDIAlgorithm.hpp"

#include "TePDIRegion.hpp"
#include "TePDICluster.hpp"

#include <string>
#include <set>

  /**
   * @brief EM classification algorithm.
   * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIClassificationAlgorithms
   *
   * @note The required parameters are:
   *
   * @param input_rasters (TePDITypes::TePDIRasterVectorType) - Input rasters
   * @param bands (vector<int>) - Input bands from each raster.
   * @param classes_to_find (int) - The number of classes to find.
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - Output raster.
   *
   * @note The optional parameters are:
   *
   * @param sx (int) - Sampling Rate X (default: 1).
   * @param sy (int) - Sampling Rate Y (default: 1).
   * @param max_iterations (int) - The maximum number of iterations (default: 3).
   * @param epsilon (double) - stop criteria (default: 15).
   * @param shift_threshold (double) - minimum distance between clusters means (default: 40).
   * @param input_means (TeMatrix) - initial Matrix of means for each cluster [bands x clusters]
   */

  class PDI_DLL TePDIEMClas : public TePDIAlgorithm
  {
    private:
      /**
      * Width, Height of image
      */
      long W, H;
  
      /**
      * Total Instances and Total Attributes
      */
      int N, TOTAL_ATTRIBUTES;
  
      /**
      * Total Classes to find
      */
      int M;
  
      /**
      * Sampling Rate, in X and Y directions
      */
      int SX, SY;
  
      /**
      * Limit of iterations
      */
      int MAX_ITERATIONS;
  
      /**
      * Stop Condition
      */
      double EPSILON;

      /**
      * Threshold for shift
      */
      double SHIFT_THRESHOLD;
  
    public :
  
      typedef TeSharedPtr< TePDIEMClas > pointer;
      typedef const TeSharedPtr< TePDIEMClas > const_pointer;
  
      /**
      * Default constructor
      *
      * @param factoryName Factory name.
      */
      TePDIEMClas();
  
      /**
      * Default Destructor
      */
      ~TePDIEMClas();
  
      /**
      * Checks if the supplied parameters fits the requirements of each
      * PDI strategy.
      *
      * Error log messages must be generated. No exceptions generated.
      *
      * @param parameters The parameters to be checked.
      * @return true if the parameters are OK. false if not.
      */
      bool CheckParameters(const TePDIParameters& parameters) const;
  
    protected :
  
    /**
    * @brief Reset the internal state to the initial state.
    *
    * @param params The new parameters referent at initial state.
    */
    void ResetState( const TePDIParameters& );
  
    /**
    * Runs the algorithm implementation.
    *
    * @return true if OK. false on errors.
    */
    bool RunImplementation();
  };

#endif
