/*
DO NOT USE!
This is a DEPRECATED file not used anymore in any part of the statics library
used by TerraLib. Its being kept for compatibility reasons and it will be removed 
from further TerraLib distributions.
*/

//---------------------------------------------------------------------------
#ifndef HashH
#define HashH
#define HASH_SIZE 1000
#include "erro.h"
class THash{

/*** Area Privada ***/
private:
  typedef struct Hash_Type{
     unsigned long Key;
     unsigned int Indice;
     struct Hash_Type *Next;
  }Hash_t,*Hash_ptr;

  Hash_ptr MyHash;
  unsigned int  Hash_Function(unsigned long);

/*** Area Publica ***/
public:
   THash();//Construtora
   void  Insere(char *,unsigned int);
   unsigned int  Pesquisa(const char *);
};
//---------------------------------------------------------------------------
#endif
