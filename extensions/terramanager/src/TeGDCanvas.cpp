// TerraManager include files
#include "TeGDCanvas.h"
#include "TeDecoderGDImage.h"

// TerraLib inlcude files
#include <TeRaster.h>
#include <TeDecoderMemory.h>
#include <TeRasterRemap.h>
#include <TeVectorRemap.h>

namespace TeMANAGER
{

inline void TeGDImageClear(gdImagePtr im, const int& color)
{ 
	for(int i = 0; i < im->sy; ++i) 
        for(int j = 0; j < im->sx; ++j)
            im->tpixels[i][j] = color; 
}

inline void TeGDImageBlend(gdImagePtr imi, gdImagePtr imo, int transp, int polcolor, int linecolor)
{ 
	for(int i = 0; i < imi->sy; ++i)
		for(int j = 0; j < imi->sx; ++j)
			if(imi->tpixels[i][j] != transp)
			{
				if(imi->tpixels[i][j] != linecolor)
					gdImageSetPixel(imo, j, i, polcolor);
				else
					//imo->tpixels[i][j] = imi->tpixels[i][j];				
					imo->tpixels[i][j] = linecolor;				
			}
}

inline void TeGDImageShallowCopy(gdImagePtr imi, gdImagePtr imo)
{
	imo->transparent = imi->transparent; 
	imo->interlace = imi->interlace; 
	imo->trueColor = imi->trueColor;
	imo->alphaBlendingFlag = imi->alphaBlendingFlag;
	imo->saveAlphaFlag = imi->saveAlphaFlag;
 
	if(imi->style) 
		gdImageSetStyle(imo, imi->style, imi->styleLength);

	if(imi->brush) 
		gdImageSetBrush(imo, imi->brush);

	if(imi->tile)
		gdImageSetTile(imo, imi->tile); 
}

}

TeMANAGER::TeGDCanvas::TeGDCanvas( ) 
	: xmin_(0.0), xmax_(0.0), ymin_(0.0), ymax_(0.0),
	  width_(0), height_(0), fX_(1.0), fY_(1.0),
	  canvasProjection_(0), dataProjection_(0),
	  backgroundColor_(0),
	  polygonTransparency_(0),
	  gdCanvas_(0), gdAuxiliaryCanvas_(0),
	  has2Remap_(false),
	  isValid_(false),
	  rtreeConflictBox_(TeBox()),
	  textOutlineEnable_(true)
{
// polygon visual
	polygonLineStyle_.color_ = gdTrueColor(0, 255, 255);
	polygonLineStyle_.style_ = TeLnTypeContinuous;
	polygonLineStyle_.width_ = 1;

	polygonFillStyle_.style_ = TePolyTypeFill;
	polygonFillStyle_.color_ = gdTrueColor(255, 100, 100);
	polygonFillStyle_.width_ = 1;

// line visual
	lineStyle_.color_ = gdTrueColor(0, 255, 255);
	lineStyle_.style_ = TeLnTypeContinuous;
	lineStyle_.width_ = 1;

// point visual
	pointStyle_.color_ = gdTrueColor(0, 255, 255);
	pointStyle_.style_ = TePtTypePlus;
	pointStyle_.width_ = 3;

// text visual
	textStyle_.color_ = gdTrueColor(0, 0, 0);
	textStyle_.width_ = 4;

    textShadowStyle_.color_ = backgroundColor_;
	textOutlineStyle_.color_ = backgroundColor_;
}

TeMANAGER::TeGDCanvas::~TeGDCanvas()
{
	delete canvasProjection_;
	delete dataProjection_;
	
	clear();
}

void TeMANAGER::TeGDCanvas::clear()
{
	isValid_ = false;

	if(gdCanvas_) 
	{
		gdImageDestroy(gdCanvas_); 
		gdCanvas_ = 0;
	}

	if(gdAuxiliaryCanvas_)
	{
		gdImageDestroy(gdAuxiliaryCanvas_);
		gdAuxiliaryCanvas_ = 0;
	}

	rtreeConflictBox_.clear();
}

void TeMANAGER::TeGDCanvas::clearConflictCache()
{
	rtreeConflictBox_.clear();
}

TeSAM::TeRTree<int, 6, 2>& TeMANAGER::TeGDCanvas::getRtree()
{
	return rtreeConflictBox_;
}

const bool& TeMANAGER::TeGDCanvas::isValid() const
{
	return isValid_;
}

void TeMANAGER::TeGDCanvas::setWorld(const TeBox& b, const int& w, const int& h)
{ 
	xmin_= b.x1();
	xmax_= b.x2(); 
	ymin_= b.y1(); 
	ymax_= b.y2();
	width_ = w; 
	height_ = h; 

	setTransformation();
	
	if(gdCanvas_)
	{
		gdImageDestroy(gdCanvas_);
		gdCanvas_ = 0;
	}

	if(gdAuxiliaryCanvas_)
	{
		gdImageDestroy(gdAuxiliaryCanvas_);
		gdAuxiliaryCanvas_ = 0;
	}

	gdCanvas_ = gdImageCreateTrueColor(width_, height_);

	//gdImageSaveAlpha(gdCanvas_, 1);
	//gdImageAlphaBlending(gdCanvas_, 0);
	//gdImageSetAntiAliasedDontBlend(gdCanvas_, backgroundColor_, 0);

	if(backgroundColor_ != 0)
        TeGDImageClear(gdCanvas_, backgroundColor_);

	isValid_ = true;
}

