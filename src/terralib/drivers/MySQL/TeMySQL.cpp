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

#include "TeMySQL.h"
#include "../../kernel/TeProject.h"

#include <mysql/mysql.h>

//STL include files
#include <stdio.h>
#include <sys/types.h>
#include <cstring>
#include <stdarg.h>



//Modificação necessária devido a problemas com a compilação 64 bit. Adaptado para funcionar em ambas versões.
/*
extern "C" {
extern char *strxmov(char* dest, const char* src, ...);
}
*/

char *strxmov(char *dst,const char *src, ...)
{
	va_list pvar;

	va_start(pvar,src);
	while (src != 0) {
		while ((*dst++ = *src++)) ;
		dst--;
		src = va_arg(pvar, char *);
	}
	va_end(pvar);
	*dst = 0;			/* there might have been no sources! */
	return dst;
}

#ifndef MAX
#define MAX(a,b) ((a>b)?a:b)
#endif
#ifndef MIN
#define MIN(a,b) ((a<b)?a:b)
#endif


double swappd(double din)
{
	char  stri[8],stro[8];
	double dout;

	memcpy (stri,&din,8);
	stro[0] = stri[7];
	stro[1] = stri[6];
	stro[2] = stri[5];
	stro[3] = stri[4];
	stro[4] = stri[3];
	stro[5] = stri[2];
	stro[6] = stri[1];
	stro[7] = stri[0];
	memcpy (&dout,stro,8);
	return (dout);
}


TeMySQL::TeMySQL()
{
	isConnected_ = false;
	dbmsName_ = "MySQL";
	bufferBlob_ = 0;
	bufferBlobSize_ = 0;
    mysql_ = mysql_init (NULL);
}


TeMySQL::~TeMySQL()
{
    if(isConnected_) close();
}

void
        TeMySQL::close()
{
	if (bufferBlob_)
    {
		delete []bufferBlob_;
        bufferBlob_ = 0;
        bufferBlobSize_ = 0;
    }

    if(isConnected_)
    {
        mysql_close(mysql_);
        isConnected_ = false;
    }
}

bool TeMySQL::beginTransaction()
{
	return this->execute("START TRANSACTION;");
}

bool TeMySQL::commitTransaction()
{
	return this->execute("COMMIT;");
}

bool TeMySQL::rollbackTransaction()
{
	return this->execute("ROLLBACK;");
}


std::string TeMySQL::getClientEncoding()
{
	std::string clientEncoding = "";

	std::string sql = "SHOW VARIABLES LIKE 'character_set_client'";

	TeDatabasePortal* portal = getPortal();

	if(portal->query(sql) && portal->fetchRow())
	{
		clientEncoding = portal->getData(0);
	}

	delete portal;
	portal =  NULL;

	return clientEncoding;
}

bool TeMySQL::setClientEncoding(const std::string& characterSet)
{
	std::string sql = "SET character_set_client = " + characterSet;
	return execute(sql);
}

bool TeMySQL::getEncodingList(std::vector<std::string>& vecEncodingList)
{
	std::string query = " SHOW CHARACTER SET";
    if (mysql_real_query(mysql_, query.c_str(), query.size()) != 0)
	{
		return false;
	}

    MYSQL_RES* result = mysql_store_result(mysql_);
	unsigned int numRows = (int)mysql_num_rows(result);

	for(unsigned int i = 0; i < numRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		vecEncodingList.push_back(row[0]);
	}

	sort(vecEncodingList.begin(), vecEncodingList.end());

	if (result != NULL)
	{
		mysql_free_result(result);
		result = 0;
	}
	return true;
}

bool TeMySQL::getEncodingList(const std::string& host, const std::string& user, const std::string& password, const int& port, std::vector<std::string>& vecEncodingList)
{
	int portToConnect = port;
	if(portToConnect < 0)
	{
		portToConnect = 0;
	}

    if (mysql_real_connect(mysql_,host.c_str(),user.c_str(),password.c_str(),"",portToConnect,NULL,0) == NULL)
	{
		return false;
	}

	std::string query = " SHOW CHARACTER SET";
    if (mysql_real_query(mysql_, query.c_str(), query.size()) != 0)
	{
		return false;
	}

    MYSQL_RES* result = mysql_store_result(mysql_);
	unsigned int numRows = (int)mysql_num_rows(result);

	for (unsigned int i = 0; i < numRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		vecEncodingList.push_back(row[0]);
	}

	if (result != NULL)
	{
		mysql_free_result(result);
		result = 0;
	}

	close();

	return true;
}


bool TeMySQL::newDatabase(const string& database, const string& user, const string& password, const string& host, const int & port, bool terralibModel, const std::string& characterSet)
{
	int portToConnect = port;
	if(portToConnect < 0)
	{
		portToConnect = 0;
	}

    mysql_init(mysql_);

    if (mysql_real_connect(mysql_,host.c_str(),user.c_str(),password.c_str(),0,portToConnect,NULL,0) == NULL)
    {
		isConnected_ = false;
		return false;
	}

	std::string create = "CREATE DATABASE " + database;
	if(!characterSet.empty())
	{
		create += " CHARACTER SET = " + characterSet;
	}

    if (mysql_query(mysql_,create.c_str()) != 0)
	{
		isConnected_ = false;
		return false;
	}

    if (mysql_select_db(mysql_,database.c_str()) != 0)
	{
		isConnected_ = false;
		return false;
	}

	if(terralibModel)
	{
		//create conceptual model
		if(!this->createConceptualModel())
			return false;
	}
	isConnected_ = true;
	user_ = user;
	host_ = host;
	password_ = password;
	database_ = database;
	portNumber_ = portToConnect;
	return true;
}


bool TeMySQL::connect (const string& host, const string& user, const string& password, const string& database, int port)
{
	if(port<0)
		port = 0;

    ///F: real connect (this is a mysql api)
    if (mysql_real_connect(mysql_,host.c_str(),user.c_str(),password.c_str(),database.c_str(),port,NULL,0) == NULL)
	{
		isConnected_ = false;
		return false;
	}

	isConnected_ = true;
	user_ = user;
	host_ = host;
	password_ = password;
	database_ = database;
	portNumber_ = port;
	return true;
}


bool TeMySQL::showDatabases (const string& host, const string& user, const string& password, vector<string>& dbNames, int port)
{
    MYSQL* msql = mysql_init(NULL);

    if(msql == NULL)
        return false;

    unsigned long i;
    MYSQL_ROW row;

    if(port<0)
        port = 0;

    if (mysql_real_connect(msql,host.c_str(),user.c_str(),password.c_str(),0,port,NULL,0) == NULL)
    {
        errorMessage_= string(mysql_error(msql));
        return false;
    }

    MYSQL_RES *result = mysql_list_dbs(msql, "%");

    if( result )
    {
        unsigned int numRows = (unsigned int)mysql_num_rows(result);

        for (i = 0; i < numRows; ++i)
        {
            row = mysql_fetch_row(result);
            dbNames.push_back(row[0]);
        }

        mysql_free_result( result );

        mysql_close(msql);

        return true;
    }
    else
    {
        mysql_close(msql);

        return false;
    }
}


bool 
        TeMySQL::execute (const string &q)
{
    if (mysql_real_query(mysql_, q.c_str(), q.size()) != 0)
	{
        errorMessage_= string(mysql_error(mysql_));
        errorNumber_= mysql_errno(mysql_);
		return false;
	}
    return true;
}

