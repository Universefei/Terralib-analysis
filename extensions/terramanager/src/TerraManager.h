/** \file TerraManager.h
  * \brief This file contains a class to handle the most common functions in a GIS application.
  * \author Gilberto Ribeiro - <gribeiro@dpi.inpe.br>
  * \author Ricardo Cartaxo - <cartaxo@dpi.inpe.br>
  */

#ifndef  __TERRAMANAGER_INTERNAL_TERRAMANGER_H
#define  __TERRAMANAGER_INTERNAL_TERRAMANGER_H

// TerraManager include files
#include "TerraManagerConfig.h"
#include "TerraManagerDatatypes.h"
#include "TeGDCanvas.h"
#include "TeImageMapCanvas.h"

// TerraLib include files
#include <TeDataTypes.h>
#include <TeLegendEntry.h>
#include <TeDatabase.h>

// STL include files
#include <string>
#include <map>
#include <vector>

namespace TeMANAGER
{

	/** \class TerraManager
	  * \brief A class to be used by Web applications.
	  *
	  *
	  *
	  * \note The methods of this class may raise exceptions of type TerraManagerException.
	  * \sa TerraManagerException
	  *
	  * \todo VER SE TABELAS TEMPORAIS ESTAO SENDO TRATADAS DE ACORDO (drawCartogram, plotgroup e etc...)!!!!!!!!!!!!!!!!!
	  * \todo Plot por geomId e nao por objectid.
	  * \todo Recuperar a legenda de agrupamentos de temas que usam a collection e que jah estejam agrupados.
	  * \todo O cache de objetos TerraLib esta limitado a uma vista; Tentar quebrar este limite no futuro.
	  * \todo Remover chamada a getTableName.
	  * \todo addThemeTable => Remover essa chamada.
	  * \todo Na linha onde eh realizado a contagem do agrupamento, podemos determinar o tipo da coluna e nao precisamos fazer getData, atof e etc...
	  * \todo Corrigir a questao de projecao do metodo drawBufferZone
	  * \todo Corrigir os metodos locateObjects porque alguns podem retornar menos IDs pois estao usando so a primeira representacao.
	  * \todo Adicionar controle de escala autom�tico
	  * \todo Otimizar os metodos de desenho de objetos selecionados: hoje esta buscando um a um... buscar todos de uma vez!
	  * \todo Otimizar o metodo setTheme: nao olhar o map de temas e procurar o tema!
	  *
	  */
	class TERRAMANAGEREXPORT TerraManager
	{
		public:	

			/** @name Main Methods
			* Methods related to TerraManager initialization.
			* These methods can raise exceptions.
			*/
			//@{

			/** \brief Empty constructor.
			*/
			TerraManager();

			/** \brief Virtual destructor.
			*/
			virtual ~TerraManager();

			/** \brief Returns the error message associated to the last error ocurried.
			*/
			std::string& getErrorMessage() const;

			//@}

			/** @name Database Methods
			* Methods related to database and connection creation.
			* These methods can raise exceptions.
			*/
			//@{	

			/** \brief Opens a connection to a database server with all parameters needed,
					the connection will be automatically closed when an object is released.
			*/
			void connect(const TeDBMSType& dbType, const std::string& host,
						const std::string& user, const std::string& password,
						const std::string& database, const int& port = 0);

			/** \brief Sets the database connection used to query.
			* \param db Database connection. (Input)
			* \note The connection will not be closed by the destructor.
			*/
			void setDatabase(TeDatabase* db);

			/** \brief Closes an open connection created by TerraManager.
			*/
			void closeConnection(void);

			//@}

			/** @name View Methods
			* Methods related to View manipulation.
			* These methods can raise exceptions.
			*/
			//@{	
			
			/** \brief Returns the views in the current database for userName or the connection userName if it's blank.
			*/
			void getViews(std::vector<std::string>& views, const std::string& userName = "") const;

			/** \brief Set the current view to the specified view.
			*/
			void setCurrentView(const std::string& viewName);
			
			/** \brief Returns the current view name or empty.
				\note Returns an empty string if there is not a current view.
			*/
			std::string getCurrentViewName(void) const;

