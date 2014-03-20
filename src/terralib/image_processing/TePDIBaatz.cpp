#define MAX_FLT 3.4e38 /* maximum float value */

// Internal includes
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <math.h>

// TerraLib includes
#include "TePDIBaatz.hpp"
#include "TePDIRaster2Vector.hpp"
#include "TePDIUtils.hpp"

using namespace std;

/**********************************************************************************/
/* TerraLib main implementation for Baatz Segmentation                            */
/**********************************************************************************/

TePDIBaatz::TePDIBaatz()
{
}

bool TePDIBaatz::CheckParameters( const TePDIParameters& parameters ) const
{
	// checking input_image
    TePDITypes::TePDIRasterPtrType input_image;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "input_image", input_image ),
		"Missing parameter: input_image");
	TEAGN_TRUE_OR_RETURN(input_image.isActive(),
		"Invalid parameter: input_image inactive");
	TEAGN_TRUE_OR_RETURN(input_image->params().status_ != TeRasterParams::TeNotReady ,
		"Invalid parameter: input_image not ready");
	vector<float> input_weights;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "input_weights", input_weights ),
    "Missing parameter: input_weights");
  TEAGN_TRUE_OR_RETURN(input_weights.size() <= (unsigned) input_image->params().nBands() && input_weights.size() > 0,
    "Input weights different from number of bands");
  vector<unsigned> input_channels;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "input_channels", input_channels ),
    "Missing parameter: input_channels");
  TEAGN_TRUE_OR_RETURN(input_channels.size() == input_weights.size(),
    "Input bands with different size from input weights");
   
	// checking output_image
    TePDITypes::TePDIRasterPtrType output_image;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "output_image", output_image ),
		"Missing parameter: output_image");

 	// checking thresholds
	float scale;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "scale", scale ),
		"Missing parameter: scale");
	TEAGN_TRUE_OR_RETURN(scale > 0,
		"Parameter scale is > 0");
 	float compactness;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "compactness", compactness ),
		"Missing parameter: compactness");
	TEAGN_TRUE_OR_RETURN(compactness > 0 && compactness <= 1,
		"Parameter compactness is > 0 and <= 1");
 	float color;
	TEAGN_TRUE_OR_RETURN(parameters.GetParameter( "color", color ),
		"Missing parameter: color");
	TEAGN_TRUE_OR_RETURN(color > 0 && color <= 1,
		"Parameter color is > 0 and <= 1");
    
   // checking output polygon sets parameter
  if( parameters.CheckParameter< TePDITypes::TePDIPolSetMapPtrType >( 
    "output_polsets" ) ) {
    TePDITypes::TePDIPolSetMapPtrType output_polsets;
    parameters.GetParameter( "output_polsets", output_polsets );
    TEAGN_TRUE_OR_RETURN( output_polsets.isActive(),
      "Invalid parameter output_polsets : Inactive polygon set pointer" );   
  }   

	return true;
}

void TePDIBaatz::ResetState( const TePDIParameters& )
{
}
     
bool TePDIBaatz::RunImplementation()
{
  TePDITypes::TePDIRasterPtrType input_image;
	params_.GetParameter( "input_image", input_image );

  vector<float> input_weights;
  params_.GetParameter( "input_weights", input_weights );

  vector<unsigned> input_channels;
  params_.GetParameter( "input_channels", input_channels );
  
	/* retrieve input_image parameters */
	int H = input_image->params().nlines_,
		W = input_image->params().ncols_,
		B = input_channels.size();

	/* retrieve algorithm parameters */
	float scale;
	params_.GetParameter( "scale", scale );
	float compactness;
	params_.GetParameter( "compactness", compactness );
	float color;
	params_.GetParameter( "color", color );

  struct segment **segments_ptr_vector = NULL;
  struct segment *initial_segment = NULL;
  struct segment *final_segment = NULL;
  struct segmentation_parameters seg_parameters;

	seg_parameters.sp = scale;
	seg_parameters.wcmpt = compactness;
	seg_parameters.wcolor = color;
	seg_parameters.bands = B;
	for (int i = 0; i < B; i++)
		seg_parameters.wband[i] = input_weights[i];

	/* run segmentation */
/*
	initialize_segments(&segments_ptr_vector, 
		&initial_segment, &final_segment, input_image, H, W, progress_enabled_);
	segmentation(segments_ptr_vector, 
		initial_segment, final_segment, H, W, seg_parameters, progress_enabled_);
*/
/*
	float **float_input_image = (float **) malloc( B * sizeof(float) );
	for (int i = 0; i < B; i++)
		float_input_image[i] = (float *) malloc( H * W * sizeof(float) );

	int p = 0;
	cout << "H: " << H << " W: " << W << " B: " << B << endl;
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			for (int k = 0; k < B; k++)
			{
				double pixel;
				input_image->getElement(j, i, pixel, k);
				float_input_image[k][p] = (float) pixel;
			}
			p++;
		}
*/
	initialize_segments(&segments_ptr_vector, 
		&initial_segment, &final_segment, input_image, input_channels, H, W, progress_enabled_);

  segmentation(segments_ptr_vector, 
		initial_segment, final_segment, H, W, seg_parameters, progress_enabled_);

  /* write results */
  
	TePDITypes::TePDIRasterPtrType output_image;
	params_.GetParameter( "output_image", output_image );
	TeRasterParams output_image_params = output_image->params();
	output_image_params.nBands( 1 );
	output_image_params.setDataType( TeUNSIGNEDLONG );
  output_image_params.boundingBoxLinesColumns( 
    input_image->params().boundingBox().x1(),
    input_image->params().boundingBox().y1(),
    input_image->params().boundingBox().x2(),
    input_image->params().boundingBox().y2(),
    input_image->params().nlines_,
    input_image->params().ncols_ );
  output_image_params.projection( input_image->params().projection() );
	TEAGN_TRUE_OR_RETURN( output_image->init(output_image_params),
    "Error initiating output image" );
     
	write_segments(output_image, initial_segment, 0, progress_enabled_); 
  free_segment_list(&initial_segment, &segments_ptr_vector); 
/*
	p = 0;
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			double pixel = (double) float_input_image[0][p++];
			output_image->setElement(j, i, pixel);
		}
*/

	/* generate output polygonset */
  if( params_.CheckParameter< TePDITypes::TePDIPolSetMapPtrType >( 
    "output_polsets" ) ) {
    TePDITypes::TePDIPolSetMapPtrType output_polsets;
    params_.GetParameter( "output_polsets", output_polsets );

    TePDIParameters params_output;
    params_output.SetParameter( "rotulated_image", output_image );
    params_output.SetParameter( "channel", (unsigned int) 0 );
    params_output.SetParameter( "output_polsets", output_polsets );

    TePDIRaster2Vector raster2Vector;  

    TEAGN_TRUE_OR_THROW( raster2Vector.Reset(params_output),
      "Invalid Parameters for raster2Vector" );

    TEAGN_TRUE_OR_THROW( raster2Vector.Apply(), 
      "Apply error" );
  }   

  return true;
}

