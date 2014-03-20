/*
DO NOT USE!
This is a DEPRECATED file not used anymore in any part of the statics library
used by TerraLib. Its being kept for compatibility reasons and it will be removed 
from further TerraLib distributions.
*/
//---------------------------------------------------------------------------
#ifdef WIN32
#pragma hdrstop
#endif
#include <stdlib.h>
#include <stdio.h>
#include "hash.h"

THash::THash(){//Construtora
   MyHash = new Hash_t[HASH_SIZE];
   if(!MyHash) 
     FatalError("Falta de memória para o Hash");
};

void  THash::Insere(char* SKey,unsigned int Indice){
  unsigned int Hash_Indice;
  unsigned long Key;
  Hash_ptr Novo;

  sscanf(SKey,"%ld",&Key);

  Novo = (Hash_ptr) malloc(sizeof(Hash_t));
  if(!Novo) FatalError("Falta de memória para o Hash");

  Hash_Indice = Hash_Function(Key);
  Novo->Next = MyHash[Hash_Indice].Next;
  Novo->Key = Key;
  Novo->Indice = Indice;
  MyHash[Hash_Indice].Next = Novo;
};

unsigned int  THash::Pesquisa(const char* SKey){
   Hash_ptr Aux;
   unsigned int Hash_Indice;
   unsigned long Key;

   sscanf(SKey,"%ld",&Key);
   Hash_Indice = Hash_Function(Key); //Obtem posicao no Hash
   Aux=MyHash[Hash_Indice].Next;//Prepara para a pesquisa
   while((Aux != NULL) && (Aux->Key != Key)) Aux=Aux->Next;//Faz a pesquisa

   if (Aux == NULL) {
    FatalError("Pesquisa no hash");
   }
   return Aux->Indice;//Retorna o resultado
};

unsigned int  THash::Hash_Function(unsigned long Key){
  return Key%HASH_SIZE;
};

//---------------------------------------------------------------------------

