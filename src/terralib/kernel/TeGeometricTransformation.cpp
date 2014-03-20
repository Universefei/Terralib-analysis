/*
TerraLib - a library for developing GIS applications.
Copyright  2001, 2002, 2003 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose. The library provided hereunder is on an "as is" basis, and the
authors have no obligation to provide maintenance, support, updates,
enhancements, or modifications.
In no event shall INPE be held liable to any party
for direct, indirect, special, incidental, or consequential damages arising
out of the use of this library and its documentation.
*/

#include "TeGeometryAlgorithms.h"
#include "TeGeometricTransformation.h"
#include "TeMutex.h"
#include "TeThreadFunctor.h"
#include "TeUtils.h"
#include "TeAgnostic.h"

#include <math.h>
#include <float.h>

#include <map>
#include <algorithm>

// This is a workaround since Visual C++ is so very very considerate and
// pollutes global namespace with macros named min and max.
#ifdef max
  #undef max
#endif
#ifdef min
  #undef min
#endif
#include <limits>

TeGeometricTransformation::TeGeometricTransformation()
{
}


TeGeometricTransformation::~TeGeometricTransformation()
{
}


void TeGeometricTransformation::getParameters( TeGTParams& params ) const
{
  params = internal_params_;
}

const TeGTParams& TeGeometricTransformation::getParameters() const
{
  return internal_params_;
}

bool TeGeometricTransformation::reset( const TeGTParams& newparams )
{
  /* If previous calculated parameters were supplied, no need to do calcules */

  if( isTransDefined( newparams ) ) 
  {
    internal_params_ = newparams;
    return true;
  } else {
    /* No previous parameters given - Need to calculate the new transformation
       parameters */
       
    const unsigned int req_tie_pts_nmb = getMinRequiredTiePoints();

    if( newparams.tiepoints_.size() < req_tie_pts_nmb ) 
    {
      return false;
    } 
    else if( newparams.tiepoints_.size() == req_tie_pts_nmb ) 
    {
      internal_params_ = newparams;
      return computeParameters( internal_params_ );
    }
    else 
    {
      internal_params_ = newparams;
      
      switch( newparams.out_rem_strat_ ) {
        case TeGTParams::NoOutRemotion :
        {
          if( computeParameters( internal_params_ ) ) {
            
            if( ( newparams.max_dmap_error_ >= 
              getDirectMappingError( internal_params_ ) ) &&
              ( newparams.max_imap_error_ >= 
              getInverseMappingError( internal_params_ ) ) ) {
              
              return true;
            }
          }   
        
          break;
        }
        case TeGTParams::ExaustiveOutRemotion :
        {
          if( internal_params_.enable_multi_thread_ ) {
            if( exaustiveOutRemotion( internal_params_,
              TeGetPhysProcNumber() - 1 ) ) {
              
              return true;
            }          
          } else {
            if( exaustiveOutRemotion( internal_params_, 0 ) ) {
              return true;
            }
          }
        
          break;
        }
        case TeGTParams::LWOutRemotion :
        {
          if( lWOutRemotion( internal_params_ ) ) {
            return true;
          }
                  
          break;
        }    
        case TeGTParams::RANSACRemotion :
        {
          if( ransacRemotion( newparams, internal_params_ ) ) {
            return true;
          }
                  
          break;
        }            
        default : 
        {
          TEAGN_LOG_AND_THROW( 
            "Invalid outliers remotion strategy" )
          break;
        }
      
      }
    }
  }
  
  internal_params_.reset();
  
  return false;
}


double TeGeometricTransformation::getDirectMappingError(  
  const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )   
    
  unsigned int tiepoints_size = params.tiepoints_.size();
  
  double max_error = 0;
  double current_error = 0;
  
  for( unsigned int tpindex = 0 ; tpindex < tiepoints_size ; ++tpindex ) {
    current_error = getDirectMappingError( params.tiepoints_[ tpindex ], 
      params );
    
    if( current_error > max_error ) {
      max_error = current_error;
    }
  }
  
  return max_error;
}


double TeGeometricTransformation::getInverseMappingError(  
  const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )  
    
  unsigned int tiepoints_size = params.tiepoints_.size();
  
  double max_error = 0;
  double current_error = 0;
  
  for( unsigned int tpindex = 0 ; tpindex < tiepoints_size ; ++tpindex ) {
    current_error = getInverseMappingError( params.tiepoints_[ tpindex ], 
      params );
    
    if( current_error > max_error ) {
      max_error = current_error;
    }
  }
  
  return max_error;
}


double TeGeometricTransformation::getDMapRMSE(  
  const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )   
    
  unsigned int tiepoints_size = params.tiepoints_.size();
  
  if( tiepoints_size == 0 )
  {
    return 0;
  }
  else
  {
    double error2_sum = 0;
    double current_error = 0;
    
    for( unsigned int tpindex = 0 ; tpindex < tiepoints_size ; ++tpindex ) {
      current_error = getDirectMappingError( params.tiepoints_[ tpindex ], 
        params );
        
      error2_sum += ( current_error * current_error );
    }
    
    return sqrt( error2_sum / ( (double)tiepoints_size ) );
  }
}


double TeGeometricTransformation::getIMapRMSE(  
  const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )   
    
  unsigned int tiepoints_size = params.tiepoints_.size();
  
  if( tiepoints_size == 0 )
  {
    return 0;
  }
  else
  {
    double error2_sum = 0;
    double current_error = 0;
    
    for( unsigned int tpindex = 0 ; tpindex < tiepoints_size ; ++tpindex ) {
      current_error = getInverseMappingError( params.tiepoints_[ tpindex ], 
        params );
        
      error2_sum += ( current_error * current_error );
    }
    
    return sqrt( error2_sum / ( (double)tiepoints_size ) );
  }  
}


double TeGeometricTransformation::getDirectMappingError( 
  const TeCoordPair& tie_point, const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )   
    
  TeCoord2D direct_mapped_point;

  directMap( params, tie_point.pt1, direct_mapped_point );
    
  double diff_x = tie_point.pt2.x() - direct_mapped_point.x();
  double diff_y = tie_point.pt2.y() - direct_mapped_point.y();
    
  return hypot( diff_x, diff_y );
}


double TeGeometricTransformation::getInverseMappingError( 
  const TeCoordPair& tie_point, const TeGTParams& params ) const
{
  TEAGN_DEBUG_CONDITION( isTransDefined( params ),
    "Transformation not defined" )   
    
  TeCoord2D inverse_mapped_point;

  inverseMap( params, tie_point.pt2, inverse_mapped_point );
    
  double diff_x = tie_point.pt1.x() - inverse_mapped_point.x();
  double diff_y = tie_point.pt1.y() - inverse_mapped_point.y();
    
  return hypot( diff_x, diff_y );
}


