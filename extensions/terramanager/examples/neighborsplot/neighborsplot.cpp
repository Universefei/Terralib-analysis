#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um layer de municípios e outro de pontos,
 *          plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
 *          passos iguais com 5 fatias, e o plota o layer de pontos de forma normal.
 *          restringindo o box de visualização.
 *          Plota em amarelo os vizinhos do município de altamira
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

	cout << "BOX: " << b.x1() << " | " << b.y1() << " | " << b.x2() << " | " << b.y2() << endl;

	TeBox newWorldBox = t.setWorld(-59.912893594101, -13.598146710832, -40.977835330074, -2.7220339762508, 800, 600);
		
	if(!t.setTheme("municipios", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeGrouping g;
	g.groupAttribute_.name_ = "LATITUDESE";
	
	g.groupMode_ = TeEqualSteps;

	g.groupNumSlices_ = 5;

	vector<string> rampColors;
	rampColors.push_back("RED");
	rampColors.push_back("GREEN");
	rampColors.push_back("BLUE");


	TeMemoryVisualMap visualMap;
	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(visualMap, legendVec, &g, rampColors))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	Keys kout;

	if(!t.locateObjects("953", TeTOUCHES, kout, 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeColor color(210, 210, 0);
	TeColor contourColor(100, 100, 100);
	TeVisual v;
	v.color(color);
	v.contourColor(contourColor);
	v.style(TePolyTypeFill);
	v.transparency(0);
	v.contourWidth(1);

	t.setDefaultVisual(v, TePOLYGONS);

	if(!t.plotSelectedObjects(kout, 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	kout.clear();
	kout.push_back("953");

	TeColor color2(225, 225, 225);
	TeColor contourColor2(100, 100, 100);
	TeVisual v2;
	v2.color(color2);
	v2.contourColor(contourColor2);
	v2.style(TePolyTypeFill);
	v2.transparency(0);
	v2.contourWidth(1);


	t.setDefaultVisual(v2, TePOLYGONS);

	if(!t.plotSelectedObjects(kout, 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}


	if(!t.setTheme("desmat2004", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	if(!t.plotVector(legendVec))
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

