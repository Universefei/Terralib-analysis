#include "TeTestsBase.hpp"

#include <terralib/kernel/TeSharedPtr.h>
#include <terralib/kernel/TeGTFactory.h>
#include <terralib/kernel/TeAffineGT.h>
#include <terralib/kernel/TeUtils.h>
#include <terralib/kernel/TePrecision.h>
#include <terralib/kernel/TeAsciiFile.h>
#include <terralib/kernel/TeAgnostic.h>

#include <iostream>

#include <time.h>
#include <stdlib.h>

bool test_direct_mapping( 
  const TeSharedPtr< TeGeometricTransformation >& transf,
  const TeCoord2D& coord, const TeCoord2D& expected_coord )
{
  TeCoord2D mapped_coord;
  
  transf->directMap( coord, mapped_coord );
  
  const double diffx = mapped_coord.x() - expected_coord.x();
  const double diffy = mapped_coord.y() - expected_coord.y();
  const double error = sqrt( ( diffx * diffx ) + ( diffy * diffy ) );

  if( error > 0.0000000001 )
  {
    TEAGN_WATCH( error );
    return false;
  }
  else
  {
    return true;
  }
}


bool test_inverse_mapping( 
  const TeSharedPtr< TeGeometricTransformation >& transf,
  const TeCoord2D& coord, const TeCoord2D& expected_coord )
{
  TeCoord2D mapped_coord;
  
  transf->inverseMap( coord, mapped_coord );
    
  if( mapped_coord == expected_coord ) {
    return true;
  } else {
    return false;
  }
}

bool loadTPSetFromFile( const std::string& fileName,
  std::vector< TeCoordPair >& tiepoints_vec )
{
  tiepoints_vec.clear();
  
  FILE* filePtr = fopen( fileName.c_str(), "r" );
  TEAGN_TRUE_OR_RETURN( filePtr, "file open error" );
  
  TeCoordPair auxPair;
  
  while( fscanf( filePtr, "%lf %lf %lf %lf", &auxPair.pt1.x_, &auxPair.pt1.y_, 
    &auxPair.pt2.x_, &auxPair.pt2.y_ ) == 4 )
  {
    tiepoints_vec.push_back( auxPair );
    
//    TEAGN_WATCH( auxPair.pt1.x_ );
//    TEAGN_WATCH( auxPair.pt1.y_ );
//    TEAGN_WATCH( auxPair.pt2.x_ );
//    TEAGN_WATCH( auxPair.pt2.y_ );
  }

  fclose( filePtr );
  
  return true;
}