bool TeGeometricTransformation::recombineSeed( std::vector<unsigned int>& seed, 
  const unsigned int& seedpos, const unsigned int& elements_nmb )
{
  unsigned int seed_size = seed.size();
  
  if( seedpos >= seed_size ) {
    return false;
  }

  if( seed[ seedpos ]  >= ( elements_nmb - seed_size + seedpos + 1 ) ) {
    if( seedpos == seed_size - 1 ) {
      return false;
    } else if( seedpos == 0 ) {
      return recombineSeed( seed, seedpos + 1, elements_nmb ) ;
    } else {
      return recombineSeed( seed, seedpos + 1, elements_nmb ) ;
    };
  } else if( seed[ seedpos ]  == 0 ) {
    if( seedpos == 0 ) {
      seed[ seedpos ] = 1 ;
      return recombineSeed( seed, seedpos + 1, elements_nmb );
    } else if( seedpos == seed_size - 1 ) {
      seed[ seedpos ] = seed[ seedpos - 1 ] + 1;
      return true;
    } else {
      seed[ seedpos ] = seed[ seedpos - 1 ] + 1;
      seed[ seedpos + 1 ] = 0;
      return recombineSeed( seed, seedpos + 1, elements_nmb );
    }
  } else {
    if( seedpos == seed_size - 1 ) {
      seed[ seedpos ] = seed[ seedpos ] + 1;
      return true;
    } else if( seedpos == 0 ) {
      if( recombineSeed( seed, seedpos + 1, elements_nmb ) ) {
        return true;
      } else {
        seed[ seedpos ] = seed[ seedpos ] + 1;
        seed[ seedpos + 1 ] = 0;
        return recombineSeed( seed, seedpos + 1, elements_nmb );
      }
    } else {
      if( recombineSeed( seed, seedpos + 1, elements_nmb ) ) {
        return true;
      } else {
        seed[ seedpos ] = seed[ seedpos ] + 1;
        seed[ seedpos + 1 ] = 0;
        return recombineSeed( seed, seedpos + 1, elements_nmb );
      }
    }
  }
}


TeGeometricTransformation* TeGeometricTransformation::DefaultObject( 
  const TeGTParams& )
{ 
  TEAGN_LOG_AND_THROW( "Trying to create an invalid "
    "TeGemetricTransformation instance" );
  
  return 0;
}; 


bool TeGeometricTransformation::exaustiveOutRemotion( 
  TeGTParams& params, unsigned int threads_nmb )
{
  TEAGN_DEBUG_CONDITION( ( params.out_rem_strat_ == 
    TeGTParams::ExaustiveOutRemotion ), 
    "Inconsistent outliers remotion strategy" )
  TEAGN_TRUE_OR_THROW( ( params.max_dmap_error_ >= 0 ),
    "Invalid maximum allowed direct mapping error" );
  TEAGN_TRUE_OR_THROW( ( params.max_imap_error_ >= 0 ),
    "Invalid maximum allowed inverse mapping error" );    

  /* Initiating seed */
  
  std::vector<unsigned int> comb_seed_vec;       
    
  unsigned int req_tie_pts_nmb = getMinRequiredTiePoints();
  
  for( unsigned int comb_seed_vec_index = 0 ; 
    comb_seed_vec_index < req_tie_pts_nmb ;
    ++comb_seed_vec_index ) {
    
    comb_seed_vec.push_back( 0 );
  }
  
  /* initializing mutexes */
  
  TeMutex comb_seed_vec_mutex;
  TeMutex trans_params_mutex;
  
  /* Initializing threads */
  
  TeGTParams best_trans_params;
  volatile double best_trans_dmap_error = DBL_MAX;
  volatile double best_trans_imap_error = DBL_MAX;
  
  std::vector< TeThreadFunctor::pointer > threads_vector;
  
  TeThreadParameters thread_params;
  
  thread_params.store( "req_tie_pts_nmb", req_tie_pts_nmb );  
  thread_params.store( "init_trans_params_ptr", 
    (TeGTParams const*)(&params) );
  thread_params.store( "best_trans_params_ptr", 
    &best_trans_params );    
  thread_params.store( "best_trans_dmap_error_ptr", 
    &best_trans_dmap_error );      
  thread_params.store( "best_trans_imap_error_ptr", 
    &best_trans_imap_error );     
  thread_params.store( "comb_seed_vec_ptr", &comb_seed_vec );
  thread_params.store( "trans_params_mutex_ptr", 
    &trans_params_mutex );
  thread_params.store( "comb_seed_vec_mutex_ptr", 
    &comb_seed_vec_mutex );  
  thread_params.store( "geo_trans_ptr", 
    (TeGeometricTransformation const*)this );        
    
  unsigned int thread_index = 0;
  
  for( thread_index = 0 ; thread_index < threads_nmb ; 
    ++thread_index ) {
    
    TeThreadFunctor::pointer aux_thread_ptr( 
      new TeThreadFunctor );
    aux_thread_ptr->setStartFunctPtr( eORThreadEntry );
    aux_thread_ptr->setParameters( thread_params );
    aux_thread_ptr->start();
  
    threads_vector.push_back( aux_thread_ptr );
  }
  
  bool my_return = eORThreadEntry( thread_params );
  bool threads_return = true;
  
  for( thread_index = 0 ; thread_index < threads_nmb ; 
    ++thread_index ) {
    
    threads_vector[ thread_index ]->waitToFinish();
    
    threads_return = threads_return & 
      threads_vector[ thread_index ]->getReturnValue();
  }  
  
  if( my_return & threads_return ) {
    if( best_trans_params.tiepoints_.size() >= req_tie_pts_nmb ) {
      params = best_trans_params;
      
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


bool TeGeometricTransformation::eORThreadEntry( 
  const TeThreadParameters& params )
{
  /* Extracting parameters */
  
  unsigned int req_tie_pts_nmb = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "req_tie_pts_nmb",
    req_tie_pts_nmb ), "Missing parameter" );
    
  TeGTParams const* init_trans_params_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "init_trans_params_ptr",
    init_trans_params_ptr ), "Missing parameter" );
  
  TeGTParams* best_trans_params_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "best_trans_params_ptr",
    best_trans_params_ptr ), "Missing parameter" );   
    
  double volatile* best_trans_dmap_error_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "best_trans_dmap_error_ptr",
    best_trans_dmap_error_ptr ), "Missing parameter" );     
    
  double volatile* best_trans_imap_error_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "best_trans_imap_error_ptr",
    best_trans_imap_error_ptr ), "Missing parameter" );     

  std::vector<unsigned int>* comb_seed_vec_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "comb_seed_vec_ptr",
    comb_seed_vec_ptr ), "Missing parameter" );
    
  TeMutex* trans_params_mutex_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "trans_params_mutex_ptr",
    trans_params_mutex_ptr ), "Missing parameter" ); 

  TeMutex* comb_seed_vec_mutex_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "comb_seed_vec_mutex_ptr",
    comb_seed_vec_mutex_ptr ), "Missing parameter" );  
    
  TeGeometricTransformation const* geo_trans_ptr = 0;
  TEAGN_TRUE_OR_THROW( params.retrive( "geo_trans_ptr",
    geo_trans_ptr ), "Missing parameter" ); 
  
  /* Optimized local variables based on the input parameters */
  
  std::vector<unsigned int>& comb_seed_vec =
    (*comb_seed_vec_ptr);  
    
  TeMutex& comb_seed_vec_mutex = (*comb_seed_vec_mutex_ptr); 
  
  const TeGeometricTransformation& geo_trans = (*geo_trans_ptr); 
  
  /* Copying some parameters to local variables to avoid
     mutex overhead */
     
  trans_params_mutex_ptr->lock();
     
  const TeGTParams initial_trans_params = (*init_trans_params_ptr);     
  const unsigned int initial_tiepoints_size = 
    initial_trans_params.tiepoints_.size();   
  
  trans_params_mutex_ptr->unLock();
        
  /* Trying to find the best tie-points by building 
    the transformation with the highest number of
    tie-points, but with an acceptable mapping error */
    
  TeGTParams local_best_params;
  double local_best_params_dmap_error = DBL_MAX;
  double local_best_params_imap_error = DBL_MAX;
  
  TeGTParams curr_params = initial_trans_params;
  double curr_params_dmap_error = DBL_MAX;
  double curr_params_imap_error = DBL_MAX; 
  
  TeGTParams cp_plus_new_point;
  double cp_plus_new_point_dmap_error = DBL_MAX;
  double cp_plus_new_point_imap_error = DBL_MAX;
  
  unsigned int tiepoints_index = 0;
  bool point_already_present = false;
  bool seed_recombined = true;
  unsigned int comb_seed_vec_index_2 = 0;
  unsigned int comb_seed_vec_index_3 = 0;
  
  while( seed_recombined ) {
    /* trying to recombine seed */
    
    comb_seed_vec_mutex.lock();
    seed_recombined = recombineSeed( comb_seed_vec, 0, 
      initial_tiepoints_size );
    comb_seed_vec_mutex.unLock();
      
    if( seed_recombined ) {
      /* Extracting tie-points from the original vector */
      
      curr_params.tiepoints_.clear();
      
      for( comb_seed_vec_index_2 = 0 ; 
        comb_seed_vec_index_2 < req_tie_pts_nmb ;
        ++comb_seed_vec_index_2 ) {
        
        curr_params.tiepoints_.push_back( 
          initial_trans_params.tiepoints_[ comb_seed_vec[ 
          comb_seed_vec_index_2 ] - 1 ] );
      }
      
      /* Trying to generate a valid transformation */
      
      if( geo_trans.computeParameters( curr_params ) ) {
        curr_params_dmap_error = geo_trans.getDirectMappingError( 
          curr_params );
        curr_params_imap_error = geo_trans.getInverseMappingError( 
          curr_params );
      
        if( ( initial_trans_params.max_dmap_error_ >= 
          curr_params_dmap_error ) &&
          ( initial_trans_params.max_imap_error_ >= 
          curr_params_imap_error ) ) {
        
          /* Trying to insert more tie-points into current 
              transformation */
          
          for( tiepoints_index = 0 ; tiepoints_index < 
            initial_tiepoints_size ; ++tiepoints_index ) {
            
            /* Verifying if the current tie-point is already 
               present */
            
            point_already_present = false;
            
            for( comb_seed_vec_index_3 = 0 ; 
              comb_seed_vec_index_3 < req_tie_pts_nmb ;
              ++comb_seed_vec_index_3 ) {
            
              if( tiepoints_index == ( 
                comb_seed_vec[ comb_seed_vec_index_3 ] - 1 ) ) {
              
                point_already_present = true;
                break;
              }
            }
            
            if( ! point_already_present ) {
              cp_plus_new_point = curr_params;
              
              cp_plus_new_point.tiepoints_.push_back( 
                initial_trans_params.tiepoints_[ tiepoints_index ] );
              
              /* Verifying if the new tie-point insertion does 
                not generate an invalid transformation */
              
              if( geo_trans.computeParameters( cp_plus_new_point ) ) {
                cp_plus_new_point_dmap_error =
                  geo_trans.getDirectMappingError( cp_plus_new_point );
                cp_plus_new_point_imap_error =
                  geo_trans.getInverseMappingError( cp_plus_new_point );
                  
                if( ( cp_plus_new_point_dmap_error <=
                  initial_trans_params.max_dmap_error_ ) &&
                  ( cp_plus_new_point_imap_error <=
                  initial_trans_params.max_imap_error_ ) ) {
                  
                  curr_params = cp_plus_new_point;
                  curr_params_dmap_error = 
                    cp_plus_new_point_dmap_error;
                  curr_params_imap_error =
                    cp_plus_new_point_imap_error;  
                }
              }
            }
          }
          
          /* A valid transformation was generated, now 
            verifying 
            if the number of tie-poits is greater then the current 
            thread local best transformation.
          */            
          
          if( curr_params.tiepoints_.size() >= 
            local_best_params.tiepoints_.size() ) {
            
            if( ( curr_params_dmap_error < 
              local_best_params_dmap_error ) &&
              ( curr_params_imap_error <
                local_best_params_imap_error ) ) {
            
              local_best_params = curr_params;
              local_best_params_dmap_error = curr_params_dmap_error;
              local_best_params_imap_error = curr_params_imap_error;
            }
          }           
        }
      } //if( geo_trans.computeParameters( curr_params ) )
    } //if( seed_recombined )
  } //while( seed_recombined )
  
  /* A valid local thread transformation was generated, now 
    verifying 
    if the error is smaller then the current 
    global transformation.
  */

  if( local_best_params.tiepoints_.size() >= req_tie_pts_nmb ) {
    trans_params_mutex_ptr->lock();
    
    if( local_best_params.tiepoints_.size() >=
      best_trans_params_ptr->tiepoints_.size() ) {
      
      if( ( local_best_params_dmap_error < 
        (*best_trans_dmap_error_ptr) ) &&
        ( local_best_params_imap_error < 
        (*best_trans_imap_error_ptr) ) ) {
      
        (*best_trans_params_ptr) = local_best_params;
        (*best_trans_dmap_error_ptr) = local_best_params_dmap_error;
        (*best_trans_imap_error_ptr) = local_best_params_imap_error;       
      }      
    }
    
    trans_params_mutex_ptr->unLock();
  }
  
  return true;
}


