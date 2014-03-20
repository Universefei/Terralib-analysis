#include "TerraManager.h"

#include <iostream>

using namespace std;

/*
 * Exemplo: Desenha textos sobre o canvas
 *             
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

	TeColor color(0, 0, 0);
	TeVisual v;
	v.color(color);
	v.width(0);
	v.family("");

	t.setDefaultVisual(v, TeTEXT);	

	TeCoord2D c;
	c.x_ = newWorldBox.x1_;
	
	c.y_ = newWorldBox.y1_ + ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 0"), 0.0, 0.5, 0.5))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	v.width(1);

	t.setDefaultVisual(v, TeTEXT);		

	c.y_ = newWorldBox.y1_ + 2 * ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 1"), 0.0, 0.5, 0.5))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	c.y_ = newWorldBox.y1_ + 3 * ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	v.width(2);

	t.setDefaultVisual(v, TeTEXT);	

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 2"), 0.0, 0.5, 0.5))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	c.y_ = newWorldBox.y1_ + 4 * ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	v.width(3);

	t.setDefaultVisual(v, TeTEXT);	

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 3"), 0.0, 0.5, 0.5))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	c.y_ = newWorldBox.y1_ + 5 * ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	v.width(4);

	t.setDefaultVisual(v, TeTEXT);	

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE DEFAULT TAMANHO 4"), 0.0, 0.5, 0.5))
	{
		cout << t.errorMessage() << endl;

		return 0;
	}

	v.width(24);
	v.family("c:\\windows\\fonts\\trebucbi.ttf");

	t.setDefaultVisual(v, TeTEXT);	

	c.y_ = newWorldBox.y1_ + 6 * ((newWorldBox.y2_ - newWorldBox.y1_) / 8);

	if(!t.plotText(c, string("ESCREVENDO SOBRE O CANVAS - FONTE TRUE TYPE"), 0.0, 0.5, 0.5))
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

