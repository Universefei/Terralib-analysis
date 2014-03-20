//---------------------------------------------------------------------------
#ifndef heapH
#define heapH

#include "TeStatDefines.h"
#include "TeSkaterGrafo.h"
//---------------------------------------------------------------------------
class STAT_DLL THeap {

/********  Area Privada  *********/
private:
    /* Tipo dos nos do heap */
    typedef struct TipoHeap {
        double Chave;
        int Indice;
      }TipoHeap_t,*TipoHeap_ptr;

    /* Estrutura do TAD Heap : Tamanho + Vetor */
    typedef struct Heap {
        int Size;
        TipoHeap_ptr Nos;
        }Heap_t, *Heap_ptr;
   int *KeyAux;
   Heap_ptr MyHeap;
   void  Build_Heap();
   void  UpHeap(int);
   void  DownHeap(int);
   double  GetKey(int);

/********  Area Publica  *********/
public:
   THeap(int);
   double Key(int Indice) {return GetKey(Indice);}
   void  SetKey(int,double);
   int  Extract_Min();
   int  Empty();
};
#endif