bool 
        TeMySQL::createTable(const string& table, TeAttributeList &attr)
{
	string createStr = " CREATE TABLE " + table + " (";
	string type;
	string pkeys;
	bool first = true;	

	TeAttributeList::iterator it = attr.begin();

	while(it != attr.end())
	{
		string name = (*it).rep_.name_;
		
		switch((*it).rep_.type_)
		{
        case TeSTRING:
            if((*it).rep_.numChar_ > 0)
            {
                type = "VARCHAR(" + Te2String((*it).rep_.numChar_) + ") ";
            }
            else
            {
                type = "LONGTEXT";
            }
            break;
        case TeREAL:
            if((*it).rep_.decimals_ > 0)
                type = "DECIMAL(53, " + Te2String((*it).rep_.decimals_) + ") ";
            else
                type = "DECIMAL(53, 15) ";
            break;
        case TeINT:
            type = "INT ";
            break;

        case TeUNSIGNEDINT:
            type = "INT UNSIGNED ";
            break;

        case TeCHARACTER:
            type = "CHAR ";
            break;
			
        case TeBOOLEAN:
            type = "BOOLEAN ";
            break;

        case TeDATETIME:
            type = "DATETIME ";  //time
            break;
        case TeBLOB:
            type = "BLOB ";
            break;

        case TePOINTTYPE:
        case TePOINTSETTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "x				DOUBLE(53,15) DEFAULT '0.0'";
            createStr += ", y			DOUBLE(53,15) DEFAULT '0.0'";
            ++it;
            continue;

        case TeLINE2DTYPE:
        case TeLINESETTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "num_coords INT UNSIGNED not null ,";
            createStr += "lower_x double(53,15) not null ,";
            createStr += "lower_y double(53,15) not null ,";
            createStr += "upper_x double(53,15) not null ,";
            createStr += "upper_y double(53,15) not null ,";
            createStr += "ext_max double(53,15) not null, ";
            createStr += "spatial_data LONGBLOB not null ";
            ++it;
            continue;

        case TePOLYGONTYPE:
        case TePOLYGONSETTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "num_coords INT UNSIGNED not null ,";
            createStr += "num_holes INT UNSIGNED not null ,";
            createStr += "parent_id INT UNSIGNED not null ,";
            createStr += "lower_x double(53,15) not null ,";
            createStr += "lower_y double(53,15) not null ,";
            createStr += "upper_x double(53,15) not null ,";
            createStr += "upper_y double(53,15) not null ,";
            createStr += "ext_max double(53,15) not null ,";
            createStr += "spatial_data LONGBLOB not null ";
            ++it;
            continue;

        case TeCELLTYPE:
        case TeCELLSETTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "lower_x		double(53,15) NOT NULL ,";
            createStr += "lower_y		double(53,15) NOT NULL ,";
            createStr += "upper_x		double(53,15) NOT NULL ,";
            createStr += "upper_y		double(53,15) NOT NULL ,";
            createStr += "col_number	INT(10) NOT NULL ,";
            createStr += "row_number	INT(10) NOT NULL ";
            ++it;
            continue;

        case TeRASTERTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "lower_x double(53,15) not null, ";
            createStr += "lower_y double(53,15) not null, ";
            createStr += "upper_x double(53,15) not null, ";
            createStr += "upper_y double(53,15) not null, ";
            createStr += "band_id int(10) unsigned not null, ";
            createStr += "resolution_factor int(10) unsigned , ";
            createStr += "subband int(10) unsigned,";
            createStr += "spatial_data LONGBLOB NOT NULL, ";
            createStr += "block_size int(10) unsigned not null ";
            ++it;
            continue;

        case TeNODETYPE:
        case TeNODESETTYPE:
            if(!first)
                createStr += ", ";
            else
                first = false;

            createStr += "x				DOUBLE(53,15) DEFAULT '0.0'";
            createStr += ", y			DOUBLE(53,15) DEFAULT '0.0'";
            ++it;
            continue;

        case TeTEXTTYPE:
        case TeTEXTSETTYPE:

        default:
            type = "VARCHAR(" + Te2String((*it).rep_.numChar_) + ") ";
            break;
		}

		if(!((*it).rep_.defaultValue_.empty()))
			type += " DEFAULT '" + (*it).rep_.defaultValue_ + "' ";

		if(!((*it).rep_.null_))
			type += " NOT NULL ";

		if((*it).rep_.isAutoNumber_)
			type += " AUTO_INCREMENT ";

		if(!first)
			createStr += ", ";
		else
			first = false;
		
		createStr += name + " " + type;

		// check if column is part of primary key
		if(((*it).rep_.isPrimaryKey_) && ((*it).rep_.type_ != TeBLOB))
		{
			if(!pkeys.empty())
				pkeys += ", ";

			pkeys += (*it).rep_.name_;
		}
		
		++it;
	}

	if(!pkeys.empty())
		createStr += ", PRIMARY KEY (" + pkeys + ") ";

	createStr += ")";

	return execute(createStr);
}


bool 
        TeMySQL::tableExist(const string& table)
{
	//if (table.empty())
	//	return false;

	//string q ="SHOW TABLES LIKE '"+ table + "'";
	////string q ="DESC "+ table;

    //if (mysql_real_query(mysql_, q.c_str(), q.size()) != 0)
	//{
    //	errorMessage_ = string(mysql_error(mysql_));
	//}

    //MYSQL_RES*	res_ = mysql_use_result(mysql_);

	//if (res_)
	//{
	//	mysql_free_result(res_);
	//	return true;
	//}
	//else
	//	return false;
	if (table.empty())
		return false;

	MYSQL_RES* myres;

    myres = mysql_list_tables(mysql_, table.c_str());
	if(myres == 0)
	{
        errorMessage_ = string(mysql_error(mysql_));
		return false;
	}
	else
	{
		bool ret = false;
		MYSQL_ROW myrow;
		while ((myrow = mysql_fetch_row(myres)))
		{
			string s = myrow[0];
			string S = TeConvertToUpperCase(s);
			string SS = TeConvertToUpperCase(table);
			if(S == SS)
			{
				ret = true;
				break;
			}
		}
		mysql_free_result(myres);
		return ret;
	}
}

bool 
        TeMySQL::columnExist(const string& table, const string&  column ,TeAttribute& attr)
{
	string q = "SHOW COLUMNS FROM " + table + " LIKE \'" + column + "\'";
    if (mysql_query(mysql_, q.c_str()) != 0)
	{
		return false;
	}
    MYSQL_RES* fieldsres = mysql_use_result(mysql_);
	if (!fieldsres)
		return false;
	MYSQL_ROW row = mysql_fetch_row(fieldsres);
	if (!row)
		return false;
	attr.rep_.name_ = column;
	
	string ftype(row[1]);
	string fpk(row[3]);
	if (ftype.find("char") != string::npos)
		attr.rep_.type_ = TeSTRING;
	else if (ftype.find("int") != string::npos)
		attr.rep_.type_ = TeINT;
	else if (ftype.find("double") != string::npos)
		attr.rep_.type_ = TeREAL;
	else if (ftype.find("blob") != string::npos)
		attr.rep_.type_ = TeBLOB;
	else if (ftype.find("longtext") != string::npos)
		attr.rep_.type_ = TeSTRING;
	else if ((ftype.find("date") != string::npos) || (ftype.find("time") != string::npos))
		attr.rep_.type_ = TeDATETIME;

	unsigned int p1 = ftype.find("(");
	unsigned int p2 = ftype.find(")");
	if (p1 != string::npos && p2 != string::npos)
		attr.rep_.numChar_ = atoi(ftype.substr(p1+1,p2-1).c_str());

	if (fpk.find("PRI") != string::npos)
		attr.rep_.isPrimaryKey_ = true;

	mysql_free_result(fieldsres);
	return true;
}

bool TeMySQL::addColumn (const string& table, TeAttributeRep &rep)
{
	string q ="ALTER TABLE " + table +" ADD ";
	string type;
	char	size[8];

	string field = TeGetExtension(rep.name_.c_str());
	if(field.empty())
		field = rep.name_;

	switch (rep.type_)
	{
    case TeSTRING:
        if(rep.numChar_ > 0)
        {
            type = " VARCHAR(" + Te2String(rep.numChar_) + ")";
        }
        else
        {
            type = " LONGTEXT";
        }
        break;
		case TeREAL:
        type = " DECIMAL(53, " + Te2String(rep.decimals_) + ") ";
        break;
		case TeINT:
        type = " INT";
        break;
		case TeCHARACTER:
        type = " CHAR";
        break;
		case TeBOOLEAN:
        type = " BOOLEAN";
        break;
		case TeDATETIME:
        type = " DATETIME";
        break;
		case TeBLOB:
        type = " LONGBLOB";
        break;
		default:
		type = " VARCHAR ";
		sprintf (size,"(%d)",rep.numChar_);
		type += string (size);
		break;
	}
	q += field;
	q += type;
	bool res = execute(q);
	if (res)
		alterTableInfoInMemory(table);
	return res;
}


bool
        TeMySQL::deleteIndex(const string& tableName, const string& indexName)
{	

	string query = "drop index "+indexName+" on "+tableName;

	if(!this->execute(query))
	{		
		return false;
	}

	return true;

}

bool
        TeMySQL::getIndexesFromTable(const string& tableName, std::vector<TeDatabaseIndex>& vecIndexes)
{

	std::vector<TeDatabaseIndex> indexes;

	string dbName = this->databaseName();

	string query1 = "select INDEX_NAME, COLUMN_NAME from information_schema.STATISTICS ";
    query1 += "where table_name = \""+ tableName +"\" and TABLE_SCHEMA =\""+ dbName +"\" and index_name != \"PRIMARY\"";

	TeDatabasePortal* portal = getPortal();

	std::vector<std::string> names;
	std::vector<std::string> cols;

	std::set<std::string> idxNames;

	std::multimap<std::string, std::string> idxAttrs;
	
	if(portal->query(query1))
	{
		while(portal->fetchRow())
		{
			std::string idxName = portal->getData("INDEX_NAME");
			std::string attrName = portal->getData("COLUMN_NAME");

			idxAttrs.insert(std::pair<std::string, std::string>(idxName, attrName));
			
			idxNames.insert(idxName);
		}		

	}else
	{
		delete portal;
		return false;
	}

	portal->freeResult();

	delete portal;
	portal =  NULL;

	std::set<std::string>::iterator it = idxNames.begin();
	while(it != idxNames.end())
	{
		std::string idxName = *it;

		TeDatabaseIndex idxx;
		idxx.setIndexName(idxName);
		idxx.setIsPrimaryKey(false);
		
		std::vector<string> attNames;
		std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> itAttrs = idxAttrs.equal_range(idxName);
		std::multimap<std::string, std::string>::iterator itA;
		for (itA=itAttrs.first; itA!=itAttrs.second; ++itA)
		{
			std::string attrName = itA->second;
			attNames.push_back(attrName);
		}

		idxx.setColumns(attNames);

		indexes.push_back(idxx);

		++it;
	}	

	vecIndexes = indexes;	

	return true;

}




