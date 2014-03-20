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

/*
 * TODO: 1. Usar os 5 primeiros dígitos do CEP para desempatar, pois eles levam 
 *          ao bairro.
 */

#include "TeAddressLocator.h"
#include "../kernel/TeUtils.h"

#include "../kernel/TeProgress.h"
#include "../kernel/TeAsciiFile.h"
#include "../kernel/TeVectorRemap.h"

#include <string>

using namespace std;

double TeSimilarity(const char* streetname, const char* rotuloname);

void validColumnName(TeAttributeRep& rep, TeAttributeList& attrList)
{
	unsigned int nStep = attrList.size();

	string repName = TeConvertToUpperCase(rep.name_);

	bool alter = true;

	while(alter)
	{
		alter = false;

		for(unsigned int i = 0; i < nStep; ++i)
		{
			if(TeConvertToUpperCase(attrList[i].rep_.name_) == repName)
			{
				rep.name_ += "_";
				alter = true;
				repName = TeConvertToUpperCase(rep.name_);
			}
		}
	}
}

void tokenize(const string& name, vector<string>& tokens)
{
	unsigned int nStep = name.size();

	string token = "";

	unsigned int i = 0;

	for(i = 0; i < nStep; ++i)
	{
		if(name[i] != ' ')
			token += name[i];
		else
		{
			if(!token.empty())
			{
				tokens.push_back(token);
				token = "";
			}
		}		
	}

	if(!token.empty())
		tokens.push_back(token);
}

//---- TeAddressLocator methods

bool TeAddressLocator::createAddressMetadataTable(void)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	if(db_->tableExist("te_address_locator"))
	{
		// Verificar se a coluna com o nome completo existe.
		// Nas versões anteriores essa coluna não era usada.

		TeAttribute attr;

		if(!db_->columnExist("te_address_locator", "location_full_name", attr))
		{
			// Se a coluna não existe, tenta adicionar a coluna à tabela de metadado
			TeAttribute at10;
			at10.rep_.type_ = TeSTRING;
			at10.rep_.name_ = "location_full_name";
			at10.rep_.numChar_ = 255;

			if(!db_->addColumn("te_address_locator", at10.rep_))
			{
				errMsg_ = "Couldn't add location_full_name column to te_address_locator table!";

				return false;
			}			
		}


		errMsg_ = "The te_address_locator table already exist!";

		return false;
	}
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeINT;
	at1.rep_.name_ = "table_id";
	at1.rep_.isPrimaryKey_ = true;
	at1.rep_.isAutoNumber_ = false;
	attList.push_back(at1);	

	TeAttribute at2;
	at2.rep_.type_ = TeSTRING;
	at2.rep_.name_ = "initial_left_number";
	at2.rep_.numChar_ = 255;
	attList.push_back(at2);	

	TeAttribute at3;
	at3.rep_.type_ = TeSTRING;
	at3.rep_.name_ = "final_left_number";
	at3.rep_.numChar_ = 255;
	attList.push_back(at3);	

	TeAttribute at4;
	at4.rep_.type_ = TeSTRING;
	at4.rep_.name_ = "initial_right_number";
	at4.rep_.numChar_ = 255;
	attList.push_back(at4);	

	TeAttribute at5;
	at5.rep_.type_ = TeSTRING;
	at5.rep_.name_ = "final_right_number";
	at5.rep_.numChar_ = 255;
	attList.push_back(at5);

	TeAttribute at6;
	at6.rep_.type_ = TeSTRING;
	at6.rep_.name_ = "location_type";
	at6.rep_.numChar_ = 255;
	attList.push_back(at6);

	TeAttribute at7;
	at7.rep_.type_ = TeSTRING;
	at7.rep_.name_ = "location_title";
	at7.rep_.numChar_ = 255;
	attList.push_back(at7);

	TeAttribute at8;
	at8.rep_.type_ = TeSTRING;
	at8.rep_.name_ = "location_preposition";
	at8.rep_.numChar_ = 255;
	attList.push_back(at8);

	TeAttribute at9;
	at9.rep_.type_ = TeSTRING;
	at9.rep_.name_ = "location_name";
	at9.rep_.numChar_ = 255;
	attList.push_back(at9);

	TeAttribute at10;
	at10.rep_.type_ = TeSTRING;
	at10.rep_.name_ = "location_full_name";
	at10.rep_.numChar_ = 255;
	attList.push_back(at10);


	TeAttribute at11;
	at11.rep_.type_ = TeSTRING;
	at11.rep_.name_ = "left_neighborhood";
	at11.rep_.numChar_ = 255;
	attList.push_back(at11);

	TeAttribute at12;
	at12.rep_.type_ = TeSTRING;
	at12.rep_.name_ = "right_neighborhood";
	at12.rep_.numChar_ = 255;
	attList.push_back(at12);

	TeAttribute at13;
	at13.rep_.type_ = TeSTRING;
	at13.rep_.name_ = "left_zip_code";
	at13.rep_.numChar_ = 255;
	attList.push_back(at13);

	TeAttribute at14;
	at14.rep_.type_ = TeSTRING;
	at14.rep_.name_ = "right_zip_code";
	at14.rep_.numChar_ = 255;
	attList.push_back(at14);

	if(!db_->createTable("te_address_locator", attList))
	{
		errMsg_ = "Could not create te_address_locator table!";

		return false;
	}

	if(!db_->createRelation("address_locator_relation", "te_address_locator", "table_id", "te_layer_table", "table_id", true))
	{
		errMsg_ = "Referential integrity could not be stabilished beteween te_address_locator and te_layer_table tables!";
	
		return false;
	}

	return true;
}

bool TeAddressLocator::insertAddressMetadata(const TeAddressMetadata& metadata)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	string sql  = "INSERT INTO te_address_locator (table_id, initial_left_number, ";
	       sql += "final_left_number, initial_right_number, final_right_number, ";
		   sql += "location_type, location_title, location_preposition, location_name, location_full_name, left_neighborhood, right_neighborhood, ";
		   sql += "left_zip_code, right_zip_code) VALUES(";
		   sql += Te2String(metadata.tableId_);
		   sql += ", '";
		   sql += metadata.initialLeftNumber_;
		   sql += "', '";
		   sql += metadata.finalLeftNumber_;
		   sql += "', '";
		   sql += metadata.initialRightNumber_;
		   sql += "', '";
		   sql += metadata.finalRightNumber_;
		   sql += "', '";
		   sql += metadata.locationType_;
		   sql += "', '";
		   sql += metadata.locationTitle_;
		   sql += "', '";
		   sql += metadata.locationPreposition_;
		   sql += "', '";
		   sql += metadata.locationName_;
		   sql += "', '";
		   sql += metadata.locationCompleteName_;
		   sql += "', '";
		   sql += metadata.leftNeighborhood_;
		   sql += "', '";
		   sql += metadata.rightNeighborhood_;
		   sql += "', '";
		   sql += metadata.leftZipCode_;
		   sql += "', '";
		   sql += metadata.rightZipCode_;
		   sql += "')";

	if(!db_->execute(sql))
	{
		errMsg_ = "Could not insert metadata information about geocoding base table!";

		return false;
	}	

	return loadAddressMetadata(metadata.tableId_);
	
	//if(!metadata.leftZipCode_.empty())
	//	removeHifen_ = verifyHifen(metadata_.tableName_, metadata.leftZipCode_);

	//return true;
}

bool TeAddressLocator::updateAddressMetadata(const TeAddressMetadata& metadata)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	//if(metadata_.tableId_ != metadata.tableId_)
	//	return false;	// Doesn't update a diferent attribute table....

	string sql  = "UPDATE te_address_locator SET initial_left_number = '";
	       sql += metadata.initialLeftNumber_;
		   sql += "', final_left_number = '";
		   sql += metadata.finalLeftNumber_;
		   sql += "', initial_right_number = '";
		   sql += metadata.initialRightNumber_;
		   sql += "', final_right_number = '";
		   sql += metadata.finalRightNumber_;
		   sql += "', location_type = '";
		   sql += metadata.locationType_;
		   sql += "', location_title = '";
		   sql += metadata.locationTitle_;
		   sql += "', location_preposition = '";
		   sql += metadata.locationPreposition_;
		   sql += "', location_name = '";
		   sql += metadata.locationName_;
		   sql += "', location_full_name = '";
		   sql += metadata.locationCompleteName_;
		   sql += "', left_neighborhood = '";
		   sql += metadata.leftNeighborhood_;
		   sql += "', right_neighborhood = '";
		   sql += metadata.rightNeighborhood_;
		   sql += "', left_zip_code = '";
		   sql += metadata.leftZipCode_;
		   sql += "', right_zip_code = '";
		   sql += metadata.rightZipCode_;
		   sql += "' WHERE table_id = ";
		   sql += Te2String(metadata.tableId_);	

	if(!db_->execute(sql))
	{
		errMsg_ = "Could not update metadata information about geocoding base table!";

		return false;
	}

	return loadAddressMetadata(metadata.tableId_);

	//if(!metadata.leftZipCode_.empty())
	//	removeHifen_ = verifyHifen(metadata_.tableName_, metadata.leftZipCode_);	

	//return true;
}

bool TeAddressLocator::deleteAddressMetadata(const int& tableId)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	string sql  = "DELETE FROM te_address_locator WHERE table_id = ";
	       sql += Te2String(tableId);

	if(db_->execute(sql))
		return true;
	else
	{
		errMsg_ = "Could not remove metadata information about geocoding base table!";

		return false;
	}
}

bool TeAddressLocator::loadAddressMetadata(const int& tableId)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	isMetadataLoaded_ = false;

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT te_address_locator.*, te_layer_table.attr_table, te_layer_table.attr_link, te_layer.layer_id";
		   sql += "  FROM te_address_locator, te_layer_table, te_layer ";
		   sql += " WHERE  te_layer_table.layer_id = te_layer.layer_id ";
		   sql += "        AND ";
		   sql += "        te_address_locator.table_id = te_layer_table.table_id ";
		   sql += "        AND ";
		   sql += "        te_address_locator.table_id = ";
	       sql += Te2String(tableId);

	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;

		errMsg_ = "Could not find metadata for geocoding base table!";

		return false;
	}

	fillAddressMetadata(metadata_, portal);

	int layerId = portal->getInt("layer_id");

	referenceLayer_ = db_->layerMap()[layerId];

	delete portal;

	if(referenceLayer_ == 0)
	{
		errMsg_ = "Could not find infolayer to be used as base for geocoding!";
				
		return false;
	}

	isMetadataLoaded_ = true;


	if(!metadata_.leftZipCode_.empty())
		removeHifen_ = !verifyHifen(metadata_.tableName_, metadata_.leftZipCode_);

	return true;
}

