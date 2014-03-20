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
/*! \file TeSemivarModelFactory.h
    \brief This file contains support do deal with factory semivar model
*/

#ifndef __TERRALIB_INTERNAL_SEMIVARMODEL_H
#define __TERRALIB_INTERNAL_SEMIVARMODEL_H

#ifdef WIN32 
#pragma warning ( disable: 4786 ) 
#endif

#include "../kernel/TeMatrix.h"
#include "../kernel/TeFactory.h"

#include "TeStatDefines.h"

/** @defgroup Semivar Classes to deal with the adjust of semivariogram models 
 *  @{
	@ingroup SpatialStatistics
 */
//! The parameters of a semivariogram model
struct STAT_DLL TeSemivarModelParams
{
	double		efeitopepita_;
	double		contribuicao_;
	double		alcance_;

	virtual string decName() const { return ""; }
};


//! An abstract semivariogram adjust model
class STAT_DLL TeSemivarModel		
{
public:
	TeSemivarModel (){};
	virtual ~TeSemivarModel (){};

	virtual TeMatrix	calculate (TeMatrix&)=0;
	static TeSemivarModel* DefaultObject(TeSemivarModelParams){ return 0; }

protected:
	double		modeloefeitopepita_;
	double		modelocontribuicao_;
	double		modeloalcance_;
};

//! A spherical semivariogram adjust model
class STAT_DLL TeEsfericSemivarModel : public TeSemivarModel
{
public:
		TeEsfericSemivarModel(const TeSemivarModelParams& params){
		modeloefeitopepita_ = params.efeitopepita_;
		modelocontribuicao_ = params.contribuicao_;
		modeloalcance_ = params.alcance_;
	}		
	
	virtual ~TeEsfericSemivarModel(){};

	virtual TeMatrix	calculate (TeMatrix&);	
};

//! An exponential semivariogram adjust model
class STAT_DLL TeExponentialSemivarModel : public TeSemivarModel
{
public:
		TeExponentialSemivarModel(const TeSemivarModelParams& params){
		modeloefeitopepita_ = params.efeitopepita_;
		modelocontribuicao_ = params.contribuicao_;
		modeloalcance_ = params.alcance_;
	}		
	
	virtual ~TeExponentialSemivarModel(){};

	virtual TeMatrix	calculate (TeMatrix&);	
};

//! A gaussian semivariogram adjust model
class STAT_DLL TeGaussianSemivarModel : public TeSemivarModel
{
public:
		TeGaussianSemivarModel(const TeSemivarModelParams& params){
		modeloefeitopepita_ = params.efeitopepita_;
		modelocontribuicao_ = params.contribuicao_;
		modeloalcance_ = params.alcance_;
	}		
	
	virtual ~TeGaussianSemivarModel(){};

	virtual TeMatrix	calculate (TeMatrix&);	
};

//! An abstract factory of a semivariogram adjust model
class STAT_DLL TeSemivarModelFactory : public TeFactory<TeSemivarModel, TeSemivarModelParams>
{
public:
	TeSemivarModelFactory( const string& name) : TeFactory<TeSemivarModel, TeSemivarModelParams>(name){}

	virtual TeSemivarModel* build( const TeSemivarModelParams&) = 0;
};


//! A concrete factory to build a spheric adjust model
class STAT_DLL TeEsfericSemivar_Factory : public TeSemivarModelFactory
{
public:
	TeEsfericSemivar_Factory (const string& name) : TeSemivarModelFactory(name){}

	virtual TeSemivarModel* build(const TeSemivarModelParams& params){return new TeEsfericSemivarModel(params);}
};


//! A concrete factory to build a exponential adjust model
class STAT_DLL TeExponentialSemivar_Factory : public TeSemivarModelFactory
{
public:
	TeExponentialSemivar_Factory (const string& name) : TeSemivarModelFactory(name){}

	virtual TeSemivarModel* build(const TeSemivarModelParams& params){return new TeExponentialSemivarModel(params);}
};


//! A concrete factory to build a gaussian adjust model
class STAT_DLL TeGaussianSemivar_Factory : public TeSemivarModelFactory
{
public:
	TeGaussianSemivar_Factory (const string& name) : TeSemivarModelFactory(name){}

	virtual TeSemivarModel* build(const TeSemivarModelParams& params){return new TeGaussianSemivarModel(params);}
};
/** @} */ 


#endif


