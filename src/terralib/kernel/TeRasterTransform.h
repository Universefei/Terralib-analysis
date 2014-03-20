/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeRasterTransform.h
    \brief This file deals with the transformation functions over raster pixels
*/
#ifndef  __TERRALIB_INTERNAL_RASTERTRANSFORM_H
#define  __TERRALIB_INTERNAL_RASTERTRANSFORM_H

#include "TeRaster.h"
#include "TeLegendEntry.h"

#include <map>
#include <vector>
using namespace std;

//! This class defines a way of evaluating the final value of a raster element
class TL_DLL TeRasterTransform
{
public:

	//! The three channels of a display
	enum TeRGBChannels { TeREDCHANNEL=0, TeGREENCHANNEL=1, TeBLUECHANNEL=2 };

	enum TeRasterTransfFunctions
	{	TeNoTransf=0, TeMono2Three=1, TeBand2Band=2, TePall2Three=3, TeLUT2Three=4, 
		TeExtractRGB=5, TeExtractBand=6, TeExtractBands=7, TeThreeBand2RGB=8, 
		TeThree2LUTThreeBand=9, TeMono2LUTMonoBand=10, TeMono2LUT = 11
	};

	//! One of the available transformation methods 
	typedef void (TeRasterTransform::*TransformFunction)(double, double, double, double);

private:

	TeRaster* rasterIn_;			
	TeRaster* rasterOut_;

	double gain_, offset_;

	map<TeRGBChannels,short> rgbmap_;

	short mband_, mbout_;

	TransformFunction transfFuncPtr_;

	double rmin_, rmax_;

	int lutSize_;

	double contrastR_, contrastG_, contrastB_;
	double contrastM_;

	unsigned int transp_;

	std::string lutTable_;

public:

	vector<unsigned char> lutr_;
	vector<unsigned char> lutg_;
	vector<unsigned char> lutb_;

	//! Constructor 
	TeRasterTransform(TeRaster* rIn=0, TeRaster* rOut=0):  
	  rasterIn_(rIn),
	  rasterOut_(rOut),
	  gain_(1),
	  offset_(0),
	  mband_(0), 
	  mbout_(0),
	  transfFuncPtr_(&TeRasterTransform::Band2Band),
	  rmax_(TeMAXFLOAT),
	  lutSize_(0),
	  contrastR_(1.0),
	  contrastG_(1.0),
	  contrastB_(1.0),
	  contrastM_(1.0),
	  transp_(255),
	  lutTable_("")
	  { 
		  rmin_ = -1 * TeMAXFLOAT;
	 }

	//! Destructor
	~TeRasterTransform()
	{
		lutr_.clear();
		lutb_.clear();
		lutg_.clear();
		rgbmap_.clear();
		transfFuncPtr_ = 0;
	}

	//! Sets the size of the LUT being used
	void setLutSize(int n)
	{	lutSize_ = n; }

	//! Gets the size of the LUT being used
	int getLutSize()
	{	return lutSize_; }

	//! Sets the input raster
	void setRasterIn(TeRaster* rIn)
	{	rasterIn_ = rIn; }

	//! Sets the input raster
	void setRasterOut(TeRaster* rOut)
	{	rasterOut_ = rOut; }
	
	//! Sets the gain
	void setGain(double g) 
	{	gain_= g;	}

	//! Gets the gain
	double getGain() const
	{	return gain_;	}

	//! Sets the offset
	void setOffset(double o) 
	{	offset_= o;	}

	//! Gets the offset
	double getOffset() const
	{	return offset_;	}

	//! Sets the transformation method to be used 
	void setTransfFunction(TeRasterTransform::TransformFunction transfFuncPtr)
	{	transfFuncPtr_ = transfFuncPtr; }

