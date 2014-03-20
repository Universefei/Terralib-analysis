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

#ifndef  __TERRALIB_INTERNAL_PLUGINMETADATA_H
#define  __TERRALIB_INTERNAL_PLUGINMETADATA_H

#include <vector>
#include <string>

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
class TLSPL_DLL PluginMetadata
{
public:
	
	std::vector<std::string> _vecPluginNames;
	std::vector<std::string> _vecTranslatedNames;
	std::vector<std::string> _vecHintTexts;

public:

	/** \brief Constructor.
		* \param signalType Signal type. (Input)
		*/
	PluginMetadata();

	virtual ~PluginMetadata();
};

#endif	// __TERRALIB_INTERNAL_PLUGINSSIGNAL_H

