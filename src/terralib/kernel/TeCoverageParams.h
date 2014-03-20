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
/*!
  \file TeCoverageParams.h
  \brief This file contains definitions about parameters of a coverage.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEPARAMS_H
#define  __TERRALIB_INTERNAL_COVERAGEPARAMS_H

#include "TeDataTypes.h"
#include "TeDatabase.h"

/*!
  \enum TeCoveragePersistenceType
  \brief Defines the type of persistence used in the coverage.
*/
enum TeCoveragePersistenceType
{	TePERSISTENCE_NONE, TePERSISTENCE_DATABASE, TePERSISTENCE_DATABASE_CACHELRU };

/*!
  \struct TeCoverageDimension
  \brief Information about a single dimension of a coverage
*/
struct TL_DLL TeCoverageDimension
{
    int dimension_id;   //!< Dimension identifier
    std::string name;   //!< Dimension name
    TeDataType type;    //!< Data type of this dimension

    //! Default constructor.
    TeCoverageDimension(){}

    //! Constructor with attributes.
    TeCoverageDimension(const int& dimension_id, const std::string& name, const TeDataType& type):
            dimension_id(dimension_id),
            name(name),
            type(type){}
};

/*!
  \class TeCoverageParams
  \brief A class to handle the parameters set associated with a coverage

  \par This class is a data structure to store and manipulate a set of
       parameters and definitions that characterizes a coverage. It contains
       parameters relative to storaging, geographical or interpretation aspects.
  \par Not all parameters are valid, or make sense, for all coverages. Private
       parameters should be accessed through the methods provided.
*/
class TL_DLL TeCoverageParams
{
public:

    //! Default constructor
    TeCoverageParams () :
      database_(0),
      layerId_(-1),
      coverageId_(""),
      coverageTable_(""),
      fileName_(""),
      dimensions_(0),
      projection_(0),
      boundingBox_(TeBox()),
      numElements_(-1),
      persistenceType_(TePERSISTENCE_DATABASE_CACHELRU)
    {
    }

    //! Constructor specifying database access
    TeCoverageParams (TeDatabase* database, const int layerId, const std::string& coverageId = "0") :
      database_(database),
      layerId_(layerId),
      coverageId_(coverageId),
      coverageTable_(""),
      fileName_(""),
      dimensions_(0),
      projection_(0),
      boundingBox_(TeBox()),
      numElements_(-1),
      persistenceType_(TePERSISTENCE_DATABASE_CACHELRU)
    {
        
    }

    //! Copy constructor
    TeCoverageParams(const TeCoverageParams& other);

    //! Operator=
    TeCoverageParams& operator=(const TeCoverageParams& rhs);

    //! Destructor
    virtual ~TeCoverageParams()
    {
    }

    //! Set pointer to the database
    void setDatabase(TeDatabase* database)
    {
        database_ = database;
    }

    //! Return a pointer to the database
    TeDatabase* getDatabase() const
    {
        return database_;
    }

    //! Set layer identifier
    void setLayerId(const int layerId)
    {
        layerId_ = layerId;
    }

    //! Return the layer identifier
    int getLayerId() const
    {
        return layerId_;
    }

    //! Set coverage identifier
    void setCoverageId(std::string& coverageId)
    {
        coverageId_ = coverageId;
    }

    //! Return the coverage identifier
    std::string getCoverageId() const
    {
        return coverageId_;
    }

    //! Set the name of the coverage table
    void setCoverageTable(std::string& coverageTable)
    {
        coverageTable_ = coverageTable;
    }

    //! Return the name of the coverage table
    std::string getCoverageTable() const
    {
        return coverageTable_;
    }

    //! Set the name of the coverage source file
    void setFileName(std::string& fileName)
    {
        fileName_ = fileName;
    }

    //! Return the name of the coverage source file
    std::string getFileName() const
    {
        return fileName_;
    }

    //! Set information about the dimensions of a coverage
    void setDimensions(std::vector<TeCoverageDimension>& dimensions)
    {
        dimensions_ = dimensions;
    }

    //! Return information about the dimensions of a coverage
    std::vector<TeCoverageDimension> getDimensions() const
    {
        return dimensions_;
    }

    //! Set the projection
    void setProjection(TeProjection* projection)
    {
        projection_ = projection;
    }

    //! Return the projection
    TeProjection* getProjection() const
    {
        return projection_;
    }

    //! Set bounding box of a coverage
    void setBoundingBox(TeBox& boundingBox)
    {
        boundingBox_ = boundingBox;
    }

    //! Return the bounding box of a coverage
    TeBox getBoundingBox() const
    {
        return boundingBox_;
    }

    //! Set number of elements of a coverage
    void setNumElements(const int numElements)
    {
        numElements_ = numElements;
    }

    //! Return the number of elements of a coverage
    int getNumElements() const
    {
        return numElements_;
    }

    //! Set persistence type of the coverage
    void setPersistenceType(const TeCoveragePersistenceType persistenceType)
    {
        persistenceType_ = persistenceType;
    }

    //! Return the persistence type of the coverage
    TeCoveragePersistenceType getPersistenceType() const
    {
        return persistenceType_;
    }

protected:
    TeDatabase* database_;          //!< Pointer to the TerraLib database
    int layerId_;                   //!< Layer identifier
    std::string coverageId_;        //!< Coverage identifier
    std::string coverageTable_;     //!< Coverage table name
    std::string fileName_;          //!< Name of a coverage source file
    std::vector<TeCoverageDimension> dimensions_;   //!< Dimensions information
    TeProjection* projection_;      //!< Projection used in a coverage
    TeBox boundingBox_;             //!< Bounding box for all elements of a coverage
    int numElements_;               //!< Number of elements of the coverage
    TeCoveragePersistenceType persistenceType_; //!< Type of data persistence
};

#endif // __TERRALIB_INTERNAL_COVERAGEPARAMS_H