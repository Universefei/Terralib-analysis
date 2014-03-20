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


#include <stdio.h>
#include <direct.h>
#include <sys/stat.h>
#include "TeAdoDB.h"
#include <TeUtils.h>
#include "TeProject.h"


#define CHUNKSIZE	240


//colTemp.DefinedSize = 24


typedef map<int,TeNode> TeNodeMap;

inline void TESTHR( HRESULT hr )
{
	if( FAILED(hr) ) _com_issue_error( hr );
}

bool ShowDataLink( _bstr_t * bstr_ConnectString )
{
	HRESULT		hr;
	OLEDB::IDataSourceLocatorPtr	p_IDSL= NULL;			// This is the Data Link dialog object
	ADODB::_ConnectionPtr			p_conn = NULL;			// We need a connection pointer too
	bool							b_ConnValid = false;
	
	try
	{
		hr = p_IDSL.CreateInstance( __uuidof( OLEDB::DataLinks ));
		TESTHR( hr );

		if( *bstr_ConnectString == _bstr_t("") )
		{
			p_conn = p_IDSL->PromptNew();
			if( p_conn != NULL ) b_ConnValid = true;
			
		}
		else 
		{
			p_conn.CreateInstance( "ADODB.Connection" );
			p_conn->ConnectionString = *bstr_ConnectString;
			IDispatch * p_Dispatch = NULL;
			p_conn.QueryInterface( IID_IDispatch, (LPVOID*) & p_Dispatch );
			
			if( p_IDSL->PromptEdit( &p_Dispatch ))
				b_ConnValid = true;
			
			p_Dispatch->Release();
		}
		if( b_ConnValid )
			*bstr_ConnectString = p_conn->ConnectionString;
	}
	catch( _com_error & e )
	{
		cout << "\nCom Exception raised\n";
		cout << "Description : " << (char*) e.Description() << "\n";
		cout << "Message     : " << (char*) e.ErrorMessage() << "\n";		
		return false;
	}
	catch(...)
	{
		return false;
	}

	return b_ConnValid;
}


