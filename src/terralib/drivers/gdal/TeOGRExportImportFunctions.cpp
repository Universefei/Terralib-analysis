// OGR Driver include files
#include "TeOGRExportImportFunctions.h"

// TerraLib include files
#include <TeDatabase.h>
#include <TeException.h>
#include <TeGDriverFactory.h>
#include <TeLayer.h>
#include <TeSTElementSet.h>
#include <TeGeoDataSet.h>

/** @name Auxiliaries methods. */
//@{
bool TeExportPolygonsToOGR(TeLayer* lrInput, TeGeoDataDriver* driver)
{
	if(lrInput == 0 || driver == 0)
		return false;

	TeAttrTableVector atts;
	std::string tableName = lrInput->tableName(TePOLYGONS);
	lrInput->getAttrTables(atts);
	if(atts.empty()) 
		return false;
	
	std::string linkName = atts[0].linkName();
	if(linkName.empty())
		return false;

	TeDatabase* db = lrInput->database();
	if(db == 0)
		return false;

	TeDatabasePortal* dbPortal = dbPortal=db->getPortal();
	if(dbPortal == 0)
		return false;

	TeAttributeList attList;

	TeAttribute att;
	att.rep_.name_="object_id";
	att.rep_.numChar_=255;
	att.rep_.type_=TeSTRING;
	attList.push_back(att);

	bool flag;
	std::string sql = "select * from " + tableName; // TODO: Now I'm doing only the export vector. The attribute is his own object_id.
	if((dbPortal->query(sql) == true) && (dbPortal->fetchRow() == true))
	{
		do
		{
			TePolygon polRead;
			TeSTInstance obj;
			TeProperty prop;

			flag = dbPortal->fetchGeometry(polRead);
			prop.attr_.rep_.name_ = "object_id";
			prop.attr_.rep_.numChar_ = 255;
			prop.attr_.rep_.type_ = TeSTRING;
			prop.value_ = polRead.objectId();

			obj.setAttrList(attList);
			obj.setObjectId(polRead.objectId());
			obj.addGeometry(polRead);
			obj.addProperty(prop);
			
			if(!polRead.empty())
				driver->addElement(&obj);

		}while(flag);
	}

	delete dbPortal;
	
	return true;
}

bool TeExportLinesToOGR(TeLayer* lrInput, TeGeoDataDriver* driver)
{
	if(lrInput == 0 || driver == 0)
		return false;

	TeAttrTableVector atts;
	std::string tableName = lrInput->tableName(TeLINES);
	lrInput->getAttrTables(atts);
	if(atts.empty()) 
		return false;
	
	std::string linkName = atts[0].linkName();
	if(linkName.empty())
		return false;

	TeDatabase* db = lrInput->database();
	if(db == 0)
		return false;

	TeDatabasePortal* dbPortal = dbPortal=db->getPortal();
	if(dbPortal == 0)
		return false;

	TeAttributeList attList;

	TeAttribute att;
	att.rep_.name_="object_id";
	att.rep_.numChar_=255;
	att.rep_.type_=TeSTRING;
	attList.push_back(att);

	bool flag;
	std::string sql = "select * from " + tableName; // TODO: Now I'm doing only the export vector. The attribute is his own object_id.
	if((dbPortal->query(sql) == true) && (dbPortal->fetchRow() == true))
	{
		do
		{
			TeLine2D lneRead;
			TeSTInstance obj;
			TeProperty prop;

			flag = dbPortal->fetchGeometry(lneRead);
			prop.attr_.rep_.name_ = "object_id";
			prop.attr_.rep_.numChar_ = 255;
			prop.attr_.rep_.type_ = TeSTRING;
			prop.value_ = lneRead.objectId();

			obj.setAttrList(attList);
			obj.setObjectId(lneRead.objectId());
			obj.addGeometry(lneRead);
			obj.addProperty(prop);
			
			if(!lneRead.empty())
				driver->addElement(&obj);

		}while(flag);
	}

	delete dbPortal;
	
	return true;
}

