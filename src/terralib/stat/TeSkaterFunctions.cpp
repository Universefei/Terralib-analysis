//---------------------------------------------------------------------------
#include <stdio.h>
#ifdef WIN32
#pragma hdrstop
#endif

#include "TeSkaterFunctions.h"
#include "../kernel/TeDefines.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeSTElementSet.h"

//---------------------------------------------------------------------------
TParticao::TParticao(TSkaterGrafo *G){
  Grafo=G;
  QuedaDesv = new TFilaDouble;
  Clusters = new TFilaR;
  Clusters->Insere(0,TeMAXFLOAT,(long)TeMAXFLOAT);
  Num_Grupos = 1;
};

//---------------------------------------------------------------------------
TParticao::~TParticao(){
  delete Clusters;
};

//---------------------------------------------------------------------------
double*  TParticao::BFS_Media(int Raiz,int Exceto,long *Populacao){
 double *Acc;
 double Peso;// Usado na busca dos filhos
 int Indice; // Usado na busca dos filhos
 int i;
 int n=0;
 int Atual; // No sendo visitado
 TFilaInt *Fila;

 Acc = new double[Grafo->MyGrafo->Num_Var];
 for(i=0;i<Grafo->MyGrafo->Num_Var;i++)
   Acc[i] =0;
 *Populacao = 0;

 Fila = new TFilaInt;
 Fila->Insere(Raiz);

 while (Fila->Size()){
   Atual = Fila->Retira();

   /** Calculo da Media **/
   n++;
   for(i=0;i<Grafo->MyGrafo->Num_Var;i++)
    Acc[i] += Grafo->MyGrafo->Nos[Atual].Variaveis[i];
   *Populacao = *Populacao + Grafo->MyGrafo->Nos[Atual].Populacao;

   /** Insere Filhos na Lista **/
   Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
   while(Indice!=-1){
      if (Indice != Exceto)
        Fila->Insere(Indice);
      Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
   };
 };
 for(i=0;i<Grafo->MyGrafo->Num_Var;i++)
   Acc[i] /=n;

 delete Fila;
 return Acc;
};
//---------------------------------------------------------------------------

double  TParticao::BFS_Desvio(int Raiz,int Exceto,double *Media){
 double Acc=0;
 double Peso;// Usado na busca dos filhos
 int Indice; // Usado na busca dos filhos
 int Atual; // No sendo visitado
 TFilaInt *Fila;

 Fila = new TFilaInt;
 Fila->Insere(Raiz);

 while (Fila->Size()){
   Atual = Fila->Retira();

   /** Calculo do desvio **/
   Acc += Dist(Atual,Media);

   /** Insere Filhos na Lista **/
   Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
   while(Indice != -1){
      if (Indice != Exceto)
        Fila->Insere(Indice);
      Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
   };
 };
 return Acc;
};

//---------------------------------------------------------------------------
double  TParticao::Dist(int Atual,double *Media){
  int i;
  double Acc=0;

  for(i=0;i<Grafo->MyGrafo->Num_Var;i++)
    Acc += (Grafo->MyGrafo->Nos[Atual].Variaveis[i] - Media[i]) *
           (Grafo->MyGrafo->Nos[Atual].Variaveis[i] - Media[i]);
  return Acc;
};

//---------------------------------------------------------------------------

void  TParticao::BFS_Main(TFilaR::Item_ptr Item){//Raiz,Populacao,Desvio
 //int i=0;
 int Atual; // No sendo visitado
 double Peso;// Usado na busca dos filhos
 int Indice=0; // Usado na busca dos filhos
 double *M1,*M2,D1,D2;
 long Pop1,Pop2;
 double D_Min = TeMAXFLOAT;
 double DMin1;
 double DMin2;
 long Raiz1,PopMin1;
 long Raiz2,PopMin2;
 TFilaInt *Fila;

 if (!Item->Desvio) return;

 Fila = new TFilaInt;
 Fila->Insere(Item->Raiz);

 M1 = new double[Grafo->MyGrafo->Num_Var];
 M2 = new double[Grafo->MyGrafo->Num_Var];

 while (Fila->Size()){
   Atual = Fila->Retira();
    //i++;
   /* Insere Filhos na fila */
     Grafo->MyGrafo->Nos[Atual].ArViz->AdjacenteB(&Indice,&Peso);
     while (Indice != -1){
      Fila->Insere(Indice);
      M1 = BFS_Media(Indice,0,&Pop1);
      D1 = BFS_Desvio(Indice,0,M1);

      M2 = BFS_Media(Item->Raiz,Indice,&Pop2);
      D2 = BFS_Desvio(Item->Raiz,Indice,M2);
      if (((D1+D2) < D_Min) && (Pop1 > PopMin) && (Pop2 > PopMin)) { //encontrou um minimo
           D_Min = (D1+D2);
           Raiz1 = Indice;
           Raiz2 = Atual;
           PopMin1 = Pop1;
           PopMin2 = Pop2;
           DMin1 = D1;
           DMin2 = D2;
      };
      Grafo->MyGrafo->Nos[Atual].ArViz->AdjacenteB(&Indice,&Peso);
     };
   };
   /*Atualizacao dos dados*/
if (D_Min != TeMAXFLOAT){
   Item->D1 = DMin1;
   Item->D2 = DMin2;
   Item->P1 = PopMin1;
   Item->P2 = PopMin2;
   Item->R1 = Raiz1;
   Item->R2 = Raiz2;
   Item->Queda = Item->Desvio - DMin1 - DMin2;
 } else
   Item->Desvio = 0;

   delete Fila;
   delete M1;
   delete M2;
};