	//! Sets the mapping from bands to R,G and B channels 
	void setRGBmap (map<TeRGBChannels,short>& rgbmap)
	{
		rgbmap_.clear();
		rgbmap_[TeREDCHANNEL] = -1;
		rgbmap_[TeGREENCHANNEL] = -1;
		rgbmap_[TeBLUECHANNEL] = -1;

		map<TeRGBChannels,short>::iterator it = rgbmap.begin();
		while (it != rgbmap.end())
		{
			if (it->first == TeREDCHANNEL)
				rgbmap_[TeREDCHANNEL] = it->second;

			else if (it->first == TeGREENCHANNEL)
				rgbmap_[TeGREENCHANNEL] = it->second;

			else if (it->first == TeBLUECHANNEL)
				rgbmap_[TeBLUECHANNEL] = it->second;
			++it;
		}
	}

	//! Sets the mapping from a particular input band to a particular output channel
	void setBChannelMapping(short bIn, TeRGBChannels bOut)
	{
		rgbmap_[bOut] = bIn;
	}

	//! Clears current mapping from bands to channel
	void clearRGBMap()
	{
		rgbmap_.clear();
	}

	//! Returns the mapping from a particular input band to a particular output channel
	map<TeRGBChannels,short>& getRGBMap()
	{
		return rgbmap_;
	}

	//! Sets the mono band to be transformed
	void setSrcBand(short n)
	{ mband_ = n; }

	//! Gets the mono band to be transformed
	short getSrcBand()
	{	return mband_; }

	//! Sets the destination of the mono band
	void setDestBand(short n)
	{ mbout_ = n; }

	//! Gets the destination of the mono band
	short getDestBand()
	{	return mbout_;	}
	
	//! Generates a LUT with nentries to display the vector of legends
	void generateLUT(TeLegendEntryVector& legs, unsigned int nentries, TeColor& backColor);

	//! Set parameters of linear transformation
	/*!
		\param vmin smallest input value
		\param vmax largest input value
		\param rmin smallest value of the output range
		\param rmax largest value of the output range
	*/
	void setLinearTransfParameters(double vmin, double vmax, double rmin, double rmax)
	{
		rmin_ = rmin;
		rmax_ = rmax;
		gain_ = (double)(rmax-rmin)/(vmax-vmin);
		offset_ = -1*gain_*vmin+rmin;
	}

//--- Simple contrast linear factors
	void setContrastR(double cR)
	{	contrastR_ = cR;	}
	void setContrastG(double cG)
	{	contrastG_ = cG;	}
	void setContrastB(double cB)
	{	contrastB_ = cB;	}
	void setContrastM(double cM)
	{	contrastM_ = cM;	}

	double getContrastR()
	{	return contrastR_;	}
	double getContrastG()
	{	return contrastG_;	}
	double getContrastB()
	{	return contrastB_;	}
	double getContrastM()
	{	return contrastM_;	}


	void setTransparency(unsigned int transp)
	{	transp_ = transp; }
	unsigned int getTransparency()
	{	return transp_;	}


//-- Lut Table
	void setLutTableName(const std::string& tableName)
	{	lutTable_ = tableName; }
	std::string getLutTableName()
	{	return lutTable_; }

//---

	//! Applies the selected transformation method
	void apply(double icol, double ilin, double ocol, double olin)
	{	(this->*transfFuncPtr_)(icol,ilin,ocol,olin); }

	//! Returns the identifier of the transformation function currently set
	TeRasterTransfFunctions getTransfFunction();

	//! Sets the associated transformation function from an identifier
	void setTransfFunction(TeRasterTransfFunctions func);

// --- The transformation functions  available ----
	//! This transformation repeats the value of the first band in input three bands of the output
	void Mono2ThreeBand(double icol, double ilin, double ocol, double olin)
	{
		double val;
		if (rasterIn_->getElement((int)icol,(int)ilin,val,mband_))
		{
			val = (val*gain_ + offset_)*contrastM_;
			if (val < rmin_)
				val = rmin_;
			else if (val > rmax_)
				val = rmax_;
			rasterOut_->setElement((int)ocol,(int)olin, val, val, val, transp_);
		}
	}

	//! This transformation repeats the value of each band in input to the same band in output
	void Band2Band(double icol, double ilin, double ocol, double olin)
	{
		double val;
		int n = rasterOut_->params().nBands();
		for (int i=0; i<n; i++)
			if (rasterIn_->getElement((int)icol,(int)ilin,val,i))
			{
				val = val*gain_ + offset_;
				if (val < rmin_)
					val = rmin_;
				else if (val > rmax_)
					val = rmax_;
				rasterOut_->setElement((int)ocol,(int)olin,val,i);
			}
	}

