/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.
This file is part of TerraView. TerraView is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

You should have received a copy of the GNU General Public License
along with TerraView.
The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The software provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use of
this program and its documentation.
************************************************************************************/

#include "TeQtGLWidget.h"
#include "qframe.h"
#include <qpainter.h>
#include "../../kernel/TeDefines.h"
#include "math.h"
#include "../../kernel/TeUtils.h"
#include "TeQtGrid.h"
#include "../../kernel/TeExternalTheme.h"
#include "../../kernel/TeDatabase.h"
#include "../../utils/TeDatabaseUtils.h"

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

TeQtGLWidget::TeQtGLWidget( QWidget* parent, const char* name, TeQtGrid* grid, string xField, string yField, string zField, QPaintDevice* xy, QPaintDevice* zx, QPaintDevice* yz, int selectedObjects, string func)
    : QGLWidget( parent, name )
{
	setFocusPolicy(QWidget::StrongFocus);

	xRot_ = yRot_ = zRot_ = 0.0;		// default object rotation
    xOff_ = yOff_ = zOff_ = -0.2;		// default object Offset
    xSize_ = ySize_ = zSize_ = 0.4;		// default object Size
    scale_ = 1.;						// default object scale_
	viewAngle_ = 60.;
	xTranslation_ = 0.;
	yTranslation_ = 0.;
	zTranslation_ = -3.5;
	frontLighting_ = true;
	drawAxis_ = true;
	xField_ = xField;
	yField_ = yField;
	zField_ = zField;
	grid_ = grid;
	theme_ = (TeTheme*)grid_->getTheme()->getTheme();

	if(theme_->type() == TeTHEME)
		db_ = theme_->layer()->database();
	else if(theme_->type() == TeEXTERNALTHEME)
		db_ = ((TeExternalTheme*)theme_)->getRemoteTheme()->layer()->database();

	selectedObjects_ = selectedObjects;
	function_ = func;
	quality_ = 12;
	addSelection_ = false;
	keyPress_ = 0;

	TeGeomRepVisualMap& vmap = theme_->defaultLegend().getVisualMap();
	if(vmap.find(TePOLYGONS) != vmap.end())
		defaultColor_ = QColor(vmap[TePOLYGONS]->color().red_, vmap[TePOLYGONS]->color().green_, vmap[TePOLYGONS]->color().blue_); 
	else if(vmap.find(TeLINES) != vmap.end())
		defaultColor_ = QColor(vmap[TeLINES]->color().red_, vmap[TeLINES]->color().green_, vmap[TeLINES]->color().blue_); 
	else if(vmap.find(TePOINTS) != vmap.end())
		defaultColor_ = QColor(vmap[TePOINTS]->color().red_, vmap[TePOINTS]->color().green_, vmap[TePOINTS]->color().blue_); 

	TeGeomRepVisualMap& pmap = theme_->pointingLegend().getVisualMap();
	if(pmap.find(TePOLYGONS) != pmap.end())
		pointingColor_ = QColor(pmap[TePOLYGONS]->color().red_, pmap[TePOLYGONS]->color().green_, pmap[TePOLYGONS]->color().blue_); 
	else if(pmap.find(TeLINES) != pmap.end())
		pointingColor_ = QColor(pmap[TeLINES]->color().red_, pmap[TeLINES]->color().green_, pmap[TeLINES]->color().blue_); 
	else if(pmap.find(TePOINTS) != pmap.end())
		pointingColor_ = QColor(pmap[TePOINTS]->color().red_, pmap[TePOINTS]->color().green_, pmap[TePOINTS]->color().blue_); 

	xyDevice_ = xy;
	zxDevice_ = zx;
	yzDevice_ = yz;
}

TeQtGLWidget::~TeQtGLWidget()
{
	deleteGLObject();
}

