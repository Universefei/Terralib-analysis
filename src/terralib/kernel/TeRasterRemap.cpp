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

#include "TeDefines.h"
#include "TeGeometryAlgorithms.h"
#include "TeRasterRemap.h"
#include "TeVectorRemap.h"
#include "TeProgress.h"
#include "TeAgnostic.h"

#include <time.h>


#include <algorithm>	// for max and min
#include <cstdlib>		// for abs

bool TePointOnLine (TeCoord2D& p, TeCoord2D& q, TeCoord2D& t, double tol)
{
	int	tx, ty, px, py, qx, qy, dx, dy;

	px = (int)(p.x () / tol);
	py = (int)(p.y () / tol);

	qx = (int)(q.x () / tol);
	qy = (int)(q.y () / tol);

	tx = (int)(t.x () / tol);
	ty = (int)(t.y () / tol);

	dx = abs(px-qx);
	dy = abs(py-qy);

	if (dx <=2 && dy <= 2)
		return true;

	int q1 = (qy-py)*(tx-px);
	int q2 = (ty-py)*(qx-px);
	int q3 = qx-px;
	int q4 = qy-py;

	if (q1 == 0 && q2 == 0 && q3 == 0 && q4 == 0)
		return true;
	if (abs(q1 - q2) > (std::max(abs(q3), abs(q4))))
		return false;
	return true;
}

TeRasterRemap::TeRasterRemap(TeRaster* rasterIn, TeRaster* rasterOut, 
  bool showProgress )
  :
  rasterIn_(rasterIn),
  rasterOut_(rasterOut),
  transformer_(0),
  interpolation_(0),
  showProgress_(showProgress),
  useOptimizedReprojection_( true ),
  ROI_(TeBox())
{
}

bool TeRasterRemap::setROI(TeBox& roi)
{	
	if (roi.isValid())
	{
		TeBox bb;
		if (rasterIn_ && !TeIntersection(roi,rasterIn_->params().boundingBox(),bb))
			return false;
		ROI_ = roi;	
		return true;
	}
	return false;
}

