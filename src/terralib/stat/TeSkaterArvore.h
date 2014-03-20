//---------------------------------------------------------------------------
#ifndef ArvoreH
#define ArvoreH

#include "TeStatDefines.h"
#include "TeSkaterGrafo.h"
#include "heap.h"

class STAT_DLL TArvore{
  private:
      THeap *Heap;
      void  Prim(TSkaterGrafo *);
  public:
      ~TArvore();
      void  Monta_Arvore(TSkaterGrafo *);

};

//---------------------------------------------------------------------------
#endif
