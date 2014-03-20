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
/*! \file TeKernelFunctions.h
    \brief This file contains functions to spatial kernel estimation: basic, adaptive and ratio 
*/

#ifndef __TERRALIB_INTERNAL_KERNELFUNCTIONS_H
#define __TERRALIB_INTERNAL_KERNELFUNCTIONS_H

#include "../kernel/TeException.h"
#include "TeKernelParams.h"
#include "TeStatDefines.h"
#include "../kernel/TeGeometryAlgorithms.h"
#include "../kernel/TeProgress.h"


#define   INVALID_KERNEL -TeMAXFLOAT

#define IDX_ATTR            0
#define IDX_KERNEL          0
#define IDX_NUMERATOR       1
#define IDX_KERNEL1         0
#define IDX_KERNEL2         1

enum TeKernelError {
 KERNEL_NO_CENTROID,
   KERNEL_ALL_NULL,
   KERNEL_INVALID_AREA,
   KERNEL_INVALID_VALUE,
   KERNEL_NO_VALUE
};

class STAT_DLL TeKernelException  {

  TeKernelError error_;

public:

  TeKernelException(TeKernelError e) {
    error_ = e;
  }

  TeKernelError getErrorCode() {
    return error_;
  }
};



/*! Kernel functions 

 * \param tau spatial threshold to define neighboorhood
 * \param distance distance between event and region centroid
 * \param intensity attribute value for event
*/
STAT_DLL double TeKernelQuartic(double tau, double distance, double intensity);
STAT_DLL double TeKernelNormal(double tau, double distance, double intensity);
STAT_DLL double TeKernelUniform(double tau, double distance, double intensity);
STAT_DLL double TeKernelTriangular(double tau, double distance, double intensity);
STAT_DLL double TeKernelNegExponential(double tau, double distance, double intensity);


/**
 * Calcula a media geometrica de um conjunto de valores atraves da 
 * utilizacao de logaritmos
 * Implementado pelo Andre
 **/
template <typename ItReg>
double TeKernelGeometricMean(ItReg& beginReg, ItReg& endReg, int idx)
{
   double mantissa, MediaE, MediaM;
   int expoente, Cont;
   int MediaETmp = 0;
   double MediaMTmp = 0;

   double log_two = log(2.0);
   Cont = 0;
   ItReg it = beginReg;
   double val;
   while (it != endReg) {  
         if ((*it).getDoubleProperty(idx, val) && (val > 0))
         {
            mantissa = frexp(val,&expoente);
            MediaMTmp = MediaMTmp+log(mantissa);
            MediaETmp = MediaETmp+expoente;
            Cont++;
         }
         ++it;
   }

   MediaE = MediaETmp;
   MediaM = (MediaMTmp+(MediaE*log_two))/Cont;
   MediaM = exp(MediaM);

   return MediaM;
}



/*! \brief Evaluates normalization factor for kernel
 * \param begin begin of event set iterator
 * \param end   end of event set iterator
 * \return sum of intensities
 **/
template <typename ItEvt>
double KernelNormalizationFactor(ItEvt itBegin, ItEvt itEnd) {

  double normFactor = 0;
  double intensity;

  ItEvt evtIter = itBegin;

   while (evtIter != itEnd) {

     (*evtIter).getDoubleProperty(IDX_ATTR, intensity);
     normFactor += intensity;
      ++evtIter;
    }

  return normFactor;

}


/*! \brief Evaluates kernel value of events with intensity (attribute)
 * for one support region, for a kernel function
 * \param center support region centroid
 * \param begin  begin of event set iterator
 * \param end    end of event set iterator
 * \param radius spatial threshold
 * \param kfunc  type of kernel function
 * This function assumes that intensity value of event is available on its first
 * property - if it does not exist, it is set to 1.0 (identity)
 **/
template <typename ItEvt>
double TeKernelValue (TeCoord2D& center, ItEvt& begin, ItEvt& end, 
					  double radius, TeKernelFunctionType kfunc)
{

  string    intstr;

	TeCoord2D location;		//Event spatial location
  double    intensity;	//Event intensity

  //Final kernel value
	double kernel = 0; 

  //Iterates through events, getting their location and intensity values
  //and updating kernel value
	ItEvt it = begin;
	while (it != end) 
	{
		(*it).centroid(location);

		//If there is no properties, assume intensity of one
	    if (!(*it).getDoubleProperty(IDX_ATTR, intensity)) {
				intensity = 1.0;
		}

		//Evaluates kernel value for one event
		double distance = TeDistance (location,center);
		double localK;

		switch(kfunc) {
		case TeKQuartic:
			localK = TeKernelQuartic(radius,distance,intensity);
			break;
		case TeKNormal:
			localK = TeKernelNormal(radius,distance,intensity);
			break;
		case TeKTriangular:
			localK = TeKernelTriangular(radius,distance,intensity);
			break;
		case TeKNegExponential:
			localK = TeKernelNegExponential(radius,distance,intensity);
			break;
		case TeKUniform:
			localK = TeKernelUniform(radius,distance,intensity);
			break;
		}
		kernel += localK; 
	    ++it;

	}
	return kernel;
}

