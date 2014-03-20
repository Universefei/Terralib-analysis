//---------------------------------------------------------------------------
#ifndef FilaIntH
#define FilaIntH

#include "TeStatDefines.h"

class STAT_DLL TFilaInt 
{
  private:
    typedef struct it {
      int Raiz;
      struct it *Proximo;
    }Item_t,*Item_ptr;

    typedef struct Fila {
      Item_ptr Head;
      Item_ptr Tail;
      int Size;
    }Fila_t,*Fila_ptr;
    Fila_ptr mFila;
   int  GetSize();
public:
  TFilaInt();
  ~TFilaInt();
  void  Insere(int);
  int  Retira();
  int Size() {return GetSize();}
};

//---------------------------------------------------------------------------
#endif