bool TeAddressLocator::setAddressLocatorMetadata(const TeAddressMetadata& m)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT te_layer.layer_id";
		   sql += "  FROM te_layer_table, te_layer ";
		   sql += " WHERE te_layer_table.layer_id = te_layer.layer_id ";
		   sql += "       AND ";
		   sql += "       te_layer_table.table_id = ";
	       sql += Te2String(m.tableId_);

	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;

		errMsg_ = "Could not find layer for geocoding base table!";

		return false;
	}

	int layerId = portal->getInt("layer_id");

	referenceLayer_ = db_->layerMap()[layerId];

	if(referenceLayer_ == 0)
	{
		errMsg_ = "Could not find infolayer to be used as base for geocoding!";
				
		return false;
	}

	metadata_ = m;

	isMetadataLoaded_ = true;

	if(!metadata_.leftZipCode_.empty())
		removeHifen_ = !verifyHifen(metadata_.tableName_, metadata_.leftZipCode_);

	return true;
}

bool TeAddressLocator::getAllAddressMetadataLayer(const int& layerId, vector<TeAddressMetadata>& addVec)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT te_address_locator.*, te_layer_table.attr_table, te_layer_table.attr_link, te_layer.layer_id ";
		   sql += "  FROM te_address_locator, te_layer_table, te_layer ";
		   sql += " WHERE te_layer_table.layer_id = te_layer.layer_id ";
		   sql += "       AND ";
		   sql += "       te_address_locator.table_id = te_layer_table.table_id ";
		   sql += "       AND ";
		   sql += "       te_layer_table.layer_id = "; 
	       sql += Te2String(layerId);
		   sql += " ORDER BY te_layer_table.table_id";

	if(!portal->query(sql))
	{
		delete portal;

		return false;
	}

	while(portal->fetchRow())
	{
		TeAddressMetadata m;

		this->fillAddressMetadata(m, portal);		

		addVec.push_back(m);
	}

	delete portal;

	return true;
}

bool TeAddressLocator::getAllAddressMetadataTheme(const int& themeId, vector<TeAddressMetadata>& addVec)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT te_address_locator.*, te_layer_table.attr_table, te_layer_table.attr_link, te_layer.layer_id";
	       sql += "  FROM te_theme_table, te_layer_table, te_address_locator, te_layer ";
		   sql += " WHERE te_layer_table.layer_id = te_layer.layer_id ";
		   sql += "       AND ";
		   sql += "       te_layer_table.table_id = te_address_locator.table_id ";
		   sql += "       AND ";
		   sql += "       te_theme_table.table_id = te_layer_table.table_id "; 
		   sql += "       AND ";
		   sql += "       te_theme_table.theme_id = ";
	       sql += Te2String(themeId);
		   sql += " ORDER BY te_theme_table.table_id";

	if(!portal->query(sql))
	{
		delete portal;

		return false;
	}

	while(portal->fetchRow())
	{
		TeAddressMetadata m;

		this->fillAddressMetadata(m, portal);		

		addVec.push_back(m);
	}

	delete portal;

	return true;
}

bool TeAddressLocator::getAddressLocatorLayers(vector<pair<int, string> >& layerNameVec)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT DISTINCT te_layer.layer_id, te_layer.name ";
	       sql += "  FROM te_address_locator, te_layer_table, te_layer ";
		   sql += " WHERE te_layer.layer_id = te_layer_table.layer_id ";
		   sql += "       AND ";
		   sql += "       te_address_locator.table_id = te_layer_table.table_id "; 
		   sql += " ORDER BY te_layer.name ASC";

	if(!portal->query(sql))
	{
		delete portal;

		return false;
	}

	while(portal->fetchRow())
	{
    pair<int, string> d(atoi(portal->getData(0)), std::string(portal->getData(1)));
		layerNameVec.push_back(d);
	}

	delete portal;

	return true;
}

bool TeAddressLocator::getAddressLocatorLayerTables(vector<pair<int, string> >& tableNameVec, const int& layerId)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string sql  = "SELECT te_layer_table.table_id, te_layer_table.attr_table ";
	       sql += "  FROM te_address_locator, te_layer_table ";
		   sql += " WHERE te_layer_table.table_id = te_address_locator.table_id";
		   sql += "       AND ";
		   sql += "       te_layer_table.layer_id = ";
		   sql += Te2String(layerId);
		   sql += " ORDER BY te_layer_table.attr_table ASC";

	if(!portal->query(sql))
	{
		delete portal;

		return false;
	}

	while(portal->fetchRow())
	{
		pair<int, string> d(atoi(portal->getData(0)), portal->getData(1));
		tableNameVec.push_back(d);
	}

	delete portal;

	return true;
}

bool TeAddressLocator::findGeocodingTables(vector<pair<int, string> >& tableNames)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	string sql  = "SELECT * FROM te_layer_table WHERE attr_table_type = ";
	       sql += Te2String(TeGeocodingData);
		   sql += " ORDER BY attr_table";

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;	

	if(!portal->query(sql))
	{
		delete portal;

		return false;
	}

	while(portal->fetchRow())
	{
		pair<int, string> d(atoi(portal->getData("table_id")), std::string(portal->getData("attr_table")));
		tableNames.push_back(d);
	}

	delete portal;

	return true;
}


// 8    -> Found by similar search
// 9    -> Found by neighborhood or zip code

// 10	 -> Found by name location and number without type
// 11	 -> Found multiples by name location and number  without type
// 12	 -> Found by name location without number and type
// 13	 -> Found multiples by name location without number and type

// 14	 -> Found by name location and number without type and title
// 15	 -> Found multiples by name location and number  without type and title
// 16	 -> Found by name location without number and type and title
// 17	 -> Found multiples by name location without number and type and title

// 18	 -> Found by name location and number without type, title and preposition
// 19	 -> Found multiples by name location and number  without type, title and preposition
// 20	 -> Found by name location without number and type, title and preposition
// 21	 -> Found multiples by name location without number and type, title and preposition



int TeAddressLocator::findAddress(const TeAddress& address, vector<TeAddressDescription>& addressesFound, const bool& doExaustiveSearch)
{
	TeAddress addAux = address;

	string fullName = "";

	normalizeAddress(addAux, fullName);

	if(fullName.empty())
		return 2;

	// Encontra exato com número
	int result = findExact(addAux, fullName, true, addressesFound);

	if(result > 3)
		return result;

	// Encontra exato sem número
	result = findExact(addAux, fullName, false, addressesFound);

	if(result > 3)
		return result;

	// Encontrar com a ajuda do dicionário sem número
	if(!dictionary_.empty())
	{
		// Dicionário com número
		result = findWithDictionary(addAux, fullName, true, addressesFound);

		if(result > 3)
			return result;


		// Dicionário sem número
		result = findWithDictionary(addAux, fullName, false, addressesFound);

		if(result > 3)
			return result;
	}

	// ***********************************
	// * Try to search by a similar name *
	// ***********************************
	TeAddressDescription descript;

	if((addAux.locationNumber_ != 0) && findSimilarAddress(addAux, fullName, descript))
	{
		addressesFound.clear();
		addressesFound.push_back(descript);

		return 8;
	}

	if(doExaustiveSearch)
	{
		// Tenta remover o tipo
		if(!typesVector_.empty())
		{
			result = findWithoutAllName(addAux, fullName, addressesFound, true);

			switch(result)
			{
				case 4 :	return 10;
				case 5 :	return 11;
				case 6 :    return 12;
				case 7 :    return 13;
			}
		}


		// Tenta remover titulo
		if(!titlesVector_.empty())
		{
			result = findWithoutAllName(addAux, fullName, addressesFound, true, true);

			switch(result)
			{
				case 4 :	return 14;
				case 5 :	return 15;
				case 6 :    return 16;
				case 7 :    return 17;
			}
		}


		// Tenta remover preposicao
		if(!prepositionsVector_.empty())
		{
			result = findWithoutAllName(addAux, fullName, addressesFound, true, true, true);

			switch(result)
			{
				case 4 :	return 18;
				case 5 :	return 19;
				case 6 :    return 20;
				case 7 :    return 21;
			}
		}
	}

	
	// *****************************************
	// * Try to match zip code or neighborhood *
	// *****************************************
	if(cmetadata_.centroidLayer_)
	{
		if(findByCentroid(addAux, addressesFound))
		{
			if(addressesFound.size() > 1)
				return 1;
			else
				return 9;
		}
	}

	return result;
}


int TeAddressLocator::findWithDictionary(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound)
{
	// Quebrar o nome em tokens
	vector<string> tokens;
	tokenize(fullName, tokens);


	// Para cada token, procurar um sinônimo
	for(unsigned int i = 0; i < tokens.size(); ++i)
	{
		pair<TeGeocoderDictionary::iterator, TeGeocoderDictionary::iterator> itRange = dictionary_.equal_range(tokens[i]);

		while(itRange.first != itRange.second)
		{
			// Monta o nome para pesquisa
			string str = "";

			for(unsigned int j = 0; j < tokens.size(); ++j)
			{
				if(j != 0)
					str += " ";

				if(j == i)
					str += (itRange.first)->second;
				else
					str += tokens[j];
			}

			// Fazer a pesquisa exata
			int res = findExact(address, str, withNumber, addressesFound);

			if(res > 3)
				return res;

			++(itRange.first);
		}
	}

	return 1;
}

