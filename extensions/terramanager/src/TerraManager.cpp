// TerraManager include files
#include "TerraManager.h"
#include "TerraManagerUtils.h"
#include "TerraManagerException.h"
#include "TeGDLegend.h"

//TerraLib include files
#include <TeInitRasterDecoders.h>
#include <TeVectorRemap.h>
#include <TeRasterRemap.h>
#include <TeBox.h>
#include <TeSpatialOperations.h>
#include <TeDatabaseUtils.h>
#include <TeGroupingAlgorithms.h>
#include <TeColorUtils.h>
#include <TeProjection.h>
#include <TeQuerier.h>
#include <TeQuerierParams.h>
#include <TeDriverSHPDBF.h>

extern DBFHandle TeCreateDBFFile (const string& dbfFilename, TeAttributeList& attList);

namespace TeMANAGER
{

inline void TeGetLabelPoint(TeLine2D& l, TeCoord2D& c, double& angle)
{
	unsigned int seg = TeMANAGER::TeInterpolatePoint(l, 0.5, c);
	TeGetAngle(l[seg], l[seg + 1], angle);
}

}

TeMANAGER::TerraManager::TerraManager()
	: db_(0), currentView_(0), currentTheme_(0), currentThemeActiveRepresentations_(0),
	  referenceTheme_(0), referenceThemeActiveRepresentations_(0), canvasBackgroundColor_(0, 0, 0),
	  conflicDetect_(false), minCollisionTol_(10),
	  /*textOutlineEnable_(false),*/ textOutlineColor_(255, 255, 255),
	  automaticScaleControlEnable_(true),
	  imcanvas_(0), closeImgMapCanvasAtEnd_(true),
	  keepThemeTransparency_(false),
	  generalizedPixels_(0), closeDatabaseAtEnd_(true)
{
	visualMap_[TePOLYGONS] = new TeVisual(TePOLYGONS);
	visualMap_[TeLINES] = new TeVisual(TeLINES);
	visualMap_[TePOINTS] = new TeVisual(TePOINTS);
	visualMap_[TeTEXT] = new TeVisual(TeTEXT);
}

TeMANAGER::TerraManager::~TerraManager()
{
	closeConnection();

	clearVisualMap();

	clearThemeVisualMap();

	delete imcanvas_;
}

std::string& TeMANAGER::TerraManager::getErrorMessage() const
{
	return errorMessage_;
}

void TeMANAGER::TerraManager::connect(const TeDBMSType& dbType, const std::string& host,
						   const std::string& user, const std::string& password,
						   const std::string& database, const int& port)
{
	closeConnection();

	closeDatabaseAtEnd_ = true;
	
	try
	{
		db_ = TeMakeConnection(dbType, host, user, password, database, port);
		
	}
	catch(const TerraManagerException& e)
	{
		errorMessage_ = "Error while opening database connection. " + e.getErrorMessage();
		throw TerraManagerException(e.getErrorMessage(), "TerraManagerException");
	}
}

void TeMANAGER::TerraManager::setDatabase(TeDatabase* db)
{
	closeConnection();
	db_ = db;
	closeDatabaseAtEnd_ = false;
}

void TeMANAGER::TerraManager::closeConnection(void)
{
	if(closeDatabaseAtEnd_ && db_)
	{
		//db_->clear();
		db_->close();
		delete db_;
		db_ = 0;
		closeDatabaseAtEnd_ = false;
	}
}