/**********************************************************************************/
/* Internal main implementation for Baatz Segmentation                            */
/**********************************************************************************/

/* remove segment from list and conects segment list neighbours */
void remove_segment(struct segment *aux_segment, 
          struct segment **first_segment, 
          struct segment **last_segment);

/* merge neighbor with current_segment and removes neighbor segment */
int merge_segment(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **first_segment, 
          struct segment **last_segment,
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols); 

/* identifies borderline pixels and change segment_matrix */
void reset_pixels(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols);
 
/* creates list of segment's image neighbours */
struct segment_neighbor* find_neighbors(struct segment *curr_segment, 
                    struct segment **segments_ptr_vector, 
                    long int nrows, 
                    long int ncols); 

/* finds the 4 neighbors of a pixel */
void find_neighbor_pixels(unsigned long int pixel_id, 
              long int nrows, 
              long int ncols, 
              long int *n0, 
              long int *n1, 
              long int *n2, 
              long int *n3);
 
/* frees list of segment's neighbors from memory */
void free_neighbor_list(struct segment_neighbor **first_neighbor); 

/* calcs mean and stddev of merging of curr_segment and curr_neighbor */
float calc_color_stats (struct segment_neighbor *curr_neighbor, 
            struct segment *curr_segment, 
            // float **input_image, 
            struct segmentation_parameters parameters);
 
/* calcs spatial heterogeneity of merging of curr_segment and curr_neighbor */
float calc_spatial_stats(struct segment_neighbor *curr_neighbor, 
             struct segment *curr_segment, 
             struct segment *segments_ptr_vector, 
             long int nrows, 
             long int ncols, 
             struct segmentation_parameters parameters);
 
/* calcs perimeter of merging of curr_segment and curr_neighbor */
void calc_perimeter(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols); 

/* calcs perimeter of merging of curr_segment and curr_neighbor, second version */
void calc_perimeter_optimized(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols); 

/* calcs bounding box of merging of curr_segment and curr_neighbor */
void calc_bounding_box(struct segment_neighbor *curr_neighbor, 
             struct segment *curr_segment); 

/* mark all segments as unused */
void reset_unused_segments_list(unsigned long int num_segments, 
                 struct segment *first_segment); 

/* search unused segment using a dither matrix */
struct segment* find_unused_by_dither(struct segment **segments_ptr_vector, 
                    int num_bits, 
                    long int nrows, 
                    long int ncols,
                    unsigned long int *cell_count);

/* finds col and row from a pixel id */
void convert_pixel_id(unsigned long int pixel_id, 
            long int ncols, 
            long int *row, 
            long int *col); 

/**********************************************************************************/
/* ranval: random number generator                                                */
/**********************************************************************************/

float ranval(float low, 
       float high)
{
  float val;

  val = (((float) rand())/RAND_MAX) * (high - low) + low;

  return(val);
}

/**********************************************************************************/
/* function dither_matrix_bits: calculate number of bits necessary to hold a      */
/*                              coordinate subscript of dither (square) matrix    */
/**********************************************************************************/
unsigned long int dither_matrix_dimension(long int nrows,
                      long int ncols,
                      int *dither_bits)
{
  int num_bits;
  unsigned long int max_dimension;
  unsigned long int dither_dimension;
  
  if (nrows>ncols)
    max_dimension=nrows*nrows;
  else
    max_dimension=ncols*ncols;

  num_bits=0;
  dither_dimension=1;

  while (dither_dimension < max_dimension)
  {
    dither_dimension = dither_dimension*2;
    num_bits++;
  }

  if (num_bits%2)
  {
    num_bits++;
    dither_dimension = dither_dimension*2;
  }

  *dither_bits = num_bits;

  return(dither_dimension);
}

/**********************************************************************************/
/* function dither_matrix_conversion: converts an pixel index into a dither       */
/*                                    matrix index                                */
/**********************************************************************************/
unsigned long int dither_matrix_conversion(int num_bits, 
                       long int nrows, 
                       long int ncols,
                       unsigned long int *cell_count)
{
  long int row, col, irow, icol;
  unsigned long int pixel_id;
  unsigned long int aux_cell_count;
  int i, j, k;
  bool found;

  aux_cell_count = *cell_count;
  found = false;

  while (!found) 
  {
    irow = icol = 0;
    j = k = 1;
    for (i=0;i<num_bits;i++)
    {
      if (i % 2)
      {
        icol = icol+(aux_cell_count&j)/k;
      }
      else
      {
        irow = irow+(aux_cell_count&j)/k;
        k = k * 2;
      }
      j = j * 2;
    }

    row = col = 0;
    j = 1;
    for (i=0;i<(num_bits/2);i++)
    {
      k = k / 2;
      if (irow & j) row = row+k;
      if (icol & j) col = col+k;
      j = j * 2;
    }
    
    if ((col >= ncols)||(row >= nrows))
    {
      found = false;
      aux_cell_count++;
    }
    else
    {
      pixel_id = (ncols*row)+col;
      found = true;
    }
  }

  *cell_count = aux_cell_count;
  return(pixel_id);
}