bool TeGeometricTransformation::lWOutRemotion( 
  TeGTParams& external_params )
{
  TEAGN_DEBUG_CONDITION( ( external_params.out_rem_strat_ == 
    TeGTParams::LWOutRemotion ), 
    "Inconsistent outliers remotion strategy" )
  TEAGN_TRUE_OR_THROW( ( external_params.tiePointsWeights_.size() ?
    ( external_params.tiePointsWeights_.size() ==
    external_params.tiepoints_.size() ) : true ),
    "The tie-points vector size do not match the tie-points "
    "weights vector size" )
  TEAGN_TRUE_OR_THROW( ( external_params.max_dmap_error_ >= 0 ),
    "Invalid maximum allowed direct mapping error" );
  TEAGN_TRUE_OR_THROW( ( external_params.max_imap_error_ >= 0 ),
    "Invalid maximum allowed inverse mapping error" );
  TEAGN_TRUE_OR_THROW( ( external_params.max_dmap_rmse_ >= 0 ),
    "Invalid maximum allowed direct mapping RMSE" );
  TEAGN_TRUE_OR_THROW( ( external_params.max_imap_rmse_ >= 0 ),
    "Invalid maximum allowed inverse mapping RMSE" );        
    
  const unsigned int req_tie_pts_nmb = getMinRequiredTiePoints();
  
  // Normalizing tie-points weights
    
  const bool useTPWeights = ( external_params.tiePointsWeights_.size() ) ?
    true : false;
  TEAGN_TRUE_OR_THROW( normalizeTPWeights( 
     external_params.tiePointsWeights_ ),
     "Error normalizing tie-points weights" );
  
  if( external_params.tiepoints_.size() == req_tie_pts_nmb ) {
    return computeParameters( external_params );
  } else if( external_params.tiepoints_.size() > req_tie_pts_nmb ) {
    /* Global vars */
    
    const double max_dmap_error = 
      external_params.max_dmap_error_;
    const double max_imap_error = 
      external_params.max_imap_error_;
    const double max_dmap_rmse = 
      external_params.max_dmap_rmse_;
    const double max_imap_rmse = 
      external_params.max_imap_rmse_;
          
    /* Computing the initial global transformation */
    
    if( ! computeParameters( external_params ) ) {
      return false;
    }    
    
    if( ( getDirectMappingError( external_params ) 
      <= max_dmap_error ) && 
      ( getInverseMappingError( external_params ) 
      <= max_imap_error ) &&
      ( getDMapRMSE( external_params ) 
      <= max_dmap_rmse ) && 
      ( getIMapRMSE( external_params ) 
      <= max_imap_rmse ) ) {
    
      /* This transformation has no outliers */
      
      return true;
    }    
    
    /* Iterating over the current transformation tie-points */
    
    TeGTParams best_params;
    double best_params_dmap_rmse = DBL_MAX;
    double best_params_imap_rmse = DBL_MAX;
    double best_params_dmap_error = DBL_MAX;
    double best_params_imap_error = DBL_MAX;
    
    bool transformation_not_updated = false;
    unsigned int iterations_remainning = (unsigned int)
      external_params.tiepoints_.size();
    std::vector< TPDataNode > norm_err_vec;     
    std::list< ExcTPDataNode > exc_tp_list;
    TeGTParams iteration_params = external_params;
    TeCoordPair excluded_tp;
    double excluded_tp_weight = 0;
    ExcTPDataNode dummy_exctpdata;
  
    while( ( iteration_params.tiepoints_.size() > 
      req_tie_pts_nmb ) && iterations_remainning )
    {
      unsigned int iter_tps_nmb = (unsigned int)
        iteration_params.tiepoints_.size();
      transformation_not_updated = true;
      
      /* Updating the normalized error map */
      
      updateTPErrVec( iteration_params, useTPWeights, norm_err_vec );
      
      /* Generating all possible transformations without 
         each tie-point, starting with the worse point */
      
      for( int norm_err_vec_idx = ((int)norm_err_vec.size()) - 1 ;
        ( norm_err_vec_idx > ( -1 ) ) ; --norm_err_vec_idx )
      {
        const unsigned int& cur_candtp_idx = 
          norm_err_vec[ norm_err_vec_idx ].tpindex_;
          
        TEAGN_DEBUG_CONDITION( ( cur_candtp_idx < 
          iteration_params.tiepoints_.size() ), "Invalid index" )

        /* Generating a transformation parameters without the
          current tie-point (bigger error)*/
            
        TeGTParams new_iteration_params = iteration_params;
        new_iteration_params.tiepoints_.clear();
        new_iteration_params.tiePointsWeights_.clear();
        new_iteration_params.direct_parameters_.Clear();
        new_iteration_params.inverse_parameters_.Clear();
            
        for( unsigned int tpindex2 = 0 ; tpindex2 < iter_tps_nmb ; 
          ++tpindex2 ) 
        {
          if( cur_candtp_idx != tpindex2 ) 
          {
            new_iteration_params.tiepoints_.push_back( 
              iteration_params.tiepoints_[ tpindex2 ] );
              
            if( useTPWeights )
            {
              new_iteration_params.tiePointsWeights_.push_back( 
                iteration_params.tiePointsWeights_[ tpindex2 ] );
            }
          }
        }            
      
        /* Trying to generate a transformation without the current
            candidate tie-point for exclusion */      
        
        if( computeParameters( new_iteration_params ) ) 
        {
          double new_it_dmap_rmse = 
            getDMapRMSE( new_iteration_params );
          double new_it_imap_rmse = 
            getIMapRMSE( new_iteration_params );
          
          if( ( best_params_dmap_rmse > new_it_dmap_rmse ) && 
            ( best_params_imap_rmse > new_it_imap_rmse ) ) 
          {
            double new_it_dmap_error = 
              getDirectMappingError( new_iteration_params );
            double new_it_imap_error = 
              getInverseMappingError( new_iteration_params );
                        
            excluded_tp = iteration_params.tiepoints_[ cur_candtp_idx ];
            if( useTPWeights )
            {
              excluded_tp_weight = iteration_params.tiePointsWeights_[ 
                cur_candtp_idx ];
            }
              
            /* Trying to insert back other tie-points excluded
               before */
            
            if( exc_tp_list.size() > 0 )
            {
              /* Updating the excluded tie points errors map */
              
              updateExcTPErrList( new_iteration_params, useTPWeights,
                exc_tp_list );
                
              /* Iterating over the excluded tps */
              
              std::list< ExcTPDataNode >::iterator 
                exc_tps_list_it = exc_tp_list.begin();
              std::list< ExcTPDataNode >::iterator 
                exc_tps_list_it_end = exc_tp_list.end();
                
              while( exc_tps_list_it != exc_tps_list_it_end )
              {
                const ExcTPDataNode& curr_exc_tp_data = 
                  *exc_tps_list_it;

                /* Does this tp has direct and inverse errors 
                   smaller than the current new_iteration_params ?? */
                        
                if( ( curr_exc_tp_data.dmap_error_ <= 
                  new_it_dmap_error ) && 
                  ( curr_exc_tp_data.imap_error_ <= 
                  new_it_imap_error ) )
                {
                  /* Trying to generate a trasformation with
                     this point */
                     
                  TeGTParams new_iteration_params_plus1tp = 
                    new_iteration_params;
                  new_iteration_params.tiepoints_.push_back(
                    curr_exc_tp_data.tp_ );
                  if( useTPWeights )
                  {  
                    new_iteration_params.tiePointsWeights_.push_back(
                      curr_exc_tp_data.tp_weight_ );
                  }
                    
                  if( computeParameters( 
                    new_iteration_params_plus1tp ) ) 
                  {
                    double newp1tp_dmap_rmse = getDMapRMSE( 
                      new_iteration_params_plus1tp );
                    double newp1tp_imap_rmse = getIMapRMSE( 
                      new_iteration_params_plus1tp );
                    
                    if( ( new_it_dmap_rmse >= newp1tp_dmap_rmse ) && 
                      ( new_it_imap_rmse >= newp1tp_imap_rmse ) ) 
                    {
                      new_iteration_params = 
                        new_iteration_params_plus1tp;
                      new_it_dmap_error = getDirectMappingError( 
                        new_iteration_params_plus1tp );
                      new_it_imap_error = getInverseMappingError( 
                        new_iteration_params_plus1tp ); 
                      new_it_dmap_rmse = newp1tp_dmap_rmse;
                      new_it_imap_rmse = newp1tp_imap_rmse;
                        
                      exc_tp_list.erase( exc_tps_list_it );
                        
                      updateExcTPErrList( new_iteration_params, 
                        useTPWeights, exc_tp_list );   
                        
                      exc_tps_list_it = exc_tp_list.begin();
                      exc_tps_list_it_end = exc_tp_list.end();                    
                    }
                    else
                    {
                      ++exc_tps_list_it;
                    }
                  } 
                  else
                  {
                    ++exc_tps_list_it;
                  }                   
                }
                else
                {  
                  ++exc_tps_list_it;
                }
              }
            }
            
            /* Updating the best transformation parameters */
            
            best_params_dmap_error = new_it_dmap_error;
            best_params_imap_error = new_it_imap_error;
            best_params_dmap_rmse = new_it_dmap_rmse;
            best_params_imap_rmse = new_it_imap_rmse;
            best_params = new_iteration_params;
            
            /* Updating the next iteration parameters */
            
            iteration_params = new_iteration_params;
            
            transformation_not_updated = false;
            
            /* Putting the excluded tie-point into the 
               excluded tie-points map */
               
            dummy_exctpdata.tp_ = excluded_tp;
            if( useTPWeights )
            {
              dummy_exctpdata.tp_weight_ = excluded_tp_weight;
            }
            exc_tp_list.push_back( dummy_exctpdata );
            
            /* break the current tie-points loop */
            
            break;
          } //if( ( best_params_dmap_error > new_it_dmap_error ) && 
        } //if( computeParameters( new_iteration_params ) ) {    
      }
      
      if( transformation_not_updated ) {
        /* There is no way to improve the current transformation
           since all tie-points were tested */
        
        break; /* break the iterations loop */
      } else {
        if( ( max_dmap_error >= best_params_dmap_error ) && 
            ( max_imap_error >= best_params_imap_error ) &&
            ( max_dmap_rmse >= best_params_dmap_rmse ) && 
            ( max_imap_rmse >= best_params_imap_rmse ) ) {
            
          /* A valid transformation was found */
          break;
        }
      }
      
      --iterations_remainning;
    }//while( params.tiepoints_.size() > req_tie_pts_nmb ) {
    
    if( ( best_params.tiepoints_.size() >= req_tie_pts_nmb ) &&
        ( max_dmap_error >= best_params_dmap_error ) && 
        ( max_imap_error >= best_params_imap_error ) &&
        ( max_dmap_rmse >= best_params_dmap_rmse ) && 
        ( max_imap_rmse >= best_params_imap_rmse ) ) {
        
      external_params = best_params;
      
      return true;
    } else {
      return false;
    }    
  }
  
  return false;
}