string 
TeAdo::systemDateTimeFormat(string& indAM, string& indPM, string& sepD, string& sepT)
{
	string key = "Control Panel\\International";
	string dateFormat = "sShortDate";	//formato da data d/M/yyyy
	string timeFormat = "sTimeFormat";	//formato da hora HH:mm:ss
	string hourFormat = "iTime";		//12 horas (0) ou 24 horas(1)
	string indicatorAM = "s1159";		//AM
	string indicatorPM = "s2359";		//PM
	string dateSeparator = "sDate";		//separador de data
	string timeSeparator = "sTime";		//separador de hora

	string rdateFormat;
	string rtimeFormat;
	string rhourFormat;
	string rdateSeparator;
	string rtimeSeparator; 
	
	HKEY    hk;
	DWORD	DataSize = 1024;
    DWORD   Type = REG_SZ;
    char    buf[1024];
    
    if (RegOpenKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hk) == ERROR_SUCCESS)
	{
		memset (buf, 0, 1024);
		DataSize = 1024;
		//date format
		if (RegQueryValueExA(hk, dateFormat.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			rdateFormat = buf;
		
		memset (buf, 0, 1024);
		DataSize = 1024;
		//date separator
		if (RegQueryValueExA(hk, dateSeparator.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			rdateSeparator = buf;
		
		memset (buf, 0, 1024);
		DataSize = 1024;
		//time format
		if (RegQueryValueExA(hk, timeFormat.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			rtimeFormat = buf;
		
		memset (buf, 0, 1024);
		DataSize = 1024;
		//hour format
		if (RegQueryValueExA(hk, hourFormat.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			rhourFormat = buf;

		memset (buf, 0, 1024);
		DataSize = 1024;
		//indicator AM
		if (RegQueryValueExA(hk, indicatorAM.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			indAM = buf;

		memset (buf, 0, 1024);
		DataSize = 1024;
		//indicator PM 
		if (RegQueryValueExA(hk, indicatorPM.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			indPM = buf;

		memset (buf, 0, 1024);
		DataSize = 1024;
		//time separator
		if (RegQueryValueExA(hk, timeSeparator.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)
			rtimeSeparator = buf;
	}
	else
		return "";
	
	sepD = rdateSeparator;
	sepT = rtimeSeparator;

	//DATE
	//first
	int pos = rdateFormat.find(rdateSeparator);
	string firstD = rdateFormat.substr(0,pos);
	string temp = rdateFormat.substr(pos+1);
	//second and third
	pos = temp.find(rdateSeparator);
	string secondD = temp.substr(0,pos);
	string thirdD = temp.substr(pos+1);

	//passar para o formato
	if(firstD.find("a")==0)
		replace(firstD.begin(), firstD.end(), 97, 89);
	else if(secondD.find("a")==0)
		replace(secondD.begin(), secondD.end(),97, 89);
	else if(thirdD.find("a")==0)
		replace(thirdD.begin(), thirdD.end(), 97, 89);

	//TIME
	//first
	pos = rtimeFormat.find(rtimeSeparator);
	string firstT = rtimeFormat.substr(0,pos);
	temp = rtimeFormat.substr(pos+1);
	//second and third
	pos = temp.find(rtimeSeparator);
	string secondT = temp.substr(0,pos);
	int posEmpth = temp.find(" ");
	string thirdT;
	
	if(posEmpth==-1)
		thirdT = temp.substr(pos+1);
	else
		thirdT = temp.substr(pos+1, (posEmpth-(pos+1)));

	int hFormat = atoi(rhourFormat.c_str());

	//passar para o formato
	firstT = TeConvertToUpperCase(firstT);
	secondT = TeConvertToUpperCase(secondT);
	thirdT = TeConvertToUpperCase(thirdT);

	if((firstT.find("M")==0))
		replace(firstT.begin(), firstT.end(), 77, 109);
	else if(secondT.find("M")==0)
		replace(secondT.begin(), secondT.end(), 77, 109);
	else if(thirdT.find("M")==0)
		replace(thirdT.begin(), thirdT.end(), 77, 109);
		
	string timef;
	if(hFormat==0)
		timef = "sTT";
	else
		timef = "";

	string result = TeConvertToUpperCase(firstD) +"s"+ 
					TeConvertToUpperCase(secondD) +"s"+ 
					TeConvertToUpperCase(thirdD) +"s"+ 
					firstT +"s"+ 
					secondT +"s"+ 
					thirdT + timef;
	
	RegCloseKey (hk);
	return result;
}


TeAdo::TeAdo()
{
    HRESULT hr = CoInitialize(0);
	dbmsName_ = "Ado";
	//return system datetime format
	systemDateTimeFormat_ = systemDateTimeFormat(systemIndAM_, systemIndPM_, systemDateSep_, systemTimeSep_);
    if(FAILED(hr))
    {
        cout << "Can't start COM!? " << endl;
    }
}

TeAdo::~TeAdo()
{
}

void 
TeAdo::connection (ADODB::_ConnectionPtr conn)
{
	close();
	connection_ = conn;
	isConnected_ = true;
	user_ = _bstr_t(connection_->Properties->GetItem("User Id")->Value);
	password_ = _bstr_t(connection_->Properties->GetItem("Password")->Value);
}

bool 
TeAdo::newDatabase(const string& database, const string& user, const string& password , const string& /* host */, const int& /* port */, bool terralibModel, const std::string& characterSet)
{
	_bstr_t	bstr_myConnectString = "";

	if (database.empty())
		return false;


	HKEY hKey;
    DWORD dwType=REG_SZ;
	DWORD bufSize = 128;
	char buf[128];

	if(database.find(".accdb",0)!=string::npos)
	{
		if(RegOpenKeyExA(HKEY_CLASSES_ROOT,"Microsoft.ACE.OLEDB.12.0", 0L, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueExA(hKey, "", NULL, &dwType,(LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
				{
					connectionString_ = "Provider=";
					connectionString_ += buf;
					connectionString_ += ";Data Source=";
					RegCloseKey(hKey);
				}
		}else
		{
			isConnected_ = false;
			RegCloseKey(hKey);
			errorMessage_ = "The Microsoft Jet Engine ACE OLEDB 12.0 is not installed";
			return false;
		}

	}else if( RegOpenKeyExA(HKEY_CLASSES_ROOT, "Microsoft.Jet.OLEDB.4.0", 0L, KEY_READ , &hKey) == ERROR_SUCCESS ||
			  RegOpenKeyExA(HKEY_CLASSES_ROOT,"Microsoft.Jet.OLEDB.3.51", 0L, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
		if (RegQueryValueExA(hKey, "", NULL, &dwType,(LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
		{
			connectionString_ = "Provider=";
			connectionString_ += buf;
			connectionString_ += ";Data Source=";
			RegCloseKey(hKey);
		}
	}
	else
	{
		isConnected_ = false;
		RegCloseKey(hKey);
		errorMessage_ = "The Microsoft Jet Engine 4.0 or 3.51 is not installed";
		return false;
	}

	connectionString_ += database;

	try
	{
		// Define ADOX object pointers to Catalog (Database)
		ADOX::_CatalogPtr m_pCatalog  = NULL;

		//Create object instances:
		m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog));

		//Create Database
		m_pCatalog->Create(connectionString_.c_str());

		connection_.CreateInstance(__uuidof(ADODB::Connection));
		bstr_myConnectString = connectionString_.c_str();
		HRESULT hr  = connection_->Open (bstr_myConnectString,"","",-1);
		TESTHR( hr );
		m_pCatalog  = NULL;
	}
	catch(_com_error &e)
	{
		isConnected_ = false;
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		isConnected_ = false;
		errorMessage_ = "Oppps !";
		return false;
	}
	isConnected_ = true;

	database_= database;

	if(terralibModel)
	{
		//create conceptual model
		if(!this->createConceptualModel())
			return false;
	}

	return true;
}

bool 
TeAdo::connect (const string& fullDatabaseName)
{
	_bstr_t	bstr_myConnectString = "";

	if (fullDatabaseName.empty())
	{
		if (!ShowDataLink( &bstr_myConnectString ))
		{
			isConnected_ = false;
			return false;
		}
	}
	
	HKEY hKey;
    DWORD dwType=REG_SZ;
	DWORD bufSize = 128;
	char buf[128];

	if(fullDatabaseName.find(".accdb",0)!=string::npos)
	{
		if(RegOpenKeyExA(HKEY_CLASSES_ROOT,"Microsoft.ACE.OLEDB.12.0", 0L, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueExA(hKey, "", NULL, &dwType,(LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
				{
					connectionString_ = "Provider=";
					connectionString_ += buf;
					connectionString_ += ";Data Source=";
					RegCloseKey(hKey);
				}
		}else
		{
			isConnected_ = false;
			RegCloseKey(hKey);
			errorMessage_ = "The Microsoft Jet Engine ACE OLEDB 12.0 is not installed";
			return false;
		}

	}else if(RegOpenKeyExA(HKEY_CLASSES_ROOT, "Microsoft.Jet.OLEDB.4.0", 0L, KEY_READ , &hKey) == ERROR_SUCCESS ||
			 RegOpenKeyExA(HKEY_CLASSES_ROOT,"Microsoft.Jet.OLEDB.3.51", 0L, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
		if (RegQueryValueExA(hKey, "", NULL, &dwType,(LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
		{
			connectionString_ = "Provider=";
			connectionString_ += buf;
			connectionString_ += ";Data Source=";
			RegCloseKey(hKey);
		}
	}
	else
	{
		isConnected_ = false;
		RegCloseKey(hKey);
		errorMessage_ = "The Microsoft Jet Engine 4.0 or 3.51 is not installed";
		return false;
	}

	connectionString_ += fullDatabaseName;
	bstr_myConnectString = connectionString_.c_str();

	try
	{
		connection_.CreateInstance(__uuidof(ADODB::Connection));
		HRESULT hr  = connection_->Open (bstr_myConnectString,"","",-1);
		TESTHR( hr );
	}
	catch(_com_error &e)
	{
		isConnected_ = false;
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		isConnected_ = false;
		errorMessage_ = "Oppps !";
		return false;
	}
	isConnected_ = true;

	database_ = fullDatabaseName;
	return true;
}


bool 
TeAdo::connect (const string& host, const string& user, const string& password, const string& database, int /* port */)
{
	char	myConnectString[512] = {0};


	if(database.find(".accdb",0)!=string::npos)
	{
		sprintf(myConnectString,"Provider=Microsoft.ACE.OLEDB.12.0;Data Source=%s;User Id=%s;Password=%s",
			    database.c_str(),user.c_str(),password.c_str());
	}else
	{
		sprintf(myConnectString,"provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=%s;Password=%s",
			    database.c_str(),user.c_str(),password.c_str());
	}

	_bstr_t		bstr_myConnectString=myConnectString;
	try
	{
		connection_.CreateInstance(__uuidof(ADODB::Connection));
		HRESULT hr  = connection_->Open (bstr_myConnectString,"","",-1);
		TESTHR( hr );
	}
	catch(_com_error &e)
	{
		isConnected_ = false;
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		isConnected_ = false;
		errorMessage_ = "Oppps !";
		return false;
	}
	isConnected_ = true;
	database_ = database;

	return true;
}


void 
TeAdo::close()
{
	if (isConnected_)
	{
		if(transactionCounter_ > 0)
		{
			connection_->RollbackTrans();
		}
		connection_->Close();
	}
	isConnected_ = false;
	transactionCounter_ = 0;
}

bool 
TeAdo::execute (const string &q)
{
	try
	{
		long options = ADODB::adCmdText;// || ADODB::adExecuteNoRecords;
		connection_->Execute(_bstr_t(q.c_str()),NULL, options);
	}
	catch(_com_error &e)
	{
		
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

string
TeAdo::escapeSequence(const string& from)
{
	int	fa = 0;
	string to = from;
	to.insert(0, " ");
	string::iterator it = to.begin();
	while(it != to.end())
	{
		int f = to.find("'", fa);
		if(f > fa)
		{
			to.insert(f, "'");
			fa = f + 2;
		}
		else
			break;
	}
	to = to.substr(1);
	return to;
}

bool 
TeAdo::tableExist(const string& table)
{
	if (table.empty())
		return false;
	ADOX::_CatalogPtr pCatalog = NULL;
    HRESULT hr = S_OK;
	try
	{
		TESTHR(hr = pCatalog.CreateInstance(__uuidof(ADOX::Catalog)));

// Connect the catalog.
		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));

		long tableCount = pCatalog->Tables->Count;
		for(long i = 0; i < tableCount; ++i)
		{
			if(pCatalog->Tables->Item[i]->Name == (_bstr_t)table.c_str())
			{
				pCatalog = NULL;
				return true;
			}
		}
		return false;
	}
	catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		return false;
    }
    catch(...)
    {
		errorMessage_ = "Oppps ! ";
		return false;
    }
	return true;
}

bool
TeAdo::listTables(vector<string>& tableList)
{
	tableList.clear();
	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
    HRESULT hr = S_OK;
	_variant_t value;
	_bstr_t	bbvalue_;
	try
	{
		TESTHR(hr = pCatalog.CreateInstance(__uuidof(ADOX::Catalog)));

		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));
		int n = pCatalog->Tables->Count;
		for (int i=0; i<n; i++)
		{
			pTable = pCatalog->Tables->GetItem((long)i);
			value = pTable->Name;
			if (value.vt != VT_NULL)
			{
				bbvalue_ = _bstr_t(value);
				tableList.push_back(string((char*)bbvalue_));
			}
		}
		pCatalog = NULL;
		pTable = NULL;
		return true;
	}
	catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		pCatalog = NULL;
		pTable = NULL;
		return false;
    }
    catch(...)
    {
		errorMessage_ = "Oppps ! ";
		pCatalog = NULL;
		pTable = NULL;
		return false;
    }
	return true;
}

bool 
TeAdo::columnExist(const string& table, const string& column, TeAttribute& attr)
{
	if (table.empty())
	{
		return false;
	}
	if (column.empty())
	{
		return false;
	}

	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
    HRESULT hr = S_OK;
	try
	{
		TESTHR(hr = pCatalog.CreateInstance(__uuidof(ADOX::Catalog)));

// Connect the catalog.
		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));

// Get the table
		pTable = pCatalog->Tables->GetItem((_bstr_t)table.c_str ());
		if (pTable)
		{
			ADOX::ColumnsPtr columns = pTable->GetColumns();
			if(columns == NULL)
			{
				pTable = NULL;
				pCatalog = NULL;
				return false;
			}
			
			ADOX::_ColumnPtr columnAdo = NULL;
			bool found = false;
			for(long i = 0; i < columns->GetCount(); ++i)
			{
				columnAdo = columns->GetItem(i);
				if(columnAdo == NULL)
				{
					pTable = NULL;
					pCatalog = NULL;
					return false;
				}
				if(columnAdo->GetName() == (_bstr_t)column.c_str())
				{
					found = true;
					break;
				}
			}			
			if(!found)
			{
				pTable = NULL;
				pCatalog = NULL;
				return false;
			}
			
			int nType = columnAdo->GetType();
			switch (nType)
			{
				case ADODB::adBoolean:
				case ADODB::adNumeric:
				case ADODB::adInteger:
				case ADODB::adSmallInt:
				case ADODB::adTinyInt:
				case ADODB::adUnsignedInt:
				case ADODB::adUnsignedSmallInt:
				case ADODB::adUnsignedTinyInt:
				case ADODB::adVarNumeric:
					attr.rep_.type_ = TeINT;
					break;

				case ADODB::adSingle:
				case ADODB::adDouble:
					attr.rep_.type_ = TeREAL;
					break;

				case ADODB::adDate:
				case ADODB::adDBDate:
				case ADODB::adDBTime:
				case ADODB::adDBTimeStamp:
					attr.rep_.type_ = TeDATETIME;
					break;

				case ADODB::adLongVarBinary:
				case ADODB::adVarBinary:
				case ADODB::adBinary:
					attr.rep_.type_ = TeBLOB;
					break;

				case ADODB::adWChar:
				case ADODB::adVarChar:
				case ADODB::adVarWChar:
				case ADODB::adLongVarWChar: 
				case ADODB::adLongVarChar:
				case ADODB::adChar: 
					attr.rep_.type_ = TeSTRING;
					break;

				case ADODB::adCurrency: // ??? CURRENCY
				default :
					attr.rep_.type_ = TeUNKNOWN;
					break;
			}
			
			attr.rep_.name_ = columnAdo->GetName();
			attr.rep_.numChar_ = columnAdo->GetDefinedSize();

			pTable = NULL;
			pCatalog = NULL;
			return true;
		}
		else
		{
			pCatalog = NULL;
			return false;
		}
	}
	catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		return false;
    }
    catch(...)
    {
		errorMessage_ = "Oppps ! ";
		return false;
    }

	return false;
	/*ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(table.c_str(),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
	}
	catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		return false;
    }
	try
	{	
		ADODB::FieldsPtr ptrFields = NULL;
		ADODB::FieldPtr ptrField = NULL;
		ptrFields = recset_->Fields;
		int ncols = recset_->Fields->Count;
        for (int i = 0; i < ncols; i++)
        {
			_variant_t vCol((long)i);
 			ptrFields->get_Item(vCol, &ptrField);
			string name = ptrField->Name;
            if (TeStringCompare(name,column,false))
			{
				int nType = ptrField->Type;
				switch (nType)
				{
					case ADODB::adBoolean:
					case ADODB::adNumeric:
					case ADODB::adInteger:
					case ADODB::adSmallInt:
					case ADODB::adTinyInt:
					case ADODB::adUnsignedInt:
					case ADODB::adUnsignedSmallInt:
					case ADODB::adUnsignedTinyInt:
					case ADODB::adVarNumeric:
						attr.rep_.type_ = TeINT;
						break;

					case ADODB::adSingle:
					case ADODB::adDouble:
						attr.rep_.type_ = TeREAL;
						break;

					case ADODB::adDate:
					case ADODB::adDBDate:
					case ADODB::adDBTime:
					case ADODB::adDBTimeStamp:
						attr.rep_.type_ = TeDATETIME;
						break;

					case ADODB::adLongVarBinary:
					case ADODB::adVarBinary:
					case ADODB::adBinary:
						attr.rep_.type_ = TeBLOB;
						break;

					case ADODB::adWChar:
					case ADODB::adVarChar:
					case ADODB::adVarWChar:
					case ADODB::adLongVarWChar: 
					case ADODB::adLongVarChar:
					case ADODB::adChar: 
						attr.rep_.type_ = TeSTRING;
						break;

					case ADODB::adCurrency: // ??? CURRENCY
					default :
						attr.rep_.type_ = TeUNKNOWN;
						break;
				}
				attr.rep_.name_ = ptrField->Name;
				attr.rep_.numChar_ = ptrField->DefinedSize;
				recset_->Close();
				return true;
		   }
		}
	}	  
	catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		recset_->Close();
		return false;
    }
    catch(...)
    {
		recset_->Close();
		errorMessage_ = "Oppps ! ";
		return false;
    }
	recset_->Close();
	return false;*/
}

bool 
TeAdo::allowEmptyString(const string& tableName, const string& column)
{
	if (tableName.empty() || column.empty())
		return false;

	HRESULT hr = S_OK;
	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
	
	try
	{
        TESTHR(hr = pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));
		pTable = pCatalog->Tables->GetItem((_bstr_t)tableName.c_str());

		pTable->Columns->GetItem(column.c_str())->Properties->GetItem("Jet OLEDB:Allow Zero Length")->Value = true;

		pCatalog->Tables->Refresh();
		pCatalog = NULL;
		pTable = NULL;
	}
    catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		pCatalog = NULL;
		pTable = NULL;
		return false;
    }
    catch(...)
    {
		errorMessage_ = "Error"; 
		pCatalog = NULL;
		pTable = NULL;
		return false;
    }
	return true;
}

bool
TeAdo::createTable(const string& table, TeAttributeList &attr)
{
	HRESULT hr = S_OK;
	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
	ADOX::_KeyPtr pKey = NULL; 

	TeAttributeList::iterator it;
	TeAttributeList validPk;   
	try
    { 
        TESTHR(hr = pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
        TESTHR(hr = pTable.CreateInstance(__uuidof (ADOX::Table)));

		pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));
		pTable->Name=table.c_str();
		pTable->ParentCatalog = pCatalog;
		it = attr.begin();
		while ( it != attr.end() )
		{
			string attName = (*it).rep_.name_;

			switch ((*it).rep_.type_)
  			{
  				case TeSTRING:
					if((*it).rep_.numChar_ > 0)
					{
						if ((*it).rep_.numChar_<256)
							pTable->Columns->Append(attName.c_str(), ADOX::adVarWChar,(*it).rep_.numChar_);
						else
							pTable->Columns->Append(attName.c_str(), ADOX::adLongVarWChar,(*it).rep_.numChar_);
					}
					else
					{
						//iSize = 66560; // o número 66560 equivale a 65K
						pTable->Columns->Append(attName.c_str(), ADOX::adLongVarWChar, 66560);
					}
  					break;
  				
				case TeREAL:
					pTable->Columns->Append(attName.c_str(),ADOX::adDouble,sizeof(double));
  					break;
  				
				case TeINT:
				case TeUNSIGNEDINT:
					pTable->Columns->Append(attName.c_str(), ADOX::adInteger,0);
  					break;
				
				case TeCHARACTER:
					pTable->Columns->Append(attName.c_str(), ADOX::adWChar,0);
					break;
				
				case TeDATETIME:
					pTable->Columns->Append(attName.c_str(), ADOX::adDate,0);
					break;
				
				case TeBLOB:
					pTable->Columns->Append(attName.c_str(), ADOX::adLongVarBinary,0);
					break;

				case TeBOOLEAN:
					pTable->Columns->Append(attName.c_str(), ADOX::adBoolean,0);
					break;
				
				case TePOINTTYPE:
				case TePOINTSETTYPE:
				case TeNODETYPE:
				case TeNODESETTYPE:
					pTable->Columns->Append("x", ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("y", ADOX::adDouble,sizeof(double));
					++it;
					continue;
				
				case TeLINE2DTYPE:
				case TeLINESETTYPE:
					pTable->Columns->Append("num_coords",ADOX::adInteger,sizeof(int));
					pTable->Columns->Append("lower_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("lower_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("ext_max",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("spatial_data",ADOX::adLongVarBinary,0);
					++it;
					continue;
				
				case TePOLYGONTYPE:
                case TePOLYGONSETTYPE:
					pTable->Columns->Append("num_coords",ADOX::adInteger,sizeof(int));
					pTable->Columns->Append("num_holes",ADOX::adInteger,sizeof(int));
					pTable->Columns->Append("parent_id",ADOX::adInteger,sizeof(int));
					pTable->Columns->Append("lower_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("lower_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("ext_max",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("spatial_data",ADOX::adLongVarBinary,0);
					++it;
					continue;

				case TeCELLTYPE:
				case TeCELLSETTYPE:
					pTable->Columns->Append("lower_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("lower_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("col_number",ADOX::adInteger,sizeof(int));
					pTable->Columns->Append("row_number",ADOX::adInteger,sizeof(int));
					++it;
					continue;

				case TeRASTERTYPE:
					pTable->Columns->Append("lower_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("lower_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_x",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("upper_y",ADOX::adDouble,sizeof(double));
					pTable->Columns->Append("band_id", ADOX::adInteger,0);
					pTable->Columns->Append("resolution_factor", ADOX::adInteger,0);
					pTable->Columns->Append("subband", ADOX::adInteger,0);
					pTable->Columns->Append("spatial_data",ADOX::adLongVarBinary,0);
					pTable->Columns->Append("block_size",ADOX::adInteger,0);
					++it;
					continue;

				case TeTEXTTYPE:
				case TeTEXTSETTYPE:
				default:
					pTable->Columns->Append(attName.c_str(), ADOX::adVarWChar,(*it).rep_.numChar_);
					break;
  			}

			//auto increment
			if ((*it).rep_.isAutoNumber_)
				pTable->Columns->GetItem(attName.c_str())->Properties->GetItem("AutoIncrement")->Value = true;

			//nullable /  a boolean column cannot be nullable
			if((*it).rep_.null_ && (!(*it).rep_.isPrimaryKey_) && ((*it).rep_.type_ != TeBOOLEAN))
				pTable->Columns->GetItem(attName.c_str())->Attributes = ADOX::adColNullable;

			// check if column is part of primary key
			if ((*it).rep_.isPrimaryKey_ && (*it).rep_.type_ != TeBLOB)
				validPk.push_back((*it));
			
			++it;
		}

		if (!validPk.empty())
		{
			TESTHR(hr = pKey.CreateInstance(__uuidof(ADOX::Key)));
			// To pass as column parameter to Key's Apppend method
			_variant_t vOptional;
			vOptional.vt = VT_ERROR;
			vOptional.scode = DISP_E_PARAMNOTFOUND;

			pKey->Name = (table+"_primkey").c_str();
			pKey->Type = ADOX::adKeyPrimary;
			
			it = validPk.begin();
			while (it != validPk.end())
			{
				switch ((*it).rep_.type_)
  				{
  				case TeSTRING:
					if((*it).rep_.numChar_ > 0)
					{
						if((*it).rep_.numChar_<256)
							pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adVarWChar,(*it).rep_.numChar_);
						else
							pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adLongVarWChar,(*it).rep_.numChar_);
					}
					else
					{
						//iSize = 66560; // o número 66560 equivale a 65K
						pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adLongVarWChar, 66560);
					}
  					break;
  				case TeREAL:
					pKey->Columns->Append((*it).rep_.name_.c_str(),ADOX::adDouble,sizeof(double));
  					break;
  				case TeINT:
				case TeUNSIGNEDINT:
					pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adInteger,0);
  					break;
				case TeCHARACTER:
					pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adWChar,0);
					break; 
				case TeDATETIME:
					pKey->Columns->Append((*it).rep_.name_.c_str(), ADOX::adDate,0);
					break;
				}
				++it;
			}
			validPk.clear();
			pTable->Keys->Append(_variant_t((IDispatch *)pKey),ADOX::adKeyPrimary,vOptional,L"",L"");
		}
		
		pCatalog->Tables->Append(_variant_t((IDispatch*)pTable));
		pCatalog->Tables->Refresh();

		it = attr.begin();
		while ( it != attr.end() )
		{
			if (!(*it).rep_.isPrimaryKey_ && (*it).rep_.type_ == TeSTRING)
				pTable->Columns->GetItem((*it).rep_.name_.c_str())->Properties->GetItem("Jet OLEDB:Allow Zero Length")->Value = true;

			if(!((*it).rep_.defaultValue_.empty()))
			{
				pTable->Columns->GetItem((*it).rep_.name_.c_str())->Properties->GetItem("Default")->Value = (*it).rep_.defaultValue_.c_str();
			}
			++it;
		}
		pCatalog->Tables->Refresh();
		pCatalog = NULL;
		pTable = NULL;
		pKey = NULL; 
	}
    catch(_com_error &e)
    {
		errorMessage_ = e.Description();
		pCatalog = NULL; 
		pTable = NULL;
		pKey = NULL; 
		return false;
    }
    catch(...)
    {
		errorMessage_ = "Error creating table "+ table;
		pCatalog = NULL; 
		pTable = NULL;
		pKey = NULL; 
		return false;
    }
	return true;
}

bool 
TeAdo::deleteColumn (const string& table, const string& colName)
{
	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
	
    HRESULT hr = S_OK;
	try
	{
		TESTHR(hr = pCatalog.CreateInstance(__uuidof(ADOX::Catalog)));
		

// Connect the catalog.
		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));

// Get the table
		pTable = pCatalog->Tables->GetItem((_bstr_t)table.c_str ());

		pTable->Columns->Delete(_variant_t(colName.c_str()));

        pCatalog->Tables->Refresh();
		pCatalog = NULL; 
		pTable = NULL;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		pCatalog = NULL; 
		pTable = NULL;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error deleting colum " + colName + "of the table " + table;
		pCatalog = NULL; 
		pTable = NULL;
		return false;
	}

	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + table + "'";
	if(portal->query(sql) && portal->fetchRow())
		tableId = portal->getData(0);

	delete portal;
	if(tableId.empty() == false)
	{
		sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
		sql += " AND related_attr = '" + colName + "')";
		sql += " OR (external_table_name = '" + table + "'";
		sql += " AND external_attr = '" + colName + "')";
		if(execute(sql) == false)
			return false;
	}
	alterTableInfoInMemory(table);
	return true;
}

bool 
TeAdo::addColumn (const string& table, TeAttributeRep &rep)
{
	if(!tableExist(table))
		return false;

	string field = TeGetExtension(rep.name_.c_str());
	if(field.empty())
		field = rep.name_;

	string tab;
	tab = " ALTER TABLE " + table + " ADD ";
	tab += field + "  ";
	
	switch (rep.type_)
	{
		case TeSTRING:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
			
		case TeREAL:
			tab += "FLOAT";	
			break;
			
		case TeINT:
			tab += "INT";
			break;

		case TeDATETIME:
			tab += "DATE";
			break;

		case TeCHARACTER:
			tab += "CHAR";
			break;

		case TeBOOLEAN:
			tab += "YESNO";
			break;
		
		case TeBLOB:
			tab = "BLOB ";
		
		default:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
	}
	if (rep.isAutoNumber_)
		tab += " AUTO_INCREMENT ";
	else
		tab += " NULL ";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error inserting a column to table " + table + " !";
		return false;
	}

	if (rep.type_ == TeSTRING && !rep.isPrimaryKey_)
	{
		HRESULT hr = S_OK;
		ADOX::_CatalogPtr m_pCatalog   = NULL;
		try
		{
			TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 			m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));
			ADOX::_ColumnPtr m_pCol = m_pCatalog->Tables->GetItem(table.c_str())->Columns->GetItem(field.c_str());

				m_pCol->Properties->GetItem("Jet OLEDB:Allow Zero Length")->Value = true;
			m_pCatalog->Tables->Refresh();
			m_pCatalog = NULL;
		}
		catch(_com_error &e)
		{
			errorMessage_ = e.Description();
			m_pCatalog  = NULL;
			return false;
		}
		catch(...)
		{
			errorMessage_ = "Error setting column property ";
			m_pCatalog = NULL;
			return false;
		}
	}
	alterTableInfoInMemory(table);
	return true;
}

bool 
TeAdo::alterTable (const string& table, TeAttributeRep &rep, const string& oldColName)
{
	if(!tableExist(table))
		return false;

	if(!oldColName.empty() && oldColName != rep.name_)
	{
		HRESULT hr = S_OK;
		ADOX::_CatalogPtr m_pCatalog   = NULL;
		ADOX::_TablePtr m_pTable  = NULL;
		ADOX::_ColumnPtr m_pColumn  = NULL;
		try
		{
			TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 			m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));

			m_pTable= m_pCatalog->Tables->GetItem(table.c_str());
			m_pColumn = m_pTable->Columns->GetItem(oldColName.c_str()); 
			m_pColumn->Name = rep.name_.c_str();
			m_pCatalog->Tables->Refresh();								// Refresh the database.
			m_pCatalog = NULL;
			m_pTable  = NULL;
		}
		catch(_com_error &e)
		{
 			errorMessage_ = e.Description();
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}

		catch(...)
		{
			errorMessage_ = "Error alter table";
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}
	}

	string tab = " ALTER TABLE " + table + " ALTER COLUMN ";
	tab += rep.name_ + "  ";
	switch (rep.type_)
	{
		case TeSTRING:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
		
		case TeREAL:
			tab += "FLOAT";	
			break;
		
		case TeINT:
			tab += "INT";
			break;

		case TeDATETIME:
			tab += "DATE";
			break;

		case TeCHARACTER:
			tab += "CHAR";
			break;

		case TeBOOLEAN:
			tab += "YESNO";
			break;

		
		default:
			if (rep.numChar_ > 255 || rep.numChar_ == 0)
				tab += "TEXT ";
			else
				tab += "VARCHAR(" + Te2String(rep.numChar_) + ") ";
			break;
	}

	tab += " NULL ";

	if(!execute(tab))
	{
		if(errorMessage_.empty())
			errorMessage_ = "Error alter table " + table + " !";
		return false;
	}
	
	if (rep.type_ == TeSTRING)  // Allow Zero Length
	{
		HRESULT hr = S_OK;
		ADOX::_CatalogPtr m_pCatalog   = NULL;
		ADOX::_TablePtr m_pTable  = NULL;
		try
		{
			TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 			m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));

			m_pTable= m_pCatalog->Tables->GetItem(table.c_str());
			if (!rep.isPrimaryKey_ && rep.type_ == TeSTRING)
				m_pTable->Columns->GetItem(rep.name_.c_str())->Properties->GetItem("Jet OLEDB:Allow Zero Length")->Value = true;
			m_pCatalog   = NULL;
			m_pTable  = NULL;
		}
		catch(_com_error &e)
		{
			errorMessage_ = e.Description();
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}

		catch(...)
		{
			errorMessage_ = "Error alter table";
			m_pCatalog   = NULL;
			m_pTable  = NULL;
			return false;
		}
	}
		
	string tableId;
	TeDatabasePortal* portal = getPortal();
	string sql = "SELECT table_id FROM te_layer_table WHERE attr_table = '" + table + "'";
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
			sql += " WHERE external_table_name = '" + table + "'";
			sql += " AND external_attr = '" + oldColName + "'";
			if(execute(sql) == false)
				return false;

			 // update grouping
			sql = "UPDATE te_grouping SET grouping_attr = '" + table + "." + rep.name_ + "'";
			sql += " WHERE grouping_attr = '" + table + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
		else // column type changed
		{
			// delete relation
			sql = "DELETE FROM te_tables_relation WHERE (related_table_id = " + tableId;
			sql += " AND related_attr = '" + rep.name_ + "')";
			sql += " OR (external_table_name = '" + table + "'";
			sql += " AND external_attr = '" + rep.name_ + "')";
			if(execute(sql) == false)
				return false;

			// delete grouping
			TeDatabasePortal* portal = getPortal();
			sql = "SELECT theme_id FROM te_grouping WHERE grouping_attr = '" + table + "." + oldColName + "'";
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
			sql += " WHERE grouping_attr = '" + table + "." + oldColName + "'";
			if(execute(sql) == false)
				return false;
		}
	}
	alterTableInfoInMemory(table);
	return true;
}

bool 
TeAdo::alterTable (const string& oldTableName, const string& newTableName)
{
	if(!tableExist(oldTableName))
		return false;

	HRESULT hr = S_OK;
	ADOX::_CatalogPtr m_pCatalog   = NULL;
	ADOX::_TablePtr m_pTable  = NULL;
	try
	{
		TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 		m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));

		m_pTable= m_pCatalog->Tables->GetItem(oldTableName.c_str());
		m_pTable->Name = newTableName.c_str();
		m_pCatalog->Tables->Refresh();								// Refresh the database.
		m_pCatalog = NULL;
		m_pTable  = NULL;
	}
	catch(_com_error &e)
	{
 		errorMessage_ = e.Description();
		m_pCatalog = NULL;
		m_pTable  = NULL;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error alter table";
		m_pCatalog = NULL;
		m_pTable  = NULL;
		return false;
	}
	
	//update te_layer_table
	string sql = " UPDATE te_layer_table ";
	sql += " SET attr_table = '"+ newTableName +"'";
	sql += " WHERE attr_table = '"+ oldTableName +"'";
	execute(sql);

	//update te_tables_relation
	sql = " UPDATE te_tables_relation ";
	sql += " SET external_table_name = '"+ newTableName +"'";
	sql += " WHERE external_table_name = '"+ oldTableName +"'";
	execute(sql);
	
	alterTableInfoInMemory(newTableName, oldTableName);
	return true;
}

bool
TeAdo::createRelation (const string& relationName, 
					   const string& foreignTbl, 
					   const string& FTKey, 
					   const string& relatedTbl, 
					   const string& RTKey, 
					   bool cascadeDeletion)
{
	HRESULT hr = S_OK;
	ADOX::_KeyPtr pKeyForeign = NULL; 
    ADOX::_CatalogPtr pCatalog   = NULL;
    try
    {
        TESTHR(hr = pKeyForeign.CreateInstance(__uuidof(ADOX::Key)));
        TESTHR(hr = pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));

        // Define the foreign key.
        pKeyForeign->Name = relationName.c_str();
        pKeyForeign->Type = ADOX::adKeyForeign;
        pKeyForeign->RelatedTable = relatedTbl.c_str();

		// Append all columns that compose the foreign key
		vector<string> FTCols, RTCols;
		if (TeSplitString(FTKey, ",",FTCols) == TeSplitString(RTKey, ",",RTCols))
		{
			vector<string>::iterator FTit, RTit;
			FTit = FTCols.begin();
			RTit = RTCols.begin();
			while (FTit != FTCols.end())
			{
				pKeyForeign->Columns->Append((*FTit).c_str(),ADOX::adVarWChar,0);
				pKeyForeign->Columns->GetItem((*FTit).c_str())->RelatedColumn = (*RTit).c_str();
				FTit++;
				RTit++;
			}
		}

        if (cascadeDeletion)
			pKeyForeign->DeleteRule = ADOX::adRICascade;

        // To pass as column parameter to Key's Apppend method
        _variant_t vOptional;
        vOptional.vt = VT_ERROR;
        vOptional.scode = DISP_E_PARAMNOTFOUND;

        // Append the foreign key.
        pCatalog->Tables->GetItem(foreignTbl.c_str())->Keys->
            Append(_variant_t((IDispatch *)pKeyForeign),
            ADOX::adKeyPrimary,vOptional,L"",L"");
		pCatalog   = NULL;
	}
    catch(_com_error &e)
    {
		int count = connection_->GetErrors()->GetCount();
		if(count > 0)
		{
			errorMessage_ = (std::string)connection_->GetErrors()->GetItem(0)->GetDescription();
		}
		
		pCatalog   = NULL;
		return false;
	}

    catch(...)
    {
		errorMessage_ = "Error creating foreign key on table " + foreignTbl;
		pCatalog   = NULL;
		return false;
    }
	return true;
}


TeDBRelationType 
TeAdo::existRelation(const string& tableName, const string& relName)
{
    HRESULT hr = S_OK;

    ADOX::_CatalogPtr m_pCatalog   = NULL;
	ADOX::_TablePtr m_pTable  = NULL;
    ADOX::_KeyPtr m_pKey = NULL; 

    try
    {
        TeDBRelationType resultRelType = TeNoRelation;
		TESTHR(hr = m_pKey.CreateInstance(__uuidof(ADOX::Key)));
        TESTHR(hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog)));
 		m_pCatalog->PutActiveConnection(variant_t((IDispatch *)connection_));

        m_pTable= m_pCatalog->Tables->GetItem(tableName.c_str());
		m_pKey = m_pTable->Keys->GetItem(relName.c_str());
		if (m_pKey)
		{
			if (m_pKey->DeleteRule == ADOX::adRICascade)
			   resultRelType = TeRICascadeDeletion;
			else
				resultRelType = TeRINoCascadeDeletion;
		}
		
		m_pCatalog   = NULL;
		m_pTable  = NULL;
		return resultRelType;
	}
    catch(...)
    {
		m_pCatalog   = NULL;
		m_pTable  = NULL;
		return TeNoRelation;
    }
}


TeDatabasePortal*  
TeAdo::getPortal ()
{
	TeAdoPortal* portal = new TeAdoPortal (this);
	return portal;
}

bool
TeAdo::insertProjection(TeProjection* proj)
{
	
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_projection",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
			recset_->AddNew();

		TeProjectionParams par = proj->params();
	
		recset_->Fields->GetItem("long0")->Value = (_variant_t) (par.lon0*TeCRD);
		recset_->Fields->GetItem("lat0")->Value = (_variant_t) (par.lat0*TeCRD);
		recset_->Fields->GetItem("offx")->Value = (_variant_t) (par.offx);
		recset_->Fields->GetItem("offy")->Value = (_variant_t) (par.offy);
		recset_->Fields->GetItem("stlat1")->Value = (_variant_t) (par.stlat1*TeCRD);
		recset_->Fields->GetItem("stlat2")->Value = (_variant_t) (par.stlat2*TeCRD);
		recset_->Fields->GetItem("scale")->Value = (_variant_t) (par.scale);
		recset_->Fields->GetItem("hemis")->Value = (_variant_t) ((long)par.hemisphere);
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (proj->name().c_str());
		recset_->Fields->GetItem("unit")->Value = (_bstr_t) (par.units.c_str());
		recset_->Fields->GetItem("datum")->Value = (_bstr_t) (proj->datum().name().c_str());
		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("projection_id");
		int id = newID.intVal;
		proj->id(id);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	int srsid = proj->epsgCode();
	return insertSRSId(proj, srsid);
}

bool 
TeAdo::insertRelationInfo(const int tableId, const string& tField,
						  const string& eTable,  const string& eField, int& relId)
{
	if(tableId < 0)
	{
		return false;
	}

	// check if relation already exists
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
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
	try
	{
		ADODB::_RecordsetPtr recset_;
		recset_.CreateInstance(__uuidof(ADODB::Recordset));

		recset_->Open("te_tables_relation",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
			recset_->AddNew();
		recset_->Fields->GetItem("related_table_id")->Value =  (_variant_t) ((long)tableId);
		recset_->Fields->GetItem("related_attr")->Value = (_bstr_t) tField.c_str();
		recset_->Fields->GetItem("external_table_name")->Value = (_bstr_t) eTable.c_str();
		recset_->Fields->GetItem("external_attr")->Value = (_bstr_t) eField.c_str();
		recset_->Update();

		_variant_t newID = recset_->GetCollect("relation_id");
		relId =  newID.intVal;
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool 
TeAdo::insertTableInfo(int layerId, TeTable &table, const string& user)
{
	if (table.name().empty())
		return false;

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		// if table is not in the layer tables metadata include it
		if (table.id() <= 0)
		{
			recset_->Open("te_layer_table",
				_variant_t((IDispatch*)connection_,true),
				ADODB::adOpenKeyset,
				ADODB::adLockOptimistic,
				ADODB::adCmdTable);
				recset_->AddNew();

			if (layerId > 0)
				recset_->Fields->GetItem("layer_id")->Value = (_variant_t) ((long)layerId);

			recset_->Fields->GetItem("attr_table")->Value = (_bstr_t) table.name().c_str ();
			
			if (!table.uniqueName().empty())
				recset_->Fields->GetItem("unique_id")->Value = (_bstr_t) table.uniqueName().c_str();

			if (!table.linkName().empty())
				recset_->Fields->GetItem("attr_link")->Value = (_bstr_t) table.linkName().c_str();
						
			if (!table.attInitialTime().empty())
				recset_->Fields->GetItem("attr_initial_time")->Value = (_bstr_t) table.attInitialTime().c_str();
			
			if (!table.attFinalTime().empty())
				recset_->Fields->GetItem("attr_final_time")->Value = (_bstr_t) table.attFinalTime().c_str();

			recset_->Fields->GetItem("attr_time_unit")->Value = (_variant_t) ((long)table.attTimeUnit());
			recset_->Fields->GetItem("attr_table_type")->Value = (_variant_t) ((long)table.tableType());
			if (!user.empty())
				recset_->Fields->GetItem("user_name")->Value = (_bstr_t) user.c_str ();

			recset_->Update();
			_variant_t newID;
			newID = recset_->GetCollect("table_id");
			int id = newID.intVal;
			table.setId(id);
			recset_->Close();
			return true;
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool 
TeAdo::insertView (TeView *view)
{
	// save it´s projection
	TeProjection* proj = view->projection();
	if ( !proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir vista sem projeção!";
		return false;
	}

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_view",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
			recset_->AddNew();

		recset_->Fields->GetItem("name")->Value = (_bstr_t) (view->name().c_str());
		recset_->Fields->GetItem("user_name")->Value = (_bstr_t) (view->user().c_str());
		recset_->Fields->GetItem("projection_id")->Value = (_bstr_t) ((long) proj->id());
		recset_->Fields->GetItem("visibility")->Value = (_variant_t) ((long)view->isVisible());
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (view->getCurrentBox().lowerLeft().x());
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (view->getCurrentBox().lowerLeft().y());
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (view->getCurrentBox().upperRight().x());
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (view->getCurrentBox().upperRight().y());
		if(view->getCurrentTheme() > 0)
			recset_->Fields->GetItem("current_theme")->Value = (_variant_t) ((long)view->getCurrentTheme());
		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("view_id");
		view->id(newID.intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return 0;
	}

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
	// Insert view in the view map
	viewMap()[view->id()] = view;
	return true;
}

bool 
TeAdo::insertTheme (TeAbstractTheme *theme)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_theme",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("view_id")->Value = (_variant_t) ((long)theme->view());
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (theme->name().c_str ());
		recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)theme->parentId());
		recset_->Fields->GetItem("priority")->Value = (_variant_t) ((long)theme->priority());
		recset_->Fields->GetItem("node_type")->Value = (_variant_t) ((long)theme->type());
		recset_->Fields->GetItem("min_scale")->Value = (_variant_t) (TeRoundD(theme->minScale(),15));
		recset_->Fields->GetItem("max_scale")->Value = (_variant_t) (TeRoundD(theme->maxScale(),15));
		
		if(!theme->attributeRest().empty())
			recset_->Fields->GetItem("generate_attribute_where")->Value = (_bstr_t) (theme->attributeRest().c_str());
		if(!theme->spatialRest().empty())
			recset_->Fields->GetItem("generate_spatial_where")->Value = (_bstr_t) (theme->spatialRest().c_str());
		if(!theme->temporalRest().empty())
			recset_->Fields->GetItem("generate_temporal_where")->Value = (_bstr_t) (theme->temporalRest().c_str());
				
		recset_->Fields->GetItem("visible_rep")->Value = (_variant_t) ((long)theme->visibleRep());
		recset_->Fields->GetItem("enable_visibility")->Value = (_variant_t) ((long)theme->visibility());

		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(theme->box().x1(),15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(theme->box().y1(),15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(theme->box().x2(),15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(theme->box().y2(),15));
		recset_->Fields->GetItem("creation_time")->Value = (_bstr_t) (theme->getCreationTime().getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str());

		if(theme->type()==TeTHEME)
		{
			recset_->Fields->GetItem("layer_id")->Value = (_variant_t) ((long)static_cast<TeTheme*>(theme)->layerId());
			if(!static_cast<TeTheme*>(theme)->collectionTable().empty())
				recset_->Fields->GetItem("collection_table")->Value = (_bstr_t) (static_cast<TeTheme*>(theme)->collectionTable().c_str());
		}
		
		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("theme_id");
		theme->id(newID.intVal);

		if((theme->type()==TeTHEME || theme->type()==TeEXTERNALTHEME) && static_cast<TeTheme*>(theme)->collectionTable().empty())
		{
			string name = "te_collection_" + Te2String(theme->id());
			static_cast<TeTheme*>(theme)->collectionTable(name);
			recset_->Fields->GetItem("collection_table")->Value = (_bstr_t) (name.c_str());
		}
		if(theme->parentId() == 0)
		{
			theme->parentId(theme->id());
			recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)theme->parentId());			
		}

		recset_->Update();
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}

	// insert grouping
	int numSlices = 0;
	if(theme->grouping().groupMode_ != TeNoGrouping)
	{
		if(!insertGrouping (theme->id(), theme->grouping()))
			return false;
		numSlices = theme->grouping().groupNumSlices_;
	}
		
	// insert legend
	theme->outOfCollectionLegend().group(-1); 
	theme->outOfCollectionLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->outOfCollectionLegend()))) 
		return false;

	theme->withoutDataConnectionLegend().group(-2); 
	theme->withoutDataConnectionLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->withoutDataConnectionLegend()))) 
		return false;

	theme->defaultLegend().group(-3); 
	theme->defaultLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->defaultLegend()))) 
		return false;

	theme->pointingLegend().group(-4); 
	theme->pointingLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->pointingLegend()))) 
		return false;

	theme->queryLegend().group(-5); 
	theme->queryLegend().theme(theme->id()); 
	if (!insertLegend(&(theme->queryLegend()))) 
		return false;

	theme->queryAndPointingLegend().group(-6); 
	theme->queryAndPointingLegend().theme(theme->id()); 
	if (!insertLegend (&(theme->queryAndPointingLegend())))
		return false;

	for (int i = 0; i < numSlices; ++i)
	{
		theme->legend()[i].group(i);
		theme->legend()[i].theme(theme->id());
		if (!insertLegend (&(theme->legend()[i])))
			return false;
	}

    //insert metadata theme
	if(!theme->saveMetadata(this))
		return false;
	
	themeMap()[theme->id()] = theme;
	
	if(theme->type()==TeTHEME && !updateThemeTable(static_cast<TeTheme*>(theme)))
		return false; 

	//save in the database particular attributes of each theme type
	return true;
}

