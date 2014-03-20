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

#ifndef TEPDIESPECDATA_HPP
  #define TEPDIESPECDATA_HPP

#include "TePDIDefines.hpp"
#include <vector>
#include "../kernel/TeMatrix.h"

/**
  * @class TePDIEspecData
  * @brief Espectral data class.
  * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
  * @ingroup PDIAux
  */
class PDI_DLL TePDIEspecData
{
  private:
    int nban;               // Number of bands
    long npix;              // Number of pixels = area
    std::vector<double> sum;            // Sum of pixels values
    //double* sum3;           // sum**3
    //double* sum4;           // sum**4
    std::vector<double> sumbij;         // sum of the produt of pixel values
                // between bands i and j
    //double* median;         // median value

  public:
    // constructor
    TePDIEspecData();
    // destructor
    ~TePDIEspecData();

    // initializer
    bool Init(int nband);
    // add a pixel
    void Add(double *pixel);
    // returns the covariance at i, j
    float Covar(int i, int j)const;
    // returns the covariance matrix
    TeMatrix Covariance()const;
    // returns the mean at band i
    float Mean(int i)const;
};

#endif
