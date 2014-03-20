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

#ifndef TEPDIKMEANSCLAS_HPP
  #define TEPDIKMEANSCLAS_HPP

#include "TePDIAlgorithm.hpp"

#include "TePDIRegion.hpp"
#include "TePDICluster.hpp"

#include <string>
#include <set>

  /**
   * @brief Kmeans classification algorithm.
   * @author Eric Silva Abreu <eric@dpi.inpe.br>
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIClassificationAlgorithms
   *
   * @note The required parameters are:
   * @param input_raster (TePDITypes::TePDIRasterPtrType) - Input raster.
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - Output raster.
   * @param bands (vector<int>) - input bands.
   * @param classes_number (int) - number of classes.
   *
   * @note The optional parameters are:
   *
   * @param sample (int) - ???? (default 1).
   * @param iterations_number (int) - number of iterations (default 10).
   * @param line_begin (int) - initial line to do classfication (default 0).
   * @param line_end (int) - end line to do classfication(default: the last raster line).
   * @param column_begin (int) - initial column to do classfication (default 0).
   * @param column_end (int) - end column to do classfication (default : the last raster column).   
   */

class PDI_DLL TePDIKMeansClas : public TePDIAlgorithm
{
  private :

  /**
    * @brief Generate classificatin paramters.
    */   
  bool GenerateClassificationParameters();
  
  /**
    * @brief Start the clusters classifying process.
    */    
  void classifySamples();
  
  /**
    * @brief Start the clusters splitting process.
    */    
  bool split();
  
  /**
    * @brief Start the kmeans algorithm.
    */  
  bool kmeans();

  /**
    * @brief Classify pixels.
    *
    * @param l1 Starting line.
    * @param l2 Ending line.
    * @param c1 Starting column.
    * @param c2 Ending column.
    * @return True if the pixel was classified correctly and false in other case.
    */
  bool classifyPixels( int l1, int l2, int c1, int c2);

  /**
    * Number of classes
    */
    int n_Class;

  /**
    * class count
    */
    int class_count;

  /**
    * Number of bands
    */
    int n_Bands;

  /**
    * Number of iterations
    */
    int n_Iter;

  /**
    * Sample
    */
    int f_Samp;

  /**
    * initial line to do classfication
    */
    int initial_line;

  /**
    * end line to do classfication
    */
    int end_line;

  /**
    * initial column to do classfication
    */
    int initial_column;

  /**
    * end column to do classfication
    */
    int end_column;

  /**
    * pixel value of each input raster
    */
  vector<int> bands;

  /**
    * Set of clusters
    */
    vector<TePDICluster*> clusters;

  /**
    * pixel value of each input raster
    */
  vector<double> tuple;

  /**
    * input rasters
    */
  TePDITypes::TePDIRasterPtrType input_raster;

  /**
    * output raster
    */
  TePDITypes::TePDIRasterPtrType output_raster;

  public :

    typedef TeSharedPtr< TePDIKMeansClas > pointer;
    typedef const TeSharedPtr< TePDIKMeansClas > const_pointer;

    /**
    * Default constructor
    *
    * @param factoryName Factory name.
    */
    TePDIKMeansClas();

    /**
    * Default Destructor
    */
    ~TePDIKMeansClas();

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
  * @brief Runs the current algorithm implementation.
  *
  * @return true if OK. false on error.
  */
  bool RunImplementation();
};

#endif
