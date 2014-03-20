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

#ifndef TEPDIBAATZ_HPP
  #define TEPDIBAATZ_HPP

  // Internal defines
  #define MAX_STEPS 20 /* maximum number of segmentation steps */
  #define NUM_BANDS 10 /* number of image bands */
  #define POSITION_SEARCH 0.00 /* minimum percentage of unused segments for segment selection by pixel position */
  #define OUTLINE_COLOR_0 255 /* segments' outline color for output image */
  #define OUTLINE_COLOR_1 000
  #define OUTLINE_COLOR_2 000

  // TerraLib includes
  #include "TePDIAlgorithm.hpp"
  #include "TePDITypes.hpp"
  #include "TePDIPIManager.hpp"
  #include "TePDIUtils.hpp"
  #include "../kernel/TeSharedPtr.h"

  /**
    * @brief Region Growing Segmentation, Baatz implementation.
    * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
    * @author Gilson Costa <gilson@ele.puc-rio.br>
    * @ingroup TePDISegmentationGroup
    *
    * @note Reference: Baatz, M.; Schape, A. Multiresolution segmentation: an 
    * optimization approach for high quality multi-scale image segmentation. 
    * In: XII Angewandte Geographische Informationsverarbeitung, 
    * Wichmann-Verlag, Heidelberg, 2000.
    *
    * @note The general required parameters :
    *
    * @param input_image (TePDITypes::TePDIRasterPtrType)
    * @param input_weights (vector<float>) one weight for each input image channel
    * @param input_channels (vector<unsigned>) used channels for input image
    * @param scale (float) Parameter scale is > 0 (Has direct influence
    * over the generated segments size).
    * @param compactness (float) Parameter compactness is > 0 and <= 1 (
    compactness = perimeter / sqrt( area ) ).
    * @param color (float) Parameter color is > 0 and <= 1 (the relative ratio
    * between spacial components and spectral components).
    * @param output_image (TePDITypes::TePDIRasterPtrType) - Output raster
    * pointer.
    *
    * @note The optional parameters (will be used only if present):
    *
    * @param output_polsets (TePDITypes::TePDIPolSetMapPtrType) - 
    * A pointer to the output polygon sets map.
    *
    */

/**********************************************************************************/
/* TerraLib main class for Baatz Segmentation                                     */
/**********************************************************************************/

  class PDI_DLL TePDIBaatz : public TePDIAlgorithm {
    public :
      
      /**
       * @brief Default Constructor.
       *
       */
      TePDIBaatz();

      /**
       * @brief Default Destructor
       */
	  ~TePDIBaatz() {};
      
      /**
       * @brief Checks if the supplied parameters fits the requirements of 
       * each PDI algorithm implementation.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      bool CheckParameters( const TePDIParameters& parameters ) const;      

  protected:    
      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& );    
     
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();
      
  };
  
/** @example TePDISegmentation_test.cpp
 *    Shows how to use this class.
 */    


/**********************************************************************************/
/* Internal definitions Baatz Segmentation                                        */
/**********************************************************************************/

/**********************************************************************************/
/* type definitions                                                               */
/**********************************************************************************/

/* node of list of segments' pixels */
struct segment_pixel 
{
  unsigned long int pixel_id; /* pixel id */
  bool borderline; /* indicates if the pixel belongs to the border of the segment */
  struct segment_pixel *next_pixel; /* next pixel */
};

/* node of segment list */
struct segment 
{
  unsigned long int id; /* segment id */
  float area; /* number of pixels in segment */
  float perimeter; /*number of border pixels */
  float b_box[4]; /* bounding box of the segment, relative to rows and cols */
  float avg_color[NUM_BANDS]; /* average colors of pixels, one for each band */
  float std_color[NUM_BANDS]; /* std of pixel colors, one for each band */
  float avg_color_square[NUM_BANDS];
  float color_sum[NUM_BANDS];
  int used; /* indicate if segment has been used in segmentation step */
  struct segment_pixel *pixel_list; /* list of indexes of the segment's pixels */
  struct segment_pixel *last_pixel; /* pointer to the last pixel in the pixel list */
  struct segment_pixel *original_pixel; /* pointer to original pixel of the segment */
  struct segment *previous_segment; /* previous not deleted segment */
  struct segment *next_segment; /* next not deleted segment */
  struct segment *previous_unused_segment; /* previous unused segment */
  struct segment *next_unused_segment; /* next unused segment */
  struct segment *next_original_segment; /* next original segment */
};

/* node of segments' neighbours */
struct segment_neighbor 
{
  float area; /* number of pixels in merged segment, neighbour plus original */
  float perimeter; /*number of border pixels in merged segment, neighbour plus original */
  float b_box[4]; /* bounding box of the segment of merged segment, neighbour plus original */
  float avg_color[NUM_BANDS]; /* average colors of pixels of merged segment, one for each band */
  float std_color[NUM_BANDS]; /* std of pixel colors of merged segment, one for each band */
  float avg_color_square[NUM_BANDS];
  float color_sum[NUM_BANDS];
  unsigned long int neighbor_id; /* id of neighbor segment */
  struct segment *neighbor; /* neighbor segment */
  struct segment_neighbor *next_neighbor; /* next neighbor */
};

/* record of segmentation parameters */
struct segmentation_parameters 
{
  float sp; /* scale parameter */
  float wband[NUM_BANDS]; /* weight of image bands, between 0 and 1 */
  float wcolor; /* factor of color/shape, between 0 and 1 */
  float wcmpt; /* factor of compactness/smothness, between 0 and 1 */
  int bands; /* number of bands */
};

/**********************************************************************************/
/* function prototypes                                                            */
/**********************************************************************************/

/* returns random number between low and high */
float randval(float low, 
        float high); 

/* read parameter file */
int read_seg_parameters(char *parameter_file_name,
            struct segmentation_parameters *parameters); 

/* creates one segment for each pixel of input image */
int initialize_segments(struct segment ***segments_ptr_vector, 
            struct segment **first_segment, 
            struct segment **last_segment, 
            // float **input_image, 
            TePDITypes::TePDIRasterPtrType input_image,
            vector<unsigned> input_channels, 
            long int nrows, 
            long int ncols,
            bool progress_enabled_);  

/* performs the segmentation, returns (num.segments + segmentation steps/100) */
float segmentation(// float **input_image, 
           struct segment **segments_ptr_vector, 
           struct segment *initial_segment, 
           struct segment *final_segment, 
           long int nrows, 
           long int ncols, 
           struct segmentation_parameters parameters,
           bool progress_enabled_); 

/* write output image */
void write_segments(// float **input_image, 
          TePDITypes::TePDIRasterPtrType output_image,
          struct segment *initial_segment, 
          int write_type,
          bool progress_enabled_); 

/* remove segments from memory */
void free_segment_list(struct segment **initial_segment, 
             struct segment ***segments_ptr_vector);

#endif
