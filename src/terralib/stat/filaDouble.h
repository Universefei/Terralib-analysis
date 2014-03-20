//---------------------------------------------------------------------------
#ifndef FilaDoubleH
#define FilaDoubleH

#include "TeStatDefines.h"

class STAT_DLL TFilaDouble {
  private:
    typedef struct it {
      double Item;
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
  TFilaDouble();
  ~TFilaDouble();
  void Insere(double);
  double  Retira();
  int Size() {return GetSize();}
};

//---------------------------------------------------------------------------
#endif
