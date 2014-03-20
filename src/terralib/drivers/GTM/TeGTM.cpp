#include <TeGTM.h>
#include <TeException.h>
#include <TeImportRaster.h>
#include <TeDecoderJPEG.h>
#include <TeInitRasterDecoders.h>

#include <stdio.h>
#include <time.h>

//Classe principal, eh ela que le de fato o arquivo
TeGTM::TeGTM(FILE* file)
{
	this->file_ = file;

	//Treta muito louca !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	char *icon_number[] = {"", "Airport", "Ball Park", "Bank", "Bar", "Boat Ramp", "Campground", "Car",
	"City (Large)", "City (Medium)", "City (Small)", "Dam", "Danger Area",
	"Drinking Water", "Fishing Area", "Gas Station", "Glider Area", "Golf Course",
	"Heliport", "Hotel", "Animals", "Information", "Man Overboard", "Marina",
	"Mine", "Medical Facility", "Parachute Area", "Park", "Parking Area",
	"Picnic Area", "Private Field", "Residence", "Restaurant", "Restroom",
	"Scenic Area", "School", "Seaplane Base", "Shipwreck", "Shopping Center",
	"Short Tower", "Policy Station", "Ski Resort", "Soft Field", "Swimming Area",
	"Tall Tower", "Telephone", "Tracback Point", "Ultralight Area", "Waypoint",
	"Boat", "Exit", "Flag", "Duck", "Buoy", "Back Track", "Beach", "Bridge",
	"Building", "Car Repair", "Cemetery", "Church", "Civil", "Convenience Store",
	"Crossing", "Fast Food", "Forest", "Ghost Town", "Levee", "Military",
	"Oil Field", "Post Office", "Rv Park", "Scales", "Summit", "Toll Booth",
	"Trail Head", "Truck Stop", "Tunnel", "Highway", "Gate", "Fall", "Fence",
	"Mata-Burro", "Fitness Center", "Movie Theater", "Live Theater", "Zoo", "Horn",
	"Bowling", "Car Rental", "City (Capitol)", "Controlled Area", "Stadium",
	"Museum", "Amusement Park", "Skull", "Department Store", "Pharmacy", "Pizza",
	"Diver Down Flag 1", "Light", "Pin", "", "Pigsty", "Tree", "Bamboo",
	"Banana Plant", "Arrow-Down", "Bifurcation", "Cavern", "River", "Rock",
	"Arrow-Up", "Trunk", "Soccer Field", "Sporting Court", "Flag, Green", "Trench",
	"Ship-Yellow", "Green Sign", "Swamp", "Lake", "Stop!",
	"Fishing Hot Spot Facility", "Speed Reducer", "Stairway", "Cactus", "Ship-Red",
	"Letter - S", "Letter - D", "Letter - N",
	"Crossing", "Cross", "Flag, Red", "Curve1", "Curve2", "Curve3", "Curve4",
	"Letter - W", "Letter - L", "Letter - R", "Radio Beacon", "Road Sign",
	"Geocache", "Geocache Found", "Traffic Light", "Bus Station", "Train Station",
	"School", "Mile Marker", "Conservation Area", "Waypoint", "Box", "Aerial",
	"Auto Repair", "Boat", "Exit Ramp", "Fixed Nav Aid", "Floating Buoy", "Garden",
	"Fish Farm", "Lighthouse", "Truck Service", "Resort", "Scuba", "Shooting",
	"Sight Seeing", "Sounding", "Winery", "Navaid, Amber", "Navaid, Black",
	"Navaid, Blue", "Navaid, Green", "Navaid, Green/Red", "Navaid, Green/White",
	"Navaid, Orange", "Navaid, Red", "Navaid, Red/Green", "Navaid, Red/White",
	"Navaid, Violet", "Navaid, White", "Navaid, White/Green", "Navaid, White/Red",
	"Buoy, White", "Dot, White", "Red Square", "Red Diamond", "Green Square",
	"Green Diamond", "Restricted Area", "Navaid (unlit)", "Dot (Small)", "Libraries", "Waypoint", "Waypoint1",
	"Waypoint2", "Mark (1)", "Mark (2)", "Mark (3)", "Cross (Red)", "Store",
	"Exclamation", "Flag (EUA)", "Flag (CAN)", "Flag (BRA)", "Man", "Animals",
	"Deer Tracks", "Tree Stand", "Bridge", "Fence", "Intersection",
	"Non Direct Beacon", "VHF Omni Range", "Vor/Tacan", "Vor-Dme",
	"1st Approach Fix", "Localizer Outer", "Missed Appr. Pt", "Tacan",
	"CheckPoint", NULL};

	std::vector<char*> aux(icon_number, icon_number + sizeof(icon_number)/sizeof(char*));
	icon_descr = aux;
	//**************************************************************************************
}

TeGTM::~TeGTM()
{
	fclose(file_);
	for(unsigned int i = 0; i < informationimages_.size(); i++)
	{
		delete informationimages_.at(i);
	}
	informationimages_.clear();
	for(unsigned int i = 0; i < waypoints_.size(); i++)
	{
		delete waypoints_.at(i);
	}
	waypoints_.clear();
	for(unsigned int i = 0; i < waypointsstyles_.size(); i++)
	{
		delete waypointsstyles_.at(i);
	}
	waypointsstyles_.clear();
	for(unsigned int i = 0; i < tracklogs_.size(); i++)
	{
		delete tracklogs_.at(i);
	}
	tracklogs_.clear();
	for(unsigned int i = 0; i < tracklogstyles_.size(); i++)
	{
		delete tracklogstyles_.at(i);
	}
	tracklogstyles_.clear();
	for(unsigned int i = 0; i < routes_.size(); i++)
	{
		delete routes_.at(i);
	}
	routes_.clear();
	/*for(unsigned int i = 0; i < layers_.size(); i++)
	{
		delete layers_.at(i);
	}
	layers_.clear();
	for(unsigned int i = 0; i < usericons_.size(); i++)
	{
		delete usericons_.at(i);
	}
	usericons_.clear();*/
}

std::string TeGTM::getDate(long date)
{
	//Data a partir de um long passado
	time_t data = date;
	time_t origem;

	//Criando um struct tm com a data base
	time_t aux;
	time(&aux);
	struct tm *origemInfo = localtime(&aux);

	//Subtracoes feitas pois o padrao no time.h
	//eh o ano de 1900 e o mes fica no intervalo [0-11]
	origemInfo->tm_year = 1989 - 1900;
	origemInfo->tm_mon = 12 - 1;
	origemInfo->tm_mday = 31;
	origemInfo->tm_hour = 0;
	origemInfo->tm_min = 0;
	origemInfo->tm_sec = 0;

	//Redefinindo o origemInfo para antender o padrao GTM
	origem = mktime(origemInfo);

	//Somando as datas
	time_t result = data + origem;
	struct tm *resultInfo = localtime(&result);

	//retorna a data em string
	char buffer [80];
	strftime (buffer,80,"%c",resultInfo);
	return buffer;
}

