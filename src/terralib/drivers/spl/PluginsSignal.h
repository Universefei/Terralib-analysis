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
/** \file PluginsSignal.h
  * \brief This file contains a base class in order to send signals in TerraView.
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  * \author Emiliano Castejon <castejon@dpi.inpe.br>
  */

#ifndef  __TERRALIB_INTERNAL_PLUGINSSIGNAL_H
#define  __TERRALIB_INTERNAL_PLUGINSSIGNAL_H

#include "../../kernel/TeCommunicator.h"
#include "PluginsParametersContainer.h"

#include "TeSPLDefines.h"

/** \class PluginsSignal
  * \brief An instance of this class represents
  *        a signal, that can be emitted by someone
  *        using PluginsEmitter singleton.
  *        When sending a signal, whe can
  *        add some data with the signal.
  *        See addData and getData methods.
  *
  * \sa PluginsEmitter
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  * \author Emiliano Castejon <castejon@dpi.inpe.br>
  */
class TLSPL_DLL PluginsSignal
{
    public :

		/** \enum PluginsSignalType
		  * \brief Type of signal.
		  */
		enum PluginsSignalType { S_INACTIVE, S_DRAW_THEME, S_CREATE_THEME_ITEM, S_SET_DATABASE, S_POINT_QUERY, S_SHOW_THEME_POPUP, S_SET_QTTHEMEICON, S_REQUEST_PLUGIN_METADATA };

		/** \enum PluginsSignalDatatype
		  * \brief Type of data informed on a signal.
		  */
		enum PluginsSignalDatatype { D_QT_CANVAS, D_QT_VIEWS, D_ABSTRACT_THEME, D_DATABASE, D_COORD2D, D_QT_THEMEITEM, D_STRING, D_PLUGIN_PARAMS, D_THEMETYPE };

		/** \brief Constructor.
		  * \param signalType Signal type. (Input)
		  */
		PluginsSignal(const PluginsSignalType& signalType);

	public:

		PluginsSignalType signalType_;							//!< Signal type.
		PluginsParametersContainer<PluginsSignalDatatype> signalData_;	//!< Signal data.
};

/** \typedef TeSingleton<TeCommunicator<PluginsSignal> > PluginsEmitter
  * \brief This class is a singleton that can be used to send signals to registered communicator instances.
  */
typedef TeSingleton<TeCommunicator<PluginsSignal> > PluginsEmitter;

#endif	// __TERRALIB_INTERNAL_PLUGINSSIGNAL_H

