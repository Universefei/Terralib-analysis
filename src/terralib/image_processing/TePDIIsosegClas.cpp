#include "TePDIIsosegClas.hpp"
#include "TePDIMathFunctions.hpp"
#include "TePDIStrategyFactory.hpp"
#include "TePDIAlgorithmFactory.hpp"
#include "TePDIUtils.hpp"

#include "../kernel/TeBox.h"
#include "../kernel/TeUtils.h"
#include "../kernel/TeRasterParams.h"
#include "../kernel/TeRaster.h"
#include "../kernel/TeAgnostic.h"

void TePDIIsosegClas::ResetState( const TePDIParameters& )
{
}

TePDIIsosegClas::TePDIIsosegClas()
{
};

TePDIIsosegClas::~TePDIIsosegClas()
{
};

bool TePDIIsosegClas::SetThreshold(double t, int Nban)
{
  double ChiTab1[10][6] =
  {
    //     75.0%           90.0%           95.0%           99.0%           99.9%           100.%
    {(double)1.32,    (double)2.71,    (double)3.84,    (double)6.64,    (double)10.83,   (double)DBL_MAX},
    {(double)2.77,    (double)4.61,    (double)5.99,    (double)9.21,    (double)13.82,   (double)DBL_MAX},
    {(double)4.11,    (double)6.25,    (double)7.82,    (double)11.35,   (double)16.27,   (double)DBL_MAX},
    {(double)5.39,    (double)7.78,    (double)9.49,    (double)13.28,   (double)18.47,   (double)DBL_MAX},
    {(double)6.63,    (double)9.24,    (double)11.07,   (double)15.09,   (double)20.52,   (double)DBL_MAX},
    {(double)7.84,    (double)10.65,   (double)12.59,   (double)16.81,   (double)22.46,   (double)DBL_MAX},
    {(double)9.04,    (double)12.02,   (double)14.07,   (double)18.48,   (double)24.32,   (double)DBL_MAX},
    {(double)10.22,   (double)13.36,   (double)15.51,   (double)20.09,   (double)26.13,   (double)DBL_MAX},
    {(double)11.39,   (double)14.68,   (double)16.92,   (double)21.67,   (double)27.88,   (double)DBL_MAX},
    {(double)12.55,   (double)15.99,   (double)18.31,   (double)23.21,   (double)29.59,   (double)DBL_MAX}
  };

  int  igl,
    it;

  if(t < 90.0)
    it = 0;
  else if(t < 95.0)
    it = 1;
  else if(t < 99.0)
    it = 2;
  else if(t < 99.9)
    it = 3;
  else if (t < 100.0)
    it = 4;
  else
    it = 5;

  igl = Nban - 1;

  threshold = ChiTab1[igl][it];

  return true;
}

bool TePDIIsosegClas::MergeClusters()
{
  double dist = 0,
      d1 = 0,
      d2 = 0,
      dmin = 0.0;
  int  ic = 0,
    ic_merge = 0,
    ic_d1 = 0,
    ic_d2 = 0;
  bool flag = false;

  // Remove the smallest clusters
  while(clusters.size() > 1)
  {
    dmin = 0.0;
    flag = false;
    for(unsigned iclu1 = 0; iclu1 < clusters.size(); iclu1++)
    {
      for(unsigned iclu2 = 0; iclu2 < clusters.size(); iclu2++)
      {
        if(clusters[iclu1] == clusters[iclu2])
          continue;

        d1 = clusters[iclu1].Distance(clusters[iclu2]) + clusters[iclu1].GetLnDet();
        d2 = clusters[iclu2].Distance(clusters[iclu1]) + clusters[iclu2].GetLnDet();
        dist = ((d1 < d2) ? d2 : d1);
        if(dist < dmin || (!flag))
        {
          ic_d1 = iclu1;
          ic_d2 = iclu2;
          flag = true;

          dmin = dist;
          ic = iclu2;
          ic_merge = iclu1;
        }
      }
    }
    d1 = clusters[ic_d1].Distance(clusters[ic_d2]);
    d2 = clusters[ic_d2].Distance(clusters[ic_d1]);
    dist = ((d1 < d2) ? d2 : d1);

    if(dist < threshold)
    {
      // merging two clusters
      clusters[ic_d1].Merge(clusters[ic_d2]);

      // setting new cluster id to the respective regions
      int  id_old_cluster = clusters[ic_d2].GetId(),
        id_new_cluster = clusters[ic_d1].GetId();
      multimap<double, TePDIRegion, greater<double> >::iterator regions_it;
      for (regions_it = regions.begin(); regions_it != regions.end(); ++regions_it)
        if (regions_it->second.GetClass() == id_old_cluster)
          regions_it->second.SetClass(id_new_cluster);

      // arrange clusters vector
      for (unsigned i = ic; i < (clusters.size() - 1); i++)
        clusters[i] = clusters[i + 1];
      clusters.pop_back();
    }
    else
      break;
  }

  return true;
}