bool TeRasterRemap::TeOptimizedInterpolateIn( const TeBox &box )
{
	TeRasterParams paramIn = rasterIn_->params();
	TeRasterParams paramOut = rasterOut_->params();

	TeProjection *projIn = paramIn.projection();
	TeProjection *projOut = paramOut.projection();

	if (!projIn || !projOut)
		return false;

	if(!TeCheckBoxConsistency(box, projOut))
	{
		return false;
	}

	projIn->setDestinationProjection(projOut);
	projOut->setDestinationProjection(projIn);

// Take coordinates of box corners in output raster projection

	TeCoord2D poll = box.lowerLeft ();
	TeCoord2D pour = box.upperRight ();

// Bring output coordinates to output line/column domain

	TeCoord2D pxoll = rasterOut_->coord2Index (poll);
	TeCoord2D pxour = rasterOut_->coord2Index (pour);

// Round output coordinates to nearest exact pixel

	int x1,y1;
	x1 = (int) (pxoll.x() - 0.5);
	y1 = (int) (pxoll.y() + 0.5);
	pxoll = TeCoord2D (x1,y1);

	int x2,y2;
	x2 = (int)(pxour.x() + 0.5);
	y2 = (int)(pxour.y() - 0.5);
	pxour = TeCoord2D (x2,y2);
//	fprintf (loga,"Coordenadas de saida - x1=%d  x2=%d y1=%d y2=%d\n",x1,x2,y1,y2);

	poll = rasterOut_->index2Coord (pxoll);
	pour = rasterOut_->index2Coord (pxour);
//	fprintf (loga,"Box de saida        - x1=%f  x2=%f y1=%f y2=%f\n\n",poll.x(),pour.x(),poll.y(),pour.y());

	TeCoord2D poul = TeCoord2D(poll.x(),pour.y());
	TeCoord2D polr = TeCoord2D(pour.x(),poll.y());

// Bring coordinates of box four corners to input raster projection

	TeCoord2D pill = projOut->PC2LL (poll);
	TeCoord2D piur = projOut->PC2LL (pour);
	TeCoord2D piul = projOut->PC2LL (poul);
	TeCoord2D pilr = projOut->PC2LL (polr);

	pill = projIn->LL2PC (pill);
	piur = projIn->LL2PC (piur);
	piul = projIn->LL2PC (piul);
	pilr = projIn->LL2PC (pilr);

// Check if linear interpolation may be performed on input raster
// Evaluate point at middle of the edges in output domain and check if their 
// corresponding points belong to the edges in input domain. If they belong, 
// a linear interpolation may be performed, else divide output image 
// in four quadrants and try interpolating again


	TeCoord2D	pou((pour.x()-poul.x())/2.+poul.x(), poul.y()), // upper edge
				pob((polr.x()-poll.x())/2.+poll.x(), poll.y()), // bottom edge
				pol(poll.x(), (poul.y()-poll.y())/2.+poll.y()), // left edge
				por(polr.x(), (pour.y()-polr.y())/2.+polr.y()); // right edge

// Evaluate corresponding points in input raster domain
	TeCoord2D	piu, pib, pil, pir;

	piu = projOut->PC2LL (pou);
	pib = projOut->PC2LL (pob);
	pil = projOut->PC2LL (pol);
	pir = projOut->PC2LL (por);

	piu = projIn->LL2PC (piu);
	pib = projIn->LL2PC (pib);
	pil = projIn->LL2PC (pil);
	pir = projIn->LL2PC (pir);

// Check if middle points belong to the edges

	double tol = MAX(paramIn.resx_ ,paramIn.resy_ );

//	if (!TeIsOnSegment (piu,piul,piur/*,tol*/) ||
//		!TeIsOnSegment (pir,pilr,piur/*,tol*/) ||
//		!TeIsOnSegment (pib,pill,pilr/*,tol*/) ||
//		!TeIsOnSegment (pil,pill,piul/*,tol*/))

	if (!TePointOnLine (piul,piur,piu,tol) ||
		!TePointOnLine (pilr,piur,pir,tol) ||
		!TePointOnLine (pill,pilr,pib,tol) ||
		!TePointOnLine (pill,piul,pil,tol))
	{
// If one of them does not belong to correspondig edge, divide output in four quadrants
		TeCoord2D	pom ((por.x()-pol.x())/2.+pol.x(), (pou.y()-pob.y())/2.+pob.y()); // center point
		TeBox quadrantul (pol, pou);
		if( ! TeOptimizedInterpolateIn ( quadrantul ) ) return false;
		TeBox quadrantur (pom, pour);
		if( ! TeOptimizedInterpolateIn ( quadrantur ) ) return false;
		TeBox quadrantll (poll, pom);
		if( ! TeOptimizedInterpolateIn ( quadrantll ) ) return false;
		TeBox quadrantlr (pob, por);
		if( ! TeOptimizedInterpolateIn ( quadrantlr ) ) return false;
    
		return true;
	}

// Start linear interpolation on input image.

	double	xl,	// x at the beginning of the line
		yl,	// y at the beginning of the line
		xr,	// x at the end of the line
		yr,	// y at the end of the line
		dx,	// inner loop x increment
		dy,	// inner loop y increment
		dxl,	// x increment at the beginning of line
		dyl,	// y increment at the beginning of line
		dxr,	// x increment at the end of line
		dyr;	// y increment at the end of line

	TeCoord2D pxill = rasterIn_->coord2Index (pill);
	TeCoord2D pxiul = rasterIn_->coord2Index (piul);
	TeCoord2D pxilr = rasterIn_->coord2Index (pilr);
	TeCoord2D pxiur = rasterIn_->coord2Index (piur);

// Evaluate the increments in x and y on both sides of input image

	int	i, j;
	double	x,y;

	x1 = (int)pxoll.x()-1;
	y1 = (int)pxour.y()-1;

	x2 = (int)pxour.x()+1;
	y2 = (int)pxoll.y()+1;

	dxl = ( (pxill.x()-pxiul.x())/(y2-y1) );
	dyl = ( (pxill.y()-pxiul.y())/(y2-y1) );

	dxr = ( (pxilr.x()-pxiur.x())/(y2-y1) );
	dyr = ( (pxilr.y()-pxiur.y())/(y2-y1) );

// Set initial values for x and y at beginning point on input image

	xl = pxiul.x() - 1;
	yl = pxiul.y() - 1;

// Set initial values for x and y at end point on input image

	xr = pxiur.x() + 1;
	yr = pxiur.y() + 1;

// Evaluate increments for the first line

	dx = (xr-xl)/(x2-x1);
	dy = (yr-yl)/(x2-x1);

	x = xl;		// round to left pixel
	y = yl;		// round to left pixel

	for (j=y1;j<=y2;j++)
	{
		for (i=x1;i<=x2;i++)
		{
			if (interpolation_ == 0)
				transformer_->apply((int)(x+0.5),(int)(y+0.5),i,j);
 			x += dx;
			y += dy;
		}

		xl += dxl;
		x = xl;
		xr += dxr;
		yl += dyl;
		y = yl;
		yr += dyr;
		dx = (xr-xl)/(x2-x1);
		dy = (yr-yl)/(x2-x1);
	}
  
  return true;
}