/*! \brief Normalizes kernel values based on type of computation. 
 *  \brief Gets value from IDX_KERNEL and stores it at the same place 
 * \param events set of spatial events
 * \param regionsBegin begin of regions iterator
 * \param reginsEnd end of region iterator
 * \param kfunc type of kernel function
 * \param ktype type of computation (SMA, density or probability)
 * \param radius spatial threshold
 **/
template<typename EventSet, typename ItRegionSet>
bool TeKernelNormalize(EventSet& events, 
                   ItRegionSet& regionsBegin,
                   ItRegionSet& regionsEnd,
                   TeKernelComputeType  kType,
                   double totKernel,
                   int idxProp)
{

  double normKernel, kernel, area;
  try {

  //Evaluates sum of attributes for normalization
    double normFactor = 
      KernelNormalizationFactor(events.begin(), events.end());

    //Reiterates through regions defining normalized kernel value,
    //given the computation type
    ItRegionSet regIter = regionsBegin;
    while (regIter != regionsEnd) 
    {
      
      (*regIter).getDoubleProperty(idxProp, kernel);
      if (kernel < 0) {
         throw TeKernelException(KERNEL_INVALID_VALUE);
      }

      //Evaluates value based on computation type
      switch(kType) {
      case TeKMovingAverage:
        normKernel = (kernel * normFactor)/totKernel;
        break;
      case TeKDensity:
        //If region does not have an area, assumes one
        (*regIter).area(area); 
        if (area <= 0) {
          throw TeKernelException(KERNEL_INVALID_AREA);
        }
        normKernel = ((kernel * normFactor)/totKernel)/area;
        break;
      case TeKProbability:
        normKernel = kernel/totKernel;
        break;
      }

      //Store final value
      (*regIter).setDoubleProperty(idxProp, normKernel);
      ++regIter;
    }  
  } catch (TeKernelException /* e */) {
//    int a = e.getErrorCode();
    return false;
  }
  return true;
}


/*! \brief Evaluates kernel value for all support regions, 
 * storing values either on IDX_KERNEL or IDX_NUMERATOR.  
 * \param events set of spatial events
 * \param regionsBegin begin of regions iterator
 * \param reginsEnd end of region iterator
 * \param kfunc type of kernel function
 * \param ktype type of computation (SMA, density or probability)
 * \param radius spatial threshold
 * \param idxProp index of double property
 **/
template<typename EventSet, typename ItRegionSet>
bool TeStatIntKernel(EventSet& events, 
                   ItRegionSet& regionsBegin,
                   ItRegionSet& regionsEnd,
                   TeKernelFunctionType kfunc, 
                   TeKernelComputeType  kType,
                   double radius,  
                   int idxProp)
{


  //Support region attributes
  TeCoord2D location;
  double    kernel;
  
  //Regions iterator
  ItRegionSet regIter = regionsBegin;
  // Sum of kernels
  double totKernel = 0;

  //Treats unexpected expections returning false
  try {

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int nsteps = 0;
    //Iterates through regions
    while (regIter != regionsEnd) 
    {
      (*regIter).centroid(location); 
      
      //Define iterators for event sets -- may optimize
      typename EventSet::iterator subSetBegin = events.begin(location,radius);
      typename EventSet::iterator subSetEnd = events.end(location,radius);
      if (kfunc == TeKNormal) {
        subSetBegin = events.begin();
        subSetEnd = events.end();
      }

      //Evaluates kernel value for region
      kernel = TeKernelValue(location, subSetBegin, subSetEnd, 
        radius, kfunc);
      
      totKernel += kernel;

      (*regIter).setDoubleProperty(idxProp, kernel);
      ++regIter; 
	  t2 = clock();
	  ++nsteps;
	  if (TeProgress::instance())
	  {
		 if (int(t2-t1) > dt)
		 {
			t1 = t2;
			if (TeProgress::instance()->wasCancelled())
				break;
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(nsteps);
		 }
	  }
	}
	if (TeProgress::instance())
		TeProgress::instance()->reset();

    if (totKernel <= 0) {
      throw TeKernelException(KERNEL_ALL_NULL);
    }

    return TeKernelNormalize(events, regionsBegin, regionsEnd, kType, totKernel,
      idxProp);

  } catch (TeKernelException /* e */) {
//    int a = e.getErrorCode();
    return false;
  }
  
  return true;
}


