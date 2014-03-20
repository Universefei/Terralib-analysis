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

#ifndef TEPDIREGION_HPP
  #define TEPDIREGION_HPP

#include <vector>
#include <iostream>
#include "../kernel/TeMatrix.h"
#include "TePDIStatistic.hpp"
#include "TePDIEspecData.hpp"
#include "../kernel/TeAgnostic.h"

using namespace std;

  /**
   * @class TePDIRegion
   * @brief This is the class for dealing with image regions.
   * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
   *
   */
class PDI_DLL TePDIRegion
{
  private:
    /**
    * @brief identifier
    */
    long id;

    /**
    * @brief area
    */
    long npix;

    /**
    * @brief class of region
    */
    int region_class;

    /**
    * @brief number of bands
    */
    int nbands;

    /**
    * @brief mean of the pixel values
    */
    vector<double> mean;

    /**
    * @brief statistics of region
    */
    TeMatrix covariance;

  public:
    /**
    * Default Constructor.
    *
    * @param id_region The ID of the region
    */
    TePDIRegion(long id_region)
    { SetId(id_region); nbands = npix = region_class = 0; }

    /**
    * Default Destructor.
    */
    ~TePDIRegion();

    /**
    * Initializes Region
    *
    * @param n number of bands
    * @param a the area
    * @param m mean vector
    * @param c covariance matrix
    */
    bool Init(int n, long a, vector<double> m, TeMatrix c);

    /**
    * Sets the region idtity.
    *
    * @param id_region The ID of the region
    */
    void SetId(long id_region)
    { id = id_region; }

    /**
    * Gets the ID
    */
    long GetId()
    { return id; }

    /**
    * Get the number of bands.
    */
    int GetNban()
    { return nbands; }

    /**
    * Set the class.
    *
    * @param clas The class of the Region
    */
    void SetClass(int clas)
    { region_class = clas; }

    /**
    * Get the class.
    */
    int GetClass()
    { return region_class; }

    /**
    * Returns the mean value
    *
    * @param ind The mean of band 'ind'
    */
    double Mean(int ind);

    /**
    * Returns all the means
    */
    vector<double> GetMean();

    /**
    * Returns the covariance value at position (i, j)
    */
    double Covar(int i, int j);

    /**
    * Returns the covariance matrix
    */
    TeMatrix GetCovar()
    { return covariance; }

    /**
    * Sets the area of the region
    *
    * @param a The new area
    */
    void SetArea(long a)
    { npix = a; }

    /**
    * Returns the area of the region
    */
    long GetArea()
    { return npix; }
};

#endif
