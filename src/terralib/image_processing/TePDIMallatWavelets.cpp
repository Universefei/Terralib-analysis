#include "TePDIMallatWavelets.hpp"

#include "TePDIUtils.hpp"
#include "TePDIStrategyFactory.hpp"
#include "TePDIMathFunctions.hpp"
#include "TePDIPIManager.hpp"

#include "../kernel/TeAgnostic.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeDefines.h"

#include <math.h>
#include <float.h>


TePDIMallatWavelets::TePDIMallatWavelets()
{
  shift_histogram_flag_ = false;
  input_image_mean_ = 0;
}


TePDIMallatWavelets::~TePDIMallatWavelets()
{
}


bool TePDIMallatWavelets::CheckParameters( 
  const TePDIParameters& parameters ) const
{
  std::string filter_task;
  TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "filter_task", filter_task ),
    "Missing parameter: filter_task" );
    
  /* Checking filters */
  
  std::vector< double > a_filter_l;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "a_filter_l", a_filter_l ),
    "Missing parameter: a_filter_l" );
  TEAGN_TRUE_OR_RETURN( a_filter_l.size() > 1, 
    "Invalid parameter: a_filter_l" );
    
  std::vector< double > a_filter_h;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "a_filter_h", a_filter_h ),
    "Missing parameter: a_filter_h" );
  TEAGN_TRUE_OR_RETURN( a_filter_h.size() > 1, 
    "Invalid parameter: a_filter_h" );    
    
  std::vector< double > s_filter_l;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "s_filter_l", s_filter_l ),
    "Missing parameter: s_filter_l" );
  TEAGN_TRUE_OR_RETURN( s_filter_l.size() > 1, 
    "Invalid parameter: s_filter_l" );
    
  std::vector< double > s_filter_h;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "s_filter_h", s_filter_h ),
    "Missing parameter: s_filter_h" );
  TEAGN_TRUE_OR_RETURN( s_filter_h.size() > 1, 
    "Invalid parameter: s_filter_h" );
    
  unsigned int max_filter_size = ( unsigned int )
    MAX( a_filter_l.size(), MAX( a_filter_h.size(), MAX( s_filter_l.size(), 
    s_filter_h.size() ) ) );
    
  /* Checking filters scale */        
  
  double filters_scale = 0;
  TEAGN_TRUE_OR_RETURN( 
    parameters.GetParameter( "filters_scale", filters_scale ),
    "Missing parameter: filters_scale" );
  TEAGN_TRUE_OR_RETURN( filters_scale != 0, "filters scale cannot be zero" );
  
  /* Additional checking for each task */
  
  if( filter_task == "SBExtract" ) {
      /* Checking input_raster */
  
      TePDITypes::TePDIRasterPtrType input_image;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_image", 
        input_image ),  "Missing parameter: input_image" );
      TEAGN_TRUE_OR_RETURN( input_image.isActive(),
        "Invalid parameter: input_image inactive" );
      TEAGN_TRUE_OR_RETURN( input_image->params().status_ != 
        TeRasterParams::TeNotReady, 
        "Invalid parameter: input_image not ready" );
    
      TEAGN_TRUE_OR_RETURN( ( input_image->params().nlines_ > 1 ), 
        "Invalid input_image lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_image->params().ncols_ > 1 ), 
        "Invalid input_image columns number" );
    
      /* Checking input band */
  
      int band;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "band", band ),
        "Missing parameter: band" );
      TEAGN_TRUE_OR_RETURN( band < input_image->nBands(),
        "Invalid parameter: band" );
        
      /* Checking photometric interpretation */
  
      TEAGN_TRUE_OR_RETURN( ( 
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeRGB ) ||
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeMultiBand ) ),
        "Invalid parameter: input_image "
        "( invalid photometric interpretation" );
        
      /* checking for the output lowlow raster */
    
      TePDITypes::TePDIRasterPtrType sub_band;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "sub_band", sub_band ),
        "Missing parameter: sub_band" );
      TEAGN_TRUE_OR_RETURN( sub_band.isActive(),
        "Invalid parameter: sub_band inactive pointer" );
      TEAGN_TRUE_OR_RETURN( sub_band->params().status_ != 
        TeRasterParams::TeNotReady, "Invalid parameter: sub_band not ready" );        
      
      int sub_band_index = 0;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "sub_band_index", 
        sub_band_index ), "Missing parameter: sub_band" );
      TEAGN_TRUE_OR_RETURN( ( ( sub_band_index >= 0 ) && 
        ( sub_band_index < 4 ) ), "Invalid parameter: sub_band_index" );
        
      /* Checking decomposition levels number */
      
      int max_levels = (int)( MIN(
          TePDIMathFunctions::DecimLevels( input_image->params().nlines_ ),
          TePDIMathFunctions::DecimLevels( input_image->params().ncols_ ) ) );
    
      int levels = 0;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "levels", levels ),
        "Missing parameter: levels" );
      TEAGN_TRUE_OR_RETURN( ( levels > 0 ) && ( levels <= max_levels ),
        "Invalid parameter: levels" );
      TEAGN_TRUE_OR_RETURN( 
        ( TePDIMathFunctions::DecimLevelSize(
        levels, MIN( input_image->params().nlines_,
        input_image->params().ncols_ ) ) >= max_filter_size ),
        "Invalid parameter: The current \"levels\" parameter and "
        "input filters are incompatible" );
    } else if( filter_task == "SBSwap" ) {
      /* Checking input_raster */
  
      TePDITypes::TePDIRasterPtrType input_image;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_image", 
        input_image ), "Missing parameter: input_image" );
      TEAGN_TRUE_OR_RETURN( input_image.isActive(),
        "Invalid parameter: input_image inactive" );
      TEAGN_TRUE_OR_RETURN( input_image->params().status_ != 
        TeRasterParams::TeNotReady, 
        "Invalid parameter: input_image not ready" );
    
      TEAGN_TRUE_OR_RETURN( ( input_image->params().nlines_ > 1 ), 
        "Invalid input_image lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_image->params().ncols_ > 1 ), 
        "Invalid input_image columns number" );
    
      /* Checking input band */
  
      int band;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "band", band ),
        "Missing parameter: band" );
      TEAGN_TRUE_OR_RETURN( band < input_image->nBands(),
        "Invalid parameter: band" );
        
      /* Checking photometric interpretation */
  
      TEAGN_TRUE_OR_RETURN( ( 
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeRGB ) ||
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeMultiBand ) ),
        "Invalid parameter: input_image "
        "( invalid photometric interpretation" );        
        
      /* Checking decomposition levels number */   
  
      int max_levels = (int)( MIN(
          TePDIMathFunctions::DecimLevels( input_image->params().nlines_ ),
          TePDIMathFunctions::DecimLevels( input_image->params().ncols_ ) ) );
    
      int levels = 0;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "levels", levels ),
        "Missing parameter: levels" );
      TEAGN_TRUE_OR_RETURN( ( levels > 0 ) && ( levels <= max_levels ),
        "Invalid parameter: levels" );
      TEAGN_TRUE_OR_RETURN( 
        ( TePDIMathFunctions::DecimLevelSize(
        levels, MIN( input_image->params().nlines_,
        input_image->params().ncols_ ) ) >= max_filter_size ),
        "Invalid parameter: The current \"levels\" parameter and "
        "input filters are incompatible" );                
            
      /* Checking output_raster */
  
      TePDITypes::TePDIRasterPtrType output_image;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "output_image", output_image ),
        "Missing parameter: output_image" );
      TEAGN_TRUE_OR_RETURN( output_image.isActive(),
        "Invalid parameter: output_image inactive" );
      TEAGN_TRUE_OR_RETURN( output_image->params().status_ != 
        TeRasterParams::TeNotReady, 
        "Invalid parameter: output_image not ready" );    
        
      /* checking for the sub-band */
    
      TePDITypes::TePDIRasterPtrType sub_band;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "sub_band", sub_band ),
        "Missing parameter: sub_band" );
      TEAGN_TRUE_OR_RETURN( sub_band.isActive(),
        "Invalid parameter: sub_band inactive pointer" );
      TEAGN_TRUE_OR_RETURN( sub_band->params().status_ != 
        TeRasterParams::TeNotReady, "Invalid parameter: sub_band not ready" );       
        
      TEAGN_TRUE_OR_RETURN( ( ((unsigned int)sub_band->params().nlines_) == 
        TePDIMathFunctions::DecimLevelSize( levels,
        input_image->params().nlines_ ) ),
        "Invalid parameter: Sub_band lines mismatch for the choosen "
        "decomposition level" );
      TEAGN_TRUE_OR_RETURN( ( ( (unsigned int)sub_band->params().ncols_) == 
        TePDIMathFunctions::DecimLevelSize( levels,
        input_image->params().ncols_ ) ),
        "Invalid parameter: Sub_band columns mismatch for the choosen "
        "decomposition level" );
        
      /* checking sub-band index */
                
      int sub_band_index = 0;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "sub_band_index", 
        sub_band_index ), "Missing parameter: sub_band" );
      TEAGN_TRUE_OR_RETURN( ( ( sub_band_index >= 0 ) && 
        ( sub_band_index < 4 ) ), "Invalid parameter: sub_band_index" );       
    } else if( filter_task == "GetPyramid" ) {
      /* Checking input_raster */
  
      TePDITypes::TePDIRasterPtrType input_image;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_image", 
        input_image ), "Missing parameter: input_image" );
      TEAGN_TRUE_OR_RETURN( input_image.isActive(),
        "Invalid parameter: input_image inactive" );
      TEAGN_TRUE_OR_RETURN( input_image->params().status_ != 
        TeRasterParams::TeNotReady, 
        "Invalid parameter: input_image not ready" );
    
      TEAGN_TRUE_OR_RETURN( ( input_image->params().nlines_ > 1 ), 
        "Invalid input_image lines number" );
      TEAGN_TRUE_OR_RETURN( ( input_image->params().ncols_ > 1 ), 
        "Invalid input_image columns number" );
    
      /* Checking input band */
  
      int band;
      TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "band", band ),
        "Missing parameter: band" );
      TEAGN_TRUE_OR_RETURN( band < input_image->nBands(),
        "Invalid parameter: band" );
        
      /* Checking photometric interpretation */
  
      TEAGN_TRUE_OR_RETURN( ( 
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeRGB ) ||
        ( input_image->params().photometric_[ band ] == 
          TeRasterParams::TeMultiBand ) ),
        "Invalid parameter: input_image "
        "( invalid photometric interpretation" );        
        
      /* Checking decomposition levels number */   
  
      int max_levels = (int)( MIN(
          TePDIMathFunctions::DecimLevels( input_image->params().nlines_ ),
          TePDIMathFunctions::DecimLevels( input_image->params().ncols_ ) ) );
    
      int levels = 0;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "levels", levels ),
        "Missing parameter: levels" );
      TEAGN_TRUE_OR_RETURN( ( levels > 0 ) && ( levels <= max_levels ),
        "Invalid parameter: levels" );
      TEAGN_TRUE_OR_RETURN( 
        ( TePDIMathFunctions::DecimLevelSize(
        levels, MIN( input_image->params().nlines_,
        input_image->params().ncols_ ) ) >= max_filter_size ),
        "Invalid parameter: The current \"levels\" parameter and "
        "input filters are incompatible" );
        
      /* Checking for the pyramid reference */
      
      TePDITypes::TePDIRasterVectorPtrType pyramid;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "pyramid", pyramid ),
        "Missing parameter: pyramid" );
      TEAGN_TRUE_OR_RETURN( 
        pyramid.isActive(), "Inactive parameter: pyramid" );
    } else if( filter_task == "RecomposePyramid" ) {
      /* Checking input_raster */
  
      TePDITypes::TePDIRasterPtrType input_image;
      
      if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
        "input_image" ) ) {
        
        TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "input_image", 
          input_image ), "Missing parameter: input_image" );
        TEAGN_TRUE_OR_RETURN( input_image.isActive(),
          "Invalid parameter: input_image inactive" );
        TEAGN_TRUE_OR_RETURN( input_image->params().status_ != 
          TeRasterParams::TeNotReady, 
          "Invalid parameter: input_image not ready" );
      
        TEAGN_TRUE_OR_RETURN( ( input_image->params().nlines_ > 1 ), 
          "Invalid input_image lines number" );
        TEAGN_TRUE_OR_RETURN( ( input_image->params().ncols_ > 1 ), 
          "Invalid input_image columns number" );
      
        /* Checking input band */
    
        int band;
        TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "band", band ),
          "Missing parameter: band" );
        TEAGN_TRUE_OR_RETURN( band < input_image->nBands(),
          "Invalid parameter: band" );
          
        /* Checking photometric interpretation */
    
        TEAGN_TRUE_OR_RETURN( ( 
          ( input_image->params().photometric_[ band ] == 
            TeRasterParams::TeRGB ) ||
          ( input_image->params().photometric_[ band ] == 
            TeRasterParams::TeMultiBand ) ),
          "Invalid parameter: input_image "
          "( invalid photometric interpretation" );          
      }
      
      /* Checking output_raster */
  
      TePDITypes::TePDIRasterPtrType output_image;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "output_image", output_image ),
        "Missing parameter: output_image" );
      TEAGN_TRUE_OR_RETURN( output_image.isActive(),
        "Invalid parameter: output_image inactive" );
      TEAGN_TRUE_OR_RETURN( output_image->params().status_ != 
        TeRasterParams::TeNotReady, 
        "Invalid parameter: output_image not ready" );          
      
      /* Checking for the pyramid reference */
      
      TePDITypes::TePDIRasterVectorPtrType pyramid;
      TEAGN_TRUE_OR_RETURN( 
        parameters.GetParameter( "pyramid", pyramid ),
        "Missing parameter: pyramid" );
      TEAGN_TRUE_OR_RETURN( 
        pyramid.isActive(), "Inactive parameter: pyramid" );      
      
      TEAGN_TRUE_OR_RETURN( ( ( pyramid->size() % 4 ) == 0 ),
        "Invalid input pyramid size" );
        
      unsigned int py_levels = pyramid->size() / 4;
      unsigned int py_sb00_index = 0;
      for( unsigned int level = 1 ; level <= py_levels ; ++level ) {
        py_sb00_index = ( level - 1 ) * 4;
        
        TEAGN_TRUE_OR_RETURN( (
          ( (*pyramid)[ py_sb00_index ]->params().nlines_ ==
            (*pyramid)[ py_sb00_index + 1 ]->params().nlines_ ) &&
          ( (*pyramid)[ py_sb00_index ]->params().nlines_ ==
            (*pyramid)[ py_sb00_index + 2 ]->params().nlines_ ) &&
          ( (*pyramid)[ py_sb00_index ]->params().nlines_ ==
            (*pyramid)[ py_sb00_index + 3 ]->params().nlines_ ) &&
          ( (*pyramid)[ py_sb00_index ]->params().ncols_ ==
            (*pyramid)[ py_sb00_index + 1 ]->params().ncols_ ) &&
          ( (*pyramid)[ py_sb00_index ]->params().ncols_ ==
            (*pyramid)[ py_sb00_index + 2 ]->params().ncols_ ) &&
          ( (*pyramid)[ py_sb00_index ]->params().ncols_ ==
            (*pyramid)[ py_sb00_index + 3 ]->params().ncols_ ) ),
          "Dimensions mismatch detected between same level sub-bands"
          " inside input pyramid" );
            
        TEAGN_TRUE_OR_RETURN( (
          ( (*pyramid)[ py_sb00_index ]->nBands() == 1 ) &&
          ( (*pyramid)[ py_sb00_index + 1 ]->nBands() == 1 ) &&
          ( (*pyramid)[ py_sb00_index + 2 ]->nBands() == 1 ) &&
          ( (*pyramid)[ py_sb00_index + 3 ]->nBands() == 1 ) ),
          "Invalid number of bands detected inside input pyramid" );
          
        /* Checking the correct dimensions between levels */
        
        if( level != py_levels ) {
          TEAGN_TRUE_OR_RETURN( (
            ( (*pyramid)[ py_sb00_index ]->params().nlines_ >
            ( 2 * ( (*pyramid)[ py_sb00_index + 4 ]->params().nlines_ - 
            1 ) ) ) 
            && ( (*pyramid)[ py_sb00_index ]->params().ncols_ >
            ( 2 * ( (*pyramid)[ py_sb00_index + 4 ]->params().ncols_ - 
            1 ) ) ) ),
            "Dimensions mismatch detected between levels inside input pyramid" 
            );
        }
      }
      
      if( input_image.isActive() ) {
        TEAGN_TRUE_OR_RETURN(
          ( (*pyramid)[ 0 ]->params().nlines_ == 
          ( (int)ceil( ((double)input_image->params().nlines_) / 2 ) ) &&
          ( (*pyramid)[ 0 ]->params().ncols_) == 
          ( (int)ceil( ((double)input_image->params().ncols_ ) / 2 ) ) ) , 
          "The supplied pyramid and reference image are not compatible" );
      }
    } else { 
      TEAGN_LOG_AND_RETURN( "Invalid filter_task parameter" );
    }
  
  return true;  
}


