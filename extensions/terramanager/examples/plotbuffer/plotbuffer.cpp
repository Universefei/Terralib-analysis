#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um layer de municípios
 *          plotando o mapa de municipios
 *          restringindo o box de visualização.
 *          Plota em vermelho uma linha contendo o buffer do objeto (polígono) 1151
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

	TeBox newWorldBox = t.setWorld(-59.912893594101, -13.598146710832, -40.977835330074, -2.7220339762508, 800, 600);

	if(!t.setTheme("municipios", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(legendVec))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeColor color(0, 255, 0);
	TeColor contourColor(100, 100, 100);
	TeVisual v;
	v.color(color);
	v.contourColor(contourColor);
	v.style(TePolyTypeFill);
	v.transparency(0);
	v.contourWidth(1);

	t.setDefaultVisual(v, TePOLYGONS);

	Keys kout;
	kout.push_back("1151");
	
	if(!t.plotSelectedObjects(kout, 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeColor color2(255, 0, 0);	
	TeVisual v2;
	v2.color(color2);
	v2.contourStyle(TeLnTypeContinuous);
	v2.contourWidth(2);

	t.setDefaultVisual(v2, TeLINES);

	if(!t.plotBufferZone("1151", 0.2, 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}	
	
	cout << endl;

	string fName  = "C:\\";
	       fName += "canvas";
		   fName += ".png";
		
	t.savePlot(fName);			

	return 0;
}

