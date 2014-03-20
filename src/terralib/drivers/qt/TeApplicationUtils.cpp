/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
*************************************************************************************/

#include "TeApplicationUtils.h"
#include "../../kernel/TeSlice.h"
#include "../../kernel/TeDatabase.h"
#include "../../utils/TeDatabaseUtils.h"
#include "../../utils/TeColorUtils.h"
#include "../../stat/TeKernelParams.h"
#include "../../kernel/TeExternalTheme.h"

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#endif

#include <errno.h>

extern void TeGroupByUniqueValue(vector<string>& vec, TeAttrDataType tipo, vector<TeSlice>& result, int precision);


bool createAppThemeTable(TeDatabase* db, bool integrity, const string& tableName)
{
	if (!db)
		return false;
		
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeINT;
	at1.rep_.name_ = "theme_id";
	at1.rep_.isPrimaryKey_ = true;
	attList.push_back(at1);
	
	TeAttribute at2;
	at2.rep_.type_ = TeINT;
	at2.rep_.name_ = "visible_out_coll";
	attList.push_back(at2);	
	
	TeAttribute at3;
	at3.rep_.type_ = TeINT;
	at3.rep_.name_ = "visible_without_conn";
	attList.push_back(at3);	

	TeAttribute at31;
	at31.rep_.type_ = TeSTRING;
	at31.rep_.numChar_ = 255;
	at31.rep_.name_ = "refine_attribute_where";
	attList.push_back(at31);	

	TeAttribute at32;
	at32.rep_.type_ = TeSTRING;
	at32.rep_.numChar_ = 255;
	at32.rep_.name_ = "refine_spatial_where";
	attList.push_back(at32);	

	TeAttribute at33;
	at33.rep_.type_ = TeSTRING;
	at33.rep_.numChar_ = 255;
	at33.rep_.name_ = "refine_temporal_where";
	attList.push_back(at33);	

	TeAttribute at4;
	at4.rep_.type_ = TeSTRING;
	at4.rep_.numChar_ = 255;
	at4.rep_.name_ = "grouping_color";
	attList.push_back(at4);	

	TeAttribute at5;
	at5.rep_.type_ = TeINT;
	at5.rep_.name_ = "grouping_color_direction";
	attList.push_back(at5);	

	TeAttribute at6;
	at6.rep_.type_ = TeINT;
	at6.rep_.name_ = "grouping_objects";
	attList.push_back(at6);	

	TeAttribute at7;
	at7.rep_.type_ = TeREAL;
	at7.rep_.numChar_ = 20;
	at7.rep_.decimals_ = 9;
	at7.rep_.name_ = "pie_diameter";
	attList.push_back(at7);	

	TeAttribute at8;
	at8.rep_.type_ = TeREAL;
	at8.rep_.numChar_ = 20;
	at8.rep_.decimals_ = 9;
	at8.rep_.name_ = "pie_max_diameter";
	attList.push_back(at8);	

	TeAttribute at9;
	at9.rep_.type_ = TeREAL;
	at9.rep_.numChar_ = 20;
	at9.rep_.decimals_ = 9;
	at9.rep_.name_ = "pie_min_diameter";
	attList.push_back(at9);	

	TeAttribute at10;
	at10.rep_.type_ = TeSTRING;
	at10.rep_.numChar_ = 255;
	at10.rep_.name_ = "pie_dimension_attr";
	attList.push_back(at10);	

	TeAttribute at11;
	at11.rep_.type_ = TeREAL;
	at11.rep_.numChar_ = 20;
	at11.rep_.decimals_ = 9;
	at11.rep_.name_ = "bar_max_height";
	attList.push_back(at11);	

	TeAttribute at12;
	at12.rep_.type_ = TeREAL;
	at12.rep_.numChar_ = 20;
	at12.rep_.decimals_ = 9;
	at12.rep_.name_ = "bar_min_height";
	attList.push_back(at12);	

	TeAttribute at13;
	at13.rep_.type_ = TeREAL;
	at13.rep_.numChar_ = 20;
	at13.rep_.decimals_ = 9;
	at13.rep_.name_ = "bar_width";
	attList.push_back(at13);	

	TeAttribute at14;
	at14.rep_.type_ = TeREAL;
	at14.rep_.numChar_ = 20;
	at14.rep_.decimals_ = 9;
	at14.rep_.name_ = "chart_max_value";
	attList.push_back(at14);	

	TeAttribute at15;
	at15.rep_.type_ = TeREAL;
	at15.rep_.numChar_ = 20;
	at15.rep_.decimals_ = 9;
	at15.rep_.name_ = "chart_min_value";
	attList.push_back(at15);	

	TeAttribute at16;
	at16.rep_.type_ = TeINT;
	at16.rep_.name_ = "chart_type";
	attList.push_back(at16);	

	TeAttribute at17;
	at17.rep_.type_ = TeINT;
	at17.rep_.name_ = "chart_objects";
	attList.push_back(at17);
	 
	TeAttribute at18;
	at18.rep_.type_ = TeINT;
	at18.rep_.name_ = "keep_dimension";
	attList.push_back(at18);

	TeAttribute at19;
	at19.rep_.type_ = TeINT;
	at19.rep_.name_ = "show_canvas_leg";
	attList.push_back(at19);

	TeAttribute at20;
	at20.rep_.type_ = TeREAL;
	at20.rep_.numChar_ = 20;
	at20.rep_.decimals_ = 9;
	at20.rep_.name_ = "canvas_leg_x";
	attList.push_back(at20);

	TeAttribute at21;
	at21.rep_.type_ = TeREAL;
	at21.rep_.numChar_ = 20;
	at21.rep_.decimals_ = 9;
	at21.rep_.name_ = "canvas_leg_y";
	attList.push_back(at21);

	TeAttribute at22;
	at22.rep_.type_ = TeINT;
	at22.rep_.name_ = "mix_color";
	attList.push_back(at22);

	TeAttribute at23;
	at23.rep_.type_ = TeINT;
	at23.rep_.name_ = "count_obj";
	attList.push_back(at23);

	TeAttribute at24;
	at24.rep_.type_ = TeSTRING;
	at24.rep_.name_ = "text_table";
	at24.rep_.numChar_ = 50;
	attList.push_back(at24);
	
	TeAttribute at26;
	at26.rep_.type_ = TeSTRING;
	at26.rep_.name_ = "chart_function";
	at26.rep_.numChar_ = 10;
	attList.push_back(at26);
	
	bool status = db->createTable(tableName,attList);
	attList.clear();
	
	if(status && integrity)
	{
		if (!db->createRelation("fk_themeAppl_theme_id", tableName, "theme_id", "te_theme", "theme_id", true))
			return false;
	}
	return status;
}


bool populateAppThemeTable(TeDatabase* db, const string& tableName)
{
	string sel, ins; 
	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return false;
	
	if(db->dbmsName()!="MySQL") //MySQL doesn´t support NOT EXISTS  
	{
		ins  = " INSERT INTO " + tableName + " (theme_id) ";
		ins += " SELECT t.theme_id FROM te_theme t ";
		ins += " WHERE NOT EXISTS ";
		ins += " ( SELECT ta.theme_id FROM ";
		ins += " " + tableName + " ta ";
		ins += " WHERE ta.theme_id = t.theme_id ) ";
		if(!db->execute(ins))
		{
			delete portal;
			return false;
		}
	}
	else // this is for MySQL only
	{
		sel = " SELECT theme_id  FROM te_theme  ";
		if(!portal->query(sel)) 
		{
			delete portal;
			return false;
		}
		
		while (portal->fetchRow())
		{
			ins  = " INSERT INTO te_theme_application (theme_id) VALUES (";
			ins += string(portal->getData(0));
			ins += " )";
			
			db->execute(ins);
		} 
	}

	////-------- Insert text table
	//portal->freeResult();

	//sel = " SELECT theme_id, geom_table ";
	//sel += " FROM te_theme INNER JOIN te_representation ON ";
	//sel += " te_theme.layer_id = te_representation.layer_id ";
	//sel += " WHERE te_representation.geom_type = 128 ";
	//
	//if(!portal->query(sel)) 
	//{
	//	delete portal;
	//	return false;
	//}
	//
	//while (portal->fetchRow())
	//{
	//	ins  = " UPDATE te_theme_application ";
	//	ins += " SET  text_table = '"+ string(portal->getData(1)) +"'";
	//	ins += " WHERE theme_id = "+ string(portal->getData(0));
	//			
	//	db->execute(ins);
	//} 

	delete portal;
	return true;
}


bool deleteAppTheme (TeDatabase* db, TeAbstractTheme* theme, const string& tableName)
{
	if (!db || !theme || theme->id() < 1) 
		return false;

	int themeId = theme->id();

	//delete proximity matrix
	deleteGPMTable(db, themeId); 

	if (!db->deleteTheme(themeId))
		return false;

	if(db->tableExist("te_kernel_metadata"))
	{
		string sql = "DELETE FROM te_kernel_metadata WHERE event_theme_id_1 = " + Te2String(themeId);
		sql += " OR event_theme_id_2 = "+ Te2String(themeId);
		db->execute(sql);
	}

	if (db->existRelation(tableName, "fk_themeAppl_theme_id") != TeRICascadeDeletion)
	{
		// Delete all chart parameters associated to the theme
		string sql = "DELETE FROM " + tableName + " WHERE theme_id =" + Te2String(themeId);
		if (!db->execute (sql))
			return false;
	}

	if (db->existRelation("te_chart_params", "fk_chartpar_theme_id") != TeRICascadeDeletion)
	{
		// Delete all chart parameters associated to the theme
		string sql = "DELETE FROM te_chart_params WHERE theme_id =" + Te2String(themeId);
		if (!db->execute (sql))
			return false;
	}
	return true;
}


bool createMediaTable(TeDatabase* db, const string& tname)
{
	if (!db)
		return false;
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeSTRING;
	at1.rep_.numChar_ = 255;
	at1.rep_.name_ = "media_name";
	at1.rep_.isPrimaryKey_ = true;
	attList.push_back(at1);	

	TeAttribute at2;
	at2.rep_.type_ = TeSTRING;
	at2.rep_.numChar_ = 50;
	at2.rep_.name_ = "media_type";
	attList.push_back(at2);	

	TeAttribute at3;
	at3.rep_.type_ = TeSTRING;
	at3.rep_.numChar_ = 255;
	at3.rep_.name_ = "description";
	attList.push_back(at3);
	
	TeAttribute at4;
	at4.rep_.type_ = TeBLOB;
	at4.rep_.name_ = "media_blob";
	attList.push_back(at4);	
	
	TeAttribute at5;
	at5.rep_.type_ = TeSTRING;
	at5.rep_.numChar_ = 255;
	at5.rep_.name_ = "media_path";
	attList.push_back(at5);
	
	vector<string> pkeys;
	pkeys.push_back("media_name");
	
	bool status = db->createTable(tname,attList);
	attList.clear();
	return status;
}


bool createLayerMediaTable(TeDatabase* db, int layerId, const string& tname)
{
	if (!db)
		return false;
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeSTRING;
	at1.rep_.numChar_ = 50;
	at1.rep_.name_ = "object_id";
	at1.rep_.isPrimaryKey_ = true;
	attList.push_back(at1);	

	TeAttribute at3;
	at3.rep_.type_ = TeSTRING;
	at3.rep_.numChar_ = 255;
	at3.rep_.name_ = "media_name";
	at3.rep_.isPrimaryKey_ = true;
	attList.push_back(at3);
	
	TeAttribute at2;
	at2.rep_.type_ = TeSTRING;
	at2.rep_.numChar_ = 50;
	at2.rep_.name_ = "media_table";
	at2.rep_.isPrimaryKey_ = true;
	attList.push_back(at2);	

	TeAttribute at4;
	at4.rep_.type_ = TeINT;
	at4.rep_.name_ = "show_default";
	attList.push_back(at4);	
	
	if(!db->createTable(tname,attList))
	{
		attList.clear();
		return false;
	}

	//insert in te_layer_table
	TeTable table(tname, attList,"object_id", "object_id", TeAttrMedia);
	
	bool status = db->insertTableInfo(layerId, table); 
	db->layerMap()[layerId]->addAttributeTable (table);
		
	attList.clear();
	return status;
}


bool insertMedia(TeDatabase* db, const string& tableName, string& fileName, const string& description, const string& type, bool blob)
{
	string ins;
	if(blob)
	{
		 if(db->dbmsName()!="OracleSpatial")
		 {
			ins = "  INSERT INTO "+ tableName +" (media_name, media_type, description) ";
			ins += " VALUES (";
			ins += "'"+ fileName + "'";
			ins += ", '"+ type +"'";
			ins += ", '"+ description +"'";
			ins += ")";
		 }
		 else
		 {
			ins = "  INSERT INTO "+ tableName +" (media_name, media_type, description, media_blob) ";
			ins += " VALUES (";
			ins += "'"+ fileName + "'";
			ins += ", '"+ type +"'";
			ins += ", '"+ description +"'";
			ins += ", EMPTY_BLOB()";
			ins += " )";
		 }
	}
	else
	{
		 ins = "  INSERT INTO "+ tableName +" (media_name, media_type, description, media_path) ";
		 ins += " VALUES (";
		 ins += "'"+ fileName + "'";
		 ins += ", '"+ type +"'";
		 ins += ", '"+ description +"'";
		 ins += ", '"+ fileName +"'";
		 ins += ")";
	}

	if(!db->execute (ins))
		return false;

	//insert blob
	TeAttributeRep rep;
	rep.name_ = "media_name";
	rep.type_ = TeSTRING;

	if(blob)
	{
		if(!db->insertBlob (tableName, "media_blob", rep, fileName, fileName))
			return false;
	}

	return true;
}