void TeGeometricTransformation::updateExcTPErrList( 
  const TeGTParams& params,
  bool useTPWeights,
  std::list< ExcTPDataNode >& exc_tp_list )
  const
{
  if( exc_tp_list.size() > 0 )
  {
    TEAGN_DEBUG_CONDITION( ( useTPWeights ?
      ( params.tiePointsWeights_.size() == 
      params.tiepoints_.size() ) : true ),
      "The tie-points vector size do not match the tie-points "
      "weights vector size" )
      
    /* Updating the new direct and inverse mapping errors */
     
    std::list< ExcTPDataNode >::iterator tp_list_it = 
      exc_tp_list.begin();
    std::list< ExcTPDataNode >::iterator tp_list_it_end = 
      exc_tp_list.end();
      
    double dmap_err_min =  DBL_MAX;
    double dmap_err_max =  (-1.0) * DBL_MAX;
    double imap_err_min =  DBL_MAX;  
    double imap_err_max =  (-1.0) * DBL_MAX;
    
    while( tp_list_it != tp_list_it_end )
    {
      ExcTPDataNode& curr_node = *tp_list_it;
      
      const TeCoordPair& cur_ex_tp = curr_node.tp_;
      
      curr_node.dmap_error_ = getDirectMappingError( cur_ex_tp, 
        params );
      curr_node.imap_error_ = getInverseMappingError( cur_ex_tp, 
        params );        
        
      if( dmap_err_min > curr_node.dmap_error_ ) {
        dmap_err_min = curr_node.dmap_error_;
      }
      if( dmap_err_max < curr_node.dmap_error_ ) {
        dmap_err_max = curr_node.dmap_error_;
      }        
        
      if( imap_err_min > curr_node.imap_error_ ) {
        imap_err_min = curr_node.imap_error_;
      }
      if( imap_err_max < curr_node.imap_error_ ) {
        imap_err_max = curr_node.imap_error_;
      }            
      
      ++tp_list_it;
    }
    
    double dmap_err_range = dmap_err_max - dmap_err_min;
    double imap_err_range = imap_err_max - imap_err_min;
    
    /* Updating the normalized error */
    
    tp_list_it = exc_tp_list.begin();
    
    if( ( dmap_err_range == 0.0 ) &&
      ( imap_err_range == 0.0 ) )
    {
      while( tp_list_it != tp_list_it_end )
      {
        tp_list_it->tp_error_ = 0.0;
        
        ++tp_list_it;
      }
    }
    else if( dmap_err_range == 0.0 ) 
    {
      if( useTPWeights )
      {       
        while( tp_list_it != tp_list_it_end )
        {
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ > 0.0,
            "Invalid tie-point weight" )
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ <= 1.0,
            "Invalid tie-point weight" )            
                    
          tp_list_it->tp_error_ =
            ( 
              (  
                tp_list_it->imap_error_ - imap_err_min              
              ) 
              / ( imap_err_range * tp_list_it->tp_weight_  )
            );

          ++tp_list_it;
        }
      }
      else
      {
        while( tp_list_it != tp_list_it_end )
        {
          tp_list_it->tp_error_ =
            ( 
              (  
                tp_list_it->imap_error_ - imap_err_min              
              ) 
              / imap_err_range 
            );
          
          ++tp_list_it;
        }
      }
    }
    else if( imap_err_range == 0.0 ) 
    {
      if( useTPWeights )
      {     
        while( tp_list_it != tp_list_it_end )
        {
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ > 0.0,
            "Invalid tie-point weight" )
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ <= 1.0,
            "Invalid tie-point weight" )            
                    
          tp_list_it->tp_error_ =
            ( 
              ( 
                tp_list_it->dmap_error_ - dmap_err_min
              ) 
              / ( dmap_err_range * tp_list_it->tp_weight_ )
            );

          ++tp_list_it;
        }
      }
      else
      {
        while( tp_list_it != tp_list_it_end )
        {
          tp_list_it->tp_error_ =
            ( 
              ( 
                tp_list_it->dmap_error_ - dmap_err_min
              ) 
              / dmap_err_range 
            );
          
          ++tp_list_it;
        }
      }
    }
    else
    {
      if( useTPWeights )
      {    
        while( tp_list_it != tp_list_it_end )
        {
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ > 0.0,
            "Invalid tie-point weight" )
          TEAGN_DEBUG_CONDITION( tp_list_it->tp_weight_ <= 1.0,
            "Invalid tie-point weight" )            
                    
          tp_list_it->tp_error_ =
            (
              ( 
                ( 
                  tp_list_it->dmap_error_ - dmap_err_min
                ) 
                / dmap_err_range 
              ) 
              + 
              ( 
                (  
                  tp_list_it->imap_error_ - imap_err_min              
                ) 
                / imap_err_range 
              ) 
            ) / ( 2.0 * tp_list_it->tp_weight_ );

          ++tp_list_it;
        }
      }
      else
      {
        while( tp_list_it != tp_list_it_end )
        {
          tp_list_it->tp_error_ =
            ( 
              ( 
                ( 
                  tp_list_it->dmap_error_ - dmap_err_min
                ) 
                / dmap_err_range 
              ) 
              + 
              ( 
                (  
                  tp_list_it->imap_error_ - imap_err_min              
                ) 
                / imap_err_range 
              ) 
            );
          
          ++tp_list_it;
        }
      }
    }
    
    /* Sorting list */
    
    exc_tp_list.sort();
    
    /* printing */
