// TerraManager include files
#include "TeImageMapCanvas.h"
#include "TerraManagerException.h"
#include <cstring>

// Defines
#define DEFAULTIMAGEMAPTEXT 262144	// 250 Kbytes
#define MAXIMAGEMAPTEXT 10485760	// 10 Mbytes

TeMANAGER::TeImageMapCanvas::TeImageMapCanvas(const std::string& mapName, const std::string& mapId)
	: imageMapStr_(0),
	  imageMapStrSize_(0), imageMapStrAlloc_(0),
	  imageMapName_(mapName), imageMapId_(mapId),
	  xmin_(0.0), xmax_(0.0), ymin_(0.0), ymax_(0.0),
	  width_(0), height_(0), fX_(1.0), fY_(1.0),
	  canvasProjection_(0), dataProjection_(0),
	  has2Remap_(false), areaShapeType_(TeIMRECT),
	  areaCircleRadius_(8), areaRectWidth_(8)
{
	imageMapStr_ = new char[DEFAULTIMAGEMAPTEXT];
	imageMapStrSize_ = DEFAULTIMAGEMAPTEXT;

	checkBuffer(imageMapName_.length() + imageMapId_.length() + 32);

	if(!imageMapName_.empty())
	{
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<map name=\"%s\"", imageMapName_.c_str());

		if(!imageMapId_.empty())
			imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, " id=\"%s\">\n", imageMapId_.c_str());
		else
			imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, ">\n");
	}
	else if(!imageMapId_.empty())
	{
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<map id=\"%s\">\n", imageMapId_.c_str());
	}
}

TeMANAGER::TeImageMapCanvas::~TeImageMapCanvas()
{
	delete [] imageMapStr_;
	delete canvasProjection_;
	delete dataProjection_;
}

void TeMANAGER::TeImageMapCanvas::setWorld(const TeBox& b, const int& w, const int& h)
{
	xmin_= b.x1();
	xmax_= b.x2(); 
	ymin_= b.y1(); 
	ymax_= b.y2();
	width_ = w; 
	height_ = h; 

	double dxw = xmax_ - xmin_;
	double dyw = ymax_ - ymin_;

	fX_ = width_ / dxw;
	fY_ = height_ / dyw;
 
	wc_ = TeBox(xmin_, ymin_, xmax_, ymax_);
}

const TeBox& TeMANAGER::TeImageMapCanvas::getWorld() const
{
	return wc_ ;
}

TeProjection* TeMANAGER::TeImageMapCanvas::getCanvasProjection() const
{
	return canvasProjection_;
}

void TeMANAGER::TeImageMapCanvas::setCanvasProjection(TeProjection& proj)
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

void TeMANAGER::TeImageMapCanvas::setDataProjection(TeProjection& proj)
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

const std::map<std::string, pair<std::string, std::string> >& TeMANAGER::TeImageMapCanvas::getAreaProperty() const
{
	return areaProperties_;
}

void TeMANAGER::TeImageMapCanvas::setAreaProperty(const std::string propertyName,
									   const std::string& propertyValue,
									   const std::string valueSrc)
{
	areaProperties_[propertyName] = pair<std::string, std::string>(propertyValue, valueSrc);
}

void TeMANAGER::TeImageMapCanvas::setAreaShape(const TeImageMapShapeType& areaShapeType)
{
	areaShapeType_ = areaShapeType;
}

void TeMANAGER::TeImageMapCanvas::setAreaCircleRadius(const int& radius)
{
	areaCircleRadius_ = radius;
}

void TeMANAGER::TeImageMapCanvas::setAreaRectWidth(const int& width)
{
	areaRectWidth_ = width;
}

void TeMANAGER::TeImageMapCanvas::draw(TeCoord2D& c)
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

void TeMANAGER::TeImageMapCanvas::draw(TePoint &p)
{
	draw(p.location());
}

void TeMANAGER::TeImageMapCanvas::draw(const TeBox& b)
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

void TeMANAGER::TeImageMapCanvas::draw(TeCell& c)
{
	draw(c.box());
}