bool TePDIIsosegClas::GenerateClusters()
{
  int total_classes = 0,
      total_classified = 0;

  // loops until all the regions are classified
  multimap<double, TePDIRegion, greater<double> >::iterator region_it,
                                                            region_it_internal;

  while (total_classified < total_regions)
  {
    // iterates over the regions, starting from the one with higher area, in descending order
    for (region_it = regions.begin(); region_it != regions.end(); ++region_it)
    {
      TePDICluster tmp_cluster;

      // if the region ins't classified, it's attributed to the class "total_classes"
      if (region_it->second.GetClass() == 0)
      {
        ++total_classes;
        int current_class = total_classes;
        region_it->second.SetClass(current_class);

        TEAGN_TRUE_OR_RETURN(tmp_cluster.Init(current_class, region_it->second), "Unable to init cluster");
        total_classified++;
        // searches for other regions with the same attributes, and put it into the same class
        double dist = 0.0;
        for (region_it_internal = regions.begin(); region_it_internal != regions.end(); ++region_it_internal)
        {
          if (region_it_internal->second.GetClass() == 0)
          {
            dist = tmp_cluster.Distance(region_it_internal->second);
            if (dist <= threshold)
            {
              region_it_internal->second.SetClass(current_class);
              TEAGN_TRUE_OR_RETURN(tmp_cluster.AddRegion(region_it_internal->second), "Unable to add region");
              total_classified++;
            }
          }
        }
        clusters.push_back(tmp_cluster);
      }
    }
  }

  return true;
}

bool TePDIIsosegClas::CheckParameters(const TePDIParameters& parameters) const
{
  // Checking input_rasters
  TePDITypes::TePDIRasterVectorType input_rasters;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_rasters", input_rasters), "Missing parameter: input_rasters");
  TEAGN_TRUE_OR_RETURN(input_rasters.size() > 0, "Invalid input rasters number");

  vector<int> bands;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("bands", bands), "Missing parameter: bands");
  TEAGN_TRUE_OR_RETURN(bands.size() == input_rasters.size(), Te2String(input_rasters.size()) + " Invalid parameter: bands number " + Te2String(bands.size()));

  for(unsigned int input_rasters_index = 0 ; input_rasters_index < input_rasters.size(); input_rasters_index++)
  {
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index].isActive(), "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " inactive");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " not ready");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().nlines_ == input_rasters[0]->params().nlines_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of lines");
    TEAGN_TRUE_OR_RETURN(input_rasters[input_rasters_index]->params().ncols_ == input_rasters[0]->params().ncols_, "Invalid parameter: input_raster " + Te2String(input_rasters_index) + " with imcompatible number of columns");
  }

  // Checking input_polygonset
  TePDITypes::TePDIPolygonSetPtrType input_polygonset;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("input_polygonset", input_polygonset), "Missing parameter: input_polygon");
  TEAGN_TRUE_OR_RETURN(input_polygonset.isActive(), "Invalid parameter: input_poligonset inactive");

  // Checking acceptance_limiar
  double  acceptance_limiar;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("acceptance_limiar", acceptance_limiar), "Missing parameter: acceptance_limiar");

  // Checking output_raster
  TePDITypes::TePDIRasterPtrType output_raster;
  TEAGN_TRUE_OR_RETURN(parameters.GetParameter("output_raster", output_raster), "Missing parameter: output_raster");
  TEAGN_TRUE_OR_RETURN(output_raster.isActive(), "Invalid parameter: output_raster inactive");
  TEAGN_TRUE_OR_RETURN(output_raster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: output_raster not ready");
  return true;
}

