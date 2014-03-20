/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeSpatialStatistics.h
    \brief This file provides support for spatial statistics functions
*/
#ifndef  __TERRALIB_INTERNAL_SPATIALSTATISTICS_H
#define  __TERRALIB_INTERNAL_SPATIALSTATISTICS_H

#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

#include "../kernel/TeStatistics.h"
#include "../kernel/TeProgress.h"
#include "../kernel/TeSTEvent.h"
#include "../kernel/TeNeighbours.h"
 
/** @defgroup SpatialStatistics Spatial Statistics Algorithms
 *  TerraLib spatial statistics algorithms.
 *  @{
*/

/** Calculates the local mean of an attribute of and the number of neighbors for a set of objects
	\param elemSet		a set of objects  
	\param proxMatrix	the proximity matrix of this object set  
	\param indexAttr	the position in the objects vector of properties that contains the requested attribute 
	\return the function adds two properties to the objects: "Local Mean" and "NumNeighbors" and returns true of false
*/
template<typename Set, typename Matrix> 
bool 
TeLocalMean (Set* elemSet, Matrix* proxMatrix, int indexAttr=0); 

/** Calculates the standard deviation of an attribute and the local mean of the standard deviation  for a set of objects
	\param elemSet		a set of objects  
	\param proxMatrix	the proximity matrix of this object set  
	\param indexAttr	the position in the objects vector of properties that contains the requested attribute 
	\return the function adds two properties to the objects: "Z" and "WZ" and returns true of false
*/
template<typename Set, typename Matrix>  
bool 
TeZAndWZ (Set* elemSet, Matrix* proxMatrix, int indexAttr);

/** @defgroup TeMoranIndex Moran Index Algorithm
 *  @ingroup SpatialStatistics
 *  @{
*/

/** Calculates the Global Moran index of an attribute  for a set of objects 
	\param elemSet		a set of objects 
	\param mean			the global mean for the choosen the atrribute
	\param var			the variance for the choosen the atrribute
	\param proxMatrix	the proximity matrix of this object set  
	\param indexAttr	the position in the objects vector of properties that contains the requested attribute  
	\return the Global Moran index
*/
template<typename Set, typename Matrix> 
double
TeMoranIndex2 (Set* elemSet, const double& mean, const double& var, Matrix* proxMatrix, int indexAttr=0); 

/** Calculates the Local and the Global Moran index of an attribute for a set of objects
	\param elemSet		a set of objects 
	\param indexZ		the position in the objects vector of properties that contains the deviations (Z)    
	\param indexWZ		the position in the objects vector of properties that contains the local mean deviations (WZ) 
	\return the function adds one property to the objects: "MoranIndex" and returns the Global Moran index
*/
template<typename Set> 
double
TeMoranIndex (Set* elemSet, int indexZ, int indexWZ); 


/** Calculates the Global Moran significance of an attribute  for a set of objects  
	\param elemSet				a set of objects  
	\param proxMatrix			the proximity matrix of this object set   
	\param indexAttr			the position in the objects vector of properties that contains the deviations (Z)    
	\param permutationsNumber	number of the permutations used to calculate the significance
	\param moranIndex			the Global Moran index 
	\return the Global Moran significance
*/
template<typename Set, typename Matrix> 
double
TeGlobalMoranSignificance (Set* elemSet, Matrix* proxMatrix, int indexAttr,
						   unsigned int permutationsNumber, double moranIndex); 


/** Classifies the objects based in the scatterplot of Moran index and its statistical significance
	\param elemSet		a set of objects 
	\param indexLISAMap	the position in the objects vector of properties that contains the statistical significances of the moran local indexes (LISA) 
	\param indexBoxMap	the position in the objects vector of properties that contains the classifications of the statistical significance of the moran local indexes 
	\return the function adds one property to the objects: "MoranMap"
*/
template<typename Set> 
void
TeMoranMap (Set* elemSet, int indexLISAMap, int indexBoxMap); 

/** @} */ 