bool TePDIMallatWavelets::RunImplementation()
{
  std::string filter_task;
  params_.GetParameter( "filter_task", filter_task );

  if( filter_task == "SBExtract" ) {
      TEAGN_TRUE_OR_RETURN( Decompose( params_ ), 
        "Unable to do Wavelet decomposition" );
        
      TePDITypes::TePDIRasterPtrType sub_band;
      params_.GetParameter( "sub_band", sub_band );
      
      int sub_band_index = 0;
      params_.GetParameter( "sub_band_index", sub_band_index );
      
      TEAGN_TRUE_OR_RETURN( GetSubBand(  pyramid_.size() - 1,
        sub_band_index, sub_band ),
        "Unable to copy sub-band to output raster" );
        
      pyramid_.clear();
    } else if( filter_task == "SBSwap" ) {
      TEAGN_TRUE_OR_RETURN( Decompose( params_ ), 
        "Unable to do Wavelet decomposition" );
        
      TePDITypes::TePDIRasterPtrType sub_band;
      params_.GetParameter( "sub_band", sub_band );
      
      int sub_band_index = 0;
      params_.GetParameter( "sub_band_index", sub_band_index );      
      
      TEAGN_TRUE_OR_RETURN( ChangeSubBand( pyramid_.size() - 1,
        sub_band_index, sub_band ), "Sub-band swap error" );
        
      TEAGN_TRUE_OR_RETURN( Recompose( pyramid_.size() - 1, params_ ),
        "Recomposition error" );
        
      pyramid_.clear();
    } else if( filter_task == "GetPyramid" ) {
      TEAGN_TRUE_OR_RETURN( Decompose( params_ ), 
        "Unable to do Wavelet decomposition" );
        
      TePDITypes::TePDIRasterVectorPtrType pyramid;
      params_.GetParameter( "pyramid", pyramid );
        
      for( unsigned int level = 1 ; level < pyramid_.size() ; ++level ) {
        TePDITypes::TePDIRasterPtrType sub_band00;
        TePDITypes::TePDIRasterPtrType sub_band01;
        TePDITypes::TePDIRasterPtrType sub_band10;
        TePDITypes::TePDIRasterPtrType sub_band11;
        
        TEAGN_TRUE_OR_RETURN(
          TePDIUtils::TeAllocRAMRaster( sub_band00, 1, 1, 1, false, TeDOUBLE, 
          0 ), "Unable to allocate temporary sub-band00 raster" );
        TEAGN_TRUE_OR_RETURN(
          TePDIUtils::TeAllocRAMRaster( sub_band01, 1, 1, 1, false, TeDOUBLE, 
          0 ), "Unable to allocate temporary sub-band01 raster" );
        TEAGN_TRUE_OR_RETURN(
          TePDIUtils::TeAllocRAMRaster( sub_band10, 1, 1, 1, false, TeDOUBLE, 
          0 ), "Unable to allocate temporary sub-band10 raster" );
        TEAGN_TRUE_OR_RETURN(
          TePDIUtils::TeAllocRAMRaster( sub_band11, 1, 1, 1, false, TeDOUBLE, 
          0 ), "Unable to allocate temporary sub-band11 raster" );
      
        TEAGN_TRUE_OR_RETURN( GetSubBand( level, 0, sub_band00 ),
          "Unable to copy sub-band00 to output raster" );
        pyramid->push_back( sub_band00 );        
        TEAGN_TRUE_OR_RETURN( GetSubBand( level, 1, sub_band01 ),
          "Unable to copy sub-band01 to output raster" );
        pyramid->push_back( sub_band01 );
        TEAGN_TRUE_OR_RETURN( GetSubBand( level, 2, sub_band10 ),
          "Unable to copy sub-band10 to output raster" );
        pyramid->push_back( sub_band10 );
        TEAGN_TRUE_OR_RETURN( GetSubBand( level, 3, sub_band11 ),
          "Unable to copy sub-band11 to output raster" );
        pyramid->push_back( sub_band11 );
      }
      
      pyramid_.clear();
    } else if( filter_task == "RecomposePyramid" ) {
      pyramid_.clear();
      
      TePDITypes::TePDIRasterPtrType input_image;
      if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
        "input_image" ) ) {
        
        params_.GetParameter( "input_image", input_image );
      }      
      
      TePDITypes::TePDIRasterVectorPtrType pyramid;
      params_.GetParameter( "pyramid", pyramid );
      
      const unsigned int levels = ( pyramid->size() - 1 ) / 3;
      unsigned int sblines = 0;
      unsigned int sbcols = 0;
      unsigned int py_sb00_index = 0;
    
      for( unsigned int level = 1 ; level <= levels ; ++level ) {
        py_sb00_index = ( level - 1 ) * 4;
      
        if( ( level == 1 ) && ( input_image.isActive() ) ) {
          sblines = ( unsigned int )
            ceil( ( (double)input_image->params().nlines_ ) / 2 );
          sbcols = ( unsigned int )
            ceil( ( (double)input_image->params().ncols_ ) / 2 );
        } else {
          sblines = (*pyramid)[ py_sb00_index + 1 ]->params().nlines_;
          sbcols = (*pyramid)[ py_sb00_index + 1 ]->params().ncols_;
        }
        
        TEAGN_TRUE_OR_RETURN( AllocateTopLevel( level, sblines, sbcols ),
          "Unable to allocate the new pyramid level " +
          Te2String( level ) );
          
        TEAGN_TRUE_OR_RETURN( ChangeSubBand( level,
          0, (*pyramid)[ py_sb00_index ] ), "Unable to copy pyramid data at"
          "level " + Te2String( level ) + " sub-band 00" );
        TEAGN_TRUE_OR_RETURN( ChangeSubBand( level,
          1, (*pyramid)[ py_sb00_index + 1 ] ), "Unable to copy pyramid data at"
          "level " + Te2String( level ) + " sub-band 01" );
        TEAGN_TRUE_OR_RETURN( ChangeSubBand( level,
          2, (*pyramid)[ py_sb00_index + 2 ] ), "Unable to copy pyramid data at"
          "level " + Te2String( level ) + " sub-band 10" );
        TEAGN_TRUE_OR_RETURN( ChangeSubBand( level,
          3, (*pyramid)[ py_sb00_index + 3 ] ), "Unable to copy pyramid data at"
          "level " + Te2String( level ) + " sub-band 11" );
      }
      
      TEAGN_TRUE_OR_RETURN( Recompose( pyramid_.size() - 1, params_ ),
        "Recomposition error" ); 
        
      pyramid_.clear();     
    } else {
      TEAGN_LOG_AND_THROW( "Invalid filter task" );
    }    
  
  return true;
}




