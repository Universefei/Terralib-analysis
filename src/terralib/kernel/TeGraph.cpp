/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2004 INPE and Tecgraf/PUC-Rio.

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

#ifdef WIN32
#pragma warning ( disable: 4786 )
#endif
#include "TeGraph.h"
#include "TeDefines.h"

#if TePLATFORM == TePLATFORMCODE_APPLE
#include <stdlib.h>
#include <errno.h>
#else
#include <math.h>
#include <float.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#endif

int set_font(int font_number);

#define PEN_UP       0     /* pen status up */
#define PEN_DOWN     1     /* pen status down */

#define ERR        -1     /* return value when error occurs */

#define HORIZ_FONT 0
#define VERT_FONT  1
#define FONT_MAX   2

/*** symbol shapes ***/

#define RECTANGLE    0     /* tells PlotSymbol to draw a rectangle */
#define TRIANGLE     1     /* tells PlotSymbol to draw a triangle */
#define DIAMOND      2     /* tells PlotSymbol to draw a diamond */
#define SCROSS       3     /* tells PlotSymbol to draw an upright cross */
#define XCROSS       4     /* tells PlotSymbol to draw an angled cross */

/*** constants ***/

#define CURVES_LIMIT   10     /* maximum number of curves */
#define MAIN_ROW       3     /* row number for main menu */
#define MAIN_COL       2     /* column number for main menu */
#define MAX_ITEMS     13     /* maximum number of items in a menu window */
#define MASK_SIZE     50     /* length of file mask string */
#define ASCII          0     /* Ascii output format */
#define BINARY         1     /* Binary output format */
#define LOAD           0     /* load options */
#define SAVE           1     /* save options */
#define SET            2     /* set options */
#define DIGITS         6     /* number of digits to display for regression
                               coefficients and mean square error */
int  Font_width[FONT_MAX]  = {9,9};
int  Font_height[FONT_MAX] = {15,15};
int  Font_direct[FONT_MAX] = {HORIZ_FONT,VERT_FONT};


char Symbol[CURVES_MAX] = {        /* symbols identifying each curve */
		RECTANGLE,  /* symbol for curve 1 is a rectangle */
		SCROSS,     /* symbol for curve 4 is an upright cross */
		TRIANGLE,   /* symbol for curve 2 is a triangle */
		DIAMOND,    /* symbol for curve 3 is a diamond */
		XCROSS,      /* symbol for curve 5 is an angled cross */
		RECTANGLE,  /* symbol for curve 6 is a rectangle */
		SCROSS,     /* symbol for curve 9 is an upright cross */
		TRIANGLE,   /* symbol for curve 7 is a triangle */
		DIAMOND,    /* symbol for curve 8 is a diamond */
		XCROSS      /* symbol for curve 10 is an angled cross */
};

unsigned Terms[] =
{                                  /* terms in each regression function */
        0,                         /* terms for polynomial set by regres_func */
        2,                         /* number of exponential terms */
        2,                         /* number of logarithmic terms */
        3                          /* number of sinusoidal terms */
};


struct Opt{                        /* Options default settings */
	char symbols[5];
	char x_major[3];
	char x_minor[3];
	char y_major[3];
	char y_minor[3];
	char y_scale[2];
	char filemask[MASK_SIZE];
	char format[7];
	char digits[3];
	char linesize[4];
	char adapter[5];
	char printer[9];
	char printer_port[5];
	char print_file[13];
	char color[4];
	char snow[4];
	char sound[4];
	char grid[4];
	char legend[4];
} Options ;       /* curve legend turned on                        */

struct Txtc{
	unsigned char background;   /* color of outermost window */
 	unsigned char pattern;      /* color of pattern in outermost window */
	unsigned char input;        /* color of input window */
	unsigned char input_prompt; /* color of input window prompt */
	unsigned char input_answer; /* color of input window answer */	
	unsigned char edit;         /* color of edit window */
	unsigned char edit_bar;     /* color of selection bar in edit window */
	unsigned char edit_answer;  /* color of answer bar in edit window */
	unsigned char edit_index;   /* color of index values in edit window */
	unsigned char edit_heading; /* color of X/Y values heading in edit window */
	unsigned char error;        /* color of error window */
	unsigned char help;         /* color of help message text */
} Text_color;

struct Grc{
    TeColor grid;                /* color of plot grid         */
    TeColor text;                /* color of plot text         */
    TeColor lines;               /* color of plot all lines    */
	TeColor symbol[CURVES_LIMIT];/* color of plot all lines    */
    TeColor curve[CURVES_LIMIT]; /* color of plot curves       */
} Graph_color;

int f_spoly(Real x, Real_Vector terms, unsigned n);
int f_sexp(Real x, Real_Vector terms, unsigned n);
int f_slog(Real x, Real_Vector terms, unsigned n);
int f_ssin(Real x, Real_Vector terms, unsigned n);

int (*Function[])(Real, Real_Vector, unsigned) =
{                                  /* array of functions used for regression  */
        f_spoly,                     /* calculate terms of polynomial equation  */
        f_sexp,                      /* calculate terms of exponential equation */
        f_slog,                      /* calculate terms of logarithmic equation */
        f_ssin                       /* calculate terms of sinusoidal equation  */
};

Real vmaxval(Real_Vector v, unsigned int n, int *imx)
{
 	// subroutine determines the maximum  value (maxx) of the
  	// vector v[] of length n and returns the index (imx) where
  	// the maximum occurs; i.e. imx, where maxx = v[imx].
 	// On exit, the maximum is returned as the value of vmaxval().

	unsigned int i;
	Real maxx;
 
	if (v == NULL)
		return (Real) 0.0;

 	//find max value

	*imx=0;
	maxx=v[0];
	for(i=1; i<n ; i++)
	{
		if(v[i] > maxx)
		{
			maxx=v[i];
			*imx=i;
		}
	}
	return maxx;
}