/** Calculates the G and G* statistics of an attribute  for a set of objects 
	\param elemSet		a set of objects 
	\param proxMatrix	the proximity matrix of this object set      
	\param indexAttr	the position in the objects vector of properties that contains the requested attribute  
	\return the function adds two properties to the objects: "G" and "GStar"
*/
template<typename Set, typename Matrix> bool 
TeGStatistics (Set* elemSet, Matrix* proxMatrix, int indexAttr=0 ); 


/** Classifies the objects in quadrants based in the scatterplot of moran index 
	\param elemSet		a set of objects 
	\param indexZ		the position in the objects vector of properties that contains the deviations (Z)    
	\param indexWZ		the position in the objects vector of properties that contains the local mean deviations (WZ) 
	\param mean			the global mean
	\return the function adds one property to the objects: "BoxMap"
*/
template<typename Set> void
TeBoxMap (Set* elemSet, int indexZ, int indexWZ, double mean); 

/** Evaluates the statistical significance of the moran local indexes (LISA) 
	\param elemSet		a set of objects 
	\param indexZ		the position in the objects vector of properties that contains the deviations (Z)    
	\param indexLISA	the position in the objects vector of properties that contains the local moran indexes (LISA) 
	\param indexNeighNum		the position in the objects vector of properties that contains the the numbers of neighbours of an object  
	\param permutationsNumber	number of the permutations used to evaluate the significance
	\return the function adds one property to the objects: "LISASig"
*/
template<typename Set> bool 
TeLisaStatisticalSignificance ( Set* elemSet, int indexZ, int indexLISA, int indexNeighNum, int permutationsNumber);  


/** Classifies the objects based in the statistical significance of the moran local indexes (LISA) 
	\param elemSet				a set of objects  
	\param indexSignifLISA		the position in the objects vector of properties that contains the statistical significances of the local moran indexes (LISA) 
	\param permutationNumber	number of the permutations used to evaluate the significance
	\return the function adds one property to the objects: "LISAMap"
*/
template<typename Set> void
TeLisaMap (Set* elemSet, int indexSignifLISA, int /* permutationNumber */); 

/** @defgroup ContBayesEstimation Bayes Estimation from general containers
 *  @ingroup SpatialStatistics
 *  @{
*/
/** Calculates the empirical Bayes estimation from a container of values */
template <typename It> void
TeEmpiricalBayes (It rBegin, It rEnd, It n, double mean, double variance, It bayes)
{
	double w= 0.;
	while (rBegin != rEnd)
	{
		w = variance / ( variance + ( mean/(*n)) );
		*bayes = w*(*rBegin) + (1-w)*mean;
		rBegin++; n++; bayes++;
	}
}

//* Calculates the Local Bayes estimation from a container of values */
template <typename It> void
TeSpatialEmpiricalBayes (It rBegin, It rEnd, It n, It mean, It variance, It bayes)
{
	double w = 0.;
	while (rBegin != rEnd)
	{
		w = (*variance)/( (*variance) + ( (*mean)/n ));	
		*bayes = w*(*rBegin) + (1-w)*(*mean);
		rBegin++; n++; mean++; variance++; bayes++;
	}
}
  
/** @} */ 
/** @} */ 

// --- Template function implementation ---
template<typename Set, typename Matrix>  bool 
TeLocalMean (Set* elemSet, Matrix* proxMatrix, int indexAttr)
{
	typename Set::iterator itObjs = elemSet->begin();
	int numberNeighbors;

	if(!proxMatrix->getWeightsParams())
		return false;
	bool isNorm = proxMatrix->getWeightsParams()->norm_;

	//adds the attributes in the set
	TeAttribute att;
	att.rep_.type_ = TeREAL;
	att.rep_.decimals_ = 15;
	att.rep_.name_ = "LocalMean";

	elemSet->addProperty(att);
	
	att.rep_.name_  = "NumNeighbors";
	att.rep_.type_ = TeINT;
	elemSet->addProperty(att);

	while (itObjs != elemSet->end())
	{
		double sum = 0.;
		double localMean = 0.;

		TeNeighboursMap neighbors = proxMatrix->getMapNeighbours((*itObjs).objectId());
		typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
		numberNeighbors = neighbors.size();
		
		while(itNeigs != neighbors.end())
		{
			//retrieve the neighbor attribute value
			string valAux;
			string objIdAux = (*itNeigs).first;
			if(!elemSet->getAttributeValue(objIdAux, indexAttr, valAux))
				return false;
			double val = atof(valAux.c_str());
						
			// find the weight associated with the neighbor (attribute of index 0)
			double weight = (*itNeigs).second.Weight();
			//verify if the weight is normalized by number of neighbours
			if(!isNorm)
				weight = weight/numberNeighbors;
			sum +=  weight * val;  
			++itNeigs;
		}

		localMean = sum;
		
		(*itObjs).addPropertyValue(Te2String(localMean,9));
		(*itObjs).addPropertyValue(Te2String(numberNeighbors)); 
		
		++itObjs;
	}

	return true;
}