void generateTestTPSet1( std::vector< TeCoordPair >& tiepoints_vec )
{
  tiepoints_vec.clear();
  
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 277.00, 82.00),TeCoord2D( 372.00, 780.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 597.00, 118.00),TeCoord2D( 558.00, 827.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 361.00, 162.00),TeCoord2D( 363.00, 795.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 594.00, 193.00),TeCoord2D( 429.00, 895.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 326.00, 206.00),TeCoord2D( 440.00, 589.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 403.00, 241.00),TeCoord2D( 328.00, 243.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 408.00, 235.00),TeCoord2D( 411.00, 246.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 529.00, 258.00),TeCoord2D( 529.00, 258.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 238.00, 282.00),TeCoord2D( 238.00, 282.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 249.00, 267.00),TeCoord2D( 249.00, 267.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 306.00, 266.00),TeCoord2D( 306.00, 266.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 352.00, 272.00),TeCoord2D( 352.00, 272.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 393.00, 276.00),TeCoord2D( 393.00, 276.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 451.00, 288.00),TeCoord2D( 451.00, 288.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 479.00, 275.00),TeCoord2D( 479.00, 275.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 503.00, 271.00),TeCoord2D( 503.00, 271.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 548.00, 265.00),TeCoord2D( 548.00, 265.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 587.00, 273.00),TeCoord2D( 587.00, 273.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 632.00, 266.00),TeCoord2D( 632.00, 266.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 236.00, 313.00),TeCoord2D( 236.00, 313.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 285.00, 319.00),TeCoord2D( 285.00, 319.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 414.00, 297.00),TeCoord2D( 414.00, 297.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 462.00, 314.00),TeCoord2D( 462.00, 314.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 511.00, 302.00),TeCoord2D( 511.00, 302.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 529.00, 314.00),TeCoord2D( 529.00, 314.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 557.00, 315.00),TeCoord2D( 557.00, 315.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 603.00, 317.00),TeCoord2D( 603.00, 317.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 645.00, 308.00),TeCoord2D( 645.00, 308.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 298.00, 333.00),TeCoord2D( 298.00, 333.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 364.00, 330.00),TeCoord2D( 364.00, 330.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 380.00, 353.00),TeCoord2D( 380.00, 353.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 431.00, 334.00),TeCoord2D( 431.00, 334.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 486.00, 342.00),TeCoord2D( 486.00, 342.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 502.00, 330.00),TeCoord2D( 502.00, 330.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 524.00, 347.00),TeCoord2D( 524.00, 347.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 556.00, 352.00),TeCoord2D( 556.00, 352.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 626.00, 345.00),TeCoord2D( 625.00, 346.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 657.00, 343.00),TeCoord2D( 657.00, 343.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 274.00, 367.00),TeCoord2D( 274.00, 367.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 371.00, 369.00),TeCoord2D( 371.00, 369.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 410.00, 365.00),TeCoord2D( 410.00, 365.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 467.00, 387.00),TeCoord2D( 467.00, 387.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 644.00, 368.00),TeCoord2D( 644.00, 368.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 231.00, 405.00),TeCoord2D( 231.00, 405.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 251.00, 400.00),TeCoord2D( 251.00, 400.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 277.00, 395.00),TeCoord2D( 277.00, 395.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 305.00, 417.00),TeCoord2D( 305.00, 417.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 373.00, 404.00),TeCoord2D( 373.00, 404.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 382.00, 418.00),TeCoord2D( 382.00, 418.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 426.00, 410.00),TeCoord2D( 426.00, 410.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 492.00, 394.00),TeCoord2D( 492.00, 394.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 577.00, 412.00),TeCoord2D( 577.00, 412.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 605.00, 412.00),TeCoord2D( 605.00, 412.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 53.00, 419.00),TeCoord2D( 259.00, 776.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 458.00, 429.00),TeCoord2D( 458.00, 429.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 476.00, 436.00),TeCoord2D( 513.00, 956.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 496.00, 427.00),TeCoord2D( 496.00, 427.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 540.00, 444.00),TeCoord2D( 540.00, 444.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 551.00, 435.00),TeCoord2D( 551.00, 435.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 620.00, 445.00),TeCoord2D( 620.00, 445.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 635.00, 425.00),TeCoord2D( 635.00, 425.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 96.00, 461.00),TeCoord2D( 429.00, 819.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 116.00, 457.00),TeCoord2D( 392.00, 850.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 267.00, 460.00),TeCoord2D( 267.00, 460.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 308.00, 460.00),TeCoord2D( 308.00, 460.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 336.00, 452.00),TeCoord2D( 336.00, 452.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 398.00, 451.00),TeCoord2D( 398.00, 451.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 413.00, 452.00),TeCoord2D( 413.00, 452.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 458.00, 455.00),TeCoord2D( 458.00, 455.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 533.00, 450.00),TeCoord2D( 346.00, 851.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 302.00, 507.00),TeCoord2D( 302.00, 507.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 334.00, 492.00),TeCoord2D( 334.00, 492.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 419.00, 485.00),TeCoord2D( 419.00, 485.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 440.00, 491.00),TeCoord2D( 440.00, 491.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 499.00, 501.00),TeCoord2D( 499.00, 501.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 554.00, 488.00),TeCoord2D( 554.00, 488.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 581.00, 485.00),TeCoord2D( 581.00, 485.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 623.00, 491.00),TeCoord2D( 623.00, 491.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 216.00, 533.00),TeCoord2D( 216.00, 533.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 340.00, 531.00),TeCoord2D( 340.00, 531.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 372.00, 531.00),TeCoord2D( 372.00, 531.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 445.00, 523.00),TeCoord2D( 445.00, 523.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 569.00, 515.00),TeCoord2D( 569.00, 515.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 618.00, 523.00),TeCoord2D( 618.00, 523.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 642.00, 518.00),TeCoord2D( 617.00, 846.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 187.00, 544.00),TeCoord2D( 736.00, 794.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 242.00, 567.00),TeCoord2D( 242.00, 567.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 336.00, 573.00),TeCoord2D( 336.00, 573.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 376.00, 548.00),TeCoord2D( 376.00, 548.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 486.00, 553.00),TeCoord2D( 486.00, 553.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 493.00, 567.00),TeCoord2D( 493.00, 567.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 541.00, 560.00),TeCoord2D( 541.00, 560.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 558.00, 549.00),TeCoord2D( 558.00, 549.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 631.00, 559.00),TeCoord2D( 631.00, 559.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 233.00, 598.00),TeCoord2D( 233.00, 598.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 277.00, 588.00),TeCoord2D( 277.00, 588.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 311.00, 593.00),TeCoord2D( 311.00, 593.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 371.00, 592.00),TeCoord2D( 371.00, 592.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 383.00, 594.00),TeCoord2D( 383.00, 594.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 411.00, 603.00),TeCoord2D( 411.00, 603.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 475.00, 594.00),TeCoord2D( 475.00, 594.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 546.00, 597.00),TeCoord2D( 546.00, 597.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 612.00, 594.00),TeCoord2D( 612.00, 594.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 642.00, 581.00),TeCoord2D( 642.00, 581.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 90.00, 627.00),TeCoord2D( 468.00, 978.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 109.00, 630.00),TeCoord2D( 414.00, 806.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 242.00, 633.00),TeCoord2D( 242.00, 633.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 288.00, 606.00),TeCoord2D( 288.00, 606.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 312.00, 631.00),TeCoord2D( 312.00, 631.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 335.00, 621.00),TeCoord2D( 335.00, 621.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 366.00, 608.00),TeCoord2D( 366.00, 608.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 411.00, 632.00),TeCoord2D( 411.00, 632.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 480.00, 612.00),TeCoord2D( 480.00, 612.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 496.00, 626.00),TeCoord2D( 496.00, 626.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 532.00, 621.00),TeCoord2D( 532.00, 621.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 585.00, 614.00),TeCoord2D( 585.00, 614.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 627.00, 611.00),TeCoord2D( 627.00, 611.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 262.00, 666.00),TeCoord2D( 262.00, 666.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 275.00, 640.00),TeCoord2D( 275.00, 640.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 311.00, 655.00),TeCoord2D( 394.00, 795.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 335.00, 641.00),TeCoord2D( 573.00, 859.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 366.00, 641.00),TeCoord2D( 366.00, 641.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 398.00, 638.00),TeCoord2D( 398.00, 638.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 541.00, 654.00),TeCoord2D( 541.00, 654.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 593.00, 659.00),TeCoord2D( 592.00, 658.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 613.00, 640.00),TeCoord2D( 613.00, 640.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 44.00, 679.00),TeCoord2D( 664.00, 696.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 228.00, 688.00),TeCoord2D( 228.00, 688.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 266.00, 694.00),TeCoord2D( 266.00, 694.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 293.00, 696.00),TeCoord2D( 293.00, 696.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 322.00, 685.00),TeCoord2D( 322.00, 685.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 374.00, 667.00),TeCoord2D( 374.00, 666.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 422.00, 684.00),TeCoord2D( 422.00, 684.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 457.00, 670.00),TeCoord2D( 457.00, 670.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 490.00, 671.00),TeCoord2D( 490.00, 671.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 562.00, 684.00),TeCoord2D( 562.00, 684.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 587.00, 675.00),TeCoord2D( 587.00, 675.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 623.00, 673.00),TeCoord2D( 623.00, 673.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 651.00, 678.00),TeCoord2D( 651.00, 678.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 221.00, 699.00),TeCoord2D( 221.00, 699.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 288.00, 721.00),TeCoord2D( 242.00, 856.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 343.00, 711.00),TeCoord2D( 343.00, 711.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 434.00, 718.00),TeCoord2D( 434.00, 718.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 508.00, 724.00),TeCoord2D( 508.00, 724.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 523.00, 700.00),TeCoord2D( 523.00, 700.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 565.00, 725.00),TeCoord2D( 565.00, 725.00 ) ) );
  tiepoints_vec.push_back( TeCoordPair( TeCoord2D( 635.00, 725.00),TeCoord2D( 635.00, 725.00 ) ) );
}


