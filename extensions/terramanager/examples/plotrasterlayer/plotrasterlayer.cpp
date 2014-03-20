#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados e plota um layer de imagem
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
	
	if(!t.setCurrentView("NATIVIDADE"))
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

	TeBox newWorldBox = t.setWorld(b.x1(), b.y1(), b.x2(), b.y2(), 800, 600);

	if(!t.setTheme("NATIVIDADE", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}	

	if(!t.plotRaster())
	{
		cout << t.errorMessage() << endl;

		return 0;
	}		

	string fName  = "C:\\";
	       fName += "canvas";
		   fName += ".jpg";
		
	t.saveImage(fName);			

	return 0;
}

