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

/*! \file TeComputeAttributeStrategies.h
    This file contains functions to compute attributes based on 
	alternative strategies. These strategies (Strategy Design Pattern)
	correspond to operations to derive information based on other  layers attributes and/or
	spatial components (polygonal, raster, point, raster or other cells). 
*/
#ifndef TeComputeAttrStrategies_H
#define TeComputeAttrStrategies_H

#include "TeAttribute.h"
#include "TeBox.h"
#include "TeUtils.h"
#include "TeNetwork.h"
#include "TeKdTree.h"
#include "TeLayer.h"

typedef TeSAM::TeAdaptativeKdTreeNode<TeCoord2D, vector<TePoint>, TePoint> KDNODE;
typedef TeSAM::TeAdaptativeKdTree<KDNODE> KDTREE;

class TeTheme;
class TeRaster;
class TeQuerier;

/** @defgroup NSCellFillStrategies Non-spatial strategy hierarchy to compute cells attributes
    @ingroup FillCellAlg
	Abstract class in the strategies hierarchy for computing attribute values based on other layers 
	(Strategy Design Pattern). 
	@{
 */
//!  Abstract class in the strategies hierarchy for computing attribute values based on other layers (Strategy Design Pattern). 
template<class Iterator>
class TeComputeAttrStrategy  
{
	public:
		//! Function to access the default value
		TeProperty  defaultValue () { return defaultValue_; }

		//! Virtual function to compute the attribute based on the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator /*first*/, Iterator /*last*/, const string& /*columnName*/)
		{
			vector<TeProperty> result;
			result.push_back (defaultValue_);
			return result;
		}

		//! Destructor
		virtual ~TeComputeAttrStrategy(){}

		bool ComputeAttrIsDummy(double value, double dummy)
		{
			return (ABS(value - dummy) < 0.001);
		}


	protected:
		//! Empty constructor
		TeComputeAttrStrategy () {};

		//! The default returning value
		TeProperty	defaultValue_;
};


//! Class to compute the average given some elements of an iterator
template<class Iterator>
class TeAverageStrategy: public TeComputeAttrStrategy<Iterator>  
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TeAverageStrategy (double dummyV = 0.0, double defaultV = 0.0) {this->defaultValue_.value_ = Te2String (defaultV); dummy = dummyV; }

		//! Destructor
		virtual ~TeAverageStrategy() {}

		//! Function to compute the average based on the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			double	tot_val = 0.0;
			int		num = 0;

			Iterator it = first;
			while (it != last)
			{
				if( !ComputeAttrIsDummy((*it), dummy) )
				{
					tot_val += (*it);
					num++;
				}
				++it;
			}
	
			TeProperty  prop;
			prop.attr_.rep_.name_ = columnName;
			prop.attr_.rep_.type_ = TeREAL;
			prop.attr_.rep_.numChar_ = 48;
			if (num > 0)
			{
				double val = tot_val/num;
				prop.value_ = Te2String (val);
			}
			else prop.value_ = this->defaultValue_.value_;
	
			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}
	private:
		double dummy;
};


//! Class to compute the average given some elements of an iterator
template<class Iterator>
class TeSTDevStrategy: public TeComputeAttrStrategy<Iterator>  
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TeSTDevStrategy (double dummyV = 0.0, double defaultV = 0.0) {this->defaultValue_.value_ = Te2String (defaultV); dummy = dummyV; }

		//! Destructor
		virtual ~TeSTDevStrategy() {}

		//! Function to compute the average based on the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			double tot_val = 0.0;
			double stdev = 0.0;
			int	   num = 0;
			
			Iterator it = first;
			while (it != last)
			{
				if( !ComputeAttrIsDummy((*it), dummy) )
				{
					tot_val += (*it);
					num++;
				}
				++it;
			}
			if(num > 0)
			{
				double mean = tot_val / num;
				it = first;
				while (it != last)
				{
					if( !ComputeAttrIsDummy((*it), dummy) ) stdev += pow((*it) - mean, 2);

					++it;
				}
				stdev = stdev / num;
				stdev = sqrt(stdev);
			}

			TeProperty  prop;
			prop.attr_.rep_.name_ = columnName;
			prop.attr_.rep_.type_ = TeREAL;
			prop.attr_.rep_.numChar_ = 48;

			if (num > 0) prop.value_ = Te2String (stdev);
			else         prop.value_ = this->defaultValue_.value_;
	
			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}	
	private:
		double dummy;
};