			/** \brief Returns the current view pointer.
				\note Returns NULL if there is not a current view.
			*/
			TeView* getCurrentView(void) const;
			
			/** \brief Returns the view MBR (including all themes) in the view projection or an invalid box.
			*/
			void getCurrentViewBox(TeBox& b);

			//@}		

			/** @name Theme Methods
			* Methods related to Theme manipulation.
			* These methods can raise exceptions.
			*/
			//@{

			/** \brief Returns the current view theme names.
			*/
			void getThemes(std::vector<std::string>& themes, const bool& onlyVisible = false) const;

			/** \brief Sets a theme as the current (0) or reference (1).
				\param themeName The name of the theme to be set as current or reference. (Input)
				\param themeType If you inform a value of '0' (the default value) the theme will be set as current
								otherwise (value of '1') it will be set as the reference. (Input)
				\note The content of the current theme can be draw on canvas and the reference theme can be used to set spatial restrictions or can be used in spatial queries.
			*/
			void setTheme(const std::string& themeName, const int& themeType = 0);

			/** \brief Sets a theme as the current (0) or reference (1).
				\param theme The theme to be set as current or reference. (Input)
				\param themeType If you inform a value of '0' (the default value) the theme will be set as current
								otherwise (value of '1') it will be set as the reference. (Input)
				\note The content of the current theme can be draw on canvas and the reference theme can be used to set spatial restrictions or can be used in spatial queries.
			*/
			void setTheme(TeTheme* theme, const int& themeType = 0);

			/** \brief Returns the current or refernce theme name.
				\note Returns an empty string if there are not a current or reference theme.
			*/
			std::string getThemeName(const int& themeType = 0) const;

			/** \brief Returns the current or refernce theme pointer.
				\note Returns NULL if there are not a current or reference theme.
			*/
			TeTheme* getTheme(const int& themeType = 0) const;

			/** \brief Returns the theme visible representations.
				\note Returns '0' if there is not a visible representation.
			*/
			int getThemeRepresentation(const int& themeType = 0) const;

			/** \brief Sets the current or reference theme default representation.
			*/
			void setActiveRepresentation(const int& repres, const int& themeType = 0);

			/** \brief Gets the current or reference theme active representation.
			*/
			int getActiveRepresentation(const int& themeType = 0) const;

			/** \brief Returns a theme box.
			*/
			TeBox getThemeBox(const int& themeType = 0, const std::string& restrictionExpression = "");

			//@}

			/** @name Canvas Methods
			* Methods related to canvas draw.
			* These methods can raise exceptions.
			*/
			//@{

			/** \brief Sets the commom projection used during all operations.
				\note If you don't want to use the same Current View Projection for canvas, you must call this method.
			*/
			void setWorkProjection(TeProjection& proj);

			/** \brief Sets the canvas size (width x height) and the data world box.
				\return If the world box and the canvas size does not keep the aspect ratio
						this method will adjust the world bounding box. It is the adjusted bounding
						box that is returned by this method.
				\note The world bounding box must be in the same unit as the current view.
			*/
			TeBox setWorld(const double& x1, const double& y1, const double& x2, const double& y2, const int& width, const int& height, const bool& keepAspectRatio = true);

			/** \brief Returns the pixel size in world coordinates.
			*/
			double getPixelSizeX(void) const;

			double getPixelSizeY(void) const;
			
			/** \brief Returns a PNG image with canvas content.
				\param imageType The type of image: 0 for PNG, 1 for JPEG and 2 for GIF. (Input)
				\param size The output image size in bytes. (Output)
				\param opaque If true the image will be opaque, otherwise it will have a transparent background. (Input)
				\param quality Applies only for JPEG image. (Input)
				\note The caller must clean memory using freeImage().
			*/
			void* getCanvasImage(const int& imageType, int& size, const bool& opaque = true, const int& quality = 75);

