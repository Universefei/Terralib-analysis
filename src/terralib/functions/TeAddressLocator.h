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
/*! \file TeAddressLocator.h
    \brief This file contains algorithms for address location generation.
*/

/**
  *@author Gilberto Ribeiro de Queiroz and Ricardo Cartaxo
  */

#ifndef  __TERRALIB_INTERNAL_ADDRESSLOCATOR_H
#define  __TERRALIB_INTERNAL_ADDRESSLOCATOR_H

#include "../kernel/TeDatabase.h"
#include "../kernel/TeLayer.h"

#include "TeFunctionsDefines.h"

#include <string>
#include <map>

using namespace std;

/** @defgroup GeoCod Geocoding 
    @{
	Structures and classes supporting geocoding operations.
 */

//! A dictionary of geocoding parameters
typedef multimap<string, string> TeGeocoderDictionary;


/*! \struct TeAddressMetadata
	Metadata for a table used as a base for geocoding
*/
struct TLFUNCTIONS_DLL TeAddressMetadata
{
	int tableId_;					//!< This value is a foreign key from te_layer_table indicates the table wich will be used for the address locator
	string tableName_;				//!< The table to use for geocoding process
	string linkColumnName_;			//!< The column used as link to line geometry
	string initialLeftNumber_;		//!< The field name with the initial location left number
	string finalLeftNumber_;		//!< The field name with the final location left number
	string initialRightNumber_;		//!< The field name with the initial location right number
	string finalRightNumber_;		//!< The field name with the final location right number
	string locationType_;			//!< The field name with location type (Av., Rua, R., Pr., Jz., Pres. ...)
	string locationTitle_;			//!< The field name with location title (Dr., Dra., PRF ...)
	string locationPreposition_;	//!< The field name with the preposition used to form the location name (Dos, Das, De, 
	string locationName_;			//!< The field name with the location name
	string locationCompleteName_;	//!< The field name with composed location name
	string leftNeighborhood_;		//!< The field name for the left location neighborhood 
	string rightNeighborhood_;		//!< The field name for the right location neighborhood
	string leftZipCode_;			//!< The field name for the left location zip code
	string rightZipCode_;			//!< The field name for the right location zip code
};

//!  Types for centroid field name finding
TLFUNCTIONS_DLL enum TeCentroidFieldType { TeNEIGHBORHOOD, TeZIPCODE };

/*! \struct TeCentroidAddressMetadata
	Metadata for a table used as a base for geocoding when an address is not found and a centroid for the zip code or neighborhood exists
*/
struct TLFUNCTIONS_DLL TeCentroidAddressMetadata
{
	string centroidName_;			//!< This field may be the neighborhood name or zip code name

	TeCentroidFieldType fieldType_;	//!< Field type

	string attributeTableName_;		//!< Attribute table name

	TeLayer* centroidLayer_;		//!< Polygon or point layer to extract centroid from

	TeCentroidAddressMetadata()
		: centroidLayer_(0)
	{
	}
};

/*! \struct TeGeocodeTableMetadata
	Metadata for the table that contains address to be geocode
*/
struct TLFUNCTIONS_DLL TeGeocodeTableMetadata
{
	string tableName_;						//!< The table to be geocoding
	string linkColumnName_;					//!< The column used as link column in geocoding
	string keyColumnName_;					//!< The column with index
	int tableId_;							//!< The id of the table for geocoding
	string locationTypeColumnName_;			//!< The field name with location type (Av., Rua, R., Pr., Jz., Pres. ...)
	string locationTitleColumnName_;		//!< The field name with location title (Dr., Dra., PRF ...)
	string locationPrepositionColumnName_;	//!< The field name with the preposition used to form the location name (Dos, Das, De, 
	string locationNameColumnName_;			//!< The field name with the location name
	string locationNumberColumnName_;		//!< The field name with the ocation number
	string neighborhoodColumnName_;			//!< The field name for the location neighborhood
	string zipCodeColumnName_;				//!< The field name for the location zip code	
};


