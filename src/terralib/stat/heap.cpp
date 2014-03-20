//---------------------------------------------------------------------------
#include <limits.h>
#include <stdlib.h>
#ifdef WIN32
#pragma hdrstop
#endif

#include "heap.h"
#include "erro.h"


/********* Mantem a propriedade de Heap (Para Baixo) *****/
void  THeap::DownHeap(int i){
   int l;// esquerda
   int r;// direita

   int max;
   TipoHeap_t swap;

   l=2*i; //filho da esquerda de i
   r=2*i+1;// filho da direita de i
   if ((l<=MyHeap->Size) && (MyHeap->Nos[l].Chave < MyHeap->Nos[r].Chave))
      max=l;
    else
      max=i;
   if ((r<=MyHeap->Size) && (MyHeap->Nos[r].Chave < MyHeap->Nos[max].Chave))
      max=r;
   if (max!=i) {
      /* Atualiza KeyAux */
      KeyAux[MyHeap->Nos[i].Indice] = max;
      KeyAux[MyHeap->Nos[max].Indice] = i;

      /*** Troca Heap[i] com Heap[max] ***/
      swap = MyHeap->Nos[i];
      MyHeap->Nos[i] = MyHeap->Nos[max];
      MyHeap->Nos[max] = swap;
      /***  Fim da troca *****/
      DownHeap(max); // Continua o processo
     };
};

/******** Mantem a propriedade de Heap (Para Cima) *********/
void THeap::UpHeap(int i){
  TipoHeap_t Aux;
  int swap;

  Aux = MyHeap->Nos[i];
  /* Guarda a posicao ocupada pelo Indice[i] no Heap */
  swap = MyHeap->Nos[i].Indice;

  while (i>1 && MyHeap->Nos[i/2].Chave > Aux.Chave){

     /* Atualiza a KeyAux */
     KeyAux[MyHeap->Nos[i/2].Indice] = i;

     MyHeap->Nos[i] = MyHeap->Nos[i/2];
     i/=2;
  };
  MyHeap->Nos[i] = Aux;
  /* Atualiza a posicao do KeyAux do item que subiu no Heap */
  KeyAux[swap] = i;
};

/******** Funcao que monta o Heap a partir de um Array *****/
void THeap::Build_Heap(){
   int i;

   for (i=(MyHeap->Size)/2;i>=1;i--)
      THeap::DownHeap(i);
};

/******** Extrai o elemento de maior prioridade e remonta o Heap ********/
int THeap::Extract_Min(){
    int min;

    if (MyHeap->Size<1) FatalError("Heap Vazio"); /*acrescentar mensagem de erro*/
    min=MyHeap->Nos[1].Indice;

    /** Atualiza o KeyAux do Item que foi pra frente */
    KeyAux[MyHeap->Nos[MyHeap->Size].Indice] = 1;
    /** Atualiza o KeyAux do Item que foi pro vinagre */
    KeyAux[MyHeap->Nos[1].Indice] = -1;

    MyHeap->Nos[1] = MyHeap->Nos[MyHeap->Size];

    (MyHeap->Size)--;
    DownHeap(1);
    return min;
};

THeap::THeap(int Size){
   int i;
  /*** Inicializacao do Heap ***/
  MyHeap = new Heap_t;
  MyHeap->Size = Size;
  MyHeap->Nos = (TipoHeap_ptr) malloc(sizeof(TipoHeap_t)*(MyHeap->Size+1));
  KeyAux = (int *) malloc(sizeof(int)*Size);
  /*** Preenchimento do Heap ***/
  MyHeap->Nos[1].Chave = 0; // Inicializacao do nodo 1
  MyHeap->Nos[1].Indice = 0;
  KeyAux[0]=1;
  for(i=2;i<=MyHeap->Size;i++) {
    KeyAux[i-1]=i;
    MyHeap->Nos[i].Chave = INT_MAX;
    MyHeap->Nos[i].Indice = i-1;
  };
};

/****************************/
 int THeap::Empty(){
   return !(MyHeap->Size);
 };

/*************************/
double  THeap::GetKey(int Indice){
   return MyHeap->Nos[KeyAux[Indice]].Chave;
};

/****************************/
void  THeap::SetKey(int Indice,double NKey){
   MyHeap->Nos[KeyAux[Indice]].Chave = NKey;
   UpHeap(KeyAux[Indice]);
};