/**********************************************************************************/
/* function create_initial_segments: creates initial segment list                 */
/**********************************************************************************/

int initialize_segments(struct segment ***segments_ptr_vector, 
            struct segment **first_segment, 
            struct segment **last_segment, 
            // float **input_image, 
            TePDITypes::TePDIRasterPtrType input_image,
            vector<unsigned> input_channels, 
            long int nrows, 
            long int ncols,
            bool progress_enabled_)
{
  struct segment *aux_segment = 0;
  struct segment *prev_segment;
  struct segment_pixel *body_pixel;
  unsigned long int id_pixel;
  int row, col;
  int allocate_memory, error;

  error = 0;

  /* allocate memory for segments matrix if it has not been created yet */
  if (*segments_ptr_vector == NULL)
  {
  allocate_memory = true;
  *segments_ptr_vector = (struct segment **) malloc(nrows*ncols * sizeof(struct segment *));
  }
  else
  allocate_memory = false;

  if(*segments_ptr_vector == NULL) error = 1;

  row = 0;
  col = 0;
  id_pixel=0;
  prev_segment = NULL;

  TePDIPIManager pim("Initializing Data", nrows * ncols, progress_enabled_);
  while ((id_pixel<(unsigned long)(nrows*ncols)) && (error!=1))
  {

  if (allocate_memory)
  {
      aux_segment = (struct segment*) malloc(sizeof(struct segment));
    if(aux_segment == NULL) error = 1;
    
    body_pixel = (struct segment_pixel*) malloc(sizeof(struct segment_pixel));
    if(body_pixel == NULL) error = 1;

    aux_segment->next_original_segment = NULL;
    aux_segment->original_pixel = body_pixel;
  }
  else
  {
    if (id_pixel==0)
      aux_segment = *first_segment;
    else
      aux_segment = aux_segment->next_original_segment;
    
    body_pixel = aux_segment->original_pixel;
  }

  if (error!=1)
  {
    (*segments_ptr_vector)[id_pixel] = aux_segment;

    aux_segment->id = id_pixel;
    aux_segment->area = 1;
    aux_segment->perimeter = 4;
    aux_segment->b_box[0] = (float)row;
    aux_segment->b_box[1] = (float)col;
    aux_segment->b_box[2] = 1;
    aux_segment->b_box[3] = 1;

    for(unsigned b = 0; b < input_channels.size(); b++)
    {
      double pixel;
      input_image->getElement(col, row, pixel, input_channels[b]);
      aux_segment->avg_color[b] = (float) pixel;
      aux_segment->std_color[b] = 0;
      aux_segment->avg_color_square[b] = (float) (pixel * pixel);
      aux_segment->color_sum[b] = (float) pixel;
    }
/*
    for(int b = 0; b < NUM_BANDS; b++)
    {
      aux_segment->avg_color[b] = input_image[b][id_pixel];
      aux_segment->std_color[b] = 0;
      aux_segment->avg_color_square[b] = (float)(input_image[b][id_pixel])*(input_image[b][id_pixel]);
      aux_segment->color_sum[b] = input_image[b][id_pixel];
    }
*/
    aux_segment->used = false;

    body_pixel->pixel_id = id_pixel;
    body_pixel->borderline = true;
    body_pixel->next_pixel = NULL;
    aux_segment->pixel_list = body_pixel;
    aux_segment->last_pixel = body_pixel;
    
    aux_segment->next_segment = NULL;
    aux_segment->previous_segment = prev_segment;

    if (prev_segment)
    {
      prev_segment->next_segment = aux_segment;
      prev_segment->next_original_segment = aux_segment;

      *last_segment = aux_segment;
    }
    else
    {
      *first_segment = aux_segment;
    }

    prev_segment = aux_segment;
    
    col++;
    if (col==ncols)
    {
      col = 0;
      row++;
    }
    
      id_pixel++;
    pim.Increment();
  }
  }
  pim.Toggle(false);
  if(error)
  {
    /* out of memory space */
    return(1);
  }
  return(0);
}

/**********************************************************************************/
/* function find_unused_by_dither : finds segment using dither matrix             */
/**********************************************************************************/

struct segment* find_unused_by_dither(struct segment **segments_ptr_vector, 
                    int num_bits, 
                    long int nrows, 
                    long int ncols,
                    unsigned long int *cell_count)

{
  struct segment *aux_segment;
  unsigned long int pixel_id;
  bool found;

  found = false;

  while (!found)
  {
  pixel_id=dither_matrix_conversion(num_bits, nrows, ncols, cell_count);
  aux_segment = segments_ptr_vector[pixel_id];

    if (aux_segment->used)
    *cell_count=*cell_count+1;
  else
    found = true;
  }
  
  return(aux_segment);
}

/**********************************************************************************/
/* function convert_pixel_id: finds col and row from pixel id                     */
/**********************************************************************************/

void convert_pixel_id(unsigned long int pixel_id, 
            long int ncols, 
            long int *row, 
            long int *col)
{
  *row = (long)floor((double)pixel_id / (double)ncols);
  *col = pixel_id % ncols;
}

/**********************************************************************************/
/* function free_neighbor_list: frees list of segment's neighbors from memory     */
/**********************************************************************************/

void free_neighbor_list(struct segment_neighbor **first_neighbor)
{
  struct segment_neighbor *aux_neighbor;
  
  while (*first_neighbor!=NULL)
  {
    aux_neighbor = (*first_neighbor)->next_neighbor;
    free(*first_neighbor);
    *first_neighbor = aux_neighbor;
  }
}

/**********************************************************************************/
/* function find_neighbors: creates list of segment's image neighbors             */
/**********************************************************************************/