template<typename Set, typename Matrix>  
bool 
TeZAndWZ (Set* elemSet, Matrix* proxMatrix, int indexAttr)
{
	// calculate the standard deviation Z
	double mean = TeFirstMoment (elemSet->begin(), elemSet->end(), 0); 

	TeAttribute att;
	att.rep_.type_ = TeREAL;
	att.rep_.decimals_ = 15;
	att.rep_.name_ = "Z";
	elemSet->addProperty(att);

	typename Set::iterator it = elemSet->begin();
	while (it != elemSet->end())
	{
		double val = (*it)[indexAttr];
		(*it).addPropertyValue(Te2String((val-mean), 9)); 
		++it;
	}

	int indexZ = elemSet->getAttributeList().size()-1;

	// calculate the local mean of Z (WZ)
	att.rep_.name_ = "WZ";
	att.rep_.type_ = TeREAL;
	elemSet->addProperty(att);
	
	typename Set::iterator itObjs = elemSet->begin();
	int numberNeighbors;

	if(!proxMatrix->getWeightsParams())
		return false;
	bool isNorm = proxMatrix->getWeightsParams()->norm_;

	while (itObjs != elemSet->end())
	{
		double sum = 0.;
		TeNeighboursMap neighbors = proxMatrix->getMapNeighbours((*itObjs).objectId());
		typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
		numberNeighbors = neighbors.size();
		
		while(itNeigs != neighbors.end())
		{
			//retrieve the neighbor attribute value
			string valAux;
			string objIdAux = (*itNeigs).first;
			if(!elemSet->getAttributeValue(objIdAux, indexZ, valAux))
				return false;
			double val = atof(valAux.c_str());
						
			// find the weight associated with the neighbor (attribute of index 0)
			double weight = (*itNeigs).second.Weight();
			//verify if the weight is normalized by number of neighbours
			if(!isNorm)
				weight = weight/numberNeighbors;
			sum +=  weight * val;  
			++itNeigs;
		}
		(*itObjs).addPropertyValue(Te2String(sum,9));	
		++itObjs;
	}
	return true;
}

template<typename Set, typename Matrix> double
TeMoranIndex2 (Set* elemSet, const double& mean, const double& var, Matrix* proxMatrix, int indexAttr)
{
	double moran = 0;
	typename Set::iterator itObjs = elemSet->begin();
	int numberObjs = elemSet->numSTInstance();
	if(!proxMatrix->getWeightsParams())
		return false;
	bool isNorm = proxMatrix->getWeightsParams()->norm_;

	while ( itObjs != elemSet->end())
	{
		double normObjVal = (*itObjs)[indexAttr] - mean;
		double li = 0.;
		double weightSum = 0.;

		
		TeNeighboursMap neighbors = proxMatrix->getMapNeighbours((*itObjs).objectId());
		typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
		int numberNeighbors = neighbors.size();

		while(itNeigs != neighbors.end())
		{
			//retrieve the neighbor attribute value
			string valAux; 
			string objIdAux = (*itNeigs).first;
			if(!elemSet->getAttributeValue (objIdAux, indexAttr, valAux))
				return 0.;
			double val = atof(valAux.c_str());	

			 // normalize the property
			double normNeighVal =  val - mean;

			// find the weight associated with the neighbor (attribute of index 0)
			double weight = (*itNeigs).second.Weight();
			//verify if the weight is normalized by number of neighbours
			if(!isNorm)
				weight = weight/numberNeighbors;
			li +=  weight * ( normNeighVal )  * ( normObjVal );  // se dividir pela vari�ncia � o �ndice local de moran
			weightSum += weight;
			++itNeigs;
		}

		if (weightSum != 0.)
			li /= weightSum;
		
		moran += li;

		++itObjs;
	}

	if ( numberObjs > 1 )
		return moran /  ( var * ( numberObjs - 1 ) );
	else 
		return moran / var;
}

