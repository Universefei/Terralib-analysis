//---------------------------------------------------------------------------
#include <stdlib.h>
#include "erro.h"
#ifdef WIN32
#pragma hdrstop
#endif

#include "filaDouble.h"
TFilaDouble::TFilaDouble(){
     mFila = (Fila_ptr) malloc(sizeof(Fila_t));
     if (!mFila) FatalError("Memória FilaInt");
     mFila->Head = NULL;
     mFila->Tail = NULL;
     mFila->Size = 0;
}
//---------------------------------------------------------------------------
TFilaDouble::~TFilaDouble(){
 delete mFila;
}
//---------------------------------------------------------------------------
void TFilaDouble::Insere(double Item){
   Item_ptr novo;

    novo=(Item_ptr) malloc(sizeof(Item_t));
    if (!novo){
        FatalError("Falta de memória!");
    }
    novo->Item = Item;
    novo->Proximo = NULL;
    if(mFila->Size)
      mFila->Tail->Proximo = novo;
    else
      mFila->Head = novo;
    mFila->Tail = novo;
    (mFila->Size)++;
}
//---------------------------------------------------------------------------
double  TFilaDouble::Retira(){
  Item_ptr Aux;
  double i;

  if(!mFila->Size) {
    FatalError("Tirando de Fila Vazia!");
  }

  Aux = mFila->Head;
  i=Aux->Item;
  mFila->Head = mFila->Head->Proximo;
  if (mFila->Size == 1) mFila->Tail = NULL;
  (mFila->Size)--;
  free(Aux);
  return i;
}
//---------------------------------------------------------------------------
int  TFilaDouble::GetSize(){
 return mFila->Size;
}