int TeAddressLocator::findWithoutAllName(const TeAddress& address, const string& fullName, vector<TeAddressDescription>& addressesFound, const bool& disabletype, const bool& disabletitle, const bool& disablepreposition)
{
	// Quebrar o nome em tokens
	vector<string> tokens;
	tokenize(fullName, tokens);

	string str = "";

	bool typeYetDisabled = false;
	bool titleYetDisabled = false;
	bool prepositionYetDisabled = false;

	unsigned int i = 0;

	// Ve se precisa eliminar algum token e já tenta sem usar dicionário
	for(i = 0; i < tokens.size(); ++i)
	{
		string aux = tokens[i];

		// verifica se o string será eliminado caso o tipo esteja desabilitado
		if(disabletype)
		{
			if(binary_search(typesVector_.begin(), typesVector_.end(), aux))
			{
				aux = "";
				typeYetDisabled = true;
				continue;
			}
		}

		if(disabletitle)
		{
			if(binary_search(titlesVector_.begin(), titlesVector_.end(), aux))
			{
				aux = "";
				titleYetDisabled = true;
				continue;
			}

		}

		if(disablepreposition && !prepositionYetDisabled)
		{
			if(binary_search(prepositionsVector_.begin(), prepositionsVector_.end(), aux))
			{
				aux = "";
				prepositionYetDisabled = true;
				continue;
			}
		}

		if(!str.empty())
			str += " ";

		str += aux;
	}

	int res = 1;

	if(typeYetDisabled || titleYetDisabled || prepositionYetDisabled)
	{
		res = findPattern(address, str, true, addressesFound, disabletype, disabletitle, disablepreposition);

		if(res > 3)
			return res;
	}

	if(typeYetDisabled || titleYetDisabled || prepositionYetDisabled)
	{
		res = findPattern(address, str, false, addressesFound, disabletype, disabletitle, disablepreposition);

		if(res > 3)
			return res;
	}

	if(dictionary_.empty())
		return 1;

	// Para cada token, procurar um sinônimo
	for(i = 0; i < tokens.size(); ++i)
	{
		pair<TeGeocoderDictionary::iterator, TeGeocoderDictionary::iterator> itRange = dictionary_.equal_range(tokens[i]);


		while(itRange.first != itRange.second)
		{
			// Monta o nome para pesquisa
			str = "";
			typeYetDisabled = false;
			titleYetDisabled = false;
			prepositionYetDisabled = false;


			for(unsigned int j = 0; j < tokens.size(); ++j)
			{
				string aux = "";

				if(j == i)
				{
					aux = (itRange.first)->second;
				}
				else
				{
					aux = tokens[j];
				}

				// verifica se o string será eliminado caso o tipo esteja desabilitado
				if(disabletype)
				{
					if(binary_search(typesVector_.begin(), typesVector_.end(), aux))
					{
						aux = "";
						typeYetDisabled = true;
						continue;
					}
				}

				if(disabletitle)
				{
					if(binary_search(titlesVector_.begin(), titlesVector_.end(), aux))
					{
						aux = "";
						titleYetDisabled = true;
						continue;
					}

				}

				if(disablepreposition && !prepositionYetDisabled)
				{
					if(binary_search(prepositionsVector_.begin(), prepositionsVector_.end(), aux))
					{
						aux = "";
						prepositionYetDisabled = true;
						continue;
					}
				}

				if(!str.empty())
					str += " ";

				str += aux;
			}

			if(typeYetDisabled || titleYetDisabled || prepositionYetDisabled)
			{
				res = findPattern(address, str, true, addressesFound, disabletype, disabletitle, disablepreposition);

				if(res > 3)
					return res;
			}

			if(typeYetDisabled || titleYetDisabled || prepositionYetDisabled)
			{
				res = findPattern(address, str, false, addressesFound, disabletype, disabletitle, disablepreposition);

				if(res > 3)
					return res;
			}

			++(itRange.first);
		}
	}

	return 1;
}

// 0	 -> Database connection problem
// 1     -> Address not found
// 3     -> Error on query the database
// 4	 -> Found by name location and number
// 5	 -> Found multiples by name location and number
// 6	 -> Found by name location without number 
// 7	 -> Found multiples by name location without number
int TeAddressLocator::findExact(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound)
{
	TeAddress addressToFind = address;

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	// The status of a query
	bool executeQuery = false;

	// String with the result query expression
	string strSQL = "";

	// String with the select...from clause
	string strSelect  = "SELECT * FROM ";
	       strSelect += metadata_.tableName_;
		   
	// String with the where clause;
	string strWhere = " WHERE ";

	// **************************************************
	// * Try to match name an number                    *
	// **************************************************		
	{
		int auxNumber = addressToFind.locationNumber_ ;

		if(!withNumber)
			addressToFind.locationNumber_ = 0;

		strWhere += mountExactWhereSQL(addressToFind, fullName);

		strSQL = strSelect + strWhere;

		queryAddresses(strSQL, executeQuery, addressesFound);

		if(addressesFound.size() > 0)
		{
			if((addressesFound.size() > 1) && (!addressToFind.zipCode_.empty()))
				this->refineFoundedLocationsByZipCode(addressToFind, addressesFound);

			if((addressesFound.size() > 1) && (!addressToFind.neighborhood_.empty()))
				this->refineFoundedLocationsByNeighBorhood(addressToFind, addressesFound);

			if((addressesFound.size() > 1) && (!metadata_.leftZipCode_.empty() || !metadata_.rightZipCode_.empty()) && (addressToFind.locationNumber_ == 0))
			{
				addressToFind.locationNumber_ = auxNumber;

				refineFoundedLocationsByNumberZipCode(addressToFind, addressesFound);

				addressToFind.locationNumber_ = 0;
			}
			else if((addressesFound.size() > 1) && (addressToFind.locationNumber_ == 0))
			{
				addressToFind.locationNumber_ = auxNumber;

				refineFoundedLocationsByNumberNeighboor(addressToFind, addressesFound);

				addressToFind.locationNumber_ = 0;
			}

	
			if(addressesFound.size() > 1)
			{
				if(addressToFind.locationNumber_ == 0)
					return 7;
				else
					return 5;							
			}
			else
			{
				if(addressToFind.locationNumber_ == 0)
					return 6;
				else
					return 4;
			}
		}
	}

	// ***************************************************************************
	// * If we are here so we did not find any address or the queries were wrong *
	// ***************************************************************************
	if(executeQuery)
		return 1;	// If at leat one query was executed, return not found any address
	else
		return 3;	// Otherwise return an error on all queries
}

int TeAddressLocator::findPattern(const TeAddress& address, const string& fullName, const bool& withNumber, vector<TeAddressDescription>& addressesFound, const bool& disabletype, const bool& disabletitle, const bool& disablepreposition)
{
	TeAddress addressToFind = address;

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	// The status of a query
	bool executeQuery = false;

	// String with the result query expression
	string strSQL = "";

	// String with the select...from clause
	string strSelect  = "SELECT * FROM ";
	       strSelect += metadata_.tableName_;
		   
	// String with the where clause;
	string strWhere = " WHERE ";

	// **************************************************
	// * Try to match name an number                    *
	// **************************************************		
	{
		int auxNumber = addressToFind.locationNumber_ ;

		if(!withNumber)
			addressToFind.locationNumber_ = 0;

		if(!metadata_.locationType_.empty() || !metadata_.locationTitle_.empty() || !metadata_.locationPreposition_.empty())
			strWhere += mountPatternWhereSQL(addressToFind, fullName, disabletype, disabletitle, disablepreposition);
		else
			strWhere += mountLikeWhereSQL(addressToFind, fullName);

		strSQL = strSelect + strWhere;

		queryAddresses(strSQL, executeQuery, addressesFound);

		if(addressesFound.size() > 0)
		{
			if(!metadata_.locationType_.empty() || !metadata_.locationTitle_.empty() || !metadata_.locationPreposition_.empty())
				cleanAddressesFound(fullName, addressesFound, disabletype, disabletitle, disablepreposition);

			if(addressesFound.size() > 0)
			{
				if((addressesFound.size() > 1) && (!addressToFind.zipCode_.empty()))
					this->refineFoundedLocationsByZipCode(addressToFind, addressesFound);

				if((addressesFound.size() > 1) && (!addressToFind.neighborhood_.empty()))
					this->refineFoundedLocationsByNeighBorhood(addressToFind, addressesFound);

				if((addressesFound.size() > 1) && (!metadata_.leftZipCode_.empty() || !metadata_.rightZipCode_.empty()) && (addressToFind.locationNumber_ == 0))
				{
					addressToFind.locationNumber_ = auxNumber;

					refineFoundedLocationsByNumberZipCode(addressToFind, addressesFound);

					addressToFind.locationNumber_ = 0;
				}
				else if((addressesFound.size() > 1) && (addressToFind.locationNumber_ == 0))
				{
					addressToFind.locationNumber_ = auxNumber;

					refineFoundedLocationsByNumberNeighboor(addressToFind, addressesFound);

					addressToFind.locationNumber_ = 0;
				}

		
				if(addressesFound.size() > 1)
				{
					if(addressToFind.locationNumber_ == 0)
						return 7;
					else
						return 5;							
				}
				else
				{
					if(addressToFind.locationNumber_ == 0)
						return 6;
					else
						return 4;
				}
			}
		}
	}

	// ***************************************************************************
	// * If we are here so we did not find any address or the queries were wrong *
	// ***************************************************************************
	if(executeQuery)
		return 1;	// If at leat one query was executed, return not found any address
	else
		return 3;	// Otherwise return an error on all queries
}