bool TeRasterRemap::TeInterpolateIn( const TeBox& box )
{
  TEAGN_DEBUG_CONDITION( rasterIn_, "Invalid raster pointer" )
  TEAGN_DEBUG_CONDITION( rasterOut_, "Invalid raster pointer" )
  TEAGN_DEBUG_CONDITION( rasterIn_->params().projection(), "Invalid proj" )
  TEAGN_DEBUG_CONDITION( rasterOut_->params().projection(), "Invalid proj" )
  TEAGN_DEBUG_CONDITION( transformer_, "Invalid transformer" )
  
  TeProjection& projIn = *( rasterIn_->params().projection() );
  TeProjection& projOut = *( rasterOut_->params().projection() );
  
  projIn.setDestinationProjection( &projOut );
  projOut.setDestinationProjection( &projIn );

  if(!TeCheckBoxConsistency(box, &projOut))
  {
	return false;
  }
  
  // Guessing the output raster data limits
  
  const TeCoord2D ll = box.lowerLeft();
  const TeCoord2D ur = box.upperRight();  
  
  const TeCoord2D idxll = rasterOut_->coord2Index( ll );
  const TeCoord2D idxur = rasterOut_->coord2Index( ur );  
  
  const double minX = MIN( idxll.x(), idxur.x() );
  const double maxX = MAX( idxll.x(), idxur.x() );
  const double minY = MIN( idxll.y(), idxur.y() );
  const double maxY = MAX( idxll.y(), idxur.y() );
  
  const double outLineStart = MAX( ( (double)TeRound( minY - 0.5 ) ), 0.0 );
  const double outColStart = MAX( ( (double)TeRound( minX - 0.5 ) ), 0.0 );
  const double outLineBound = MIN( ( (double)TeRound( maxY + 0.5 ) ),
    (double)rasterOut_->params().nlines_ );
  const double outColBound = MIN( ( (double)TeRound( maxX + 0.5 ) ),
    (double)rasterOut_->params().ncols_ );
  
  TEAGN_DEBUG_CONDITION( outLineStart <= outLineBound, "Invalid range" );
  TEAGN_DEBUG_CONDITION( outColStart <= outColBound, "Invalid range" );
  
  // Inverse mapping each point inside the given intersection box
  // over the output raster  
  
  double outLine = 0;
  double outCol = 0;
  TeCoord2D outIdxCoord;
  TeCoord2D outCoord;
  TeCoord2D inCoord;
  TeCoord2D inIdxCoord;
  TeCoord2D auxLLCoord;
  TeRaster& inRaster = *rasterIn_;
  TeRaster& outRaster = *rasterOut_;
  TeRasterTransform& transf = *transformer_;
    
  for( outLine = outLineStart ; outLine < outLineBound ; ++outLine )
  {
    outIdxCoord.y( outLine );
    
    for( outCol = outColStart ; outCol < outColBound ; ++outCol )
    {
      outIdxCoord.x( outCol );
      
      outCoord = outRaster.index2Coord( outIdxCoord );
      
      auxLLCoord = projOut.PC2LL( outCoord );
      
      inCoord = projIn.LL2PC( auxLLCoord );
      
      inIdxCoord = inRaster.coord2Index( inCoord );
      
      transf.apply( inIdxCoord.x(),inIdxCoord.y(), outCol, outLine );
    }
  }
  
  return true;
}

