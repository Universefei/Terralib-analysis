//---------------------------------------------------------------------------
#ifdef WIN32
#pragma hdrstop
#endif
#include <stdlib.h>
#include "TeSkaterArvore.h"
#include "heap.h"
void  TArvore::Monta_Arvore(TSkaterGrafo *Grafo){
  Heap = new THeap(Grafo->MyGrafo->Size);
  Prim(Grafo);
};

/**********************************/
void  TArvore::Prim(TSkaterGrafo *Grafo) {
  int Menor;
  int Viz;
  double Peso;
  int* Pai;

	Pai = (int*)malloc(sizeof(int)*(Grafo->MyGrafo->Size));
	if (Pai == NULL)
		FatalError("Falta memoria");
	try
	{
		while (!(Heap->Empty()))
		{
			Menor = Heap->Extract_Min();
			Grafo->MyGrafo->Nos[Menor].Mark=0; //Marca que o nodo esta fora da Fila
			for(Grafo->Adjacente(&Viz,&Peso,Menor);Viz!=-1;Grafo->Adjacente(&Viz,&Peso,Menor))
			{
			  if (Grafo->MyGrafo->Nos[Viz].Mark && Peso<(Heap->Key(Viz)))
			  {
				 Pai[Viz]=Menor;
				 Heap->SetKey(Viz,Peso);
			  };
			}
			
			if(Menor)
			{ 
				try
				{
					//Testa se nao e a raiz. Raiz nao tem pai nem e filho
					Grafo->MyGrafo->Nos[Pai[Menor]].ArViz->Insere(Menor,0);
				}
				catch(...)
				{
					continue;
				}
			}
		}
	}
	catch(...)
	{
		return;
	}
};
//---------------------------------------------------------------------------
TArvore::~TArvore(){
 delete Heap;
};


