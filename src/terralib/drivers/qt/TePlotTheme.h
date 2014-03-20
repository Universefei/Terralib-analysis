/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/*! \file TePlotTheme.h
    \brief This file contains support to plot a theme on a canvas
*/
#ifndef  __TERRALIB_INTERNAL_PLOTTHEME_H
#define  __TERRALIB_INTERNAL_PLOTTHEME_H

#include "TeAppTheme.h"
#include "TeExternalTheme.h"
#include "TeDatabase.h"
#include "TeGeometryAlgorithms.h"
#include "TeVectorRemap.h"
#include "TeDecoderMemory.h"
#include <time.h>
#include <set>
#include "TeApplicationUtils.h"
#include <TeDatabaseUtils.h>

#include <TeThemeFunctions.h>
#include <PluginsSignal.h>

template<class Canvas, class Progress>
void TePlotTextWV(TeAppTheme* appTheme, Canvas* canvas, Progress * progress, TeBox box = TeBox())
{
	TeAbstractTheme* theme = appTheme->getTheme();

	if(theme->minScale() != 0 && theme->maxScale() != 0 && theme->minScale() > theme->maxScale())
	{
		if(theme->minScale() < canvas->scaleApx() || theme->maxScale() > canvas->scaleApx())
			return;
	}
	if ((theme->visibleRep() & TeTEXT) == 0)
		return;

	if(appTheme->textTable().empty())
		return;

	TeDatabase* db = appTheme->getLocalDatabase();

	if(!db || !db->tableExist(appTheme->textTable()))
		return;
	canvas->setDataProjection(theme->getThemeProjection());
	TeDatabasePortal* plot = db->getPortal();

	string	tabtx = appTheme->textTable();
	string	tabs = appTheme->getLocalCollectionTable();

	string wherebox;
	if(box.isValid())
	{
		wherebox = "NOT (x >= " + Te2String(box.x2()+4*box.width());
		wherebox += " OR y >= " + Te2String(box.y2()+2*box.height());
		wherebox += " OR x <= " + Te2String(box.x1()-4*box.width());
		wherebox += " OR y <= " + Te2String(box.y1()-2*box.height()) + ")";
	}
	else
	{
		wherebox = getWhereBox(plot, appTheme, canvas, TeTEXT, box);
		if(wherebox.empty())
		{
			TeBox box = canvas->getDataWorld();
			wherebox = " x >= " + Te2String(box.x1());
			wherebox += " AND x <= " + Te2String(box.x2());
			wherebox += " AND y >= " + Te2String(box.y1());
			wherebox += " AND y <= " + Te2String(box.y2());
		}
	}

	// calculate initial filed number for collection and legenda (sind and vind)
	int sind=0, vind=0;
	string ind = "SELECT * FROM " + tabtx + " WHERE object_id < ' '";
	plot->freeResult();
	if (plot->query (ind))
		sind = plot->getAttributeList().size();

	ind = "SELECT * FROM " + tabs + " WHERE c_object_id < ' '";
	plot->freeResult();
	if (plot->query (ind))
		vind = plot->getAttributeList().size() + sind;

	int numrows = 0;
	string conta = "SELECT COUNT(*) FROM " + tabs;
	plot->freeResult();
	if (plot->query (conta))
	{
		if(plot->fetchRow())
			numrows = atoi(plot->getData(0));
	}
	delete plot;

	string tabv = tabtx + "_txvisual";
	string sa = "SELECT " + tabtx + ".*, " + tabv + ".*, " + tabs + ".* ";
	sa += "FROM " + tabtx + ", " + tabv + ", " + tabs;
	sa += " WHERE (" + tabs + ".c_object_id = " + tabtx + ".object_id";
	sa += " AND " + tabtx + ".geom_id = " + tabv + ".geom_id)";
	string squery;
	squery = sa + " AND (" + wherebox + ")";
	plotTextWV(appTheme, canvas, squery, progress);

	string othersQuery = "SELECT " + tabtx + ".*, " + tabv + ".* ";
	othersQuery += "FROM " + tabtx + ", " + tabv;
	othersQuery += " WHERE " + tabtx + ".geom_id = " + tabv + ".geom_id";
	othersQuery += " AND " + tabtx + ".object_id = 'TE_NONE_ID'";
	plotTextWV(appTheme, canvas, othersQuery, progress);
}

template<class Canvas, class Progress>
void plotTextWV(TeAppTheme *appTheme, Canvas *canvas, string query, Progress *progress)
{
	TeText	text;
	int	np = 0;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;

	TeAbstractTheme* theme = appTheme->getTheme();
	TeDatabase* db = appTheme->getLocalDatabase();
	if(!db)
		return;

	TeDatabasePortal* plot = db->getPortal();
	
	if(progress == 0)
	{
		if(plot->query(query) && plot->fetchRow())
		{
			t1 = clock();
			t2 = t1;
			bool flag = true;
			do
			{
				bool bold = false, italic = false;
				int	size;
				int dot_height = atoi(plot->getData(10));
				bool fixedSize = atoi(plot->getData(11));
				int rgb = atoi(plot->getData(12));
				TeColor	cor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
				string	family = plot->getData(13);
				if(atoi(plot->getData(14)))
					bold = true;
				if(atoi(plot->getData(15)))
					italic = true;

				flag = plot->fetchGeometry(text);
				if(fixedSize == false && text.height() > 0.)
				{
					TeBox wbox = canvas->getDataWorld();
					TeCoord2D p1(wbox.x1_, wbox.y1_);
					TeCoord2D p2(p1.x() + text.height(), p1.y() + text.height());
					TeBox box(p1, p2);
					canvas->mapDWtoV(box);
					size = int(box.height());
				}
				else
					size = dot_height;

				canvas->setTextColor(cor.red_, cor.green_, cor.blue_);
				canvas->setTextStyle (family, size, bold, italic );
				TeVisual visual;
				visual.size(size);
				visual.family(family);
				visual.bold(bold);
				visual.italic(italic);
				visual.fixedSize(fixedSize);
				visual.ptAngle((int)text.angle());

				canvas->plotText (text, visual);
				np++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
				}
			} while (flag);
			canvas->copyPixmapToWindow();
		}
	}
	else
	{
		int steps = 0;
		string conta = query;
		int ff;
		if((ff=conta.find(" FROM ")) >= 0)
			conta.replace(0, ff, "SELECT COUNT(*)");
		if((ff=conta.find("ORDER BY")) >= 0)
			conta = conta.substr(0, ff);
		plot->freeResult();
		if(plot->query(conta) && plot->fetchRow())
			steps = atoi(plot->getData(0));

		plot->freeResult();
		if(plot->query(query) && plot->fetchRow())
		{
			string caption = "Plotando tema: " + theme->name();
			progress->setCaption(caption.c_str());
			progress->setLabelText("      Plotagem de textos em andamento, aguarde...      ");
			progress->setTotalSteps(steps);
			t2 = clock();
			t0 = t1 = t2;
			bool flag = true;
			do
			{
				bool bold = false, italic = false;
				int	size;
				int dot_height = atoi(plot->getData(10));
				bool fixedSize = atoi(plot->getData(11));
				int rgb = atoi(plot->getData(12));
				TeColor	cor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
				string	family = plot->getData(13);
				if(atoi(plot->getData(14)))
					bold = true;
				if(atoi(plot->getData(15)))
					italic = true;

				flag = plot->fetchGeometry(text);
				if(fixedSize == false && text.height() > 0.)
				{
					TeBox wbox = canvas->getDataWorld();
					TeCoord2D p1(wbox.x1_, wbox.y1_);
					TeCoord2D p2(p1.x() + text.height(), p1.y() + text.height());
					TeBox box(p1, p2);
					canvas->mapDWtoV(box);
					size = int(box.height());
					size = (int)((double)size / canvas->printerFactor() +.5);
				}
				else
					size = dot_height;

				canvas->setTextColor(cor.red_, cor.green_, cor.blue_);
				canvas->setTextStyle (family, size, bold, italic );
				TeVisual visual;
				visual.size(size);
				visual.family(family);
				visual.bold(bold);
				visual.italic(italic);
				visual.fixedSize(fixedSize);
				visual.ptAngle((int)text.angle());

				canvas->plotText (text, visual);
				np++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
					if(progress->wasCancelled())
					{
						progress->reset();
						break;
					}
					if((int)(t2-t0) > dt2)
						progress->setProgress(np);
				}
			} while (flag);
			canvas->copyPixmapToWindow();
		}
		progress->reset();
	}
	delete plot;
}

template<class Canvas, class Progress>
void TePlotTexts(TeAppTheme* appTheme, Canvas* canvas, Progress * progress)
{
	TeTheme* theme = static_cast<TeTheme*> (appTheme->getTheme());
	TeDatabase* db = appTheme->getLocalDatabase(); 
	if(!db)
		return;
	
	if(theme->minScale() != 0 && theme->maxScale() != 0 && theme->minScale() > theme->maxScale())
	{
		if(theme->minScale() < canvas->scaleApx() || theme->maxScale() > canvas->scaleApx())
			return;
	}
	if ((theme->visibleRep() & TeTEXT) == 0)
		return;

	if(appTheme->textTable().empty())
		return;
	
	if(db->tableExist(appTheme->textTable()) == false)
		return;
	canvas->setDataProjection (theme->layer()->projection());
	TeDatabasePortal* plot = db->getPortal();

	string	tabtx = appTheme->textTable();
	string	tabs = appTheme->getLocalCollectionTable(); 
	if(tabs.empty())
		return;

	string wherebox = getWhereBox(plot, theme, canvas, TeTEXT);
	if(wherebox.empty())
	{
		TeBox box = canvas->getDataWorld();
		wherebox = " x >= " + Te2String(box.x1());
		wherebox += " AND x <= " + Te2String(box.x2());
		wherebox += " AND y >= " + Te2String(box.y1());
		wherebox += " AND y <= " + Te2String(box.y2());
	}

	// calculate initial filed number for collection and legenda (sind and vind)
	int sind=0, vind=0;
	string ind = "SELECT * FROM " + tabtx + " WHERE object_id < ' '";
	plot->freeResult();
	if (plot->query (ind))
		sind = plot->getAttributeList().size();

	ind = "SELECT * FROM " + tabs + " WHERE c_object_id < ' '";
	plot->freeResult();
	if (plot->query (ind))
		vind = plot->getAttributeList().size() + sind;

	int numrows = 0;
	string conta = "SELECT COUNT(*) FROM " + tabs;
	plot->freeResult();
	if (plot->query (conta))
	{
		if(plot->fetchRow())
			numrows = atoi(plot->getData(0));
	}
	string sa;
	string sel;
	if(plot->getDatabase()->dbmsName() == "OracleSpatial")	
		sel = " SELECT /*+ ordered */ * "; 
	else
		sel = " SELECT * ";

	delete plot;
	if(numrows > 0)
		sa = sel + " FROM (" + tabtx + " INNER JOIN " + tabs;
	else
		sa = sel + " FROM (" + tabtx + " LEFT JOIN " + tabs;

	sa += " ON " + tabtx + ".object_id = " + tabs + ".c_object_id) WHERE ";

	string squery;
	squery = sa + wherebox;
	plotTexts(appTheme, canvas, squery, progress);

	string othersQuery = "SELECT * FROM " + tabtx + " WHERE object_id = 'TE_NONE_ID'";
	plotTexts(appTheme, canvas, othersQuery, progress);
}