bool insertMediaObjLayer (TeDatabase* db, TeLayer* layer, const string& objectId, const string& mediaName, const string& mediaTable)
{
	string table = layer->mediaTable();
	int show = 0;

	string ins = "  INSERT INTO "+ table +" (object_id, media_name, media_table, show_default) ";
	ins += " VALUES (";
	ins += "'"+  objectId + "'";
	ins += ", '"+ mediaName +"'";
	ins += ", '"+ mediaTable +"'";
	ins += ", "+ Te2String(show);
	ins += ")";

	if(!db->execute (ins))
		return false;

	return true;
}


bool setDefaultMedia(TeDatabase* db, TeLayer* layer, const string& objectId, const string& mediaName, const string& mediaTable)
{
	string table = layer->mediaTable();
	
	string up = "UPDATE " + table + " SET show_default = 0 WHERE ";
	up += "object_id = '" + objectId + "'";
	if(!db->execute(up))
		return false;
	
	up = "UPDATE "+ table +" SET ";
	up += " show_default = 1 ";
	up += "	WHERE 	object_id = '"+ objectId +"'";
	up += " AND		media_name = '"+ mediaName +"'";
	up += " AND		media_table = '"+ mediaTable +"'";

	if(!db->execute (up))
		return false;

	return true;
}


bool updateMediaDescription(TeDatabase* db, const string& mediaName, const string& mediaTable, const string& desc)
{
	string up = "UPDATE " + mediaTable + " SET description = '" + desc.c_str() + "'";
	up += " WHERE media_name = '" + mediaName + "'";
	if(!db->execute(up))
		return false;
	return true;
}


bool insertInfo2Blob(TeDatabase* db, int layerId, const string& tableName, const  string& fileName, const  string& type)
{
	string ins;
	string ntype = TeConvertToUpperCase(type);
	string id = Te2String(layerId);
	if(db->dbmsName()!="OracleSpatial")
	{
		ins = "  INSERT INTO "+ tableName +" (layer_id, name_, type_)";
		ins += " VALUES (" + id + ", '" + fileName + "', '" + ntype + "')";
	}
	else
	{
		ins = "  INSERT INTO "+ tableName +" (layer_id, name_, type_, media_blob)";
		ins += " VALUES (" + id + ", '" + fileName + "', '" + ntype + "', EMPTY_BLOB())";
	}

	if(!db->execute (ins))
		return false;


	if(ntype == "HTM" || ntype == "HTML" || ntype == "URL")
		return true;

	//insert blob
	TeAttributeRep rep;
	rep.name_ = "layer_id";
	rep.type_ = TeINT;

	if(!db->insertBlob (tableName, "media_blob", rep, id, fileName))
		return false;

	return true;
}

string getInfoBlob2TemporaryFile(TeDatabase* db, int layerId, const string& tableName, string& type)
{
// load media data to archive from database
	size_t	f;
	string file, swdir;
	TeDatabasePortal *portal = db->getPortal();
	string query = "SELECT * FROM " + tableName + " WHERE layer_id = " + Te2String(layerId);
	if(portal->query(query) && portal->fetchRow())
	{
		file = portal->getData(1);
		type = portal->getData(2);
		type = TeConvertToUpperCase(type);
	}

	if(file.empty())
	{
//		QMessageBox::warning(this, tr("Warning"),
//			tr("There is no information to show!\nInsert one first."));
		delete portal;
		return "";
	}

	if(type == "HTM" || type == "HTML" || type == "URL")
	{
		delete portal;
		return file;
	}

	string tempName = file;
#ifdef WIN32
	char wdir[100];
	GetWindowsDirectory((LPTSTR)wdir, 100);
	swdir = wdir;
	f = swdir.find(":\\");
	if(f != string::npos)
		swdir = swdir.substr(0, f);
	swdir.append(":\\TERRAVIEW_TEMP");
	if( access (swdir.c_str(), 06) == -1 ) // test if directory exists
	{
		if( errno == ENOENT )  // directory does not exist so create it
		{
			if ( _mkdir(swdir.c_str()) == -1)
			{
//				QString msg = tr("The directory ");
//                msg += swdir.c_str();
//                msg += tr(" could not be created");
//				QMessageBox::critical(this, tr("Error"),
//            		tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
				delete portal;
				return "";
			}
		}
	}
	swdir.append("\\");
#else
    swdir = "/tmp/TERRAVIEW_TEMP";
	if( access (swdir.c_str(), F_OK) == -1 ) // test if directory exists
	{
		if( errno == ENOENT )  // directory does not exist so create it
		{
            char buf[20];
            sprintf(buf, "mkdir %s", swdir.c_str());
			if (system(buf) != 0)
			{
//            	QMessageBox::critical(this, tr("Error"),
//            		tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
            	delete portal;
            	return "";
			}
		}
	}        
	swdir.append("/");
#endif
	f = tempName.rfind("/");
	if(f == string::npos)
		f = tempName.rfind("\\");

	if (f != string::npos)
		tempName = tempName.substr(f+1);
	tempName.insert(0, swdir);

	unsigned char *data = 0;
	long size;
	if((portal->getBlob("media_blob", data, size)) == false)
	{
		delete portal;
		return "";
	}
	delete portal;

	FILE *fp = fopen(tempName.c_str(), "wb");
	if(fp == 0)
		return "";
	if(fwrite(data, sizeof(unsigned char), size, fp) < (unsigned long)size)
		return "";
	if(data)
		delete []data;
	fclose(fp);
	
	return tempName;
}


bool createChartParamsTable(TeDatabase* db, bool integrity)
{
	if (!db)
		return false;
	
	string table = "te_chart_params";
	
	TeAttributeList attList;

	TeAttribute at5;
	at5.rep_.type_ = TeINT;
	at5.rep_.name_ = "theme_id";
	at5.rep_.isPrimaryKey_ = true;
	attList.push_back(at5);	
	
	TeAttribute at1;
	at1.rep_.type_ = TeSTRING;
	at1.rep_.numChar_ = 50;
	at1.rep_.name_ = "attr_name";
	at1.rep_.isPrimaryKey_ = true;
	attList.push_back(at1);	
	
	TeAttribute at2;
	at2.rep_.type_ = TeINT;
	at2.rep_.name_ = "red";
	attList.push_back(at2);	
	
	TeAttribute at3;
	at3.rep_.type_ = TeINT;
	at3.rep_.name_ = "blue";
	attList.push_back(at3);	
	
	TeAttribute at4;
	at4.rep_.type_ = TeINT;
	at4.rep_.name_ = "green";
	attList.push_back(at4);	
	
	bool status = db->createTable(table,attList);
	attList.clear();

	if(status && integrity)
	{
		if(db->tableExist ("te_theme_application"))
		{
			if (!db->createRelation("fk_chartpar_theme_id", "te_chart_params", "theme_id", "te_theme_application", "theme_id", true))
				return false;
		}
		else
			return false;
	}
	return status;
}


bool updateGridColor(TeDatabase* db, int themeId, TeObjectSelectionMode sel, TeColor& color)
{
	if (!db || themeId <= 0) 
		return false;

	string ins = "UPDATE te_legend SET red = " + Te2String(color.red_);
	ins += ", green = " +  Te2String(color.green_);
	ins += ", blue = "  + Te2String (color.blue_);
	ins += " WHERE theme_id = " + Te2String(themeId) + " AND ";
	if(sel == TeDefaultSelection)
		ins += "group_id = -3";
	else if(sel == TePointingSelection)
		ins += "group_id = -4";
	else if(sel == TeQuerySelection)
		ins += "group_id = -5";
	else
		ins += "group_id = -6";
	return db->execute(ins);
}

bool createKernelMetadataTable(TeDatabase* db)
{
	if (!db)
		return false;
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeINT;
	at1.rep_.name_ = "kernel_id";
	at1.rep_.isPrimaryKey_ = true;
	at1.rep_.isAutoNumber_ = true;
	attList.push_back(at1);	

	TeAttribute at2;
	at2.rep_.type_ = TeINT;
	at2.rep_.name_ = "event_theme_id_1";
	attList.push_back(at2);	
	
	TeAttribute at3;
	at3.rep_.type_ = TeSTRING;
	at3.rep_.numChar_ = 255;
	at3.rep_.name_ = "intensity_attr_name_1";
	attList.push_back(at3);
	
	TeAttribute at4;
	at4.rep_.type_ = TeSTRING;
	at4.rep_.numChar_ = 255;
	at4.rep_.name_ = "intensity_attr_table_1";
	attList.push_back(at4);

	TeAttribute at5;
	at5.rep_.type_ = TeINT;
	at5.rep_.name_ = "kernel_function_1";
	attList.push_back(at5);	

	TeAttribute at6;
	at6.rep_.type_ = TeINT;
	at6.rep_.name_ = "compute_type_1";
	attList.push_back(at6);	

	TeAttribute at7;
	at7.rep_.type_ = TeREAL;
	at7.rep_.name_ = "radius_value_1";
	attList.push_back(at7);	

	TeAttribute at8;
	at8.rep_.type_ = TeINT;
	at8.rep_.name_ = "combination_type";
	attList.push_back(at8);	

	TeAttribute at9;
	at9.rep_.type_ = TeINT;
	at9.rep_.name_ = "event_theme_id_2";
	attList.push_back(at9);	
	
	TeAttribute at10;
	at10.rep_.type_ = TeSTRING;
	at10.rep_.numChar_ = 255;
	at10.rep_.name_ = "intensity_attr_name_2";
	attList.push_back(at10);
	
	TeAttribute at11;
	at11.rep_.type_ = TeSTRING;
	at11.rep_.numChar_ = 255;
	at11.rep_.name_ = "intensity_attr_table_2";
	attList.push_back(at11);

	TeAttribute at12;
	at12.rep_.type_ = TeINT;
	at12.rep_.name_ = "kernel_function_2";
	attList.push_back(at12);	

	TeAttribute at13;
	at13.rep_.type_ = TeINT;
	at13.rep_.name_ = "compute_type_2";
	attList.push_back(at13);	

	TeAttribute at14;
	at14.rep_.type_ = TeREAL;
	at14.rep_.name_ = "radius_value_2";
	attList.push_back(at14);
	
	TeAttribute at15;
	at15.rep_.type_ = TeINT;
	at15.rep_.name_ = "support_theme_id";
	attList.push_back(at15);

	TeAttribute at17;
	at17.rep_.type_ = TeSTRING;
	at17.rep_.numChar_ = 255;
	at17.rep_.name_ = "support_theme_table";
	attList.push_back(at17);

	TeAttribute at18;
	at18.rep_.type_ = TeSTRING;
	at18.rep_.numChar_ = 255;
	at18.rep_.name_ = "support_theme_attr";
	attList.push_back(at18);

	TeAttribute at19;
	at19.rep_.type_ = TeINT;
	at19.rep_.name_ = "generated_layer_id";
	attList.push_back(at19);

	if(!db->createTable("te_kernel_metadata",attList))
	{
		attList.clear();
		return false;
	}

	db->createIndex("te_kernel_metadata", "kernel_meta_ind1", "support_theme_id, support_theme_table, support_theme_attr");
	db->createIndex("te_kernel_metadata", "kernel_meta_ind2", "generated_layer_id");

	return true;
}


bool insertKernelMetadata(TeDatabase* db, TeKernelParams& kernelPar)
{
	string ins = " INSERT INTO te_kernel_metadata (event_theme_id_1, intensity_attr_name_1, ";
	ins += " intensity_attr_table_1, kernel_function_1, compute_type_1, radius_value_1, ";
	ins += " combination_type, event_theme_id_2, intensity_attr_name_2, intensity_attr_table_2, ";
	ins += " kernel_function_2, compute_type_2, radius_value_2, support_theme_id,  ";
	ins += " support_theme_table, support_theme_attr, generated_layer_id)";
	ins += " VALUES ( ";
	ins += Te2String(kernelPar.eventThemeId1_) + ",";
	ins += "'"+ kernelPar.intensityAttrName1_ + "',";
	ins += "'"+ kernelPar.intensityAttrTable1_ + "',";
	ins += Te2String((int)kernelPar.kernelFunction1_) + ",";
	ins += Te2String((int)kernelPar.computeType1_) + ",";
	ins += Te2String(kernelPar.radiusValue1_) + ",";
	ins += Te2String((int)kernelPar.combinationType_) + ",";
	ins += Te2String(kernelPar.eventThemeId2_) + ",";
	ins += "'"+ kernelPar.intensityAttrName2_ + "',";
	ins += "'"+ kernelPar.intensityAttrTable2_ + "',";
	ins += Te2String((int)kernelPar.kernelFunction2_) + ",";
	ins += Te2String((int)kernelPar.computeType2_) + ",";
	ins += Te2String(kernelPar.radiusValue2_) + ",";
	ins += Te2String(kernelPar.supportThemeId_)+ ",";
	ins += "'"+ kernelPar.supportThemeTable_+ "',";
	ins += "'"+ kernelPar.supportThemeAttr_+ "',";
	ins += Te2String(kernelPar.generatedLayerId_);
	ins += ")";

	if(!db->execute (ins))
		return false;

	return true;
}


