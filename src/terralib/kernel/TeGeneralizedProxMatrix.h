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
/*! \file TeGeneralizedProxMatrix.h
	\brief This file contains structures and definitions about generalized proximity matrices support in TerraLib
*/

#ifndef TeProxMatrix_H
#define TeProxMatrix_H

#include "TeNeighbours.h"
#include "TeProxMatrixImplementation.h"
#include "TeProxMatrixConstructionStrategy.h"
#include "TeProxMatrixSlicingStrategy.h"
#include "TeProxMatrixWeightsStrategy.h"
#include "TeSTElementSet.h"


//! A class to represent a generalized proximity matrix 
template<typename Set> 
class TeGeneralizedProxMatrix  
{
private:
	TeProxMatrixImplementation*	imp_;  
	TeProxMatrixConstructionStrategy<Set>* sc_;   
	TeProxMatrixSlicingStrategy* ss_;   
	TeProxMatrixWeightsStrategy* sw_;  
	
protected:
	//! Get the implementation from a concrete factory defined by impl_type
	TeProxMatrixImplementation*				getImplementation(const TeGPMImplementation& implementation_type = TeGraphBreymann);

	//! Verify if a matrix was created correctly by the constructors.	
	bool isValid() const;
	
public:
	int						gpm_id_;	
	bool					is_default_;
	int						total_slices_;
	string					neighbourhood_table_;
	TeGPMImplementation		impl_strategy_;

	//! Empty constructor
	TeGeneralizedProxMatrix ():
		imp_(0), sc_(0), ss_(0), sw_(0), gpm_id_(-1),
		is_default_(false), total_slices_(1), neighbourhood_table_(""), 
		impl_strategy_(TeGraphBreymann)
	  {}

  //! Constructor
	TeGeneralizedProxMatrix (const int& id, const string& tableName, TeProxMatrixImplementation* imp, TeProxMatrixWeightsStrategy* ws):
		imp_(imp), sc_(0), ss_(0), sw_(ws),
		gpm_id_(id), is_default_(false), total_slices_(1), 
		neighbourhood_table_(tableName), impl_strategy_(TeGraphBreymann)
	  {}

	//! Constructor parametrized with specific strategies. Each stragegy must be previously created and correctly parametrized.
	TeGeneralizedProxMatrix (TeProxMatrixConstructionStrategy<Set>* sc,  TeProxMatrixWeightsStrategy* sw = 0, 
		TeProxMatrixSlicingStrategy* ss = 0, const TeGPMImplementation& type = TeGraphBreymann, 
		const int& gpmId=-1, const bool& isDefault=false,  const string& neigsTable ="", const int& slices=1);

	//! Constructor based on default strategies: (a) Local adjacency of first order; (b) No siling;  and (c)  No weighs (all equal to 1).
	TeGeneralizedProxMatrix(TeSTElementSet* objects, TeGeomRep geom_type, const TeGPMImplementation& type = TeGraphBreymann);

	//! Copy constructor
	TeGeneralizedProxMatrix(const TeGeneralizedProxMatrix& p);

	//! Attribution Operator 
	TeGeneralizedProxMatrix& operator=(const TeGeneralizedProxMatrix& p);

	bool clearImplementation ();

	//! Comparison Operator
	bool operator==(const TeGeneralizedProxMatrix& p) const;
	
	
	/** @name getNeighbours Methods for return the neighbours
	*  All methods return the  neighbours of a given object in a given slice. The default is the first slice.
	*  Slices are defined according to the Slicing Strategy in use (e.g., according to distance zones, corresponding to neighbourhood orders, weights intervals, etc.). 
	*  If the parameter slice is not provided, the first slice is returned.
	*  The operator[] method should preferably be used 
	*  For each object, all the connection attributes are stored as properties (weight is the first).
	*/
	//@{ 
	//! Return the neighbours of an object in a slice, packed in a TeNeighbours 
	TeNeighbours getNeighbours (const string& object_id, int slice = 1);

	//! Return the neighbours of an object in a slice, packed in a TeNeighboursMap
	TeNeighboursMap	getMapNeighbours (const string& object_id, int slice = 1);
	
	//! Operator [], return the neighbours packed in a TeNeighbours
	TeNeighbours operator[] (const string& object_id); 

	//! Return the neighbours of an object and their attributes in a spatial temporal element set (TeSTElementSet)
	TeSTElementSet getSTENeighbours(const string& object_id); 
	//@}

	
	/** @name ChangeStrategies Methods for changing current strategies 
	*  change current strategies 
	*/
	//@{ 
	//! Set current construction strategy 
	bool setCurrentConstructionStrategy (TeProxMatrixConstructionStrategy<Set>* sc);  
	
