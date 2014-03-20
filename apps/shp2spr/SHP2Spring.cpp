#ifdef WIN32
#pragma warning ( disable: 4786 )
#endif

#include "TeException.h"
#include "TeStdFile.h"
#include "TeSPRFile.h"
#include "TeGeometry.h"
#include "TeTable.h"
#include "TeDefines.h"
#include "TeUtils.h"
#include "TeLayer.h"
#include "TeAttribute.h" 
#include "TeErrorMessage.h"
#include "TeMemoryDB.h"

#include <string>
#include <iostream>
using namespace std;

bool TeImportShape(TeLayer& layer, const string& shpFileName, string attrTableName_,  string indexName_, unsigned int chunkSize);
void TeExportSPR(TeLayer& layer, const string& sprFileName, const string& objectCategory, int dType);
void TeReadSHPAttributeList(const string& fileName, TeAttributeList& attList);

int 
main(int argc, char* argv[])
{
	TeDatabase* db_ = 0; // the database to be used by this application
	TeLayer* layer = 0;  // a layer to be used by this application

	map<string,int> models;
	models["-c"] = 1;
	models["-t"] = 2;
	models["-s"] = 3;
	models["-n"] = 4;

	int option;
	int columnChoosen;
	string objectCategory;
	string shpFileName;
	string sprFileName;
	bool interf = 0;

    if (argc < 4 )
	{
		cout << "Error: worng number of parameters" << endl;
		cout << "Usage: shp2spr [-c] [-s] [-t] [-n] <shpFile> <sprFile> \n" << endl;
		cout << "	[-c] to convert to the SPRING cadastral model" << endl;
		cout << "	[-t] to convert to the SPRING thematic model" << endl;
		cout << "	[-s] to convert to the SPRING MNT model" << endl;
		cout << "	[-n] to convert to the SPRING network model" << endl;
		exit ( 0 );
	}
	
	if ( string(argv[1]) == "-i")
	{
		option = atoi(argv[2]);
		columnChoosen = atoi(argv[3]);

		objectCategory = argv[4];
		if (objectCategory == "none")
			objectCategory = "Objetos";

		shpFileName = string(argv[5]) + ".shp";
		// Create output SPRING file name
		sprFileName = string(argv[6]) + ".spr";
		interf = 1;
	}
	else
	{
		option = models[string(argv[1])];
		if ( option == 0 )
		{
				cout << "Error: spring model option invalid!" << endl;
				cout << "Usage: shp2spr [-c] [-s] [-t] [-n] <shpFile> <sprFile> \n" << endl;
				cout << " [-c] to convert to the SPRING cadastral model" << endl;
				cout << " [-t] to convert to the SPRING thematic model" << endl;
				cout << " [-s] to convert to the SPRING MNT model" << endl;
				cout << " [-n] to convert to the SPRING network model" << endl;
				exit ( 0 );
		}

		shpFileName = TeGetName(argv[2]) + ".shp";
		sprFileName = TeGetName(argv[3]) + ".spr";

		cout << "In: "<< shpFileName << " Out: " << sprFileName << endl << endl;
	}

	try
	{
		if (!interf && (option == 1 || option == 4))
		{
			cout << "Enter SPRING object Category (0 = default) : " << endl ;
			cin >> objectCategory;
			cout << endl;

			if ( objectCategory == "2" )
				objectCategory = "Objetos";
		}

		TeBox bb_;
		// creates a memory database to be used by this application
		if (!db_)
		{
			db_ = new TeMemoryDB();
			if (!db_->connect ("","","",""))
			{
				cerr <<"Falha na conexão.." <<endl;
				exit(0);
			}
		}

		layer = new TeLayer("layer",db_);		
		TeAttributeList attList;

		// Retrieve the attributes information data from the MIF file
		TeReadSHPAttributeList(shpFileName,attList);

		if (!interf)
		{
			if ( attList.size()	> 0 ) 
			{
				cout << endl << "Attribute names:" << endl;
				TeAttributeList::iterator it = attList.begin();

				int i=0;
				while ( it != attList.end() )
				{
					TeAttribute att = *it;
					cout << i+1 << " : " << att.rep_.name_ << "\t"; 
					++it;
					i++;
					if ( (i % 3 ) == 0 )
					cout << endl;
				}
				cout << endl << endl;
						
				if (option == 1  || option == 4)
					cout << "Enter column number to be used as index (0 to NONE ): " << endl;
				else if (option == 2)
					cout << "Enter column number that has the themes information (0 to NONE ): " << endl;
				else if (option == 3)
					cout << "Enter column number to be used as the quote value (0 to NONE ):  " << endl;

				cin >> columnChoosen;
				cout << endl;
				if (columnChoosen > attList.size())
					throw TeException ( NO_DTM_QUOTE_VALUE );
			}
		}
		
		string indexName;
		// If a column was choosen as an index
		if ( columnChoosen > 0 )
			indexName = attList[ columnChoosen - 1 ].rep_.name_ ;

    	// Import the geometries from MIF File
		TeImportShape(*layer,shpFileName,"",indexName,30);
		TeExportSPR(*layer, sprFileName, objectCategory, option );
	}
	catch (TeException e)
	{
		cout <<  e.message();
	}
	if ( !interf )
	{ 
		cout << "\nEnd of convertion!" << endl;
	}
	return 0;
}