template<class Canvas, class Progress>
void plotTexts(TeAppTheme *appTheme, Canvas *canvas, string query, Progress *progress)
{
	TeText	text;
	int	np = 0;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	
	TeAbstractTheme* theme = appTheme->getTheme();
	TeDatabase* db = appTheme->getLocalDatabase();
	if(!db)
		return;
	
	TeDatabasePortal* plot = db->getPortal();

	if(progress == 0)
	{
		if(plot->query(query) && plot->fetchRow())
		{
			t1 = clock();
			t2 = t1;
			bool flag = true;
			do
			{
				flag = plot->fetchGeometry(text);
				setTextAttributes(theme, canvas, text);
				canvas->plotText (text, theme->defaultLegend().visual(TeTEXT));
				np++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
				}
			} while (flag);
			canvas->copyPixmapToWindow();
		}
	}
	else
	{
		int steps = 0;
		string conta = query;
		conta.replace(0, strlen("SELECT *")+1, "SELECT COUNT(*)");
		int ff;
		if((ff=conta.find("ORDER BY")) >= 0)
			conta = conta.substr(0, ff);
		plot->freeResult();
		if(plot->query(conta) && plot->fetchRow())
			steps = atoi(plot->getData(0));

		plot->freeResult();
		if(plot->query(query) && plot->fetchRow())
		{
			string caption = "Plotando tema: " + theme->name();
			progress->setCaption(caption.c_str());
			progress->setLabelText("      Plotagem de textos em andamento, aguarde...      ");
			progress->setTotalSteps(steps);
			t2 = clock();
			t0 = t1 = t2;
			bool flag = true;
			do
			{
				flag = plot->fetchGeometry(text);
				setTextAttributes(theme, canvas, text);
				canvas->plotText (text, theme->defaultLegend().visual(TeTEXT));
				np++;
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
					if(progress->wasCancelled())
					{
						progress->reset();
						break;
					}
					if((int)(t2-t0) > dt2)
						progress->setProgress(np);
				}
			} while (flag);
			canvas->copyPixmapToWindow();
		}
		progress->reset();
	}
	delete plot;
}

template<class Canvas>
void setTextAttributes(TeAppTheme* appTheme, Canvas* canvas, TeText tx)
{
	TeColor	cor;
	static	TeVisual* snc;
	string	family;
	static int		ssize;
	int		size = 0;
	bool	bold, italic, fixedSize;

	if(appTheme == 0)
	{
		ssize = 0;
		snc->family("notFamily");
		return;
	}

	TeAbstractTheme* theme = appTheme->getTheme();

	fixedSize = theme->defaultLegend().visual(TeTEXT)->fixedSize(); // font size is fixed
	if(snc == theme->defaultLegend().visual(TeTEXT))
	{
		if(fixedSize == false && tx.height() > 0.)
		{
			TeBox wbox = canvas->getDataWorld();
			TeCoord2D p1(wbox.x1_, wbox.y1_);
			TeCoord2D p2(p1.x() + tx.height(), p1.y() + tx.height());
			TeBox box(p1, p2);
			canvas->mapDWtoV(box);
			size = int(box.height());
		}
		else
			size = theme->defaultLegend().visual(TeTEXT)->size(); // font size
	}

	if(size == 0)
	{
		if(fixedSize == false && tx.height() > 0.)
		{
			TeBox wbox = canvas->getDataWorld();
			TeCoord2D p1(wbox.x1_, wbox.y1_);
			TeCoord2D p2(p1.x() + tx.height(), p1.y() + tx.height());
			TeBox box(p1, p2);
			canvas->mapDWtoV(box);
			size = int(box.height());
		}
		else
			size = theme->defaultLegend().visual(TeTEXT)->size(); // font size
	}

	cor = theme->defaultLegend().visual(TeTEXT)->color();
	family = theme->defaultLegend().visual(TeTEXT)->family();
	bold = theme->defaultLegend().visual(TeTEXT)->bold();
	italic = theme->defaultLegend().visual(TeTEXT)->italic();
	fixedSize = theme->defaultLegend().visual(TeTEXT)->fixedSize(); // font size is fixed

	canvas->setTextColor(cor.red_, cor.green_, cor.blue_);
	canvas->setTextStyle (family, size, bold, italic );
	snc = theme->defaultLegend().visual(TeTEXT);
	ssize = size;
}