void printTPs( const std::vector< TeCoordPair >& tpsVec )
{
  std::cout << std::endl;
  
  for( unsigned int idx = 0 ; idx < tpsVec.size() ; ++idx )
  {
    std::cout << "[" << tpsVec[idx].pt1.x() << " , " <<
      tpsVec[idx].pt1.y() << "] -> [" << tpsVec[idx].pt2.x() << " , " <<
      tpsVec[idx].pt2.y() << "]" << std::endl ;
  }
}

void TeAffineGT_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}

void TeAffineGT_decompose_test()
{
  // translations test
  
  {
    TeGTParams transf_params;
    transf_params.transformation_name_ = "affine";
    
    TeSharedPtr< TeGeometricTransformation > affine_ptr( 
      TeGTFactory::make( transf_params ) );
    TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
      "Factory product instatiation error" );
    
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
      TeCoord2D( 10, 5 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
      TeCoord2D( 20, 5 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
      TeCoord2D( 10, 15 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
      TeCoord2D( 20, 15 ) ) );
  
    TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
      "Transformation reset error" );
    
    double translationX = 0;
    double translationY = 0;
    double scalingFactorX = 0;
    double scalingFactorY = 0;
    double skew = 0;
    double squeeze = 0;
    double scaling = 0;
    double rotation = 0;
    
    TEAGN_TRUE_OR_THROW( TeAffineGT::decompose( 
      affine_ptr->getParameters().direct_parameters_,
      translationX, translationY, scalingFactorX, scalingFactorY, skew, 
      squeeze, scaling, rotation ),
      "matrix decompose error" )
      
    TEAGN_CHECK_EPS( translationX, 10.0, 0.00000000001, "invalid translationX" )
    TEAGN_CHECK_EPS( translationY, 5.0, 0.00000000001, "invalid translationY" )
    TEAGN_CHECK_EPS( scalingFactorX, 1.0, 0.00000000001, "invalid scalingX" )
    TEAGN_CHECK_EPS( scalingFactorY, 1.0, 0.00000000001, "invalid scalingY" )
    TEAGN_CHECK_EPS( skew, 0.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( squeeze, 1.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( scaling, 1.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( rotation, 0.0, 0.00000000001, "invalid rotation" )
  }
  
  // Scales test
  
  {
    TeGTParams transf_params;
    transf_params.transformation_name_ = "affine";
    
    TeSharedPtr< TeGeometricTransformation > affine_ptr( 
      TeGTFactory::make( transf_params ) );
    TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
      "Factory product instatiation error" );
    
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
      TeCoord2D( 0, 0 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
      TeCoord2D( 30, 0 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
      TeCoord2D( 0, 5 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
      TeCoord2D( 30, 5 ) ) );
  
    TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
      "Transformation reset error" );
    
    double translationX = 0;
    double translationY = 0;
    double scalingFactorX = 0;
    double scalingFactorY = 0;
    double skew = 0;
    double squeeze = 0;
    double scaling = 0;
    double rotation = 0;
    
    TEAGN_TRUE_OR_THROW( TeAffineGT::decompose( 
      affine_ptr->getParameters().direct_parameters_,
      translationX, translationY, scalingFactorX, scalingFactorY, skew, 
      squeeze, scaling, rotation ),
      "matrix decompose error" )
      
    TEAGN_CHECK_EPS( translationX, 0.0, 0.00000000001, "invalid translationX" )
    TEAGN_CHECK_EPS( translationY, 0.0, 0.00000000001, "invalid translationY" )
    TEAGN_CHECK_EPS( scalingFactorX, 3.0, 0.00000000001, "invalid scalingX" )
    TEAGN_CHECK_EPS( scalingFactorY, 0.5, 0.00000000001, "invalid scalingY" )
    TEAGN_CHECK_EPS( skew, 0.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( squeeze, 2.4494897427831783, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( scaling, 1.2247448713915892, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( rotation, 0.0, 0.00000000001, "invalid rotation" )
  }
  
  // Rotation test
  
  {
    TeGTParams transf_params;
    transf_params.transformation_name_ = "affine";
    
    TeSharedPtr< TeGeometricTransformation > affine_ptr( 
      TeGTFactory::make( transf_params ) );
    TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
      "Factory product instatiation error" );
    
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0.5, 0.5 ), 
      TeCoord2D( 0.0, 1.414213562373095049 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( -0.5, 0.5 ), 
      TeCoord2D( -1.414213562373095049, 0.0 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( -0.5, -0.5 ), 
      TeCoord2D( 0.0, -1.414213562373095049 ) ) );
    transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0.5, -0.5 ), 
      TeCoord2D( 1.414213562373095049, 0.0 ) ) );
  
    TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
      "Transformation reset error" );
    
    double translationX = 0;
    double translationY = 0;
    double scalingFactorX = 0;
    double scalingFactorY = 0;
    double skew = 0;
    double squeeze = 0;
    double scaling = 0;
    double rotation = 0;
    
    TEAGN_TRUE_OR_THROW( TeAffineGT::decompose( 
      affine_ptr->getParameters().direct_parameters_,
      translationX, translationY, scalingFactorX, scalingFactorY, skew, 
      squeeze, scaling, rotation ),
      "matrix decompose error" )
      
    TEAGN_CHECK_EPS( translationX, 0.0, 0.00000000001, "invalid translationX" )
    TEAGN_CHECK_EPS( translationY, 0.0, 0.00000000001, "invalid translationY" )
    TEAGN_CHECK_EPS( scalingFactorX, 2.0, 0.00000000001, "invalid scalingX" )
    TEAGN_CHECK_EPS( scalingFactorY, 2.0, 0.00000000001, "invalid scalingY" )
    TEAGN_CHECK_EPS( skew, 0.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( squeeze, 1.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( scaling, 2.0, 0.00000000001, "invalid skew" )
    TEAGN_CHECK_EPS( rotation, 45.0 * ( 3.141592653589793 / 180.0 ), 
      0.00000000001, "invalid rotation" )
  }  
}