	//! Set current weights strategy 
	bool setCurrentWeightsStrategy (TeProxMatrixWeightsStrategy* sw); 
	
	//! Set current slicing strategy 
	bool setCurrentSlicingStrategy (TeProxMatrixSlicingStrategy* ss); 

	//! Get current construction params 
	TeProxMatrixConstructionParams* getConstructionParams() 
	{	
		if(sc_)
			return &(sc_->constructionParams()); 
		return 0;
	} 
	
	//! Get current weights params 
	TeProxMatrixWeightsParams* getWeightsParams() 
	{ 
		if(sw_)
			return &(sw_->weightsParams()); 
		return 0;
	}  
	
	//! Get current slicing params 
	TeProxMatrixSlicingParams* getSlicingParams() 
	{ 
		if(ss_)
            return &(ss_->slicingParams()); 
		return 0;
	} 
	//@}
	

	//! Reconstruct matrix and recompute weights and slicing, accornding to current strategies.
	bool constructMatrix ();

	//! Recomputes the weigths, given a new strategy. The matrix is not reconstructed.
	bool recomputeWeights ();

	//! Sets the slicing strategy for neighbours selection.
	bool recomputeSlicing ();

	//! Verify if two objects are connected
	bool isConnected (const string& object_id1, const string& object_id2); 

	//! Connect two objects
	bool connectObjects (const string& object_id1, const string& object_id2, const TeProxMatrixAttributes& attr); 
	
	//! Connect two objects
	bool connectObjects (const string& object_id1, const string& object_id2);

	//! Disconnect two objects
	bool disconnectObjects (const string& object_id1, const string& object_id2); 

	//! Remove object
	bool removeObject (const string& object_id);  

	//! Get connection attributes
	bool getConnectionAttributes (const string& object_id1, string& object_id2, TeProxMatrixAttributes& attr); 

	//! Return the number of objects
	int numberOfObjects (); 

	//! Return the number of slices
	int numerOfSlices () {return total_slices_;}

	//! Save the matrix in a text file
	bool saveTextFile (const string& name, map<string, string>* ids=0); 

	//! Save the matrix in a text file
	bool saveGALFile (const string& name, map<string, string>* ids=0); 	
	
	//! Save the matrix in a text file
	bool saveGWTFile (const string& name, map<string, string>* ids=0); 

	//! Save the matrix in a text file
	bool saveTextFile (const string& name, vector<string>* ids); 

	//! Save the matrix in a text file
	bool saveGALFile (const string& name, vector<string>* ids); 	
	
	//! Save the matrix in a text file
	bool saveGWTFile (const string& name, vector<string>* ids); 

	//! Destructor
	virtual ~TeGeneralizedProxMatrix()
	{
		if (imp_) 
			delete imp_; //It is not counted
	}
};

//--- template implementations ---
template<typename Set>
TeGeneralizedProxMatrix<Set>::TeGeneralizedProxMatrix(TeSTElementSet* objects, TeGeomRep geom_type, const TeGPMImplementation& imp_type)
{
	gpm_id_=-1;
    is_default_=false; 
	total_slices_=1; 
	neighbourhood_table_=""; 
    impl_strategy_=TeGraphBreymann;
	
	imp_ = 0;
	imp_ = getImplementation(imp_type);
	if ((geom_type==TePOLYGONS) || (geom_type==TeCELLS))
	{
		sc_ = new TeProxMatrixLocalAdjacencyStrategy (objects, geom_type);
		ss_ = new TeProxMatrixNoSlicingStrategy;
		sw_ = new TeProxMatrixNoWeightsStrategy;
	}
	else
	{
		imp_ = 0;
		sc_ = 0;
		ss_ = 0;
		sw_ = 0;
	}
}

template<typename Set>
TeGeneralizedProxMatrix<Set>::TeGeneralizedProxMatrix (TeProxMatrixConstructionStrategy<Set>* sc,  TeProxMatrixWeightsStrategy* sw, 
		TeProxMatrixSlicingStrategy* ss, const TeGPMImplementation& type, 
		const int& gpmId, const bool& isDefault,  const string& neigsTable, const int& slices):
	sc_(sc),
	ss_(ss),
	sw_(sw),
	gpm_id_(gpmId), 
	is_default_(isDefault),
	total_slices_(slices),
	neighbourhood_table_(neigsTable),
	impl_strategy_(type)
	