template<class Canvas, class Progress>
void plotPieBars(TeAppTheme *appTheme, Canvas *canvas, Progress *progress, TeBox ebox = TeBox())
{
	if(	appTheme->getTheme()->getProductId() != TeTHEME && 
		appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
		return;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();
	if(theme->minScale() != 0 && theme->maxScale() != 0 && theme->minScale() > theme->maxScale())
	{
		if(theme->minScale() < canvas->scaleApx() || theme->maxScale() > canvas->scaleApx())
			return;
	}
	if((theme->visibleRep() & 0x80000000) == 0)
		return;

	int steps = 0;
	int	np = 0;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 2;
	clock_t	t0, t1, t2;
	
	TeDatabase* db = 0;
	TeLayer* layer = 0;
	if(theme->getProductId() == TeTHEME)
		layer = theme->layer();
	else if(theme->getProductId() == TeEXTERNALTHEME)
		layer = ((TeExternalTheme*)theme)->getRemoteTheme()->layer();

		db = layer->database();

	if(!db)
		return; 

	TeDatabasePortal* plot = db->getPortal();

	canvas->setDataProjection (layer->projection());

	if(appTheme->chartAttributes_.size() == 0)
	{
		delete plot;
		return;
	}
	double	width = appTheme->barWidth();
	double	maxh = appTheme->barMaxHeight();
	double	minh = appTheme->barMinHeight();
	double	maxd = appTheme->pieMaxDiameter();
	double	mind = appTheme->pieMinDiameter();
	double	diameter = appTheme->pieDiameter();
	string FUNC = appTheme->chartFunction();

	if(appTheme->keepDimension() != 0)
	{
		width = canvas->mapVtoDW((int)width);
		maxh = canvas->mapVtoDW((int)maxh);
		minh = canvas->mapVtoDW((int)minh);
		maxd = canvas->mapVtoDW((int)maxd);
		mind = canvas->mapVtoDW((int)mind);
		diameter = canvas->mapVtoDW((int)diameter);
	}

	vector<string> itenVec, objVec;
	unsigned int i;
	string	input, cinput, sqlGridFrom, CT, CE;
	TeSelectedObjects  selectedObjects = appTheme->chartObjects();
	if(theme->getProductId() == TeTHEME)
	{
		sqlGridFrom = theme->sqlGridFrom();
		CT = theme->collectionTable();
		CE = theme->collectionAuxTable();
		if(selectedObjects == TeSelectedByPointing)
		{
			input = "(grid_status = 1 OR grid_status = 3";
			input += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3)))";
			cinput = "(c_object_status = 1 OR c_object_status = 3)";
		}
		else if(selectedObjects == TeNotSelectedByPointing)
		{
			input = "(grid_status = 0 OR grid_status = 2";
			input += " OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2)))";
			cinput = "(c_object_status = 0 OR c_object_status = 2)";
		}
		else if(selectedObjects == TeSelectedByQuery)
		{
			input = "(grid_status = 2 OR grid_status = 3";
			input += " OR (grid_status is null AND (c_object_status = 2 OR c_object_status = 3)))";
			cinput = "(c_object_status = 2 OR c_object_status = 3)";
		}
		else if(selectedObjects == TeNotSelectedByQuery)
		{
			input = "(grid_status = 0 OR grid_status = 1";
			input += " OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 1)))";
			cinput = "(c_object_status = 0 OR c_object_status = 1)";
		}
		else if(selectedObjects == TeGrouped)
		{
			input = "c_legend_id <> 0";
			cinput = "c_legend_id <> 0";
		}
		else if(selectedObjects == TeNotGrouped)
		{
			input = "c_legend_id = 0";
			cinput = "c_legend_id = 0";
		}
	}
	else if(theme->getProductId() == TeEXTERNALTHEME)
	{
		itenVec = getItems(theme, selectedObjects);
		objVec = getObjects(theme, selectedObjects);
		sqlGridFrom = ((TeExternalTheme*)theme)->getRemoteTheme()->sqlGridFrom();
		CT = ((TeExternalTheme*)theme)->getRemoteTheme()->collectionTable();
		CE = ((TeExternalTheme*)theme)->getRemoteTheme()->collectionAuxTable();
	}

	canvas->setLineStyle(TeLnTypeContinuous, 0);
	canvas->setLineColor(0, 0, 0);
	TeChartType chartType = (TeChartType)appTheme->chartType();
	if(chartType == TePieChart)
	{
		string wherebox;
		double delta = diameter / 2.;
		if(!(appTheme->pieDimAttribute() == "NONE"))
			delta = maxd / 2.;

		TeBox box;
		if(ebox.isValid())
			box = ebox;
		else
			box = canvas->getDataWorld();

		box.x1_ = box.x1_ - delta;
		box.x2_ = box.x2_ + delta;
		box.y1_ = box.y1_ - delta;
		box.y2_ = box.y2_ + delta;

		wherebox = "NOT(label_x > " + Te2String(box.x2_, 10) + " OR ";
		wherebox = wherebox + "label_x < " + Te2String(box.x1_, 10) + " OR ";
		wherebox = wherebox + "label_y > " + Te2String(box.y2_, 10) + " OR ";
		wherebox = wherebox + "label_y < " + Te2String(box.y1_, 10) + ")";

		string attrs, q;
		for(i=0; i<appTheme->chartAttributes_.size(); i++)
			attrs += FUNC + "(" + appTheme->chartAttributes_[i] + "),";
		attrs += "AVG(label_x),AVG(label_y)";
		if(!(appTheme->pieDimAttribute() == "NONE"))
			attrs += "," + FUNC + "(" + appTheme->pieDimAttribute() + ")";

		if(theme->getProductId() == TeTHEME)
		{
			if(wherebox.empty() && itenVec.empty())
				q = "SELECT " + attrs + sqlGridFrom;
			else if(wherebox.empty() && input.empty()==false)
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + input;
			else if(wherebox.empty()==false && input.empty())
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + wherebox;
			else if(wherebox.empty()==false && input.empty()==false)
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + input + " AND " + wherebox;
			q += " GROUP BY " + CT + ".c_object_id";

			string conta = "SELECT COUNT(*) FROM " + CT;
			if(wherebox.empty() && cinput.empty()==false)
				conta += " WHERE " + cinput;
			else if(wherebox.empty()==false && cinput.empty())
				conta += " WHERE " + wherebox;
			else if(wherebox.empty()==false && cinput.empty()==false)
				conta += " WHERE " + cinput + " AND " + wherebox;
			
			plot->freeResult();
			if(plot->query(conta) && plot->fetchRow())
				steps = atoi(plot->getData(0));

			plot->freeResult();
			if(plot->query(q))
			{
				if(progress)
				{
					string caption = "Plotando tema: " + theme->name();
					progress->setCaption(caption.c_str());
					progress->setLabelText("      Plotagem de tortas em andamento, aguarde...      ");
					progress->setTotalSteps(steps);
				}
				t2 = clock();
				t0 = t1 = t2;
				bool b = plot->fetchRow(0);
				while(b)
				{
					double tot = 0.;
					for(i=0; i<appTheme->chartAttributes_.size(); i++)
					{
						string v = plot->getData(i);
						if(v.empty())
						{
							tot = 0.;
							break;
						}
						tot += atof(v.c_str());
					}
					if(tot == 0.)
					{
						b = plot->fetchRow();
						np++;
						continue;
					}

					if(!(appTheme->pieDimAttribute() == "NONE"))
					{
						if(appTheme->chartMaxVal() - appTheme->chartMinVal() == 0)
							diameter = 0.;
						else
						{
							double	adim = atof(plot->getData(appTheme->chartAttributes_.size()+2));
							double a = (adim - appTheme->chartMinVal()) / (appTheme->chartMaxVal() - appTheme->chartMinVal());
							diameter = a * (maxd - mind) + mind;
						}
					}
					double	x = atof(plot->getData(appTheme->chartAttributes_.size())) - diameter / 2.;
					double	y = atof(plot->getData(appTheme->chartAttributes_.size()+1)) - diameter / 2.;

					double	a = 0.;
					for(i=0; i<appTheme->chartAttributes_.size(); i++)
					{
						double b = atof(plot->getData(i)) / tot * 360.;
						TeColor	cor = appTheme->chartColors_[i];
						canvas->setPieColor(cor.red_, cor.green_, cor.blue_);
						canvas->plotPie(x, y, diameter, diameter, a, b);
						a += b;
					}
					np++;
					t2 = clock();
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmapToWindow();
						if(progress)
						{
							if(progress->wasCancelled())
							{
								progress->reset();
								break;
							}
							if((int)(t2-t0) > dt2)
								progress->setProgress(np);
						}
					}
					b = plot->fetchRow();
				}
				canvas->copyPixmapToWindow();
			}
		}
		else if(theme->getProductId() == TeEXTERNALTHEME)
		{
			string conta = "SELECT COUNT(*) FROM " + CT;			
			plot->freeResult();
			if(plot->query(conta) && plot->fetchRow())
				steps = atoi(plot->getData(0));
        
      std::vector< std::string >::iterator it_begin = itenVec.begin();
      std::vector< std::string >::iterator it_end = itenVec.end();

			vector<string> sv = generateItemsInClauses(it_begin, it_end, theme);
			vector<string>::iterator it;
			for(it=sv.begin(); it!=sv.end(); ++it)
			{
				string s = *it;
				string q = "SELECT " + attrs + sqlGridFrom;
				if((*it).empty() == false && wherebox.empty())
					q += " WHERE " + CE + ".unique_id IN " + *it;
				else if((*it).empty() && wherebox.empty()==false)
					q += " WHERE " + wherebox;
				else if((*it).empty()==false && wherebox.empty()==false)
					q += " WHERE " + CE + ".unique_id IN " + *it + " AND " + wherebox;
				q += " GROUP BY " + CT + ".c_object_id";

				plot->freeResult();
				if(plot->query(q))
				{
					if(progress)
					{
						string caption = "Plotando tema: " + theme->name();
						progress->setCaption(caption.c_str());
						progress->setLabelText("      Plotagem de tortas em andamento, aguarde...      ");
						progress->setTotalSteps(steps);
					}
					t2 = clock();
					t0 = t1 = t2;
					bool b = plot->fetchRow(0);
					while(b)
					{
						double tot = 0.;
						for(i=0; i<appTheme->chartAttributes_.size(); i++)
						{
							string v = plot->getData(i);
							if(v.empty())
							{
								tot = 0.;
								break;
							}
							tot += atof(v.c_str());
						}
						if(tot == 0.)
						{
							b = plot->fetchRow();
							np++;
							continue;
						}

						if(!(appTheme->pieDimAttribute() == "NONE"))
						{
							if(appTheme->chartMaxVal() - appTheme->chartMinVal() == 0)
								diameter = 0.;
							else
							{
								double	adim = atof(plot->getData(appTheme->chartAttributes_.size()+2));
								double a = (adim - appTheme->chartMinVal()) / (appTheme->chartMaxVal() - appTheme->chartMinVal());
								diameter = a * (maxd - mind) + mind;
							}
						}
						double	x = atof(plot->getData(appTheme->chartAttributes_.size())) - diameter / 2.;
						double	y = atof(plot->getData(appTheme->chartAttributes_.size()+1)) - diameter / 2.;

						double	a = 0.;
						for(i=0; i<appTheme->chartAttributes_.size(); i++)
						{
							double b = atof(plot->getData(i)) / tot * 360.;
							TeColor	cor = appTheme->chartColors_[i];
							canvas->setPieColor(cor.red_, cor.green_, cor.blue_);
							canvas->plotPie(x, y, diameter, diameter, a, b);
							a += b;
						}
						np++;
						t2 = clock();
						if (int(t2-t1) > dt)
						{
							t1 = t2;
							canvas->copyPixmapToWindow();
							if(progress)
							{
								if(progress->wasCancelled())
								{
									progress->reset();
									break;
								}
								if((int)(t2-t0) > dt2)
									progress->setProgress(np);
							}
						}
						b = plot->fetchRow();
					}
					canvas->copyPixmapToWindow();
				}
			}
		}
	}
	else
	{
		double	n = appTheme->chartAttributes_.size();
		double	maxv = appTheme->chartMaxVal();
		double	minv = appTheme->chartMinVal();
		double delta = maxh / 2.;

		string wherebox;

		TeBox box;
		if(ebox.isValid())
			box = ebox;
		else
			box = canvas->getDataWorld();

		box.x1_ = box.x1_ - delta;
		box.x2_ = box.x2_ + delta;
		box.y1_ = box.y1_ - delta;
		box.y2_ = box.y2_ + delta;

		wherebox = "NOT(label_x > " + Te2String(box.x2_, 10) + " OR ";
		wherebox = wherebox + "label_x < " + Te2String(box.x1_, 10) + " OR ";
		wherebox = wherebox + "label_y > " + Te2String(box.y2_, 10) + " OR ";
		wherebox = wherebox + "label_y < " + Te2String(box.y1_, 10) + ")";

		string attrs, q;
		for(i=0; i<appTheme->chartAttributes_.size(); i++)
			attrs += FUNC + "(" + appTheme->chartAttributes_[i] + "),";
		attrs += "AVG(label_x),AVG(label_y)";

		if(theme->getProductId() == TeTHEME)
		{
			if(wherebox.empty() && input.empty())
				q = "SELECT " + attrs + sqlGridFrom;
			else if (wherebox.empty() == false && input.empty() == false)
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + input + " AND " + wherebox;
			else if (wherebox.empty() == false && input.empty())
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + wherebox;
			else if (wherebox.empty() && input.empty() == false)
				q = "SELECT " + attrs + sqlGridFrom + " WHERE " + input;
			q += " GROUP BY " + CT + ".c_object_id";

			string conta = "SELECT COUNT(*) FROM " + CT;
			if(wherebox.empty() && cinput.empty()==false)
				conta += " WHERE " + cinput;
			else if(wherebox.empty()==false && cinput.empty())
				conta += " WHERE " + wherebox;
			else if(wherebox.empty()==false && cinput.empty()==false)
				conta += " WHERE " + cinput + " AND " + wherebox;

			plot->freeResult();
			if(plot->query(conta) && plot->fetchRow())
				steps = atoi(plot->getData(0));

			vector<double> dvec;
			plot->freeResult();
			if(plot->query(q))
			{
				if(progress)
				{
					string caption = "Plotando tema: " + theme->name();
					progress->setCaption(caption.c_str());
					progress->setLabelText("      Plotagem de barras em andamento, aguarde...      ");
					progress->setTotalSteps(steps);
				}
				t2 = clock();
				t0 = t1 = t2;
				bool b = plot->fetchRow(0);
				while(b)
				{
					double	x = atof(plot->getData(appTheme->chartAttributes_.size())) - n * width / 2.;
					double	y = atof(plot->getData(appTheme->chartAttributes_.size()+1));

					dvec.clear();
					for(i=0; i<appTheme->chartAttributes_.size(); i++)
					{
						string val = plot->getData(i);
						if(val.empty())
						{
							dvec.clear();
							break;
						}
						dvec.push_back(atof(val.c_str()));
					}

					for(i=0; i<dvec.size(); i++)
					{
						double height;
						double v = dvec[i];
						if(maxv - minv == 0)
							height = 0.;
						else
							height = (v - minv) * (maxh - minh) / (maxv - minv) + minh;
						TeColor	cor = appTheme->chartColors_[i];
						canvas->setRectColor(cor.red_, cor.green_, cor.blue_);
						canvas->plotRect(x, y, width, height);
						x += width;
					}
					np++;
					t2 = clock();
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmapToWindow();
						if(progress)
						{
							if(progress->wasCancelled())
							{
								progress->reset();
								break;
							}
							if((int)(t2-t0) > dt2)
								progress->setProgress(np);
						}
					}
					b = plot->fetchRow();
				}
				canvas->copyPixmapToWindow();
			}
		}
		if(theme->getProductId() == TeEXTERNALTHEME)
		{
			string conta = "SELECT COUNT(*) FROM " + CT;			
			plot->freeResult();
			if(plot->query(conta) && plot->fetchRow())
				steps = atoi(plot->getData(0));

      std::vector< std::string >::iterator it_begin = itenVec.begin();
      std::vector< std::string >::iterator it_end = itenVec.end();

      vector<string> sv = generateItemsInClauses(it_begin, it_end, theme);

			vector<string>::iterator it;
			for(it=sv.begin(); it!=sv.end(); ++it)
			{
				string s = *it;
				string q = "SELECT " + attrs + sqlGridFrom;
				if((*it).empty() == false && wherebox.empty())
					q += " WHERE " + CE + ".unique_id IN " + *it;
				else if((*it).empty() && wherebox.empty()==false)
					q += " WHERE " + wherebox;
				else if((*it).empty()==false && wherebox.empty()==false)
					q += " WHERE " + CE + ".unique_id IN " + *it + " AND " + wherebox;
				q += " GROUP BY " + CT + ".c_object_id";

				vector<double> dvec;
				plot->freeResult();
				if(plot->query(q))
				{
					if(progress)
					{
						string caption = "Plotando tema: " + theme->name();
						progress->setCaption(caption.c_str());
						progress->setLabelText("      Plotagem de barras em andamento, aguarde...      ");
						progress->setTotalSteps(steps);
					}
					t2 = clock();
					t0 = t1 = t2;
					bool b = plot->fetchRow(0);
					while(b)
					{
						double	x = atof(plot->getData(appTheme->chartAttributes_.size())) - n * width / 2.;
						double	y = atof(plot->getData(appTheme->chartAttributes_.size()+1));

						dvec.clear();
						for(i=0; i<appTheme->chartAttributes_.size(); i++)
						{
							string val = plot->getData(i);
							if(val.empty())
							{
								dvec.clear();
								break;
							}
							dvec.push_back(atof(val.c_str()));
						}

						for(i=0; i<dvec.size(); i++)
						{
							double height;
							double v = dvec[i];
							if(maxv - minv == 0)
								height = 0.;
							else
								height = (v - minv) * (maxh - minh) / (maxv - minv) + minh;
							TeColor	cor = appTheme->chartColors_[i];
							canvas->setRectColor(cor.red_, cor.green_, cor.blue_);
							canvas->plotRect(x, y, width, height);
							x += width;
						}
						np++;
						t2 = clock();
						if (int(t2-t1) > dt)
						{
							t1 = t2;
							canvas->copyPixmapToWindow();
							if(progress)
							{
								if(progress->wasCancelled())
								{
									progress->reset();
									break;
								}
								if((int)(t2-t0) > dt2)
									progress->setProgress(np);
							}
						}
						b = plot->fetchRow();
					}
					canvas->copyPixmapToWindow();
				}
			}
		}
	}
	if(progress)
		progress->reset();
	delete plot;
}