void TePDIMallatWavelets::ResetState( const TePDIParameters& )
{
  shift_histogram_flag_ = false;
  input_image_mean_ = 0;
}


bool TePDIMallatWavelets::Decompose( const TePDIParameters& params )
{
  /* Retriving parameters */
    
  TePDITypes::TePDIRasterPtrType input_image;
  params.GetParameter( "input_image", input_image );
  
  int band;
  params.GetParameter( "band", band );  
  
  int levels = 0;
  params.GetParameter( "levels", levels );
  
  /* Guessing dummy use */
  
  bool input_image_uses_dummy = input_image->params().useDummy_;
  double input_image_dummy = 0;
  if( input_image_uses_dummy ) {
    input_image_dummy = input_image->params().dummy_[ band ];
  }
  
  /* Generationg subband pyramid level 0 */
    
  const unsigned int inputlines = input_image->params().nlines_;
  const unsigned int inputcolumns = input_image->params().ncols_;
  
  pyramid_.clear();
    
  TePDIMatrix<double> subband0;
  
  TePDIMatrix< TePDIMatrix< double > > levelzeromatrix; 
  TEAGN_TRUE_OR_THROW( levelzeromatrix.Reset( 1, 1 ), "Matrix reset error" );
  levelzeromatrix( 0, 0 ) = subband0;
  
  pyramid_.push_back( levelzeromatrix );
  
  TEAGN_TRUE_OR_RETURN( 
    pyramid_[ 0 ]( 0, 0 ).Reset( inputlines, inputcolumns,
    TePDIMatrix<double>::AutoMemPol ),
    "Unable to create level 0 sub-band" );
    
  double pixel_value;
  TePDIMatrix< double >& l0matrix = pyramid_[ 0 ]( 0, 0 );
  
  input_image_mean_ = 0;
  
  unsigned int inputline;
  unsigned int inputcolumn;
  
  for( inputline = 0 ; inputline < inputlines ; ++inputline ) {
    for( inputcolumn = 0 ; inputcolumn < inputcolumns ; ++inputcolumn ) {
      if( input_image->getElement( inputcolumn, inputline, pixel_value, 
          band ) ) {
  
        l0matrix( inputline, inputcolumn ) = pixel_value;
        input_image_mean_ += pixel_value;
      } else {
        TEAGN_TRUE_OR_RETURN( input_image_uses_dummy, "Raster read error" );
      
        l0matrix( inputline, inputcolumn ) = input_image_dummy;
        input_image_mean_ += input_image_dummy;        
      }
    }
  }
  
  input_image_mean_ = 
    input_image_mean_ / ( (double)( inputlines * inputcolumns ) );  
  
  if( shift_histogram_flag_ ) {
  
    /* Removing mean */  
    
    for( inputline = 0 ; inputline < inputlines ; ++inputline ) {
      for( inputcolumn = 0 ; inputcolumn < inputcolumns ; ++inputcolumn ) {
        l0matrix( inputline, inputcolumn ) = 
        l0matrix( inputline, inputcolumn ) - input_image_mean_;
      }
    }  
  }
  
  /* Generating the other subband pyramid levels */
  
  for( int level = 1 ; level <= levels ; ++level ) {
    TEAGN_TRUE_OR_RETURN( GenerateLevel( level, params ),
      "Unable to generate level " + 
      Te2String( level ) );
  }
  
  return true;  
}