/*  
    tp_list_it = exc_tp_list.begin();
    tp_list_it_end = exc_tp_list.end();
    std::cout << std::endl << "Excluded Tie points list" << std::endl;
    while( tp_list_it != tp_list_it_end )
    {
      const ExcTPDataNode& node = *tp_list_it;
      
      std::cout << node.norm_error_ << " " <<
       " [" + Te2String( node.tp_.pt1.x(),2 ) + "," +
       Te2String( node.tp_.pt1.y(),2 ) + "-" +
       Te2String( node.tp_.pt2.x(),2 ) + "," +
       Te2String( node.tp_.pt2.y(),2 ) + "]"
       << std::endl;      
      
      ++tp_list_it;
    }    
    std::cout << std::endl;
*/ 
  }
}


void TeGeometricTransformation::updateTPErrVec( 
  const TeGTParams& params, bool useTPWeights,
  std::vector< TPDataNode >& errvec ) const
{
  TEAGN_DEBUG_CONDITION( ( useTPWeights ?
    ( params.tiePointsWeights_.size() == 
    params.tiepoints_.size() ) : true ),
    "The tie-points vector size do not match the tie-points "
    "weights vector size" )
    
  /* Calculating the two mapping errors */
  
  const unsigned int iter_tps_nmb = (unsigned int)
    params.tiepoints_.size();  
    
  errvec.clear();

  double dmap_err_vec_min = DBL_MAX;
  double dmap_err_vec_max = ( -1.0 ) * dmap_err_vec_min;
  double imap_err_vec_min = DBL_MAX;
  double imap_err_vec_max = ( -1.0 ) * imap_err_vec_min;      
  const TPDataNode dummy_struct;
  
  for( unsigned int par_tps_vec_idx = 0 ; par_tps_vec_idx < iter_tps_nmb ; 
    ++par_tps_vec_idx ) 
  {
    const TeCoordPair& cur_tp = params.tiepoints_[ 
      par_tps_vec_idx ];
      
    errvec.push_back( dummy_struct );
    TPDataNode& newNode = errvec[ errvec.size() - 1 ];
      
    newNode.tpindex_ = par_tps_vec_idx;
    newNode.dmap_error_ = getDirectMappingError( cur_tp, 
      params );
    newNode.imap_error_ = getInverseMappingError( cur_tp, 
      params );
    
    if( dmap_err_vec_min > newNode.dmap_error_ ) {
      dmap_err_vec_min = newNode.dmap_error_;
    }
    if( dmap_err_vec_max < newNode.dmap_error_ ) {
      dmap_err_vec_max = newNode.dmap_error_;
    }        
    
    if( imap_err_vec_min > newNode.imap_error_ ) {
      imap_err_vec_min = newNode.imap_error_;
    }
    if( imap_err_vec_max < newNode.imap_error_ ) {
      imap_err_vec_max = newNode.imap_error_;
    }            
  }
  
  const double dmap_err_vec_range = 
    dmap_err_vec_max - dmap_err_vec_min;
  const double imap_err_vec_range = 
    imap_err_vec_max - imap_err_vec_min;
  TEAGN_DEBUG_CONDITION( dmap_err_vec_range >= 0, "Invalid range" )
  TEAGN_DEBUG_CONDITION( imap_err_vec_range >= 0, "Invalid range" )

  const std::vector< double >& tpwVec = params.tiePointsWeights_;
    
    /* Updating normalized mapping errors */
    
  if( ( dmap_err_vec_range == 0.0 ) && ( imap_err_vec_range == 0.0 ) )
  {
    for( unsigned int errvec_idx = 0 ; errvec_idx < iter_tps_nmb ; 
      ++errvec_idx ) 
    {
      errvec[ errvec_idx ].tp_error_ = 0;
    }
  }
  else if( dmap_err_vec_range == 0.0 )
  {
    for( unsigned int errvec_idx = 0 ; errvec_idx < iter_tps_nmb ; 
      ++errvec_idx ) 
    {
      TPDataNode& curr_elem = errvec[ errvec_idx ];
      
      if( useTPWeights )
      {
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] > 0.0,
          "Invalid tie-point weight" )
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] <= 1.0,
          "Invalid tie-point weight" )            
                
        curr_elem.tp_error_ = 
          ( 
            (  
              curr_elem.imap_error_ - 
              imap_err_vec_min              
            ) 
            / ( imap_err_vec_range * tpwVec[ errvec_idx ] )
          );
      }
      else
      {
        curr_elem.tp_error_ = 
          ( 
            (  
              curr_elem.imap_error_ - 
              imap_err_vec_min              
            ) 
            / imap_err_vec_range 
          );
      }
    }
  }
  else if( imap_err_vec_range == 0.0 )
  {
    for( unsigned int errvec_idx = 0 ; errvec_idx < iter_tps_nmb ; 
      ++errvec_idx ) 
    {
      TPDataNode& curr_elem = errvec[ errvec_idx ];
      
      if( useTPWeights )
      {        
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] > 0.0,
          "Invalid tie-point weight" )
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] <= 1.0,
          "Invalid tie-point weight" )            
                  
        curr_elem.tp_error_ = 
          ( 
            ( 
              curr_elem.dmap_error_ - 
              dmap_err_vec_min
            ) 
            / ( dmap_err_vec_range * tpwVec[ errvec_idx ] )
          );
      }
      else
      {
        curr_elem.tp_error_ = 
          ( 
            ( 
              curr_elem.dmap_error_ - 
              dmap_err_vec_min
            ) 
            / dmap_err_vec_range 
          );
      }
    }
  }
  else
  {
    for( unsigned int errvec_idx = 0 ; errvec_idx < iter_tps_nmb ; 
      ++errvec_idx ) 
    {
      TPDataNode& curr_elem = errvec[ errvec_idx ];
      
      if( useTPWeights )
      { 
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] > 0.0,
          "Invalid tie-point weight" )
        TEAGN_DEBUG_CONDITION( tpwVec[ errvec_idx ] <= 1.0,
          "Invalid tie-point weight" )            
                  
        curr_elem.tp_error_ = 
          (
            ( 
              ( 
                curr_elem.dmap_error_ - 
                dmap_err_vec_min
              ) 
              / dmap_err_vec_range 
            ) 
            + 
            ( 
              (  
                curr_elem.imap_error_ - 
                imap_err_vec_min              
              ) 
              / imap_err_vec_range 
            )
          ) / ( 2.0 * tpwVec[ errvec_idx ] );
      }
      else
      {
        curr_elem.tp_error_ = 
          ( 
            ( 
              curr_elem.dmap_error_ - 
              dmap_err_vec_min
            ) 
            / dmap_err_vec_range 
          ) 
          + 
          ( 
            (  
              curr_elem.imap_error_ - 
              imap_err_vec_min              
            ) 
            / imap_err_vec_range 
          );
      }
    }
  }

  sort( errvec.begin(), errvec.end() );

    /* printing */