{
	
	imp_ = 0;
	imp_ = getImplementation(type);
	
	if(!ss)
		ss_ = new TeProxMatrixNoSlicingStrategy();

	if(!sw)
		sw_ = new TeProxMatrixNoWeightsStrategy();
}


template<typename Set> bool
TeGeneralizedProxMatrix<Set>::isValid () const
{
	if ((imp_) && (sc_) && (ss_) && (sw_))
		return true;

	return false;
}


template<typename Set> TeProxMatrixImplementation*
TeGeneralizedProxMatrix<Set>::getImplementation (const TeGPMImplementation& type)
{
	 if (imp_ == 0) 
		imp_ = TeProxMatrixAbstractFactory::MakeConcreteImplementation (type); 
    return imp_;
}



template<typename Set>
TeGeneralizedProxMatrix<Set>::TeGeneralizedProxMatrix(const TeGeneralizedProxMatrix<Set>& pm)
{
	if (pm.imp_ == 0)
		imp_ = 0;
	else 
		imp_ = pm.imp_->createCopy ();

	sc_ = pm.sc_;
	ss_ = pm.ss_;
	sw_ = pm.sw_;

	gpm_id_=pm.gpm_id_;	
	is_default_=pm.is_default_;
	total_slices_=pm.total_slices_;
	neighbourhood_table_=pm.neighbourhood_table_;
	impl_strategy_=pm.impl_strategy_;
}


template<typename Set> TeGeneralizedProxMatrix<Set>& 
TeGeneralizedProxMatrix<Set>::operator=(const TeGeneralizedProxMatrix<Set>& pm) 
{
	if (*this == pm) return *this;

	if (imp_) delete imp_;
	imp_ = 0;
	if (pm.imp_)	imp_ = pm.imp_->createCopy ();

	sc_ = pm.sc_;
	ss_ = pm.ss_;
	sw_ = pm.sw_;
	gpm_id_=pm.gpm_id_;	
	is_default_=pm.is_default_;
	total_slices_=pm.total_slices_;
	neighbourhood_table_=pm.neighbourhood_table_;
	impl_strategy_=pm.impl_strategy_;

	return *this;

}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::operator==(const TeGeneralizedProxMatrix<Set>& pm) const
{
	if (isValid() && pm.isValid()) 
	{	
		if ((sc_->IsEqual (*(pm.sc_))) &&
			(ss_->operator==(*(pm.ss_))) &&
			(sw_->operator==(*(pm.sw_))) &&
			(total_slices_ == pm.total_slices_) &&
			(imp_->isEqual(*(pm.imp_)))) 
			return true;
	} 
	else 	
		if (!isValid() && !pm.isValid()) 
			return true;
	return false;
}

template<typename Set> TeNeighbours
TeGeneralizedProxMatrix<Set>:: getNeighbours (const string& object_id, int slice) 
{
	TeNeighbours neigh2;
	if (slice > total_slices_) 
		return neigh2;

	if (imp_) 
	{
		TeNeighbours neigh1;
		imp_->getNeighbours (object_id, neigh1);
		for (int i=0; i < neigh1.size(); i++) 
			if (neigh1.Attributes(i).Slice() == slice) 
				neigh2.Insert (neigh1.ObjectId(i), neigh1.Attributes(i));
	}

	return neigh2;
}


template<typename Set> TeNeighbours 
TeGeneralizedProxMatrix<Set>::operator[](const string& object_id) 
{
	return getNeighbours(object_id);
}


template<typename Set> TeNeighboursMap
TeGeneralizedProxMatrix<Set>::getMapNeighbours (const string& object_id, int slice) 
{
	TeNeighboursMap neighMap;
	if (slice > total_slices_) 
		return neighMap;

	if (imp_) 
	{
		TeNeighbours neigh;
		imp_->getNeighbours (object_id, neigh);

		for (int i=0; i < neigh.size(); i++) 
			if (neigh.Attributes(i).Slice() == slice) 
				neighMap[neigh.ObjectId(i)] = neigh.Attributes(i);
	}

	return neighMap;
}


