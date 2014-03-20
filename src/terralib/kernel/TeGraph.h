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
/*! \file TeGraph.h
    \brief This file deals with graphs in TerraLib
*/

#ifndef __TERRALIB_INTERNAL_GRAPH_H
#define __TERRALIB_INTERNAL_GRAPH_H

#include "TeLegendEntry.h"
#define CURVES_MAX   10
#define AXIS_MAX     64
#define TITLE_MAX    64
#define LABEL_MAX    64

#define XY_ASCII   0
#define XY_BINARY  1
#define XYY_ASCII  2
#define XYY_BINARY 3
#define PIE_BINARY 4
#define BAR_BINARY 5
#define ROSE_BINARY 6
#define POINT_BINARY 7

#define Real double
typedef Real* Real_Vector;
typedef struct {
	char title[TITLE_MAX+1];
	char xaxis[AXIS_MAX+1];
	char yaxis[AXIS_MAX+1];
	char curve[CURVES_MAX][LABEL_MAX+1];
} Labels;

class TL_DLL TeGraphPlot
{
protected:
	int label_field;       /* maximum no. of characters in a label field */
	int major_tick;        /* length of major tick mark (in pixels) */
	int minor_tick;        /* length of minor tick mark (in pixels) */
	int x_major;           /* number of x axis divisions */
	int y_major;           /* number of y axis divisions */
	int x_minor;           /* number of x axis subdivisions */
	int y_minor;           /* number of y axis subdivisions */
	Real xminval;          /* minimum value on x axis */
	Real xmaxval;          /* maximum value on x axis */
	Real yminval;          /* minimum value on y axis */
	Real ymaxval;          /* maximum value on y axis */
	int xupleft;           /* x pixel coordinate (upper left) */
	int yupleft;           /* y pixel coordinate (upper left) */
	int width;             /* width of axes (in pixels) */
	int height;            /* height of axes (in pixels) */
	int label_width;       /* width of label character */
	int label_height;      /* height of label character */
	int label_font;        /* font number to use for labels */
	int title_width;       /* width of title characters */
	int title_height;      /* height of title characters */
	int title_font;        /* font number to use for title */
	int xaxis_width;       /* width of x axis characters */
	int xaxis_height;      /* height of x axis characters */
	int xaxis_font;        /* font number to use for x axis */
	int yaxis_width;       /* width of y axis characters */
	int yaxis_height;      /* height of y axis characters */
	int yaxis_font;        /* font number to use for y axis */
	int yaxis_direct;      /* direction of y axis characters */
	int X_pixels;          /* number of pixels in horizontal direction */
	int Y_pixels;          /* number of pixels in vertical direction */
	int Xaxis_y;           /* y pixel coordinate of X axis title */
	int Yaxis_x;           /* x pixel coordinate of Y axis title */
	int Title_y;           /* y pixel coordinate of main title */


	int Plot_curves;       /* number of curves */
	int Plot_symbols;      /* maximum number of symbols on curve */
	int Curve_choice;      /* number of selected curve */
	int Plot_choice;       /* graphics menu item choice */
	int Plot_points[CURVES_MAX];    /* number of points in each curve */
	Real_Vector Plot_x[CURVES_MAX]; /* X data points for each curve */
	Real_Vector Plot_y[CURVES_MAX]; /* Y data points for each curve */
	Labels Plot_label;              /* labels for title, axes, and curves */

	int Scale_curve;           /* number of curve to scale (-1 for all) */
	int Scale_factor;          /* used to scale Y axis for printing */
	int Output_format;         /* output format for xywrite */
	int Input_format[CURVES_MAX];          /* format of file loaded */
	char Data_filename[13];    /* name of last file loaded */
	TeLegendEntryVector* sliceVector;/* vector of TeLegendEntry */
	TeColor GPpcor;			   /*piechart color */
	char *GPpieleg;
	int GPpointwidth, GPpointheight;

// Private members

	void	draw_plot(void);
	void	draw_symbols(int curve);
	void	plot_axes(void);
	void	plot_ticks(void);
	void	plot_grid(void);
	int		plot_labels(void);
	int		plot_title(char *text);
	int		plot_xaxis(char *text);
	int		plot_yaxis(char *text);
	int     point_visible(Real x, Real y);
	void    value_to_pixel(Real x_value, Real y_value, int *x_pixel, int *y_pixel);
	int     plot_curve(Real_Vector x, Real_Vector y, int points, TeColor c);
	void    label_format(Real values[], int nvalues, int field_width, int *digits_left, int *digits_right, int *exp_flag);
	void	plot_horiz(char *text, int y, int width);
	int     plot_point(Real x_value, Real y_value, int pen_status);
	void    find_intersect(Real x_in, Real y_in, Real *x_out, Real *y_out);
	void    draw_symbol(int x, int y, int symbol, int width, int height);
	void    draw_symbol_legend(int x, int y, int symbol, int width, int height);
	void    get_plot_fonts();
	int		get_font_size(int font_number, int *width, int *height);

	virtual int	getWidth () = 0;
	virtual int	getHeight () = 0;
   	virtual void	setLineAttributes(TeColor c, int , int ) = 0;
	virtual void	setFillAreaColor (TeColor c) = 0;
	virtual void	setTextColor (TeColor c) = 0;
	virtual void	fillRectangle (int x, int y, int dx, int dy) = 0;
	virtual void	drawRectangle (int x, int y, int dx, int dy) = 0;
	virtual void	drawLine (int sx, int sy, int dx, int dy) = 0;
	virtual void	drawText (int x, int y, char* str, double angle = 0.) = 0;
	virtual void	fillArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c) = 0;
	virtual void	drawArc (int xc, int yc, int w, int h, double a1, double a2, TeColor c) = 0;
	virtual void	textExtent ( string &str, int &w, int &h, double angle = 0.) = 0;

// Public members

public :
	TeGraphPlot ();
	virtual ~TeGraphPlot ();
	int		loadDirect ( int format, int curv, int points, Labels& labels, Real_Vector x, Real_Vector y);
	void	loadHistogram (TeLegendEntryVector& sv, const string& label);
	void	loadPie (TeLegendEntryVector& sv, const string& label);
	void	plotInit( int , int , int , int );
	void	autoScale( void);
	void	setScale( Real x1, Real x2 , Real y1, Real y2);
	int		kill( int order);
	int		plotSymbol( Real x_value, Real y_value,int symbol, int width, int height, int curve=0);
	void	plotCurves (int with_simbols = 0);
	void	pixelToValue( int x_p, int y_p, Real& x_v, Real& y_v);
	void	reset();
	void    setColorGrid( int , int , int );
	void    setColorText( int , int , int );
	void    setColorLine( int , int , int );
	int		setColorCurve( int ,int , int , int );
	void    setColorSymbol( int , int, int, int=-1, int=7, int=7); // r, g, b, curve, width, height
	void    setSymbolType( int, int=0); // type , curve
	void	drawLegend( void);
	int		setGridSize( int );
	int		setGridSize( int, int, int, int );
	void 	drawMoranText (double x1, double y1, double x2, double y2);

	void	pieLegends ();
};

// Utility functions

TL_DLL void show_err(char const *s);
TL_DLL void ftos(Real f, int digits_left, int digits_right, int exp_flag, char *s);

#endif
