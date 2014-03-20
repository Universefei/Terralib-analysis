#include "TeFileTheme.h"
#include "TeDatabase.h"
#include "TeSTElementSet.h"
#include "TeGeoDataDriver.h"
#include "TeGDriverFactory.h"
#include "TeRaster.h"

bool TeFileTheme::createFileThemeTable(TeDatabase* db)
{
	if (!db)
		return false;

	if(db->tableExist("te_file_theme"))
		return true;

	TeAttributeList attList;

	TeAttribute att1;
	att1.rep_.name_ = "theme_id";
	att1.rep_.isAutoNumber_ = false;
	att1.rep_.isPrimaryKey_ = true;
	att1.rep_.null_ = false;
	att1.rep_.type_ = TeINT;
	att1.rep_.numChar_ = 0;
	attList.push_back(att1);

	TeAttribute att2;
	att2.rep_.name_ = "file_name";
	att2.rep_.isAutoNumber_ = false;
	att2.rep_.isPrimaryKey_ = false;
	att2.rep_.null_ = false;
	att2.rep_.type_ = TeSTRING;
	att2.rep_.numChar_ = 255;
	attList.push_back(att2);

	TeAttribute att3;
	att3.rep_.name_ = "geom_rep";
	att3.rep_.isAutoNumber_ = false;
	att3.rep_.isPrimaryKey_ = false;
	att3.rep_.null_ = false;
	att3.rep_.type_ = TeINT;
	att3.rep_.numChar_ = 0;
	attList.push_back(att3);

	TeAttribute att4;
	att4.rep_.name_ = "proj_id";
	att4.rep_.isAutoNumber_ = false;
	att4.rep_.isPrimaryKey_ = false;
	att4.rep_.null_ = false;
	att4.rep_.type_ = TeINT;
	att4.rep_.numChar_ = 0;
	attList.push_back(att4);

	if (!db->createTable("te_file_theme", attList)||
		!db->createRelation("fk_file_theme_id", "te_file_theme", "theme_id", "te_theme", "theme_id", true) ||
		!db->createRelation("fk_file_theme_proj_id", "te_file_theme", "proj_id", "te_projection", "projection_id", false))
		return false;
	return 1;
}



TeFileTheme::TeFileTheme(const string& name, TeViewNode* parent, int view, int id):
		TeAbstractTheme(name, parent, view, id, TeFILETHEME),
		fileName_(""), 
		localDb_(0),
		numObjects_(0),
		projection_(0),
		dataDriver_(0),
		raster_(0)
{
}

TeFileTheme::TeFileTheme(const std::string& themeName, TeGeoDataDriver* dataDriver, TeDatabase* localDb, TeViewNode* parent, int view):
		TeAbstractTheme(themeName, parent, view, 0, TeFILETHEME),
		localDb_(localDb),
		numObjects_(0),
		projection_(0),
		dataDriver_(0),
		raster_(0)
{
	if(dataDriver != 0)
	{
		fileName_= dataDriver->getFileName();
		setDataDriver(dataDriver);
	}
}

TeFileTheme::TeFileTheme(const TeFileTheme& other):
		TeAbstractTheme(static_cast<const TeAbstractTheme&>(other)),
		dataDriver_(0)
{
	fileName_ = other.fileName_;
	localDb_ = other.localDb_;
	projection_ = other.projection_;

	if(other.dataDriver_ != 0)
		setDataDriver(other.dataDriver_);

	if(other.raster_ != 0)
		setRasterFile(other.fileName_);
}

TeFileTheme::~TeFileTheme()
{
	if (dataDriver_)
		delete dataDriver_;
	dataDriver_ = 0; 

	delete projection_;

	delete raster_;
}

TeFileTheme& TeFileTheme::operator=(TeFileTheme& rhs)
{
	if ( this != &rhs )
	{
		*(TeAbstractTheme*)this = rhs;
		fileName_ = rhs.fileName_;
		localDb_ = rhs.localDb_;

		projection_ = rhs.projection_;

		if(rhs.dataDriver_ != 0)
			setDataDriver(rhs.dataDriver_);

		if(rhs.raster_ != 0)
			setRasterFile(rhs.fileName_);

	}
	return *this;
}

TeViewNode* TeFileTheme::clone()
{
	TeFileTheme* theme = new TeFileTheme();
	*theme = *this;

	projection_ = TeProjectionFactory::make(projection_->params());
	dataDriver_ = 0;

	return theme;
}

void TeFileTheme::setRasterFile(const std::string& fileName)
{
	if(fileName.empty())
		return;

	delete raster_;
	
	raster_ = new TeRaster(fileName);

	if(!raster_->init())
	{
		delete raster_;
		raster_ = 0;
	}
	else
	{
		this->fileName_ = fileName;
		this->visibleRep(TeRASTERFILE);
		this->setThemeBox(raster_->box());
	}
}