bool 
TeAdo::insertThemeGroup (TeViewTree* tree)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_theme",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (tree->name().c_str());
		recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)tree->parentId());
		recset_->Fields->GetItem("node_type")->Value = (_variant_t) ((long)1);
		recset_->Fields->GetItem("view_id")->Value = (_variant_t) ((long)tree->view());
		recset_->Fields->GetItem("priority")->Value = (_variant_t) ((long)tree->priority());

		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("theme_id");
		tree->id(newID.intVal);

		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}

	return true;
}

bool 
TeAdo::insertThemeTable	(int themeId, int tableId, int relationId, int tableOrder)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_theme_table",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("theme_id")->Value = (_variant_t) ((long)themeId);
		recset_->Fields->GetItem("table_id")->Value = (_variant_t) ((long)tableId);
		
		if(relationId > 0)
			recset_->Fields->GetItem("relation_id")->Value = (_variant_t) ((long)relationId);
		
		recset_->Fields->GetItem("table_order")->Value = (_variant_t) ((long)tableOrder);

		recset_->Update();
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}


bool 
TeAdo::insertViewTree (TeViewTree *tree)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_theme",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();

		recset_->Fields->GetItem("name")->Value = (_bstr_t) (tree->name().c_str ());
		recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)tree->parentId());
		recset_->Fields->GetItem("node_type")->Value = (_variant_t) ((long)tree->type());
		recset_->Fields->GetItem("view_id")->Value = (_variant_t) ((long)tree->view());
		recset_->Fields->GetItem("priority")->Value = (_variant_t) ((long)tree->priority());

		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("theme_id");
		tree->id(newID.intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool TeAdo::insertProject(TeProject *project)
{
	if (!project)
		return false;
	_variant_t newID;
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_project",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		recset_->AddNew();
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (project->name().c_str());
		recset_->Fields->GetItem("description")->Value = (_bstr_t) (project->description().c_str());
		recset_->Fields->GetItem("current_view")->Value = (_variant_t) ((long)project->getCurrentViewId());
		recset_->Update();
		newID = recset_->GetCollect("project_id");
		project->setId(newID.intVal);
		recset_->Close();		
		projectMap()[project->id()] = project;
		for (unsigned int i=0; i<project->getViewVector().size(); i++)
			insertProjectViewRel(project->id(), project->getViewVector()[i]);
		return true;

	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
}
bool 
TeAdo::updateBBox(const string& tableName, const string& idName, int idValue, const TeBox& box)
{
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;
	try
	{
		string q ="SELECT * FROM "+tableName;
		q += " WHERE "+idName+" = " + Te2String(idValue);
		if (!portal->query (q) || !portal->fetchRow())
		{
			delete portal;
			return false;
		}
		
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(box.x1(),15));
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(box.y1(),15));
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(box.x2(),15));
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(box.y2(),15));
		portal->recset_->Update();
		portal->recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return false;
	}
	delete portal;
	return true;
}

