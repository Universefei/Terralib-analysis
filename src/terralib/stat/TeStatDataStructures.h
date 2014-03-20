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
/*! \file TeStatDataStructures.h
    \brief This file deals with special data structures for spatial statistics 
*/
#ifndef __TERRALIB_INTERNAL_STATDATASTRUCTURES_H
#define __TERRALIB_INTERNAL_STATDATASTRUCTURES_H

#include "TeStatDefines.h"
#include "../kernel/TeProjection.h"
#include "../kernel/TeRaster.h"
#include "../kernel/TeAttribute.h"
#include "../kernel/TeCoord2D.h"
#include "../kernel/TeSTInstance.h"

class TeSTElementSet;

class STAT_DLL TeGridElement
{
	TeRaster* parent_;

public:
	int col_;
	int lin_;

	TeGridElement(TeRaster* rst=0, int col=0, int lin=0):
	parent_(rst),
	col_(col),
	lin_(lin)
	{}

	void setColLin(int col, int lin)
	{ 	col_ = col; lin_ = lin; 	}

	void setRaster(TeRaster* rst)
	{	parent_ = rst; }

	void addProperty(TeProperty propK)
	{	
		double val = atof(propK.value_.c_str());
		parent_->setElement(col_,lin_,val);
	}

	bool getDoubleProperty(int idx, double& value) 
	{	return parent_->getElement(col_, lin_, value, idx);	}

	bool setDoubleProperty(int idx , double value) 
	{	return parent_->setElement(col_,lin_, value, idx);	}

	bool getPropertyValue(const string& /* name */, string& value) 
	{
		double val;
		if (parent_->getElement(col_,lin_,val)) 
		{
			value = Te2String(val,12);
			return true;
		}
		value = Te2String(-1.0);
		return false;
	}

	bool setPropertyValue(const string& /* name */, const string&  value) 
	{
		double val;
		val = atof(value.c_str());
		return parent_->setElement(col_,lin_,val);
	}

	bool centroid(TeCoord2D& loc)
	{
		TeCoord2D pt(col_,lin_);
		loc = parent_->index2Coord(pt);
		return true;
	}

	bool area(double& a)
	{
		a = parent_->params().resx_ * parent_->params().resy_;
		return true;
	}
};
//!  A grid support to generate a map of kernel
/*! 
	 This is a wrapper around TeRaster class in order to use  it as
	 the output of a kernel algorithm. It provides iterators and  methods
	 so that Kernel Map algorithms can use it in the same way as when using 
	 a set of polygonal regions as support.
*/
class STAT_DLL TeKernelGridSupport
{
	TeRaster* rasterImp_;
public:

	TeKernelGridSupport(TeBox& bb, int ncols, TeProjection* proj, unsigned int nBands = 1)
	{
		TeRasterParams par;
		par.decoderIdentifier_=  "MEM";
		par.fileName_ = "rasterTemp";
		rasterImp_ =0;

		double resx = bb.width()/ncols;
		TeBox newBox = adjustToCut(bb,resx,resx);
		par.boundingBoxResolution(newBox.x1_,newBox.y1_,newBox.x2_,newBox.y2_,resx,resx);
		newBox = par.boundingBox();
		par.nBands(nBands);
		par.projection(proj);
		par.setDataType(TeDOUBLE);
		par.setDummy(-TeMAXFLOAT);
		par.setPhotometric(TeRasterParams::TeMultiBand);
		par.useDummy_ = true;
		par.mode_ = 'c';

		rasterImp_ = new TeRaster(par);
		if(!rasterImp_)
			rasterImp_ = 0;

		if (!rasterImp_->init())
		{
			delete rasterImp_;
			rasterImp_ = 0;
		}
	}


	~TeKernelGridSupport()
	{
		if (rasterImp_)
		{
			delete rasterImp_;
			rasterImp_ = 0;
		}
	}

	bool status()
	{	return (rasterImp_ && rasterImp_->status()); }

	double totalArea() 
	{ return TeGeometryArea(rasterImp_->params().boundingBox()); }

