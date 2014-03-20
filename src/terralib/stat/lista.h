#ifndef ListaH
#define ListaH

#include "TeStatDefines.h"
#include "erro.h"

class STAT_DLL TListaVizinho{

private:
    typedef struct Vizinho {
      int Indice;
      double Peso;
      struct Vizinho *Proximo;
    }Vizinho_t,*Vizinho_ptr;

    typedef struct ListaVizinho{
      Vizinho_ptr Head;
      int Size;
      Vizinho_ptr Next; /* Utilizado para pesquisa */
      Vizinho_ptr NextB; /* Utilizado para pesquisa e apelacao */
      }Lista_t, *Lista_ptr;

    Lista_ptr Lista;
    int  GetSize();
public:
  TListaVizinho();
  void  Insere(int,double);
  void  Retira(int);
  void  Adjacente(int *,double *);
  void  AdjacenteB(int *,double *);
  int Size;
};
#endif