TeRaster* TeFileTheme::getRasterFile()
{
	return raster_;
}

void TeFileTheme::setDataDriver(TeGeoDataDriver* dataDriver)
{ 
	if (!dataDriver)
		return;

	if (dataDriver_)
		delete dataDriver_;

	dataDriver_ = dataDriver;

	numObjects_ = 0;
	TeBox bb = dataDriver_->getBox();
	int rep = dataDriver_->getRepresentation();

	this->visibleRep(rep);
	this->setThemeBox(bb);
}

TeGeoDataDriver* TeFileTheme::getDataDriver()
{
	return dataDriver_;
}

/*
TeSTElementSet* TeFileTheme::getData()
{
	return data_;
}*/

void TeFileTheme::setFileName(const std::string& fileName)
{
	if(fileName.empty())
		return;

	fileName_ = fileName;

	std::string fileExtension = TeGetExtension(fileName_.c_str()); 
	fileExtension = TeConvertToUpperCase(fileExtension);

	TeGDriverParams params(fileExtension, fileName_);
	
	TeGeoDataDriver* dataDriver = TeGDriverFactory::make(&params);
	setDataDriver(dataDriver);
}

std::string TeFileTheme::getFileName()
{
	return fileName_;
}

void TeFileTheme::setLocalDatabase(TeDatabase* db)
{	
	this->localDb_ = db; 
}

TeDatabase* TeFileTheme::getLocalDatabase()
{ 
	return localDb_; 
}

void TeFileTheme::clearData()
{ 
	//data_->clear();
}

unsigned int TeFileTheme::getNumberOfObjects()
{	
	return numObjects_; 
}

void TeFileTheme::setNumberOfObjects(unsigned int n)
{
	numObjects_ = n;
}

bool TeFileTheme::retrieveDataFromFile()
{
	if(!dataDriver_)
		return false;
	
	fileName_= dataDriver_->getFileName();

	if(!dataDriver_->initData())
	{
		return false;
	}

	TeBox bb = dataDriver_->getBox();
	int rep = dataDriver_->getRepresentation();

	this->visibleRep(rep);
	this->setThemeBox(bb);


	return true;
}

bool TeFileTheme::loadMetadata(TeDatabase* localDb)
{
	if(!localDb)
		return false;

	localDb_ = localDb;

	//second: load specific theme information from te_file_theme table
	TeDatabasePortal* portal = localDb_->getPortal();
	if(!portal)
		return false;

	//table: theme_id, file_name, geom_rep
	std::string sql = " SELECT te_file_theme.*, ";	
	sql += " te_projection.*, te_datum.radius, te_datum.flattening, te_datum.dx, te_datum.dy, te_datum.dz";
	sql += " FROM te_file_theme, te_projection, te_datum";
	sql += " WHERE te_file_theme.proj_id = te_projection.projection_id ";
	sql += " AND te_projection.datum = te_datum.name";
	sql += " AND theme_id = "+ Te2String(id());

	if(!portal->query(sql) || !portal->fetchRow())
	{
		delete portal;
		return false;
	}

	std::string fileName = portal->getData(1);
	TeAbstractTheme::visibleRep(portal->getInt(2));
	
	delete projection_;
	portal->getProjection(&projection_, 4);

	delete portal;

	setFileName(fileName);

	return true;
}

bool TeFileTheme::saveMetadata(TeDatabase* localDb_)
{
	if(!localDb_)
		return false;

	//insert theme in database 
	if(id()==0)
	{
		if(!localDb_->insertTheme(this)) //updateThemeTable
		{
			localDb_->deleteTheme(this->id());
			return false;
		}
	}

	if (!projection_)
		projection_ = new TeNoProjection();
	if (projection_->id() <= 0)
		localDb_->insertProjection(projection_);

	// remove the metadata currently stored
	string strSQL;
	strSQL = "DELETE FROM te_file_theme WHERE theme_id = ";
	strSQL += Te2String(id());
	localDb_->execute(strSQL);

	// insert the current metadata
	strSQL  = "INSERT INTO te_file_theme (theme_id, file_name, geom_rep, proj_id) VALUES (";
	strSQL += Te2String(id());
	strSQL += ", '";
	strSQL += localDb_->escapeSequence(fileName_);
	strSQL += "', ";
	strSQL += Te2String(visibleRep_);
	strSQL += ", ";
	strSQL += Te2String(projection_->id());
	strSQL += ")";

	if(!localDb_->execute(strSQL))
	{
		localDb_->deleteTheme(this->id());
		return false;
	}
	return true;
}