bool 
TeAdo::updateLayerBox(TeLayer* layer)
{
	if (!layer)
		return false;
	return TeAdo::updateBBox("te_layer", "layer_id", layer->id(), layer->box());
}

bool 
TeAdo::updateLayer(TeLayer* layer)
{
	if (!layer)
		return false;
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;
	try
	{
		string q ="SELECT * FROM te_layer";
		q += " WHERE layer_id = " + Te2String(layer->id());

		if (!portal->query(q) || !portal->fetchRow())
		{
			delete portal;
			return false;
		}
		if (layer->projection())
			portal->recset_->Fields->GetItem("projection_id")->Value = (_variant_t) ((long)layer->projection()->id());
		portal->recset_->Fields->GetItem("name")->Value = (_bstr_t) (layer->name().c_str());
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(layer->box().x1(),15));
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(layer->box().y1(),15));
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(layer->box().x2(),15));
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(layer->box().y2(),15));

		if(layer->getEditionTime().isValid())
		{
			portal->recset_->Fields->GetItem("edition_time")->Value = (_bstr_t) (layer->getEditionTime().getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str());
		}

		portal->recset_->Update();
		portal->recset_->Close();
		if (layer->projection())
			updateProjection(layer->projection());
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return false;
	}
	delete portal;
	return true;
}