bool TePDIMallatWavelets::Recompose( unsigned int ref_level,
  const TePDIParameters& params )
{
  TePDITypes::TePDIRasterPtrType input_image;
  if( params.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "input_image" ) ) {
      
    params.GetParameter( "input_image", input_image );    
  }

  TEAGN_TRUE_OR_RETURN( ( ( pyramid_.size() > ref_level ) &&
    ( ref_level > 0 ) ), 
    "Invalid reference level" );
    
  /* Retriving the systesis filters */
  
  
  std::vector< double > s_filter_l;
  std::vector< double > s_filter_h;
  int lowshift;
  int highshift;
  
  TEAGN_TRUE_OR_RETURN( GetSynthesisFilters( s_filter_l, s_filter_h,
    lowshift, highshift, params ),
    "Unable to get synthesis filters" );  
  
  /* Reconstruction of levels */
  
  TePDIMatrix< double > rec_result;
  
  {
    TePDIMatrix< double > temp_llsb;
    
    unsigned int llsblines;
    unsigned int llsbcols;
    
    TePDIPIManager progress( "Recomposing levels...", ref_level,
      progress_enabled_ );
    
    for( unsigned int level = ref_level ; level != 0 ; --level ) {
      TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    
      if( level == 1 ) {
        if( pyramid_[ 0 ]( 0, 0 ).IsEmpty() ) {
          llsblines = ( ( pyramid_[ 1 ]( 0, 0 ).GetLines() - 1 ) * 2 ) + 1;
          llsbcols = ( ( pyramid_[ 1 ]( 0, 0 ).GetColumns() - 1 ) * 2 ) + 1;
        } else {
          llsblines = pyramid_[ 0 ]( 0, 0 ).GetLines();
          llsbcols = pyramid_[ 0 ]( 0, 0 ).GetColumns();
        }
      } else {
        llsblines = pyramid_[ level - 1 ]( 0, 1 ).GetLines();
        llsbcols = pyramid_[ level - 1 ]( 0, 1 ).GetColumns();
      }
      
      TEAGN_TRUE_OR_RETURN( temp_llsb.Reset( llsblines, llsbcols,
        TePDIMatrix<double>::AutoMemPol ),
        "Unable to allocate memory for low-low sub-band reconstruction "
        "at level " + Te2String( level ) );    
      
      if( level == ref_level ) {
        TEAGN_TRUE_OR_RETURN( ReconstructLL( pyramid_[ level ]( 0, 0 ), 
          pyramid_[ level ]( 0, 1 ), pyramid_[ level ]( 1, 0 ),
          pyramid_[ level ]( 1, 1 ), s_filter_l, 
          s_filter_h, lowshift, highshift, temp_llsb ),
          "Low-Low sub-band reconstruction error at level "
          + Te2String( level ) );
      } else {
        TEAGN_TRUE_OR_RETURN( ReconstructLL( rec_result, 
          pyramid_[ level ]( 0, 1 ), pyramid_[ level ]( 1, 0 ),
          pyramid_[ level ]( 1, 1 ), s_filter_l, 
          s_filter_h, lowshift, highshift, temp_llsb ),
          "Low-Low sub-band reconstruction error at level "
          + Te2String( level ) ); 
      }
      
      rec_result = temp_llsb;
    }
    
    progress.Toggle( false );
  }
  
  /* Retriving output raster reference from parameters */
  
  TePDITypes::TePDIRasterPtrType output_image;
  params.GetParameter( "output_image", output_image );
    
  /* Updating output raster geo parameters if available */
  
  if( input_image.isActive() ) {
    TeRasterParams output_image_params = output_image->params();
    
    output_image_params.nBands( 1 );
    
    if( input_image->projection() != 0 ) {
      TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
        input_image->projection()->params() ) );    
      output_image_params.projection( proj.nakedPointer() );
    }    
    
    output_image_params.boxResolution( input_image->params().box().x1(), 
      input_image->params().box().y1(), input_image->params().box().x2(), 
      input_image->params().box().y2(), 
      input_image->params().resx_, input_image->params().resy_ );    
    
    TEAGN_TRUE_OR_RETURN( output_image->init( output_image_params ),
      "Output raster reset error" );
  } else {
    TeRasterParams output_image_params = output_image->params();
    
    output_image_params.nBands( 1 );
    
    output_image_params.setNLinesNColumns( rec_result.GetLines(),
      rec_result.GetColumns() );
    
    TEAGN_TRUE_OR_RETURN( output_image->init( output_image_params ),
      "Output raster reset error" );  
  }
    
  double channel_min_level = 0;
  double channel_max_level = 0;
  
  TEAGN_TRUE_OR_RETURN( TePDIUtils::TeGetRasterMinMaxBounds(
    output_image, 0, channel_min_level,
    channel_max_level ), "Unable to get output raster channel level bounds" );    
 
  unsigned int line, column;
  double value;
  
  for( line = 0 ; line < rec_result.GetLines() ; ++line ) {
    for( column = 0 ; column < rec_result.GetColumns() ; ++column ) {
      value = rec_result( line, column );
      
      if( shift_histogram_flag_ ) {
        value += input_image_mean_;
      }
      
      /* Level range filtering */

      if( value < channel_min_level ) {
        value = channel_min_level;
      } else if( value > channel_max_level ) {
        value = channel_max_level;
      }  
            
      TEAGN_TRUE_OR_RETURN( 
        output_image->setElement( column, line, value, 0 ),
        "Unable to set output raster element" );
    }
  }  
  
  return true;
}


bool TePDIMallatWavelets::GetSubBand( unsigned int level, 
  unsigned int subband, 
  const TePDITypes::TePDIRasterPtrType& sb_raster ) const
{
  TEAGN_TRUE_OR_RETURN( pyramid_.size() > level, 
    "Invalid level number" );
  TEAGN_TRUE_OR_RETURN( sb_raster.isActive(), 
    "Inactive sub-band raster" );
    
    
  TePDIMatrix< double >* sb_ptr = 0;
    
  switch( subband ) {
    case 0 :
    {
      sb_ptr = &( pyramid_[ level ]( 0 , 0 ) );
      break;
    }
    case 1 :
    {
      sb_ptr = &( pyramid_[ level ]( 0 , 1 ) );
      break;
    }
    case 2 :
    {
      sb_ptr = &( pyramid_[ level ]( 1 , 0 ) );
      break;
    }
    case 3 :
    {
      sb_ptr = &( pyramid_[ level ]( 1 , 1 ) );
      break;
    }
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid subband" );
    }
  }
  
  unsigned int lines = sb_ptr->GetLines();
  unsigned int columns = sb_ptr->GetColumns();
      
  TeRasterParams sb_raster_params = sb_raster->params();
  sb_raster_params.setDataType( TeDOUBLE, -1 );
  sb_raster_params.nBands( 1 );
  sb_raster_params.setNLinesNColumns( lines, columns );
  TEAGN_TRUE_OR_RETURN( sb_raster->init( sb_raster_params ),
    "Unable to reset sb_raster" );   
        
  unsigned int line;
  unsigned int col;
      
  for( line = 0; line < lines ; ++line ) {
    for( col = 0; col < columns ; ++col ) {
      TEAGN_TRUE_OR_RETURN( sb_raster->setElement( col, line,
        (*sb_ptr)( line, col ), 0 ),
        "Unable to copy pixel value into low-low output raster" );
    }
  }    
  
  return true;
}


bool TePDIMallatWavelets::ChangeSubBand( 
  unsigned int level, unsigned int subband_index,
  const TePDITypes::TePDIRasterPtrType& newsband )
{
  TEAGN_TRUE_OR_RETURN( pyramid_.size() > level, "Invalid level number" );
  TEAGN_TRUE_OR_RETURN( newsband.isActive(), 
    "The new sub-band is inactive" );
    
  TePDIMatrix< double >* target_sb = 0;
    
  switch( subband_index ) {
    case 0 :
    {
      target_sb = &( pyramid_[ level ]( 0, 0 ) );
      
      break;
    }
    case 1 :
    {
      target_sb = &( pyramid_[ level ]( 0, 1 ) );
      
      break;

    }
    case 2 :
    {
      target_sb = &( pyramid_[ level ]( 1, 0 ) );
      
      break;

    }
    case 3 :
    {
      target_sb = &( pyramid_[ level ]( 1, 1 ) );
      
      break;

    }
    default :
    {
      TEAGN_LOG_AND_RETURN( "Invalid subband" );
    }
  }
  
  const int sblines = (int)target_sb->GetLines();
  const int sbcols = (int)target_sb->GetColumns();
  const int raster_lines = newsband->params().nlines_;
  const int raster_cols = newsband->params().ncols_;  
  
  TEAGN_TRUE_OR_RETURN( 
    ( ( sblines == raster_lines ) && ( sbcols == raster_cols ) ),
    "Incompatible sub-band dimensions" );  
  
  int line;
  int col;
      
  for( line = 0; line < sblines ; ++line ) {
    for( col = 0; col < sbcols ; ++col ) {
      if( ( line < raster_lines ) && ( col < raster_cols ) ) {
        TEAGN_TRUE_OR_RETURN( newsband->getElement( col, line,
          (*target_sb)( line, col ), 0 ),
          "Unable to copy pixel value from input raster" );
      } else {
        (*target_sb)( line, col ) = 0;
      }
    }
  }    
  
  return true;
}


bool TePDIMallatWavelets::GenerateLevel( unsigned int level,
  const TePDIParameters& params )
{
  TEAGN_TRUE_OR_RETURN( level > 0, "Invalid level" );
  TEAGN_TRUE_OR_RETURN( pyramid_.size() >= level, 
    "Not enought generated levels to generate the new required level" );
    
  const unsigned int subblines = ( unsigned int )
    ceil( ( (double)pyramid_[ level - 1 ]( 0 , 0 ).GetLines() ) / 2 );
  const unsigned int subbcolumns = ( unsigned int )
    ceil( ( (double)pyramid_[ level - 1 ]( 0, 0 ).GetColumns() ) / 2 );

  /* Deleting all higher levels */
     
  while( pyramid_.size() > ( level + 1 ) ) {
    pyramid_.pop_back();
  }
  
  /* Allocating space for the new level, if it doesn't exists */
  
  TEAGN_TRUE_OR_RETURN( AllocateTopLevel( level, subblines, subbcolumns ),
    "Unable to allocate memory for the new level" );
    
  /* Retriving the analysis filters */
  
  std::vector< double > a_filter_l;
  std::vector< double > a_filter_h;
  int lowshift = 0;
  int highshift = 0;
  
  TEAGN_TRUE_OR_RETURN( GetAnalysisFilters( a_filter_l, a_filter_h,
     lowshift, highshift, params ), "Unable to retrive analysis filters" );
  
  /* Applying filters */
  
  TePDIMatrix< double > temp_sb;
  TEAGN_TRUE_OR_RETURN( 
    temp_sb.Reset( pyramid_[ level - 1 ]( 0 , 0 ).GetLines(), subbcolumns,
    TePDIMatrix<double>::AutoMemPol ),
    "Unable to create temporary sub-band" );
    
  TePDIPIManager progress( "Generating sub-bands for level " + 
    Te2String( level ), 8, progress_enabled_ );
    
  ApplyHorizontalFilter( 
    pyramid_[ level - 1 ]( 0, 0 ), a_filter_l, lowshift, temp_sb );
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
  ApplyVerticalFilter( 
    temp_sb, a_filter_l, lowshift, pyramid_[ level ]( 0, 0 ) );
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    
  ApplyHorizontalFilter( 
    pyramid_[ level - 1 ]( 0, 0 ), a_filter_l, lowshift, temp_sb );
  progress.Increment();
  ApplyVerticalFilter( 
    temp_sb, a_filter_h, highshift, pyramid_[ level ]( 0, 1 ) );
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
      
  ApplyHorizontalFilter( 
    pyramid_[ level - 1 ]( 0, 0 ), a_filter_h, highshift, temp_sb );
  progress.Increment();
  ApplyVerticalFilter( 
    temp_sb, a_filter_l, lowshift, pyramid_[ level ]( 1, 0 ) );
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );

  ApplyHorizontalFilter( 
    pyramid_[ level - 1 ]( 0, 0 ), a_filter_h, highshift, temp_sb );
  progress.Increment();
  ApplyVerticalFilter( 
    temp_sb, a_filter_h, highshift, pyramid_[ level ]( 1, 1 ) );
  TEAGN_FALSE_OR_RETURN( progress.Increment(), "Canceled by the user" );
    
  progress.Toggle( false );
  
  return true;
}