struct segment_neighbor* find_neighbors(struct segment *curr_segment, 
                    struct segment **segments_ptr_vector, 
                    long int nrows, long int ncols)
{
  int i, not_in_neighbors_list;
  long int neighbor_pixel[4][2]; /* first col contain pixel id, second contains segment id */
  struct segment_neighbor *first_neighbor;
  struct segment_neighbor *aux_neighbor;
  struct segment_neighbor *new_neighbor;
  struct segment_pixel *aux_pixel;
  struct segment *aux_segment;
   
  aux_pixel = curr_segment->pixel_list;
  first_neighbor = NULL;

  /* for each outline pixel */
  while (aux_pixel!=NULL)
  {
    if (aux_pixel->borderline==true)
    {
      find_neighbor_pixels(aux_pixel->pixel_id, nrows, ncols, 
      neighbor_pixel[0], neighbor_pixel[1], neighbor_pixel[2], neighbor_pixel[3]);
      /* finds segments to which pixel belongs to */
      for (i=0;i<4;i++)
      {
        if (neighbor_pixel[i][0]!=-1)
        {
          aux_segment = segments_ptr_vector[neighbor_pixel[i][0]];
          neighbor_pixel[i][1] = aux_segment->id;

          if (neighbor_pixel[i][1] == (int) curr_segment->id)
            neighbor_pixel[i][1]=-1;
        }
        else
        {
          neighbor_pixel[i][1]=-1;
        }
        /* if pixel segment is not yet in the list of segment neighbors, includes it */
        if (neighbor_pixel[i][1]>-1)
        {
          not_in_neighbors_list = true;
          aux_neighbor = first_neighbor;
          while ((aux_neighbor!=NULL)&&(not_in_neighbors_list))
          {
            if (aux_neighbor->neighbor_id == (unsigned) neighbor_pixel[i][1])
            {
              not_in_neighbors_list = false;
            }
            else
            {
              aux_neighbor = aux_neighbor->next_neighbor;
            }
          }
          if (not_in_neighbors_list)
          {
            new_neighbor = (struct segment_neighbor*) malloc(sizeof(struct segment_neighbor));
            new_neighbor->neighbor_id = neighbor_pixel[i][1];
            new_neighbor->neighbor = segments_ptr_vector[neighbor_pixel[i][1]];

            if (first_neighbor==NULL)
            {
              new_neighbor->next_neighbor = NULL;
            }
            else
            {
              new_neighbor->next_neighbor = first_neighbor;
            }
            first_neighbor = new_neighbor;
          }
        }
      }
    }
    aux_pixel = aux_pixel->next_pixel;
  }
  return (first_neighbor);
}

/**********************************************************************************/
/* function find_neighbor_pixels: returns the ids of the (4) neighbors of a pixel */
/**********************************************************************************/

void find_neighbor_pixels(unsigned long int pixel_id, 
              long int nrows, 
              long int ncols, 
              long int *n0, 
              long int *n1, 
              long int *n2, 
              long int *n3)
{
  long int row, col;

  convert_pixel_id(pixel_id, ncols, &row, &col);

  *n0 = *n1 = *n2 = *n3 = -1;
  
  if (row>0)         *n0=((row-1)*ncols) + col;      

  if (col>0)         *n1=(row*ncols) + (col-1);

  if (row<(nrows-1)) *n2=((row+1)*ncols) + col;

  if (col<(ncols-1)) *n3=(row*ncols) + (col+1);
}

/**********************************************************************************/
/* function calc_color_stats: calculate the mean color and the standard deviation */
/*           of the merging of two segments and writes it to the current neighbor */
/**********************************************************************************/

float calc_color_stats (struct segment_neighbor *curr_neighbor, 
            struct segment *curr_segment, 
            //float **input_image, 
            struct segmentation_parameters parameters)
{
  float mean[NUM_BANDS], colorSum[NUM_BANDS];
  float squarePixels[NUM_BANDS];
  float stddev[NUM_BANDS];
  float stddevNew[NUM_BANDS];
  float sum_wband, wband_norm[NUM_BANDS];
  float color_f[NUM_BANDS];
  float color_h;
  int b;
  long int count;
  count = 0;
  float a_current, a_neighbor, a_sum;
  struct segment *neighbor;

  neighbor = curr_neighbor->neighbor;

  a_current = curr_segment->area;
  a_neighbor = neighbor->area;
  a_sum = a_current+a_neighbor;
  sum_wband = 0;

  for (b = 0; b < parameters.bands; b++)
  {
    mean[b] = ((curr_segment->avg_color[b]*a_current)+(neighbor->avg_color[b]*a_neighbor))/a_sum;
  squarePixels[b] = (curr_segment->avg_color_square[b])+(neighbor->avg_color_square[b]);
  colorSum[b] = curr_segment->color_sum[b] + neighbor->color_sum[b];
    stddev[b] = 0;
  stddevNew[b] = 0;
  sum_wband = sum_wband + parameters.wband[b];
  }

  for(b = 0; b < parameters.bands; b++)
  {
    stddevNew[b] = squarePixels[b] - 2*mean[b]*colorSum[b] + a_sum*mean[b]*mean[b];
  }

  /* stores stddev and mean in neighbor */
  /* calculates color factor per band and total */
  color_h = 0;
  for (b = 0; b < parameters.bands; b++)
  {
    stddev[b] = sqrt(stddevNew[b]/a_sum);
  wband_norm[b] = parameters.wband[b]; /* new implementation, bands already weighted */

    color_f[b] = (a_current*curr_segment->std_color[b]) + 
    (a_neighbor*neighbor->std_color[b]);
    color_f[b] = wband_norm[b] * ((a_sum*stddev[b])-color_f[b]);
    color_h = color_h + color_f[b];
    
    curr_neighbor->avg_color[b] = mean[b];
    curr_neighbor->std_color[b] = stddev[b];
  curr_neighbor->avg_color_square[b] = squarePixels[b];
  curr_neighbor->color_sum[b] = colorSum[b];
  }
  return(color_h);
}

