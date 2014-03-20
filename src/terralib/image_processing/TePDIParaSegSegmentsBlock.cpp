#include "TePDIParaSegSegmentsBlock.hpp"

#include <limits.h>

#include <set>

TePDIParaSegSegmentsBlock::TePDIParaSegSegmentsBlock()
{
  startX_ = startY_ = UINT_MAX;
  status_ = BlockNotProcessed;
  width_ = height_ = 0;
  segsPtrsMatrix_.Reset( SegmentsPointersMatrixT::RAMMemPol );
}


TePDIParaSegSegmentsBlock::~TePDIParaSegSegmentsBlock()
{
  clear();
}

void TePDIParaSegSegmentsBlock::clear()
{
  // Getting all segments pointers
  
  std::set< TePDIParaSegSegment* > segsPtrsSet;
  
  unsigned int mtxLine = 0;
  unsigned int mtxLines = segsPtrsMatrix_.GetLines();
  unsigned int mtxCol = 0;
  unsigned int mtxCols = segsPtrsMatrix_.GetColumns();
  TePDIParaSegSegment** mtxLinePtr = 0;
  
  for( mtxLine = 0 ; mtxLine < mtxLines ; ++mtxLine )
  {
    mtxLinePtr = segsPtrsMatrix_[ mtxLine ];
  
    for( mtxCol = 0 ; mtxCol < mtxCols ; ++mtxCol )
    {
      segsPtrsSet.insert( mtxLinePtr[ mtxCol ] );
    }
  }
  
  std::set< TePDIParaSegSegment* >::iterator setIt = segsPtrsSet.begin();
  const std::set< TePDIParaSegSegment* >::iterator setItEnd = segsPtrsSet.end();
  
  while( setIt != setItEnd )
  {
    delete (*setIt);
  
    ++setIt;
  }
  
  // Clear matrix
  
  segsPtrsMatrix_.clear();
}