template<class Portal, class Canvas>
string getWhereBox(Portal *plot, TeAppTheme *appTheme, Canvas *canvas, TeGeomRep rep, TeBox bbox = TeBox())
{
	string wherebox;
	if(	appTheme->getTheme()->getProductId() != TeTHEME && 
		appTheme->getTheme()->getProductId() != TeEXTERNALTHEME )
		return wherebox;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();
	TeLayer* layer = theme->layer();
	std::string tableName = layer->tableName(rep);

	if(appTheme->chartSelected_.empty() == true)
	{
		TeBox box;
		if(bbox.isValid())
			box = bbox;
		else
		{
			if(canvas->canvasAndDataProjectionEqual())
			{
				box = canvas->getDataWorld();
			}
			else
			{
				TeBox canvasBox = TeRemapBoxPC2Geodetic(canvas->getWorld(), canvas->projection());
				TeBox layerBox = TeRemapBoxPC2Geodetic(layer->box(), layer->projection());

				TeBox ibox;
				if (TeIntersection(canvasBox, layerBox, ibox))
					box = TeRemapBoxGeodetic2PC(ibox, layer->projection());
			}
		}
		wherebox = plot->getDatabase()->getSQLBoxWhere(box, rep, tableName);
	}
	else
	{
		if (rep == TePOINTS)
		{
			double	width = appTheme->barWidth();
			double	maxh = appTheme->barMaxHeight();
			double	minh = appTheme->barMinHeight();
			double	maxd = appTheme->pieMaxDiameter();
			double	mind = appTheme->pieMinDiameter();
			double	diameter = appTheme->pieDiameter();
			double	delta;

			if(appTheme->keepDimension() != 0)
			{
				width = canvas->mapVtoCW((int)width);
				maxh = canvas->mapVtoCW((int)maxh);
				minh = canvas->mapVtoCW((int)minh);
				maxd = canvas->mapVtoCW((int)maxd);
				mind = canvas->mapVtoCW((int)mind);
				diameter = canvas->mapVtoCW((int)diameter);
			}

			TeChartType chartType = (TeChartType)appTheme->chartType();
			if(chartType == TePieChart)
			{
				delta = diameter / 2.;
				if(!(appTheme->pieDimAttribute() == "NONE"))
					delta = maxd / 2.;
			}
			else
				delta = maxh / 2.;

			string sel = "SELECT label_x, label_y FROM " + theme->collectionTable();
			sel += " WHERE c_object_id = '" + appTheme->chartSelected_ + "'";
			plot->freeResult();
			if(plot->query(sel))
			{
				bool b = plot->fetchRow(0);
				if(b == false)
					return wherebox;
				if(b)
				{
					double x = atof(plot->getData(0));
					double y = atof(plot->getData(1));
					string xmin = Te2String(x - delta);
					string xmax = Te2String(x + delta);
					string ymin = Te2String(y - delta);
					string ymax = Te2String(y + delta);
					wherebox = "label_x > " + xmin + " AND ";
					wherebox += "label_x < " + xmax + " AND ";
					wherebox += "label_y > " + ymin + " AND ";
					wherebox += "label_y < " + ymax;
				}
			}
		}
		else if(rep == TePOLYGONS || rep == TeLINES || rep == TeCELLS)
		{
			string tab;
			if(rep == TePOLYGONS)
				tab = theme->layer()->tableName(TePOLYGONS);
			else if (rep == TeLINES)
				tab = theme->layer()->tableName(TeLINES);
			else if (rep == TeCELLS)
				tab = theme->layer()->tableName(TeCELLS);
			
			plot->freeResult();

			TeBox b;
			bool result = plot->getDatabase()->getMBRGeom(tab, appTheme->chartSelected_, b, "spatial_data");
			if(!result)
				return wherebox;
			else
				wherebox = plot->getDatabase()->getSQLBoxWhere(b, rep, tableName);
		}
	}
	return wherebox;
}

template<class Portal>
TeVisual getVisual(Portal* portal, TeAppTheme* appTheme, TeGeomRep GEOMREP, int resPos, int groPos, int ownPos)
{
	TeVisual visual;
	TeAbstractTheme* theme = appTheme->getTheme();
	TeDatabase* db = appTheme->getLocalDatabase();
	if(!db)
		return visual;

	TeLegendEntry* legendEntry;

	int status = atoi(portal->getData(resPos));
	int groId = atoi(portal->getData(groPos));
	int ownId = atoi(portal->getData(ownPos));

	if(groId && (theme->visibleRep()&0x40000000))
		legendEntry = db->legendMap()[groId];
	else if(ownId != 0)
		legendEntry = db->legendMap()[ownId];
	else
		legendEntry = &(theme->defaultLegend());
	
//	if(legendEntry)
//		visual = legendEntry->visual(GEOMREP);
//	else
//		visual = TeVisual(GEOMREP);
/////////////////////////////////////////////////////////
	TeGeomRepVisualMap& vm = legendEntry->getVisualMap();
	if(legendEntry && vm.find(GEOMREP) != vm.end())
		visual = *(legendEntry->visual(GEOMREP));
	else
	{
		if(ownId != 0)
			legendEntry = db->legendMap()[ownId];
		else
			legendEntry = &(theme->defaultLegend());

		visual = *(legendEntry->visual(GEOMREP));
	}
/////////////////////////////////////////////////////////

	if(status == 1)
		visual.color((theme->pointingLegend().visual(TePOLYGONS))->color());
	else if(status == 2)
		visual.color((theme->queryLegend().visual(TePOLYGONS))->color());
	else if(status == 3)
		visual.color((theme->queryAndPointingLegend().visual(TePOLYGONS))->color());

	if(status == 0)
	{
		if(GEOMREP == TePOLYGONS || GEOMREP == TeCELLS)
		{
			if(visual.transparency() == 100 && visual.style() != TePolyTypeTransparent)
				visual.style(TePolyTypeTransparent);
		}
	}

	return visual;
}

