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

#ifndef  __TERRASTAT_H
#define  __TERRASTAT_H

#include "terraViewBase.h"
#include <qpopupmenu.h>

class SkaterGraphWindow;

//! A base class for TerraCrime application
class TerraStat : public TerraViewBase
{
	Q_OBJECT
public:
	TerraStat(QWidget* parent = 0, const char* name = 0);

	~TerraStat () {}

  virtual SkaterGraphWindow * getSkaterGraph() {
    return skaterGraph_;
  }

  QAction* analysisSkaterAction;
  QAction* empiricalBayesAction;


  virtual void checkWidgetEnabling();

  
private:

  SkaterGraphWindow* skaterGraph_;

public slots:

  void skaterAction_actived();
  void empiricalBayesAction_activated();

private slots:
	void about();
};
#endif