bool TeAdo::insertLayer (TeLayer* layer)
{
	TeProjection* proj = layer->projection();
	if (!proj || !insertProjection(proj))
	{
		errorMessage_ = "Não é possível inserir layer sem projeção";
		return false;
	}

	_variant_t newID;
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_layer",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		recset_->AddNew();
		recset_->Fields->GetItem("projection_id")->Value = (_variant_t) ((long)proj->id());
		recset_->Fields->GetItem("name")->Value = (_bstr_t) (layer->name().c_str());
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(layer->box().x1(),15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(layer->box().y1(),15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(layer->box().x2(),15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(layer->box().y2(),15));
		recset_->Fields->GetItem("edition_time")->Value = (_bstr_t) (layer->getEditionTime().getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str());

		recset_->Update();
		newID = recset_->GetCollect("layer_id");
		layer->id(newID.intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	layerMap()[layer->id()] = layer;
	return true;
}

bool 
TeAdo::insertRepresentation (int layerId, TeRepresentation& rep)
{
	if (layerId <= 0)
		return false;

	_variant_t newID;
	int id;
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_representation",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();

		recset_->Fields->GetItem("layer_id")->Value = (_variant_t)((long)layerId);
		recset_->Fields->GetItem("geom_type")->Value = (_variant_t) static_cast<long>(rep.geomRep_);
		recset_->Fields->GetItem("geom_table")->Value = (_bstr_t) rep.tableName_.c_str();
		recset_->Fields->GetItem("description")->Value = (_bstr_t) rep.description_.c_str();
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(rep.box_.x1(),15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(rep.box_.y1(),15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(rep.box_.x2(),15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(rep.box_.y2(),15));
		recset_->Fields->GetItem("res_x")->Value = (_variant_t) (rep.resX_);
		recset_->Fields->GetItem("res_y")->Value = (_variant_t) (rep.resY_);
		recset_->Fields->GetItem("num_cols")->Value = (_variant_t) ((long)rep.nCols_);
		recset_->Fields->GetItem("num_rows")->Value = (_variant_t) ((long)rep.nLins_);
		recset_->Update();
		newID = recset_->GetCollect("repres_id");
		id = newID.intVal;
		rep.id_ = id;
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool 
TeAdo::insertLegend (TeLegendEntry *legend)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open("te_legend",
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("theme_id")->Value = (_variant_t) ((long)legend->theme());
		recset_->Fields->GetItem("group_id")->Value = (_variant_t) ((long)legend->group());

// Group parameters
		recset_->Fields->GetItem("num_objs")->Value = (_variant_t) ((long)legend->count());
		recset_->Fields->GetItem("lower_value")->Value = (_bstr_t) (legend->from().c_str());
		recset_->Fields->GetItem("upper_value")->Value = (_bstr_t) (legend->to().c_str());
		recset_->Fields->GetItem("label")->Value = (_bstr_t) (legend->label().c_str());

		recset_->Update();
		_variant_t newID;
		newID = recset_->GetCollect("legend_id");
		legend->id(newID.intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return 0;
	}

	legendMap()[legend->id()] = legend;
	return insertVisual(legend);
}

bool 
TeAdo::insertPolygonSet (const string& table, TePolygonSet &ps)
{
	if (ps.empty())
		return false;

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

	unsigned int i;
	unsigned int n = ps.size();
	try
	{
		// for each polygon in the polygon set
		for (i = 0; i < n; ++i)
		{
			unsigned int k;
			int parentId = 0;
			unsigned int numberHoles = ps[i].size()-1;
			// for each ring 
			for (k=0; k<ps[i].size(); ++k)
			{
				recset_->AddNew();
				recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ps[i].objectId().c_str());
				recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)ps[i][k].size());
				recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)numberHoles);
				recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
				recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(ps[i][k].box().lowerLeft().x_,15));
				recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(ps[i][k].box().lowerLeft().y_,15));
				recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(ps[i][k].box().upperRight().x_,15));
				recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(ps[i][k].box().upperRight().y_,15));
				recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(ps[i][k].box().width(),ps[i][k].box().height()));

				// create and save the BLOB
				VARIANT varPoints;
				if(!LinearRingToVariant(varPoints, ps[i][k]))
				{
					recset_->Close();
					return false;
				}

				recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
				recset_->Update();
				SafeArrayDestroy (varPoints.parray);

				// retrieve the geometry id of the ring (given by the auto increment property)
				ps[i][k].geomId(recset_->GetCollect("geom_id").intVal);
 
				if (k==0)
				{
					parentId = ps[i][k].geomId(); // save the geometry id of the parent
					ps[i].geomId(parentId);
					// update in the database the parent id of an outter ring 
					recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
					recset_->Update();
					numberHoles=0;

					ps[i].geomId(parentId);
				}
			}
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return false;
	}
	recset_->Close();
	return true;
}


bool 
TeAdo::insertPolygon (const string& table, TePolygon& pol)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));

	recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

	try
	{
		unsigned int k;
		int parentId = 0;
		unsigned int numberHoles = pol.size()-1;
		// for each ring
		for (k=0; k<pol.size(); ++k)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (pol.objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)pol[k].size());
			recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)numberHoles);
			recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(pol[k].box().lowerLeft().x_,15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(pol[k].box().lowerLeft().y_,15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(pol[k].box().upperRight().x_,15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(pol[k].box().upperRight().y_,15));
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(pol[k].box().width(),pol[k].box().height()));

			// create and save the BLOB
			VARIANT varPoints;
			if(!LinearRingToVariant(varPoints,pol[k]))
			{
				recset_->Close();
				return false;
			}

			recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			recset_->Update();
			SafeArrayDestroy (varPoints.parray);

			// retrieve the geometry id of the ring (given by the auto increment property)
			pol[k].geomId(recset_->GetCollect("geom_id").intVal);

			if (k==0)
			{
				parentId = pol[k].geomId();// save the geometry id of the parent
				// update in the database the parent id of an outter ring 
				recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)parentId);
				recset_->Update();

				pol.geomId(parentId);
				numberHoles=0;
			}
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return false;
	}
	recset_->Close();
	return true;
}


bool 
TeAdo::updatePolygon (const string& table, TePolygon &p )
{
	double	extmax;
	unsigned int k, ni, size;
//	double	*points	= NULL;

	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	try
	{
		TeBox b = p[0].box();
		for ( k = 0; k < p.size(); k++ )
		{
			TeLinearRing ring ( p[k] );
			string q ="SELECT * FROM " + table;

			q += " WHERE geom_id = " + Te2String(ring.geomId());

			if (!portal->query (q))
			{
				delete portal;
				return false;
			}

			if (!portal->fetchRow())
			{
				portal->freeResult();
				delete portal;
				return false;
			}
			size = ring.size();
			ni = 0;
			if (k==0)
			{
				extmax = MAX(b.width(),b.height());
				ni = p.size()-1;
			}
			else
				ni = 0;

			portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
			portal->recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
			portal->recset_->Fields->GetItem("num_holes")->Value = (_variant_t) ((long)ni);
			portal->recset_->Fields->GetItem("parent_id")->Value = (_variant_t) ((long)p[0].geomId());
			portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(b.lowerLeft().x(),15));
			portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(b.lowerLeft().y(),15));
			portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(b.upperRight().x(),15));
			portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(b.upperRight().y(),15));
			portal->recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);
// Create and save the BLOB
			VARIANT varPoints;

			if(!LinearRingToVariant(varPoints, ring))
			{
				delete portal;
				return false;
			}

			portal->recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			portal->recset_->Update();
			SafeArrayDestroy (varPoints.parray);
			portal->recset_->Close();
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}


bool 
TeAdo::insertLineSet (const string& table, TeLineSet &ls)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenKeyset,
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);
	try
	{
		// for each line
		for (unsigned int i = 0; i < ls.size(); ++i)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ls[i].objectId().c_str());
			recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)ls[i].size());
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(ls[i].box().lowerLeft().x_,15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(ls[i].box().lowerLeft().y_,15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(ls[i].box().upperRight().x_,15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(ls[i].box().upperRight().y_,15));
			recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(ls[i].box().width(),ls[i].box().height()));

			// create and save the BLOB
			VARIANT varPoints;
			if (!LineToVariant(varPoints,ls[i]))
			{
				recset_->Close();
				return false;
			}

			recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
			recset_->Update();
			SafeArrayDestroy (varPoints.parray);
			
			// retrieve the geometry id of the line (given by the auto increment property)		
			ls[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
	}
	catch(_com_error &e)
	{
		recset_->Close();
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		recset_->Close();
		errorMessage_ = "Oppps !";
		return 0;
	}
	recset_->Close();
	return true;
}

bool 
TeAdo::insertLine (const string& table, TeLine2D &line)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));

	try
	{	
		recset_->Open(_bstr_t(table.c_str()),
		_variant_t((IDispatch*)connection_,true),
		ADODB::adOpenKeyset,
		ADODB::adLockOptimistic,
		ADODB::adCmdTable);

		recset_->AddNew();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
		recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)line.size());
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(line.box().lowerLeft().x_,15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(line.box().lowerLeft().y_,15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(line.box().upperRight().x_,15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(line.box().upperRight().y_,15));
		recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (MAX(line.box().width(),line.box().height()));

		// create and save the BLOB
		VARIANT varPoints;
		if (!LineToVariant(varPoints, line))
		{
			recset_->Close();
			return false;
		}

		recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
		recset_->Update();
		SafeArrayDestroy (varPoints.parray);
		
		// retrieve the geometry id of the line (given by the auto increment property)		
		line.geomId (recset_->GetCollect("geom_id").intVal);
	}
	catch(_com_error &e)
	{
		recset_->Close();
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		recset_->Close();
		errorMessage_ = "Oppps !";
		return 0;
	}
	recset_->Close();
	return true;
}

bool 
TeAdo::updateLine (const string& table, TeLine2D &line)
{
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	string q ="SELECT * FROM " + table;
	q += " WHERE geom_id = " + Te2String(line.geomId());

	if (!portal->query (q))
	{
		delete portal;
		return false;
	}

	if (!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return false;
	}

	double	extmax;
	int		size;
//	double	*points	= NULL;
	size = line.size();
	TeBox b = line.box();
	extmax = MAX(b.width(),b.height());

	try
	{
		portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (line.objectId().c_str());
		portal->recset_->Fields->GetItem("num_coords")->Value = (_variant_t) ((long)size);
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(b.lowerLeft().x(),15));
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(b.lowerLeft().y(),15));
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(b.upperRight().x(),15));
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(b.upperRight().y(),15));
		portal->recset_->Fields->GetItem("ext_max")->Value = (_variant_t) (extmax);

// Create and save the BLOB
		VARIANT varPoints;

		if (!LineToVariant(varPoints, line))
		{
			delete portal;
			return false;
		}

		portal->recset_->Fields->Item["spatial_data"]->AppendChunk (varPoints);
		portal->recset_->Update();
		portal->recset_->Close();

		SafeArrayDestroy (varPoints.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}


