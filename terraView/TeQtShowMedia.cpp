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

#include <TeQtShowMedia.h>
#include <TeAttribute.h>
#include <qfiledialog.h> 
#include <qlineedit.h> 
#include <qstring.h> 
#include <qtextedit.h> 
#include <qmessagebox.h>
#include <qfile.h>
#include <errno.h>
#include <qprocess.h>
#include <qapplication.h>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif


TeQtShowMedia :: TeQtShowMedia(QWidget* parent, const char* name)
	: QTable(2, 2, parent, name)
{
	layer_ = 0;
	db_ = 0;
	cmdLocal_ = false;
	row_ = -1;
	popup_ = new QPopupMenu(this);
	popup_->insertItem(tr("Show Media..."), this, SLOT(slotShow()));
	popup_->insertItem(tr("Insert Media..."), this, SLOT(slotInsert(QString&)));
	popup_->insertItem(tr("Insert URL..."), this, SLOT(slotInsertURL()));
	popup_->insertItem(tr("Remove..."), this, SLOT(slotRemove()));
	popup_->insertItem(tr("Set the Default Media"), this, SLOT(slotSetDefault()));
	popup_->insertItem(tr("Description..."), this, SLOT(slotDescription()));

	connect(this, SIGNAL(pressed(int, int, int, const QPoint&)),
		 this, SLOT(slotpressed(int, int, int, const QPoint&)));

	connect(this, SIGNAL(doubleClicked(int, int, int, const QPoint&)),
		 this, SLOT(slotDoubleClicked(int, int, int, const QPoint&)));
}

TeQtShowMedia :: ~TeQtShowMedia()
{
}

void TeQtShowMedia :: init(string objId, TeAppTheme* theme, TeQtGrid* grid)
{
	setNumRows(0);
	id_ = objId;
	nattrs_ = grid->numCols();
	layer_ = ((TeTheme*)theme->getTheme())->layer();
	table_ = layer_->mediaTable();
	db_ = layer_->database();
	TeDatabasePortal* portal = db_->getPortal();
	row_ = -1;
	
	TeAttributeList	colAttrList;
	db_->getAttributeList(((TeTheme*)theme->getTheme())->collectionTable(), colAttrList);

	horizontalHeader()->setLabel(0, tr("Attribute"));
	horizontalHeader()->setLabel(1, tr("Value"));

	int	i, nMidias = 0;
	if(table_.empty() == false)
	{
		string q = "SELECT COUNT(*) FROM " + table_;
		q += " WHERE object_id = '" + id_ + "'";
		if(portal->query(q))
		{
			if(portal->fetchRow())
				nMidias = atoi(portal->getData(0));
		}
	}

	string sel = ((TeTheme*)theme->getTheme())->sqlJoin() + " WHERE c_object_id = '" + id_ + "'";

	portal->freeResult();
	if(portal->query(sel))
	{
		int ncol = portal->getAttributeList().size() - colAttrList.size() + nMidias;

		setNumRows(ncol);

		if(portal->fetchRow())
		{
			for(i=0; i<grid->numCols(); i++)
			{
				string fname = portal->getAttribute(i).rep_.name_;
				string data = portal->getData(i);
				setText(i, 0, fname.c_str());
				setText(i, 1, data.c_str());
			}
		}

		if(table_.empty() == false)
		{
			string q = "SELECT media_name, media_table, show_default FROM " + table_;
			q += " WHERE object_id = '" + id_ + "'";
			portal->freeResult();
			if(portal->query(q))
			{
				TeDatabasePortal* portal2 = db_->getPortal();
				while(portal->fetchRow())
				{
					string type;
					string mediaName = portal->getData(0);
					string mediaTable = portal->getData(1);
					bool showDefault = atoi(portal->getData(2));

					string q = "SELECT media_type FROM " + mediaTable;
					q += " WHERE media_name = '" + mediaName + "'";
					portal2->freeResult();
					if(portal2->query(q))
					{
						if(portal2->fetchRow())
							type = portal2->getData(0);
					}

					string ss;
					if(showDefault)
						ss = "*media: "; // + type;
					else
						ss = "media: "; // + type;

					setText(i, 0, ss.c_str());
					setText(i, 1, mediaName.c_str());
					i++;
				}
				delete portal2;
			}
		}
		show();
		moveDown();
	}
	delete portal;
}

bool TeQtShowMedia :: eventFilter(QObject* o, QEvent* e)
{
//	if (e->type() == QEvent::MouseButtonDblClick)
//		return true;
	return QTable::eventFilter(o, e);    // standard event processing
}

void TeQtShowMedia :: slotpressed( int row, int /* col */, int button, const QPoint & mousePos )
{
	if(button == RightButton)
	{
		cmdLocal_ = true;
		row_ = row;
		popup_->move(mousePos.x() + x() - contentsX(), mousePos.y() + y() - contentsY());
		popup_->exec();
		cmdLocal_ = false;
	}
}

void TeQtShowMedia :: slotDoubleClicked( int row, int /* col */, int button, const QPoint & /*mousePos */)
{
	if(button == LeftButton)
	{
		row_ = row;
		slotShow();
	}
}

