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
/*! \file TeGridIndex.h
    \brief This file contains an implementation of a fixed grid data structure.	
*/

#ifndef  __TERRALIB_INTERNAL_GRIDINDEX_H
#define  __TERRALIB_INTERNAL_GRIDINDEX_H

#include "TeGeometry.h"
#include "TeIntersector.h"

#include <map>

/* 
 * WARNING:
 *
 *       1. This module IS UNDER DEVELOPMENT!
 *
 *       2. These data structures means to be used only in TerraLib kernel and should not be used by anyone because
 *          the support and interfaces can be changed in future.
 *
 */

namespace TeSAM
{
/** @defgroup GridStructure Fixed Grid Data Structure
 *  TerraLib Fixed Grid Data Structure.
 *  @{
 */


//! A class that represents a fixed grid
/*!	
	.	

	WARNING:

	1. XXXX

	2. XXXX.

	3. 

 */
class TL_DLL TeGridIndex
{
	public:

		//! grid column/line cell index
		typedef pair<int, int> gridKey;							

		//! grid structure
		typedef multimap<gridKey, unsigned int> gridType;		// em cada linha e coluna podemos associar uma lista de outros objetos

		double dx_;			//!< grid resolution in x axis.
		double dy_;			//!< grid resolution in y axis.
		int npartsX_;		//!< Number of columns
		int npartsY_;		//!< Number of lines
		double xi_;			//!< Lower left x.
		double yi_;			//!< Lower left y.
		double xf_;			//!< Upper right x.
		double yf_;			//!< Upper right y.
		gridType grid_;		//!< the grid.
		unsigned int size_;	//!< Number of elements in the grid (if one element intersects more than one grid cell, they will count many times).

	public:

		//! Empty constructor
		TeGridIndex()
			: dx_(0.0), dy_(0.0), npartsX_(0), npartsY_(0), xi_(0.0), yi_(0.0), xf_(0.0), yf_(0.0), size_(0u)
		{
		}

		//! Constructor of the grid
		TeGridIndex(const TeBox& b, const double& resX, const double& resY)
			: size_(0u)
		{
			setBox(b);

			setResolution(resX, resY);			
		}

		//! Adjusts grid bounding box
		void setBox(const TeBox& b)
		{
			xi_ = b.x1_;
			yi_ = b.y1_;

			xf_ = b.x2_;
			yf_ = b.y2_;
		}

		//! Returns the number of associatade cells
		unsigned int size() const
		{
			return size_;
		}

		//! Sets grid resolution factor
		void setResolution(const double& resX, const double& resY)
		{
			dx_ = resX;
			dy_ = resY;

			if((dx_ <= 0.0) || (dy_ <= 0.0))
			{
				npartsX_ = 0;
				npartsY_ = 0;
			}
			else
			{
				npartsX_ = TeRound((xf_ - xi_) / dx_) + 1;
				npartsY_ = TeRound((yf_ - yi_) / dy_) + 1;
			}
		}

		//! Return number of columns
		int getNumCols() const
		{
			return npartsX_;
		}
		
		//! Return number of lines
		int getNumLines() const
		{
			return npartsY_;
		}

		//! Tells if the grid definition is correct
		bool isValid() const
		{
			return (npartsX_ != 0 && npartsY_ != 0);
		}


		//! Inserts an item with one coordinate
		void insert(const TeCoord2D& c, const unsigned int& itemId)
		{
			int col = int((c.x_ - xi_) / dx_);
			int line = int((c.y_ - yi_) / dy_);

			gridKey key(col, line);

			grid_.insert(gridType::value_type(key, itemId));

			++size_;
		}

		//! Returns lower and upper grid positions
		void getIndex(const TeCoord2D& c1, const TeCoord2D& c2, int& col1, int& line1, int& col2, int& line2)
		{
			double lowerX = ((c1.x_ < c2.x_) ? c1.x_ : c2.x_);
			double lowerY = ((c1.y_ < c2.y_) ? c1.y_ : c2.y_);
			double upperX = ((c1.x_ > c2.x_) ? c1.x_ : c2.x_);
			double upperY = ((c1.y_ > c2.y_) ? c1.y_ : c2.y_);

			col1 = int((lowerX - xi_) / dx_);
			line1 = int((lowerY - yi_) / dy_);
			col2 = int((upperX - xi_) / dx_) + 1;
			line2 = int((upperY - yi_) / dy_) + 1;	
		}

		//! Inserts intem into the grid
		void insert(const TeCoord2D& c1, const TeCoord2D& c2, const unsigned int& itemId)
		{
			gridKey key(0, 0);

			int col1;
			int line1;
			int col2;
			int line2;

			getIndex(c1, c2, col1, line1, col2, line2);

			for(int i = col1; i <= col2; ++i)
			{
				key.first  = i;

				for(int j = line1; j <= line2; ++j)
				{
					
					key.second = j;

					grid_.insert(gridType::value_type(key, itemId));

					++size_;
				}
			}
		}

		//! Put line segments over the grid cells
		void indexLine(const TeLine2D& l)
		{
			unsigned int nstep = l.size() - 1;

			for(unsigned int i = 0; i < nstep; ++i)
				insert(l[i], l[i+1], i);
		}

