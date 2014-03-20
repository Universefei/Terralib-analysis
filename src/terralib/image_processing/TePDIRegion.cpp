#include "TePDIRegion.hpp"
#include <iostream>

using namespace std;

TePDIRegion::~TePDIRegion()
{
}

// bool TePDIRegion::Init(int n, long a, TePDIStatistic s)
bool TePDIRegion::Init(int n, long a, vector<double> m, TeMatrix c)
{
  // sets the number of bands
  TEAGN_TRUE_OR_RETURN(n > 0, "Invalid number of bands");
  nbands = n;
  // sets the area
  npix = a;
  // sets the mean vector
  mean = m;
  // sets the covariance matrix
  covariance = c;

  return true;
};

// Returns the mean value.
double TePDIRegion::Mean(int ind)
{
  TEAGN_TRUE_OR_RETURN((ind >= 0 && ind < nbands), "Invalid index in Mean Vector")
  return mean[ind];
}

// Returns the covarariance value.
double TePDIRegion::Covar(int i, int j)
{
  TEAGN_TRUE_OR_RETURN((i >= 0 && i < nbands) && (j >= 0 && j < nbands), "Invalid index in Covariance Matrix")
  return covariance(i, j);
}

vector<double> TePDIRegion::GetMean()
{
  return mean;
}