/*! \struct TeAddress
	Information of an address that will be geocoded
*/
struct TLFUNCTIONS_DLL TeAddress
{
	string locationLink_;			//!< The value that unique identifies the location	
	string locationType_;			//!< Location type
	string locationTitle_;			//!< Location title
	string locationPreposition_;	//!< Location preposition
	string locationName_;			//!< Location name
	int locationNumber_;			//!< Location number
	string neighborhood_;			//!< Location neighborhood
	string zipCode_;				//!< Location zip code
};


/* \struct TeAddressDescription
	Information about a found address
 */
struct TLFUNCTIONS_DLL TeAddressDescription
{
	string locationLink_;			//!< The value that unique identifies the location
	int initialLeftNumber_;			//!< The initial location left number
	int finalLeftNumber_;			//!< The final location left number
	int initialRightNumber_;		//!< The initial location right number
	int finalRightNumber_;			//!< The final location right number
	string locationType_;			//!< The location type (Av., Rua, R., Pr., Jz., Pres. ...)
	string locationTitle_;			//!< The location title (Dr., Dra., PRF ...)
	string locationPreposition_;	//!< The preposition used to form the location name (Dos, Das, De, 
	string locationName_;			//!< The location name
	string locationCompleteName_;		//!< The complete location name
	string leftNeighborhood_;		//!< The left location neighborhood 
	string rightNeighborhood_;		//!< The right location neighborhood
	string leftZipCode_;			//!< The eft location zip code
	string rightZipCode_;			//!< The right location zip code
	TeCoord2D pt_;					//!< point location of the address
	bool isValid_;					//!< Tells if pt_ is a valid coordinate or the segment range was not found

	TeAddressDescription()
		: locationLink_(""), initialLeftNumber_(0), finalLeftNumber_(0),
		  initialRightNumber_(0), finalRightNumber_(0), locationType_(""),
		  locationTitle_(""), locationPreposition_(""), locationName_(""), locationCompleteName_(""), 
		  leftNeighborhood_(""), rightNeighborhood_(""), leftZipCode_(""),
		  rightZipCode_(""), isValid_(false)
	{
	}
};


//! An abstract class to support interactivity in geocoding
/*!
	Subclasses of this class must implement the "operator()" method
*/
class TLFUNCTIONS_DLL TeAbstractAddressNotifyFunctor
{
	public:

		virtual int operator()(const TeAddress& searchedAddress, vector<TeAddressDescription>& foundAddresses) = 0;

		virtual ~TeAbstractAddressNotifyFunctor()
		{
		}
};

//! A class to handle the geocoder support
/*!
	Instances of this class contains information about
	a table that can be used for geocoding and methods
	to geocode a given address.
*/
class TLFUNCTIONS_DLL TeAddressLocator
{
	protected:

		TeDatabase* db_;							//!< Database connection
		TeAddressMetadata metadata_;				//!< Metadata information for a geocoding base table
		TeLayer* referenceLayer_;					//!< Ther layer used as reference for geocoding 
		TeCentroidAddressMetadata cmetadata_;		//!< Centroid metadata
		bool isMetadataLoaded_;						//!< Flag to indicate if the metadata is loaded
		string errMsg_;								//!< Error message description
		TeGeocoderDictionary dictionary_;			//!< Use a geocoder dictionary
		bool changeSpecialChars_;					//!< If true should replace special characters
		bool removeHifen_;							//!< If true the hifen is removed in zip code
		bool removeHifenFromCentroid_;				//!< If true the hifen is removed in zip code from the centroid table
		char hifen_;
		string splitChar_;							//!< Used to split address name and number
		
		vector<string> typesVector_;				//!< Vector with default types for street names
		vector<string> titlesVector_;				//!< Vector with default titles for street names
		vector<string> prepositionsVector_;			//!< Vector with default prepositions for street names

	public:

		//! Constructor
		TeAddressLocator(TeDatabase* db)
			: db_(db), referenceLayer_(0), isMetadataLoaded_(false), errMsg_(""), changeSpecialChars_(false), removeHifen_(false), removeHifenFromCentroid_(false)
		{
		}

