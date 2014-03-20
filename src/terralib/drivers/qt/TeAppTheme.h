/************************************************************************************
 TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeAppTheme.h
    \brief This file contains an extension to the definition of a Theme in TerraLib
*/
#ifndef  __TERRALIB_INTERNAL_THEMEAPLICATION_H
#define  __TERRALIB_INTERNAL_THEMEAPLICATION_H

#include "../../kernel/TeAbstractTheme.h"
#include <set>

#include "../../kernel/TeDefines.h"

class TeLayer;


//! A map from representation to geometries
/*
\internal
*/
struct TL_DLL RepMap {
	map<int, TePolygon> polygonMap_;
	map<int, TeLine2D> lineMap_;
};

//! An extension to the definition of a Theme in TerraLib
/*
This class extends the definition of a Theme in TerraLib adding some application
funcionalities such as the insertion of graph charts (pie and bar). This extension
materalizes the needs of TerraView like applications.
*/
class TL_DLL TeAppTheme
{
public:

	//! Constructors
	TeAppTheme(TeAbstractTheme* theme = 0);

	//! Destructor
	~TeAppTheme();

	// Copy constructor
	TeAppTheme(const TeAppTheme& other);

	// Assignment operator
	TeAppTheme& operator =(const TeAppTheme& other);

	//! Returns the theme which this theme application is related
	TeAbstractTheme* getTheme() 
	{ return theme_;}
	
	//! Returns TRUE if objects of the layer that don�t belong to the theme are visible
	bool outOfCollectionVisibility() 
	{ return outOfCollectionVisibility_;}
	//! Set whether objects of the layer that don�t belong to the theme are visible
	void outOfCollectionVisibility(bool v) 
	{ outOfCollectionVisibility_ = v;}

	//! Returns TRUE if objects whithout descriptive attributes are visible
	bool withoutDataConnectionVisibility() 
	{ return withoutDataConnectionVisibility_;}
	//! Set whether objects whithout descriptive attributes are visible
	void withoutDataConnectionVisibility(bool v) 
	{ withoutDataConnectionVisibility_ = v;}

	//! Returns a attribute restriction (where clause) used to refine the selection defined by the theme
	string refineAttributeWhere() { return refineAttributeWhere_; }
	//! Set a attribute restriction (where clause) to refine the selection defined by the theme
	void refineAttributeWhere(const string& s) { refineAttributeWhere_ = s; }

	//! Returns a spatial restriction (where clause) used to refine the selection defined by the theme
	string refineSpatialWhere() { return refineSpatialWhere_; }
	//! Set a spatial restriction (where clause) used to refine the selection defined by the theme
	void refineSpatialWhere(const string& s) { refineSpatialWhere_ = s; }

	//! Returns a temporal restriction (where clause) used to refine the selection defined by the theme
	string refineTemporalWhere() { return refineTemporalWhere_; }
	//! Set a temporal restriction (where clause) used to refine the selection defined by the theme
	void refineTemporalWhere(const string& t) { refineTemporalWhere_ = t; }

	//! Returns the grouping colors
	string groupColor () { return groupColor_ ; }
	//! Set the grouping colors
	void groupColor (const string& c) { groupColor_ = c; }
	
	//! Returns the grouping colors direction
	TeColorDirection groupColorDir() { return groupColorDir_; }
	//! Set the grouping colors direction
	void groupColorDir(TeColorDirection n){ groupColorDir_ = n; }

	//! Returns a code identifying which objects should be grouped
	TeSelectedObjects groupObjects() { return groupObjects_; }
	//! Set a code identifying which objects should be  grouped
	void groupObjects(TeSelectedObjects n) { groupObjects_ = n; }

	//! Returns the diameter in pixels of the pie charts
	double pieDiameter() { return pieDiameter_; };
	//! Set the diameter in pixels of the pie charts
	void pieDiameter(double n) { pieDiameter_ = n; };

	//! Returns the name of the attribute which gives the dimension of the diameter in pixels of the pie charts
	string pieDimAttribute() { return pieAttr_; }
	//! Set the name of the attribute which gives the dimension of the diameter in pixels of the pie charts
	void pieDimAttribute(const string& s) { pieAttr_ = s; }

	//! Returns the minimum diameter in pixels of the pie charts
	double pieMinDiameter() { return pieMinDiameter_; };
	//! Set the minimum diameter in pixels of pie charts
	void pieMinDiameter(double n) { pieMinDiameter_ = n; };

	//! Returns the maximum diameter in pixels of the pie charts
	double pieMaxDiameter() { return pieMaxDiameter_; };
	//! Set the maximum diameter in pixels of pie charts
	void pieMaxDiameter(double n) { pieMaxDiameter_ = n; };

	//! Returns the minimum height in pixels of the bar charts
	double barMinHeight() { return barMinHeight_; };
	//! Set the minimum height in pixels of the bar charts
	void barMinHeight(double n) { barMinHeight_ = n; };

	//! Returns the maximum height in pixels of the bar charts
	double barMaxHeight() { return barMaxHeight_; };
	//! Set the maximum height in pixels of the bar charts
	void barMaxHeight(double n) { barMaxHeight_ = n; };

	//! Returns the bar width in pixels of the bar charts
	double barWidth() { return barWidth_; };
	//! Set the bar width in pixels of the bar charts
	void barWidth(double n) { barWidth_ = n; };

