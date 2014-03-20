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
/*! \file TeKMeansGrouping.h
    \brief This file contains functions related to the K Means clustering algorithm
*/
#ifndef  __TERRALIB_INTERNAL_KMEANSGROUPING_H
#define  __TERRALIB_INTERNAL_KMEANSGROUPING_H

#include "TeGroupingAlgorithms.h"

template<typename I, typename T>  int
TeMinimumDistance(I it, T& centroides )
{
	double distmin = TeMAXFLOAT, dist; 
	int j = -1;
	for (int i=0; i< centroides.size(); i++)
	{
	     dist = TeEuclidianDistance(it,centroides[i]);
	     if ( dist < distmin )
	     {  
			 j = i; 
			 distmin = dist; 
		 }
	}
	return j;
}

template<typename I, typename T> 
double
TeEuclidianDistance(I it, T& centroide)
{
	double val = 0;
	int n = centroide.size();
	for (int i=0; i<n; i++)
	{
		double v1 = (*it)[i];
		double v2 = centroide[i];
		val += pow(v1-v2,2);
	}
	return sqrt(val);
}


template<typename T> 
void
Initialize(T& centroide, vector<double>& minValue, vector<double>& maxValue)
{
	int n = centroide.size();
	for (int i=0; i<n; i++)
	{
		double val = (rand()* (maxValue[i]-minValue[i]))/RAND_MAX;
		centroide[i] = val;
	}
	return;
}

template<typename T> 
bool 
CheckConvergence(T& toUpdate, T& newCentroides, double limiar )
{

	int k = newCentroides.size();
	int n = newCentroides[0].size();
	for (int i=0; i<k; i++)
		for (int j = 0; j<n; j++)
		{
			double num = newCentroides[i][j];
			double den = toUpdate[i][j];
/*			if (num > den)
			{
				double aux = den;
				den = num;
				num = aux;
			}
			val = (1-num/den)*100.;
			if (val > limiar)
				return false;
*/
			if (num != den)
				return false;
		}
	return true;
}

template<typename It, typename C> 
void
ReCalculate(It it, C& nearest)
{
	for (int i=0; i<nearest.size(); i++)
		nearest[i] = (nearest[i] + (*it)[i])/2.0;
	return;
}


template<typename It, typename C> void
TeKMeans (It begin, It end, C& result, double conv) 
{
	srand( static_cast<unsigned>(time(NULL)));
	C centroides(result), centr_aux(result);
    
	int n = result[0].size();
	vector<double> minValue(n);
	vector<double> maxValue(n);

	TeMinimumValue(begin, end, minValue);
	TeMaximumValue(begin, end, maxValue);

	for (int i=0; i<result.size();  i++)
	{
		// Falta verificar se não gera centroides iniciais iguais
		Initialize(centroides[i], minValue, maxValue);
	}

	centr_aux = centroides;
	bool converge = false;
	int  j, niter=0;

	while (!converge)
	{
		for (It it=begin; it!= end; it++) 
		{
			j = TeMinimumDistance(it, centroides);
			ReCalculate(it,centr_aux[j]);
		}
		converge = CheckConvergence(centroides, centr_aux,conv);
		centroides = centr_aux;
		niter++;
   }
   result = centroides;
}

template<typename ItIn, typename C, typename ItOut> void
TeEuclidianClassify (ItIn begin, ItIn end, ItOut& result, C& kmeans) 
{
	int j;
	for (ItIn it=begin; it!= end; it++) 
	{
		j = TeMinimumDistance(it, kmeans);
		(*result) = j;
		result++;
	}
}
#endif