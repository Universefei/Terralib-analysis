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
/*! \file TeMSVFactory.h
    \brief This file contains support do deal with Measures of Spatial Variability
*/
#ifndef __TERRALIB_INTERNAL_SPATIALMEASUREFACTORY_H
#define __TERRALIB_INTERNAL_SPATIALMEASUREFACTORY_H

#ifdef WIN32 
#pragma warning ( disable: 4786 ) 
#endif


#include <map>
#include <string>
#include "../kernel/TeMatrix.h"
#include "../kernel/TeFactory.h"

#include "TeStatDefines.h"

using namespace std;

/** @defgroup MSV Classes and structures to deal with measures of spatial variability 
 *  @{
 	@ingroup SpatialStatistics
 */
//! Struct of parameters for measures of spatial variability 
struct STAT_DLL TeMSVParams
{
	TeMatrix	MSVPmatrix_;
	double		MSVPdir_;
	double		MSVPtoldir_;
	double		MSVPincr_;
	double		MSVPnlag_;

	virtual string decName() const { return ""; }
};

//! An abstract class for measures of spatial variability 
class STAT_DLL TeMSV		
{
public:
	TeMSV (){};
	virtual ~TeMSV () {}

	virtual TeMatrix	calculate ()=0;
	static TeMSV* DefaultObject(TeMSVParams){ return 0; }		
	void setMatrix(TeMatrix* m) {MSVmatrix_ = m;}

protected:
	TeMatrix*	MSVmatrix_;
	double		MSVdir_;
	double		MSVtoldir_;
	double		MSVincr_;
	double		MSVnlag_;		
};


//! A class that represents a semivariogram
class STAT_DLL TeSemivariogram : public TeMSV
{
public:
	TeSemivariogram(const TeMSVParams& params){
		MSVdir_ = params.MSVPdir_;
		MSVtoldir_ = params.MSVPtoldir_;
		MSVincr_ = params.MSVPincr_;
		MSVnlag_ = params.MSVPnlag_;
	}
	
	~TeSemivariogram() { }

	virtual TeMatrix	calculate ();	
};


//! A class that represents a correlogram
class STAT_DLL TeCorrelogram : public TeMSV
{
public:
		TeCorrelogram(const TeMSVParams& params){
		MSVdir_ = params.MSVPdir_;
		MSVtoldir_ = params.MSVPtoldir_;
		MSVincr_ = params.MSVPincr_;
		MSVnlag_ = params.MSVPnlag_;
	}		
	
	~TeCorrelogram(){};

	virtual TeMatrix	calculate ();	
};

//! A class that represents a Semimadogram
class STAT_DLL TeSemimadogram : public TeMSV
{
public:
		TeSemimadogram(const TeMSVParams& params){
		MSVdir_ = params.MSVPdir_;
		MSVtoldir_ = params.MSVPtoldir_;
		MSVincr_ = params.MSVPincr_;
		MSVnlag_ = params.MSVPnlag_;
	}		
	
	~TeSemimadogram(){};

	virtual TeMatrix	calculate ();	
};

//! A class that represents a covariance
class STAT_DLL TeCovariance : public TeMSV
{
public:
		TeCovariance(const TeMSVParams& params){
		MSVdir_ = params.MSVPdir_;
		MSVtoldir_ = params.MSVPtoldir_;
		MSVincr_ = params.MSVPincr_;
		MSVnlag_ = params.MSVPnlag_;
	}		
	
	~TeCovariance(){};

	virtual TeMatrix	calculate ();	
};


//! An abstract factory of spatial variability measures
class STAT_DLL TeMSVFactory : public TeFactory<TeMSV, TeMSVParams>
{
public:
	TeMSVFactory(const string& name) : TeFactory<TeMSV, TeMSVParams>(name){}
	virtual TeMSV* build(const TeMSVParams&) = 0;
};


//! A concrete factory of semivariograms
class STAT_DLL TeSemivariogram_Factory : public TeMSVFactory
{
public:
	TeSemivariogram_Factory (const string& name) : TeMSVFactory(name){}

	virtual TeMSV* build(const TeMSVParams& params){return new TeSemivariogram(params);}
};


//! A concrete factory of correlograms
class STAT_DLL TeCorrelogram_Factory : public TeMSVFactory
{
public:
	TeCorrelogram_Factory (const string& name) : TeMSVFactory (name){}

	virtual TeMSV* build (const TeMSVParams& params){return new TeCorrelogram(params);}
};


//! A concrete factory of semimadograms
class STAT_DLL TeSemimadogram_Factory : public TeMSVFactory
{
public:
	TeSemimadogram_Factory (const string& name) : TeMSVFactory (name){}

	virtual TeMSV* build (const TeMSVParams& params){return new TeSemimadogram(params);}
};


//! A concrete factory of covariances
class STAT_DLL TeCovariance_Factory : public TeMSVFactory
{
public:
	TeCovariance_Factory (const string& name) : TeMSVFactory (name){}

	virtual TeMSV* build (const TeMSVParams& params){return new TeCovariance(params);}
};
/** @} */ 
#endif