bool TeRasterRemap::apply( bool showProgress )
{
	// check if input and output raster are set
	if (!rasterIn_ || !rasterOut_)
		return false;

	TeRasterParams paramIn = rasterIn_->params();
	TeRasterParams paramOut = rasterOut_->params();

	TeProjection* projIn = paramIn.projection();
	TeProjection* projOut = paramOut.projection();

	// if only one of the rasters don´t have projection can´t do remapping
	if ((projIn && !projOut) || (projOut && !projIn))
		return false;

	showProgress_ = showProgress;

	// if no transformer defined create a new on based on typical cases of 
	// visualization and import operations
	bool delTransf = false;
	if (!transformer_)
	{
		delTransf = true;		// remember to delete it 
		transformer_ = new TeRasterTransform();
		transformer_->setRasterIn(rasterIn_);
		transformer_->setRasterOut(rasterOut_);

		// pallete raster to a RGB device -> apply input raster pallete
		if ((paramIn.photometric_[0] == TeRasterParams::TePallete) &&
			 paramOut.nBands() == 3)
			transformer_->setTransfFunction(&TeRasterTransform::Pallete2ThreeBand);
		// mono band raster to a RGB device -> repeat band to 3 channels
		else if (paramIn.nBands() == 1 && paramOut.nBands() == 3)
			transformer_->setTransfFunction(&TeRasterTransform::Mono2ThreeBand);
		// copy first n input bands to first n output bands
		else		
			transformer_->setTransfFunction(&TeRasterTransform::Band2Band);
	}
	else if(transformer_->getTransfFunction() == TeRasterTransform::TeNoTransf)
	{
		transformer_->setTransfFunction(&TeRasterTransform::Band2Band);
	}

	bool result;
	// check if datum's are different
	if (projIn->datum().name() != projOut->datum().name())
	{
		projIn->setDestinationProjection(projOut);
		projOut->setDestinationProjection(projIn);
	}

	if (projIn && projOut && (*projIn == *projOut))	// same projection
	{
		TeBox	boxIn = paramIn.box();
		TeBox	boxOut = paramOut.box();

		
		if (!(boxIn == boxOut) ||					// different resolutions or boxes
			paramIn.resx_   != paramOut.resx_ || 
			paramIn.resy_   != paramOut.resy_ ||
			paramIn.ncols_  != paramOut.ncols_ ||
			paramIn.nlines_ != paramOut.nlines_ )
			result = resample();
		else										// same dimensions
			result = copy();
	}
	else											// different projection
		result = remap();

	if (delTransf)
	{
		delete transformer_;
		transformer_ = 0;
	}
	return result;
}

