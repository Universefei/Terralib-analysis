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

#ifndef TEPDIRASTERRUBUFFER_HPP
  #define TEPDIRASTERRUBUFFER_HPP

  #include "TePDIDefines.hpp"
  #include "../kernel/TeRaster.h"
  
  /**
   * @class TePDIRasterRUBuffer
   * @brief A raster rool-up lines buffer.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup PDIAux
   */
  class PDI_DLL TePDIRasterRUBuffer {
    public :
      
      TePDIRasterRUBuffer();

      ~TePDIRasterRUBuffer();
      
      /**
       * @brief Reset this roll up buffer instance.
       *
       * @param inputRaster Input raster.
       * @param inRasterChn Input raster channel/band.
       * @param bufferLines Number of roll-up buffer lines.
       * @return true if OK, false on errors.
       * @note No lines will be pre-loaded.
       *
       */
      bool reset( TeRaster& inputRaster, unsigned int inRasterChn, 
        unsigned int bufferLines );
      
      /**
       * @brief Buffer roll-up (read the next line from input raster)
       * @return true if OK, false on errors.
       */      
      bool roolUp();
      
      /**
       * @brief Returns a pointer to the current buffer.
       * @return A pointer to the current buffer.
       * @note The returned pointer will be invalidated if reset or roolUp
       * methods were called.
       */         
      double** getBufferPtr();        
 
    protected :
      
      TeRaster* rasterPtr_;
      
      unsigned int rasterChn_;
    
      double** bufferPtr_;
      
      unsigned int bufferLines_;
      
      unsigned int bufferCols_;
      
      unsigned int lastReadedLine_;
      
      void clear();
  };

#endif