//void TeQtShowMedia :: slotShow()
//{
//	if(row_ > 0 && nattrs_ > row_)
//		return;
//
//	string name;
//	if(row_ >= 0)
//		name = text(row_, 1).latin1();
//
//    size_t  pos;
//    char    buf[300];
//	vector<string>	varg;
//    string  cmd, arg, sname;
//	string	swdir;
//	string	mediaTable;
//	string	type;
//
//// load media data to archive from database
//	TeDatabasePortal *portal = db_->getPortal();
//	string query = "SELECT media_table, media_name FROM " + table_;
//	if(row_ >= 0)
//		query += " WHERE object_id = '" + id_ + "' AND media_name = '" + name + "'";
//	else
//		query += " WHERE object_id = '" + id_ + "' AND show_default = 1";
//	if(portal->query(query))
//	{
//		if(portal->fetchRow())
//		{
//			mediaTable = portal->getData(0);
//			name = portal->getData(1);
//		}
//	}
//
//	if(mediaTable.empty())
//	{
//		QMessageBox::warning(this, tr("Warning"),
//			tr("There is no media to show!\nInsert one first."));
//		delete portal;
//		return;
//	}
//
//	query = "SELECT * FROM " + mediaTable + " WHERE media_name = '" + name + "'";
//	portal->freeResult();
//	if(portal->query(query))
//		if(portal->fetchRow())
//			type = portal->getData("media_type");
//
//	if(type.empty())
//	{
//		QMessageBox::warning(this, tr("Warning"),
//			tr("There is no media in the table!"));
//		delete portal;
//		return;
//	}
//
//	string tempName =  name;
//	if(type != "url")
//	{
//        size_t f;
//#ifdef WIN32
//		char wdir[100];
//		GetWindowsDirectory((LPTSTR)wdir, 100);
//		swdir = wdir;
//		f = swdir.find(":\\");
//		if(f != string::npos)
//			swdir = swdir.substr(0, f);
//		swdir.append(":\\TERRAVIEW_TEMP");
//		if( access (swdir.c_str(), 06) == -1 ) // test if directory exists
//		{
//			if( errno == ENOENT )  // directory does not exist so create it
//			{
//				if ( _mkdir(swdir.c_str()) == -1)
//				{
//                    QString msg = tr("The directory ");
//                    msg += swdir.c_str();
//                    msg += tr(" could not be created");
//            		QMessageBox::critical(this, tr("Error"),
//            		    tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
//					delete portal;
//					return;
//				}
//			}
//		}
//		swdir.append("\\");
//#else
//    	swdir = "/tmp/TERRAVIEW_TEMP";
//		if( access (swdir.c_str(), F_OK) == -1 ) // test if directory exists
//		{
//			if( errno == ENOENT )  // directory does not exist so create it
//			{
//                char buf[20];
//                sprintf(buf, "mkdir %s", swdir.c_str());
//				if (system(buf) != 0)
//				{
//            		QMessageBox::critical(this, tr("Error"),
//            		    tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
//            		delete portal;
//            		return;
//				}
//			}
//		}        
//		swdir.append("/");
//#endif
//		f = tempName.rfind("/");
//		if(f == string::npos)
//			f = tempName.rfind("\\");
//		if(f != string::npos)
//			tempName = tempName.substr(f+1);
//		tempName.insert(0, swdir);
//
//		pos = tempName.rfind(".");
//		if (pos == string::npos)
//		{
//			tempName.append(".");
//			tempName.append(type);
//		}
//		unsigned char *data = 0;
//		long size;
////		string description;
////		string type;
//		if((portal->getBlob("media_blob", data, size)) == false)
//		{
//			delete portal;
//			return;
//		}
//		FILE *fp = fopen(tempName.c_str(), "wb");
//		if(fp == 0)
//		{
//			delete portal;
//			return;
//		}
//		if(fwrite(data, sizeof(unsigned char), size, fp) < (unsigned long)size)
//		{
//			delete portal;
//			return;
//		}
//		if(data)
//			delete []data;
//		fclose(fp);
//
//		pos = tempName.rfind(".");
//		if (pos == string::npos)
//		{
//			delete portal;
//			return;
//		}
//
//		memset (buf, 0, 300);
//		tempName.copy (buf, tempName.size()-pos, pos);
//	}
//	else
//		strcpy(buf, ".html");
//
//	delete portal;
//
//#ifdef WIN32
//	if (TeConvertToUpperCase (buf) == ".EXE")
//	{
//		_spawnl (P_NOWAIT, tempName.c_str(), tempName.c_str(), NULL);
//		return;
//	}
//	// if application is user defined find current application
//	// else use default application
//	string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\";
//	key += buf;
//
//	string keycmd = getSystemKeyValue(HKEY_CURRENT_USER, key, "Application");
//	if(keycmd.empty() == false)
//	{
//		string com = keycmd;
//		key = "Applications\\" + com + "\\SHELL\\PLAY\\COMMAND";
//		keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//		if(keycmd.empty())
//		{
//			key = "Applications\\" + com + "\\SHELL\\OPEN\\COMMAND";
//			keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//			if(keycmd.empty())
//			{
//				key = "Applications\\" + com + "\\SHELL\\EDIT\\COMMAND";
//				keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//			}
//		}
//	}
//	else
//	{
//		key = buf;
//		keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//		if(keycmd.size() <= 1)
//		{
//			key += "\\SHELL\\PLAY\\COMMAND";
//			keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//			if(keycmd.empty())
//			{
//				key = buf;
//				key += "\\SHELL\\OPEN\\COMMAND";
//				keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//				if(keycmd.empty())
//				{
//					key = buf;
//					key += "\\SHELL\\EDIT\\COMMAND";
//					keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//				}
//			}
//		}
//		else
//		{
//			memset (buf, 0, 300);
//			strcpy(buf, keycmd.c_str());
//
//			key = buf;
//			key += "\\SHELL\\PLAY\\COMMAND";
//			keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//			if(keycmd.empty())
//			{
//				key = buf;
//				key += "\\SHELL\\OPEN\\COMMAND";
//				keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//				if(keycmd.empty())
//				{
//					key = buf;
//					key += "\\SHELL\\EDIT\\COMMAND";
//					keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, key, "");
//				}
//			}
//		}
//	}
//
//	if(keycmd.empty())
//	{
//		QMessageBox::warning(this, tr("Warning"),
//			tr("The media cannot be shown!\nCheck if there is a program defined to open it."));
//		return;
//	}
//
//// get command and arguments from keycmd
//    getCommand(keycmd, cmd, varg);
//	if((pos = tempName.find(" ")) != string::npos)
//		tempName = "\"" + tempName + "\"";
//
//	char curdir[256];
//	_getcwd(curdir, 256);
//	_chdir(swdir.c_str());
//
//	if (varg.size() == 0)
//		_spawnl (P_NOWAIT, cmd.c_str(), tempName.c_str(), NULL);
//	else if (varg.size() == 1)
//        _spawnl (P_NOWAIT, cmd.c_str(), varg[0].c_str(), tempName.c_str(), NULL);
//	else if (varg.size() == 2)
//	{
//		const char* p = varg[1].c_str();
//		if(varg[1] == "%L" || varg[1] == "%1")
//			_spawnl (P_NOWAIT, cmd.c_str(), varg[0].c_str(), tempName.c_str(), NULL);
//		else
//			_spawnl (P_NOWAIT, cmd.c_str(), varg[0].c_str(), p, tempName.c_str(), NULL);
//	}
//	else if (varg.size() == 3)
//	{
//		const char* p = varg[1].c_str();
//		const char* p1 = varg[2].c_str();
//		size_t f = varg[0].find("/prefetch:");
//		if(f != string::npos && (varg[2] == "%L" || varg[2] == "%1"))
//			_spawnl (P_NOWAIT, cmd.c_str(), p, tempName.c_str(), NULL);
//		else
//			_spawnl (P_NOWAIT, cmd.c_str(), varg[0].c_str(), p, p1, tempName.c_str(), NULL);
//
//
//	}
//
//	_chdir(curdir);
//#else
//    string command;
//    //Mount the vector of directory paths that are
//    //stored in the PATH environment variable
//    string path =  getenv("PATH");
//    vector<string> dirVector;
//    string dirPath;
//    size_t idx = 0;
//    size_t len = path.find(':', idx);
//    while (len != string::npos)
//    {
//        dirPath = path.substr(idx, len-idx);
//        dirVector.push_back(dirPath);
//        idx = len+1;
//        len = path.find(':', idx);
//    }
//
//
//    // get the last directory path
//    idx = path.rfind(':', path.size());
//    dirPath = path.substr(idx+1, path.size());
//    dirVector.push_back(dirPath);
//       
//	string fs; 
//    if(type != "url")
//	{
//        bool exists = false;
//        for (unsigned int i = 0; i < dirVector.size(); ++i)
//        {
//			QString qfs = dirVector[i].c_str();
//			qfs += "/konqueror";
//            QFile file(qfs);
//            if (file.exists() == true)
//            {
//                exists = true;
//                break;
//            }
//        }
//        if (exists == true)
//            command = "konqueror " + tempName + " &";
//        else
//        {
//			QString msg = tr("The program konqueror used to display your media is not") + "\n";
//			msg += tr("present in your PATH environment variable or KDE is not installed!");
//            QMessageBox::warning(this, tr("Warning"), msg);
//            return;
//        }      
//    }
//    else
//    {
//        unsigned int i;
//        for (i = 0; i < dirVector.size(); ++i)
//        {
//			QString qfs = dirVector[i].c_str();
//			qfs += "/netscape";
//            QFile file(qfs);
//            if (file.exists() == true)
//            {
//                command = "netscape -remote 'openURL(" + tempName + ")' &";              
//                break;
//            }
//        }
//
//        if (command.empty() == true)
//        {
//            for (i = 0; i < dirVector.size(); ++i)
//            {
//				QString qfs = dirVector[i].c_str();
//				qfs += "/mozilla";
//                QFile file(qfs);
//                if (file.exists() == true)
//                {
//                    command = "mozilla -remote 'openURL(" + tempName + ")' &";
//                    break;
//                }
//            }
//        }
//         
//        if (command.empty() == true)
//        {
//            for (i = 0; i < dirVector.size(); ++i)
//            {
//				QString qfs = dirVector[i].c_str();
//				qfs += "/konqueror";
//				QFile file(qfs);
//                if (file.exists() == true)
//                {
//                    command = "konqueror " + tempName + " &";
//                    break;
//                }
//           }
//        }
//        
//        if (command.empty() == true)
//        {
// 			QString msg = tr("The program konqueror used to display your media is not") + "\n";
//			msg += tr("present in your PATH environment variable or KDE is not installed!");
//            QMessageBox::warning(this, tr("Warning"), msg);
//             return;
//        }
//    }
//    
//    system(command.c_str());
//#endif
//}