template<typename Set> TeSTElementSet 
TeGeneralizedProxMatrix<Set>::getSTENeighbours(const string& object_id)
{
	TeSTElementSet selected_objects;
	if (imp_ && sc_) 
	{
		TeNeighbours neigh;
		imp_->getNeighbours (object_id, neigh);
		
		for (int i = 0; i < neigh.size(); i++) 
		{
			// Construct a sto instance with its attributes
			TeSTInstance obj;
			obj.objectId(neigh.ObjectId(i));  
			
			//load the attributes
			TePropertyVector propVector;
			sc_->objects()->getPropertyVector(object_id, propVector);
			obj.properties(propVector);
				
			// insert object in the return vector
			selected_objects.insertSTInstance(obj);
		}
	}
	return selected_objects;
}


template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::setCurrentConstructionStrategy (TeProxMatrixConstructionStrategy<Set>* sc)  
{	
	if (sc == 0) 
		return false; 
	sc_ = sc; 
	return true;
}


template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::setCurrentWeightsStrategy (TeProxMatrixWeightsStrategy* sw) 
{	
	if (sw == 0) 
		return false; 
	sw_ = sw; 
	return true;
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::setCurrentSlicingStrategy (TeProxMatrixSlicingStrategy* ss) 
{
	if (ss == 0) 
		return false; 
	ss_ = ss; 
	return true;
}

template<typename Set> bool
TeGeneralizedProxMatrix<Set>::constructMatrix ()
{
	if(!isValid())
		return false;

	//	ClearImplementation();
	imp_ = 0;
	imp_ = getImplementation();

	if (sc_) 
	{
		if (sc_->Construct (imp_)) 
		{
			if (ss_) 
				ss_->Slice (imp_);
			if (sw_) 
				sw_->ComputeWeigths (imp_); 
			return true;
		} 
	}
	imp_ = 0;
	sc_ = 0;
	ss_ = 0;
	sw_ = 0;
	return false;
}


template<typename Set> bool
TeGeneralizedProxMatrix<Set>::clearImplementation ()
{
	TeProxMatrixImplementation* aux;
	if (imp_ == 0)  
		aux = getImplementation ();
	else	
		aux = getImplementation (imp_->type());

	if (aux == 0) 
		return false;
	
	delete imp_;
	imp_ = aux;

	return true;
}

template<typename Set> bool
TeGeneralizedProxMatrix<Set>::recomputeWeights ()
{
	if (isValid()){
		sw_->ComputeWeigths (imp_); return true;
	} 
	return false;
}


template<typename Set> bool
TeGeneralizedProxMatrix<Set>::recomputeSlicing ()
{
	if (isValid()){
		ss_->Slice (imp_); return true;
	} 
	return false;
}


template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::isConnected (const string& object_id1, const string& object_id2) 
{
	if (imp_ == 0) 
		return false;  
	return imp_->isConnected (object_id1, object_id2);
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::connectObjects (const string& object_id1, const string& object_id2, const TeProxMatrixAttributes& attr)
{
	if (!imp_) 
		getImplementation();  
	imp_->connectObjects (object_id1, object_id2, attr);
	return true;
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::connectObjects (const string& object_id1, const string& object_id2)
{
	if (!imp_) 
		getImplementation();  
	TeProxMatrixAttributes attr;
	imp_->connectObjects (object_id1, object_id2, attr);
	return true;
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::disconnectObjects (const string& object_id1, const string& object_id2)
{
	if (imp_ == 0) 
		return false;  
	return imp_->disconnectObjects (object_id1, object_id2);
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::removeObject (const string& object_id)
{
	if (imp_ == 0) 
		return false;  
	return imp_->removeObject (object_id);
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::getConnectionAttributes (const string& object_id1, string& object_id2, TeProxMatrixAttributes& attr)
{
	if (imp_ == 0) 
		return false;
	return imp_->getConnectionAttributes (object_id1, object_id2, attr); 
}

template<typename Set> int  
TeGeneralizedProxMatrix<Set>::numberOfObjects () 
{
	if (imp_ == 0) 
		return 0;  
	return imp_->numberOfObjects ();
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveTextFile (const string& name, map<string, string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveTextFile (name, ids); 
}


template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveGALFile (const string& name, map<string, string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveGALFile (name, ids); 
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveGWTFile (const string& name, map<string, string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveGWTFile (name, ids); 
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveTextFile (const string& name, vector<string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveTextFile (name, ids); 
}


template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveGALFile (const string& name, vector<string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveGALFile (name, ids); 
}

template<typename Set> bool 
TeGeneralizedProxMatrix<Set>::saveGWTFile (const string& name, vector<string>* ids) 
{	
	if (imp_ == 0) 
		return false;
	return imp_->saveGWTFile (name, ids); 
}


#endif