bool 
        TeMySQL::alterTable(const string& tableName, TeAttributeRep& rep, const string& oldColName)
{
	if(!tableExist(tableName))
		return false;

	string tab;

	if(oldColName.empty())
	{
		tab = " ALTER TABLE " + tableName + " MODIFY ";
		tab += rep.name_ +"  ";
	}
	else
	{
		tab = " ALTER TABLE " + tableName + " CHANGE ";
		tab += oldColName +" "+ rep.name_ +"  ";
	}

	switch (rep.type_)
	{
    case TeSTRING:
        if(rep.numChar_ > 0)
        {
            tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
        }
        else
        {
            tab += "LONGTEXT";
        }
        break;

		case TeREAL:
        tab += "DOUBLE(24, 15)";
        break;

		case TeINT:
        tab += "INT";
        break;

		case TeDATETIME:
        tab += "DATETIME";
        break;

		case TeCHARACTER:
        tab += "CHAR";
        break;

		case TeBOOLEAN:
        tab += "BOOLEAN";
        break;

		case TeBLOB:
        tab += "LONGBLOB";
        break;
		
		default:
        tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
        break;
	}

	tab += " NULL ";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error alter table " + tableName + " !";
		return false;
	}

	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + tableName + "'";
	if(portal->query(sql) && portal->fetchRow())
		tableId = portal->getData(0);

	delete portal;

	if(tableId.empty() == false)
	{
		if(oldColName.empty() == false) // column name changed
		{
            // update relation
			sql = "UPDATE te_tables_relation SET related_attr = '" + rep.name_ + "'";
			sql += " WHERE related_table_id = " + tableId;
			sql += " AND related_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

			sql = "UPDATE te_tables_relation SET external_attr = '" + rep.name_ + "'";
			sql += " WHERE external_table_name = '" + tableName + "'";
			sql += " AND external_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

            // update grouping
			sql = "UPDATE te_grouping SET grouping_attr = '" + tableName + "." + rep.name_ + "'";
			sql += " WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
		else // column type changed
		{
			// delete relation
			sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
			sql += " AND related_attr = '" + rep.name_ + "')";
			sql += " OR (external_table_name = '" + tableName + "'";
			sql += " AND external_attr = '" + rep.name_ + "')";
			if(execute(sql) == false)
				return false;

			// delete grouping
			TeDatabasePortal* portal = getPortal();
			sql = "SELECT theme_id FROM te_grouping WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(portal->query(sql) && portal->fetchRow())
			{
				string themeId = portal->getData(0);

				sql = "DELETE FROM te_legend WHERE theme_id = " + themeId + " AND group_id >= 0";
				if(execute(sql) == false)
				{
					delete portal;
					return false;
				}
			}
			delete portal;

			sql = "DELETE FROM te_grouping";
			sql += " WHERE grouping_attr = '" + tableName + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
	}

	alterTableInfoInMemory(tableName);
	return true;
}


TeDatabasePortal* TeMySQL::getPortal ()
{
	TeMySQLPortal* portal = new TeMySQLPortal (this);
	return portal;
}

string TeMySQL::errorMessage ()
{
    return string(mysql_error(mysql_));
}

int	TeMySQL::errorNum ()
{
    return mysql_errno(mysql_);
}

bool TeMySQL::defineIntegrity(void)
{
	return true;		// mysql doesn't have referencial integrity
    // there is nothing to be done
}


string TeMySQL::getConcatFieldsExpression(const vector<string>& fNamesVec)
{
	if (fNamesVec.empty() == true)
		return "";

	string concatExp = "CONCAT(";
	for (unsigned int i = 0; i < fNamesVec.size(); ++i)
	{
		if (i != 0)
			concatExp += ", ";
		concatExp += fNamesVec[i];
	}
	concatExp += ")";

	return concatExp;
}




TeMySQLPortal::TeMySQLPortal ( TeMySQL  *m)
{
	mysql_ = m->getMySQL();
	numRows_ = 0;
	numFields_ = 0;
	result_ = 0;
	row_ = 0;
	db_ = m;
}


TeMySQLPortal::TeMySQLPortal ( const TeMySQLPortal& p) : TeDatabasePortal()
{
	mysql_ = p.mysql_;
	numRows_ = 0;
	numFields_ = 0;
	result_ = 0;
	row_ = 0;
}


TeMySQLPortal::~TeMySQLPortal ()
{
	freeResult ();
}


string 
        TeMySQL::escapeSequence(const string& from)
{
	string ret = "";
	if (!from.empty())
	{
		char	*to = new char[from.size()*2+1];
        mysql_real_escape_string(mysql_,to,from.c_str(),from.size());
		ret = to;
		delete  []to;
	}

	return ret;
}

bool TeMySQLPortal::query (const string &q, TeCursorLocation /* l */, TeCursorType /* t */, TeCursorEditType /* e */, TeCursorDataType /*dt*/)
{
	if (result_)
		freeResult();
	attList_.clear ();

	if (mysql_real_query(mysql_, q.c_str(), q.size()) != 0)
	{
		string erro = string(mysql_error(mysql_));
		return false;
	}

	result_ = mysql_store_result(mysql_);
	numRows_ = (int)mysql_num_rows(result_);
	if (result_)  // there are rows
	{
		numFields_ = mysql_num_fields(result_);

		MYSQL_FIELD *fields = mysql_fetch_fields(result_);
		
		int i;
		for(i = 0; i < numFields_; i++)
		{
			TeAttribute attribute;
			switch (fields[i].type)
			{
			case FIELD_TYPE_TINY:
				attribute.rep_.type_ = TeBOOLEAN;
				break;

			case FIELD_TYPE_DECIMAL:
			case FIELD_TYPE_SHORT:
			case FIELD_TYPE_LONG:
			case FIELD_TYPE_INT24:
				attribute.rep_.type_ = TeINT;
				attribute.rep_.numChar_ = fields[i].length;
				break;

			case FIELD_TYPE_FLOAT:
			case FIELD_TYPE_DOUBLE:
			case FIELD_TYPE_LONGLONG:
			case FIELD_TYPE_NEWDECIMAL:
				attribute.rep_.type_ = TeREAL;
				attribute.rep_.numChar_ = fields[i].length;
				attribute.rep_.decimals_ = fields[i].decimals;
				break;

			case FIELD_TYPE_DATE:
			case FIELD_TYPE_TIME:
			case FIELD_TYPE_DATETIME:
			case FIELD_TYPE_YEAR:
			case FIELD_TYPE_NEWDATE:
				attribute.rep_.type_ = TeDATETIME;
				attribute.dateTimeFormat_ = "YYYYsMMsDDsHHsmmsSS24";
				attribute.dateChronon_ = TeSECOND;
				break;

			case FIELD_TYPE_TINY_BLOB:
			case FIELD_TYPE_MEDIUM_BLOB:
			case FIELD_TYPE_LONG_BLOB:
			case FIELD_TYPE_BLOB:
				if (fields[i].flags & BINARY_FLAG)
				{
					attribute.rep_.type_ = TeBLOB;
				}
				else
				{
					attribute.rep_.type_ = TeSTRING; //como tipo TEXT
				}
				break;

			case FIELD_TYPE_VAR_STRING:
			case FIELD_TYPE_STRING:
				attribute.rep_.type_ = TeSTRING;
				attribute.rep_.numChar_ = fields[i].length;
				break;

			case FIELD_TYPE_ENUM:
			case FIELD_TYPE_SET:
			default :
                    attribute.rep_.type_ = TeUNKNOWN;
            break;
        }
			if (fields[i].flags & PRI_KEY_FLAG)
				attribute.rep_.isPrimaryKey_ = true;

			if (fields[i].flags & AUTO_INCREMENT_FLAG)
				attribute.rep_.isAutoNumber_ = true;

			attribute.rep_.name_ = fields[i].name;
			attList_.push_back ( attribute );
		}
    }
    else  // mysql_store_result() returned nothing; should it have?
    {
        if(mysql_field_count(mysql_) == 0)
        {
	        // query does not return data
            // (it was not a SELECT)
			numFields_ = 0;
            numRows_ = (int)mysql_affected_rows(mysql_);
        }
        else // mysql_store_result() should have returned data
        {
            return false;
        }
    }
	return true;
}


bool TeMySQLPortal::fetchRow ()
{
	if(result_ == NULL)
	{
		return false;
	}
	if ((row_ = mysql_fetch_row(result_)))
		return true;
	else
		return false;
}


bool TeMySQLPortal::fetchRow (int i)
{
	if(result_ == NULL)
	{
		return false;

	}
	if (i >= 0 && i < numRows_)
	{
		mysql_data_seek (result_, i);
		return fetchRow ();
	}
	else
		return false;
}


void TeMySQLPortal::freeResult ()
{
	if (result_)
	{
		mysql_free_result(result_);
		result_ = 0;
	}
}

string TeMySQLPortal::errorMessage()
{
    return string(mysql_error(mysql_));
} //: last error message()

int	TeMySQLPortal::errorNum()
{
    return mysql_errno(mysql_);
}


int TeMySQLPortal::insertId ()
{
	return (int)mysql_insert_id(mysql_);
}


int TeMySQL::insertId ()
{
    return (int)mysql_insert_id(mysql_);
}



bool TeMySQL::insertProjection (TeProjection *proj)
{
	char q[512];
	sprintf (q,"INSERT INTO te_projection VALUES(0,'%s',%.15f,%.15f,%.15f,%.15f,%.15f,%.15f,'%s',%.15f,%d,'%s')",
             proj->name().c_str(),
             proj->params().lon0*TeCRD,
             proj->params().lat0*TeCRD,
             proj->params().offx,
             proj->params().offy,
             proj->params().stlat1*TeCRD,
             proj->params().stlat2*TeCRD,
             proj->params().units.c_str(),
             proj->params().scale,
             proj->params().hemisphere,
             proj->datum().name().c_str());

	if (!execute(q))
		return false;

	proj->id(insertId());

	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}


bool TeMySQL::insertView (TeView *view)
{
	TeProjection* proj = view->projection();
	// save its projection
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Error inserting projection";
		return false;
	}

	char q[512];
	if(view->getCurrentTheme() > 0)
	{
		sprintf (q,"INSERT INTO te_view VALUES(0,%d,'%s','%s',%d,%E,%E,%E,%E,%d)",
                 proj->id(),view->name().c_str(),view->user().c_str(),view->isVisible(),
                 view->getCurrentBox().lowerLeft().x(),view->getCurrentBox().lowerLeft().y(),
                 view->getCurrentBox().upperRight().x(),view->getCurrentBox().upperRight().y(),
                 view->getCurrentTheme());
	}
	else
	{
		sprintf (q,"INSERT INTO te_view VALUES(0,%d,'%s','%s',%d,%E,%E,%E,%E,NULL)",
                 proj->id(),view->name().c_str(),view->user().c_str(),view->isVisible(),
                 view->getCurrentBox().lowerLeft().x(),view->getCurrentBox().lowerLeft().y(),
                 view->getCurrentBox().upperRight().x(),view->getCurrentBox().upperRight().y());
	}

	if (!execute(q))
		return false;

	view->id(insertId());

	for (unsigned int th=0; th<view->size(); th++)
	{
		TeViewNode* node = view->get (th);
		if (node->type() == TeTHEME)
		{
			TeTheme *theme = (TeTheme*) node;
			insertTheme (theme);
		}
		else
		{
			TeViewTree* tree = (TeViewTree*)node;
			insertViewTree (tree);
		}
	}

	viewMap()[view->id()] = view;
	return true;
}


