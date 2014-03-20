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
/*! \file TeBayesFunctions.h
    \brief This file contains functions to calculate Bayesian Indexes 
*/

#ifndef __TEBAYESFUNCTIONSH__
#define __TEBAYESFUNCTIONSH__

#include "TeStatDataStructures.h"
#include "../kernel/TeSTElementSet.h"
#include "../kernel/TeNeighbours.h"

/** @defgroup STEBayesEstimation Bayes Estimation from TerraLib's spatio-temporal containers
 *  @ingroup SpatialStatistics
 *  @{
*/
//! Error codes
enum TeGlobalBayesError {
   BAYES_GREATER_CASES,
   BAYES_NULL_POPULATION
};

//! A class of exceptions
class STAT_DLL TeGlobalBayesException  {
   TeGlobalBayesError error_;

public:

  TeGlobalBayesException(TeGlobalBayesError e) {
    error_ = e;
  }

  TeGlobalBayesError getErrorCode() {
    return error_;
  }
};


//! Calculates the Global Empirical Bayes 
STAT_DLL bool TeGlobalEmpiricalBayes(TeSTStatInstanceSet *rs, double rate);

/* Taxa Bayesiana Local -- recebe conjunto de areas, cria matriz de vizinhanca,
   e estima taxa local baseado na vizinhanca.

   As duas primeiras propriedades das areas sao populacao e casos.
   Ha mais duas propriedades double onde serao armazenados total de casos e total
   de populacao dos vizinhos.

  teta[i]=w[i]*r[i]+(1-w[i])*c[i]
  w[i]=V/(V+r[i]))
  V=Sum(Area i,Pop[i]*(ri-m)^2)/Sum(Area i,Pop[i])-m/PopMedia
*/
/** Calculates the local Empirical Bayes	*/
template<typename Matrix> bool 
TeLocalEmpiricalBayes(TeSTElementSet *rs, Matrix* neighMatrix, string& colname, double rate);
/** @} */

//Implementation 
template<typename Matrix> bool 
TeLocalEmpiricalBayes(TeSTElementSet *rs, Matrix* neighMatrix, string& colname, double rate)
{
	//Percorre areas, calculando populacao e contagem total dos vizinhos
	typename TeSTElementSet::iterator it = rs->begin(); 
	double 	neiPop,  //Somatorio da populacao de uma area e seus vizinhos
		neiCases; //Somatorio de casos de uma area e seus vizinhos
	double 	myPop,    //Populacao de uma area
		myCases;  //Casos de uma area

	std::string val;

	//Adds a new property in the element set
	TeAttribute r;
	r.rep_.name_ = "neiPop";
	r.rep_.type_ = TeREAL;
	r.rep_.decimals_ = 15;
    	rs->addProperty(r);
	r.rep_.name_ = "neiCases";
	rs->addProperty(r);

	//! Gets the indexes of the added attributes
	int indexAttrPop = rs->getAttributeIndex("neiPop");
	int indexAttrCases = rs->getAttributeIndex("neiCases");
   
	//Calcula numero de casos e populacao de uma area e seus vizinhos para todas as 
   	//areas --> propriedades neiPop e neiCases
    	while ( it != rs->end()) 
	{
		
		//Inicializa total de casos e de populacao
		(*it).getPropertyValue(val, 0);
		myPop = atof(val.c_str());
		neiPop = myPop;
		(*it).getPropertyValue(val, 1);
		neiCases = atof(val.c_str());

		TeNeighboursMap  neighbors = neighMatrix->getMapNeighbours((*it).objectId());
		if ((myPop > 0)  && (neighbors.size() > 0)) 
		{
			typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
			//Total de casos e de populacao
			while(itNeigs != neighbors.end())
			{
			//Pega numero de casos e de populacao
			rs->getAttributeValue((*itNeigs).first, 0, val);
			neiPop += atof(val.c_str());
			rs->getAttributeValue((*itNeigs).first, 1, val);
			neiCases += atof(val.c_str());
			(++itNeigs);
			}
        	}

        	(*it).addPropertyValue(Te2String(neiPop, 9));
		(*it).addPropertyValue(Te2String(neiCases, 9));
        	(++it);
	}

	double 	m, //taxa de uma area e seus vizinhos
		variance, //variancia de uma area e seus vizinhos
		V, W, LocalBayesRate;
	double 	neiJPop, //Populacao de uma area vizinha
		neiJCases; //Casos de uma area vizinha

	// adds rate property
	r.rep_.name_ = colname;
	r.rep_.type_ = TeREAL;
	r.rep_.decimals_ = 15;
    	rs->addProperty(r);
	it = rs->begin(); 
	//Calcula taxa corrigida local para cada area
   	 while ( it != rs->end())
	{
		//Pega numero de casos e de populacao
		(*it).getPropertyValue(val, 0);
		myPop = atof(val.c_str());
		(*it).getPropertyValue(val, 1);
		myCases = atof(val.c_str());
	
		(*it).getPropertyValue(val, indexAttrPop);
		neiPop = atof(val.c_str());
		(*it).getPropertyValue(val, indexAttrCases);
		neiCases = atof(val.c_str());

        	if (neiPop <= 0) 
			return false;
		m = (double)neiCases/(double)neiPop;

        	TeNeighboursMap  neighbors = neighMatrix->getMapNeighbours((*it).objectId());
	        if (neighbors.size() == 0) 
		{
            		LocalBayesRate = (myPop > 0) ? (double)myCases/(double)myPop : 0.0;
        	}
        	else 
		{
			//Calculo da variancia
			typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
			variance = (double)myPop * pow((double)myCases/(double)myPop - m,2);
			//Total de casos e de populacao
			while(itNeigs != neighbors.end())
			{
				//Pega numero de casos e de populacao
				rs->getAttributeValue((*itNeigs).first, 0, val);
				neiJPop = atof(val.c_str());
				rs->getAttributeValue((*itNeigs).first, 1, val);
				neiJCases = atof(val.c_str());
			
				variance += (double)neiJPop * pow((double)neiJCases/(double)neiJPop - m,2);
				(++itNeigs);
			}
        		variance /= (double) neiPop;
			//Calculo da taxa
			V = variance - (m * ((double)neighbors.size()+1)/(double)neiPop);
			if (V < 0) 
			V = 0;
			if ((V == 0) && (m == 0))
			W  = 1;
			else 
			W = V/(V + (m/(double)myPop));
			LocalBayesRate = W * (double)myCases/(double)myPop + (1 - W) * m;

    		}
    		LocalBayesRate *= rate;
		(*it).addPropertyValue(Te2String(LocalBayesRate, 9));
    		(++it);
  	}
	return true;
}
#endif