bool TeExportPointsToOGR(TeLayer* lrInput, TeGeoDataDriver* driver)
{
	if(lrInput == 0 || driver == 0)
		return false;

	TeAttrTableVector atts;
	std::string tableName = lrInput->tableName(TePOINTS);
	lrInput->getAttrTables(atts);
	if(atts.empty()) 
		return false;
	
	std::string linkName = atts[0].linkName();
	if(linkName.empty())
		return false;

	TeDatabase* db = lrInput->database();
	if(db == 0)
		return false;

	TeDatabasePortal* dbPortal = dbPortal=db->getPortal();
	if(dbPortal == 0)
		return false;

	TeAttributeList attList;

	TeAttribute att;
	att.rep_.name_="object_id";
	att.rep_.numChar_=255;
	att.rep_.type_=TeSTRING;
	attList.push_back(att);

	bool flag;
	std::string sql = "select * from " + tableName; // TODO: Now I'm doing only the export vector. The attribute is his own object_id.
	if((dbPortal->query(sql) == true) && (dbPortal->fetchRow() == true))
	{
		do
		{
			TePoint ptoRead;
			TeSTInstance obj;
			TeProperty prop;

			flag = dbPortal->fetchGeometry(ptoRead);
			prop.attr_.rep_.name_ = "object_id";
			prop.attr_.rep_.numChar_ = 255;
			prop.attr_.rep_.type_ = TeSTRING;
			prop.value_ = ptoRead.objectId();

			obj.setAttrList(attList);
			obj.setObjectId(ptoRead.objectId());
			obj.addGeometry(ptoRead);
			obj.addProperty(prop);
			driver->addElement(&obj);

		}while(flag);
	}

	delete dbPortal;
	
	return true;
}
//@}

bool populateTable(TeTable& tb, std::vector<std::string> props)
{
	if(props.empty())
		return false;

	TeTableRow row;
	std::vector<std::string>::iterator it;
	for(it = props.begin(); it != props.end(); ++it)
	{
		std::string value = (*it);
		row.push_back(value);
	}

	tb.add(row);
	
	return true;
}

TeAttribute getAutoLinkName(const TeAttributeList& attList)
{
	unsigned int i = 0;
	unsigned int j = 1;

	std::string linkName = "fid";
	while(i < attList.size())
	{
		linkName = "fid_" + Te2String(j);
		if(TeConvertToUpperCase(linkName) == TeConvertToUpperCase(attList[i].rep_.name_))
		{
			j++;
			i = 0;
			continue;
		}
		i++;
	}
	
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 16;
	at.rep_.name_ = linkName;
	at.rep_.isPrimaryKey_ = true;

	return at;
}

bool TeExportToOGR(TeLayer* lrInput, const std::string& fileName, const std::string& ogrType)
{
	if(lrInput == 0)
		return false;

	if(fileName.empty())
		return false;

	TeGDriverParams params("OGR", fileName);
	params.writing_ = true;

	TeGeoDataDriver* driver = TeGDriverFactory::make(&params);
	if(driver == 0)
		return false;

	TeAttributeList list;
	TeAttribute at;
	at.rep_.name_ = "object_id";
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 255;
	list.push_back(at);

	TeTable table;
	table.setAttributeList(list);

	if(!driver->createLayer(lrInput, lrInput->geomRep()))
		return false;
	
	if(!driver->createAttributeTable(table))
		return false;
	
	if(lrInput->hasGeometry(TePOLYGONS))
		TeExportPolygonsToOGR(lrInput, driver);
	else if(lrInput->hasGeometry(TeLINES))
		TeExportLinesToOGR(lrInput, driver);
	else if(lrInput->hasGeometry(TePOINTS))
		TeExportPointsToOGR(lrInput, driver);
	
	delete driver;

	return true;
}

