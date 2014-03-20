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

#ifndef TEPDIISOSEGCLAS
  #define TEPDIISOSEGCLAS

#include "TePDIAlgorithm.hpp"

#include "TePDIRegion.hpp"
#include "TePDICluster.hpp"

#include <string>
#include <set>

  /**
   * @brief Isoseg classification algorithm.
   * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIClassificationAlgorithms
   *
   * @note The required parameters are:
   * @param input_rasters (TePDITypes::TePDIRasterVectorType) - Input rasters
   * @param bands (vector<int>) - Input bands
   * @param input_polygons (TePolygonSet) - Input polygonset
   * @param acceptance_limiar (double) - Acceptance Limiar (90%, 95%, 99%, 100%)
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - Output raster.
   *
   */

class PDI_DLL TePDIIsosegClas : public TePDIAlgorithm
{
  private:
    /**
    * Generates the Clusters, starting from the Region with higher area
    * inserting into the same cluster other regions whose distance to this 
    * Cluster is lower then the Threshold
    *
    * @return true if everything was done OK
    */
    bool GenerateClusters();

    /**
    * Merge similar clusters
    *
    * @return true if everything was done OK
    */
    bool MergeClusters();

    /**
    * Set the threshold value, based on the ChiTab1
    */
    bool SetThreshold(double t, int Nban);

    /**
    * Limiar of acceptance
    */
    double threshold;

    /**
    * Width and Height of image
    */
    long W, H;

    /**
    * Number of regions present in the PolygonSet
    */
    int total_regions;

    /**
    * Set of regions, ordered by area
    */
    multimap<double, TePDIRegion, greater<double> > regions;

    /**
    * Set of clusters
    */
    vector<TePDICluster> clusters;

  public :

    typedef TeSharedPtr< TePDIIsosegClas > pointer;
    typedef const TeSharedPtr< TePDIIsosegClas > const_pointer;

    /**
    * Default constructor
    *
    * @param factoryName Factory name.
    */
    TePDIIsosegClas();

    /**
    * Default Destructor
    */
    ~TePDIIsosegClas();

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