/*
  std::cout << std::endl << "Tie points error vector" << std::endl;
  for( unsigned int errvec_idx = 0 ; errvec_idx < iter_tps_nmb ; 
    ++errvec_idx ) 
  {
    const unsigned int& tp_pars_vec_idx = errvec[ errvec_idx ].tpindex_;
    const TeCoordPair& currtp = params.tiepoints_[ 
    tp_pars_vec_idx ];
    
    std::cout << errvec[ errvec_idx ].norm_error_ << " " <<
      " [" + Te2String( currtp.pt1.x(),2 ) + "," +
      Te2String( currtp.pt1.y(),2 ) + "-" +
      Te2String( currtp.pt2.x(),2 ) + "," +
      Te2String( currtp.pt2.y(),2 ) + "]"
      << std::endl;
  }    
*/   
}

bool TeGeometricTransformation::ransacRemotion( 
  const TeGTParams& inputParams, TeGTParams& outputParams ) const
{
  TEAGN_DEBUG_CONDITION( ( inputParams.out_rem_strat_ == 
    TeGTParams::RANSACRemotion ), 
    "Inconsistent outliers remotion strategy" )
  TEAGN_TRUE_OR_THROW( ( inputParams.max_dmap_error_ >= 0 ),
    "Invalid maximum allowed direct mapping error" );
  TEAGN_TRUE_OR_THROW( ( inputParams.max_imap_error_ >= 0 ),
    "Invalid maximum allowed inverse mapping error" );
  TEAGN_TRUE_OR_THROW( ( inputParams.max_dmap_rmse_ >= 0 ),
    "Invalid maximum allowed direct mapping RMSE" );
  TEAGN_TRUE_OR_THROW( ( inputParams.max_imap_rmse_ >= 0 ),
    "Invalid maximum allowed inverse mapping RMSE" );   
         
  TEAGN_TRUE_OR_THROW( ( inputParams.tiePointsWeights_.size() ?
    ( inputParams.tiePointsWeights_.size() ==
    inputParams.tiepoints_.size() ) : true ),
    "The tie-points vector size do not match the tie-points "
    "weights vector size" )
        
  // Checking the number of required tie-points
    
  const unsigned int reqTPsNmb = getMinRequiredTiePoints();
  const unsigned int inputTPNmb = (unsigned int)inputParams.tiepoints_.size();
  
  if( inputTPNmb < reqTPsNmb )
  {
    return false;  
  }
  else
  {
    // generating the tie-points accumulated probabilities map
	  // with positive values up to RAND_MAX + 1
    std::map< double, TeCoordPair > tpsMap;
    {
      if( inputParams.tiePointsWeights_.size() > 0 )
      {
        // finding normalization factors

        double originalWSum = 0.0;
        unsigned int tpIdx = 0;

        for( tpIdx = 0 ; tpIdx < inputTPNmb ; ++tpIdx )
        {
          originalWSum += inputParams.tiePointsWeights_[ tpIdx ];
        }

        TEAGN_TRUE_OR_THROW( originalWSum > 0.0, 
          "Invalid tie-points weighs sum" );

        // map fill 
        
        double newWSum = 0.0;
        double newW = 0.0;
        
        for( tpIdx = 0 ; tpIdx < inputTPNmb ; ++tpIdx )
        {
          newW = inputParams.tiePointsWeights_[ tpIdx ];
          newW /= originalWSum;
          newW *= ((double)RAND_MAX) + 1.0;

          newWSum += newW;

          tpsMap[ newWSum ] = inputParams.tiepoints_[ tpIdx ];
        }
      }
      else
      {
        double wSum = 0;
        const double increment = ( ((double)RAND_MAX) + 1.0 ) / 
          ((double)inputParams.tiepoints_.size());
        
        for( unsigned int tpIdx = 0 ; tpIdx < inputTPNmb ; ++tpIdx )
        {
          wSum += increment;
          tpsMap[ wSum ] = inputParams.tiepoints_[ tpIdx ];
        }
      }

/*
      std::map< double, TeCoordPair >::const_iterator mapIt =
        tpsMap.begin();
      while( mapIt != tpsMap.end() )
      {
        std::cout << " [" << mapIt->first 
          << "," << mapIt->second.pt1.x_
          << "," << mapIt->second.pt1.y_
          << "," << mapIt->second.pt2.x_
          << "," << mapIt->second.pt2.y_
          << "]";
        ++mapIt;
      }
*/
    }
    
    // The Maximum Number of iterations
    
    const RansacItCounterT maxIterations = std::numeric_limits< RansacItCounterT >::max();
      
    // Transformation maximum errors
    
    const double maxDMapErr = inputParams.max_dmap_error_;
    const double maxIMapErr = inputParams.max_imap_error_;
    const double maxDRMSE = inputParams.max_dmap_rmse_;
    const double maxIRMSE = inputParams.max_imap_rmse_;
      
    // This is the maximum number of allowed consecutive invalid
    // consensus sets (no transformation generated)
    const RansacItCounterT maxConsecutiveInvalidBaseSets = ( 
      ((RansacItCounterT)inputTPNmb) * 
      ((RansacItCounterT)reqTPsNmb) * ((RansacItCounterT)reqTPsNmb) * 
      ((RansacItCounterT)reqTPsNmb) );
      
    // This is the minimum number of consecutive valid
    // consensus sets (with the same number of tie-points)
    const RansacItCounterT minConsecutiveValidBaseSets = 
      ( ((RansacItCounterT)inputTPNmb) * ((RansacItCounterT)reqTPsNmb) );
      
    // If the difference between consecutive valid sets RMSE drops
    // below this value the process is stoped.
    const double consecValidBaseSetsDRMSEThr = MIN( maxDMapErr / 2.0,
      maxDRMSE );
    const double consecValidBaseSetsIRMSEThr = MIN( maxIMapErr / 2.0,
      maxIRMSE );
    
    TeGTParams bestParams;
    bestParams.transformation_name_ = inputParams.transformation_name_;
    
    double bestParamsMaxDMapErr = maxDMapErr;
    double bestParamsMaxIMapErr = maxIMapErr;
    double bestParamsDRMSE = maxDRMSE;
    double bestParamsIRMSE = maxIRMSE;
    double bestParamsConvexHullArea = -1.0;

    TeGTParams consensusSetParams;
    consensusSetParams.transformation_name_ = inputParams.transformation_name_;
    
    double consensusSetMaxDMapErr = 0;
    double consensusSetMaxIMapErr = 0;
    double consensusSetDRMSE = 0;
    double consensusSetIRMSE = 0;
    double consensusSetIRMSEDiff = 0; // diff against bestParamsIRMSE
    double consensusSetDRMSEDiff = 0; // diff against bestParamsDRMSE
    unsigned int consensusSetSize = 0;
    double consensusSetConvexHullArea = 0.0;
    std::vector< TeCoord2D > consensusSetPt1ConvexHullPoits;
    
    double tiePointDMapErr = 0;
    double tiePointIMapErr = 0;    
    
    std::map< double, TeCoordPair >::const_iterator tpsMapIt;
    unsigned int inputTpIdx = 0;
      
    RansacItCounterT selectedTpsCounter = 0;
    std::vector< TeCoordPair* > selectedTpsPtrsVec;
    selectedTpsPtrsVec.resize( reqTPsNmb, 0 );
    unsigned int selectedTpsPtrsVecIdx = 0;
    bool selectedTpsNotSelectedBefore = false;
      
    // RANSAC main loop
    
    RansacItCounterT consecutiveInvalidBaseSetsLeft = 
      maxConsecutiveInvalidBaseSets;  
    RansacItCounterT iterationsLeft = maxIterations;       
    RansacItCounterT consecutiveValidBaseSets = 0;
      
    while( iterationsLeft )
    {
      // Trying to find a valid base consensus set
      // with the minimum number of required tie-points
      // Random selecting n distinc tpoints from the original set

      consensusSetParams.tiepoints_.clear();
      selectedTpsCounter = 0;

      while( selectedTpsCounter < reqTPsNmb )
      {
        tpsMapIt = tpsMap.upper_bound( (double)rand() );
        TEAGN_DEBUG_CONDITION( tpsMapIt != tpsMap.end(),
          "Tie-point random selection error" );
          
        // Checking if this TP was already selected before
        
        selectedTpsNotSelectedBefore = true;
        
        for( selectedTpsPtrsVecIdx = 0 ; selectedTpsPtrsVecIdx <
          selectedTpsCounter ; ++selectedTpsPtrsVecIdx )
        {
          if( selectedTpsPtrsVec[ selectedTpsPtrsVecIdx ] ==
            &(tpsMapIt->second) )
          {
            selectedTpsNotSelectedBefore = false;
            break;
          }
        }
        
        if( selectedTpsNotSelectedBefore )
        {
          consensusSetParams.tiepoints_.push_back( tpsMapIt->second );
        }
        
        ++selectedTpsCounter;
      }

      /* Trying to generate a valid base consensus transformation with the 
      selected points */      
      
      if( computeParameters( consensusSetParams ) ) 
      {
//        consecutiveInvalidBaseSetsLeft = maxConsecutiveInvalidBaseSets;
        
        // finding those tie-points in agreement with the generated
        // consensus basic transformation

        consensusSetParams.tiepoints_.clear();
        consensusSetDRMSE = 0;
        consensusSetIRMSE = 0;
        consensusSetMaxDMapErr = 0;
        consensusSetMaxIMapErr = 0;

        for( inputTpIdx = 0 ; inputTpIdx < inputTPNmb ; ++inputTpIdx )
        {
          const TeCoordPair& curTP = inputParams.tiepoints_[ inputTpIdx ];
          
          tiePointDMapErr = getDirectMappingError( curTP, consensusSetParams );
          tiePointIMapErr = getInverseMappingError( curTP, consensusSetParams );

          if( ( tiePointDMapErr < maxDMapErr ) && 
            ( tiePointIMapErr < maxIMapErr ) )
          {
            consensusSetParams.tiepoints_.push_back( curTP );
            consensusSetDRMSE += ( tiePointDMapErr * tiePointDMapErr );
            consensusSetIRMSE += ( tiePointIMapErr * tiePointIMapErr );
            if( tiePointDMapErr > consensusSetMaxDMapErr )
              consensusSetMaxDMapErr = tiePointDMapErr;
            if( tiePointIMapErr > consensusSetMaxIMapErr )
              consensusSetMaxIMapErr = tiePointIMapErr;              
          }
        }
        
        consensusSetSize = (unsigned int)consensusSetParams.tiepoints_.size();
        
        consensusSetDRMSE = sqrt( consensusSetDRMSE );
        consensusSetIRMSE = sqrt( consensusSetIRMSE );
        
        consensusSetIRMSEDiff = ABS( bestParamsIRMSE - consensusSetIRMSE );
        consensusSetDRMSEDiff = ABS( bestParamsDRMSE - consensusSetDRMSE );

        /* Is this an acceptable consensus set ?? */      
        
        if( ( consensusSetDRMSE < maxDRMSE ) &&
            ( consensusSetIRMSE < maxIRMSE ) &&
            ( consensusSetMaxDMapErr < maxDMapErr ) &&
            ( consensusSetMaxIMapErr < maxIMapErr ) )
        {
          // Is this consensus set better than the current better one ??
          // (by using the number of tie-points as parameter
          // since we are interested in consensus sets with
          // the higher number of tie-points
        
          if( consensusSetSize > bestParams.tiepoints_.size() )
          {
            bestParams = consensusSetParams;
            bestParamsDRMSE = consensusSetDRMSE;
            bestParamsIRMSE = consensusSetIRMSE;
            bestParamsMaxDMapErr = consensusSetMaxDMapErr;
            bestParamsMaxIMapErr = consensusSetMaxIMapErr;
            bestParamsConvexHullArea = -1.0;
            
            consecutiveValidBaseSets = 0;
            
            consecutiveInvalidBaseSetsLeft = maxConsecutiveInvalidBaseSets;
          }
          else if( consensusSetSize == bestParams.tiepoints_.size() )
          {
            // Calculating the best consensus set convexhull area , if needed
            
            if( bestParamsConvexHullArea < 0.0 )
            {
              bestParamsConvexHullArea = getPt1ConvexHullArea( 
                bestParams.tiepoints_ );
            }
            
            // Calculating the convexhull area covered by the tie-points (PT1 space)
            
            consensusSetConvexHullArea = getPt1ConvexHullArea( 
              consensusSetParams.tiepoints_ );
              
            if( consensusSetConvexHullArea > bestParamsConvexHullArea )
            {
              bestParams = consensusSetParams;
              bestParamsDRMSE = consensusSetDRMSE;
              bestParamsIRMSE = consensusSetIRMSE;
              bestParamsMaxDMapErr = consensusSetMaxDMapErr;
              bestParamsMaxIMapErr = consensusSetMaxIMapErr;
              bestParamsConvexHullArea = consensusSetConvexHullArea;
              
              ++consecutiveValidBaseSets;
            
              consecutiveInvalidBaseSetsLeft = maxConsecutiveInvalidBaseSets;            
            }
            else
            {
              // This consensus set isn't good enough
              
              --consecutiveInvalidBaseSetsLeft;
            }
          }
          else
          {
            // This consensus set isn't good enough
                        
            --consecutiveInvalidBaseSetsLeft;
          }
        }
        else
        {
          /* This isn't an acceptable consensus */           

          --consecutiveInvalidBaseSetsLeft;
        }
      } 
      else
      {
        // decrement the number of remaining consecutive
        // invalid base sets left
                    
        --consecutiveInvalidBaseSetsLeft;

      }
      
                        
      if( consecutiveInvalidBaseSetsLeft  == 0 ) break;
      
      if( ( consecutiveValidBaseSets > minConsecutiveValidBaseSets ) &&
        ( consensusSetDRMSEDiff < consecValidBaseSetsDRMSEThr ) &&
        ( consensusSetIRMSEDiff < consecValidBaseSetsIRMSEThr ) )
      {
        break;
      }
      
      --iterationsLeft;
    }
    
    if( bestParams.tiepoints_.size() >= reqTPsNmb )
    {
      // Generating the smoothed transformation
      
      if( computeParameters( bestParams ) ) 
      {
        outputParams = bestParams;
        
        return true;
      }
      else
      {
        return false;
      }      
    }
    else
    {
      return false;
    }
  }
}