void TeMANAGER::TeGDCanvas::setTransformation()
{
	double dxw = xmax_ - xmin_;
	double dyw = ymax_ - ymin_;
	fX_ = width_ / dxw;
	fY_ = height_ / dyw;
 
	wc_ = TeBox(xmin_, ymin_, xmax_, ymax_);
}

const TeBox& TeMANAGER::TeGDCanvas::getWorld() const
{
	return wc_ ;
}

const int& TeMANAGER::TeGDCanvas::getWidth() const
{
	return width_;
}

const int& TeMANAGER::TeGDCanvas::getHeight() const
{
	return height_;
}

TeBox TeMANAGER::TeGDCanvas::getDataWorld()
{ 
	if(has2Remap_)
	{ 
		TeBox b = TeRemapBox(wc_, canvasProjection_, dataProjection_);
		return b;
	} 

	return wc_ ;
}

double TeMANAGER::TeGDCanvas::getPixelSizeX() const
{
	return (xmax_ - xmin_) / static_cast<double>(width_);
}

double TeMANAGER::TeGDCanvas::getPixelSizeY() const
{
	return (ymax_ - ymin_) / static_cast<double>(height_);
}

void TeMANAGER::TeGDCanvas::setBackgroudColor(const int& r, const int& g, const int& b)
{		
	backgroundColor_ = gdTrueColor(r, g, b);
	textShadowStyle_.color_ = backgroundColor_;
	textOutlineStyle_.color_ = backgroundColor_;
}

void TeMANAGER::TeGDCanvas::setTextOutlineEnable(const bool& enable)
{
	textOutlineEnable_ = enable;
}

void* TeMANAGER::TeGDCanvas::getImage(const int& imageType, int& size, const bool& opaque, const int& quality)
{
	size = 0;

	if(gdCanvas_)
	{
		if(!opaque)
			gdImageColorTransparent(gdCanvas_, backgroundColor_);

		void* x = 0;

		if(imageType == 0)
		{
            x = gdImagePngPtr(gdCanvas_, &size);
		}
		else if(imageType == 1)
		{
			x = gdImageJpegPtr(gdCanvas_, &size, quality);
		}
		else if(imageType == 2)
		{
			x = gdImageGifPtr(gdCanvas_, &size);
		}

		gdImageColorTransparent(gdCanvas_, -1);

		return x;
	}

	return 0;
}

void TeMANAGER::TeGDCanvas::saveImage(const int& imageType, const string& filename, const bool& opaque, const int& quality)
{
	if(!gdCanvas_)
		return;

	if(!opaque)
		gdImageColorTransparent(gdCanvas_, backgroundColor_);

	FILE *out; 
	out = fopen(filename.c_str(), "wb");

	if(imageType == 0)
	{
        gdImagePng(gdCanvas_, out);
	}
	else if(imageType == 1)
	{
		gdImageJpeg(gdCanvas_, out, quality);
	}
	else if(imageType == 2)
	{
		gdImageGif(gdCanvas_, out);
	}

	fclose(out);

	gdImageColorTransparent(gdCanvas_, -1);
}

void TeMANAGER::TeGDCanvas::freeImage(void* ptr)
{
	gdFree(ptr);
}

TeProjection* TeMANAGER::TeGDCanvas::getCanvasProjection() const
{
	return canvasProjection_;
}

void TeMANAGER::TeGDCanvas::setCanvasProjection(TeProjection& proj)
{
	delete canvasProjection_;
    canvasProjection_ = 0;
	has2Remap_ = false;

	canvasProjection_ = TeProjectionFactory::make(proj.params());

	if(dataProjection_)
	{
		dataProjection_->setDestinationProjection(canvasProjection_);
		canvasProjection_->setDestinationProjection(dataProjection_);

		if((*canvasProjection_) == (*dataProjection_))
			has2Remap_ = false;
		else
			has2Remap_ = true;
	}
}

TeProjection* TeMANAGER::TeGDCanvas::getDataProjection() const
{
	return dataProjection_;
}
		
void TeMANAGER::TeGDCanvas::setDataProjection(TeProjection& proj)
{
	delete dataProjection_;
	dataProjection_ = 0;
	has2Remap_ = false;

	dataProjection_ = TeProjectionFactory::make(proj.params());

	if(canvasProjection_)
	{
		dataProjection_->setDestinationProjection(canvasProjection_);
		canvasProjection_->setDestinationProjection(dataProjection_);

		if((*canvasProjection_) == (*dataProjection_))
			has2Remap_ = false;
		else
			has2Remap_ = true;
	}
}

void TeMANAGER::TeGDCanvas::world2Viewport(const double& wx, const double& wy, int& vx, int& vy)
{
	vx = static_cast<int>((wx - xmin_) * fX_);
	vy = static_cast<int>(height_ - ((wy - ymin_) * fY_));
}

void TeMANAGER::TeGDCanvas::dataWorld2World(TeCoord2D& dw, TeCoord2D& w)
{
	w = dataProjection_->PC2LL(dw);
	w = canvasProjection_->LL2PC(w);
}

void TeMANAGER::TeGDCanvas::world2DataWorld(TeCoord2D& w, TeCoord2D& dw)
{
	dw = canvasProjection_->PC2LL(w);
	dw = dataProjection_->LL2PC(dw);
}