void TeMANAGER::TeImageMapCanvas::draw(TeLine2D& l)
{
	if(areaShapeType_ == TeMANAGER::TeImageMapCanvas::TeIMCIRCLE)
	{
		TeCoord2D c = l.box().center();
		draw(c);
	}
	else
	{
		const TeBox& b = l.box();
		draw(b);
	}
}

void TeMANAGER::TeImageMapCanvas::draw(TePolygon& p)
{
	int xi, yi, xf, yf;

	if(has2Remap_)
	{
		TeCoord2D cw;
		
		const unsigned int nstep = p[0].size();
		const TeLine2D& l = p[0];

		dataWorld2World(l[0], cw);
		world2Viewport(cw.x_, cw.y_, xi, yi);
		
		dataWorld2World(l[1], cw);
		world2Viewport(cw.x_, cw.y_, xf, yf);

		checkBuffer(22);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<area shape=\"polygon\" ");

		addAreaAttributes();

		checkBuffer(69);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "coords=\"%d,%d", xi, yi);

		if((xf != xi) || (yf != yi))
		{
			checkBuffer(61);
			imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, ",%d,%d", xf, yf);
		}

        for(unsigned int j = 2 ; j < nstep; ++j)
        { 
			xi = xf;
			yi = yf;

			dataWorld2World(l[j], cw);
			world2Viewport(cw.x_, cw.y_, xf, yf);

			if((xf == xi) && (yf == yi))
				continue;
			
			checkBuffer(62);
            imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, ",%d,%d", xf, yf);
		}

		checkBuffer(4);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "\"/>\n");
	}
	else
	{
		const unsigned int nstep = p[0].size();
		const TeLine2D& l = p[0];

		world2Viewport(l[0].x_, l[0].y_, xi, yi);
		world2Viewport(l[1].x_, l[1].y_, xf, yf);
        
		checkBuffer(22);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<area shape=\"polygon\" ");

		addAreaAttributes();

		checkBuffer(69);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "coords=\"%d,%d", xi, yi);

		if((xf != xi) || (yf != yi))
		{
			checkBuffer(61);
			imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, ",%d,%d", xf, yf);
		}

        for(unsigned int j = 2 ; j < nstep; ++j)
        { 
			xi = xf;
			yi = yf;

			world2Viewport(l[j].x_, l[j].y_, xf, yf);

			if((xf == xi) && (yf == yi))
				continue;

			checkBuffer(62);
            imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, ",%d,%d", xf, yf);
		}

		checkBuffer(4);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "\"/>\n");
	}
}

void TeMANAGER::TeImageMapCanvas::drawPoint(const int& x, const int& y)
{
	if(areaShapeType_ == TeMANAGER::TeImageMapCanvas::TeIMCIRCLE)
	{
		checkBuffer(21);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<area shape=\"circle\" ");

		addAreaAttributes();

		checkBuffer(104);
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "coords=\"%d,%d,%d\"/>\n", x, y, areaCircleRadius_);
	}
	else
	{
		drawBox(x - (areaRectWidth_ / 2), y - (areaRectWidth_ / 2), x + (areaRectWidth_ / 2), y + (areaRectWidth_ / 2));
	}
}

void TeMANAGER::TeImageMapCanvas::drawSegment(const int& xi, const int& yi, const int& xf, const int& yf)
{
	throw TerraManagerException("The method draw doesn't applies to segments.", "TeImageMapCanvas");
}

void TeMANAGER::TeImageMapCanvas::drawBox(const int& ulx, const int& uly, const int& lrx, const int& lry)
{
	checkBuffer(19);
	imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "<area shape=\"rect\" ");

    addAreaAttributes();

	checkBuffer(135);
    imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "coords=\"%d,%d,%d,%d\"/>\n", ulx, uly, lrx, lry);
}

void TeMANAGER::TeImageMapCanvas::close()
{
	checkBuffer(6);
	imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "</map>");
}