//! Class to compute the sum of the elements of an iterator
template<class Iterator>
class TeSumStrategy: public TeComputeAttrStrategy<Iterator>   
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TeSumStrategy (double dummyV = 0.0, double defaultV = 0.0) {this->defaultValue_.value_ = Te2String (defaultV); dummy = dummyV; }

		//! Destructor
		virtual ~TeSumStrategy() {}

		//! Function to compute the sum of the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{  
			double sum = 0.0;
			Iterator it = first;
			bool found = false;
			while (it != last)
			{
				if( !ComputeAttrIsDummy((*it), dummy) )
				{
					sum += (*it);
					found = true;
				}
				++it;
			}

			TeProperty  prop;

			if (first == last || !found) prop = this->defaultValue_;
			else  prop.value_ = Te2String (sum);

			prop.attr_.rep_.name_ = columnName;
			prop.attr_.rep_.type_ = TeREAL;
			prop.attr_.rep_.numChar_ = 48;
			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}
	private:
		double dummy;
};


//! Class to compute the minimum value of an iterator
template<class Iterator>
class TeMinimumStrategy: public TeComputeAttrStrategy<Iterator>   
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TeMinimumStrategy (double dummyV = 0.0, double defaultV = 0.0) {this->defaultValue_.value_ = Te2String (defaultV); dummy = dummyV; }

		//! Destructor
		virtual ~TeMinimumStrategy() {}

		//! Function to compute the minimum value of the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			double min = TeMAXFLOAT;
			Iterator it = first;
			while (it != last)
			{
				if( !ComputeAttrIsDummy((*it), dummy) )
				{
					double val = (*it);
					if (val < min) min = val;
				}
				++it;
			}

			TeProperty  prop;
			prop.attr_.rep_.name_ = columnName;
			prop.attr_.rep_.type_ = TeREAL;
			prop.attr_.rep_.numChar_ = 48;

			if (first == last || min == TeMAXFLOAT)
				prop.value_ = this->defaultValue_.value_; 
			else  
				prop.value_ = Te2String (min);

			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}
	private:
		double dummy;
};


//! Class to compute the maximum value of an iterator
template<class Iterator>
class TeMaximumStrategy: public TeComputeAttrStrategy<Iterator>   
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TeMaximumStrategy (double dummyV = 0.0, double defaultV = 0.0) {this->defaultValue_.value_ = Te2String (defaultV); dummy = dummyV; }

		//! Destructor
		virtual ~TeMaximumStrategy() {}

		//! Function to compute the maximum value of the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			double	max =  TeMINFLOAT;
			Iterator it = first;
			while (it != last)
			{
				if( !ComputeAttrIsDummy((*it), dummy) )
				{
					double val = (*it);
					if (val > max) max = val;
				}
				++it;
			}

			TeProperty  prop;
			prop.attr_.rep_.name_ = columnName;
			prop.attr_.rep_.type_ = TeREAL;
			prop.attr_.rep_.numChar_ = 48;

			if (first == last || max == TeMINFLOAT)
				prop.value_ = this->defaultValue_.value_; 
			else  
				prop.value_ = Te2String (max);

			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}
	private:
		double dummy;
};


