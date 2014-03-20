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

#ifndef TEPDIMIXMODELSPECTRALBANDLIST_HPP
#define TEPDIMIXMODELSPECTRALBANDLIST_HPP

#include <string>
#include <vector>

#include "../kernel/TeAgnostic.h"
#include "TePDIMixModelSpectralBand.hpp"

using namespace std;

/**
* @class TePDIMixModelSpectralBandList
* @brief This is the class contains the list of spectral bands for mixmodel algorithms.
* @author Felipe Castro da Silva <felipe@dpi.inpe.br>
* @ingroup PDIMixModel
*/
class PDI_DLL TePDIMixModelSpectralBandList
{
	public:
/**Default constructor.*/
		TePDIMixModelSpectralBandList();
/**Default destructor.*/
		~TePDIMixModelSpectralBandList();
/**
* Insert spectral band.
* @param sb spectral band to be stored.
* @return true if the spectral band was inserted, false if not.
*/
		bool insertSpectralBand(TePDIMixModelSpectralBand sb);
/**
* Load spectral band file.
* @note This file have the following format: sensor_number band_number lower_spectral_range_bound upper_spectral_range_bound spectral_band_name (e.g. 160 1 0.450000 0.520000 Landsat5-TM1)
* @param filename file name with spectral bands information.
* @return true if the file was loaded sucessfully, false if not.
*/
		bool loadSpectralBandsFromFile(string filename);
/**
* Return spectral band list size.
* @return return the spectralbandlist size.
*/
		unsigned int getSize();
/**
* Return spectral band name.
* @param b band number.
* @return return the spectral band name.
*/
		string getSpectralBandLabel(unsigned int b);
/**
* Return spectral band.
* @param b band number.
* @return return the spectral band.
*/
		TePDIMixModelSpectralBand getSpectralBand(unsigned int b);
    
    // overload
    bool operator==( const TePDIMixModelSpectralBandList& external ) const;
        
	private:
/**Store the spectral bands*/
		vector<TePDIMixModelSpectralBand> bands;
};
#endif