void TeGTM::readGTM()
{
	//Lendo cabecalho
	header_.readGTMHeader(file_);
	//Lendo Grid e Datum
	griddatum_.readGTMGridDatum(file_);
	//Lendo images information
	for(unsigned int i = 0; i < header_.getN_maps(); i++)
	{
		TeGTMInformationImages* pt = new TeGTMInformationImages();
		pt->readGTMInformationImages(file_);
		informationimages_.push_back(pt);
	}
	//Lendo waypoints
	for(unsigned int i = 0; i < header_.getNwpts(); i++)
	{
		TeGTMWaypoints* pt = new TeGTMWaypoints();
		pt->readGTMWaypoints(file_);
		waypoints_.push_back(pt);
	}
	//Lendo waypoints styles
	for(unsigned int i = 0; i < header_.getNwptstyles(); i++)
	{
		TeGTMWaypointsStyles* pt = new TeGTMWaypointsStyles();
		pt->readGTMWaypointsStyles(file_);
		waypointsstyles_.push_back(pt);
	}
	//Lendo tracklogs
	for(unsigned int i = 0; i < header_.getNtracks(); i++)
	{
		TeGTMTrackLogs* pt = new TeGTMTrackLogs();
		pt->readGTMTrackLogs(file_);
		tracklogs_.push_back(pt);
	}
	//Lendo tracklog styles
	for(unsigned int i = 0; i < header_.getN_tk(); i++)
	{
		TeGTMTrackLogStyles* pt = new TeGTMTrackLogStyles();
		pt->readGTMTrackLogStyles(file_);
		tracklogstyles_.push_back(pt);
	}
	//Lendo routes
	for(unsigned int i = 0; i < header_.getNrtes(); i++)
	{
		TeGTMRoutes* pt = new TeGTMRoutes();
		pt->readGTMRoutes(file_);
		routes_.push_back(pt);
	}
	//Lendo user layers (Nao implementado ainda na versao 211, mas jah esta pronto aqui)
	/*for(unsigned int i = 0; i < header_.getLayers(); i++)
	{
		TeGTMLayers* pt = new TeGTMLayers();
		pt->readGTMLayers(file_);
		layers_.push_back(pt);
	}*/
	//Lendo user icons (Nao implementado ainda na versao 211, mas jah esta pronto aqui)
	/*for(unsigned int i = 0; i < header_.getIconnum(); i++)
	{
		TeGTMUserIcons* pt = new TeGTMUserIcons();
		pt->readGTMUserIcons(file_);
		usericons_.push_back(pt);
	}*/
	//Lendo as imagens anexadas no final do arquivo
	//e guardando elas no atributo blob_ da classe TeGTMInformationImages
	for(unsigned int i = 0; i < header_.getN_maps(); i++)
	{
		char* blob;
		long len = (informationimages_[i])->getImagelen();
		blob = new char[len+1];
		fread(blob, (size_t)len, 1, file_);
		blob[len] = '\0';
		(informationimages_[i])->setBlob(blob);
	}
}

void TeGTM::waypointAttributeList(TeAttributeList& attList)
{
	std::vector<std::string> names;

	names.push_back("NOME");
	names.push_back("COMENTARIOS");
	names.push_back("ENTIDADE");
	names.push_back("DISPLAY");
	names.push_back("DATA");
	names.push_back("ROTACAO");
	names.push_back("ALTITUDE");
	names.push_back("LAYER");

	for( unsigned int i = 0; i< names.size();i++)
    {
		TeAttribute attribute;
		attribute.rep_.type_ = TeSTRING;
		attribute.rep_.numChar_ = 255;		
		attribute.rep_.name_ = names[i];
		attribute.rep_.isPrimaryKey_ = false;
		attList.push_back(attribute);
	}
}

void TeGTM::tracklogAttributeList(TeAttributeList& attList)
{
	std::vector<std::string> names;

	names.push_back("DATA");
	names.push_back("TIPO");
	names.push_back("ALTITUDE");
	
	for( unsigned int i = 0; i< names.size();i++)
    {
		TeAttribute attribute;
		attribute.rep_.type_ = TeSTRING;
		attribute.rep_.numChar_ = 255;		
		attribute.rep_.name_ = names[i];
		attribute.rep_.isPrimaryKey_ = false;
		attList.push_back(attribute);
	}
}

void TeGTM::routeAttributeList(TeAttributeList& attList)
{
	std::vector<std::string> names;

	names.push_back("NOME");
	names.push_back("COMENTARIOS");
	names.push_back("ROTA");
	names.push_back("ENTIDADE");
	names.push_back("DISPLAY");
	names.push_back("TIPO");
	names.push_back("DATA");
	names.push_back("ROTACAO");
	names.push_back("ALTITUDE");
	names.push_back("LAYER");

	for( unsigned int i = 0; i< names.size();i++)
    {
		TeAttribute attribute;
		attribute.rep_.type_ = TeSTRING;
		attribute.rep_.numChar_ = 255;		
		attribute.rep_.name_ = names[i];
		attribute.rep_.isPrimaryKey_ = false;
		attList.push_back(attribute);
	}
}

bool TeGTM::importWaypoints(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize )
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}

	if (!layer->database()->beginTransaction())
		return false;

	int linkCol = -1;
    unsigned int j = 0;

	//Recuperando lista de atributos
	TeAttributeList attList;
	waypointAttributeList(attList);

	bool attributeTableCreated = createAttributeTable(layer, attList, attrTableName, objectIdAtt, oldPrec, linkCol);
	if(attributeTableCreated == false)
		return false;

	string objid, value;
	TePointSet points;
	TeTableRow row;
	TeAttrTableVector attTables;

	int nEntities = header_.getNwpts();
    if(TeProgress::instance())
    {
        TeProgress::instance()->reset();
    }
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	for (j = 0; j < nEntities; j++)
	{
		//Preenchendo a linha da tabela de atributos
		row.clear();
		row.push_back(waypoints_[j]->getName());
		row.push_back(waypoints_[j]->getWname());
		row.push_back(icon_descr[waypoints_[j]->getIco()]);
		row.push_back(string(1,waypoints_[j]->getDspl()));
		//row.push_back(Te2String(waypoints_[j]->getWdate()));
		row.push_back(getDate(waypoints_[j]->getWdate()));
		row.push_back(Te2String(waypoints_[j]->getWrot()));
		row.push_back(Te2String(waypoints_[j]->getWalt()));
		row.push_back(Te2String(waypoints_[j]->getWlayer()));

		//Criando indice automatico
		row.push_back(Te2String(j));

		if (row[linkCol].empty())
			row[linkCol] = "te_nulo";
		objid = row[linkCol];

		//Pegando a tabela de atributos criada pelo metodo createAttributeTable
		layer->getAttrTables(attTables, TeAttrStatic);
		attTables[0].add(row);

		//Criando o TePoint
		TePoint point(waypoints_[j]->getLongitude(), waypoints_[j]->getLatitude());
		point.objectId(objid);
		points.add(point);

		if ((j%chunkSize) == 0)
		{
			if(!layer->addPoints(points))
				return false;
			points.clear();
			if(!layer->saveAttributeTable(attTables[0]))
				return false;
			attTables[0].clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
			else
				TeProgress::instance()->setProgress(j+1);
			}
		}
	}
	if (points.size() > 0)
	{
		if(!layer->addPoints(points))
			return false;
		points.clear();
	}
	if (attTables[0].size() > 0)
	{
		if(!layer->saveAttributeTable(attTables[0]))
			return false;
		attTables[0].clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	if (!layer->database()->commitTransaction())
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TePOINTS)
	{
		if(!layer->database()->insertMetadata(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), 0.0005,0.0005,layer->box()))
			return false;
		if(!layer->database()->createSpatialIndex(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), (TeSpatialIndexType)TeRTREE))
			return false;
	}

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