//! Class to compute the percentages of each class (described in a map) of an iterator
template<class Iterator>
class TeCategoryPercentageStrategy: public TeComputeAttrStrategy<Iterator>   
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
			\param classes			classes for regrouping the values	
		*/	
		TeCategoryPercentageStrategy (map<string, string>& classes, double defaultValue = 0.0) 
		{
			this->defaultValue_.value_ = Te2String (defaultValue); 
			classesMap_ = classes;
			usingMap = true;
		}

		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
			\param dummyV			the ignored value
		*/	

		TeCategoryPercentageStrategy(double defaultValue = 0.0, string dummyV = "")
		{
			usingMap = false;
			this->defaultValue_.value_ = Te2String (defaultValue); 
			dummy = dummyV;
		}

		//! Destructor
		virtual ~TeCategoryPercentageStrategy() {}

		//! Function to compute the percentages based on the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			TeProperty category;
			int num = 0;

			// initialize count
			map<string, int>  count;

			if(usingMap)
			{
				map<string, string>::iterator itMap = classesMap_.begin();
				while (itMap != classesMap_.end())
				{
					count[(*itMap).second] = 0;
					++itMap;
				}

				Iterator it = first;
				while (it != last)
				{
					if (it.getProperty (category))
					{
						count[classesMap_[category.value_]]++;
						num++;
					}
					++it;
				}
			}
			else // use the data to initialize the counter
			{
				Iterator it = first;
				while (it != last)
				{
					if (it.getProperty (category) && category.value_ != dummy)
						count[category.value_] = 0;

					++it;
				}

				it = first;
				while (it != last)
				{
					if (it.getProperty (category) && category.value_ != dummy)
					{
						count[category.value_]++;
						num++;
					}
					++it;
				}
			}

			TePropertyVector  result;
			map <string, int>:: iterator count_it = count.begin();

			while(count_it != count.end())  
			{
				string value_ =  (*count_it).first;
				for (unsigned i = 0; i < value_.size(); i++)
					if(value_[i] == '.' || value_[i] == '+' || value_[i] == '-')
						// remove them from value because they cant be part of a column name
						value_[i] = '_';

				TeProperty prop;
				prop.attr_.rep_.name_ = columnName + value_;
				prop.attr_.rep_.type_ = TeREAL;
				prop.attr_.rep_.numChar_ = 48;

				double percent = 0.0;
				double value = (double)(*count_it).second;
				if (num != 0) percent = value/num;
				prop.value_    = Te2String (percent);
				result.push_back (prop);
				++count_it;
			}
			return result;
		}

	private:
		//! Map of classes
		map<string, string> classesMap_;
		bool usingMap;
		string dummy;
};


//! Class to compute if there is any value in an iterator
template<class Iterator>
class TePresenceStrategy: public TeComputeAttrStrategy<Iterator>
{
	public:
		//! Constructor
		/* 
			\param defaultValue		the returned value of the compute method when the iterator is empty	
		*/	
		TePresenceStrategy (bool defaultValue = 0) 
		{
			this->defaultValue_.attr_.rep_.type_ = TeINT;
			this->defaultValue_.value_ = Te2String (defaultValue); 
			this->defaultValue_.attr_.rep_.numChar_ = 48;
		}

		//! Destructor
		virtual ~TePresenceStrategy() {}

		//! Function to compute the presence based on the iterator
		//! It has value "1" if there is any element, otherwise it has the value of the constructor
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			TeProperty prop;
			if (first != last) 
				prop.value_ = "1";
			else
				prop = this->defaultValue_;
	
			TePropertyVector  result;
			prop.attr_.rep_.name_ = columnName; 
			prop.attr_.rep_.type_ = TeINT;
			prop.attr_.rep_.numChar_ = 48;
			result.push_back (prop);
			return result;
		}
};

//! Class for computing the majority in a given iterator of values without classes
template<class Iterator>
class TeMajorityStrategy: public TeComputeAttrStrategy<Iterator>
{
	public:
		//! Constructor
		/* 
			\param defaultType		the type of the default value	
		*/	
		TeMajorityStrategy (TeAttrDataType defaultType = TeINT, string defaultValue = "", string dummyV = "")
		{
			this->defaultValue_.attr_.rep_.type_ = defaultType;
			this->defaultValue_.value_ = defaultValue; 
			dummy = dummyV;
		}

		//! Destructor
		virtual ~TeMajorityStrategy() {}

