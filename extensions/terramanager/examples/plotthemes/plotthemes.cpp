#include "TerraManager.h"

#include <iostream>
#include <map>
#include <vector>
using namespace std;

/*
 * Exemplo: Conecta com um banco de dados e lista para cada vista armzenada no banco,
 *          o seu retângulo envolvente e plota os temas em cada vista, escreve um arquivo
 *          com o conteúdo do canvas por vista e um arquivo com a legenda da vista
 */

int main(int argc, char* argv[])
{
	TerraManager t;

	if(!t.connect(TeADODB, "C:/Mundo.mdb", "", "", "", 0))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}
	
	if(!t.setCurrentView("pegasus"))
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

	TeBox newWorldBox = t.setWorld(b.x1(), b.y1(), b.x2(), b.y2(), 800, 600);

	TeLegendEntryVectorThemeMap legend;

	vector<string> temas;

	t.getThemes(temas);

	TeActiveRepresLegendVector legendVec;

	for(unsigned int j = 0; j < temas.size(); ++j)
	{
		if(!t.setTheme(temas[j], 0))
		{
			cout << t.errorMessage() << endl;

			return 0;
		}

		//TeActiveRepresLegendVector legendVec;

		TeGrouping g;
		g.groupAttribute_.name_ = "AREA";
		g.groupNumSlices_ = 5;
		g.groupMode_ = TeEqualSteps;

		TeMemoryVisualMap visualMap;
		
		vector<string> rampColors;
		rampColors.push_back("GREEN");
			

		if(!t.plotVector(visualMap, legendVec, &g, rampColors))
		{
			cout << t.errorMessage() << endl;

			return 0;
		}
				
		legend[temas[j]] = legendVec;
	}	
			
	string fName  = "C:\\MUNDO.png";
			
	t.savePlot(fName);

	TeColor color(0, 0, 0);
	TeVisual v;
	v.color(color);
	v.width(0);
	v.family("");

	t.setDefaultVisual(v, TeTEXT);	


	t.plotLegend(legend, 350);
	
	

	t.saveLegend("C:\\mundlegend.png");

	return 0;
}

	




