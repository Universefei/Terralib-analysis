/** \file TerraManagerUtils.h
  * \brief This file contains utilities functions used by TerraManager.
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  */

#ifndef  __TERRAMANAGER_INTERNAL_TERRAMANAGERUTILS_H
#define  __TERRAMANAGER_INTERNAL_TERRAMANAGERUTILS_H

// TerraManager include files
#include "TerraManagerConfig.h"
#include "TerraManagerDatatypes.h"

// STL include files
#include <string>

// Forward declarations
class TeDatabase;
class TeLine2D;
class TeCoord2D;

namespace TeMANAGER
{

TERRAMANAGEREXPORT TeDatabase* TeMakeConnection(const TeDBMSType& dbType, const std::string& host,
											    const std::string& user, const std::string& password,
											    const std::string& database, const int& port);

/** \fn unsigned int TeInterpolatePoint(const TeLine2D& l, const double& location, TeCoord2D& cout)
  * \brief Locates the point along the line where the given percentage location occurs.
  * \param location Location must between 0 and 1 => percentage
  * \return Returns the index of the segment that contains this percentage and the exact point where percentage occurs in the line.
  * \note If line size is less than 1 the result is unpredictable.
  */
TERRAMANAGEREXPORT unsigned int TeInterpolatePoint(const TeLine2D& l, const double& location, TeCoord2D& cout);

/** \fn void TeGetAngle(const TeCoord2D& first, const TeCoord2D& last, double& ang)
  * \brief Returns the angle, in radians, that segments first-last makes along x-axis.
  */
TERRAMANAGEREXPORT void TeGetAngle(const TeCoord2D& first, const TeCoord2D& last, double& ang);

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_TERRAMANAGERUTILS_H