void TeQtGLWidget::paintGL()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	int w = width();
	int h = height();

	if (w <= h) 
		gluPerspective(viewAngle_ * (GLfloat) h / (GLfloat) w, (GLfloat) h / (GLfloat) w, 1.0, 16.0); 
	else 
		gluPerspective(viewAngle_ * (GLfloat) w / (GLfloat) h, (GLfloat) w / (GLfloat) h, 1.0, 18.0); 

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 

	glTranslatef(xTranslation_, yTranslation_, zTranslation_); 
    glRotatef( xRot_, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot_, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot_, 0.0, 0.0, 1.0 );

    glLineWidth( 1.0 );
    qglColor( magenta );
    glBegin( GL_LINE_LOOP );
    glVertex3f( xOff_ + xSize_,  yOff_ + ySize_, zOff_ );
    glVertex3f( xOff_ + xSize_, yOff_ + ySize_, zOff_ + zSize_ );
    glVertex3f( xOff_, yOff_ + ySize_, zOff_ + zSize_ );
    glVertex3f( xOff_, yOff_ + ySize_, zOff_ );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f( xOff_ + xSize_,  yOff_, zOff_ );
    glVertex3f( xOff_, yOff_, zOff_ );
    glVertex3f( xOff_, yOff_, zOff_ + zSize_ );
    glVertex3f( xOff_ + xSize_, yOff_, zOff_ + zSize_ );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f( xOff_ + xSize_,  yOff_ + ySize_, zOff_ );   glVertex3f( xOff_ + xSize_,  yOff_, zOff_ );
    glVertex3f( xOff_ + xSize_, yOff_ + ySize_, zOff_ + zSize_ );   glVertex3f( xOff_ + xSize_,  yOff_, zOff_ + zSize_ );
    glVertex3f( xOff_, yOff_ + ySize_, zOff_ + zSize_ );   glVertex3f( xOff_, yOff_, zOff_ + zSize_ );
    glVertex3f( xOff_,  yOff_ + ySize_, zOff_ );   glVertex3f( xOff_, yOff_, zOff_ );
    glEnd();


	// draw white cube
    qglColor( white );
    glBegin( GL_LINE_LOOP );
    glVertex3f( 1.0,  1.0, -1.0 );
    glVertex3f( 1.0, 1.0, 1.0 );
    glVertex3f( -1.0, 1.0, 1.0 );
    glVertex3f( -1.0,  1.0, -1.0 );
    glEnd();

    glBegin( GL_LINES );
	if(!drawAxis_)
	{
		glVertex3f( 1.0,  -1.0, -1.0 );	glVertex3f( -1.0, -1.0, -1.0 );
		glVertex3f( -1.0, -1.0, -1.0 );	glVertex3f( -1.0, -1.0, 1.0 );
	}
    glVertex3f( -1.0, -1.0, 1.0 );	glVertex3f( 1.0,  -1.0, 1.0 );
	glVertex3f( 1.0,  -1.0, 1.0 );	glVertex3f( 1.0,  -1.0, -1.0 );
    glVertex3f( 1.0,  1.0, -1.0 );   glVertex3f( 1.0,  -1.0, -1.0 );
    glVertex3f( 1.0, 1.0, 1.0 );   glVertex3f( 1.0,  -1.0, 1.0 );
    glVertex3f( -1.0, 1.0, 1.0 );   glVertex3f( -1.0, -1.0, 1.0 );
	if(!drawAxis_)
		glVertex3f( -1.0,  1.0, -1.0 );   glVertex3f( -1.0, -1.0, -1.0 );
    glEnd();


	GLfloat mat_specular[] = { .1, .1, .1, 1.0 };
	GLfloat mat_shininess[] = { 30.0 };
	GLfloat light_Front[] = { 10.0, 10.0, 10.0, 1.0 };
	GLfloat light_Back[] = { -10.0, -10.0, -10.0, 1.0 };
	GLfloat lightHigh[] = { 1.0, 1.0, 1.0 };
	GLfloat lightLow[] = { 0.5, 0.5, 0.5 };
	GLfloat lmodel_ambient[] = { 0.1, 0.1, 0.1, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	if(frontLighting_)
	{
		glLightfv(GL_LIGHT0, GL_POSITION, light_Front);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightHigh );
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightHigh );
		glLightfv(GL_LIGHT1, GL_POSITION, light_Back);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightLow );
		glLightfv(GL_LIGHT1, GL_SPECULAR, lightLow );
	}
	else
	{
		glLightfv(GL_LIGHT0, GL_POSITION, light_Front);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightLow );
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightLow );
		glLightfv(GL_LIGHT1, GL_POSITION, light_Back);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightHigh );
		glLightfv(GL_LIGHT1, GL_SPECULAR, lightHigh );
	}
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	if(drawAxis_)
	{
		// draw x axis
		glLineWidth( 2.0 );
		glBegin( GL_LINES );
		qglColor( red );
		glVertex3f( -1.0, -1.0, -1.0 );    glVertex3f( 1.5, -1.0, -1.0 );
		glVertex3f( 1.4, -1.1, -1.0 );    glVertex3f( 1.5, -1.2, -1.0 );
		glVertex3f( 1.5, -1.1, -1.0 );    glVertex3f( 1.4, -1.2, -1.0 );
		glVertex3f( 1.35, -1.22, -1.0 );    glVertex3f( 1.55, -1.22, -1.0 );
		glEnd();
		glPushMatrix();
		glRotatef(90, 0.0, 1.0, 0.0);
		glTranslatef(1.0, -1.0, 1.4);
		glCallList(cone_);
		glPopMatrix();

		// draw y axis
		glBegin( GL_LINES );
		qglColor( green );
		glVertex3f( -1.0, -1.0, -1.0 );    glVertex3f( -1.0, 1.5, -1.0 );
		glVertex3f( -1.2, 1.5, -1.0 );    glVertex3f( -1.15, 1.45, -1.0 );
		glVertex3f( -1.1, 1.5, -1.0 );    glVertex3f( -1.2, 1.4, -1.0 );
		glVertex3f( -1.05, 1.38, -1.0 );    glVertex3f( -1.25, 1.38, -1.0 );
		glEnd();
		glPushMatrix();
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTranslatef( -1.0, 1.0, 1.4);
		glCallList(cone_);
		glPopMatrix();

		// draw z axis
		glBegin( GL_LINES );
		qglColor( blue );
		glVertex3f( -1.0, -1.0, -1.0 );    glVertex3f( -1.0, -1.0, 1.5 );
		glVertex3f( -1.0, -1.1, 1.5 );    glVertex3f( -1.0, -1.1, 1.4 );
		glVertex3f( -1.0, -1.1, 1.4 );    glVertex3f( -1.0, -1.2, 1.5 );
		glVertex3f( -1.0, -1.2, 1.5 );    glVertex3f( -1.0, -1.2, 1.4 );
		glVertex3f( -1.0, -1.22, 1.55 );    glVertex3f( -1.0, -1.22, 1.35 );
		glEnd();
		glPushMatrix();
		glTranslatef(-1.0, -1.0, 1.4);
		glCallList(cone_);
		glPopMatrix();
	}

	glLineWidth( 1.0 );

	GLfloat rp = (GLfloat)pointingColor_.red()/255.;
 	GLfloat gp = (GLfloat)pointingColor_.green()/255.;
 	GLfloat bp = (GLfloat)pointingColor_.blue()/255.;

	GLfloat r = (GLfloat)defaultColor_.red()/255.;
 	GLfloat g = (GLfloat)defaultColor_.green()/255.;
 	GLfloat b = (GLfloat)defaultColor_.blue()/255.;

	int i, size = (int)ballVec_.size();

	glColor3f(r, g, b);
	for(i=0; i<selIni_; ++i)
	{
		glPushMatrix();
		if(ballVec_[i])
			glCallList(ballVec_[i]);
		glPopMatrix();
	}

	glColor3f(rp, gp, bp);
	for(i=selIni_; i<size; ++i)
	{
		glPushMatrix();
		if(ballVec_[i])
			glCallList(ballVec_[i]);
		glPopMatrix();
	}

	glFlush();
	glDisable(GL_LIGHTING);
}

