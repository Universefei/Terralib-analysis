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

#ifndef TEPDIGARGUETMIXMODELSTRATEGY_HPP
#define TEPDIGARGUETMIXMODELSTRATEGY_HPP

#include "TePDIStrategyFactory.hpp"
#include "TePDIAlgorithmFactory.hpp"
#include "TePDIUtils.hpp"
#include "../kernel/TeAgnostic.h"
#include "../kernel/TeRasterParams.h"
#include "../kernel/TeRaster.h"
#include "../kernel/TeSharedPtr.h"
#include "TePDIMixModelStrategy.hpp"
#include "TePDIParameters.hpp"
#include "TePDIMixModelComponentList.hpp"
#include "TePDIMixModelSpectralBandList.hpp"

#include <string>

/**
* @brief This is the class for princo mixmodel strategy.
* @author Felipe Castro da Silva <tkorting@dpi.inpe.br>
* @ingroup PDIStrategies
*
* @note The required parameters are:
* @param mixmodel_type (std::string): Must be "princo", "mqp" or "mpr".
* @param component_list (TePDIMixModelComponentList): components list.
* @param spectral_band_list (TePDIMixModelSpectralBandList): spectral bands list.
* @param input_rasters (TePDITypes::TePDIRasterVectorType): input rasters.
* @param bands (std::vector<int>): band number of each input raster.
* @param output_rasters (TePDITypes::TePDIRasterVectorType): output rasters (ratio images).
* @param compute_error_rasters (int): if 1, the algorithm compute the error rasters, if 0, not.
* @param output_error_rasters (TePDITypes::TePDIRasterVectorType): if compute_error_rasters, here we have to set output error rasters.
* @param compute_average_error (int): if 1, the algorithm compute the average error, if 0, not.
* @param average_error (double): if compute_average_error, here we have the average error value.
* @param normalize (double): if 1, the output images are normalized, if 0, not.
*/

class PDI_DLL TePDIPrincoMixModelStrategy : public TePDIMixModelStrategy
{
 	friend class TePDIPrincoMixModelSF;
	public :
 		typedef TeSharedPtr< TePDIPrincoMixModelStrategy > pointer;
 		typedef const TeSharedPtr< TePDIPrincoMixModelStrategy > const_pointer;
/**Default destructor.*/
		~TePDIPrincoMixModelStrategy();
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
* Default constructor
*
* @param factoryName Factory name.
*/
		TePDIPrincoMixModelStrategy();
/**
* Runs the algorithm implementation.
*
* @return true if OK. false on errors.
*/
		bool Implementation(const TePDIParameters& params);
};

#endif
