//---------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifdef WIN32
#pragma hdrstop
#endif

#include "TeSkaterGrafo.h"


TSkaterGrafo::TSkaterGrafo(bool hasPop){

   MyGrafo= new MyGrafo_t;
   MyGrafo->Nos = (Grafo_ptr) malloc(sizeof(Grafo_t));
   MyGrafo->Size = 0;
   Tem_Pop = (hasPop) ? 1 : 0;
};


/*************************/
int TSkaterGrafo::GetSize(){

  return MyGrafo->Size; //Retorna o tamanho do grafo
};


/*************************/
double TSkaterGrafo::Distancia(int L_indice, int V_indice){

   int i;
   double Acc=0;

   for(i=0;i < MyGrafo->Num_Var;i++)
      Acc += (MyGrafo->Nos[L_indice].Variaveis[i] - MyGrafo->Nos[V_indice].Variaveis[i]) *
             (MyGrafo->Nos[L_indice].Variaveis[i] - MyGrafo->Nos[V_indice].Variaveis[i]);
   return sqrt(Acc);
};

//-------------------------------------
void  TSkaterGrafo::Escalona(double *X, double *Y){
  *X = Cx *(*X + Gamax);
  *Y = Cy *(*Y + Gamay);
};

/***************************/
//Retorna o Indice e o peso do proximo Vizinho -1 se nao houver mais vizinhos
void  TSkaterGrafo::Adjacente(int *Indice,double *Peso,int i){
   MyGrafo->Nos[i].Vizinhos->Adjacente(Indice,Peso);
};

void  TSkaterGrafo::Adjacente1(int *Indice,double *Peso,int i){
   MyGrafo->Nos[i].ArViz->Adjacente(Indice,Peso);
};

//--------------------------------------------------------
void  TSkaterGrafo::Escala(int Altura,int Largura){

  Cx = (Largura - 2*(RAIO))/(X_max-X_min);
  Gamax = -RAIO/Cx - X_min;

  Cy = (Altura - 2*(RAIO))/(Y_max-Y_min);
  Gamay = -RAIO/Cy - Y_min;
};
//--------------------------------------------------------
double  TSkaterGrafo::Get_X(int Indice){
   return MyGrafo->Nos[Indice].X;
};

//--------------------------------------------------------
double  TSkaterGrafo::Get_Y(int Indice){
   return MyGrafo->Nos[Indice].Y;
};
//---------------------------------------------
int  TSkaterGrafo::GetNum_Var(){
  return MyGrafo->Num_Var;
};
