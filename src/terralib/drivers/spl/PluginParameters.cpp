#include "PluginParameters.h"

PluginParameters::PluginParameters()
{
  init();
}


PluginParameters::PluginParameters(
  QMainWindow* qtmain_widget_ptr,
  TeProgressBase* teprogressbase_ptr,
  TeQtDatabasesListView* teqtdatabaseslistview_ptr,
  TeQtViewsListView* teqtviewslistview_ptr,
  TeQtGrid* teqtgrid_ptr,
  TeQtCanvas* teqtcanvas_ptr,
  TeDatabase* current_database_ptr,
  TeLayer* current_layer_ptr,
  TeView* current_view_ptr,
  TeAppTheme* current_theme_ptr,
  TeCommunicator<PluginsSignal>* signal_emitter,
  bool use_collection,
  std::string currentPluginName,
  std::string currentUser,
  getCurrentDatabasePtrFPT getCurrentDatabasePtrFP,
  getCurrentLayerPtrFPT getCurrentLayerPtrFP,
  getCurrentViewPtrFPT getCurrentViewPtrFP,
  getCurrentThemeAppPtrFPT getCurrentThemeAppPtrFP,
  setCurrentThemeAppPtrFPT setCurrentThemeAppPtrFP,
  updateTVInterfaceFPT updateTVInterfaceFP,
  loadTranslationFileFPT loadTranslationFileFP,
  updatePluginParametersFPT updatePluginParametersFP,
  plotTVDataFPT plotTVDataFP,
  setPluginMetadataFPT setPluginMetadataFP,
  getCurrentUserPtrFPT getCurrentUserPtrFP
  )
{
  init();

  qtmain_widget_ptr_ = qtmain_widget_ptr;
  teprogressbase_ptr_ = teprogressbase_ptr;
  teqtdatabaseslistview_ptr_ = teqtdatabaseslistview_ptr;
  teqtviewslistview_ptr_ = teqtviewslistview_ptr;
  teqtgrid_ptr_ = teqtgrid_ptr;
  teqtcanvas_ptr_ = teqtcanvas_ptr;
  signal_emitter_ = signal_emitter;
  use_collection_ = use_collection;
  currentPluginName_ = currentPluginName;
  currentUser_ = currentUser;
  
	getCurrentDatabasePtrFP_ = getCurrentDatabasePtrFP;
	getCurrentLayerPtrFP_ = getCurrentLayerPtrFP;
	getCurrentViewPtrFP_ = getCurrentViewPtrFP;
	getCurrentThemeAppPtrFP_ = getCurrentThemeAppPtrFP;
	setCurrentThemeAppPtrFP_ = setCurrentThemeAppPtrFP;
	updateTVInterfaceFP_ = updateTVInterfaceFP;
	plotTVDataFP_ = plotTVDataFP;
	loadTranslationFileFP_ = loadTranslationFileFP;
	setPluginMetadataFP_ = setPluginMetadataFP;
	getCurrentUserPtrFP_ = getCurrentUserPtrFP;
	
	/* The following variables are DEPRECATED */
	
  current_database_ptr_ = current_database_ptr;
  current_layer_ptr_ = current_layer_ptr;
  current_view_ptr_ = current_view_ptr;
  current_theme_ptr_ = current_theme_ptr;
  updatePluginParametersFP_ = updatePluginParametersFP;  
}   
        

PluginParameters::~PluginParameters()
{
}


void PluginParameters::init()
{
  qtmain_widget_ptr_ = 0;
  teprogressbase_ptr_ = 0;
  teqtdatabaseslistview_ptr_ = 0;
  teqtviewslistview_ptr_ = 0;
  teqtgrid_ptr_ = 0;
  teqtcanvas_ptr_ = 0;
  signal_emitter_ = 0;
  
	getCurrentDatabasePtrFP_ = 0;
	getCurrentLayerPtrFP_ = 0;
	getCurrentViewPtrFP_ = 0;
	getCurrentThemeAppPtrFP_ = 0;
	setCurrentThemeAppPtrFP_ = 0;
	updateTVInterfaceFP_ = 0;
	plotTVDataFP_ = 0;
	loadTranslationFileFP_ = 0;
	setPluginMetadataFP_ = 0;
	getCurrentUserPtrFP_ = 0;

	use_collection_ = true;
	currentPluginName_ = "";
	
	/* The following variables are DEPRECATED */
	
  current_database_ptr_ = 0;
  current_layer_ptr_ =0;
  current_view_ptr_ =0;
  current_theme_ptr_ =0;
  updatePluginParametersFP_ = 0;	
}

std::string PluginParameters::getCurrentUserPtr() const
{
  if( getCurrentUserPtrFP_ ) {
    return ( getCurrentUserPtrFP_( *this ) );
  } else {
    return "";
  }
}

TeDatabase* PluginParameters::getCurrentDatabasePtr() const
{
  if( getCurrentDatabasePtrFP_ ) {
    return ( getCurrentDatabasePtrFP_( *this ) );
  } else {
    return 0;
  }
}


TeLayer* PluginParameters::getCurrentLayerPtr() const
{
  if( getCurrentLayerPtrFP_ ) {
    return ( getCurrentLayerPtrFP_( *this ) );
  } else {
    return 0;
  }
}


TeView* PluginParameters::getCurrentViewPtr() const
{
  if( getCurrentViewPtrFP_ ) {
    return ( getCurrentViewPtrFP_( *this ) );
  } else {
    return 0;
  }
}


TeAppTheme* PluginParameters::getCurrentThemeAppPtr() const
{
  if( getCurrentThemeAppPtrFP_ ) {
    return ( getCurrentThemeAppPtrFP_( *this ) );
  } else {
    return 0;
  }
}

void PluginParameters::setCurrentThemeAppPtr(TeAppTheme* appTheme)
{
  if( setCurrentThemeAppPtrFP_ ) {
    ( setCurrentThemeAppPtrFP_( appTheme, *this ) );
  }
}


void PluginParameters::updateTVInterface( const PluginParameters& ) const
{
  if( updateTVInterfaceFP_ ) {
    updateTVInterfaceFP_( *this );
  }
}

void PluginParameters::updateTVInterface() const
{
  if( updateTVInterfaceFP_ ) {
    updateTVInterfaceFP_( *this );
  }
}

void PluginParameters::plotTVData() const
{
  if( plotTVDataFP_ ) {
    plotTVDataFP_( *this );
  }
}

void PluginParameters::setPluginMetadata(const PluginMetadata& pluginMetadata) const
{
  if( setPluginMetadataFP_ ) {
    setPluginMetadataFP_( *this, pluginMetadata );
  }
}

bool PluginParameters::loadTranslationFile( const PluginParameters&,
  const std::string& filename ) const
{
  if( loadTranslationFileFP_ ) {
    return loadTranslationFileFP_( *this, filename );
  } else {
    return false;
  }
}


bool PluginParameters::loadTranslationFile( const std::string& filename ) const
{
  if( loadTranslationFileFP_ ) {
    return loadTranslationFileFP_( *this, filename );
  } else {
    return false;
  }
}


void PluginParameters::updatePluginParameters( PluginParameters& pp ) const
{
  if( updatePluginParametersFP_ ) {
    updatePluginParametersFP_( pp );
  }
}

bool PluginParameters::useCollection() const
{
	return use_collection_;
}

