#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Conecta com um banco de dados que possui um layer de municípios (poligonos),
 *          um de desmatamento (pontos) e outros de hidrografia (linhas),
 *          plotando o mapa de municipios agrupado pelo atributo LATITUDESE usando o modo de agrupamento
 *          passos iguais com 5 fatias, plota o layer de pontos e linhas de forma normal
 *          e então plota a legenda. Os objetos plotados serão restritos a um determinado box.
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
	
	unsigned int i = 0;

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


	TeLegendEntryVectorThemeMap legend;
	TeMemoryVisualMap visualMap;
	TeActiveRepresLegendVector legendVec;

	if(!t.plotVector(visualMap, legendVec, &g, rampColors))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	legend["municipios - Passos Iguais (LATITUDESE)"] = legendVec;

	if(!t.setTheme("RiosPermanentesBrasil", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	if(!t.plotVector(legendVec))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	legend["Rios Permanentes Brasil"] = legendVec;	

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

	legend["desmatamento 2004"] = legendVec;	
	
	cout << endl;

	string fName  = "C:\\";
	       fName += "canvas";
		   fName += ".png";
		
	t.savePlot(fName);
	
	TeColor color(0, 0, 0);
	TeVisual v;
	v.color(color);
	v.width(3);

	t.setDefaultVisual(v, TeTEXT);

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