/**********************************************************************************/
/* function calc_spatial_stats: calcs spatial heterogeneity of merging of         */
/*                              curr_segment and curr_neighbor                    */
/**********************************************************************************/

float calc_spatial_stats(struct segment_neighbor *curr_neighbor, 
             struct segment *curr_segment, 
             struct segment **segments_ptr_vector, 
             long int nrows, 
             long int ncols, 
             struct segmentation_parameters parameters)
{
  struct segment *neighbor;  
  float spatial_h, smooth_f, compact_f;
  float area[3], perimeter[3], b_box_len[3]; /* 0-current segment; 
                          1-neighbor segment; 
                        2-merged (new) segment */

  neighbor = curr_neighbor->neighbor;

  /* area */
  area[0] = curr_segment->area;
  area[1] = neighbor->area;
  area[2] = area[0]+area[1];
  curr_neighbor->area = area[2];
  
  /* perimeter */
  perimeter[0] = curr_segment->perimeter;
  perimeter[1] = neighbor->perimeter;
  if (area[2]<4) /* valid only if pixel neighborhood==4 */
  {
    if (area[2]==2)
    curr_neighbor->perimeter = 6;
  else
    curr_neighbor->perimeter = 8; 
  }
  else
  {
//    calc_perimeter(curr_neighbor, curr_segment, segments_ptr_vector, nrows, ncols);
//cout << "original: " << curr_neighbor->perimeter << endl;
    calc_perimeter_optimized(curr_neighbor, curr_segment, segments_ptr_vector, nrows, ncols);
//cout << "optimized: " << curr_neighbor->perimeter << endl;

//    calc_perimeter(curr_neighbor, curr_segment, segments_ptr_vector, nrows, ncols);
  }
  perimeter[2] = curr_neighbor->perimeter;
  
  /* bounding box lenght */
  calc_bounding_box(curr_neighbor, curr_segment);
  b_box_len[0] = curr_segment->b_box[2]*2 + curr_segment->b_box[3]*2;
  b_box_len[1] = neighbor->b_box[2]*2 + neighbor->b_box[3]*2;
  b_box_len[2] = curr_neighbor->b_box[2]*2 + curr_neighbor->b_box[3]*2;

  /* smoothness factor */
  smooth_f = (area[2]*perimeter[2]/b_box_len[2] - 
    (area[1]*perimeter[1]/b_box_len[1] + area[0]*perimeter[0]/b_box_len[0]));
  
  /* compactness factor */
  compact_f = (area[2]*perimeter[2]/sqrt(area[2]) - 
    (area[1]*perimeter[1]/sqrt(area[1]) + area[0]*perimeter[0]/sqrt(area[0])));

  /* spatial heterogeneity */
  spatial_h = parameters.wcmpt*compact_f + (1-parameters.wcmpt)*smooth_f;
  return(spatial_h);
}

/**********************************************************************************/
/* function calc_perimeter: calculates the perimeter of merging of curr_segment   */
/*                          and curr_neighbor                                     */
/**********************************************************************************/

void calc_perimeter(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols)
{
  int i, j, k;
  float perimeter_total;
  unsigned long int neighbor_id, segment_id;
  long int neighbor_pixel[4][2]; /* first col contain pixel id, second contains segment id */
  struct segment_pixel *aux_pixel;
  struct segment *aux_segment; 

  perimeter_total = 0;

  for (j=0;j<2;j++) /* once for the current segment, once for the neighbor segment */
  {
    neighbor_id = curr_neighbor->neighbor_id;
    segment_id = curr_segment->id;
    if (j==1)
    {
      aux_pixel = curr_segment->pixel_list;
    }
    else
    {
      aux_pixel = curr_neighbor->neighbor->pixel_list;
    }
    /* for each outline pixel */
    while (aux_pixel!=NULL)
    {
      if (aux_pixel->borderline==true)
      {
        find_neighbor_pixels(aux_pixel->pixel_id, nrows, ncols, 
      &neighbor_pixel[0][0], &neighbor_pixel[1][0], &neighbor_pixel[2][0], &neighbor_pixel[3][0]);
        /* count how many of the surrounding pixels are from the own segment or neighbor */
        k = 0;
    for (i=0; i<4; i++)
          if (neighbor_pixel[i][0]==-1) /* image limit */
      k++;
          else
          {
            aux_segment = segments_ptr_vector[neighbor_pixel[i][0]];
            neighbor_pixel[i][1] = aux_segment->id;

            if ((neighbor_pixel[i][1] != (int) segment_id)&&(neighbor_pixel[i][1] != (int) neighbor_id))
              k++;
          }
        perimeter_total=perimeter_total+k; 
      }
      aux_pixel = aux_pixel->next_pixel;
    }
  }
  curr_neighbor->perimeter = perimeter_total;


}

/**********************************************************************************/
/* function calc_bounding_box: calcs bounding box of merging of curr_segment      */
/*   and curr_neighbor                                                            */
/**********************************************************************************/

void calc_bounding_box(struct segment_neighbor *curr_neighbor, 
             struct segment *curr_segment)
{
  float min_row[3], max_row[3], min_col[3], max_col[3];

  min_row[0] = curr_segment->b_box[0];
  max_row[0] = curr_segment->b_box[0]+curr_segment->b_box[2];
  min_col[0] = curr_segment->b_box[1];
  max_col[0] = curr_segment->b_box[1]+curr_segment->b_box[3];
  
  min_row[1] = curr_neighbor->neighbor->b_box[0];
  max_row[1] = curr_neighbor->neighbor->b_box[0]+curr_neighbor->neighbor->b_box[2];
  min_col[1] = curr_neighbor->neighbor->b_box[1];
  max_col[1] = curr_neighbor->neighbor->b_box[1]+curr_neighbor->neighbor->b_box[3];