bool TeAddressLocator::addressGeocode(TeGeocodeTableMetadata& tableMetadata, 
									  const string& newLayerName, 
									  const string& reportName, 
									  const string& reportDir, 
									  const bool& doExaustive, 
									  TeAbstractAddressNotifyFunctor* /* notifyFunction */)
{
	errMsg_ = "";

	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	if(tableMetadata.locationNameColumnName_.empty())
	{
		errMsg_ = "The name field must be filled!";
		
		return false;
	}

	// Verifica se o metadado para o processo de espacialização foi previamente carregado
	if(!isMetadataLoaded())
	{
		errMsg_ = "Load metadata for geocoding layer before!";
		
		return false;
	}

	// Verifica se o metadado para o processo de espacialização foi previamente carregado
	if(metadata_.locationCompleteName_.empty())
	{
		errMsg_ = "Please, check the configuration for geocoding layer before! The complete name is empty!";
		
		return false;
	}



	// Carrega os tipos, preposições e títulos existentes na base para poder 
	// realizar substituições nos nomes de endereços no caso da pesquisa alternativa
	if(!loadAddressCompoundNames())
	{	
		return false;
	}	

	// Verifica se os metadados dos layers foram carregados
	if(db_->layerMap().empty())
	{
		if(!db_->loadLayerSet())
		{
			errMsg_ = "Could not load infolayers metadata!";
		
			return false;
		}
	}

	// Verifica se já existe um layer com o nome escolhido
	if(db_->layerExist(newLayerName))
	{
		errMsg_ = "There is another infolayer with the same name!";
		
		return false;
	}

	// Tenta acrescentar as duas colunas de relatório
	pair<string, string> newCols;
	if(!addReportColumn(tableMetadata, newCols))
		return false;	

	// Recupera as informações da tabela a ser geocodificada
	if(!findTableMetadata(tableMetadata))
		return false;
	
	// Tenta pegar o portal para a tabela a ser geocodificada
	int nlo = 0;
	TeDatabasePortal* portal = getAddressTablePortal(tableMetadata, nlo);

	if(!portal)
		return false;

	// Cria um novo layer baseado no layer selecionado
	TeLayer* newLayer = new TeLayer(newLayerName, db_, referenceLayer_->box(), referenceLayer_->projection());

	// Se não conseguir criar a representação de pontos, aborta a operação.
	if(!newLayer->addGeometry(TePOINTS))
	{
		errMsg_ = "Could not add point representation for new layer!";

		db_->deleteLayer(newLayer->id());

		//delete newLayer;

		delete portal;
		
		return false;
	}

	// Navega pelos registros da tabela a ser espacializada determinando os endereços
	TePointSet pSet;

	int lo = 0;

	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nlo);
		TeProgress::instance()->setCaption("Address Geocoding");
		TeProgress::instance()->setMessage("Geocoding addresses...      ");
	}

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;

	string reportText  = "===> FINAL REPORT OF ADDRESS GEOCODING PROCESS <===\n";
	       reportText += "     =========================================     \n";
		   reportText += "\n\n\n";		   

		   reportText += "     ---------------------------------\n\n";
		   reportText += "SEARCHED ADDRESS|FOUND ADDRESS\n";
		   reportText += "--------------------------------------------------------------------------------\n";
		   reportText += "GEOCODING RESULT|UNIQUE ID|TYPE|TITLE|PREPOSITION|LOCATION NAME|NUMBER|NEIGHBOR|ZIPCODE|FOUND ADDRESS====>|UNIQUE ID|TYPE|TITLE|PREPOSITION|LOCATION NAME|INITIAL LEFT|FINAL LEFT|INITIAL RIGHT|FINAL RIGHT|FOUND LEFT NEIGHBOOR|FOUND RIGHT NEIGHBOR|FOUND LEFT ZIPCODE|FOUND RIGHT ZIPCODE\n";

	string reportSummary  = "===> FINAL REPORT OF ADDRESS GEOCODING PROCESS <===\n";
	       reportSummary += "     =========================================     \n";
		   reportSummary += "\n\n\n";		   


	string reportFileSummary = reportDir + "/" + reportName + "_summary.txt";
	string reportFile        = reportDir + "/" + reportName + ".txt";	

	TeWriteToFile(reportFile, reportText, "w");
	TeWriteToFile(reportFileSummary, reportSummary, "w");

	vector<int> summarySum;
	unsigned int summaryLen = 22;
	for(unsigned int i = 0; i < summaryLen; ++i)
		summarySum.push_back(0);


	do
	{
		if(TeProgress::instance() && TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			errMsg_ = "The process was canceled by user!";

			db_->deleteLayer(newLayer->id());

			delete portal;

			return false;
		}

		TeAddress add;

		fillAddress(add, tableMetadata, portal);

		++lo;

		vector<TeAddressDescription> addressesFound;

		int status = findAddress(add, addressesFound, doExaustive);

		summarySum[status] = summarySum[status] + 1;

		string reportLine  = Te2String(status);
		       reportLine += "|";

		// SEARCHED LOCATION
		reportLine += add.locationLink_;
		reportLine += "|";
		reportLine += add.locationType_;
		reportLine += "|";
		reportLine += add.locationTitle_;
		reportLine += "|";
		reportLine += add.locationPreposition_;
		reportLine += "|";
		reportLine += add.locationName_;
		reportLine += "|";
		reportLine += Te2String(add.locationNumber_);			
		reportLine += "|";			
		reportLine += add.neighborhood_;
		reportLine += "|";
		reportLine += add.zipCode_;
		reportLine += "|***********|";

			
		if(status >= 4)	// Encontrou algum endereço?
		{
			TeAddressDescription addDescSimilar = addressesFound[0];
			
			if(status != 9)
				findCoordinate(add, addDescSimilar);

			// REPORT LINE


			// FOUNDED LOCATION
			reportLine += addDescSimilar.locationLink_;
		    reportLine += "|";
		    reportLine += addDescSimilar.locationType_;
			reportLine += "|";
			reportLine += addDescSimilar.locationTitle_;
			reportLine += "|";
			reportLine += addDescSimilar.locationPreposition_;
			reportLine += "|";
			reportLine += (addDescSimilar.locationName_.empty()) ? addDescSimilar.locationCompleteName_ : addDescSimilar.locationName_ ;
			reportLine += "|";
			reportLine += Te2String(addDescSimilar.initialLeftNumber_);
			reportLine += "|";
			reportLine += Te2String(addDescSimilar.finalLeftNumber_);
			reportLine += "|";
			reportLine += Te2String(addDescSimilar.initialRightNumber_);
			reportLine += "|";
			reportLine += Te2String(addDescSimilar.finalRightNumber_);
			reportLine += "|";
			reportLine += addDescSimilar.leftNeighborhood_;
			reportLine += "|";
			reportLine += addDescSimilar.rightNeighborhood_;
			reportLine += "|";
			reportLine += addDescSimilar.leftZipCode_;
			reportLine += "|";
			reportLine += addDescSimilar.rightZipCode_;
			reportLine += "\n";						

			if(addDescSimilar.isValid_)
			{
				// Cria um ponto e o adiciona no pointset
				TePoint	pt;
				pt.add(addDescSimilar.pt_);
				pt.objectId(add.locationLink_);
				pSet.add(pt);
			}
			else
				status = -1;
		}
		else
		{
			reportLine += " | | | | | | | | | | | | \n";
		}
		
		TeWriteToFile(reportFile, reportLine, "a");

		this->updateAddressTable(newCols.first, status, tableMetadata, add.locationLink_);

		if(pSet.size() > 100)
		{
			if(!newLayer->addPoints(pSet))
			{
				errMsg_ = "Could not insert generated points!";

				db_->deleteLayer(newLayer->id());

				delete portal;

				return false;
			}

			pSet.clear();
		}

		if(TeProgress::instance())
		{
			t2 = clock();
			if(int(t2-t1) > dt)
			{
				t1 = t2;
				
				if((int)(t2-t0) > dt2)
					TeProgress::instance()->setProgress(lo);
			}
		}
	}while(portal->fetchRow());	// Vai para o próximo registro

	if((pSet.size() > 0) && !newLayer->addPoints(pSet))
	{
		errMsg_ = "Could not insert generated points!";

		db_->deleteLayer(newLayer->id());

		delete portal;

		return false;
	}


	delete portal;

	string summary  = "\n=================";
		   summary += "\n===> SUMMARY <===";
		   summary += "\n=================\n";
		   summary += "\nNumber of ocurrences of each code:\n";

	int sum = 0;
	for(unsigned int j = 0; j < summarySum.size(); ++j)
	{
		summary += "CODE " + Te2String(j);
		summary += " => ";
		summary += Te2String(summarySum[j]);
		summary += " - ";
		summary += Te2String((double(summarySum[j]) / double(lo)) * 100.0);
		summary += "\n";

		if(j > 3)
			sum = sum + summarySum[j];
	}

	summary += "=================\n";
	summary += "TOTAL DE ENDERECOS: " + Te2String(lo) + "\n";
	summary += "TOTAL DE ENCONTRADOS: " + Te2String(sum) + "\n";
	summary += "Resultado = " + Te2String((double(sum) / double(lo)) * 100.0) + "\n\n";
	summary += "LEGEND:\n";
	summary += "0  -> Database connection problem\n";
	summary += "1  -> Address not found\n";
	summary += "2  -> full name empty\n";
	summary += "3  -> Error on query the database\n";
	summary += "4  -> Found by name location and number\n";
	summary += "5  -> Found multiples by name location and number\n";
	summary += "6  -> Found by name location without number\n";
	summary += "7  -> Found multiples by name location without number\n";
	summary += "8  -> Found by similar search\n";
	summary += "9  -> Found by neighborhood or zip code\n";
	summary += "10 -> Found by name location and number without type\n";
	summary += "11 -> Found multiples by name location and number  without type\n";
	summary += "12 -> Found by name location without number and type\n";
	summary += "13 -> Found multiples by name location without number and type\n";
	summary += "14 -> Found by name location and number without type and title\n";
	summary += "15 -> Found multiples by name location and number  without type and title\n";
	summary += "16 -> Found by name location without number and type and title\n";
	summary += "17 -> Found multiples by name location without number and type and title\n";
	summary += "18 -> Found by name location and number without type, title and preposition\n";
	summary += "19 -> Found multiples by name location and number  without type, title and preposition\n";
	summary += "20 -> Found by name location without number and type, title and preposition\n";
	summary += "21 -> Found multiples by name location without number and type, title and preposition\n";


	TeWriteToFile(reportFileSummary, summary, "a");

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	// If the table is registered as an geocoding table, so we remove its information from te_layer_table
	string deleteGeocodingTable  = "DELETE FROM te_layer_table WHERE (layer_id IS NULL) AND table_id = ";
	       deleteGeocodingTable += Te2String(tableMetadata.tableId_);

	db_->execute(deleteGeocodingTable);

	// Tenta registrar a tabela selecionada no metadado do novo layer
	TeTable attTable(tableMetadata.tableName_);
	
	attTable.setTableType(TeAttrStatic);
	attTable.setLinkName(tableMetadata.linkColumnName_);
	attTable.setUniqueName(tableMetadata.keyColumnName_);

	if(!newLayer->createAttributeTable(attTable))
	{
		errMsg_ = "Não foi possível atualizar as informações da tabela de atributos para o novo layer";

		db_->deleteLayer(newLayer->id());

		return false;
	}

	delete newLayer;

	return true;
}

bool TeAddressLocator::loadDictionary(const string& dictionaryFileName)
{	
	errMsg_ = "";

	dictionary_.clear();

	if(dictionaryFileName.empty())
	{
		errMsg_ = "Enter with a valid dictionary file name and location!";

		return false;	
	}

	try
	{
		TeAsciiFile dicFile(dictionaryFileName);

		while(dicFile.isNotAtEOF())
		{
			vector<string> strList;

			dicFile.readStringListCSV(strList, ',');

			dicFile.findNewLine();

			if(strList.size() <= 1)
				continue;
			
			string str1 = TeConvertToUpperCase(strList[0]);

			str1 = str1.substr(1);
			str1 = str1.substr(0, str1.size() - 1);

			for(unsigned int i = 1; i < strList.size(); ++i)
			{
				string str2 = TeConvertToUpperCase(strList[i]);
				str2 = str2.substr(1);
				str2 = str2.substr(0, str2.size() - 1);

				dictionary_.insert(TeGeocoderDictionary::value_type(str2, str1));
			}
		}

	}
	catch(...)
	{
		errMsg_ = "Could open file: " + dictionaryFileName;

		return false;
	}
	
	return true;
}



//PROTECTED MEMBERS
bool TeAddressLocator::findSimilarAddress(const TeAddress& address, const string& fullName, TeAddressDescription& descript)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	// String with the result query expression
	string strSQL = "";

	// String with the select...from clause
	string strSelect  = "SELECT * FROM ";
	       strSelect += metadata_.tableName_;
		   
	// Order by of the result query
	string strOrderBy  = " ORDER BY ";
		   strOrderBy += metadata_.linkColumnName_;

	// String with the where clause;
	string strWhere = " WHERE ";

	strWhere += mountSimilarWhereSQL(address);

	strSQL = strSelect + strWhere + strOrderBy;

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	bool found = false;

	// Check if there is a segment that satisfies the clause
	if(portal->query(strSQL))
	{
		double simil, similmax = 75.0;		

		while(portal->fetchRow() && similmax < 100.0)
		{

			string label = portal->getData(metadata_.locationCompleteName_);

			simil = TeSimilarity(fullName.c_str(), label.c_str());
		
			if(simil > similmax)
			{
				found = true;

				fillAddressDescription(descript, portal);

				similmax = simil;
			}
		}
	}

	delete portal;

	portal = 0;

	return found;
}