void TePDIMallatWavelets::ApplyVerticalFilter( 
  const TePDIMatrix< double >& inMatrix, 
  const std::vector< double >& filter,
  int filter_shift,
  TePDIMatrix< double >& outMatrix ) const
{
  TEAGN_TRUE_OR_THROW( inMatrix.GetLines() > outMatrix.GetLines(),
    "inMatrix lines number must be greater than outMatrix lines number" );
  TEAGN_TRUE_OR_THROW( inMatrix.GetColumns() == outMatrix.GetColumns(),
    "inMatrix columns != outMatrix columns" );
  TEAGN_TRUE_OR_THROW( inMatrix.GetLines() > filter.size(),
    "inMatrix lines < filter size" );
    
  const unsigned int filter_size = filter.size();
  const unsigned int makeeven = filter_shift % 2;
  const unsigned int in_lines = inMatrix.GetLines();
  const unsigned int in_columns = inMatrix.GetColumns();
  const unsigned int vitual_in_lines_start = 
    ( filter_size - 1 );/* The index where start the apply of
    filter index 0 */
  const unsigned int vitual_in_lines_bound = 
    in_lines + vitual_in_lines_start;
  
  unsigned int virtual_in_line;
  unsigned int in_column;
  unsigned filter_index;
  double out_level;
  
  for( in_column = 0 ; in_column < in_columns ; ++in_column ) {
       
    for( virtual_in_line = vitual_in_lines_start + makeeven; 
         virtual_in_line < vitual_in_lines_bound ; 
         virtual_in_line += 2 ) {
         
      out_level = 0;  
    
      for( filter_index = 0 ; filter_index < filter_size ; ++filter_index ) {
        
        out_level +=
          inMatrix( ( virtual_in_line - filter_index ) % in_lines, 
          in_column ) * filter[ filter_index ];
      }
      
      outMatrix( ( ( virtual_in_line - vitual_in_lines_start + 
        filter_shift ) % in_lines ) / 2, in_column ) = out_level;
    }
  }
}


void TePDIMallatWavelets::ApplyHorizontalFilter( 
  const TePDIMatrix< double >& inMatrix,
  const std::vector< double >& filter,
  int filter_shift,
  TePDIMatrix< double >& outMatrix ) const
{
  TEAGN_TRUE_OR_THROW( inMatrix.GetColumns() > outMatrix.GetColumns(),
    "inMatrix columns  < outMatrix columns" );
  TEAGN_TRUE_OR_THROW( inMatrix.GetLines() == outMatrix.GetLines(),
    "inMatrix lines != outMatrix lines" );
  TEAGN_TRUE_OR_THROW( inMatrix.GetColumns() > filter.size(),
    "inMatrix columns < filter size" );
    
  const unsigned int filter_size = filter.size();
  const unsigned int makeeven = filter_shift % 2;
  const unsigned int in_lines = inMatrix.GetLines();
  const unsigned int in_columns = inMatrix.GetColumns();
  const unsigned int virtual_in_column_start = 
    ( filter_size - 1 ) ;/* The index where start the apply of
    filter index 0 */
  const unsigned int virtual_in_columns_bound = 
    in_columns + virtual_in_column_start;

  unsigned virtual_in_column;  
  unsigned int in_line;
  unsigned int filter_index;
  double out_level;
  
  for( in_line = 0 ; in_line < in_lines ; ++in_line ) {
       
    for( virtual_in_column = virtual_in_column_start + makeeven; 
         virtual_in_column < virtual_in_columns_bound ; 
         virtual_in_column += 2 ) {
         
      out_level = 0;  
    
      for( filter_index = 0 ; filter_index < filter_size ; ++filter_index ) {
        
        out_level += 
          inMatrix( in_line, ( virtual_in_column -  
            filter_index ) % in_columns ) * filter[ filter_index ];
      }
      
      outMatrix( in_line, 
        ( ( virtual_in_column - virtual_in_column_start + 
        filter_shift ) % in_columns ) / 2 ) = out_level;
    }
  }
}


bool TePDIMallatWavelets::BuildSuperimposedLevel( unsigned int level,
  TePDIMatrix< double >& level_matrix, bool normalize, double min_norm_level, 
  double max_norm_level ) const
{
  TEAGN_TRUE_OR_RETURN( ( ! level_matrix.IsEmpty() ), 
    "level_matrix is empty" );
  TEAGN_TRUE_OR_RETURN( ( level > 0 ) && ( level < pyramid_.size() ),
    "Invalid level" );
    
  if( level == ( pyramid_.size() - 1 ) ) {
    if( normalize ) {
      TePDIMatrix< double > sb00;
      TEAGN_TRUE_OR_RETURN( sb00.Reset( pyramid_[ level ]( 0, 0 ).GetLines(),
        pyramid_[ level ]( 0, 0 ).GetColumns() ), "Matrix reset error" );
        
      TePDIMatrix< double > sb01;
      TEAGN_TRUE_OR_RETURN( sb01.Reset( pyramid_[ level ]( 0, 1 ).GetLines(),
        pyramid_[ level ]( 0, 1 ).GetColumns() ), "Matrix reset error" );
                
      TePDIMatrix< double > sb10;
      TEAGN_TRUE_OR_RETURN( sb10.Reset( pyramid_[ level ]( 1, 0 ).GetLines(),
        pyramid_[ level ]( 1, 0 ).GetColumns() ), "Matrix reset error" );        
        
      TePDIMatrix< double > sb11;
      TEAGN_TRUE_OR_RETURN( sb11.Reset( pyramid_[ level ]( 1, 1 ).GetLines(),
        pyramid_[ level ]( 1, 1 ).GetColumns() ), "Matrix reset error" );        
      
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 0, 0 ), sb00,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 0, 1 ), sb01,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 1, 0 ), sb10,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 1, 1 ), sb11,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      
      TEAGN_TRUE_OR_RETURN( AgregateSubBands( sb00, sb01, sb10, sb11,
        level_matrix ), "Unable to agregate sub-bands at level " +
        Te2String( level ) );
    } else {
      TEAGN_TRUE_OR_RETURN( AgregateSubBands( pyramid_[ level ]( 0, 0 ),
        pyramid_[ level ]( 0, 1 ), pyramid_[ level ]( 1, 0 ),
        pyramid_[ level ]( 1, 1 ), level_matrix ), 
        "Unable to agregate sub-bands at level " +
        Te2String( level ) );    
    };
      
    return true;
  } else {
    TePDIMatrix< double > sb00;
    TEAGN_TRUE_OR_RETURN( sb00.Reset( pyramid_[ level ]( 0, 1 ).GetLines(),
      pyramid_[ level ]( 0, 1 ).GetColumns(), 
      TePDIMatrix<double>::AutoMemPol ),
      "Unable to allocate memory for sub-band [0,0] at level " +
      Te2String( level ) );
    
    TEAGN_TRUE_OR_RETURN( BuildSuperimposedLevel( level + 1, sb00, normalize,
      min_norm_level, max_norm_level ),
      "Unable to build superimposed level " +
      Te2String( level ) );
      
    if( normalize ) {
      TePDIMatrix< double > sb01;
      TEAGN_TRUE_OR_RETURN( sb01.Reset( pyramid_[ level ]( 0, 1 ).GetLines(),
        pyramid_[ level ]( 0, 1 ).GetColumns() ), "Matrix reset error" );         
        
      TePDIMatrix< double > sb10;
      TEAGN_TRUE_OR_RETURN( sb10.Reset( pyramid_[ level ]( 1, 0 ).GetLines(),
        pyramid_[ level ]( 1, 0 ).GetColumns() ), "Matrix reset error" );         
        
      TePDIMatrix< double > sb11;
      TEAGN_TRUE_OR_RETURN( sb11.Reset( pyramid_[ level ]( 1, 1 ).GetLines(),
        pyramid_[ level ]( 1, 1 ).GetColumns() ), "Matrix reset error" );         
      
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 0, 1 ), sb01,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 1, 0 ), sb10,
        min_norm_level, max_norm_level ), "Matrix normalization error" );
      TEAGN_TRUE_OR_RETURN( NormalizeMatrix( 
        pyramid_[ level ]( 1, 1 ), sb11,
        min_norm_level, max_norm_level ), "Matrix normalization error" );      
      
      TEAGN_TRUE_OR_RETURN( AgregateSubBands( sb00, sb01, sb10, sb11,
        level_matrix ), "Unable to agregate sub-bands at level " +
        Te2String( level ) );
    } else {
      TEAGN_TRUE_OR_RETURN( AgregateSubBands( sb00, pyramid_[ level ]( 0, 1 ),
        pyramid_[ level ]( 1, 0 ), pyramid_[ level ]( 1, 1 ), level_matrix ),
        "Unable to agregate sub-bands at level " +
        Te2String( level ) );      
    }  
      
    return true;
  }
}


