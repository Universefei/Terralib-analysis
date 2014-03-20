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

#include "TeSemivarModelFactory.h"
#include <math.h>
#include <vector>
using namespace std;

static TeEsfericSemivar_Factory mesf("Esferic");
static TeExponentialSemivar_Factory mexp("Exponential");
static TeGaussianSemivar_Factory mgau("Gaussian");


TeMatrix
TeEsfericSemivarModel :: calculate (TeMatrix& g)
{
	TeMatrix		mxesf;
	vector<double>	esf;
	int				i, nlag;
	double			c0, c1, a;

	c0=modeloefeitopepita_;
	c1=modelocontribuicao_;
	a=modeloalcance_;
	nlag = g.Nrow();
	mxesf.Init(nlag+1, 2);

	for (i=0; i<nlag; ++i)
	{
		if (g(i,0) == 0)
			esf.push_back(c0);

		else if (g(i,0) > 0 && g(i,0) < 1000000.0)
		{
			esf.push_back( c0 + c1*(1.5*(g(i,0)/a)) - 0.5*(pow(g(i,0)/a,3) ));
			if (esf[i] > c0+c1)
				esf[i]=c0+c1;
		}

		else 
            esf.push_back(c0+c1);
    
		// Escreve Saida
		mxesf(i+1,0) = g(i,0); // h
        mxesf(i+1,1) = esf[i]; // gama(h)
    }

	// quando h=0 => gama(h)=c0
	mxesf(0,1)=c0;

	// acrescentar o �ltimo valor
	mxesf(nlag,0)= mxesf(nlag-1,0) + (mxesf(nlag-1,0) - mxesf(nlag-2,0));
    mxesf(nlag,1)= mxesf(nlag-1,1);

	return mxesf;
}

TeMatrix
TeExponentialSemivarModel :: calculate (TeMatrix& g)
{
	TeMatrix		mxexp;
	vector<double>	expo;
	int				i, nlag;
	double			c0, c1, a;

	c0=modeloefeitopepita_;
	c1=modelocontribuicao_;
	a=modeloalcance_;
	nlag = g.Nrow();
	mxexp.Init(nlag+1, 2);

	for (i=0; i<nlag; ++i)
	{
		if (g(i,0) == 0)
			expo.push_back(c0);

		//expo(i) = c0 + c1*( 1 - exp( -(3*g(i,1))/a) );
		expo.push_back(c0 + c1*( 1 - exp( -(3*g(i,0))/a) ));

		// Escreve Saida
		mxexp(i+1,0) = g(i,0); // h
        mxexp(i+1,1) = expo[i]; // gama(h)
    }

	// quando h=0 => gama(h)=c0
	mxexp(0,1)=c0;

	// acrescentar o �ltimo valor
	mxexp(nlag,0)= mxexp(nlag-1,0) + (mxexp(nlag-1,0) - mxexp(nlag-2,0));
    mxexp(nlag,1)= mxexp(nlag-1,1);

	return mxexp;
}

TeMatrix
TeGaussianSemivarModel :: calculate (TeMatrix& g)
{
	TeMatrix		mxgau;
	vector<double>	gau;
	int				i, nlag;
	double			c0, c1, a;

	c0=modeloefeitopepita_;
	c1=modelocontribuicao_;
	a=modeloalcance_;
	nlag = g.Nrow();
	mxgau.Init(nlag+1, 2);

	for (i=0; i<nlag; ++i)
	{
		if (g(i,0) == 0)
			gau.push_back(c0);

	   // gaus(i) = c0 + c1*( 1 - exp(-3*(g(i,1)/a)^2) );
		gau.push_back( c0 + c1*( 1 - exp(-3*pow(g(i,0)/a,2))) );

		// Escreve Saida
		mxgau(i+1,0) = g(i,0); // h
        mxgau(i+1,1) = gau[i]; // gama(h)
    }

	// quando h=0 => gama(h)=c0
	mxgau(0,1)=c0;

	// acrescentar o �ltimo valor
	mxgau(nlag,0)= mxgau(nlag-1,0) + (mxgau(nlag-1,0) - mxgau(nlag-2,0));
    mxgau(nlag,1)= mxgau(nlag-1,1);

	return mxgau;
}


/*
for i=1:nlag;
    if (g(i,1) == 0)
        sphe(i) = c0;
        gaus(i) = c0;
        expo(i) = c0;
    elseif (g(i,1) > 0 & g(i,1) < a)
        sphe(i) = c0 + c1*((1.5*(g(i,1)/a) - 0.5*((g(i,1)/a)^3)));
    else 
        if (g(i,1) >= a)
            sphe(i)=c0+c1;
        end
    end
    gaus(i) = c0 + c1*( 1 - exp(-3*(g(i,1)/a)^2) );
    expo(i) = c0 + c1*( 1 - exp( -(3*g(i,1))/a) );
    
    % Escreve Saida
    m(i+1,1) = g(i,1);% lag distance

    switch (model)
        case 1
            m(i+1,2) = sphe(i);
        case 2
            m(i+1,2) = expo(i);
        case 3
            m(i+1,2) = gaus(i);
        otherwise      
            m(i+1,2) = 0;
    end
end

%Para o modelo quando h=0 => gamma(h)=c0
m(1,2)=c0;

%Para o modelo quando h >= a => gamma(h)=c0+c1
switch (model)
    case 1 %Para modelo esferico
    m(nlag+2,2)= c0+c1;

    case 2 %Para modelo exponencial
    m(nlag+2,2)= m(nlag+1,2);
    
    case 3 %Para modelo gaussiano
    m(nlag+2,2)= m(nlag+1,2);
end
*/