void TeAffineGT_exaustive_out_remotion_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 1.0;
  transf_params.max_imap_error_ = 1.0;
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ) );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 20, 20 ), 
    TeCoord2D( 110, 710 ) ) ); 
    
  transf_params.out_rem_strat_ = TeGTParams::ExaustiveOutRemotion;   

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  printTPs( affine_ptr->getParameters().tiepoints_ );
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}


void TeAffineGT_lwo_remotion_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 0.05;
  transf_params.max_imap_error_ = 0.05;
  transf_params.max_dmap_rmse_ = 0.05;
  transf_params.max_imap_rmse_ = 0.05;
  
  generateTestTPSet1( transf_params.tiepoints_ );
    
  transf_params.out_rem_strat_ = TeGTParams::LWOutRemotion;   

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDirectMappingError() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getInverseMappingError() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDMapRMSE() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getIMapRMSE() <= 0.05 ),
    "Invalid transformation" )
    
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
}

void TeAffineGT_lwo_weighted_tps_remotion_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 0.5;
  transf_params.max_imap_error_ = 0.5;
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );      
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );        
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 5, 5 ), 
    TeCoord2D( 6, 6 ) ) ); 
  transf_params.tiePointsWeights_.push_back( 0.5 );  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 6, 6 ), 
    TeCoord2D( 5, 5 ) ) ); 
  transf_params.tiePointsWeights_.push_back( 0.1 );         
    
  transf_params.out_rem_strat_ = TeGTParams::LWOutRemotion;   

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  printTPs( affine_ptr->getParameters().tiepoints_ );
  
  TEAGN_TRUE_OR_THROW( affine_ptr->getParameters().tiepoints_.size() == 4,
    "Invalid number of tie-poins" )
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}