void TeQtGLWidget::initializeGL()
{
     makeObject();		// Generate an OpenGL display list
}

void TeQtGLWidget::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
}

void TeQtGLWidget::makeObject()
{
	deleteGLObject();

	ballVec_.clear();
	ballMap_.clear();
	getValues();
	draw2DPixmaps();

    GLUquadricObj* qobj = gluNewQuadric();
	//gluQuadricDrawStyle(qobj, GLU_FILL);

	double	x, y, z;
	double w = (max_ - min_) / 2.;
	double wm = min_ + w;
	int i, size = (int)xVec_.size();

	for(i=0; i<size; ++i)
	{
		string id = idVec_[i];
		int k = glGenLists( 1 );
		ballVec_.push_back(k);
		ballMap_[id] = k;

		glNewList( k, GL_COMPILE );

		x = xVec_[i];
		y = yVec_[i];
		z = zVec_[i];

		glBegin( GL_COMPILE);
		x = (x-wm)/w;
		y = (y-wm)/w;
		z = (z-wm)/w;
		glPushMatrix();
		glTranslatef (x, y, z);
		gluSphere(qobj, /* radius */ .02, /* slices */ quality_, /* stacks */ quality_);
		glPopMatrix();
		glEnd();
		glEndList();
	}

	cone_ = glGenLists( 1 );
	glNewList( cone_, GL_COMPILE );
	glBegin( GL_COMPILE);
	glPushMatrix();
	gluCylinder(qobj, .035, 0, .15, 12, 12);
	glPopMatrix();
	glEnd();
	glEndList();

	gluDeleteQuadric(qobj);

	paint2D();
}