 	int numObjects() 
	{
		TeRasterParams par;
		par = rasterImp_->params();
		return par.ncols_ * par.nlines_;
	}

	//-----
	class iterator
	{
		friend class TeKernelGridSupport;

	protected:
		TeRaster::iterator  curItRaster_; //guarda iterador que pode ser de poligono ou nao
		TeGridElement elem_;

	public:
		iterator(TeRaster* rst=0): curItRaster_(0)
		{ elem_.setRaster(rst); }

		~iterator() {}

		iterator(TeRaster::iterator itr, TeRaster* rst=0) :
			curItRaster_(itr)
		{ 
			elem_.setRaster(rst);
			elem_.setColLin(curItRaster_.currentColumn(), curItRaster_.currentLine());
		}

		iterator& operator++()
		{
			//curItRaster_->operator++();
			curItRaster_.moveForward();
			elem_.setColLin(curItRaster_.currentColumn(), curItRaster_.currentLine());
			return *this;
		}

		bool operator==(const iterator& rhs) const
		{ return ((this->curItRaster_) ==   (rhs.curItRaster_)); }

		bool operator!=(const iterator& rhs) const
		{ return !((this->curItRaster_) ==   (rhs.curItRaster_));}

		iterator& operator=(const iterator& other)
		{
			if ( this != &other )
			{
				curItRaster_ = other.curItRaster_;
				elem_ = other.elem_;
			}
			return *this;
		}

		TeGridElement& operator*() { return elem_; }

		TeGridElement& operator->() { return elem_; }
	};

  	//-----
	class iteratorPoly
	{
		friend class TeKernelGridSupport;

	protected:
    TeRaster::iteratorPoly curItRaster_; //guarda iterador que pode ser de poligono ou nao
		TeGridElement elem_;

	public:
		iteratorPoly(TeRaster* rst=0): curItRaster_(0)
		{ elem_.setRaster(rst); }

        ~iteratorPoly() {}

		iteratorPoly(TeRaster::iteratorPoly itr, TeRaster* rst=0) :
			curItRaster_(itr)
		{ 
			elem_.setRaster(rst);
			elem_.setColLin(curItRaster_.currentColumn(), curItRaster_.currentLine());
		}

		iteratorPoly& operator++()
		{
			//curItRaster_->operator++();
			curItRaster_.moveForward();
			elem_.setColLin(curItRaster_.currentColumn(), curItRaster_.currentLine());
			return *this;
		}

		bool operator==(const iteratorPoly& rhs) const
		{	return ((this->curItRaster_) ==   (rhs.curItRaster_)); }

		bool operator!=(const iteratorPoly& rhs) const
		{ return !((this->curItRaster_) ==   (rhs.curItRaster_)); }

		iteratorPoly& operator=(const iteratorPoly& other)
		{
			if ( this != &other )
			{
				curItRaster_ = other.curItRaster_;
				elem_ = other.elem_;
			}
			return *this;
		}

		TeGridElement& operator*() { return elem_; }

		TeGridElement& operator->() { return elem_; }
	};


	//-----

	TeRaster* raster()
	{	return rasterImp_; }

	iterator begin()
	{
		TeRasterParams par = rasterImp_->params();
		TeRaster::iterator itbegin(0,0,par.ncols_,par.nlines_,par.nBands(),rasterImp_);
		TeKernelGridSupport::iterator it(itbegin,rasterImp_);
		return it;
	}

	iteratorPoly begin(TePolygon poly)
	{
		TeRaster::iteratorPoly itRasterBegin_ = rasterImp_->begin(poly,TeBBoxPixelInters);
		TeKernelGridSupport::iteratorPoly it(itRasterBegin_, rasterImp_);
		return it;
	}

	iterator end()
	{
		TeRasterParams par = rasterImp_->params();
		TeRaster::iterator itend =  TeRaster::iterator(par.ncols_,par.nlines_-1,par.ncols_,par.nlines_,par.nBands(),rasterImp_);
		TeKernelGridSupport::iterator it(itend,rasterImp_);
		return it;
	}