		//! Function to compute the majority of the iterator
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual  vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			TeProperty category;
			category.attr_.rep_.type_ = this->defaultValue_.attr_.rep_.type_;
	
			map<string, int>  count;
			Iterator it = first;
			while (it != last)
			{
				if (it.getProperty (category) && category.value_ != dummy)
					count[category.value_]++;

				++it;
			}
	
			TeProperty prop;
			prop.attr_.rep_.type_ = category.attr_.rep_.type_;
			prop.value_ = this->defaultValue_.value_;
			prop.attr_.rep_.name_ = columnName; 
			prop.attr_.rep_.numChar_ = 48;

			int max = 0;
			map <string, int>:: iterator count_it = count.begin();
			while(count_it != count.end())  
			{
				if ((*count_it).second > max)
				{
					prop.value_ = (*count_it).first;
					max = (*count_it).second;
				}
				++count_it;		
			}
			TePropertyVector  result;
			result.push_back (prop);
			return result;
		}
	private:
		string dummy;
};


//! Class for compute the category with more occurences in the elements of a given iterator, using classes
template<class Iterator>
class TeMajorityCategoryStrategy: public TeComputeAttrStrategy<Iterator>   
{
	public:
		//! Constructor
		/*
			\param classesMap		the returning values for each categoty
			\param defaultValue		the type of the attribute to be created
		*/	
		TeMajorityCategoryStrategy (const map<string, string>& classesMap, TeAttrDataType defaultType = TeINT) 
		{
			this->defaultValue_.attr_.rep_.type_ = defaultType;
			this->defaultValue_.value_ = "0"; 
			classesMap_ = classesMap;
		}

		//! Destructor
		virtual ~TeMajorityCategoryStrategy() {}

		//! Function to compute the category with more occurrences
		/* 
			\param first		the first element of the Iterator
			\param last			the last element of the Iterator
			\param columnName	the attribute name to be generated in the TeProperty's
		*/
		virtual vector<TeProperty> compute (Iterator first, Iterator last, const string& columnName)
		{
			TeProperty category;
			category.attr_.rep_.type_ = this->defaultValue_.attr_.rep_.type_;

			// initialize count
			map<string, int>  count;
			map<string, string>::iterator itMap = classesMap_.begin();
			while (itMap != classesMap_.end())
			{
				count[(*itMap).second] = 0;
				++itMap;
			}

			Iterator it = first;
			while (it != last)
			{
				if (it.getProperty (category))
				{
					count[classesMap_[category.value_]]++;
				}
				++it;
			}

			TePropertyVector  result;
			TeProperty prop;
			prop.attr_.rep_.type_ = TeSTRING;
			//	prop.value_ = defaultValue_.value_;
			prop.attr_.rep_.name_ = columnName; 
			prop.attr_.rep_.numChar_ = 48;

			int max = 0;
			map <string, int>:: iterator count_it = count.begin();
			while(count_it != count.end())  
			{
				if ((*count_it).second > max)
				{
					prop.value_ = (*count_it).first;
					max = (*count_it).second;
				}
				++count_it;		
			}

			result.push_back (prop);
			return result;
		}

	protected:
			map<string, string> classesMap_;
};
/** @} */   



/** @defgroup CellFillStrategies Spatial strategy hierarchy to compute cells attributes
    @ingroup FillCellAlg
	Abstract class in the strategies hierarchy for computing attribute values based on other layers 
	(Strategy Design Pattern). 
	@{
 */
//! Virtual class for computing attributes based on the spatial characteristics of the objects (Strategy Design Pattern)
class TL_DLL TeComputeSpatialStrategy  
{
	public:
		//! Destructor
		virtual ~TeComputeSpatialStrategy() {}

		//! Virtual function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute(TeBox box) = 0;

		//! The type of attriute generated by the class. Default is TeREAL
		virtual TeAttrDataType Type() { return TeREAL; }