//---------------------------------------------------------------------------------
Real
vminval(Real_Vector v, unsigned int n, int *imn)
{
// subroutine determines the minimum  value (minx) of the
// vector v[] of length n and returns the index (imn) where
// the minimum occurs; i.e. imn, where minx = v[imn].
// On exit, the minimum is returned as the value of vminval().

	unsigned int i;
	Real minx;
 
	if (v == NULL)
		return (Real) 0.0;

	//find min value
	*imn=0;
	minx=v[0];
	for(i=1; i<n ; i++)
	{
		if(v[i] < minx)
		{
			minx=v[i];
			*imn=i;
		}
	}
	return minx;
}

//---------------------------------------------------------------------------------

TeGraphPlot :: TeGraphPlot ()
{
	GPpointwidth = 7;
	GPpointheight = 7;

	label_field = 11;
	major_tick = 5;
	minor_tick = 2;
	x_major = 8;
	y_major = 10;
	x_minor = 8;
	y_minor = 5;
	Scale_curve = -1;
	Plot_curves = 0;
	Plot_symbols = 0;
	Curve_choice = 0;
	Plot_choice = 0;
	int i;
	for( i = 0; i < CURVES_MAX; i++ )
	{
		Plot_points[i] = 0;
		Plot_x[i] = 0;
		Plot_y[i] = 0;
	}

	Graph_color.grid.init(0,100,100);
	Graph_color.text.init(0,0,0);
	Graph_color.lines.init(30,30,30);// preto

	for( i = 0; i < CURVES_LIMIT; i++ )
	{
		Graph_color.symbol[i].init(0,255,0);
		Graph_color.curve[i].init(0,0,0);
	}

	sliceVector = 0;
}

//---------------------------------------------------------------------------------