	//! This transformation repeats the value of the raster LUT in input to the first 3 bands of output
	void Pallete2ThreeBand(double icol, double ilin, double ocol, double olin)
	{
		double val;
		if (rasterIn_->getElement((int)icol,(int)ilin,val))
		{
			if (val>= 0 && (unsigned int)val < rasterIn_->params().lutr_.size())
			{
				rasterOut_->setElement((int)ocol,(int)olin,rasterIn_->params().lutr_[(int)val],
														   rasterIn_->params().lutg_[(int)val],
														   rasterIn_->params().lutb_[(int)val], transp_);
			}
		}
	}

	//! This transformation repeats the value of the first band in input three bands of the output
	void ThreeBand2RGB(double icol, double ilin, double ocol, double olin)
	{
		double valR, valG, valB;
		bool flag = rasterIn_->getElement((int)icol,(int)ilin,valR,0);
		flag = rasterIn_->getElement((int)icol,(int)ilin,valG,1) || flag;
		flag = rasterIn_->getElement((int)icol,(int)ilin,valB,2) || flag;

		if (flag)
			rasterOut_->setElement((int)ocol,(int)olin, (valR*gain_ + offset_)*contrastR_, 
														(valG*gain_ + offset_)*contrastG_, 
														(valB*gain_ + offset_)*contrastB_, transp_);
	}

	//! This transformation repeats the value of an external LUT to the first 3 bands of output
	void LUT2ThreeBand(double icol, double ilin, double ocol, double olin)
	{
		double idx, val; 
		if (rasterIn_->getElement((int)icol,(int)ilin,val,mband_))
		{
			idx = (val*gain_) + offset_;
			if (idx >= 0 && idx < lutSize_)
			{
				rasterOut_->setElement((int)ocol,(int)olin,lutr_[(int)idx]*contrastR_,
														   lutg_[(int)idx]*contrastG_,
														   lutb_[(int)idx]*contrastB_, transp_);
			}
		}
	}

	
	void ThreeBand2LUTThreeBand(double icol, double ilin, double ocol, double olin)
	{
		double idxR, valR;
		double idxG, valG;
		double idxB, valB;
		double outR = 0.;
		double outG = 0.;
		double outB = 0.;

		if(rgbmap_.empty())
		{
			return;
		}

		bool flag = rasterIn_->getElement((int)icol,(int)ilin,valR,rgbmap_[TeREDCHANNEL]);

		idxR = (valR*gain_) + offset_;
		if (idxR >= 0 && idxR < lutSize_)
		{
			outR = lutr_[(int)idxR]*contrastR_;
		}

		flag = rasterIn_->getElement((int)icol,(int)ilin,valG,rgbmap_[TeGREENCHANNEL]) || flag;

		idxG = (valG*gain_) + offset_;
		if (idxG >= 0 && idxG < lutSize_)
		{
			outG = lutg_[(int)idxG]*contrastG_;
		}

		flag = rasterIn_->getElement((int)icol,(int)ilin,valB,rgbmap_[TeBLUECHANNEL]) || flag;

		idxB = (valB*gain_) + offset_;
		if (idxB >= 0 && idxB < lutSize_)
		{
			outB = lutb_[(int)idxB]*contrastB_;
		}

		if (flag)
			rasterOut_->setElement((int)ocol,(int)olin, outR, outG, outB, transp_);

	}

	void MonoBand2LUTMonoBand(double icol, double ilin, double ocol, double olin)
	{
		double idxMono, valMono;
		double outMono = 0.;

		bool flag = rasterIn_->getElement((int)icol,(int)ilin,valMono,mband_);

		if(flag)
		{
			idxMono = (valMono*gain_) + offset_;
			if (idxMono >= 0 && idxMono < lutSize_)
				outMono = lutr_[(int)idxMono]*contrastM_;

			if(rasterOut_->params().nBands() == 1)
				rasterOut_->setElement((int)ocol,(int)olin, outMono);
			else
				rasterOut_->setElement((int)ocol,(int)olin, outMono, outMono, outMono, transp_);
		}
	}