bool updateKernelMetadata(TeDatabase* db, TeKernelParams& kernelPar)
{
  string del = "DELETE FROM te_kernel_metadata WHERE support_theme_id = ";
  del += Te2String(kernelPar.supportThemeId_);

  if (!db->execute(del)) 
  {
    return false;
  }

  return insertKernelMetadata(db,kernelPar);  
}


bool loadKernelParams (TeDatabase* db, int supThemeId, const string& supTableName, const string& supAttrName, TeKernelParams& KernelPar)
{
	if (!db) 
		return false;

	TeDatabasePortal* portal = db->getPortal();

	string q ="SELECT event_theme_id_1, intensity_attr_name_1, ";
	q += " intensity_attr_table_1, kernel_function_1, compute_type_1, radius_value_1, ";
	q += " combination_type, event_theme_id_2, intensity_attr_name_2, intensity_attr_table_2, ";
	q += " kernel_function_2, compute_type_2, radius_value_2, support_theme_id,  ";
	q += " support_theme_table, support_theme_attr, generated_layer_id ";
	q += " FROM te_kernel_metadata WHERE ";
	q += " support_theme_id = "+ Te2String (supThemeId);
	q += " AND support_theme_table = '"+ supTableName +"'";
	q += " AND support_theme_attr = '"+ supAttrName +"'";

	if (!portal->query(q))
	{	
		delete portal;
		return false;
	}
	
	if (portal->fetchRow())
	{
		KernelPar.eventThemeId1_ = atoi(portal->getData(0));
		KernelPar.intensityAttrName1_ = string(portal->getData(1));
		KernelPar.intensityAttrTable1_ = string(portal->getData(2));
		KernelPar.kernelFunction1_ = TeKernelFunctionType(atoi(portal->getData(3)));
		KernelPar.computeType1_ = TeKernelComputeType(atoi(portal->getData(4)));
		KernelPar.radiusValue1_ = portal->getDouble(5);
		KernelPar.combinationType_ = TeKernelCombinationType(atoi(portal->getData(6)));
		KernelPar.eventThemeId2_ = atoi(portal->getData(7));
		KernelPar.intensityAttrName2_ = string(portal->getData(8));
		KernelPar.intensityAttrTable2_ = string(portal->getData(9));
		KernelPar.kernelFunction2_ = TeKernelFunctionType(atoi(portal->getData(10)));
		KernelPar.computeType2_ = TeKernelComputeType(atoi(portal->getData(11)));
		KernelPar.radiusValue2_ = portal->getDouble(12);
		KernelPar.supportThemeId_ = atoi(portal->getData(13));
		KernelPar.supportThemeTable_ = string(portal->getData(14));
		KernelPar.supportThemeAttr_ = string(portal->getData(15));
		KernelPar.generatedLayerId_ =  atoi(portal->getData(16));

		vector<int> ids;
		if(KernelPar.eventThemeId1_>0)
			ids.push_back (KernelPar.eventThemeId1_);
		if(KernelPar.eventThemeId2_>0)
			ids.push_back (KernelPar.eventThemeId2_);
		if(KernelPar.supportThemeId_>0)
			ids.push_back (KernelPar.supportThemeId_);

		string sqlIds; 
		vector<int>::iterator it = ids.begin();
		
		while (it != ids.end ())
		{
			sqlIds += Te2String((*it)) +",";
			++it;
		}

		sqlIds.replace (sqlIds.size()-1, 1, " ");
		string sql = "SELECT theme_id, name FROM te_theme WHERE theme_id IN ("+ sqlIds +")";

		portal->freeResult();
		if(!portal->query(sql))
		{
			delete portal;
			return false;
		}

		while(portal->fetchRow())
		{
			int themeId = atoi(portal->getData(0));
			string themeName = string(portal->getData(1));

			if(themeId==KernelPar.eventThemeId1_)
				KernelPar.eventThemeName1_ = themeName;
			else if (themeId==KernelPar.eventThemeId2_)
				KernelPar.eventThemeName2_ = themeName;
			else if (themeId==KernelPar.supportThemeId_)
				KernelPar.supportThemeName_ = themeName;
		}

		if(KernelPar.generatedLayerId_>0)
		{
			sql = "SELECT name FROM te_layer WHERE layer_id = "+ KernelPar.generatedLayerId_;
			
			portal->freeResult();
			if(!portal->query(sql) || !portal->fetchRow())
			{
				delete portal;
				return false;
			}
			
			KernelPar.generatedLayerName_= string(portal->getData(0));
		}

		delete portal;
		return true;
	}
	
	delete portal;
	return false;
}


bool loadKernelParams (TeDatabase* db, int layerId, TeKernelParams& KernelPar)
{
	if (!db) 
		return false;

	TeDatabasePortal* portal = db->getPortal();

	string q ="SELECT event_theme_id_1, intensity_attr_name_1, ";
	q += " intensity_attr_table_1, kernel_function_1, compute_type_1, radius_value_1, ";
	q += " combination_type, event_theme_id_2, intensity_attr_name_2, intensity_attr_table_2, ";
	q += " kernel_function_2, compute_type_2, radius_value_2, support_theme_id,  ";
	q += " support_theme_table, support_theme_attr, generated_layer_id ";
	q += " FROM te_kernel_metadata WHERE ";
	q += " generated_layer_id = "+ Te2String (layerId);

	if (!portal->query(q))
	{	
		delete portal;
		return false;
	}
	
	if (portal->fetchRow())
	{
		KernelPar.eventThemeId1_ = atoi(portal->getData(0));
		KernelPar.intensityAttrName1_ = string(portal->getData(1));
		KernelPar.intensityAttrTable1_ = string(portal->getData(2));
		KernelPar.kernelFunction1_ = TeKernelFunctionType(atoi(portal->getData(3)));
		KernelPar.computeType1_ = TeKernelComputeType(atoi(portal->getData(4)));
		KernelPar.radiusValue1_ = portal->getDouble(5);
		KernelPar.combinationType_ = TeKernelCombinationType(atoi(portal->getData(6)));
		KernelPar.eventThemeId2_ = atoi(portal->getData(7));
		KernelPar.intensityAttrName2_ = string(portal->getData(8));
		KernelPar.intensityAttrTable2_ = string(portal->getData(9));
		KernelPar.kernelFunction2_ = TeKernelFunctionType(atoi(portal->getData(10)));
		KernelPar.computeType2_ = TeKernelComputeType(atoi(portal->getData(11)));
		KernelPar.radiusValue2_ = portal->getDouble(12);
		KernelPar.supportThemeId_ = atoi(portal->getData(13));
		KernelPar.supportThemeTable_ = string(portal->getData(14));
		KernelPar.supportThemeAttr_ = string(portal->getData(15));
		KernelPar.generatedLayerId_ =  atoi(portal->getData(16));

		vector<int> ids;
		if(KernelPar.eventThemeId1_>0)
			ids.push_back (KernelPar.eventThemeId1_);
		if(KernelPar.eventThemeId2_>0)
			ids.push_back (KernelPar.eventThemeId2_);
		if(KernelPar.supportThemeId_>0)
			ids.push_back (KernelPar.supportThemeId_);

		string sqlIds; 
		vector<int>::iterator it = ids.begin();
		
		while (it != ids.end ())
		{
			sqlIds += Te2String((*it)) +",";
			++it;
		}

		sqlIds.replace (sqlIds.size()-1, 1, " ");
		string sql = "SELECT theme_id, name FROM te_theme WHERE theme_id IN ("+ sqlIds +")";

		portal->freeResult();
		if(!portal->query(sql))
		{
			delete portal;
			return false;
		}

		while(portal->fetchRow())
		{
			int themeId = atoi(portal->getData(0));
			string themeName = string(portal->getData(1));

			if(themeId==KernelPar.eventThemeId1_)
				KernelPar.eventThemeName1_ = themeName;
			else if (themeId==KernelPar.eventThemeId2_)
				KernelPar.eventThemeName2_ = themeName;
			else if (themeId==KernelPar.supportThemeId_)
				KernelPar.supportThemeName_ = themeName;
		}

		if(KernelPar.generatedLayerId_>0)
		{
			sql = "SELECT name FROM te_layer WHERE layer_id = " + Te2String(KernelPar.generatedLayerId_);		
			portal->freeResult();
			if(!portal->query(sql) || !portal->fetchRow())
			{
				delete portal;
				return false;
			}			
			KernelPar.generatedLayerName_= string(portal->getData(0));
		}
		delete portal;
		return true;
	}
	delete portal;
	return false;
}


int createTeLayerMetadata(TeDatabase* db)
{
	TeAttributeList attr;
	TeAttribute		at;

	at.rep_.type_ = TeINT;
	at.rep_.name_ = "layer_id";
	at.rep_.isPrimaryKey_ = true;
	attr.push_back(at);

	at.rep_.isPrimaryKey_ = false;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "name";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "author";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "source";
	at.rep_.numChar_ = 255;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "quality";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "description";
	at.rep_.numChar_ = 255;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "date_";
	at.rep_.numChar_ = 64;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "hour_";
	at.rep_.numChar_ = 64;
	attr.push_back(at);

	at.rep_.type_ = TeINT;
	at.rep_.name_ = "transf";
	attr.push_back(at);

	if(db->createTable ("te_layer_metadata", attr) == false)
		return 1;

	bool b = db->tableExist("te_layer_metadata");
	while(b == false)
		b = db->tableExist("te_layer_metadata");

	if (db->createRelation("fk_te_layer_met_id", "te_layer_metadata", "layer_id", "te_layer", "layer_id", true) == false)
		return 2;
	return 3;
}

int createTeTableMetadata(TeDatabase* db)
{
	TeAttributeList attr;
	TeAttribute		at;

	at.rep_.type_ = TeINT;
	at.rep_.name_ = "table_id";
	at.rep_.isPrimaryKey_ = true;
	attr.push_back(at);

	at.rep_.isPrimaryKey_ = false;
	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "name";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "author";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "source";
	at.rep_.numChar_ = 255;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "quality";
	at.rep_.numChar_ = 128;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "description";
	at.rep_.numChar_ = 255;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "date_";
	at.rep_.numChar_ = 64;
	attr.push_back(at);

	at.rep_.type_ = TeSTRING;
	at.rep_.name_ = "hour_";
	at.rep_.numChar_ = 64;
	attr.push_back(at);

	at.rep_.type_ = TeINT;
	at.rep_.name_ = "transf";
	attr.push_back(at);

	if(db->createTable ("te_table_metadata", attr) == false)
		return 1;

	bool b = db->tableExist("te_table_metadata");
	while(b == false)
		b = db->tableExist("te_table_metadata");

	if (db->createRelation("fk_te_table_met_id", "te_table_metadata", "table_id", "te_layer_table", "table_id", true) == false)
		return 2;
	return 3;
}


bool deleteTextVisual(TeLayer* layer)
{
	TeDatabase*	db = layer->database();
	TeDatabasePortal*	portal = db->getPortal();

	string sel = "SELECT geom_table FROM te_representation";
	sel += " WHERE layer_id = " + Te2String(layer->id());
	sel += " AND geom_type = " + Te2String(TeTEXT);

	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			string s = portal->getData(0);
			string drop = "DROP TABLE " + s + "_txvisual";
			if(db->tableExist(s + "_txvisual"))
			{
				if(db->execute(drop) == false)
				{
					delete portal;
					return false;	
				}
			}
		}
	}
	delete portal;
	return true;
}


