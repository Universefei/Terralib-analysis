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

#include "TeTempFilesRemover.h"
#include "TeAgnostic.h"


TeTempFilesRemover::TeTempFilesRemover()
{
}


TeTempFilesRemover::~TeTempFilesRemover()
{
  removeAllFiles();
}

void TeTempFilesRemover::addFile( const std::string& fullFileName )
{
  if( ! fullFileName.empty() )
  {
    ListNode newNode;
    newNode.filePtr_ = 0;
    newNode.fullFileName_ = fullFileName;
    
    filesNamesList_.push_back( newNode );
  }
}
      
void TeTempFilesRemover::addFile( const std::string& fullFileName, 
  FILE* filePtr )
{
  if( ! fullFileName.empty() )
  {
    ListNode newNode;
    newNode.filePtr_ = filePtr;
    newNode.fullFileName_ = fullFileName;
    
    filesNamesList_.push_back( newNode );
  }
}
      
void TeTempFilesRemover::removeFileName( const std::string& fullFileName )
{
  if( ! fullFileName.empty() )
  {
    std::list< ListNode >::iterator it = filesNamesList_.begin();
    std::list< ListNode >::iterator itEnd = filesNamesList_.end();
      
    while( it != itEnd )
    {
      if( it->fullFileName_ == fullFileName )
      {
        filesNamesList_.erase( it );
        
		return;
      }
          
      ++it;
    }
  }
}
      
void TeTempFilesRemover::removeFile( const std::string& fullFileName )
{
  if( ! fullFileName.empty() )
  {
    std::list< ListNode >::iterator it = filesNamesList_.begin();
    std::list< ListNode >::iterator itEnd = filesNamesList_.end();
      
    while( it != itEnd )
    {
      if( it->fullFileName_ == fullFileName )
      {
        if( it->filePtr_ )
          fclose( it->filePtr_ );
        
        remove( it->fullFileName_.c_str() );
        
        filesNamesList_.erase( it );
        
		return;
      }
          
      ++it;
    }
  }
}

void TeTempFilesRemover::removeAllFiles()
{
  std::list< ListNode >::iterator it = filesNamesList_.begin();
  std::list< ListNode >::iterator itEnd = filesNamesList_.end();

  while( it != itEnd )
  {
    if( it->filePtr_ )
      fclose( it->filePtr_ );
    
    remove( it->fullFileName_.c_str() );
        
    ++it;
  }
  
  filesNamesList_.clear();
}