template<class Portal>
TeVisual getVisualRemote(Portal* /* portal */, TeAppTheme* appTheme, TeGeomRep GEOMREP, string objId)
{
	TeVisual visual;
	if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
		return visual; 
	
	TeExternalTheme* theme = static_cast<TeExternalTheme*>(appTheme->getTheme());
	TeDatabase* db = theme->getSourceDatabase();
	TeLegendEntry* legendEntry;
	
	int status;
	std::map<std::string, int>::iterator itLS = theme->getObjStatusMap().find(objId);
	if( itLS != theme->getObjStatusMap().end())
		status = itLS->second;
	else
		status = 0;
	
	int groId;
	std::map<std::string, int>::iterator itOL = theme->getObjLegendMap().find(objId);
	if( itOL != theme->getObjLegendMap().end())
		groId = itOL->second;
	else
		groId = 0;
		
	
	int ownId;
	std::map<std::string, int>::iterator itOLO = theme->getObjOwnLegendMap().find(objId);
	if( itOLO != theme->getObjOwnLegendMap().end())
		ownId= itOLO->second;
	else
		ownId = 0;
		

	if(groId && (theme->visibleRep()&0x40000000))
		legendEntry = db->legendMap()[groId];
	else if(ownId != 0)
		legendEntry = db->legendMap()[ownId];
	else
		legendEntry = &(theme->defaultLegend());
	
//	if(legendEntry)
//		visual = legendEntry->visual(GEOMREP);
//	else
//		visual = TeVisual(GEOMREP);
/////////////////////////////////////////////////////////
	TeGeomRepVisualMap& vm = legendEntry->getVisualMap();
	if(legendEntry && vm.find(GEOMREP) != vm.end())
		visual = *(legendEntry->visual(GEOMREP));
	else
	{
		if(ownId != 0)
			legendEntry = db->legendMap()[ownId];
		else
			legendEntry = &(theme->defaultLegend());

		visual = *(legendEntry->visual(GEOMREP));
	}
/////////////////////////////////////////////////////////

	if(status == 1)
		visual.color((theme->pointingLegend().visual(TePOLYGONS))->color());
	else if(status == 2)
		visual.color((theme->queryLegend().visual(TePOLYGONS))->color());
	else if(status == 3)
		visual.color((theme->queryAndPointingLegend().visual(TePOLYGONS))->color());

	return visual;
}