	iteratorPoly end(TePolygon poly)
	{
		TeRaster::iteratorPoly itRasterEnd_ =  rasterImp_->end(poly,TeBBoxPixelInters);
		TeKernelGridSupport::iteratorPoly it(itRasterEnd_,rasterImp_);
		return it;
	}
};


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//! An instance in time of a spatial object, with specialized members for TerraStat
class STAT_DLL TeSTStatInstance : public  TeSTInstance
{

  protected:

	vector<double> dblProperties_;

	//! inicialize the vector of double
	void initDbl(int n) 
	{
		for(int i=0; i<n; ++i)
			dblProperties_.push_back (0.0);
	}

  public:
    
	//! Constructor
	TeSTStatInstance(int nDlbProp = 0); 

    //! Constructor
    TeSTStatInstance(TeSTInstance& other, int nd, int ad); 
    
	//! Copy constructor
    TeSTStatInstance(const TeSTStatInstance& other);     

	//! Destructor
    virtual ~TeSTStatInstance()
	{ }

	//! Assign operator
	TeSTStatInstance& operator=(const TeSTStatInstance& other);

    bool setDoubleProperty(unsigned int idx, double value) 
	{
		if (idx >= dblProperties_.size())
        	return false;
		dblProperties_[idx] = value;
		return true;
    }

    bool getDoubleProperty(unsigned int idx, double& value) 
	{
		if (idx >= dblProperties_.size())
			return false;
		value = dblProperties_[idx];
		return true;
    }

};
 
class STAT_DLL TeSTStatInstanceSet  
{
  
  
  protected:
    
    // map of object identification to its instances in time
    vector<TeSTStatInstance >	objs_;	
    
  public:
    
   //! Constructor 
    TeSTStatInstanceSet() 
	{}

	//! Destructor
	virtual ~TeSTStatInstanceSet()
	{}

	//! Copy constructor
    TeSTStatInstanceSet(const TeSTStatInstanceSet& other)
	{
		this->objs_ = other.objs_;
	}

	//! Assign operator
	virtual TeSTStatInstanceSet& operator=(const TeSTStatInstanceSet& other);

    
    void Copy(TeSTElementSet& stoSet, int nDbl=0, int aDbl = 0); 
    
    int numObjects(); 
    
    class iterator {
      vector<TeSTStatInstance >::iterator it_;
      
    public:
      iterator(vector<TeSTStatInstance >::iterator i){it_ = i;}
      
      ~iterator(){}
      
      iterator& operator++()
      {
        ++it_;
        return *this;
      }
      
      bool operator==(const iterator& rhs) const
      {
        return ((this->it_) ==   (rhs.it_));
      }
      
      bool operator!=(const iterator& rhs) const
      {
        return !((this->it_) ==   (rhs.it_));
      }
      
      iterator& operator=(const iterator& other)
      {
        if ( this != &other )
        {
          it_ = other.it_;
        }
        return *this;
      }
      
      TeSTStatInstance& operator*() { return *(it_); }
    };
    
    iterator begin() {  
      return TeSTStatInstanceSet::iterator(objs_.begin());
    }
    
    iterator end() {
      return TeSTStatInstanceSet::iterator(objs_.end());
    }
    
};

// Classe de eventos pontuais
class STAT_DLL TePointTInstanceSet : public TeSTStatInstanceSet
{

  public:

	TePointTInstanceSet() : TeSTStatInstanceSet() 
	{}

  //! Destructor
	~TePointTInstanceSet()
	{}

  
  iterator begin(const TeCoord2D& /* center */, const double& /* radius */)
  {
    return TeSTStatInstanceSet::begin();
  }

  iterator end(const TeCoord2D& /* center */, const double& /* radius */)
  {
    return TeSTStatInstanceSet::end();
  }

  iterator begin()
  {
    return TeSTStatInstanceSet::begin();
  }

  iterator end()
  {
    return TeSTStatInstanceSet::end();
  }
};

#endif
