// TerraManager include files
#include "TeGDLegend.h"

TeMANAGER::TeGDLegend::TeGDLegend(TeGDCanvas* gdCanvas)
	: gdCanvas_(gdCanvas)
{
}

TeMANAGER::TeGDLegend::~TeGDLegend()
{
}

void TeMANAGER::TeGDLegend::setTextVisual(TeVisual& vis)
{
	gdCanvas_->setTextColor(vis.color().red_, vis.color().green_, vis.color().blue_);
	gdCanvas_->setTextSize(vis.width());
	gdCanvas_->setTextStyle(vis.family());
}

void TeMANAGER::TeGDLegend::draw(std::vector<TeLegendEntryVector>& legends,
					  std::vector<std::string>& legendTitles,
					  const int& width)
{
// verifica quantas linhas tera a legenda: uma para cada legenda e mais uma para cada agrupamento
	int legendVecSize = 0;

	for(unsigned int i = 0; i < legends.size(); ++i)
	{
		if(legends[i].size() > 1)	// se existe mais de uma legenda, entao temos um agrupamento!
			legendVecSize += legends[i].size() + 1;
		else						// caso contrario, temos somente uma legenda
			++legendVecSize;
	}

	if(legendVecSize == 0)
		return;

// determina as dimensoes do canvas
	int legendHeight =  legendVecSize * 20;

	double nlin = (double)legendHeight / 20.0;
    double ncol = (double)width/20.0;

	TeCoord2D c1(0.0, 0.0);
	TeCoord2D c2(ncol, nlin);
	TeBox b(0.0, 0.0, ncol, nlin);

// ajusta o canvas
	gdCanvas_->setWorld(b, width, legendHeight);

// marca a posicao inicial de escrita
	int currentLinePosition = 0;

	double posd = legendVecSize - 1;

	TeCoord2D c(ncol/2.0, posd + 0.60);

// desenha as legendas
	for(unsigned int i = 0; i < legends.size(); ++i)
	{
		for(unsigned int j = 0; j < legends[i].size(); ++j)
		{
			posd = legendVecSize - 1 - currentLinePosition;

			if((j == 0) && legends[i].size() > 1)
			{
				TeCoord2D c(0.2, posd + 0.60);

				gdCanvas_->draw(c, legendTitles[i], 0.0, 0.5, 0.0);

				++currentLinePosition;

				posd = legendVecSize - 1 - currentLinePosition;
			}

			bool hasPOLYGONS = (legends[i][j].getVisualMap().find(TePOLYGONS) != legends[i][j].getVisualMap().end());
			bool hasLINES    = (legends[i][j].getVisualMap().find(TeLINES) != legends[i][j].getVisualMap().end());
			bool hasPOINTS   = (legends[i][j].getVisualMap().find(TePOINTS) != legends[i][j].getVisualMap().end());

			double dx = 0;

			if(hasPOLYGONS)
				dx = 1.75;

			if(hasLINES)
				dx += 1.75;

			if(hasPOINTS)
				dx += 1.75;

			if((j == 0) && legends[i].size() == 1)
			{
				TeCoord2D c(dx, posd + 0.60);

				gdCanvas_->draw(c, legendTitles[i], 0.0, 0.5, 0.0);
			}
			else
			{
				TeCoord2D c(dx, posd + 0.60);

				gdCanvas_->draw(c, legends[i][j].label(), 0.0, 0.5, 0.0);
			}

			if(hasPOLYGONS)
			{
				TeCoord2D c1, c2, c3, c4;
				
				c1.setXY(0.2, posd + 0.15);
				c2.setXY(0.2, posd + 0.9);
				c3.setXY(1.5, posd + 0.9);
				c4.setXY(1.5, posd + 0.15);

				TeLine2D l;
				l.add(c1); l.add(c2); l.add(c3); l.add(c4); l.add(c1);

				TeLinearRing r(l);

				TePolygon p;
				p.add(r);
				
				// Draw the legend polygon
				TeGeomRepVisualMap& visualMap = legends[i][j].getVisualMap();
				TeVisual& polygonVisual = *(visualMap[TePOLYGONS]);
				TeColor& polygonColor = polygonVisual.color();
				TeColor& polygonContourColor = polygonVisual.contourColor();

				gdCanvas_->setPolygonColor(polygonColor.red_, polygonColor.green_, polygonColor.blue_, polygonVisual.transparency());
				gdCanvas_->setPolygonStyle(polygonVisual.style());
				gdCanvas_->setPolygonLineColor(polygonContourColor.red_, polygonContourColor.green_, polygonContourColor.blue_);
				gdCanvas_->setPolygonLineStyle(polygonVisual.contourStyle(), polygonVisual.contourWidth());

				gdCanvas_->draw(p);
			}

			if(hasLINES)
			{
				dx = 0;

				if(hasPOLYGONS)
					dx = 1.75;

				TeCoord2D c1, c2, c3, c4;
				
				c1.setXY(0.2 + dx, posd + 0.15);
				c2.setXY(0.6 + dx, posd + 0.9);
				c3.setXY(1.0 + dx, posd + 0.15);
				c4.setXY(1.5 + dx, posd + 0.9);

				TeLine2D l;
				l.add(c1); l.add(c2); l.add(c3); l.add(c4);

				TeGeomRepVisualMap& visualMap = legends[i][j].getVisualMap();
				TeVisual& lnVisual = *(visualMap[TeLINES]);
				TeColor& lnColor = lnVisual.color();			

				gdCanvas_->setLineColor(lnColor.red_, lnColor.green_, lnColor.blue_);
				gdCanvas_->setLineStyle(lnVisual.style(), lnVisual.width() + 1);
	
				gdCanvas_->draw(l);
			}

			if(hasPOINTS)
			{
				dx = 0;

				if(hasPOLYGONS)
					dx = 1.75;

				if(hasLINES)
					dx += 1.75;

				TeCoord2D c(0.6 + dx, posd + 0.6);
				TePoint pt(c);				

				TeGeomRepVisualMap& visualMap = legends[i][j].getVisualMap();
				TeVisual& ptVisual = *(visualMap[TePOINTS]);
				TeColor& ptColor = ptVisual.color();

				gdCanvas_->setPointColor(ptColor.red_, ptColor.green_, ptColor.blue_);
				gdCanvas_->setPointStyle(ptVisual.style(), ptVisual.size() + 1);
				gdCanvas_->draw(pt);				
			}

			++currentLinePosition;
		}
	}
}