void TeQtShowMedia :: slotShow()
{
	if(row_ > 0 && nattrs_ > row_)
		return;

	string name;
	if(row_ >= 0)
		name = text(row_, 1).latin1();

    size_t  pos;
    char    buf[300];
	vector<string>	varg;
    string  cmd, arg, sname;
	string	swdir;
	string	mediaTable;
	string	type;

// load media data to archive from database
	TeDatabasePortal *portal = db_->getPortal();
	string query = "SELECT media_table, media_name FROM " + table_;
	if(row_ >= 0)
		query += " WHERE object_id = '" + id_ + "' AND media_name = '" + name + "'";
	else
		query += " WHERE object_id = '" + id_ + "' AND show_default = 1";
	if(portal->query(query))
	{
		if(portal->fetchRow())
		{
			mediaTable = portal->getData(0);
			name = portal->getData(1);
		}
	}

	if(mediaTable.empty())
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("There is no media to show!\nInsert one first."));
		delete portal;
		return;
	}

	query = "SELECT * FROM " + mediaTable + " WHERE media_name = '" + name + "'";
	portal->freeResult();
	if(portal->query(query))
		if(portal->fetchRow())
			type = portal->getData("media_type");

	if(type.empty())
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("There is no media in the table!"));
		delete portal;
		return;
	}

	string tempName =  name;
	if(type != "url")
	{
        size_t f;
#ifdef WIN32
		char wdir[100];
		GetWindowsDirectory((LPTSTR)wdir, 100);
		swdir = wdir;
		f = swdir.find(":\\");
		if(f != string::npos)
			swdir = swdir.substr(0, f);
		swdir.append(":\\TERRAVIEW_TEMP");
		if( access (swdir.c_str(), 06) == -1 ) // test if directory exists
		{
			if( errno == ENOENT )  // directory does not exist so create it
			{
				if ( _mkdir(swdir.c_str()) == -1)
				{
                    QString msg = tr("The directory ");
                    msg += swdir.c_str();
                    msg += tr(" could not be created");
            		QMessageBox::critical(this, tr("Error"),
            		    tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
					delete portal;
					return;
				}
			}
		}
		swdir.append("\\");
#else
    	swdir = "/tmp/TERRAVIEW_TEMP";
		if( access (swdir.c_str(), F_OK) == -1 ) // test if directory exists
		{
			if( errno == ENOENT )  // directory does not exist so create it
			{
                char buf[20];
                sprintf(buf, "mkdir %s", swdir.c_str());
				if (system(buf) != 0)
				{
            		QMessageBox::critical(this, tr("Error"),
            		    tr("The directory /tmp/TERRAVIEW_TEMP could not be created!"));
            		delete portal;
            		return;
				}
			}
		}        
		swdir.append("/");
#endif
		f = tempName.rfind("/");
		if(f == string::npos)
			f = tempName.rfind("\\");
		if(f != string::npos)
			tempName = tempName.substr(f+1);
		tempName.insert(0, swdir);

		pos = tempName.rfind(".");
		if (pos == string::npos)
		{
			tempName.append(".");
			tempName.append(type);
		}
		unsigned char *data = 0;
		long size;
//		string description;
//		string type;
		if((portal->getBlob("media_blob", data, size)) == false)
		{
			delete portal;
			return;
		}
		FILE *fp = fopen(tempName.c_str(), "wb");
		if(fp == 0)
		{
			delete portal;
			return;
		}
		if(fwrite(data, sizeof(unsigned char), size, fp) < (unsigned long)size)
		{
			delete portal;
			return;
		}
		if(data)
			delete []data;
		fclose(fp);

		pos = tempName.rfind(".");
		if (pos == string::npos)
		{
			delete portal;
			return;
		}

		memset (buf, 0, 300);
		tempName.copy (buf, tempName.size()-pos, pos);
	}
	else
		strcpy(buf, ".html");

	delete portal;

#ifdef WIN32

	if(isVistaOperatingSystem() == false)
	{
		//converte ASCII to UNICODE
		int	nextPos=0;

		wchar_t fName[256]; // file name
		wchar_t dName[256]; // dir name
		size_t tt = tempName.rfind("\\");
		if(tt == string::npos)
			tt = tempName.rfind("/");
		if(tt == string::npos)
			tt = 0;
		else nextPos=1;
		int i;
		for(i=0; i<(int)tt; ++i)
			dName[i] = tempName.c_str()[i];
		dName[i] = '\0';
		for(i=(int)tt + nextPos; i<(int)tempName.length(); ++i)
			fName[i-((int)tt+nextPos)] = tempName.c_str()[i];
		fName[i-((int)tt+nextPos)] = '\0';
		
		
		if(this->isIPAddress(tempName))
		{
			std::string completeIP;
			completeIP=this->getCompleteIPAddress(tempName);
			for(i=0;i<(int)completeIP.length();i++)
			{
				fName[i]=completeIP.c_str()[i];
			}
			fName[i]=0;
		}else if( this->isHTTPS(tempName)|| this->isURLFile(tempName) )
		{
			for(i=0;i<(int)tempName.length();i++)
			{
				fName[i]=tempName.c_str()[i];
			}
			fName[i]=0;
		}

		//HINSTANCE hhhh = ShellExecute(NULL, L"open", fName , L"/Play", dName, SW_SHOWNORMAL);
		SHELLEXECUTEINFO shExecInfo;
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = NULL;
		shExecInfo.hwnd = NULL;
		shExecInfo.lpVerb = L"open";
		shExecInfo.lpFile = fName;
		shExecInfo.lpParameters = L"/Open";
		shExecInfo.lpDirectory = dName;
		shExecInfo.nShow = SW_SHOWNORMAL;
		shExecInfo.hInstApp = NULL;

		ShellExecuteEx(&shExecInfo);
	}
	else // windows VISTA
	{
		QProcess proc( this );
		string cmdName;
		if (TeConvertToUpperCase (buf) == ".EXE")
		{
			cmdName = tempName;
			proc.addArgument(cmdName.c_str());
			proc.start();
			return;
		}
		// find user defined current application
		string fileType = buf;
		string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\";

		key += fileType + "\\UserChoice"; // para vista dever ser essa opcao (testar)
		string keyFile = getSystemKeyValue(HKEY_CURRENT_USER, key, "Progid"); // para vista dever ser essa opcao (testar)
		if(keyFile.empty()) // para vista dever ser essa opcao (testar)
			keyFile = getKeyFileFromProgId(fileType); // para vista dever ser essa opcao (testar)

		keyFile += "\\shell\\open\\command";		
		string keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, keyFile, "");
		if(keycmd.size() <= 1)
        {
 			QString msg = tr("Sorry! Problem to display midia.\nPlease verify if exist default program associated with this type of midia.");
            QMessageBox::warning(this, tr("Warning"), msg);
             return;
		}

		getCommandToQtProcess(keycmd, cmd);
		cmdName = cmd;
		size_t ft;
		ft = cmdName.find("%");
		if(ft != string::npos)
			cmdName.replace(ft, 2, tempName);
		else
			cmdName += " " + tempName;

		QString qCmd(cmdName.c_str());
		qCmd = qCmd.simplifyWhiteSpace();
		proc.addArgument(qCmd);
		proc.start();

		//proc.clearArguments();
		//proc.addArgument( "\"C:\\Program Files\\Adobe\\Photoshop Album Starter Edition\\3.2\\Apps\\ComponentLauncher.exe\" module=jpegviewer bg params \"filepath=c:\\TERRAVIEW_TEMP\\winter.jpg\"" );
		////proc.addArgument( "\"C:\\Program Files\\Adobe\\Photoshop Album Starter Edition\\3.2\\Apps\\ComponentLauncher.exe\" module=jpegviewer bg params filepath=c:\\TERRAVIEW_TEMP\\winter.jpg" );
		//proc.start();

	}
  