bool TeGTM::importTracklogs(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize)
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}

	if (!layer->database()->beginTransaction())
		return false;

	int linkCol = -1;
    unsigned int j = 0;

	//Recuperando lista de atributos
	TeAttributeList attList;
	tracklogAttributeList(attList);

	bool attributeTableCreated = createAttributeTable(layer, attList, attrTableName, objectIdAtt, oldPrec, linkCol);
	if(attributeTableCreated == false)
		return false;

	string objid, value;
	TePointSet points;
	TeTableRow row;
	TeAttrTableVector attTables;

	int nEntities = header_.getNtracks();
    if(TeProgress::instance())
    {
        TeProgress::instance()->reset();
    }
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	for (j = 0; j < nEntities; j++)
	{
		//Preenchendo a linha da tabela de atributos
		row.clear();
		row.push_back(getDate(tracklogs_[j]->getTdate()));
		row.push_back(string(1,tracklogs_[j]->getTnum()));
		row.push_back(Te2String(tracklogs_[j]->getTalt()));

		//Criando indice automatico
		row.push_back(Te2String(j));

		if (row[linkCol].empty())
			row[linkCol] = "te_nulo";
		objid = row[linkCol];

		//Pegando a tabela de atributos criada pelo metodo createAttributeTable
		layer->getAttrTables(attTables, TeAttrStatic);
		attTables[0].add(row);

		//Criando o TePoint
		TePoint point(tracklogs_[j]->getLongitude(), tracklogs_[j]->getLatitude());
		point.objectId(objid);
		points.add(point);

		if ((j%chunkSize) == 0)
		{
			if(!layer->addPoints(points))
				return false;
			points.clear();
			if(!layer->saveAttributeTable(attTables[0]))
				return false;
			attTables[0].clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
			else
				TeProgress::instance()->setProgress(j+1);
			}
		}
	}
	if (points.size() > 0)
	{
		if(!layer->addPoints(points))
			return false;
		points.clear();
	}
	if (attTables[0].size() > 0)
	{
		if(!layer->saveAttributeTable(attTables[0]))
			return false;
		attTables[0].clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	if (!layer->database()->commitTransaction())
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TePOINTS)
	{
		if(!layer->database()->insertMetadata(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), 0.0005,0.0005,layer->box()))
			return false;
		if(!layer->database()->createSpatialIndex(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), (TeSpatialIndexType)TeRTREE))
			return false;
	}

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

bool TeGTM::importTracklogs2(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize)
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}

	if (!layer->database()->beginTransaction())
		return false;

	int linkCol = -1;
    unsigned int j = 0;

	//Recuperando lista de atributos
	TeAttributeList attList;
	TeAttribute attribute;
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.numChar_ = 255;		
	attribute.rep_.name_ = "NOME";
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back(attribute);

	bool attributeTableCreated = createAttributeTable(layer, attList, attrTableName, objectIdAtt, oldPrec, linkCol);
	if(attributeTableCreated == false)
		return false;

	string objid, value;
	TeLineSet lines;
	TeTableRow row;
	TeAttrTableVector attTables;

	int nEntities = header_.getNtracks();
    if(TeProgress::instance())
    {
        TeProgress::instance()->reset();
    }
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	//contador de tracklogs
	int count = 0;
	//tracklog lines
	TeLine2D line = TeLine2D();
	for (j = 0; j < nEntities; j++)
	{
		//Criando as linhas dos tracklogs*********************************************
		TeCoord2D coord(tracklogs_[j]->getLongitude(), tracklogs_[j]->getLatitude());
		int tnum = tracklogs_[j]->getTnum();
		if((tnum == 1) || (j == nEntities - 1))
		{
			if(line.size() > 0)
			{
				//Preenchendo a linha da tabela de atributos
				row.clear();
				row.push_back(tracklogstyles_[count]->getTname());

				//Criando indice automatico
				row.push_back(Te2String(count));

				if (row[linkCol].empty())
					row[linkCol] = "te_nulo";
				objid = row[linkCol];

				//Pegando a tabela de atributos criada pelo metodo createAttributeTable
				layer->getAttrTables(attTables, TeAttrStatic);
				attTables[0].add(row);

				line.objectId(objid);
				lines.add(line);

				if ((count%chunkSize) == 0)
				{
					if(!layer->addLines(lines))
						return false;
					lines.clear();
					if(!layer->saveAttributeTable(attTables[0]))
						return false;
					attTables[0].clear();
					if(TeProgress::instance())
					{
						if (TeProgress::instance()->wasCancelled())
							break;
					else
						TeProgress::instance()->setProgress(j+1);
					}
				}
				count++;
			}
			line = TeLine2D();
		}
		line.add(coord);
		//***********************************************************************************
	}
	if (lines.size() > 0)
	{
		if(!layer->addLines(lines))
			return false;
		lines.clear();
	}
	if (attTables[0].size() > 0)
	{
		if(!layer->saveAttributeTable(attTables[0]))
			return false;
		attTables[0].clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	if (!layer->database()->commitTransaction())
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TeLINES)
	{
		if(!layer->database()->insertMetadata(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), 0.0005,0.0005,layer->box()))
			return false;
		if(!layer->database()->createSpatialIndex(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), (TeSpatialIndexType)TeRTREE))
			return false;
	}

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

bool TeGTM::importRoutes(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize)
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}

	if (!layer->database()->beginTransaction())
		return false;

	int linkCol = -1;
    unsigned int j = 0;

	//Recuperando lista de atributos
	TeAttributeList attList;
	routeAttributeList(attList);

	bool attributeTableCreated = createAttributeTable(layer, attList, attrTableName, objectIdAtt, oldPrec, linkCol);
	if(attributeTableCreated == false)
		return false;

	string objid, value;
	TePointSet points;
	TeTableRow row;
	TeAttrTableVector attTables;

	int nEntities = header_.getNrtes();
    if(TeProgress::instance())
    {
        TeProgress::instance()->reset();
    }
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	for (j = 0; j < nEntities; j++)
	{
		//Preenchendo a linha da tabela de atributos
		row.clear();
		row.push_back(routes_[j]->getWname());
		row.push_back(routes_[j]->getWcomment());
		row.push_back(routes_[j]->getRname());
		row.push_back(icon_descr[routes_[j]->getIco()]);
		row.push_back(string(1,routes_[j]->getDspl()));
		row.push_back(string(1,routes_[j]->getTnum()));
		row.push_back(getDate(routes_[j]->getTdate()));
		row.push_back(Te2String(routes_[j]->getWrot()));
		row.push_back(Te2String(routes_[j]->getWalt()));
		row.push_back(Te2String(routes_[j]->getWlayer()));

		//Criando indice automatico
		row.push_back(Te2String(j));

		if (row[linkCol].empty())
			row[linkCol] = "te_nulo";
		objid = row[linkCol];

		//Pegando a tabela de atributos criada pelo metodo createAttributeTable
		layer->getAttrTables(attTables, TeAttrStatic);
		attTables[0].add(row);

		//Criando o TePoint
		TePoint point(routes_[j]->getLongitude(), routes_[j]->getLatitude());
		point.objectId(objid);
		points.add(point);

		if ((j%chunkSize) == 0)
		{
			if(!layer->addPoints(points))
				return false;
			points.clear();
			if(!layer->saveAttributeTable(attTables[0]))
				return false;
			attTables[0].clear();
			if(TeProgress::instance())
			{
				if (TeProgress::instance()->wasCancelled())
					break;
			else
				TeProgress::instance()->setProgress(j+1);
			}
		}
	}
	if (points.size() > 0)
	{
		if(!layer->addPoints(points))
			return false;
		points.clear();
	}
	if (attTables[0].size() > 0)
	{
		if(!layer->saveAttributeTable(attTables[0]))
			return false;
		attTables[0].clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	if (!layer->database()->commitTransaction())
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TePOINTS)
	{
		if(!layer->database()->insertMetadata(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), 0.0005,0.0005,layer->box()))
			return false;
		if(!layer->database()->createSpatialIndex(layer->tableName(TePOINTS),layer->database()->getSpatialIdxColumn(TePOINTS), (TeSpatialIndexType)TeRTREE))
			return false;
	}

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

