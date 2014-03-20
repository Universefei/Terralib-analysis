//---------------------------------------------------------------------------
#include <stdlib.h>
#ifdef WIN32
#pragma hdrstop
#endif

#include "filaR.h"
#include "erro.h"

TFilaR::TFilaR(){
     mFila = (Fila_ptr) malloc(sizeof(Fila_t));
     if (!mFila) FatalError("Memória FilaR");
     mFila->Head = NULL;
     mFila->Next = NULL;
     mFila->Atual = NULL;
     mFila->Size = 0;
}
//---------------------------------------------------------------------------
TFilaR::~TFilaR(){
 delete mFila;
}
//---------------------------------------------------------------------------
void  TFilaR::Insere(int Raiz,double Desvio,long Populacao){
   Item_ptr novo;

    novo=(Item_ptr) malloc(sizeof(Item_t));
    if (!novo) {
      FatalError("Falta de memória!");
    }
    novo->Raiz = Raiz;
    novo->Desvio = Desvio;
    novo->Queda = 0;
    novo->Populacao = Populacao;
    novo->Proximo = mFila->Head;

    mFila->Head = novo;
    mFila->Atual = novo;
    mFila->Next = novo;
    (mFila->Size)++;
}
//---------------------------------------------------------------------------
void  TFilaR::Retira(int Raiz){
  Item_ptr Aux,Prev;

  if(!mFila->Size) {
    FatalError("Fila Vazia!");
  }

  Aux = mFila->Head;
  if(Aux->Raiz == Raiz) {
    mFila->Head = Aux->Proximo;
    (mFila->Size)--;
  } else {
     while (Aux && Aux->Raiz != Raiz){
       Prev = Aux;
       Aux = Aux->Proximo;
     }
     Prev->Proximo = Aux->Proximo;
     (mFila->Size)--;
     free(Aux);
  }
  mFila->Atual = mFila->Head;
  mFila->Next = mFila->Head;
}
//---------------------------------------------------------------------------
int  TFilaR::Proximo(){
  int i;
  if(!mFila->Next) {
    mFila->Next = mFila->Head;
    mFila->Atual = mFila->Head;
    return -1;
  }
  i = mFila->Next->Raiz;
  mFila->Atual = mFila->Next;
  mFila->Next = mFila->Next->Proximo;
  return i;
}
//---------------------------------------------------------------------------

