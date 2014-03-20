
#include <terralib/kernel/TeSharedPtr.h>
#include <terralib/kernel/TeRaster.h>
#include <terralib/kernel/TeGeometry.h>

#include <terralib/kernel/TeAgnostic.h>

#include <stdlib.h>

/* Bugzilla Bug 220 - TeRasterClipping fails when using MEM decoder */
void MEMDecoterClipping_test()
{
  /* Creating test raster */
  
  TeRasterParams RAMRaster_params;
  RAMRaster_params.nBands( 1 );
  RAMRaster_params.setDataType( TeUNSIGNEDCHAR );
  RAMRaster_params.setDummy( 0, -1 );
  RAMRaster_params.setNLinesNColumns( 100, 100 );
  RAMRaster_params.decoderIdentifier_ = "MEM";
  
  TeNoProjection proj_instance;
  RAMRaster_params.projection( &proj_instance );
  
  TeRaster RAMRaster( RAMRaster_params );
  
  TEAGN_TRUE_OR_THROW( RAMRaster.init(),
    "RAM Raster init error" );
    
  for( int line = 0 ; line < RAMRaster.params().nlines_ ; ++line ) {
    for( int col = 0 ; col < RAMRaster.params().ncols_ ; ++col ) {
      RAMRaster.setElement( col, line, 1.0, 0 );
    }
  }
  
  /* Defining clipping polygon set */
  
  TeLine2D line;
  line.add( RAMRaster.index2Coord( TeCoord2D( 5,5 ) ) );
  line.add( RAMRaster.index2Coord( TeCoord2D( 94,5 ) ) );
  line.add( RAMRaster.index2Coord( TeCoord2D( 94,94 ) ) );
  line.add( RAMRaster.index2Coord( TeCoord2D( 5,94 ) ) );
  line.add( RAMRaster.index2Coord( TeCoord2D( 5,5 ) ) );
  
  TePolygon pol;
  pol.add( line );
  
  TePolygonSet ps;
  ps.add( pol );
  
  /* Clipping */
  
  TeSharedPtr< TeRaster > clipped_raster( TeRasterClipping( &RAMRaster,
    ps, RAMRaster.projection(), "", 0.0, "MEM" ) );
    
  TEAGN_TRUE_OR_THROW( clipped_raster.isActive(), "Clipping error" );
}


int main()
{
  TEAGN_LOGMSG( "Test started." );

  try{
    TEAGN_DEBUG_MODE_CHECK;
    
    MEMDecoterClipping_test();
  }
  catch( TeException& excpt ){
    TEAGN_LOGERR( excpt.message() );
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Test OK." );
  return EXIT_SUCCESS;
}