void TeQtGLWidget::getValues()
{
	idVec_.clear();

	xVec_.clear();
	yVec_.clear();
	zVec_.clear();

	TeDatabasePortal *portal = db_->getPortal();
	double	a, b, c;
	string CT, CA, from;

	if(theme_->type() == TeTHEME)
	{
		CT = theme_->collectionTable();
		CA = theme_->collectionAuxTable();
		from = theme_->sqlGridFrom();
	}
	else if(theme_->type() == TeEXTERNALTHEME)
	{
		CT = ((TeExternalTheme*)theme_)->getRemoteTheme()->collectionTable();
		CA = ((TeExternalTheme*)theme_)->getRemoteTheme()->collectionAuxTable();
		from = ((TeExternalTheme*)theme_)->getRemoteTheme()->sqlGridFrom();
	}

	string id;
	string atr;
	if(function_ != "NONE")
	{
		id = CT + ".c_object_id";
		atr = "MIN(" + id + "), " + function_ + "(" + xField_ + "), " + function_ + "(" + yField_ + "), " + function_ + "(" + zField_ + ")";
	}
	else
	{
		id = CA + ".unique_id";
		atr = id + ", " + xField_ + ", " + yField_ + ", " + zField_;
	}

	if(theme_->type() == TeTHEME)
	{
		string query = "SELECT " + atr + from;

		if(function_ == "NONE")
		{
			if(selectedObjects_ == TeAll)
			{
				query += " WHERE (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2)))";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				query += " WHERE (grid_status = 2  OR (grid_status is null AND c_object_status = 2))";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				query += " WHERE (grid_status = 0 OR (grid_status is null AND c_object_status = 0))";
			}
			else if(selectedObjects_ == TeGrouped)
				query += " WHERE ((c_legend_id <> 0) AND (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2))))";
			else if(selectedObjects_ == TeNotGrouped)
				query += " WHERE ((c_legend_id = 0) AND (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2))))";
		}
		else
		{
			if(selectedObjects_ == TeAll)
			{
				query += " WHERE (c_object_status = 0 OR c_object_status = 2)";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				query += " WHERE (c_object_status = 2)";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				query += " WHERE (c_object_status = 0)";
			}
			else if(selectedObjects_ == TeGrouped)
				query += " WHERE ((c_legend_id <> 0) AND (c_object_status = 0 OR c_object_status = 2))";
			else if(selectedObjects_ == TeNotGrouped)
				query += " WHERE ((c_legend_id = 0) AND (c_object_status = 0 OR c_object_status = 2))";
		}

		if(function_ != "NONE")
			query += " GROUP BY " + CT + ".c_object_id";
		if(portal->query(query) == false)
		{
			delete portal;
			return;
		}

		min_ = TeMAXFLOAT;
		max_ = -TeMAXFLOAT;
		string id, sa, sb, sc;
		while(portal->fetchRow())
		{
			id = portal->getData(0);
			sa = portal->getData(1);
			sb = portal->getData(2);
			sc = portal->getData(3);
			if(sa.empty()==false && sb.empty()==false && sc.empty()==false)
			{
				a = atof(sa.c_str());
				b = atof(sb.c_str());
				c = atof(sc.c_str());
				xVec_.push_back(a);
				yVec_.push_back(b);
				zVec_.push_back(c);
				min_ = MIN(min_, a);
				min_ = MIN(min_, b);
				min_ = MIN(min_, c);
				max_ = MAX(max_, a);
				max_ = MAX(max_, b);
				max_ = MAX(max_, c);
				idVec_.push_back(id);
			}
		}
		selIni_ = (int)xVec_.size();

		query = "SELECT " + atr + from;
		if(function_ == "NONE")
		{
			if(selectedObjects_ == TeAll)
			{
				query += " WHERE (grid_status = 1 OR grid_status = 3";
				query += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3)))";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				query += " WHERE (grid_status = 3  OR (grid_status is null AND c_object_status = 3))";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				query += " WHERE ((grid_status = 1  OR (grid_status is null AND c_object_status = 1)))";
			}
			else if(selectedObjects_ == TeGrouped)
			{
				query += " WHERE ((c_legend_id <> 0) AND (grid_status = 1 OR grid_status = 3";
				query += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3))))";
			}
			else if(selectedObjects_ == TeNotGrouped)
			{
				query += " WHERE ((c_legend_id = 0) AND (grid_status = 1 OR grid_status = 3";
				query += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3))))";
			}
		}
		else
		{
			if(selectedObjects_ == TeAll)
			{
				query += " WHERE (c_object_status = 1 OR c_object_status = 3)";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				query += " WHERE (c_object_status = 3)";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				query += " WHERE (c_object_status = 1)";
			}
			else if(selectedObjects_ == TeGrouped)
			{
				query += " WHERE ((c_legend_id <> 0) AND (c_object_status = 1 OR c_object_status = 3))";
			}
			else if(selectedObjects_ == TeNotGrouped)
			{
				query += " WHERE ((c_legend_id = 0) AND (c_object_status = 1 OR c_object_status = 3))";
			}
		}

		if(function_ != "NONE")
			query += " GROUP BY " + CT + ".c_object_id";

		portal->freeResult();
		if(portal->query(query) == false)
		{
			delete portal;
			return;
		}

		while(portal->fetchRow())
		{
			id = portal->getData(0);
			sa = portal->getData(1);
			sb = portal->getData(2);
			sc = portal->getData(3);
			if(sa.empty()==false && sb.empty()==false && sc.empty()==false)
			{
				a = atof(sa.c_str());
				b = atof(sb.c_str());
				c = atof(sc.c_str());
				xVec_.push_back(a);
				yVec_.push_back(b);
				zVec_.push_back(c);
				min_ = MIN(min_, a);
				min_ = MIN(min_, b);
				min_ = MIN(min_, c);
				max_ = MAX(max_, a);
				max_ = MAX(max_, b);
				max_ = MAX(max_, c);
				idVec_.push_back(id);
			}
		}
	}
	else if(theme_->type() == TeEXTERNALTHEME)
	{
		string query = "SELECT " + atr + from;

		string where;
		if(function_ == "NONE")
		{
			if(selectedObjects_ == TeAll)
			{
				where = " WHERE (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2)))";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				where = " WHERE (grid_status = 2  OR (grid_status is null AND c_object_status = 2))";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				where = " WHERE (grid_status = 0 OR (grid_status is null AND c_object_status = 0))";
			}
			else if(selectedObjects_ == TeGrouped)
				where = " WHERE ((c_legend_id <> 0) AND (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2))))";
			else if(selectedObjects_ == TeNotGrouped)
				where = " WHERE ((c_legend_id = 0) AND (grid_status = 0 OR grid_status = 2 OR (grid_status is null AND (c_object_status = 0 OR c_object_status = 2))))";
		}
		else
		{
			if(selectedObjects_ == TeAll)
			{
				where = " WHERE (c_object_status = 0 OR c_object_status = 2)";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				where = " WHERE (c_object_status = 2)";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				where = " WHERE (c_object_status = 0)";
			}
			else if(selectedObjects_ == TeGrouped)
				where = " WHERE ((c_legend_id <> 0) AND (c_object_status = 0 OR c_object_status = 2))";
			else if(selectedObjects_ == TeNotGrouped)
				where = " WHERE ((c_legend_id = 0) AND (c_object_status = 0 OR c_object_status = 2))";
		}

		vector<string> sv = generateItemsInClauseVec(theme_, where);
		vector<string>::iterator it;

		min_ = TeMAXFLOAT;
		max_ = -TeMAXFLOAT;
		string id, sa, sb, sc;
		for(it=sv.begin(); it!=sv.end(); ++it)
		{
			string sel = query + " WHERE " + CA + ".unique_id IN " + *it;

			if(function_ != "NONE")
				sel += " GROUP BY " + CT + ".c_object_id";

			if(portal->query(sel) == false)
			{
				delete portal;
				return;
			}

			while(portal->fetchRow())
			{
				id = portal->getData(0);
				sa = portal->getData(1);
				sb = portal->getData(2);
				sc = portal->getData(3);
				if(sa.empty()==false && sb.empty()==false && sc.empty()==false)
				{
					a = atof(sa.c_str());
					b = atof(sb.c_str());
					c = atof(sc.c_str());
					xVec_.push_back(a);
					yVec_.push_back(b);
					zVec_.push_back(c);
					min_ = MIN(min_, a);
					min_ = MIN(min_, b);
					min_ = MIN(min_, c);
					max_ = MAX(max_, a);
					max_ = MAX(max_, b);
					max_ = MAX(max_, c);
					idVec_.push_back(id);
				}
			}
			portal->freeResult();
		}

		selIni_ = (int)xVec_.size();

		query = "SELECT " + atr + from;
		if(function_ == "NONE")
		{
			if(selectedObjects_ == TeAll)
			{
				where = " WHERE (grid_status = 1 OR grid_status = 3";
				where += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3)))";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				where = " WHERE (grid_status = 3  OR (grid_status is null AND c_object_status = 3))";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				where = " WHERE ((grid_status = 1  OR (grid_status is null AND c_object_status = 1)))";
			}
			else if(selectedObjects_ == TeGrouped)
			{
				where = " WHERE ((c_legend_id <> 0) AND (grid_status = 1 OR grid_status = 3";
				where += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3))))";
			}
			else if(selectedObjects_ == TeNotGrouped)
			{
				where = " WHERE ((c_legend_id = 0) AND (grid_status = 1 OR grid_status = 3";
				where += " OR (grid_status is null AND (c_object_status = 1 OR c_object_status = 3))))";
			}
		}
		else
		{
			if(selectedObjects_ == TeAll)
			{
				where = " WHERE (c_object_status = 1 OR c_object_status = 3)";
			}
			else if(selectedObjects_ == TeSelectedByQuery)
			{
				where = " WHERE (c_object_status = 3)";
			}
			else if(selectedObjects_ == TeNotSelectedByQuery)
			{
				where = " WHERE (c_object_status = 1)";
			}
			else if(selectedObjects_ == TeGrouped)
			{
				where = " WHERE ((c_legend_id <> 0) AND (c_object_status = 1 OR c_object_status = 3))";
			}
			else if(selectedObjects_ == TeNotGrouped)
			{
				where = " WHERE ((c_legend_id = 0) AND (c_object_status = 1 OR c_object_status = 3))";
			}
		}
		
		sv = generateItemsInClauseVec(theme_, where);

		for(it=sv.begin(); it!=sv.end(); ++it)
		{
			string sel = query + " WHERE " + CA + ".unique_id IN " + *it;

			if(function_ != "NONE")
				sel += " GROUP BY " + CT + ".c_object_id";

			if(portal->query(sel) == false)
			{
				delete portal;
				return;
			}

			while(portal->fetchRow())
			{
				id = portal->getData(0);
				sa = portal->getData(1);
				sb = portal->getData(2);
				sc = portal->getData(3);
				if(sa.empty()==false && sb.empty()==false && sc.empty()==false)
				{
					a = atof(sa.c_str());
					b = atof(sb.c_str());
					c = atof(sc.c_str());
					xVec_.push_back(a);
					yVec_.push_back(b);
					zVec_.push_back(c);
					min_ = MIN(min_, a);
					min_ = MIN(min_, b);
					min_ = MIN(min_, c);
					max_ = MAX(max_, a);
					max_ = MAX(max_, b);
					max_ = MAX(max_, c);
					idVec_.push_back(id);
				}
			}
			portal->freeResult();
		}
	}
	delete portal;

	min_ = min_ - (max_ - min_) * 0.0001;
	max_ = max_ + (max_ - min_) * 0.0001;

	if(ballVec_.empty() == false)
	{
		ballVec_.clear();
		int i, size = (int)idVec_.size();
		map<string, GLuint>::iterator it;

		for(i=0; i<size; ++i)
		{
			string id = idVec_[i];
			it = ballMap_.find(id);
			if(it != ballMap_.end())
				ballVec_.push_back(it->second);
		}
	}
}

