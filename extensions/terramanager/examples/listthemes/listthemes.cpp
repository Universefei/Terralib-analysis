#include "TerraManager.h"

#include <iostream>

using namespace std;


/*
 * Exemplo: Conecta-se com um banco de dados e lista para cada vista armazenada no banco,
 *          o seu retângulo envolvente e os temas contidos nela
 *
 */

int main(int argc, char* argv[])
{
	TerraManager t;

	if(!t.connect(TeADODB, "C:/terramanagerdb.mdb", "", "", "", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}
	
	vector<string> views;

	if(!t.getViews(views))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	unsigned int i = 0;

	for(i = 0; i < views.size(); ++i)
	{
		cout << "VISTA: " << views[i] <<  " --- ";

		if(!t.setCurrentView(views[i]))
		{
			cout << t.errorMessage() << endl;

			return 0;
		}

		TeBox b;

		if(!t.getCurrentViewBox(b))
		{
			cout << t.errorMessage() << endl;

			return 0;
		}

		cout << "BOX: " << b.x1() << " | " << b.y1() << " | " << b.x2() << " | " << b.y2() << endl;

		vector<string> temas;

		if(!t.getThemes(temas))
		{
			cout << t.errorMessage() << endl;;

			return 0;
		}

		for(unsigned int j = 0; j < temas.size(); ++j)
			cout << "->TEMA: " << temas[j] << endl;


		cout << endl;
	}

	return 0;
}

