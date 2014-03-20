/** \file TeGDCanvas.h
    \brief This file contais an implementation of a canvas built on top of GD graphics library.
	\author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
	\author Ricardo Cartaxo <cartaxo@dpi.inpe.br>
  */

#ifndef __TERRAMANAGER_INTERNAL_GDCANVAS_H
#define __TERRAMANAGER_INTERNAL_GDCANVAS_H

// TerraManager include files
#include "TerraManagerConfig.h"

// TerraLib include files
#include <TeCoord2D.h>
#include <TeBox.h>
#include <TeGeometry.h>
#include <TeProjection.h>
#include <TeRTree.h>

// GD library include files
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>
#include <gdfontl.h>
#include <gdfontg.h>

// STL include files
#include <string>

// Forward declarations
class TeRaster;
class TeRasterTransform;

namespace TeMANAGER
{

	/** \class TeGDCanvas
	  * \brief A Canvas built on top of GD library.
	  *
	  *
	  * \todo Corrigir a implementacao de angulos em texto.
	  *
	  * \sa TeDecoderGDImage, TeGDLegend
  	  *
	  */
	class TERRAMANAGEREXPORT TeGDCanvas 
	{
		public:

			TeGDCanvas();

			~TeGDCanvas();

			void clear();

			void clearConflictCache();

			TeSAM::TeRTree<int, 6, 2>& getRtree();

			const bool& isValid() const;

			void setWorld(const TeBox& b, const int& w = 0, const int& h = 0);

			void setTransformation();

			const TeBox& getWorld() const;

			const int& getWidth() const;

			const int& getHeight() const;

			TeBox getDataWorld();

			/* Returns pixel size in world coordinates. */
			double getPixelSizeX() const;

			double getPixelSizeY() const;

			void setBackgroudColor(const int& r, const int& g, const int& b);

			void setTextOutlineEnable(const bool& enable);

			/* Gets canvas content as an image in the desired format (0 = PNG, 1 = JPEG, 2 = GIF). */
			void* getImage(const int& imageType, int& size, const bool& opaque = true, const int& quality = 75);

			void saveImage(const int& imageType, const string& filename, const bool& opaque = true, const int& quality = 75);

			static void freeImage(void* ptr);

			TeProjection* getCanvasProjection() const;

			/* Sets canvas projection. */
			void setCanvasProjection(TeProjection& proj);

			TeProjection* getDataProjection() const;
			
			/* Sets the projection of next incoming data. */
			void setDataProjection(TeProjection& proj);

			void world2Viewport(const double& wx, const double& wy, int& vx, int& vy);

			void dataWorld2World(TeCoord2D& dw, TeCoord2D& w);

			void world2DataWorld(TeCoord2D& w, TeCoord2D& dw);

			void dataWorld2Viewport(TeCoord2D& dw, int& vx, int& vy);

			void setPointColor(const int& r, const int& g, const int& b);

			void setPointStyle(const int& s, const int& w = 3);

			void setLineColor(const int& r, const int& g, const int& b);

			void setLineStyle(const int& s, const int& w = 1);

			void setPolygonColor(const int& r, const int& g, const int& b, const int& transparency = 0);

			void setPolygonStyle(const int& s, const int& width = 1);

			void setPolygonLineColor(const int& r, const int& g, const int& b);

			void setPolygonLineStyle(const int& s, const int& width = 1);

			void setTextColor(const int& r, const int& g, const int& b);

			void setTextSize(const int& size);

			void setTextStyle(const std::string& family);

			void setTextOutlineColor(const int& r, const int& g, const int& b);

			void setTextShadowColor(const int& r, const int& g, const int& b);

			void setTextShadowSize(const int& size);

			void draw(TeCoord2D& c);

			void draw(TePoint &p);

			void draw(const TeBox& b);

			void draw(const TeCell& c);

			void draw(const TeLine2D& l);

			void draw(const TePolygon& p);

			void draw(TeCoord2D& c, const std::string& strText,
					const double& angle = 0.0,
					const double& alignh = 0.0, const double& alignv = 0.0);

			void draw(TeCoord2D& c, const std::string& strText,
					const int& minCollisionTol,
					const double& angle = 0.0,
					const double& alignh = 0.0, const double& alignv = 0.0);

			void draw(TeRaster* raster, TeRasterTransform* transf, bool initRaster = true);
	 

			void drawPoint(const int& x, const int& y);

			void drawSegment(const int& xi, const int& yi, const int& xf, const int& yf);

			void drawBox(const int& ulx, const int& uly, const int& lrx, const int& lry);

			void drawArc(const int& cx, const int& cy, const int& w, const int& h, const int& startAngle, const int& endAngle);

			void getTextMBR(int x, int y, const std::string& strText, const double& angle,
							const double& alignh, const double& alignv, TeBox& bout) const;

			/** \brief Returns the text bounding box in viewport (device) coordinate system.
				\param x Viewport x coordinate (column) where the text will be draw. (Input)
				\param x Viewport y coordinate (line) where the text will be draw. (Input)
				\param angle The angle to rotate the text. (Input)
				\param bout Text bounding box in viewport coordinate system. (Output)
			*/
			void getTextMBR(const int& x, const int& y, const std::string& strText, const double& angle, TeBox& bout) const;

		protected:

			void drawTransparent(const TePolygon& p);

			void drawGDText(const int& x, const int& y, unsigned char* strText, gdFontPtr font);

			void drawTTText(const int& x, const int& y, char* strText, const double& angle = 0.0) const;

			void getGDTextMBR(const int& x, const int& y, const int& textLen, gdFontPtr font, const int& minCollisionTol,  TeBox& bout) const;

			void getTTTextMBR(const int& x, const int& y, char* strText,  const double& angle, const int& minCollisionTol,  TeBox& bout) const;

			void getTTTextMBR(const int& x, const int& y, char* strText, const double& angle, const double& alignh, const double& alignv, const int& minCollisionTol, TeBox& bout) const;

			void dataWorld2Viewport(const TeLine2D& l, gdPointPtr pts, const unsigned int& npts);

			/** \struct TeGDStyle
			  * \brief A Struct representing a GD style.
			  *
			  *
			  * \sa TeGDCanvas.
			  *
			  */ 
			struct TeGDStyle
			{
				int color_;
				int style_;
				int width_;
			}; 
		
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

			int backgroundColor_;

			TeGDStyle polygonLineStyle_;
			TeGDStyle polygonFillStyle_;
			int polygonTransparency_;

			TeGDStyle lineStyle_;
			TeGDStyle pointStyle_;
			TeGDStyle textStyle_;
			TeGDStyle textShadowStyle_;
			TeGDStyle textOutlineStyle_;
			std::string textTTFont_;

			gdImagePtr gdCanvas_;			//!< A pointer to a GD image where vectorial representations are draw.
			gdImagePtr gdAuxiliaryCanvas_;	//!< A pointer to an auxiliary GD image used during drawing of polygons with holes.

			bool has2Remap_;

			bool isValid_;

			TeSAM::TeRTree<int, 6, 2> rtreeConflictBox_;

			bool textOutlineEnable_;
	}; 

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_GDCANVAS_H
 
