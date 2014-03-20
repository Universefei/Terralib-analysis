/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.

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
#include <math.h>
#include "TeTreeMinimum.h"
#include "../kernel/TeSTElementSet.h"
#include "../kernel/TeGeneralizedProxMatrix.h"

bool 
compare_dissimilarity(const TeMSTGraphLink &graph_link1, const TeMSTGraphLink &graph_link2)
{
	if (graph_link1.Dissimilarity_ < graph_link2.Dissimilarity_) 
		return true;
	else
		return false;
}

void TeCreateDissMatrix (TeSTElementSet* objects, TeGeneralizedProxMatrix* proxMatrix,
					   TeMSTMatrix& mat_diss, double& Diss_max)
{
	
	string objectId1, objectId2;
	TeNeighboursMap neighbors; 
	int num_neighbors;
	double val1, val2;
	int DISS_TYPE = 1;
	double Diss = 0.;	
	Diss_max = 0.;	
	TeMSTGraph graph_;
	int tamanho = 0;

	TeSTElementSet::iterator it = objects->begin();
	while ( it != objects->end() )
	{
		it->slice(0);
		objectId1 = it->objectId();
		
		neighbors = proxMatrix->getMapNeighbours(objectId1);
		TeNeighboursMap::iterator itNeigs = neighbors.begin();
		num_neighbors = neighbors.size();
		graph_.clear();
		tamanho = graph_.size();
		double difer = 0.;
		while(itNeigs != neighbors.end())
		{
			objectId2 = itNeigs->first;
			Diss = 0.; 
			TePropertyVector* propVec = objects->getPropertyVector(objectId2);
			if(!propVec)
				continue;

			for(unsigned j=0; j<propVec->size(); ++j)
			{
				val1 = it->operator[](j);
				val2 = atof(propVec->operator[](j).value_.c_str());		
				difer = val1 - val2;
				if (DISS_TYPE)
					Diss+= difer*difer;		// distancia euclidiana (y=1)
				else
					Diss+= double(fabs(difer));		// distancia quarteirao (y=0)
			}
			if (DISS_TYPE)
				Diss = sqrt(Diss);
			
			//	guarda maior valor
			if (Diss > Diss_max)
				Diss_max = Diss;

			//escrever na estrutura map de grafos
			if (objectId1 != objectId2)
				graph_.push_back(TeMSTGraphLink(objectId1,objectId2,Diss));
			
			++itNeigs;
		}
		sort(graph_.begin(), graph_.end(), compare_dissimilarity); 
		mat_diss[objectId1] = graph_;
		++it;
    }
}


void
TeCreateMinimumTree (TeSTElementSet* objects, TeMSTMatrix& DissMatrix, 
					 TeMSTGraph& Min_Tree, double Diss_max)
{
	// GERAR ARVORE MINIMA
	vector<string> nos_MimTree;
	
	// escolha do primeiro n�
	TeSTElementSet::iterator it = objects->begin();
	string objId1 = it->objectId();	
	int num_nos = 0;	//n�mero de n�s na �rvore m�nima

	//armazena o primeiro vizinho do primeiro objeto. Os vizinhos est�o ordenados pela dissimilaridade
	nos_MimTree.push_back(objId1); 
	++num_nos;
	string objId2 = DissMatrix[objId1][0].GeoId2_;  //A matriz possui as linhas ordenadas pela menor dissimilaridade
	nos_MimTree.push_back(objId2);
	++num_nos;

	//Min_Tree: grafo da �rvore m�nima
	TeMSTGraphLink link1(objId1, objId2, DissMatrix[objId1][0].Dissimilarity_);
	Min_Tree.push_back(link1);

	//apaga o vizinho que entrou na �rvore m�nima da matrix
	double Diss_mim;
	TeMSTGraph::iterator it_link = DissMatrix[objId1].begin();
	if(it_link!=DissMatrix[objId1].end())
		DissMatrix[objId1].erase(it_link);	//apaga link j� analisado

	int num_objects = objects->numSTInstance();
	bool no_novo = false;

	//Min_Tree ter� num_objects n�s e num_objects-1 arestas
	while ((num_nos) < num_objects)
	{
		Diss_mim = Diss_max;
		no_novo = false;
		//para cada n� da �rvore m�nima selecione as arestas que ligam a um v�tice ainda n�o existente na 
		//�rvore m�nima
		// e escolha a aresta com menor dissimilaridade para entrar na �rvore m�nima
		for (unsigned int i = 0; i < nos_MimTree.size(); i++)
		{
			string no_atual = nos_MimTree[i];
			unsigned int size = DissMatrix[no_atual].size();
			double diss = DissMatrix[no_atual][0].Dissimilarity_;
			if ((size != 0) && (diss <= Diss_mim))
			{
				objId1 = DissMatrix[nos_MimTree[i]][0].GeoId1_; 
				objId2 = DissMatrix[nos_MimTree[i]][0].GeoId2_;
				Diss_mim = DissMatrix[nos_MimTree[i]][0].Dissimilarity_; 
				no_novo = true;
			}
		}

		if(!no_novo) //n�o achou nenhuma aresta
			num_nos = num_objects;

		// verifica se n� j� pertence � �rvore
		//se objId2 j� existe na �rove n�o podemos considerar essa aresta
		vector<string>::iterator itFind = find(nos_MimTree.begin(), nos_MimTree.end(), objId2);
		if(itFind!=nos_MimTree.end())
			no_novo = false;
				
		// se n� � novo, inclui link na �rvore m�nima
		if (no_novo)
		{
			//nos_MimTree[num_nos++] = objId2;
			nos_MimTree.push_back(objId2);
			++num_nos;
			TeMSTGraphLink link2(objId1, objId2, Diss_mim);
			Min_Tree.push_back(link2);
		}
		
		TeMSTGraph::iterator it_link2 = DissMatrix[objId1].begin();
		if(it_link2!=DissMatrix[objId1].end())
			DissMatrix[objId1].erase(it_link2);	//apaga link j� analisado
	}
}