void TeAffineGT_RANSAC_weighted_tps_remotion_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 1;
  transf_params.max_imap_error_ = 1;
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );      
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );
  transf_params.tiePointsWeights_.push_back( 0.7 );        
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 5, 5 ), 
    TeCoord2D( 6, 6 ) ) ); 
  transf_params.tiePointsWeights_.push_back( 0.5 );  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 6, 6 ), 
    TeCoord2D( 5, 5 ) ) ); 
  transf_params.tiePointsWeights_.push_back( 0.1 );         
    
  transf_params.out_rem_strat_ = TeGTParams::RANSACRemotion;   

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  printTPs( affine_ptr->getParameters().tiepoints_ );
  
  TEAGN_TRUE_OR_THROW( affine_ptr->getParameters().tiepoints_.size() == 4,
    "Invalid number of tie-poins" )
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}

void TeAffineGT_RANSAC_remotion_test1()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 0.01;
  transf_params.max_imap_error_ = 0.01;
  transf_params.max_dmap_rmse_ = 0.01;
  transf_params.max_imap_rmse_ = 0.01;
  
  generateTestTPSet1( transf_params.tiepoints_ );
    
  transf_params.out_rem_strat_ = TeGTParams::RANSACRemotion;   

  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDirectMappingError() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getInverseMappingError() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDMapRMSE() <= 0.05 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getIMapRMSE() <= 0.05 ),
    "Invalid transformation" )
    
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( affine_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
}