string deletePointedObjects(TeTheme* theme, TeBox& box)
{
	unsigned int i, j;
	TeLayer* layer = theme->layer();		
	TeDatabase* db = layer->database();
	TeThemeMap&	themeMap = db->themeMap();
	string CT = theme->collectionTable();
	string CAT = theme->collectionAuxTable();
	string tsel, sel, del;
	vector<TeTable> table;
	vector<TeTable> ttable;
	vector<string> selVec;
	vector<string> selAuxVec;

	TeAttrTableVector layerTablesVector;
	layer->getAttrTables(layerTablesVector);

// delete temporal instances of TeFixedGeomDynAttr tables
	for(i=0; i<layerTablesVector.size(); i++)
	{
		TeTable t = layerTablesVector[i];
		TeAttrTableType type = t.tableType();

		if(type == TeFixedGeomDynAttr)
			ttable.push_back(t);
	}

	if(ttable.size())
	{
		tsel = "(SELECT aux0 FROM " + CAT;
		tsel += " WHERE grid_status = 1 OR grid_status = 3)";

		for(i=0; i<ttable.size(); i++)
		{
			del = "DELETE FROM " + ttable[i].name() + " WHERE " + ttable[i].uniqueName() + " IN " + tsel;
			if(db->execute(del) == false)
				return db->errorMessage();
		}
	}

// get selected object ids
	TeDatabasePortal* portal = db->getPortal();
	vector<string> oidVec;
	sel = "SELECT DISTINCT object_id FROM " + CAT;
	sel += " WHERE (grid_status = 1 OR grid_status = 3) AND object_id NOT IN";
	sel += "(SELECT DISTINCT object_id FROM " + CAT;
	sel += " WHERE grid_status = 0 OR grid_status = 2)";
	portal->freeResult();
	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			string oid = portal->getData(0);
			oidVec.push_back(oid);
		}
	}

	sel = "SELECT * FROM " + CT + " as T LEFT JOIN " + CAT + " as TT";
	sel += " ON T.c_object_id = TT.object_id WHERE TT.grid_status is null";
	sel += " AND (T.c_object_status = 1 OR T.c_object_status = 3)";
	portal->freeResult();
	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			string oid = portal->getData(0);
			oidVec.push_back(oid);
		}
	}
	vector<string>::iterator itB = oidVec.begin();
	vector<string>::iterator itE = oidVec.end();
	selVec = generateInClauses(itB,itE, db, true);

// delete attributes of TeAttrStatic tables and TeAttrEvent tables
	for(i=0; i<layerTablesVector.size(); i++)
	{
		TeTable t = layerTablesVector[i];
		TeAttrTableType type = t.tableType();

		if(type == TeAttrStatic || type == TeAttrEvent)
			table.push_back(t);
	}

	for(i=0; i<table.size(); i++)
	{
		for(j=0; j<selVec.size(); j++)
		{
			del = "DELETE FROM " + table[i].name() + " WHERE ";
			del += table[i].linkName() + " IN " + selVec[j];
			if(db->execute(del) == false)
			{
				delete portal;
				return db->errorMessage();
			}
		}
	}

	double minX = TeMAXFLOAT, minY = TeMAXFLOAT, maxX = -TeMAXFLOAT, maxY = -TeMAXFLOAT;
// delete selected geometries
	if(layer->geomRep() & TePOLYGONS)
	{
		string geo = layer->tableName(TePOLYGONS);
		for(i=0; i<selVec.size(); i++)
		{
			string boxsel = "SELECT * FROM " + geo;
			boxsel += " WHERE object_id IN " + selVec[i];

			portal->freeResult();
			if(portal->query(boxsel))
			{
				while(portal->fetchRow())
				{
					double x1 = portal->getDouble("lower_x");
					double y1 = portal->getDouble("lower_y");
					double x2 = portal->getDouble("upper_x");
					double y2 = portal->getDouble("upper_y");
					TeBox b(x1, y1, x2, y2);
					box = TeUnion(box, b);
				}
			}
			del = "DELETE FROM " + geo + " WHERE object_id IN " + selVec[i];
			if(db->execute(del) == false)
			{
				delete portal;
				return db->errorMessage();
			}
		}
		string bb = "SELECT MIN(lower_x), MIN(lower_y), MAX(upper_x), MAX(upper_y) FROM " + geo;
		portal->freeResult();
		if(portal->query(bb) && portal->fetchRow())
		{
			minX = MIN(minX, portal->getDouble(0));
			minY = MIN(minY, portal->getDouble(1));
			maxX = MAX(maxX, portal->getDouble(2));
			maxY = MAX(maxY, portal->getDouble(3));
		}
	}
	if(layer->geomRep() & TePOINTS)
	{
		string geo = layer->tableName(TePOINTS);
		for(i=0; i<selVec.size(); i++)
		{
			string boxsel = "SELECT * FROM " + geo;
			boxsel += " WHERE object_id IN " + selVec[i];

			portal->freeResult();
			if(portal->query(boxsel))
			{
				while(portal->fetchRow())
				{
					double x = portal->getDouble("x");
					double y = portal->getDouble("y");
					TeBox b(x, y, x, y);
					box = TeUnion(box, b);
				}
			}
			del = "DELETE FROM " + geo + " WHERE object_id IN " + selVec[i];
			if(db->execute(del) == false)
			{
				delete portal;
				return db->errorMessage();
			}
		}
		string bb = "SELECT MIN(x), MIN(y), MAX(x), MAX(y) FROM " + geo;
		portal->freeResult();
		if(portal->query(bb) && portal->fetchRow())
		{
			minX = MIN(minX, portal->getDouble(0));
			minY = MIN(minY, portal->getDouble(1));
			maxX = MAX(maxX, portal->getDouble(2));
			maxY = MAX(maxY, portal->getDouble(3));
		}
	}
	if(layer->geomRep() & TeCELLS)
	{
		string geo = layer->tableName(TeCELLS);
		for(i=0; i<selVec.size(); i++)
		{
			string boxsel = "SELECT * FROM " + geo;
			boxsel += " WHERE object_id IN " + selVec[i];

			portal->freeResult();
			if(portal->query(boxsel))
			{
				while(portal->fetchRow())
				{
					double x1 = portal->getDouble("lower_x");
					double y1 = portal->getDouble("lower_y");
					double x2 = portal->getDouble("upper_x");
					double y2 = portal->getDouble("upper_y");
					TeBox b(x1, y1, x2, y2);
					box = TeUnion(box, b);
				}
			}
			del = "DELETE FROM " + geo + " WHERE object_id IN " + selVec[i];
			if(db->execute(del) == false)
			{
				delete portal;
				return db->errorMessage();
			}
		}
		string bb = "SELECT MIN(lower_x), MIN(lower_y), MAX(upper_x), MAX(upper_y) FROM " + geo;
		portal->freeResult();
		if(portal->query(bb) && portal->fetchRow())
		{
			minX = MIN(minX, portal->getDouble(0));
			minY = MIN(minY, portal->getDouble(1));
			maxX = MAX(maxX, portal->getDouble(2));
			maxY = MAX(maxY, portal->getDouble(3));
		}
	}
	if(layer->geomRep() & TeLINES)
	{
		string geo = layer->tableName(TeLINES);
		for(i=0; i<selVec.size(); i++)
		{
			string boxsel = "SELECT * FROM " + geo;
			boxsel += " WHERE object_id IN " + selVec[i];

			portal->freeResult();
			if(portal->query(boxsel))
			{
				while(portal->fetchRow())
				{
					double x1 = portal->getDouble("lower_x");
					double y1 = portal->getDouble("lower_y");
					double x2 = portal->getDouble("upper_x");
					double y2 = portal->getDouble("upper_y");
					TeBox b(x1, y1, x2, y2);
					box = TeUnion(box, b);
				}
			}
			del = "DELETE FROM " + geo + " WHERE object_id IN " + selVec[i];
			if(db->execute(del) == false)
			{
				delete portal;
				return db->errorMessage();
			}
		}
		string bb = "SELECT MIN(lower_x), MIN(lower_y), MAX(upper_x), MAX(upper_y) FROM " + geo;
		portal->freeResult();
		if(portal->query(bb) && portal->fetchRow())
		{
			minX = MIN(minX, portal->getDouble(0));
			minY = MIN(minY, portal->getDouble(1));
			maxX = MAX(maxX, portal->getDouble(2));
			maxY = MAX(maxY, portal->getDouble(3));
		}
	}

	double dx = maxX - minX;
	double dy = maxY - minY;
	minX -= dx/20.;
	maxX += dx/20.;
	minY -= dy/20.;
	maxY += dy/20.;

	string up = "UPDATE te_layer SET lower_x = " + Te2String(minX);
	up += ", lower_y = " + Te2String(minY);
	up += ", upper_x = " + Te2String(maxX);
	up += ", upper_y = " + Te2String(maxY);
	up += " WHERE layer_id = " + Te2String(layer->id());
	if(db->execute(up) == false)
	{
		delete portal;
		return db->errorMessage();
	}
	TeBox nbox(minX, minY, maxX, maxY);
	layer->setLayerBox(nbox);

	if(!selVec.empty())
	{
		if(!layer->updateLayerEditionTime())
		{
		}
	}

// get collection names of all themes that use this layer
	vector<string> CATFVec;
	vector<string> CATVec;
	vector<string> CTVec;
	//string ss = "SELECT theme_id FROM te_theme WHERE layer_id = " + Te2String(layer->id());
	string ss = "SELECT te_theme.theme_id, te_theme.name, te_view.user_name ";
	ss += " FROM te_theme INNER JOIN te_view ON te_theme.view_id = te_view.view_id";
	ss += " WHERE layer_id = " + Te2String(layer->id());

	string originalUser = db->user();

	portal->freeResult();
	if(portal->query(ss))
	{
		while(portal->fetchRow())
		{
			int id = atoi(portal->getData(0));
			std::string themeName = string(portal->getData(1));
			std::string userName = string(portal->getData(2));

			if(id != theme->id())
			{
				bool diffUser = false;
				TeTheme* t = NULL;

				TeThemeMap::iterator itTheme = themeMap.find(id);
				if(itTheme != themeMap.end())
				{
					t = (TeTheme*)itTheme->second;
				}
				else
				{
					db->user(userName);
					t = new TeTheme(themeName);
					if(!db->loadTheme(t))
					{
						delete portal;
						return db->errorMessage();
					}

					diffUser = true;
				}

				TeAttrTableVector& tableVec = t->attrTables();
				for(i=0; i<tableVec.size(); i++)
				{
					string s = t->collectionAuxTable();
					if(tableVec[i].tableType() == TeFixedGeomDynAttr)
						CATFVec.push_back(s);
					else
						CATVec.push_back(s);
				}
				string s = t->collectionTable();
				CTVec.push_back(s);

				if(diffUser)
				{
					TeThemeMap::iterator itTheme = themeMap.find(id);
					if(itTheme != themeMap.end())
					{
						themeMap.erase(itTheme);
					}
					delete t;
				}
			}
		}
	}
	delete portal;

	db->user(originalUser);

// delete selected objects of the collections of other themes
	for(i=0; i<CTVec.size(); i++)
	{
		for(j=0; j<selVec.size(); j++)
		{
			del = "DELETE FROM " + CTVec[i] + " WHERE c_object_id IN " + selVec[j];
			if(db->execute(del) == false)
				return db->errorMessage();
		}
	}

// delete selected objects of the collection this theme
	for(i=0; i<selVec.size(); i++)
	{
		del = "DELETE FROM " + CT + " WHERE c_object_id IN " + selVec[i];
		if(db->execute(del) == false)
			return db->errorMessage();
	}

	up = "UPDATE " + CT + " SET c_object_status = c_object_status - 1 WHERE (c_object_status = 1 OR c_object_status = 3)";
	if(db->execute(up) == false)
		return db->errorMessage();

// delete selected instances of the auxiliar collection of other themes (only TeFixedGeomDynAttr)
	if(ttable.size())
	{
		for(i=0; i<CATFVec.size(); i++)
		{
			del = "DELETE FROM " + CATFVec[i] + " WHERE aux0 IN " + tsel;
			if(db->execute(del) == false)
				return db->errorMessage();
		}
	}

// delete selected instances of the auxiliar collection of other themes (TeAttrStatic and TeAttrEvent)
	for(i=0; i<CATVec.size(); i++)
	{
		for(j=0; j<selVec.size(); j++)
		{
			del = "DELETE FROM " + CATVec[i] + " WHERE object_id IN " + selVec[j];
			if(db->execute(del) == false)
				return db->errorMessage();
		}
	}

// delete selected instances of the auxiliar collection this theme
	del = "DELETE FROM " + CAT + " WHERE grid_status = 1 OR grid_status = 3";
	if(db->execute(del) == false)
		return db->errorMessage();

	return "";
}