bool TeMySQL::insertViewTree (TeViewTree *tree)
{
	string save;
	save = "INSERT INTO te_theme (name, layer_id, node_type, view_id, priority) VALUES('','";
	save += tree->name() +  "," + Te2String(tree->parentId()) + "," + Te2String((int)tree->type());
	save += "," + Te2String(tree->view()) + "," + Te2String(tree->priority()) + ")";
	if (!execute(save))
		return false;
	tree->id(insertId());
	return true;
}


bool TeMySQL::insertTheme (TeAbstractTheme *tem)
{
	string save;
	save = "INSERT INTO te_theme ";
	save += "(layer_id, view_id, name, parent_id, priority, node_type, min_scale, max_scale, ";
	save += "generate_attribute_where, generate_spatial_where, generate_temporal_where, ";
	save += "collection_table, visible_rep, enable_visibility, lower_x, lower_y, upper_x, upper_y, creation_time) ";
	save += " VALUES(";
	
	if(tem->type()==TeTHEME)
		save += Te2String(static_cast<TeTheme*>(tem)->layerId()) +", " ;
	else
		save += " NULL, ";

	save += Te2String(tem->view()) +", ";
	save += "'" + escapeSequence(tem->name()) + "', ";
	save += Te2String(tem->parentId()) +", ";
	save += Te2String(tem->priority()) +", ";
	save += Te2String(tem->type()) +", ";
	save += Te2String(tem->minScale(), 15) +", ";
	save += Te2String(tem->maxScale(), 15) +", ";
	save += "'" + escapeSequence(tem->attributeRest()) + "', ";
	save += "'" + escapeSequence(tem->spatialRest()) + "', ";
	save += "'" + escapeSequence(tem->temporalRest()) + "', ";

	if(tem->type()==TeTHEME)
		save += " '" + escapeSequence(static_cast<TeTheme*>(tem)->collectionTable()) + "', ";
	else
		save += " '', ";

	save += Te2String(tem->visibleRep()) +", ";
	save += Te2String(tem->visibility()) +", ";
	save += Te2String(tem->box().x1()) +", ";
	save += Te2String(tem->box().y1()) +", ";
	save += Te2String(tem->box().x2()) +", ";
	save += Te2String(tem->box().y2()) +",  ";
	TeTime creationTime = tem->getCreationTime();
	save += getSQLTime(creationTime);
	save += " ) ";

	if (!this->execute(save))
		return false;

	tem->id(this->insertId());

	if((tem->type()==TeTHEME || tem->type()==TeEXTERNALTHEME)&& static_cast<TeTheme*>(tem)->collectionTable().empty())
	{
		string colTab = "te_collection_" + Te2String(tem->id());
		static_cast<TeTheme*>(tem)->collectionTable(colTab);
		save = "UPDATE te_theme SET collection_table='" + colTab + "' WHERE theme_id=" ;
		save += Te2String(tem->id());
		this->execute(save);
	}
	if(tem->parentId() == 0)
	{
		std::string sql = "UPDATE te_theme SET";
		sql += "  parent_id = " + Te2String(tem->id());
		sql += " WHERE theme_id = ";
		sql += Te2String(tem->id());

		tem->parentId(tem->id());

		if(!this->execute(sql))
			return false;
	}

	bool status;

	// insert grouping
	int numSlices = 0;
	if(tem->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (tem->id(), tem->grouping()))
			return false;
		numSlices = tem->grouping().groupNumSlices_;
	}

	tem->outOfCollectionLegend().group(-1); 
	tem->outOfCollectionLegend().theme(tem->id()); 
	status = insertLegend (&(tem->outOfCollectionLegend())); 
	if (!status)
		return status;

	tem->withoutDataConnectionLegend().group(-2); 
	tem->withoutDataConnectionLegend().theme(tem->id()); 
	status = insertLegend (&(tem->withoutDataConnectionLegend())); 
	if (!status)
		return status;

	tem->defaultLegend().group(-3); 
	tem->defaultLegend().theme(tem->id()); 
	status = insertLegend (&(tem->defaultLegend())); 
	if (!status)
		return status;

	tem->pointingLegend().group(-4); 
	tem->pointingLegend().theme(tem->id()); 
	status = insertLegend (&(tem->pointingLegend())); 
	if (!status)
		return status;

	tem->queryLegend().group(-5); 
	tem->queryLegend().theme(tem->id()); 
	status = insertLegend (&(tem->queryLegend())); 
	if (!status)
		return status;

	tem->queryAndPointingLegend().group(-6); 
	tem->queryAndPointingLegend().theme(tem->id()); 
	status = insertLegend (&(tem->queryAndPointingLegend())); 
	if (!status)
		return status;

	for (int i = 0; i < numSlices; i++)
	{
		tem->legend()[i].group(i);
		tem->legend()[i].theme(tem->id());
		status = insertLegend (&(tem->legend()[i]));
		if (!status)
			return status;
	}
	if (!status)
		return status;

	//insert metadata theme
	if(!tem->saveMetadata(this))
		return false;

	themeMap()[tem->id()] = tem;

	if(tem->type()==TeTHEME && !updateThemeTable(static_cast<TeTheme*>(tem)))
		return false;

	return true;
}

bool TeMySQL::insertThemeGroup (TeViewTree* tree)
{
	string save;
	save = "INSERT INTO te_theme ";
	save += "(name,parent_id,node_type,view_id,priority) ";
	save += " VALUES ('" + tree->name() + "',1,1, ";
	save += Te2String(tree->view()) +", ";
	save += Te2String(tree->priority()) +") ";
	if (!this->execute(save))
		return false;
	tree->id(this->insertId());
	return true;
}


bool TeMySQL::generateLabelPositions (TeTheme *theme, const std::string& objectId)
{
	string	geomTable, query;
	string	collTable = theme->collectionTable();

	if((collTable.empty()) || (!tableExist(collTable)))
		return false;

	if (theme->layer()->hasGeometry(TeLINES))
	{	
		geomTable = theme->layer()->tableName(TeLINES);

		string extMax = "CREATE TEMPORARY TABLE maxTable";
		extMax += " SELECT object_id, lower_x, lower_y, upper_x, upper_y";
		extMax += " FROM " + geomTable; 
		if(!objectId.empty())
			extMax += " WHERE object_id = '"+ objectId +"'";
		extMax += " ORDER BY object_id, ext_max DESC ";
		
		if(!execute(extMax))
			return false;

		query = "UPDATE " + collTable + ",maxTable SET label_x = (lower_x + upper_x)/2,";
		query += "label_y = (lower_y + upper_y)/2 WHERE c_object_id = object_id";

		if (execute(query) == false)
		{
			execute("DROP TABLE maxTable");
			return false;
		}

		execute("DROP TABLE maxTable");
	}

	if (theme->layer()->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);

		query = "UPDATE " + collTable + "," + geomTable + " SET label_x = x,";
		query += "label_y = y WHERE c_object_id = object_id";

		if(!objectId.empty())
			query += " AND c_object_id = '"+ objectId +"'";

		if(execute(query) == false)
			return false;
	}
	
	if (theme->layer()->hasGeometry(TePOLYGONS))
	{	
		geomTable = theme->layer()->tableName(TePOLYGONS);

		string extMax = "CREATE TEMPORARY TABLE maxTable";
		extMax += " SELECT object_id, lower_x, lower_y, upper_x, upper_y";
		extMax += " FROM " + geomTable; 
		if(!objectId.empty())
			extMax += " WHERE object_id = '"+ objectId +"'";
		extMax += " ORDER BY object_id, ext_max DESC ";

		if(!execute(extMax))
			return false;

		query = "UPDATE " + collTable + ",maxTable SET label_x = (lower_x + upper_x)/2,";
		query += "label_y = (lower_y + upper_y)/2 WHERE c_object_id = object_id";
		if(!objectId.empty())
			query += " AND c_object_id = '"+ objectId +"'";

		if (execute(query) == false)
		{
			execute("DROP TABLE maxTable");
			return false;
		}
		
		execute("DROP TABLE maxTable");
	}
	
	if(theme->layer()->hasGeometry(TeCELLS))
	{
		geomTable = theme->layer()->tableName(TeCELLS);

		query = "UPDATE " + collTable + "," + geomTable + " SET label_x = (lower_x + upper_x)/2,";
		query += "label_y = (lower_y + upper_y)/2 WHERE c_object_id = object_id";
		if(!objectId.empty())
			query += " AND c_object_id = '"+ objectId +"'";

		if (execute(query) == false)
			return false;	
	}
	return true;
}


bool TeMySQL::insertLayer (TeLayer* layer)
{
	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Error inserting projection";
		return false;
	}
	int pid = proj->id();
	string save = "INSERT INTO te_layer (layer_id,projection_id,name,lower_x,lower_y,upper_x,upper_y,edition_time) VALUES(0,";
	save += Te2String(pid);
	save += ",'" + layer->name() + "', ";
	save += Te2String(layer->box().x1_, 15) + ", ";
	save += Te2String(layer->box().y1_, 15) + ", ";
	save += Te2String(layer->box().x2_, 15) + ", ";
	save += Te2String(layer->box().y2_, 15) + ", ";
	TeTime editionTime = layer->getEditionTime();
	save += getSQLTime(editionTime) + ")";

	// if layer insertion fails remove projection
	if (!this->execute (save))
	{
		string sql = "DELETE FROM te_projection WHERE  projection_id = " + Te2String(pid);
		this->execute(sql);	
		return false;
	}

	layer->id(this->insertId());
	layerMap()[layer->id()] = layer;

	return true;
}

