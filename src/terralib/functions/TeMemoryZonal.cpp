
#include "TeMemoryZonal.h"
#include "TeRasterIteratorSet.h"

bool TeMemoryZonal(TeRaster* raster, TePolygon& poly, TeStatisticsDimensionVect& result)
{
  TeRaster::iteratorPoly itBegin = raster->begin(poly, TeBoxPixelIn);
  TeRaster::iteratorPoly itEnd = raster->end(poly, TeBoxPixelIn);
  if (itBegin == itEnd)  // try an approximation for the special case it is a special case
  {
    TeCoord2D pll = raster->coord2Index(TeCoord2D(poly.box().lowerLeft()));
    TeCoord2D pur = raster->coord2Index(TeCoord2D(poly.box().upperRight()));

    TeCoord2D ii(TeRoundRasterIndex(pll.x()),TeRoundRasterIndex(pll.y()));
    TeCoord2D jj(TeRoundRasterIndex(pur.x()),TeRoundRasterIndex(pur.y()));

    TeCoord2D ll = raster->index2Coord(pll);
    TeCoord2D ur = raster->index2Coord(pur);

    ll.x_-= raster->params().resx_/2.;
    ll.y_-= raster->params().resy_/2.;
    
    ur.x_+= raster->params().resx_/2.;
    ur.y_+= raster->params().resy_/2.;

    TeBox bb(ll,ur);
    TePolygon pol = polygonFromBox(bb);
    itBegin = raster->begin(pol, TeBoxPixelIn);
    itEnd = raster->end(pol, TeBoxPixelIn);
    if (itBegin == itEnd)
      return false;
  }
  if(!TeCalculateStatistics (itBegin, itEnd, result))
    return false;

  return true;
}


bool TePolygonSetMemoryZonal(TeRaster* raster, TePolygonSet& ps, TeStatisticsDimensionVect& result)
{
  RasterIteratorSet<TeRaster::iteratorPoly> iset;

  // Inclui em iset pares de iteradores para cada poligono do conjunto
  for(int i=0, size=ps.size(); i<size; i++)
  {
    TePolygon& poly = ps[i];
    TeRaster::iteratorPoly itBegin = raster->begin(poly, TeBoxPixelIn);
    TeRaster::iteratorPoly itEnd   = raster->end(poly, TeBoxPixelIn);
    if(itBegin == itEnd)  // try an approximation for the special case it is a special case
    {
      TeCoord2D pll = raster->coord2Index(TeCoord2D(poly.box().lowerLeft()));
      TeCoord2D pur = raster->coord2Index(TeCoord2D(poly.box().upperRight()));

      TeCoord2D ii(TeRoundRasterIndex(pll.x()),TeRoundRasterIndex(pll.y()));
      TeCoord2D jj(TeRoundRasterIndex(pur.x()),TeRoundRasterIndex(pur.y()));

      TeCoord2D ll = raster->index2Coord(pll);
      TeCoord2D ur = raster->index2Coord(pur);

      ll.x_-= raster->params().resx_/2.;
      ll.y_-= raster->params().resy_/2.;
    
      ur.x_+= raster->params().resx_/2.;
      ur.y_+= raster->params().resy_/2.;

      TeBox bb(ll,ur);
      TePolygon pol = polygonFromBox(bb);
      itBegin = raster->begin(pol, TeBoxPixelIn);
      itEnd = raster->end(pol, TeBoxPixelIn);
      if(itBegin == itEnd)
        continue;
    }
    iset.addIterator(itBegin, itEnd);
  }
  
  // Calcula esttísticas passando iterador que percorre todos os poligonos 
  // recebidos.  
  // Obs: typedef abaixo necessário para o gcc se entender com os tipos
  typedef RasterIteratorSet<TeRaster::iteratorPoly>::iterator setIterator;
  setIterator begin = iset.begin(); 
  setIterator end   = iset.end(); 
  if(!TeCalculateStatistics (begin, end, result))
    return false;

  return true;
}                     



