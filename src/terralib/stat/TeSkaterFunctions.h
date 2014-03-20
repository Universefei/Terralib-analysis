//---------------------------------------------------------------------------
#ifndef ParticaoH
#define ParticaoH



#define SKATER_SEM_POPULACAO 0
#define SKATER_COM_POPULACAO 1


#define SKATER_POR_GRUPO     0
#define SKATER_POR_POPULACAO 1
#define SKATER_POR_AMBOS     2

#include "TeSkaterGrafo.h"
#include "TeSkaterArvore.h"
#include "filaR.h"
#include "filaDouble.h"
#include "filaInt.h"
#include <string>
#include <vector>
using namespace std;

class TeElementSet;

class STAT_DLL TParticao{
private:
  double*  BFS_Media(int,int,long*);
  double  BFS_Desvio(int,int,double*);
  double  Dist(int,double*);
  void  BFS_Main(TFilaR::Item_ptr);//Raiz,Populacao,Desvio
  TSkaterGrafo *Grafo;
  long PopMin;
  int Num_Grupos;
public:
  TFilaR *Clusters;
  TParticao(TSkaterGrafo*);
  ~TParticao();
  TFilaDouble *QuedaDesv; 
  void  Particiona(int,long);
  void  BFS_Salva(TeSTElementSet& regSet, string name);
};

/** Funcao de regionalizacao de um conjunto de poligonos
 *  hasPop: se ha atributo de populacao, que devera ser o ultimo
 *  tipo:   tipo de quebra, por grupo, por populacao ou por ambos
 *  nGrupos: numero de grupos de quebra
 *  popMin:  tamanho minimo da populacao
 *  regSet:  conjunto de areas a serem regionalizadas, com seus atributos
 *  hetDrop: heterogeneidade dado cada uma das divisoes
 **/
template<typename Matrix> bool 
TeSkaterFunction(bool hasPop, int tipo, int nGrupos, int popMin, TeSTElementSet& regSet, 
				  Matrix* proxMatrix, string resName, vector<double>& hetDrop); 


template<typename Matrix> bool 
TeSkaterFunction(bool hasPop, int tipo, int nGrupos, int popMin, TeSTElementSet& regSet, 
				 Matrix* proxMatrix, string name, vector<double>& hetDrop) 
{
  
	TParticao *Particao = 0;
	try
	{
		//Monta grafo a partir do conjunto 
	  TSkaterGrafo g(hasPop);
	  if (!g.MontaGrafo(regSet, proxMatrix)) {
		return false;
	  }

	  TArvore* a = new TArvore();
	  a->Monta_Arvore(&g);
	  delete a;

  
	  Particao = new TParticao(&g);
	  switch (tipo) {
	  case SKATER_POR_GRUPO:
		 Particao->Particiona(nGrupos,-1);
		 break;
	  case SKATER_POR_POPULACAO:
		 Particao->Particiona(0,popMin);
		 break;
	  case SKATER_POR_AMBOS:
		 Particao->Particiona(nGrupos,popMin);
		 break;
	  }

	  Particao->BFS_Salva(regSet, name);

	  //Copia as quedas a partir do numero de grupos para hetDrop,
	  //comecando do menor
	  hetDrop.clear();
	  int size = Particao->QuedaDesv->Size();
	  for (int gr = 0; gr < size; gr++) {
		double het = Particao->QuedaDesv->Retira();
		hetDrop.push_back(het);
	  }
	  delete Particao;
	}
	catch(...)
	{
		if(Particao)
			delete Particao;

		return false;
	}

  return true;
}

//---------------------------------------------------------------------------
#endif
