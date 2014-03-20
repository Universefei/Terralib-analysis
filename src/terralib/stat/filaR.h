//---------------------------------------------------------------------------
#ifndef FilaRH
#define FilaRH

#include "TeStatDefines.h"

class STAT_DLL TFilaR {
public:
    typedef struct Item {
      int Raiz; // Indice no vetor da raiz do grupamento
      double Centro_X,Centro_Y; // Centroide do grupamento
      int Num_Areas; // Numero de areas dentro do grupamento
      double Desvio; // Desvio dentro do grupamento
      double Queda; // Quanto cai o desvio com uma particao
      double D1,D2; // desvio nos dois grupos gerados
      int R1,R2; // Indice no vetor das raizes dos grupos gerados
      long P1,P2; // populacao nos grupos gerados
      long Populacao; // Populacao do grupamento
      struct Item *Proximo; // Proximo item na lista de raizes
    }Item_t,*Item_ptr;

    typedef struct Fila {
      Item_ptr Head;
      int Size;
      Item_ptr Atual;// Utilizado para acesso aos dados
      Item_ptr Next; //Para Pesquisa
    }Fila_t,*Fila_ptr;

  Fila_ptr mFila;
  
  TFilaR();
  ~TFilaR();
  void  Insere(int,double,long);
  void  Retira(int);
  int  Proximo();
};
//---------------------------------------------------------------------------
#endif