void TeMANAGER::TeGDCanvas::dataWorld2Viewport(TeCoord2D& dw, int& vx, int& vy)
{
	if(has2Remap_)
	{
		TeCoord2D w;

		w = dataProjection_->PC2LL(dw);
        w = canvasProjection_->LL2PC(w);

        vx = static_cast<int>((w.x_ - xmin_) * fX_);
        vy = static_cast<int>(height_ - ((w.y_ - ymin_) * fY_));
	}
	else
	{
        vx = static_cast<int>((dw.x_ - xmin_) * fX_);
		vy = static_cast<int>(height_ - ((dw.y_ - ymin_) * fY_));
	}
}

void TeMANAGER::TeGDCanvas::setPointColor(const int& r, const int& g, const int& b)
{
	pointStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setPointStyle(const int& s, const int& w)
{
	pointStyle_.style_ = s;
	pointStyle_.width_ = w;
}

void TeMANAGER::TeGDCanvas::setLineColor(const int& r, const int& g, const int& b)
{
	lineStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setLineStyle(const int& s, const int& w)
{
	lineStyle_.style_ = s;
	lineStyle_.width_ = w;
}

void TeMANAGER::TeGDCanvas::setPolygonColor(const int& r, const int& g, const int& b, const int& transparency)
{
	polygonTransparency_ = transparency;

	if(polygonTransparency_ == 0)
	{
		polygonFillStyle_.color_ = gdTrueColor(r, g, b);
	}
	else
	{
		int a = (127 * transparency) / 100;
        polygonFillStyle_.color_ = gdTrueColorAlpha(r, g, b, a);				
	}
}

void TeMANAGER::TeGDCanvas::setPolygonStyle(const int& s, const int& width)
{
	polygonFillStyle_.style_ = s;
	polygonFillStyle_.width_ = width;
}

void TeMANAGER::TeGDCanvas::setPolygonLineColor(const int& r, const int& g, const int& b)
{
	polygonLineStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setPolygonLineStyle(const int& s, const int& width)
{
	polygonLineStyle_.style_ = s;
	polygonLineStyle_.width_ = width;
}

void TeMANAGER::TeGDCanvas::setTextColor(const int& r, const int& g, const int& b)
{
	textStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setTextSize(const int& size)
{
	textStyle_.width_ = size;
}

void TeMANAGER::TeGDCanvas::setTextStyle(const std::string& family)
{
	textTTFont_ = family;
}

void TeMANAGER::TeGDCanvas::setTextOutlineColor(const int& r, const int& g, const int& b)
{
	textOutlineStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setTextShadowColor(const int& r, const int& g, const int& b)
{
	textShadowStyle_.color_ = gdTrueColor(r, g, b);
}

void TeMANAGER::TeGDCanvas::setTextShadowSize(const int& size)
{
	textShadowStyle_.width_ = size;
}

void TeMANAGER::TeGDCanvas::draw(TeCoord2D& c)
{
	int x, y;

	if(has2Remap_)
	{
		TeCoord2D cw;
		dataWorld2World(c, cw);
		world2Viewport(cw.x_, cw.y_, x, y);
	}
	else
	{
		world2Viewport(c.x_, c.y_, x, y);
	}

	drawPoint(x, y);
}

void TeMANAGER::TeGDCanvas::draw(TePoint &p)
{
	draw(p.location());
}

void TeMANAGER::TeGDCanvas::draw(const TeBox& b)
{
	int llx, lly, urx, ury;

	if(has2Remap_)
	{
		TeCoord2D c(b.x1_, b.y1_);
		TeCoord2D cw;
		dataWorld2World(c, cw);
		world2Viewport(cw.x_, cw.y_, llx, lly);

		c.x_ = b.x2_;
		c.y_ = b.y2_;
		dataWorld2World(c, cw);
		world2Viewport(cw.x_, cw.y_, urx, ury);
	}
	else
	{
		world2Viewport(b.x1_, b.y1_, llx, lly);
		world2Viewport(b.x2_, b.y2_, urx, ury);
	}

	drawBox(llx, ury, urx, lly);
}

void TeMANAGER::TeGDCanvas::draw(const TeCell& c)
{
	draw(c.box());
}

void TeMANAGER::TeGDCanvas::draw(const TeLine2D& l)
{ 
	gdImageSetThickness(gdCanvas_, lineStyle_.width_);
 
	unsigned int nstep = l.size();

	if(nstep < 2)
		return;
	
	int xi, yi, xf, yf;

	//gdImageSetAntiAliased(gdCanvas_, lineStyle_.color_);

	if(has2Remap_)
	{
		TeCoord2D cw;

		dataWorld2World(l[0], cw);
		world2Viewport(cw.x_, cw.y_, xi, yi);
		
		dataWorld2World(l[1], cw);
		world2Viewport(cw.x_, cw.y_, xf, yf);

		//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdAntiAliased);
		gdImageLine(gdCanvas_, xi, yi, xf, yf, lineStyle_.color_);
		//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdTrueColor(255, 0, 0));

		for(unsigned int i = 2 ; i < nstep; ++i)
		{ 
			xi = xf;
			yi = yf;

			dataWorld2World(l[i], cw);
			world2Viewport(cw.x_, cw.y_, xf, yf);

			//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdAntiAliased);
			gdImageLine(gdCanvas_, xi, yi, xf, yf, lineStyle_.color_);
			//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdTrueColor(255, 0, 0));
		}
	}
	else
	{
		//gdImageLine(gdCanvas_, 0, 0, 1, 1, gdTrueColor(255, 255, 0));

		world2Viewport(l[0].x_, l[0].y_, xi, yi);
		world2Viewport(l[1].x_, l[1].y_, xf, yf);

		//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdAntiAliased);
		//if(xi != xf && yi != yf)
        gdImageLine(gdCanvas_, xi, yi, xf, yf, lineStyle_.color_);
		//else
		//	gdImageSetPixel(gdCanvas_, xi, yi, lineStyle_.color_);
		//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdTrueColor(255, 0, 0));

		for(unsigned int i = 2 ; i < nstep; ++i)
		{ 
			xi = xf;
			yi = yf;

			world2Viewport(l[i].x_, l[i].y_, xf, yf);
			
			//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdAntiAliased);

			//if(xi != xf && yi != yf)
            gdImageLine(gdCanvas_, xi, yi, xf, yf, lineStyle_.color_);
			//else
			//	gdImageSetPixel(gdCanvas_, xi, yi, lineStyle_.color_);

			//gdImageLine(gdCanvas_, xi, yi, xf, yf, gdTrueColor(255, 0, 0));
		} 
	}
}

void TeMANAGER::TeGDCanvas::draw(const TePolygon& p)
{
	if(polygonTransparency_ == 100 || polygonFillStyle_.style_ == TePolyTypeTransparent)
	{
		drawTransparent(p);
	} 
	else if(p.size() == 1)
	{
		gdImageSetThickness(gdCanvas_, polygonLineStyle_.width_);
		unsigned int ringSize = p[0].size();
		gdPointPtr parray = new gdPoint[ringSize]; 
		dataWorld2Viewport(p[0], parray, ringSize);

		gdImageFilledPolygon(gdCanvas_, parray, ringSize, polygonFillStyle_.color_);
		gdImagePolygon(gdCanvas_, parray, ringSize, polygonLineStyle_.color_);
		delete[] parray;
	} 
	else
	{ 
		gdImageSetThickness(gdCanvas_, polygonLineStyle_.width_);
// there are holes... create an axiliary gdImage to store the partial draw
		if(gdAuxiliaryCanvas_)
		{
			TeGDImageClear(gdAuxiliaryCanvas_, backgroundColor_);
		}
		else
		{
			gdAuxiliaryCanvas_ = gdImageCreateTrueColor(width_, height_);
			if(backgroundColor_ != 0)
				TeGDImageClear(gdAuxiliaryCanvas_, backgroundColor_);
		}
 
		TeGDImageShallowCopy(gdCanvas_, gdAuxiliaryCanvas_); 		

// draw external ring in the auxiliary gdImage with polygon fill color
		unsigned int ringSize = p[0].size();
		gdPointPtr parray = new gdPoint[ringSize]; 
		dataWorld2Viewport(p[0], parray, ringSize); 
		gdImageFilledPolygon(gdAuxiliaryCanvas_, parray, ringSize, polygonFillStyle_.color_);
		gdImagePolygon(gdAuxiliaryCanvas_, parray, ringSize, polygonLineStyle_.color_);
		delete[] parray;

// draw inner rings with canvas background color
		unsigned int nrings = p.size();

		for(unsigned int i = 1; i < nrings; ++i)
  		{
			ringSize = p[i].size();
			gdPointPtr parray = new gdPoint[ringSize];
			dataWorld2Viewport(p[i], parray, ringSize);
			gdImageFilledPolygon(gdAuxiliaryCanvas_, parray, ringSize, backgroundColor_);
			gdImagePolygon(gdAuxiliaryCanvas_, parray, ringSize, polygonLineStyle_.color_);
			delete[] parray;
		} 

		TeGDImageBlend(gdAuxiliaryCanvas_, gdCanvas_, backgroundColor_, polygonFillStyle_.color_, polygonLineStyle_.color_);
	}
}

void TeMANAGER::TeGDCanvas::draw(TeCoord2D& c, const std::string& strText, const double& angle, const double& alignh, const double& alignv)
{
	int x, y;

	if(has2Remap_)
	{
		TeCoord2D cw;
		dataWorld2World(c, cw);
		world2Viewport(cw.x_, cw.y_, x, y);
	}
	else
	{
		world2Viewport(c.x_, c.y_, x, y);
	}

	if(textTTFont_.empty())
	{
		gdFontPtr fptr = 0;

		if(textStyle_.width_ == 0)
			fptr = gdFontGetTiny();
		else if(textStyle_.width_ == 1)
			fptr = gdFontGetSmall();
		else if(textStyle_.width_ == 2)
			fptr = gdFontGetMediumBold();
		else if(textStyle_.width_ == 3)
			fptr = gdFontGetLarge();
		else //if(textStyle_.width_ == 4)
			fptr = gdFontGetGiant();

		int newx = x - (int)((double)(strText.size() * (fptr->w)) * 0.5) + (int)((double)(strText.size() * (fptr->w)) * alignh);
		int newy = y - (fptr->h / 2) - (int)((double)(fptr->h) * alignv);

        drawGDText(newx, newy, (unsigned char*)(strText.c_str()), fptr);
	}
	else
	{
		int brect[8];

		gdImageStringFT(NULL, brect, textStyle_.color_, (char*)(textTTFont_.c_str()), textStyle_.width_, 0.0, x, y, (char*)(strText.c_str()));

		double dx = double(brect[2] - brect[0]);
		double dy = double(brect[1] - brect[5]);

		int newx = x - (int)(dx / 2.0) + (int)(dx * alignh);
		int newy = y + (int)(dy / 2.0) - (int)(dy * alignv);

		if(angle != 0.0)
		{
			int xx = newx - x;
			int yy = y - newy; // desenho eh invertido

			newx = x + (cos(angle)*xx - sin(angle)*yy);
			newy = y - (sin(angle)*xx + cos(angle)*yy);	// - => desenho eh invertido
		}

		drawTTText(newx, newy, (char*)(strText.c_str()), angle);
	}
}

void TeMANAGER::TeGDCanvas::draw(TeCoord2D& c, const std::string& strText, const int& minCollisionTol, const double& angle, const double& alignh, const double& alignv)
{
	int x, y;

	if(has2Remap_)
	{
		TeCoord2D cw;
		dataWorld2World(c, cw);
		world2Viewport(cw.x_, cw.y_, x, y);
	}
	else
	{
		world2Viewport(c.x_, c.y_, x, y);
	}

	std::vector<int> candidatesToConflict;

	if(rtreeConflictBox_.search(TeBox(x, y, x + minCollisionTol, y + minCollisionTol), candidatesToConflict) > 0)
		return;

	if(textTTFont_.empty())
	{
		gdFontPtr fptr = 0;

		if(textStyle_.width_ == 0)
			fptr = gdFontGetTiny();
		else if(textStyle_.width_ == 1)
			fptr = gdFontGetSmall();
		else if(textStyle_.width_ == 2)
			fptr = gdFontGetMediumBold();
		else if(textStyle_.width_ == 3)
			fptr = gdFontGetLarge();
		else //if(textStyle_.width_ == 4)
			fptr = gdFontGetGiant();

		int newx = x - (int)((double)(strText.size() * (fptr->w)) * 0.5) + (int)((double)(strText.size() * (fptr->w)) * alignh);
		int newy = y - (fptr->h / 2) - (int)((double)(fptr->h) * alignv);

		TeBox b;

        getGDTextMBR(newx, newy, (int)(strText.size()), fptr, minCollisionTol, b);

        if(rtreeConflictBox_.search(b, candidatesToConflict) > 0)
			return;

		getGDTextMBR(newx, newy, (int)(strText.size()), fptr, 0, b);

        rtreeConflictBox_.insert(b, 0);

        drawGDText(newx, newy, (unsigned char*)(strText.c_str()), fptr);
	}
	else
	{
		int brect[8];

		gdImageStringFT(NULL, brect, textStyle_.color_, (char*)(textTTFont_.c_str()), textStyle_.width_, 0.0, x, y, (char*)(strText.c_str()));

		double dx = double(brect[2] - brect[0]);
		double dy = double(brect[1] - brect[5]);

		int newx = x - (int)(dx / 2.0) + (int)(dx * alignh);
		int newy = y + (int)(dy / 2.0) - (int)(dy * alignv);

		if(angle != 0.0)
		{
			int xx = newx - x;
			int yy = y - newy; // desenho eh invertido

			newx = x + (cos(angle) * xx - sin(angle) * yy);
			newy = y - (sin(angle) * xx + cos(angle) * yy);	// - => desenho eh invertido
		}

		TeBox b;
		getTTTextMBR(newx, newy, (char*)(strText.c_str()), angle, minCollisionTol, b);

        if(rtreeConflictBox_.search(b, candidatesToConflict) > 0)
			return;

        rtreeConflictBox_.insert(b, 0);

		drawTTText(newx, newy, (char*)(strText.c_str()), angle);
	}
}

void TeMANAGER::TeGDCanvas::draw(TeRaster* raster, TeRasterTransform* transf, bool initRaster)
{
	if(!raster)
		return;

	if(initRaster && !raster->init())
		return;

// creates a raster parameters
	TeRasterParams	params;
	params.ncols_ = width_; 
	params.nlines_ = height_; 
	params.projection(getCanvasProjection());
	params.resx_ = getPixelSizeX();
	params.resy_ = getPixelSizeY();
	params.boundingBoxLinesColumns(wc_.x1(),wc_.y1(),wc_.x2(),wc_.y2(), height_, width_);
	params.nBands(3);
	params.mode_ = 'w';
	params.decoderIdentifier_ = "";
	params.mode_ = 'c';
	params.useDummy_ = true;
	params.setDummy(gdTrueColorGetRed(backgroundColor_), 0);
	params.setDummy(gdTrueColorGetGreen(backgroundColor_), 1);
	params.setDummy(gdTrueColorGetBlue(backgroundColor_), 2);
	params.fileName_ = raster->params().fileName_;

// creates raster decoder for GD images
	TeDecoderGDImage* dec = new TeDecoderGDImage(params);
	dec->setImage(gdCanvas_);

// creates the raster
	TeRaster* backRaster = new TeRaster();
    backRaster->setDecoder(dec);
    backRaster->init();

// calculates the box of input image that intersects the box of the canvas
    TeBox bboxBackRaster = backRaster->params().boundingBox();
    TeBox bboxSearched = TeRemapBox(bboxBackRaster, backRaster->projection(), raster->projection());
	TeBox bboxIntersection;
	
	if(!TeIntersection(raster->params().boundingBox(), bboxSearched, bboxIntersection))
		return;		// no intersection

// create a remapping tool to back raster
    TeRasterRemap remap;

    if(transf)
        remap.setTransformer(transf);

    remap.setOutput(backRaster);

    TeBox b = raster->params().boundingBox();

    remap.setROI(b);

// calculates best resolution level to display the input image on this canvas
    //int res = raster->decoder()->bestResolution(params.resx_ / raster->params().resx_);
	int res = raster->decoder()->bestResolution(bboxIntersection, params.ncols_, params.nlines_, raster->projection());

// check if raster blocks in best level of resolution that intersects the canvas box
    TeRasterParams parBlock;

    if(raster->selectBlocks(bboxIntersection, res, parBlock))
    {
        params.resolution_ = res;

// process each block as an independent raster decoded in memory
        TeRaster* block = new TeRaster;
        TeDecoderMemory* decMem = new TeDecoderMemory(parBlock);
		decMem->init();
		remap.setInput(block);

// portal of raster block selection behaves as portal of geometries
// use the "bool flag - do - while" scheme
        int numBlockProcessed = 0;

        bool flag = true;

        do
        {
            flag = raster->fetchRasterBlock(decMem);
            block->setDecoder(decMem);
            remap.apply();
            ++numBlockProcessed;
			TeRasterParams par = decMem->params();
        }while(flag);

        decMem->clear();
        delete block;
		
		raster->clearBlockSelection();
	}
    else
	{
// no blocks found try to remap the whole raster
        remap.setInput(raster);
        remap.apply(true);	
	}

}

void TeMANAGER::TeGDCanvas::drawPoint(const int& x, const int& y)
{
	const int& w = pointStyle_.width_;

	if(pointStyle_.style_ == TePtTypePlus)
	{
		gdImageLine(gdCanvas_, x - (w / 2), y, x + (w / 2), y, pointStyle_.color_);
		gdImageLine(gdCanvas_, x, y - (w / 2), x, y + (w / 2), pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeStar)
	{
		gdImageLine(gdCanvas_, x - (w / 2), y, x + (w / 2), y, pointStyle_.color_);
		gdImageLine(gdCanvas_, x, y - (w / 2), x, y + (w / 2), pointStyle_.color_);
		gdImageLine(gdCanvas_, x - (w / 2), y - (w / 2), x + (w / 2), y + (w / 2), pointStyle_.color_);
		gdImageLine(gdCanvas_, x + (w / 2), y - (w / 2), x - (w / 2), y + (w / 2), pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeCircle)
	{
		gdImageFilledEllipse(gdCanvas_, x, y, w, w, pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeX)
	{
		gdImageLine(gdCanvas_, x - (w / 2), y - (w / 2), x + (w / 2), y + (w / 2), pointStyle_.color_);
		gdImageLine(gdCanvas_, x - (w / 2), y + (w / 2), x + (w / 2), y - (w / 2), pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeBox)
	{
		gdImageFilledRectangle(gdCanvas_, x - (w / 2), y - (w / 2), x + (w / 2), y + (w / 2), pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeDiamond)
	{
		gdPoint pa[5];
		pa[0].x = x - (w / 2);
		pa[0].y = y;
		pa[1].x = x;
		pa[1].y = y - (w / 2);
		pa[2].x = x + (w / 2);
		pa[2].y = y;
		pa[3].x = x;
		pa[3].y = y + (w / 2);
 		pa[4].x = x - (w / 2);
 		pa[4].y = y;
		gdImageFilledPolygon(gdCanvas_, pa, 5, pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeHollowCircle)
	{
		gdImageArc(gdCanvas_, x, y, w, w, 0, 360, pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeHollowBox)
	{
		gdImageRectangle (gdCanvas_, x - (w / 2), y - (w / 2), x + (w / 2), y + (w / 2), pointStyle_.color_);
	}
	else if(pointStyle_.style_ == TePtTypeHollowDiamond)
	{
		gdImageLine(gdCanvas_, x - (w / 2), y, x, y - (w / 2), pointStyle_.color_);
		gdImageLine(gdCanvas_, x, y - (w / 2), x + (w / 2), y, pointStyle_.color_);
		gdImageLine(gdCanvas_, x + (w / 2), y, x, y + (w / 2), pointStyle_.color_);
		gdImageLine(gdCanvas_, x, y + (w / 2), x - (w / 2), y, pointStyle_.color_);
	}
}

void TeMANAGER::TeGDCanvas::drawSegment(const int& xi, const int& yi, const int& xf, const int& yf)
{
	gdImageSetThickness(gdCanvas_, lineStyle_.width_);
	gdImageLine(gdCanvas_, xi, yi, xf, yf, lineStyle_.color_);
}

void TeMANAGER::TeGDCanvas::drawBox(const int& ulx, const int& uly, const int& lrx, const int& lry)
{
	gdImageSetThickness(gdCanvas_, polygonLineStyle_.width_);

	if((polygonTransparency_ == 100) || (polygonFillStyle_.style_ == TePolyTypeTransparent))
	{
		gdImageRectangle(gdCanvas_, ulx, uly, lrx, lry, polygonLineStyle_.color_);				
	}
	else
	{
		gdImageFilledRectangle(gdCanvas_, ulx, uly, lrx, lry, polygonFillStyle_.color_);
		gdImageRectangle(gdCanvas_, ulx, uly, lrx, lry, polygonLineStyle_.color_);				
	}			
}

void TeMANAGER::TeGDCanvas::drawArc(const int& cx, const int& cy, const int& w, const int& h, const int& startAngle, const int& endAngle)
{			
	gdImageFilledArc(gdCanvas_, cx, cy, w, h, startAngle, endAngle, polygonFillStyle_.color_, gdPie);
	gdImageFilledArc(gdCanvas_, cx, cy, w, h, startAngle, endAngle, polygonLineStyle_.color_, gdEdged | gdNoFill);
}

void TeMANAGER::TeGDCanvas::getTextMBR(int x, int y, const std::string& strText,
							const double& angle,
			                const double& alignh, const double& alignv, TeBox& bout) const
{
	if(textTTFont_.empty())
	{
		gdFontPtr fptr = 0;

		if(textStyle_.width_ == 0)
			fptr = gdFontGetTiny();
		else if(textStyle_.width_ == 1)
			fptr = gdFontGetSmall();
		else if(textStyle_.width_ == 2)
			fptr = gdFontGetMediumBold();
		else if(textStyle_.width_ == 3)
			fptr = gdFontGetLarge();
		else //if(textStyle_.width_ == 4)
			fptr = gdFontGetGiant();

		int newx = x - (int)((double)(strText.size() * (fptr->w)) * 0.5) + (int)((double)(strText.size() * (fptr->w)) * alignh);
		int newy = y - (fptr->h / 2) - (int)((double)(fptr->h) * alignv);

		getGDTextMBR(newx, newy, (int)(strText.size()), fptr, 0, bout);
	}
	else
	{
		getTTTextMBR(x, y, (char*)(strText.c_str()), angle, alignh, alignv, 0, bout);
	}
}

void TeMANAGER::TeGDCanvas::getTextMBR(const int& x, const int& y, const std::string& strText,
							const double& angle,
							TeBox& bout) const
{
	if(textTTFont_.empty())
	{
		gdFontPtr font = 0;

		if(textStyle_.width_ == 0)
			font = gdFontGetTiny();
		else if(textStyle_.width_ == 1)
			font = gdFontGetSmall();
		else if(textStyle_.width_ == 2)
			font = gdFontGetMediumBold();
		else if(textStyle_.width_ == 3)
			font = gdFontGetLarge();
		else //if(textStyle_.width_ == 4)
			font = gdFontGetGiant();

		getGDTextMBR(x, y, (int)(strText.size()), font, 0, bout);
	}
	else
	{
		getTTTextMBR(x, y, (char*)(strText.c_str()), angle, 0, bout);
	}
}

void TeMANAGER::TeGDCanvas::drawTransparent(const TePolygon& p)
{
	gdImageSetThickness(gdCanvas_, polygonLineStyle_.width_);

	const unsigned int nrings = p.size(); 

	int xi, yi, xf, yf;

	if(has2Remap_)
	{
		TeCoord2D cw;

		for(unsigned int i = 0 ; i < nrings; ++i)
		{
			const unsigned int nstep = p[i].size();
			const TeLine2D& l = p[i];			

			dataWorld2World(l[0], cw);
			world2Viewport(cw.x_, cw.y_, xi, yi);
			
			dataWorld2World(l[1], cw);
			world2Viewport(cw.x_, cw.y_, xf, yf);

			gdImageLine(gdCanvas_, xi, yi, xf, yf, polygonLineStyle_.color_);

            for(unsigned int j = 2 ; j < nstep; ++j)
            { 
				xi = xf;
				yi = yf;

				dataWorld2World(l[j], cw);
				world2Viewport(cw.x_, cw.y_, xf, yf);
				
				gdImageLine(gdCanvas_, xi, yi, xf, yf, polygonLineStyle_.color_);
			}
		}		
	}
	else
	{
		for(unsigned int i = 0 ; i < nrings; ++i)
		{
			const unsigned int nstep = p[i].size();
			const TeLine2D& l = p[i];

			world2Viewport(l[0].x_, l[0].y_, xi, yi);
			world2Viewport(l[1].x_, l[1].y_, xf, yf);
            
			gdImageLine(gdCanvas_, xi, yi, xf, yf, polygonLineStyle_.color_);

            for(unsigned int j = 2 ; j < nstep; ++j)
            { 
				xi = xf;
				yi = yf;

				world2Viewport(l[j].x_, l[j].y_, xf, yf);

                gdImageLine(gdCanvas_, xi, yi, xf, yf, polygonLineStyle_.color_);
			}
		}
	}
}

void TeMANAGER::TeGDCanvas::drawGDText(const int& x, const int& y, unsigned char* strText, gdFontPtr font)
{
	if(textOutlineEnable_)
	{
		gdImageString(gdCanvas_, font, x, y - 1, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x, y + 1, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x + 1, y, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x - 1, y, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x - 1 , y - 1, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x - 1, y + 1, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x + 1, y - 1, strText, textOutlineStyle_.color_);
		gdImageString(gdCanvas_, font, x + 1, y + 1, strText, textOutlineStyle_.color_);
	}

	//if(textShadowStyle_.color_ != textOutlineStyle_.color_)
	//	gdImageString(gdCanvas_, font, x + textShadowStyle_.width_, y + textShadowStyle_.width_, strText, textShadowStyle_.color_);

	gdImageString(gdCanvas_, font, x, y, strText, textStyle_.color_);
}

void TeMANAGER::TeGDCanvas::drawTTText(const int& x, const int& y, char* strText, const double& angle) const
{
	int brect[8];

    if(textOutlineEnable_)
    {	
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x, y - 1, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x, y + 1, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x + 1, y, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x - 1, y, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x - 1, y - 1, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x - 1, y + 1, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x + 1, y - 1, strText);
		gdImageStringFT(gdCanvas_, brect, textOutlineStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x + 1, y + 1, strText);
	}

	//if(textShadowStyle_.color_ != textOutlineStyle_.color_)
	//	gdImageStringFT(gdCanvas_, brect, textShadowStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x + textShadowStyle_.width_, y + textShadowStyle_.width_, strText);

	gdImageStringFT(gdCanvas_, brect, textStyle_.color_, (char*)(textTTFont_.c_str()), (double)(textStyle_.width_), angle, x, y, strText);
}


void TeMANAGER::TeGDCanvas::getGDTextMBR(const int& x, const int& y, const int& textLen, gdFontPtr font, const int& minCollisionTol,  TeBox& bout) const
{
	bout.x1_ = (double)(x - minCollisionTol);
	bout.y1_ = (double)(y - minCollisionTol);

	bout.x2_ = (double)(x + textLen * font->w + minCollisionTol);
	bout.y2_ = (double)(y + font->h + minCollisionTol);
}

void TeMANAGER::TeGDCanvas::getTTTextMBR(const int& x, const int& y, char* strText,  const double& angle, const int& minCollisionTol,  TeBox& bout) const
{
	int brect[8];

	gdImageStringFT(NULL, brect, textStyle_.color_, (char*)(textTTFont_.c_str()), textStyle_.width_, angle, x, y, strText);

	int minx1 = (brect[0] < brect[2]) ? brect[0] : brect[2];
	int minx2 = (brect[4] < brect[6]) ? brect[4] : brect[6];
	bout.x1_ = (double)((minx1 < minx2) ? minx1 - minCollisionTol : minx2 - minCollisionTol);

	int maxx1 = (brect[0] > brect[2]) ? brect[0] : brect[2];
	int maxx2 = (brect[4] > brect[6]) ? brect[4] : brect[6];
	bout.x2_ = (double)((maxx1 > maxx2) ? maxx1 + minCollisionTol : maxx2 + minCollisionTol);

	int miny1 = (brect[1] < brect[3]) ? brect[1] : brect[3];
	int miny2 = (brect[5] < brect[7]) ? brect[5] : brect[7];
	bout.y1_ = (double)((miny1 < miny2) ? miny1 - minCollisionTol : miny2 - minCollisionTol);
	
	int maxy1 = (brect[1] > brect[3]) ? brect[1] : brect[3];
	int maxy2 = (brect[5] > brect[7]) ? brect[5] : brect[7];

	bout.y2_ = (double)((maxy1 > maxy2) ? maxy1 + minCollisionTol : maxy2 + minCollisionTol);
}

void TeMANAGER::TeGDCanvas::getTTTextMBR(const int& x, const int& y, char* strText, const double& angle, const double& alignh, const double& alignv, const int& minCollisionTol, TeBox& bout) const
{
	int brect[8];

	gdImageStringFT(NULL, brect, textStyle_.color_, (char*)(textTTFont_.c_str()), textStyle_.width_, angle, x, y, strText);

	int minx1 = (brect[0] < brect[2]) ? brect[0] : brect[2];
	int minx2 = (brect[4] < brect[6]) ? brect[4] : brect[6];
	int minx  = (minx1 < minx2) ? minx1 : minx2;

	int maxx1 = (brect[0] > brect[2]) ? brect[0] : brect[2];
	int maxx2 = (brect[4] > brect[6]) ? brect[4] : brect[6];
	int maxx  = (maxx1 > maxx2) ? maxx1 : maxx2;

	double dx = double(maxx - minx);

	int miny1 = (brect[1] < brect[3]) ? brect[1] : brect[3];
	int miny2 = (brect[5] < brect[7]) ? brect[5] : brect[7];
	int miny  = (miny1 < miny1) ? miny1 : miny2;
	
	int maxy1 = (brect[1] > brect[3]) ? brect[1] : brect[3];
	int maxy2 = (brect[5] > brect[7]) ? brect[5] : brect[7];
	int maxy  = (maxy1 > maxy2) ? maxy1 : maxy2;

	double dy = double(maxy - miny);

	int newx = x - (int)(dx / 2.0) + (int)(dx * alignh);
	int newy = y + (int)(dy / 2.0) - (int)(dy * alignv);

	bout.x1_ = (double)(newx - minCollisionTol);
	bout.y1_ = ((double)(newy)) - dy - ((double)(minCollisionTol));

	bout.x2_ = ((double)(newx)) + dx + ((double)(minCollisionTol));
	bout.y2_ = (double)(newy + minCollisionTol);
}

void TeMANAGER::TeGDCanvas::dataWorld2Viewport(const TeLine2D& l, gdPointPtr pts, const unsigned int& npts)
{
	if(has2Remap_)
	{
		TeCoord2D cw;				

		for(unsigned int i = 0; i < npts; ++i)
		{
            cw = dataProjection_->PC2LL(l[i]);
			cw = canvasProjection_->LL2PC(cw);

            pts[i].x = static_cast<int>((cw.x_ - xmin_) * fX_);
			pts[i].y = static_cast<int>(height_ - ((cw.y_ - ymin_) * fY_));
		}
	}
	else
	{
		for(unsigned int i = 0; i < npts; ++i)
		{
			pts[i].x = static_cast<int>((l[i].x_ - xmin_) * fX_);
			pts[i].y = static_cast<int>(height_ - ((l[i].y_ - ymin_) * fY_));
		}
	}
}