template<class Canvas , class Progress>
void TePlotObjects (TeAppTheme* appTheme, Canvas *canvas, map<int, RepMap>& layerRepMap, Progress *progress = 0, TeBox box = TeBox())
{
	string tableName, objectId;
	TeColor color;
	TeVisual visual;
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	int nSteps, nLoops;

/*	if(appTheme->getTheme()->getProductId()==TeFILETHEME)
	{
		TePlotFileTheme<Canvas, Progress> (appTheme, canvas, progress, box);
		return;
	}
*/

	if(	appTheme->getTheme()->getProductId() != TeTHEME && 
		appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
	{
		PluginsSignal sigdraw(PluginsSignal::S_DRAW_THEME);
	
		sigdraw.signalData_.store(PluginsSignal::D_QT_CANVAS, canvas);
		sigdraw.signalData_.store(PluginsSignal::D_ABSTRACT_THEME, appTheme->getTheme());

		PluginsEmitter::instance().send(sigdraw);

		return;
	}

	TeTheme* theme = appTheme->getSourceTheme();
	if (theme == 0)
		return;
	TeLayer* layer = theme->layer();
	if(appTheme->getTheme()->minScale() != 0 && appTheme->getTheme()->maxScale() != 0 && appTheme->getTheme()->minScale() > appTheme->getTheme()->maxScale())
	{
		if(appTheme->getTheme()->minScale() < canvas->scaleApx() || appTheme->getTheme()->maxScale() > canvas->scaleApx())
			return;
	}

	TeDatabase *db = layer->database();
	TeDatabasePortal *portal = db->getPortal();
	canvas->setDataProjection (layer->projection());
	map<int, RepMap>::iterator it = layerRepMap.find(layer->id());

	TeBox vb = box;
	if(box.isValid())
	{
		canvas->mapDWtoCW(vb);
		canvas->mapCWtoV(vb);
	}

	if (((appTheme->getTheme()->visibleRep() & TeRASTER) && layer->hasGeometry(TeRASTER)) || 
		((appTheme->getTheme()->visibleRep() & TeRASTERFILE) && layer->hasGeometry(TeRASTERFILE)))
	{
		TeBox bIn;
		if (box.isValid())
		{
			canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
			bIn = vb;	
		}
	
		TeRaster* raster = theme->layer()->raster();
		if (raster && (raster->params().status_ == TeRasterParams::TeReadyToRead || raster->params().status_ == TeRasterParams::TeReadyToWrite))
		{
			if (!theme->rasterVisual())
				theme->createRasterVisual(raster);
			canvas->plotRaster(raster,theme->rasterVisual(),progress);
		}
	}
	if(layer->hasGeometry(TePOLYGONS) && (appTheme->getTheme()->visibleRep() & TePOLYGONS))
	{
		nLoops = 0;
		// query the database table containing the polygons� geometry
		string wherebox = getWhereBox(portal, appTheme, canvas, TePOLYGONS, box);
		tableName = layer->tableName(TePOLYGONS);

		string queryPolygons;
		if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryPolygons  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
		else											queryPolygons  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
		queryPolygons += " FROM " + tableName + ", " + theme->collectionTable();
		queryPolygons += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryPolygons += " AND " + wherebox;

		queryPolygons += " ORDER BY " + db->getSQLOrderBy(TePOLYGONS);

		//Calculate the number of polygons to be drawn
		string selectPolyCount; 
		if(db->dbmsName()=="SqlServerAdo")
			selectPolyCount="SELECT COUNT(DISTINCT(parent_id)) FROM ";
		else if(db->dbmsName() != "OracleSpatial" && db->dbmsName() != "PostGIS" && db->dbmsName() !="SqlServerAdoSpatial")
			selectPolyCount = "SELECT DISTINCT parent_id FROM ";
		else
			selectPolyCount = "SELECT DISTINCT object_id FROM ";

		selectPolyCount += tableName + ", " + theme->collectionTable() + " WHERE c_object_id = object_id";
		if(wherebox.empty() == false)
			selectPolyCount += " AND " + wherebox;

		string polyCount;
		if(db->dbmsName() =="SqlServerAdo" || db->dbmsName() == "SqlServerAdoSpatial")
			polyCount=selectPolyCount;
		else if(db->dbmsName() == "PostgreSQL" || db->dbmsName() == "PostGIS")
			polyCount = "SELECT COUNT(*) FROM (" + selectPolyCount + ") AS r";		
		else if(db->dbmsName() == "MySQL")
		{
			string createTempTable = "CREATE TEMPORARY TABLE count_table " + selectPolyCount;
			db->execute(createTempTable);
			polyCount = "SELECT COUNT(*) FROM count_table";
		}
		else 
			polyCount = "SELECT COUNT(*) FROM (" + selectPolyCount + ")";
		
		if (portal->query(polyCount))
		{
			if (portal->fetchRow())
				nSteps = atoi(portal->getData(0));
		}
		else
		{
			delete portal;
			return;
		}

		if (db->dbmsName() == "MySQL")
			db->execute("DROP TABLE count_table");

		portal->freeResult();

		//Plot the polygons whose number was placed in nSteps
		if(db->dbmsName() == "PostgreSQL" || db->dbmsName() == "PostGIS")
		{
			 if(portal->query(queryPolygons, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
			 {
				delete portal;
				return;
			 } 
		}
		else
		{
			if (portal->query(queryPolygons) == false)
			{
				delete portal;
				return;
			}
		}

		int	groPos = portal->getAttributeList().size() - 3;
		int	ownPos = portal->getAttributeList().size() - 2;
		int	resPos = portal->getAttributeList().size() - 1;
		
		if(progress != NULL)
		{
			progress->setTotalSteps(nSteps);
		}
		t2 = clock();
		t0 = t1 = t2;
		if(portal->fetchRow())
		{
			bool flag;
			flag = true;
			do
			{
				int geomId = atoi(portal->getData(0));					
				TePolygon poly;
				if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
					visual = getVisual(portal, appTheme, TePOLYGONS, resPos, groPos, ownPos);					
				else
				{
					string objId = portal->getData(1);
					visual = getVisualRemote(portal, appTheme, TePOLYGONS, objId);					
				}
	
				color = visual.color();
				canvas->setPolygonColor(color.red_, color.green_, color.blue_);
				canvas->setPolygonStyle((int)visual.style(),visual.transparency());
				color = visual.contourColor();
				canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
				canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
				if(box.isValid())
					canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
				if(it == layerRepMap.end())
				{
					flag = portal->fetchGeometry(poly);
					canvas->plotPolygon(poly);
				}
				else
				{
					canvas->plotPolygon(layerRepMap[layer->id()].polygonMap_[geomId]);
					int parent_id = atoi(portal->getData(4));
					int parent = parent_id;
					while(parent == parent_id)
					{
						flag = portal->fetchRow();
						if(flag)
							parent = atoi(portal->getData(4));
						else
							parent = -1;
					}
				}
				t2 = clock();
				nLoops++;
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
					if(progress != NULL)
					{
						if(progress->wasCancelled())
						{
							progress->reset();
							break;
						}
						if((int)(t2-t0) > dt2)
							progress->setProgress(nLoops);
					}
				}
			}
			while (flag);
		}
		if(progress != NULL)
		{
			progress->reset();
		}
	}

	if(box.isValid())
		canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
	if(layer->hasGeometry(TeCELLS) && (appTheme->getTheme()->visibleRep() & TeCELLS))
	{
		nLoops = 0;
		// query the database table containing the polygons� geometry
		string wherebox = getWhereBox(portal, appTheme, canvas, TeCELLS, box);
		tableName = layer->tableName(TeCELLS);

		string queryCells;
		if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryCells  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
		else											queryCells  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
		queryCells += " FROM " + tableName + ", " + theme->collectionTable();
		queryCells += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryCells += " AND " + wherebox;

		string cellsCount = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		cellsCount += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			cellsCount += " AND " + wherebox;
		
		portal->freeResult();
		if (portal->query(cellsCount))
		{
			if (portal->fetchRow())
				nSteps = atoi(portal->getData(0));
		}
		else
		{
			delete portal;
			return;
		}

		portal->freeResult();
		if (portal->query(queryCells, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}
		int	groPos = portal->getAttributeList().size() - 3;
		int	ownPos = portal->getAttributeList().size() - 2;
		int	resPos = portal->getAttributeList().size() - 1;

		if(progress != NULL)
		{
			progress->setTotalSteps(nSteps);
		}
		t2 = clock();
		t0 = t1 = t2;
		if(portal->fetchRow())
		{
			bool flag;
			flag = true;
			do
			{
				TeCell cell;
				
				if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
					visual = getVisual(portal, appTheme, TePOLYGONS, resPos, groPos, ownPos);					
				else
				{
					string objId = portal->getData(1);
					visual = getVisualRemote(portal, appTheme, TePOLYGONS, objId);					
				}
				
				color = visual.color();
				canvas->setPolygonColor(color.red_, color.green_, color.blue_);
				canvas->setPolygonStyle((int)visual.style(),visual.transparency());
				color = visual.contourColor();
				canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
				canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
				flag = portal->fetchGeometry(cell);
				canvas->plotCell(cell);
				t2 = clock();
				nLoops++;
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
					
					if(progress != NULL)
					{
						if(progress->wasCancelled())
						{
							progress->reset();
							break;
						}
						if((int)(t2-t0) > dt2)
							progress->setProgress(nLoops);
					}
				}
			}
			while (flag);
		}
		if(progress != NULL)
		{
			progress->reset();
		}
	}

	if (layer->hasGeometry(TeLINES) && (appTheme->getTheme()->visibleRep() & TeLINES))
	{
		nLoops = 0;
		string wherebox = getWhereBox(portal, appTheme, canvas, TeLINES, box);
		// query the database table containing the lines� geometry
		tableName = layer->tableName(TeLINES);

		string queryLines;
		if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryLines  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
		else											queryLines  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
		queryLines += " FROM " + tableName + ", " + theme->collectionTable();
		queryLines += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryLines += " AND " + wherebox;

		string linesCount = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		linesCount += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			linesCount += " AND " + wherebox;

		portal->freeResult();
		if (portal->query(linesCount))
		{
			if (portal->fetchRow())
				nSteps = atoi(portal->getData(0));
		}
		else
		{
			delete portal;
			return;
		}

		portal->freeResult();
		if (portal->query(queryLines, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}
		int	groPos = portal->getAttributeList().size() - 3;
		int	ownPos = portal->getAttributeList().size() - 2;
		int	resPos = portal->getAttributeList().size() - 1;

		if(progress != NULL)
		{
			progress->setTotalSteps(nSteps);
		}

		t2 = clock();
		t0 = t1 = t2;
		if(portal->fetchRow())
		{
			bool flag = true;
			do
			{
				int geomId = atoi(portal->getData(0));					
				TeLine2D line;
				if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
					visual = getVisual(portal, appTheme, TeLINES, resPos, groPos, ownPos);					
				else
				{
					string objId = portal->getData(1);
					visual = getVisualRemote(portal, appTheme, TeLINES, objId);					
				}				
				color = visual.color();
				canvas->setLineColor(color.red_, color.green_, color.blue_);
				canvas->setLineStyle((int)visual.style(),visual.width());
				if(it == layerRepMap.end())
				{
					flag = portal->fetchGeometry(line);
					canvas->plotLine(line);
				}
				else
				{
					canvas->plotLine(layerRepMap[layer->id()].lineMap_[geomId]);
					flag = portal->fetchRow();
				}
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();

					if(progress != NULL)
					{
						if(progress->wasCancelled())
						{
							progress->reset();
							break;
						}
						if((int)(t2-t0) > dt2)
							progress->setProgress(int(t2));
					}
				}
			}
			while (flag);
		}
		if(progress != NULL)
		{
			progress->reset();
		}
	}

	if (layer->hasGeometry(TePOINTS) && (appTheme->getTheme()->visibleRep() & TePOINTS))
	{
		nLoops = 0;
		string wherebox = getWhereBox(portal, appTheme, canvas, TePOINTS, box);
		// query the database table containing the points� geometry
		tableName = layer->tableName(TePOINTS);

		string queryPoints;
		if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryPoints  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
		else											queryPoints  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
		queryPoints += " FROM " + tableName + ", " + theme->collectionTable();
		queryPoints += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryPoints += " AND " + wherebox;

		string pointsCount = "SELECT COUNT(*) FROM " + tableName + ", " + theme->collectionTable();
		pointsCount += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			pointsCount += " AND " + wherebox;

		portal->freeResult();
		if (portal->query(pointsCount))
		{
			if (portal->fetchRow())
				nSteps = atoi(portal->getData(0));
		}
		else
		{
			delete portal;
			return;
		}

		portal->freeResult();
		if (portal->query(queryPoints, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}
		int	groPos = portal->getAttributeList().size() - 3;
		int	ownPos = portal->getAttributeList().size() - 2;
		int	resPos = portal->getAttributeList().size() - 1;

		if(progress != NULL)
		{
			progress->setTotalSteps(nSteps);
		}
		t2 = clock();
		t0 = t1 = t2;
		if(portal->fetchRow())
		{
			bool flag = true;
			do
			{
				TePoint point;
				if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
					visual = getVisual(portal, appTheme, TePOINTS, resPos, groPos, ownPos);					
				else
				{
					string objId = portal->getData(1);
					visual = getVisualRemote(portal, appTheme, TePOINTS, objId);					
				}					
				color = visual.color();
				canvas->setPointColor(color.red_, color.green_, color.blue_);
				canvas->setPointStyle((int)visual.style(),visual.size());
				flag = portal->fetchGeometry(point);
				canvas->plotPoint(point);
				t2 = clock();
				if (int(t2-t1) > dt)
				{
					t1 = t2;
					canvas->copyPixmapToWindow();
					if(progress != NULL)
					{
						if(progress->wasCancelled())
						{
							progress->reset();
							break;
						}
						if((int)(t2-t0) > dt2)
							progress->setProgress(int(t2));
					}
				}
			}
			while (flag);
		}
		if(progress != NULL)
		{
			progress->reset();
		}
	}
	delete portal;

	if(box.isValid())
		canvas->setClipping(false);
}



template<class Canvas , class Progress>
TeBox TePlotSelectedObjects(TeAppTheme* appTheme, Canvas *canvas, set<string>& drawSet, map<int, RepMap>& layerRepMap, Progress * /* progress */ = 0, TeGeomRepVisualMap* visualMap = 0)
{
	TeBox	boxRet;
	int i, nLoops;
	int dt = CLOCKS_PER_SEC/4;
	clock_t	t0, t1, t2;
	vector<string> inClauseVector;

	if(	appTheme->getTheme()->getProductId() != TeTHEME && 
		appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
		return boxRet;

	TeTheme* theme = appTheme->getSourceTheme();
	if (theme == 0)
		return boxRet;
	TeLayer* layer = theme->layer();

	if(theme->minScale() != 0 && theme->maxScale() != 0 && theme->minScale() > theme->maxScale())
	{
		if(theme->minScale() < canvas->scaleApx() || theme->maxScale() > canvas->scaleApx())
			return boxRet;
	}

	TeDatabase *db = layer->database();
	TeDatabasePortal *portal = db->getPortal();
	canvas->setDataProjection (layer->projection());
	string TC = theme->collectionTable();

	string tableName, objectId;
	TeColor color;
	TeVisual	visual;

	set<string>::iterator itB = drawSet.begin();
	set<string>::iterator itE = drawSet.end();
	inClauseVector = generateInClauses(itB,itE, db);

	if (layer->hasGeometry(TePOLYGONS) && (appTheme->getTheme()->visibleRep() & TePOLYGONS))
	{
		map<int, RepMap>::iterator it = layerRepMap.find(layer->id());
//		nLoops = 0;
//		progress->setTotalSteps(drawSet.size());
		t2 = clock();
		t0 = t1 = t2;

		bool hasVisual = false;

		if(visualMap)
			hasVisual = (visualMap->find(TePOLYGONS) != visualMap->end());

		TeVisual* visualPol;

		if(hasVisual)
			visualPol = (*visualMap)[TePOLYGONS];


		for (i = 0; i < (int)inClauseVector.size(); ++i)
		{
			// query the database table containing the polygons� geometry
			tableName = layer->tableName(TePOLYGONS);
			string queryPolygons;
			if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryPolygons  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
			else											queryPolygons  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
			queryPolygons += " FROM " + tableName + ", " + theme->collectionTable();
			queryPolygons += " WHERE object_id = c_object_id AND object_id IN " + inClauseVector[i];
			
			queryPolygons += " ORDER BY " + db->getSQLOrderBy(TePOLYGONS);

			portal->freeResult();
			if (portal->query(queryPolygons, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
			{
				delete portal;
				return boxRet;
			}
			int	groPos = portal->getAttributeList().size() - 3;
			int	ownPos = portal->getAttributeList().size() - 2;
			int	resPos = portal->getAttributeList().size() - 1;

			if(portal->fetchRow())
			{
				bool flag = true;
				do
				{
					TePolygon poly;

					if(hasVisual)
					{
						color = visualPol->color();
						canvas->setPolygonColor(color.red_, color.green_, color.blue_);
						canvas->setPolygonStyle((int)visualPol->style(),visualPol->transparency());
						color = visualPol->contourColor();
						canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
						canvas->setPolygonLineStyle((int)visualPol->contourStyle(),visualPol->contourWidth());
					}
					else
					{
						if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
							visual = getVisual(portal, appTheme, TePOLYGONS, resPos, groPos, ownPos);					
						else
						{
							string objId = portal->getData(1);
							visual = getVisualRemote(portal, appTheme, TePOLYGONS, objId);					
						}		

						color = visual.color();
						canvas->setPolygonColor(color.red_, color.green_, color.blue_);
						canvas->setPolygonStyle((int)visual.style(),visual.transparency());
						color = visual.contourColor();
						canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
						canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
					}

					if(it == layerRepMap.end())
					{
						flag = portal->fetchGeometry(poly);
						boxRet = TeUnion(boxRet, poly.box());
						canvas->plotPolygon(poly, true);
					}
					else
					{
						int geomId = atoi(portal->getData(0));					
						int parent_id = atoi(portal->getData(4));
						int parent = parent_id;
						while(parent == parent_id)
						{
							flag = portal->fetchRow();
							if(flag)
								parent = atoi(portal->getData(4));
							else
								parent = -1;
						}
						boxRet = TeUnion(boxRet, layerRepMap[layer->id()].polygonMap_[geomId].box());
						canvas->plotPolygon(layerRepMap[layer->id()].polygonMap_[geomId], true);
					}
					t2 = clock();
//					nLoops++;
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmap1ToWindow();
//						if(progress->wasCancelled())
//							break;
//						progress->setProgress(nLoops);
					}
				}
				while (flag);
			}
		}
//		progress->reset();
	}

	if (layer->hasGeometry(TeCELLS) && (appTheme->getTheme()->visibleRep() & TeCELLS))
	{
//		nLoops = 0;
//		progress->setTotalSteps(drawSet.size());
		t2 = clock();
		t0 = t1 = t2;

		bool hasVisual = false;

		if(visualMap)
			hasVisual = (visualMap->find(TeCELLS) != visualMap->end());

		TeVisual* visualCell;

		if(hasVisual)
			visualCell = (*visualMap)[TeCELLS];


		for (i = 0; i < (int)inClauseVector.size(); ++i)
		{
			// query the database table containing the lines� geometry
			tableName = layer->tableName(TeCELLS);
			string queryCells;
			if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryCells  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
			else											queryCells  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
			queryCells += " FROM " + tableName + ", " + theme->collectionTable();
			queryCells += " WHERE object_id = c_object_id AND object_id IN " + inClauseVector[i];

			portal->freeResult();
			if (portal->query(queryCells, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
			{
				delete portal;
				return boxRet;
			}

			int	groPos = portal->getAttributeList().size() - 3;
			int	ownPos = portal->getAttributeList().size() - 2;
			int	resPos = portal->getAttributeList().size() - 1;
			
			if(portal->fetchRow())
			{
				bool flag = true;
				do
				{
					TeCell cell;

					if(hasVisual)
					{
						color = visualCell->color();
						canvas->setPolygonColor(color.red_, color.green_, color.blue_);
						canvas->setPolygonStyle((int)visualCell->style(),visualCell->transparency());
						color = visualCell->contourColor();
						canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
						canvas->setPolygonLineStyle((int)visualCell->contourStyle(),visualCell->contourWidth());
					}
					else
					{

						if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
							visual = getVisual(portal, appTheme, TePOLYGONS, resPos, groPos, ownPos);					
						else
						{
							string objId = portal->getData(1);
							visual = getVisualRemote(portal, appTheme, TePOLYGONS, objId);					
						}		
						color = visual.color();
						canvas->setPolygonColor(color.red_, color.green_, color.blue_);
						canvas->setPolygonStyle((int)visual.style(),visual.transparency());
						color = visual.contourColor();
						canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
						canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
					}

					flag = portal->fetchGeometry(cell);
					boxRet = TeUnion(boxRet, cell.box());
					canvas->plotCell(cell, true);
					t2 = clock();
//					nLoops++;
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmap1ToWindow();
//						if(progress->wasCancelled())
//							break;
//						progress->setProgress(nLoops);
					}
				}
				while (flag);
			}
		}
//		progress->reset();
	}

	if (layer->hasGeometry(TeLINES) && (appTheme->getTheme()->visibleRep() & TeLINES))
	{
//		nLoops = 0;
//		progress->setTotalSteps(drawSet.size());
		t2 = clock();
		t0 = t1 = t2;

		bool hasVisual = false;

		if(visualMap)
			hasVisual = (visualMap->find(TeLINES) != visualMap->end());

		TeVisual* visualLin;

		if(hasVisual)
			visualLin = (*visualMap)[TeLINES];


		for (i = 0; i < (int)inClauseVector.size(); ++i)
		{
			// query the database table containing the lines� geometry
			tableName = layer->tableName(TeLINES);
			string queryLines;
			if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryLines  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
			else											queryLines  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
			queryLines += " FROM " + tableName + ", " + theme->collectionTable();
			queryLines += " WHERE object_id = c_object_id AND object_id IN " + inClauseVector[i];

			portal->freeResult();
			if (portal->query(queryLines, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
			{
				delete portal;
				return boxRet;
			}
			int	groPos = portal->getAttributeList().size() - 3;
			int	ownPos = portal->getAttributeList().size() - 2;
			int	resPos = portal->getAttributeList().size() - 1;

			if(portal->fetchRow())
			{
				bool flag = true;
				do
				{
					TeLine2D line;

					if(hasVisual)
					{
						color = visualLin->color();
						canvas->setLineColor(color.red_, color.green_, color.blue_);
						canvas->setLineStyle((int)visualLin->style(),visualLin->width());
					}
					else
					{
						if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
							visual = getVisual(portal, appTheme, TeLINES, resPos, groPos, ownPos);					
						else
						{
							string objId = portal->getData(1);
							visual = getVisualRemote(portal, appTheme, TeLINES, objId);					
						}						
						color = visual.color();
						canvas->setLineColor(color.red_, color.green_, color.blue_);
						canvas->setLineStyle((int)visual.style(),visual.width());
					}

					flag = portal->fetchGeometry(line);
					boxRet = TeUnion(boxRet, line.box());
					canvas->plotLine(line);
					t2 = clock();
//					nLoops++;
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmap1ToWindow();
//						if(progress->wasCancelled())
//							break;
//						progress->setProgress(nLoops);
					}
				}
				while (flag);
			}
		}
//		progress->reset();
	}

	if (layer->hasGeometry(TePOINTS) && (appTheme->getTheme()->visibleRep() & TePOINTS))
	{
		nLoops = 0;
//		progress->setTotalSteps(drawSet.size());
		t2 = clock();
		t0 = t1 = t2;

		bool hasVisual = false;

		if(visualMap)
			hasVisual = (visualMap->find(TePOINTS) != visualMap->end());

		TeVisual* visualPt;

		if(hasVisual)
			visualPt = (*visualMap)[TePOINTS];



		for (i = 0; i < (int)inClauseVector.size(); ++i)
		{
			// query the database table containing the points� geometry
			tableName = layer->tableName(TePOINTS);
			string queryLines;
			if ( db->dbmsName() == "SqlServerAdoSpatial" )	queryLines  = "SELECT geom_id, object_id, spatial_data.STAsBinary() as spatial_data, c_legend_id, c_legend_own, c_object_status";
			else											queryLines  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
			queryLines += " FROM " + tableName + ", " + theme->collectionTable();
			queryLines += " WHERE object_id = c_object_id AND object_id IN " + inClauseVector[i];

			portal->freeResult();
			if (portal->query(queryLines, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
			{
				delete portal;
				return boxRet;
			}
			int	groPos = portal->getAttributeList().size() - 3;
			int	ownPos = portal->getAttributeList().size() - 2;
			int	resPos = portal->getAttributeList().size() - 1;

			if(portal->fetchRow())
			{
				bool flag = true;
				do
				{
					TePoint point;

					if(hasVisual)
					{
						color = visualPt->color();
						canvas->setPointColor(color.red_, color.green_, color.blue_);
						canvas->setPointStyle((int)visualPt->style(),visualPt->size());
					}
					else
					{
						if(appTheme->getTheme()->getProductId() != TeEXTERNALTHEME)
							visual = getVisual(portal, appTheme, TePOINTS, resPos, groPos, ownPos);					
						else
						{
							string objId = portal->getData(1);
							visual = getVisualRemote(portal, appTheme, TePOINTS, objId);					
						}					
						color = visual.color();
						canvas->setPointColor(color.red_, color.green_, color.blue_);
						canvas->setPointStyle((int)visual.style(),visual.size());
					}

					flag = portal->fetchGeometry(point);
					double dw = canvas->mapVtoCW(visual.size());
					TeCoord2D p = point.location();
					TeBox b(p.x()-dw, p.y()-dw, p.x()+dw, p.y()+dw);
					boxRet = TeUnion(boxRet, b);
					canvas->plotPoint(point);
					t2 = clock();
					nLoops++;
					if (int(t2-t1) > dt)
					{
						t1 = t2;
						canvas->copyPixmap1ToWindow();
//						if(progress->wasCancelled())
//							break;
//						progress->setProgress(nLoops);
					}
				}
				while (flag);
			}
		}
//		progress->reset();
	}
	delete portal;
	return boxRet;
}

template<class Canvas>
bool objBoxInCanvas(TeAppTheme* appTheme, Canvas *canvas, set<string>& drawSet, TeGeomRep repType, TeBox& bout, double tol)
{
	//calculate the box that contains all the objects in objectIdSet.
	//If this box is inside the canvas box, return true to indicate that
	//is enough to paint the objects selected. If this box is not inside
	//the canvas box, center this box in the canvas, and return false
	//to indicate that a plot in the whole canvas area is necessary.

	TeTheme* theme = (TeTheme*)appTheme->getTheme();
	TeLayer *layer = theme->layer();
	TeDatabase *db = layer->database();
	TeDatabasePortal *portal = db->getPortal();

	string tableName;
	TeGeomRep rep;
	if (repType & TePOLYGONS)
	{
		tableName = theme->layer()->tableName(TePOLYGONS);
		rep = TePOLYGONS;
	}
	else if (repType & TeLINES)
	{
		tableName = theme->layer()->tableName(TeLINES);
		rep = TeLINES;
	}
	else if (repType & TePOINTS)
	{
		tableName = theme->layer()->tableName(TePOINTS);
		rep = TePOINTS;
	}
	else if (repType & TeCELLS)
	{
        tableName = theme->layer()->tableName(TeCELLS);
		rep = TeCELLS;
	}

	set<string>::iterator it, beginIt;
	int i, chunk = 200, size = drawSet.size();
	string inClause, plic, fields, queryString;

	beginIt = drawSet.begin();
	while (size > 0)
	{
		portal->freeResult();
		inClause = "(";
		plic = "'";
		i = 0;
		for (it = beginIt; it != drawSet.end(); ++it)
		{
			if (i >= chunk)
				break;
			inClause += plic + *it + "',";
			++i;
		}
		inClause[inClause.size() - 1] = ')';
		beginIt = it;
		size -= chunk;

		if((repType & TePOLYGONS) || (repType & TeLINES) || (repType & TeCELLS))
		{
			string where = " object_id IN " + inClause;
			string from = " " + tableName; 

			TeBox ibox;

			db->getMBRSelectedObjects(tableName, "spatial_data", from, where, "", rep, ibox, tol);

			updateBox(bout, ibox);
		}
		else if(repType & TePOINTS)
		{
			string where = " object_id IN " + inClause;
			string from = " " + tableName; 

			TeBox ibox;

			db->getMBRSelectedObjects(tableName, "spatial_data", from, where, "", rep, ibox, tol);

			double x = ibox.x1();
			double y = ibox.y1(); 

			ibox.x1_ = x - tol;
			ibox.y1_ = y - tol;
			ibox.x2_ = x + tol;
			ibox.y2_ = y + tol;

			updateBox(bout, ibox);
		}
	}

	TeBox cbox = canvas->getDataWorld();
	if(TeWithin(bout, cbox) == true)
	{
		delete portal;
		return true;
	}

	//box of the objects selected is not inside the canvas box
	if(bout.width() < cbox.width())
	{
		double w = cbox.width();
		double tw = bout.width();
		double dw = (w - tw) / 2.;
		bout.x1_ = bout.x1_ - dw;
		bout.x2_ = bout.x2_ + dw;
	}

	if(bout.height() < cbox.height())
	{
		double h = cbox.height();
		double th = bout.height();
		double dh = (h - th) / 2.;
		bout.y1_ = bout.y1_ - dh;
		bout.y2_ = bout.y2_ + dh;
	}

	delete portal;

	// layer to canvas projection
	bout = TeRemapBox(bout, theme->layer()->projection(), canvas->projection());
	return false;
}


template<class Canvas>
void TePlotFrame(TeAppTheme* appTheme, Canvas* canvas, map<int, RepMap>& layerRepMap, map<string, TeGeomRepVisualMap*>& objVisualMap)
{
	string tableName, objectId;
	TeColor color;
	TeVisual visual;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();

	if(theme->minScale() != 0 && theme->maxScale() != 0 && theme->minScale() > theme->maxScale())
	{
		if(theme->minScale() < canvas->scaleApx() || theme->maxScale() > canvas->scaleApx())
			return;
	}

	TeLayer *layer = theme->layer();
	TeDatabase *db = layer->database();
	TeDatabasePortal *portal = db->getPortal();
	canvas->setDataProjection (layer->projection());
	map<int, RepMap>::iterator it = layerRepMap.find(layer->id());
	TeGeomRepVisualMap* vmap;

	TeBox box;
	TeBox vb = box;
	if(box.isValid())
	{
		canvas->mapDWtoCW(vb);
		canvas->mapCWtoV(vb);
	}

	if (layer->hasGeometry(TeRASTER) || layer->hasGeometry(TeRASTERFILE) && (theme->visibleRep() & TeRASTER))
	{
		TeBox bIn;
		if (box.isValid())
		{
			canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
			bIn = vb;	
		}
	
		TeRaster* raster = theme->layer()->raster();
		if (!theme->rasterVisual())
			theme->createRasterVisual(raster);
	
		canvas->plotRaster(raster,theme->rasterVisual(), 0);
	}	
	if(layer->hasGeometry(TePOLYGONS) && (theme->visibleRep() & TePOLYGONS))
	{
		// query the database table containing the polygons� geometry
		string wherebox = getWhereBox(portal, appTheme, canvas, TePOLYGONS, box);
		tableName = layer->tableName(TePOLYGONS);

		string queryPolygons  = "SELECT " + tableName + ".*";
		queryPolygons += " FROM " + tableName + ", " + theme->collectionTable();
		queryPolygons += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryPolygons += " AND " + wherebox;

		queryPolygons += " ORDER BY " + db->getSQLOrderBy(TePOLYGONS);

		portal->freeResult();

		//Plot the polygons whose number was placed in nSteps
		if (portal->query(queryPolygons, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}
		
		if(portal->fetchRow())
		{
			bool flag;
			flag = true;
			do
			{
				TePolygon poly;
				int geomId = atoi(portal->getData(0));
				string oid = portal->getData(1);
				if (objVisualMap.find(oid) != objVisualMap.end())
					vmap = objVisualMap[oid];
				else
				{
					flag = portal->fetchGeometry(poly);
					continue;
				}

				visual = *(vmap->operator[](TePOLYGONS));					
				color = visual.color();
				canvas->setPolygonColor(color.red_, color.green_, color.blue_);
				canvas->setPolygonStyle((int)visual.style(),visual.transparency());
				color = visual.contourColor();
				canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
				canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
				if(box.isValid())
					canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
				if(it == layerRepMap.end())
				{
					flag = portal->fetchGeometry(poly);
					canvas->plotPolygon(poly);
				}
				else
				{
					canvas->plotPolygon(layerRepMap[layer->id()].polygonMap_[geomId]);
					int parent_id = atoi(portal->getData(4));
					int parent = parent_id;
					while(parent == parent_id)
					{
						flag = portal->fetchRow();
						if(flag)
							parent = atoi(portal->getData(4));
						else
							parent = -1;
					}
				}
			}
			while (flag);
		}
	}

	if(box.isValid())
		canvas->setClipRegion((int)vb.x1(), (int)vb.y1(), (int)vb.width(), (int)vb.height());
	if(layer->hasGeometry(TeCELLS) && (theme->visibleRep() & TeCELLS))
	{
		// query the database table containing the polygons� geometry
		string wherebox = getWhereBox(portal, appTheme, canvas, TeCELLS, box);
		tableName = layer->tableName(TeCELLS);

		string queryCells  = "SELECT " + tableName + ".*";
		queryCells += " FROM " + tableName + ", " + theme->collectionTable();
		queryCells += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryCells += " AND " + wherebox;

		portal->freeResult();
		if (portal->query(queryCells, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}

		if(portal->fetchRow())
		{
			bool flag;
			flag = true;
			do
			{
				TeCell cell;
				string oid = portal->getData(1);
				if (objVisualMap.find(oid) != objVisualMap.end())
					vmap = objVisualMap[oid];
				else
				{
					flag = portal->fetchGeometry(cell);
					continue;
				}
				visual = *(vmap->operator[](TePOLYGONS));					
				color = visual.color();
				canvas->setPolygonColor(color.red_, color.green_, color.blue_);
				canvas->setPolygonStyle((int)visual.style(),visual.transparency());
				color = visual.contourColor();
				canvas->setPolygonLineColor (color.red_, color.green_, color.blue_);
				canvas->setPolygonLineStyle((int)visual.contourStyle(),visual.contourWidth());
				flag = portal->fetchGeometry(cell);
				canvas->plotCell(cell);
			}
			while (flag);
		}
	}

	if (layer->hasGeometry(TeLINES) && (theme->visibleRep() & TeLINES))
	{
		string wherebox = getWhereBox(portal, appTheme, canvas, TeLINES, box);
		// query the database table containing the lines� geometry
		tableName = layer->tableName(TeLINES);

		string queryLines  = "SELECT " + tableName + ".*";
		queryLines += " FROM " + tableName + ", " + theme->collectionTable();
		queryLines += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryLines += " AND " + wherebox;

		portal->freeResult();
		if (portal->query(queryLines, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}

		if(portal->fetchRow())
		{
			bool flag = true;
			do
			{
				TeLine2D line;
				int geomId = atoi(portal->getData(0));					
				string oid = portal->getData(1);
				if (objVisualMap.find(oid) != objVisualMap.end())
					vmap = objVisualMap[oid];
				else
				{
					flag = portal->fetchGeometry(line);
					continue;
				}

				visual = *(vmap->operator[](TeLINES));					
				color = visual.color();
				canvas->setLineColor(color.red_, color.green_, color.blue_);
				canvas->setLineStyle((int)visual.style(),visual.width());
				if(it == layerRepMap.end())
				{
					flag = portal->fetchGeometry(line);
					canvas->plotLine(line);
				}
				else
				{
					canvas->plotLine(layerRepMap[layer->id()].lineMap_[geomId]);
					flag = portal->fetchRow();
				}
			}
			while (flag);
		}
	}

	if (layer->hasGeometry(TePOINTS) && (theme->visibleRep() & TePOINTS))
	{
		string wherebox = getWhereBox(portal, appTheme, canvas, TePOINTS, box);
		// query the database table containing the points� geometry
		tableName = layer->tableName(TePOINTS);

		string queryPoints  = "SELECT " + tableName + ".*";
		queryPoints += " FROM " + tableName + ", " + theme->collectionTable();
		queryPoints += " WHERE object_id = c_object_id";
		if(wherebox.empty() == false)
			queryPoints += " AND " + wherebox;

		portal->freeResult();
		if(portal->query(queryPoints, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
		{
			delete portal;
			return;
		}

		if(portal->fetchRow())
		{
			bool flag = true;
			do
			{
				TePoint point;
				string oid = portal->getData(1);
				if (objVisualMap.find(oid) != objVisualMap.end())
					vmap = objVisualMap[oid];
				else
				{
					flag = portal->fetchGeometry(point);
					continue;
				}

				visual = *(vmap->operator[](TePOINTS));					
				color = visual.color();
				canvas->setPointColor(color.red_, color.green_, color.blue_);
				canvas->setPointStyle((int)visual.style(),visual.size());
				flag = portal->fetchGeometry(point);
				canvas->plotPoint(point);
			}
			while (flag);
		}
	}
	delete portal;

	if(box.isValid())
		canvas->setClipping(false);
}

#endif