#else
    string command;
    //Mount the vector of directory paths that are
    //stored in the PATH environment variable
    string path =  getenv("PATH");
    vector<string> dirVector;
    string dirPath;
    size_t idx = 0;
    size_t len = path.find(':', idx);
    while (len != string::npos)
    {
        dirPath = path.substr(idx, len-idx);
        dirVector.push_back(dirPath);
        idx = len+1;
        len = path.find(':', idx);
    }


    // get the last directory path
    idx = path.rfind(':', path.size());
    dirPath = path.substr(idx+1, path.size());
    dirVector.push_back(dirPath);
       
	string fs; 
    if(type != "url")
	{
        bool exists = false;
        for (unsigned int i = 0; i < dirVector.size(); ++i)
        {
			QString qfs = dirVector[i].c_str();
			qfs += "/konqueror";
            QFile file(qfs);
            if (file.exists() == true)
            {
                exists = true;
                break;
            }
        }
        if (exists == true)
            command = "konqueror " + tempName + " &";
        else
        {
			QString msg = tr("The program konqueror used to display your media is not") + "\n";
			msg += tr("present in your PATH environment variable or KDE is not installed!");
            QMessageBox::warning(this, tr("Warning"), msg);
            return;
        }      
    }
    else
    {
        unsigned int i;
        for (i = 0; i < dirVector.size(); ++i)
        {
			QString qfs = dirVector[i].c_str();
			qfs += "/netscape";
            QFile file(qfs);
            if (file.exists() == true)
            {
                command = "netscape -remote 'openURL(" + tempName + ")' &";              
                break;
            }
        }

        if (command.empty() == true)
        {
            for (i = 0; i < dirVector.size(); ++i)
            {
				QString qfs = dirVector[i].c_str();
				qfs += "/mozilla";
                QFile file(qfs);
                if (file.exists() == true)
                {
                    command = "mozilla -remote 'openURL(" + tempName + ")' &";
                    break;
                }
            }
        }
         
        if (command.empty() == true)
        {
            for (i = 0; i < dirVector.size(); ++i)
            {
				QString qfs = dirVector[i].c_str();
				qfs += "/konqueror";
				QFile file(qfs);
                if (file.exists() == true)
                {
                    command = "konqueror " + tempName + " &";
                    break;
                }
           }
        }
        
        if (command.empty() == true)
        {
 			QString msg = tr("The program konqueror used to display your media is not") + "\n";
			msg += tr("present in your PATH environment variable or KDE is not installed!");
            QMessageBox::warning(this, tr("Warning"), msg);
             return;
        }
    }
    
    system(command.c_str());
#endif

}

void TeQtShowMedia :: slotInsert(QString& dir)
{
	if(cmdLocal_ == false)
		hide();
	QFileDialog filed(dir, QString::null, 0, 0, true);
	QString qfile = filed.getOpenFileName (QString::null, QString::null, 0, 0);

	if(qfile.isEmpty())
	{
		QMessageBox::warning(this, tr("Warning"), tr("No media was selected!"));
		show();
		return;
	}

	string file = qfile.latin1();
	dir = TeGetPath(file.c_str()).c_str();

	string type;
	size_t f = file.rfind(".");
	if (f != string::npos)
	{
		type = file;
		type = type.substr(f+1);
	}

	string mTable = "media_table_" + Te2String(layer_->id());
	if(table_.empty())
	{
		string tname = "media_layer_" + Te2String(layer_->id());
		if(createLayerMediaTable(db_, layer_->id(), tname) == false)
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Fail to create the table that will store media names!"));
			return;
		}
		table_ = layer_->mediaTable();
		if(createMediaTable(db_, mTable) == false)
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Fail to create the media table!"));
			return;
		}
	}

	int response = 1;

	TeDatabasePortal *portal = db_->getPortal();
	string sel = "SELECT media_name, object_id FROM "  + table_ + " WHERE media_table = '" + mTable + "'";
	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			string s = portal->getData(0);
			if(s == file)
			{
				response = QMessageBox::question(this, tr("Question"),
				tr("The media already exists! Do you wish to use it even so?"), tr("Yes"), tr("No"));
				if(response == 1)
				{
					QMessageBox::warning(this, tr("Warning"),
						tr("Rename the file and try again!"));
					delete portal;
					return;
				}
				else
				{
					string s = portal->getData(1);
					if(s == id_)
					{
						QMessageBox::warning(this, tr("Warning"),
							tr("There is already a media associated to this object!"));
						delete portal;
						return;
					}
				}
				break;
			}
		}
	}

	if(response == 1)
	{
		if(insertMedia(db_, mTable, file, "", type) == false)
		{
			QString msg = tr("Fail to insert media:") + " ";
			msg += db_->errorMessage().c_str();
			QMessageBox::critical(this, tr("Error"), msg);
			delete portal;
			return;
		}
	}
	if(insertMediaObjLayer(db_, layer_, id_, file, mTable) == false)
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Fail to insert the \"media_name\" field in the table!"));
		delete portal;
		return;
	}

	setNumRows(numRows() + 1);
	setText(numRows()-1, 1, file.c_str());

	int nmidias = 0;
	string conta = "SELECT COUNT(*) FROM "  + table_ + " WHERE object_id = '" + id_ + "'";
	portal->freeResult();
	if(portal->query(conta))
	{
		if(portal->fetchRow())
			nmidias = atoi(portal->getData(0));
	}

	string title = "media: ";

	if(nmidias <= 1)
	{
		title = "*media: "; // + type;
		string up = "UPDATE " + table_ + " SET show_default = 1 WHERE object_id = '" + id_ + "'";
		db_->execute(up);
	}
	delete portal;
	setText(numRows()-1, 0, title.c_str());

	if(cmdLocal_ == false)
		hide();
	else
	{
		moveDown();
		show();
		raise();

	}
}