bool TeMySQL::insertProject (TeProject* project)
{
	if (!project)
		return false;

	string save = "INSERT INTO te_project (project_id, name, description, current_view) VALUES(0,";
	save += "'" + project->name() + "', ";
	save += "'" + project->description() + "', ";
	save += Te2String(project->getCurrentViewId())+ ")";

	// if layer insertion fails remove projection
	if (!this->execute (save))
		return false;
	
	project->setId(this->insertId());
	projectMap()[project->id()] = project;
	for (unsigned int i=0; i<project->getViewVector().size(); i++)
		insertProjectViewRel(project->id(), project->getViewVector()[i]);
	return true;
}

bool TeMySQL::insertLegend (TeLegendEntry *legend)
{
	char q[1024];
	string save;
	sprintf (q,"INSERT INTO te_legend VALUES(0,%d,%d,%d,'%s','%s','%s')",
             legend->theme(),
             legend->group(),
             legend->count(),
             escapeSequence(legend->from()).c_str(),
             escapeSequence(legend->to()).c_str(),
             escapeSequence(legend->label()).c_str());
	save = q;

	if (!this->execute(save))
		return false;

	int legId = insertId();
	legend->id(legId);
	legendMap()[legend->id()] = legend;
	return insertVisual(legend);
}


bool TeMySQL::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;
	
	string sql;
	sql = "INSERT INTO te_representation (layer_id, geom_type, geom_table, description,";
	sql += "lower_x, lower_y, upper_x, upper_y, res_x, res_y, num_cols, num_rows) VALUES( ";
	sql += Te2String(layerId) + ", ";
	sql += Te2String(static_cast<int>(rep.geomRep_)) + ", ";
	sql += "'" + rep.tableName_ + "', ";
	sql += "'" + rep.description_ + "',";
	sql += Te2String(rep.box_.x1_,15) + ", ";
	sql += Te2String(rep.box_.y1_,15) + ", ";
	sql += Te2String(rep.box_.x2_,15) + ", ";
	sql += Te2String(rep.box_.y2_,15) + ", ";
	sql += Te2String(rep.resX_) + ", ";
	sql += Te2String(rep.resY_) + ", ";
	sql += Te2String(rep.nCols_) + ", ";
	sql += Te2String(rep.nLins_) + ") ";
	if (execute(sql))
	{
		rep.id_ = (this->insertId());
		return true;
	}
	return false;
}

bool TeMySQL::insertRelationInfo(const int tableId, const string& tField,
                                    const string& eTable, const string& eField, int& relId)
{
	// check if relation already exists
	TeMySQLPortal* portal = (TeMySQLPortal*) this->getPortal();
	if (!portal)
		return false;

	relId = -1;
	string sel = "SELECT relation_id FROM te_tables_relation WHERE";
	sel += " related_table_id = " + Te2String(tableId);
	sel += " AND related_attr = '" + tField + "'";
	sel += " AND external_table_name = '" + eTable + "'";
	sel += " AND external_attr = '" + eField + "'";
	if (!portal->query(sel))
	{
		delete portal;
		return false;
	}

	if (portal->fetchRow())
	{
		relId = atoi(portal->getData(0));
		delete portal;
		return true;
	}
	delete portal;

	string sql = "INSERT INTO te_tables_relation(related_table_id, related_attr, external_table_name, external_attr) ";
	sql += " VALUES( ";
	sql += Te2String(tableId);
	sql += ",'" + tField + "'";
	sql += ",'" + eTable + "'";
	sql += ",'" + eField + "')";
	if (!this->execute(sql))
		return false;
	relId = this->insertId(); 
	return true;
}