void TeMANAGER::TerraManager::getViews(std::vector<std::string>& views, const std::string& userName) const
{
	views.clear();
	verifyDatabaseConnection();	

	TeDatabasePortal* portal = getPortal();

	std::string sql = "";

	if(userName.empty())
		sql = "SELECT * FROM te_view WHERE user_name = '" + db_->user() + "'";
	else
		sql = "SELECT * FROM te_view WHERE user_name = '" + userName + "'";
	
	if(!portal->query(sql))
	{
		errorMessage_ = "Couldn't read te_view table. " + db_->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	while(portal->fetchRow())
		views.push_back(portal->getData("name"));
		
	portal->freeResult();
	delete portal;

	return;
}

void TeMANAGER::TerraManager::setCurrentView(const std::string& viewName)
{
	if(viewName.empty())
	{
		errorMessage_ = "View name is empty.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
		
	verifyDatabaseConnection();

	db_->clear();

	currentView_ = new TeView(viewName, db_->user());

	if(!db_->loadView(currentView_))
	{
		errorMessage_ = "Could not load view: " + db_->errorMessage();
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.getCanvasProjection())
		canvas_.setCanvasProjection(*(currentView_->projection()));

}

std::string TeMANAGER::TerraManager::getCurrentViewName(void) const
{
	if(currentView_)
		return currentView_->name();
	
	return "";
}

TeView* TeMANAGER::TerraManager::getCurrentView(void) const
{
	return currentView_;
}

void TeMANAGER::TerraManager::getCurrentViewBox(TeBox& b)
{
	verifyDatabaseConnection();
	verifyCurrentView();

	b = currentView_->box(true);
}

struct TePrioritySorter
{
	bool operator()(TeAbstractTheme* t1, TeAbstractTheme* t2) const
	{
		if(t1->priority() < t2->priority())
			return true;

		return false;
	}
};

void TeMANAGER::TerraManager::getThemes(vector<std::string>& themes, const bool& onlyVisible) const
{
	themes.clear();
	verifyDatabaseConnection();
	verifyCurrentView();

	TeThemeMap::iterator it = db_->themeMap().begin();

	vector<TeAbstractTheme*> themesAux;

	while(it != db_->themeMap().end())
	{
		if(onlyVisible)
		{
			if(it->second->visibility())
                //themes.push_back(it->second->name());
				themesAux.push_back(it->second);
		}
		else
			//themes.push_back(it->second->name());
			themesAux.push_back(it->second);

		++it;
	}

	sort(themesAux.begin(), themesAux.end(), TePrioritySorter());

	for(unsigned int i = 0; i < themesAux.size(); ++i)
		themes.push_back(themesAux[i]->name());
}

void TeMANAGER::TerraManager::setTheme(const std::string& themeName, const int& themeType)
{
	if(themeName.empty())
	{
		errorMessage_ = "Please, enter the name of the theme.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	verifyCurrentView();

	TeAbstractTheme* theme = 0;
	string searchThemeName = TeConvertToUpperCase(themeName);

	TeThemeMap::iterator it = db_->themeMap().begin();

	while(it != db_->themeMap().end())
	{
		if(TeConvertToUpperCase(it->second->name()) == searchThemeName)
		{
			theme = it->second;
			break;
		}
		++it;
	}

	if(!theme)
	{
		errorMessage_ = "Couldn't find the informed theme.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	setTheme(dynamic_cast<TeTheme*>(theme), themeType);
}

void TeMANAGER::TerraManager::setTheme(TeTheme* theme, const int& themeType)
{
	curentThemeLabelingField_.clear();
	curentThemePriorityField_.clear();
	clearThemeVisualMap();

	int repres = theme->visibleRep() & 0xbfffffff;

	if(repres == 0)
	{
		errorMessage_ = "Theme: " + theme->name() + " doesn't have an active representation.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(themeType == 0)
	{
		currentTheme_ = theme;
		currentThemeActiveRepresentations_ = repres;
	}
	else
	{
		referenceTheme_ = theme;
		referenceThemeActiveRepresentations_ = repres;
	}
}

std::string TeMANAGER::TerraManager::getThemeName(const int& themeType) const
{
	if(themeType == 0 && currentTheme_)
	{
		return currentTheme_->name();
	}
	else if(themeType == 1 && referenceTheme_)
	{
		return referenceTheme_->name();
	}

	return "";
}

TeTheme* TeMANAGER::TerraManager::getTheme(const int& themeType) const
{
	if((themeType == 0) && currentTheme_)
	{
		return currentTheme_;
	}
	else if((themeType == 1) && referenceTheme_)
	{
		return referenceTheme_;
	}
	else
	{
		errorMessage_ = "There is no current or reference themes. Please set one first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	return 0;
}

int TeMANAGER::TerraManager::getThemeRepresentation(const int& themeType) const
{
	if(themeType == 0 && currentTheme_)
	{
		return currentTheme_->visibleRep() & 0xbfffffff;
	}
	else if(themeType == 1 && referenceTheme_)
	{
		return referenceTheme_->visibleRep() & 0xbfffffff;
	}
			
	return 0;
}

void TeMANAGER::TerraManager::setActiveRepresentation(const int& repres, const int& themeType)
{
	if(themeType == 0)
	{
		if(!currentTheme_)
		{
			errorMessage_ = "Please, first you need to set the current theme.";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}

		TeGeomRep visRep = (TeGeomRep)currentTheme_->visibleRep();

		if(visRep & repres)
			currentThemeActiveRepresentations_ = repres;
		else
		{
			errorMessage_ = "The theme doesn't have the informed visible representation!";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}
	}
	else
	{
		if(!referenceTheme_)
		{
			errorMessage_ = "Please, first you need to set the reference theme.";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}

		TeGeomRep visRep = (TeGeomRep)referenceTheme_->visibleRep();

		if(visRep & repres)
			referenceThemeActiveRepresentations_ = repres;
		else
		{
			errorMessage_ = "The theme doesn't have the informed visible representation!";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}
	}
}

int TeMANAGER::TerraManager::getActiveRepresentation(const int& themeType) const
{
	return (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;
}

TeBox TeMANAGER::TerraManager::getThemeBox(const int& themeType, const std::string& restrictionExpression)
{
	verifyDatabaseConnection();
	verifyCurrentView();

	TeTheme* theme = getTheme(themeType);
	
	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;

	std::string tableName;
	
	if(repres & TePOLYGONS)
		tableName = currentTheme_->layer()->tableName(TePOLYGONS);
	else if(repres & TeLINES)
		tableName = currentTheme_->layer()->tableName(TeLINES);
	else if(repres & TePOINTS)
		tableName = currentTheme_->layer()->tableName(TePOINTS);
	else if(repres & TeCELLS)
		tableName = currentTheme_->layer()->tableName(TeCELLS);

	std::string queryObjects  = "";

	if((repres & TePOLYGONS) || (repres & TeLINES) || (repres & TeCELLS))
	{
		if(db_->dbmsName() == "PostgreSQL")
		{
			queryObjects  = "SELECT MIN(";
			queryObjects += "((" + tableName + ".spatial_box[1])[0])), MIN(";
			queryObjects += "((" + tableName + ".spatial_box[1])[1])), MAX(";
			queryObjects += "((" + tableName + ".spatial_box[0])[0])), MAX(";
			queryObjects += "((" + tableName + ".spatial_box[0])[1])) ";
		}
		else if(db_->dbmsName() == "PostGIS")
		{
			queryObjects  = "SELECT xmin(extent(" + tableName + ".spatial_data)::BOX3D), ymin(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D), xmax(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D), ymax(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D) ";
		}
		else
		{
			queryObjects  = "SELECT MIN(" + tableName + ".lower_x), MIN(";
			queryObjects += tableName + ".lower_y), MAX(";
			queryObjects += tableName + ".upper_x), MAX(";
			queryObjects += tableName + ".upper_y) ";
		}
	}
	else if(repres & TePOINTS)
	{
		if(db_->dbmsName() == "PostGIS")
		{
			queryObjects  = "SELECT xmin(extent(" + tableName + ".spatial_data)::BOX3D), ymin(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D), xmax(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D), ymax(extent(";
			queryObjects += tableName + ".spatial_data)::BOX3D) ";
		}
		else
		{
			queryObjects  = "SELECT MIN(" + tableName + ".x), MIN(";
			queryObjects += tableName + ".y), MAX(";
			queryObjects += tableName + ".x), MAX(";
			queryObjects += tableName + ".y) ";
		}
	}
	else 
	{
		errorMessage_ = "Couldn't determine box for this representation.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	queryObjects += currentTheme_->sqlGridFrom(tableName);

	std::string sqlWhere = currentTheme_->attributeRest();

	if(restrictionExpression.empty() == false)
	{
		if(sqlWhere.empty())
			sqlWhere += restrictionExpression;
		else
			sqlWhere += " AND "+ restrictionExpression;
	}

	if(sqlWhere.empty() == false)
		queryObjects += " WHERE " + sqlWhere;

	TeDatabasePortal* portal = getPortal();

	if(portal->query(queryObjects, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
	{
		errorMessage_ = "Couldn't get theme box. " + db_->errorMessage();
        portal->freeResult();
        delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!portal->fetchRow())
	{
		errorMessage_ = "Couldn't get theme box.";
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeBox box;
	box.x1_ = portal->getDouble(0);
	box.y1_ = portal->getDouble(1);
	box.x2_ = portal->getDouble(2);
	box.y2_ = portal->getDouble(3);

	if(theme->layer()->projection() && canvas_.getCanvasProjection() && !(*(theme->layer()->projection()) == *(canvas_.getCanvasProjection()))) 
	{
		canvas_.setDataProjection(*theme->layer()->projection());

		TeCoord2D c = box.lowerLeft();
		canvas_.dataWorld2World(c, c);
		box.x1_ = c.x();
		box.y1_ = c.y();

		c = box.upperRight();
		canvas_.dataWorld2World(c, c);
		box.x2_ = c.x();
		box.y2_ = c.y();
	}

	portal->freeResult();
	delete portal;

	return box;
}

void TeMANAGER::TerraManager::setWorkProjection(TeProjection& proj)
{
	canvas_.setCanvasProjection(proj);
}

TeBox TeMANAGER::TerraManager::setWorld(const double& x1, const double& y1, const double& x2, const double& y2, const int& width, const int& height, const bool& keepAspectRatio)
{
	TeBox b(x1, y1, x2, y2);

	if(keepAspectRatio)
	{
		double worldDX = x2 - x1;
		double worldDY = y2 - y1;

		double vpDX = (double)width;
		double vpDY = (double)height;

		double fx = vpDX  / worldDX;
		double fy = vpDY / worldDY;

		// a funcao de transformacao escolhera para a proporcao do eixo que provoca a maior degradacao
		// da imagem: ou seja, a que a relacao (cumprimento_imagem / cumprimento_mundo) for menor
		if (fx > fy)
		{
			// o eixo x e redimensionado (aumentado o seu cumprimento)
			double dxw = vpDX / fy;

			double dyw = vpDY / fy;

			double xc = (b.x2_ + b.x1_) / 2.0;
			double yc = (b.y2_ + b.y1_) / 2.0;

			b.x1_ = xc - (dxw / 2.0);
			b.x2_ = xc + (dxw / 2.0);

			b.y1_ = yc - (dyw / 2.0);
			b.y2_ = yc + (dyw / 2.0);
		}
		else
		{
			double dyw = vpDY / fx;
	        
			double dxw = vpDX / fx;

			double xc = (b.x2_ + b.x1_) / 2.0;
			double yc = (b.y2_ + b.y1_) / 2.0;

			b.x1_ = xc - (dxw / 2.0);
			b.x2_ = xc + (dxw / 2.0);

			b.y1_ = yc - (dyw / 2.0);
			b.y2_ = yc + (dyw / 2.0);
		}
	}	
	
	canvas_.setWorld(b, width, height);
	
	return canvas_.getWorld();
}

double TeMANAGER::TerraManager::getPixelSizeX(void) const
{
	return canvas_.getPixelSizeX();
}

double TeMANAGER::TerraManager::getPixelSizeY(void) const
{
	return canvas_.getPixelSizeY();
}

void* TeMANAGER::TerraManager::getCanvasImage(const int& imageType, int& size, const bool& opaque, const int& quality)
{
    return canvas_.getImage(imageType, size, opaque, quality);
}

void TeMANAGER::TerraManager::saveCanvasImage(const int& imageType, const std::string& fileName, const bool& opaque, const int& quality)
{
	return canvas_.saveImage(imageType, fileName, opaque, quality);
}

void TeMANAGER::TerraManager::cleanCanvas()
{
	canvas_.clear();
}

void TeMANAGER::TerraManager::freeImage(void* ptr)
{
	TeGDCanvas::freeImage(ptr);
}

void TeMANAGER::TerraManager::setCanvasBackgroundColor(const int& r, const int& g, const int& b)
{
	canvasBackgroundColor_ = TeColor(r, g, b);

	canvas_.setBackgroudColor(r, g, b);
}

void TeMANAGER::TerraManager::setDefaultVisual(const TeVisual& vis, const TeGeomRep& representation)
{
	*(visualMap_[representation]) = vis;
	return;
}

TeVisual& TeMANAGER::TerraManager::getDefaultVisual(const TeGeomRep& representation)
{
	return *(visualMap_[representation]);
}

void TeMANAGER::TerraManager::setTextOutLineEnable(const bool& turnon)
{
	/*textOutlineEnable_ = turnon;*/
	canvas_.setTextOutlineEnable(turnon);
}

void TeMANAGER::TerraManager::setTextOutLineColor(const int& r, const int& g, const int& b)
{
	canvas_.setTextOutlineColor(r, g, b);
	/*textOutlineColor_.init(r, g, b);*/
}

void TeMANAGER::TerraManager::draw(TeCoord2D& c)
{
	TeVisual& v = *(visualMap_[TePOINTS]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
	canvas_.setPointColor(v.color().red_, v.color().green_, v.color().blue_);
	canvas_.setPointStyle(v.style(), v.size());	
	canvas_.draw(c);
}

void TeMANAGER::TerraManager::draw(const TeBox& b)
{
	TeVisual& v = *(visualMap_[TePOLYGONS]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
    canvas_.setPolygonColor(v.color().red_, v.color().green_, v.color().blue_, v.transparency());
	canvas_.setPolygonStyle(v.style());
	canvas_.setPolygonLineColor(v.contourColor().red_, v.contourColor().green_, v.contourColor().blue_);
	canvas_.setPolygonLineStyle(v.contourStyle(), v.contourWidth());
	canvas_.draw(b);
}

void TeMANAGER::TerraManager::draw(const TeLine2D& l)
{
	TeVisual& v = *(visualMap_[TeLINES]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
	canvas_.setLineColor(v.color().red_, v.color().green_, v.color().blue_);
	canvas_.setLineStyle(v.style(), v.width());
	canvas_.draw(l);
}

void TeMANAGER::TerraManager::draw(const TePolygon& p)
{
	TeVisual& v = *(visualMap_[TePOLYGONS]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
	canvas_.setPolygonColor(v.color().red_, v.color().green_, v.color().blue_, v.transparency());
	canvas_.setPolygonStyle(v.style());
	canvas_.setPolygonLineColor(v.contourColor().red_, v.contourColor().green_, v.contourColor().blue_);
	canvas_.setPolygonLineStyle(v.contourStyle(), v.contourWidth());
	canvas_.draw(p);
}

void TeMANAGER::TerraManager::draw(TeCoord2D& c, const std::string& text, const double& angle,
			            const double& horizontalAlign, const double& verticalAlign)
{
	TeVisual& v = *(visualMap_[TeTEXT]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
	canvas_.setTextColor(v.color().red_, v.color().green_, v.color().blue_);
	canvas_.setTextStyle(v.family());
	canvas_.setTextSize(v.width());

/*	if(textOutlineEnable_)
		canvas_.setTextOutlineColor(textOutlineColor_.red_, textOutlineColor_.green_, textOutlineColor_.blue_);
	else
		canvas_.setTextOutlineColor(v.color().red_, v.color().green_, v.color().blue_);
*/
    canvas_.draw(c, text, angle, horizontalAlign, verticalAlign);
}

void TeMANAGER::TerraManager::draw(TeCoord2D& c, const std::string& text, const int& minCollisionTol, const double& angle,
				        const double& horizontalAlign, const double& verticalAlign)
{
	TeVisual& v = *(visualMap_[TeTEXT]);

	canvas_.setDataProjection(*canvas_.getCanvasProjection());
	canvas_.setTextColor(v.color().red_, v.color().green_, v.color().blue_);
	canvas_.setTextStyle(v.family());
	canvas_.setTextSize(v.width());

/*	if(textOutlineEnable_)
		canvas_.setTextOutlineColor(textOutlineColor_.red_, textOutlineColor_.green_, textOutlineColor_.blue_);
*/
    canvas_.draw(c, text, minCollisionTol, angle, horizontalAlign, verticalAlign);
}

void TeMANAGER::TerraManager::drawCurrentTheme()
{
	TeLegendEntryVector legendVec;
	TeGrouping group;
	std::vector<std::string> ramp;

	drawCurrentTheme(legendVec, group, ramp, "", "", "", "");
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec)
{
	TeGrouping group;
	std::vector<std::string> ramp;

	drawCurrentTheme(legendVec, group, ramp, "", "", "", "");
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec, const std::string& restrictionExpression)
{
	TeGrouping group;
	std::vector<std::string> ramp;

	drawCurrentTheme(legendVec, group, ramp, "", "", "", restrictionExpression);
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group, vector<std::string>& rampColors)
{
	drawCurrentTheme(legendVec, group, rampColors, "", "", "", "");
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group, vector<std::string>& rampColors, const std::string& restrictionExpression)
{
	drawCurrentTheme(legendVec, group, rampColors, "", "", "", restrictionExpression);
}

void TeMANAGER::TerraManager::drawCurrentTheme(const std::string& from, const std::string& linkAttr, 
			                        const std::string& restrictionExpression)
{
	TeLegendEntryVector legendVec;
	TeGrouping group;
	std::vector<std::string> ramp;

	drawCurrentTheme(legendVec, group, ramp, "", from, linkAttr, restrictionExpression);
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec, const std::string& from,
			                        const std::string& linkAttr, const std::string& restrictionExpression)
{
	TeGrouping group;
	std::vector<std::string> ramp;

	drawCurrentTheme(legendVec, group, ramp, "", from, linkAttr, restrictionExpression);
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeGrouping& group, std::vector<std::string>& rampColors,
			                        const std::string& fields, const std::string& from,
						            const std::string& linkAttr, const std::string& restrictionExpression)
{
	TeLegendEntryVector legendVec;
	drawCurrentTheme(legendVec, group, rampColors, fields, from, linkAttr, restrictionExpression);
}

void TeMANAGER::TerraManager::drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group,
						            std::vector<std::string>& rampColors,
			                        const std::string& fields, const std::string& from,
						            const std::string& linkAttr, const std::string& restrictionExpression)
{
//	verifyCurrentView();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.isValid())
	{
		errorMessage_ = "The canvas is not prepared. Please, first use setWorld method to adjust canvas!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	legendVec.clear();

// see if scale is turned on
	if(automaticScaleControlEnable_)
	{
		double scale = getScale();
		double minscale = currentTheme_->minScale();
		double maxscale = currentTheme_->maxScale();
		double epsilon = 0.000001;

		if(minscale == 0.0)
			minscale = +TeMAXFLOAT;

		if(!((scale > (maxscale - epsilon)) && (scale < (minscale + epsilon))))
			return;
	}

// set canvas and data projection => the projection in wich we will draw the theme objects
	canvas_.setDataProjection(*(currentTheme_->layer()->projection()));

	const TeBox& canvasBBox = canvas_.getWorld();
	TeBox themeBBox = currentTheme_->layer()->box();
	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = themeBBox.lowerLeft();
		canvas_.dataWorld2World(c, c);
		themeBBox.x1_ = c.x();
		themeBBox.y1_ = c.y();

		c = themeBBox.upperRight();
		canvas_.dataWorld2World(c, c);
		themeBBox.x2_ = c.x();
		themeBBox.y2_ = c.y();
	}

// if layer and canvas bounding boxes intersects, draw the theme
	TeBox intersectionBox;
	if(!TeIntersection(canvasBBox, themeBBox, intersectionBox))
		return;

	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = intersectionBox.lowerLeft();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x1_ = c.x();
		intersectionBox.y1_ = c.y();

		c = intersectionBox.upperRight();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x2_ = c.x();
		intersectionBox.y2_ = c.y();
	}

	std::map<std::string, unsigned int> visualMap;

	if(group.groupNumSlices_ > 0)
	{
// it is a normal grouping or a SQL
		buildGrouping(group, rampColors, restrictionExpression, legendVec, visualMap, fields, from, linkAttr);
	}
	else
	{
		if(currentTheme_->legend().empty())
			legendVec.push_back(currentTheme_->defaultLegend());
		else
			legendVec = currentTheme_->legend();
	}

	if((currentTheme_->visibleRep() & TeRASTER || currentTheme_->visibleRep() & TeRASTERFILE)
		&& (currentThemeActiveRepresentations_ & TeRASTER || currentThemeActiveRepresentations_ & TeRASTERFILE))
		drawRasterRepresentation( intersectionBox );
	
	if((currentTheme_->visibleRep() & TePOLYGONS) && (currentThemeActiveRepresentations_ & TePOLYGONS))
	{
		drawPolygonRepresentation(legendVec, visualMap, restrictionExpression, intersectionBox, from, linkAttr);
	}

	if((currentTheme_->visibleRep() & TeCELLS) && (currentThemeActiveRepresentations_ & TeCELLS))
	{
		drawCellRepresentation(legendVec, visualMap, restrictionExpression, intersectionBox, from, linkAttr);
	}

	if(!(((currentTheme_->visibleRep() & TePOLYGONS) && (currentThemeActiveRepresentations_ & TePOLYGONS)) ||
         ((currentTheme_->visibleRep() & TeCELLS) && (currentThemeActiveRepresentations_ & TeCELLS))))
	{
		for(unsigned int i = 0; i < legendVec.size(); ++i)
			legendVec[i].getVisualMap().erase(TePOLYGONS);
	}

	if((currentTheme_->visibleRep() & TeLINES) && (currentThemeActiveRepresentations_ & TeLINES))
	{
		drawLineRepresentation(legendVec, visualMap, restrictionExpression, intersectionBox, from, linkAttr);
	}
	else
	{
		for(unsigned int i = 0; i < legendVec.size(); ++i)
			legendVec[i].getVisualMap().erase(TeLINES);
	}

	if((currentTheme_->visibleRep() & TePOINTS) && (currentThemeActiveRepresentations_ & TePOINTS))
	{
		drawPointRepresentation(legendVec, visualMap, restrictionExpression, intersectionBox, from, linkAttr);
	}
	else
	{
		for(unsigned int i = 0; i < legendVec.size(); ++i)
			legendVec[i].getVisualMap().erase(TePOINTS);
	}

	if(!curentThemeLabelingField_.empty())
		drawTextLabeling(restrictionExpression, intersectionBox, from, linkAttr);
}

void TeMANAGER::TerraManager::drawSelectedObjects(TeKeys& objIds, const int& themeType)
{
	verifyCurrentView();

	if(objIds.empty())
	{
		errorMessage_ = "Please, enter a list of selected objects.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeKeys::iterator itB = objIds.begin();
	TeKeys::iterator itE = objIds.end();

	std::vector<std::string> inClauseVector = generateInClauses(itB, itE, db_);

	if(inClauseVector.empty())
	{
		errorMessage_ = "Error while mounting in clauses in order to draw selected objects.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeTheme* theme = getTheme(themeType);

	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;
	
	canvas_.setDataProjection(*(theme->layer()->projection()));

	if((theme->visibleRep() & TePOLYGONS) && (repres & TePOLYGONS))
        drawSelectedPolygons(inClauseVector, themeType);

	if((theme->visibleRep() & TeCELLS) && (repres & TeCELLS))
        drawSelectedCells(inClauseVector, themeType);

    if((theme->visibleRep() & TeLINES) && (repres & TeLINES))
		drawSelectedLines(inClauseVector, themeType);

    if((theme->visibleRep() & TePOINTS) && (repres & TePOINTS))
		drawSelectedPoints(inClauseVector, themeType);
}

void TeMANAGER::TerraManager::drawBufferZone(const std::string& objectId, const double& distance, const int& themeType)
{
	verifyCurrentView();

	if(objectId.empty())
	{
		errorMessage_  = "Please, enter an objectid";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
	
	TeTheme* theme = getTheme(themeType);

	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;

	canvas_.setDataProjection(*(theme->layer()->projection()));

	TeKeys k;
    k.push_back(objectId);
	TePolygonSet bufferSet;

	if((theme->visibleRep() & TePOLYGONS) && (repres & TePOLYGONS))
	{
		std::string actGeomTable = theme->layer()->tableName(TePOLYGONS);

		TePolygonSet bufferSetAux;
		TeGetBuffer(actGeomTable, TePOLYGONS, k, db_, bufferSetAux, distance);

		for(unsigned int i = 0; i < bufferSetAux.size(); ++i)
			bufferSet.add(bufferSetAux[i]);
	}

	if((theme->visibleRep() & TeLINES) && (repres & TeLINES))
	{
		std::string actGeomTable = theme->layer()->tableName(TeLINES);

		TePolygonSet bufferSetAux;
		TeGetBuffer(actGeomTable, TeLINES, k, db_, bufferSetAux, distance);

		for(unsigned int i = 0; i < bufferSetAux.size(); ++i)
			bufferSet.add(bufferSetAux[i]);
	}

	if((theme->visibleRep() & TePOINTS) && (repres & TePOINTS))
	{
		std::string actGeomTable = theme->layer()->tableName(TePOINTS);

		TePolygonSet bufferSetAux;
		TeGetBuffer(actGeomTable, TePOINTS, k, db_, bufferSetAux, distance);

		for(unsigned int i = 0; i < bufferSetAux.size(); ++i)
			bufferSet.add(bufferSetAux[i]);
	}

	if((theme->visibleRep() & TeCELLS) && (repres & TeCELLS))
	{
		std::string actGeomTable = theme->layer()->tableName(TeCELLS);

		TePolygonSet bufferSetAux;
		TeGetBuffer(actGeomTable, TeCELLS, k, db_, bufferSetAux, distance);

		for(unsigned int i = 0; i < bufferSetAux.size(); ++i)
			bufferSet.add(bufferSetAux[i]);
	}

    unsigned int size = bufferSet.size();

    TeVisual& v = *(visualMap_[TeLINES]);

    canvas_.setLineColor(v.color().red_, v.color().green_, v.color().blue_);
    canvas_.setLineStyle(v.style(), v.width());

    for(unsigned int i = 0; i < size; ++i)
    {
        TePolygon& poly = bufferSet[i];

        for(unsigned int j = 0; j < poly.size(); ++j)
		{
            TeLine2D& line = poly[j];
            canvas_.draw(line);
        }
    }	

	canvas_.setDataProjection(*currentView_->projection());
}

bool TeMANAGER::TerraManager::locateObject(std::pair<std::string, std::string>& identifier, TeCoord2D& pt, const double& tol, const int& themeType)
{
//	verifyCurrentView();

	TeTheme* theme = getTheme(themeType);

	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;

	TeCoord2D ptAux = pt;
	double tolAux = tol;

	canvas_.setDataProjection(*(theme->layer()->projection()));

	if(theme->layer()->projection() &&
	   canvas_.getCanvasProjection() &&
       !(*(theme->layer()->projection()) == *(canvas_.getCanvasProjection())))
	{ 
// reproject given position in current view projection to current theme projection
		canvas_.world2DataWorld(pt, ptAux);

		TeCoord2D tolPtAux(pt.x() + tol, pt.y());
		canvas_.world2DataWorld(tolPtAux, tolPtAux);
		tolAux = fabs(tolPtAux.x() - ptAux.x());
	}

	if(repres & TePOINTS)
	{
		TePoint point;

		if(theme->locatePoint(ptAux, point, tolAux))
		{
			identifier.first = point.objectId();
			identifier.second = Te2String(point.geomId());
			return true;
		}
	}

	if(repres & TeLINES)
	{
		TeLine2D l;

		if(theme->locateLine(ptAux, l, tolAux))
		{
			identifier.first = l.objectId();
			identifier.second = Te2String(l.geomId());
			return true;
		}
	}

	if(repres & TeCELLS)
	{
		TeCell c;

		if(theme->locateCell(ptAux, c, tolAux))
		{
			identifier.first = c.objectId();
			identifier.second = Te2String(c.geomId());
			return true;
		}
	}

	if(repres & TePOLYGONS)
	{
		TePolygon p;

		if(theme->locatePolygon(ptAux, p, tolAux))
		{
			identifier.first = p.objectId();
			identifier.second = Te2String(p.geomId());
			return true;
		}
	}

	return false;
}

bool TeMANAGER::TerraManager::locateObjects(const TeBox& box, const int& relation, TeKeys& objIds, const int& themeType)
{
//	verifyCurrentView();

	objIds.clear();

	if(!box.isValid())
	{
		errorMessage_  = "The infomed box is not valid.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeTheme* theme = getTheme(themeType);

	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;

	TeBox b = box;

	canvas_.setDataProjection(*(theme->layer()->projection()));

	if(theme->layer()->projection() &&
	   canvas_.getCanvasProjection() &&
       !(*(theme->layer()->projection()) == *(canvas_.getCanvasProjection())))
	{
		TeCoord2D c = b.lowerLeft();
		canvas_.world2DataWorld(c, c);
		b.x1_ = c.x();
		b.y1_ = c.y();

		c = b.upperRight();
		canvas_.world2DataWorld(c, c);
		b.x2_ = c.x();
		b.y2_ = c.y();
	}

	TePolygon poly = polygonFromBox(b);

	TePrecision::instance().setPrecision(TeGetPrecision(theme->layer()->projection()));

	if((theme->visibleRep() & TePOLYGONS) && (repres & TePOLYGONS))
        if(db_->spatialRelation(theme->layer()->tableName(TePOLYGONS), TePOLYGONS, &poly, objIds, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TeLINES) && (repres & TeLINES))
        if(db_->spatialRelation(theme->layer()->tableName(TeLINES), TeLINES, &poly, objIds, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TePOINTS) && (repres & TePOINTS))
        if(db_->spatialRelation(theme->layer()->tableName(TePOINTS), TePOINTS, &poly, objIds, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TeCELLS) && (repres & TeCELLS))
        if(db_->spatialRelation(theme->layer()->tableName(TeCELLS), TeCELLS, &poly, objIds, relation, theme->collectionTable()))
			return true;

	return false;
}

bool TeMANAGER::TerraManager::locateObjects(const string& objectId, const int& relation, TeKeys& objIds, const int& themeType)
{
	if(objectId.empty())
	{
		errorMessage_ = "You should enter a valid object id.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeKeys objIdsIn;
	objIdsIn.push_back(objectId);

	return locateObjects(objIdsIn, relation, objIds, themeType);
}

bool TeMANAGER::TerraManager::locateObjects(TeKeys& objIdsIn, const int& relation, TeKeys& objIdsOut, const int& themeType)
{
//	verifyCurrentView();

	objIdsOut.clear();

	TeTheme* theme = getTheme(themeType);

	int repres = (themeType == 0) ? currentThemeActiveRepresentations_ : referenceThemeActiveRepresentations_;

	TeLayer* layer = theme->layer();

	TePrecision::instance().setPrecision(TeGetPrecision(theme->layer()->projection()));

	if((theme->visibleRep() & TePOLYGONS) && (repres & TePOLYGONS))
        if(db_->spatialRelation(layer->tableName(TePOLYGONS), TePOLYGONS, objIdsIn, objIdsOut, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TeLINES) && (repres & TeLINES))
        if(db_->spatialRelation(layer->tableName(TeLINES), TeLINES, objIdsIn, objIdsOut, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TePOINTS) && (repres & TePOINTS))
        if(db_->spatialRelation(layer->tableName(TePOINTS), TePOINTS, objIdsIn, objIdsOut, relation, theme->collectionTable()))
			return true;

	if((theme->visibleRep() & TeCELLS) && (repres & TeCELLS))
        if(db_->spatialRelation(layer->tableName(TeCELLS), TeCELLS, objIdsIn, objIdsOut, relation, theme->collectionTable()))
			return true;

	return false;
}

bool TeMANAGER::TerraManager::locateObjects(const string& objectId, const int& relation, TeKeys& objIds)
{
	if(objectId.empty())
	{
		errorMessage_ = "You should enter a valid object id.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeKeys objIdsIn;
	objIdsIn.push_back(objectId);

	return locateObjects(objIdsIn, relation, objIds);
}

bool TeMANAGER::TerraManager::locateObjects(TeKeys& objIdsIn, const int& relation, TeKeys& objIdsOut)
{
//	verifyCurrentView();

	objIdsOut.clear();


	if(!currentTheme_ || !referenceTheme_)
	{
		errorMessage_  = "This type of spatial query needs that the two themes (current and reference) are set";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeLayer* currentLayer = currentTheme_->layer();
	TeLayer* referenceLayer = referenceTheme_->layer();

	TePrecision::instance().setPrecision(TeGetPrecision(currentLayer->projection()));

	if((currentTheme_->visibleRep() & TePOLYGONS) && (currentThemeActiveRepresentations_ & TePOLYGONS))
	{
		std::string activeGeomTable = currentLayer->tableName(TePOLYGONS);

		if((referenceTheme_->visibleRep() & TePOLYGONS) && (referenceThemeActiveRepresentations_ & TePOLYGONS))
		{			
			std::string visibleGeomTable = referenceLayer->tableName(TePOLYGONS);

			if(db_->spatialRelation(activeGeomTable, TePOLYGONS, objIdsIn, visibleGeomTable, TePOLYGONS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}		

		if((referenceTheme_->visibleRep() & TeCELLS) && (referenceThemeActiveRepresentations_ & TeCELLS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeCELLS);

			if(db_->spatialRelation(activeGeomTable, TePOLYGONS, objIdsIn, visibleGeomTable, TeCELLS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TeLINES) && (referenceThemeActiveRepresentations_ & TeLINES))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeLINES);

			if(db_->spatialRelation(activeGeomTable, TePOLYGONS, objIdsIn, visibleGeomTable, TeLINES, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TePOINTS) && (referenceThemeActiveRepresentations_ & TePOINTS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TePOINTS);

			if(db_->spatialRelation(activeGeomTable, TePOLYGONS, objIdsIn, visibleGeomTable, TePOINTS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}
	}

	if((currentTheme_->visibleRep() & TeCELLS) && (currentThemeActiveRepresentations_ & TeCELLS))
	{
		std::string activeGeomTable = currentLayer->tableName(TeCELLS);

		if((referenceTheme_->visibleRep() & TePOLYGONS) && (referenceThemeActiveRepresentations_ & TePOLYGONS))
		{			
			std::string visibleGeomTable = referenceLayer->tableName(TePOLYGONS);

			if(db_->spatialRelation(activeGeomTable, TeCELLS, objIdsIn, visibleGeomTable, TePOLYGONS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}		

		if((referenceTheme_->visibleRep() & TeCELLS) && (referenceThemeActiveRepresentations_ & TeCELLS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeCELLS);

			if(db_->spatialRelation(activeGeomTable, TeCELLS, objIdsIn, visibleGeomTable, TeCELLS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TeLINES) && (referenceThemeActiveRepresentations_ & TeLINES))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeLINES);

			if(db_->spatialRelation(activeGeomTable, TeCELLS, objIdsIn, visibleGeomTable, TeLINES, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TePOINTS) && (referenceThemeActiveRepresentations_ & TePOINTS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TePOINTS);

			if(db_->spatialRelation(activeGeomTable, TeCELLS, objIdsIn, visibleGeomTable, TePOINTS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}
	}

	if((currentTheme_->visibleRep() & TeLINES) && (currentThemeActiveRepresentations_ & TeLINES))
	{
		std::string activeGeomTable = currentLayer->tableName(TeLINES);

		if((referenceTheme_->visibleRep() & TePOLYGONS) && (referenceThemeActiveRepresentations_ & TePOLYGONS))
		{			
			std::string visibleGeomTable = referenceLayer->tableName(TePOLYGONS);

			if(db_->spatialRelation(activeGeomTable, TeLINES, objIdsIn, visibleGeomTable, TePOLYGONS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}		

		if((referenceTheme_->visibleRep() & TeCELLS) && (referenceThemeActiveRepresentations_ & TeCELLS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeCELLS);

			if(db_->spatialRelation(activeGeomTable, TeLINES, objIdsIn, visibleGeomTable, TeCELLS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TeLINES) && (referenceThemeActiveRepresentations_ & TeLINES))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeLINES);

			if(db_->spatialRelation(activeGeomTable, TeLINES, objIdsIn, visibleGeomTable, TeLINES, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TePOINTS) && (referenceThemeActiveRepresentations_ & TePOINTS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TePOINTS);

			if(db_->spatialRelation(activeGeomTable, TeLINES, objIdsIn, visibleGeomTable, TePOINTS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}
	}

	if((currentTheme_->visibleRep() & TePOINTS) && (currentThemeActiveRepresentations_ & TePOINTS))
	{
		std::string activeGeomTable = currentLayer->tableName(TePOINTS);

		if((referenceTheme_->visibleRep() & TePOLYGONS) && (referenceThemeActiveRepresentations_ & TePOLYGONS))
		{			
			std::string visibleGeomTable = referenceLayer->tableName(TePOLYGONS);

			if(db_->spatialRelation(activeGeomTable, TePOINTS, objIdsIn, visibleGeomTable, TePOLYGONS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}		

		if((referenceTheme_->visibleRep() & TeCELLS) && (referenceThemeActiveRepresentations_ & TeCELLS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeCELLS);

			if(db_->spatialRelation(activeGeomTable, TePOINTS, objIdsIn, visibleGeomTable, TeCELLS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TeLINES) && (referenceThemeActiveRepresentations_ & TeLINES))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TeLINES);

			if(db_->spatialRelation(activeGeomTable, TePOINTS, objIdsIn, visibleGeomTable, TeLINES, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}

		if((referenceTheme_->visibleRep() & TePOINTS) && (referenceThemeActiveRepresentations_ & TePOINTS))
		{
			std::string visibleGeomTable = referenceLayer->tableName(TePOINTS);

			if(db_->spatialRelation(activeGeomTable, TePOINTS, objIdsIn, visibleGeomTable, TePOINTS, objIdsOut, relation, referenceTheme_->collectionTable()))
				return true;
		}
	}	

	return false;
}

void TeMANAGER::TerraManager::fetchAttributes(const string& objectId, std::map<std::string, std::string>& objVal, const int& themeType)
{
//	verifyCurrentView();

	TeTheme* theme = 0;
	
	if(themeType == 0 && currentTheme_)
	{
		theme = currentTheme_;
	}
	else if(themeType == 1 && referenceTheme_)
	{
		theme = referenceTheme_;
	}
	else
	{
		errorMessage_ = "There is no current or reference themes. Please set one first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

// get attribute tables associated to theme
	TeAttrTableVector& attrs = theme->attrTables();

// build sql to retrieve all attributes of all tables
	std::string sql = " SELECT ";

	for(unsigned int i = 0u; i < attrs.size(); ++i)
	{
		if(i > 0)
			sql += ", ";

		sql += attrs[i].name()+ ".*";
	}

	sql += theme->sqlFrom();
	
// get first table name and link to build WHERE clause
	std::string tablename = attrs[0].name();
	std::string link = attrs[0].linkName();
	sql += " WHERE " + tablename + "." + link + " = '" + objectId + "'";

// query database
	TeDatabasePortal* portal = getPortal();

	if(!portal->query(sql))
	{
		errorMessage_ = "Database query error during fetch attribute: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
	
// build map with all attributes and values
	TeAttributeList attrList = portal->getAttributeList();

	while(portal->fetchRow())
	{
		std::string val;
		for(unsigned int i = 0u; i < attrList.size(); ++i)
		{
			val = portal->getData(i);
			objVal[attrList[i].rep_.name_] = val;
		}
	}

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::drawLegend(std::vector<TeLegendEntryVector>& legends,
							  std::vector<std::string>& legendTitles,
							  const int& width)
{
	legendCanvas_.setBackgroudColor(canvasBackgroundColor_.red_, canvasBackgroundColor_.green_, canvasBackgroundColor_.blue_);
	TeGDLegend legend(&legendCanvas_);
	legend.setTextVisual(*(visualMap_[TeTEXT]));
	legend.draw(legends, legendTitles, width);
}

void* TeMANAGER::TerraManager::getLegendImage(const int& imageType, int& size, const bool& opaque, const int& quality)
{
    return legendCanvas_.getImage(imageType, size, opaque, quality);
}

void TeMANAGER::TerraManager::saveLegendImage(const int& imageType, const std::string& fileName, const bool& opaque, const int& quality)
{
	return legendCanvas_.saveImage(imageType, fileName, opaque, quality);
}

void TeMANAGER::TerraManager::setAutomaticScaleControlEnable(const bool& on)
{
	automaticScaleControlEnable_ = on;
}

double TeMANAGER::TerraManager::getScale() const
{
	double canvasWorldWidth = canvas_.getWorld().width();
	std::string canvasUnit = canvas_.getCanvasProjection()->units();

	if(canvasUnit != "Meters")
	{
		if(canvasUnit == "DecimalDegrees")
			canvasWorldWidth = canvasWorldWidth * (6378137.0 * 2.0 * TePI) / 360.0;
		else if(canvasUnit == "Kilometers")
			canvasWorldWidth = canvasWorldWidth * 1000.0;
		else if(canvasUnit == "Centimeters")
			canvasWorldWidth = canvasWorldWidth / 100.0;
		else if(canvasUnit == "Millimeters")
			canvasWorldWidth = canvasWorldWidth / 1000.0;
		else if(canvasUnit == "Feet")
			canvasWorldWidth = canvasWorldWidth * (12. * 0.0254);
		else if(canvasUnit == "Inches")
			canvasWorldWidth = canvasWorldWidth * 0.0254;
	}

// pixel size is defined to be 0,28 mm * 0,28 mm = 0,00028m * 0,00028m
	return canvasWorldWidth / canvas_.getWidth() / 0.00028;
}

void TeMANAGER::TerraManager::buildGrouping(TeGrouping& group, std::vector<std::string>& rampColors,
			                     const std::string& restrictionExpression, TeLegendEntryVector& legendVec,
						         std::map<std::string, unsigned int>& visualMap, const std::string& fields,
						         const std::string& fromClause, const std::string& linkAttr)
{
//	verifyCurrentView();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, first you need set a current theme.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	std::string objIdLinkCol = currentTheme_->attrTables()[0].name() + "." + currentTheme_->attrTables()[0].linkName();
	
	std::string query = "";
	
	bool normal = false;
	
	if(!fromClause.empty())
	{
		query  = "SELECT " + currentTheme_->collectionTable() + ".c_object_id, " + fields + " FROM " + fromClause;
		query += " INNER JOIN " + currentTheme_->collectionTable() +  " ON (" + currentTheme_->collectionTable() + ".c_object_id = " + linkAttr + ") ";
		
		if(!restrictionExpression.empty())
			query += " WHERE " + restrictionExpression;
	}
	else
	{
		if(group.groupAttribute_.name_.empty())
		{
			errorMessage_ = "Attribute name for grouping is empty.";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}
	
		std::string sqlFrom = currentTheme_->sqlFrom();
		std::string sqlWhere = "";
		
		if(!restrictionExpression.empty())
			sqlWhere = " WHERE " + restrictionExpression;
			
		std::string aggrFunc = "";

		if(group.groupFunction_.empty())
			aggrFunc = " MIN";
		else
			aggrFunc = group.groupFunction_;		
	
		if(group.groupNormAttribute_.empty())
		{
				query = " SELECT "+ objIdLinkCol + ", " + aggrFunc +"("+ group.groupAttribute_.name_ +")";  
		}
		else
		{
			query = " SELECT "+ objIdLinkCol + ", " + aggrFunc +"("+ group.groupAttribute_.name_ +") / "+ aggrFunc +"("+ group.groupNormAttribute_ + ")";
			normal = true;
		}
		
		query += sqlFrom + sqlWhere + " GROUP BY " + objIdLinkCol;
	}

	TeDatabasePortal* portal = getPortal();

	if(!portal->query(query))
	{
		errorMessage_ = "Error on grouping query: " + query + "\n " + db_->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}

// mount vector in memory
	std::vector<double> dValues;
	std::vector<std::string> sValues;

	std::vector<std::pair<std::string, double> > objectidsDouble;
	std::vector<std::pair<std::string, std::string> > objectidsString;

	double mean, sum; 

	mean = sum = 0.;

	int	nullValues = 0;

	group.groupAttribute_.type_ = portal->getAttribute(1).rep_.type_;

	do  
	{
		string val = portal->getData(1);
		string valNorm = Te2String(atof(val.c_str()),group.groupPrecision_);

		if (!val.empty())
		{
			if(group.groupMode_== TeUniqueValue)
			{
				if(normal)
				{
					objectidsString.push_back(pair<string, string>(portal->getData(0), valNorm));
					sValues.push_back(valNorm);
				}
				else
				{
					objectidsString.push_back(pair<string, string>(portal->getData(0), val));
					sValues.push_back(val);
				}
			}
			else
			{
				objectidsDouble.push_back(pair<string, double>(portal->getData(0), atof(valNorm.c_str())));
				dValues.push_back(atof(valNorm.c_str()));
				sum += atof(valNorm.c_str());
			}

		}
		else
			++nullValues;
	}while(portal->fetchRow());	

	portal->freeResult();
	delete portal;
	
	if(dValues.empty() && sValues.empty())
	{
		errorMessage_ = "Couldn't get double or string values.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	vector<TeSlice> slices;

	if(group.groupMode_== TeEqualSteps)
		TeGroupByEqualStep(dValues.begin(), dValues.end(), group.groupNumSlices_, slices, group.groupPrecision_);
	else if(group.groupMode_== TeQuantil)
		TeGroupByQuantil(dValues.begin(), dValues.end(), group.groupNumSlices_, slices, group.groupPrecision_);
	else if(group.groupMode_== TeStdDeviation)
	{
		std::string m = Te2String(mean);
		TeGroupByStdDev(dValues.begin(), dValues.end(), group.groupStdDev_, slices, m, group.groupPrecision_);
	}
	else if(group.groupMode_== TeUniqueValue)
		TeGroupByUniqueValue(sValues, group.groupAttribute_.type_, slices, group.groupPrecision_);

	if(group.groupNullAttr_ && nullValues > 0)
	{
		TeSlice ps;
		ps.count_ = nullValues;
		ps.from_ = "Missing Data";
		ps.to_ = "Missing Data";
		slices.push_back(ps);
		group.groupNumSlices_ = slices.size() - 1;
	}
	else
		group.groupNumSlices_ = slices.size();

	if(slices.size() == 0)
	{
		errorMessage_ = "Error during build group";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

// gera a lista de objectid/classe
	visualMap.clear();

	if(group.groupAttribute_.type_ == TeSTRING)
	{
		for(unsigned int i = 0; i < objectidsString.size(); ++i)
		{
			for(unsigned int j = 0; j < slices.size(); ++j)
			{
				if(group.groupMode_== TeUniqueValue)
				{
					if(objectidsString[i].second == slices[j].from_ || objectidsString[i].second == slices[j].to_)
					{
						visualMap[objectidsString[i].first] = j;
						break;
					}
				}
				else
				{
					if(objectidsString[i].second >= slices[j].from_ && objectidsString[i].second <= slices[j].to_)
					{
						visualMap[objectidsDouble[i].first] = j;
						break;
					}
				}
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < objectidsDouble.size(); ++i)
		{
			for(unsigned int j = 0; j < slices.size(); ++j)
			{
				if(objectidsDouble[i].second >= atof(slices[j].from_.c_str()) && objectidsDouble[i].second < atof(slices[j].to_.c_str()))
				{
					visualMap[objectidsDouble[i].first] = j;
					break;
				}
			}
		}
	}
	
	if(visualMap.empty())
	{
		errorMessage_ = "Memory visual map is empty.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
	
// define uma cor padrao caso nao seja informado
	if(rampColors.size() == 0u)
		rampColors.push_back("BLUE");

	vector<TeColor> colors;

	if(!getColors(rampColors, slices.size(), colors))
	{
		errorMessage_ = "Couldn't determine ramp colors.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	legendVec.clear();

	TeVisual vPol   = *(currentTheme_->defaultLegend().visual(TePOLYGONS));
	vPol.style(TePolyTypeFill);
	TeVisual vPt    = *(currentTheme_->defaultLegend().visual(TePOINTS));
	TeVisual vLines = *(currentTheme_->defaultLegend().visual(TeLINES));

	if(!keepThemeTransparency_)
	{
		vPol.transparency(0);
		vPt.transparency(0);
		vLines.transparency(0);
	}

	for(unsigned k = 0; k < slices.size(); ++k)
	{
		vPol.color(colors[k]);
		vPt.color(colors[k]);
		vLines.color(colors[k]);

		TeLegendEntry legend(slices[k]);

		legend.group(k);		
		legend.setVisual(vPol, TePOLYGONS);
		legend.setVisual(vPt, TePOINTS);
		legend.setVisual(vLines, TeLINES);

		legendVec.push_back(legend);
	}
}

void TeMANAGER::TerraManager::drawRasterRepresentation( const TeBox& box )
{
	verifyDatabaseConnection();
//	verifyCurrentView();

	TeInitRasterDecoders();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!currentTheme_->rasterVisual())
		currentTheme_->createRasterVisual();

	vector<string> rstObjects;
	
	currentTheme_->layer()->getRasterGeometries( rstObjects, 0, box );

	if(rstObjects.empty())
		return;

	for(unsigned int i = 0; i < rstObjects.size(); ++i)
	{
		TeRaster* raster = currentTheme_->layer()->raster(rstObjects[i]);
		canvas_.draw(raster, currentTheme_->rasterVisual());
	}
}

void TeMANAGER::TerraManager::drawPolygonRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
							                 const std::string& restrictionExpression, TeBox& box,
									         const std::string& fromClause, const std::string& linkAttr)
{
	
	TeDatabasePortal* portal = getGeometryPortal(TePOLYGONS, box, restrictionExpression, fromClause, linkAttr);

	if(!portal)
		return;

	int groPos = (int)(portal->getAttributeList().size() - 3);
	int ownPos = (int)(portal->getAttributeList().size() - 2);
	int resPos = (int)(portal->getAttributeList().size() - 1);	
		
	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}
	
	bool flag = true;

	bool useMemoryLegend = !visualMap.empty();

	if(useMemoryLegend)	// do we have classes of polygons ?
	{
		do
		{
			unsigned int& visid = visualMap[std::string(portal->getData(1))];
			TeVisual& visual = *(legendVec[visid].visual(TePOLYGONS));
			canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
			canvas_.setPolygonStyle(visual.style());
			canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
			canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

			TePolygon poly;			
			flag = portal->fetchGeometry(poly);
			canvas_.draw(poly);
		}while(flag);		
	}
	else
	{
		TeGeomRepVisualMap::iterator it = themeVisualMap_.find(TePOLYGONS);

		if(it != themeVisualMap_.end())	// do we have a fixed visual?
		{
			TeVisual& visual = *(it->second);
			canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
			canvas_.setPolygonStyle(visual.style());
			canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
			canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

			do
			{
				TePolygon poly;			
				flag = portal->fetchGeometry(poly);
				canvas_.draw(poly);
			}while(flag);			
		}
		else	// use visual from TerraLib
		{
			do
			{		
				TeVisual& visual = getVisual(portal, TePOLYGONS, resPos, groPos, ownPos);			
				canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
				canvas_.setPolygonStyle(visual.style());
				canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
				canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

				TePolygon poly;			
				flag = portal->fetchGeometry(poly);
				canvas_.draw(poly);
			}while(flag);
		}
	}

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::drawCellRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
								    const std::string& restrictionExpression, TeBox& box,
									const std::string& fromClause, const std::string& linkAttr)
{
	TeDatabasePortal* portal = getGeometryPortal(TeCELLS, box, restrictionExpression, fromClause, linkAttr);

	if(!portal)
		return;

	int	groPos = (int)(portal->getAttributeList().size() - 3);
	int	ownPos = (int)(portal->getAttributeList().size() - 2);
	int	resPos = (int)(portal->getAttributeList().size() - 1);

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}

	bool flag = true;

	bool useMemoryLegend = !visualMap.empty();

	if(useMemoryLegend)	// do we have classes of polygons ?
	{
		do
		{
			unsigned int& visid = visualMap[std::string(portal->getData(1))];
			TeVisual& visual = *(legendVec[visid].visual(TePOLYGONS));
			canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
			canvas_.setPolygonStyle(visual.style());
			canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
			canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

			TeCell cell;
			flag = portal->fetchGeometry(cell);
            canvas_.draw(cell);
		}while(flag);
	}
	else
	{
		TeGeomRepVisualMap::iterator it = themeVisualMap_.find(TePOLYGONS);

		if(it != themeVisualMap_.end())	// do we have a fixed visual?
		{
			TeVisual& visual = *(it->second);
			canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
			canvas_.setPolygonStyle(visual.style());
			canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
			canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

			do
			{
				TeCell cell;
				flag = portal->fetchGeometry(cell);				
				canvas_.draw(cell);
			}while(flag);
		}
		else	// use visual from TerraLib
		{
			do
			{
                TeVisual& visual = getVisual(portal, TePOLYGONS, resPos, groPos, ownPos);
				canvas_.setPolygonColor(visual.color().red_, visual.color().green_, visual.color().blue_, visual.transparency());
				canvas_.setPolygonStyle(visual.style());
				canvas_.setPolygonLineColor(visual.contourColor().red_, visual.contourColor().green_, visual.contourColor().blue_);
				canvas_.setPolygonLineStyle(visual.contourStyle(), visual.contourWidth());

				TeCell cell;
				flag = portal->fetchGeometry(cell);				
				canvas_.draw(cell);
			}while(flag);
		}
	}

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::drawLineRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
									      const std::string& restrictionExpression, TeBox& box,
									      const std::string& fromClause, const std::string& linkAttr)
{
	TeDatabasePortal* portal = getGeometryPortal(TeLINES, box, restrictionExpression, fromClause, linkAttr);

	if(!portal)
		return;

	int	groPos = (int)(portal->getAttributeList().size() - 3);
	int	ownPos = (int)(portal->getAttributeList().size() - 2);
	int	resPos = (int)(portal->getAttributeList().size() - 1);

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}

	bool flag = true;

	bool useMemoryLegend = !visualMap.empty();

	if(useMemoryLegend)	// do we have classes of polygons ?
	{
		do
		{
			unsigned int& visid = visualMap[std::string(portal->getData(1))];
			TeVisual& visual = *(legendVec[visid].visual(TeLINES));
			canvas_.setLineColor(visual.color().red_, visual.color().green_, visual.color().blue_);
			canvas_.setLineStyle(visual.style(), visual.width());

			TeLine2D line;
			flag = portal->fetchGeometry(line);
			canvas_.draw(line);
		}while(flag);
	}
	else
	{
		TeGeomRepVisualMap::iterator it = themeVisualMap_.find(TeLINES);

		if(it != themeVisualMap_.end())	// do we have a fixed visual?
		{
			TeVisual& visual = *(it->second);
			canvas_.setLineColor(visual.color().red_, visual.color().green_, visual.color().blue_);
			canvas_.setLineStyle(visual.style(), visual.width());

			do
			{
				TeLine2D line;
				flag = portal->fetchGeometry(line);
				canvas_.draw(line);
			}
			while(flag);
		}
		else
		{
			do
			{
				TeVisual& visual = getVisual(portal, TeLINES, resPos, groPos, ownPos);
				canvas_.setLineColor(visual.color().red_, visual.color().green_, visual.color().blue_);
				canvas_.setLineStyle(visual.style(), visual.width());

				TeLine2D line;
                flag = portal->fetchGeometry(line);
                canvas_.draw(line);
			}
			while(flag);
		}
	}

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::drawPointRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
			                               const std::string& restrictionExpression, TeBox& box,
									       const std::string& fromClause, const std::string& linkAttr)
{
	TeDatabasePortal* portal = getGeometryPortal(TePOINTS, box, restrictionExpression, fromClause, linkAttr);

	if(!portal)
		return;

	int	groPos = (int)(portal->getAttributeList().size() - 3);
	int	ownPos = (int)(portal->getAttributeList().size() - 2);
	int	resPos = (int)(portal->getAttributeList().size() - 1);

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}

	bool flag = true;

	bool useMemoryLegend = !visualMap.empty();

	if(useMemoryLegend)	// do we have classes of polygons ?
	{
		do
		{
			unsigned int& visid = visualMap[std::string(portal->getData(1))];
			TeVisual& visual = *(legendVec[visid].visual(TePOINTS));
			canvas_.setPointColor(visual.color().red_, visual.color().green_, visual.color().blue_);
			canvas_.setPointStyle(visual.style(), visual.width());

			TePoint point;
			flag = portal->fetchGeometry(point);
            canvas_.draw(point);
		}while(flag);
	}
	else
	{
		TeGeomRepVisualMap::iterator it = themeVisualMap_.find(TePOINTS);

		if(it != themeVisualMap_.end())	// do we have a fixed visual?
		{
			TeVisual& visual = *(it->second);
			canvas_.setPointColor(visual.color().red_, visual.color().green_, visual.color().blue_);
			canvas_.setPointStyle(visual.style(), visual.size());

			do
			{
                TePoint point;
				flag = portal->fetchGeometry(point);
				canvas_.draw(point);
			}while(flag);
		}
		else
		{
			do
			{
				TeVisual& visual = getVisual(portal, TePOINTS, resPos, groPos, ownPos);
				canvas_.setPointColor(visual.color().red_, visual.color().green_, visual.color().blue_);
				canvas_.setPointStyle(visual.style(), visual.size());

				TePoint point;
				flag = portal->fetchGeometry(point);
				canvas_.draw(point);
			}while(flag);
		}
	}
	
	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::drawSelectedPolygons(std::vector<std::string>& inClauseVector, const int& themeType)
{
	TeVisual& visualPol = *(visualMap_[TePOLYGONS]);
	TeColor& color = visualPol.color();
	TeColor& contourColor = visualPol.contourColor();

	for(unsigned int i = 0; i < inClauseVector.size(); ++i)
	{
		TeDatabasePortal* portal = getSelectedGeometryPortal(TePOLYGONS, inClauseVector[i], themeType);

		if(!portal)
			return;

		if(!portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			continue;
		}

		bool flag = true;

		do
		{		
			TePolygon poly;		

			canvas_.setPolygonColor(color.red_, color.green_, color.blue_, visualPol.transparency());
			canvas_.setPolygonStyle(visualPol.style());
			canvas_.setPolygonLineColor(contourColor.red_, contourColor.green_, contourColor.blue_);
			canvas_.setPolygonLineStyle(visualPol.contourStyle(), visualPol.contourWidth());

			flag = portal->fetchGeometry(poly);
			canvas_.draw(poly);
		}while(flag);

		portal->freeResult();
		delete portal;
	}
}

void TeMANAGER::TerraManager::drawSelectedCells(std::vector<string>& inClauseVector, const int& themeType)
{
	TeVisual& visualCell = *(visualMap_[TePOLYGONS]);
	TeColor& color = visualCell.color();
	TeColor& contourColor = visualCell.contourColor();

	for(unsigned int i = 0; i < inClauseVector.size(); ++i)
	{
		TeDatabasePortal* portal = getSelectedGeometryPortal(TeCELLS, inClauseVector[i], themeType);

		if(!portal)
			return;

		if(!portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			continue;
		}

		bool flag = true;

		do
		{		
			TeCell cell;		

			canvas_.setPolygonColor(color.red_, color.green_, color.blue_, visualCell.transparency());
			canvas_.setPolygonStyle(visualCell.style());
			canvas_.setPolygonLineColor(contourColor.red_, contourColor.green_, contourColor.blue_);
			canvas_.setPolygonLineStyle(visualCell.contourStyle(), visualCell.contourWidth());

			flag = portal->fetchGeometry(cell);
			canvas_.draw(cell);
		}while(flag);

		portal->freeResult();
		delete portal;
	}
}

void TeMANAGER::TerraManager::drawSelectedLines(std::vector<std::string>& inClauseVector, const int& themeType)
{
	TeVisual& visualLines = *(visualMap_[TeLINES]);
	TeColor& color = visualLines.color();

	for(unsigned int i = 0; i < inClauseVector.size(); ++i)
	{
		TeDatabasePortal* portal = getSelectedGeometryPortal(TeLINES, inClauseVector[i], themeType);

		if(!portal)
			return;

		if(!portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			continue;
		}

		bool flag = true;

		do
		{		
			TeLine2D line;		

			canvas_.setLineColor(color.red_, color.green_, color.blue_);
			canvas_.setLineStyle(visualLines.style(), visualLines.width());

			flag = portal->fetchGeometry(line);
			canvas_.draw(line);

		}while(flag);

		portal->freeResult();
		delete portal;
	}
}

void TeMANAGER::TerraManager::drawSelectedPoints(std::vector<std::string>& inClauseVector, const int& themeType)
{
	TeVisual& visualPts = *(visualMap_[TePOINTS]);
	TeColor& color = visualPts.color();

	for(unsigned int i = 0; i < inClauseVector.size(); ++i)
	{
		TeDatabasePortal* portal = getSelectedGeometryPortal(TePOINTS, inClauseVector[i], themeType);

		if(!portal)
			return;

		if(!portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			continue;
		}

		bool flag = true;

		do
		{		
			TePoint pt;		

			canvas_.setPointColor(color.red_, color.green_, color.blue_);
			canvas_.setPointStyle(visualPts.style(), visualPts.size());

			flag = portal->fetchGeometry(pt);
			canvas_.draw(pt);

		}while(flag);

		portal->freeResult();
		delete portal;
	}
}

void TeMANAGER::TerraManager::drawTextLabeling(const std::string& restrictionExpression,
								    TeBox& box,
                                    const std::string& fromClause,
									const std::string& linkAttr)
{
	TeDatabasePortal* portal = getTextLabelingPortal(restrictionExpression, box, fromClause, linkAttr);

	if(!portal)
		return;

	TeVisual& vis = *(visualMap_[TeTEXT]);
	
	canvas_.setTextColor(vis.color().red_, vis.color().green_, vis.color().blue_);
	canvas_.setTextSize(vis.width());
	canvas_.setTextStyle(vis.family());

/*	if(textOutlineEnable_)
		canvas_.setTextOutlineColor(textOutlineColor_.red_, textOutlineColor_.green_, textOutlineColor_.blue_);
	else
		canvas_.setTextOutlineColor(vis.color().red_, vis.color().green_, vis.color().blue_);
*/
	if(conflicDetect_)
	{
		while(portal->fetchRow())
		{
			std::string label = portal->getData(0);
			double x = portal->getDouble(1);
			double y = portal->getDouble(2);	

			TeCoord2D c(x, y);

			canvas_.draw(c, label, minCollisionTol_,  vis.ptAngle(), vis.alignmentHoriz(), vis.alignmentVert());
		}
	}
	else
	{
		while(portal->fetchRow())
		{
			std::string label = portal->getData(0);
			double x = portal->getDouble(1);
			double y = portal->getDouble(2);
			TeCoord2D c(x, y);

			canvas_.draw(c, label, (double)(vis.ptAngle()), vis.alignmentHoriz(), vis.alignmentVert());
		}
	}

	portal->freeResult();
	delete portal;
}

TeDatabasePortal* TeMANAGER::TerraManager::getGeometryPortal(const TeGeomRep& geomRep, TeBox& box,
			                                      const std::string& restrictionExpression,
												  const std::string& fromClause,
                                                  const string& linkAttr)
{
	TeDatabasePortal* portal = getPortal();

	std::string tableName = currentTheme_->layer()->tableName(geomRep);
	std::string wherebox = db_->getSQLBoxWhere(box, geomRep, tableName);
	std::string collectionTableName = currentTheme_->collectionTable();

	std::string queryObjects  = "";

	if(fromClause.empty())
	{
		if(restrictionExpression.empty())
		{
			queryObjects  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
			queryObjects += " FROM " + tableName + ", " + currentTheme_->collectionTable();
			queryObjects += " WHERE object_id = c_object_id";
	
			if(wherebox.empty() == false)
				queryObjects += " AND " + wherebox;
		}
		else
		{
			queryObjects  = "SELECT " + tableName + ".*, " + collectionTableName + ".c_legend_id, ";
			queryObjects += collectionTableName + ".c_legend_own, " + collectionTableName + ".c_object_status ";
			queryObjects += currentTheme_->sqlGridFrom(tableName);
	
			std::string sqlWhere = "";
	
			if(wherebox.empty() == false)
			{
				if(sqlWhere.empty())
					sqlWhere += wherebox;
				else
					sqlWhere += " AND "+ wherebox;
			}
	
			if(restrictionExpression.empty() == false)
			{
				if(sqlWhere.empty())
					sqlWhere += restrictionExpression;
				else
					sqlWhere += " AND "+ restrictionExpression;
			}
	
			queryObjects += " WHERE " + sqlWhere;
		}
	}
	else
	{
		queryObjects  = "SELECT " + tableName + ".*, " + collectionTableName + ".c_legend_id, ";
		queryObjects += collectionTableName + ".c_legend_own, " + collectionTableName + ".c_object_status ";
		queryObjects += " FROM ("  + fromClause;
		queryObjects += " INNER JOIN " + collectionTableName;
		queryObjects += " ON (" + collectionTableName + ".c_object_id = " + linkAttr;
		queryObjects += ")) INNER JOIN " +  tableName + " ON (" + tableName + ".object_id" + " = " + collectionTableName + ".c_object_id)";
		
		std::string sqlWhere = "";
	
		if(wherebox.empty() == false)
		{
			if(sqlWhere.empty())
				sqlWhere += wherebox;
			else
				sqlWhere += " AND "+ wherebox;
		}

		if(restrictionExpression.empty() == false)
		{
			if(sqlWhere.empty())
				sqlWhere += restrictionExpression;
			else
				sqlWhere += " AND "+ restrictionExpression;
		}

		queryObjects += " WHERE " + sqlWhere;
	}

	if(geomRep == TePOLYGONS)
	{
		if(db_->dbmsName() != "OracleSpatial" && db_->dbmsName() != "PostGIS")
			queryObjects += " ORDER BY "+ tableName +".parent_id ASC, "+ tableName +".num_holes DESC";
	}

	if(portal->query(queryObjects, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
	{
		errorMessage_ = "Couldn't load objects geometry: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	return portal;
}

TeDatabasePortal* TeMANAGER::TerraManager::getSelectedGeometryPortal(const TeGeomRep& geomRep, const string& inClause, const int& themeType)
{
	TeTheme* theme = 0;
	
	if(themeType == 0 && currentTheme_)
	{
		theme = currentTheme_;
	}
	else if(themeType == 1 && referenceTheme_)
	{
		theme = referenceTheme_;
	}
	else
	{
		errorMessage_ = "Couldn't find specified theme type or there isn't an active theme.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
	
	TeDatabasePortal* portal = getPortal();

	std::string tableName = theme->layer()->tableName(geomRep);

	std::string collectionTableName = theme->collectionTable();

	std::string queryObjects  = "SELECT " + tableName + ".*, c_legend_id, c_legend_own, c_object_status";
		        queryObjects += " FROM " + tableName + ", " + collectionTableName;
		        queryObjects += " WHERE object_id = c_object_id AND object_id IN " + inClause;

	if(geomRep == TePOLYGONS)
	{
		if(db_->dbmsName() != "OracleSpatial" && db_->dbmsName() != "PostGIS")
			queryObjects += " ORDER BY "+ tableName +".object_id ASC, "+ tableName +".parent_id ASC, "+ tableName +".num_holes DESC";
	}

	if(portal->query(queryObjects, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
	{
		errorMessage_ = "Couldn't load objects geometry: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	} 

	return portal;
}

TeDatabasePortal* TeMANAGER::TerraManager::getTextLabelingPortal(const std::string& restrictionExpression,
													  TeBox& box,
                                                      const std::string& fromClause,
													  const std::string& linkAttr)
{
	if(curentThemeLabelingField_.empty())
		return 0;

// collection table has two fields: label_x and label_y.
// we will get only attributes for wich label_x and label_y are inside the bounding box
// and that satisfy the restriction expression
	std::string collectionTableName = currentTheme_->collectionTable();
	
	std::string wherebox  = " (" + collectionTableName + ".label_x > " + Te2String(box.x1_, 15);
	            wherebox += " AND " + collectionTableName + ".label_y > " + Te2String(box.y1_, 15);
	            wherebox += " AND " + collectionTableName + ".label_x < " + Te2String(box.x2_, 15);
	            wherebox += " AND " + collectionTableName + ".label_y < " + Te2String(box.y2_, 15) + ") ";

// mount SQL
	std::string queryObjects = "";

	if(fromClause.empty())
	{		
        queryObjects  = "SELECT MIN(" + curentThemeLabelingField_ + "), MIN(label_x), MIN(label_y) ";
        queryObjects += currentTheme_->sqlFrom();
        queryObjects += " WHERE " + wherebox;

        if(restrictionExpression.empty() == false)
            queryObjects += " AND "  + restrictionExpression;
		
		queryObjects += " GROUP BY " + collectionTableName + ".c_object_id ";

        if(!curentThemePriorityField_.empty())
            queryObjects += " ORDER BY MIN(" + curentThemePriorityField_ + ") DESC ";
	}
	else
	{
		queryObjects  = "SELECT " + curentThemeLabelingField_ + ", label_x, label_y ";
        queryObjects += " FROM ("  + fromClause;
        queryObjects += " INNER JOIN " + collectionTableName;
        queryObjects += " ON (" + collectionTableName + ".c_object_id = " + linkAttr;
        queryObjects += ")) WHERE " + wherebox;

        if(!restrictionExpression.empty())
            queryObjects += " AND "  + restrictionExpression;

        if(!curentThemePriorityField_.empty())
            queryObjects += " ORDER BY " + curentThemePriorityField_ + " DESC ";
	}

	TeDatabasePortal* portal = getPortal();

	if(portal->query(queryObjects) == false)
	{
		errorMessage_ = "Couldn't load text for labeling: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	return portal;
}

TeVisual& TeMANAGER::TerraManager::getVisual(TeDatabasePortal* portal, const TeGeomRep& geomRep,
			                      const int& resPos, const int& groPos, const int& ownPos)
{
	int status = portal->getInt(resPos);
	int ownId = portal->getInt(ownPos);
	int groId = portal->getInt(groPos);

	if(status == 0)
	{
// so pode ser visual proprio ou outros visual qualquer
		if(ownId != 0)	// tem visual proprio?
			return *(db_->legendMap()[ownId]->visual(geomRep));
		else if(groId != 0)
			return *(db_->legendMap()[groId]->visual(geomRep));
		else
			return *(currentTheme_->defaultLegend().visual(geomRep));
	}
	else if(status == 1)	// apontado
		return *(currentTheme_->pointingLegend().visual(geomRep));
	else if(status == 2)	// consultado
		return *(currentTheme_->queryLegend().visual(geomRep));
	else if(status == 3)	// apontado e consultado
		return *(currentTheme_->queryAndPointingLegend().visual(geomRep));
	else	// default
		return *(currentTheme_->defaultLegend().visual(geomRep));
}

void TeMANAGER::TerraManager::verifyDatabaseConnection() const
{
	if(db_)
		return;

    errorMessage_ = "Database connection is empty!";
    throw TerraManagerException(errorMessage_, "TerraManagerException");
}

void TeMANAGER::TerraManager::verifyCurrentView() const
{
	if(currentView_)
		return;

    errorMessage_ = "Please, set the current view first!";
    throw TerraManagerException(errorMessage_, "TerraManagerException");
}

TeDatabasePortal* TeMANAGER::TerraManager::getPortal() const
{
	TeDatabasePortal* portal = db_->getPortal();

	if(!portal)
	{
		errorMessage_ = "Couldn't get a database portal. ";
		
		if(db_)
			errorMessage_ += db_->errorMessage();

		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}	

	return portal;
}

/////////////////////////////////////////////////
void TeMANAGER::TerraManager::setConflictDetect(const bool& detect)
{
	conflicDetect_ = detect;
}

void TeMANAGER::TerraManager::setMinCollisionTolerance(const int& numPixels)
{
	if((numPixels < 0) || (numPixels > canvas_.getWidth()))
	{
		errorMessage_ = "The collision tolerance is out of bounds!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
		
	minCollisionTol_ = numPixels;
}

void TeMANAGER::TerraManager::clearConflictCache()
{
	canvas_.clearConflictCache();
}

void TeMANAGER::TerraManager::setPriorityField(const std::string& fieldName)
{
	curentThemePriorityField_ = fieldName;
}

void TeMANAGER::TerraManager::setGeneralizedPixels(const int& n)
{
	generalizedPixels_ = n;
}

void TeMANAGER::TerraManager::setLabelField(const std::string& fieldName)
{
	curentThemeLabelingField_ = fieldName;
}

void TeMANAGER::TerraManager::setImageMapProperties(const std::string& mapName, const std::string& mapId)
{
	delete imcanvas_;
	imcanvas_ = new TeImageMapCanvas(mapName, mapId);
}

void TeMANAGER::TerraManager::closeImageMap(const bool& hasToClose)
{
	closeImgMapCanvasAtEnd_ = hasToClose;
}

void TeMANAGER::TerraManager::setAreaShape(const TeImageMapCanvas::TeImageMapShapeType& areaShapeType)
{
	if(!imcanvas_)
	{
		errorMessage_ = "You must set Map Properties before calling setAreaShape method.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

    imcanvas_->setAreaShape(areaShapeType);
}

void TeMANAGER::TerraManager::setAreaProperty(const std::string propertyName,
			                       const std::string& propertyValue,
						           const std::string valueSrc)
{
	if(!imcanvas_)
	{
		errorMessage_ = "You must set Map Properties before calling setAreaProperty method.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	imcanvas_->setAreaProperty(propertyName, propertyValue, valueSrc);
}

void TeMANAGER::TerraManager::setAreaCircleRadius(const int& radius)
{
	if(!imcanvas_)
	{
		errorMessage_ = "You must set Map Properties before calling setAreaCircleRadius method.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	imcanvas_->setAreaCircleRadius(radius);
}

void TeMANAGER::TerraManager::setAreaRectWidth(const int& width)
{
	if(!imcanvas_)
	{
		errorMessage_ = "You must set Map Properties before calling setAreaRectWidth method.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	imcanvas_->setAreaRectWidth(width);
}

char* TeMANAGER::TerraManager::getImageMap(const std::string& from,
			                    const std::string& linkAttr,
						        const std::string& restrictionExpression)
{
	verifyCurrentView();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.isValid())
	{
		errorMessage_ = "The canvas is not prepared. Please, first use setWorld method to adjust canvas!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!imcanvas_)
		imcanvas_ = new TeImageMapCanvas("", "");
	
	imcanvas_->setWorld(canvas_.getWorld(), canvas_.getWidth(), canvas_.getHeight());

// see if scale is turned on
	if(automaticScaleControlEnable_)
	{
		double scale = getScale();
		double minscale = currentTheme_->minScale();
		double maxscale = currentTheme_->maxScale();
		double epsilon = 0.000001;

		if(minscale == 0.0)
			minscale = +TeMAXFLOAT;

		if(!((scale > (maxscale - epsilon)) && (scale < (minscale + epsilon))))
			return 0;
	}

// set canvas and data projection => the projection in wich we will draw the theme objects
	imcanvas_->setCanvasProjection(*canvas_.getCanvasProjection());
	imcanvas_->setDataProjection(*(currentTheme_->layer()->projection()));	

	const TeBox& canvasBBox = imcanvas_->getWorld();
	TeBox themeBBox = currentTheme_->layer()->box();
	if(!(*currentTheme_->layer()->projection() == *imcanvas_->getCanvasProjection()))
	{
		TeCoord2D c = themeBBox.lowerLeft();
		imcanvas_->dataWorld2World(c, c);
		themeBBox.x1_ = c.x();
		themeBBox.y1_ = c.y();

		c = themeBBox.upperRight();
		imcanvas_->dataWorld2World(c, c);
		themeBBox.x2_ = c.x();
		themeBBox.y2_ = c.y();
	}

// if layer and canvas bounding boxes intersects, draw the theme
	TeBox intersectionBox;
	if(!TeIntersection(canvasBBox, themeBBox, intersectionBox))
		return 0;

// find pixel size in theme projection
	double pixelSizeX = canvas_.getPixelSizeX();
	double pixelSizeY = canvas_.getPixelSizeY();

// send intersection box to theme projection
	if(!(*currentTheme_->layer()->projection() == *imcanvas_->getCanvasProjection()))
	{
		TeCoord2D refPt = intersectionBox.lowerLeft();
		refPt.setXY(refPt.x_ + pixelSizeX, refPt.y_ + pixelSizeY);
		imcanvas_->world2DataWorld(refPt, refPt);

		TeCoord2D c = intersectionBox.lowerLeft();
		imcanvas_->world2DataWorld(c, c);
		intersectionBox.x1_ = c.x();
		intersectionBox.y1_ = c.y();

		c = intersectionBox.upperRight();
		imcanvas_->world2DataWorld(c, c);
		intersectionBox.x2_ = c.x();
		intersectionBox.y2_ = c.y();

		pixelSizeX = fabs(refPt.x_ - intersectionBox.x1());
		pixelSizeY = fabs(refPt.y_ - intersectionBox.y1());
	}

	std::string fields;

	std::map<std::string, pair<std::string, string> >::const_iterator it = imcanvas_->getAreaProperty().begin();

	while(it != imcanvas_->getAreaProperty().end())
	{
		if(it->second.second.empty())
		{
			++it;
			continue;
		}

		if(!fields.empty())
			fields += ", ";
			
		fields += it->second.second;

		++it;
	}

	double extmin = MAX(pixelSizeX * ((double)generalizedPixels_), pixelSizeY * ((double)generalizedPixels_));

	if((currentTheme_->visibleRep() & TePOLYGONS) && (currentThemeActiveRepresentations_ & TePOLYGONS))
		getPolygonImageMap(fields, from, linkAttr, restrictionExpression, intersectionBox, extmin);

	if((currentTheme_->visibleRep() & TeCELLS) && (currentThemeActiveRepresentations_ & TeCELLS))
		getCellImageMap(fields, from, linkAttr, restrictionExpression, intersectionBox, extmin);

	if((currentTheme_->visibleRep() & TeLINES) && (currentThemeActiveRepresentations_ & TeLINES))
		getLineImageMap(fields, from, linkAttr, restrictionExpression, intersectionBox, extmin);

	if((currentTheme_->visibleRep() & TePOINTS) && (currentThemeActiveRepresentations_ & TePOINTS))
		getPointImageMap(fields, from, linkAttr, restrictionExpression, intersectionBox, extmin);
		
	if(closeImgMapCanvasAtEnd_)
		imcanvas_->close();

	return imcanvas_->getImageMap();
}

int TeMANAGER::TerraManager::getImageMapSize() const
{
	if(imcanvas_)
		return imcanvas_->getImageMapSize();
		
	return 0;
}

void TeMANAGER::TerraManager::getPolygonImageMap(const std::string& fields,
								      const std::string& from,
			                          const std::string& linkAttr,
					                  const std::string& restrictionExpression,
								      TeBox& box,
							          const double& extmin)
{
	TeDatabasePortal* portal = getImageMapGeometryPortal(TePOLYGONS, box, extmin, fields, restrictionExpression, from, linkAttr);

	if(!portal)
		return;

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}
	
	bool flag = true;

	int nfields = 0;
	std::vector<std::string> propertyNameVec;
	std::vector<std::string> propertyValVec;

	std::map<std::string, pair<std::string, string> >::const_iterator it = imcanvas_->getAreaProperty().begin();

	while(it != imcanvas_->getAreaProperty().end())
	{
		if(!it->second.second.empty())
		{
			propertyNameVec.push_back(it->first);
			propertyValVec.push_back(it->second.first);

			++nfields;
		}

		++it;
	}

	do
	{
		for(int i = 0; i < nfields; ++i)
			imcanvas_->setAreaProperty(propertyNameVec[i], propertyValVec[i], portal->getData(i));

		TePolygon poly;
		
		flag = portal->fetchGeometry(poly);

		imcanvas_->draw(poly);
	}while(flag);

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::getLineImageMap(const std::string& fields,
							       const std::string& from,
			                       const std::string& linkAttr,
					               const std::string& restrictionExpression,
								   TeBox& box,
							       const double& extmin)
{
	TeDatabasePortal* portal = getImageMapGeometryPortal(TeLINES, box, extmin, fields, restrictionExpression, from, linkAttr);

	if(!portal)
		return;

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}
	
	bool flag = true;

	int nfields = 0;
	std::vector<std::string> propertyNameVec;
	std::vector<std::string> propertyValVec;

	std::map<std::string, pair<std::string, string> >::const_iterator it = imcanvas_->getAreaProperty().begin();

	while(it != imcanvas_->getAreaProperty().end())
	{
		if(!it->second.second.empty())
		{
			propertyNameVec.push_back(it->first);
			propertyValVec.push_back(it->second.first);

			++nfields;
		}

		++it;
	}

	do
	{
		for(int i = 0; i < nfields; ++i)
			imcanvas_->setAreaProperty(propertyNameVec[i], propertyValVec[i], portal->getData(i));

		TeLine2D line;
		
		flag = portal->fetchGeometry(line);

		imcanvas_->draw(line);
	}while(flag);

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::getCellImageMap(const std::string& fields,
							       const std::string& from,
			                       const std::string& linkAttr,
					               const std::string& restrictionExpression,
							       TeBox& box,
							       const double& extmin)
{
	TeDatabasePortal* portal = getImageMapGeometryPortal(TeCELLS, box, extmin, fields, restrictionExpression, from, linkAttr);

	if(!portal)
		return;

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}
	
	bool flag = true;

	int nfields = 0;
	std::vector<std::string> propertyNameVec;
	std::vector<std::string> propertyValVec;

	std::map<std::string, pair<std::string, string> >::const_iterator it = imcanvas_->getAreaProperty().begin();

	while(it != imcanvas_->getAreaProperty().end())
	{
		if(!it->second.second.empty())
		{
			propertyNameVec.push_back(it->first);
			propertyValVec.push_back(it->second.first);

			++nfields;
		}

		++it;
	}

	do
	{
		for(int i = 0; i < nfields; ++i)
			imcanvas_->setAreaProperty(propertyNameVec[i], propertyValVec[i], portal->getData(i));

		TeCell cell;
		
		flag = portal->fetchGeometry(cell);

		imcanvas_->draw(cell);
	}while(flag);

	portal->freeResult();
	delete portal;
}

void TeMANAGER::TerraManager::getPointImageMap(const std::string& fields,
                                    const std::string& from,
                                    const std::string& linkAttr,
                                    const std::string& restrictionExpression,
                                    TeBox& box,
									const double& extmin)
{
	TeDatabasePortal* portal = getImageMapGeometryPortal(TePOINTS, box, extmin, fields, restrictionExpression, from, linkAttr);

	if(!portal)
		return;

	if(!portal->fetchRow())
	{
		portal->freeResult();
		delete portal;
		return;
	}
	
	bool flag = true;

	int nfields = 0;
	std::vector<std::string> propertyNameVec;
	std::vector<std::string> propertyValVec;

	std::map<std::string, pair<std::string, string> >::const_iterator it = imcanvas_->getAreaProperty().begin();

	while(it != imcanvas_->getAreaProperty().end())
	{
		if(!it->second.second.empty())
		{
			propertyNameVec.push_back(it->first);
			propertyValVec.push_back(it->second.first);

			++nfields;
		}

		++it;
	}

	do
	{
		for(int i = 0; i < nfields; ++i)
			imcanvas_->setAreaProperty(propertyNameVec[i], propertyValVec[i], portal->getData(i));

		TePoint pt;
		
		flag = portal->fetchGeometry(pt);

		imcanvas_->draw(pt);
	}while(flag);

	portal->freeResult();
	delete portal;
}

TeDatabasePortal* TeMANAGER::TerraManager::getImageMapGeometryPortal(const TeGeomRep& geomRep,
			                                              TeBox& box,
														  const double& extmin,
													      const std::string& fields,
			                                              const std::string& restrictionExpression,
													      const std::string& fromClause,
											              const string& linkAttr)
{
	TeDatabasePortal* portal = getPortal();

	std::string tableName = currentTheme_->layer()->tableName(geomRep);
	std::string wherebox = db_->getSQLBoxWhere(box, geomRep, tableName);


	if((extmin > 0.0) && ((geomRep & TePOLYGONS) || (geomRep & TeLINES)))
	{
		if(!wherebox.empty())
			wherebox += " AND ";

		wherebox += "ext_max > " + Te2String(extmin, 15) + " ";
	}

	std::string collectionTableName = currentTheme_->collectionTable();

	std::string queryObjects  = "";

	if(fromClause.empty())
	{
		if(restrictionExpression.empty())
		{
			queryObjects  = "SELECT " + fields + ", " + tableName + ".* ";
			queryObjects += currentTheme_->sqlGridFrom(tableName);
	
			if(wherebox.empty() == false)
				queryObjects += " WHERE " + wherebox;
		}
		else
		{
			queryObjects  = "SELECT " + fields + ", " + tableName + ".* ";
			queryObjects += currentTheme_->sqlGridFrom(tableName);
	
			std::string sqlWhere = "";
	
			if(wherebox.empty() == false)
			{
				if(sqlWhere.empty())
					sqlWhere += wherebox;
				else
					sqlWhere += " AND "+ wherebox;
			}
	
			if(restrictionExpression.empty() == false)
			{
				if(sqlWhere.empty())
					sqlWhere += restrictionExpression;
				else
					sqlWhere += " AND "+ restrictionExpression;
			}
	
			queryObjects += " WHERE " + sqlWhere;
		}
	}
	else
	{
		queryObjects  = "SELECT " + fields + ", " + tableName + ".* ";
		queryObjects += " FROM ("  + fromClause;
		queryObjects += " INNER JOIN " + collectionTableName;
		queryObjects += " ON (" + collectionTableName + ".c_object_id = " + linkAttr;
		queryObjects += ")) INNER JOIN " +  tableName + " ON (" + tableName + ".object_id" + " = " + collectionTableName + ".c_object_id)";
		
		std::string sqlWhere = "";
	
		if(wherebox.empty() == false)
		{
			if(sqlWhere.empty())
				sqlWhere += wherebox;
			else
				sqlWhere += " AND "+ wherebox;
		}

		if(restrictionExpression.empty() == false)
		{
			if(sqlWhere.empty())
				sqlWhere += restrictionExpression;
			else
				sqlWhere += " AND "+ restrictionExpression;
		}

		queryObjects += " WHERE " + sqlWhere;
	}

	if(geomRep == TePOLYGONS)
	{
		if(db_->dbmsName() != "OracleSpatial" && db_->dbmsName() != "PostGIS")
			queryObjects += " ORDER BY "+ tableName +".parent_id ASC, "+ tableName +".num_holes DESC";
	}

	if(portal->query(queryObjects, TeSERVERSIDE, TeUNIDIRECTIONAL, TeREADONLY, TeBINARYCURSOR) == false)
	{
		errorMessage_ = "Couldn't load image map objects geometry portal: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	return portal;
}

void TeMANAGER::TerraManager::drawCartogram(const std::vector<std::string>& fields,
								 const std::string& fieldsFunc,
						         const TeCartogramParameters& cartogramParameters,
						         const std::string& proportionalToField,
			                     const std::string& from,
			                     const std::string& linkAttr,
						         const std::string& restrictionExpression)
{
	verifyCurrentView();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.isValid())
	{
		errorMessage_ = "The canvas is not prepared. Please, first use setWorld method to adjust canvas!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

// see if scale is turned on
	if(automaticScaleControlEnable_)
	{
		double scale = getScale();
		double minscale = currentTheme_->minScale();
		double maxscale = currentTheme_->maxScale();
		double epsilon = 0.000001;

		if(minscale == 0.0)
			minscale = +TeMAXFLOAT;

		if(!((scale > (maxscale - epsilon)) && (scale < (minscale + epsilon))))
			return;
	}

// set canvas and data projection => the projection in wich we will draw the theme objects
	canvas_.setDataProjection(*(currentTheme_->layer()->projection()));

	const TeBox& canvasBBox = canvas_.getWorld();
	TeBox themeBBox = currentTheme_->layer()->box();
	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = themeBBox.lowerLeft();
		canvas_.dataWorld2World(c, c);
		themeBBox.x1_ = c.x();
		themeBBox.y1_ = c.y();

		c = themeBBox.upperRight();
		canvas_.dataWorld2World(c, c);
		themeBBox.x2_ = c.x();
		themeBBox.y2_ = c.y();
	}

// if layer and canvas bounding boxes intersects, draw the theme
	TeBox intersectionBox;
	if(!TeIntersection(canvasBBox, themeBBox, intersectionBox))
		return;

	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = intersectionBox.lowerLeft();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x1_ = c.x();
		intersectionBox.y1_ = c.y();

		c = intersectionBox.upperRight();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x2_ = c.x();
		intersectionBox.y2_ = c.y();
	}

// encontrar os maiores e menores valores de cada atributo
	double chartMaxValue;
	double chartMinValue;
	getChartBoundValues(chartMaxValue, chartMinValue, fields, fieldsFunc, proportionalToField, cartogramParameters.type_, from, linkAttr, restrictionExpression);

// ajusta a linha de desenho do grafico
	canvas_.setPolygonLineStyle(TeLnTypeContinuous, 1);
	canvas_.setPolygonLineColor(100, 100, 100);

// recupera os atributos de cada objeto e desenha no canvas
	if(cartogramParameters.type_ == 0)
	{
		double	nfields = fields.size();

		//double delta = cartogramParameters.maxSize_ / 2.0;
		double delta = 1.0e-6;

		TeBox box = intersectionBox;

		box.x1_ = box.x1_ - delta;
		box.x2_ = box.x2_ + delta;
		box.y1_ = box.y1_ - delta;
		box.y2_ = box.y2_ + delta;

		std::string wherebox  = "NOT(label_x > " + Te2String(box.x2_, 10) + " OR ";
					wherebox += "label_x < " + Te2String(box.x1_, 10) + " OR ";
					wherebox += "label_y > " + Te2String(box.y2_, 10) + " OR ";
					wherebox += "label_y < " + Te2String(box.y1_, 10) + ")";

		double pixelSizeX = canvas_.getPixelSizeX();
		double pixelSizeY = canvas_.getPixelSizeY();

		std::string attrs;

		for(unsigned int i = 0; i < fields.size(); ++i)
			attrs += fieldsFunc + "(" + fields[i] + "), ";

		attrs += "AVG(label_x), AVG(label_y) ";

		std::string q;

		if(!from.empty())
		{
			q  = "SELECT " + attrs;
			q += " FROM ("  + from;
			q += " INNER JOIN " + currentTheme_->collectionTable();
			q += " ON (" + currentTheme_->collectionTable() + ".c_object_id = " + linkAttr;
			q += ")) ";

			std::string sqlWhere = wherebox;
	
			if(!restrictionExpression.empty())
			{
				if(sqlWhere.empty())
					sqlWhere += restrictionExpression;
				else
					sqlWhere += " AND "+ restrictionExpression;
			}

			if(!sqlWhere.empty())
				q += " WHERE " + sqlWhere;
		}
		else if(wherebox.empty())
		{
			q = "SELECT " + attrs + currentTheme_->sqlGridFrom();
		}
		else if(!wherebox.empty())
		{
			q = "SELECT " + attrs + currentTheme_->sqlGridFrom() + " WHERE " + wherebox;
		}

		q += " GROUP BY " + currentTheme_->collectionTable() + ".c_object_id";

		if(!curentThemePriorityField_.empty())
			q += " ORDER BY " + fieldsFunc + "(" + curentThemePriorityField_ + ") DESC";
	
		TeDatabasePortal* plot = getPortal();

		if(!plot->query(q))
		{			
			errorMessage_ = "Could not get portal for bar cartogram: " + plot->errorMessage();
			plot->freeResult();
			delete plot;
			throw TerraManagerException(errorMessage_, "TerraManager");
		}

		TeCoord2D c;
		int x, y;
		int dx = nfields * (cartogramParameters.width_) / 2;
		int deltasize = cartogramParameters.maxSize_ - cartogramParameters.minSize_;
		double deltaVal = (chartMaxValue - chartMinValue);
		double deltaHeight = double((deltasize)) / deltaVal;

		while(plot->fetchRow())
		{
			c.x_ = atof(plot->getData(nfields));
			c.y_ = atof(plot->getData(nfields + 1));

			canvas_.dataWorld2Viewport(c, x, y);

			x = x - dx;

            std::vector<double> dvec;

            for(unsigned int i = 0; i < nfields; ++i)
			{
				std::string val = plot->getData(i);

                if(val.empty())
				{
					dvec.clear();
					break;
				}
				
				dvec.push_back(atof(val.c_str()));
			}

			if(conflicDetect_)
			{
				std::vector<int> candidatesToConflict;
				int nvals = dvec.size();

				if(canvas_.getRtree().search(TeBox(x - minCollisionTol_, y - cartogramParameters.maxSize_ - minCollisionTol_, x + (nvals * cartogramParameters.width_) + minCollisionTol_, y +  minCollisionTol_), candidatesToConflict) > 0)
					continue;

				canvas_.getRtree().insert(TeBox(x, y - cartogramParameters.maxSize_, x + (nvals * cartogramParameters.width_), y), 0);
			}

			for(unsigned int i = 0; i < dvec.size(); ++i)
			{
                double height;
				
				if(deltaVal == 0.0)
					height = 0.;
				else
					height = (dvec[i] - double(chartMinValue)) * deltaHeight  + (double)cartogramParameters.minSize_;

				const TeColor& cor = cartogramParameters.colors_[i];

				canvas_.setPolygonColor(cor.red_, cor.green_, cor.blue_);

				//if(conflicDetect_)
                //    canvas_.drawBox(x, y - height, x + cartogramParameters.width_, y, minCollisionTol_);
				//else
                    canvas_.drawBox(x, y - height, x + cartogramParameters.width_, y);

				x += cartogramParameters.width_;
			}
		}

		plot->freeResult();
		delete plot;
	}
	else
	{
		double	nfields = fields.size();

		//double delta = cartogramParameters.width_ / 2.0;
		double delta = 1.0e-6;

		//if(!proportionalToField.empty())
		//	delta = chartMaxValue / 2.;

		TeBox box = intersectionBox;

		box.x1_ = box.x1_ - delta;
		box.x2_ = box.x2_ + delta;
		box.y1_ = box.y1_ - delta;
		box.y2_ = box.y2_ + delta;

		std::string wherebox  = "NOT(label_x > " + Te2String(box.x2_, 10) + " OR ";
					wherebox += "label_x < " + Te2String(box.x1_, 10) + " OR ";
					wherebox += "label_y > " + Te2String(box.y2_, 10) + " OR ";
					wherebox += "label_y < " + Te2String(box.y1_, 10) + ")";

		std::string attrs;

		for(unsigned int i = 0; i < fields.size(); ++i)
			attrs += fieldsFunc + "(" + fields[i] + "), ";

		attrs += "AVG(label_x), AVG(label_y) ";

		if(!proportionalToField.empty())
			attrs += ", " + fieldsFunc + "(" + proportionalToField + ") ";

		std::string q;

		if(!from.empty())
		{
			q  = "SELECT " + attrs;
			q += " FROM ("  + from;
			q += " INNER JOIN " + currentTheme_->collectionTable();
			q += " ON (" + currentTheme_->collectionTable() + ".c_object_id = " + linkAttr;
			q += ")) ";

			std::string sqlWhere = wherebox;

			if(!restrictionExpression.empty())
				sqlWhere += " AND "+ restrictionExpression;

			if(!sqlWhere.empty())
				q += " WHERE " + sqlWhere;
		}
		else if(wherebox.empty())
			q = "SELECT " + attrs + currentTheme_->sqlGridFrom();
		else if (!wherebox.empty())
			q = "SELECT " + attrs + currentTheme_->sqlGridFrom() + " WHERE " + wherebox;

		q += " GROUP BY " + currentTheme_->collectionTable() + ".c_object_id";

		if(!curentThemePriorityField_.empty())
			q += " ORDER BY " + fieldsFunc + "(" + curentThemePriorityField_ + ") DESC";
	
		TeDatabasePortal* plot = getPortal();

		if(!plot->query(q))
		{			
			errorMessage_ = "Could not get portal for piebar cartogram: " + plot->errorMessage();
			plot->freeResult();
			delete plot;
			throw TerraManagerException(errorMessage_, "TerraManager");
		}

		double deltaVal = (chartMaxValue - chartMinValue);
		double diameter = cartogramParameters.width_;

		while(plot->fetchRow())
		{
			double tot = 0.;

			for(unsigned int i = 0; i < nfields; ++i)
			{
				std::string v = plot->getData(i);
				
				if(v.empty())
				{
					tot = 0.;
					break;
				}

				tot += atof(v.c_str());
			}

            if(tot == 0.)
				continue;			

			if(!proportionalToField.empty())
			{
				if(deltaVal == 0.0)
					diameter = cartogramParameters.maxSize_;
				else
				{
					double	adim = atof(plot->getData(nfields + 2));
					double a = (adim - double(chartMinValue)) / deltaVal;
					diameter = a * double(cartogramParameters.maxSize_ - cartogramParameters.minSize_) + double(cartogramParameters.minSize_);
				}
			}
			
			TeCoord2D c;
			c.x_ = atof(plot->getData(nfields));
			c.y_ = atof(plot->getData(nfields + 1));
			int x, y;
			canvas_.dataWorld2Viewport(c, x, y);

			//x = x - diameter / 2;
			//y = y - diameter / 2;

			double	a = 0.;

			if(conflicDetect_)
			{
				std::vector<int> candidatesToConflict;

				if(canvas_.getRtree().search(TeBox(x - (diameter/2) - minCollisionTol_, y - (diameter/2) -  minCollisionTol_, x + (diameter/2) + minCollisionTol_, y + (diameter/2) +  minCollisionTol_), candidatesToConflict) > 0)
					continue;

				canvas_.getRtree().insert(TeBox(x - (diameter/2), y - (diameter/2), x + (diameter/2), y + (diameter/2)), 0);
			}

			for(unsigned int i = 0; i < nfields; ++i)
			{
				double b = atof(plot->getData(i)) / tot * 360. + a;
				const TeColor& cor = cartogramParameters.colors_[i];
				canvas_.setPolygonColor(cor.red_, cor.green_, cor.blue_);
				//if(conflicDetect_)
                //    canvas_.drawArc(x, y, diameter, diameter, a, b, minCollisionTol_);
				//else
					canvas_.drawArc(x, y, diameter, diameter, a, b);

				a = b;
			}
		}
	}
}

void TeMANAGER::TerraManager::saveThemeToFile(const std::string& filaName)
{
	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.isValid())
	{
		errorMessage_ = "The canvas is not prepared. Please, first use setWorld method to adjust canvas!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

// set canvas and data projection => the projection in wich we will draw the theme objects
	canvas_.setDataProjection(*(currentTheme_->layer()->projection()));

	const TeBox& canvasBBox = canvas_.getWorld();
	TeBox themeBBox = currentTheme_->layer()->box();
	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = themeBBox.lowerLeft();
		canvas_.dataWorld2World(c, c);
		themeBBox.x1_ = c.x();
		themeBBox.y1_ = c.y();

		c = themeBBox.upperRight();
		canvas_.dataWorld2World(c, c);
		themeBBox.x2_ = c.x();
		themeBBox.y2_ = c.y();
	}

// if layer and canvas bounding boxes intersects, draw the theme
	TeBox intersectionBox;
	if(!TeIntersection(canvasBBox, themeBBox, intersectionBox))
		return;

	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = intersectionBox.lowerLeft();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x1_ = c.x();
		intersectionBox.y1_ = c.y();

		c = intersectionBox.upperRight();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x2_ = c.x();
		intersectionBox.y2_ = c.y();
	}

	TeQuerierParams qParams(true, true);
	qParams.setParams(currentTheme_);
	qParams.setSpatialRest(intersectionBox, TeINTERSECTS, static_cast<TeGeomRep>(currentThemeActiveRepresentations_));
	TeQuerier querier(qParams);
	if(!TeExportQuerierToShapefile(&querier, filaName))
	{
		errorMessage_ = "Could not generate shape file.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}
}

void TeMANAGER::TerraManager::setThemeVisual(const TeVisual& vis, const TeGeomRep& representation)
{
	TeGeomRepVisualMap::iterator it = themeVisualMap_.find(representation);

	if(it != themeVisualMap_.end())
	{
		*(it->second) = vis;
	}
	else
	{
		TeVisual* newvis = new TeVisual(vis);
		themeVisualMap_[representation] = newvis;
	}
	
	return;
}

void TeMANAGER::TerraManager::drawText()
{
	verifyCurrentView();

	if(!currentTheme_)
	{
		errorMessage_ = "Please, set a current theme first.";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	if(!canvas_.isValid())
	{
		errorMessage_ = "The canvas is not prepared. Please, first use setWorld method to adjust canvas!";
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

// see if scale is turned on
	if(automaticScaleControlEnable_)
	{
		double scale = getScale();
		double minscale = currentTheme_->minScale();
		double maxscale = currentTheme_->maxScale();
		double epsilon = 0.000001;

		if(minscale == 0.0)
			minscale = +TeMAXFLOAT;

		if(!((scale > (maxscale - epsilon)) && (scale < (minscale + epsilon))))
			return;
	}

// set canvas and data projection => the projection in wich we will draw the theme objects
	canvas_.setDataProjection(*(currentTheme_->layer()->projection()));

	const TeBox& canvasBBox = canvas_.getWorld();
	TeBox themeBBox = currentTheme_->layer()->box();
	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = themeBBox.lowerLeft();
		canvas_.dataWorld2World(c, c);
		themeBBox.x1_ = c.x();
		themeBBox.y1_ = c.y();

		c = themeBBox.upperRight();
		canvas_.dataWorld2World(c, c);
		themeBBox.x2_ = c.x();
		themeBBox.y2_ = c.y();
	}

// if layer and canvas bounding boxes intersects, draw the theme
	TeBox intersectionBox;
	if(!TeIntersection(canvasBBox, themeBBox, intersectionBox))
		return;

	if(!(*currentTheme_->layer()->projection() == *canvas_.getCanvasProjection()))
	{
		TeCoord2D c = intersectionBox.lowerLeft();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x1_ = c.x();
		intersectionBox.y1_ = c.y();

		c = intersectionBox.upperRight();
		canvas_.world2DataWorld(c, c);
		intersectionBox.x2_ = c.x();
		intersectionBox.y2_ = c.y();
	}

	if(!((currentTheme_->visibleRep() & TeLINES) && (currentThemeActiveRepresentations_ & TeLINES)))
		return;


	if(curentThemeLabelingField_.empty())
		return;

	std::string collectionTableName = currentTheme_->collectionTable();
	
	std::string wherebox  = " (" + collectionTableName + ".label_x > " + Te2String(intersectionBox.x1_, 15);
	            wherebox += " AND " + collectionTableName + ".label_y > " + Te2String(intersectionBox.y1_, 15);
	            wherebox += " AND " + collectionTableName + ".label_x < " + Te2String(intersectionBox.x2_, 15);
	            wherebox += " AND " + collectionTableName + ".label_y < " + Te2String(intersectionBox.y2_, 15) + ") ";

	std::string tableName = currentTheme_->layer()->tableName(TeLINES);

// mount SQL
	std::string queryObjects = "";

	queryObjects  = "SELECT " + curentThemeLabelingField_ + ", " + tableName + ".* ";
    queryObjects += currentTheme_->sqlGridFrom(tableName);
    queryObjects += " WHERE " + wherebox;

	TeDatabasePortal* portal = getPortal();

	if(portal->query(queryObjects) == false)
	{
		errorMessage_ = "Couldn't load text for labeling: " + portal->errorMessage();
		portal->freeResult();
		delete portal;
		throw TerraManagerException(errorMessage_, "TerraManagerException");
	}

	TeVisual& vis = *(visualMap_[TeTEXT]);
	
	canvas_.setTextColor(vis.color().red_, vis.color().green_, vis.color().blue_);
	canvas_.setTextSize(vis.width());
	canvas_.setTextStyle(vis.family());

	if(conflicDetect_)
	{
		bool flag = portal->fetchRow();

		do
		{
            std::string label = portal->getData(0);
			TeLine2D l;
			flag = portal->fetchGeometry(l);

			double angle = 0.0;
			TeCoord2D c;
			TeGetLabelPoint(l, c, angle);

			canvas_.draw(c, label, minCollisionTol_,  angle, vis.alignmentHoriz(), vis.alignmentVert());
		}while(flag);
	}
	else
	{
		bool flag = portal->fetchRow();

		do
		{
            std::string label = portal->getData(0);
			TeLine2D l;
			flag = portal->fetchGeometry(l);

			double angle = 0.0;
			TeCoord2D c;
			TeGetLabelPoint(l, c, angle);

			canvas_.draw(c, label, angle, vis.alignmentHoriz(), vis.alignmentVert());
		}while(flag);
	}

	portal->freeResult();
	delete portal;

}

void TeMANAGER::TerraManager::getChartBoundValues(double& chartMaxValue,
			                           double& chartMinValue,
									   const std::vector<std::string>& fields,
									   const std::string& fieldsFunc,
								       const std::string& proportionalToField,
									   const int& chartType,
								       const std::string& from,
									   const std::string& attrLink,
								       const std::string& restrictionExpression)
{
	chartMaxValue = -TeMAXFLOAT;
	chartMinValue = +TeMAXFLOAT;

	if(chartType == 0)
	{
		string q = "SELECT ";

		for(unsigned int i = 0; i < fields.size(); ++i)
		{
			if(i != 0)
				q += ", ";

			q += fieldsFunc + "(" + fields[i] + ") AS F" + Te2String(i);
		}

		q += " ";

		if(from.empty())
		{
			q += currentTheme_->sqlGridFrom();
			if(!restrictionExpression.empty())
			{
				q += " WHERE " + restrictionExpression;
			}
			q += " GROUP BY " + currentTheme_->collectionTable() + ".c_object_id";
		}
		else
		{
			q += " FROM " + from;
			if(!restrictionExpression.empty())
			{
				q += " WHERE " + restrictionExpression;
			}
			q += " GROUP BY " + attrLink;
		}	

		string qq = "SELECT ";

		for(unsigned int i = 0; i < fields.size(); ++i)
		{
			if(i != 0)
				qq += ", ";

			qq += "MIN(T.F" + Te2String(i) + "), MAX(T.F" + Te2String(i) + ")";
		}

		if((db_->dbmsName() != "OracleAdo") && (db_->dbmsName()!="OracleSpatial"))
			qq += " FROM (" + q + ") AS T";
		else
			qq += " FROM (" + q + ") T";

		TeDatabasePortal* portal = getPortal();

		if(!portal->query(qq) || !portal->fetchRow())
		{
			portal->freeResult();
			delete portal;
			errorMessage_ = "Could not find MIN and MAX values in order to draw the cartogram.";
			throw TerraManagerException(errorMessage_, "TerraManagerException");
		}

		for(unsigned int i = 0; i < fields.size(); ++i)
		{
			double minv = atof(portal->getData(i * 2));
			double maxv = atof(portal->getData(i * 2 + 1));

			chartMaxValue = MAX(maxv, chartMaxValue);
			chartMinValue = MIN(minv, chartMinValue);
		}

		portal->freeResult();
		delete portal;
	}
	else
	{
		if(!proportionalToField.empty())
		{
			string q = "SELECT " + fieldsFunc + "(" + proportionalToField + ") AS F ";

			if(from.empty())
			{
				q += currentTheme_->sqlGridFrom();
				if(!restrictionExpression.empty())
				{
					q += " WHERE " + restrictionExpression;
				}
				q += " GROUP BY " + currentTheme_->collectionTable() + ".c_object_id";
			}
			else
			{
				q += " FROM " + from;
				if(!restrictionExpression.empty())
				{
					q += " WHERE " + restrictionExpression;
				}
				q += " GROUP BY " + attrLink;
			}

			string qq;
			if((db_->dbmsName()!="OracleAdo") && (db_->dbmsName()!="OracleSpatial"))
				qq = "SELECT MIN(T.F), MAX(T.F) FROM (" + q + ") AS T";
			else
				qq = "SELECT MIN(T.F), MAX(T.F) FROM (" + q + ") T";

			TeDatabasePortal* portal = getPortal();

			if(!portal->query(qq) || !portal->fetchRow())
			{
				portal->freeResult();
				delete portal;
				errorMessage_ = "Could not find MIN and MAX values in order to draw the cartogram.";
				throw TerraManagerException(errorMessage_, "TerraManagerException");
			}

			chartMinValue = atof(portal->getData(0));
			chartMaxValue = atof(portal->getData(1));

			portal->freeResult();
			delete portal;
		}
	}
}

void TeMANAGER::TerraManager::clearVisualMap()
{
	TeGeomRepVisualMap::iterator it = visualMap_.begin();

	while(it != visualMap_.end())
	{
		delete (it->second);
		++it;
	}

	visualMap_.clear();
}

void TeMANAGER::TerraManager::clearThemeVisualMap()
{
	TeGeomRepVisualMap::iterator it = themeVisualMap_.begin();

	while(it != themeVisualMap_.end())
	{
		delete (it->second);
		++it;
	}

	themeVisualMap_.clear();
}




