
//---------------------------------------------------------------------------
#ifndef GrafoH
#define GrafoH
//---------------------------------------------------------------------------

#include "../kernel/TeSTElementSet.h"
#include "../kernel/TeNeighbours.h"
#include "lista.h"

#define RAIO 4

class STAT_DLL TSkaterGrafo {

public:

    typedef struct No_Grafo
	{
      char Label[50];
      double X;
      double Y;
      TListaVizinho *Vizinhos;
      TListaVizinho *ArViz;
      double *Variaveis;
      int Populacao;
      char Mark;
    } Grafo_t, *Grafo_ptr;

    typedef struct Grafo
	{
       Grafo_ptr Nos;
       int Num_Var;
       int Size;
    }MyGrafo_t,*MyGrafo_ptr;

/**** Funcoes Publicas ****/
 TSkaterGrafo(bool hasPop);
 template<typename Matrix> bool  MontaGrafo(TeSTElementSet& regSet, Matrix* proxMatrix);
 void  Escalona(double *,double *);
 void  Escala(int,int);
 void  Adjacente(int *,double *,int i);
 void  Adjacente1(int *,double *,int i);
 int Size() {return GetSize();}
 double MinX() {return X_min;}
 double MaxX() {return X_max;}
 double MinY() {return Y_min;}
 double MaxY() {return Y_max;}
 double Coord_X(int Indice){return Get_X(Indice);}
 double Coord_Y(int Indice) {return Get_Y(Indice);}
 int Num_Var() {return GetNum_Var();}


 /***** Area Privada *****/
private:
    MyGrafo_ptr MyGrafo;
	std::map<std::string,unsigned int> Hash_;

	double X_min,X_max;//Usado para conversao de escala
    double Y_min,Y_max;//Usado para conversao de escala
    double Cx,Gamax;   //Usados na conversao de escala
    double Cy,Gamay;   //Usados na conversao de escala

    double  Distancia(int,int);
    double  Get_X(int Indice);
    double  Get_Y(int Indice);
    int GetSize();
    int  GetNum_Var();

     //Variaveis que sao inicializadas de fora
    char *NArq;
    char *NViz;
    char Tem_Pop;

    friend class TArvore;
    friend class TParticao;
    friend class TPrincipal;
    friend class TEstatistica;
};

template<typename Matrix> bool  
TSkaterGrafo::MontaGrafo(TeSTElementSet& regSet, Matrix* proxMatrix)
{

//  long nAreas = regSet.numElements();

  typename TeSTElementSet::iterator it = regSet.begin(); 
  int  nCov   = (*it).getProperties().size();


  //Monta lista de areas

  TeCoord2D centroid;
  (*it).centroid(centroid);
  X_min=X_max=centroid.x();
  Y_min=Y_max=centroid.y();
  //Obtem o numero de variaveis --> se tem populacao e menos um
  if (this->Tem_Pop)
          MyGrafo->Num_Var = nCov-1; //Obtem o numero de variaveis
  else
          MyGrafo->Num_Var = nCov;

  string val;
  int i;

  //Monta areas
  while (it != regSet.end()) 
  {
		(*it).centroid(centroid);    
		MyGrafo->Nos = 
		  (Grafo_ptr) realloc(MyGrafo->Nos,sizeof(Grafo_t)*((MyGrafo->Size)+1));
		sprintf(MyGrafo->Nos[MyGrafo->Size].Label,"%s",(*it).objectId().c_str());
		MyGrafo->Nos[MyGrafo->Size].Vizinhos = NULL;
		MyGrafo->Nos[MyGrafo->Size].ArViz = NULL;

		Hash_[std::string(MyGrafo->Nos[MyGrafo->Size].Label)] = MyGrafo->Size;

		MyGrafo->Nos[MyGrafo->Size].X = centroid.x();
		if(centroid.x() < X_min) X_min =centroid.x();// Procura limites da tela
		else if(centroid.x() > X_max) X_max = centroid.x();
    
		MyGrafo->Nos[MyGrafo->Size].Y =centroid.y();
		if(centroid.y()< Y_min) Y_min = centroid.y(); //Procura limites da tela
		else if(centroid.y() > Y_max) Y_max = centroid.y();
    
		MyGrafo->Nos[MyGrafo->Size].Variaveis = 
		  (double *) malloc(sizeof(double)*MyGrafo->Num_Var);
		for(i=0;i<MyGrafo->Num_Var;i++) {
		  (*it).getPropertyValue(val, i);
		  MyGrafo->Nos[MyGrafo->Size].Variaveis[i] = atof(val.c_str()); 
		}
		if (Tem_Pop) {
		  (*it).getPropertyValue(val, MyGrafo->Num_Var);
		  MyGrafo->Nos[MyGrafo->Size].Populacao = (int)atof(val.c_str());//covs[a*nCov+nCov-1];
		}
		else
		  MyGrafo->Nos[MyGrafo->Size].Populacao = 0;
    
		MyGrafo->Nos[MyGrafo->Size].Mark=1;
		(MyGrafo->Size)++;
		(++it);
  }

  int L_indice,V_indice;
  double dist;

  it = regSet.begin();
	
	while ( it != regSet.end())
	{
		TeNeighboursMap	neighbors = proxMatrix->getMapNeighbours((*it).objectId());

		//Grafo eh desconexo
		if (neighbors.size() == 0) 
		{
			Hash_.clear();
		  return false;
		}
		
		std::map<std::string,unsigned int>::iterator ith = Hash_.find((*it).objectId());
		if (ith != Hash_.end())
			L_indice = ith->second;
		else
			L_indice = 0;

		MyGrafo->Nos[L_indice].Vizinhos = new TListaVizinho;
		MyGrafo->Nos[L_indice].ArViz = new TListaVizinho;

		typename TeNeighboursMap::iterator itNeigs = neighbors.begin();
	 			
		while(itNeigs != neighbors.end())
		{
			ith = Hash_.find((*itNeigs).first);
			if (ith != Hash_.end())
				V_indice = ith->second;
			else
				V_indice = 0;
		   dist = Distancia(L_indice,V_indice);
		   MyGrafo->Nos[L_indice].Vizinhos->Insere(V_indice,dist);
		   (++itNeigs);
		}
		(++it);
	}

  Hash_.clear();
  return true;
}

#endif