		//! Return a reference for internal grid representation
		gridType& getGrid()
		{
			return grid_;
		}

		//! Only for debug purpose
		void getLines(TeLineSet& lset)
		{
			for(int i = 0; i <= npartsX_; ++i)
			{
				TeCoord2D c1, c2;
				c1.x_ = xi_ + dx_ * i;
				c1.y_ = yi_;
				
				c2.x_ = xi_ + dx_ * i;
				c2.y_ = yi_ + npartsY_ * dy_;

				TeLine2D l;

				l.add(c1); l.add(c2);

				lset.add(l);
			}

			for(int j = 0; j <= npartsY_; ++j)
			{
				TeCoord2D c1, c2;
				c1.x_ = xi_;
				c1.y_ = yi_ + dy_ * j;
				
				c2.x_ = xi_ + npartsX_ * dx_;
				c2.y_ = yi_ + dy_ * j;

				TeLine2D l;

				l.add(c1); l.add(c2);

				lset.add(l);
			}
		}		

	private:

		//! No copy allowed
		TeGridIndex(const TeGridIndex& rhs);

		//! No copy allowed
		TeGridIndex& operator=(const TeGridIndex& rhs);

};	// end of class TeGridIndex

//! Get average length os segments in each direction
TL_DLL inline void getAverage(const TeLine2D& l, double& dx, double& dy)
{
	unsigned int nstep = l.size() - 1;

	double sumX = 0.0;
	double sumY = 0.0;

	for(unsigned int i = 0; i < nstep; ++i)
	{
		sumX += fabs(l[i + 1].x_ - l[i].x_);
		sumY += fabs(l[i + 1].y_ - l[i].y_);
	}

	dx = (sumX / double(nstep)) * 8.0;
	dy = (sumY / double(nstep)) * 8.0;

	return;
}

//! Computes intersections
TL_DLL inline bool TeSafeIntersectionsGrid(const TeLine2D& redLine, const TeLine2D& blueLine, TeINTERSECTOR2::TeVectorBoundaryIP& report, const unsigned int& redObjId = 0, const unsigned int& blueObjId = 0)
{
	double dx = 0.0;
	double dy = 0.0;
	
	getAverage(redLine, dx, dy);

	TeGridIndex redGrid(redLine.box(), dx, dy);

	redGrid.indexLine(redLine);

    TeGridIndex::gridKey key(0, 0);

	int colmax = redGrid.getNumCols();
	int colmin = 0;
	int linemax = redGrid.getNumLines();
	int linemin = 0;	

	int col1 = 0;
	int col2 = 0;
	int line1 = 0;
	int line2 = 0;

	bool hasIntersections = false;

	TeSegmentIntersectionType t = TeImproperIntersection;

// Para cada segmento azul, verificar os quadrantes interceptados
	unsigned int nStep = blueLine.size() - 1;

	for(unsigned int k = 0; k < nStep; ++k)
	{
		redGrid.getIndex(blueLine[k], blueLine[k+1], col1, line1, col2, line2);

		if((col2 < colmin) || (line2 < linemin) || (col1 > colmax) || (line1 > linemax))
			continue;

		//if(((col1 > colmax) && (col2 > colmax)) || ((line1 > linemax) && (line2 > linemax)))
		//	continue;

		if(col1 < colmin)
			col1 = colmin;
		
		if(col2 > colmax)
			col2 = colmax;

		if(line1 < linemin)
			line1 = linemin;

        if(line2 > linemax)
			line2 = linemax;

		for(int col = col1; col <= col2; ++col)
			for(int lin = line1; lin <= line2; ++lin)
			{
				key.first  = col;
				key.second = lin;

				pair<TeGridIndex::gridType::iterator, TeGridIndex::gridType::iterator> its = redGrid.getGrid().equal_range(key);

				while(its.first != its.second)
				{
					TeINTERSECTOR2::TeBoundaryIP ip;

					if(TeINTERSECTOR2::TeIntersection(redLine[its.first->second], redLine[its.first->second + 1], blueLine[k], blueLine[k+1], ip, t))
					{
						hasIntersections = true;

						ip.redPartNum_ = redObjId;
						ip.redSegNum_ = its.first->second;
						ip.bluePartNum_ = blueObjId;
						ip.blueSegNum_ = k;

						if(ip.coords_.size() == 2)	//overlap
						{
							// Verificar se os pontos estão em ordem crescente
							if(ip.coords_[0].x_ < ip.coords_[1].x_)
							{
								report.push_back(ip);
							}
							else if(ip.coords_[0].x_ > ip.coords_[1].x_)
							{
								swap(ip.coords_[0], ip.coords_[1]);
								report.push_back(ip);
							}
							else if(ip.coords_[0].y_ < ip.coords_[1].y_)
							{
								report.push_back(ip);						
							}
							else
							{
								swap(ip.coords_[0], ip.coords_[1]);
								
								report.push_back(ip);						
							}
						}
						else
						{
							report.push_back(ip);
						}
					}

					++(its.first);
				}
			}
	}

	return hasIntersections;
}

/** @} */ 

};	// end namespace TeSAM


#endif	// __TERRALIB_INTERNAL_GRIDINDEX_H