string insertObjectIntoCollection(TeTheme* theme, TeCoord2D p, string newId, string newTId)
{
	string erro;
	string sel;
	TeLayer* layer = theme->layer();
	TeDatabase* db = layer->database();
	string CT = theme->collectionTable();
	string CAT = theme->collectionAuxTable();

	TeDatabasePortal* portal = db->getPortal();
	if(newId.empty() == false)
	{
		sel = "SELECT * FROM " + CT + " WHERE c_object_id = '" + newId + "'";
		if(portal->query(sel) && portal->fetchRow())
		{
//			erro = "There is already the object identifier:" + newId;
//			erro += " in the collection table:" + CT;
			delete portal;
			return "There is already";
		}
	}

//	if(newTId.empty() == false)
//	{
//		sel = "SELECT * FROM " + CAT + " WHERE aux0 = '" + newTId + "'";
//		portal->freeResult();
//		if(portal->query(sel) && portal->fetchRow())
//		{
//			erro = "There is already the object identifier:" + newTId;
//			erro += " in the collection auxliar table:" + CAT;
//			delete portal;
//			return erro;
//		}
//	}
	delete portal;

	unsigned int i;
	vector<TeTable> staticTableVec, eventTableVec, temporalTableVec;
	TeAttrTableVector layerTablesVector;
	if(layer->getAttrTables(layerTablesVector))
	{
		for(i=0; i<layerTablesVector.size(); i++)
		{
			TeTable t = layerTablesVector[i];
			if(t.tableType() == TeAttrStatic)
				staticTableVec.push_back(t);
			else if(t.tableType() == TeAttrEvent)
				eventTableVec.push_back(t);
			else if(t.tableType() == TeFixedGeomDynAttr)
				temporalTableVec.push_back(t);
		}
	}
	else
	{
		erro = "Fail in the layer method to gets attribute table associated";
		return erro;
	}

	bool ret;
	if(newId.empty() == false)
	{
		string ins = "INSERT INTO " + CT + " (c_object_id, c_legend_id,";
		ins += " label_x, label_y, c_legend_own, c_object_status) VALUES (";
		ins += "'" + newId + "', 0, " + Te2String(p.x()) + ", " + Te2String(p.y());
		ins += ", 0, 1)";
		ret = db->execute(ins);
		if(ret == false)
			return db->errorMessage();

		for(i=0; i<staticTableVec.size(); i++)
		{
			TeTable t = staticTableVec[i];
			ins = "INSERT INTO " + t.name() + " (" + t.linkName() + ")";
			ins += " VALUES ('" + newId + "')";
			ret = db->execute(ins);
			if(ret == false)
				return db->errorMessage();
		}
		for(i=0; i<eventTableVec.size(); i++)
		{
			TeTable t = eventTableVec[i];
			ins = "INSERT INTO " + t.name() + " (" + t.linkName() + ")";
			ins += " VALUES ('" + newId + "')";
			ret = db->execute(ins);
			if(ret == false)
				return db->errorMessage();
		}
	}

	string uniqueId = newId;
	if(newTId.empty() == false)
	{
		uniqueId = newId + newTId;
		string ins = "INSERT INTO " + CAT + " (object_id, aux0, unique_id, grid_status) VALUES (";
		ins += "'" + newId + "', '" + newTId + "', " + uniqueId + ", 1)";
		ret = db->execute(ins);
		if(ret == false)
			return db->errorMessage();

		for(i=0; i<temporalTableVec.size(); i++)
		{
			TeTable t = temporalTableVec[i];
			ins = "INSERT INTO " + t.name() + " (" + t.linkName() + ", " + t.uniqueName() + ")";
			ins += " VALUES ('" + newId + "', '" + newTId + "')";
			ret = db->execute(ins);
			if(ret == false)
				return db->errorMessage();
		}
	}
	else
	{
		string ins = "INSERT INTO " + CAT + " (object_id, grid_status) VALUES (";
		ins += "'" + newId + "', 1)";
		ret = db->execute(ins);
		if(ret == false)
			return db->errorMessage();
	}

	return "";
}


bool createGPMMetadataTable(TeDatabase* db, const bool& integrity)
{
	if (!db)
		return false;
	
	string table = "te_gpm";
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeINT;
	at1.rep_.name_ = "gpm_id";
	at1.rep_.isPrimaryKey_ = true;
	at1.rep_.isAutoNumber_ = true;
	attList.push_back(at1);
	
	TeAttribute at2;
	at2.rep_.type_ = TeINT;
	at2.rep_.name_ = "gpm_default";
	attList.push_back(at2);	
	
	TeAttribute at3;
	at3.rep_.type_ = TeINT;
	at3.rep_.name_ = "theme_id1";
	attList.push_back(at3);	

	TeAttribute at31;
	at31.rep_.type_ = TeINT;
	at31.rep_.name_ = "geom_rep1";
	attList.push_back(at31);	

	TeAttribute at32;
	at32.rep_.type_ = TeINT;
	at32.rep_.name_ = "theme_id2";
	attList.push_back(at32);	

	TeAttribute at33;
	at33.rep_.type_ = TeINT;
	at33.rep_.name_ = "geom_rep2";
	attList.push_back(at33);	

	TeAttribute at4;
	at4.rep_.type_ = TeSTRING;
	at4.rep_.numChar_ = 50;
	at4.rep_.name_ = "neighbours_table";
	attList.push_back(at4);	

	TeAttribute at5;
	at5.rep_.type_ = TeDATETIME;
	at5.rep_.name_ = "construction_date";
	attList.push_back(at5);	

	TeAttribute at6;
	at6.rep_.type_ = TeINT;
	at6.rep_.name_ = "construction_strategy";
	attList.push_back(at6);	

	TeAttribute at7;
	at7.rep_.type_ = TeREAL;
	at7.rep_.name_ = "const_max_distance";
	attList.push_back(at7);	

	TeAttribute at61;
	at61.rep_.type_ = TeINT;
	at61.rep_.name_ = "const_num_neighbours";
	attList.push_back(at61);	

	TeAttribute at8;
	at8.rep_.type_ = TeREAL;
	at8.rep_.name_ = "const_max_net_distance";
	attList.push_back(at8);	

	TeAttribute at81;
	at81.rep_.type_ = TeREAL;
	at81.rep_.name_ = "const_max_con_distance";
	attList.push_back(at81);	

	TeAttribute at9;
	at9.rep_.type_ = TeINT;
	at9.rep_.name_ = "slicing_strategy";
	attList.push_back(at9);	

	TeAttribute at91;
	at91.rep_.type_ = TeREAL;
	at91.rep_.name_ = "slicing_zone_dist";
	attList.push_back(at91);	

	TeAttribute at92;
	at92.rep_.type_ = TeINT;
	at92.rep_.name_ = "slicing_zone_local";
	attList.push_back(at92);	

	TeAttribute at10;
	at10.rep_.type_ = TeINT;
	at10.rep_.name_ = "weight_strategy";
	attList.push_back(at10);	

	TeAttribute at11;
	at11.rep_.type_ = TeINT;
	at11.rep_.name_ = "weight_norm";
	attList.push_back(at11);	

	TeAttribute at12;
	at12.rep_.type_ = TeREAL;
	at12.rep_.name_ = "weight_a";
	attList.push_back(at12);	

	TeAttribute at13;
	at13.rep_.type_ = TeREAL;
	at13.rep_.name_ = "weight_b";
	attList.push_back(at13);	

	TeAttribute at14;
	at14.rep_.type_ = TeREAL;
	at14.rep_.name_ = "weight_c";
	attList.push_back(at14);	

	TeAttribute at15;
	at15.rep_.type_ = TeREAL;
	at15.rep_.name_ = "weight_factor";
	attList.push_back(at15);	

	TeAttribute at16;
	at16.rep_.type_ = TeREAL;
	at16.rep_.name_ = "weight_dist_raio";
	attList.push_back(at16);	

	TeAttribute at17;
	at17.rep_.type_ = TeREAL;
	at17.rep_.name_ = "weigth_max_local_dist";
	attList.push_back(at17);
	 
	bool status = db->createTable(table,attList);
	attList.clear();
	
	//create integrity
	if(status && integrity)
	{
		if (!db->createRelation("fk_gpm_theme1_id", "te_gpm", "theme_id1", "te_theme", "theme_id", true))
			return false;

		if (!db->createRelation("fk_gpm_theme2_id", "te_gpm", "theme_id2", "te_theme", "theme_id", true))
			return false;
	}

	//create index
	if(status)
	{
		db->createIndex("te_gpm", "idx_gpm_theme_id1", "theme_id1");
	}

	return status;
}

bool createNeighbourhoodTable(TeDatabase* db, const string& tableName)
{
	if (!db)
		return false;
	
	TeAttributeList attList;

	TeAttribute at1;
	at1.rep_.type_ = TeINT;
	at1.rep_.name_ = "neighbourhood_id";
	at1.rep_.isPrimaryKey_ = true;
	at1.rep_.isAutoNumber_ = true;
	attList.push_back(at1);
	
	TeAttribute at2;
	at2.rep_.type_ = TeSTRING;
	at2.rep_.numChar_=50;
	at2.rep_.name_ = "object_id1";
	attList.push_back(at2);	
	
	TeAttribute at3;
	at3.rep_.type_ = TeSTRING;
	at3.rep_.numChar_=50;
	at3.rep_.name_ = "object_id2";
	attList.push_back(at3);	

	TeAttribute at31;
	at31.rep_.type_ = TeREAL;
	at31.rep_.name_ = "centroid_distance";
	attList.push_back(at31);	

	TeAttribute at32;
	at32.rep_.type_ = TeREAL;
	at32.rep_.name_ = "weight";
	attList.push_back(at32);	

	TeAttribute at33;
	at33.rep_.type_ = TeINT;
	at33.rep_.name_ = "slice";
	attList.push_back(at33);	

	TeAttribute at4;
	at4.rep_.type_ = TeINT;
	at4.rep_.name_ = "order_neig";
	attList.push_back(at4);	

	TeAttribute at6;
	at6.rep_.type_ = TeREAL;
	at6.rep_.name_ = "borders_length";
	attList.push_back(at6);	

	TeAttribute at7;
	at7.rep_.type_ = TeREAL;
	at7.rep_.name_ = "net_objects_distance";
	attList.push_back(at7);	

	TeAttribute at61;
	at61.rep_.type_ = TeREAL;
	at61.rep_.name_ = "net_minimum_path";
	attList.push_back(at61);	

	bool status = db->createTable(tableName,attList);
	attList.clear();
	
	//create index
	if(status)
		db->createIndex(tableName, "idx_"+tableName+"_objectId1", "object_id1");
	
	return status;
}

//! Delete the generalized proximity matrix of a theme - neighbourhoodTable and metadata
bool deleteGPMTable(TeDatabase* db, const int& themeId, const int& gpmId)
{
	if(!db)
	{
		return false;
	}

	if(!db->tableExist("te_gpm"))
	{
		return false;
	}

	string del = "SELECT neighbours_table, gpm_default FROM te_gpm ";
	del += " WHERE theme_id1 = "+ Te2String(themeId);
	if(gpmId>-1)
		del += " AND gpm_id = "+ Te2String(gpmId);

	TeDatabasePortal* portal = db->getPortal();
	if(!portal)
		return false;
	
	if(!portal->query(del))
	{
		delete portal;
		return false;
	}

	int isDefault = false;
	while(portal->fetchRow())
	{
		string tableName = portal->getData(0);
		isDefault = portal->getInt(1);
		string del2 = " DROP TABLE "+ tableName;
		db->execute(del2);
	}

	del = "DELETE FROM te_gpm ";
	del += " WHERE theme_id1 = "+ Te2String(themeId);
	if(gpmId>-1)
		del += " AND gpm_id = "+ Te2String(gpmId);
	
	db->execute(del);

	portal->freeResult();

	//if deleted matrix was default, select other matrix as default
	if(gpmId>-1 && (isDefault != 0))
	{
		del = " SELECT gpm_id FROM te_gpm ";
		del += " WHERE theme_id1 = "+ Te2String(themeId);
		if((!portal->query(del)) || (!portal->fetchRow()))
		{
			delete portal;
			return true;
		}

		int aux = portal->getInt(0);

		string up =  " UPDATE te_gpm SET gpm_default = 1 WHERE gpm_id = "+ Te2String(aux);
		db->execute(up); 
	}
	delete portal;
	return true;
}



bool loadAppTheme (TeDatabase* db, TeAppTheme *theme, const string& tableName)
{
	if (!db || !theme) 
		return false;

//	if (!db->loadTheme((TeTheme*)theme->getTheme()))
//		return false;

	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return false;

	string q = "SELECT * FROM " + tableName + " WHERE theme_id= ";
	q += Te2String (((TeTheme*)theme->getTheme())->id());

	if (!portal->query(q) || !portal->fetchRow())
	{	
		delete portal;
		return false;
	}
	theme->outOfCollectionVisibility(portal->getBool(1));
	theme->withoutDataConnectionVisibility(portal->getBool(2));
	theme->refineAttributeWhere(string(portal->getData (3)));
	theme->refineSpatialWhere(string(portal->getData (4)));
	theme->refineTemporalWhere(string(portal->getData (5)));

	string name = portal->getData(6);
	if (!name.empty())
		theme->groupColor(name);
	theme->groupColorDir(TeColorDirection(atoi(portal->getData(7))));
	theme->groupObjects(TeSelectedObjects(atoi(portal->getData(8))));
	theme->pieDiameter(portal->getDouble(9));
	theme->pieMaxDiameter(portal->getDouble(10));
	theme->pieMinDiameter(portal->getDouble(11));
	name = portal->getData(12);
	if (!name.empty())
		theme->pieDimAttribute(name);
	theme->barMaxHeight(portal->getDouble(13));
	theme->barMinHeight(portal->getDouble(14));
	theme->barWidth(portal->getDouble(15));
	theme->chartMaxVal(portal->getDouble(16));
	theme->chartMinVal(portal->getDouble(17));
	theme->chartType(TeChartType(atoi(portal->getData(18))));
	theme->chartObjects(TeSelectedObjects(atoi(portal->getData(19))));
	theme->keepDimension(atoi(portal->getData(20)));
	theme->showCanvasLegend(portal->getBool(21));
	TeCoord2D p;
	p.x(portal->getDouble(22));
	p.y(portal->getDouble(23));
	theme->canvasLegUpperLeft(p);
	bool n = portal->getBool(24);
	theme->mixColor(n);
	n = portal->getBool(25);
	theme->countObj(n);
	name = portal->getData(26);
	if (!name.empty())
		theme->textTable(name);
	theme->chartFunction(portal->getData(27));
	portal->freeResult();
	delete portal;
	return loadChartParams(db,theme);
}