  if (min_row[0]<min_row[1]) min_row[2]=min_row[0]; else min_row[2]=min_row[1];
  if (min_col[0]<min_col[1]) min_col[2]=min_col[0]; else min_col[2]=min_col[1];
  if (max_row[0]>max_row[1]) max_row[2]=max_row[0]; else max_row[2]=max_row[1];
  if (max_col[0]>max_col[1]) max_col[2]=max_col[0]; else max_col[2]=max_col[1];

  curr_neighbor->b_box[0] = min_row[2];
  curr_neighbor->b_box[1] = min_col[2];
  curr_neighbor->b_box[2] = max_row[2]-min_row[2];
  curr_neighbor->b_box[3] = max_col[2]-min_col[2];
}

/**********************************************************************************/
/* function reset_pixels: sets pixels from neighbor as belonging to current      */
/*                         segment and corrects its outline                       */
/**********************************************************************************/

void reset_pixels(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols)
{
  int i;
  unsigned long int neighbor_id, segment_id;
  long int neighbor_pixel[4][2]; /* first col contain pixel id, second col contains segment id */
  struct segment_pixel *aux_pixel;
  struct segment *aux_segment;

  neighbor_id = curr_neighbor->neighbor_id;
  segment_id = curr_segment->id;

  /* for each pixel of the neighbor segment to be merged */
  aux_pixel = curr_neighbor->neighbor->pixel_list;
  while (aux_pixel!=NULL)
  {
    /* changes the value of the pixel in the segment matrix (assign it to the current segment) */
    segments_ptr_vector[aux_pixel->pixel_id] = curr_segment;

    /* if it is outline pixel, check if that must be changed */
    if ((aux_pixel->borderline==true)&&(curr_segment->area>6)) /* curr_segment->area>6, 
                                  only valid for pixel neighborhood==4 */
    {
      find_neighbor_pixels(aux_pixel->pixel_id, nrows, ncols, 
      neighbor_pixel[0], neighbor_pixel[1], neighbor_pixel[2], neighbor_pixel[3]);
      /* if pixel is surrounded by pixels of the same segment or of the merged neighbor, 
       it's no longer a border pixel */
      i=0;
      while ((i<4)&&(i>-1))
      {
        if (neighbor_pixel[i][0]==-1) /* image limit */
        {
          i=-1;
        }
        else
        {
      aux_segment = segments_ptr_vector[neighbor_pixel[i][0]];
          neighbor_pixel[i][1] = aux_segment->id;

          if ((neighbor_pixel[i][1] != (int) segment_id)&&(neighbor_pixel[i][1] != (int) neighbor_id))
          {
            i=-1;
          }
          else
          {
            i++;
          }
        }
      }
      if (i!=-1) /* no longer outline pixel */
      {
        aux_pixel->borderline=false;
      }
    }
    aux_pixel = aux_pixel->next_pixel;
  }
  
  if (curr_segment->area>6) /* curr_segment->area>6, only valid for pixel neighborhood==4 */
  {
    /* for each outline pixel of the current segment */
    aux_pixel = curr_segment->pixel_list;
    while (aux_pixel!=NULL)
    {
      if (aux_pixel->borderline==true)
      {
        find_neighbor_pixels(aux_pixel->pixel_id, nrows, ncols, 
      neighbor_pixel[0], neighbor_pixel[1], neighbor_pixel[2], neighbor_pixel[3]);
        /* if pixel is surrounded by pixels of the same segment or of the merged neighbor, 
       it's no longer a border pixel */
        i=0;
        while ((i<4)&&(i>-1))
        {
          if (neighbor_pixel[i][0]==-1) /* image limit */
          {
            i=-1;
          }
          else
          {
            aux_segment = segments_ptr_vector[neighbor_pixel[i][0]];
            neighbor_pixel[i][1] = aux_segment->id;

            if ((neighbor_pixel[i][1] != (int) segment_id)&&(neighbor_pixel[i][1] != (int) neighbor_id))
            {
              i=-1;
            }
            else
            {
              i++;
            }
          }
        }
        if (i!=-1) /* no longer outline pixel */
        {
          aux_pixel->borderline=false;
        }
      }
      aux_pixel = aux_pixel->next_pixel;
    }
  }

  /* include pixel list of neighbor in the list of curr_segment */
  curr_segment->last_pixel->next_pixel = curr_neighbor->neighbor->pixel_list;
  curr_segment->last_pixel = curr_neighbor->neighbor->last_pixel;
  curr_neighbor->neighbor->pixel_list = NULL;
}

/**********************************************************************************/
/* function remove_segment: remove segment from list                              */
/**********************************************************************************/

void remove_segment(struct segment *aux_segment, 
          struct segment **first_segment, 
          struct segment **last_segment)
{
  if (aux_segment==*first_segment)
  {
    *first_segment = aux_segment->next_segment;
    (*first_segment)->previous_segment = NULL;
  }
  else
  {
    if (aux_segment==*last_segment)
    {
      *last_segment = aux_segment->previous_segment;
      (*last_segment)->next_segment = NULL;
    }
    else
    {
      aux_segment->previous_segment->next_segment = aux_segment->next_segment;
      aux_segment->next_segment->previous_segment = aux_segment->previous_segment;
    }
  }
}


/**********************************************************************************/
/* function merge_segment: merge neighbor with current_segment and removes        */
/*                         neighbor segment. Returns 0 if merged segment has been */
/*                         used before, 1 otherwise.                              */
/**********************************************************************************/

int merge_segment(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **first_segment, 
          struct segment **last_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols)
{
  int i, not_used;
  struct segment *neighbor;

  neighbor = curr_neighbor->neighbor;

  /* check if selected neighbor had already been used, if not set it as used */
  if (neighbor->used)
  {
    not_used = false;
  }
  else
  {
    not_used = true;
    neighbor->used = true;
  }
  