		//! Virtual destructor
		virtual ~TeAddressLocator()
		{
		}
		
		//! Creates the table to store metadata for geocoding tables
		bool createAddressMetadataTable(void);

		//! Inserts a new metadata for an attribute table layer and saves in the database
		bool insertAddressMetadata(const TeAddressMetadata& metadata);

		//! Updates the metadata of an attribute table layer and saves in the database
		bool updateAddressMetadata(const TeAddressMetadata& metadata);

		//! Removes the metadata for the attribute table layer on the database an clear memory
		bool deleteAddressMetadata(const int& tableId);
		
		//! Loads a metadata for an attribute table layer
		bool loadAddressMetadata(const int& tableId);

		//! Returns true if metadata was loaded
		bool isMetadataLoaded() const
		{
			return isMetadataLoaded_;
		}

		//! Returns the metadata of the geocode base table
		TeAddressMetadata getAddressLocatorMetadata() const
		{
			return metadata_;
		}

		//! Set the metadata
		bool setAddressLocatorMetadata(const TeAddressMetadata& m);

		//! Returns the error message
		string errorMessage() const
		{
			return errMsg_;
		}

		//! Information about a layer to be used as an alternative for geocoding
		void setCentroidAddressMetadata(const TeCentroidAddressMetadata& cMetadata)
		{
			removeHifenFromCentroid_ = false;

			cmetadata_ = cMetadata;

			if(cmetadata_.fieldType_ == TeZIPCODE && !cmetadata_.centroidName_.empty() && !cmetadata_.attributeTableName_.empty())
				removeHifenFromCentroid_ = !verifyHifen(cmetadata_.attributeTableName_, cmetadata_.centroidName_);
		}

		void setReplaceSpecialChars(const bool& r)
		{
			changeSpecialChars_ = r;
		}

		bool getReplaceSpecialChars(void) const
		{
			return changeSpecialChars_;
		}

		void setSplitChar(const string& splitChar)
		{
			splitChar_ = splitChar;
		}
		
		//! Retrieves all metadata for a layer
		bool getAllAddressMetadataLayer(const int& layerId, vector<TeAddressMetadata>& addVec);

		//! Retrieves all metadata for a theme
		bool getAllAddressMetadataTheme(const int& themeId, vector<TeAddressMetadata>& addVec);

		//! Retrieves all layer name that has an address location table
		bool getAddressLocatorLayers(vector<pair<int, string> >& layerNameVec);

		//! Retrieves all layer table names that is register as an address find table for a given layerId
		bool getAddressLocatorLayerTables(vector<pair<int, string> >& tableNameVec, const int& layerId);

		//! Finds all geocoding table in a TerraLib database
		bool findGeocodingTables(vector<pair<int, string> >& tableNames);
		
		//! Looks the attribute description to transform a name location on an address
		int findAddress(const TeAddress& address, vector<TeAddressDescription>& addressesFound, const bool& doExaustiveSearch = false);

		//! Finds segment and does an interpolation
		void findCoordinates(const TeAddress& add, vector<TeAddressDescription>& addresses);

		//! Finds segment and does an interpolation
		void findCoordinate(const TeAddress& add, TeAddressDescription& address);

		//! Spatialize a table based on the indicated fields
		bool addressGeocode(TeGeocodeTableMetadata& tableMetadata,
							const string& newLayerName,
							const string& reportName,
							const string& reportDir,
							const bool& doExaustive = false,
							TeAbstractAddressNotifyFunctor* notifyFunction = 0);

		//! Load a dictionary to change words in the searched addresses 
		bool loadDictionary(const string& dictionaryFileName);

		//! 
		bool loadAddressCompoundNames();

		//! Used to make columns in upper case
		bool convertColumnNameToUpper();

		//! Used to join columns in one field
		bool addCompleteNameColumn();

	protected:

		//! Looks the attribute description to transform a name location on an address
		int findExact(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound);

		//! Looks the attribute description to transform a name location on an address
		int findPattern(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound, const bool& disabletype = false, const bool& disabletitle = false, const bool& disablepreposition = false);