void
TeCreateSubTrees(TeSTElementSet* objects, TeMSTGraph& Min_Tree, int num_regioes)
{ 
	double	SSA1 = 0., SSA2 = 0., SSTO = 0.;
	TeMSTGraph::iterator it1 = Min_Tree.begin();
	TeMSTGraph::iterator it2 = Min_Tree.begin();
	TeMSTGraph::iterator itMax = Min_Tree.begin();
	
	vector<string> subtree_vertexes_1, subtree1;
	vector<string> subtree_vertexes_2, subtree2;

	int attributes_number = objects->begin()->getPropertyVector().size();

	//estruturas para armazenar estatisticas
	vector <double> sum1(attributes_number, 0.);
	vector <double> sum2(attributes_number, 0.);
	vector <double> total_sum(attributes_number, 0.);
	vector <double> mean1(attributes_number, 0.);
	vector <double> mean2(attributes_number, 0.);
	vector <double> global_mean(attributes_number, 0.);

	//pegar os atributos e colocar em um map
	map<string, vector<double> >  attrObjects;
	TeSTElementSet::iterator attrIt =  objects->begin();
	while(attrIt!=objects->end())
	{
		vector<double> attrs;
		for(int i=0; i<attributes_number; ++i)
			attrs.push_back(atof((*attrIt).getPropertyVector()[i].value_.c_str()));
		attrObjects[(*attrIt).objectId()]=attrs;

		++attrIt;
	}
	
	// h� necessidade de zerar o slice!
	for (int regiao = 1; regiao < num_regioes; regiao++)
	{
		double link_cost = 0.;
		TeMSTGraphLink bigest_cost("","",0.); 
		int contagem = 0;
		it1 = Min_Tree.begin();
		while (it1 != Min_Tree.end())
		{
			TeMSTGraph MT_copy;
			 
			string GeoId1 = it1->GeoId1_;
			string GeoId2 = it1->GeoId2_;
			subtree_vertexes_1.clear();			
			subtree_vertexes_2.clear();

			//estruturas para armazenar as estat�sticas
			fill(sum1.begin(), sum1.end(), 0.);
			fill(sum2.begin(), sum2.end(), 0.);
			fill(total_sum.begin(), total_sum.end(), 0.);
			fill(mean1.begin(), mean1.end(), 0.);
			fill(mean2.begin(), mean2.end(), 0.);
			fill(global_mean.begin(), global_mean.end(), 0.);

			subtree_vertexes_1.push_back(GeoId1);
			subtree_vertexes_2.push_back(GeoId2);
			for (int i = 0; i < attributes_number  ; ++i)
			{
                sum1[i] += attrObjects[GeoId1][i];
				sum2[i] += attrObjects[GeoId2][i];
			}

			string new_vertixe;

			unsigned int actual_vert = 0;
			bool new_v = false;

			//percorre o primeiro vetor
			while (actual_vert < subtree_vertexes_1.size())
			{
				it2 = Min_Tree.begin();
				new_v = false; //verificar a posi��o desse
				while (it2 != Min_Tree.end())
				{
					if ((subtree_vertexes_1[actual_vert] == it2->GeoId1_) && 
						(subtree_vertexes_2[0] != it2->GeoId2_ ))
					{
						new_vertixe = it2->GeoId2_; 
						new_v = true;
					} 
					else if ((subtree_vertexes_1[actual_vert] == it2->GeoId2_) && 
						(subtree_vertexes_2[0] != it2->GeoId1_ ))
					{
						new_vertixe = it2->GeoId1_; 
						new_v = true;
					}
					if ((new_v) && (find(subtree_vertexes_1.begin(), subtree_vertexes_1.end(), new_vertixe) == subtree_vertexes_1.end()))
					{
							subtree_vertexes_1.push_back(new_vertixe);
							//pegar a soma
							for (int i = 0; i < attributes_number  ; ++i)
								sum1[i] += attrObjects[new_vertixe][i];
					}
					++it2;
				}
				actual_vert++;
			}

			actual_vert = 0;
			while (actual_vert < subtree_vertexes_2.size())
			{
				it2 = Min_Tree.begin();
				new_v = false;

				while (it2 != Min_Tree.end())
				{
					if ((subtree_vertexes_2[actual_vert] == it2->GeoId1_) && 
						(subtree_vertexes_1[0] != it2->GeoId2_ ))
					{
						new_vertixe = it2->GeoId2_; new_v = true;
					} else if ((subtree_vertexes_2[actual_vert] == it2->GeoId2_) && 
						(subtree_vertexes_1[0] != it2->GeoId1_ ))
					{
						new_vertixe = it2->GeoId1_; new_v = true;
					}
					if ((new_v) && (find(subtree_vertexes_2.begin(), subtree_vertexes_2.end(), new_vertixe) == subtree_vertexes_2.end()))
					{
							subtree_vertexes_2.push_back(new_vertixe);
							//pegar a soma
							for (int i = 0; i < attributes_number  ; ++i)
								sum2[i] += attrObjects[new_vertixe][i];
					}
					++it2;
				}
				++actual_vert;
			}				

			//-----------------------------------------------------------
			//Calcular o vetor m�dia
			int tam1 = subtree_vertexes_1.size();
			int tam2 = subtree_vertexes_2.size();
			
			for (int k = 0; k < attributes_number ; k++)
				total_sum[k] = sum1[k] + sum2[k];
			
			for (int j = 0; j < attributes_number ; j++)
			{
				 mean1[j] = sum1[j]/tam1; 
				 mean2[j] =	sum2[j]/tam2;
				 global_mean[j] = total_sum[j]/(tam1+tam2);
			}			

			// Calculo do SSTO e SSA's		
			double SSA1 = 0.; SSA2 = 0.; SSTO = 0.; 
			double desvio1 = 0., desvio2 = 0., desvioGlobal = 0.;
			double valor = 0.;
			vector<string>::iterator it_s = subtree_vertexes_1.begin();
			while ( it_s != subtree_vertexes_1.end() ) //calcula desvio dos atributos dos objetos que est�o no vetor 1
			{
				for (int i=0; i<attributes_number; i++)
				{
					valor = attrObjects[(*it_s).c_str()][i];
					desvio1 = valor - mean1[i];
					desvioGlobal = valor - global_mean[i];
					SSA1 += desvio1*desvio1;
					SSTO += desvioGlobal*desvioGlobal;
				}
				++it_s;
			}

			it_s = subtree_vertexes_2.begin();
			while ( it_s != subtree_vertexes_2.end() )  //calcula desvio dos atributos dos objetos que est�o no vetor 2
			{
				for (int i=0; i<attributes_number; i++)
				{
					valor = attrObjects[(*it_s).c_str()][i];
					desvio2 = valor - mean2[i];
					desvioGlobal = valor - global_mean[i];
					SSA2 += desvio2*desvio2;
					SSTO += desvioGlobal*desvioGlobal;
				}
				++it_s;
			}
			
			// Calculo do custo de aresta
			double link_cost = SSTO - SSA1 - SSA2;

			if (link_cost > bigest_cost.Dissimilarity_)
			{
				bigest_cost.Dissimilarity_ = link_cost;
				bigest_cost.GeoId1_= GeoId1;
				bigest_cost.GeoId2_= GeoId2;
				subtree1 = subtree_vertexes_1;
				subtree2 = subtree_vertexes_2;
				itMax = it1; //armazena o iterator para o n� mais caro
			}

			++it1;
		}
		// excluir link
		string teste1 = itMax->GeoId1_;
		string teste2 = itMax->GeoId2_;
		Min_Tree.erase(itMax);

		// atualizar slice
		vector<string>::iterator it_s = subtree2.begin();
		int contador = 0;
		while (it_s != subtree2.end())
		{
			TeSTInstance* st = objects->getSTInstance((*it_s).c_str());
			if(st)
				st->slice (regiao);
			it_s++;
			contador++;
		}
	}
}