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

#include "TeMSVFactory.h"
#include <vector>

using namespace std;

static TeSemivariogram_Factory semivar("Semivariogram");
static TeCorrelogram_Factory	 correlo("Correlogram");
static TeSemimadogram_Factory  semimad("Semimadogram");
static TeCovariance_Factory	 covaria("Covariance");


///////////////////////////////
TeMatrix
TeSemivariogram :: calculate ()
{
	TeMatrix		msvar;
	vector<double>	dis, gam, np;
	double			dmin, dmax, dirmin, dirmax, tolincr, inc;
	int				lin, col, nl, nrow, ncol;

	// Le o n�mero de linha e coluna da matriz de entrada
	nrow = MSVmatrix_->Nrow()-1;
	ncol = MSVmatrix_->Ncol()-1;

	// Inicializa a matriz de retorno
	msvar.Init((int)MSVnlag_, 2.);

	// Inicializa vetores auxiliares
	for (nl=0; nl<MSVnlag_; ++nl)
	{
		np.push_back(0.);
		dis.push_back(0.);
		gam.push_back(0.);
	}

	// Define a tolerancia do incremento = 50% do increm.
	tolincr = MSVincr_/2.;

	// Define dire��o minima e m�xima
	dirmin = dirmax = 0.0;
	if (MSVtoldir_ != 90.0)
	{
		dirmin = MSVdir_ - MSVtoldir_;
		dirmax = MSVdir_ + MSVtoldir_;
	}

	// Inicializa variavel auxiliar
	inc = MSVincr_;

	for (nl=0; nl<MSVnlag_; ++nl)
	{
		dmin = inc - tolincr;
		dmax = inc + tolincr;

		for (lin=0; lin<nrow; ++lin)
		{
			for (col=0; col<ncol; ++col)
			{
				if ( MSVtoldir_ == 90.0) // Omnidirecional
				{
					if ( lin<col && (*MSVmatrix_)(lin,col)>0. && (*MSVmatrix_)(lin,col)<=tolincr && nl==0 )
					{
						np[nl] = np[nl] + 1.;
						dis[nl] = dis[nl] + (*MSVmatrix_)(lin,col);
						gam[nl] = gam[nl] + (( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ) * ( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ));
					}
					else if ( lin<col && (*MSVmatrix_)(lin,col)>dmin && (*MSVmatrix_)(lin,col)<=dmax && nl!=0 )
					{
						np[nl] = np[nl] + 1.;
						dis[nl] = dis[nl] + (*MSVmatrix_)(lin,col);
						gam[nl] = gam[nl] + (( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ) * ( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ));
					}
				}
				else // Direcional
				{
					if ( lin<col && (*MSVmatrix_)(lin,col)>0. && (*MSVmatrix_)(lin,col)<=tolincr && (*MSVmatrix_)(col,lin)>=dirmin && (*MSVmatrix_)(col,lin)<=dirmax && nl==0 )
					{
						np[nl] = np[nl] + 1.;
						dis[nl] = dis[nl] + (*MSVmatrix_)(lin,col);
						gam[nl] = gam[nl] + (( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ) * ( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ));
					}
					else if ( lin<col && (*MSVmatrix_)(lin,col)>dmin && (*MSVmatrix_)(lin,col)<=dmax && (*MSVmatrix_)(col,lin)>=dirmin && (*MSVmatrix_)(col,lin)<=dirmax && nl!=0 )
					{
						np[nl] = np[nl] + 1.;
						dis[nl] = dis[nl] + (*MSVmatrix_)(lin,col);
						gam[nl] = gam[nl] + (( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ) * ( (*MSVmatrix_)(lin,lin) - (*MSVmatrix_)(col,col) ));
					}
				}
			}
		}
		if (nl != 0)
			inc = MSVincr_ + inc;
	}

	// Reporta o semivariograma
	for (nl=0; nl<MSVnlag_; ++nl)
	{

		if (np[nl] != 0)
		{
			dis[nl] = dis[nl] / np[nl];
			gam[nl] = (gam[nl] / np[nl])*.5;
		}
    
    	//Preenche a matriz de retorno da funcao
    	msvar(nl,0) = dis[nl];
    	msvar(nl,1) = gam[nl];
	}
	return msvar;
}


///////////////////////////////
TeMatrix
TeCorrelogram :: calculate ()
{
	TeMatrix		msvar;

	return msvar;
}


///////////////////////////////
TeMatrix
TeSemimadogram :: calculate ()
{
	TeMatrix		msvar;

	return msvar;
}


///////////////////////////////
TeMatrix
TeCovariance :: calculate ()
{
	TeMatrix		msvar;

	return msvar;
}
