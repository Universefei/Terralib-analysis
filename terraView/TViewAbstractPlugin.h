/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

/*! \file TViewAbstractPlugin.h
    \brief Provides a base class for plug-ins.
*/

#ifndef __TERRALIB_INTERNAL_TVIEWABSPLUGIN_H
#define __TERRALIB_INTERNAL_TVIEWABSPLUGIN_H

//TView include files
#include "TViewDefines.h"
#include "PluginParameters.h"
#include <terraViewBase.h>

//Qt include files
#include <qpopupmenu.h>

//Forward declarations
class QTranslator;

/** class TViewAbstractPlugin
 *  \brief Defines an abstract class for terraView plug-ins.
 *	
 *	TerraView plugins must inherit this class.
 *
 * \SA PluginParameters
 */
class TVIEW_DLL TViewAbstractPlugin
{
public:

	/** \name Constructor and destructor methods.
	 *	Methods related to instantiation and deletion.
	 */
	//@{

	/** \brief Contrutor
	 *	\param params Parameters to be used by plug-ins.
	 */
	TViewAbstractPlugin(PluginParameters* params);

	/** \brief Virtual destructor.
	 */
	virtual ~TViewAbstractPlugin();
	//@}

	/** \name Plug-in abstract methods.
	 *	Methods to be implemented by plug-ins. Each plug-in must implement these functions.
	 * 
	 */
	//@{

	/**	\brief Initializer method.
	 * 
	 *	This method will be called on plug-in initialization.
	 */
	virtual void init() = 0;

	/**	\brief Finalizer method.
	 * 
	 *	This method will be called on plug-in finalization.
	 */
	virtual void end() = 0;
	//@}

protected:

	/** \brief Returns the terraView main window.
	 *  \return TerraView main window pointer.
	 */
	TerraViewBase* getMainWindow();

    /** \brief Returns a TerraView menu given a name.
	 *  \return A valid TerraView menu pointer, 0 case not found.
	 */
	QPopupMenu* getMenu(const std::string& menuName);

	/** \brief Returns the TerraView plug-in menu.
	 *  \return TerraView plug-in menu pointer.
	 */
	QPopupMenu* getPluginsMenu(const std::string& plgMenu = "");

	/** \brief Loads the plug-in translation files.
	 */
	void loadTranslationFiles(const QString& filePrefix);

	PluginParameters* params_;	//!< Parameters to be used by plug-ins.

	QTranslator* translator_;
};

#endif //__TERRALIB_INTERNAL_TVIEWABSPLUGIN_H