template<typename Set> double
TeMoranIndex (Set* elemSet, int indexZ, int indexWZ)
{
	double variance = 0.;
	double sum = 0;
	int number = 0;

	variance = TeSecondMoment (elemSet->begin(), elemSet->end(), 0, indexZ);  
	typename Set::iterator it = elemSet->begin();

	//keep 
	TeAttribute att;
	att.rep_.type_ = TeREAL;
	att.rep_.decimals_ = 15;
	att.rep_.name_ = "MoranIndex"; 
	elemSet->addProperty(att); 
	
	while (it != elemSet->end())
	{
		double z = (*it)[indexZ];
		double wz = (*it)[indexWZ];
		double ZxWZ;
		if(variance==0)
			ZxWZ = 0.;
		else
			ZxWZ = (z*wz)/variance; 

		//keep 
		(*it).addPropertyValue(Te2String(ZxWZ, 9)); 
		
		sum += ZxWZ;
		number++; 
		++it; 
	}
	
	return sum /= number;
}

template<typename Set, typename Matrix> double
TeGlobalMoranSignificance (Set* elemSet, Matrix* proxMatrix, int indexAttr,
						   unsigned int permutationsNumber, double moranIndex)
{
	
	vector<double> permutationsResults(permutationsNumber);
	double significance;
	
	unsigned int objectsNumber = elemSet->numSTInstance();
	
	//progress bar
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(permutationsNumber);
		
	vector<double> deviations(objectsNumber);
	vector<double>::iterator itDev = deviations.begin();
	typename Set::iterator itObjs = elemSet->begin();
	while(itObjs != elemSet->end())
	{
		(*itDev) = (*itObjs)[indexAttr]; //attribute value
		++itObjs;
		++itDev;
	}

	TeStatisticValMap stat;
	if(!TeCalculateStatistics(elemSet->begin(), elemSet->end(), stat, 0))
		return 0.;

	double mean = stat[TeMEAN];
	double var = stat[TeVARIANCE];

	srand(time(0));	
	unsigned int i;
	for (i = 0; i < permutationsNumber; i++)
	{
		Set changedObjects;
		if(elemSet->getTheme())
			changedObjects.setTheme(elemSet->getTheme());
		else if(elemSet->getLayer())
			changedObjects.setLayer(elemSet->getLayer());

		changedObjects.setAttributeList(elemSet->getAttributeList());

		itObjs = elemSet->begin();

		// chande values 
		while(itObjs != elemSet->end())
		{
			TeSTEvent stoChange;
			stoChange.objectId((*itObjs).objectId());

			double ranaux = rand();
			int randon = (int) ((ranaux * (objectsNumber-1))/RAND_MAX);
			double value = deviations[randon];
			stoChange.addPropertyValue(Te2String(value,9));
			changedObjects.insertSTInstance (stoChange);
			++itObjs;
		}
		
		permutationsResults[i] = TeMoranIndex2 (&changedObjects, mean, var, proxMatrix, 0);

		if(TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return 0.;
			}
			else
				TeProgress::instance()->setProgress(i);
		}
	}

	// verify the significance
	int position = 0;
	significance = 0;
	unsigned int k;
	for (k = 0; k < permutationsNumber; k++)
	{
		if (moranIndex < permutationsResults[k]) //>
			position++;
	}
	if ( moranIndex >= 0)
		significance = (double) (position+1) / (permutationsNumber+1);
	else
		significance = (double) (permutationsNumber-position)/(permutationsNumber+1);
		
	
	if (TeProgress::instance())
		TeProgress::instance()->reset();
		
	return significance;
}


