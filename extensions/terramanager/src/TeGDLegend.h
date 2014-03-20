/** \file TeGDLegend.h
  * \brief A class to draw legends.
  * \author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
  */

#ifndef  __TERRAMANAGER_INTERNAL_GDLEGEND_H
#define  __TERRAMANAGER_INTERNAL_GDLEGEND_H

// TerraManager include files
#include "TerraManagerConfig.h"
#include "TeGDCanvas.h"

// TerraLib include files
#include <TeLegendEntry.h>

// STL include files
#include <vector>

namespace TeMANAGER
{

	/** \class TeGDLegend
	  * \brief A class to draw legends.
	  *
	  *
	  * \sa TeGDCanvas
	  *
	  */
	class TERRAMANAGEREXPORT TeGDLegend
	{
		public:
			
			TeGDLegend(TeGDCanvas* gdCanvas);

			virtual ~TeGDLegend();

			/** \brief Sets the visual used in legend labels.
			*/
			void setTextVisual(TeVisual& vis);

			/** \brief Draws the legend.
			*/
			void draw(std::vector<TeLegendEntryVector>& legends, std::vector<std::string>& legendTitles, const int& width = 150);

		protected:

			TeGDLegend(const TeGDLegend& rhs);

			TeGDLegend& operator=(const TeGDLegend& rhs);

			TeGDCanvas* gdCanvas_;	//!< A pointer to an internal canvas where we will draw the legend.
	};

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_GDLEGEND_H


