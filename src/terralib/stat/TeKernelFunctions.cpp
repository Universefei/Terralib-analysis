/************************************************************************************
Exploring and analysis of geographical data using TerraLib and TerraView
Copyright � 2003-2007 INPE and LESTE/UFMG.

Partially funded by CNPq - Project SAUDAVEL, under grant no. 552044/2002-4,
SENASP-MJ and INPE

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed hoping it will be useful, however, WITHOUT ANY 
WARRANTIES; neither to the implicit warranty of MERCHANTABILITY OR SUITABILITY FOR
AN SPECIFIC FINALITY. Consult the GNU General Public License for more details.

You must have received a copy of the GNU General Public License with this program.
In negative case, write to the Free Software Foundation, Inc. in the following
address: 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.
***********************************************************************************/

#include "TeKernelFunctions.h"
#include "../kernel/TeDefines.h"
#include <math.h>


double TeKernelQuartic(double tau, double distance, double intensity) 
{
    if (distance > tau)
		return 0.0;

	return intensity * (3.0 / (tau * tau * TePI)) *
		pow(1 - ((distance * distance)/ (tau * tau)),2.0);
}

double TeKernelNormal(double tau, double distance, double intensity)
{
	return intensity * (1.0 / (tau * tau * 2 * TePI)) *
		exp(-1.0 * (distance * distance)/ (2 * tau * tau));
}

double TeKernelUniform(double tau, double distance, double intensity)
{
    if (distance > tau)
		return 0.0;

	return intensity;
}

double TeKernelTriangular(double tau, double distance, double intensity)
{
    if (distance > tau)
		return 0.0;

	return intensity * (1.0 - 1.0/tau) * distance;
}

double TeKernelNegExponential(double tau, double distance, double intensity)
{
    if (distance > tau)
		return 0.0;

	return intensity * exp(-3.0 * distance);
}


