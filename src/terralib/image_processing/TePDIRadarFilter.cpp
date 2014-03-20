#include "TePDIRadarFilter.hpp"

TePDIRadarFilter::TePDIRadarFilter()
{
  max_adapt_mask_width_ = 9;
}


TePDIRadarFilter::~TePDIRadarFilter()
{
}


void TePDIRadarFilter::ResetState( const TePDIParameters& params )
{
  TePDIBufferedFilter::ResetState( params );
}