bool 
TeAdo::insertPointSet (const string& table, TePointSet &ps)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		for (unsigned int i = 0; i < ps.size(); ++i)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ps[i].objectId().c_str());
			recset_->Fields->GetItem("x")->Value = (_variant_t) TeRoundD(ps[i].location().x_,15);
			recset_->Fields->GetItem("y")->Value = (_variant_t) TeRoundD(ps[i].location().y_,15);
			recset_->Update();
			ps[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
		recset_->Close();
	}
	catch(_com_error &e)
	{
		recset_->Close();
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		recset_->Close();
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool 
TeAdo::insertPoint (const string& table, TePoint &p)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
		recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(p.location().x_,15));
		recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(p.location().y_,15));
		recset_->Update();
		p.geomId (recset_->GetCollect("geom_id").intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return 0;
	}
	return true;
}

bool 
TeAdo::updatePoint (const string& table, TePoint &p)
{
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	try
	{
		string q ="SELECT * FROM " + table;
		q += " WHERE geom_id = " + Te2String(p.geomId());

		if (!portal->query (q))
		{
			delete portal;
			return false;
		}

//		bool update = false;
		if (!portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			return false;
		}

		portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (p.objectId().c_str());
		portal->recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(p.location().x(),15));
		portal->recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(p.location().y(),15));
		portal->recset_->Update();
		portal->recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}


bool 
TeAdo::insertCellSet (const string& table, TeCellSet &cs)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
				_variant_t((IDispatch*)connection_,true),
				ADODB::adOpenKeyset,
				ADODB::adLockOptimistic,
				ADODB::adCmdTable);
		for (unsigned int i = 0; i < cs.size(); ++i)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (cs[i].objectId().c_str());
			recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(cs[i].box().lowerLeft().x_,15));
			recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(cs[i].box().lowerLeft().y_,15));
			recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(cs[i].box().upperRight().x_,15));
			recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(cs[i].box().upperRight().y_,15));
			recset_->Fields->GetItem("col_number")->Value = (_variant_t) ((long)cs[i].column());
			recset_->Fields->GetItem("row_number")->Value = (_variant_t) ((long)cs[i].line());
			recset_->Update();
			cs[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	recset_->Close();
	return true;
}

bool 
TeAdo::insertCell (const string& table, TeCell &c)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));

	recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
	try
	{
		recset_->AddNew();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (c.objectId().c_str());
		recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(c.box().lowerLeft().x_,15));
		recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(c.box().lowerLeft().y_,15));
		recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(c.box().upperRight().x_,15));
		recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(c.box().upperRight().y_,15));
		recset_->Fields->GetItem("col_number")->Value = (_variant_t) ((long)c.column());
		recset_->Fields->GetItem("row_number")->Value = (_variant_t) ((long)c.line());
		recset_->Update();
		c.geomId (recset_->GetCollect("geom_id").intVal);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		recset_->Close();
		errorMessage_ = "Oppps !";
		return 0;
	}
	recset_->Close();
	return true;
}

bool 
TeAdo::insertTextSet (const string& table, TeTextSet &ts)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		for (unsigned int i = 0; i < ts.size(); ++i)
		{
			recset_->AddNew();
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (ts[i].objectId().c_str());
			recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(ts[i].location().x_,15));
			recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(ts[i].location().y_,15));
			recset_->Fields->GetItem("text_value")->Value = (_bstr_t) (ts[i].textValue().c_str());
			recset_->Fields->GetItem("angle")->Value = (_variant_t) (ts[i].angle());
			recset_->Fields->GetItem("height")->Value = (_variant_t) (ts[i].height());
			recset_->Fields->GetItem("alignment_vert")->Value = (_variant_t) (ts[i].alignmentVert());
			recset_->Fields->GetItem("alignment_horiz")->Value = (_variant_t) (ts[i].alignmentHoriz());
			recset_->Update();
			ts[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::insertText(const string& table, TeText &t)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (t.objectId().c_str());
		recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(t.location().x(),15));
		recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(t.location().y(),15));
		recset_->Fields->GetItem("text_value")->Value = (_bstr_t) (t.textValue().c_str());
		recset_->Fields->GetItem("angle")->Value = (_variant_t) (t.angle());
		recset_->Fields->GetItem("height")->Value = (_variant_t) (t.height());
		recset_->Fields->GetItem("alignment_vert")->Value = (_variant_t)(t.alignmentVert());
		recset_->Fields->GetItem("alignment_horiz")->Value = (_variant_t)(t.alignmentHoriz());
		recset_->Update();
		t.geomId (recset_->GetCollect("geom_id").intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::insertArcSet (const string& table, TeArcSet &as)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		for ( unsigned int i = 0; i < as.size(); i++ )
		{
			TeArc arc = as [i];
			recset_->AddNew();
			recset_->Fields->GetItem("geom_id")->Value = (_variant_t) ((long)arc.geomId());
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (arc.objectId().c_str());
			recset_->Fields->GetItem("from_node")->Value = (_variant_t) ((long)arc.fromNode().geomId());
			recset_->Fields->GetItem("to_node")->Value = (_variant_t) ((long)arc.toNode().geomId());
			recset_->Update();
			as[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::insertArc (const string& table, TeArc &arc)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
		recset_->AddNew();
		recset_->Fields->GetItem("geom_id")->Value = (_variant_t) ((long)arc.geomId());
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (arc.objectId().c_str());
		recset_->Fields->GetItem("from_node")->Value = (_variant_t) ((long)arc.fromNode().geomId());
		recset_->Fields->GetItem("to_node")->Value = (_variant_t) ((long)arc.toNode().geomId());
		recset_->Update();
		recset_->Close();
		arc.geomId (recset_->GetCollect("geom_id").intVal);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::insertNodeSet (const string& table, TeNodeSet &ns)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		for ( unsigned int i = 0; i < ns.size(); i++ )
		{
			TeNode node = ns [i];
			recset_->AddNew();
			recset_->Fields->GetItem("geom_id")->Value = (_variant_t) ((long)node.geomId());
			recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (node.objectId().c_str());
			recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(node.location().x(),15));
			recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(node.location().y(),15));
			recset_->Update();
			ns[i].geomId (recset_->GetCollect("geom_id").intVal);
		}
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::insertNode (const string& table, TeNode &n)
{
	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(table.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);

		recset_->AddNew();
		recset_->Fields->GetItem("geom_id")->Value = (_variant_t) ((long)n.geomId());
		recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (n.objectId().c_str());
		recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(n.location().x(),15));
		recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(n.location().y(),15));
		recset_->Update();
		n.geomId (recset_->GetCollect("geom_id").intVal);
		recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		recset_->Close();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		recset_->Close();
		return 0;
	}
	return true;
}

bool 
TeAdo::updateNode (const string& table, TeNode &node)
{
	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
	if (!portal)
		return false;

	try
	{
		string q ="SELECT * FROM " + table;
		q += " WHERE geom_id = " + Te2String(node.geomId());

		if (!portal->query (q))
		{
			delete portal;
			return false;
		}

//		bool update = false;
		if (!portal->fetchRow())
		{
			delete portal;
			return false;
		}

		portal->recset_->Fields->GetItem("object_id")->Value = (_bstr_t) (node.objectId().c_str());
		portal->recset_->Fields->GetItem("x")->Value = (_variant_t) (TeRoundD(node.location().x(),15));
		portal->recset_->Fields->GetItem("y")->Value = (_variant_t) (TeRoundD(node.location().y(),15));
		portal->recset_->Update();
		portal->recset_->Close();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return 0;
	}
	delete portal;
	return true;
}


bool 
TeAdo::insertTable (TeTable &table)
{
	string				tableName = table.name();	
	TeAttributeList		att = table.attributeList();
	TeAttributeList::iterator	it;

	TeTableRow row;
	unsigned int i;
	
	_variant_t index;
	index.vt = VT_I4;

	ADODB::_RecordsetPtr recset_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	try
	{
		recset_->Open(_bstr_t(tableName.c_str()),
			_variant_t((IDispatch*)connection_,true),
			ADODB::adOpenKeyset,
			ADODB::adLockOptimistic,
			ADODB::adCmdTable);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error insert table!";
		return false;
	}

	bool rec_open = false;
	for ( i = 0; i < table.size(); i++  )
	{
		try 
		{
			if (!rec_open)
				recset_->AddNew();
			row = table[i];
			it = att.begin();

			for(unsigned int j=0; j<row.size(); j++) 
			{
				if (row[j].empty() || (*it).rep_.isAutoNumber_)
				{
					++it;
					continue;
				}

				TeTime dt;
				index.lVal = j;
				string value = row[j];
				if((*it).rep_.type_==TeDATETIME)
				{
					TeTime t(string(row[j].c_str()), (*it).dateChronon_, (*it).dateTimeFormat_, (*it).dateSeparator_,  (*it).timeSeparator_, (*it).indicatorPM_);
					dt=t;
				}
				switch ((*it).rep_.type_)
  				{
  					case TeREAL:
						recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) atof(row[j].c_str());
  					break;
  					case TeINT:
						recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) atol(row[j].c_str());
  					break;
					case TeDATETIME:
						recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_bstr_t) (dt.getDateTime(systemDateTimeFormat_, systemDateSep_, systemTimeSep_, systemIndPM_, systemIndAM_).c_str()); 
  					break;
					
					case TeBOOLEAN:
						{
							int value = 0;
							if(row[j] == "1" || TeConvertToLowerCase(row[j]) == "t" || TeConvertToLowerCase(row[j]) == "true")
							{
								value = 1;
							}
							recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_variant_t) value;
						}
					break;

					case TeBLOB:
						BYTE *pByte;
						VARIANT var;

						SAFEARRAY FAR* psa;
						SAFEARRAYBOUND rgsabound[1];
						rgsabound[0].lLbound = 0;	
						rgsabound[0].cElements =  value.size();

						// create a single dimensional byte array
						psa = SafeArrayCreate(VT_I1, 1, rgsabound);

						// set the data of the array with data in the edit box
						if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
							memcpy((LPVOID)pByte,(LPVOID)(char*)value.c_str(), value.size() * sizeof(char));
						SafeArrayUnaccessData(psa);
						var.vt = VT_ARRAY | VT_UI1;
						var.parray = psa;

						recset_->Fields->Item[(*it).rep_.name_.c_str()]->AppendChunk (var);
						SafeArrayDestroy (var.parray);						
						break;
  					default:
						if (value.size() > (unsigned int)((*it).rep_.numChar_))
							value = value.substr(0,(*it).rep_.numChar_);
						recset_->GetFields()->GetItem((*it).rep_.name_.c_str())->Value = (_bstr_t) (value.c_str());
  					break;
  				}
				++it;
			}
			recset_->Update();
			rec_open =  false;
		}
		catch(_com_error &e)
		{
			errorMessage_ = e.Description();
			rec_open = true;
		}
		catch(...)
		{
			errorMessage_ = "Error insert table!";
			rec_open = true;
		}
	}
	return true;
}

