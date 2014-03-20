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
*************************************************************************************/


#include <terraView.h>
#include <qmessagebox.h>
#include <qstring.h>

#include <TeVersion.h>

//! A base class for applications similar to TerraView
TerraView::TerraView(QWidget* parent, const char* name) : TerraViewBase(parent, name)
{
}


void TerraView::about()
{
	QMessageBox::about(this, tr("About TerraView"),
		tr("<h2>TerraView 4.2.2 (19/12/2012)</h2>" )
		+ QString( "<p>" ) + tr( "Product based on the TerraLib" ) + QString( " " ) 
		+ QString( TERRALIB_VERSION ) + QString( "<BR>" )
		+ tr( "Database Model version 4.1.0<BR>" )
		+ tr( "Build using QT" ) + QString( " " ) + QString( QT_VERSION_STR ) 
		+ QString( "<BR>" )
		+ tr( "Spatial Statistics Algorithms - TerraStat<BR>"
		   "<p>Copyright &copy; 2001-2012 INPE, Tecgraf PUC-Rio, and FUNCATE<BR>"
		   "Partner: LESTE/UFMG"
		   "<p>TerraView Page: http://www.dpi.inpe.br/terraview<BR>"
		   "TerraLib Page: http://www.terralib.org" ) );
}

void TerraView::languageChange()
{
	TerraViewBase::languageChange();
	setCaption(appCaption_);
}
