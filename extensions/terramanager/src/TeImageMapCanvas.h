/** \file TeImageMapCanvas.h
    \brief This file contais an implementation of a canvas that builds XHTML image map.
	\author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  */

#ifndef __TERRAMANAGER_INTERNAL_IMAGEMAPCANVAS_H
#define __TERRAMANAGER_INTERNAL_IMAGEMAPCANVAS_H

// TerraManager include files
#include "TerraManagerConfig.h"

// TerraLib include files
#include <TeCoord2D.h>
#include <TeBox.h>
#include <TeGeometry.h>
#include <TeProjection.h>

// STL include files
#include <string>

namespace TeMANAGER
{

	/** \class TeImageMapCanvas
	  * \brief A Canvas that builds XHTML image map.
	  */ 
	class TERRAMANAGEREXPORT TeImageMapCanvas
	{
		public:

			enum TeImageMapShapeType { TeIMRECT, TeIMCIRCLE, TeIMPOLYGON };

			TeImageMapCanvas(const std::string& mapName = "", const std::string& mapId = "");

			~TeImageMapCanvas();

			void setWorld(const TeBox& b, const int& w = 1, const int& h = 1);

			const TeBox& getWorld() const;

			TeProjection* getCanvasProjection() const;

			void setCanvasProjection(TeProjection& proj);

			void setDataProjection(TeProjection& proj);

			const std::map<std::string, pair<std::string, std::string> >& getAreaProperty() const;

			void setAreaProperty(const std::string propertyName,
								const std::string& propertyValue,
								const std::string valueSrc = "");

			void setAreaShape(const TeImageMapShapeType& areaShapeType);

			void setAreaCircleRadius(const int& radius);

			void setAreaRectWidth(const int& width);

			void draw(TeCoord2D& c);

			void draw(TePoint &p);

			void draw(const TeBox& b);

			void draw(TeCell& c);

			void draw(TeLine2D& l);

			void draw(TePolygon& p);

			void drawPoint(const int& x, const int& y);

			void drawSegment(const int& xi, const int& yi, const int& xf, const int& yf);
			
			void drawBox(const int& ulx, const int& uly, const int& lrx, const int& lry);

			void close();

			char* getImageMap() const;
			
			int getImageMapSize() const;

			void world2Viewport(const double& wx, const double& wy, int& vx, int& vy);

			void dataWorld2World(TeCoord2D& dw, TeCoord2D& w);

			void world2DataWorld(TeCoord2D& w, TeCoord2D& dw);

		protected:		

			void addAreaAttributes();

			void checkBuffer(const int& nbytesToBeWritten);

		protected:

			char* imageMapStr_;
			int imageMapStrSize_;
			int imageMapStrAlloc_;

			std::string imageMapName_;
			std::string imageMapId_;

			TeBox wc_;

			double xmin_;
			double xmax_;
			double ymin_;
			double ymax_;

			int width_;
			int height_;

			double fX_;
			double fY_;

			TeProjection* canvasProjection_;
			TeProjection* dataProjection_;

			bool has2Remap_;

			std::map<std::string, pair<std::string, std::string> > areaProperties_;

			TeImageMapShapeType areaShapeType_;
			int areaCircleRadius_;
			int areaRectWidth_;
	}; 

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_IMAGEMAPCANVAS_H
 