bool TeGTM::importRoutes2(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize)
{
	double oldPrec = TePrecision::instance().precision();

	if(layer)
	{
		TePrecision::instance().setPrecision(TeGetPrecision(layer->projection()));
	}
	else
	{
		TePrecision::instance().setPrecision(TeGetPrecision(0));
	}

	if (!layer->database()->beginTransaction())
		return false;

	int linkCol = -1;
    unsigned int j = 0;

	//Recuperando lista de atributos
	TeAttributeList attList;
	TeAttribute attribute;
	attribute.rep_.type_ = TeSTRING;
	attribute.rep_.numChar_ = 255;		
	attribute.rep_.name_ = "NOME";
	attribute.rep_.isPrimaryKey_ = false;
	attList.push_back(attribute);

	bool attributeTableCreated = createAttributeTable(layer, attList, attrTableName, objectIdAtt, oldPrec, linkCol);
	if(attributeTableCreated == false)
		return false;

	string objid, value;
	TeLineSet lines;
	TeTableRow row;
	TeAttrTableVector attTables;

	int nEntities = header_.getNrtes();
    if(TeProgress::instance())
    {
        TeProgress::instance()->reset();
    }
	if(TeProgress::instance())
	{
		TeProgress::instance()->setTotalSteps(nEntities);
		TeProgress::instance()->setProgress(j+1);
	}

	//contador de routes
	int count = 0;
	//route lines
	TeLine2D line = TeLine2D();
	for (j = 0; j < nEntities; j++)
	{
		//Criando as linhas das rotas************************************************
		TeCoord2D coord(routes_[j]->getLongitude(), routes_[j]->getLatitude());
		int tnum = routes_[j]->getTnum();
		if((tnum == 1) || (j == nEntities - 1))
		{
			if(line.size() > 0)
			{
				//Preenchendo a linha da tabela de atributos
				row.clear();
				row.push_back(routes_[j]->getRname());

				//Criando indice automatico
				row.push_back(Te2String(count));

				if (row[linkCol].empty())
					row[linkCol] = "te_nulo";
				objid = row[linkCol];

				//Pegando a tabela de atributos criada pelo metodo createAttributeTable
				layer->getAttrTables(attTables, TeAttrStatic);
				attTables[0].add(row);

				line.objectId(objid);
				lines.add(line);

				if ((count%chunkSize) == 0)
				{
					if(!layer->addLines(lines))
						return false;
					lines.clear();
					if(!layer->saveAttributeTable(attTables[0]))
						return false;
					attTables[0].clear();
					if(TeProgress::instance())
					{
						if (TeProgress::instance()->wasCancelled())
							break;
					else
						TeProgress::instance()->setProgress(j+1);
					}
				}
				count++;
			}
			line = TeLine2D();
		}
		line.add(coord);
		//**************************************************************************************
	}
	if (lines.size() > 0)
	{
		if(!layer->addLines(lines))
			return false;
		lines.clear();
	}
	if (attTables[0].size() > 0)
	{
		if(!layer->saveAttributeTable(attTables[0]))
			return false;
		attTables[0].clear();
	}
	if(TeProgress::instance())
		TeProgress::instance()->cancel();

	if (!layer->database()->commitTransaction())
		return false;

	// Create the spatial indexes
	int rep = layer->geomRep();
	if (rep & TeLINES)
	{
		if(!layer->database()->insertMetadata(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), 0.0005,0.0005,layer->box()))
			return false;
		if(!layer->database()->createSpatialIndex(layer->tableName(TeLINES),layer->database()->getSpatialIdxColumn(TeLINES), (TeSpatialIndexType)TeRTREE))
			return false;
	}

	TePrecision::instance().setPrecision(oldPrec);

	return true;
}

int TeGTM::importImages(TeLayer* layer)
{
	int count = 0;
	for(unsigned int i = 0; i < informationimages_.size(); i++)
	{
		//Criando a imagem temporariamente em arquivo
		std::string nome = "./";
		nome += informationimages_[i]->getName_map();
		std::string base = TeGetExtension(nome.c_str());
		if(base == "")
			continue;
		FILE* temp = fopen(nome.c_str(), "wb");
		fwrite(informationimages_[i]->getBlob(),informationimages_[i]->getImagelen(),1,temp);
		fclose(temp);

		// Acess input image
		TeInitRasterDecoders();
		TeRaster image(nome.c_str());
		if(image.init())
		{
			TeRasterParams& par = image.params();
			par.boundingBoxLinesColumns(informationimages_[i]->getGposx(),informationimages_[i]->getGposy()-informationimages_[i]->getGheight(),
				informationimages_[i]->getGposx()+informationimages_[i]->getGwidth(),informationimages_[i]->getGposy(),
				par.nlines_, par.ncols_);
			par.projection(getProjection());
			if(!TeImportRaster(layer,&image,512,512,TeRasterParams::TeNoCompression,Te2String(i),255,true,TeRasterParams::TeNoExpansible))
				continue;
			count++;
		}
		//removendo o arquivo temporario
		remove(nome.c_str());
	}
	return count;
}

bool TeGTM::createAttributeTable(TeLayer* layer, TeAttributeList attList, std::string attrTableName, std::string objectIdAtt, 
								 double oldPrec, int& linkCol)
{
	unsigned int j = 0;
	int count = 0;

	// Como nao existem indices no GTM, vamos criar um padrao
	objectIdAtt = "object_id_" + Te2String(layer->id());
	string s2 = TeConvertToUpperCase(objectIdAtt);
	while (true)
	{
		for (j = 0; j < attList.size(); j++)
		{
			string s0 = attList[j].rep_.name_;
			string s1 = TeConvertToUpperCase(s0);
			if ( s1 == s2)
				break;
		}
		if (j < attList.size())
		{
			++count;
			objectIdAtt = "object_id_" + Te2String(count);
		}
		else
			break;
	}
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 16;
	at.rep_.name_ = objectIdAtt;
	at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);
	linkCol = attList.size()-1;

	TeTable attTable (attrTableName,attList,objectIdAtt,objectIdAtt,TeAttrStatic);
	//Criando a tabela no banco
	if (!layer->createAttributeTable(attTable))
	{
		layer->database()->rollbackTransaction();
		TePrecision::instance().setPrecision(oldPrec);
		return false;
	}
	return true;
}