bool insertAppTheme(TeDatabase* db, TeAppTheme *appTheme, const string& tableName)
{
	if (db == 0 || appTheme == 0) 
		return false;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();

	if (db->insertTheme(theme) == false)
		return false;

	string q = "INSERT INTO " + tableName + " (theme_id, visible_out_coll, visible_without_conn, ";
	q += "refine_attribute_where, refine_spatial_where, refine_temporal_where, ";
	q += "grouping_color, grouping_color_direction, grouping_objects, ";
	q += "pie_diameter, pie_max_diameter, pie_min_diameter, pie_dimension_attr, ";
	q += "bar_max_height, bar_min_height, bar_width, ";
	q += "chart_max_value, chart_min_value, chart_type, chart_objects, ";
	q += "keep_dimension, show_canvas_leg, canvas_leg_x, canvas_leg_y, ";
	q += "mix_color, count_obj, text_table, chart_function) VALUES (";

	q += Te2String(theme->id()) + ", ";
	if (appTheme->outOfCollectionVisibility())
		q += "1, ";
	else
		q += "0, ";
	if (appTheme->withoutDataConnectionVisibility())
		q += "1, '";
	else
		q += "0, '";
	q += appTheme->refineAttributeWhere () + "', '" + appTheme->refineSpatialWhere() + "', '";
	q += appTheme->refineTemporalWhere () + "', '" + appTheme->groupColor() + "', ";
	q += Te2String(appTheme->groupColorDir()) + ", " + Te2String(appTheme->groupObjects()) + ", ";
	q += Te2String(appTheme->pieDiameter(),8) + ", " + Te2String(appTheme->pieMaxDiameter(),8) + ", ";
	q += Te2String(appTheme->pieMinDiameter(),8) + ", '" + appTheme->pieDimAttribute() + "', ";
	q += Te2String(appTheme->barMaxHeight(),8) + ", " + Te2String(appTheme->barMinHeight(),8) + ", ";
	q += Te2String(appTheme->barWidth(),8) + ", " + Te2String(appTheme->chartMaxVal(),8) + ", ";
	q += Te2String(appTheme->chartMinVal(),8)+ ", " + Te2String(appTheme->chartType()) + ", ";
	q += Te2String(appTheme->chartObjects()) + ", "; 
	q += Te2String(appTheme->keepDimension()) + ", ";

	if (appTheme->showCanvasLegend())
		q += "1, ";
	else
		q += "0, ";

	q += Te2String(appTheme->canvasLegWidth(),8) + ", ";
	q += Te2String(appTheme->canvasLegHeight(),8) + ", ";

	if (appTheme->mixColor())
		q += "1, ";
	else
		q += "0, ";

	if (appTheme->countObj())
		q += "1, '";
	else
		q += "0, '";
	q += appTheme->textTable() + "', '";
	q += appTheme->chartFunction() + "')";

	if(!db->execute(q))
		return false;

	for (unsigned int i=0; i<appTheme->chartAttributes_.size(); i++)
	{
		q = "INSERT INTO te_chart_params VALUES (attr_name, red, blue, green, theme_id) VALUES (";
		string attname = appTheme->chartAttributes_[i];
		map<int, map<string, string> > mapTAA = db->mapThemeAlias();
		map<int, map<string, string> >::iterator tit = mapTAA.find(theme->id());
		if(tit != mapTAA.end())
		{
			map<string, string> mapAA = tit->second;
			appTheme->concatTableName(attname);
			map<string, string>::iterator it = mapAA.find(attname);
			if(it != mapAA.end())
			{
				string alias = it->second;
				attname += "(" + alias + ")";
			}
		}
		q += "'" + attname + "', " + Te2String(appTheme->chartColors_[i].red_)+ ", ";
		q += Te2String(appTheme->chartColors_[i].green_)+ ", " + Te2String(appTheme->chartColors_[i].blue_) +", ";
		q += Te2String(theme->id()) + ")";
		if (!db->execute(q))
			return false;
	}

    return true; 
}

bool updateAppTheme (TeDatabase* db, TeAppTheme* appTheme, const string& tableName)
{
	if (db == 0 || appTheme == 0)
		return false;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();
	if (db->updateTheme(theme) == false) 
		return false;

	if (theme->id() <= 0 )  // theme doesn´t exist in the database yet
		return insertAppTheme (db, appTheme, tableName);

	// update theme metadata
	string sql = "UPDATE " + tableName + " SET ";
	sql += "  visible_out_coll=";
	if (appTheme->outOfCollectionVisibility())
		sql += "1";
	else 
		sql += "0";
	sql += ", visible_without_conn=";
	if (appTheme->withoutDataConnectionVisibility())
		sql += "1";
	else 
		sql += "0";

	sql += ", refine_attribute_where='" + appTheme->refineAttributeWhere()+"'";
	sql += ", refine_spatial_where='" + appTheme->refineSpatialWhere()+"'";
	sql += ", refine_temporal_where='" + appTheme->refineTemporalWhere()+"'";
	sql += ", grouping_color='" + appTheme->groupColor() + "'";
	sql += ", grouping_color_direction=" + Te2String(appTheme->groupColorDir());
	sql += ", grouping_objects=" + Te2String (appTheme->groupObjects());
	sql += ", pie_diameter=" + Te2String(appTheme->pieDiameter());
	sql += ", pie_max_diameter=" + Te2String(appTheme->pieMaxDiameter());
	sql += ", pie_min_diameter=" + Te2String(appTheme->pieMinDiameter());
	sql += ", pie_dimension_attr='" + appTheme->pieDimAttribute()+"'";
	sql += ", bar_max_height=" + Te2String(appTheme->barMaxHeight());
	sql += ", bar_min_height=" + Te2String(appTheme->barMinHeight());
	sql += ", bar_width=" + Te2String(appTheme->barWidth());
	sql += ", chart_max_value=" + Te2String(appTheme->chartMaxVal());
	sql += ", chart_min_value=" + Te2String(appTheme->chartMinVal());
	sql += ", chart_type=" + Te2String (appTheme->chartType());
	sql += ", chart_objects=" + Te2String (appTheme->chartObjects());
	sql += ", keep_dimension=" + Te2String(appTheme->keepDimension());
	sql += ", show_canvas_leg=";
	if (appTheme->showCanvasLegend())
		sql += "1";
	else 
		sql += "0";
	sql += ", canvas_leg_x=" + Te2String(appTheme->canvasLegUpperLeft().x());
	sql += ", canvas_leg_y=" + Te2String(appTheme->canvasLegUpperLeft().y());
	sql += ", mix_color=";
	if (appTheme->mixColor())
		sql += "1";
	else 
		sql += "0";
	sql += ", count_obj=";
	if (appTheme->countObj())
		sql += "1";
	else 
		sql += "0";
	sql += ", text_table='" + appTheme->textTable () + "'";
	sql += ", chart_function='" + appTheme->chartFunction () + "'";
	sql += " WHERE theme_id=" + Te2String (theme->id());

	if (!db->execute (sql))
		return false;
         
	// remove it´s chart parameters and reinsert them
	deleteChartParams(db, appTheme);
	if(!insertChartParams (db, appTheme))
		return false;

	return true;
}


bool insertChartParams(TeDatabase* db, TeAppTheme *theme)
{
	if (!db || !theme) 
		return false;

	unsigned int n = theme->chartColors_.size();
	int themeId = ((TeTheme*)theme->getTheme())->id();
	TeColor cor;
	string ins, insert;
	for (unsigned int i=0; i<n; i++)
	{
		cor = theme->chartColors_[i];
		ins = "INSERT INTO te_chart_params (theme_id, attr_name, red, green, blue)";
		insert = ins + " VALUES (" + Te2String(themeId) +",'";
		string attname = theme->chartAttributes_[i];
		map<int, map<string, string> > mapTAA = db->mapThemeAlias();
		map<int, map<string, string> >::iterator tit = mapTAA.find(((TeTheme*)theme->getTheme())->id());
		if(tit != mapTAA.end())
		{
			map<string, string> mapAA = tit->second;
			theme->concatTableName(attname);
			map<string, string>::iterator it = mapAA.find(attname);
			if(it != mapAA.end())
			{
				string alias = it->second;
				attname += "(" + alias + ")";
			}
		}
		insert += attname +"',";
		insert += Te2String(cor.red_) + ",";
		insert += Te2String(cor.green_) + ",";
		insert += Te2String(cor.blue_) + ")";
		if (!db->execute(insert))
			return false;
	}
	return true;
}


bool deleteChartParams (TeDatabase* db, TeAppTheme* theme)
{
	if (!db || !theme) 
		return false;

	string id = Te2String(((TeTheme*)theme->getTheme())->id());
	string del = "DELETE FROM te_chart_params WHERE theme_id = " + id;
	return db->execute(del);
}


bool locatePieBar (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt, double delta) 
{
	if (!db || !theme) 
		return false;

	theme->chartSelected_.clear();
	string	TC = ((TeTheme*)theme->getTheme())->collectionTable();
	string xmin = Te2String(pt.x()-delta);
	string xmax = Te2String(pt.x()+delta);
	string ymin = Te2String(pt.y()-delta);
	string ymax = Te2String(pt.y()+delta);

	TeDatabasePortal* portal = db->getPortal();
	if (!portal)
		return false;

	string sel = "SELECT c_object_id FROM " + TC + " WHERE ";
	sel += "label_x > " + xmin + " AND ";
	sel += "label_x < " + xmax + " AND ";
	sel += "label_y > " + ymin + " AND ";
	sel += "label_y < " + ymax;

	if(!portal->query(sel))
	{
		delete portal;
		return false;
	}

	if(portal->fetchRow() == false)
	{
		delete portal;
		return false;
	}

	theme->chartSelected_ = portal->getData("c_object_id");

	delete portal;

	return true;
}


bool loadChartParams (TeDatabase* db, TeAppTheme *theme)
{
	if (!db || !theme) 
		return false;

	TeDatabasePortal* portal = db->getPortal();

	string q ="SELECT * FROM te_chart_params WHERE theme_id= ";
	q += Te2String (((TeTheme*)theme->getTheme())->id());

	if (!portal->query(q))
	{	
		delete portal;
		return false;
	}
	while (portal->fetchRow())
	{
		TeColor cor = portal->getColor ();
		string attname = portal->getData("attr_name");
		int f = attname.find("(");
		if(f >= 0)
		{
			string alias = attname;
			attname = attname.substr(0, f);
			alias = alias.substr(f+1);
			alias = alias.substr(0, alias.size()-1);
			map<string, string>& mapAA = db->mapThemeAlias()[((TeTheme*)theme->getTheme())->id()];
			theme->concatTableName(attname);
			mapAA[attname] = alias;
		}
		theme->chartAttributes_.push_back(attname);
		theme->chartColors_.push_back(cor);
	}
	delete portal;
	return true;
}

bool updatePieBarLocation (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt) 
{
	if (!db || !theme) 
		return false;

	if (theme->chartSelected_.empty() == true)
		return false;

	string	x = Te2String(pt.x());
	string	y = Te2String(pt.y());
	string	TC = ((TeTheme*)theme->getTheme())->collectionTable();
	string mover = "UPDATE " + TC + " SET label_x = " + x + ", label_y = " + y;
	mover += " WHERE c_object_id = '" + theme->chartSelected_ + "'";
	return db->execute(mover);
}

bool updatePointLocation (TeDatabase* db, TeAppTheme* theme, TeCoord2D pt) 
{
	if (!db || !theme) 
		return false;

	string	x = Te2String(pt.x());
	string	y = Te2String(pt.y());
	string	table = ((TeTheme*)theme->getTheme())->layer()->tableName(TePOINTS);
	string mover = "UPDATE " + table + " SET x = " + x + ", y = " + y;
	mover += " WHERE geom_id = " + Te2String(theme->tePoint_.geomId());
	return db->execute(mover);
}

