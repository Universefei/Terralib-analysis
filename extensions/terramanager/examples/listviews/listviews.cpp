#include "TerraManager.h"

#include <iostream>

using namespace std;


/*
 * Exemplo: Conecta-se a um banco de dados e lista todas as vistas armazenadas neste banco
 *
 */

int main(int argc, char* argv[])
{
	TerraManager t;

	if(!t.connect(TeADODB, "C:/terramanagerdb.mdb", "", "", "", 0))
	{
		cout << "Nao foi possivel conectar-se ao banco de dados" << endl;

		return 0;
	}
	
	vector<string> views;

	if(!t.getViews(views))
	{
		cout << "Nao foi possivel recuperar as vistas" << endl;

		return 0;
	}

	cout << "VISTAS NO BANCO:\n";

	unsigned int i = 0;

	for(i = 0; i < views.size(); ++i)
		cout << views[i] << endl;

	return 0;
}

