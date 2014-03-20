#include "tedxflayer.h"
#include "TeGeometry.h"

TeDXFLayer::TeDXFLayer(const string & layerName) 
{
	name_ = layerName;
	_visible = false;
}

TeDXFLayer::~TeDXFLayer() 
{
}

const string TeDXFLayer::getName() const 
{
  return name_;
}

void TeDXFLayer::addPoint(const TePoint & pt) 
{
	_points.add(pt);
}

void TeDXFLayer::addLine(const TeLine2D & line) 
{
	_lines.add(line);
}

void TeDXFLayer::addPolygon(const TePolygon & poly) 
{
	_polygons.add(poly);
}

const TePointSet TeDXFLayer::getPoints() const 
{
  return _points;
}

const TeLineSet TeDXFLayer::getLines() const 
{
  return _lines;
}

const TePolygonSet TeDXFLayer::getPolygons() const 
{
  return _polygons;
}

void TeDXFLayer::setVisible(const bool & value) 
{
  _visible = value;
}

const bool TeDXFLayer::getVisible() const 
{
  return _visible;
}