bool generateCountLegends(TeAppTheme* appTheme, string table, string field)
{
	if(appTheme == 0)
		return false;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();

	appTheme->groupObjects(TeAll);
	TeAttributeRep rep;
	rep.name_ = table + "." + field;
	rep.type_ = TeINT;
	
	TeGrouping group;
	group.groupAttribute_ = rep;
	group.groupNormAttribute_ = "";
	group.groupMode_ = TeUniqueValue;
	group.groupPrecision_ = 0;
	theme->grouping(group);
	
	string	scor = "Cy-G-Y-Or-R";
	appTheme->groupColor(scor);
	appTheme->countObj(true);
	appTheme->mixColor(true);
	appTheme->groupColorDir(TeColorAscSatEnd);
	
	string	fieldId = theme->collectionTable() + ".c_object_id";
	TeLayer* layer = theme->layer();
	TeDatabase* db = layer->database();
    vector<TeSlice> result;
	vector<string> valVec;
	vector<string> idVec;

	string sel = "SELECT " + fieldId + ", " + field + theme->sqlFrom() + " ORDER BY " + field;

	TeDatabasePortal* portal = db->getPortal();
	if(portal->query(sel) == false)
	{
		delete portal;
		return false;
	}

	while(portal->fetchRow())
	{
		string objectId = portal->getData(0);
		string val = portal->getData(1);
		if (val.empty() == false)
		{
			valVec.push_back(val);
			idVec.push_back(objectId);
		}
	}
	delete portal;

	if(valVec.size()==0)
		return false;

	TeGroupByUniqueValue(valVec, TeINT, result, 0);
	theme->grouping().groupNumSlices_ = result.size();

	vector<string> colorRamps;
	colorRamps.push_back("CYAN");
	colorRamps.push_back("GREEN");
	colorRamps.push_back("YELLOW");
	colorRamps.push_back("ORANGE");
	colorRamps.push_back("RED");
	vector<TeColor> colorVec;
	getColors(colorRamps,10, colorVec);

	TeVisual visual;
	if((layer->geomRep() & TePOLYGONS) || (layer->geomRep() & TeCELLS))
		visual = *(theme->defaultLegend().visual(TePOLYGONS));
	else if((layer->geomRep() & TeLINES))
		visual = *(theme->defaultLegend().visual(TeLINES));
	else if((layer->geomRep() & TePOINTS))
		visual = *(theme->defaultLegend().visual(TePOINTS));

	TeLegendEntryVector legVec;
	unsigned int i, j = 0;
	for(i = 0; i < result.size(); i++)
	{
		TeColor cor = colorVec[i];
		visual.color(cor);

		TeLegendEntry leg;
		leg.from(result[i].from_);
		leg.to(result[i].to_);
		if((layer->geomRep() & TePOLYGONS) || (layer->geomRep() & TeCELLS))
			leg.setVisual(visual.copy(), TePOLYGONS);
		else if((layer->geomRep() & TeLINES))
			leg.setVisual(visual.copy(), TeLINES);
		else if((layer->geomRep() & TePOINTS))
			leg.setVisual(visual.copy(), TePOINTS);

		int count = 0;
		while(j < valVec.size())
		{
			if(valVec[j] == leg.from())
			{
				count++;
				j++;
			}
			else
				break;
		}
		leg.count(count);
		leg.group(i);
		leg.theme(theme->id());
		string label = leg.from();
		leg.label(label);
		legVec.push_back(leg);
	}

	colorRamps.clear();
	colorVec.clear();

	db->deleteLegend(theme->id());
	map<int, vector<string> > legMap;
	for(i=j=0; i<legVec.size(); i++)
	{
		theme->legend().push_back(legVec[i]);
		if (db->insertLegend(&(theme->legend()[i])) == false)
			return false;

		vector<string> ids;
		TeLegendEntry& leg = theme->legend()[i];
		while(j < valVec.size())
		{
			if(valVec[j] == leg.from())
			{
				ids.push_back(idVec[j]);
				j++;
			}
			else
				break;
		}
		legMap[leg.id()] = ids;
	}

	map<int, vector<string> > :: iterator it = legMap.begin();
	while(it != legMap.end())
	{
		vector<string>::iterator itB = it->second.begin();
		vector<string>::iterator itE = it->second.end();
		vector<string> svec = generateInClauses(itB,itE,db);				

		for(i=0; i<svec.size(); i++)
		{
			string up = "UPDATE " + theme->collectionTable() + " SET c_legend_id = " + Te2String(it->first);
			up += " WHERE c_object_id IN " + svec[i];

			if(db->execute(up) == false)
				return false;
		}
		it++;
	}

	if(theme->legend().size())
		theme->visibleRep(theme->visibleRep() | 0x40000000);
	else
		theme->visibleRep(theme->visibleRep() & 0xbfffffff);

	if((db->updateGrouping (theme->id(), theme->grouping())) == false)
		return false;

	return(updateAppTheme(db, appTheme));
}

void changeObjectStatus(ObjectInfo& info, string& uniqueId, string mode, int pointed)
{
	if(uniqueId.empty())
	{
		if(mode == "pointing")
			info.status_ ^= 0x1; // toggle least significant bit
		else if(mode == "shiftAndPointing")
		{
			if(pointed == 1)
				info.status_ |= 0x1; // set least significant bit
			else
				info.status_ &= 0x2; // reset least significant bit
		}
		return;
	}

	int& gs = info.uniqueMap_[uniqueId];
	int	os;

	if(mode == "pointing")
	{
		gs ^= 0x1; // toggle least significant bit

		set<int> statusSet;
		map<string, int>::iterator it = info.uniqueMap_.begin();
		while(it != info.uniqueMap_.end())
		{
			statusSet.insert(it->second);
			it++;
		}
		if(statusSet.find(3) != statusSet.end())
			os = 3;
		else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) != statusSet.end())
			os = 3;
		else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) == statusSet.end())
			os = 2;
		else if(statusSet.find(2) == statusSet.end() && statusSet.find(1) != statusSet.end())
			os = 1;
		else
			os = 0;
	}
	else if(mode == "shiftAndPointing")
	{
		if(pointed == 1)
			gs |= 0x1; // set least significant bit
		else
			gs &= 0x2; // reset least significant bit

		set<int> statusSet;
		map<string, int>::iterator it = info.uniqueMap_.begin();
		while(it != info.uniqueMap_.end())
		{
			statusSet.insert(it->second);
			it++;
		}
		if(statusSet.find(3) != statusSet.end())
			os = 3;
		else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) != statusSet.end())
			os = 3;
		else if(statusSet.find(2) != statusSet.end() && statusSet.find(1) == statusSet.end())
			os = 2;
		else if(statusSet.find(2) == statusSet.end() && statusSet.find(1) != statusSet.end())
			os = 1;
		else
			os = 0;
	}

	info.status_ = os;
}

bool saveObjectStatus(TeAppTheme *appTheme, set<string>& objectSet, map<string, ObjectInfo>& objectMap)
{
	unsigned int i;
    int os, gs;
	string	q, oid, uid;

	TeAbstractTheme* theme = appTheme->getTheme();
	TeDatabase *db = appTheme->getLocalDatabase();
	string colTable = appTheme->getLocalCollectionTable();
	
	map<int, vector<string> > sameObjectStMap;
	map<int, vector<string> > sameGridStMap;

	set<string>::iterator setIt = objectSet.begin();
	while(setIt != objectSet.end())
	{
		oid = *setIt;
		ObjectInfo& info = objectMap[oid];
		os = info.status_;
		sameObjectStMap[os].push_back(oid);

		map<string, int>& gridMap = info.uniqueMap_;
		map<string, int>::iterator gridIt = gridMap.begin();
		while(gridIt != gridMap.end())
		{
			uid = gridIt->first;
			gs = gridIt->second;
			sameGridStMap[gs].push_back(uid);
			gridIt++;
		}
		setIt++;
	}

	string qUpdate = "UPDATE " + colTable;
	map<int, vector<string> >::iterator it;
	for (it = sameObjectStMap.begin(); it != sameObjectStMap.end(); ++it)
	{
		vector<string>::iterator itB = it->second.begin();
		vector<string>::iterator itE = it->second.end();
		vector<string> inClauseVector = generateInClauses(itB,itE,db);				
		for (i = 0; i < inClauseVector.size(); ++i)
		{
			q = qUpdate + " SET c_object_status = " + Te2String(it->first);
			q += " WHERE c_object_id IN " + inClauseVector[i];

			if (db->execute(q) == false)
				return false;
		}
		vector<string> itemVec;
		theme->setStatus(it->second, itemVec, it->first);
	}

	qUpdate = "UPDATE " + colTable + "_aux";
	for (it = sameGridStMap.begin(); it != sameGridStMap.end(); ++it)
	{
		vector<string>::iterator itB = it->second.begin();
		vector<string>::iterator itE = it->second.end();
		vector<string> inClauseVector = generateInClauses(itB,itE,db, false);				
		for (i = 0; i < inClauseVector.size(); ++i)
		{
			q = qUpdate + " SET grid_status = " + Te2String(it->first);
			q += " WHERE unique_id IN " + inClauseVector[i];

			if (db->execute(q) == false)
				return false;
		}
		vector<string> objVec;
		theme->setStatus(objVec, it->second, it->first);
	}

	return true;
}

TeStatisticType getStatisticFromString(const string& s)
{
	TeStatisticType sType;

	if(s == "TeSUM")
		sType = TeSUM;
	else if(s == "TeMAXVALUE")
		sType = TeMAXVALUE;
	else if(s == "TeMINVALUE")
		sType = TeMINVALUE;
	else if(s == "TeCOUNT")
		sType = TeCOUNT;
	else if(s == "TeVALIDCOUNT")
		sType = TeVALIDCOUNT;
	else if(s == "TeSTANDARDDEVIATION")
		sType = TeSTANDARDDEVIATION;
	else if(s == "TeKERNEL")
		sType = TeKERNEL;
	else if(s == "TeMEAN")
		sType = TeMEAN;
	else if(s == "TeVARIANCE")
		sType = TeVARIANCE;
	else if(s == "TeSKEWNESS")
		sType = TeSKEWNESS;
	else if(s == "TeKURTOSIS")
		sType = TeKURTOSIS;
	else if(s == "TeAMPLITUDE")
		sType = TeAMPLITUDE;
	else if(s == "TeMEDIAN")
		sType = TeMEDIAN;
	else if(s == "TeVARCOEFF")
		sType = TeVARCOEFF;
	else if(s == "TeMODE")
		sType = TeMODE;
	else if(s == "TeVALUE")
		sType = TeNOSTATISTIC;

	return sType;
}