			/** \brief Saves the canvas content to a image file (PNG, JPEG or GIF).
				\param imageType The type of image file: 0 for PNG, 1 for JPEG and 2 for GIF. (Input)
				\param fileName  File name with full path. (Input)
				\param opaque If true the image will be opaque, otherwise it will have a transparent background. (Input)
				\param quality Applies only for JPEG image. (Input)
			*/
			void saveCanvasImage(const int& imageType, const std::string& fileName, const bool& opaque = true, const int& quality = 75);

			/** \brief Clean canvas.
			*/
			void cleanCanvas();

			/** \brief Free the memory returned for an image.
			*/
			static void freeImage(void* ptr);

			/** \brief Sets canvas background color.
				\note This method must be called before setWorld.
			*/
			void setCanvasBackgroundColor(const int& r, const int& g, const int& b);		

			//@}

			/** @name Visual Methods
			* Methods related on canvas drawing.
			* These methods can raise exceptions.
			*/
			//@{

			/** \brief Sets the visual used to draw objects.
			*/
			void setDefaultVisual(const TeVisual& vis, const TeGeomRep& representation);

			/** \brief Returns the visual used to draw objects.
			*/
			TeVisual& getDefaultVisual(const TeGeomRep& representation);

			/** \brief Turns on/off the text outline.
			*/
			void setTextOutLineEnable(const bool& on = false);

			/** \brief Sets the color used in text outline.
			*/
			void setTextOutLineColor(const int& r, const int& g, const int& b);

			//@}

			/** @name Drawing Methods
			* Methods related to draw themes and objects on canvas.
			* These methods can raise exceptions.
			*/
			//@{
			
			/** \brief Draws a point on canvas.
				\note Point coordinates must be on view projection.
			*/
			void draw(TeCoord2D& c);

			/** \brief Draws a box on canvas.
				\note Box coordinates must be on view projection.
			*/
			void draw(const TeBox& b);

			/** \brief Draws a line on canvas.
				\note Line coordinates must be on view projection.
			*/
			void draw(const TeLine2D& l);

			/** \brief Draws a polygon on canvas.
				\note Polygon coordinates must be on view projection.
			*/
			void draw(const TePolygon& p);

			/** \brief Draws a text on canvas.
			*/
			void draw(TeCoord2D& c, const std::string& text, const double& angle = 0.0,
					const double& horizontalAlign = 0.0, const double& verticalAlign = 0.0);

			/** \brief Draws a text on canvas.
			*/
			void draw(TeCoord2D& c, const std::string& text, const int& minCollisionTol,
					const double& angle = 0.0, const double& horizontalAlign = 0.0,
					const double& verticalAlign = 0.0);

			/** \brief Draws the current theme active representations on canvas within canvas bounding box.
				\note This is a facade for the main drawCurrentTheme method.
			*/
			void drawCurrentTheme();

			/** \brief Draws the current theme active representations on canvas within canvas bounding box.
				\param legendVec The legend vector of this theme. (Output)
				\note This is a facade for the main drawCurrentTheme method.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec);

			/** \brief Draws the current theme active representations on canvas within canvas bounding box. Only
					the objects that satisfy the restriction (restrictionExpression) will be drawn.
				\param legendVec The legend vector of this theme. (Output)
				\param restrictionExpression A restriction over theme objects. (Input)
				\note This is a facade for the main drawCurrentTheme method.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec, const std::string& restrictionExpression);

			/** \brief Draws the current theme active representations. It will generate classes
					for the objects based on the attribute group and the colors.
				\param legendVec The legend vector of this theme. (Output)
				\param group Information on how to classify the theme objects. (Input)
				\param rampColors List of base color used to draw each class of objects. (Input)
				\note This is a facade for the main drawCurrentTheme method.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group,
								std::vector<std::string>& rampColors);

			/** \brief Draws the current theme active representations. It will generate classes
					for the objects based on the attribute group and the colors. Only the obects
					that satisfy the restriction will be drawn.
				\param legendVec The legend vector of this theme. (Output)
				\param group Information on how to classify the theme objects. (Input)
				\param rampColors List of base color used to draw each class of objects. (Input)
				\param restrictionExpression A restriction over theme objects. (Input)
				\note This is the main method used to draw themes.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group,
								std::vector<std::string>& rampColors, const std::string& restrictionExpression);

			/** \brief Draws the current theme using an external SQL: linkAttr indicates
					the column used to join with object_id.
			*/
			void drawCurrentTheme(const std::string& from, const std::string& linkAttr,
								const std::string& restrictionExpression);