template<typename Set>  void
TeMoranMap (Set* elemSet, int indexLISAMap, int indexBoxMap)
{
	//keep
	TeAttribute att;
	att.rep_.decimals_ = 15;
	att.rep_.type_ = TeREAL;
	att.rep_.name_ = "MoranMap"; 
	elemSet->addProperty(att); 

	typename Set::iterator it = elemSet->begin();
	while ( it != elemSet->end())
	{
		double result = 0.0;
		int lisaMap = (int)((*it)[indexLISAMap]);

		if (lisaMap != 0)
			result = (*it)[indexBoxMap];
				
		//keep
		(*it).addPropertyValue(Te2String(result, 9)); 

		++it;
	}
}

template<typename Set, typename Matrix>  bool 
TeGStatistics (Set* elemSet, Matrix* proxMatrix, int indexAttr)
{

	typename Set::iterator itObjs = elemSet->begin();
	double totalSum = 0.;
	double excludSum = 0.;

	totalSum = TeSum(itObjs, elemSet->end(), indexAttr);
	
	itObjs = elemSet->begin();

	//progress bar
	int step = 0;
	int numSteps = elemSet->numSTInstance();
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(numSteps);

	//adds the index attributes in the sET
	TeAttribute att;
	att.rep_.name_ = "G"; 
	att.rep_.type_ = TeREAL;
	att.rep_.decimals_ = 15;
	elemSet->addProperty(att); 
	
	att.rep_.name_ = "GStar";
	elemSet->addProperty(att); 
	
	while ( itObjs != elemSet->end() )
	{
		double valObj = (*itObjs)[indexAttr];
		excludSum = totalSum - valObj;
		
		double G = 0;
		double GStar = valObj;
	
		TeNeighboursMap neighbors = proxMatrix->getMapNeighbours((*itObjs).objectId());
		typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
		int neigNumber = neighbors.size();

		if(itNeigs != neighbors.end())
		{
			while(itNeigs != neighbors.end())
			{
				// retrieve the property value associated to this neighbor 
				double val;
				string valAux;
				string objIdAux = (*itNeigs).first;
				if(!elemSet->getAttributeValue (objIdAux, indexAttr, valAux))
					return false;
				val = atof(valAux.c_str());
				G +=  val;
				GStar +=  val;
				
				++itNeigs;
			}

			G /= neigNumber;
			GStar /= (neigNumber+1);
				
			G /= excludSum; 
			GStar /= totalSum;
		}
	
		//insert the index G in the set
		(*itObjs).addPropertyValue(Te2String(G, 9)); 
		//insert the index GStar in the set
		(*itObjs).addPropertyValue(Te2String(GStar,9)); 

		++itObjs;

		if(TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			else
				TeProgress::instance()->setProgress(step);
		}	
		++step;
	}
	

	if (TeProgress::instance())
		TeProgress::instance()->reset();
	return true;
}

template<typename Set>  void
TeBoxMap (Set* elemSet, int indexZ, int indexWZ, double mean)
{	
	//keep in the set
	TeAttribute att;
	att.rep_.name_ = "BoxMap"; 
	att.rep_.type_ = TeINT;
	elemSet->addProperty(att);

	typename Set::iterator it_begin = elemSet->begin(); 
	while ( it_begin != elemSet->end())
	{
		int result=0;

		if ( ((*it_begin)[indexZ] >= mean) &&  ((*it_begin)[indexWZ] >= mean))   
			result = 1;
		else if ( ((*it_begin)[indexZ] < mean)  &&  ((*it_begin)[indexWZ] >= mean))	
			result = 4;	
		else if ( ((*it_begin)[indexZ] < mean)  &&  ((*it_begin)[indexWZ] < mean))
			result = 2;		
		else if ( ((*it_begin)[indexZ] >= mean) &&  ((*it_begin)[indexWZ] < mean))
			result = 3;

		(*it_begin).addPropertyValue(Te2String(result)); 

		++it_begin;
	}	
}