void TeAffineGT_RANSAC_remotion_test2()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "affine";
  
  TeSharedPtr< TeGeometricTransformation > affine_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( affine_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.max_dmap_error_ = 60;
  transf_params.max_imap_error_ = 1.0;
  transf_params.max_dmap_rmse_ = DBL_MAX;
  transf_params.max_imap_rmse_ = DBL_MAX;
  
//  TEAGN_TRUE_OR_THROW( loadTPSetFromFile( TETESTSRESPATH "tie_points_set1.txt",
//    transf_params.tiepoints_ ), "error loading tie-points file" );
    
  TeAsciiFile csvFile( TETESTSRESPATH "tie_points_set1.csv", "r" );
  
  TEAGN_TRUE_OR_THROW( csvFile.readCoordPairVect2CSV( 
    transf_params.tiepoints_ ), "error loading tie-points file" );
    
  TEAGN_WATCH( transf_params.tiepoints_.size() );
    
  transf_params.out_rem_strat_ = TeGTParams::RANSACRemotion;   

  time_t startTime;
  time_t endTime;
  time(&startTime);
  
  TEAGN_TRUE_OR_THROW( affine_ptr->reset( transf_params ),
    "Transformation reset error" );
    
  time(&endTime); 
  TEAGN_LOGMSG( "Time taken to build transformation: " +
    Te2String( difftime( endTime, startTime ), 2 ) +
    " seconds" );  
    
  TEAGN_WATCH( affine_ptr->getParameters().tiepoints_.size() );
    
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDirectMappingError() < 60.0 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getInverseMappingError() < 1.0 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getDMapRMSE() < 60.0 ),
    "Invalid transformation" )
  TEAGN_TRUE_OR_THROW( ( affine_ptr->getIMapRMSE() < 1.0 ),
    "Invalid transformation" )
 
  TeCoordPair testTP;  
  
  testTP.pt1.x_ = 3248.0;
  testTP.pt1.y_ = 7205.0;
  testTP.pt2.x_ = -36607.962121;
  testTP.pt2.y_ = -180059.551655;  
  TEAGN_TRUE_OR_THROW( affine_ptr->getDirectMappingError( testTP ) <=
    transf_params.max_dmap_error_, "invalid mapping" );
  TEAGN_TRUE_OR_THROW( affine_ptr->getInverseMappingError( testTP ) <=
    transf_params.max_imap_error_, "invalid mapping" );  
      
  testTP.pt1.x_ = 3236.0;
  testTP.pt1.y_ = 7159.0;
  testTP.pt2.x_ = -37273.470149;
  testTP.pt2.y_ = -177509.338899;  
  TEAGN_TRUE_OR_THROW( affine_ptr->getDirectMappingError( testTP ) <=
    transf_params.max_dmap_error_, "invalid mapping" );
  TEAGN_TRUE_OR_THROW( affine_ptr->getInverseMappingError( testTP ) <=
    transf_params.max_imap_error_, "invalid mapping" );       

  testTP.pt1.x_ = 3189.0;
  testTP.pt1.y_ = 7161.0;
  testTP.pt2.x_ = -39944.910860;
  testTP.pt2.y_ = -177604.760166;  
  TEAGN_TRUE_OR_THROW( affine_ptr->getDirectMappingError( testTP ) <=
    transf_params.max_dmap_error_, "invalid mapping" );
  TEAGN_TRUE_OR_THROW( affine_ptr->getInverseMappingError( testTP ) <=
    transf_params.max_imap_error_, "invalid mapping" );
}

