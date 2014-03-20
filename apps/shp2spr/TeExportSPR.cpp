#ifdef WIN32
#pragma warning ( disable: 4786 )
#endif
#include "TeException.h"
#include "TeImportExport.h"
#include "TeSPRFile.h"

#include "TeDataTypes.h"
#include "TeUtils.h"
#include "TeLayer.h"
#include "TeTable.h"
#include "TeGeometry.h"
#include "TeGeometryAlgorithms.h"
#include "TeDatabase.h"

// --- Spring
void TeExportSPRLineSet(TeLayer& layer, TeSPRFile& sprFile, const string& objectCategory, int dType );
void TeExportSPRPointSet(TeLayer& layer, TeSPRFile& sprFile, const string& objectCategory, int dType );
void TeExportSPRTable ( TeLayer& layer, TeSPRFile& tabFile, const string& objectCategory, int dType);

void
TeExportSPR ( TeLayer& layer, const string& sprFileName, const string& objectCategory, int dType)
{
	TeSPRFile sprFile ( sprFileName, "w+" );

	// DTM/Network data can have POINTS 3D or contour line 3D, so the
	// header SAMPLE should be written only once 
	if ( dType == 3 )
	{
		sprFile.writeSampleHeader();
		if ( layer.projection() )
			sprFile.writeProjection ( layer.projection() );
		sprFile.writeBox ( layer.box() );
		sprFile.writeInfoEnd();
	}
	else if ( dType == 4 )
	{
		sprFile.writeNetworkHeader();
		if ( layer.projection() )
			sprFile.writeProjection ( layer.projection() );
		sprFile.writeBox ( layer.box() );
		sprFile.writeInfoEnd();
	}
	// Export linear geometries
	TeExportSPRLineSet( layer, sprFile, objectCategory, dType );

	// Export points
	TeExportSPRPointSet( layer, sprFile, objectCategory, dType );

	// Export table
	if ( dType == 1 || dType == 4 ) 
	{
		string tabFileName = TeGetName ( sprFileName.c_str() ) + "_TAB.spr";
		TeSPRFile tabFile ( tabFileName, "w+" );
		TeExportSPRTable ( layer, tabFile, objectCategory, dType );
	}

}

void
TeExportSPRLineSet ( TeLayer& layer, TeSPRFile& sprFile, const string& objectCategory, int dType )
{
	TeLineSet lines;
	layer.getLines(lines);
	TePolygonSet regions;
	layer.getPolygons(regions);

	TeAttrTableVector atts;
	layer.getAttrTables(atts,TeAttrStatic);
	if (atts.empty())
		return;
	string indexName = atts[0].indexName();

	if (lines.size() == 0 && regions.size() == 0 )
		return;

	if (dType != 3 && dType != 4) 
	{
		sprFile.writeLinesHeader ( );
		if ( layer.projection() )
			sprFile.writeProjection ( layer.projection() );
		sprFile.writeBox ( layer.box() );
		sprFile.writeInfoEnd();
	}

	// STEP 1: export single lines from line set
	unsigned int i;	
	for ( i = 0; i < lines.size(); i++ )
	{
		TeLine2D line = lines[i];
		 
		// if lines are being exported as a sample file...
		if (dType == TeSPRMNT)
		{
			sprFile.writeLine3DHeader();

			// get the quote associated to each line, stored as the
			// object id.
			double quote = 0.0;
			if (!indexName.empty())
				quote = atof( line.objectId().c_str() );
			sprFile.writeSampleQuote( quote );
		}
		// if lines are being exported to the network model
		if (dType == 4)
			sprFile.writeNetworkLineHeader(i+1);

		for (unsigned  int k = 0; k < line.size(); k++ )
		{
			sprFile.writePoint ( line[k] );
		}
		sprFile.writeEnd();
	}
	
	// STEP 2: Export lines from polygon set
	for ( i = 0; i < regions.size(); i++ )
	{
		TePolygon poly = regions[i];

		for (unsigned int k = 0; k < poly.size(); k++ )
		{
			TeLinearRing ring = poly[k]; 
			
			if ( dType == 3 )
			{
				sprFile.writeLine3DHeader();
				double quote = 0.0;
				if (!indexName.empty())
					quote = atof( ring.objectId().c_str() );
				sprFile.writeSampleQuote( quote );
			}
			// if lines are being exported to the network model
			if (dType == 4)
				sprFile.writeNetworkLineHeader(k+1);

			for (unsigned int l = 0 ; l < ring.size(); l++)
				sprFile.writePoint(ring[l]);

			sprFile.writeEnd();
		}
	}

	// check if label points should be written
	if ( dType == 3)
		return;
	if ( (dType == 2) && (indexName.empty())) 
		return;

	// otherwise write the identifiers
	sprFile.writeEnd();	 
	sprFile.writeLabelHeader( dType);
		
	// STEP 3: write identifiers of opened lines
	for ( i = 0; i < lines.size(); i++ )
	{
		TeLine2D line = lines[i];

		TeCoord2D pt = TeFindCentroid( line );

		if (dType == 2 )
			sprFile.writeThematicLabel ( pt.x(), pt.y(), line.objectId());
		else if (dType == 1)
			sprFile.writeCadastralLabel ( line.objectId(), pt.x(), pt.y(), objectCategory);
		else if (dType == 4)
			sprFile.writeNetworkLabel ( line.objectId(), pt.x(), pt.y(), objectCategory);
	}

	// STEP 4: write identifiers of polygons 
	for ( i = 0; i < regions.size(); i++ )
	{
		TePolygon pol = regions[i];

		TeCoord2D pt = TeFindCentroid( pol );

		if (dType == 2 )
			sprFile.writeThematicLabel ( pt.x(), pt.y(), pol.objectId());
		else if (dType == 1)
			sprFile.writeCadastralLabel ( pol.objectId(), pt.x(), pt.y(), objectCategory);
		else if (dType == 4)
			sprFile.writeNetworkLabel ( pol.objectId(), pt.x(), pt.y(), objectCategory);
	}
	sprFile.writeEnd();	 
}