template<typename Set>  bool
TeLisaStatisticalSignificance (Set* elemSet, int indexZ, int indexLISA, 
									 int indexNeighNum, int permutationsNumber) 
 {
	double sum;
	double WZperm;
	double significance;

	typename Set::iterator it = elemSet->begin();

	double variance = TeSecondMoment (it, elemSet->end(), 0, indexZ);

	int index = 0;
	it = elemSet->begin();
	int objectsNumber = elemSet->numSTInstance();

	//progress bar
	int step = 0;
	if(TeProgress::instance())
		TeProgress::instance()->setTotalSteps(objectsNumber);

	vector<double> deviations(objectsNumber);
	vector<double>::iterator it_dev = deviations.begin();
	while(it != elemSet->end())
	{
		(*it_dev) = (*it)[indexZ];
		++it;
		++it_dev;
	}

	it = elemSet->begin();
	srand(time(0));

	//keep
	TeAttribute att;
	att.rep_.name_ = "LISASig"; 
	att.rep_.type_ = TeREAL;
	att.rep_.decimals_ = 15;
	elemSet->addProperty(att); 

	while ( it != elemSet->end())
	{
		int neighborsNumber = (int)((*it)[indexNeighNum]);

		vector<double> permut(permutationsNumber);
		
		for (int j = 0; j < permutationsNumber; j++)
		{
			int randon = 0;
			sum = 0;
			set<int> setNeigh;
			for (int k = 0; k < neighborsNumber; k++)
			{
				double ranaux = rand();
				randon = (int)((ranaux * (objectsNumber-1))/RAND_MAX);
				if (index == randon || (setNeigh.find(randon) != setNeigh.end()) )
					k--;	// raffle annulled	
				else
				{		
					setNeigh.insert(randon);
					sum += deviations[randon];
				}
			}

			if(neighborsNumber==0)
				WZperm = 0.;
			else
				WZperm = sum/neighborsNumber;

			if(variance==0)
				permut[j] = 0.;
			else
				permut[j] = deviations[index] * WZperm / variance;
		}	

		int position = 0;
		for (int k = 0; k < permutationsNumber; k++)
		{
			if (((*it)[indexLISA]) > permut[k])
				position++;
		}
 		if ( ((*it)[indexLISA]) >= 0)
			significance = (double) (permutationsNumber-position)/(permutationsNumber+1);
		else
			significance = (double) position/( permutationsNumber + 1 );
			
		(*it).addPropertyValue(Te2String(significance, 9)); 

		++it;
		++index;

		if(TeProgress::instance())
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				return false;
			}
			else
				TeProgress::instance()->setProgress(step);
		}	
		++step;
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}


template<typename Set> void 
TeLisaMap (Set* elemSet, int indexSignifLISA, int /* permutationNumber */)
{
	//keep
	TeAttribute att;
	att.rep_.name_ = "LISAMap"; 
	att.rep_.type_ = TeINT;
	elemSet->addProperty(att); 

	typename Set::iterator begin = elemSet->begin();
	while (begin != elemSet->end())
	{
		int significanceClass = 0;
		if ( ( (*begin)[indexSignifLISA] <= 0.001 ))  /*&& (permutationNumber >= 999)*/ 
			significanceClass = 3;
		else if ( ((*begin)[indexSignifLISA] <= 0.01) && ((*begin)[indexSignifLISA] > 0.001 )) /*&& (permutationNumber >= 99)*/ 
			significanceClass = 2;
		else if ( ((*begin)[indexSignifLISA] <= 0.05) && ((*begin)[indexSignifLISA] > 0.01))
			significanceClass = 1;

		//keep
		(*begin).addPropertyValue(Te2String(significanceClass));

		++begin;
	}
}

/*! \example proxMatrixAndSpatialStatistics.cpp
	Shows how to create a proximity matrix from a Spatial Temporal Element Set (STElementSet) and calculate spatial statistics
   */
#endif