bool TeAddressLocator::findByCentroid(const TeAddress& addressToFind, vector<TeAddressDescription>& addressesFound)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	if(!cmetadata_.centroidLayer_ || cmetadata_.centroidName_.empty() || cmetadata_.attributeTableName_.empty())
		return false;	

	string strValue = (cmetadata_.fieldType_ == TeNEIGHBORHOOD) ? (addressToFind.neighborhood_)  : (addressToFind.zipCode_);

	if(strValue.empty())
		return false;

	strValue = TeConvertToUpperCase(strValue);
	
	if(removeHifenFromCentroid_ && cmetadata_.fieldType_ == TeZIPCODE)
		removeHifen(strValue);
	else if(!removeHifenFromCentroid_ && cmetadata_.fieldType_ == TeZIPCODE)
		insertHifen(strValue);


	string likeSQL = " = '";
	       likeSQL += strValue;
		   likeSQL += "'";

	TeTable table;
	
	if(!cmetadata_.centroidLayer_->getAttrTablesByName(cmetadata_.attributeTableName_, table))
		return false;
	       
	// String with the result query expression
	string strSQL  = "SELECT ";
		   strSQL += table.linkName();
		   strSQL += ", ";
	       strSQL += cmetadata_.centroidName_;
		   strSQL += " FROM ";
		   strSQL += cmetadata_.attributeTableName_;
		   strSQL += " WHERE ";
		   strSQL += db_->toUpper(cmetadata_.centroidName_);
		   strSQL += likeSQL;

	TeDatabasePortal* p = db_->getPortal();

	if(!p)
		return false;

	if(!p->query(strSQL))
	{
		delete p;

		return false;
	}

	while(p->fetchRow())
	{
		TeAddressDescription add;

		if(cmetadata_.centroidLayer_->hasGeometry(TePOINTS))
		{
			TePointSet pset;

			if(cmetadata_.centroidLayer_->loadGeometrySet(p->getData(0), pset))
			{
				add.pt_ = TeFindCentroid(pset);
				add.isValid_ = true;
			}
			else
				continue;
		}
		else if(cmetadata_.centroidLayer_->hasGeometry(TeLINES))
		{
			TeLineSet lset;

			if(cmetadata_.centroidLayer_->loadGeometrySet(p->getData(0), lset))
			{
				add.pt_ = TeFindCentroid(lset);
				add.isValid_ = true;
			}
			else
				continue;
		}
		else if(cmetadata_.centroidLayer_->hasGeometry(TePOLYGONS))
		{
			TePolygonSet pset;

			if(cmetadata_.centroidLayer_->loadGeometrySet(p->getData(0), pset))
			{
				add.pt_ = TeFindCentroid(pset);
				add.isValid_ = true;
			}
			else
				continue;

		}
		else if(cmetadata_.centroidLayer_->hasGeometry(TeCELLS))
		{
			TeCellSet cset;

			if(cmetadata_.centroidLayer_->loadGeometrySet(p->getData(0), cset))
			{
				add.pt_ = TeFindCentroid(cset);
				add.isValid_ = true;
			}
			else
				continue;
		}
		else
			continue;

		TeVectorRemap(add.pt_, cmetadata_.centroidLayer_->projection(), add.pt_, referenceLayer_->projection());

		if(cmetadata_.fieldType_ == TeZIPCODE)
		{
			add.leftZipCode_ = p->getData(1);
			add.rightZipCode_ = p->getData(1);
		}
		else
		{
			add.leftNeighborhood_ = p->getData(1);
			add.rightNeighborhood_ = p->getData(1);
		}

		addressesFound.push_back(add);
	}

	return addressesFound.size() > 0;
}


void TeAddressLocator::fillAddressMetadata(TeAddressMetadata& metadata, TeDatabasePortal* p)
{
	metadata.tableId_ = atoi(p->getData("table_id"));
	metadata.initialLeftNumber_ = p->getData("initial_left_number");
	metadata.finalLeftNumber_ = p->getData("final_left_number");
	metadata.initialRightNumber_ = p->getData("initial_right_number");
	metadata.finalRightNumber_ = p->getData("final_right_number");
	metadata.locationType_ = p->getData("location_type");
	metadata.locationTitle_ = p->getData("location_title");
	metadata.locationPreposition_ = p->getData("location_preposition");
	metadata.locationName_ = p->getData("location_name");
	metadata.locationCompleteName_ = p->getData("location_full_name");
	metadata.leftNeighborhood_ = p->getData("left_neighborhood");
	metadata.rightNeighborhood_ = p->getData("right_neighborhood");
	metadata.leftZipCode_ = p->getData("left_zip_code");
	metadata.rightZipCode_ = p->getData("right_zip_code");
	metadata.tableName_ = p->getData("attr_table");
	metadata.linkColumnName_ = p->getData("attr_link");
	return;
}

void TeAddressLocator::fillAddress(TeAddress& add, const TeGeocodeTableMetadata& tableMetadata, TeDatabasePortal* p)
{
	add.locationLink_ = p->getData(tableMetadata.linkColumnName_);

	add.locationType_ = (tableMetadata.locationTypeColumnName_.empty()) ? "" : p->getData(tableMetadata.locationTypeColumnName_);
	TeTrim(add.locationType_);

	add.locationTitle_ = (tableMetadata.locationTitleColumnName_.empty()) ? "" : p->getData(tableMetadata.locationTitleColumnName_);
	TeTrim(add.locationTitle_);
	
	add.locationPreposition_ = (tableMetadata.locationPrepositionColumnName_.empty()) ? "" : p->getData(tableMetadata.locationPrepositionColumnName_);
	TeTrim(add.locationPreposition_);

	add.locationName_ = p->getData(tableMetadata.locationNameColumnName_);		
	TeTrim(add.locationName_);

	add.neighborhood_ = (tableMetadata.neighborhoodColumnName_.empty()) ? "" : p->getData(tableMetadata.neighborhoodColumnName_);
	TeTrim(add.neighborhood_);

	add.zipCode_ = (tableMetadata.zipCodeColumnName_.empty()) ? "" : p->getData(tableMetadata.zipCodeColumnName_);
	TeTrim(add.zipCode_);

	if(!tableMetadata.locationNumberColumnName_.empty())
		add.locationNumber_ = atoi(p->getData(tableMetadata.locationNumberColumnName_));	
	else
		splitAddress(add);

	return;
}

string TeAddressLocator::mountSimilarWhereSQL(const TeAddress& address)
{	
	string sql = metadata_.locationName_ + " <> '' ";
	
	if(address.locationNumber_ != 0)
	{
		string snumber = Te2String(address.locationNumber_);

		if(!sql.empty())
			sql += " AND ";

		if((address.locationNumber_ % 2) == 0)
			sql += metadata_.initialRightNumber_ + " <= " + snumber + " AND " + metadata_.finalRightNumber_ + " >= " + snumber;
		else
			sql += metadata_.initialLeftNumber_ + " <= " + snumber +" AND " + metadata_.finalLeftNumber_ + " >= " + snumber;
	}

	return sql;
}

string TeAddressLocator::mountExactWhereSQL(const TeAddress& address, const string& fullName)
{
	vector<string> fields;
	string values = "";
	string sql = "";

	if(address.locationNumber_ != 0)
	{
		string snumber = Te2String(address.locationNumber_);

		// Se for endereços pares
		if((address.locationNumber_ % 2) == 0)
			sql += " ( " + metadata_.initialRightNumber_ +" <= " + snumber + " AND " + metadata_.finalRightNumber_ +" >= " + snumber + ") ";
		else	// Senão, se for ímpares
			sql += " ( " + metadata_.initialLeftNumber_ + " <= " + snumber + " AND " + metadata_.finalLeftNumber_ + " >= " + snumber + ") ";
	}

	if(sql.empty())
		sql += " ( " + metadata_.locationCompleteName_ + " = '";
	else
		sql += " AND (" + metadata_.locationCompleteName_ + " = '";

	sql += db_->escapeSequence(fullName);

	sql += "') ";

	return sql;
}

string TeAddressLocator::mountLikeWhereSQL(const TeAddress& address, const string& fullName)
{
	vector<string> fields;
	string values = db_->escapeSequence(fullName);
	string sql  = " ( " + metadata_.locationCompleteName_ + " LIKE '%";
           sql += values;
		   sql += "') ";

	if(address.locationNumber_ != 0)
	{
		string snumber = Te2String(address.locationNumber_);

		// Se for endereços pares
		if((address.locationNumber_ % 2) == 0)
			sql += " AND ( " + metadata_.initialRightNumber_ +" <= " + snumber + " AND " + metadata_.finalRightNumber_ +" >= " + snumber + ") ";
		else	// Senão, se for ímpares
			sql += " AND ( " + metadata_.initialLeftNumber_ + " <= " + snumber + " AND " + metadata_.finalLeftNumber_ + " >= " + snumber + ") ";
	}
	
	return sql;

}

string TeAddressLocator::mountPatternWhereSQL(const TeAddress& address, const string& fullName, const bool& disabletype, const bool& disabletitle, const bool& disablepreposition)
{
	vector<string> fields;
	string values = "";
	string sql = "(";

	if(!metadata_.locationType_.empty() && !disabletype)
		fields.push_back(db_->toUpper(metadata_.locationType_));

	if(!metadata_.locationTitle_.empty() && !disabletitle)
		fields.push_back(db_->toUpper(metadata_.locationTitle_));

	if(!metadata_.locationPreposition_.empty() && !disablepreposition)
		fields.push_back(db_->toUpper(metadata_.locationPreposition_));

	if(!metadata_.locationName_.empty())
		fields.push_back(db_->toUpper(metadata_.locationName_));

	sql += db_->concatValues(fields, "");
	sql += ") LIKE '";

	values += db_->escapeSequence(fullName);
	
	sql += makeLikeExpression(values);
	sql += "' ";

	if(address.locationNumber_ != 0)
	{
		string snumber = Te2String(address.locationNumber_);

		// Se for endereços pares
		if((address.locationNumber_ % 2) == 0)
			sql += " AND ( " + metadata_.initialRightNumber_ +" <= " + snumber + " AND " + metadata_.finalRightNumber_ +" >= " + snumber + ") ";
		else	// Senão, se for ímpares
			sql += " AND ( " + metadata_.initialLeftNumber_ + " <= " + snumber + " AND " + metadata_.finalLeftNumber_ + " >= " + snumber + ") ";
	}
	
	return sql;
}