bool TeRasterRemap::copy ()
{
	const TeRasterParams& paramOut = rasterOut_->params();
	if(showProgress_ && TeProgress::instance())
		TeProgress::instance()->setTotalSteps(paramOut.nlines_);

	int li, lf, ci, cf;
	if (ROI_.isValid())
	{
		TeCoord2D aux = rasterIn_->coord2Index(ROI_.lowerLeft());
		lf = TeRoundRasterIndex(aux.y_);
		ci = TeRoundRasterIndex(aux.x_);
		aux = rasterIn_->coord2Index(ROI_.upperRight());
		li = TeRoundRasterIndex(aux.y_);
		cf = TeRoundRasterIndex(aux.x_);
	}
	else
	{
		li = ci = 0;
		lf = paramOut.nlines_;
		cf = paramOut.ncols_;
	}

	int c, l;
	clock_t	ti, tf;
	ti = clock();
	for (l=li; l<lf; ++l)
	{
		for (c=ci; c<cf; ++c)
			transformer_->apply(c,l,c,l);

		if (showProgress_ && TeProgress::instance())
		{
			tf = clock();
			if (int((tf-ti)/CLOCKS_PER_SEC) > 3)
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				TeProgress::instance()->setProgress(l);
				ti = tf;
			}
		}
	}
	if(showProgress_ && TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

bool TeRasterRemap::resample ()
{
	TeRasterParams paramIn = rasterIn_->params();
	TeRasterParams paramOut = rasterOut_->params();

	TeBox	boxIn = paramIn.boundingBox();  
	TeBox	boxOut = paramOut.boundingBox(); 

// Evaluate intersection of input and output rasters in output raster projection domain

	TeBox boxInter;

	if (!TeIntersection (boxIn,boxOut,boxInter))
		return false;

	if (ROI_.isValid())
	{
		if (!TeIntersection (ROI_,boxInter,boxInter))
			return false;
	}

// Take coordinates of intersection box

	TeCoord2D poll = boxInter.lowerLeft ();
	TeCoord2D pour = boxInter.upperRight ();

// Bring it to output line/column domain

	TeCoord2D pxoll = rasterOut_->coord2Index (poll);
	TeCoord2D pxour = rasterOut_->coord2Index (pour);

// Round it to nearest pixel
	int x1,y1,x2,y2;
	x1 = TeRoundRasterIndex(pxoll.x_) - 1;
	y2 = TeRoundRasterIndex(pxoll.y_) + 1;
	pxoll = TeCoord2D (x1,y2);

	x2 = TeRoundRasterIndex(pxour.x_) + 1;
	y1 = TeRoundRasterIndex(pxour.y_) - 1;
	pxour = TeCoord2D (x2,y1);
		
// Bring it to output projection domain
	poll = rasterOut_->index2Coord (pxoll);
	pour = rasterOut_->index2Coord (pxour);

// Bring upper left point to input line/column domain
	TeCoord2D ulPoint = rasterIn_->coord2Index(TeCoord2D(poll.x(),pour.y()));

//  - -
// |A|B|
//  - - 
// |C| |
//  - -
// Get output projection domain of corner ABC
	TeCoord2D A = rasterOut_->index2Coord(TeCoord2D(0,0));
	TeCoord2D B = rasterOut_->index2Coord(TeCoord2D(1,0));
	TeCoord2D C = rasterOut_->index2Coord(TeCoord2D(0,1));

// Calculate equivalent input line/column coordinates 
	TeCoord2D Ae = rasterIn_->coord2Index(A);
	TeCoord2D Be = rasterIn_->coord2Index(B);
	TeCoord2D Ce = rasterIn_->coord2Index(C);

// Calculate input increments in both directions, in number of pixels
// for each output 1 pixel increment in 
	double eDxJ = Be.x()-Ae.x();		// increment in X direction when moving from A->B	
	double eDyJ = Be.y()-Ae.y();		// increment in Y direction when moving from A->B
	double eDyI = Ce.y()-Ae.y();		// increment in Y direction when moving from C->A
	double eDxI = Ce.x()-Ae.x();		// increment in X direction when moving from C->A

	int lin_sai,col_sai;
	double lin_entra,col_entra; 

	double lin_entra_i = ulPoint.y();	// starting at the upper-left corner	
	double col_entra_i = ulPoint.x();

	if( showProgress_ && TeProgress::instance())
		TeProgress::instance()->setTotalSteps((int)(y2-y1));

	int count = 1;
	clock_t	ti, tf;
	ti = clock();
	for (lin_sai=y1;lin_sai<=y2;lin_sai++)
	{
		lin_entra = lin_entra_i;
		col_entra = col_entra_i;
		for (col_sai=x1;col_sai<=x2;col_sai++)
		{
			transformer_->apply(TeRoundRasterIndex(col_entra),TeRoundRasterIndex(lin_entra),col_sai,lin_sai);
  			col_entra += eDxJ;
			lin_entra += eDyJ;
		}
		count++;
		lin_entra_i += eDyI;
		col_entra_i += eDxI;
		if(showProgress_ && TeProgress::instance())
		{
			tf = clock();
			if (int((tf-ti)/CLOCKS_PER_SEC) > 3)
			{
				if (TeProgress::instance()->wasCancelled())
					break;
				TeProgress::instance()->setProgress(count);
				ti = tf;
			}
		}
	}
	if (showProgress_ && TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

bool TeRasterRemap::remap()
{
  TeRasterParams paramIn = rasterIn_->params();
  TeRasterParams paramOut = rasterOut_->params();

  TeProjection *projIn = paramIn.projection();
  TeProjection *projOut = paramOut.projection();


  if ( projIn == 0 || projOut == 0)
    return false;

  projIn->setDestinationProjection(projOut);

  TeBox	boxIn =  paramIn.box();
  TeBox	boxOut = paramOut.box();

// Evaluate intersection of input and output rasters in output raster projection domain
  TeBox boxInter;
  boxIn = TeRemapBox (boxIn, projIn, projOut);
  if (!TeIntersection (boxIn,boxOut,boxInter))
    return false;

  if (ROI_.isValid())
  {
    boxIn = TeRemapBox (ROI_, projIn, projOut);
    if (!TeIntersection (boxIn,boxInter,boxInter))
      return false;
  }

    // Start trying to remap the input quadrilateral
  if( useOptimizedReprojection_ )
  {    
	  return TeOptimizedInterpolateIn ( boxInter );
  }
  else
  {
    return TeInterpolateIn( boxInter );
  }
}