void TeQtGLWidget::selectFromHexaedro()
{
	double x1, y1, z1, x2, y2, z2;
	getCubeCoordinate(x1, y1, z1, x2, y2, z2);
	string xmin = Te2String(x1, 6);
	string ymin = Te2String(y1, 6);
	string zmin = Te2String(z1, 6);
	string xmax = Te2String(x2, 6);
	string ymax = Te2String(y2, 6);
	string zmax = Te2String(z2, 6);

	string CT, CA, from;

	if(theme_->type() == TeTHEME)
	{
		CT = theme_->collectionTable();
		CA = theme_->collectionAuxTable();
		from = theme_->sqlGridFrom();
	}
	else if(theme_->type() == TeEXTERNALTHEME)
	{
		CT = ((TeExternalTheme*)theme_)->getRemoteTheme()->collectionTable();
		CA = ((TeExternalTheme*)theme_)->getRemoteTheme()->collectionAuxTable();
		from = ((TeExternalTheme*)theme_)->getRemoteTheme()->sqlGridFrom();
	}

	string obj = CT + ".c_object_id";
	string sel, atr, where;
	bool func = false;

	if(function_ != "NONE")
	{
		func = true;
		atr = "MIN(" + obj + ") as objas, " + function_ + "(" + xField_ + ") as xas, ";
		atr += function_ + "(" + yField_ + ") as yas, " + function_ + "(" + zField_ + ") as zas ";
	}
	else
		atr = xField_ + ", " + yField_ + ", " + zField_;


	if(func == false)
	{
		if(selectedObjects_ == TeAll)
			where = " WHERE (grid_status >= 0 OR (grid_status is null AND c_object_status >= 0))";
		else if(selectedObjects_ == TeSelectedByQuery)
			where = " WHERE (grid_status >= 2  OR (grid_status is null AND c_object_status >= 2))";
		else if(selectedObjects_ == TeNotSelectedByQuery)
			where = " WHERE (grid_status <> 2 OR (grid_status is null AND c_object_status <> 2))";
		else if(selectedObjects_ == TeGrouped)
			where = " WHERE ((c_legend_id <> 0) AND (grid_status >= 0 OR (grid_status is null AND c_object_status >= 0)))";
		else if(selectedObjects_ == TeNotGrouped)
			where = " WHERE ((c_legend_id = 0) AND (grid_status >= 0 OR (grid_status is null AND c_object_status >= 0)))";
	}
	else
	{
		if(selectedObjects_ == TeAll)
			where = " WHERE (c_object_status >= 0)";
		else if(selectedObjects_ == TeSelectedByQuery)
			where = " WHERE (c_object_status >= 2)";
		else if(selectedObjects_ == TeNotSelectedByQuery)
			where = " WHERE (c_object_status <> 2)";
		else if(selectedObjects_ == TeGrouped)
			where = " WHERE (c_legend_id <> 0 AND c_object_status >= 0)";
		else if(selectedObjects_ == TeNotGrouped)
			where = " WHERE (c_legend_id = 0 AND c_object_status >= 0)";
	}

	set<string> uniqueIdSet, objectIdSet;
	TeDatabasePortal *portal = db_->getPortal();
	if(theme_->type() == TeTHEME)
	{
		if(func)
		{
			sel = "SELECT objas, xas, yas, zas FROM (SELECT " + atr + from;
			sel += where + " GROUP BY " + CT + ".c_object_id)";
			sel += " WHERE ";
			sel += "xas > " + xmin + " AND ";
			sel += "xas < " + xmax + " AND ";
			sel += "yas > " + ymin + " AND ";
			sel += "yas < " + ymax + " AND ";
			sel += "zas > " + zmin + " AND ";
			sel += "zas < " + zmax;
		}
		else
		{
			sel = "SELECT " + CA + ".unique_id, " + CT + ".c_object_id, " + atr + from;
			sel += where + " AND ";

			sel += xField_ + " > " + xmin + " AND ";
			sel += xField_ + " < " + xmax + " AND ";
			sel += yField_ + " > " + ymin + " AND ";
			sel += yField_ + " < " + ymax + " AND ";
			sel += zField_ + " > " + zmin + " AND ";
			sel += zField_ + " < " + zmax;
		}

		string val;
		if(portal->query(sel))
		{
			while(portal->fetchRow())
			{
				if(func)
						objectIdSet.insert(portal->getData(0));
				else
				{
					val = portal->getData(0);
					if(val.empty())
						objectIdSet.insert(portal->getData(1));
					else
						uniqueIdSet.insert(val);
				}
			}
		}
	}
	else if(theme_->type() == TeEXTERNALTHEME)
	{
		vector<string> sv = generateItemsInClauseVec(theme_, where);
		vector<string>::iterator it;
		string val;

		for(it=sv.begin(); it!=sv.end(); ++it)
		{
			if(func)
			{
				sel = "SELECT objas, xas, yas, zas FROM (SELECT " + atr + from;
				sel += " WHERE " + CA + ".unique_id IN " + *it;
				sel += " GROUP BY " + CT + ".c_object_id)";
				sel += " WHERE ";
				sel += "xas > " + xmin + " AND ";
				sel += "xas < " + xmax + " AND ";
				sel += "yas > " + ymin + " AND ";
				sel += "yas < " + ymax + " AND ";
				sel += "zas > " + zmin + " AND ";
				sel += "zas < " + zmax;
			}
			else
			{
				sel = "SELECT " + CA + ".unique_id, " + CT + ".c_object_id, " + atr + from;
				sel += " WHERE " + CA + ".unique_id IN " + *it;
				sel += " AND ";
				sel += xField_ + " > " + xmin + " AND ";
				sel += xField_ + " < " + xmax + " AND ";
				sel += yField_ + " > " + ymin + " AND ";
				sel += yField_ + " < " + ymax + " AND ";
				sel += zField_ + " > " + zmin + " AND ";
				sel += zField_ + " < " + zmax;
			}

			if(portal->query(sel))
			{
				while(portal->fetchRow())
				{
					if(func)
							objectIdSet.insert(portal->getData(0));
					else
					{
						val = portal->getData(0);
						if(val.empty())
							objectIdSet.insert(portal->getData(1));
						else
							uniqueIdSet.insert(val);
					}
				}
			}
			portal->freeResult();
		}
	}
	delete portal;

	if(addSelection_)
		grid_->putColorOnGrid(uniqueIdSet, objectIdSet, "addPointing");
	else
		grid_->putColorOnGrid(uniqueIdSet, objectIdSet, "newPointing");
}