  /* copy data from curr_neighbor to curr_seg */
  curr_segment->area = curr_neighbor->area;
  curr_segment->perimeter = curr_neighbor->perimeter;
  for (i=0;i<4;i++)
  {
    curr_segment->b_box[i] = curr_neighbor->b_box[i];   
  }
  for (i=0;i<NUM_BANDS;i++)
  {
    curr_segment->avg_color[i] = curr_neighbor->avg_color[i];
    curr_segment->std_color[i] = curr_neighbor->std_color[i];
  curr_segment->avg_color_square[i] = curr_neighbor->avg_color_square[i];
  curr_segment->color_sum[i] = curr_neighbor->color_sum[i];
  }

  /* sets pixels from neighbor as belonging to current segment and corrects outline */
  reset_pixels(curr_neighbor, curr_segment, segments_ptr_vector, nrows, ncols);
  
  /* remove neighbor segment from list */
  remove_segment(neighbor, first_segment, last_segment);

  return(not_used);
}

/**********************************************************************************/
/* function free_segment_list: remove all segment list and frees memory         */
/**********************************************************************************/

void free_segment_list(struct segment **initial_segment, 
             struct segment ***segments_ptr_vector)
{
  struct segment *aux_segment;
  struct segment *next_segment;
  struct segment_pixel *aux_pixel;
  struct segment_pixel *next_pixel;

  /* frees segment list */
  aux_segment=*initial_segment;
  while (aux_segment!=NULL)
  {
    /* frees pixels lists */
    aux_pixel = aux_segment->pixel_list;
    while (aux_pixel!=NULL)
    {
      next_pixel = aux_pixel->next_pixel;
      free(aux_pixel);
      aux_pixel = next_pixel;
    }
    next_segment=aux_segment->next_original_segment;
    free(aux_segment);
    aux_segment=next_segment;
  }
  initial_segment=NULL;

  /* frees segment vector */
  if (segments_ptr_vector!=NULL)
  {
    free(*segments_ptr_vector);
    *segments_ptr_vector=NULL;
  }
}

/**********************************************************************************/
/* function reset_unused_segments_list: mark all segments as unused                */
/**********************************************************************************/

void reset_unused_segments_list(unsigned long int num_segments, 
                 struct segment *first_segment)
{
  unsigned long int i;
  struct segment *aux_segment;

  aux_segment = first_segment;

  /* mark all segments as unused */
  for (i=0; i<num_segments; i++)
  {
    aux_segment->used = false;
    aux_segment = aux_segment->next_segment;
  }
}

/**********************************************************************************/
/* function write_segments: write output image                                    */
/**********************************************************************************/

void write_segments(// float **input_image, 
          TePDITypes::TePDIRasterPtrType output_image,
          struct segment *first_segment, 
          int write_type,
          bool progress_enabled_)
{
  int color[NUM_BANDS];
  struct segment_pixel *aux_pixel;
  struct segment *aux_segment;
  long  ncols = output_image->params().ncols_,
      nrows = output_image->params().nlines_,
    col, row;

  /* searches for first segment with a pixel list */
  aux_segment = first_segment;
  while (aux_segment->pixel_list==NULL)
  {
    aux_segment = aux_segment->next_original_segment;
  }

  /* for each segment */
  TePDIPIManager pim("Writing segments", nrows * ncols, progress_enabled_);
  long pixel_id = 1;
  while (aux_segment!=NULL)
  {
    /* define segment color */
    switch (write_type)
    {
      case 0: /* random colors */
        color[0] = pixel_id++; //(int) floor(ranval(0, 256));
        //color[1] = (int) floor(ranval(0, 256));
        //color[2] = (int) floor(ranval(0, 256));
        break;
      case 3: /* encoded segment id */
        color[0] = (int) floor((double)aux_segment->id / 256.0);
        //color[1] = ((int) aux_segment->id) % 256;
        //color[2] = 0;
        break;
      default: /* average colors */
        color[0] = (int) floor(aux_segment->avg_color[0]);
        //color[1] = (int) floor(aux_segment->avg_color[1]);
        //color[2] = (int) floor(aux_segment->avg_color[2]);
        break;      
    }

    /* for each pixel of the segment */
    aux_pixel = aux_segment->pixel_list;
    while (aux_pixel!=NULL)
    {
   convert_pixel_id(aux_pixel->pixel_id, ncols, &row, &col);
   output_image->setElement(col, row, color[0]);
   // input_image[0][aux_pixel->pixel_id] = (unsigned char) color[0];
     //input_image[1][aux_pixel->pixel_id] = (unsigned char) color[1];
     //input_image[2][aux_pixel->pixel_id] = (unsigned char) color[2];
     if (aux_pixel->borderline)
      {
        if (write_type==2)
        {
      convert_pixel_id(aux_pixel->pixel_id, ncols, &row, &col);
      output_image->setElement(col, row, OUTLINE_COLOR_0);
          // input_image[0][aux_pixel->pixel_id] = (unsigned char) OUTLINE_COLOR_0;
          //input_image[1][aux_pixel->pixel_id] = (unsigned char) OUTLINE_COLOR_1;
          //input_image[2][aux_pixel->pixel_id] = (unsigned char) OUTLINE_COLOR_2;
        }
        if (write_type==3)
        {
          //input_image[2][aux_pixel->pixel_id] = (unsigned char) 255;
        }
      }
      aux_pixel = aux_pixel->next_pixel;
    }
    aux_segment=aux_segment->next_segment;
  pim.Increment();
  }
  pim.Toggle(false);
}

/**********************************************************************************/
/* function segmentation: performs the region growing segmentation                */
/**********************************************************************************/

