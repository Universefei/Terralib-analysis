#include "TePDICluster.hpp"

#include <iostream>

using namespace std;

TePDICluster::TePDICluster(int nBands) : nbands(nBands), nElems(0)
{
  mean.clear();
  variance.clear();
  eVec.clear();
  e2Vec.clear();

  for(int i = 0; i < nbands; ++i)
  {
    mean.push_back(0);
    variance.push_back(0);
    eVec.push_back(0);
    e2Vec.push_back(0);
  }
}

TePDICluster::~TePDICluster()
{
}

// Assignment operator
const TePDICluster& TePDICluster::operator=(const TePDICluster& clu)
{
  id = clu.id;
  nbands = clu.nbands;
  area = clu.area;
  nreg = clu.nreg;
  mean.reserve(nbands);
  covar = clu.covar;
  lu = clu.lu;
  lndet = clu.lndet;
  initialized = clu.initialized;
  nElems = clu.nElems;
  mean = clu.mean;
  variance = clu.variance;
  eVec = clu.eVec;
  e2Vec = clu.e2Vec;

  return (*this);
}

// equal operator
bool TePDICluster::operator==(TePDICluster clu)
{
  bool r = true;

  r = r && (id == clu.id);
  r = r && (nbands == clu.nbands);
  r = r && (area == clu.area);
  r = r && (nreg == clu.nreg);
  r = r && (covar == clu.covar);
  r = r && (lu == clu.lu);
  r = r && (lndet == clu.lndet);
  r = r && (initialized == clu.initialized);

  for(int i = 0; i < nbands; i++)
    r = r && (mean[i] == clu.mean[i]);

  return r;
}

// Initialize cluster with its identifier and with statistics
bool TePDICluster::Init(int ident, TePDIRegion reg)
{
  if(ident <= 0)
    return false;

  id = ident;
  nbands = reg.GetNban();
  area = (long) reg.GetArea();
  nreg = 1;
  mean.reserve(nbands);
  mean = reg.GetMean();
  initialized = true;

  TEAGN_TRUE_OR_RETURN(covar.Init(nbands, nbands, 0.0), "Problem initializing covar Matrix");
  covar = reg.GetCovar();
  if(!covar.Initialized())
    return false;
  lndet = covar.Determinant();
  lndet = ((lndet > 0.0) ? log(lndet) : 0.0);
  if (!covar.MatTransf(lu))
    lu.Init(nbands, 1.0);

  return true;
}

// Add a region statistics to the cluster.
bool TePDICluster::AddRegion(TePDIRegion reg)
{
  if(id <= 0)
    return false;
  if(nbands != reg.GetNban())
    return false;

  for(int i1 = 0; i1 < nbands; i1++)
    mean[i1] = (mean[i1] * area + reg.Mean(i1) * reg.GetArea()) / (area + reg.GetArea());
  area += reg.GetArea();
  nreg++;

  return true;
}

// Compute the Mahalanobis distance between the center of a region and the current cluster
double TePDICluster::Distance(TePDIRegion reg)
{
  if (initialized == false)
    return 100000.0;
  if(nbands != reg.GetNban())
    return 100000.0;


  // Spring Method
  double  dist = 0.0,
      soma = 0.0;

  for(int ban = 0; ban < nbands; ban++)
  {
    soma = 0.0;
    for(int ban1 = 0; ban1 <= ban; ban1++)
      soma += (mean[ban1] - reg.Mean(ban1)) * lu(ban, ban1);
    dist += soma * soma;
  }

/*
  // Hot method
  TeMatrix  term_1,
        term_2,
        inverse,
        result,
        tmp;
  double  dist = 0.0;

  term_1.Init(1, nbands, 0.0);
  term_2.Init(nbands, 1, 0.0);
  inverse.Init(nbands, nbands, 0.0);
  result.Init(1, 1, 0.0);
  tmp.Init(nbands, 1, 0.0);

  covar.Inverse(inverse);
  for (int b = 0; b < nbands; b++)
    term_1(0, b) = mean[b] - reg.Mean(b);
  term_1.Transpose(term_2);
  tmp = inverse * term_2;
  result = term_1 * tmp;
  dist = sqrt(result(0, 0));
*/
  return dist;
}