	// This transformation is used to define a particular mapping from input bands to RGB channels
	void ExtractRGB(double icol, double ilin, double ocol, double olin)
	{
		double valR, valG, valB;
		bool flag = rasterIn_->getElement((int)icol,(int)ilin,valR,rgbmap_[TeREDCHANNEL]);
		     flag = rasterIn_->getElement((int)icol,(int)ilin,valG,rgbmap_[TeGREENCHANNEL]) || flag;
		     flag = rasterIn_->getElement((int)icol,(int)ilin,valB,rgbmap_[TeBLUECHANNEL]) || flag;

		if (flag)
			rasterOut_->setElement((int)ocol,(int)olin, (valR*gain_ + offset_)*contrastR_, 
														(valG*gain_ + offset_)*contrastG_, 
														(valB*gain_ + offset_)*contrastB_, transp_);
	}

	//! This transformation repeats the value of a particular band in input to a particular band of the output
	void ExtractBand(double icol, double ilin, double ocol, double olin)
	{
		double val;
		if (rasterIn_->getElement((int)icol,(int)ilin,val,mband_))
		{
			val = val*gain_ + offset_;
			if (val < rmin_)
				val = rmin_;
			else if (val > rmax_)
				val = rmax_;
			rasterOut_->setElement((int)ocol,(int)olin,val*contrastM_,mbout_);
		}
	}

	// This transformation extracts selected bands of the input raster and 
	// writes each to a particular band of the output
	void ExtractBands(double icol, double ilin, double ocol, double olin)
	{
		double val;
		int bOut;
		if (rgbmap_[TeREDCHANNEL] != -1)
		{
			bOut = 0;
			if (rasterIn_->getElement((int)icol,(int)ilin,val,rgbmap_[TeREDCHANNEL]))
				rasterOut_->setElement((int)ocol,(int)olin,val*contrastR_,bOut);
		}
		if (rgbmap_[TeGREENCHANNEL] != -1)
		{
			bOut = 1;
			if (rasterIn_->getElement((int)icol,(int)ilin,val,rgbmap_[TeGREENCHANNEL]))
				rasterOut_->setElement((int)ocol,(int)olin,val*contrastG_,bOut);
		}
		if (rgbmap_[TeBLUECHANNEL] != -1)
		{
			bOut = 2;
			if (rasterIn_->getElement((int)icol,(int)ilin,val,rgbmap_[TeBLUECHANNEL]))
				rasterOut_->setElement((int)ocol,(int)olin,val*contrastB_,bOut);
		}
	}

	void Mono2LUT(double icol, double ilin, double ocol, double olin)
	{
		double val; 
		int idx = -1;
		if (rasterIn_->getElement((int)icol,(int)ilin,val,mband_))
		{
			//idx = (val*gain_) + offset_;
			for (size_t i = 0; i < range_.size(); i++)
			{
				double atI = range_[i];
				if ( val <= (atI+0.001) )
				{
					idx = i;
					break;
				}

			}
			if (idx >= 0 && idx < lutSize_)
			{
				rasterOut_->setElement((int)ocol,(int)olin,lutr_[(int)idx]*contrastR_,
					lutg_[(int)idx]*contrastG_,
					lutb_[(int)idx]*contrastB_, transp_);
			}
		}
	}

	void setLegend(TeLegendEntryVector legend)
	{
		legend_ = legend;

		range_.clear();
		lutr_.clear();
		lutg_.clear();
		lutb_.clear();	

		for (size_t n=0; n <legend_.size(); n++)
		{
			range_.push_back( atof(legend_[n].slice().from_.c_str()) );
			lutr_.push_back(legend_[n].getVisualMap()[TePOLYGONS]->color().red_);
			lutg_.push_back(legend_[n].getVisualMap()[TePOLYGONS]->color().green_);
			lutb_.push_back(legend_[n].getVisualMap()[TePOLYGONS]->color().blue_);
		}
		lutSize_ = lutr_.size();
	}

	private:
		TeLegendEntryVector legend_;
		std::vector<double> range_;

};
#endif