bool TeMySQL::insertTable(TeTable &table)
{
	string tableName = table.name();
	int size = table.size();
	TeAttributeList att = table.attributeList();
	TeAttributeList::iterator it = att.begin();
	TeAttributeList::iterator itEnd = att.end();
	TeTableRow row;
	
	if (!beginTransaction())
		return false;

	int i;
	unsigned int j;
	for ( i = 0; i < size; i++ )	
	{
		row = table[i];
		it = att.begin();
		string attrs;
		string values;
		j = 1;
		unsigned int jj = 0;
		while ( it != itEnd )
		{
			if ((jj>=row.size()) || row[jj].empty() || (*it).rep_.isAutoNumber_)
			{
				++it;
				j++;
				jj++;
				continue;
			}
			
			if (!values.empty())
			{
				attrs += ", ";
				values += ", ";
			}			
			attrs += (*it).rep_.name_;			
			switch ((*it).rep_.type_)
			{
			case TeSTRING:
				values += "'"+ escapeSequence( row[jj] ) +"'";
				break;
			case TeREAL:
				{
                    std::string strValue = row[jj];
                    replace(strValue.begin(), strValue.end(), ',', '.');
                    values += strValue;
				}
				break;
			case TeINT:
				values += row[jj];
				break;
			case TeDATETIME:
				{
					const string temp_dt = string(row[jj].c_str());
					TeTime t(temp_dt, (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_, (*it).timeSeparator_, (*it).indicatorPM_);
					values += this->getSQLTime(t);
				}
				break;
			case TeCHARACTER:
				values += "'" + escapeSequence(row[jj]) + "'";
				break;
			case TeBOOLEAN:
				{
					std::string value = "0";
					if(row[jj] == "1" || TeConvertToLowerCase(row[jj]) == "t" || TeConvertToLowerCase(row[jj]) == "true")
					{
						value = "1";
					}
					values += value;
				}
				break;
			case TeBLOB:
				values += "'" + escapeSequence(row[jj]) + "'";
			default:
				values += "'"+ escapeSequence(row[jj]) +"'";
				break;
			}
			++it;
			j++;
			jj++;
		}

		if (values.empty()) 
			continue;

		string q = "INSERT INTO "+tableName + " ("+ attrs+") " + " VALUES ("+values+") ";
		if(!execute(q))
		{
			rollbackTransaction();
			return false;
		}
	}

	if (!commitTransaction())
	{
		rollbackTransaction();
		return false;
	}
	return true;
}

bool
        TeMySQL::updateTable(TeTable &table)
{
	TeAttributeList& att = table.attributeList();
	unsigned int i;
	string uniqueVal;
	bool isUniqueValString = false;
	
	if (!beginTransaction())
		return false;

	string uniqueName = table.uniqueName(); // primary key explicitly defined or 
	if (table.uniqueName().empty())			// check in the attribute list
	{								
		for (i=0; i<att.size(); ++i)
			if (att[i].rep_.isPrimaryKey_)
			{
            uniqueName = att[i].rep_.name_;
            table.setUniqueName(uniqueName);
            break;
        }
	}

	TeAttributeList::iterator it;
	TeTableRow row;
	unsigned int j;
	for (i = 0; i < table.size(); i++  )
	{
		bool useComma = false;
		row = table[i];
		it = att.begin();
		string q;
		j = 1;
		int jj = 0;
		while ( it != att.end() )
		{
			if (uniqueName != (*it).rep_.name_)
			{
				if ((*it).rep_.isAutoNumber_)
				{
					++it;
					j++;
					jj++;
					continue;
				}			
				if(useComma == true)
				{
					q += ", ";
				}
				else
				{
					useComma = true;
				}
				q += (*it).rep_.name_ + " = ";

				if(row[jj].empty())
				{
					q += " null";

					++it;
					j++;
					jj++;
					continue;
				}

  				switch ((*it).rep_.type_)
  				{
                case TeSTRING:
                    q += "'"+escapeSequence(row[jj])+"'";
  					break;
                case TeREAL:
					{
						std::string value = row[jj];
						replace(value.begin(), value.end(), ',', '.');
						q += value;
					}
  					break;
                case TeINT:
                    q += row[jj];
  					break;
                case TeDATETIME:
					{
						const string temp_dt = string(row[jj].c_str());
						TeTime t(temp_dt, (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_, (*it).timeSeparator_, (*it).indicatorPM_);
						q += this->getSQLTime(t);
					}
  					break;
                case TeCHARACTER:
                    q += "'" + escapeSequence(row[jj]) + "'";
  					break;
                case TeBLOB:
                    q += "'" + escapeSequence(row[jj]) + "'";
					break;
                default:
                    q += "'"+escapeSequence(row[jj])+"'";
  					break;
  				}
			}
			else
			{
				uniqueVal = row[jj];
				isUniqueValString = ((*it).rep_.type_ == TeSTRING);
			}
			++it;
			j++;
			jj++;
		}
		if (q.empty())
			continue;
		
		if (!uniqueName.empty() && !uniqueVal.empty())  
		{
			if(isUniqueValString)
				q += " WHERE " + uniqueName + " = '" + uniqueVal +"'";
			else
				q += " WHERE " + uniqueName + " = " + uniqueVal;
		}
		string sql = "UPDATE "+ table.name() + " SET " + q;
		if (!execute(sql))
		{
			rollbackTransaction();
			return false;
		}
	}
	if (!commitTransaction())
	{
		rollbackTransaction();
		return false;
	}
	return true;
}

bool 
        TeMySQL::insertTableInfo (int layerId, TeTable &table, const string& user)
{
	string sql;

	sql = "INSERT INTO te_layer_table (unique_id, attr_link,attr_initial_time, attr_final_time, attr_time_unit,attr_table_type, ";
	sql += " user_name, attr_table ";
	if (layerId > 0)
		sql += ",layer_id";
	sql += ") VALUES( ";
	sql += "'" + table.uniqueName() + "', ";
	sql += "'" + table.linkName() + "', ";
	sql += "'" + table.attInitialTime() + "', ";
	sql += "'" + table.attFinalTime() + "', ";
	sql += "" + Te2String(table.attTimeUnit()) + ", ";
	sql += ""+ Te2String(table.tableType()) + ", ";
	sql += "'" + user + "','" + table.name() +"'";
	if (layerId > 0)
		sql += ", " + Te2String(layerId) ;
	sql += ")";

	if (!this->execute(sql))
		return false;

	int id = (this->insertId());
	table.setId(id);
	return true;
}

bool 
        TeMySQL::insertThemeTable (int themeId, int tableId, int relationId, int tableOrder)
{
	string relId;
	if (relationId > 0)
		relId = Te2String(relationId);
	else
		relId = "NULL";

	string sql = "INSERT INTO te_theme_table VALUES (0,";
	sql += Te2String(themeId) + "," + Te2String(tableId) + ",";
	sql += relId + "," + Te2String(tableOrder) + ")";


	if (!execute(sql))
		return false;

	return true;
}

bool
        TeMySQL::insertPolygonSet(const string& table, TePolygonSet &ps)
{
	double		ext_max;
	unsigned int	i, ni, size, npts=100;
	unsigned int k;
	double		*points	= new double[2*sizeof(double)*100];
	char		buf[256], *query, *end;
	int			parentId = 0;
	unsigned int querySize = 100*4*sizeof(double)+256+4;
	query = new char[querySize];
	npts = 0;
	this->beginTransaction();
	for (unsigned int p = 0; p < ps.size(); ++p)
	{
		TePolygon& poly = ps [p];
		parentId = 0;
		for (k=0; k<poly.size(); ++k)
		{
			TeLinearRing& ring(poly[k]);
			TeBox b = ring.box();
			size = ring.size();
			ni = 0;
			ext_max = MAX(b.width(),b.height());
			if (k==0)
				ni = poly.size()-1;
			else
				ni = 0;

			// Create and save the BLOB
			if (points && size > npts)
			{
				delete [] points;
				points = new double[2*sizeof(double)*size];
				npts = size;
			}
			int iac = 0;
			for (i=0;i<size;++i)
			{
				points[iac++]=swappd(ring[i].x());
				points[iac++]=swappd(ring[i].y());
			}

			sprintf (buf,"INSERT INTO %s values(0,'%s',%d,%d,%d,%.15f,%.15f,%.15f,%.15f,%.15f,",
                     table.c_str(),escapeSequence(poly.objectId()).c_str(),size,ni,parentId,b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);

			if (query && (size*4*sizeof(double)+strlen(buf)+4) > querySize)
			{
				delete [] query;
				querySize = size*4*sizeof(double)+strlen(buf)+4;
				query = new char[querySize];
			}
			end = strxmov(query,buf,(char*)0);
			*end++ = '\'';
            end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
			*end++ = '\'';
			*end++ = ')';

            if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
			{
                errorMessage_ = string(mysql_error(mysql_));
				delete []query;
				break;
			}
			ring.geomId (insertId());
			if (k==0)
			{
				parentId = ring.geomId();
				ps[p].geomId(parentId);
			}
		}
		sprintf(buf,"UPDATE %s SET parent_id=%d WHERE geom_id=%d",table.c_str(),parentId,parentId);
        if (mysql_real_query(mysql_,buf,(unsigned int)strlen(buf)) != 0)
		{
            errorMessage_ = string(mysql_error(mysql_));
			delete []query;
			break;
		}	
	}
	if (points)
		delete []points;
	if (query)
		delete []query;
	this->commitTransaction();
	return true;
}

bool 
        TeMySQL::insertPolygon (const string& table, TePolygon &poly )
{
	double	ext_max;

	int	i,
    ni,
    size;
	unsigned int k;
	double	*points	= NULL;
	char	buf[256], *query, *end;


	int parentId = 0;
	for ( k = 0; k < poly.size(); k++ )
	{
		TeLinearRing ring ( poly[k] );
		TeBox b = ring.box();
		size = ring.size();
		ni = 0;
		ext_max = MAX(b.width(),b.height());
		if (k==0)
			ni = poly.size()-1;
		else
			ni = 0;

		// Create and save the BLOB
		points = new double[2*sizeof(double)*size];

		int iac = 0;
		for (i=0;i<size;i++)
		{
			points[iac++]=swappd(ring[i].x());
			points[iac++]=swappd(ring[i].y());
		}

		sprintf (buf,"INSERT INTO %s values(0,'%s',%d,%d,%d,%.15f,%.15f,%.15f,%.15f,%.15f,",
                 table.c_str(),escapeSequence(poly.objectId()).c_str(),size,ni,parentId,b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);

		query = new char[size*4*sizeof(double)+strlen(buf)+4];
		end = strxmov(query,buf,(char*)0);
		*end++ = '\'';
        end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
		*end++ = '\'';
		*end++ = ')';
		delete []points;

        if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
		{
            errorMessage_ = string(mysql_error(mysql_));
			delete []query;
			return false;
		}
		delete []query;
		query = 0;
		ring.geomId (insertId());
		if (k==0)
		{
			parentId = ring.geomId();
			poly.geomId(parentId);
		}
	}
	sprintf(buf,"UPDATE %s SET parent_id=%d WHERE geom_id=%d",table.c_str(),parentId,parentId);
    if (mysql_real_query(mysql_,buf,(unsigned int)strlen(buf)) != 0)
	{
        errorMessage_ = string(mysql_error(mysql_));
		delete []query;
		return false;
	}	
	return true;
}

bool 
        TeMySQL::updatePolygon (const string& table, TePolygon &poly )
{
	double	ext_max;
	int ni;
	unsigned int i,k,size,parentId;
	double	*points	= NULL;
	char	buf[256], message[256], *query, *end;
	TeBox b = poly.box();

	std::string objectId = poly.objectId();
	for ( k = 0; k < poly.size(); k++ )
	{
		TeLinearRing ring ( poly[k] );
		size = ring.size();
		ni = 0;
		if (k==0)
		{
			ext_max = MAX(b.width(),b.height());
			ni = poly.size()-1;
			parentId = ring.geomId();
		}
		else
			ni = 0;

        // Create and save the BLOB
		points = new double[2*sizeof(double)*size];
		int iac = 0;
		for (i=0;i<size;i++)
		{
			points[iac++]=swappd(ring[i].x());
			points[iac++]=swappd(ring[i].y());
		}

		sprintf (buf,"UPDATE %s SET object_id=%s, num_coords=%d, num_holes=%d, parent_id=%d, lower_x=%.15f, lower_y=%.15f, upper_x=%.15f, upper_y=%.15f, ext_max=%.15f, spatial_data=",
                 table.c_str(),escapeSequence(objectId).c_str(),size,ni,parentId,b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);
		query = new char[size*4*sizeof(double)+strlen(buf)+50];
		end = strxmov(query,buf,(char*)0);
		*end++ = '\'';
        end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
		*end++ = '\'';
		sprintf (message," WHERE geom_id=%d",ring.geomId());
		end = strxmov(end,message,0);
		delete []points;

        if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
		{
            errorMessage_ = string(mysql_error(mysql_));
			delete []query;
			return false;
		}
		delete []query;
		query = 0;
	}
	return true;
}


bool 
        TeMySQLPortal::fetchGeometry (TePolygon& poly)
{
	int ni,j;
	TeLinearRing ring = this->getLinearRing(ni);
	poly.objectId ( ring.objectId() );
	poly.geomId ( ring.geomId() );
	poly.add ( ring );
	int parentId = poly.geomId();
	while (fetchRow())
	{
		if (atoi(this->getData("parent_id")) == parentId)
		{
			TeLinearRing ring = getLinearRing(j);
			poly.add ( ring );
		}
		else
			return true;
	}
	return false;
}

bool 
        TeMySQLPortal::fetchGeometry (TePolygon& poly, const unsigned int& initIndex)
{
	int np, ni,k;
	double x,y,*data;
	
	TeLine2D line;
	line.geomId(atoi(getData(initIndex)));
	line.objectId(getData(initIndex+1));
	np = atoi (getData(initIndex+2));
	ni = atoi (getData(initIndex+3));
	data = (double*)getData(initIndex+10);
	for (k=0;k<np;k++)
	{
		x = swappd(*data++);
		y = swappd(*data++);
		TeCoord2D pt(x,y);
		line.add(pt);
	}

	TeLinearRing ring(line); 
	poly.objectId ( ring.objectId() );
	poly.geomId ( ring.geomId() );
	poly.add ( ring );
	int parentId = poly.geomId();
	while (fetchRow())
	{
		if (atoi(this->getData(initIndex+4)) == parentId)
		{
			TeLine2D hole;
			hole.geomId(atoi(getData(initIndex)));
			hole.objectId(getData(initIndex+1));
			np = atoi (getData(initIndex+2));
			data = (double*)getData(initIndex+10);
			for (k=0;k<np;k++)
			{
				x = swappd(*data++);
				y = swappd(*data++);
				TeCoord2D pt(x,y);
				hole.add(pt);
			}

			TeLinearRing ring(hole); 
			poly.add ( ring );
		}
		else
			return true;
	}
	return false;
}


TeLinearRing 
        TeMySQLPortal::getLinearRing (int &ni)
{
	int index = atoi(getData("geom_id"));
	string object_id = getData("object_id");
	int np = atoi (getData("num_coords"));
	ni = atoi (getData("num_holes"));
	TeBox b (getDouble("lower_x"),getDouble("lower_y"),getDouble("upper_x"),getDouble("upper_y"));
	int k;
	TeLine2D line;
	double x,y,*data = (double*)getData("spatial_data");

	for (k=0;k<np;k++)
	{
		x = swappd(*data++);
		y = swappd(*data++);
		TeCoord2D pt(x,y);
		line.add(pt);
	}

	TeLinearRing ring ( line );
	ring.objectId (object_id);
	ring.geomId (index);
	ring.setBox (b);
	return ring;
}

bool 
        TeMySQL::insertLineSet(const string& table, TeLineSet &ls)
{
	char	buf[256],
    *query,
    *end;
	unsigned int npts = 100, size;
	double	*points	= new double[2*sizeof(double)*npts],
    ext_max;
	int		iac;
	TeBox	b;
	unsigned int querySize = 100*4*sizeof(double)+256+4;
	query = new char[querySize];

	unsigned int nl, i;
	this->beginTransaction();
	for ( nl = 0; nl < ls.size(); ++nl)
	{
		TeLine2D& l = ls[nl];
		b=l.box();
		ext_max = MAX(b.width(),b.height());

		size = l.size();
		if (size > npts)
		{
			delete []points;
			points = new double[2*sizeof(double)*size];
			npts = size;
		}
		iac = 0;
		for (i=0;i<size;i++)
		{
			points[iac++]=l[i].x();
			points[iac++]=l[i].y();
		}

		sprintf (buf,"INSERT INTO %s values(0,'%s',%d,%.15f,%.15f,%.15f,%.15f,%.15f,",
                 table.c_str(),
                 escapeSequence(l.objectId()).c_str(),
                 size,
                 b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);

		if ((size*4*sizeof(double)+strlen(buf)+4) > querySize)
		{
			delete []query;
			querySize = size*4*sizeof(double)+strlen(buf)+4;
			query = new char[querySize];
		}
		end = strxmov(query,buf,0);
		*end++ = '\'';
        end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
		*end++ = '\'';
		*end++ = ')';

        if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
		{
            errorMessage_ = string(mysql_error(mysql_));
			break;
		}

		l.geomId (insertId());
	}
	this->commitTransaction();
	delete []points;
	delete []query;
	return true;
}

bool 
        TeMySQL::insertLine (const string& table, TeLine2D &l)
{
	char	buf[256],
    *query,
    *end;
	double	*points	= NULL,
    ext_max;
	int		size=l.size(),
    i,
    iac;
	TeBox	b=l.box();

	ext_max = MAX(b.width(),b.height());
	points = new double[2*sizeof(double)*size];
	iac = 0;

	for (i=0;i<size;i++)
	{
		points[iac++]=l[i].x();
		points[iac++]=l[i].y();
	}

	sprintf (buf,"INSERT INTO %s values(0,'%s',%d,%.15f,%.15f,%.15f,%.15f,%.15f,",
             table.c_str(),
             escapeSequence(l.objectId()).c_str(),
             size,
             b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);

	query = new char[size*4*sizeof(double)+strlen(buf)+4];

	end = strxmov(query,buf,0);
	*end++ = '\'';
    end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
	*end++ = '\'';
	*end++ = ')';
	delete []points;

    if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
	{
        errorMessage_ = string(mysql_error(mysql_));
		delete [] query;
		return false;
	}

	l.geomId (insertId());
	delete [] query;
	return true;
}


bool 
        TeMySQL::updateLine (const string& table, TeLine2D &l)
{
	char	buf[256],
    message[256],
    *query,
    *end;
	double	*points	= NULL,
    ext_max;
	int		size=l.size(),
    i,
    iac;
	TeBox	b=l.box();

	ext_max = MAX(b.width(),b.height());
	points = new double[2*sizeof(double)*size];
	iac = 0;

	for (i=0;i<size;i++)
	{
		points[iac++]=l[i].x();
		points[iac++]=l[i].y();
	}

	sprintf (buf,"UPDATE %s SET num_coords=%d, lower_x=%.15f, lower_y=%.15f, upper_x=%.15f, upper_y=%.15f, ext_max=%.15f, spatial_data=",
             table.c_str(),size,b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),ext_max);
	query = new char[size*4*sizeof(double)+strlen(buf)+50];
	end = strxmov(query,buf,(char*)0);
	*end++ = '\'';
    end += mysql_real_escape_string(mysql_,end,(char*)points,size*2*sizeof(double));
	*end++ = '\'';
	delete []points;

	sprintf (message," WHERE geom_id=%d",l.geomId());
	end = strxmov(end,message,(char*)0);

    if (mysql_real_query(mysql_,query,(unsigned int) (end - query)) != 0)
	{
        errorMessage_ = string(mysql_error(mysql_));
		delete []query;
		return false;
	}
	delete []query;
	return true;
}



bool
        TeMySQLPortal::fetchGeometry(TeLine2D& line)
{
	int index = atoi (getData ("geom_id"));
	string object_id = getData ("object_id");
	int np = atoi (getData("num_coords"));
	TeBox b (atof(getData("lower_x")),atof(getData("lower_y")),atof(getData("upper_x")),atof(getData("upper_y")));
	int k;
	double x,y,*data = (double*)getData("spatial_data");

	for (k=0;k<np;k++)
	{
		x = *data++;
		y = *data++;
		TeCoord2D pt(x,y);
		line.add(pt);
	}
	line.objectId (object_id);
	line.geomId (index);
	line.setBox (b);
	return (fetchRow());
}

bool
        TeMySQLPortal::fetchGeometry(TeLine2D& line, const unsigned int& initIndex)
{
	int index = atoi (getData (initIndex));
	string object_id = getData (initIndex+1);
	int np = atoi (getData(initIndex+2));
	int k;
	double x,y,*data = (double*)getData(initIndex+8);

	for (k=0;k<np;k++)
	{
		x = *data++;
		y = *data++;
		TeCoord2D pt(x,y);
		line.add(pt);
	}
	line.objectId (object_id);
	line.geomId (index);
	return (fetchRow());
}


bool 
        TeMySQL::insertPoint(const string& table,TePoint &p)
{
	string q = "INSERT INTO " + table + " VALUES(0,";
	q += "'" + escapeSequence(p.objectId()) + "',";
	q += Te2String(p.location().x_,15) + ",";
	q += Te2String(p.location().y_,15) + ")";
	
	if (!this->execute(string(q)))
		return false;
	p.geomId(this->insertId());
	return true;
}


bool 
        TeMySQLPortal::fetchGeometry(TePoint& p)
{
	TeCoord2D c(atof (getData("x")), atof (getData("y")));
	p.geomId( atoi(getData("geom_id")));
	p.objectId( string(getData("object_id")));
	p.add(c);
	return (fetchRow());
}

bool 
        TeMySQLPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	TeCoord2D c(atof (getData(initIndex+2)), atof (getData(initIndex+3)));
	p.geomId( atoi(getData(initIndex)));
	p.objectId( string(getData(initIndex+1)));
	p.add(c);
	return (fetchRow());
}