		//! Return all the geometries within a given box
		/* 
			\param box		the box to be used
			\patam att		the attribute returned together with the objects. If none, this function returns only the geometries
		*/
		TeQuerier* getWithinGeometry(TeBox box, string att = "");

	protected:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param rep_			the representation to be used	
		*/	
		TeComputeSpatialStrategy (TeTheme* theme_, TeGeomRep rep_) { theme = theme_; rep = rep_; }

		//! Theme used to compute the attributes
		TeTheme* theme;

		//! The geometry of the theme used
		TeGeomRep rep;
};


//! Class to compute the average of elements weighted by their areas
//! It is useful when working with values representing averages
class TL_DLL TeAverageWeighByAreaStrategy: public TeComputeSpatialStrategy 
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param att_			the attribute used to compute the sum. It must be numerical
			\param rep_			the representation to be used	
		*/	
		TeAverageWeighByAreaStrategy  (TeTheme* theme_, string att_, TeGeomRep rep_) : TeComputeSpatialStrategy(theme_, rep_) { attribute = att_; }

		//! Destructor
		virtual ~TeAverageWeighByAreaStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);

	private:
		//! The attribute used to calculate the average
		string attribute;
};


//! Class to compute the sum of elements weighted by their areas
//! This function distribute the values keeping the overall sum of the elements on the layer
class TL_DLL TeSumWeighByAreaStrategy: public TeComputeSpatialStrategy  
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param att_			the attribute used to compute the sum. It must be numerical
			\param rep_			the representation to be used	
		*/	
		TeSumWeighByAreaStrategy (TeTheme* theme_, string att_, TeGeomRep rep_) : TeComputeSpatialStrategy(theme_, rep_) { attribute = att_; }

		//! Destructor
		virtual ~TeSumWeighByAreaStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual  string compute (TeBox box);

	private:
		//! The attribute used to calculate the average
		string attribute;
};


//! Class to compute the value that occurs more frequently in a given box 
class TL_DLL TeCategoryMajorityStrategy: public TeComputeSpatialStrategy  
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param att_			the attribute used to compute the majority
			\param classesMap	a map to reclassify the values
		*/	
		TeCategoryMajorityStrategy (TeTheme* theme_, string att_, map<string, string>& classesMap) : TeComputeSpatialStrategy(theme_, TePOLYGONS) 
        	{classesMap = classesMap; attribute = att_;}

		//! Destructor
		virtual ~TeCategoryMajorityStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual  string compute (TeBox box);	

	protected:
			//! The map of values
			map<string, string> classesMap;

			//! The attribute used in the majority
			string attribute;
};


//! Class to count the elements in a given box 
//! This class returns the number of elements whose BOX intercepts the box in the argument of compute
//! For safety, use the count classes that inherit this one
class TL_DLL TeCountObjectsStrategy: public TeComputeSpatialStrategy  
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param geom_		the representation to be used	
		*/	
		TeCountObjectsStrategy (TeTheme* theme_, TeGeomRep geom_) : TeComputeSpatialStrategy(theme_, geom_) { }

		//! Destructor
		virtual ~TeCountObjectsStrategy() { }

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);

		//! This class generates TeINT attributes
		virtual TeAttrDataType Type() { return TeINT; }
};


//! Class to compute the number of polygons in a given box
class TL_DLL TeCountPolygonalObjectsStrategy: public TeCountObjectsStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeCountPolygonalObjectsStrategy (TeTheme* theme_) : TeCountObjectsStrategy(theme_, TePOLYGONS) {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);
};


//! Class to compute the number of lines in a given box
class TL_DLL TeCountLineObjectsStrategy: public TeCountObjectsStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeCountLineObjectsStrategy (TeTheme* theme_) : TeCountObjectsStrategy(theme_, TeLINES) {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);
};


//! Class to count the number of points in a given box
class TL_DLL TeCountPointObjectsStrategy: public TeCountObjectsStrategy 
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeCountPointObjectsStrategy (TeTheme* theme_) : TeCountObjectsStrategy(theme_, TePOINTS) {}
};


