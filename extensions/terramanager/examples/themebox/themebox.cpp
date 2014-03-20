#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um layer de municípios
 *          plotando do mapa de municipios apenas os municípios com o atributo AREA_1 >= 38162.317
 *          com o box da imagem igual ao box dos objetos do tema
 */

int main(int argc, char* argv[])
{
	TerraManager t;

	if(!t.connect(TeADODB, "C:/terramanagerdb.mdb", "", "", "", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}
	
	if(!t.setCurrentView("BRASIL"))
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

	if(!t.setTheme("municipios", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	if(!t.getThemeBox(b, 0, "AREA_1 >= 38162.317"))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeBox newWorldBox = t.setWorld(b.x1(), b.y1(), b.x2(), b.y2(), 800, 600);

	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(legendVec, "AREA_1 >= 38162.317"))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	string fName  = "C:\\";
	       fName += "canvas";
		   fName += ".png";
		
	t.savePlot(fName);			

	return 0;
}