bool 
        TeMySQL::insertText(const string& table, TeText &t)
{
	char q[256];
	sprintf (q,"INSERT INTO %s values(0,'%s',%.15f,%.15f,'%s',%.15f, %.15f, %.15f, %.15f)",
             table.c_str(), escapeSequence(t.objectId()).c_str(),
             t.location().x_, t.location().y_,
             t.textValue().c_str(), t.angle(), t.height(),t.alignmentVert(),t.alignmentHoriz());
	if (!this->execute(string(q)))
		return false;
	t.geomId(this->insertId());

	return true;
}


bool 
        TeMySQL::insertArc(const string& table, TeArc &arc)
{
	char q[256];
	sprintf (q,"INSERT INTO %s values(0,'%s',%d,%d)",
             table.c_str(),escapeSequence(arc.objectId()).c_str(),arc.fromNode().geomId(),arc.toNode().geomId());
	if (!this->execute(string(q)))
		return false;
	arc.geomId(this->insertId());
	return true;
}


bool
        TeMySQL::insertNode(const string& table, TeNode &node)
{
	char q[256];
	sprintf (q,"INSERT INTO %s values(0,'%s',%.15f,%.15f)",table.c_str(),node.objectId().c_str(),node.location().x(),node.location().y());
	if (!this->execute(string(q)))
		return false;
	node.geomId(this->insertId());
	return true;
}

