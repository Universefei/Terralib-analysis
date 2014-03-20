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
/*! \file TeKernelParams.h
    \brief This file contains structures and definitions about Kernel parameters
*/
#ifndef __TERRALIB_INTERNAL_KERNELPARAMS_H
#define __TERRALIB_INTERNAL_KERNELPARAMS_H

#include "TeStatDefines.h"
#include "../kernel/TeUtils.h"

#include <string>
#include <vector>
using namespace std;

//! Kernel Algorithm types
enum TeKernelFunctionType 
 {TeKQuartic, TeKNormal, TeKUniform, TeKTriangular, TeKNegExponential};


//! Kernel Evaluation types
enum TeKernelComputeType 
{TeKDensity, TeKMovingAverage, TeKProbability};

enum TeKernelCombinationType
{TeKRatio, TeKLogRatio, TeAbsDifference, TeRelDifference, TeAbsSum, TeRelSum};


/*! \struct TeKernelParams
	Kernel parameters representation
*/
struct STAT_DLL TeKernelParams
{
	//first event theme
	int						eventThemeId1_;			// event theme id
	string					eventThemeName1_;		// event theme name
	string					intensityAttrName1_;	// intensity attribute name 		
	string					intensityAttrTable1_;	// table name that keeps the intensity attribute
	TeKernelFunctionType	kernelFunction1_;		// kernel function
	TeKernelComputeType		computeType1_;
	double					radiusValue1_;			// radius value if the kernel is not adaptative 

	TeKernelCombinationType	combinationType_;
	
	//second event theme (if Kernel ratio)
	int						eventThemeId2_;			// event theme id
	string					eventThemeName2_;		// event theme name
	string					intensityAttrName2_;	// intensity attribute name 		
	string					intensityAttrTable2_;	// table name that keeps the intensity attribute
	TeKernelFunctionType	kernelFunction2_;		// kernel function
	TeKernelComputeType		computeType2_;
	double					radiusValue2_;			// radius value if the kernel is not adaptative 
	
	//informations about the kernel result
	int						supportThemeId_;		// support theme id
	string					supportThemeName_;		// support theme name 
	string					supportThemeTable_;		// support theme table that keeps the kernel results 
	string					supportThemeAttr_;		// attribute name of the support theme table that keeps the kernel results   	
	int						generatedLayerId_;		// id of the generated layer (when it�s raster) 
	string					generatedLayerName_;	// name of the generated layer (when it�s raster)
	
	//! Empty constructor
	TeKernelParams():
		eventThemeId1_(0),
		eventThemeName1_(""),
		intensityAttrName1_(""),
		intensityAttrTable1_(""),
		kernelFunction1_(TeKQuartic),
		computeType1_(TeKDensity),
		radiusValue1_(0.0),
		combinationType_(TeKRatio),
		eventThemeId2_(0),
		eventThemeName2_(""),
		intensityAttrName2_(""),
		intensityAttrTable2_(""),
		kernelFunction2_(TeKQuartic),
		computeType2_(TeKDensity),
		radiusValue2_(0.0),
		supportThemeId_(0),
		supportThemeName_(""),
		supportThemeTable_(""),
		supportThemeAttr_(""),
		generatedLayerId_(0),
		generatedLayerName_("")
		{}

};

//! A vector of TeKernelParams
typedef vector<TeKernelParams> TeKernelParamsVector;


#endif

