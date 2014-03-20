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

#ifndef TEPDIRADARFILTER_HPP
  #define TEPDIRADARFILTER_HPP

  #include "TePDIBufferedFilter.hpp"
  #include "TePDIParameters.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
   * @class TePDIRadarFilter
   * @brief This is the base class for radar filters.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIFiltersGroup
   */
  class PDI_DLL TePDIRadarFilter : public TePDIBufferedFilter {
    public :
      /** @typedef Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIRadarFilter > pointer;
      /** @typedef Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIRadarFilter > const_pointer;

      /**
       * @brief Default Destructor
       */
      virtual ~TePDIRadarFilter();

    protected :
      /**
       * @brief The maximum mask width for adaptative filters
       */
      unsigned int max_adapt_mask_width_;

      /**
       * @brief Default Constructor.
       *
       */
      TePDIRadarFilter();

      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      virtual void ResetState( const TePDIParameters& params );

  };

#endif //TEPDIRADARFILTER_HPP