bool 
        TeMySQL::insertBlob (const string& tableName, const string& columnBlob , const string& whereClause, unsigned char* data, int size)
{
	if (whereClause.empty())
		return false;

	TeMySQLPortal* portal = (TeMySQLPortal*)this->getPortal();
	if (!portal)
		return false;

	string q = "SELECT * FROM "+ tableName +" WHERE "+ whereClause;
	if ((!portal->query(q)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}
	delete portal;

	char	message[256],
    *query,
    *end;
	sprintf (message,"UPDATE %s SET %s = ",tableName.c_str(), columnBlob.c_str());

	query = new char[2*size+strlen(message)+4];
	end = strxmov(query,message,0);
	*end++ = '\'';
    end += mysql_real_escape_string(mysql_,end,(char*)data,size);
	*end++ = '\'';

	sprintf (message," WHERE %s",whereClause.c_str());

	end = strxmov(end,message,0);

    if (mysql_real_query(mysql_,query,(unsigned int) (end - query)))
	{
        errorMessage_ = string(mysql_error(mysql_));
		delete []query;
		return false;
	}
	delete []query;	
	return true;
}

bool
        TeMySQL::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur,
                                   unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{
	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeMySQLPortal* portal = (TeMySQLPortal*) this->getPortal();
	if (!portal)
		return false;

	bool update = false;
	string q ="SELECT * FROM " + table; 
	q += " WHERE block_id='" + blockId + "'";

	if (!portal->query (q))
	{
		delete portal;
		return false;
	}
    // check if this block is alread in the database
	if (portal->fetchRow())
		update = true;
	delete portal;

	char	message[256],
    *end;
	if (!update)
	{
		sprintf (message,"INSERT INTO %s values('%s',%.15f,%.15f,%.15f,%.15f,%d,%d,%d,",
                 table.c_str(),blockId.c_str(),ll.x(),ll.y(),ur.x(),ur.y(),band,res,subband);
	}
	else
	{
		sprintf (message,"UPDATE %s SET spatial_data=",table.c_str());
	}

	if (bufferBlobSize_ < (long)(2*size+strlen(message)+20))
	{
		if (bufferBlob_)
			delete []bufferBlob_;
		bufferBlobSize_ = 2*size+strlen(message)+20;
		bufferBlob_ = new char[bufferBlobSize_];
	}
	end = strxmov(bufferBlob_,message,0);
	*end++ = '\'';
    end += mysql_real_escape_string(mysql_,end,(char*)buf,size);
	*end++ = '\'';

	if (update)
	{
		sprintf (message,", block_size=%ld WHERE block_id='%s'",size,blockId.c_str());
		end = strxmov(end,message,0);
	}
	else
	{
		sprintf (message,",%ld)",size);
		end = strxmov(end,message,0);
	}

    if (mysql_real_query(mysql_,bufferBlob_,(unsigned int) (end - bufferBlob_)))
	{
        errorMessage_ = string(mysql_error(mysql_));
		delete []bufferBlob_;
		bufferBlobSize_ = 0;
		bufferBlob_ = 0;
		return false;
	}
	return true;
}

bool
        TeMySQLPortal::fetchGeometry(TeNode& n)
{
	TeCoord2D c(atof (getData("x")), atof (getData("y")));
	n.geomId( atoi(getData("geom_id")));
	n.objectId( string(getData("object_id")));
	n.add(c);
	return (fetchRow());
}

bool
        TeMySQLPortal::fetchGeometry(TeNode& n, const unsigned int& initIndex)
{
	TeCoord2D c(atof (getData(initIndex+2)), atof (getData(initIndex+3)));
	n.geomId( atoi(getData(initIndex)));
	n.objectId( string(getData(initIndex+1)));
	n.add(c);
	return (fetchRow());
}

bool
        TeMySQL::insertCell(const string& table, TeCell &c)
{
	TeBox	b=c.box();
	char	q[256];
	sprintf (q,"INSERT INTO %s values(0,'%s',%.15f,%.15f,%.15f,%.15f,%d,%d)",
             table.c_str(),c.objectId().c_str(),
             b.lowerLeft().x(),b.lowerLeft().y(),b.upperRight().x(),b.upperRight().y(),
             c.column(),c.line());

	if (!this->execute (string(q)))
		return false;
	c.geomId(this->insertId());
	return true;
}


TeTime
        TeMySQLPortal::getDate (int i)
{ 
	string s = getData(i);
	TeTime t(s, TeSECOND, "YYYYsMMsDDsHHsmmsSS","-"); 
	return t; 
}

TeTime
        TeMySQLPortal::getDate (const string& s)
{ 
	string sv = getData(s);
	TeTime t(sv, TeSECOND, "YYYYsMMsDDsHHsmmsSS","-"); 
	return t; 
}

string 
        TeMySQLPortal::getDateAsString(const string& s)
{
	TeTime t = this->getDate(s);
	if (t.isValid())
	{		
		string tval = "\'"+t.getDateTime("YYYYsMMsDDsHHsmmsSS", "-")+"\'";
		return tval;
	}
	else
		return "";
}

string 

        TeMySQLPortal::getDateAsString(int i)
{
	TeTime t = this->getDate(i);
	if (t.isValid())
	{		
		string tval = "\'"+t.getDateTime("YYYYsMMsDDsHHsmmsSS", "-")+"\'";
		return tval;
	}
	else
		return "";
}

double TeMySQLPortal::getDouble (const string& s)
{
	return atof(getData(s));
}


double TeMySQLPortal::getDouble (int i)
{
	if (row_ == 0)
		return 0.;

	if (row_[i])
		return atof(row_[i]);
	else
		return 0.;
}

int TeMySQLPortal::getInt (const string& s)
{
	return atoi(getData(s));
}


int TeMySQLPortal::getInt (int i)
{
	if (row_ == 0)
		return 0;

	if (row_[i])
		return atoi(row_[i]);
	else
		return 0;
}


char* 
        TeMySQLPortal::getData (int i)
{
	if (row_ == 0)
    {
        static char emptyStr = '\0';
		return &emptyStr;
    }

	if (row_[i])
		return row_[i];
	else
    {
        static char emptyStr = '\0';
        return &emptyStr;
    }
}


char* 
        TeMySQLPortal::getData (const string& s)
{
	string fieldName;
    //	size_t pos = s.find(".", string::npos,1);
	size_t pos = s.find(".", 0, 1);
	if (pos != string::npos)
		fieldName = s.substr(pos+1);
	else
		fieldName = s;

	if (row_ == 0)
    {
        static char emptyStr = '\0';
        return &emptyStr;
    }

	TeAttributeList::iterator it = attList_.begin();

	int j = 0;
	while ( it != attList_.end() )
	{
		if ((TeConvertToUpperCase(fieldName) == TeConvertToUpperCase((*it).rep_.name_)) ||
		    (TeConvertToUpperCase(s) == TeConvertToUpperCase((*it).rep_.name_)))
		{
			if (row_[j])
				return row_[j];
			else
            {
                static char emptyStr = '\0';
                return &emptyStr;
            }
		}
		++it;
		j++;
	}
	
    static char emptyStr = '\0';
    return &emptyStr;
}

bool
        TeMySQLPortal::getBlob (const string& s, unsigned char* &data, long& size)
{
	string fieldName;
    //	size_t pos = s.find(".", string::npos,1);
	size_t pos = s.find(".", 0, 1);
	if (pos != string::npos)
		fieldName = s.substr(pos+1);
	else
		fieldName = s;

	TeAttributeList::iterator it = attList_.begin();
	int j = 0, i=-1;
	while ( it != attList_.end() )
	{
		if (fieldName == (*it).rep_.name_)
		{
			i=j;
			break;
		}
		++it;
		j++;
	}
	if (i<0)
		return false;

	unsigned long *lengths;
	lengths = mysql_fetch_lengths(result_);
	size = lengths[i];
	if (row_[i])
	{
        //		data = (unsigned char*) row_[i];
		data = new unsigned char[size];
		memcpy(data, (unsigned char*) row_[i], size * sizeof(char));
		return true;
	}
	else
	{
		data = 0;
		return false;
	}
}


bool 
        TeMySQLPortal::getBool (const string& s)
{
	int val = atoi(getData(s));
	return val ? true : false;
}


bool 
        TeMySQLPortal::getBool (int i)
{
	if (row_ == 0)
		return false;
	int val = atoi(getData(i));
	return val ? true : false;
}

bool 
        TeMySQLPortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{
	
	// get the actual length of the compressed data
	size = atoi(getData("block_size"));
	if ( (size>0) && row_[8])
	{
		memcpy((void*)ptData,(void*)(row_[8]),size);
		return true;
	}
	else
	{
		ptData = 0;
		return false;
	}
}

bool
        TeMySQL::inClauseValues(const string& query, const string& attribute, vector<string>& inClauseVector)
{
	int i = 0, numRows = 0, count = 0, chunk = 200;
	string id, inClause = "(", plic = "'";

	TeDatabasePortal *pt = getPortal();
	if(pt->query(query) == false)
	{
		delete pt;
		return false;
	}

	while (numRows < pt->numRows())
	{
		if(count < chunk && pt->fetchRow())
		{
			id = pt->getData(attribute);
			inClause = inClause + plic + id + "',";
			++count;
			++numRows;
		}
		else
		{
			inClause = inClause.substr(0, inClause.size()-1);
			inClause += ")";
			inClauseVector.push_back(inClause);
			count = 0;
			++i;
			inClause = "(";
		}
	}
	if (numRows == 0)
	{
		delete pt;
		return true;
	}
	else
	{
		inClause = inClause.substr(0, inClause.size()-1);
		inClause += ")";
		inClauseVector.push_back(inClause);
	}

	delete pt;
	return true;
}

string 
        TeMySQL::getSQLTime(const TeTime& time) const
{
	string result = "'"+ time.getDateTime("YYYYsMMsDDsHHsmmsSS", "-", ":") +"'";
	return result;
}

string
        TeMySQL::getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel,
                                     const string& initialTime, const string& finalTime)
{
	
	//In MySQL the first day of the week is monday with index 0
	if(chr==TeDAYOFWEEK)
	{
		time1 = time1-1;
		if(time1<0)
			time1=6;
		time2 = time2-1;
		if(time2<0)
			time2=6;
	}

	return(TeDatabase::getSQLTemporalWhere(time1, time2, chr, rel, initialTime, finalTime));
}


string TeMySQL::concatValues(vector<string>& values, const string& unionString)
{
	string concat = "CONCAT(";
	
	for(unsigned int i = 0; i < values.size(); ++i)
	{
		if(i != 0)
		{
			concat += ", ";

			if(!unionString.empty())
			{
				concat += "'";
				concat += unionString;
				concat += "'";
				concat += ", ";
			}
		}

		concat += values[i];
	}

	concat += ")";

	return concat;
}

string TeMySQL::toUpper(const string& value)
{
	string result  = "upper(";
    result += value;
    result += ")";

	return result;
}


bool TeMySQL::listDatabases(std::vector<std::string> &databaseList) 
{ 
	std::string				sql;
	std::string				value;
	TeDatabasePortal		*myPortal=0;


	sql = "SHOW DATABASES";
	databaseList.clear();
	if((myPortal= getPortal()) != 0)
	{
		if(myPortal->query(sql) == true)
		{
			while(myPortal->fetchRow() == true)
			{
				value=myPortal->getData(0);
				databaseList.push_back(value);
			}
			myPortal->freeResult();
		}
		delete myPortal;
		myPortal =  NULL;
	}
	return !databaseList.empty();
}


bool TeMySQL::dropDatabase(const std::string &databaseName) 
{ 
	std::string sql;

	sql="DROP DATABASE " + databaseName;
	return this->execute(sql);
} 