bool TeImportOGR(TeLayer* lrInput, const std::string& fileName, const std::string& linkName, const int& chunkSize)
{
	if(lrInput == 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Invalid null layer!");
	
	if(fileName.empty()) 
		throw TeException(UNKNOWN_ERROR_TYPE, "Invalid empty file name!");	

	TeAttributeList attList;
	std::vector<std::string> propertyValues;
	bool saveTable;
	int i = 0;
	bool status = false;

	TePolygonSet plsRead;
	TeLineSet lnsRead;
	TePointSet ptsRead;
	TeTextSet txsRead;
		
	try
	{
		std::string textTableName = lrInput->name() + "_text";

		if(TeProgress::instance() != 0)
			TeProgress::instance()->setProgress(0);

		TeGDriverParams params("OGR",fileName);
		TeGeoDataDriver* driver = TeGDriverFactory::make(&params);		

		if(driver != 0)
		{
			TeGeoDataSet* dataSet = driver->getData();
			int rep = driver->getRepresentation();

			attList = driver->getAttributesList();
			std::string link_name = linkName;
			if(linkName.empty())
			{
				TeAttribute at = getAutoLinkName(attList);
				attList.insert(attList.begin(),at);
				link_name = at.rep_.name_;
			}

			TeTable teTable(lrInput->name(), attList, link_name, link_name, TeAttrStatic);
			teTable.clear();
			if(lrInput->createAttributeTable(teTable))
			{
				unsigned int numObjects = dataSet->size();				
				for(unsigned int i = 0; i < numObjects; ++i)
				{
					if(TeProgress::instance() != 0)
						TeProgress::instance()->setTotalSteps(numObjects);
										
					if(rep & TePOLYGONS)
					{
						TePolygonSet ps;
						dataSet->getGeometry(i, ps);

						plsRead.copyElements(ps);
					}
					if(rep & TeLINES)
					{
						TeLineSet ls;
						dataSet->getGeometry(i, ls);

						lnsRead.copyElements(ls);
					}
					if(rep & TePOINTS)
					{
						TePointSet ps;
						dataSet->getGeometry(i, ps);
						ptsRead.copyElements(ps);
					}
					if(rep & TeTEXT)
					{
						TeTextSet ts;
						//dataSet->getGeometry(i, ps);
						txsRead.copyElements(ts);
					}

					dataSet->getAttributes(i, propertyValues);
					populateTable(teTable, propertyValues);

					saveTable = false;
					if(plsRead.size() >= (unsigned int)chunkSize)
					{
						lrInput->addPolygons(plsRead);
						saveTable = true;
						plsRead.clear();
					}
						
					if(lnsRead.size() >= (unsigned int)chunkSize)
					{
						lrInput->addLines(lnsRead);
						saveTable = true;
						lnsRead.clear();
					}

					if(ptsRead.size() >= (unsigned int)chunkSize)
					{
						lrInput->addPoints(ptsRead);
						saveTable=true;
						ptsRead.clear();
					}

					if(txsRead.size() >= (unsigned int)chunkSize)
					{
						lrInput->addText(txsRead, textTableName);
						saveTable = true;
						txsRead.clear();
					}

					if(saveTable == true)
					{
						lrInput->saveAttributeTable(teTable);
						teTable.clear();
						saveTable = false;
					}
					i++;
					
					if(TeProgress::instance() != 0)
					{
						if (TeProgress::instance()->wasCancelled())	break;
						TeProgress::instance()->setProgress(i);
					}
				}
				
				if(!plsRead.empty())
					lrInput->addPolygons(plsRead);
				
				if(!lnsRead.empty())
					lrInput->addLines(lnsRead);
				
				if(!ptsRead.empty())
					lrInput->addPoints(ptsRead);
				
				if(!txsRead.empty())
					lrInput->addText(txsRead, textTableName);
				
				if(teTable.size() > 0)
					lrInput->saveAttributeTable(teTable);
				
				status = true;
			}
			delete dataSet;
		}

		delete driver;		
	}
	catch(...)
	{
		status = false;
	}

	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	return status;
}