bool 
TeAdo::insertBlob (const string& tableName, const string& columnBlob, const string& whereClause, unsigned char* data, int size)
{
	if (whereClause.empty())
		return false;

	TeAdoPortal* portal = (TeAdoPortal*)getPortal();
	if (!portal)
		return false;

	string q = "SELECT * FROM "+ tableName +" WHERE " + whereClause;
	if ((!portal->query(q)) || (!portal->fetchRow()))
	{
		delete portal;
		return false;
	}

	try
	{
		BYTE *pByte;
		VARIANT var;
		
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;	
		rgsabound[0].cElements = size;

		// create a single dimensional byte array
		psa = SafeArrayCreate(VT_I1, 1, rgsabound);
			
		// set the data of the array with data in the edit box
		if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
			memcpy((LPVOID)pByte,(LPVOID)data, size * sizeof(unsigned char));
		SafeArrayUnaccessData(psa);
		var.vt = VT_ARRAY | VT_UI1;
		var.parray = psa;

		portal->recset_->Fields->Item[columnBlob.c_str()]->AppendChunk (var);
		portal->recset_->Update();
		portal->recset_->Close();
		SafeArrayDestroy (var.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error insert blob!";
		delete portal;
		return false;
	}

	delete portal;
	return true;
}

bool 
TeAdo::insertRasterBlock(const string& table, const string& blockId, const TeCoord2D& ll, const TeCoord2D& ur, 
					     unsigned char *buf,unsigned long size, int band, unsigned int res, unsigned int subband)
{

	if (blockId.empty()) // no block identifier provided
	{
		errorMessage_ = "bloco sem identificador";
		return false;
	}

	TeAdoPortal* portal = (TeAdoPortal*) this->getPortal();
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

	try
	{
		if (!update)
		{
			portal->freeResult();
			portal->recset_->Open(_bstr_t(table.c_str()),
				_variant_t((IDispatch*)connection_,true),
				ADODB::adOpenKeyset,
				ADODB::adLockOptimistic,
				ADODB::adCmdTable);
			portal->recset_->AddNew();
		}
		portal->recset_->Fields->GetItem("block_id")->Value = (_bstr_t) blockId.c_str();
		portal->recset_->Fields->GetItem("lower_x")->Value = (_variant_t) (TeRoundD(ll.x(),15));
		portal->recset_->Fields->GetItem("lower_y")->Value = (_variant_t) (TeRoundD(ll.y(),15));
		portal->recset_->Fields->GetItem("upper_x")->Value = (_variant_t) (TeRoundD(ur.x(),15));
		portal->recset_->Fields->GetItem("upper_y")->Value = (_variant_t) (TeRoundD(ur.y(),15));
		portal->recset_->Fields->GetItem("band_id")->Value = (long) band;
		portal->recset_->Fields->GetItem("resolution_factor")->Value = (long) res;
		portal->recset_->Fields->GetItem("subband")->Value = (long) subband;
		portal->recset_->Fields->GetItem("block_size")->Value = (long) size;

		VARIANT var;
		BYTE *pByte;

		// create a single dimensional byte array
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;	
		rgsabound[0].cElements = size;

		psa = SafeArrayCreate(VT_I1, 1, rgsabound);
			
		// set the data of the array with compressed data
		if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
			memcpy((LPVOID)pByte,(LPVOID)buf,size);
		SafeArrayUnaccessData(psa);

		var.vt = VT_ARRAY | VT_UI1;
		var.parray = psa;

		portal->recset_->Fields->Item["spatial_data"]->AppendChunk (var);
		portal->recset_->Update();
		portal->recset_->Close();
		SafeArrayDestroy (var.parray);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		delete portal;
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		delete portal;
		return false;
	}
	delete portal;
	return true;
}


string 
TeAdo::getSQLTime(const TeTime& time) const
{
	//Insert datetime in a database ACCESS always in the format MMsDDsYYYY HHsMMsSS
	string result = "#";		
	result += time.getDateTime("MMsDDsYYYYsHHsmmsSS"); 
	result += "#";
	return result;
}

string
TeAdo::getSQLTemporalWhere(int time1, int time2, TeChronon chr, TeTemporalRelation rel, 
						   const string& initialTime, const string& finalTime)
{
	
	//In Access the first day of the week is sunday with index 1
	if(chr==TeDAYOFWEEK)
	{
		time1 = time1+1;
		time2 = time2+1;
	}

	return(TeDatabase::getSQLTemporalWhere(time1, time2, chr, rel, initialTime, finalTime));
}

string TeAdo::concatValues(vector<string>& values, const string& unionString)
{
	string concat = "";
	
	for(unsigned int i = 0; i < values.size(); ++i)
	{
		if(i != 0)
		{
			concat += " + ";

			if(!unionString.empty())
			{
				concat += "'";
				concat += unionString;
				concat += "'";
				concat += " + ";
			}
		}

		concat += values[i];
	}

	return concat;
}

string TeAdo::toUpper(const string& value)
{
	string result  = "UCASE(";
	       result += value;
		   result += ")";

	return result;
}

bool 
TeAdo::getAttributeList(const string& tableName,TeAttributeList& attList)
{
	if (!TeDatabase::getAttributeList (tableName, attList))
		return false;

	vector<string> pks;
	ADOX::_CatalogPtr pCatalog = NULL;
	ADOX::_TablePtr pTable = NULL;
    HRESULT hr = S_OK;
	try
	{
		TESTHR(hr = pCatalog.CreateInstance(__uuidof(ADOX::Catalog)));
		pCatalog->PutActiveConnection(_variant_t((IDispatch *)connection_));
		pTable = pCatalog->Tables->GetItem((_bstr_t)tableName.c_str ());
        int n = pTable->Keys->Count;
		for (int i=0; i<n; i++)
		{
			if (pTable->Keys->GetItem((long)i)->Type == ADOX::adKeyPrimary)
			{
				_bstr_t	bbvalue_;
				_variant_t value;
				int m = pTable->Keys->GetItem((long)i)->Columns->Count;
				for (int j = 0; j < m; j++)
				{
					value = pTable->Keys->GetItem((long)i)->Columns->GetItem((long)j)->Name;
					bbvalue_ = _bstr_t(value);
					pks.push_back(string((char*)bbvalue_));
				}
			}
		}
	}
	catch(...) 
	{
		pCatalog = NULL;
		pTable = NULL;
		return false;
	}
	vector<string>::iterator itn;
	TeAttributeList::iterator it = attList.begin();
	while (it != attList.end())
	{
		itn = find(pks.begin(),pks.end(),(*it).rep_.name_);
		if (itn != pks.end())
			(*it).rep_.isPrimaryKey_ = true;
		++it;
	}
	pks.clear();
	pCatalog = NULL;
	pTable = NULL;
	return true;
}

bool TeAdo::beginTransaction()
{
	transactionCounter_++;

	if (transactionCounter_ > 1)
	{
		return true;
	}
	else
	{
		HRESULT hr = connection_->BeginTrans();
		return !(FAILED(hr));
	}
}

bool TeAdo::commitTransaction()
{
	transactionCounter_--;

	if (transactionCounter_ > 0)
	{
		return true;
	}
	else
	{
		HRESULT hr = connection_->CommitTrans();
		return !(FAILED(hr));
	}
}

bool TeAdo::rollbackTransaction()
{
	transactionCounter_--;

	if (transactionCounter_ > 0)
	{
		return true;
	}
	else
	{
		HRESULT hr = connection_->RollbackTrans();
		return !(FAILED(hr));
	}
}


//----- TeAdoPortal methods ---

TeAdoPortal::TeAdoPortal ( TeDatabase*  pDatabase)
{
	db_ = pDatabase;
	connection_ = ((TeAdo*)pDatabase)->connection_;
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
}


TeAdoPortal::TeAdoPortal ()
{
	recset_.CreateInstance(__uuidof(ADODB::Recordset));
	curRow_ = 0;
}

TeAdoPortal::~TeAdoPortal ()
{
	freeResult();
}

bool 
TeAdoPortal::isConnected()
{
	if(connection_ != NULL)
	{
		if(connection_->State==ADODB::adStateOpen)
			return true;
	}
	return false;
}

bool 
TeAdoPortal::isEOF()
{
	if (recset_->IsEOF)
		return true;
	return false;
}

bool 
TeAdoPortal::moveFirst()
{
	try
	{
		HRESULT hr = recset_->MoveFirst();
		TESTHR(hr);
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}
	return true;
}

bool 
TeAdo::LinearRingToVariant(VARIANT &varArray, TeLinearRing& ring)
{
	unsigned long i, iac = 0;
	double *points = new double[2*ring.size()];
	for (i=0;i<ring.size();i++)
	{
		points[iac++]=ring[i].x();
		points[iac++]=ring[i].y();
	}

	if (!PointsToVariant(varArray, points, ring.size()))
	{
		delete [] points;
		return false;
	}

	delete [] points;
	return true;
}

bool 
TeAdo::LineToVariant(VARIANT &varArray, TeLine2D& line)
{
	unsigned long i, iac = 0;
	double *points = new double[2*line.size()];
	for (i=0;i<line.size();i++)
	{
		points[iac++]=line[i].x();
		points[iac++]=line[i].y();
	}

	if (!PointsToVariant(varArray, points, line.size()))
	{
		delete [] points;
		return false;
	}

	delete [] points;
	return true;
}

bool 
TeAdo::PointsToVariant(VARIANT &varArray, double* points, int npoints)
{
	BYTE *pByte;
	try
	{
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;	
		rgsabound[0].cElements = 2 * npoints * sizeof(double);

// create a single dimensional byte array
		psa = SafeArrayCreate(VT_I1, 1, rgsabound);
			
// set the data of the array with data in the edit box
		if(SafeArrayAccessData(psa,(void **)&pByte) == NOERROR)
			memcpy((LPVOID)pByte,(LPVOID)points,2 * npoints * sizeof(double));
		SafeArrayUnaccessData(psa);

		varArray.vt = VT_ARRAY | VT_UI1;
		varArray.parray = psa;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error PointsToVariant";
		return false;
	}
	return true;
}

bool 
TeAdoPortal::BlobToLine(TeLine2D& line)
{
	long		size;
	_variant_t	varBLOB;
	int			k,
				np = atoi (getData("num_coords"));
	double		x,
				y,
				*points = 0;
	try
	{
// get the actual length of the data
		size = recset_->Fields->Item["spatial_data"]->ActualSize;
	
		if(size > 0)
		{
			VariantInit(&varBLOB);
// get the chunk of data 
			varBLOB = recset_->Fields->Item["spatial_data"]->GetChunk(size);
			
// If the data retrieved is array of bytes then get data from the array
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				SafeArrayAccessData(varBLOB.parray,(void **)&points);
				line.reserve(np);
				for (k=0;k<np;k++)
				{
					x = *points++;
					y = *points++;
					TeCoord2D pt(x,y);
					line.add(pt);
				}
				SafeArrayUnaccessData(varBLOB.parray);
			}
			
		}
// some of the providers return -1 as the data length 
// in that case loop through to get the data in chunks
		else
		if(size < 0)
		{
			int nDataLenRetrieved = 0;
			do
			{
				VariantInit(&varBLOB);
				int kkk = recset_->Fields->Item["spatial_data"]->Attributes ;
				if (kkk & ADODB::adFldLong)
				{

					varBLOB = recset_->Fields->Item["spatial_data"]->GetChunk((long)CHUNKSIZE);

					// If the data retrieved is array of bytes then get data from the array and set the 
					// data value variable of the edit box 
					if(varBLOB.vt == (VT_ARRAY | VT_UI1))
					{
						nDataLenRetrieved = varBLOB.parray->rgsabound[0].cElements;
						SafeArrayAccessData(varBLOB.parray,(void **)&points);

						SafeArrayUnaccessData(varBLOB.parray);
						// get the length of chunk retrieved
					}
				}
			}
			while(nDataLenRetrieved == CHUNKSIZE);  // loop through to get the in chunks of data of CHUNKSIZE size
//			errorMessage_ = "Esta vindo em chunks";
//			return false;
		}


	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error in BlobToLine!";
		return false;
	}
	return true;
}

bool 
TeAdoPortal::moveNext()
{
    HRESULT hr;

	hr = recset_->MoveNext();
	if (FAILED(hr))
		return false;
	return true;
}

bool 
TeAdoPortal::query (const string &qry, TeCursorLocation l, TeCursorType /* t */, TeCursorEditType  e , TeCursorDataType /* dt */  )
{
   
 	try
	{
		if(l==TeCLIENTESIDE)
			recset_->CursorLocation=ADODB::adUseClient;
		else
			recset_->CursorLocation=ADODB::adUseServer;

		ADODB::LockTypeEnum lockType = ADODB::adLockReadOnly;
		if(e == TeREADWRITE)
		{
			lockType = ADODB::adLockOptimistic;
		}

		recset_->Open(qry.c_str(),connection_.GetInterfacePtr(),
					ADODB::adOpenKeyset,
					lockType,
					ADODB::adCmdText);
		numRows_ = recset_->GetRecordCount();
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}

	ADODB::FieldsPtr ptrFields = NULL;
	ADODB::FieldPtr ptrField = NULL;
	ptrFields = recset_->Fields;
	numFields_ = ptrFields->Count; 

	attList_.clear ();
	int i;
	for(i = 0; i < numFields_; i++)
	{
		_variant_t vCol((long)i);
		ptrFields->get_Item(vCol, &ptrField);
		TeAttribute attribute;
		int nType = ptrField->Type;
		switch (nType)
		{
		case ADODB::adBoolean:
			attribute.rep_.type_ = TeBOOLEAN;
			break;

		case ADODB::adVarNumeric:
		case ADODB::adInteger:
		case ADODB::adSmallInt:
		case ADODB::adTinyInt:
		case ADODB::adUnsignedInt:
		case ADODB::adUnsignedSmallInt:
		case ADODB::adUnsignedTinyInt:
			attribute.rep_.type_ = TeINT;
			break;

		case ADODB::adNumeric:
		case ADODB::adDecimal:
			attribute.rep_.type_ =(ptrField->GetNumericScale()=='0' ? TeINT : TeREAL);
			break;

		case ADODB::adSingle:
		case ADODB::adDouble:
			attribute.rep_.type_ = TeREAL;
			break;

		case ADODB::adDate:
		case ADODB::adDBDate:
		case ADODB::adDBTime:
		case ADODB::adDBTimeStamp:
			attribute.rep_.type_ = TeDATETIME;
			attribute.dateChronon_ = TeSECOND;
			attribute.dateTimeFormat_ = TeAdo::systemDateTimeFormat(attribute.indicatorAM_, attribute.indicatorPM_, attribute.dateSeparator_, attribute.timeSeparator_);
			break;

		case ADODB::adLongVarBinary:
		case ADODB::adVarBinary:
		case ADODB::adBinary:
			attribute.rep_.type_ = TeBLOB;
			break;

		case ADODB::adChar: 
		case ADODB::adWChar:
		case ADODB::adVarChar:
		case ADODB::adVarWChar:
		case ADODB::adLongVarChar:
		case ADODB::adLongVarWChar: 
			attribute.rep_.type_ = TeSTRING;
			break;
		break;

		case ADODB::adCurrency: // ??? CURRENCY
		default :
			attribute.rep_.type_ = TeUNKNOWN;
			break;
		}
		attribute.rep_.name_ = ptrField->Name;
		attribute.rep_.numChar_ = ptrField->DefinedSize;
		attList_.push_back ( attribute );
	}

	curRow_=-1;
	return true;
}

bool 
TeAdoPortal::fetchRow ()
{
	try
	{
		if (curRow_ == -1)
			moveFirst();
		else
			moveNext();
		if( isEOF() )
			return false;
		curRow_++;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchRow!";
		return false;
	}

	return true;
}

bool 
TeAdoPortal::fetchRow (int i)
{
	curRow_ = i;
	try
	{
		moveFirst ();
		HRESULT hr = recset_->Move(i);
		TESTHR(hr);
	}
	catch(_com_error &e)
	{
		errorMessage_ = (char*)e.ErrorMessage();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return false;
	}	return true;
}