// Compute the Mahalanobis distance between the center of a cluster and the current cluster
double TePDICluster::Distance(TePDICluster clu)
{
  if((initialized == false) || (clu.initialized == false))
    return 100000.0;
  if(nbands != clu.GetNban())
    return 100000.0;

  // Spring method
  double  dist = 0.0,
      sum = 0.0;

  for(int ban = 0; ban < nbands; ban++)
  {
    sum = 0.0;
    for(int ban1 = 0; ban1 <= ban; ban1++)
      sum += (mean[ban1] - clu.mean[ban1]) * lu(ban, ban1);
    dist += sum * sum;
  }

/*
  // Hot Method
  TeMatrix  term_1,
        term_2,
        inverse,
        result,
        tmp;
  double  dist = 0.0;

  term_1.Init(1, nbands, 0.0);
  term_2.Init(nbands, 1, 0.0);
  inverse.Init(nbands, nbands, 0.0);
  result.Init(1, 1, 0.0);
  tmp.Init(nbands, 1, 0.0);

  covar.Inverse(inverse);
  for (int b = 0; b < nbands; b++)
    term_1(0, b) = mean[b] - clu.Mean(b);
  term_1.Transpose(term_2);
  tmp = inverse * term_2;
  result = term_1 * tmp;
  dist = sqrt(result(0, 0));
*/
  return dist;
}

// Returns the mean value.
double TePDICluster::Mean(int ind)
{
  if (ind < nbands && ind >= 0)
    return mean[ind];
  return 0.0;
}

void TePDICluster::Merge(TePDICluster clu)
{
  for( int ban = 0; ban < nbands; ban++ )
    mean[ban] = (mean[ban] * area + clu.mean[ban] * clu.area) / (area + clu.area);
  if(area < clu.area)
  {
    lu = clu.lu;
    covar = clu.covar;
    lndet = clu.lndet;
  }
  nreg += clu.nreg;
}

void TePDICluster::add( vector<double> pixValue )
{
  for(int i = 0 ; i < nbands; i++ )
  {
    eVec[i]  += pixValue[i];
    e2Vec[i] += pixValue[i]*pixValue[i];
  }

  ++nElems;
}

double  TePDICluster::dist( vector<double> pixValue )const
{
  double dist = 0;
  double diff = 0;

  for(int i = 0 ; i < nbands; i++ )
  {
    diff = pixValue[i] - mean[i];
    dist += diff * diff;
  }

  return dist;
}

double TePDICluster::getVariance( int ind )const
{
  return variance[ind];
}

double TePDICluster::maxVar()const
{
  if( nElems <= 1 )
    return 0;

  double maxvar = 0;

  for(int i = 0 ; i < nbands; i++ )
  {
    if( variance[i] > maxvar )
    {
      maxvar = variance[i];
    }
  }

  return maxvar;
}

int TePDICluster::maxVarBand()const
{
  if( nElems <= 1 )
    return 0;

  double maxvar = 0;
  int ind = -1;
  for(int i = 0 ; i < nbands; i++ )
  {
    if( variance[i] > maxvar )
    {
      maxvar = variance[i];
      ind = i;
    }
  }

  return ind;
}

void TePDICluster::resetCount()
{
  nElems = 0;
  for(int i = 0 ; i < nbands; i++ )
  {
    eVec[i] = 0;
    e2Vec[i] = 0;
  }
}

void TePDICluster::updateStatistics()
{
  for(int i = 0 ; i < nbands; i++ )
  {
    double e2 = e2Vec[i] /(double)nElems;
    double e  = eVec[i] /(double)nElems;
    variance[i]  = nElems > 0 ? e2 - e * e : 0;
    mean[i] = nElems > 0 ? e : 0;
  }
}

TePDICluster TePDICluster::splitLeft()const
{
  int band = maxVarBand();
  if( band < 0 )
    return TePDICluster(0);

  TePDICluster result(*this);
  result.mean[band] = mean[band] - sqrt(variance[band]);
  result.resetCount();
  return result;
}

TePDICluster TePDICluster::splitRight()const
{
  int band = maxVarBand();
  if( band < 0 )
    return TePDICluster(0);

  TePDICluster result(*this);
  result.mean[band] = mean[band] + sqrt(variance[band]);
  result.resetCount();
  return result;
}

void TePDICluster::clear()
{
  nbands = 0;
  nElems = 0;

  mean.clear();
  variance.clear();
  eVec.clear();
  e2Vec.clear();
}