// bool TePDIIsosegClas::Implementation(const TePDIParameters& params)
bool TePDIIsosegClas::RunImplementation()
{
  // Setting the parameters
  TePDITypes::TePDIRasterVectorType input_rasters;
  params_.GetParameter("input_rasters", input_rasters);

  vector<int> bands;
  params_.GetParameter("bands", bands);

  W = input_rasters[0]->params().ncols_;
  H = input_rasters[0]->params().nlines_;

  TePDITypes::TePDIPolygonSetPtrType input_polygonset;
  params_.GetParameter("input_polygonset", input_polygonset);

  TePDITypes::TePDIRasterPtrType output_raster;
  params_.GetParameter("output_raster", output_raster);

  double  acceptance_limiar;
  params_.GetParameter("acceptance_limiar", acceptance_limiar);

  /* Setting the output raster */
  TeRasterParams output_raster_params = output_raster->params();

  output_raster_params.setDataType( TeDOUBLE, -1 );
  output_raster_params.nBands( 1 );
  if( input_rasters[0]->projection() != 0 ) {
    TeSharedPtr< TeProjection > proj( TeProjectionFactory::make(
      input_rasters[0]->projection()->params() ) );
    output_raster_params.projection( proj.nakedPointer() );
  }
  output_raster_params.boxResolution( input_rasters[0]->params().box().x1(),
    input_rasters[0]->params().box().y1(), input_rasters[0]->params().box().x2(),
    input_rasters[0]->params().box().y2(), input_rasters[0]->params().resx_,
    input_rasters[0]->params().resy_ );
  // output_raster_params.setPhotometric( TeRasterParams::TeRGB, -1 );

  TEAGN_TRUE_OR_RETURN( output_raster->init( output_raster_params ),
    "Output raster reset error" );

  for (unsigned pols = 0; pols < input_polygonset->size(); pols++)
  {
    TePolygon polygon(input_polygonset->operator[](pols));

    // this iterator "walks" in the image, on the region defined by a specific polygon
    TeRaster::iteratorPoly it = input_rasters[0]->begin(polygon, TeBoxPixelIn, 0);
    TeRaster::iteratorPoly it_end = input_rasters[0]->end(polygon, TeBoxPixelIn, 0);

    long area = (long)TeGeometryArea(polygon);
    // pixel vector for each band
    vector<vector<double> > pixels_per_band(bands.size());
    vector<double> sums(bands.size());
    vector<double> tmp(1);
    for (unsigned band = 0; band < bands.size(); band++)
      pixels_per_band.push_back(tmp);

    long npix = 0;
    double tmp_pixel;
    while(it != it_end)
    {
      int col = it.currentColumn(),
          lin = it.currentLine();

      if ((col >= 0 && col < W) && (lin >=0 && lin < H))
      {
        for (unsigned band = 0; band < bands.size(); band++)
        {
          input_rasters[band]->getElement(col, lin, tmp_pixel, band);
          pixels_per_band[band].push_back(tmp_pixel);
          sums[band] += tmp_pixel;
        }
        npix++;
      }

      ++it;
    }

    vector<double> tmp_mean;
    tmp_mean.reserve(bands.size());
    for (unsigned band = 0; band < bands.size(); band++)
    {
      tmp_mean.push_back(sums[band]/npix);
    }

    // sets the covarariance matrix
    TeMatrix tmp_covar;
    int nbands = bands.size();
    TEAGN_TRUE_OR_RETURN(tmp_covar.Init(nbands, nbands, 0.0), "Unable to Init tmp_covar");
    double sum;
    for(int i = 0; i < nbands; i++)
      for(int j = 0; j < nbands; j++)
      {
        sum = 0.0;
        for (int p = 0; p < npix; p++)
          sum += (pixels_per_band[i][p] - tmp_mean[i]) * (pixels_per_band[j][p] - tmp_mean[j]);
        if (npix == 1)
          tmp_covar(i,j) = 0.0;
        else
          tmp_covar(i,j) = (double)(sum / (npix - 1));
      }

    TePDIRegion tmp_region(pols);
    TEAGN_TRUE_OR_RETURN(tmp_region.Init(bands.size(), npix, tmp_mean, tmp_covar), "Unable to Init tmp_region");
    regions.insert(pair<double, TePDIRegion>(area, tmp_region));
  }
  total_regions = regions.size();

  TEAGN_TRUE_OR_RETURN(SetThreshold(acceptance_limiar, bands.size()), "Unable to SetThreshold");
  TEAGN_TRUE_OR_RETURN(GenerateClusters(), "Unable to GenerateClusters");
  TEAGN_TRUE_OR_RETURN(MergeClusters(), "Unable to MergeClusters");

// remap cluster values to 1 -> N
  std::set<unsigned int> ulabels;
  std::set<unsigned int>::iterator lit;
  std::map<unsigned int, unsigned int> colormap;

  for (int pols = 0; pols < total_regions; pols++)
  {
    TePolygon polygon(input_polygonset->operator[](pols));

    // searches for the region with Id = pols
    multimap<double, TePDIRegion, greater<double> >::iterator regions_it;

    for (regions_it = regions.begin(); regions_it != regions.end(); ++regions_it)
      if (regions_it->second.GetId() == pols)
        break;

    ulabels.insert(regions_it->second.GetClass());
  }

  unsigned int color = 1;
  for (lit = ulabels.begin(); lit != ulabels.end(); ++lit)
    colormap[*lit] = color++;

// paint output_raster with the correspondent classes
  for (int pols = 0; pols < total_regions; pols++)
  {
    TePolygon polygon(input_polygonset->operator[](pols));

    // this iterator "walks" in the image, on the region defined by a specific polygon
    TeRaster::iteratorPoly it = input_rasters[0]->begin(polygon, TeBoxPixelIn, 0);
    TeRaster::iteratorPoly it_end = input_rasters[0]->end(polygon, TeBoxPixelIn, 0);

    // searches for the region with Id = pols
    multimap<double, TePDIRegion, greater<double> >::iterator  regions_it,
                        regions_tmp = regions.begin();
    for (regions_it = regions.begin(); regions_it != regions.end(); ++regions_it)
      if (regions_it->second.GetId() == pols)
      {
        regions_tmp = regions_it;
        break;
      }

    unsigned bit_class = colormap[regions_it->second.GetClass()];

/*
    // here, a set of colors for up to 81 classes, C(3, 4) = tree bands, four levels
    vector<int> levels;
    levels.push_back(0);
    levels.push_back(50);
    levels.push_back(100);
    levels.push_back(150);
    levels.push_back(200);
    levels.push_back(255);
	int change = levels.size();
    vector<int> colors_R, colors_G, colors_B;
    for (int c = 0, i = 0, j = 0, k = 0; c < 81; c++)
    {
      colors_R.push_back(levels[i]);
      colors_G.push_back(levels[j]);
      colors_B.push_back(levels[k]);

      i++;
      if (i >= change)
      {
        i = 0;
        j++;
      }
      if (j >= change)
      {
        j = 0;
        k++;
      }
      if (k >= change)
        k = 0;
    }

	if (bit_class >= colors_R.size())
		bit_class = 0;

    double  R = colors_R[bit_class],
        G = colors_G[bit_class],
        B = colors_B[bit_class];
*/
    // paint output_raster with specific color class
    while(it != it_end)
    {
      int  i = it.currentColumn(),
           j = it.currentLine();

      output_raster->setElement(i, j, bit_class);
      ++it;
    }
  }

  return true;
}
