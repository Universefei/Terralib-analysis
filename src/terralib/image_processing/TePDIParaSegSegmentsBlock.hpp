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

#ifndef TEPDIPARASEGSEGMENTSLOCK_HPP
  #define TEPDIPARASEGSEGMENTSLOCK_HPP

  #include "TePDIDefines.hpp"
  #include "TePDIParaSegSegment.hpp"
  #include "TePDIMatrix.hpp"

  /*!
      \class TePDIParaSegSegmentsBlock
      \brief Multi-threaded image segmenter segments block description class.
      \author Emiliano F. Castejon <castejon@dpi.inpe.br>.
      \note All segments will be deleted when this instance is deleted.
      \ingroup PDIAux
  */
  class PDI_DLL TePDIParaSegSegmentsBlock
  {
    public :
    
      /*! Raster data block (one band/channel)*/
      typedef TePDIMatrix< TePDIParaSegSegment* > SegmentsPointersMatrixT;          
      
      /*! Block status flag. */
      enum BlockStatus
      {
        /*! This block was not processed yet */
        BlockNotProcessed = 0,
        /*! This block is under segmentation */
        BlockUnderSegmentation = 1,        
        /*! This block already was segmented */
        BlockSegmented = 2,
        /*! This block already was alread merged */
        BlockMerged = 3,
        /*! This block already was flushed to the output raster */
        BlockFlushed = 4
      };    
      
      /*! Block status - default:BlockNotProcessed*/
      unsigned int status_;      

      /*! Block uppler left X over input full image - default:UINT_MAX*/
      unsigned int startX_;
     
      /*! Block uppler left Y over input full image - default:UINT_MAX*/
      unsigned int startY_;
      
      /*! Block width - default:0*/
      unsigned int width_;
     
      /*! Block height - default:0*/
      unsigned int height_;      
      
      /*! 
        \brief Segments pointers matrix - default: empty matrix
        \note The pointed pointed segments will not be deleted by this class.
        \note The default memory policy is RAMMemPol.
      */
      SegmentsPointersMatrixT segsPtrsMatrix_;
      
      TePDIParaSegSegmentsBlock();

      ~TePDIParaSegSegmentsBlock();   
         
      /*! Clear all internal data including all segments objects */   
      void clear();
  };


#endif