	//! Returns the type of the chart 
	TeChartType	chartType() {return chartType_;}
	//! Set the type of the chart 
	void chartType(TeChartType n) { chartType_ = n; }

	//! Get the chart aggregate function
	string chartFunction() { return chartFunction_; }

	//! Set the chart aggregate function
	void chartFunction(string s) { chartFunction_ = s; }

	//! Returns the objects where the charts should be plotted 
	TeSelectedObjects chartObjects() { return chartObjects_; };
	//! Set the objects where the charts should be plotted 
	void chartObjects(TeSelectedObjects n) { chartObjects_ = n; };

	//! Returns the maximum value used to plot charts
	double chartMaxVal() { return chartMaxVal_; }
	//! Set the maximum value used to plot charts
	void chartMaxVal(double n) { chartMaxVal_ = n; }

	//! Returns the minimum value used to plot charts
	double chartMinVal() { return chartMinVal_; }
	//! Set the minimum value used to plot charts
	void chartMinVal(double n) { chartMinVal_ = n; }

	//! Returns TRUE if charts keep its dimension in zooming operations 
	int	keepDimension() { return keepDimension_; };
	//! Set if charts keep its dimension in zooming operations 
	void keepDimension(int n) { keepDimension_ = n; };

	//! Returns TRUE if the legends of the grouping should be plot in the canvas
	bool showCanvasLegend()
	{	return showCanvasLegend_; }
	//! Set whether the legends of the grouping should be plot in the canvas
	void showCanvasLegend(bool cl)
	{	showCanvasLegend_ = cl; }

	//! Returns the upper left position of the canvas legend
	TeCoord2D canvasLegUpperLeft()
	{	return canvasLegUpperLeft_; }
	//! Set the upper left position of the canvas legend
	void canvasLegUpperLeft(TeCoord2D c)
	{	canvasLegUpperLeft_ = c; }
	//! Returns the canvas legend width
	double canvasLegWidth()
	{	return canvasLegWidth_; }
	//! Set the canvas legend width
	void canvasLegWidth(double w)
	{	canvasLegWidth_ = w; }
	//! Returns the canvas legend height
	double canvasLegHeight()
	{	return canvasLegHeight_; }
	//! Set the canvas legend height
	void canvasLegHeight(double h)
	{	canvasLegHeight_ = h; }
	
	//! Returns the geometrical representation being edited
	TeGeomRep editRep() {return editRep_;}
	//! Set the geometrical representation being edited
	void editRep(TeGeomRep r) {editRep_ = r;}

	//! Get the table index from the list of the theme tables, through a given field
	unsigned int getTableIndexFromField(string fieldName);

	bool mixColor()
	{	return mix_color_; }

	void mixColor(bool mixcolor)
	{	mix_color_ = mixcolor;	}

	bool countObj()
	{	return count_obj_; }

	void countObj(bool countobj)
	{	count_obj_ = countobj;	}

	//! Returns the name of a text table that is visible by this theme (used to label objects)
	string textTable() { return textTable_; }

	//! Set the name of a text table that is visible by this theme (used to label objects)
	void textTable(const string& t) { textTable_ = t; }

	bool locateText	(TeCoord2D &pt, TeText &text, const double& tol = 0.0);

	TeVisual getTextVisual(const TeText &text);

	bool setAlias(string att, string attAlias);

	string getLegendTitleAlias();

	string getPieBarAlias(string atr);

	void removeAlias(string att);

	void removeAllAlias();

	void concatTableName(string &attr);

	//! Returns the number of objects in a theme
	int  numObjects();
  
	//! Returns the bounding box of the theme
	TeBox& boundingBox();  

//	TeBox getPieBarBox(TeQtCanvas* canvas);

	//! Vector of colors used to plot the charts
	vector<TeColor> chartColors_;	
	//! Vector of attributes used to plot the charts
	vector<string>	chartAttributes_;

	string		chartSelected_;
	TeCoord2D	chartPoint_;
	TePoint		tePoint_;

	//! Get the source theme
	TeTheme*		getSourceTheme();

	//! Get local database where metadata is stored
	TeDatabase*		getLocalDatabase();

	//! Get the collection table atored in the local database
	string			getLocalCollectionTable();
	

private:

	TeAbstractTheme* theme_;
	
	bool			outOfCollectionVisibility_; 
	bool			withoutDataConnectionVisibility_; 

	// Refining 
	string			refineAttributeWhere_;
	string			refineSpatialWhere_;
	string			refineTemporalWhere_;

	string				groupColor_;
	TeColorDirection	groupColorDir_;
	TeSelectedObjects	groupObjects_;
		
	double			pieDiameter_;
	string			pieAttr_;
	double			pieMinDiameter_;
	double			pieMaxDiameter_;

	double			barMinHeight_;
	double			barMaxHeight_;
	double			barWidth_;

	double			  chartMaxVal_;
	double			  chartMinVal_;
	TeChartType		  chartType_;
	TeSelectedObjects chartObjects_;
	string			  chartFunction_;
	
	int				keepDimension_;
	
	bool			showCanvasLegend_;		//to show or not the legend in the canvas
	TeCoord2D		canvasLegUpperLeft_;	//legend�s upper left point  
	double			canvasLegWidth_;		//legend width
	double			canvasLegHeight_;		//legend height

	TeGeomRep		editRep_;

	bool			mix_color_;
	bool			count_obj_;

	string			textTable_;
};


#endif