void TeAddressLocator::findCoordinates(const TeAddress& add, vector<TeAddressDescription>& addresses)
{
	for(unsigned int i = 0; i < addresses.size(); ++i)
		findCoordinate(add, addresses[i]);
}

void TeAddressLocator::findCoordinate(const TeAddress& add, TeAddressDescription& desc)
{
	// Evaluate the relative position of "number" (relativePosition) between initial and final numbers
	// If number is zero, point to begin of line

	int initialLeft, initialRight, finalLeft, finalRight;

	double relativePosition = 0.0;

	if(add.locationNumber_ != 0)
	{
		if((add.locationNumber_ % 2) == 0)
		{
			initialRight = desc.initialRightNumber_;
			finalRight = desc.finalRightNumber_;
				
			if(initialRight != finalRight)
				relativePosition = double(add.locationNumber_ - initialRight) / double(finalRight - initialRight);
		}
		else
		{
			initialLeft = desc.initialLeftNumber_;
			finalLeft = desc.finalLeftNumber_;
				
			if(initialLeft != finalLeft)
				relativePosition = double(add.locationNumber_ - initialLeft) / double(finalLeft - initialLeft);
		}

		if(relativePosition > 1.0)
			relativePosition = 1.0;
		else if(relativePosition < 0.0)
			relativePosition = 0.0;
	}

	// Retrieve street segment geometry
	TeLineSet ls;

	if(!referenceLayer_->loadGeometrySet(desc.locationLink_, ls))	// Recupero a linha geométrica do trecho que contém o endereço em questão
		return;

	// Find the actual position of number on the street
	if(ls.size() >= 1)
	{
		TeLine2D line = ls[0];

		TeCoord2D pt1, pt2;

		pt1 = line[0];
		pt2 = line[1];

		double x, y;

		if(relativePosition <= 0.0)
		{
			x = pt1.x();
			y = pt1.y();
		}
		else if(relativePosition >= 1.0)
		{
			pt1 = line[line.size() - 1];

			x = pt1.x();
			y = pt1.y();
		}		
		else
		{
			if(line.size () > 2)
			{

				// Find the distance from number to the begin of street based on the relative position
				double targetLen = TeLength(line) * relativePosition;
				double acumLen = 0.0;
				double segLen;

				// Find in which segment number is comprised
				for(unsigned int i = 1 ; i < line.size(); ++i)
				{
					pt2 = line[i];

					segLen = TeDistance(pt2, pt1);	//sqrt((pt2.x() - pt1.x()) * (pt2.x() - pt1.x()) + (pt2.y() - pt1.y()) * (pt2.y() - pt1.y()));
					
					acumLen += segLen;

					if(acumLen >= targetLen)
					{
						double targetLenInSeg = targetLen - (acumLen - segLen);

						relativePosition = targetLenInSeg / segLen;

						break;
					}

					pt1 = pt2;
				}
			}

			x = pt1.x() + (pt2.x() - pt1.x()) * relativePosition;
			y = pt1.y() + (pt2.y() - pt1.y()) * relativePosition;
		}
		

		desc.pt_ = TeCoord2D (x,y);

		desc.isValid_ = true;
	}
	else
		desc.isValid_ = false;	// A linha pertencente ao trecho não foi encontrada... problemas sérios na base de dados!
}


void TeAddressLocator::fillAddressesDescription(vector<TeAddressDescription>& addresses, TeDatabasePortal* p)
{
	addresses.clear();

	if(p == 0)
		return;

	while(p->fetchRow())
	{
		TeAddressDescription descript;

		fillAddressDescription(descript, p);

		addresses.push_back(descript);
	}

	return;
}

void TeAddressLocator::fillAddressDescription(TeAddressDescription& descript, TeDatabasePortal* p)
{
	descript.locationLink_ = p->getData(metadata_.linkColumnName_);
	descript.locationType_ = (metadata_.locationType_.empty()) ? "" : p->getData(metadata_.locationType_);
	descript.locationTitle_ = (metadata_.locationTitle_.empty()) ? "" : p->getData(metadata_.locationTitle_);
	descript.locationPreposition_ = (metadata_.locationPreposition_.empty()) ? "" : p->getData(metadata_.locationPreposition_);
	descript.locationName_ = (metadata_.locationName_.empty()) ? "" : p->getData(metadata_.locationName_);		
	descript.locationCompleteName_ = p->getData(metadata_.locationCompleteName_);
	descript.initialLeftNumber_ = (metadata_.initialLeftNumber_.empty()) ? 0 : atoi(p->getData(metadata_.initialLeftNumber_));
	descript.finalLeftNumber_ = (metadata_.finalLeftNumber_.empty()) ? 0 : atoi(p->getData(metadata_.finalLeftNumber_));
	descript.initialRightNumber_ = (metadata_.initialRightNumber_.empty()) ? 0 : atoi(p->getData(metadata_.initialRightNumber_));
	descript.finalRightNumber_ = (metadata_.finalRightNumber_.empty()) ? 0 : atoi(p->getData(metadata_.finalRightNumber_));
	descript.leftNeighborhood_ = (metadata_.leftNeighborhood_.empty()) ? "" : p->getData(metadata_.leftNeighborhood_);
	descript.rightNeighborhood_ = (metadata_.rightNeighborhood_.empty()) ? "" : p->getData(metadata_.rightNeighborhood_);
	descript.leftZipCode_ = (metadata_.leftZipCode_.empty()) ? "" : p->getData(metadata_.leftZipCode_);
	descript.rightZipCode_ = (metadata_.rightZipCode_.empty()) ? "" : p->getData(metadata_.rightZipCode_);

	return;
}

void TeAddressLocator::normalizeAddress(TeAddress& address, string& completeName)
{
	completeName = "";

	if(!address.locationType_.empty())
	{
		TeTrim(address.locationType_);

		if(changeSpecialChars_)
			removeSpecialCharacters(address.locationType_);

		address.locationType_ = TeConvertToUpperCase(address.locationType_);

		completeName = address.locationType_;
	}

	if(!address.locationTitle_.empty())
	{
		TeTrim(address.locationTitle_);

		if(changeSpecialChars_)
			removeSpecialCharacters(address.locationTitle_);

		address.locationTitle_ = TeConvertToUpperCase(address.locationTitle_);

		if(!completeName.empty())
			completeName += " ";

		completeName += address.locationTitle_;
	}

	if(!address.locationPreposition_.empty())
	{
		TeTrim(address.locationPreposition_);

		if(changeSpecialChars_)
			removeSpecialCharacters(address.locationPreposition_);

		address.locationPreposition_ = TeConvertToUpperCase(address.locationPreposition_);

		if(!completeName.empty())
			completeName += " ";

		completeName += address.locationPreposition_;
	}
	
	if(!address.locationName_.empty())
	{
		TeTrim(address.locationName_);
		
		if(changeSpecialChars_)
			removeSpecialCharacters(address.locationName_);

		address.locationName_ = TeConvertToUpperCase(address.locationName_);

		if(!completeName.empty())
			completeName += " ";

		completeName += address.locationName_;
	}

	if(!address.neighborhood_.empty())
	{
		TeTrim(address.neighborhood_);

		if(changeSpecialChars_)
			removeSpecialCharacters(address.neighborhood_);

		address.neighborhood_ = TeConvertToUpperCase(address.neighborhood_);		
	}

	if(!address.zipCode_.empty())
	{
		TeTrim(address.zipCode_);

		if(removeHifen_)
			removeHifen(address.zipCode_);
		else
			insertHifen(address.zipCode_);
	}

	return;
}

void TeAddressLocator::removeHifen(string& zipCode) const
{
	if(zipCode.size() == 9)
	{
		unsigned int i = 0;

		string newZipCode = "";

		for(i = 0; i < 5; ++i)
			newZipCode += zipCode[i];

		for(i = 6; i < 9; ++i)
			newZipCode += zipCode[i];

		zipCode = newZipCode;
	}	
	
	return;
}

void TeAddressLocator::insertHifen(string& zipCode) const
{
	if(zipCode.size() == 8)
	{
		unsigned int i = 0;

		string newZipCode = "";

		for(i = 0; i < 5; ++i)
			newZipCode += zipCode[i];

		newZipCode += hifen_;

		for(i = 5; i < 8; ++i)
			newZipCode += zipCode[i];

		zipCode = newZipCode;
	}	
	
	return;
}

string TeAddressLocator::makeLikeExpression(const string& str) const
{
	string aux = "";

	for(unsigned int i = 0; i < str.size(); ++i)
		aux += (str[i] == ' ') ? ('%') : str[i];

	return aux;
}

void TeAddressLocator::removeSpecialCharacters(string& str) const
{
	string newStr = "";

	unsigned int nStep = str.size();

	for(unsigned int i = 0; i < nStep; ++i)
	{
		switch(str[i])
		{
			case 'Á' :  newStr += "A"; break;
			case 'á' :  newStr += "a"; break;
			case 'Â' :  newStr += "A"; break;
			case 'â' :  newStr += "a"; break;
			case 'Ã' :  newStr += "A"; break;
			case 'ã' :  newStr += "a"; break;
			case 'À' :  newStr += "A"; break;
			case 'à' :	newStr += "a"; break;

			case 'É' :  newStr += "E"; break;
			case 'é' :  newStr += "e"; break;
			case 'Ê' :  newStr += "E"; break;
			case 'ê' :  newStr += "e"; break;

			case 'Í' :  newStr += "I"; break;
			case 'í' :  newStr += "i"; break;

			case 'Ó' :  newStr += "O"; break;
			case 'ó' :  newStr += "o"; break;
			case 'Õ' :  newStr += "O"; break;
			case 'õ' :  newStr += "o"; break;

			case 'Ú' :  newStr += "U"; break;
			case 'ú' :  newStr += "u"; break;
			case 'Ü' :  newStr += "U"; break;
			case 'ü' :  newStr += "u"; break;

			case 'ç' :  newStr += "c"; break;
			case 'Ç' :  newStr += "C"; break;

			//case '.' :	break;

			default  :  newStr += str[i];
		}
	}

	str = newStr;
}

bool TeAddressLocator::verifyHifen(const string& tableName, const string& columnName)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	TeDatabasePortal* p = db_->getPortal();

	if(!p || columnName.empty() || tableName.empty())
		return false;

	string q  = "SELECT MAX(";
	       q += columnName;
		   q += ") FROM ";
		   q += tableName;

	if(!p->query(q) && !p->fetchRow())
	{
		delete p;

		return false;
	}

	string zipCode = p->getData(0);

	delete p;

	if(zipCode.size() == 9)
	{
		hifen_ = zipCode[5];

		return true;
	}

	return false;
}