char* TeMANAGER::TeImageMapCanvas::getImageMap() const
{
	*(imageMapStr_ + imageMapStrAlloc_) = '\0';
	return imageMapStr_;
}

int TeMANAGER::TeImageMapCanvas::getImageMapSize() const
{
	return imageMapStrAlloc_;
}

void TeMANAGER::TeImageMapCanvas::world2Viewport(const double& wx, const double& wy, int& vx, int& vy)
{
	vx = static_cast<int>((wx - xmin_) * fX_);
	vy = static_cast<int>(height_ - ((wy - ymin_) * fY_));
}

void TeMANAGER::TeImageMapCanvas::dataWorld2World(TeCoord2D& dw, TeCoord2D& w)
{
	w = dataProjection_->PC2LL(dw);
	w = canvasProjection_->LL2PC(w);
}

void TeMANAGER::TeImageMapCanvas::world2DataWorld(TeCoord2D& w, TeCoord2D& dw)
{
	dw = canvasProjection_->PC2LL(w);
	dw = dataProjection_->LL2PC(dw);
}

void TeMANAGER::TeImageMapCanvas::addAreaAttributes()
{
	std::map<std::string, pair<std::string, string> >::const_iterator it = areaProperties_.begin();

	while(it != areaProperties_.end())
	{
		std::string propertyVal;

		size_t pos = it->second.first.find("%s");
		
		if(pos != string::npos)
		{
			propertyVal  = it->second.first.substr(0, pos);
			propertyVal += it->second.second;
			propertyVal += it->second.first.substr(pos + 2, it->second.first.length() - 1);			
		}
		else
		{
			propertyVal = it->second.first;
		}

		
		checkBuffer(4 + it->first.length() + propertyVal.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "%s=\"%s\" ", it->first.c_str(), propertyVal.c_str());

		++it;
	}
}

/*
void TeMANAGER::TeImageMapCanvas::addAreaAttributes()
{
	if(!areaHREF_.empty())
	{
		checkBuffer(8 + areaHREF_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "href=\"%s\" ", areaHREF_.c_str());
	}

	if(!areaTarget_.empty())
	{
		checkBuffer(10 + areaTarget_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "target=\"%s\" ", areaTarget_.c_str());
	}

	if(!areaAlt_.empty())
	{
		checkBuffer(7 + areaAlt_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "alt=\"%s\" ", areaAlt_.c_str());
	}

	if(!areaTitle_.empty())
	{
		checkBuffer(9 + areaTitle_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "title=\"%s\" ", areaTitle_.c_str());
	}

	if(!areaOnMouseOver_.empty())
	{
		checkBuffer(15 + areaOnMouseOver_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "onMouseOver=\"%s\" ", areaOnMouseOver_.c_str());
	}

	if(!areaOnMouseOut_.empty())
	{
		checkBuffer(14 + areaOnMouseOut_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "onMouseOut=\"%s\" ", areaOnMouseOut_.c_str());
	}

	if(!areaOnClick_.empty())
	{
		checkBuffer(11 + areaOnClick_.length());
		imageMapStrAlloc_ += sprintf(imageMapStr_ + imageMapStrAlloc_, "onClick=\"%s\" ", areaOnClick_.c_str());
	}
}
*/
void TeMANAGER::TeImageMapCanvas::checkBuffer(const int& nbytesToBeWritten)
{
	if((nbytesToBeWritten + 512) > (imageMapStrSize_ - imageMapStrAlloc_))
	{
		if(imageMapStrSize_ == MAXIMAGEMAPTEXT)
		{
			throw TerraManagerException("There is no sufficient space to build the image map because the buffer limit was reached.", "TeImageMapCanvas");
		}
		else
		{			
			char* newbuffer = new char[imageMapStrSize_ + DEFAULTIMAGEMAPTEXT];
			memcpy(newbuffer, imageMapStr_, imageMapStrSize_);
			delete [] imageMapStr_;
			imageMapStrSize_ += DEFAULTIMAGEMAPTEXT;
			imageMapStr_ = newbuffer;
		}
	}
}

