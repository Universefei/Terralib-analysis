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

#ifndef  __TERRALIB_INTERNAL_QTSHOWMEDIA_H
#define  __TERRALIB_INTERNAL_QTSHOWMEDIA_H

#ifdef WIN32
#include <windows.h>
#endif

#include "TeQtGrid.h"

#include <urlWindow.h> 
#include <mediaDescription.h> 

#include "TeDatabase.h"
#include "TeDatabaseUtils.h"

#include <TViewDefines.h>

#include <qpopupmenu.h>
#include <qtable.h>

#include <string>

using namespace std;

class TVIEW_DLL TeQtShowMedia : public QTable
{
	Q_OBJECT
	QPopupMenu*			popup_;
	URLWindow*			urlWindow_;
	MediaDescription*	descriptionWindow_;

public:
	int				row_;
	string			id_;
	string			table_;
	TeDatabase*		db_;
	TeLayer*		layer_;
	int				nattrs_;
	bool			cmdLocal_;
	TeQtShowMedia(QWidget* parent = 0, const char* name = 0);

	~TeQtShowMedia();
	void moveDown();
	void init(string objId, TeAppTheme* theme, TeQtGrid* grid);

protected:
	bool eventFilter(QObject*, QEvent*);
#ifdef WIN32
	void getCommand(string keycom, string& cmd, vector<string>& varg);
	void getCommandToQtProcess(string keycom, string& cmd);
	string getSystemKeyValue(HKEY hkey, string key, string value);
	string getString(TCHAR* u, DWORD size);
	void charToUnicode(const char* c, TCHAR* u);
	string QueryKey(HKEY hKey); 
	string getKeyFileFromProgId(string& fileType);
	bool isVistaOperatingSystem();
#endif

	virtual bool	isIPAddress(const std::string &value);

	virtual std::string getCompleteIPAddress(const std::string &value);

	virtual std::string removeURLHead(const std::string &value);

	virtual bool isHTTPS(const std::string &value);

	virtual bool isURLFile(const std::string &value);


public slots:
	void slotpressed(int, int, int, const QPoint&);
	void slotDoubleClicked(int, int, int, const QPoint&);
	void slotShow();
	void slotShowFile(string file);
	void slotInsert(QString& dir);
	void slotInsertURL();
	void slotRemove();
	void slotSetDefault();
	void slotDescription();
};
#endif
