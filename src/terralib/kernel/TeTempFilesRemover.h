/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/*! \file TeTempFilesRemover.h
    \brief Temporary files remover.
*/

#ifndef TETEMPFILEREMOVER_H
  #define TETEMPFILEREMOVER_H
  
  #include "TeSingleton.h"
  #include "TeDefines.h"

  #include <stdio.h>
  #include <string>
  #include <list>

  /*!
   * \class TeTempFilesRemover
   * \brief Hold Temporary files references for file system remotion when 
   * appropriate or when the singleton is destroyed.
   * \author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * \ingroup Utils
   */
  class TL_DLL TeTempFilesRemover : public TeSingleton< TeTempFilesRemover >
  {
    public :
      
      TeTempFilesRemover();
      
      /*! Add a file to be removed.
       * \param fullFileName Full file name.
       */
      void addFile( const std::string& fullFileName );
      
      /*! Add a file to be removed.
       * \param fullFileName Full file name.
       * \param filePtr File pointer.
       * \note The file will be closed just before its deletion.
       */      
      void addFile( const std::string& fullFileName, FILE* filePtr );
      
      /*! Remove the file reference - The file will not be deleted from
       * the file system.
       * \param fullFileName Full file name.
       */      
      void removeFileName( const std::string& fullFileName );
      
      /*! Remove the file reference and delete it from
       * the file system.
       * \param fullFileName Full file name.
       */       
      void removeFile( const std::string& fullFileName );
      
      /*! Remove all files reference and also from the from
       * the file system.
       * \param fullFileName Full file name.
       */        
      void removeAllFiles();
      
      ~TeTempFilesRemover();
      
    protected :
      
      struct ListNode
      {
        FILE* filePtr_;
        std::string fullFileName_;
      };
      
      std::list< ListNode > filesNamesList_;
  };

#endif