void TeProjectiveGT_deterministic_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "projective";
  
  TeSharedPtr< TeGeometricTransformation > tranf_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( tranf_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );

  TEAGN_TRUE_OR_THROW( tranf_ptr->reset( transf_params ),
    "Transformation reset error" );
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}


void TeProjectiveGT_nondeterministic_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "projective";
  
  TeSharedPtr< TeGeometricTransformation > tranf_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( tranf_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ) );    

  TEAGN_TRUE_OR_THROW( tranf_ptr->reset( transf_params ),
    "Transformation reset error" );
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( tranf_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( tranf_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}

void Te2ndDegPolinomialGT_test()
{
  TeGTParams transf_params;
  transf_params.transformation_name_ = "2ndDegPolinomial";
  
  TeSharedPtr< TeGeometricTransformation > transf_ptr( 
    TeGTFactory::make( transf_params ) );
  TEAGN_TRUE_OR_THROW( transf_ptr.isActive(),
    "Factory product instatiation error" );
  
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 10, 0 ), 
    TeCoord2D( 20, 0 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 0, 3 ), 
    TeCoord2D( 0, 6 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 11, 11 ), 
    TeCoord2D( 22, 22 ) ) );
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 5, 0 ), 
    TeCoord2D( 10, 0 ) ) );    
  transf_params.tiepoints_.push_back( TeCoordPair( TeCoord2D( 9, 10 ), 
    TeCoord2D( 18, 20 ) ) );    

  TEAGN_TRUE_OR_THROW( transf_ptr->reset( transf_params ),
    "Transformation reset error" );
  
  TEAGN_TRUE_OR_THROW( test_direct_mapping( transf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( transf_ptr, TeCoord2D( 10, 0 ), 
    TeCoord2D( 20, 0 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( transf_ptr, TeCoord2D( 0, 10 ), 
    TeCoord2D( 0, 20 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( transf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 20, 20 ) ), "Direct mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_direct_mapping( transf_ptr, TeCoord2D( 5, 5 ), 
    TeCoord2D( 10, 10 ) ), "Direct mapping checking failed" );
  
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( transf_ptr, TeCoord2D( 0, 0 ), 
    TeCoord2D( 0, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( transf_ptr, TeCoord2D( 20, 0 ), 
    TeCoord2D( 10, 0 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( transf_ptr, TeCoord2D( 0, 20 ), 
    TeCoord2D( 0, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( transf_ptr, TeCoord2D( 20, 20 ), 
    TeCoord2D( 10, 10 ) ), "Inverse mapping checking failed" );
  TEAGN_TRUE_OR_THROW( test_inverse_mapping( transf_ptr, TeCoord2D( 10, 10 ), 
    TeCoord2D( 5, 5 ) ), "Inverse mapping checking failed" );
}


int main()
{
  std::cout << std::endl << "Test started." << std::endl;

  TEAGN_DEBUG_MODE_CHECK;
  
  TePrecision::instance().setPrecision( 0.00001 );
  
  // Testing the parameters estimation without outliers
  // remotion

  TeAffineGT_test();
  TeAffineGT_decompose_test();
  TeProjectiveGT_deterministic_test();
  TeProjectiveGT_nondeterministic_test();
  Te2ndDegPolinomialGT_test();
  
  // Testing Outliers remotion  
  
  TeAffineGT_lwo_remotion_test();
  TeAffineGT_lwo_weighted_tps_remotion_test();
  TeAffineGT_exaustive_out_remotion_test();
  TeAffineGT_lwo_remotion_test();
  TeAffineGT_RANSAC_remotion_test1();
  TeAffineGT_RANSAC_remotion_test2();
  TeAffineGT_RANSAC_weighted_tps_remotion_test();

  std::cout << std::endl << "Test OK." << std::endl;
  return EXIT_SUCCESS;
}