void getVisual(TeAppTheme* appTheme, TeGeomRep geomRep,
			   map<string, TeVisual>& objVisualMap, set<string>& oidSet, bool allObjects)
{
	string oid;
	int status;

	TeLegendEntry* legEntry;
	TeVisual visual;

	map<string, int>::iterator mapIt;
	set<string>::iterator setIt;

	TeTheme* theme = (TeTheme*)appTheme->getTheme();
	TeDatabase* db = theme->layer()->database();
	map<int, TeLegendEntry*>& legMap = db->legendMap();

	map<string, int>& oidStatusMap = theme->getObjStatusMap();
	map<string, int>& objLegendMap = theme->getObjLegendMap();

	bool setGroupingVisual = false;
	if ((theme->grouping().groupMode_ != TeNoGrouping) && (theme->visibleRep() & 0x40000000))
		setGroupingVisual = true;

	// Get the objects that are not grouped
	// and set a visual for them
	set<string> oidNotGroupedSet;
	if (setGroupingVisual == true && (objLegendMap.size() != theme->getObjects().size()))
	{
		oidNotGroupedSet = theme->getObjects();
		map<string, int>::iterator mapIt;
		for (mapIt = objLegendMap.begin(); mapIt != objLegendMap.end(); ++mapIt)
			oidNotGroupedSet.erase(mapIt->first);
	}

	set<string> oidNullSet;
	if (allObjects == true)
	{
		if (setGroupingVisual == true)
		{
			for (mapIt = objLegendMap.begin(); mapIt != objLegendMap.end(); ++mapIt)
			{
				oid = mapIt->first;
				legEntry = legMap[mapIt->second];

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;
			}
		}
		else
		{
			for (setIt = oidSet.begin(); setIt != oidSet.end(); ++setIt)
			{
				oid = *setIt;
				legEntry = &(theme->defaultLegend());

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;
			}

			map<string, int>& objOwnLegMap = theme->getObjOwnLegendMap();
			for (mapIt = objOwnLegMap.begin(); mapIt != objOwnLegMap.end(); ++mapIt)
			{
				oid = mapIt->first;
				legEntry = legMap[mapIt->second];

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;
			}
		}
	}
	else if (allObjects == false)
	{
		if (setGroupingVisual == true)
		{
			for (setIt = oidSet.begin(); setIt != oidSet.end(); ++setIt)
			{
				oid = *setIt;
				if (objLegendMap.find(oid) != objLegendMap.end())
					legEntry = legMap[objLegendMap[oid]];
				else
					legEntry = &(theme->defaultLegend());

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;

				if (legEntry->from() == "Missing Data")
					oidNullSet.insert(oid);
			}
		}
		else
		{
			for (setIt = oidSet.begin(); setIt != oidSet.end(); ++setIt)
			{
				oid = *setIt;
				legEntry = &(theme->defaultLegend());

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;
			}
		}

		if (setGroupingVisual == false)
		{
			map<string, int>& objOwnLegMap = theme->getObjOwnLegendMap();
			for (mapIt = objOwnLegMap.begin(); mapIt != objOwnLegMap.end(); ++mapIt)
			{
				oid = mapIt->first;
				legEntry = legMap[mapIt->second];

				TeGeomRepVisualMap& vm = legEntry->getVisualMap();
				if(legEntry && vm.find(geomRep) != vm.end())
					visual = *(legEntry->visual(geomRep));

				objVisualMap[oid] = visual;
			}
		}
	}

	// Set the color of the objects according their status
	map<string, TeVisual>::iterator it;
	for (it = objVisualMap.begin(); it != objVisualMap.end(); ++it)
	{
		oid = it->first;
		TeVisual& visual = it->second;
		status = oidStatusMap[oid];

		if (status == 0)
		{
//			TeColor cor;
//			cor = theme->defaultLegend().visual(TePOLYGONS)->color();
//			visual->color(theme->defaultLegend().visual(TePOLYGONS)->color());
			oidStatusMap.erase(oid);
		}

		if(status == 1)
			visual.color(theme->pointingLegend().visual(TePOLYGONS)->color());
		else if(status == 2)
			visual.color(theme->queryLegend().visual(TePOLYGONS)->color());
		else if(status == 3)
			visual.color(theme->queryAndPointingLegend().visual(TePOLYGONS)->color());

//		objVisualMap[oid] = visual;
	}

	if (oidNullSet.empty() == false)
	{
		for (setIt = oidNullSet.begin(); setIt != oidNullSet.end(); ++setIt)
		{
			oid = *setIt;		
			status = oidStatusMap[oid];
			legEntry = &(theme->defaultLegend());
			TeGeomRepVisualMap& geomRepVisualMap = legEntry->getVisualMap();
			if(geomRepVisualMap.find(geomRep) != geomRepVisualMap.end())
				visual = *(legEntry->visual(geomRep));

			objVisualMap[oid] = visual;

			TeVisual& objVisual = objVisualMap[oid];
			if (status == 0)
			{
				oidStatusMap.erase(oid);
				TeColor color(255, 255, 255);
				objVisual.color(color);
			}
			else if (status == 1)
				objVisual.color(theme->pointingLegend().visual(TePOLYGONS)->color());
			else if (status == 2)
				objVisual.color(theme->queryLegend().visual(TePOLYGONS)->color());
			else if (status == 3)
				objVisual.color(theme->queryAndPointingLegend().visual(TePOLYGONS)->color());

//			objVisualMap[oid] = objVisual;
		}
	}

	if (oidNotGroupedSet.empty() == false)
	{
		for (setIt = oidNotGroupedSet.begin(); setIt != oidNotGroupedSet.end(); ++setIt)
		{
			oid = *setIt;		
			status = oidStatusMap[oid];
			legEntry = &(theme->defaultLegend());
			TeGeomRepVisualMap& geomRepVisualMap = legEntry->getVisualMap();
			if(geomRepVisualMap.find(geomRep) != geomRepVisualMap.end())
				visual = *(legEntry->visual(geomRep));

			objVisualMap[oid] = visual;

			TeVisual& objVisual = objVisualMap[oid];
			if (status == 0)
			{
				oidStatusMap.erase(oid);
				TeColor color(0, 0, 0);
				objVisual.color(color);
			}
			else if (status == 1)
				objVisual.color(theme->pointingLegend().visual(TePOLYGONS)->color());
			else if (status == 2)
				objVisual.color(theme->queryLegend().visual(TePOLYGONS)->color());
			else if (status == 3)
				objVisual.color(theme->queryAndPointingLegend().visual(TePOLYGONS)->color());

//			objVisualMap[oid] = objVisual;
		}
	}
}

template<> TeCoord2D calculateLabelPosition(const TePolygonSet& geomSet)
{
	TePolygon maxPoly = geomSet[0];
	double maxPolyBxArea = TeGeometryArea(maxPoly.box());

	for(unsigned  int i = 1; i < geomSet.size(); i++) //Finds the polygon with greater area
	{
		double polyBxArea = TeGeometryArea(geomSet[i].box());
		
		if(polyBxArea > maxPolyBxArea)
		{
			maxPoly.clear();
			maxPoly.copyElements(geomSet[i]);
			maxPolyBxArea = polyBxArea;
		}
	}

	TeCoord2D labelPosition(0, 0); 

	if(maxPoly.empty())
	{
		return labelPosition;
	}

	labelPosition = maxPoly.box().center();
	
	if(!TeWithin(labelPosition, maxPoly))
	{
		labelPosition = TeFindCentroid(maxPoly);
	}

	return labelPosition;
}

//bool TeAdjustSegment(TeCoord2D P0, TeCoord2D P1, double d0, TeCoord2D &P0out, TeCoord2D &P1out)
//{
//	
//	double		vL_norm1;
//	double		vL_norm2;
//
//	try
//	{
//		if( P0 == P1 )
//			return false;
//		
//		TeCoord2D vL1( (P1.x() - P0.x()), (P1.y() - P0.y()) );
//		TeCoord2D vL2( -1 * (P1.x() - P0.x()), -1 * (P1.y() - P0.y()) );
//		vL_norm1 = sqrt( vL1.x() * vL1.x() + vL1.y() * vL1.y() );
//		vL_norm2 = sqrt( vL2.x() * vL2.x() + vL2.y() * vL2.y() );
//
//
//		TeCoord2D uL1( ( vL1.x() / vL_norm1 ), ( vL1.y() / vL_norm1 ) );
//		TeCoord2D uL2( ( vL2.x() / vL_norm2 ), ( vL2.y() / vL_norm2 ) );
//		TeCoord2D pFim1 (P0.x() + uL1.x() * d0, P0.y() + uL1.y() * d0);
//		TeCoord2D pFim2 (P0.x() + uL2.x() * d0, P0.y() + uL2.y() * d0);
//
//		if ( TeDistance(pFim1, P1) <= TeDistance(pFim2, P1) )
//			{
//				P0out = P0;
//				P1out = pFim1; 
//			}
//		else
//			{
//				P0out = P0;
//				P1out = pFim2;
//			}
//	}
//	catch(...)
//	{ return (false); }
//
//	return (true);
//}

bool getMiddleLine(TeLine2D line, TeCoord2D &p)
{

	double		iDistancia;
	float		fMiddle=0.;
	float		fSize=0.;
	int			iCont=0;
	TeLine2D	lneResult;


	iDistancia=TeLength(line);
	if(iDistancia<=0)
	{
		p=line[0];
		return false;
	}

	fMiddle=(float)(((float) iDistancia)/2.);

	while(fSize<fMiddle)
	{
		lneResult.add(line[iCont]);
		iCont++;
		if(lneResult.size()>1)
			fSize=(float)TeLength(lneResult);
	}

	if(fSize>fMiddle)
	{
		double			dDistance=0;
		TeCoord2D		coorP1,coorP2;

		dDistance=TeDistance(lneResult[lneResult.size()-2],lneResult[lneResult.size()-1]);

		dDistance-=(fSize-fMiddle);

		TeAdjustSegment(lneResult[lneResult.size()-2],lneResult[lneResult.size()-1],dDistance,coorP1,coorP2);
		p=coorP2;
		
	}
	else
		p=lneResult[lneResult.size()-1];
		
	return true;
}


template<> TeCoord2D calculateLabelPosition(const TeLine2D& geomSet)
{
	TeCoord2D labelPosition = geomSet.box().center();

	getMiddleLine(geomSet, labelPosition);

	return labelPosition;
}

bool saveCentroidLabel(TeDatabase* db, const TePolygonSet& polys, const std::string& collTable)
{
	TeCoord2D labelPosition = calculateLabelPosition(polys);

	std::string sql = "UPDATE " + collTable + " SET label_x=" + Te2String(labelPosition.x()) + ",label_y=" +
		Te2String(labelPosition.y()) + " WHERE c_object_id=" + "'" +polys[0].objectId() + "'";

	if(!db->execute(sql))
	{
		return false;
	}

	return true;
}

bool getNextObject(TeDatabasePortal* portal, TePolygonSet& polys, TePolygon& nextPoly)
{
	bool hasObjects = true; 
	TePolygon p;

	if(nextPoly.empty())
	{
		hasObjects = portal->fetchGeometry(p);
		std::string oId = p.objectId();
	}
	else
	{
		p.copyElements(nextPoly);
		nextPoly.clear();
	}

	polys.add(p);

	while(hasObjects) 
	{
		TePolygon p2;
		hasObjects = portal->fetchGeometry(p2);

		if(p2.objectId() != p.objectId())
		{
			nextPoly.copyElements(p2);
			break;
		}

		polys.add(p2);
	}

	return hasObjects;
}

bool generateTextUsingCentroid(TeTheme* theme, const std::string& objectId)
{
	string	geomTable;
	string	collTable = theme->collectionTable();
	TeLayer* layer = theme->layer();
	TeDatabase* db = layer->database();
	
	if((collTable.empty()) || (!db->tableExist(collTable)))
		return false;

	if(layer->hasGeometry(TeCELLS)    || 
	   layer->hasGeometry(TePOLYGONS) ||
	   layer->hasGeometry(TeLINES))
	{
		geomTable = layer->tableName(TeCELLS);
		
		if(geomTable.empty())
		{
			geomTable = layer->tableName(TePOLYGONS);
			if(geomTable.empty())
			{
				geomTable = layer->tableName(TeLINES);

				if(geomTable.empty())
				{
					return false;
				}
				else
				{
					std::string sql = "SELECT * FROM " + geomTable + "," + collTable + 
						" WHERE object_id = c_object_id";

					if(!objectId.empty())
					{
						sql += " AND objectId='" + objectId + "'";
					}

					TeDatabasePortal* portal = db->getPortal();

					if((!portal->query(sql)) || (!portal->fetchRow()))
					{
						return false;
					}

					bool hasObjects;

					do
					{
						TeLine2D l;
						hasObjects = portal->fetchGeometry(l);
						TeCoord2D labelPosition = calculateLabelPosition(l);

						std::string sql = "UPDATE " + collTable + " SET label_x=" + 
							Te2String(labelPosition.x()) + ",label_y=" + Te2String(labelPosition.y()) + 
							" WHERE c_object_id=" + "'" +l.objectId() + "'";

						if(!db->execute(sql))
						{
							return false;
						}

					} while(hasObjects);

					portal->freeResult();
					delete portal;		
				}
			}
			else
			{
				std::string sql = "SELECT * FROM " + geomTable + "," + collTable + 
					" WHERE object_id = c_object_id";

				if(!objectId.empty())
				{
					sql += " AND objectId='" + objectId + "'";
				}

				/*if(!(db->dbmsName() == "OracleSpatial" || db->dbmsName() == "PostGIS"))
					sql += " ORDER BY object_id ASC, parent_id ASC, num_holes DESC";*/
				sql += " ORDER BY " + db->getSQLOrderBy(TePOLYGONS);

				TeDatabasePortal* portal = db->getPortal();

				if((!portal->query(sql)) || (!portal->fetchRow()))
				{
					return false;
				}

				bool hasObjects = true; 
				TePolygon auxP;

				while(hasObjects) 
				{
					TePolygonSet polys;
					hasObjects = getNextObject(portal, polys, auxP);
					saveCentroidLabel(db, polys, collTable);
				}
				
				if(!auxP.empty())
				{
					TePolygonSet polys;
					polys.add(auxP);
					saveCentroidLabel(db, polys, collTable);
				}

				portal->freeResult();
				delete portal;
			}
		}
		else
		{
			string lower_x =  "(((" + geomTable + ".spatial_box[1])[0]))";
			string lower_y =  "(((" + geomTable + ".spatial_box[1])[1]))";
			string upper_x =  "(((" + geomTable + ".spatial_box[0])[0]))";
			string upper_y =  "(((" + geomTable + ".spatial_box[0])[1]))";
			
			std::string upd  = " UPDATE " + collTable + " SET ";
			upd += " label_x = (SELECT MAX(" + lower_x + " + (" + upper_x;
			upd += " - " + lower_x + ") / 2.0) ";
			upd += "FROM " + geomTable + " WHERE object_id = c_object_id), ";
			
			upd += " label_y = (SELECT MAX(" + lower_y + " + (" + upper_y;
			upd += " - " + lower_y + ") / 2.0) ";
			upd += "FROM " + geomTable + " WHERE object_id = c_object_id) ";

			upd += " WHERE (label_x IS NULL) OR (label_y IS NULL)";

			if(!db->execute(upd))
			{
				return false;
			}
		}
	}	
	else if(layer->hasGeometry(TePOINTS))
	{
		geomTable = theme->layer()->tableName(TePOINTS);
	
		std::string upd  = " UPDATE " + collTable + " SET ";
		upd += " label_x = (SELECT MAX(x)";
		upd += " FROM " + geomTable + " WHERE object_id = c_object_id), ";


		upd += " label_y = (SELECT MAX(y)";
		upd += " FROM " + geomTable + " WHERE object_id = c_object_id) ";

		upd += " WHERE (label_x IS NULL) OR (label_y IS NULL)";

		if(!db->execute(upd))
		{
			return false;
		}
	}

	return true;
}

