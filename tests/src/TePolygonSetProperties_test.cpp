
#include <terralib/kernel/TePolygonSetProperties.h>
#include <terralib/kernel/TeGeometry.h>

#include <terralib/kernel/TeUtils.h>
#include <terralib/kernel/TePrecision.h>
#include <terralib/kernel/TeAgnostic.h>

#include <iostream>

#include <stdlib.h>

void TePolygonSetProperties_test()
{
  // Creates a set of polygons

  // A simple polygon
  TeLine2D line;
  line.add(TeCoord2D(900,900));
  line.add(TeCoord2D(900,1000));
  line.add(TeCoord2D(1000,1000));
  line.add(TeCoord2D(1000,900));
  line.add(TeCoord2D(900,900));

  TeLinearRing r1(line);
  TePolygon poly1;
  poly1.add(r1);

  // A polygon with a hole
  TeLine2D line2;
  line2.add(TeCoord2D(200,200));
  line2.add(TeCoord2D(200,400));
  line2.add(TeCoord2D(400,400));
  line2.add(TeCoord2D(400,200));
  line2.add(TeCoord2D(200,200));

  TeLinearRing r2(line2);

  TeLine2D line3;
  line3.add(TeCoord2D(250,250));
  line3.add(TeCoord2D(250,300));
  line3.add(TeCoord2D(300,300));
  line3.add(TeCoord2D(300,250));
  line3.add(TeCoord2D(250,250));

  TeLinearRing r3(line3);

  TePolygon poly2;
  poly2.add(r2);
  poly2.add(r3);

  // A simple polygon
  TeLine2D line4;
  line4.add(TeCoord2D(100,200));
  line4.add(TeCoord2D(150,150));
  line4.add(TeCoord2D(200,100));
  line4.add(TeCoord2D(250,150));
  line4.add(TeCoord2D(300,200));
  line4.add(TeCoord2D(350,250));
  line4.add(TeCoord2D(400,300));
  line4.add(TeCoord2D(450,350));
  line4.add(TeCoord2D(500,400));
  line4.add(TeCoord2D(400,500));
  line4.add(TeCoord2D(300,400));
  line4.add(TeCoord2D(100,200));

  TeLinearRing r4(line4);
  TePolygon poly3;
  poly3.add(r4);

  TePolygonSet polygons;
  polygons.add(poly1);
  polygons.add(poly2);
  polygons.add(poly3);
  
  TePolygonSetProperties polygonProperties( polygons );
  polygonProperties.printProperties();
}

int main()
{
  std::cout << std::endl << "Test started." << std::endl;

  try{
    TEAGN_DEBUG_MODE_CHECK;
    TePrecision::instance().setPrecision( 0.00001 );
  
    TePolygonSetProperties_test();
  }
  catch(TeException& excpt){
    TEAGN_LOGERR( excpt.message() );
    TEAGN_LOGERR( "Test Failed.");
    return EXIT_FAILURE;
  }
  catch(...){
    TEAGN_LOGERR( "Unhandled exception - Test Failed.");
    return EXIT_FAILURE;
  }  

  std::cout << std::endl << "Test OK." << std::endl;
  return EXIT_SUCCESS;
}