bool TePDIMallatWavelets::AgregateSubBands(
  const TePDIMatrix< double >& upper_left,
  const TePDIMatrix< double >& upper_right,
  const TePDIMatrix< double >& lower_left,
  const TePDIMatrix< double >& lower_right,
  TePDIMatrix< double >& result ) const
{
  TEAGN_TRUE_OR_RETURN( 
    ( ! result.IsEmpty() ), "result matrix is empty" );

  TEAGN_CHECK_EPS( upper_left.GetColumns(), lower_left.GetColumns(), 0,
    "Size mismatch" );
  TEAGN_CHECK_EPS( upper_right.GetColumns(), lower_right.GetColumns(), 0,
    "Size mismatch" );
  TEAGN_CHECK_EPS( upper_left.GetLines(), upper_right.GetLines(), 0,
    "Size mismatch" );
  TEAGN_CHECK_EPS( lower_left.GetLines(), lower_right.GetLines(), 0,
    "Size mismatch" );
    
  unsigned int line = 0;
  unsigned int column = 0;
  const unsigned int lines_offset = upper_left.GetLines();
  const unsigned int columns_offset = upper_left.GetColumns();
  unsigned int lines_bound = 0;
  unsigned int columns_bound = 0;
  
  /* Copying the upper_left sub-band */

  lines_bound = 
    MIN( result.GetLines(), upper_left.GetLines() );
  columns_bound = 
    MIN( result.GetColumns(), upper_left.GetColumns() );
  
  for( line = 0 ; line < lines_bound ; ++line ) {
    for( column = 0 ; column < columns_bound ; ++column ) {
      result( line, column ) = upper_left( line, column );
    }
  }
  
  /* Copying the upper_right sub-band */
    
  if( columns_offset < result.GetColumns() ) {
    lines_bound = MIN( result.GetLines(), upper_right.GetLines() );
    columns_bound = MIN( result.GetColumns() - columns_offset, 
        upper_right.GetColumns() );  
    
    for( line = 0 ; line < lines_bound ; ++line ) {
      for( column = 0 ; column < columns_bound ; ++column ) {
        result( line, column + columns_offset ) = 
          upper_right( line, column );
      }
    }
  }
  
  /* Copying the lower_left sub-band */
  
  if( lines_offset < result.GetLines() ) {
    lines_bound = 
      MIN( result.GetLines() - lines_offset, 
      lower_left.GetLines() );
    columns_bound =
      MIN( result.GetColumns(), lower_left.GetColumns() );   
    
    for( line = 0 ; line < lines_bound ; ++line ) {
      for( column = 0 ; column < columns_bound ; ++column ) {
        result( line + lines_offset, column ) = 
          lower_left( line, column );
      }
    }
  }
  
  /* Copying the lower_right sub-band */
    
  if( ( lines_offset < result.GetLines() ) &&
      ( columns_offset < result.GetColumns() ) ) {
  
    lines_bound = MIN( 
      result.GetLines() - lines_offset, lower_right.GetLines() );
    columns_bound = MIN( 
      result.GetColumns() - columns_offset, lower_right.GetColumns() );    
    
    for( line = 0 ; line < lines_bound ; ++line ) {
      for( column = 0 ; column < columns_bound ; ++column ) {
        result( line + lines_offset, column + columns_offset ) = 
          lower_right( line, column );
      }
    }
  }
  
  return true;          
}


bool TePDIMallatWavelets::PushLevel( unsigned int level, 
  const TePDIMatrix< double >& sb0, const TePDIMatrix< double >& sb1, 
  const TePDIMatrix< double >& sb2, const TePDIMatrix< double >& sb3 )
{
  TEAGN_TRUE_OR_RETURN( level > 0, "Invalid level" );
  
  TEAGN_TRUE_OR_RETURN( 
    ( sb0.GetLines() == sb1.GetLines() ) &&
    ( sb1.GetLines() == sb2.GetLines() ) &&
    ( sb2.GetLines() == sb3.GetLines() ) &&
    ( sb0.GetColumns() == sb1.GetColumns() ) &&
    ( sb1.GetColumns() == sb2.GetColumns() ) &&
    ( sb2.GetColumns() == sb3.GetColumns() ),
    "All sub-bands must have the same dimensions" );
    
  TEAGN_TRUE_OR_RETURN( AllocateTopLevel( level, sb0.GetLines(), 
    sb0.GetColumns() ), "Unable to allocate memory for the new level" );
  
  pyramid_[ level ]( 0, 0 ) = sb0;
  pyramid_[ level ]( 0, 1 ) = sb1;
  pyramid_[ level ]( 1, 0 ) = sb2;
  pyramid_[ level ]( 1, 1 ) = sb3;
  
  return true;
}


bool TePDIMallatWavelets::AllocateTopLevel( unsigned int level, 
  unsigned int sblines, unsigned int sbcolumns )
{
  /* Checking if the required levels already exists */

  if( pyramid_.size() > 1 ) {
    TEAGN_TRUE_OR_RETURN( 
      ( ( level == pyramid_.size() ) || ( ( level + 1 ) == pyramid_.size() ) ),
      "Invalid level" );
  
    if( ( level + 1 ) == pyramid_.size() ) {
      if( ( pyramid_[ level ]( 0, 0 ).GetLines() == sblines ) &&
        ( pyramid_[ level ]( 0, 0 ).GetColumns() == sbcolumns ) ) {
        
        return true;
      }
    }
  } else {
    TEAGN_TRUE_OR_RETURN( level == 1, "Invalid level" );
  }
  
  /* Creating a dummy level 0 if necessary */
  
  if( pyramid_.size() == 0 ) {
    TePDIMatrix<double> subband0;
  
    TePDIMatrix< TePDIMatrix< double > > levelzeromatrix; 
    TEAGN_TRUE_OR_RETURN( levelzeromatrix.Reset( 1, 1 ), 
      "Matrix reset error" );
    levelzeromatrix( 0, 0 ) = subband0;
  
    pyramid_.push_back( levelzeromatrix );  
  }
  
  /* Checking if the supplied level dimensions are correct */
  
  if( level > 1 ) {
    sblines = ( unsigned int )
      ceil( ( (double)pyramid_[ level - 1 ]( 0 , 0 ).GetLines() ) / 2 );
    sbcolumns = ( unsigned int )
      ceil( ( (double)pyramid_[ level - 1 ]( 0, 0 ).GetColumns() ) / 2 );           
  } else {
    TEAGN_TRUE_OR_RETURN( ( sblines > 0 ) && ( sbcolumns > 0 ),
      "Number of lines and number of columns must not be zero" );
  }
  
  /* Allocating the new level */
      
  TePDIMatrix< TePDIMatrix< double > > newlevel;
  pyramid_.push_back( newlevel );
  
  if( ! pyramid_[ level ].Reset( 2, 2 ) ) {
    pyramid_.pop_back();
    TEAGN_LOG_AND_RETURN( 
      "Unable to create level " + Te2String( level ) );
  }
  
  /* Allocating the new sub-bands for the new level */
  
  TePDIMatrix< double > tempsb;
  
  pyramid_[ level ]( 0, 0 ) = tempsb;
  pyramid_[ level ]( 0, 1 ) = tempsb;
  pyramid_[ level ]( 1, 0 ) = tempsb;
  pyramid_[ level ]( 1, 1 ) = tempsb;
  
    
  if( pyramid_[ level ]( 0, 0 ).Reset( sblines, sbcolumns, 
    TePDIMatrix<double>::AutoMemPol ) &&
    pyramid_[ level ]( 0, 1 ).Reset( sblines, sbcolumns, 
    TePDIMatrix<double>::AutoMemPol ) &&
    pyramid_[ level ]( 1, 0 ).Reset( sblines, sbcolumns,
    TePDIMatrix<double>::AutoMemPol ) &&
    pyramid_[ level ]( 1, 1 ).Reset( sblines, sbcolumns,
    TePDIMatrix<double>::AutoMemPol ) ) {
    
    /* Fill the new level with 0's */
    {
      TePDIMatrix< double >& sb00_ref = pyramid_[ level ]( 0, 0 );
      TePDIMatrix< double >& sb01_ref = pyramid_[ level ]( 0, 1 );
      TePDIMatrix< double >& sb10_ref = pyramid_[ level ]( 1, 0 );
      TePDIMatrix< double >& sb11_ref = pyramid_[ level ]( 1, 1 );
    
      unsigned int line;
      unsigned int col;
      
      for( line = 0 ; line < sblines ; ++line ) {
        for( col = 0 ; col < sbcolumns ; ++col ) {
          sb00_ref( line, col ) = 0;
          sb01_ref( line, col ) = 0;
          sb10_ref( line, col ) = 0;
          sb11_ref( line, col ) = 0;
        }
      }
    }
    
    return true;    
  } else {
    pyramid_.pop_back();
    TEAGN_LOG_AND_RETURN( "Unable to allocate all sub-bands into pyramid" );
  }
}


bool TePDIMallatWavelets::GetAFiltersShift( 
  const std::vector< double >& a_lowfilter,
  const std::vector< double >& a_highfilter,
  int& lowshift, int& highshift ) const
{
  const unsigned int fsizel = a_lowfilter.size();
  const unsigned int fsizeh = a_highfilter.size();
  
  TEAGN_TRUE_OR_RETURN( fsizel > 1, "Low Filter size < 2" );
  TEAGN_TRUE_OR_RETURN( fsizeh > 1, "High Filter size < 2" );
  
  lowshift = ( fsizel - 1 ) / 2;
  highshift = ( fsizeh - 1 ) / 2;
  
  if( lowshift <= highshift ) {
    highshift = lowshift;
  } 
    
  if( lowshift == 0 ) lowshift = 1;
  if( highshift == 0 ) highshift = 1;
  
  /* This is for odd symstric filters with different lenghts */
  
  if( ( ( lowshift % 2 ) != 0 ) && ( ( highshift % 2 ) == 0 ) ) {
    --lowshift;
  }
  
  if( ( ( lowshift % 2 ) == 0 ) && ( ( highshift % 2 ) != 0 ) ) {
    --highshift;
  }
  
  /* This is for non-symmetrical filters */
  
  if( ( a_lowfilter[ 0 ] != a_lowfilter[ fsizel - 1 ] ) && 
    ( ( fsizel % 2 ) == 0 ) ) {
    
    /* updating the lowshift with the index of the higher absolute value 
       inside the low filter */
       
    bool hindex_found = false;
    double lowf_hav = 0; /* higher absolute value inside low filter */
    
    for( unsigned int lindex = 0 ; lindex < fsizel ; ++lindex ) {
      if( hindex_found ) {
        if( ABS( a_lowfilter[ lindex ] ) > lowf_hav ) {
          lowf_hav = ABS( a_lowfilter[ lindex ] );
          lowshift = lindex;
        }
      } else {
        hindex_found = true;
        lowf_hav = ABS( a_lowfilter[ lindex ] );
        lowshift = lindex;
      }
    }
    
    lowshift = fsizel - lowshift - 1;
    highshift = fsizel - lowshift - 1;
    
    if( ( lowshift % 2 ) != ( highshift % 2 ) ) {
      ++highshift;
    }
  }
  
  return true;
}