void TeQtShowMedia :: moveDown()
{
	int x = contentsX();
	int height = visibleHeight(); 
	int nlins = rowAt(height);
	int y = rowPos(numRows()-nlins);
	setContentsPos(x, y);
	adjustColumn(0);
	adjustColumn(1);
}

void TeQtShowMedia :: slotRemove()
{
	if(nattrs_ > row_)
		return;

	int x = contentsX();
	int y = contentsY();

	vector<string> atr;
	vector<string> val;
	int	i;

	for(i=0; i<numRows(); i++)
	{
		string a = text(i, 0).latin1();
		atr.push_back(a);
		string b = text(i, 1).latin1();
		val.push_back(b);
	}

	string	name = val[row_];
	vector<string>::iterator ait = atr.begin();
	ait += row_;
	atr.erase(ait, ait+1);
	vector<string>::iterator vit = val.begin();
	vit += row_;
	val.erase(vit, vit+1);

	setNumRows(0);
	string mTable = "media_table_" + Te2String(layer_->id());
	string remover = "DELETE FROM " + table_ + " WHERE object_id = '" + id_ + "'";
	remover += " AND media_name = '" + name + "'" + " AND media_table = '" + mTable + "'";
	db_->execute(remover);

	bool aindaUsa = false;
	TeDatabasePortal* portal = db_->getPortal();
	string sel = "SELECT * FROM " + table_ + " WHERE media_name = '" + name + "'";
	sel += " AND media_table = '" +  mTable + "'";
	if(portal->query(sel))
		if(portal->fetchRow())
			aindaUsa = true;

	if(aindaUsa == false)
	{
		string remover = "DELETE FROM " + mTable + " WHERE media_name = '" + name + "'";
		db_->execute(remover);
	}

	string defname;
	bool defshow = false;
	sel = "SELECT media_name, show_default FROM " + table_ + " WHERE object_id = '" + id_ + "'";
	portal->freeResult();
	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			defname = portal->getData(0);
			defshow = atoi(portal->getData(1));
			if(defshow)
				break;
		}
	}
	delete portal;

	if(defshow ==  false && defname.empty() == false)
	{
		string up = "UPDATE " + table_ + " SET show_default = 1 WHERE media_name = '" + defname + "'";
		db_->execute(up);
	}

	setNumRows(atr.size());

	unsigned int k;
	for(k=0; k<atr.size(); k++)
	{
		if(defshow ==  false && defname.empty() == false)
		{
			if(val[k] == defname)
				atr[k].insert(0, "*");
		}
		setText(k, 0, atr[k].c_str());
		setText(k, 1, val[k].c_str());
	}
	setContentsPos(x, y);
	raise();
}

void TeQtShowMedia :: slotInsertURL()
{
	if(cmdLocal_ == false)
		hide();

	urlWindow_ = new URLWindow(this, "urlInsertion", true);
	if (urlWindow_->exec() == QDialog::Rejected)
	{
		delete urlWindow_;
		return;
	}
	QString qsite = urlWindow_->URLLineEdit->text();
	delete urlWindow_;
	if(qsite.isNull() || qsite.isEmpty())
		return;

	string site = qsite.latin1();
	string type = "url";


	string mTable = "media_table_" + Te2String(layer_->id());
	if(table_.empty())
	{
		string tname = "media_layer_" + Te2String(layer_->id());
		if(createLayerMediaTable(db_, layer_->id(), tname) == false)
		{
			QMessageBox::critical(this,tr("Error"),
				tr("Fail to create the table to store media names!"));
			return;
		}
		if(createMediaTable(db_, mTable) == false)
		{
			QMessageBox::critical(this, tr("Error"), tr("Fail to create the media tables!"));
			return;
		}
	}


	bool jatem = false;

	TeDatabasePortal *portal = db_->getPortal();
	string sel = "SELECT media_name, object_id FROM "  + table_ + " WHERE media_table = '" + mTable + "'";
	if(portal->query(sel))
	{
		while(portal->fetchRow())
		{
			string s = portal->getData(0);
			if(s == site)
			{
				jatem = true;
				string ss = portal->getData(1);
				if(ss == id_)
				{
					QMessageBox::warning(this, tr("Warning"),
						tr("This URL is already associated to this object!"));
					delete portal;
					return;
				}
				break;
			}
		}
	}

	if(jatem == false)
	{
		if(insertMedia(db_, mTable, site, "", type, false) == false)
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Fail to insert the \"media_blob\" field in the table!"));
			delete portal;
			return;
		}
	}
	if(insertMediaObjLayer(db_, layer_, id_, site, mTable) == false)
	{
		QMessageBox::critical(this, tr("Error"),
			tr("Fail to insert the \"media_name\" field in the table!"));
		delete portal;
		return;
	}

	setNumRows(numRows() + 1);
	setText(numRows()-1, 1, site.c_str());

	int nmidias = 0;
	string conta = "SELECT COUNT(*) FROM "  + table_ + " WHERE object_id = '" + id_ + "'";
	portal->freeResult();
	if(portal->query(conta))
	{
		if(portal->fetchRow())
			nmidias = atoi(portal->getData(0));
	}

	string title;

	if(nmidias <= 1)
	{
		title = "*media: "; // + type;
		string up = "UPDATE " + table_ + " SET show_default = 1 WHERE object_id = '" + id_ + "'";
		db_->execute(up);
	}

	delete portal;
	setText(numRows()-1, 0, title.c_str());

	if(cmdLocal_ == false)
		hide();
	else
	{
		moveDown();
		show();
		raise();
	}
}