bool TeAddressLocator::addReportColumn(TeGeocodeTableMetadata& tableMetadata, pair<string, string>& newColumns)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}
	
	// Adiciona as colunas para relatório de acerto dos pontos
	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;

	string allFieldQuery  = "SELECT * FROM ";
	       allFieldQuery += tableMetadata.tableName_;
		   allFieldQuery += " WHERE 1 = 2";

	if(!portal->query(allFieldQuery))
	{
		errMsg_ = "Could not find column names for the table to be geocoding!";

		delete portal;

		return false;
	}

	TeAttributeRep rep1;
	rep1.name_ = "geocoding_result_id";
	rep1.type_ = TeINT;
	rep1.isPrimaryKey_ = false;
	rep1.isAutoNumber_ = false;

	TeAttributeList& attrList = portal->getAttributeList();
	validColumnName(rep1, attrList);

	TeAttributeRep rep2;
	rep2.name_ = "geocoding_result_description";
	rep2.type_ = TeSTRING;
	rep2.numChar_ = 255;
	rep2.isPrimaryKey_ = false;
	rep2.isAutoNumber_ = false;

	validColumnName(rep2, attrList);

	delete portal;

	portal = 0;

	if(!db_->addColumn(tableMetadata.tableName_, rep1))
	{
		errMsg_ = "Could not add a column to do the report!";

		delete portal;

		return false;
	}	

	//if(!db_->addColumn(tableMetadata.tableName_, rep2))
	//{
	//	errMsg_ = "Could not add a column to do the report!";

	//	delete portal;

	//	return false;
	//}

	newColumns.first = rep1.name_;
	newColumns.second = rep2.name_;

	return true;
}

bool TeAddressLocator::findTableMetadata(TeGeocodeTableMetadata& tableMetadata)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}
	
	// Adiciona as colunas para relatório de acerto dos pontos
	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return false;


	// Procura o nome do campo chave da tabela a ser geocodificada
	string sql  = "SELECT * FROM te_layer_table WHERE table_id = ";
	       sql += Te2String(tableMetadata.tableId_);	

	portal = db_->getPortal();

	if(!portal)
		return false;

	if(!portal->query(sql) || !portal->fetchRow())
	{
		errMsg_ = "Could not read te_layer_table table for geocoding base table!";

		delete portal;

		return false;
	}

	tableMetadata.keyColumnName_ = portal->getData("unique_id");
	tableMetadata.linkColumnName_ = portal->getData("attr_link");
	tableMetadata.tableName_ = portal->getData("attr_table");

	delete portal;

	portal = 0;

	return true;
}

bool TeAddressLocator::updateAddressTable(const string& c1, const int& v1, TeGeocodeTableMetadata& tableMetadata, const string& objectId)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return false;
	}

	string sql  = "UPDATE ";
	       sql += tableMetadata.tableName_;
		   sql += " SET ";
		   sql += c1;
		   sql += " = ";
		   sql += Te2String(v1);
		   sql += " WHERE ";
		   sql += tableMetadata.linkColumnName_;
		   sql += " = '";
		   sql += objectId;
		   sql += "'";

	return db_->execute(sql);
}

TeDatabasePortal* TeAddressLocator::getAddressTablePortal(TeGeocodeTableMetadata& tableMetadata, int& numRows)
{
	if(!db_)
	{
		errMsg_ = "You should connect to a database first!";

		return 0;
	}

	string sql = "SELECT count(*) FROM " + tableMetadata.tableName_;

	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
	{
		errMsg_ = "Could not get a portal to query database!";

		return 0;
	}

	if(!portal->query(sql) || !portal->fetchRow())
	{
		errMsg_ = "Could not count the number of records in the geocoding base table!";

		return 0;
	}

	numRows = portal->getInt(0);

	portal->freeResult();

	// Recupera um portal para a tabela a ser espacializada e verifica se a tabela tem conteúdo
	sql  = "SELECT ";
	sql += tableMetadata.linkColumnName_;
	sql += ", ";
	sql += tableMetadata.locationNameColumnName_;

	if(!tableMetadata.locationNumberColumnName_.empty())
	{
	       sql += ", ";
	       sql += tableMetadata.locationNumberColumnName_;
	}

	if(!tableMetadata.neighborhoodColumnName_.empty())
	{
		sql += ", ";
		sql += tableMetadata.neighborhoodColumnName_;
	}

	if(!tableMetadata.zipCodeColumnName_.empty())
	{
		sql += ", ";
		sql += tableMetadata.zipCodeColumnName_;
	}

	if(!tableMetadata.locationTypeColumnName_.empty())
	{
		sql += ", ";
		sql += tableMetadata.locationTypeColumnName_;
	}

	if(!tableMetadata.locationTitleColumnName_.empty())
	{
		sql += ", ";
		sql += tableMetadata.locationTitleColumnName_;
	}

	if(!tableMetadata.locationPrepositionColumnName_.empty())
	{
		sql += ", ";
		sql += tableMetadata.locationPrepositionColumnName_;
	}

	sql += " FROM ";
	sql += tableMetadata.tableName_;

	if(!portal->query(sql) || !portal->fetchRow())
	{
		errMsg_ = "Could not read the geocoding base table!";

		delete portal;

		return 0;
	}

	return portal;
}



void TeAddressLocator::refineFoundedLocationsByNeighBorhood(const TeAddress& address, vector<TeAddressDescription>& addressesFound)
{
	unsigned int nStep = addressesFound.size();

	vector<TeAddressDescription> newAdds;

	for(unsigned int i = 0; i < nStep; ++i)
	{
		TeAddressDescription& addFound = addressesFound[i];

		if(address.neighborhood_ == TeConvertToUpperCase(addFound.leftNeighborhood_) || address.neighborhood_ == TeConvertToUpperCase(addFound.rightNeighborhood_))
			newAdds.push_back(addFound);
	}

	if(newAdds.size() > 0)
	{
		addressesFound.clear();

		addressesFound = newAdds;
	}

	return;
}

void TeAddressLocator::refineFoundedLocationsByZipCode(const TeAddress& address, vector<TeAddressDescription>& addressesFound)
{
	unsigned int nStep = addressesFound.size();

	vector<TeAddressDescription> newAdds;

	for(unsigned int i = 0; i < nStep; ++i)
	{
		TeAddressDescription& addFound = addressesFound[i];

		if(address.zipCode_ == addFound.leftZipCode_ || address.zipCode_ == addFound.rightZipCode_)
			newAdds.push_back(addFound);
	}

	if(newAdds.size() > 0)
	{
		addressesFound.clear();

		addressesFound = newAdds;
	}

	return;
}

void TeAddressLocator::refineFoundedLocationsByNumberNeighboor(const TeAddress& address, vector<TeAddressDescription>& addressesFound)
{
	unsigned int nStep = addressesFound.size();

	// id/dist
	typedef pair<unsigned int, int> nearestItem; 

	map<string, nearestItem> nearestMap;

	bool par = ((address.locationNumber_ % 2) == 0);

	for(unsigned int i = 0; i < nStep; ++i)
	{
		TeAddressDescription& addFound = addressesFound[i];
		
		int distBegin = 0;
		int distEnd = 0;

		int dist = 0;

		if(par)
		{	

			distBegin = abs(address.locationNumber_ - addFound.initialRightNumber_);
			distEnd = abs(address.locationNumber_ - addFound.finalRightNumber_);

			dist = MIN(distBegin, distEnd);
		}
		else
		{
			distBegin = abs(address.locationNumber_ - addFound.initialLeftNumber_);
			distEnd = abs(address.locationNumber_ - addFound.finalLeftNumber_);

			dist = MIN(distBegin, distEnd);
		}


		map<string, nearestItem>::iterator itLeft = nearestMap.find(addFound.leftNeighborhood_);

		if(itLeft != nearestMap.end())
		{
			if(dist < itLeft->second.second)
				nearestMap[addFound.leftNeighborhood_] = nearestItem(i, dist);
		}
		else
		{
			nearestMap[addFound.leftNeighborhood_] = nearestItem(i, dist);
		}

		if(addFound.leftNeighborhood_ != addFound.rightNeighborhood_)
		{
			map<string, nearestItem>::iterator itRight = nearestMap.find(addFound.rightNeighborhood_);

			if(itRight != nearestMap.end())
			{
				if(dist < itRight->second.second)
					nearestMap[addFound.rightNeighborhood_] = nearestItem(i, dist);					
			}
			else
			{
				nearestMap[addFound.rightNeighborhood_] = nearestItem(i, dist);
			}
		}		
	}

	if(!nearestMap.empty())
	{
		map<string, nearestItem>::iterator it = nearestMap.begin();
		vector<TeAddressDescription> newAdds;
		set<unsigned int> alreadyPushed;

		while(it != nearestMap.end())
		{
			set<unsigned int>::iterator itaux = alreadyPushed.find(it->second.first);

			// Se o endereço já não tiver sido colocado na lista, inclui, caso contrário não faz nada
			if(itaux == alreadyPushed.end())
				newAdds.push_back(addressesFound[it->second.first]);			

			alreadyPushed.insert(it->second.first);

			++it;
		}

		addressesFound.clear();
		addressesFound = newAdds;
	
	}

	return;
}

void TeAddressLocator::refineFoundedLocationsByNumberZipCode(const TeAddress& address, vector<TeAddressDescription>& addressesFound)
{
	unsigned int nStep = addressesFound.size();

	// id/dist
	typedef pair<unsigned int, int> nearestItem; 

	map<string, nearestItem> nearestMap;

	bool par = ((address.locationNumber_ % 2) == 0);

	for(unsigned int i = 0; i < nStep; ++i)
	{
		TeAddressDescription& addFound = addressesFound[i];
		
		int distBegin = 0;
		int distEnd = 0;

		int dist = 0;

		if(par)
		{	

			distBegin = abs(address.locationNumber_ - addFound.initialRightNumber_);
			distEnd = abs(address.locationNumber_ - addFound.finalRightNumber_);

			dist = MIN(distBegin, distEnd);
		}
		else
		{
			distBegin = abs(address.locationNumber_ - addFound.initialLeftNumber_);
			distEnd = abs(address.locationNumber_ - addFound.finalLeftNumber_);

			dist = MIN(distBegin, distEnd);
		}


		map<string, nearestItem>::iterator itLeft = nearestMap.find(addFound.leftZipCode_);

		if(itLeft != nearestMap.end())
		{
			if(dist < itLeft->second.second)
				nearestMap[addFound.leftZipCode_] = nearestItem(i, dist);
		}
		else
		{
			nearestMap[addFound.leftZipCode_] = nearestItem(i, dist);
		}

		if(addFound.leftZipCode_ != addFound.rightZipCode_)
		{
			map<string, nearestItem>::iterator itRight = nearestMap.find(addFound.rightZipCode_);

			if(itRight != nearestMap.end())
			{
				if(dist < itRight->second.second)
					nearestMap[addFound.rightZipCode_] = nearestItem(i, dist);
			}
			else
			{
				nearestMap[addFound.rightZipCode_] = nearestItem(i, dist);
			}
		}		
	}

	if(!nearestMap.empty())
	{
		map<string, nearestItem>::iterator it = nearestMap.begin();
		vector<TeAddressDescription> newAdds;
		set<unsigned int> alreadyPushed;

		while(it != nearestMap.end())
		{
			set<unsigned int>::iterator itaux = alreadyPushed.find(it->second.first);

			// Se o endereço já não tiver sido colocado na lista, inclui, caso contrário não faz nada
			if(itaux == alreadyPushed.end())
				newAdds.push_back(addressesFound[it->second.first]);			

			alreadyPushed.insert(it->second.first);

			++it;
		}

		addressesFound.clear();
		addressesFound = newAdds;
	
	}

	return;
}

