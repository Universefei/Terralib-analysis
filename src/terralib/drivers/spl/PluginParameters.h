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

#ifndef PLUGINPARAMETERS_H
  #define PLUGINPARAMETERS_H
  
  #define PLUGINPARAMETERS_VERSION "PLUGINPARAMETERS_VERSION_201010210938" \
    "_TERRALIB_VERSION_" TERRALIB_VERSION 
//    "_QT_VERSION_" QT_VERSION_STR
  
//  #include <TeQtDatabasesListView.h>
//  #include <TeQtViewsListView.h>
//  #include <TeQtGrid.h>
//  #include <TeQtCanvas.h>
//
//  #include <TeDatabase.h>
//  #include <TeLayer.h>
//  #include <TeView.h>
//  #include <TeAppTheme.h>
//  #include <TeProgress.h>
//  #include <TeDatabaseFactory.h>
//  #include <TeVersion.h>
//  #include "PluginsSignal.h"
//  #include <PluginMetadata.h>
//
//  #include <qmainwindow.h>
#include "../../kernel/TeCommunicator.h"
#include "../../kernel/TeVersion.h"
#include "TeSPLDefines.h"

//Qt include files
//#include <qglobal.h>

//STL include files
#include <string>

//Forward declarations
class TeQtDatabasesListView;
class TeQtViewsListView;
class TeQtGrid;
class TeQtCanvas;
class TeDatabase;
class TeLayer;
class TeView;
class TeAppTheme;
class TeProgressBase;
class TeDatabaseFactory;
class TeVersion;
class PluginsSignal;
class PluginMetadata;
class QMainWindow;

  /**
   * @brief A class to deal with plugin parameters.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   */  
  class TLSPL_DLL PluginParameters
  {
    public :
    
      /* DO NOT MOVE THIS FUNCTION FROM THE CLASS BEGINNING -------------- */
      
      /**
       * @brief A function to get the parameters version.
       * @note This must be the first declared method  
       * for the correct pointer alignment when calling methods
       * from plugin parameters with different versions.
       */      
      static std::string getVersion()
      {
        return  PLUGINPARAMETERS_VERSION;
      };
      
      /* DO NOT MOVE THIS FUNCTION FROM THE CLASS BEGINNING -------------- */
      
      /**
       * @name Function pointers type definitions.
       * 
       * @brief Type definitions for pointers to internal TerraView plugin 
       * support functions.
       */      
      
		/**
       * @brief A type definition to a function to get the 
       * current database pointer.
       */ 
	  typedef std::string (*getCurrentUserPtrFPT)( 
        const PluginParameters& ); 

      /**
       * @brief A type definition to a function to get the 
       * current database pointer.
       */ 
      typedef TeDatabase* (*getCurrentDatabasePtrFPT)( 
        const PluginParameters& ); 
      
      /**
       * @brief A type definition to a function to get the 
       * current layer pointer.
       */ 
      typedef TeLayer* (*getCurrentLayerPtrFPT)( 
        const PluginParameters& );          
    
      /**
       * @brief A type definition to a function to get the 
       * current view pointer.
       */ 
      typedef TeView* (*getCurrentViewPtrFPT)( 
        const PluginParameters& );     
        
      /**
       * @brief A type definition to a function to get the 
       * current theme application pointer.
       */ 
      typedef TeAppTheme* (*getCurrentThemeAppPtrFPT)( 
        const PluginParameters& );

	  /**
       * @brief A type definition to a function to get the 
       * current theme application pointer.
       */ 
      typedef void (*setCurrentThemeAppPtrFPT)( 
        TeAppTheme*, const PluginParameters& );
        
      /**
       * @brief A type definition to a function to reload the database 
       * and update the TerraView interface (terraViewBase).
       */ 
      typedef void (*updateTVInterfaceFPT)( 
        const PluginParameters& );

	  /**
       * @brief A type definition to a function to plot data
       * in TerraView interface (terraViewBase).
       */ 
      typedef void (*plotTVDataFPT)( 
        const PluginParameters& );

	  /**
       * @brief A type definition to a function set the plugin metadata
       * in TerraView interface (terraViewBase).
       */ 
	  typedef void (*setPluginMetadataFPT)( 
        const PluginParameters&, const PluginMetadata& );
        
      /**
       * @brief A type definition to a function to load the translation 
       * file dictionary into TerraView translation tables.
       */ 
      typedef bool (*loadTranslationFileFPT)( 
        const PluginParameters&, const std::string& );         

      /**
       * @brief DEPRECATED - A pointer to a function to update the supllied 
       * parameters with the current TerraView interface state.
       *
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE.
       */ 
      typedef void (*updatePluginParametersFPT)( PluginParameters& );
    
      /**
       * @name Object instance pointers
       * 
       * @brief Pointers to internal TerraView object instances.
       */
        
      /**
       * @brief A pointer to the current main widget instance (always valid).
       * @note This is a casted pointer to TerraView main window and can be 
       * used as parent widget for all plugin QT derived windows.
       * @note This pointer is always valid.
       */    
      QMainWindow* qtmain_widget_ptr_;
    
      /**
       * @brief The current progress base active instance pointer ( always 
       * valid ).
       */
      TeProgressBase* teprogressbase_ptr_;
      
      /**
       * @brief The current databases list view pointer ( always valid ).
       */
      TeQtDatabasesListView* teqtdatabaseslistview_ptr_;          

      /**
       * @brief The current views list view pointer ( always valid ).
       */
      TeQtViewsListView* teqtviewslistview_ptr_;          
      
      /**
       * @brief The current grid instance pointer ( always valid ).
       */      
      TeQtGrid* teqtgrid_ptr_;

      /**
       * @brief The current theme application instance pointer ( always 
       * valid ).
       */      
      TeQtCanvas* teqtcanvas_ptr_;      
            
      /**
       * @brief DEPRECATED - The current database instance pointer.
       *
       * @note This pointer will be valid only for modal plugins. For
       * non-modal plugins use the TeQtDatabasesListView pointer to
       * get the current active database.
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - 
       * Use getCurrentDatabasePtr() instead.
       */      
      TeDatabase* current_database_ptr_;
      
      /**
       * @brief DEPRECATED - The current layer instance pointer.
       *
       * @note This pointer will be valid only for modal plugins. For
       * non-modal plugins use the TeQtViewsListView pointer to
       * get the current active layer.
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - 
       * Use getCurrentLayerPtr() instead.       
       */      
      TeLayer* current_layer_ptr_;      

      /**
       * @brief DEPRECATED - The current view instance pointer.
       *
       * @note This pointer will be valid only for modal plugins. For
       * non-modal plugins use the TeQtViewsListView pointer to
       * get the current active view.   
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - 
       * Use getCurrentViewPtr() instead.              
       */      
      TeView* current_view_ptr_;
      
      /**
       * @brief DEPRECATED - The current theme application instance pointer  ( always 
       * valid ).
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - 
       * Use getCurrentThemeAppPtr() instead.        
       */      
      TeAppTheme* current_theme_ptr_;
  
	    /**
       * @brief A pointer to plugins emitter singleton ( always 
       * valid ).
       */      
      TeCommunicator<PluginsSignal>* signal_emitter_;

	    /**
       * @brief If TRUE, the theme must be draw using the collection. If FALSE, 
	   * it will be draw using its only restrictions clauses.
       */
	  bool	use_collection_;

	  /**
       * @brief Stores the name of the plugin that is currently being loaded. Used to request the current plugin information.
       */
	  std::string currentPluginName_;

	  /**
       * @brief Stores the name of the current user. Views must be loaded for this given user.
       */
	  std::string currentUser_;
      
      /**
       * @name Class constructors/destructor.
       * 
       * @brief Class constructors/destructor.
       */      
      
      /**
       * @brief Default constructor.
       */    
      PluginParameters();
      
      /**
       * @brief Alternative constructor.
       * @param qtmain_widget_ptr A pointer to the current main 
       * widget instance.
       * @param teprogressbase_ptr The current progress base 
       * active instance pointer.
       * @param teqtdatabaseslistview_ptr The current databases list 
       * view pointer.
       * @param teqtviewslistview_ptr The current views list view pointer.
       * @param teqtgrid_ptr The current grid instance pointer.
       * @param teqtcanvas_ptr The current theme application instance pointer.
       * @param current_database_ptr The current database instance pointer.
       * @param current_layer_ptr The current layer instance pointer.
       * @param current_view_ptr The current view instance pointer.
       * @param current_theme_ptr The current theme application instance 
       * pointer.
       * @param signal_emitter A pointer to plugins emitter singleton.
	   * @param use_collection If TRUE, use collection to draw theme. Othewise, dont use it.
	   * @param currentPluginName The name of the plugin that is currently being loaded.
	   * @param currentUser The name of the user that is connected. Views must be loaded for this given user.
       * @param getCurrentDatabasePtrFP A pointer to a function to get the 
       * current database pointer.
       * @param getCurrentLayerPtrFP A pointer to a function to get the 
       * current layer pointer.
       * @param getCurrentViewPtrFP A pointer to a function to get the 
       * current view pointer.
       * @param getCurrentThemeAppPtrFP A pointer to a function to get the 
       * current theme application pointer. 
       * @param updateTVInterfaceFP A pointer to a function to reload the database 
       * and update the TerraView interface (terraViewBase).
	   * @param plotTVDataFP A pointer to a function to plot data 
       * in TerraView interface (terraViewBase).
       * @param loadTranslationFileFP A pointer to a function to load the translation 
       * file dictionary into TerraView translation tables.
       * @param updatePluginParametersFP A pointer to a function to 
       * update the supllied  parameters with the current TerraView 
       * interface state.
       */    
      PluginParameters(
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
        );      
        
      /**
       * @brief Default destructor.
       */      
      ~PluginParameters();
      
      /**
       * @name TerraView interaction Methods.
       * 
       * @brief Methods to interact with TerraView main widget.
       */

	   /**
      * @brief Returns the current user.
      * @return The current user or "" if 
      * there are no current user.
      */
	  std::string getCurrentUserPtr() const;   
       
      /**
      * @brief Returns the current database pointer.
      * @return The current database pointer or zero if 
      * there are no current database.
      */
      TeDatabase* getCurrentDatabasePtr() const;    
      
      /**
      * @brief Returns the current layer pointer.
      * @return The current layer pointer or zero if 
      * there are no current layer.
      */
      TeLayer* getCurrentLayerPtr() const;      
      
      /**
      * @brief Returns the current view pointer.
      * @return The current view pointer or zero if 
      * there are no current view.
      */
      TeView* getCurrentViewPtr() const;
      
      /**
      * @brief Returns the current theme application pointer.
      * @return The current theme application pointer or zero if 
      * there are no current theme application.
      */
      TeAppTheme* getCurrentThemeAppPtr() const;

	  /**
      * @brief Returns the current theme application pointer.
      * @return The current theme application pointer or zero if 
      * there are no current theme application.
      */
      void setCurrentThemeAppPtr(TeAppTheme*);
      
      /**
       * @brief DEPRECATED - A function to reload the database and update the 
       * TerraView interface (terraViewBase)  ( always valid ).
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - Use
       * the updateTVInterface() function instead.
       * @param pp Plugin parameters reference.
       */        
      void updateTVInterface( const PluginParameters& pp ) const;
      
      /**
       * @brief A function to reload the database and 
       * update the TerraView interface (terraViewBase).
       * 
       */        
      void updateTVInterface() const;      

	  /**
       * @brief A function to plot data  
       * on TerraView interface (terraViewBase).
       * 
       */        
      void plotTVData() const;

	  /**
      * @brief Used by the plugin to set its metadata to the application.
      * @param pluginMetadata The plugin metadata.
      */
	  void setPluginMetadata(const PluginMetadata& pluginMetadata) const;
      
      /**
       * @brief DEPRECATED - A pointer to a function to load the translation file 
       * dictionary into TerraView translation tables ( always valid ).
       * @param pp Plugin parameters reference.
       * @return true if OK, false on errors.
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE - Use
       * the loadTranslationFile( const std::string& filename ) 
       * function instead.       
       */        
      bool loadTranslationFile( const PluginParameters& pp,
        const std::string& filename ) const; 
      
      /**
       * @brief A pointer to a function to load the translation file 
       * dictionary into TerraView translation tables ( always valid ).
       * @param filename The translation file name.
       * @return true if OK, false on errors.
       */        
      bool loadTranslationFile( const std::string& filename ) const;        

      /**
       * @brief DEPRECATED - A function to update the supllied 
       * parameters with the current TerraView interface state.
       * @param pp Plugin parameters reference.
       * @note DEPRECATED - WILL BE REMOVED IN THE FUTURE.
       */ 
      void updatePluginParameters( PluginParameters& pp ) const; 

	    /**
       * @brief If TRUE, collection must be used to draw the theme. 
       * @return true if collection must be used to draw the collection.
       */  
	  bool useCollection() const;
      
    protected :
    
      /**
       * @name PluginParameters protected data/methods.
       */   
            

		/**
       * @brief A pointer to a function to get the 
       * current user.
       */         
      getCurrentUserPtrFPT getCurrentUserPtrFP_;

      /**
       * @brief A pointer to a function to get the 
       * current database pointer.
       */         
      getCurrentDatabasePtrFPT getCurrentDatabasePtrFP_;
      
      /**
       * @brief A pointer to a function to get the 
       * current layer pointer.
       */       
      getCurrentLayerPtrFPT getCurrentLayerPtrFP_;
      
      /**
       * @brief A pointer to a function to get the 
       * current view pointer.
       */       
      getCurrentViewPtrFPT getCurrentViewPtrFP_;
    
      /**
       * @brief A pointer to a function to get the 
       * current theme application pointer.
       */     
      getCurrentThemeAppPtrFPT getCurrentThemeAppPtrFP_;

	  /**
       * @brief A pointer to a function to get the 
       * current theme application pointer.
       */     
      setCurrentThemeAppPtrFPT setCurrentThemeAppPtrFP_;
      
      /**
       * @brief A pointer to a function to reload the database 
       * and update the TerraView interface (terraViewBase).
       */     
      updateTVInterfaceFPT updateTVInterfaceFP_;
      
      /**
       * @brief A pointer to a function to load the translation 
       * file dictionary into TerraView translation tables.
       */     
      loadTranslationFileFPT loadTranslationFileFP_;   
      
      /**
       * @brief A pointer to a function to update the supllied 
       * parameters with the current TerraView interface state
       */        
      updatePluginParametersFPT updatePluginParametersFP_;

	  /**
       * @brief A pointer to a function to plot data 
       * in Terra View using display window pointer
       */        
      plotTVDataFPT plotTVDataFP_;

	   /**
       * @brief A pointer to a function used bay the plugin to set its metadata to Terraview
       */ 
	  setPluginMetadataFPT setPluginMetadataFP_;
      
      /**
       * @brief Initiate the internal state variables.
       */    
      void init();             
    
  };

#endif
