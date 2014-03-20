#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um plano de informação de municípios brasileiros,
 *          plota o mapa agrupado pelo atributo LATITUDESE usando o modo de agrupamento
 *          passos iguais com 5 fatias, e gera a legenda do agrupamento
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
	rampColors.push_back("GREEN");
	rampColors.push_back("BLUE");

	TeLegendEntryVectorThemeMap legend;
	TeMemoryVisualMap visualMap;
	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(visualMap, legendVec, &g, rampColors))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}
	
	cout << endl;

	string fName  = "C:\\";
	       fName += "canvas";
		   fName += ".png";
		
	t.savePlot(fName);			

	TeColor color(0, 0, 0);
	TeVisual v;
	v.color(color);
	v.width(3);
	v.family("");

	t.setDefaultVisual(v, TeTEXT);

	legend["municipios(Passos Iguais - LATITUDESE)"] = legendVec;

	if(!t.plotLegend(legend, 350))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}	

	string f2Name  = "C:\\";
	       f2Name += "canvaslegend";
		   f2Name += ".png";
		
	t.saveLegend(f2Name);			

	return 0;
}

