#include "tedxfvisitor.h"
#ifdef __APPLE__
	#include "../../../dxflib/dl_entities.h"
	#include "../../../dxflib/dl_dxf.h"
#else
	#include "dl_entities.h"
	#include "dl_dxf.h"
#endif 

#include "tedxflayer.h"

TeDXFVisitor::TeDXFVisitor(const string & fileName) 
{
	DL_Dxf* dxf = new DL_Dxf();

    if (!dxf->in(fileName, this)) 
	{ // if file open failed
		delete dxf;
        return;
    }

	delete dxf;
}

TeDXFVisitor::~TeDXFVisitor() 
{
}

void TeDXFVisitor::addLayer(const DL_LayerData & data) 
{
	TeDXFLayer* layer = new TeDXFLayer(data.name);
	layers_.push_back(layer);
}

void TeDXFVisitor::addPoint(const DL_PointData & data) 
{
	TePoint p(data.x, data.y);

	if(!layers_.empty())
		layers_[layers_.size()-1]->addPoint(p);
}

void TeDXFVisitor::addLine(const DL_LineData & data) 
{
	TeLine2D l;
	l.add(TeCoord2D(data.x1, data.y1));
	l.add(TeCoord2D(data.x2, data.y2));

	if(!layers_.empty())
		layers_[layers_.size()-1]->addLine(l);
}

void TeDXFVisitor::addPolyline(const DL_PolylineData & /*data*/) 
{
}

void TeDXFVisitor::addVertex(const DL_VertexData & data) 
{
	TeCoord2D c(data.x, data.y);

	line_.add(c);
}

void TeDXFVisitor::endEntity() 
{
	if(!line_.empty())
	{
		if(line_[0] == line_[line_.size()-1])
		{
			TePolygon p;
			p.add(line_);

			if(!layers_.empty())
				layers_[layers_.size()-1]->addPolygon(p);
		}
		else if(!layers_.empty())
			layers_[layers_.size()-1]->addLine(line_);
	}

	line_ = TeLinearRing();
}

const vector<TeDXFLayer *> & TeDXFVisitor::getLayers() const 
{
  return layers_;
}