			/** \brief Draws the current theme using an external SQL: linkAttr indicates
					the column used to join with object_id.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec, const std::string& from,
								const std::string& linkAttr, const std::string& restrictionExpression);

			/** \brief Draws the current theme using an external SQL and makes a thematic map (grouping): linkAttr indicates
					the column used to join with object_id.
			*/
			void drawCurrentTheme(TeGrouping& group, std::vector<std::string>& rampColors,
								const std::string& fields, const std::string& from,
								const std::string& linkAttr, const std::string& restrictionExpression);

			/** \brief Draws the current theme using an external SQL and makes a thematic map (grouping): linkAttr indicates
					the column used to join with object_id.
			*/
			void drawCurrentTheme(TeLegendEntryVector& legendVec, TeGrouping& group,
								std::vector<std::string>& rampColors,
								const std::string& fields, const std::string& from,
								const std::string& linkAttr, const std::string& restrictionExpression);

			/** \brief Draws the selected objects indicated by objIds.
			*/
			void drawSelectedObjects(TeKeys& objIds, const int& themeType = 0);

			/** \brief Generates a buffer zone around the object with objectId.
			*/
			void drawBufferZone(const std::string& objectId, const double& distance, const int& themeType = 0);

			//@}

			/** @name Spatial Query Methods
			* Methods related to localization of objects and spatial queries.
			*/
			//@{

			/** \brief Returns the object identifier (objectId) and the geometry
					identifier (geomId) of an object that contains the point pt,
					with pt in view projection.
			*/
			bool locateObject(std::pair<std::string, std::string>& identifier, TeCoord2D& pt, const double& tol = 0.0, const int& themeType = 0);

			/** \brief Given a box and a relation find objects related to the box in the
					current or reference theme, the objectid of found objects will be in set objIds.
			*/
			bool locateObjects(const TeBox& box, const int& relation, TeKeys& objIds, const int& themeType = 0);

			/** \brief Given an objectid and a relation find objects related in the current or reference theme.
			*/
			bool locateObjects(const string& objectId, const int& relation, TeKeys& objIds, const int& themeType);

			/** \brief Given objectids and a relation find objects related in the current or reference theme.
			*/
			bool locateObjects(TeKeys& objIdsIn, const int& relation, TeKeys& objIdsOut, const int& themeType);

			/** \brief Given an objectid in the current theme find objects related in the reference theme.
			*/
			bool locateObjects(const string& objectId, const int& relation, TeKeys& objIds);

			/** \brief Given objectids and a relation find objects from current theme related to objects in reference theme.
			*/
			bool locateObjects(TeKeys& objIdsIn, const int& relation, TeKeys& objIdsOut);

			//@}

			/** @name Attribute Retrieval
			* These methods retrive objects attributes.
			*/
			//@{
			
			/** \brief Returns the object attributes.
			*/
			void fetchAttributes(const string& objectId, std::map<std::string, std::string>& objVal, const int& themeType = 0);

			//@}

			/** @name Legend Methods
			* These methods adjusts how TerraManager will draw legends.
			*/
			//@{

			/** \brief Draws a legend picture.
			*/
			void drawLegend(std::vector<TeLegendEntryVector>& legends, std::vector<std::string>& legendTitles, const int& width = 150);

			/** \brief Returns a PNG image with legend content.
				\param imageType The type of image: 0 for PNG, 1 for JPEG and 2 for GIF. (Input)
				\param size The output image size in bytes. (Output)
				\param opaque If true the image will be opaque, otherwise it will have a transparent background. (Input)
				\param quality Applies only for JPEG image. (Input)
				\note The caller must clean memory using freeImage().
			*/
			void* getLegendImage(const int& imageType, int& size, const bool& opaque = true, const int& quality = 75);