void 
TeAdoPortal::freeResult ()
{
    long state = ADODB::adStateClosed;

	recset_->get_State(&state);
	if (state == ADODB::adStateOpen)
  		recset_->Close();
}

bool 
TeAdoPortal::fetchGeometry(TePolygon& pol)
{
	try 
	{
		TeLine2D line;
		int numHoles = atoi(this->getData("num_holes"));
		this->fetchGeometry(line);	// get outter ring
		
		if(line.empty())
		{
			fetchRow(); 
			if(recset_->IsEOF)
				return false;
			else
				return true;
		}
					
		TeLinearRing ring(line);
		pol.objectId ( ring.objectId());
		pol.geomId ( ring.geomId() );
		pol.add ( ring );
		int parentId = pol.geomId();
		while (!recset_->IsEOF && numHoles>0 && (atoi(this->getData("parent_id")) == parentId)) // get holes
		{
			TeLinearRing hole = getLinearRing();
			pol.add ( hole );
			fetchRow();
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	if(recset_->IsEOF)
		return false;
	else
		return true;
}

bool 
TeAdoPortal::fetchGeometry(TePolygon& pol, const unsigned int& initIndex)
{
	try 
	{
		TeLine2D line;
		int numHoles = atoi(this->getData(initIndex+3));
		// get outter ring
		line.objectId (string(getData (initIndex+1)));
		line.geomId (atoi(getData(initIndex)));
		BlobToLine(line);
		this->fetchRow();
		
		if(line.empty())
		{
			fetchRow(); 
			if(recset_->IsEOF)
				return false;
			else
				return true;
		}
					
		TeLinearRing ring(line);
		pol.objectId ( ring.objectId());
		pol.geomId ( ring.geomId() );
		pol.add ( ring );
		int parentId = pol.geomId();
		while (!recset_->IsEOF && numHoles>0 && (atoi(this->getData(initIndex+4)) == parentId)) // get holes
		{
			TeLine2D line;
			BlobToLine(line);
			TeLinearRing hole( line );
			hole.objectId (string (getData (initIndex+1)));
			hole.geomId (atoi(getData(initIndex)));
			pol.add ( hole );
			fetchRow();
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	if(recset_->IsEOF)
		return false;
	else
		return true;
}

bool 
TeAdoPortal::fetchGeometry(TeLine2D& line)
{
	try {
		int index = atoi (getData ("geom_id"));
		TeBox b (getDouble("lower_x"),getDouble("lower_y"),getDouble("upper_x"),getDouble("upper_y"));
		line.objectId (string (getData ("object_id")));
		line.geomId (index);
		line.setBox (b);
		BlobToLine(line);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeAdoPortal::fetchGeometry(TeLine2D& line, const unsigned int& initIndex)
{
	try 
	{
		line.objectId (string (getData (initIndex+1)));
		line.geomId (atoi (getData (initIndex)));
		BlobToLine(line);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeAdoPortal::fetchGeometry(TePoint& p)
{
	try {
		p.geomId( atoi(getData("geom_id")));
		p.objectId( string(getData("object_id")));
		double x = getDouble("x");
		double y = getDouble("y");
		TeCoord2D c(x, y);
		p.add(c);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

bool 
TeAdoPortal::fetchGeometry(TePoint& p, const unsigned int& initIndex)
{
	try 
	{
		p.geomId( atoi(getData(initIndex)));
		p.objectId( string(getData(initIndex+1)));
		double x = getDouble(initIndex+2);
		double y = getDouble(initIndex+3);
		TeCoord2D c(x, y);
		p.add(c);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
}

TeLinearRing 
TeAdoPortal::getLinearRing()
{
	int index = atoi(getData ("geom_id"));
	TeBox b (getDouble("lower_x"),getDouble("lower_y"),getDouble("upper_x"),getDouble("upper_y"));
	TeLine2D line;
	BlobToLine(line);
	TeLinearRing ring( line );
	ring.objectId (string (getData ("object_id")));
	ring.geomId (index);
	ring.setBox (b);
	return(ring);
}

bool 
TeAdoPortal::fetchGeometry (TeNode& n)
{
	try {
		TeCoord2D c(getDouble("x"), getDouble("y"));
		n.geomId( atol(getData("geom_id")));
		n.objectId( string(getData("object_id")));
		n.add(c);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	return true;
}

bool 
TeAdoPortal::fetchGeometry (TeNode& n, const unsigned int& initIndex)
{
	try {
		TeCoord2D c(getDouble(initIndex+2), getDouble(initIndex+3));
		n.geomId( atol(getData(initIndex)));
		n.objectId( string(getData(initIndex+1)));
		n.add(c);
		return(this->fetchRow());
	} 
	catch (_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error fetchGeometry!";
		return false;
	}
	return true;
}

char* 
TeAdoPortal::getData (int i)
{
    _variant_t vtIndex;
	_variant_t value;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return 0;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return "";
	}
	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		return (char*)bvalue_;
	}
	else
		return "";
}

char* 
TeAdoPortal::getData (const string& s)
{
	_variant_t value;
	try
	{
		value = recset_->GetCollect(s.c_str());
	}
	catch(_com_error &e)
	{
		string field = TeGetExtension(s.c_str());
		if (!field.empty())
		{
			try
			{
				value = recset_->GetCollect(field.c_str());
			}
			catch(_com_error &e)
			{
				errorMessage_ = e.Description();
 				return "";
			}
			catch(...)
			{
				errorMessage_ = "Error getData!";
				return false;
			}
		}
		else
		{
			errorMessage_ = e.Description();
			return "";
		}
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return "";
	}

	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		return (char*)bvalue_;
	}
	else
		return "";
}

double 
TeAdoPortal::getDouble (int i)
{
    _variant_t vtIndex;
	_variant_t value;
	double d = 0.;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
		if(value.vt!=VT_NULL)
			d = (double)value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
	}

	return d;
}

double 
TeAdoPortal::getDouble (const string& s)
{
	_variant_t value;
	double d = 0.;
	try
	{
		value = recset_->GetCollect(s.c_str());
	}
	catch(_com_error &e)
	{
		string field = TeGetExtension(s.c_str());
		if (!field.empty())
		{
			try
			{
				value = recset_->GetCollect(field.c_str());
			}
			catch(_com_error &e)
			{
				errorMessage_ = e.Description();
				return 0.;
			}
			catch(...)
			{
				errorMessage_ = "Error getDouble!";
				return 0.;
			}
		}
		else
		{
			errorMessage_ = e.Description();
			return 0.;
		}
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return 0.;
	}

	if(value.vt!=VT_NULL)
		d =(double)value;
	return d;
}


int 
TeAdoPortal::getInt (int i)
{
    _variant_t vtIndex;
	_variant_t value;
	int d = 0;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
		if(value.vt!=VT_NULL)
			d = (int)value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
	}

	return d;
}

int 
TeAdoPortal::getInt(const string& s)
{
	_variant_t value;
	int d = 0;
	try
	{
		value = recset_->GetCollect(s.c_str());
	}
	catch(_com_error &e)
	{
		string field = TeGetExtension(s.c_str());
		if (!field.empty())
		{
			try
			{
				value = recset_->GetCollect(field.c_str());
			}
			catch(_com_error &e)
			{
				errorMessage_ = e.Description();
				return 0;
			}
			catch(...)
			{
				errorMessage_ = "Error getDouble!";
				return 0;
			}
		}
		else
		{
			errorMessage_ = e.Description();
			return 0;
		}
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return 0;
	}

	if(value.vt!=VT_NULL)
		d = (int)value;
	return d;
}


bool 
TeAdoPortal::getBool(const string& s)
{
	_variant_t value;
	try
	{
		value = recset_->GetCollect(s.c_str());
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error getBool!";
		return false;
	}

	if (value.vt != VT_NULL)
		return (bool)value;
		
	return false;	
}

bool
TeAdoPortal::getBool (int i)
{
    _variant_t vtIndex;
	_variant_t value;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error getBool!";
		return false;
	}
	if (value.vt != VT_NULL)
		return (bool)value;
		
	return false;	
}

TeTime
TeAdoPortal::getDate (const string& name)
{
	TeTime t;
	_variant_t value;

	TeAdo* adoDb = (TeAdo*) this->getDatabase();

	try
	{
		value = recset_->GetCollect(name.c_str());
	}
	catch(_com_error &e)
	{
		string field = TeGetExtension(name.c_str());
		if (!field.empty())
		{
			try
			{
				value = recset_->GetCollect(field.c_str());
			}
			catch(_com_error &e)
			{
				errorMessage_ = e.Description();
				return t;
			}
			catch(...)
			{
				errorMessage_ = "Error getDate!";
				return t;
			}
		}
		else
		{
			errorMessage_ = e.Description();
			return t;
		}
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return t;
	}

	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		TeTime t = TeTime( string((char*)bvalue_), TeSECOND, adoDb->systemDateTimeFormat_, 
			adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_); 
		return t;
	}
	else
		return TeTime();
}


TeTime
TeAdoPortal::getDate (int i)
{
    _variant_t vtIndex;
	_variant_t value;

    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	
	TeTime t;
	TeAdo* adoDb = (TeAdo*) this->getDatabase();
	
	try
	{
		value = recset_->GetFields()->GetItem(vtIndex)->Value;
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return t;
	}
	catch(...)
	{
		errorMessage_ = "Oppps !";
		return t;
	}
	if (value.vt != VT_NULL)
	{
		bvalue_ = _bstr_t(value);
		TeTime time (string((char*)bvalue_), TeSECOND, adoDb->systemDateTimeFormat_, 
			adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_);
		return time;
	}
	else
		return t;
}



string 
TeAdoPortal::getDateAsString(const string& s)
{
	TeTime t = this->getDate(s);
	TeAdo* adoDb = (TeAdo*) this->getDatabase();
	if (t.isValid())
	{		string tval = "#"+t.getDateTime(adoDb->systemDateTimeFormat_, adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_, adoDb->systemIndAM_)+"#";
		return tval;
	}
	else
		return "";
}

string 
TeAdoPortal::getDateAsString(int i)
{
	TeTime t = this->getDate(i);
	TeAdo* adoDb = (TeAdo*) this->getDatabase();
	if (t.isValid())
	{		string tval = "#"+t.getDateTime(adoDb->systemDateTimeFormat_, adoDb->systemDateSep_, adoDb->systemTimeSep_, adoDb->systemIndPM_, adoDb->systemIndAM_)+"#";
		return tval;
	}
	else
		return "";
}

bool
TeAdoPortal::getBlob (int i, unsigned char* &data, long& size)
{
	_variant_t	varBLOB;
	_variant_t vtIndex;
    vtIndex.vt = VT_I4;
	vtIndex.lVal = i;
	char		*cdata = 0;
	try
	{
		// get the actual length of the data
		size = recset_->GetFields()->GetItem(vtIndex)->ActualSize;
		if(size > 0)
		{
			VariantInit(&varBLOB);
			// get the chunk of data 
			varBLOB = recset_->GetFields()->GetItem(vtIndex)->GetChunk(size);
			
			// If the data retrieved is array of bytes then get data from the array
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				SafeArrayAccessData(varBLOB.parray,(void **)&cdata);
				data = new unsigned char[size];
				memcpy(data, cdata, size * sizeof(char));
				SafeArrayUnaccessData(varBLOB.parray);
			}
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error getBlob!";
		return false;
	}
	return true;
}

bool
TeAdoPortal::getBlob (const string& s, unsigned char* &data, long& size)
{
	_variant_t	varBLOB;
	char		*cdata = 0;
	try
	{
		// get the actual length of the data
		size = recset_->Fields->Item[s.c_str()]->ActualSize;
		if(size > 0)
		{
			VariantInit(&varBLOB);
			// get the chunk of data 
			varBLOB = recset_->Fields->Item[s.c_str()]->GetChunk(size);
			
			// If the data retrieved is array of bytes then get data from the array
			if(varBLOB.vt == (VT_ARRAY | VT_UI1))
			{
				SafeArrayAccessData(varBLOB.parray,(void **)&cdata);
				data = new unsigned char[size];
				memcpy(data, cdata, size * sizeof(char));
				SafeArrayUnaccessData(varBLOB.parray);
			}
		}
	}
	catch(_com_error &e)
	{
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error getBlob!";
		return false;
	}
	return true;
}

bool
TeAdoPortal::getRasterBlock(unsigned long& size, unsigned char* ptData)
{

	// gets the spatial data
	unsigned long	len;
	_variant_t	var;
	BYTE	*buffer;
	try
	{
		// get the actual length of the compressed data
		len = recset_->Fields->Item["spatial_data"]->ActualSize;
		if(len > 0)
		{
			VariantInit(&var);
			// get the chunk of data 
			var = recset_->Fields->Item["spatial_data"]->GetChunk(len);
				
			// if the data retrieved is array of bytes then get data from the array
			if(var.vt == (VT_ARRAY | VT_UI1))
			{
				size = len;
				SafeArrayAccessData(var.parray,(void **)&buffer);
				memcpy((LPVOID)ptData,(LPVOID)buffer,len);
				SafeArrayUnaccessData(var.parray);
			}
			VariantClear(&var);

		}
	}
	catch(_com_error &e)
	{
		size = 0;
		errorMessage_ = e.Description();
		return false;
	}
	catch(...)
	{
		errorMessage_ = "Error getRasterBlock!";
		size = 0;
		return false;
	}
	return true;
}