void TeQtGLWidget::setXRotation( int degrees )
{
	int graus = 360 * degrees / 99;
    xRot_ = (GLfloat)(graus % 360);
    updateGL();
}

void TeQtGLWidget::setYRotation( int degrees )
{
	int graus = 360 * degrees / 99;
    yRot_ = (GLfloat)(graus % 360);
    updateGL();
}

void TeQtGLWidget::setZRotation( int degrees )
{
	int graus = 360 * degrees / 99;
    zRot_ = (GLfloat)(graus % 360);
    updateGL();
}

void TeQtGLWidget::setXOffset( int offset )
{
	xOff_ = 2. * (double)offset / 99. - 1.;
	if((xSize_ + xOff_) > 1.)
	{
		xOff_ = 1. - xSize_;
		int newOffset = TeRound((xOff_ + 1.) * 99. / 2.);
		emit setXOffsetSignal(newOffset);
	}
    updateGL();
	paint2D();
}

void TeQtGLWidget::setYOffset( int offset )
{
	yOff_ = 2. * (double)offset / 99. - 1.;
	if((ySize_ + yOff_) > 1.)
	{
		yOff_ = 1. - ySize_;
		int newOffset = TeRound((yOff_ + 1.) * 99. / 2.);
		emit setYOffsetSignal(newOffset);
	}
    updateGL();
	paint2D();
}

void TeQtGLWidget::setZOffset( int offset )
{
	zOff_ = 2. * (double)offset / 99. - 1.;
	if((zSize_ + zOff_) > 1.)
	{
		zOff_ = 1. - zSize_;
		int newOffset = TeRound((zOff_ + 1.) * 99. / 2.);
		emit setZOffsetSignal(newOffset);
	}
    updateGL();
	paint2D();
}


void TeQtGLWidget::setXSize( int size )
{
	xSize_ = 2. * (double)size / 99.;
	if((xSize_ + xOff_) > 1.)
	{
		xSize_ = 1. - xOff_;
		int newSize = TeRound(xSize_ * 99. / 2.);
		emit setXSizeSignal(newSize);
	}
    updateGL();
	paint2D();
}

void TeQtGLWidget::setYSize( int size )
{
	ySize_ = 2. * (double)size / 99.;
	if((ySize_ + yOff_) > 1.)
	{
		ySize_ = 1. - yOff_;
		int newSize = TeRound(ySize_ * 99. / 2.);
		emit setYSizeSignal(newSize);
	}
    updateGL();
	paint2D();
}

void TeQtGLWidget::setZSize( int size )
{
	zSize_ = 2. * (double)size / 99.;
	if((zSize_ + zOff_) > 1.)
	{
		zSize_ = 1. - zOff_;
		int newSize = TeRound(zSize_ * 99. / 2.);
		emit setZSizeSignal(newSize);
	}
    updateGL();
	paint2D();
}

void TeQtGLWidget::setViewAngle( int a )
{
	viewAngle_ = 120. - (double)a;
    updateGL();
}

void TeQtGLWidget::setLightingMode( int )
{
	frontLighting_ = !frontLighting_;
    updateGL();
}

void TeQtGLWidget::mousePressEvent(QMouseEvent* e)
{
	mousePosition_ = e->pos();
}

