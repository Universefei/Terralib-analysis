//---------------------------------------------------------------------------
#include <stdlib.h>
#ifdef WIN32
#pragma hdrstop
#endif

#include "erro.h"
#include "filaInt.h"
TFilaInt::TFilaInt(){
     mFila = (Fila_ptr) malloc(sizeof(Fila_t));
     if (!mFila) FatalError("Memória FilaInt");
     mFila->Head = NULL;
     mFila->Tail = NULL;
     mFila->Size = 0;
}
//---------------------------------------------------------------------------
TFilaInt::~TFilaInt(){
 delete mFila;
}
//---------------------------------------------------------------------------
void  TFilaInt::Insere(int Raiz){
   Item_ptr novo;

    novo=(Item_ptr) malloc(sizeof(Item_t));
    if (!novo){
       FatalError("Falta de memória!");
    }
    novo->Raiz = Raiz;
    novo->Proximo = NULL;
    if(mFila->Size)
      mFila->Tail->Proximo = novo;
    else
      mFila->Head = novo;
    mFila->Tail = novo;
    (mFila->Size)++;
}
//---------------------------------------------------------------------------
int  TFilaInt::Retira(){
  Item_ptr Aux;
  int i;

  if(!mFila->Size) {
    FatalError("Tirando de Fila Vazia!");
  }

  Aux = mFila->Head;
  i=Aux->Raiz;
  mFila->Head = mFila->Head->Proximo;
  if (mFila->Size == 1) mFila->Tail = NULL;
  (mFila->Size)--;
  free(Aux);
  return i;
}
//---------------------------------------------------------------------------
int  TFilaInt::GetSize(){
 return mFila->Size;
}