bool TeFileTheme::eraseMetadata(TeDatabase* localDb_)
{
	if (!localDb_)
		return false;

	int proid = -1;
	string strSQL = "SELECT proj_id FROM te_file_theme WHERE theme_id = "+ Te2String(this->id());
	TeDatabasePortal* portal = localDb_->getPortal();
	if (portal->query(strSQL) && portal->fetchRow())
		proid = portal->getInt(0);
	delete portal;

	strSQL  = " DELETE FROM te_file_theme ";
	strSQL += " WHERE theme_id = "+ Te2String(this->id());
	
	if (!localDb_->execute(strSQL))
		return false;

	if (proid > 0)
	{
		if(localDb_->deleteProjection(proid) == false)
		{
			return false;
		}
	}
	return true;
}


TeProjection* TeFileTheme::getThemeProjection()
{	
	if(projection_ == 0)
	{
		if(dataDriver_ != 0)
		{
			projection_ = TeProjectionFactory::make(dataDriver_->getDataProjection()->params());
		}
		else if(raster_ != 0)
		{
			projection_ = TeProjectionFactory::make(raster_->params().projection()->params());
		}
	}

	return projection_;
}

void TeFileTheme::setThemeProjection(TeProjection* proj)
{ 
	if (projection_)
		delete projection_;
	projection_ = proj;
}

bool TeFileTheme::buildGrouping(const TeGrouping& , TeSelectedObjects , vector<double>* )
{
	return true;
}

bool TeFileTheme::buildGrouping(const TeGrouping& , TeChronon , vector<map<string, string> >& )
{
	return true;
}

bool TeFileTheme::saveGrouping(TeSelectedObjects )
{
	return true;
}


void TeFileTheme::setLegendsForObjects()
{
}

bool TeFileTheme::deleteGrouping()
{
	return true;
}

bool TeFileTheme::save()
{
	//insert theme in database 
	if(id()==0)
	{
		if(!localDb_->insertTheme(this)) //updateThemeTable
		{
			localDb_->deleteTheme(this->id());
			return false;
		}
	}
	else
	{
		if(!localDb_->updateTheme(this)) //updateThemeTable
		{
			localDb_->deleteTheme(this->id());
			return false;
		}
	}
	return true;
}

bool TeFileTheme::locatePolygon(TeCoord2D &pt, TePolygon &polygon, const double& tol)
{
	TePolygonSet aux;
	if(!locatePolygonSet(pt, tol, aux))
		return false;
	if(aux.empty())
		return false;
	
	polygon = aux[0];
	return true;	
}

bool TeFileTheme::locatePolygonSet(TeCoord2D& /*pt*/, double /*tol*/, TePolygonSet& /*polygons*/)
{
	//	if(!data_->locateGeometry(pt, polygons, tol))
	//		return false;
	return true;
}

bool TeFileTheme::locateLine(TeCoord2D& /*pt*/, TeLine2D& line, const double& /*tol*/)
{
	TeLineSet aux;
	//	if(!data_->locateGeometry(pt, aux, tol))
	//		return false;
	if(aux.empty())
		return false;
	line = aux[0];
	return true;
}

bool TeFileTheme::locatePoint(TeCoord2D& /*pt*/, TePoint &point, const double& /*tol*/)
{
	TePointSet aux;
	//	if(!data_->locateGeometry(pt, aux, tol))
	//		return false;
	if(aux.empty())
		return false;
	point = aux[0];
	return true;
}

bool TeFileTheme::locateCell(TeCoord2D&, TeCell&, const double&)
{
	return false;
}


std::set<std::string> TeFileTheme::getObjects(TeSelectedObjects )
{
	std::set<std::string> set;
	return set;
}


std::set<std::string> TeFileTheme::getObjects(const vector<string>& )
{
	std::set<std::string> set;
	return set;
}


std::vector<std::string> TeFileTheme::getItemVector(TeSelectedObjects )
{
	std::vector<std::string> set;
	return set;
}


std::vector<std::string> TeFileTheme::getItemVector(const set<string>& )
{
	std::vector<std::string> set;
	return set;
}

void TeFileTheme::setOwnLegendsForObjects()
{

}

bool TeFileTheme::beforeCopyThemeTo(TeAbstractTheme* absThemeCopy, TeDatabase* outputDatabase)
{
	if(TeAbstractTheme::beforeCopyThemeTo(absThemeCopy, outputDatabase) == false)
	{
		return false;
	}

	//Teste se o tema eh realmente um TeExternalTheme
	TeFileTheme* themeCopy = dynamic_cast<TeFileTheme*>(absThemeCopy);
	if(themeCopy == 0)
	{
		return false;
	}

	if(themeCopy->createFileThemeTable(outputDatabase) == false)
	{
		return true;
	}

	themeCopy->setLocalDatabase(outputDatabase);
	TeProjection* projIn = themeCopy->getThemeProjection();
	TeProjection* projOut = TeProjectionFactory::make(projIn->params());

	if(!outputDatabase->insertProjection(projOut))
	{
		return false;
	}

	themeCopy->setThemeProjection(projOut);
	
	return true;
}