			/** \brief Saves the legend content to a image file (PNG, JPEG or GIF).
				\param imageType The type of image file: 0 for PNG, 1 for JPEG and 2 for GIF. (Input)
				\param fileName  File name with full path. (Input)
				\param opaque If true the image will be opaque, otherwise it will have a transparent background. (Input)
				\param quality Applies only for JPEG image. (Input)
			*/
			void saveLegendImage(const int& imageType, const std::string& fileName, const bool& opaque = true, const int& quality = 75);

			//@}		

			/** @name Scale Control
			* These methods controls the use of scale by TerraManager
			*/
			//@{

			/** \brief Turns on/off the automatic scale control when drawing themes.
				\note By default the automatic scale control is on.
				\note We control the roundoff: using epsilon = 1e-6.
			*/
			void setAutomaticScaleControlEnable(const bool& on = true);

			/** \brief Gets scale dedominator associted to current theme when we have canvas world already set.
				\note In this implementation we adopted the WMS International Standard,
					where the common pixel size is defined to be 0,28 mm � 0,28 mm.		    
			*/
			double getScale() const;

			//@}
			
			/** @name Methods that controls conflicts
			* These methods adjusts how TerraManager will draw texts from an attribute table.
			*/
			//@{

			/** \brief This method can be used to avoid conflict during drawing of texts.
				\param detect If TRUE, turns on conflict labeling detection and if FALSE turns off. (Input)
			*/
			void setConflictDetect(const bool& detect);

			/** \brief Sets the number of pixel that will be used to expand label box inr order to test conflict
					against another label.
			*/
			void setMinCollisionTolerance(const int& numPixels);

			/** Clears conflict cache
			*/
			void clearConflictCache();

			/** \brief Sets the field (attribute table name + field name or a formula) from the current theme wich value will be used to draw on canvas.
			*/
			void setPriorityField(const std::string& fieldName);
			
			/** \brief Optional parameter that defines the level of resolution to be
					used on the spatial filter query. This may be useful to avoid rendering too many
					elements at the same position of the screen.
				param n The number of pixels used to filter (the unit is screen pixels).
			*/
			void setGeneralizedPixels(const int& n);

			//@}

			/** @name Labeling Methods
			* These methods adjusts how TerraManager will draw texts from an attribute table.
			*/
			//@{

			/** \brief Sets the field (attribute table name + field name or a formula) from the current theme wich value will be used to draw on canvas.
			*/
			void setLabelField(const std::string& fieldName);

			//@}

			/** @name Image Map methods
			* These methods can be used to generate HTML image maps.
			*/
			//@{

			void setImageMapProperties(const std::string& mapName, const std::string& mapId);

			void closeImageMap(const bool& hasToClose);

			void setAreaShape(const TeImageMapCanvas::TeImageMapShapeType& areaShapeType);

			void setAreaProperty(const std::string propertyName,
								const std::string& propertyValue,
								const std::string valueSrc = "");

			void setAreaCircleRadius(const int& radius);

			void setAreaRectWidth(const int& width);

			char* getImageMap(const std::string& from,
							const std::string& linkAttr,
							const std::string& restrictionExpression);
							  
			int getImageMapSize() const;

			//@}

			/** @name XXXXX
			* These methods XXXXXXX.
			*/
			//@{

			void drawCartogram(const std::vector<std::string>& fields,
							const std::string& fieldsFunc,
							const TeCartogramParameters& cartogramParameters,						   
							const std::string& proportionalToField = "",
							const std::string& from = "",
							const std::string& linkAttr = "",
							const std::string& restrictionExpression = "");



			//@}

			/** @name XXXXX
			* These methods XXXXXXX.
			*/
			//@{

			void saveThemeToFile(const std::string& fileName);

			//@}

			/** @name XXXXX
			* These methods XXXXXXX.
			*/
			//@{

			/** \brief Sets the visual used to draw theme objects.
			*/
			void setThemeVisual(const TeVisual& vis, const TeGeomRep& representation);

			//@}