bool TeGeometricTransformation::normalizeTPWeights( 
  std::vector< double >& tpWeights ) const
{
  const unsigned int size = (unsigned int)tpWeights.size();
  
  if( size )
  {
    double wSum = 0;
    unsigned int idx = 0;
    
    for( ; idx < size ; ++idx )
    {
      if( tpWeights[ idx ] < 0.0 ) return false;

      wSum += tpWeights[ idx ];
    }
    
    if( wSum > 0.0 )
    {
      for( idx = 0 ; idx < size ; ++idx )
      {
        tpWeights[ idx ] /= wSum;
      }
    }
  }

  return true;
}

double TeGeometricTransformation::getPt1ConvexHullArea( 
  const TeCoordPairVect& tiePoints ) const
{
  const unsigned int tiePointsSize = (unsigned int)tiePoints.size();
  
  if( tiePointsSize < 3 )
  {
    return 0;
  }
  else
  {
    std::vector< TeCoord2D > coords;
    coords.resize( tiePointsSize );
    
    for( unsigned int tpIdx = 0 ; tpIdx < tiePointsSize ; ++tpIdx )
    {
      coords[ tpIdx ] = tiePoints[ tpIdx ].pt1;
    }
    
    return TeGeometryArea( TeConvexHull( coords ) );
  }
}

