#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um layer de municípios e outro de pontos,
 *          plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
 *          passos iguais com 5 fatias em tons de vermelho
 *          Plota em amarelo os polígonos que interagem (intersects) com um dado box mostrado em verde
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

	cout << "BOX: " << b.x1() << " | " << b.y1() << " | " << b.x2() << " | " << b.y2() << endl;

	TeBox newWorldBox = t.setWorld(b.x1(), b.y1(), b.x2(), b.y2(), 800, 600);

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

	TeMemoryVisualMap visualMap;
	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(visualMap, legendVec, &g, rampColors))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	TeBox window(-58.003751271716, -14.778673212769, 	-49.718428929108, 	-9.8074798072041);

	Keys kout;

	if(!t.locateObjects(window, TeINTERSECTS, kout, 0))
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

	TeColor color2(0, 255, 0);	
	TeVisual v2;
	v2.color(color2);
	v2.contourStyle(TeLnTypeContinuous);
	v2.contourWidth(3);

	t.setDefaultVisual(v2, TeLINES);

	if(!t.plotBox(window, false))
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