TeGraphPlot :: ~TeGraphPlot ()
{
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: setColorGrid(int r, int g, int b)
{
	Graph_color.grid.init(r,g,b);
}

//---------------------------------------------------------------------------------
void	
TeGraphPlot :: setColorText(int r, int g, int b)
{
	Graph_color.text.init(r,g,b);
}

//---------------------------------------------------------------------------------
void	
TeGraphPlot :: setColorSymbol(int r, int g, int b, int curve, int w, int h)
{
	if (curve==-1)
		for (int i=0; i<CURVES_LIMIT; i++)
			Graph_color.symbol[i].init(r,g,b);
	else
		Graph_color.symbol[curve].init(r,g,b);

	GPpointwidth = w;
	GPpointheight = h;
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: setColorLine(int r, int g, int b)
{
	Graph_color.lines.init(r,g,b);
}

//---------------------------------------------------------------------------------
int
TeGraphPlot :: setColorCurve(int c,int r, int g, int b)
{
	if( c < 0 || c > CURVES_LIMIT-1 )
	{
		return false;
	}

 	Graph_color.curve[c].init(r,g,b);

	return true;
}

void
TeGraphPlot :: pieLegends ()
{
	if (sliceVector == 0)
		return;
	int	w;
	int	xt, yt, pw, ph;
	
	TeColor	cor;

	cor.init (0, 0, 0);
    setTextColor(cor);

	w = min(width,height);
	pw = 2*label_height;
	ph = label_height;

	xt = xupleft + w + pw;
	yt = yupleft;
	int textext = 0;
	for (unsigned int i = 0; i < sliceVector->size (); i++)
	{
		TeLegendEntry slice = (*sliceVector)[i];
		TeVisual  vis;
		//vis = slice.visual (TePOLYGONS);
		TeColor cor = vis.color();
		
		setFillAreaColor (cor);
		string leg = slice.label ();
		int xw,xh;
		textExtent (leg, xw, xh);
		textext = max (textext, xw);
		fillRectangle (xt, yt-ph, pw, ph);
		drawText ((int)(xt+1.2*pw), yt-2, (char*)leg.c_str ());
		yt += (int)1.5*ph;
		if (yt > (yupleft + height))
		{
			xt += (int)(textext+2.2*pw);
			textext = 0;
			yt = yupleft;
		}
	}
}


//---------------------------------------------------------------------------------
int
TeGraphPlot :: setGridSize(int s)
{
	if( s <= 0 )
	{
		return false;
	}
	x_major = s;
	y_major = s;
	x_minor = s;
	y_minor = s;

	return true;
}

//---------------------------------------------------------------------------------
int
TeGraphPlot :: setGridSize(int xma, int yma, int xmi, int ymi )
{
	if( xma <= 0 || yma <= 0 || xmi <= 0 || ymi <= 0 )
	{
		return false;
	}

	x_major = xma;
	y_major = yma;
	x_minor = xmi;
	y_minor = ymi;

	return true;
}

//---------------------------------------------------------------------------------
int
TeGraphPlot :: loadDirect (int format, int curv, int points, Labels& labels, Real_Vector x, Real_Vector y)
{
	if( curv < 0 || curv > CURVES_LIMIT-1 )
	{
		return false;
	}

	strcpy(Plot_label.title, labels.title);
	strcpy(Plot_label.xaxis, labels.xaxis);
	strcpy(Plot_label.yaxis, labels.yaxis);
	Input_format[curv] = format;

        strcpy(Plot_label.curve[curv], labels.curve[curv]);

	Plot_x[curv] = x;
	Plot_y[curv] = y;
	Plot_points[curv] = points;

	if (Plot_curves < (curv+1) )
	{
		Plot_curves= curv+1;
	}

	return true;
}

void TeGraphPlot::loadHistogram (TeLegendEntryVector& sv, const string& label)
{
// Manage number of curves, only one histogram is allowed per plotting
	Plot_curves = 1;
	int order = 0;
	Input_format[order] = BAR_BINARY;
	sliceVector = &sv;
// Allocate space for data arrays
	unsigned int size = sv.size ();
	if (Plot_x[order])
		delete [] Plot_x[order];
	if (Plot_y[order])
		delete [] Plot_y[order];
	Plot_x[order] = new double [size];
	Plot_y[order] = new double [size];
	Plot_points[order] = size;

// Transfer data from TeLegendEntrys to double array
	int vmax = 0;
	for (unsigned int i=0;i<size;i++)
	{
		TeLegendEntry sl = sv[i];
		Plot_y[order][i] = sl.count();
		Plot_x[order][i] = (double)i+1;
		vmax = max (vmax,sl.count() );
	}

// Fill in labels
	strcpy(Plot_label.title, label.c_str ());
	strcpy(Plot_label.xaxis, "Fatia");
	strcpy(Plot_label.yaxis, "Objetos");
	strcpy(Plot_label.curve[order],"");
	setScale(1., Real(size+1) , 0., Real (vmax));
	setGridSize(size, 8, 1, 1); 
}

void TeGraphPlot::loadPie (TeLegendEntryVector& sv, const string& label)
{
// Manage number of curves, only one pie is allowed per plotting
	Plot_curves = 1;
	int order = 0;
	Input_format[order] = PIE_BINARY;
	height = Y_pixels - yupleft;
	sliceVector = &sv;
// Allocate space for data arrays
	unsigned int size = sv.size ();
	if (Plot_x[order])
		delete [] Plot_x[order];
	if (Plot_y[order])
		delete [] Plot_y[order];
	Plot_x[order] = new double [size];
	Plot_y[order] = new double [size];
	Plot_points[order] = size;

// Transfer data from TeLegendEntrys to double array
	int vmax = 0;
	for (unsigned int i=0;i<size;i++)
	{
		TeLegendEntry sl = sv[i];
		Plot_y[order][i] = sl.count();
		Plot_x[order][i] = (double)i+1;
		vmax = max (vmax,sl.count() );
	}

// Fill in labels
	strcpy(Plot_label.title, label.c_str ());
//	strcpy(Label.curve[order],label.c_str ());
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: autoScale(void)
{
// scale axes for Scale_curve (for all curves if Scale_curve == -1) 

	int i, index;
	Real value;

	if (Scale_curve == -1)
		index = 0;
	else
		index = Scale_curve;

	xminval = vminval(Plot_x[index], Plot_points[index], &i);
	xmaxval = vmaxval(Plot_x[index], Plot_points[index], &i);
	yminval = vminval(Plot_y[index], Plot_points[index], &i);
	ymaxval = vmaxval(Plot_y[index], Plot_points[index], &i);

	if (Scale_curve == -1)
	{
		for (i = 1; i < Plot_curves; i++)
		{
			value = vminval(Plot_x[i], Plot_points[i], &index);
			if (value < xminval) xminval = value;
			value = vmaxval(Plot_x[i], Plot_points[i], &index);
			if (value > xmaxval) xmaxval = value;
			value = vminval(Plot_y[i], Plot_points[i], &index);
			if (value < yminval) yminval = value;
			value = vmaxval(Plot_y[i], Plot_points[i], &index);
			if (value > ymaxval) ymaxval = value;
		}
	}

	for (int curva=0; curva < Plot_curves; curva++)
	{
		if (Input_format[curva] == ROSE_BINARY)
		{
			yminval = - ymaxval;
			xmaxval = ymaxval;
			xminval = yminval;
		}
	}
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: setScale(Real x1, Real x2 , Real y1, Real y2)
{
   	xminval = x1;
    	xmaxval = x2;
    	yminval = y1;
    	ymaxval = y2;

	for (int curva=0; curva < Plot_curves; curva++)
	{
		if (Input_format[curva] == ROSE_BINARY)
		{
			yminval = - ymaxval;
			xmaxval = ymaxval;
			xminval = yminval;
		}
	}
}

int f_spoly(Real x, Real_Vector terms, unsigned n)
{
    	unsigned i;
    	terms[0] = 1.0;
    	for (i = 1; i < n; i++) terms[i] = x * terms[i-1];
   	return 0;
}

int f_sexp(Real x, Real_Vector terms, unsigned n)
{
    double value;
   	value = exp(x);
    if (value != HUGE_VAL)
	{
        terms[0] = 1.0;
        terms[n-1] = (Real) value;
        return 0;
    }
	else
	{
       show_err("Value of exp(x) is too large");
       return -1;
    }
}

int f_slog(Real x, Real_Vector terms, unsigned n)
{
    	if (x > 0.0) {
        	terms[0] = 1.0;
        	terms[n-1] = (Real) log(x);
        	return 0;
    	}
    	else {
        	show_err("Value of x is less than or equal to 0");
        	return -1;
    	}
}

int f_ssin(Real x, Real_Vector terms, unsigned n)
{
   	errno = 0;
	terms[0] = 1.0;
    terms[1] = (Real) sin(x);
    terms[n-1] = (Real) cos(x);
    if (errno == 0)
		return 0;
    else
	{
        show_err("Loss of significance for cos(x) or sin(x)");
       	return -1;
    }
}

void show_err (char const* /* mess */)
{
//	printf("%s\n",mess);
}

//---------------------------------------------------------------------------------

int
TeGraphPlot :: kill(int order)
{
	int i, count = 0;
	if (Plot_curves == 0)
		return 0;
	for (i = 0; i < Plot_curves; i++)
	{
		if (Plot_x[i] == Plot_x[order]) count++;
	}

	if (count == 1)
	{
		delete [] Plot_x[order];
		Plot_x[order] = 0;
		delete [] Plot_y[order];
		Plot_y[order] = 0;
	}

	for (i = order; i < Plot_curves-1; i++)
	{
		Plot_x[i] = Plot_x[i+1];
		Plot_y[i] = Plot_y[i+1];
		Plot_points[i] = Plot_points[i+1];
		strcpy(Plot_label.curve[i], Plot_label.curve[i+1]);
	}
   	Plot_curves--;
	return 1;
}


//---------------------------------------------------------------------------------
void
TeGraphPlot :: plotCurves ( int with_simbols)
{
	short	drawplot = true;
	Plot_symbols = with_simbols; // numero de pontos
				     // que devem ser plotados
				     // sobre a curva
	for (int curva=0; curva < Plot_curves; curva++)
	{
		if (Plot_y[curva] == NULL)
			continue;

		if (Input_format[curva] == PIE_BINARY && sliceVector)
		{
			plot_title(Plot_label.title);

			int		i, w, h, xc, yc, points;
			
			w = min(width,height);
			h = min(width,height);
			xc = xupleft ;
			yc = yupleft ;
			points =  Plot_points[0];

			Real_Vector y = Plot_y[0];

			Real	count = 0., value;

			for (i = 0; i < points; i++) count += y[i];
			if (count == 0.) break;

			double	a1 = 0.,
					a2 = 0.;

			for (i = 0; i < points; i++)
			{
				TeLegendEntry slice = (*sliceVector)[i];
				TeColor cor = slice.visual(TePOLYGONS)->color ();
				value = y[i]/count;
				a2 += value*360.;
				setFillAreaColor(cor);
				fillArc (xc, yc, w, h, a1, a2, cor);
				if (value > .08)
				{
					char percent[16];
					sprintf (percent,"     %.1f",value*100);
					drawText (xc+w/2, yc+h/2, percent, (a1+a2)/2);
				}
				a1 = a2;
			}
			pieLegends ();
		}
		else if (Input_format[curva] == BAR_BINARY)
		{
		//-------------------------------------
		//           ^
		//	     |	    .....
		//	     |	    :   :....
		//	     |	    :   :   :
		//	     |	....:   :   :    
		//	     |  :   :   :   :
		//           |__:_|_:_|_:_|_:____
		//                1   2   3 
		//------------------------------------- 
			int		x1, y1, x2, y2;
			int i;

//			Real_Vector	x = Plot_x[0],
//					y = Plot_y[0];
			Real_Vector	x = Plot_x[curva],
					y = Plot_y[curva];
 
//   			setLineAttributes(Graph_color.curve[0],0,0);
//			setFillAreaColor(Graph_color.curve[0]);
   			setLineAttributes(Graph_color.curve[curva],TeLnTypeContinuous,0);
			setFillAreaColor(Graph_color.curve[curva]);

			for (i = 0; i < Plot_points[curva]; i++)
			{
				// Juan begin
				if(sliceVector)
				{
					TeLegendEntry slice = (*sliceVector)[i];
					TeColor cor = slice.visual(TePOLYGONS)->color ();
					setFillAreaColor (cor);
				}
				// Juan end
//				if (i < Plot_points[0]-1)
				if (i < Plot_points[curva]-1)
				{
	        			value_to_pixel(x[i], y[i], &x1, &y1);
	        			value_to_pixel(x[i+1], y[i+1], &x2, &y2);
					if ((height + yupleft - y1 - 1) > 0)
	        				fillRectangle(x1-(x2-x1)/2, y1, (x2-x1-2), height + yupleft - y1);
				}
				else
				{
	        			value_to_pixel(x[i-1], y[i-1], &x1, &y1);
	        			value_to_pixel(x[i], y[i], &x2, &y2);
					if ((height + yupleft - y2 - 1) > 0)
	        				fillRectangle(x2-(x2-x1)/2, y2, (x2-x1-2), height + yupleft - y2);
				}
			}
			draw_plot();
		}
		else if (Input_format[curva] == ROSE_BINARY)
		{
			int i;
  			int x1, y1, x2, y2;
			int	a1 = 0,
				a2 = 0;
			Real_Vector	x = Plot_x[0],
					y = Plot_y[0];

			plot_title(Plot_label.title);

			for (i = 0; i < Plot_points[0]-1; i++)
			{
				if (y[i]==0.)continue;
				a1 = (int)(x[i] * 64.);
				a2 = (int)(x[i+1] * 64.) - a1;
        		value_to_pixel(-y[i], y[i], &x1, &y1);
        		value_to_pixel(y[i], -y[i], &x2, &y2);
				fillArc (x1, y1, x2-x1, y2-y1, a1, a2, Graph_color.curve[0]);
			}

			Real step = ymaxval / (Real)major_tick;

			for(Real j = step ; j <= ymaxval; j += step )
			{ 
        			value_to_pixel(-j, j, &x1, &y1);
        			value_to_pixel(j, -j, &x2, &y2);

   				setLineAttributes(Graph_color.lines,TeLnTypeContinuous,0);
				drawArc(x1, y1, x2-x1, y2-y1,0,360*64,Graph_color.lines);
			}
		}
		else if (Input_format[curva] == POINT_BINARY)
		{
			draw_plot();
		
        		for (int j = 0; j < Plot_points[curva]; j ++)
				plotSymbol(Plot_x[curva][j], Plot_y[curva][j], Symbol[curva], GPpointwidth, GPpointheight, curva);

		}
		else
		{
			if (drawplot)
			{
				draw_plot();
				drawplot = false;
			}

			plot_curve(Plot_x[curva], Plot_y[curva], Plot_points[curva],Graph_color.curve[curva]);
		
			if (Plot_symbols)
				draw_symbols(curva);
		}
	}
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: draw_plot(void)
{

	plot_axes();
	plot_ticks();
//	if (Options.grid[1] == 'n')
	plot_grid();
	plot_labels();
	plot_title(Plot_label.title);
	plot_xaxis(Plot_label.xaxis);
	plot_yaxis(Plot_label.yaxis);
//	if (Options.legend[1] == 'n') 
	drawLegend();
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: setSymbolType(int type, int curve)
{
	if(curve>=0 && curve<=9)
		Symbol[curve] = type;
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: draw_symbols(int curve)
{
	int i, step;

	if (Plot_symbols > 0)
	{
		step = Plot_points[curve]/Plot_symbols;
		if (Plot_points[curve] % Plot_symbols != 0) step++;
		for (i = 0; i < Plot_points[curve]; i += step)
			plotSymbol(Plot_x[curve][i], Plot_y[curve][i], Symbol[curve], GPpointwidth, GPpointheight, curve);
	}
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: plot_axes(void)
{
    setLineAttributes(Graph_color.lines,TeLnTypeContinuous,0);
    drawRectangle(xupleft, yupleft, width, height);
}

//---------------------------------------------------------------------------------
void
TeGraphPlot :: plot_ticks(void)
{
    int i, x, y, maximum, delta, length;
    int x_ticks = x_major * x_minor;
    int y_ticks = y_major * y_minor;

    setLineAttributes(Graph_color.lines,TeLnTypeContinuous,0);
	delta = (width-1)/x_ticks;
	x = xupleft + delta;
	y = yupleft + 1;
	maximum = yupleft + height ;
	for (i = 1; i < x_ticks; i++)
	{
		if (i % x_minor == 0)
			length = major_tick;
		else
			length = minor_tick;
		drawLine(x, y, x, y + length );
		drawLine(x, maximum, x, maximum - length );
		x += delta;
	}
	delta = (height-1)/y_ticks;
	x = xupleft + 1;
	y = yupleft + delta;
	maximum = xupleft + width -1;
	for (i = 1; i < y_ticks; i++)
	{
		if (i % y_minor == 0)
			length = major_tick;
		else
			length = minor_tick;
		drawLine(x, y, x + length , y);
		drawLine(maximum, y, maximum - length , y);
		y += delta;
	}
}

void TeGraphPlot :: plot_grid(void)
{
    	int i, x, y, maximum, delta;
    	setLineAttributes(Graph_color.grid,TeLnTypeDotted,0);
    	delta = (width-1)/x_major;
    	x = xupleft + delta;
    	maximum = yupleft + height - major_tick - 1;
    	for (i = 1; i < x_major; i++)
		{
        	y = yupleft + 1 + major_tick;
 //       	while (y < maximum) {
 //           		drawPoint (x,y);
 //           		y += 3;
 //       	}
			drawLine(x,y,x,maximum);
        	x += delta;
    	}
    	delta = (height-1)/y_major;
    	y = yupleft + delta;
    	maximum = xupleft + width - major_tick - 1;
    	for (i = 1; i < y_major; i++)
		{
        	x = xupleft + 1 + major_tick;
 //       	while (x < maximum) {
 //           		drawPoint (x,y);
 //           		x += 4;
 //         }
			drawLine(x,y,maximum,y);
        	y += delta;
    	}
}

int TeGraphPlot :: plot_labels(void)
{
    char text[80];
    int i, x, y, delta, font;
    int digits_left, digits_right, exp_flag;
    Real value, values[3];
	setTextColor(Graph_color.text);

	if ((font = set_font(label_font)) == ERR) return ERR;
	values[0] = yminval;
	values[1] = ymaxval;
	values[2] = (values[1] - values[0]) / y_major;
	label_format(values, 3, label_field, &digits_left, &digits_right, &exp_flag);
	delta = (height-1) / y_major;
	y = yupleft - label_height/2;
	value = ymaxval;
	Yaxis_x = xupleft;
	for (i = 0; i <= y_major; i++)
	{
		ftos(value, digits_left, digits_right, exp_flag, text);
		x = xupleft - strlen(text) * label_width - label_width/2;
		if (x < Yaxis_x)
			Yaxis_x = x;
		drawText (x, y+8, text);
		y += delta;
		value -= values[2];
	}
	if (yaxis_direct == HORIZ_FONT)
	{
		Yaxis_x -= (yaxis_width + label_width/2);
		if (Yaxis_x < 0)
			Yaxis_x = 0;
	}
	else
	{
		Yaxis_x -= label_width/2;
		if (Yaxis_x < yaxis_height)
			Yaxis_x = yaxis_height;
	}
	values[0] = xminval;
	values[1] = xmaxval;
	values[2] = (values[1] - values[0]) / x_major;
	label_format(values, 3, label_field, &digits_left, &digits_right, &exp_flag);
	delta = (width-1) / x_major;
	y = yupleft + height + label_height/2;
	value = xminval;
	for (i = 0; i <= x_major; i++)
	{
		ftos(value, digits_left, digits_right, exp_flag, text);
		x = xupleft + i*delta - strlen(text) * label_width / 2;
		drawText (x, y+8, text);
		value += values[2];
		if (x_major > 5)
		{
			i++;
			value += values[2];
          	// if (x_major > 9) {
           	//     	i++;
            	//    	value += values[2];
           	//}
		}
	}
//    	set_font(font);
	return 0;
}

int TeGraphPlot :: plot_title(char *text)
{
    	int font;
    	if ((font = set_font(title_font)) == ERR) return ERR;
    	plot_horiz(text, Title_y, title_width);
    	set_font(font);
    	return 0;
}

int TeGraphPlot :: plot_xaxis(char *text)
{
    	int font;
    	if ((font = set_font(xaxis_font)) == ERR) return ERR;
    	plot_horiz(text, Xaxis_y, xaxis_width);
    	set_font(font);
	return 0;
}

void
TeGraphPlot :: plot_horiz(char *text, int y, int w)
{
    	int x, l, count;
    	char *ptr = NULL,buffer[80];
    	setTextColor(Graph_color.text);
    	l = strlen(text);
    	x = xupleft + (width-1)/2 - (l * w)/2;
    	ptr = text;
    	if (x < xupleft) {
        	count = (xupleft - x)/w;
        	buffer[79] = '\0';
        	strncpy(buffer, text, sizeof(buffer)-1);
        	buffer[l-count-1] = '\0';
        	x = xupleft;
        	ptr = buffer;
    	}
    	drawText (x, y, ptr);
    	ptr = NULL;
}

int
TeGraphPlot :: plot_yaxis(char *text)
{
    int y, length, count, font, delta;
	char *ptr= NULL, buffer[80], c[2];
    setTextColor(Graph_color.text);
	c[0] = '*'; c[1] = '\0';
	if ((font = set_font(yaxis_font)) == ERR) return ERR;
	length = strlen(text);
	ptr = text;
	delta = yaxis_direct == HORIZ_FONT ?
            yaxis_height : yaxis_width;
	y = (Title_y + Xaxis_y + delta * length) / 2;
	buffer[79] = '\0';
	strncpy(buffer, text, sizeof(buffer)-1);
	if (y < Title_y)
	{
		count = 2 * (Title_y - y)/delta;
		buffer[length-count-1] = '\0';
		y = Title_y;
		ptr = buffer;
	}
	if (yaxis_direct == HORIZ_FONT)
	{
		while (*ptr != '\0')
		{
			c[0] = *ptr++;
			drawText (Yaxis_x, y, c);
			y += yaxis_height;
		}
	}
	else
		drawText (Yaxis_x, y, ptr, 90.);
	ptr = NULL;

	set_font(font);
	return 0;
}

int
TeGraphPlot :: plot_curve(Real_Vector x, Real_Vector y,int points,TeColor c)
{
    	int i, count;

	setLineAttributes(c,TeLnTypeContinuous,0);

    	count = plot_point(x[0], y[0], PEN_UP);
    	for (i = 1; i < points; i++)
        	count += plot_point(x[i], y[i], PEN_DOWN);
    	return count;
}

int
TeGraphPlot :: plot_point(Real x_value, Real y_value, int pen_status)
{
    static Real last_x_value, last_y_value;
    static int last_x, last_y;
    Real temp_x, temp_y;
    int x, y, status=0;

    if (point_visible(x_value, y_value))
	{
        value_to_pixel(x_value, y_value, &x, &y);
        if (pen_status == PEN_UP)
		{
//				SET_PIXEL(x, y, color);
		}
        else
		{
            if (!point_visible(last_x_value, last_y_value))
			{
				temp_x = last_x_value;
				temp_y = last_y_value;
                find_intersect(x_value, y_value, &temp_x, &temp_y);
                value_to_pixel(temp_x, temp_y, &last_x,	&last_y);
            }
			if (last_y_value < TeMAXFLOAT)
            	drawLine(last_x, last_y, x, y);
        }
        last_x = x;
        last_y = y;
        status = 1;
    }
    else if (pen_status == PEN_DOWN && point_visible(last_x_value, last_y_value))
	{
		if (y_value < TeMAXFLOAT)
		{
			temp_x = x_value;
			temp_y = y_value;
			find_intersect(last_x_value, last_y_value, &temp_x, &temp_y);
			value_to_pixel(temp_x, temp_y, &x, &y);
			drawLine(last_x, last_y, x, y);
		}
        status = 0;
    }
    last_x_value = x_value;
   	last_y_value = y_value;
    return status;
}

int
TeGraphPlot :: plotSymbol(Real x_value, Real y_value, int symbol, int w, int h, int curve)
{
    int x, y;

	setLineAttributes(Graph_color.symbol[curve],TeLnTypeContinuous,1);

	if (point_visible(x_value, y_value))
	{
        	value_to_pixel(x_value, y_value, &x, &y);
        	draw_symbol(x, y, symbol, w, h);
        	return true;
    }
    return false;
}

void
TeGraphPlot :: draw_symbol(int x, int y,int symbol, int w, int h)
{
	if( symbol < 0 )		//V2.0-01 GAIA
	{
		symbol = 0;
	}
	symbol = symbol % 5;		//V2.0-01 GAIA

    	switch (symbol)
	{
        case RECTANGLE:
            	drawLine(x - w/2, y - h/2, x + w/2, y - h/2);
		drawLine(x + w/2, y - h/2, x + w/2, y + h/2);
		drawLine(x + w/2, y + h/2, x - w/2, y + h/2);
            	drawLine(x - w/2, y + h/2, x - w/2, y - h/2);
		break;

        case TRIANGLE:
		drawLine(x - w/2, y + h/2, x, y - h/2);
		drawLine(x, y - h/2, x + w/2, y + h/2);
            	drawLine(x + w/2, y + h/2, x - w/2, y + h/2);
            	break;

        case DIAMOND:
            	drawLine(x - w/2, y, x, y - h/2);
            	drawLine(x, y - h/2, x + w/2, y);
            	drawLine(x + w/2, y, x, y + h/2);
            	drawLine(x, y + h/2, x - w/2, y);
            	break;

        case SCROSS:
            	drawLine(x - w/2, y, x + w/2, y);
		drawLine(x, y - h/2, x, y + h/2);
		break;

        case XCROSS:
		drawLine(x - w/2, y + h/2, x + w/2, y - h/2);
		drawLine(x - w/2, y - h/2, x + w/2, y + h/2);
		break;
    	}
}


void TeGraphPlot :: draw_symbol_legend(int x, int y, int symbol, int w, int h)
{
	if( symbol < 0 )		//V2.0-01 GAIA
	{
		symbol = 0;
	}
	symbol = symbol % 5;		//V2.0-01 GAIA

    	switch (symbol)
	{
        case RECTANGLE:
		drawLine(x - w/2, y - h/2, x + w/2, y - h/2);
		drawLine(x + w/2, y - h/2, x + w/2, y + h/2);
		drawLine(x + w/2, y + h/2, x - w/2, y + h/2);
		drawLine(x - w/2, y + h/2, x - w/2, y - h/2);
		break;

        case TRIANGLE:
		drawLine(x - w/2, y + h/2, x, y - h/2);
		drawLine(x, y - h/2, x + w/2, y + h/2);
		drawLine(x + w/2, y + h/2, x - w/2, y + h/2);
		break;

        case DIAMOND:
		drawLine(x - w/2, y, x, y - h/2);
		drawLine(x, y - h/2, x + w/2, y);
		drawLine(x + w/2, y, x, y + h/2);
		drawLine(x, y + h/2, x - w/2, y);
		break;

        case SCROSS:
		drawLine(x - w/2, y, x + w/2, y);
		drawLine(x, y - h/2, x, y + h/2);
		break;

        case XCROSS:
		drawLine(x - w/2, y + h/2, x + w/2, y - h/2);
		drawLine(x - w/2, y - h/2, x + w/2, y + h/2);
		break;
    }
}


void
TeGraphPlot :: label_format(Real values[], int nvalues, int field_width,
                   int *digits_left, int *digits_right, int *exp_flag)
{
	char *str = NULL;
	int i, j, digits, decimal, sign, trail0;

	digits = sizeof(Real) == sizeof(float) ? FLT_DIG : DBL_DIG;
	*digits_left = *digits_right = *exp_flag = 0;
    	for (i = 0; i < nvalues && *exp_flag == 0; i++) {
		str = ecvt(values[i], digits, &decimal, &sign);
        	if (decimal > *digits_left) *digits_left = decimal;
        	for (j = digits-1, trail0 = 0; j >= 0 && str[j] == '0'; j--) trail0++;
        	j = digits - trail0 - decimal;
        	if (j > *digits_right) *digits_right = j;
        	if (*digits_right + *digits_left > field_width-2) {
            		*exp_flag = 1;
            		*digits_left = 1;
            		*digits_right = field_width - 8;
            		if (*digits_right < 0) *digits_right = 0;
        	}
    	}
   	 str = NULL;
}

void TeGraphPlot :: find_intersect(Real x_in, Real y_in,
                    Real *x_out, Real *y_out)
{
    	// find values for x_out and y_out that intersect the
       	// plot boundary on a line between the points (x_in, y_in) inside
       	// the plot region and (x_out, y_out) outside the plot region

    	Real m, b, temp_x, temp_y;
    	m = (y_in - *y_out) / (x_in - *x_out);
    	b = y_in - m * x_in;
    	if (*x_out >= xminval && *x_out <= xmaxval) {
        	// only y_out is outside plot region
        	if (*y_out < yminval) *y_out = yminval;
        	else if (*y_out > ymaxval) *y_out = ymaxval;
        	*x_out = (*y_out - b) / m;
    	}
    	else
	{ 
		if (*y_out >= yminval && *y_out <= ymaxval) {
        		// only x_out is outside plot region
        		if (*x_out < xminval) *x_out = xminval;
        		else if (*x_out > xmaxval) *x_out = xmaxval;
        		*y_out = *x_out * m + b;
    		}
		else
		{
        		// both x_out and y_out are outside plot region
        		if (*y_out < yminval) temp_y = yminval;
        		else temp_y = ymaxval;
        		temp_x = (temp_y - b) / m;
        		if (temp_x >= xminval && temp_x <= xmaxval) {
            			*y_out = temp_y;
            			*x_out = temp_x;
        		}
        		else {
            			if (*x_out < xminval) *x_out = xminval;
            			else *x_out = xmaxval;
            			*y_out = *x_out * m + b;
			}
		}
	}
}

//----------------------------------------------------------------------------------------
void
TeGraphPlot :: drawLegend(void)
{
    int x, y, h, delta;
    int i, count = 0;

	for (i = 0; i < Plot_curves; i++)
	{
        if (Plot_label.curve[i][0] != '\0') count++;
	}

    if (count != 0)
	{
       	h = Font_height[0] + 4;
		if (count > 3) h += Font_height[0] + 3;
        y = getHeight() - h;

		setLineAttributes(Graph_color.lines,TeLnTypeContinuous,0);
		drawRectangle(0, y, width - 1, h);

        x = Font_width[0] + 1;
		delta = Font_height[0] + 3;
        for (count = 0, i = 0; i < Plot_curves; i++)
	{
            if (Plot_label.curve[i][0] != '\0' && count < 7)
			{
                if (count++ == 3) {
                    x = Font_width[0] + 1;
                    y += Font_height[0] + 6;
                }

				setLineAttributes(Graph_color.curve[i],TeLnTypeContinuous,0);
				draw_symbol_legend(x, y + delta - Font_height[0]/2, Symbol[i], 7, 7);
                x += Font_width[0];

				setTextColor(Graph_color.text);
                drawText (x, y + delta - Font_height[0], Plot_label.curve[i]);
				x += 10 * Font_width[0];
            }
        }
	}
}

//----------------------------------------------------------------------------------------
void
TeGraphPlot :: value_to_pixel(Real x_value, Real y_value, int *x_pixel, int *y_pixel)
{
	*x_pixel = (int) ((x_value - xminval) /
               (xmaxval - xminval) * (width - 1) + xupleft);
	*y_pixel = (int) ((ymaxval - y_value) /
               (ymaxval - yminval) * (height - 1) + yupleft);
}

//----------------------------------------------------------------------------------------
void
TeGraphPlot :: pixelToValue(int x_pixel, int y_pixel, Real& x_value, Real& y_value)
{
	x_value = (Real) (x_pixel - xupleft) / (width-1) *
               (xmaxval - xminval) + xminval;
	y_value = (Real) (yupleft - y_pixel) / (height-1) *
               (ymaxval - yminval) + ymaxval;
}

//----------------------------------------------------------------------------------------
int
TeGraphPlot :: point_visible(Real x, Real y)
{
    	if (x >= xminval && x <= xmaxval &&
        y >= yminval && y <= ymaxval) return 1;
    	else return 0;
}

//FAMI0395 int set_font(int font_number)
int set_font(int)
{
/*
    	static int font;
    	int previous_font;
	// set font to specified font number
	if (_set_font(font_number) == 0) {
        	previous_font = font;
        	font = font_number;
        	return previous_font;
    	}
    	else {
        	video_err = FONT_ERR;
        	return ERR;
    	} */

	return (int)true;	//FAMI0394
}

void ftos(Real f, int digits_left, int digits_right, int exp_flag, char *s)
{
	int i = 0, decimal, sign;
	char *str = NULL;
	str = ecvt(f, digits_left+digits_right, &decimal, &sign);
	if (sign) *s++ = '-';
	if (exp_flag)
	{ 	/* exponential format */
		for (i=0; i < digits_left; i++) *s++ = *str++;
		*s++ = '.';
		for (i=0; i < digits_right; i++) *s++ = *str++;
		*s++ = 'e';
		decimal -= digits_left;
		if (decimal < 0)
		{
			*s++ = '-';
			decimal = -decimal;
		}
		else *s++ = '+';
		s += 3;
		*s-- = '\0';
		for (i = 0; i < 3; i++)
		{
			*s-- = (char) (decimal % 10 + (int) '0');
			decimal /= 10;
		}
	}
	else
	{          /* fixed point format */
		if (decimal <= 0)
		{
			if (digits_left > 0) *s++ = '0';
			if (digits_right > 0) *s++ = '.';
			for (i = decimal; i != 0 && digits_right > 0 ; i++)
			{
				*s++ = '0';
				digits_right--;
			}
			for (i = 0; i < digits_right; i++) *s++ = *str++;
		}
		else
		{
			for (i = 0; i < decimal; i++) *s++ = *str++;
			if (digits_right > 0)
			{
				*s++ = '.';
				for (i = 0; i < digits_right; i++) *s++ = *str++;
			}
		}
		*s = '\0';
	}
	str = NULL;
}

//----------------------------------------------------------------------------------------

void
TeGraphPlot :: plotInit(int pixels_above, int pixels_below, int pixels_left, int pixels_right)
{
// initialize Plot variables, reserving the number of pixels specified by
// pixels_above, pixels_below, pixels_left, and pixels_right for use by
// the program

	int delta, x_ticks, y_ticks;

// initialize variables appropriately for current plotting device
	X_pixels = getWidth ();
	Y_pixels = getHeight ();

    get_plot_fonts();
    get_font_size(title_font, &title_width, &title_height);
    get_font_size(label_font, &label_width, &label_height);
    get_font_size(xaxis_font, &xaxis_width, &xaxis_height);
    yaxis_direct = get_font_size(yaxis_font, &yaxis_width, &yaxis_height);
    delta = yaxis_direct == HORIZ_FONT ? yaxis_width : yaxis_height;
	xupleft = pixels_left + delta + (label_field + 1) * label_width;
    delta = title_height + label_height/2;
    yupleft = pixels_above + delta + title_height;
    Title_y = yupleft - delta;

// make width of Plot region a multiple of x_ticks
	x_ticks = x_major * x_minor;
	width = (X_pixels - pixels_right - xupleft -
		label_field * label_width / 2 + 1)
		/x_ticks*x_ticks + 1;

// make height of Plot region a multiple of y_ticks
	y_ticks = y_major * y_minor;
	delta = 2 * label_height + 1;
	height = (Y_pixels - pixels_below - yupleft - delta -
		xaxis_height)/y_ticks*y_ticks + 1;

// define y coordinate for x axis title
	Xaxis_y = yupleft + height - 1 + delta;
}

//----------------------------------------------------------------------------------------
void
TeGraphPlot :: get_plot_fonts()
{
    	title_font = 0;
    	label_font = 0;
    	xaxis_font = 0;
    	yaxis_font = 1;
}

//----------------------------------------------------------------------------------------
int
TeGraphPlot :: get_font_size(int font_number, int *w, int *h)
{
// return width, height, and direction of specified font number
	if (font_number < FONT_MAX)
	{
		*w = Font_width[font_number];
		*h = Font_height[font_number];
		return Font_direct[font_number];
	}
	else
	{
		return ERR;
	}
}

//----------------------------------------------------------------------------------------
void
TeGraphPlot :: reset()
{
	for (int i=0; i<Plot_curves; i++)
	{
		Plot_x[i] = NULL;
		Plot_y[i] = NULL;
	}

	Plot_curves = 0;  
	Plot_symbols = 0;  
	Curve_choice = 0;         
	Plot_choice = 0;
}

void TeGraphPlot::drawMoranText (double dx1, double dy1, double dx2, double dy2)
{
	int	w, h, x, y;
	int x1, y1, x2, y2, mx, my;

	setTextColor(Graph_color.text);

	value_to_pixel(dx1, dy1, &x1, &y1);
	value_to_pixel(dx2, dy2, &x2, &y2);
	value_to_pixel(0., 0., &mx, &my);

	string tx = "AA";
	textExtent(tx, w, h);
	x = (int)(mx + (double)(x2-mx)/2. - (double)w/2.);
	y = (int)(my - (double)(my-y2)/2. + (double)h/2.);
	drawText(x, y, (char*)tx.c_str());

	tx = "AB"; 
	textExtent(tx, w, h);
	x = (int)(mx - (double)(mx-x1)/2. - (double)w/2.);
	drawText(x, y, (char*)tx.c_str());

	tx = "BB"; 
	textExtent(tx, w, h);
	y = (int)(my + (double)(y1-my)/2. + (double)h/2.);
	drawText(x, y, (char*)tx.c_str());

	tx = "BA"; 
	textExtent(tx, w, h);
	x = (int)(mx + (double)(x2-mx)/2. - (double)w/2.);
	drawText(x, y, (char*)tx.c_str());
}
