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

/*! \file TeTreeMinimum.h
    This file contains functions related to the Tree Minimum clustering algorithm
*/
#ifndef  __TERRALIB_INTERNAL_TREEMINIMUM_H
#define  __TERRALIB_INTERNAL_TREEMINIMUM_H

#include <vector>
#include <map>
#include <string>

class TeSTElementSet;
class TeGeneralizedProxMatrix;

using namespace std;

class TeMSTGraphLink
{
public:
	string GeoId1_;
	string GeoId2_;
	double Dissimilarity_;

	~TeMSTGraphLink()
	{}


	TeMSTGraphLink() :
		GeoId1_(""),
		GeoId2_("")
	{}

	TeMSTGraphLink(const string& GeoId1, const string& GeoId2):
		GeoId1_(GeoId1),
		GeoId2_(GeoId2)
	{}

	TeMSTGraphLink(const string& GeoId1, const string& GeoId2, const double Dissimilarity)  :
		GeoId1_(GeoId1),
		GeoId2_(GeoId2),
		Dissimilarity_(Dissimilarity)
	{}

 
	void set_Dissimilatiry (const double Dissimilarity)
	{	Dissimilarity_ = Dissimilarity;	}

	bool compare_dissimilarity(const TeMSTGraphLink& graph_link1, const TeMSTGraphLink& graph_link2);
};



typedef vector<TeMSTGraphLink> TeMSTGraph;

typedef map<string,TeMSTGraph> TeMSTMatrix;

//! Calcula a dissimilaridade entre os vizinhos e gera um grafo de dissimilaridade 
void TeCreateDissMatrix (TeSTElementSet* Objects, TeGeneralizedProxMatrix* proxMatrix, TeMSTMatrix& mat_diss, double& Diss_max);

//! Gera a árvore mínima
void TeCreateMinimumTree (TeSTElementSet* Objects, TeMSTMatrix& DissMatrix, TeMSTGraph& Min_Tree, double Diss_max);

//! Divide a árvore mínima em sub árvores regionalizadas
void TeCreateSubTrees(TeSTElementSet* objects, TeMSTGraph& Min_Tree, int num_regioes);

#endif