float segmentation(//float **input_image, 
           struct segment **segments_ptr_vector, 
           struct segment *initial_segment, 
           struct segment *final_segment, 
           long int nrows, 
           long int ncols, 
           struct segmentation_parameters parameters,
           bool progress_enabled_)
{
  int curr_step;
  bool no_merges_prev_step = false;
  float spectral_h, spatial_h;
  float spectral_f, spatial_f;
  float min_fusion_f, fusion_f, ssp;
  long int num_segments, num_segments_unused;
  unsigned long int num_pixels;
  int dither_bits;
  unsigned long int num_dither_cells, dither_cell_count;
  struct segment *first_original_segment; /* first segment in segment list */
  struct segment *first_segment; /* first not deleted segment in segment list */
  struct segment *last_segment; /* last not deleted segment in segment list */
  struct segment *curr_segment; /* current segment */
  struct segment_neighbor *curr_neighbor; /* current neighbor */
  struct segment_neighbor *first_neighbor; /* root of neighbor segment list */
  struct segment_neighbor *best_neighbor = 0; /* lowest fusion factor to current segment */

  /* initializes variables */
  num_segments = num_pixels = nrows * ncols;
  first_segment = initial_segment;
  first_original_segment = initial_segment;
  last_segment = final_segment;
 
  /* square of scale parameter */
  ssp = parameters.sp*parameters.sp;

  /* calculate dimensions of dither matrix */
  num_dither_cells = dither_matrix_dimension(nrows, ncols, &dither_bits);

  curr_step = 0;

  /* segmentation step */
  TePDIPIManager pim("Segmenting image", MAX_STEPS, progress_enabled_);
  while ((curr_step<MAX_STEPS) && (no_merges_prev_step!=true))
  {
    /* flag that indicates if there were merges in the step */
    no_merges_prev_step = true;

    /* mark all segments as not used */
    num_segments_unused = num_segments;

    /* initialize cell count for dither search */
  dither_cell_count = 0;

  /* while there are segments not used (num_segments_unused>0)*/
    while (num_segments_unused>0)
    {
      /* select one segment and mark that segment as used */
      curr_segment = find_unused_by_dither(segments_ptr_vector, 
      dither_bits, nrows, ncols, &dither_cell_count);
    dither_cell_count++;
    
    curr_segment->used = true;
      num_segments_unused--;

      first_neighbor = curr_neighbor = NULL;

      /* find segment neighbors */
      first_neighbor = find_neighbors(curr_segment, segments_ptr_vector, nrows, ncols);
      curr_neighbor = first_neighbor;

      /* calculate heterogeinity factors for each neighbor */
      min_fusion_f =  (float) MAX_FLT;
      while (curr_neighbor != NULL)
      {
        /* calculates spectral heterogeneity */
        spectral_h = calc_color_stats(curr_neighbor, curr_segment, /* input_image, */ parameters);
    spectral_f = parameters.wcolor * spectral_h;

        if (spectral_f<ssp)
    {
      /* calculates spatial heterogeneity */
      spatial_h = calc_spatial_stats(curr_neighbor, curr_segment, 
        segments_ptr_vector, nrows, ncols, parameters);
      spatial_f = (1-parameters.wcolor) * spatial_h;

      /* calculates fusion factor and identify best neighbor */
      fusion_f = spectral_f + spatial_f;
      if (fusion_f<min_fusion_f)
      {
        min_fusion_f = fusion_f;
        best_neighbor = curr_neighbor;  
      }
    }
        curr_neighbor = curr_neighbor->next_neighbor; 
      }

      /* checks if merge of current segment with best neighbor is possible */
      if (min_fusion_f<ssp)
      {
        /* merges current segment with best neighbor and removes best neighbor */
        num_segments_unused = num_segments_unused - 
      merge_segment(best_neighbor, curr_segment, &first_segment, &last_segment, 
          segments_ptr_vector, nrows, ncols);
        num_segments--;
        no_merges_prev_step = false;
      }
      free_neighbor_list(&first_neighbor);
    }
    reset_unused_segments_list(num_segments, first_segment);
    
  curr_step++;
  pim.Increment();
    
  }
  return(1.0);
}

/**********************************************************************************/
/* function calc_perimeter_optimized: calculates the perimeter of merging         */
/*                         of curr_segment and curr_neighbor                      */
/**********************************************************************************/

void calc_perimeter_optimized(struct segment_neighbor *curr_neighbor, 
          struct segment *curr_segment, 
          struct segment **segments_ptr_vector, 
          long int nrows, 
          long int ncols)
{
  int i;
  long int perimeter_total;
  long int neighbor_pixel[4][2]; /* first col contain pixel id, second contains segment id */
  struct segment_pixel *aux_pixel;
  struct segment_pixel *id_nb;
  struct segment *aux_segment; 

	//optimization: perimeter total is the sum of perimeters less the points of colision between them
	
	//sum perimeters
	perimeter_total = curr_segment->perimeter + curr_neighbor->neighbor->perimeter;

	//choose segment with less perimeter
	if ( curr_segment->perimeter <= curr_neighbor->neighbor->perimeter )
	{
    aux_pixel = curr_segment->pixel_list;
    id_nb = curr_neighbor->neighbor->pixel_list;
	}
	else
	{
    aux_pixel = curr_neighbor->neighbor->pixel_list;
    id_nb = curr_segment->pixel_list;
	}

  // for each pixel from the less perimeter segment
  while ( aux_pixel != NULL )
  {
		// just for borderline
		if ( aux_pixel->borderline == true )
		{
			find_neighbor_pixels( aux_pixel->pixel_id, nrows, ncols, 
				                    &neighbor_pixel[ 0 ][ 0 ], &neighbor_pixel[ 1 ][ 0 ], 
                            &neighbor_pixel[ 2 ][ 0 ], &neighbor_pixel[ 3 ][ 0 ] );

			//verify the neighbor pixels
			for ( i = 0; i < 4; i++ )
			{
			  if ( neighbor_pixel[i][0] != -1 ) // if it is not image limit 
			  {
          aux_segment = segments_ptr_vector[ neighbor_pixel[ i ][ 0 ] ];
          neighbor_pixel[i][1] = aux_segment->id;

          if ( neighbor_pixel[ i ][ 1 ] == (long int)id_nb->pixel_id ) //if the pixel neighbor is from grater perimeter segment
            perimeter_total = perimeter_total - 2;
			  }
			}
		}
    aux_pixel = aux_pixel->next_pixel;
  }

  curr_neighbor->perimeter = perimeter_total;
}