void
TeExportSPRArcSet ( TeLayer& layer, TeSPRFile& sprFile, const string& objectCategory )
{
	// To be written...
	//sprFile.writeArcHeader ( *it );

}

void
TeExportSPRPointSet ( TeLayer& layer, TeSPRFile& sprFile, const string& objectCategory, int dType )
{
	TePointSet points;
	layer.getPoints(points);

	if ( points.size() == 0 ) // layer has points
		return;

	TeAttrTableVector atts;
	layer.getAttrTables(atts,TeAttrStatic);
	if (atts.empty())
		return;
	string indexName = atts[0].indexName();

	if ( dType == 3 )
	{
		sprFile.writePoint3DHeader();
	}
	else
	{
		sprFile.writePointsHeader ( );
		sprFile.writeProjection ( layer.projection() );
		sprFile.writeBox ( layer.box() );
		sprFile.writeInfoEnd();
	}

	// Write the points
	for (unsigned int i = 0; i < points.size(); i++ )
	{
		TePoint pt = points [i];
		TeCoord2D xy = pt.location();

		if ( dType == 3 )
		{
			double quote = 0.0;
			if (!indexName.empty())
				quote = atof( pt.objectId().c_str() );			
			sprFile.writePoint3D( xy.x(), xy.y(), quote );
		}
		else if (dType == 2)
		{
			if (!indexName.empty())
				sprFile.writeThematicLabel ( xy.x(), xy.y(), pt.objectId() );
			else
				sprFile.writeThematicLabel ( xy.x(), xy.y(), "Nenhuma" );
		}
		else if (dType == 1)
			sprFile.writeCadastralLabel ( pt.objectId(), xy.x(), xy.y(), objectCategory);
		else if (dType == 4)
			sprFile.writeNetworkLabel ( pt.objectId(), xy.x(), xy.y(), objectCategory);
	}	
	sprFile.writeEnd();	 
}
 

void
TeExportSPRTable ( TeLayer& layer, TeSPRFile& tabFile, const string& objectCategory, int dType)
{

	TeTable table;

	TeAttrTableVector atts;
	layer.getAttrTables(atts,TeAttrStatic);
	if (atts.empty())
		return;

	// see if the table has an index;
	int index = atts[0].index();

	layer.database()->loadTable(atts[0].name(),table);

	// Retrieve the attribute information 
	TeAttributeList attInfo = table.attributeList();

	
	// Write the attribute information ( skip the index if exists)
	tabFile.writeTableHeader( dType );
	
	if ( index == - 1 )
		tabFile.writeAttributeInfo ( attInfo, objectCategory );
	else
		tabFile.writeAttributeInfo ( attInfo, objectCategory, attInfo [ index ].rep_.name_ );
	
	tabFile.writeInfoEnd();

	for (unsigned int row = 0; row < table.size(); row++ )
	{
		// read the values
		const TeTableRow& values = table[row];

		string label;

		if ( index == - 1 )
			label =  Te2String ((int)row+1 ) ;
		else
			label = values [ index ];
		
		if (!label.empty())
		{
			tabFile.writeGeoId (label);

			// Iterate thru each row
			for (unsigned int col = 0; col < values.size(); col++ )
			{
				// skip the index - already written
				if ( col == (unsigned int) index ) 
					continue;

				// write the value
				tabFile.writeValue ( values[col], attInfo [ col ].rep_.type_);
			}
			tabFile.writeNewLine();
		}
	}
	tabFile.writeEnd();
}