//! Class to compute the length of lines within a given box 
class TL_DLL TeLineLengthStrategy: public TeComputeSpatialStrategy 
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeLineLengthStrategy (TeTheme* theme_) : TeComputeSpatialStrategy(theme_, TeLINES){}

		//! Destructor
		virtual ~TeLineLengthStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual  string compute (TeBox box);
};


//! Class to compute the area of polygons covering a given box
class TL_DLL TeTotalAreaPercentageStrategy: public TeComputeSpatialStrategy  
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeTotalAreaPercentageStrategy (TeTheme* theme_) : TeComputeSpatialStrategy(theme_, TePOLYGONS){}

		//! Destructor
		virtual ~TeTotalAreaPercentageStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);
};

/*
// pedro@20061214
//! Class to compute the distance based on a net 
class TeMinimumDistanceNetStrategy:  public TeComputeSpatialStrategy
{
	public:
		//! Constructor
		TeMinimumDistanceNetStrategy(TeTheme* theme_, TeGeomRep rep_, TeGraphNetwork* net, double dist_ = 50) : TeComputeSpatialStrategy(theme_, rep_) { maxDist = dist_; tree = NULL; }
		
		//! Destructor
		virtual ~TeMinimumDistanceNetStrategy() { if(tree) delete tree; }

        void CreatePoints(TeCoord2D p1, TeCoord2D p2); // create all intermediate points between them, using the maximum distance

		virtual string compute (TeBox box);

		virtual TeAttrDataType Type() { return TeREAL; }

	private:	

		double maxDist;

		vector<pair<TeCoord2D, TePoint> > dataSet;

		KDTREE* tree;
};
*/


//! Class to compute distances using KD-trees
class TL_DLL TeMinimumDistanceStrategy:  public TeComputeSpatialStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param gep_			the geometry to be used
			\param distError_	the maximum error tolerated by this operator
		*/	
		TeMinimumDistanceStrategy(TeTheme* theme_, TeGeomRep rep_, double distError_ = 50); 

		//! Destructor
		virtual ~TeMinimumDistanceStrategy() { if(tree) delete tree; }

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);

	protected:
		//! Create intermediate points between two given points, using the maximum distance
        void CreatePoints(TeCoord2D p1, TeCoord2D p2);

		//! The maximum error in the computation
		double maxDist;

		//!
		vector<pair<TeCoord2D, TePoint> > dataSet;

		//! The tree to store the points
		KDTREE* tree;
};


//! Class to compute the minimum distance to a theme of polygons. The calculatd distance is approximated, and has an error of at most
//! the parameter dist_ in the constructor
class TL_DLL TeMinimumDistancePolygonsStrategy: public TeMinimumDistanceStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param distError_	the maximum error tolerated by this operator
		*/	
		TeMinimumDistancePolygonsStrategy(TeTheme* theme_, double distError_ = 150);

		//! Destructor
		virtual ~TeMinimumDistancePolygonsStrategy() {}

		//! Function to compute the attribute
		/* 
			\param box		compute the attribute of a cell having this box
		*/
		virtual string compute (TeBox box);
};


//! Class to compute 
class TL_DLL TeMinimumDistanceLinesStrategy : public TeMinimumDistanceStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
			\param distError_	the maximum error tolerated by this operator
		*/	
		TeMinimumDistanceLinesStrategy(TeTheme* theme_, double distError_ = 150);

		//! Destructor
		virtual ~TeMinimumDistanceLinesStrategy() {}
};


//! Class to calculate the minimum distance of a box to points.
//! It calculates the exact value once the KD-tree calculates the exact distance to points
class TL_DLL TeMinimumDistancePointsStrategy: public TeMinimumDistanceStrategy
{
	public:
		//! Constructor
		/* 
			\param theme_		the theme to be used as basis for computing the attribute
		*/	
		TeMinimumDistancePointsStrategy(TeTheme* theme_);

		//! Destructor
		virtual ~TeMinimumDistancePointsStrategy() {}
};

/** @} */
#endif