void TeGTM::exportGTM(TeTheme *theme, std::string fileName, FILE* file)
{
	waypoints_.clear();
	waypointsstyles_.clear();
	tracklogs_.clear();
	tracklogstyles_.clear();
	//Capturando banco e tipo de representacao
	TeDatabase *db = theme->layer()->database();
	int rep = theme->layer()->geomRep();

	//Convertendo o mbr do tema para projecao LatLong
	TeProjection* proj = theme->layer()->projection();
	TeBox box = theme->layer()->box();
	TeBox mbr = TeBox();
	if(proj->name() != "LatLong")
	{
		mbr.lowerLeft() = proj->PC2LL(box.lowerLeft());
		mbr.upperRight() = proj->PC2LL(box.upperRight());
	}
	else
	{
		mbr = TeBox(box);
	}

	TePointSet pointSet;
	TeLineSet lineSet;
	//Populando os objetos GTM
	std::string fontName = "Times New Roman";
	std::string code = "TrackMaker";
	std::string vazio = "";
	if(rep & TePOINTS)
	{
		theme->layer()->getPoints(pointSet);
		for(unsigned int i = 0; i < pointSet.size(); i++)
		{
			//Convertendo o pointSet do tema para projecao LatLong
			TeCoord2D coord;
			if(proj->name() != "LatLong")
				coord = proj->PC2LL(pointSet[i].location());
			else
				coord = pointSet[i].location();
			TeGTMWaypoints* pt = new TeGTMWaypoints();
			pt->setLatitude(coord.y());
			pt->setLongitude(coord.x());
			pt->setDspl(1);
			pt->setIco(48);
			//*****************************
			std::string valor = "         ";
			char* nome = new char[10];
			strcpy(nome, valor.c_str());
			pt->setName(nome);
			//*****************************
			pt->setWalt(0.);
			pt->setWdate(0.);
			pt->setWlayer(0);
			pt->setWname((char*)vazio.c_str());
			pt->setWrot(0);
			waypoints_.push_back(pt);
		}
			//Precisam ser gravado no minimo 4 estilos
			//Estilo 1
			TeGTMWaypointsStyles* pt1 = new TeGTMWaypointsStyles();
			pt1->setAlignment(0);
			pt1->setBackcolor(0);
			pt1->setBackground(false);
			pt1->setBorder(0);
			pt1->setColor(0);
			pt1->setDspl(0);
			pt1->setFacename((char*)fontName.c_str());
			pt1->setHeight(-11);
			pt1->setItalic(0);
			pt1->setScale1(0);
			pt1->setStrikeout(0);
			pt1->setunderline(0);
			pt1->setWeight(400);
			waypointsstyles_.push_back(pt1);
			//Estilo 2
			TeGTMWaypointsStyles* pt2 = new TeGTMWaypointsStyles();
			pt2->setAlignment(0);
			pt2->setBackcolor(0);
			pt2->setBackground(false);
			pt2->setBorder(0);
			pt2->setColor(0);
			pt2->setDspl(1);
			pt2->setFacename((char*)fontName.c_str());
			pt2->setHeight(-11);
			pt2->setItalic(0);
			pt2->setScale1(0);
			pt2->setStrikeout(0);
			pt2->setunderline(0);
			pt2->setWeight(400);
			waypointsstyles_.push_back(pt2);
			//Estilo 3
			TeGTMWaypointsStyles* pt3 = new TeGTMWaypointsStyles();
			pt3->setAlignment(0);
			pt3->setBackcolor(0);
			pt3->setBackground(false);
			pt3->setBorder(0);
			pt3->setColor(0);
			pt3->setDspl(2);
			pt3->setFacename((char*)fontName.c_str());
			pt3->setHeight(-11);
			pt3->setItalic(0);
			pt3->setScale1(0);
			pt3->setStrikeout(0);
			pt3->setunderline(0);
			pt3->setWeight(400);
			waypointsstyles_.push_back(pt3);
			//Estilo 4
			TeGTMWaypointsStyles* pt4 = new TeGTMWaypointsStyles();
			pt4->setAlignment(1);
			pt4->setBackcolor(65535);
			pt4->setBackground(true);
			pt4->setBorder(139);
			pt4->setColor(0);
			pt4->setDspl(3);
			pt4->setFacename((char*)fontName.c_str());
			pt4->setHeight(-11);
			pt4->setItalic(0);
			pt4->setScale1(0);
			pt4->setStrikeout(0);
			pt4->setunderline(0);
			pt4->setWeight(400);
			waypointsstyles_.push_back(pt4);
	}
	if(rep & TeLINES)
	{
		theme->layer()->getLines(lineSet);
		for(unsigned int i = 0; i < lineSet.size(); i++)
		{
			TeLine2D line = lineSet[i];
			for(unsigned int j = 0; j < line.size(); j++)
			{
				//Convertendo o lineSet do tema para projecao LatLong
				TeCoord2D coord;
				if(proj->name() != "LatLong")
					coord = proj->PC2LL(line[j]);
				else
					coord = line[j];
				TeGTMTrackLogs* pt = new TeGTMTrackLogs();
				pt->setLatitude(coord.y());
				pt->setLongitude(coord.x());
				pt->setTalt(0.);
				pt->setTdate(0);
				if(j == 0)
				{
					pt->setTnum(1);

					TeGTMTrackLogStyles* pt1 = new TeGTMTrackLogStyles();
					pt1->setTcolor(0);
					pt1->setTlabel(0);
					pt1->setTlayer(0);
					pt1->setTname((char*)vazio.c_str());
					pt1->setTscale(0);
					pt1->setTtype(1);
					tracklogstyles_.push_back(pt1);
				}
				else
					pt->setTnum(0);
				tracklogs_.push_back(pt);
			}
		}
	}

	//Populando dados do cabeçalho
	header_.setBcolor(16777215);
	header_.setCode((char*)code.c_str());
	header_.setCoriml(0);
	header_.setGradcolor(0);
	header_.setGradfont((char*)fontName.c_str());
	header_.setGradnum(8);
	header_.setIconnum(0);
	header_.setLabelcolor(0);
	header_.setlabelFont((char*)fontName.c_str());
	header_.setLabelsize(10);
	header_.setLayers(0);
	header_.setMap(false);
	if(mbr.lowerLeft().y() > mbr.upperRight().y())
	{
		header_.setMaxlat(mbr.lowerLeft().y());
		header_.setMinlat(mbr.upperRight().y());
	}
	else
	{
		header_.setMaxlat(mbr.upperRight().y());
		header_.setMinlat(mbr.lowerLeft().y());
	}
	header_.setMaxlon(mbr.upperRight().x());
	header_.setMinlon(mbr.lowerLeft().x());
	header_.setN_maps(0);
	header_.setN_tk(tracklogstyles_.size());
	header_.setNdatum(0);
	header_.setNewdatum((char*)vazio.c_str());
	header_.setNrtes(routes_.size());
	header_.setNtracks(tracklogs_.size());
	header_.setNwpts(waypoints_.size());
	header_.setNwptstyles(waypointsstyles_.size());
	header_.setRectangular(false);
	header_.setTruegrid(false);
	header_.setUsercolor(0);
	header_.setUserfont((char*)vazio.c_str());
	header_.setUseritalic(false);
	header_.setUsernegrit(false);
	header_.setUsernum(0);
	header_.setUsersublin(false);
	header_.setUsertachado(false);
	header_.setVersion(211);
	header_.setVwt(0);
	header_.setWli(0);
	header_.setWptnum(0);
	//Escrevendo no arquivo
	header_.writeGTMHeader(file);
	header_.setCode(0);
	header_.setGradfont(0);
	header_.setlabelFont(0);
	header_.setNewdatum(0);
	header_.setUserfont(0);

	//Populando dados de grid e datum
	griddatum_.setAxis(proj->datum().radius());
	griddatum_.setDx(0);
	griddatum_.setDy(0);
	griddatum_.setDz(0);
	griddatum_.setFalseeast(0);
	griddatum_.setFalsenorthing(0);
	griddatum_.setFlattening(proj->datum().flattening());
	griddatum_.setNdatum(setDatum(proj->datum()));
	griddatum_.setNgrid(0);
	griddatum_.setOrigin(0);
	griddatum_.setScale1(0);
	//Escrevendo no arquivo
	griddatum_.writeGTMGridDatum(file);

	//Escrevendo waypoints e waypointsStyles
	for(unsigned int i = 0; i < waypoints_.size(); i++)
	{
		waypoints_.at(i)->writeGTMWaypoints(file);
		waypoints_.at(i)->setWname(0);
	}
	for(unsigned int i = 0; i < waypointsstyles_.size(); i++)
	{
		waypointsstyles_.at(i)->writeGTMWaypointsStyles(file);
		waypointsstyles_.at(i)->setFacename(0);
	}
	//Escrevendo tracklogs e tracklogStyles
	for(unsigned int i = 0; i < tracklogs_.size(); i++)
	{
		tracklogs_.at(i)->writeGTMTrackLogs(file);
	}
	for(unsigned int i = 0; i < tracklogstyles_.size(); i++)
	{
		tracklogstyles_.at(i)->writeGTMTrackLogStyles(file);
		tracklogstyles_.at(i)->setTname(0);
	}
	fclose(file);
}