void TeQtGLWidget::mouseMoveEvent(QMouseEvent* e)
{
	if(e->state() == Qt::NoButton)
		return;

	QPoint p = e->pos();
	double w = (double)width();
	double h = (double)height();
	double dx = (double)(p.x() - mousePosition_.x());
	double dy = (double)(mousePosition_.y() - p.y());

	if(fabs(dx) <= 3.)
		dx = 0.;
	if(fabs(dy) <= 3.)
		dy = 0.;
	if(dx == 0. && dy == 0.)
		return;

	dx = dx * viewAngle_ / 40.;
	dy = dy * viewAngle_ / 40.;

	if(keyPress_ == 0)
	{
		if(e->state() == Qt::LeftButton)  // translation
		{
			xTranslation_ += 2. * (dx / w);
			yTranslation_ += 2. * (dy / h);
		}
		else if(e->state() == (Qt::LeftButton | Qt::AltButton))// x and y and z rotation
		{
			xRot_ += dx;
			if(xRot_ > 360.)
				xRot_ = xRot_ - 360.;
			else if(xRot_ < 0.)
				xRot_ = 360. + xRot_;
			emit setXRotationSignal(int(xRot_));
	
			yRot_ += dy;
			if(yRot_ > 360.)
				yRot_ = yRot_ - 360.;
			else if(yRot_ < 0.)
				yRot_ = 360. + yRot_;
			emit setYRotationSignal(int(yRot_));

			zRot_ += (dx+dy);
			if(zRot_ > 360.)
				zRot_ = zRot_ - 360.;
			else if(zRot_ < 0.)
				zRot_ = 360. + zRot_;
			emit setZRotationSignal(int(zRot_));
		}
		else if(e->state() == Qt::MidButton) // zoom
		{
			viewAngle_ += dy;
			if(viewAngle_ < 1.)
				viewAngle_ = 1;
			else if(viewAngle_ > 120.)
				viewAngle_ = 120.;

			int a = (int)(120. - viewAngle_);
			emit setZoomSignal(a);
		}
	}
	else if(keyPress_ == Qt::Key_X)
	{
		xRot_ += dy;
		if(xRot_ > 360.)
			xRot_ = xRot_ - 360.;
		else if(xRot_ < 0.)
			xRot_ = 360. + xRot_;
		emit setXRotationSignal(int(xRot_));
	}
	else if(keyPress_ == Qt::Key_Y)
	{
		yRot_ += dy;
		if(yRot_ > 360.)
			yRot_ = yRot_ - 360.;
		else if(yRot_ < 0.)
			yRot_ = 360. + yRot_;
		emit setYRotationSignal(int(yRot_));
	}
	else if(keyPress_ == Qt::Key_Z)
	{
		zRot_ += dy;
		if(zRot_ > 360.)
			zRot_ = zRot_ - 360.;
		else if(zRot_ < 0.)
			zRot_ = 360. + zRot_;
		emit setZRotationSignal(int(zRot_));
	}

	mousePosition_ = p;
    updateGL();
}

void TeQtGLWidget::mouseReleaseEvent(QMouseEvent*)
{
}

void TeQtGLWidget::keyPressEvent(QKeyEvent* e)
{
	keyPress_ = e->key();
	if(keyPress_ == Qt::Key_Shift || keyPress_ == Qt::Key_Control || keyPress_ == Qt::Key_Alt)
		keyPress_ = 0;

	if(keyPress_ == Qt::Key_R)
	{
		viewAngle_ = 60.;
		xTranslation_ = 0.;
		yTranslation_ = 0.;
		zTranslation_ = -3.5;
		xRot_ = yRot_ = zRot_ = 0;
		emit setXRotationSignal(int(xRot_));
		emit setYRotationSignal(int(yRot_));
		emit setZRotationSignal(int(zRot_));
	}
}

void TeQtGLWidget::keyReleaseEvent(QKeyEvent*)
{
	keyPress_ = 0;
}

void TeQtGLWidget::draw2DPixmaps()
{
	QRect r = ((QFrame*)(xyDevice_))->rect();
	xyPixmap_.resize(r.width(), r.height());
	zxPixmap_.resize(r.width(), r.height());
	yzPixmap_.resize(r.width(), r.height());
	xyPixmap_.fill(QColor(Qt::black));
	zxPixmap_.fill(QColor(Qt::black));
	yzPixmap_.fill(QColor(Qt::black));

	QPainter xyPainter(&xyPixmap_);
	QPainter zxPainter(&zxPixmap_);
	QPainter yzPainter(&yzPixmap_);
	QFont font;
	int fsize = 10;
	font.setPointSize(10);
	font.setBold(true);
	xyPainter.setFont(font);
	zxPainter.setFont(font);
	yzPainter.setFont(font);
	QPen pen(QColor(red), 2);

	double width = (double)r.width();
	double w = max_ - min_;

	xyPainter.setPen(pen);
	xyPainter.drawLine(0, (int)(width - 1), (int)(width - 1), (int)(width - 1));
	xyPainter.drawText((int)(width - fsize - 2), (int)(width - 5), "X");
	pen.setColor(QColor(green));
	xyPainter.setPen(pen);
	xyPainter.drawLine(0, 0, 0, (int)(width - 1));
	xyPainter.drawText(4, fsize + 3, "Y");

	pen.setColor(QColor(blue));
	zxPainter.setPen(pen);
	zxPainter.drawLine(0, (int)(width - 1), (int)(width - 1), (int)(width - 1));
	zxPainter.drawText((int)(width - fsize - 2), (int)(width - 5), "Z");
	pen.setColor(QColor(red));
	zxPainter.setPen(pen);
	zxPainter.drawLine(0, 0, 0, (int)(width - 1));
	zxPainter.drawText(4, fsize + 3, "X");

	pen.setColor(QColor(green));
	yzPainter.setPen(pen);
	yzPainter.drawLine(0, (int)(width - 1), (int)(width - 1), (int)(width - 1));
	yzPainter.drawText((int)(width - fsize - 2), (int)(width - 5), "Y");
	pen.setColor(QColor(blue));
	yzPainter.setPen(pen);
	yzPainter.drawLine(0, 0, 0, (int)(width - 1));
	yzPainter.drawText(4, fsize + 3, "Z");

	int i, size = (int)xVec_.size();
	double	v;
	int	x, y, z;
	QPointArray xypa(size);
	QPointArray zxpa(size);
	QPointArray yzpa(size);

	pen.setColor(defaultColor_);
	pen.setWidth(5);
	xyPainter.setPen(pen);
	zxPainter.setPen(pen);
	yzPainter.setPen(pen);
	for(i=0; i<size; ++i)
	{
		v = xVec_[i];
		x = TeRound(width * (v - min_) / w);
		v = yVec_[i];
		y = TeRound(width * (v - min_) / w);
		v = zVec_[i];
		z = TeRound(width * (v - min_) / w);

		// the vertical axis must be inverted
		xypa.setPoint(i, x, (int)(width - y));
		zxpa.setPoint(i, z, (int)(width - x));
		yzpa.setPoint(i, y, (int)(width - z));
	}
	xyPainter.drawPoints(xypa, 0, selIni_);
	zxPainter.drawPoints(zxpa, 0, selIni_);
	yzPainter.drawPoints(yzpa, 0, selIni_);

	pen.setColor(pointingColor_);
	xyPainter.setPen(pen);
	zxPainter.setPen(pen);
	yzPainter.setPen(pen);

	xyPainter.drawPoints(xypa, selIni_);
	zxPainter.drawPoints(zxpa, selIni_);
	yzPainter.drawPoints(yzpa, selIni_);
}