bool TePDIMallatWavelets::GetSFiltersShift( 
  const std::vector< double >& a_lowfilter,
  const std::vector< double >& a_highfilter,
  const std::vector< double >& s_lowfilter,
  const std::vector< double >& s_highfilter,
  int& lowshift, int& highshift ) const
{
  const unsigned int afsizel = a_lowfilter.size();
  const unsigned int afsizeh = a_highfilter.size();
  const unsigned int sfsizel = s_lowfilter.size();
  const unsigned int sfsizeh = s_highfilter.size();
  
  TEAGN_TRUE_OR_RETURN( afsizel > 1, "Analysis Low Filter size < 2" );
  TEAGN_TRUE_OR_RETURN( afsizeh > 1, "Analysis High Filter size < 2" );
  TEAGN_TRUE_OR_RETURN( sfsizel > 1, "Syntesis Low Filter size < 2" );
  TEAGN_TRUE_OR_RETURN( sfsizeh > 1, "Syntesis High Filter size < 2" );
  
  lowshift = ( sfsizel - 1 ) / 2;
  highshift = ( sfsizeh - 1 ) / 2;
    
  if( lowshift == 0 ) lowshift = 1;
  if( highshift == 0 ) highshift = 1;
  
  /* This is for odd symstric filters with different lenghts */
  
  if( ( ( lowshift % 2 ) != 0 ) && ( ( highshift % 2 ) == 0 ) ) {
    ++highshift;
  }
  
  if( ( ( lowshift % 2 ) == 0 ) && ( ( highshift % 2 ) != 0 ) ) {
    ++lowshift;
  }
  
  /* General case for even lenght filters */
  
  if( ( ( afsizel % 2 ) == 0 ) && ( ( afsizeh % 2 ) == 0 ) &&
      ( ( sfsizel % 2 ) == 0 ) && ( ( sfsizeh % 2 ) == 0 ) ) {
      
    ++lowshift;
    ++highshift;
  }
  
  /* This is for non-symmetrical filters */
  
  if( s_lowfilter[ 0 ] != s_lowfilter[ sfsizel - 1 ] ) {
    
    /* updating the lowshift with the index of the higher absolute value 
       inside the low filter */
    
    bool hav_found = false;
    double lowf_hav = 0; /* low filter highest value */
    
    for( unsigned int lindex = 0 ; lindex < sfsizel ; ++lindex ) {
      if( hav_found ) {
        if( ABS( s_lowfilter[ lindex ] ) > lowf_hav ) {
          lowf_hav = ABS( s_lowfilter[ lindex ] );
          lowshift = lindex;
        }
      } else {
        hav_found = true;
        lowf_hav = ABS( s_lowfilter[ lindex ] );
        lowshift = lindex;
      }
    }
    
    lowshift = sfsizel - lowshift - 1;
    highshift = sfsizel - lowshift - 2;
    
    if( ( lowshift % 2 ) != ( highshift % 2 ) ) {
      --highshift;
    }
  }
  
  return true;
}



bool TePDIMallatWavelets::ReconstructLL( 
  const TePDIMatrix< double >& sb00_in,
  const TePDIMatrix< double >& sb01_in,
  const TePDIMatrix< double >& sb10_in,
  const TePDIMatrix< double >& sb11_in,
  const std::vector< double >& s_low_filter,
  const std::vector< double >& s_high_filter,
  int lowshift, int highshift,
  TePDIMatrix< double >& sb00_out ) const
{
  TEAGN_TRUE_OR_RETURN( ( ! sb00_out.IsEmpty() ), 
    "Output sub-band ( 0 , 0 ) is empty" );
    
  TEAGN_TRUE_OR_RETURN( 
    ( ( sb00_in.GetLines() == sb01_in.GetLines() ) && 
      ( sb01_in.GetLines() == sb10_in.GetLines() ) &&
      ( sb10_in.GetLines() == sb11_in.GetLines() ) &&
      ( sb00_in.GetColumns() == sb01_in.GetColumns() ) &&
      ( sb01_in.GetColumns() == sb10_in.GetColumns() ) &&
      ( sb10_in.GetColumns() == sb11_in.GetColumns() ) ), 
    "Dimensions mismatch detected between input sub-bands" );
    
  /* Allocating space for temporary sub-bands */    
    
  TePDIMatrix< double > tempsb1;
  TEAGN_TRUE_OR_RETURN( tempsb1.Reset( sb00_out.GetLines(), 
    sb00_out.GetColumns(), TePDIMatrix<double>::AutoMemPol ), 
    "Unable to allocate memory for temporary sub-band 1" );
  
  TePDIMatrix< double > tempsb2;
  TEAGN_TRUE_OR_RETURN( tempsb2.Reset( sb00_out.GetLines(), 
    sb00_out.GetColumns(), TePDIMatrix<double>::AutoMemPol ), 
    "Unable to allocate memory for temporary sub-band 2" );
    
  /* Cleanning output data */
  
  for( unsigned int outline = 0 ; outline < sb00_out.GetLines() ; ++outline ) {
    for( unsigned int outcolumn = 0 ; outcolumn < sb00_out.GetColumns() ; 
      ++outcolumn ) {
  
       sb00_out( outline, outcolumn ) = 0.0;
    }
  }
    
  /* Processing sub-band [ 0, 0 ] */

  TEAGN_TRUE_OR_RETURN( UpSampleMatrix( sb00_in, tempsb1 ), 
    "Upsample error" );
  TEAGN_TRUE_OR_RETURN( ApplyHorizontalRFilter( 
    tempsb1, s_low_filter, lowshift, tempsb2 ), 
    "Horizontal filter apply error" );
  TEAGN_TRUE_OR_RETURN( ApplyVerticalRFilter( 
    tempsb2, s_low_filter, lowshift, tempsb1 ), 
    "Vertical filter apply error" );    
  TEAGN_TRUE_OR_RETURN( AddMatrixes( tempsb1, sb00_out, sb00_out ), 
    "Addition error" );    

  /* Processing sub-band [ 0, 1 ] */

  TEAGN_TRUE_OR_RETURN( UpSampleMatrix( sb01_in, tempsb1 ), 
    "Upsample error" );
  TEAGN_TRUE_OR_RETURN( ApplyHorizontalRFilter( 
    tempsb1, s_low_filter, lowshift, tempsb2 ), 
    "Horizontal filter apply error" );
  TEAGN_TRUE_OR_RETURN( ApplyVerticalRFilter( 
    tempsb2, s_high_filter, highshift, tempsb1 ), 
    "Vertical filter apply error" );    
  TEAGN_TRUE_OR_RETURN( AddMatrixes( tempsb1, sb00_out, sb00_out ), 
    "Addition error" );    

  /* Processing sub-band [ 1, 0 ] */

  TEAGN_TRUE_OR_RETURN( UpSampleMatrix( sb10_in, tempsb1 ), 
    "Upsample error" );
  TEAGN_TRUE_OR_RETURN( ApplyHorizontalRFilter( 
    tempsb1, s_high_filter, highshift, tempsb2 ), 
    "Horizontal filter apply error" );
  TEAGN_TRUE_OR_RETURN( ApplyVerticalRFilter( 
    tempsb2, s_low_filter, lowshift, tempsb1 ), 
    "Vertical filter apply error" );    
  TEAGN_TRUE_OR_RETURN( AddMatrixes( tempsb1, sb00_out, sb00_out ), 
    "Addition error" );      

  /* Processing sub-band [ 1, 1 ] */

  TEAGN_TRUE_OR_RETURN( UpSampleMatrix( sb11_in, tempsb1 ), 
    "Upsample error" );
  TEAGN_TRUE_OR_RETURN( ApplyHorizontalRFilter( 
    tempsb1, s_high_filter, highshift, tempsb2 ), 
    "Horizontal filter apply error" );
  TEAGN_TRUE_OR_RETURN( ApplyVerticalRFilter( 
    tempsb2, s_high_filter, highshift, tempsb1 ), 
    "Vertical filter apply error" );    
  TEAGN_TRUE_OR_RETURN( AddMatrixes( tempsb1, sb00_out, sb00_out ), 
    "Addition error" );

  return true;
}


bool TePDIMallatWavelets::UpSampleMatrix( const TePDIMatrix< double >& in,
  TePDIMatrix< double >& out ) const
{
  TEAGN_TRUE_OR_RETURN( ( ! out.IsEmpty() ), 
    "Invalid output matrix" );
    
  const unsigned int outlines = out.GetLines();
  const unsigned int outcolumns = out.GetColumns();
  
  const unsigned int inlines = in.GetLines();
  const unsigned int incolumns = in.GetColumns();
  
  const unsigned int required_out_lines = ( 2 * ( inlines - 1 ) ) + 1;
  const unsigned int required_out_columns = ( 2 * ( incolumns - 1 ) ) + 1;
  
  TEAGN_TRUE_OR_RETURN( ( outlines >= required_out_lines ), 
    "Insuficient output lines" );
  TEAGN_TRUE_OR_RETURN( ( outcolumns >= required_out_columns ), 
    "Insuficient output columns" );
    
  unsigned int line;
  unsigned int column;
  
  /* Cleanning output matrix */

  for( line = 0 ; line < outlines ; ++line ) {
    for( column = 0 ; column < outcolumns ; ++column ) {
      out( line, column ) = 0.;
    }
  }
    
  /* Transfering values to output */
  
  for( line = 0 ; line < inlines ; ++line ) {
    for( column = 0 ; column < incolumns ; ++column ) {
       out( line * 2, column * 2 ) = in( line, column ) * 4;
    }
  }   
  
  return true; 
}