std::vector<TeLayer*> TeGTM::importGTM(TeDatabase *db, std::string fileName)
{
	std::vector<TeLayer*> layers;

	if(header_.getN_maps() > 0)
	{
		if(!db->layerExist(fileName+"_Images"))
		{
			TeBox box = TeBox(header_.getMinlon(), header_.getMinlat(), header_.getMaxlon(), header_.getMaxlat()); 
			TeLayer* layer = new TeLayer(fileName+"_Images", db, box, getProjection());
			int count = importImages(layer);
			if(count > 0)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_Images already exists!");
	}
	if(header_.getNwpts() > 0)
	{
		if(!db->layerExist(fileName+"_Waypoints"))
		{
			TeLayer* layer = new TeLayer(fileName+"_Waypoints", db, getProjection());
			bool ok = importWaypoints(layer, layer->name(),"",100);
			if(ok)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_Waypoints already exists!");
	}
	if(header_.getNtracks() > 0)
	{
		if(!db->layerExist(fileName+"_TrackLogs_P"))
		{
			TeLayer* layer = new TeLayer(fileName+"_TrackLogs_P", db, getProjection());
			bool ok = importTracklogs(layer, layer->name(),"",100);
			if(ok)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_TrackLogs_P already exists!");

		if(!db->layerExist(fileName+"_TrackLogs_L"))
		{
			TeLayer* layer = new TeLayer(fileName+"_TrackLogs_L", db, getProjection());
			bool ok = importTracklogs2(layer, layer->name(),"",100);
			if(ok)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_TrackLogs_L already exists!");
	}
	if(header_.getNrtes() > 0)
	{
		if(!db->layerExist(fileName+"_Routes_P"))
		{
			TeLayer* layer = new TeLayer(fileName+"_Routes_P", db, getProjection());
			bool ok = importRoutes(layer, layer->name(),"",100);
			if(ok)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_Routes_P already exists!");

		if(!db->layerExist(fileName+"_Routes_L"))
		{
			TeLayer* layer = new TeLayer(fileName+"_Routes_L", db, getProjection());
			bool ok = importRoutes2(layer, layer->name(),"",100);
			if(ok)
				layers.push_back(layer);
			else
				db->deleteLayer(layer->id());
		}
		else
			throw TeException(UNKNOWN_ERROR_TYPE, "Layer "+fileName+"_Routes_L already exists!");
	}
	return layers;
}

std::string TeGTM::getLayerNames(std::string fileName)
{
	layersNames_ = "";
	if(header_.getN_maps() > 0)
	{
		layersNames_ += fileName+"_Images,";
	}
	if(header_.getNwpts() > 0)
	{
		layersNames_ += fileName+"_Waypoints,";
	}
	if(header_.getNtracks() > 0)
	{
		layersNames_ += fileName+"_TrackLogs,";
	}
	if(header_.getNrtes() > 0)
	{
		layersNames_ += fileName+"_Routes,";
	}

	return layersNames_;
}

TeProjection* TeGTM::getProjection()
{
	TeProjectionParams par;
	par.name = "LatLong";
	par.units = "DecimalDegrees";
	par.datum = getDatum();
	return TeProjectionFactory::make(par);
}

TeDatum TeGTM::getDatum()
{
	string name;
	short ndatum = griddatum_.getNdatum();

												//Datum suportado pela Terralib
	if (ndatum == 46)							{ name = "Astro-Chua";}
	else if (ndatum == 47)						{ name = "CorregoAlegre";}
	else if (ndatum == 88)						{ name = "Indian";}
	else if (ndatum == 123)						{ name = "NAD27";}
	else if (ndatum == 139)						{ name = "NAD83";}
	else if (ndatum == 196 || ndatum == 231)	{ name = "SAD69";}
	else if (ndatum == 217)						{ name = "WGS84";}
	else										{ name = "";}

	if(!name.empty())
		return TeDatumFactory::make(name);
	else
		throw TeException(DATUM_ERROR, "Datum not supported!");
}

TeProjection* TeGTM::setProjection(TeProjection* proj)
{
	TeProjectionParams par;
	par.name = "LatLong";
	par.units = "DecimalDegrees";
	par.datum = proj->datum();
	return TeProjectionFactory::make(par);
}

short TeGTM::setDatum(TeDatum datum)
{
	short ndatum;

	if		(datum.name() == "Astro-Chua")					{ ndatum = 46;}
	else if (datum.name() == "CorregoAlegre")				{ ndatum = 47;}
	else if (datum.name() == "Indian")						{ ndatum = 88;}
	else if (datum.name() == "NAD27")						{ ndatum = 123;}
	else if (datum.name() == "NAD83")						{ ndatum = 139;}
	else if (datum.name() == "SAD69")						{ ndatum = 196;}
	else if (datum.name() == "WGS84")						{ ndatum = 217;}
	else													{ ndatum = 217;}

	return ndatum;
}

//Cabecalho do arquivo
TeGTMHeader::TeGTMHeader()
{
	code_ = NULL;
	gradfont_ = NULL;
	labelfont_ = NULL;
	userfont_ = NULL;
	newdatum_ = NULL;
}

TeGTMHeader::~TeGTMHeader()
{
	if(code_ != NULL)
		delete code_;
	if(gradfont_ != NULL)
		delete gradfont_;
	if(labelfont_ != NULL)
		delete labelfont_;
	if(userfont_ != NULL)
		delete userfont_;
	if(newdatum_ != NULL)
		delete newdatum_;
}

void TeGTMHeader::readGTMHeader(FILE* file)
{
	if(file != NULL)
		rewind(file);
	fread(&version_, 2, 1, file);

	if(version_ != 211)
		throw TeException(UNKNOWN_ERROR_TYPE, "Version not supported!");

	code_ = new char[11];
	fread(code_, 10, 1, file);
	code_[10] = '\0';

	fread(&wli_, 1, 1, file);
	fread(&vwt_, 1, 1, file);
	fread(&gradnum_, 1, 1, file);
	fread(&wptnum_, 1, 1, file);
	fread(&usernum_, 1, 1, file);
	fread(&coriml_, 1, 1, file);
	fread(&ndatum_, 1, 1, file);

	fread(&gradcolor_, 4, 1, file);
	fread(&bcolor_, 4, 1, file);
	fread(&nwptstyles_, 4, 1, file);
	fread(&usercolor_, 4, 1, file);
	fread(&nwpts_, 4, 1, file);
	fread(&ntrcks_, 4, 1, file);
	fread(&nrtes_, 4, 1, file);

	fread(&maxlon_, 4, 1, file);
	fread(&minlon_, 4, 1, file);
	fread(&maxlat_, 4, 1, file);
	fread(&minlat_, 4, 1, file);

	fread(&n_maps_, 4, 1, file);
	fread(&n_tk_, 4, 1, file);

	fread(&layers_, 4, 1, file);
	fread(&iconnum_, 4, 1, file);

	fread(&rectangular_, 2, 1, file);
	fread(&truegrid_, 2, 1, file);

	fread(&labelcolor_, 4, 1, file);

	fread(&usernegrit_, 2, 1, file);
	fread(&useritalic_, 2, 1, file);
	fread(&usersublin_, 2, 1, file);
	fread(&usertachado_, 2, 1, file);
	fread(&map_, 2, 1, file);

	fread(&labelsize_, 2, 1, file);

	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	gradfont_ = new char[aux+1];
	fread(gradfont_, (size_t)aux, 1, file);
	gradfont_[aux] = '\0';

	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	labelfont_ = new char[aux+1];
	fread(labelfont_, (size_t)aux, 1, file);
	labelfont_[aux] = '\0';

	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	userfont_ = new char[aux+1];
	fread(userfont_, (size_t)aux, 1, file);
	userfont_[aux] = '\0';

	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	newdatum_ = new char[aux+1];
	fread(newdatum_, (size_t)aux, 1, file);
	newdatum_[aux] = '\0';
}

void TeGTMHeader::writeGTMHeader(FILE* file)
{
	if(file != NULL)
		rewind(file);
	fwrite(&version_, 2, 1, file);

	fwrite(code_, 10, 1, file);

	fwrite(&wli_, 1, 1, file);
	fwrite(&vwt_, 1, 1, file);
	fwrite(&gradnum_, 1, 1, file);
	fwrite(&wptnum_, 1, 1, file);
	fwrite(&usernum_, 1, 1, file);
	fwrite(&coriml_, 1, 1, file);
	fwrite(&ndatum_, 1, 1, file);

	fwrite(&gradcolor_, 4, 1, file);
	fwrite(&bcolor_, 4, 1, file);
	fwrite(&nwptstyles_, 4, 1, file);
	fwrite(&usercolor_, 4, 1, file);
	fwrite(&nwpts_, 4, 1, file);
	fwrite(&ntrcks_, 4, 1, file);
	fwrite(&nrtes_, 4, 1, file);

	fwrite(&maxlon_, 4, 1, file);
	fwrite(&minlon_, 4, 1, file);
	fwrite(&maxlat_, 4, 1, file);
	fwrite(&minlat_, 4, 1, file);

	fwrite(&n_maps_, 4, 1, file);
	fwrite(&n_tk_, 4, 1, file);

	fwrite(&layers_, 4, 1, file);
	fwrite(&iconnum_, 4, 1, file);

	fwrite(&rectangular_, 2, 1, file);
	fwrite(&truegrid_, 2, 1, file);

	fwrite(&labelcolor_, 4, 1, file);

	fwrite(&usernegrit_, 2, 1, file);
	fwrite(&useritalic_, 2, 1, file);
	fwrite(&usersublin_, 2, 1, file);
	fwrite(&usertachado_, 2, 1, file);
	fwrite(&map_, 2, 1, file);

	fwrite(&labelsize_, 2, 1, file);

	int i = strlen(gradfont_);
	fwrite(&i, 2, 1, file);
	fwrite(gradfont_, i, 1, file);

	i = strlen(labelfont_);
	fwrite(&i, 2, 1, file);
	fwrite(labelfont_, i, 1, file);

	i = strlen(userfont_);
	fwrite(&i, 2, 1, file);
	fwrite(userfont_, i, 1, file);

	i = strlen(newdatum_);
	fwrite(&i, 2, 1, file);
	fwrite(newdatum_, i, 1, file);
}

//Classe com informacoes do datum utilizado
TeGTMGridDatum::TeGTMGridDatum()
{
}

TeGTMGridDatum::~TeGTMGridDatum()
{
}

void TeGTMGridDatum::readGTMGridDatum(FILE* file)
{
	fread(&ngrid_, 2, 1, file);

	fread(&origin_, 8, 1, file);
	fread(&falseeast_, 8, 1, file);
	fread(&scale1_, 8, 1, file);
	fread(&falsenorthing_, 8, 1, file);

	fread(&ndatum_, 2, 1, file);

	fread(&axis_, 8, 1, file);
	fread(&flattening_, 8, 1, file);

	fread(&dx_, 2, 1, file);
	fread(&dy_, 2, 1, file);
	fread(&dz_, 2, 1, file);
}

void TeGTMGridDatum::writeGTMGridDatum(FILE* file)
{
	fwrite(&ngrid_, 2, 1, file);

	fwrite(&origin_, 8, 1, file);
	fwrite(&falseeast_, 8, 1, file);
	fwrite(&scale1_, 8, 1, file);
	fwrite(&falsenorthing_, 8, 1, file);

	fwrite(&ndatum_, 2, 1, file);

	fwrite(&axis_, 8, 1, file);
	fwrite(&flattening_, 8, 1, file);

	fwrite(&dx_, 2, 1, file);
	fwrite(&dy_, 2, 1, file);
	fwrite(&dz_, 2, 1, file);
}

//Classe com informacoes das imagens existentes no arquivo
TeGTMInformationImages::TeGTMInformationImages()
{
	name_map_ = NULL;
	comments_ = NULL;
	blob_ = NULL;
}

TeGTMInformationImages::~TeGTMInformationImages()
{
	if(name_map_ != NULL)
		delete name_map_;
	if(comments_ != NULL)
		delete comments_;
	if(blob_ != NULL)
		delete blob_;
}

void TeGTMInformationImages::readGTMInformationImages(FILE* file)
{
	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	name_map_ = new char[aux+1];
	fread(name_map_, (size_t)aux, 1, file);
	name_map_[aux] = '\0';

	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	comments_ = new char[aux+1];
	fread(comments_, (size_t)aux, 1, file);
	comments_[aux] = '\0';

	fread(&gposx_, 4, 1, file);
	fread(&gposy_, 4, 1, file);
	fread(&gwidth_, 4, 1, file);
	fread(&gheight_, 4, 1, file);

	fread(&imagelen_, 4, 1, file);

	fread(&metax_, 4, 1, file);
	fread(&metay_, 4, 1, file);

	fread(&metamapa_, 1, 1, file);
	fread(&tnum_, 1, 1, file);
}

//Classe de waypoints
TeGTMWaypoints::TeGTMWaypoints()
{
	name_ = NULL;
	wname_ = NULL;
}

TeGTMWaypoints::~TeGTMWaypoints()
{
	if(name_ != NULL)
		delete name_;
	if(wname_ != NULL)
		delete wname_;
}

void TeGTMWaypoints::readGTMWaypoints(FILE* file)
{
	fread(&latitude_, 8, 1, file);
	fread(&longitude_, 8, 1, file);

	name_ = new char[11];
	fread(name_, 10, 1, file);
	name_[10] = '\0';

	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	wname_ = new char[aux+1];
	fread(wname_, (size_t)aux, 1, file);
	wname_[aux] = '\0';

	fread(&ico_, 2, 1, file);

	fread(&dspl_, 1, 1, file);

	fread(&wdate_, 4, 1, file);

	fread(&wrot_, 2, 1, file);

	fread(&walt_, 4, 1, file);

	fread(&wlayer_, 2, 1, file);
}

void TeGTMWaypoints::writeGTMWaypoints(FILE* file)
{
	fwrite(&latitude_, 8, 1, file);
	fwrite(&longitude_, 8, 1, file);

	fwrite(name_, 10, 1, file);

	int i = strlen(wname_);
	fwrite(&i, 2, 1, file);
	fwrite(wname_, i, 1, file);

	fwrite(&ico_, 2, 1, file);

	fwrite(&dspl_, 1, 1, file);

	fwrite(&wdate_, 4, 1, file);

	fwrite(&wrot_, 2, 1, file);

	fwrite(&walt_, 4, 1, file);

	fwrite(&wlayer_, 2, 1, file);
}

//Classe de waypoints
TeGTMWaypointsStyles::TeGTMWaypointsStyles()
{
	facename_ = NULL;
}

TeGTMWaypointsStyles::~TeGTMWaypointsStyles()
{
	if(facename_ != NULL)
		delete facename_;
}

void TeGTMWaypointsStyles::readGTMWaypointsStyles(FILE* file)
{
	fread(&height_, 4, 1, file);

	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	facename_ = new char[aux+1];
	fread(facename_, (size_t)aux, 1, file);
	facename_[aux] = '\0';

	fread(&dspl_, 1, 1, file);

	fread(&color_, 4, 1, file);
	fread(&weight_, 4, 1, file);

	fread(&scale1_, 4, 1, file);

	fread(&border_, 1, 1, file);

	fread(&background_, 2, 1, file);

	fread(&backcolor_, 4, 1, file);

	fread(&italic_, 1, 1, file);
	fread(&underline_, 1, 1, file);
	fread(&strikeout_, 1, 1, file);
	fread(&alignment_, 1, 1, file);
}

void TeGTMWaypointsStyles::writeGTMWaypointsStyles(FILE* file)
{
	fwrite(&height_, 4, 1, file);

	int i = strlen(facename_);
	fwrite(&i, 2, 1, file);
	fwrite(facename_, i, 1, file);

	fwrite(&dspl_, 1, 1, file);

	fwrite(&color_, 4, 1, file);
	fwrite(&weight_, 4, 1, file);

	fwrite(&scale1_, 4, 1, file);

	fwrite(&border_, 1, 1, file);

	fwrite(&background_, 2, 1, file);

	fwrite(&backcolor_, 4, 1, file);

	fwrite(&italic_, 1, 1, file);
	fwrite(&underline_, 1, 1, file);
	fwrite(&strikeout_, 1, 1, file);
	fwrite(&alignment_, 1, 1, file);
}

//Classe de waypoints
TeGTMTrackLogs::TeGTMTrackLogs()
{
}

TeGTMTrackLogs::~TeGTMTrackLogs()
{
}

void TeGTMTrackLogs::readGTMTrackLogs(FILE* file)
{
	fread(&latitude_, 8, 1, file);
	fread(&longitude_, 8, 1, file);

	fread(&tdate_, 4, 1, file);

	fread(&tnum_, 1, 1, file);

	fread(&talt_, 4, 1, file);
}

void TeGTMTrackLogs::writeGTMTrackLogs(FILE* file)
{
	fwrite(&latitude_, 8, 1, file);
	fwrite(&longitude_, 8, 1, file);

	fwrite(&tdate_, 4, 1, file);

	fwrite(&tnum_, 1, 1, file);

	fwrite(&talt_, 4, 1, file);
}

//Classe de waypoints styles
TeGTMTrackLogStyles::TeGTMTrackLogStyles()
{
	tname_ = NULL;
}

TeGTMTrackLogStyles::~TeGTMTrackLogStyles()
{
	if(tname_ != NULL)
		delete tname_;
}

void TeGTMTrackLogStyles::readGTMTrackLogStyles(FILE *file)
{
	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	tname_ = new char[aux+1];
	fread(tname_, (size_t)aux, 1, file);
	tname_[aux] = '\0';

	fread(&ttype_, 1, 1, file);

	fread(&tcolor_, 4, 1, file);

	fread(&tscale_, 4, 1, file);

	fread(&tlabel_, 1, 1, file);

	fread(&tlayer_, 2, 1, file);
}

void TeGTMTrackLogStyles::writeGTMTrackLogStyles(FILE *file)
{
	int i = strlen(tname_);
	fwrite(&i, 2, 1, file);
	fwrite(tname_, i, 1, file);

	fwrite(&ttype_, 1, 1, file);

	fwrite(&tcolor_, 4, 1, file);

	fwrite(&tscale_, 4, 1, file);

	fwrite(&tlabel_, 1, 1, file);

	fwrite(&tlayer_, 2, 1, file);
}

//Classe de routes
TeGTMRoutes::TeGTMRoutes()
{
	wname_ = NULL;
	wcomment_ = NULL;
	rname_ = NULL;
}

TeGTMRoutes::~TeGTMRoutes()
{
	if(wname_ = NULL)
		delete wname_;
	if(wcomment_ = NULL)
		delete wcomment_;
	if(rname_ = NULL)
		delete rname_;
}

void TeGTMRoutes::readGTMRoutes(FILE *file)
{
	fread(&latitude_, 8, 1, file);
	fread(&longitude_, 8, 1, file);

	wname_ = new char[11];
	fread(wname_, 10, 1, file);
	wname_[10] = '\0';

	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	wcomment_ = new char[aux+1];
	fread(wcomment_, (size_t)aux, 1, file);
	wcomment_[aux] = '\0';

	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	rname_ = new char[aux+1];
	fread(rname_, (size_t)aux, 1, file);
	rname_[aux] = '\0';

	fread(&ico_, 2, 1, file);

	fread(&dspl_, 1, 1, file);
	fread(&tnum_, 1, 1, file);

	fread(&tdate_, 4, 1, file);

	fread(&wrot_, 2, 1, file);

	fread(&walt_, 4, 1, file);

	fread(&wlayer_, 2, 1, file);
}

//Classe de layers
TeGTMLayers::TeGTMLayers()
{
	name_ = NULL;
}

TeGTMLayers::~TeGTMLayers()
{
	if(name_ != NULL)
		delete name_;
}

void TeGTMLayers::readGTMLayers(FILE *file)
{
	fread(&index_, 2, 1, file);

	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	name_ = new char[aux+1];
	fread(name_, (size_t)aux, 1, file);
	name_[aux] = '\0';

	fread(&color_, 4, 1, file);

	fread(&lock_, 1, 1, file);
	fread(&view_, 1, 1, file);
	fread(&reserva1_, 1, 1, file);

	fread(&reserva2_, 2, 1, file);
}

//Classe de user icons
TeGTMUserIcons::TeGTMUserIcons()
{
	name_ = NULL;
}

TeGTMUserIcons::~TeGTMUserIcons()
{
	if(name_ != NULL)
		delete name_;
}

void TeGTMUserIcons::readGTMUserIcons(FILE *file)
{
	short aux;
	fread(&aux, 2, 1, file);
	if(aux < 0)
		throw TeException(UNKNOWN_ERROR_TYPE, "Error while reading the file");
	name_ = new char[aux+1];
	fread(name_, (size_t)aux, 1, file);
	name_[aux] = '\0';

	fread(&number_, 1, 1, file);

	fread(&size_, 4, 1, file);
}