//---------------------------------------------------------------------------
void  TParticao::Particiona(int N_Grp,long Pop){
  int i;
  int Raiz;
  int Sai;
  double QMax;
  int R1,R2;
  double D1,D2;
  long P1,P2;
  TFilaR::Item_ptr Item;
  double Acc;

  PopMin = Pop;

  if (Pop == -1){ // Nao levar em conta populacao
    while (N_Grp != Num_Grupos){
      Item = Clusters->mFila->Head;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (!Item->Queda) BFS_Main(Item);
        Item = Item->Proximo;
      };
      Item = Clusters->mFila->Head;
      QMax = 0;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (Item->Queda > QMax){
          QMax = Item->Queda;
          Raiz = Item->Raiz;
          R1 = Item->R1;
          R2 = Item->R2;
          D1 = Item->D1;
          D2 = Item->D2;
          P1 = Item->P1;
          P2 = Item->P2;
        };
        Item = Item->Proximo;
      };
     Grafo->MyGrafo->Nos[R2].ArViz->Retira(R1);
     Num_Grupos++;
     Clusters->Insere(R1,D1,P1);
     Clusters->Retira(Raiz);
     Clusters->Insere(Raiz,D2,P2);
     Acc = 0;
     while (Clusters->Proximo() != -1) Acc += Clusters->mFila->Atual->Desvio;
     QuedaDesv->Insere(Acc);
    };
  } else
    if (N_Grp == 0)
  {
    Sai=0;
    while (!Sai){
      Item = Clusters->mFila->Head;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (!Item->Queda) BFS_Main(Item);
        Item = Item->Proximo;
      };

      Item = Clusters->mFila->Head;
      QMax = 0;
      Sai = 1;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (Item->Desvio) Sai = 0;
        if (Item->Queda > QMax){
          QMax = Item->Queda;
          Raiz = Item->Raiz;
          R1 = Item->R1;
          R2 = Item->R2;
          D1 = Item->D1;
          D2 = Item->D2;
          P1 = Item->P1;
          P2 = Item->P2;
        };
        Item = Item->Proximo;
      };
     if (!Sai){
       Grafo->MyGrafo->Nos[R2].ArViz->Retira(R1);
       Num_Grupos++;
       Clusters->Insere(R1,D1,P1);
       Clusters->Retira(Raiz);
       Clusters->Insere(Raiz,D2,P2);
       Acc = 0;
       while (Clusters->Proximo() != -1) Acc += Clusters->mFila->Atual->Desvio;
       QuedaDesv->Insere(Acc);
      };
    };//while
  } else { // Grupos com populacao restringida
    Sai=0;
    while (!Sai){
      Item = Clusters->mFila->Head;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (!Item->Queda) BFS_Main(Item);
        Item = Item->Proximo;
      };

      Item = Clusters->mFila->Head;
      QMax = 0;
      Sai = 1;
      for(i=0;i<Clusters->mFila->Size;i++){
        if (Item->Desvio) Sai = 0;
        if (Item->Queda > QMax){
          QMax = Item->Queda;
          Raiz = Item->Raiz;
          R1 = Item->R1;
          R2 = Item->R2;
          D1 = Item->D1;
          D2 = Item->D2;
          P1 = Item->P1;
          P2 = Item->P2;
        };
        Item = Item->Proximo;
      };
     if (!Sai){
       Grafo->MyGrafo->Nos[R2].ArViz->Retira(R1);
       Num_Grupos++;
       Clusters->Insere(R1,D1,P1);
       Clusters->Retira(Raiz);
       Clusters->Insere(Raiz,D2,P2);
       Acc = 0;
       while (Clusters->Proximo() != -1) Acc += Clusters->mFila->Atual->Desvio;
       QuedaDesv->Insere(Acc);
       if (Num_Grupos >= N_Grp) Sai = 1;
      };
    };//while
 }//else
};
//---------------------------------------------------------------

void  TParticao::BFS_Salva(TeSTElementSet& regSet, string name){
 double Peso;// Usado na busca dos filhos
 int Indice; // Usado na busca dos filhos
 int Atual; // No sendo visitado
 int i=1;
 TFilaInt *Fila;
 int Raiz;

 TeAttributeRep rep;
 rep.name_ = name;
 rep.type_ = TeINT;
 regSet.addProperty(rep, "");
 int index = regSet.getAttributeIndex(name); //get the index of the inserted attribute

 Raiz=Clusters->Proximo();
 while (Raiz != -1){

   string value = Te2String(i);

   Fila = new TFilaInt;
   Fila->Insere(Raiz);

   while (Fila->Size()){
     Atual = Fila->Retira();

     /** Insere Filhos na Lista **/
     Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
     while(Indice != -1)
	 {
        Fila->Insere(Indice);
        Grafo->MyGrafo->Nos[Atual].ArViz->Adjacente(&Indice,&Peso);
     };
     //set the attribute in the set
	 TeSTInstance* ins = regSet.getSTInstance(Grafo->MyGrafo->Nos[Atual].Label);
	 if(ins)
		ins->setPropertyValue(index, value); 
   }
   i++;
   Raiz=Clusters->Proximo();
  }
}




