//---------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#pragma hdrstop
#endif

#include "lista.h"
//---------------------------------------------------------------------------
TListaVizinho::TListaVizinho(){
     Lista = (Lista_ptr) malloc(sizeof(Lista_t));
     if (!Lista) {
       FatalError("Erro interno A1! \n Entre em contato com o desenvolvedor.");
     };
     Lista->Head = NULL;
     Lista->Next = NULL;
     Lista->NextB = NULL;
     Lista->Size = 0;
  };

//---------------------------------------------------------------------------
void  TListaVizinho::Insere(int indice,double peso){
    Vizinho_ptr novo;

    novo=(Vizinho_ptr) malloc(sizeof(Vizinho_t));
    if (!novo) FatalError("Lista Erro"); /* Incluir mensagem de erro */

    novo->Indice = indice;
    novo->Peso = peso;
    novo->Proximo = Lista->Head;

    Lista->Head = novo;
    Lista->Next=novo;
    Lista->NextB=novo;
    (Lista->Size)++;

 };

//---------------------------------------------------------------------------
void  TListaVizinho::Adjacente(int *Indice,double *Peso) 
{
	try
	{
		if(!Lista) 
		{
			*Indice =-1;
			return;
		}
		
		if(Lista->Size==0)
		{ 
			*Indice =-1;
			return; 
		}

		if (Lista->Next) {
		 *Indice = Lista->Next->Indice;
		 *Peso = Lista->Next->Peso;
		 Lista->Next=Lista->Next->Proximo;
	   }
	   else {
		 *Indice = -1;
		 Lista->Next = Lista->Head;
	   }
	}
	catch(...)
	{
		*Indice = -1;
		return;
	}
 };

//---------------------------------------------------------------------------
void TListaVizinho::AdjacenteB(int *Indice,double *Peso){


   if (Lista->NextB) {
     *Indice = Lista->NextB->Indice;
     *Peso = Lista->NextB->Peso;
     Lista->NextB=Lista->NextB->Proximo;
   }
   else {
     *Indice = -1;
     Lista->NextB = Lista->Head;
   };
 };

//---------------------------------------------------------------------------
int TListaVizinho::GetSize(){
  return Lista->Size;
};

//---------------------------------------------------------------------------
void TListaVizinho::Retira(int Indice){
  Vizinho_ptr Aux,Prev;

  if(!Lista->Size) {
    FatalError("Erro interno A2! \n\nEntre em contato com o desenvolvedor.");
  };

  Aux = Lista->Head;
  if(Aux->Indice == Indice) {
    Lista->Head = Aux->Proximo;
    (Lista->Size)--;
    Lista->Next = Aux->Proximo;
    Lista->NextB = Aux->Proximo;
  } else {
     Prev = Aux;
     for(;Aux->Indice != Indice;Aux = Aux->Proximo)
       Prev = Aux;
     Prev->Proximo = Aux->Proximo;
     (Lista->Size)--;
     free(Aux);
  };
};