bool TePDIMallatWavelets::AddMatrixes( const TePDIMatrix< double >& in1,
  const TePDIMatrix< double >& in2, TePDIMatrix< double >& res ) const
{
  TEAGN_TRUE_OR_RETURN( in1.GetLines() == in2.GetLines(),
    "in1 lines != in2 lines" );
  TEAGN_TRUE_OR_RETURN( in1.GetColumns() == in2.GetColumns(),
    "in1 columns != in2 columns" );
  
  TEAGN_TRUE_OR_RETURN( in1.GetLines() == res.GetLines(),
    "in1 lines != res lines" );
  TEAGN_TRUE_OR_RETURN( in1.GetColumns() == res.GetColumns(),
    "in1 columns != res columns" );
        
  const unsigned int lines = in1.GetLines();
  const unsigned int columns = in1.GetColumns();
  
  unsigned int line;
  unsigned int column;
  
  for( line = 0 ; line < lines ; ++line ) {
    for( column = 0 ; column < columns ; ++column ) {
      res( line, column ) = in1( line, column ) + in2( line, column );
    }
  } 
  
  return true;   
}


bool TePDIMallatWavelets::ApplyVerticalRFilter( 
  const TePDIMatrix< double >& inMatrix, 
  const std::vector< double >& filter,
  int filter_shift,
  TePDIMatrix< double >& outMatrix ) const
{
  TEAGN_TRUE_OR_RETURN( inMatrix.GetLines() == outMatrix.GetLines(),
    "inMatrix lines != outMatrix lines" );
  TEAGN_TRUE_OR_RETURN( inMatrix.GetColumns() == outMatrix.GetColumns(),
    "inMatrix columns != outMatrix columns" );
  TEAGN_TRUE_OR_RETURN( inMatrix.GetLines() > filter.size(),
    "inMatrix lines < filter size" );

  const unsigned int in_lines = inMatrix.GetLines();
  const unsigned int in_columns = inMatrix.GetColumns();
  const unsigned int filter_size = filter.size();
  unsigned int virtual_in_line_start = filter_size - 1;
  unsigned int virtual_in_lines_bound = in_lines + virtual_in_line_start;  
  
  unsigned int virtual_in_line;
  unsigned int in_column;
  unsigned int filter_index;
  double out_level;
  int ircli; /* input raster convolution line index */
  
  for( in_column = 0 ; in_column < in_columns ; ++in_column ) {
       
    for( virtual_in_line = virtual_in_line_start ; 
         virtual_in_line < virtual_in_lines_bound ; 
         ++virtual_in_line ) {
    
      out_level = 0;  
    
      for( filter_index = 0 ; filter_index < filter_size ; ++filter_index ) {
        
        ircli = ( virtual_in_line - filter_index ) % in_lines;
        
        if( ircli % 2 == 0 ) {
          out_level += 
            inMatrix( ircli, in_column ) * filter[ filter_index ];
        }
      }
      
      outMatrix( ( virtual_in_line - virtual_in_line_start + 
        filter_shift ) % in_lines, in_column ) = out_level;
    }
  }
  
  return true;
}


bool TePDIMallatWavelets::ApplyHorizontalRFilter( 
  const TePDIMatrix< double >& inMatrix,
  const std::vector< double >& filter,
  int filter_shift,
  TePDIMatrix< double >& outMatrix ) const
{
  TEAGN_TRUE_OR_RETURN( inMatrix.GetColumns() == outMatrix.GetColumns(),
    "inMatrix columns != outMatrix columns" );
  TEAGN_TRUE_OR_RETURN( inMatrix.GetLines() == outMatrix.GetLines(),
    "inMatrix lines != outMatrix lines" );
  TEAGN_TRUE_OR_RETURN( inMatrix.GetColumns() > filter.size(),
    "inMatrix columns < filter size" );

  const unsigned int in_lines = inMatrix.GetLines();
  const unsigned int in_columns = inMatrix.GetColumns();
  const unsigned int filter_size = filter.size();
  unsigned int virtual_in_column_start = filter_size - 1;
  unsigned int virtual_in_columns_bound = in_columns + virtual_in_column_start;
  
  unsigned int virtual_in_column;
  unsigned int in_line;
  unsigned int filter_index;
  double out_level;
  int ircci; /* input raster convolution column index */
  
  for( in_line = 0 ; in_line < in_lines ; in_line += 2 ) {
       
    for( virtual_in_column = virtual_in_column_start ; 
         virtual_in_column < virtual_in_columns_bound ; 
         ++virtual_in_column ) {
    
      out_level = 0;  
    
      for( filter_index = 0 ; filter_index < filter_size ; ++filter_index ) {
        
        ircci = ( virtual_in_column - filter_index ) % in_columns;
        
        if( ircci % 2 == 0 ) {
          out_level += 
            inMatrix( in_line, ircci ) * filter[ filter_index ];
        }
      }
      
      outMatrix( in_line, ( virtual_in_column - virtual_in_column_start + 
        filter_shift ) % in_columns ) = out_level;
    }
  }
  
  return true;
}


bool TePDIMallatWavelets::GetAnalysisFilters( std::vector< double >& low_filter,
  std::vector< double >& high_filter, int& low_shift, int& high_shift,
  const TePDIParameters& params ) const
{
  /* Retriving the analysis filters and normalizing */
  
  double filters_scale = 0;
  params.GetParameter( "filters_scale", filters_scale );  
  
  params.GetParameter( "a_filter_l", low_filter );
    
  params.GetParameter( "a_filter_h", high_filter );
  
  double factor = sqrt( 2.0 ) * sqrt( filters_scale );
  
  for( unsigned int a_filter_l_i = 0 ; a_filter_l_i < low_filter.size() ;
       ++a_filter_l_i ) {
       
       low_filter[ a_filter_l_i ] *= factor;
  }
  
  for( unsigned int a_filter_h_i = 0 ; a_filter_h_i < high_filter.size() ;
       ++a_filter_h_i ) {
       
       high_filter[ a_filter_h_i ] *= factor;
  }  
  
  /* Calculating filters shift */
  
  TEAGN_TRUE_OR_RETURN( GetAFiltersShift( low_filter, high_filter, low_shift, 
    high_shift ), "Unable to calculate filters shift" );
    
  return true;
}


bool TePDIMallatWavelets::GetSynthesisFilters( std::vector< double >& low_filter,
  std::vector< double >& high_filter, int& low_shift, int& high_shift,
  const TePDIParameters& params ) const
{
  /* Retriving the analysis filters */
  
  std::vector< double > a_low_filter;
  std::vector< double > a_high_filter;
  int a_low_shift;
  int a_high_shift;
  
  TEAGN_TRUE_OR_RETURN( GetAnalysisFilters( a_low_filter, a_high_filter,
    a_low_shift, a_high_shift, params ), "Unable to get analysis filters" );
    
  /* Retriving the synthesis filters */
  
  double filters_scale = 0;
  params.GetParameter( "filters_scale", filters_scale );  
  
  params.GetParameter( "s_filter_l", low_filter );
    
  params.GetParameter( "s_filter_h", high_filter );
  
  double factor = 1 / ( sqrt( 2.0 ) * sqrt( filters_scale ) );
  
  for( unsigned int s_filter_l_i = 0 ; s_filter_l_i < low_filter.size() ;
       ++s_filter_l_i ) {
       
       low_filter[ s_filter_l_i ] *= factor;
  }
  
  for( unsigned int s_filter_h_i = 0 ; s_filter_h_i < high_filter.size() ;
       ++s_filter_h_i ) {
       
       high_filter[ s_filter_h_i ] *= factor;
  }  
  
  /* Calculating filters shift */
  
  TEAGN_TRUE_OR_RETURN( GetSFiltersShift( a_low_filter, a_high_filter,
    low_filter, high_filter, low_shift, high_shift ), 
    "Unable to calculate filters shift" );
    
  return true;
}


bool TePDIMallatWavelets::NormalizeMatrix( 
    const TePDIMatrix< double >& input, 
    TePDIMatrix< double >& output, double min, double max ) const
{
  TEAGN_TRUE_OR_RETURN( min < max, "Invalid range" );
  TEAGN_TRUE_OR_RETURN( 
    ( ( ( max / 4. ) - ( min / 4. ) ) < ( DBL_MAX / 2. ) ),
    "min and max are out of allowed range" );
      
  TEAGN_TRUE_OR_RETURN( ( input.GetLines() == output.GetLines() ),
    "input lines != output lines" );
  TEAGN_TRUE_OR_RETURN( ( input.GetColumns() == output.GetColumns() ),
    "input columns != output columns" );
  
  const unsigned int lines = input.GetLines();
  const unsigned int columns = input.GetColumns();
    
  if( ( lines == 0 ) || ( columns == 0 ) ) {
    return true;
  }
    
  unsigned int line;
  unsigned int column;
  double value;   
    
  /* Finding the current min and max */
  
  double cmin = input( 0, 0 );
  double cmax = cmin;
    
  for( line = 0 ; line < lines ; ++line ) {
    for( column = 0 ; column < columns ; ++column ) {
      value = input( line, column );
      
      if( value < cmin ) {
        cmin = value;
      }
      if( value > cmax ) {
        cmax = value;
      }
    }
  }
    
  TEAGN_TRUE_OR_RETURN( 
    ( ( ( cmax / 4. ) - ( cmin / 4. ) ) < ( DBL_MAX / 2. ) ),
    "current image min and max are out of allowed range" );
    
  double offset = min - cmin;
  double scale = ( cmax - cmin ) / ( max - min );
    
  if( scale == 0 ) return true;
    
  /* Remapping values */
      
  for( line = 0 ; line < lines ; ++line ) {
    for( column = 0 ; column < columns ; ++column ) {
      output( line, column ) = ( input( line, column ) / scale ) + offset;
    }
  }
    
  return true;
}  