void TeQtShowMedia :: slotSetDefault()
{
	if(nattrs_ > row_)
		return;

	string media_name = text(row_, 1).latin1();
	string mTable = "media_table_" + Te2String(layer_->id());
	setDefaultMedia(db_, layer_, id_, media_name, mTable);

	string type = text(row_, 0).latin1();
//	int f = type.find("*", string::npos);
	size_t f = type.find("*");
	if(f != string::npos)
		return;
	else
	{
		int i;
		for(i = nattrs_; i<numRows(); i++)
		{
			string s = text(i, 0).latin1();
			f = s.find("*", 0);
			if(f != string::npos)
			{

				setText(i, 0, "media:");
				break;
			}
		}
		setText(row_, 0, "*media:");
	}

	if(cmdLocal_ == false)
		hide();
	else
		raise();
}

void TeQtShowMedia :: slotDescription()
{
	if(row_ > 0 && nattrs_ > row_)
		return;

	string mTable = "media_table_" + Te2String(layer_->id());
	TeDatabasePortal *ptal = db_->getPortal();
	string name;

	if(row_ < 0)
	{
		if(table_.empty() == false)
		{
			string s = "SELECT media_name FROM " + table_ + " WHERE object_id = '" + id_ + "'";
			s += " AND show_default = 1";
			if(ptal->query(s))
			{
				if(ptal->fetchRow())
					name = ptal->getData(0);
			}
		}
	}
	else
		name = text(row_, 1).latin1();

	string q = "SELECT * FROM " + mTable + " WHERE media_name = '" + name + "'";

	string description = "";
	ptal->freeResult();

	if(db_->tableExist(mTable))
	{
		if(ptal->query(q))
		{
			if(ptal->fetchRow())
				description = ptal->getData("description");
		}
	}
	delete ptal;
	descriptionWindow_ = new MediaDescription(this, "mediaDescription", true);
	QString qdesc;
	qdesc.setLatin1(description.c_str(), description.size());
	descriptionWindow_->descriptionTextEdit->setText(qdesc);
	descriptionWindow_->exec();

	QString qtext = descriptionWindow_->descriptionTextEdit->text();
	if(qtext.isEmpty() || qtext.isNull())
		description.clear();
	else
		description = qtext.latin1();
	if(description.size() > 255)
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("The description is longer than 255 caracteres!\nIt will be truncated."));
		description = description.substr(0, 255);
	}
	if(description.empty() == false)
		updateMediaDescription(db_, name, mTable, description);

	delete descriptionWindow_;

	if(cmdLocal_ == false)
		hide();
	else
		raise();
}

#ifdef WIN32
void TeQtShowMedia :: getCommand(string keycom, string& cmd, vector<string>& varg)
{
    string  s = keycom;
    string  ss = keycom;
    string  S = TeConvertToUpperCase(keycom);
    unsigned int i;
    size_t  pos, n;
	string	arg;
	bool	isDll = false;

	varg.clear();
    cmd.clear();
    arg.clear();

// find command
	if((pos=s.find(".")) != string::npos)
	{
		if((n=s.find(" ", pos)) != string::npos)
		{
			s = s.substr(0, n);
			S = S.substr(0, n);
			ss = ss.substr(n);
		}
	}

// remove ""
	while((pos=s.find("\"")) != string::npos)
	{
		string cs = s, CS = S;
		s = cs.substr(0, pos) + cs.substr(pos+1);
		S = CS.substr(0, pos) + CS.substr(pos+1);
	}
// replace %SystemRoot%
	if((pos=S.find("%SYSTEMROOT%")) != string::npos)
	{
		int len = strlen("%SystemRoot%");
		string systemRoot = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot");
		s.replace(pos, len, systemRoot);
		S.replace(pos, len, systemRoot);
	}

// replace %ProgramFiles%
	if((pos=S.find("%PROGRAMFILES%")) != string::npos)
	{
		int len = strlen("%ProgramFiles%");
		string programFiles = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProgramFilesDir");
		s.replace(pos, len, programFiles);
		S.replace(pos, len, programFiles);
	}

// replace %CommonProgramFiles%
	if((pos=S.find("%COMMOMFILES%")) != string::npos)
	{
		int len = strlen("%CommonFiles%");
		string commonFiles = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "CommonFilesDir");
		s.replace(pos, len, commonFiles);
		S.replace(pos, len, commonFiles);
	}

// rundll32.exe
	if((pos=S.find("RUNDLL32")) == 0)
	{
		string dllDirectory = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"System\\ControlSet001\\Control\\Session Manager\\KnownDLLs", "DllDirectory");
		string  DIR = TeConvertToUpperCase(dllDirectory);
		if((pos=DIR.find("%SYSTEMROOT%")) != string::npos)
		{

			int len = strlen("%SystemRoot%");
			string systemRoot = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot");
			dllDirectory.replace(pos, len, systemRoot);
		}
		cmd = dllDirectory + "\\rundll32.exe";
		isDll = true;
	}
	else
		cmd = s;

	string command = TeConvertToUpperCase(cmd);
	bool percentOut = false;
	if((pos=command.find("MSACCESS.EXE")) != string::npos)

		percentOut = true;
	if((pos=command.find("POWERPNT.EXE")) != string::npos)
		percentOut = true;

	s = ss; // arguments
	if(isDll)
	{
		varg.push_back("rundll32.exe");
		S = TeConvertToUpperCase(s);
		if((pos=S.find("%SYSTEMROOT%")) != string::npos)
		{
			int len = strlen("%SystemRoot%");
			string systemRoot = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot");
			s.replace(pos, len, systemRoot);
		}
		if(s[0] == ' ')
			s = s.substr(1);

		for(i=0; i<s.size(); i++)
			if(s[i] == '/' || s[i] == '-' || s[i]=='%')
				break;
		string arg = s;
		arg = arg.substr(0, i-1);
		varg.push_back(arg);
		s = s.substr(i-1);
	}

// if runddl32 the option has "/" or "-" and termineted with " " 
// else the option has "/", "-" or "%" and termineted with " "
	if(s.find("=") == string::npos)
	{
		for(i=0; i<s.size(); i++)
		{
			if(isDll || percentOut)
			{
				if(s[i] == '/' || s[i] == '-')
				{
					if((pos=s.find(" ", i)) != string::npos)
					{
						string arg = s;

						arg = arg.substr(0, pos);
						arg = arg.substr(i, s.size() - i);
						// remove ""
						while((n=arg.find("\"")) != string::npos)
						{
							string carg = arg;
							arg = carg.substr(0, n) + carg.substr(n+1);
						}

						varg.push_back(arg);
						i = pos;
					}
					else
					{
						string arg = s;
						arg = arg.substr(i);
						// remove ""
						while((n=arg.find("\"")) != string::npos)
						{
							string carg = arg;
							arg = carg.substr(0, n) + carg.substr(n+1);
						}

						varg.push_back(arg);
						break;
					}
				}
			}
			else
			{
				if(s[i] == '/' || s[i] == '-' || s[i] == '%')
				{
					if((pos=s.find(" ", i)) != string::npos)
					{
						string arg = s;
						arg = arg.substr(0, pos);
						arg = arg.substr(i);
						// remove ""
						while((n=arg.find("\"")) != string::npos)
						{
							string carg = arg;
							arg = carg.substr(0, n) + carg.substr(n+1);
						}

						varg.push_back(arg);
						i = pos;
					}
					else
					{
						string arg = s;
						arg = arg.substr(i);
						// remove ""
						while((n=arg.find("\"")) != string::npos)
						{
							string carg = arg;
							arg = carg.substr(0, n) + carg.substr(n+1);
						}

						varg.push_back(arg);
						break;
					}
				}
			}
		}
	}
	else
	{
		int p = s.find("%1");
		if(p != string::npos)
		{
		}
	}
}

