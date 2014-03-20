#include "TViewAbstractPlugin.h"

//Qt include files
#include <qmenubar.h>
#include <qobject.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <qfileinfo.h>

TViewAbstractPlugin::TViewAbstractPlugin(PluginParameters* params) :
params_(params),
translator_(0)
{
}

TViewAbstractPlugin::~TViewAbstractPlugin()
{
	/*Unload translator*/
	if(translator_)
		qApp->removeTranslator(translator_);

	delete translator_;
}

TerraViewBase* TViewAbstractPlugin::getMainWindow()
{
	return dynamic_cast<TerraViewBase*>(params_->qtmain_widget_ptr_);
}

QPopupMenu* TViewAbstractPlugin::getMenu(const std::string& menuName)
{
    int idx = -1;

    QMainWindow* mainWin = getMainWindow();
    QMenuBar* mnu = mainWin->menuBar();
    
    for(unsigned int i = 0; i < mnu->count() -1; i++)
    {
        idx = mnu->idAt(i);
		if(mnu->text(idx).isEmpty())
			continue;
        if(mnu->text(idx) == QObject::tr(menuName.c_str()))
            break;
    }

    if(idx == -1)
        return 0;

    return mnu->findItem(idx)->popup();
}

QPopupMenu* TViewAbstractPlugin::getPluginsMenu(const std::string& plgMenu)
{
	std::vector<std::string> plgTexts;
	QMainWindow* mainWin = getMainWindow();
    QPopupMenu* insertion_menu = getMenu("&Plugins");

	if(!plgMenu.empty())
	{
		TeSplitString(plgMenu, ".", plgTexts);
		TEAGN_TRUE_OR_THROW( ( plgTexts.size() > 0 ), QObject::tr("Invalid plugin meny entries number") );
		  
		unsigned int pme_index = 1;

		while( pme_index < plgTexts.size() ) {
		  QPopupMenu* target_sub_menu = 0;
		    
		  for( unsigned int insertion_menu_index = 0 ; 
			insertion_menu_index < insertion_menu->count() ;
			++insertion_menu_index ) {
		    
			int menuid = insertion_menu->idAt( insertion_menu_index );
		    
			if( insertion_menu->text( menuid ) == 
				plgTexts[ pme_index ].c_str() ) {
				target_sub_menu = insertion_menu->findItem(menuid)->popup();

				TEAGN_TRUE_OR_THROW( ( target_sub_menu != 0 ), "Popup menu map error" )
		      
			  break; 
			}
		  }
		  
		  if( target_sub_menu == 0 ) {
			int new_insertion_menu_index = insertion_menu->count();
		    
			target_sub_menu = new QPopupMenu( insertion_menu,
			  plgTexts[ pme_index ].c_str() ); 
		   
			insertion_menu->insertItem( plgTexts[ 
			  pme_index ].c_str(), target_sub_menu, -1, 
				new_insertion_menu_index );
		        
			int new_insertion_menu_id = insertion_menu->idAt( 
			  new_insertion_menu_index );
		  }         

		  insertion_menu = target_sub_menu;
		  
		  ++pme_index;
		};        
	}

	return insertion_menu;
}

void TViewAbstractPlugin::loadTranslationFiles(const QString& filePrefix)
{
	QDir dir(qApp->applicationDirPath()+ "\\translation");

	QFileInfo info(dir , filePrefix);

	translator_ = new QTranslator(0);

	translator_->load(info.absFilePath() + QTextCodec::locale(), ".");

	qApp->installTranslator(translator_);
}