			/** @name XXXXX
			* These methods XXXXXXX.
			*/
			//@{

			/** \brief Draws a text for lines.
			*/
			void drawText();

			//@}

		protected:

			/** @name Auxiliary Methods for Grouping
			* These methods can be used to create a thematic map on the fly, without need persistence.
			*/
			//@{

			/** \brief Returns a grouping legend vector and a map of objectid to legendid.
			*/
			void buildGrouping(TeGrouping& group, std::vector<std::string>& rampColors,
							const std::string& restrictionExpression, TeLegendEntryVector& legendVec,
							std::map<std::string, unsigned int>& visualMap, const std::string& fields = "",
							const std::string& fromClause = "", const std::string& linkAttr = "");

			//@}

			/** @name Auxiliary drawing Methods
			* These methods do the real job of drawing current theme representations.
			*/
			//@{

			/** \brief Draws the raster representation of the current theme.
			*/
			void drawRasterRepresentation( const TeBox& box );

			/** \brief Draws the polygon objects int the current theme that intersects the box.
			*/
			void drawPolygonRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
										const std::string& restrictionExpression, TeBox& box,
										const std::string& fromClause = "", const std::string& linkAttr = "");

			/** \brief Draws the cell objects int the current theme that intersects the box.
			*/
			void drawCellRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
										const std::string& restrictionExpression, TeBox& box,
										const std::string& fromClause = "", const std::string& linkAttr = "");

			/** \brief Draws the lines objects int the current theme that intersects the box.
			*/
			void drawLineRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
										const std::string& restrictionExpression, TeBox& box,
										const std::string& fromClause = "", const std::string& linkAttr = "");

			/** \brief Draws the points objects int the current theme that intersects the box.
			*/
			void drawPointRepresentation(TeLegendEntryVector& legendVec, std::map<std::string, unsigned int>& visualMap,
										const std::string& restrictionExpression, TeBox& box,
										const std::string& fromClause = "", const std::string& linkAttr = "");

			/** \brief Draws selected polygon objects using current or reference theme.
			*/
			void drawSelectedPolygons(std::vector<std::string>& inClauseVector, const int& themeType = 0);

			/** \brief Draws selected cell objects using current theme.
			*/
			void drawSelectedCells(std::vector<string>& inClauseVector, const int& themeType = 0);

			/** \brief Draws selected line objects using current or reference theme.
			*/
			void drawSelectedLines(std::vector<std::string>& inClauseVector, const int& themeType = 0);

			/** \brief Draws selected point objects using current or reference theme.
			*/
			void drawSelectedPoints(std::vector<std::string>& inClauseVector, const int& themeType = 0);

			/** \brief Draws the polygon objects int the current theme that intersects the box.
			*/
			void drawTextLabeling(const std::string& restrictionExpression, TeBox& box,
								const std::string& fromClause = "", const std::string& linkAttr = "");

			//@}

			/** @name Auxiliary SQL methods
			* These methods mounts the correct SQL used by drawing methods.
			*/
			//@{

			/** \brief Returns a pointer to the database portal of geometries or null on error.
			*/
			TeDatabasePortal* getGeometryPortal(const TeGeomRep& geomRep, TeBox& box,
												const std::string& restrictionExpression, const std::string& fromClause = "",
												const string& linkAttr = "");

			/** \brief Returns a pointer to the database portal of geometries or null on error.
			*/
			TeDatabasePortal* getSelectedGeometryPortal(const TeGeomRep& geomRep, const string& inClause, const int& themeType = 0);

			/** \brief Returns a pointer to the database portal for candidated texts that can be draw on canvas.
			*/
			TeDatabasePortal* getTextLabelingPortal(const std::string& restrictionExpression, TeBox& box,
													const std::string& fromClause = "", const std::string& linkAttr = "");

			//@}		

			/** @name Auxiliary methods
			* Utilitary methods.
			*/
			//@{

			/** \brief Returns the visual associated to an object.
			*/
			TeVisual& getVisual(TeDatabasePortal* portal, const TeGeomRep& geomRep,
								const int& resPos, const int& groPos, const int& ownPos);

			/** \brief Verifies if there is a current database otherwise raises an exception.
			*/
			void verifyDatabaseConnection() const;

			/** \brief Verifies if there is a current view otherwise raises an exception.
			*/
			void verifyCurrentView() const;

			/** \brief Try to create a portal and on error it raises an exception.
			*/
			TeDatabasePortal* getPortal() const;

			//@}

			/** \brief No copy constructor allowed.
				\param rhs Other terramanager instance to copy from. (Input)
			*/
			TerraManager(const TerraManager& rhs);

			/** \brief No assignement allowed.
				\param rhs Other terramanager instance to copy from. (Input)
			*/
			TerraManager& operator=(const TerraManager& rhs);


			////////////////////////
			void getPolygonImageMap(const std::string& fields,
									const std::string& from,
									const std::string& linkAttr,
									const std::string& restrictionExpression,
									TeBox& box,
									const double& extmin);

			void getCellImageMap(const std::string& fields,
								const std::string& from,
								const std::string& linkAttr,
								const std::string& restrictionExpression,
								TeBox& box,
								const double& extmin);

			void getLineImageMap(const std::string& fields,
								const std::string& from,
								const std::string& linkAttr,
								const std::string& restrictionExpression,
								TeBox& box,
								const double& extmin);

			void getPointImageMap(const std::string& fields,
								const std::string& from,
								const std::string& linkAttr,
								const std::string& restrictionExpression,
								TeBox& box,
								const double& extmin);

			TeDatabasePortal* getImageMapGeometryPortal(const TeGeomRep& geomRep,
														TeBox& box,
														const double& extmin,
														const std::string& fields,
														const std::string& restrictionExpression,
														const std::string& fromClause = "",
														const string& linkAttr = "");

			void getChartBoundValues(double& chartMaxValue,
									double& chartMinValue,
									const std::vector<std::string>& fields,
									const std::string& fieldsFunc,
									const std::string& proportionalToField,
									const int& chartType,
									const std::string& from,
									const std::string& attrLink,
									const std::string& restrictionExpression);

			void clearVisualMap();
			void clearThemeVisualMap();

		protected:

			TeDatabase* db_;							//!< A pointer to the current database connection.
			TeView* currentView_;						//!< Stores a pointer to the current view.
			TeTheme* currentTheme_;						//!< A pointer to the current theme that will be used for query and drawing its objects.
			int currentThemeActiveRepresentations_;		//!< The current theme active representation (may be 0).
			TeTheme* referenceTheme_;					//!< A pointer to the theme used as reference for restriction on spatial queries.
			int referenceThemeActiveRepresentations_;	//!< The current theme active representation (may be 0).
			TeGDCanvas canvas_;							//!< An internal canvas where the output will be draw.
			TeColor canvasBackgroundColor_;				//!< Canvas background color.
			TeGeomRepVisualMap visualMap_;				//!< Default visual to plot objects.
			bool conflicDetect_;						//!< If true conflict labeling detect will be turned on.
			std::string curentThemeLabelingField_;		//!< Current theme labeling field.
			std::string curentThemePriorityField_;		//!< Current theme field that can be used to stabilish a pripority during drawing.
			int minCollisionTol_;								//!< Number of pixels that the text bounding box will be adjusted in order to test conflict against another label.
			TeGDCanvas legendCanvas_;							//!< An internal canvas where the output will be draw.
			mutable std::string errorMessage_;					//!< An error message associated to last error ocurried.
			/*bool textOutlineEnable_;*/							//!< A flag that indicates if text outline is "on" or "off".
			TeColor textOutlineColor_;							//!< Color used to draw an outline text around labels.
			bool automaticScaleControlEnable_;					//!< A flag that indicates if we can make use of automatic scale control during drawing a theme.
			TeImageMapCanvas* imcanvas_;
			bool closeImgMapCanvasAtEnd_;
			bool keepThemeTransparency_;
			int generalizedPixels_;
			TeGeomRepVisualMap themeVisualMap_;
			bool closeDatabaseAtEnd_;
	};

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_TERRAMANGER_H