void TeQtShowMedia :: getCommandToQtProcess(string keycom, string& cmd)
{
    string  s = keycom;
    string  S = TeConvertToUpperCase(keycom);
    int		len;
    size_t  pos;

    cmd.clear();

// replace %SystemRoot%
	if((pos=S.find("%SYSTEMROOT%")) != string::npos)
	{
		len = strlen("%SystemRoot%");
		string systemRoot = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot");
		s.replace(pos, len, systemRoot);
		S.replace(pos, len, systemRoot);
	}

// replace %ProgramFiles%
	if((pos=S.find("%PROGRAMFILES%")) != string::npos)
	{
		len = strlen("%ProgramFiles%");
		string programFiles = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProgramFilesDir");
		s.replace(pos, len, programFiles);
		S.replace(pos, len, programFiles);
	}

// replace %CommonProgramFiles%
	if((pos=S.find("%COMMOMFILES%")) != string::npos)
	{
		len = strlen("%CommonFiles%");
		string commonFiles = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "CommonFilesDir");
		s.replace(pos, len, commonFiles);
		S.replace(pos, len, commonFiles);
	}

// rundll32.exe
	if((pos=S.find("RUNDLL32")) == 0) // se for no inicio
	{
		string dllDirectory = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"System\\ControlSet001\\Control\\Session Manager\\KnownDLLs", "DllDirectory");
		string  DIR = TeConvertToUpperCase(dllDirectory);
		if((pos=DIR.find("%SYSTEMROOT%")) != string::npos)
		{
			len = strlen("%SystemRoot%");
			string systemRoot = getSystemKeyValue(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot");
			dllDirectory.replace(pos, len, systemRoot);
		}
		cmd = dllDirectory + "\\rundll32.exe";
		len = strlen("RUNDLL32.EXE");
		s.replace(0, len, cmd);
	}
	cmd = s;
}

string TeQtShowMedia :: getString(TCHAR* u, DWORD size)
{
	int	i;
	char buf[512];

	for(i = 0; i < (int)size; ++i)
		buf[i] = u[i];
	buf[i] = '\0';

	string ret = buf;
	return ret;
}

void TeQtShowMedia :: charToUnicode(const char* c, TCHAR* u)
{
	int i;
	int len = strlen(c);
	for(i=0; i < len; ++i)
		u[i] = c[i];
	u[i] = '\0';
}

string TeQtShowMedia :: QueryKey(HKEY hKey) 
{ 
	string ret;

    TCHAR    achKey[512];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[512] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = 512;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    DWORD i, retCode; 
 
    TCHAR  achValue[512]; 
    DWORD cchValue = 512; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
 
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    
    if (cSubKeys)
    {
		for (i=0; i<cSubKeys; i++) 
		{ 
			cbName = 512;
			retCode = RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime); 
			if (retCode != ERROR_SUCCESS) 
				return ret;
		}
    } 
 
    // Enumerate the key values. 
    if (cValues) 
    {
        cchValue = 512; 
        achValue[0] = '\0'; 
        retCode = RegEnumValue(hKey, cValues-1, achValue, &cchValue, NULL, NULL, NULL, NULL);

        if (retCode == ERROR_SUCCESS ) 
			ret = getString(achValue, cchValue);
    }
	return ret; 
}

string TeQtShowMedia :: getKeyFileFromProgId(string& fileType)
{
	string ret;
	DWORD retCode;
	TCHAR tkey[512];
	string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + fileType + "\\OpenWithProgids";
	charToUnicode(key.c_str(), tkey);

	HKEY hTestKey;

	retCode = RegOpenKeyEx(HKEY_CURRENT_USER, tkey, 0, KEY_READ, &hTestKey);
	if(retCode == ERROR_SUCCESS)
		ret = QueryKey(hTestKey);
	RegCloseKey(hTestKey);
	return ret;
}

string TeQtShowMedia :: getSystemKeyValue(HKEY hkey, string key, string value)
{
	HKEY    hk;
	DWORD	DataSize = 1024;
    DWORD   Type = REG_SZ;
    char    buf[1024];
    string  keycmd;

    if (RegOpenKeyExA(hkey, key.c_str(), 0, KEY_READ, &hk) == ERROR_SUCCESS)
	{
		memset (buf, 0, 1024);
		if (RegQueryValueExA(hk, value.c_str(), NULL, &Type, (LPBYTE)buf, &DataSize) == ERROR_SUCCESS)

			keycmd = buf;
	}
	if(hk)
		RegCloseKey (hk);
	return keycmd;
}

bool TeQtShowMedia :: isVistaOperatingSystem()
{
   OSVERSIONINFO osvi;

   ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   GetVersionEx(&osvi);
   if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
	   return true;

	return false;
}

#endif


bool TeQtShowMedia::isHTTPS(const std::string &value)
{
	size_t			pos;
	std::string		newValue;

	newValue=TeConvertToUpperCase(value);
	pos=newValue.find("HTTPS://");
	if(pos!=std::string::npos)
		return true;
	return false;
}
bool TeQtShowMedia::isURLFile(const std::string &value)
{
	size_t			pos;
	std::string		newValue;

	newValue=TeConvertToUpperCase(value);
	pos=newValue.find("FILE://");
	if(pos!=std::string::npos)
		return true;
	return false;
}

std::string TeQtShowMedia::removeURLHead(const std::string &value)
{
	std::string		newValue;
	std::string		cutedValue;
	size_t			pos;
	basic_string <char>::iterator it;



	newValue=TeConvertToUpperCase(value);
	pos=newValue.find("HTTP://");
	if(pos!=std::string::npos)
	{
		cutedValue=newValue.substr(7,newValue.size() - 7);
		return cutedValue;
	}else
	{
		pos=newValue.find("HTTPS://");
		if(pos!=std::string::npos)
		{
			cutedValue=newValue.substr(8,newValue.size() - 8);
			return cutedValue;
		}
	}
	return newValue;
}

bool	TeQtShowMedia ::isIPAddress(const std::string &value)
{
	unsigned int		i;
	short				count=0;
	char				number[255];
	QString				qValue;
	bool				ok;
	std::string			newValue;
	
	newValue=removeURLHead(value);

	for(i=0;i<newValue.size();i++)
	{
		if(newValue[i] == '.') count++;
	}
	if(count<3)	return false;// um ip no minimo tem que ter 3 "."

	//verifico se antes do primeiro ponto existe  3 caracteres
	// e se estes caracteres são numericos
	for(i=0;(i<newValue.size()) && (i<255);i++)
	{
		if(newValue[i] == '.') break;
		number[i]=newValue[i];
	}
	if(i != 3) return false;
	number[3]=0;
	qValue=QString(number);
	qValue.toInt(&ok);
	return ok;
}

