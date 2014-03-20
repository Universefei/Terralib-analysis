/************************************************************************************
TerraView - visualization and exploration of geographical databases
using TerraLib.
Copyright © 2001-2004 INPE and Tecgraf/PUC-Rio.
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
*************************************************************************************/


#include <terraStat.h>
#include <qmessagebox.h>
#include <skaterGraphWindow.h>
#include <skaterWindow.h>
#include <qaction.h>
#include "bayesWindow.h"

//! A base class for applications TerraCrime
TerraStat::TerraStat(QWidget* parent, const char* name) : TerraViewBase(parent, name)
{
	setCaption( tr( "TerraCrime 2.0 Beta" ) );

	helpAboutAction->setMenuText( tr( "&About TerraCrime..." ) );

	skaterGraph_ = new SkaterGraphWindow();

	analysisSkaterAction = new QAction( this, "analysisSkaterAction" );
	analysisSkaterAction->setToggleAction( FALSE );
	analysisSkaterAction->addTo( analysisMenu );
	analysisSkaterAction->setText( tr( "&Skater..." ) );
	analysisSkaterAction->setEnabled(false);
	connect( analysisSkaterAction, SIGNAL( activated() ), this, SLOT( skaterAction_actived() ) );


	empiricalBayesAction = new QAction( this, "empiricalBayesAction" );
	empiricalBayesAction->addTo( analysisMenu );
	empiricalBayesAction->setText( tr( "Global Empirical Bayes..." ) );
	empiricalBayesAction->setMenuText( tr( "Global Empirical &Bayes..." ) );
	empiricalBayesAction->setEnabled(false);
	connect( empiricalBayesAction, SIGNAL( activated() ), this, SLOT( empiricalBayesAction_activated() ) );
}


void TerraStat::about()
{
	QMessageBox::about(this, tr("About TerraCrime"),
		tr("<h2>TerraCrime 2.0</h2>"
		   "<p>Product based on the TerraLib library version 3.1<BR>"
		   "Copyright &copy; 2003-2004 INPE, LESTE/UFMG"
		   "<p>TerraCrime Page: http://www.est.ufmg.br/leste/terracrime<BR>"
		   "TerraLib Page: http://www.terralib.org"));
}



void TerraStat::skaterAction_actived()
{
	  SkaterWindow skater(this,"Skater","");
	  skater.exec();
}


void TerraStat::empiricalBayesAction_activated()
{
	if (currentDatabase() && currentView() && currentLayer() && currentTheme() )
	{
		BayesWindow *bayes = new BayesWindow();
	  bayes->exec();
		  delete bayes;
	  return;
	}

	QString msg;
	if(currentDatabase() == 0)
		msg = tr("Select a database!");
	else if(currentLayer() == 0)
		msg = tr("Select a layer!");
	else if(currentView() == 0)
		msg = tr("Select a view!");
	else if(currentTheme()  == 0)
		msg = tr("Select a theme!");

	QMessageBox::warning(this, tr("Warning"), msg);
}


void TerraStat::checkWidgetEnabling()
{
	TerraViewBase::checkWidgetEnabling();

	analysisSkaterAction->setEnabled(false);
	empiricalBayesAction->setEnabled(false);

	if ((currentDatabase_ != 0) && (currentView_ != 0))
	{
			analysisSkaterAction->setEnabled(true);
	}
	if ((currentDatabase_ != 0) && (currentTheme_ != 0))
	empiricalBayesAction->setEnabled(true);
}