void TeAddressLocator::splitAddress(TeAddress& add)
{
	unsigned int i;
	string aux = add.locationName_;

	add.locationName_ = "";
	add.locationNumber_ = 0;

	string number = "";

	if(!splitChar_.empty())
	{
		unsigned int nChars = aux.size();

		for(i = 0; i < nChars; ++i)
		{
			if(splitChar_[0] == aux[i])
			{
				++i;
				break;
			}

			add.locationName_ += aux[i];
		}

		number = "";

		for(; i < nChars; ++i)
		{
			if((aux[i] >= '0') && (aux[i] <= '9'))
				number += aux[i];
		}
		
	}
	else
	{
		unsigned int nChars = aux.size();

		for(i = 0; i < nChars; ++i)
		{
			if(aux[i] >= '0' && aux[i] <= '9')
				break;

			add.locationName_ += aux[i];
		}		

		for(; i < nChars; ++i)
		{
			if(aux[i] < '0' || aux[i] > '9')
				break;

			number += aux[i];
		}
	}

	TeTrim(add.locationName_);
	TeTrim(number);
	add.locationNumber_ = atoi(number.c_str());
}


void TeAddressLocator::queryAddresses(const string& strSQL, bool& queryExecuted, vector<TeAddressDescription>& addressesFound)
{
	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
		return;

	// Check if there is a segment that satisfies the clause
	if(portal->query(strSQL))
	{
		queryExecuted = true;

		fillAddressesDescription(addressesFound, portal);
	}

	delete portal;
}


bool TeAddressLocator::loadAddressCompoundNames()
{
	typesVector_.clear();
	titlesVector_.clear();
	prepositionsVector_.clear();

	if(!metadata_.locationType_.empty())
	{
		TeDatabasePortal *p = db_->getPortal();

		if(!p)
		{
			errMsg_ = "Could not get portal to find types in base table!";

			return false;
		}


		string sql  = "SELECT DISTINCT ";
		       sql += metadata_.locationType_;
			   sql += " FROM ";
			   sql += metadata_.tableName_;
			   sql += " ORDER BY ";
			   sql += metadata_.locationType_;

		if(p->query(sql))
		{
			while(p->fetchRow())
			{
				string aux = p->getData(metadata_.locationType_);

				TeTrim(aux);

				if(aux.empty())
					continue;

				typesVector_.push_back(aux);
			}
		}

		delete p;
	}

	// Título
	if(!metadata_.locationTitle_.empty())
	{
		TeDatabasePortal *p = db_->getPortal();

		if(!p)
		{
			errMsg_ = "Could not get portal to find titles in base table!";

			return false;
		}


		string sql  = "SELECT DISTINCT ";
		       sql += metadata_.locationTitle_;
			   sql += " FROM ";
			   sql += metadata_.tableName_;
			   sql += " ORDER BY ";
			   sql += metadata_.locationTitle_;

		if(p->query(sql))
		{
			while(p->fetchRow())
			{
				string aux = p->getData(metadata_.locationTitle_);

				TeTrim(aux);

				if(aux.empty())
					continue;

				titlesVector_.push_back(aux);
			}
		}

		delete p;
	}

	// Preposição
	if(!metadata_.locationPreposition_.empty())
	{
		TeDatabasePortal *p = db_->getPortal();

		if(!p)
		{
			errMsg_ = "Could not get portal to find prepositions in base table!";

			return false;
		}


		string sql  = "SELECT DISTINCT ";
		       sql += metadata_.locationPreposition_;
			   sql += " FROM ";
			   sql += metadata_.tableName_;
			   sql += " ORDER BY ";
			   sql += metadata_.locationPreposition_;

		if(p->query(sql))
		{
			while(p->fetchRow())
			{
				string aux = p->getData(metadata_.locationPreposition_);

				TeTrim(aux);

				if(aux.empty())
					continue;

				prepositionsVector_.push_back(aux);
			}
		}

		delete p;
	}

	return true;
}


bool TeAddressLocator::convertColumnNameToUpper()
{
	errMsg_ = "";
	
	if(!db_)
	{
		errMsg_ = "Connect to a database first!";

		return false;
	}

	string sql  = "UPDATE ";
		   sql += metadata_.tableName_;
		   sql += " SET ";
		   sql += metadata_.locationCompleteName_;
		   sql += " = ";
		   sql += db_->toUpper(metadata_.locationCompleteName_);

	if(!db_->execute(sql))
	{
		errMsg_ = "Couldn't convert to uppercase name column!";

		return false;
	}

	return true;
}

bool TeAddressLocator::addCompleteNameColumn()
{
	errMsg_ = "";
	
	if(!db_)
	{
		errMsg_ = "Connect to a database first!";

		return false;
	}

	TeAttribute attr;

	if(db_->columnExist(metadata_.tableName_, metadata_.locationCompleteName_, attr))
	{
		errMsg_ = "The specified column name already exist!";

		return false;
	}

	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = metadata_.locationCompleteName_;
	at.rep_.numChar_ = 255;

	if(!db_->addColumn(metadata_.tableName_, at.rep_))
	{
		errMsg_ = "Couldn't add specified column!";

		return false;
	}

	TeDatabasePortal* p = db_->getPortal();

	if(!p)
	{
		errMsg_ = "Couldn't get portal!";
		
		return false;
	}

	string sql = "SELECT count(*) FROM " + metadata_.tableName_;

	if(!p->query(sql) && !p->fetchRow())
	{
		errMsg_ = "Couldn't get number of lines to update!";

		delete p;
		
		return false;
	}

	int lo = 0;
	int nlo = p->getInt(0);

	p->freeResult();

	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nlo);
		TeProgress::instance()->setCaption("Address Geocoding Config");
		TeProgress::instance()->setMessage("Creating new column...");
	}

	int dt = CLOCKS_PER_SEC/4;
	int dt2 = CLOCKS_PER_SEC * 5;
	clock_t	t0, t1, t2;
	t2 = clock();
	t0 = t1 = t2;


	sql = "SELECT * FROM " + metadata_.tableName_;

	if(!p->query(sql))
	{
		errMsg_ = "Couldn't query address reference table!";

		delete p;
		
		return false;
	}

	while(p->fetchRow())
	{
		if(TeProgress::instance() && TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			
			errMsg_ = "The process was canceled by user!";

			delete p;

			return false;
		}


		++lo;

		string completeName = "";

		if(!metadata_.locationType_.empty())
			completeName = TeConvertToUpperCase(p->getData(metadata_.locationType_));

		if(!metadata_.locationTitle_.empty())
		{
			string aux = TeConvertToUpperCase(p->getData(metadata_.locationTitle_));

			if(!completeName.empty() && !aux.empty())
				completeName += " ";

			completeName += aux;			
		}

		if(!metadata_.locationPreposition_.empty())
		{
			string aux = TeConvertToUpperCase(p->getData(metadata_.locationPreposition_));

			if(!completeName.empty() && !aux.empty())
				completeName += " ";

			completeName += aux;			
		}

		if(!metadata_.locationName_.empty())
		{
			string aux = TeConvertToUpperCase(p->getData(metadata_.locationName_));

			if(!completeName.empty() && !aux.empty())
				completeName += " ";

			completeName += aux;			
		}

		string value = p->getData(metadata_.linkColumnName_);

		sql  = "UPDATE " + metadata_.tableName_ + " SET ";
		sql += metadata_.locationCompleteName_ + " = '" + db_->escapeSequence(completeName) + "' WHERE ";
		sql += metadata_.linkColumnName_ + " = '" + value + "'";
		
		if(!db_->execute(sql))
		{
			delete p;

			errMsg_ = "Couldn't update complete name column!";
		
			return false;
		}

		if(TeProgress::instance())
		{
			t2 = clock();
			if(int(t2-t1) > dt)
			{
				t1 = t2;
				
				if((int)(t2-t0) > dt2)
					TeProgress::instance()->setProgress(lo);
			}
		}
	}

	delete p;

	if(TeProgress::instance())
		TeProgress::instance()->reset();


	return true;
}


void TeAddressLocator::cleanAddressesFound(const string& locName, vector<TeAddressDescription>& addressesFound, const bool& disabletype, const bool& disabletitle, const bool& disablepreposition)
{
	vector<TeAddressDescription> newAddresses;

	for(unsigned int i = 0; i < addressesFound.size(); ++i)
	{
		TeAddressDescription add = addressesFound[i];
		
		string name = "";

		if(!add.locationType_.empty() && !disabletype)
		{
			TeTrim(add.locationType_);

			add.locationType_ = TeConvertToUpperCase(add.locationType_);

			name = add.locationType_;
		}

		if(!add.locationTitle_.empty() && !disabletitle)
		{
			TeTrim(add.locationTitle_);

			add.locationTitle_ = TeConvertToUpperCase(add.locationTitle_);

			if(!name.empty() && !add.locationTitle_.empty())
				name += " ";

			name += add.locationTitle_;
		}

		if(!add.locationPreposition_.empty() && !disablepreposition)
		{
			TeTrim(add.locationPreposition_);


			add.locationPreposition_ = TeConvertToUpperCase(add.locationPreposition_);

			if(!name.empty() && !add.locationPreposition_.empty())
				name += " ";

			name += add.locationPreposition_;
		}
		
		if(!add.locationName_.empty())
		{
			TeTrim(add.locationName_);
			
			add.locationName_ = TeConvertToUpperCase(add.locationName_);

			if(!name.empty() && !add.locationName_.empty())
				name += " ";

			name += add.locationName_;
		}

		if(name == locName)
			newAddresses.push_back(addressesFound[i]);
	}

	addressesFound = newAddresses;
}