		//! Looks the attribute description to transform a name location on an address
		int findWithDictionary(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound);

		//! Looks the attribute description to transform a name location on an address
		int findWithoutAllName(const TeAddress& address, const string& fullName, vector<TeAddressDescription>& addressesFound, const bool& disabletype = false, const bool& disabletitle = false, const bool& disablepreposition = false);

		//! Looks the attribute description to transform a name location on an address
		bool findSimilarAddress(const TeAddress& address, const string& fullName, TeAddressDescription& descript);

		//! Try to find a centroid for zip code or neighborhood
		bool findByCentroid(const TeAddress& addressToFind, vector<TeAddressDescription>& addressesFound);

		//! Fill the metadata parameter
		void fillAddressMetadata(TeAddressMetadata& metadata, TeDatabasePortal* p);

		//! Fill address descriptions
		void fillAddressesDescription(vector<TeAddressDescription>& addresses, TeDatabasePortal* p);

		//!
		void fillAddressDescription(TeAddressDescription& descript, TeDatabasePortal* p);

		//! Fill address information
		void fillAddress(TeAddress& add, const TeGeocodeTableMetadata& tableMetadata, TeDatabasePortal* p);

		//! Mount a SQL to look for an address
		string mountExactWhereSQL(const TeAddress& address, const string& fullName);

		string mountLikeWhereSQL(const TeAddress& address, const string& fullName);

		string mountPatternWhereSQL(const TeAddress& address, const string& fullName, const bool& disabletype = false, const bool& disabletitle = false, const bool& disablepreposition = false);

		//!
		string mountSimilarWhereSQL(const TeAddress& address);

		//! Do some conversions in address fields
		void normalizeAddress(TeAddress& address, string& completeName);

		//! Removes hifen if remove flag is true
		void removeHifen(string& zipCode) const;

		//! Removes hifen if remove flag is true
		void insertHifen(string& zipCode) const;

		//! Inserts a special character in string for Like comparasion
		string makeLikeExpression(const string& str) const;

		//! Remove caracteres como ponto e troca acentuados por n�o acentuados
		void removeSpecialCharacters(string& str) const;

		//!
		bool verifyHifen(const string& tableName, const string& columnName);

		//!
		bool addReportColumn(TeGeocodeTableMetadata& tableMetadata, pair<string, string>& newColumns);

		//!
		bool findTableMetadata(TeGeocodeTableMetadata& tableMetadata);

		//! Update the table that contains address to be geocoded with the code returned by the geocoding process and a description of the process match
		bool updateAddressTable(const string& c1, const int& v1, TeGeocodeTableMetadata& tableMetadata, const string& objectId);

		//! Returns a portal to the address table
		TeDatabasePortal* getAddressTablePortal(TeGeocodeTableMetadata& tableMetadata, int& numRows);

		//! 
		void refineFoundedLocationsByNeighBorhood(const TeAddress& address, vector<TeAddressDescription>& addressesFound);

		//! 
		void refineFoundedLocationsByZipCode(const TeAddress& address, vector<TeAddressDescription>& addressesFound);		

		//! 
		void refineFoundedLocationsByNumberNeighboor(const TeAddress& address, vector<TeAddressDescription>& addressesFound);

		//! 
		void refineFoundedLocationsByNumberZipCode(const TeAddress& address, vector<TeAddressDescription>& addressesFound);

		void cleanAddressesFound(const string& locName, vector<TeAddressDescription>& addressesFound, const bool& disabletype = false, const bool& disabletitle = false, const bool& disablepreposition = false);

		//! Split address name and number
		void splitAddress(TeAddress& add);

		//! Only to query addresses and fill result
		void queryAddresses(const string& strSQL, bool& queryExecuted, vector<TeAddressDescription>& addressesFound);

		//! No copy allowed
		TeAddressLocator(const TeAddressLocator& other);
		TeAddressLocator& operator=(const TeAddressLocator& other);
};
/** @} */
#endif	// __TERRALIB_INTERNAL_ADDRESSLOCATOR_H