/*! \brief Evaluates adaptive kernel value for all support regions, 
 * storing values (intermediary and final) in double property idxProp  
 * \param events set of spatial events
 * \param regionsBegin begin of regions iterator
 * \param reginsEnd end of region iterator
 * \param kfunc type of kernel function
 * \param ktype type of computation (SMA, density or probability)
 * \param numReg number of support regions -- need to evaluate initial radius
 * \param totalArea total area of support regions -- need to evaluate initial radius
 * \param idxProp index of double property
 **/
template<typename EventSet, typename ItRegionSet>
bool TeStatAdaptiveGeoMeanIntKernel(EventSet& events, 
                                    ItRegionSet& regionsBegin,
                                    ItRegionSet& regionsEnd,
                                    TeKernelFunctionType kfunc,
                                    TeKernelComputeType ktype,
                                    int /* numReg */,
                                    double totalArea,
                                    int idxProp) 
{

  try {

    //Evaluate kernel with a fixed radius, based on formula, storing in vecValues
    double radius = 0.68*pow((double)events.numObjects(),-0.2)*sqrt(totalArea);
    double sqArea = sqrt(totalArea);
    if (!TeStatIntKernel(events, regionsBegin, regionsEnd,
      kfunc, ktype, radius, idxProp))
      return false;
    
    //Evaluate geometric mean of kernel values, to adjust radius
    double meanKernel = TeKernelGeometricMean(regionsBegin, regionsEnd, idxProp);
    
    if (meanKernel <= 0) {
      throw TeKernelException(TeKernelException(KERNEL_ALL_NULL));
    }
    
    //Now, reassign radius, evaluating final value for kernel
    
    ItRegionSet regIter = regionsBegin;
    double    prevKernel;
    double    newKernel;
    double    newRadius;
    double    totKernel = 0;
    TeCoord2D location;
    
	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;
	int nsteps = 0;

    //Iterates through regions
    while (regIter != regionsEnd) 
    {
      (*regIter).centroid(location); 
      
      //Define iterators for event sets -- may optimize
      typename EventSet::iterator subSetBegin = events.begin(location,radius);
      typename EventSet::iterator subSetEnd = events.end(location,radius);
      if (kfunc == TeKNormal) {
        subSetBegin = events.begin();
        subSetEnd = events.end();
      }

      if (!((*regIter).getDoubleProperty(idxProp, prevKernel)) || (prevKernel < 0)) {
         throw TeKernelException(KERNEL_INVALID_VALUE);
      }
  
      if (prevKernel > 0) {
        newRadius = radius*pow((meanKernel/prevKernel),0.5);
        if (newRadius > sqArea/4.0)
          newRadius = sqArea/4.0; //Para funcionar quando o valor eh muito pequeno
        newKernel = TeKernelValue(location, subSetBegin, subSetEnd,  newRadius, kfunc);
      }
      else {
        newKernel = 0.0;
      }
      
      totKernel += newKernel;
      (*regIter).setDoubleProperty(idxProp, newKernel);
      ++regIter; 
	  t2 = clock();
	  ++nsteps;
	  if (TeProgress::instance())
	  {
		 if (int(t2-t1) > dt)
		 {
			t1 = t2;
			if (TeProgress::instance()->wasCancelled())
				break;
			if((int)(t2-t0) > dt2)
				TeProgress::instance()->setProgress(nsteps);
		 }
	  }
    }
	if (TeProgress::instance())
		TeProgress::instance()->reset();

    if (totKernel <= 0) {
      throw TeKernelException(KERNEL_ALL_NULL);
    }

    return TeKernelNormalize(events, regionsBegin, regionsEnd, ktype, totKernel,
      idxProp);
  } catch (TeKernelException /* e */) { 
//      int a = e.getErrorCode();
      return false;
  }
    
  return true;
}



/*! Class to apply kernel method
*
*/
class STAT_DLL TeStatKernel {

public:

  //! type of kernel function
  TeKernelFunctionType kfunc_;
  //! type of computation
  TeKernelComputeType  ktype_;
  //! spatial threshold
  double               radius_;
  //! sum of support region areas
  double               totalArea_;
  //! number of support regions
  int                  numReg_;
  
  TeStatKernel() {
    kfunc_ = TeKQuartic;
    ktype_ = TeKDensity;
    radius_ = 0;
    totalArea_ = 0;
    numReg_ = 0;
  }