void TeQtGLWidget::paint2D()
{
	QPainter xyPainter(xyDevice_);
	QPainter zxPainter(zxDevice_);
	QPainter yzPainter(yzDevice_);
	QPen pen(QColor(magenta), 1);
	xyPainter.setPen(pen);
	zxPainter.setPen(pen);
	yzPainter.setPen(pen);
	QRect r = xyPainter.viewport();

	double width = (double)r.width();
	double w = max_ - min_;
	double ww = w / 2.;
	double	v;
	int	x1, x2, y1, y2, z1, z2;

	bitBlt(xyDevice_, 0, 0, &xyPixmap_, 0, 0, (int)width, (int)width, Qt::CopyROP);
	bitBlt(zxDevice_, 0, 0, &zxPixmap_, 0, 0, (int)width, (int)width, Qt::CopyROP);
	bitBlt(yzDevice_, 0, 0, &yzPixmap_, 0, 0, (int)width, (int)width, Qt::CopyROP);

	v = (xOff_ + 1.) * ww + min_;
	x1 = TeRound(width * (v - min_) / w);
	v = (yOff_ + 1.) * ww + min_;
	y1 = TeRound(width * (v - min_) / w);
	v = (xOff_ + xSize_ + 1.) * ww + min_;
	x2 = TeRound(width * (v - min_) / w);
	v = (yOff_ + ySize_ + 1.) * ww + min_;
	y2 = TeRound(width * (v - min_) / w);
	v = (zOff_ + 1.) * ww + min_;
	z1 = TeRound(width * (v - min_) / w);
	v = (zOff_ + zSize_ + 1.) * ww + min_;
	z2 = TeRound(width * (v - min_) / w);

	// the vertical axis must be inverted
	xyPainter.drawRect(x1, (int)(width - y1 - (y2 - y1)), x2 - x1, y2 - y1);
	zxPainter.drawRect(z1, (int)(width - x1 - (x2 - x1)), z2 - z1, x2 - x1);
	yzPainter.drawRect(y1, (int)(width - z1 - (z2 - z1)), y2 - y1, z2 - z1);

	xyPainter.flush();
	zxPainter.flush();
	yzPainter.flush();
}

void TeQtGLWidget::xOffsetReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::yOffsetReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::zOffsetReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::xSizeReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::ySizeReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::zSizeReleased()
{
	drawSelecteds();
}

void TeQtGLWidget::drawSelecteds()
{
	selectFromHexaedro();
	getValues();
	draw2DPixmaps();
	paint2D();
	updateGL();
}

void TeQtGLWidget::getCubeCoordinate(double& x1, double& y1, double& z1, double& x2, double& y2, double& z2)
{
	double w = max_ - min_;
	double ww = w / 2.;

	x1 = (xOff_ + 1.) * ww + min_;
	y1 = (yOff_ + 1.) * ww + min_;
	z1 = (zOff_ + 1.) * ww + min_;
	x2 = (xOff_ + xSize_ + 1.) * ww + min_;
	y2 = (yOff_ + ySize_ + 1.) * ww + min_;
	z2 = (zOff_ + zSize_ + 1.) * ww + min_;
}

void TeQtGLWidget::paint3D()
{
	updateGL();
}

void TeQtGLWidget::changeFunction(string function)
{
	function_ = function;
	initGLObject();
}

void TeQtGLWidget::changeInputObjects(int v)
{
	selectedObjects_ = v;
	initGLObject();
}

void TeQtGLWidget::changeQuality(int v)
{
	quality_ = TeRound(30. * (double)v / 99.) + 6;
	initGLObject();
}

void TeQtGLWidget::updateSelection()
{
	getValues();
	draw2DPixmaps();
	paint2D();
	updateGL();
	addSelection_ = false;
}

void TeQtGLWidget::initGLObject()
{
	makeObject();
	updateGL();
}

void TeQtGLWidget::deleteGLObject()
{
	int i, size = (int)ballVec_.size();

	makeCurrent();
	for(i=0; i<size; ++i)
		glDeleteLists( ballVec_[i], 1 );
	ballVec_.clear();
	glDeleteLists (cone_, 1);
}
