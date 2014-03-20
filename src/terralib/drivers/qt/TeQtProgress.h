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

#ifndef  __TERRALIB_INTERNAL_QTPROGRESS_H
#define  __TERRALIB_INTERNAL_QTPROGRESS_H

#include <qprogressdialog.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qaction.h>
#include "../../kernel/TeProgress.h"
#include <qeventloop.h>

#include "../../kernel/TeDefines.h"

class TL_DLL TeQtProgress : public QProgressDialog, public TeProgressBase
{
	Q_OBJECT

protected:
	int		numberOfCursors_;	//<! Number of cursors that were pushed on application stack.

	void enterEvent ( QEvent * )
	{
		if(QApplication::overrideCursor())
		{
			if(QApplication::overrideCursor()->shape() != Qt::ArrowCursor)
			{
				QApplication::setOverrideCursor( Qt::ArrowCursor );
				numberOfCursors_++;
			}
		}
	}

	void leaveEvent ( QEvent * )
	{
		if(QApplication::overrideCursor())
		{
			if(QApplication::overrideCursor()->shape() == Qt::ArrowCursor)
			{
				QApplication::restoreOverrideCursor();
				numberOfCursors_--;
			}
		}
	}

public:
	TeQtProgress( QWidget * creator = 0, const char * name = 0, bool modal = FALSE, WFlags f = 0)
		: QProgressDialog(creator, name, modal, f)	
	{
		numberOfCursors_ = 0;
	}

	~TeQtProgress() {}

	void reset()
	{
		int i;
		QProgressDialog::reset();
		QProgressDialog::setLabelText("");
		QProgressDialog::setCaption("");

		//Restore all cursors that were set by TeQtProgress.
		for (i=0; i<numberOfCursors_; i++)
		{
			if(QApplication::overrideCursor())
				QApplication::restoreOverrideCursor();
		}
		numberOfCursors_ = 0;
	}

	void setTotalSteps (int steps)
	{
		QProgressDialog::setTotalSteps(steps);
		if(QApplication::overrideCursor())
		{
			if(QApplication::overrideCursor()->shape() != Qt::WaitCursor)
			{
				QApplication::setOverrideCursor( Qt::WaitCursor );
				numberOfCursors_++;
			}
		}
		else
		{
			QApplication::setOverrideCursor( Qt::WaitCursor );
			numberOfCursors_++;
		}
	}

	void setMessage(const string& text)
	{
		QProgressDialog::setLabelText(text.c_str());
	}

	string getMessage()
	{
		return QProgressDialog::labelText().latin1();
	}

	void setProgress(int steps)
	{
		QProgressDialog::setProgress(steps);
	}

	 bool wasCancelled()
	 {
		 hasMouse();
		 return QProgressDialog::wasCancelled();
	 }

	 void setCaption(const string& cap)
	 {
		 QProgressDialog::setCaption(cap.c_str());
	 }

	 void cancel() 
	 {
		 QProgressDialog::cancel();
	 }

};
#endif