  /*! \brief Apply kernel method to a specif set of support regions
   * and events, storing results in resName_.
   * \param events set of point events
   * \param regionsBegin iterator for support regions
   * \param regionsEnd   iterator for support regions
   * If radius_ is 0, then applies adaptive kernel.
   */
  template<typename EventSet, typename ItRegionSet>
  bool apply(EventSet& events, 
    ItRegionSet regionsBegin,
    ItRegionSet regionsEnd) {
    if (totalArea_ <= 0) {
      return false;
    }

    if (numReg_ <= 0) {
      return false;
    }

    bool result;
    //Radius defined, kernel with a fixed radius
    if (radius_ > 0) {
      result = TeStatIntKernel(events,regionsBegin, regionsEnd, 
        kfunc_,ktype_, radius_, IDX_KERNEL);
      
    }
    //Radius undefined, adaptive kernel
    else {
      result = TeStatAdaptiveGeoMeanIntKernel(events,  regionsBegin, regionsEnd, 
        kfunc_, ktype_, numReg_, totalArea_, IDX_KERNEL); 

    }
    return result;
  }

};



class STAT_DLL TeStatKernelRatio
{
  public:
  int						numReg_;
  
  public:

  TeKernelCombinationType	kComb_;
  TeKernelComputeType		  ktype_;
  TeKernelFunctionType		kfunc1_;
  double					        radius1_;
  TeKernelFunctionType		kfunc2_;
  double				        	radius2_;
  double				        	totalArea_;

  
  TeStatKernelRatio() {
    kComb_ = TeKRatio;
    kfunc1_ =  kfunc2_ = TeKQuartic;
    ktype_ = TeKDensity;
    radius1_ = radius2_ = 0;
    totalArea_ = 0;
    numReg_ = 0;
  }
   
  template <typename EventSet1, typename EventSet2, typename ItRegionSet>
  bool apply (EventSet1& ev1, 
    EventSet2& ev2, 
    ItRegionSet regBegin, 
    ItRegionSet regEnd) {
   
    
    if (totalArea_ <= 0)
      return false;
    
    if (numReg_ <= 0) 
      return false;
    
    //Calcula kernel para primeiro conjunto

    bool result;
    //Radius defined, kernel with a fixed radius
    if (radius1_ > 0) {
      result = TeStatIntKernel(ev1,regBegin, regEnd, 
        kfunc1_,ktype_, radius1_, IDX_KERNEL1);
      
    }
    //Radius undefined, adaptive kernel
    else {
      result = TeStatAdaptiveGeoMeanIntKernel(ev1,  regBegin, regEnd, 
        kfunc1_, ktype_, numReg_, totalArea_, IDX_KERNEL1); 

    }
    
    if (!result)
      return result;

  
    //Calcula kernel para segundo conjunto
     //Radius defined, kernel with a fixed radius
    if (radius2_ > 0) {
      result = TeStatIntKernel(ev2,regBegin, regEnd, 
        kfunc2_,ktype_, radius2_, IDX_KERNEL2);
      
    }
    //Radius undefined, adaptive kernel
    else {
      result = TeStatAdaptiveGeoMeanIntKernel(ev2,  regBegin, regEnd, 
        kfunc2_, ktype_, numReg_, totalArea_, IDX_KERNEL2); 

    }

    if (!result)
      return result;
    
    //Itera calculando o tipo de combinacao e armazenando no valor da regiao
    //Percorre todas as regioes
//    int Cont = 0;
    double kernel;
    double k1;
    double k2;
    double area;
    
    ItRegionSet regIter = regBegin;
 
    while (regIter != regEnd) 
    {
  
      (*regIter).area(area);
      
      (*regIter).getDoubleProperty(IDX_KERNEL1, k1);
      (*regIter).getDoubleProperty(IDX_KERNEL2, k2);
      
      switch(kComb_) 
      {
      case TeKRatio:
        if (k2 == 0.0)
          kernel = 0.0;//INVALID_KERNEL;
        else
          kernel = k1/k2;
        break;
      case TeKLogRatio:
        if (k2 == 0.0)
          kernel = INVALID_KERNEL;
        else
          kernel = log(k1/k2);
        break;
      case TeAbsDifference:
        kernel = k1 - k2;
        break;
      case TeRelDifference:
        kernel = (k1 - k2) * area;
        break;
      case TeAbsSum:
        kernel = k1+k2;
        break;
      case TeRelSum:
        kernel = (k1 + k2) * area;
        break;
      }
	  (*regIter).setDoubleProperty(IDX_KERNEL, kernel);
	  ++regIter; 
    }
    
    return true;
  }
                       
};


#endif





