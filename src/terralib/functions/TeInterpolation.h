/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.

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

/*! \file TeInterpolation.h
    This file contains routines for interpolation of cell themes and raster themes.	
*/

#ifndef  __TERRALIB_INTERNAL_INTERPOLATION_H
#define  __TERRALIB_INTERNAL_INTERPOLATION_H

#include "TeTheme.h"

#include "TeFunctionsDefines.h"


/*
 * NOTE: You should call TeCellInterpolate or TeRasterInterpolate with the required parameters. Don't use
 *       the class TeInterpolationAlgorithms, it is for internal use only!
 *
 */


/** @defgroup InterpolationDataStructure Interpolation Algorithms and Data Structures
 *  TerraLib Interpolation Data Structure.
 *  @{
 */

/*! \enum TeInterpolationAlgorithm

	Methods of interpolation, may be:
    - TeNNInterpolation                  Interpolation that uses the nearest neighbor value
	- TeAvgInterpolation                 Interpolation that uses the average of k-nearest neighbors values
	- TeDistWeightAvgInterpolation       Interpolation with weight average (inverse of square distance or other) of k-nearest neighbors values
	- TeAvgInBoxInterpolation			 Interpolation with simple average of elements in box
	- TeDistWeightAvgInBoxInterpolation  Interpolation with weight average of elements in box
*/ 
enum TeInterpolationAlgorithm { TeNNInterpolation, TeAvgInterpolation, TeDistWeightAvgInterpolation,
                                TeAvgInBoxInterpolation, TeDistWeightAvgInBoxInterpolation };

//! Class that supports several types of interpolation methods
/*!
	Given a KdTree and a key, choose a method of interpolation.
	
	WARNING:	1. This class MUST BE USED ONLY BY in the routines below (TeCellInterpolate and TeRasterInterpolate) 
	               and should NOT be used by anyone because the support and interfaces can be changed in future.
				   THIS IS FOR INTERNAL USE ONLY.
	
				2. As in simple average as in weight average, if all neighbours aren't find the calculus considerates 
	               the number of found neighbours.

			    3. If an unexpected situation occurs so -TeMAXFLOAT is returned.

				4. If a sample exist in the same position of a key, so the sample value is returned, i.e.,
				   not all neighbours are considered.
 */
template<class ADAPTATIVEKDTREE> class TeInterpolationAlgorithms
{
	protected:

		typedef typename ADAPTATIVEKDTREE::kdKey kdKey;
		typedef typename ADAPTATIVEKDTREE::kdDataItem kdDataItem;
		typedef typename ADAPTATIVEKDTREE::kdNode kdNode;

		//! reference to an adptative kdtree 
		const ADAPTATIVEKDTREE& kd_;

	public:		

		//! Constructor
		TeInterpolationAlgorithms(const ADAPTATIVEKDTREE& kd)
			: kd_(kd)
		{
		}

		//! Destructor
		~TeInterpolationAlgorithms()
		{
		}

		//! Returns the nearest neighbor value
		double nearestNeighbor(const kdKey& key)
		{
			vector<kdDataItem> report;
			vector<double> sqrDists;

			fillNNVector(report, 1);

			kd_.nearestNeighborSearch(key, report, sqrDists, 1);

			if(sqrDists[0] >= TeMAXFLOAT)
				return -TeMAXFLOAT;
			else
				return report[0].value();
		}

		//! Simple Average of Nearest Neighbors. If an error occur returns -TeMAXFLOAT
		double avgNearestNeighbor(const kdKey& key, const unsigned int& numberOfNeighbors)
		{
			vector<kdDataItem> report;
			vector<double> sqrDists;

			fillNNVector(report, numberOfNeighbors);

			kd_.nearestNeighborSearch(key, report, sqrDists, numberOfNeighbors);

			double numElements = numberOfNeighbors;

			double sum = 0.0;

			for(unsigned int i = 0; i < numberOfNeighbors; ++i)
			{
				if(sqrDists[i] >= TeMAXFLOAT)
				{
					--numElements;
					continue;
				}

				if(sqrDists[i] == 0.0)
					return report[i].value();

				sum += report[i].value();
			}

			if(numElements > 0.0)
                return sum / numElements;
			else
				return -TeMAXFLOAT;
		}

		//! Weight Average of Nearest Neighbors. If an error occur returns -TeMAXFLOAT
		double distWeightAvgNearestNeighbor(const kdKey& key, const unsigned int& numberOfNeighbors, const int& powValue)
		{
			vector<kdDataItem> report;
			vector<double> sqrDists;

			fillNNVector(report, numberOfNeighbors);

			kd_.nearestNeighborSearch(key, report, sqrDists, numberOfNeighbors);

			double num = 0.0;
			double den = 0.0;

			if(powValue == 1.0)
			{
				for(unsigned int i = 0; i < numberOfNeighbors; ++i)
				{
					if(sqrDists[i] >= TeMAXFLOAT)
						continue;

					if(sqrDists[i] == 0.0)
						return report[i].value();

					double wi = 1.0 / (sqrt(sqrDists[i]));

					num += (wi * report[i].value());
					den += wi;
				}
			}
			else if(powValue == 2.0)
			{
				for(unsigned int i = 0; i < numberOfNeighbors; ++i)
				{
					if(sqrDists[i] >= TeMAXFLOAT)
						continue;

					if(sqrDists[i] == 0.0)
						return report[i].value();

					double wi = 1.0 / (sqrDists[i]);

					num += (wi * report[i].value());
					den += wi;
				}
			}
			else
			{
				for(unsigned int i = 0; i < numberOfNeighbors; ++i)
				{
					if(sqrDists[i] >= TeMAXFLOAT)
						continue;

					if(sqrDists[i] == 0.0)
						return report[i].value();

					double wi = 1.0 / pow(sqrt(sqrDists[i]), powValue);

					num += (wi * report[i].value());
					den += wi;
				}
			}

			if(den != 0.0)
                return num / den;
			else
				return -TeMAXFLOAT;
		}

		//! Simple Average of Elements in Box. If an error occur returns -TeMAXFLOAT
		double boxAvg(const TeBox& box)
		{
			vector<kdNode*> report;

			kd_.search(box, report);

			unsigned int numberOfNodes = report.size();

			unsigned int numElements = 0;

			double sum = 0.0;

			for(unsigned int i = 0; i < numberOfNodes; ++i)
			{
				unsigned int nodeSize = report[i]->getData().size();

				for(unsigned int j = 0; j < nodeSize; ++j)
				{
					if(TeIntersects(report[i]->getData()[j].location(), box))
					{
                        sum += report[i]->getData()[j].value();
						++numElements;
					}
				}				
			}

			if(numElements > 0)
                return sum / double(numElements);
			else
				return -TeMAXFLOAT;
		}

		//! Distance Weight Average of Elements in Box. If an error occur returns -TeMAXFLOAT
		double boxDistWeightAvg(const kdKey& key, const TeBox& box, const int& powValue)
		{
			vector<kdNode*> report;

			kd_.search(box, report);

			unsigned int numberOfNodes = report.size();

			double num = 0.0;
			double den = 0.0;

			for(unsigned int i = 0; i < numberOfNodes; ++i)
			{
				unsigned int nodeSize = report[i]->getData().size();

				for(unsigned int j = 0; j < nodeSize; ++j)
				{
					if(TeIntersects(report[i]->getData()[j].location(), box))
					{
						double wi = 1 / pow(TeDistance(key, report[i]->getData()[j].location()), powValue);
						
						num += wi * report[i]->getData()[j].value();
						den += wi;
					}
				}				
			}

			if(den != 0.0)
                return num / den;
			else
				return -TeMAXFLOAT;
		}

	protected:

		//! Fills the nearest neighbour vector with default values
		void fillNNVector(vector<kdDataItem>& report, const unsigned int& numberOfNeighbors) const
		{
			for(unsigned int i = 0; i < numberOfNeighbors; ++i)
			{
				TeCoord2D c(TeMAXFLOAT, TeMAXFLOAT);
				kdDataItem item(c, -TeMAXFLOAT);

				report.push_back(item);
			}
		}

	private:

		//! No copy allowed
		TeInterpolationAlgorithms(const TeInterpolationAlgorithms& rhs);

		//! No copy allowed
		TeInterpolationAlgorithms& operator=(const TeInterpolationAlgorithms& rhs);		
};



