/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

#ifndef  __TERRALIB_INTERNAL_QTGLBOX_H
#define  __TERRALIB_INTERNAL_QTGLBOX_H

#include <qgl.h>
#include <qpixmap.h>
#include "../../kernel/TeDataTypes.h"
#include <vector>

#include "../../kernel/TeDefines.h"

using namespace std;

class TeTheme;
class TeDatabase;
class TeQtGrid;

class TL_DLL TeQtGLWidget : public QGLWidget
{
    Q_OBJECT

public:

    TeQtGLWidget( QWidget* parent, const char* name, TeQtGrid* grid, string xField, string yField, string zField, QPaintDevice* xy, QPaintDevice* xz, QPaintDevice* yz, int selectedObjects = TeAll, string func = "NONE");
    ~TeQtGLWidget();
	void drawAxis(bool b) {drawAxis_ = b; updateGL();}

    GLfloat xRot_, yRot_, zRot_;
	GLfloat xTranslation_, yTranslation_, zTranslation_;
	GLfloat viewAngle_;
	GLfloat xOff_, yOff_, zOff_;
	GLfloat xSize_, ySize_, zSize_;
	string xField_, yField_, zField_;
	bool frontLighting_;
	bool addSelection_;
	TeTheme* theme_;
	TeDatabase* db_;

public slots:

	// the values of degrees, offset, size, translation and angle are between 0 - 99.
	// therefore it shoud be conveted to adequate value
	
    void	setXRotation( int degrees );
    void	setYRotation( int degrees );
    void	setZRotation( int degrees );

    void	setXOffset( int offset );
    void	setYOffset( int offset );
    void	setZOffset( int offset );

    void	setXSize( int size );
    void	setYSize( int size );
    void	setZSize( int size );

    void	xOffsetReleased();
    void	yOffsetReleased();
    void	zOffsetReleased();

    void	xSizeReleased();
    void	ySizeReleased();
    void	zSizeReleased();

    void	setViewAngle( int angle );

    void	setLightingMode( int m );

	void	paint2D();
	void	paint3D();
	void	getCubeCoordinate(double& x1, double& y1, double& z1, double& x2, double& y2, double& z2);
	void	drawSelecteds();
	void	changeFunction(string function);
	void	changeInputObjects(int v);
	void	changeQuality(int v);
	void	updateSelection();
	void	deleteGLObject();
	void	initGLObject();

protected:

    void	initializeGL();
    void	paintGL();
    void	resizeGL( int w, int h );
	void	mousePressEvent(QMouseEvent*);
	void	mouseMoveEvent(QMouseEvent*);
	void	mouseReleaseEvent(QMouseEvent*);
	void	keyPressEvent(QKeyEvent*);
	void	keyReleaseEvent(QKeyEvent*);
	void	draw2DPixmaps();
    void	makeObject();
	void	getValues();
	void	selectFromHexaedro();

signals:
	void setXOffsetSignal(int);
	void setYOffsetSignal(int);
	void setZOffsetSignal(int);
	void setXSizeSignal(int);
	void setYSizeSignal(int);
	void setZSizeSignal(int);
	void setXRotationSignal(int);
	void setYRotationSignal(int);
	void setZRotationSignal(int);
	void setZoomSignal(int);


private:

	int selIni_;
	int quality_;
	GLuint cone_;
	vector<GLuint> ballVec_;
	vector<string> idVec_;
	map<string, GLuint> ballMap_;
	GLfloat scale_;
	GLfloat min_, max_;
	QPoint mousePosition_;
	vector<double> xVec_, yVec_, zVec_;
	bool drawAxis_;
	bool drawSelecteds_;
	QPaintDevice *xyDevice_, *zxDevice_, *yzDevice_;
	QPixmap xyPixmap_, zxPixmap_, yzPixmap_;
	QColor defaultColor_;
	QColor pointingColor_;
	string function_;
	int selectedObjects_;
	TeQtGrid* grid_;
	int	keyPress_;
};


#endif // __TERRALIB_INTERNAL_QTGLBOX_H