std::string TeQtShowMedia ::getCompleteIPAddress(const std::string &value)
{
	std::string		cplIP;
	
	if(isHTTPS(value))	cplIP="https://" + removeURLHead(value); 
	else				cplIP="http://" + removeURLHead(value); 
	return cplIP;
}

void TeQtShowMedia :: slotShowFile(string file)
{
    size_t			pos;
    string			cmd;

	string tempName = file;

	string type;
	pos = file.rfind(".");
	if (pos != string::npos)
	{
		type = file;
		type = type.substr(pos);
		type = TeConvertToUpperCase(type);
	}

#ifdef WIN32

	if(type != ".URL")
	{
		pos = tempName.find("/");
		while(pos != string::npos)
		{
			tempName.replace(pos, 1, "\\");
			pos = tempName.find("/");
		}
	}
	else
	{
		tempName = tempName.substr(0, tempName.size()-4);
		type = ".HTM";
	}

	if(isVistaOperatingSystem() == false)
	{
		//converte ASCII to UNICODE
		wchar_t fName[256]; // file name
		wchar_t dName[256]; // dir name
		size_t tt = tempName.rfind("\\");
		if(tt == string::npos)
			tt = tempName.rfind("/");
		if(tt == string::npos)
			tt = 0;
		int i;
		for(i=0; i<(int)tt; ++i)
			dName[i] = tempName.c_str()[i];
		dName[i] = '\0';
		for(i=(int)tt+1; i<(int)tempName.length(); ++i)
			fName[i-((int)tt+1)] = tempName.c_str()[i];
		fName[i-((int)tt+1)] = '\0';

		//HINSTANCE hhhh = ShellExecute(NULL, L"open", fName , L"/Play", dName, SW_SHOWNORMAL);
		SHELLEXECUTEINFO shExecInfo;
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = NULL;
		shExecInfo.hwnd = NULL;
		shExecInfo.lpVerb = L"open";
		shExecInfo.lpFile = fName;
		shExecInfo.lpParameters = L"/Open";
		shExecInfo.lpDirectory = dName;
		shExecInfo.nShow = SW_SHOWNORMAL;
		shExecInfo.hInstApp = NULL;

		ShellExecuteEx(&shExecInfo);
	}
	else // windows VISTA
	{
		QProcess proc( this );
		string cmdName;
		if (TeConvertToUpperCase (type) == ".EXE")
		{
			cmdName = tempName;
			proc.addArgument(cmdName.c_str());
			proc.start();
			return;
		}
		// find user defined current application
		string fileType = type;
		string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\";

		key += fileType + "\\UserChoice"; // para vista dever ser essa opcao (testar)
		string keyFile = getSystemKeyValue(HKEY_CURRENT_USER, key, "Progid"); // para vista dever ser essa opcao (testar)
		if(keyFile.empty()) // para vista dever ser essa opcao (testar)
			keyFile = getKeyFileFromProgId(fileType); // para vista dever ser essa opcao (testar)

		keyFile += "\\shell\\open\\command";		
		string keycmd = getSystemKeyValue(HKEY_CLASSES_ROOT, keyFile, "");
		if(keycmd.size() <= 1)
        {
 			QString msg = tr("Sorry! Problem to display midia.\nPlease verify if exist default program associated with this type of midia.");
            QMessageBox::warning(this, tr("Warning"), msg);
             return;
		}

		getCommandToQtProcess(keycmd, cmd);
		cmdName = cmd;
		size_t ft;
		ft = cmdName.find("%");
		if(ft != string::npos)
			cmdName.replace(ft, 2, tempName);
		else
			cmdName += " " + tempName;

		QString qCmd(cmdName.c_str());
		qCmd = qCmd.simplifyWhiteSpace();
		proc.addArgument(qCmd);
		proc.start();

		//proc.clearArguments();
		//proc.addArgument( "\"C:\\Program Files\\Adobe\\Photoshop Album Starter Edition\\3.2\\Apps\\ComponentLauncher.exe\" module=jpegviewer bg params \"filepath=c:\\TERRAVIEW_TEMP\\winter.jpg\"" );
		////proc.addArgument( "\"C:\\Program Files\\Adobe\\Photoshop Album Starter Edition\\3.2\\Apps\\ComponentLauncher.exe\" module=jpegviewer bg params filepath=c:\\TERRAVIEW_TEMP\\winter.jpg" );
		//proc.start();

	}

#else
    string command;
    string name = file;
    //Mount the vector of directory paths that are
    //stored in the PATH environment variable
    string path =  getenv("PATH");
    vector<string> dirVector;
    string dirPath;
    size_t idx = 0;
    size_t len = path.find(':', idx);
    while (len != string::npos)
    {
        dirPath = path.substr(idx, len-idx);
        dirVector.push_back(dirPath);
        idx = len+1;
        len = path.find(':', idx);
    }


    // get the last directory path
    idx = path.rfind(':', path.size());
    dirPath = path.substr(idx+1, path.size());
    dirVector.push_back(dirPath);
       
	string fs; 
    if(type != ".URL")
	{
        bool exists = false;
        for (unsigned int i = 0; i < dirVector.size(); ++i)
        {
			fs = dirVector[i] + "/konqueror";
            QFile file(fs.c_str());
            if (file.exists() == true)
            {
                exists = true;
                break;
            }
        }
        if (exists == true)
            command = "konqueror " + name + " &";
        else
        {
			QString msg = tr("The program konqueror used to display your media is not") + "\n";
			msg += tr("present in your PATH environment variable or KDE is not installed!");
            QMessageBox::warning(this, tr("Warning"), msg);
             return;
        }      
    }
    else
    {
		tempName = tempName.substr(0, tempName.size()-4);
		name = tempName;

        unsigned int i;
        for (i = 0; i < dirVector.size(); ++i)
        {
			QString qfs = dirVector[i].c_str();
			qfs += "/netscape";
            QFile file(qfs);
            if (file.exists() == true)
            {
                command = "netscape -remote 'openURL(" + name + ")' &";              
                break;
            }
        }

        if (command.empty() == true)
        {
            for (i = 0; i < dirVector.size(); ++i)
            {
				QString qfs = dirVector[i].c_str();
				qfs += "/mozilla";
                QFile file(qfs);
                if (file.exists() == true)
                {
                    command = "mozilla -remote 'openURL(" + name + ")' &";
                    break;
                }
            }
        }
         
        if (command.empty() == true)
        {
            for (i = 0; i < dirVector.size(); ++i)
            {
				QString qfs = dirVector[i].c_str();
				qfs += "/konqueror";
				QFile file(qfs);
                if (file.exists() == true)
                {
                    command = "konqueror " + name + " &";
                    break;
                }
           }
        }
        
        if (command.empty() == true)
        {
			QString msg = tr("The program konqueror used to display your media is not") + "\n";
			msg += tr("present in your PATH environment variable or KDE is not installed!");
            QMessageBox::warning(this, tr("Warning"), msg);
             return;
        }
    }
    
    system(command.c_str());
#endif
}