//! Interpolates a cell theme
/*
		\param inputTheme			The theme with samples to be used: samples are treated as a layer of points with a column with a value (double or integer)
		\param inputAttrTableName   Theme table with samples
		\param sampleColumnName     Table column with sample values: must be int or double values
		\param outputTheme          The theme where output will be stored: this theme must already exist
		\param outputAttrTableName  Theme table where output will be stored: this table must already exist
		\param outputColumnName     Table column where data will be stored: this column must already exist
		\param algorithm		    Type of algorithm to be used: see enum TeInterpolationAlgorithm
		\param numberOfNeighbors    Number of nearest neighbor to be used in interpolation
		\param powValue			    Pow parameter for inverse distance function
		\param boxRatio				Ratio of the box used to search samples
		\return TRUE if there isn't an error during interpolation otherwise return FALSE
 */
TLFUNCTIONS_DLL bool TeCellInterpolate(TeTheme* inputTheme, const string& inputAttrTableName, const string& sampleColumnName,
                       TeTheme* outputTheme, const string& outputAttrTableName, const string& outputColumnName,
				       const TeInterpolationAlgorithm& algorithm = TeNNInterpolation,
                       const unsigned int& numberOfNeighbors = 1, const int& powValue = 1, const double& boxRatio = 1.0);

//! Interpolates a a raster
/*
		\param inputTheme			The theme with samples to be used: samples are treated as a layer of points with a column with a value (double or integer)
		\param inputAttrTableName   Theme table with samples
		\param sampleColumnName     Table column with sample values
		\param outputRaster         A pointer to a raster where interpolated data will be stored: this raster mus already exist
		\param band                 The band to be interpolated
		\param algorithm		    Type of algorithm to be used: see enum TeInterpolationAlgorithm
		\param numberOfNeighbors    Number of nearest neighbor to be used in interpolation
		\param powValue			    Pow parameter for inverse distance function
		\param boxRatio				Ratio of the box used to search samples
		\return TRUE if there isn't an error during interpolation otherwise return FALSE
 */
TLFUNCTIONS_DLL bool TeRasterInterpolate(TeTheme* inputTheme, const string& inputAttrTableName, const string& sampleColumnName,
                         TeRaster* outputRaster, const int& band = 0,
				         const TeInterpolationAlgorithm& algorithm = TeNNInterpolation,
                         const unsigned int& numberOfNeighbors = 1, const int& powValue = 1, const double& boxRatio = 1.0);

		

/** @} */ 




#endif	// __TERRALIB_INTERNAL_INTERPOLATION_H




