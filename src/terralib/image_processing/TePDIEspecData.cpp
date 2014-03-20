#include "TePDIEspecData.hpp"
#include <iostream>

using namespace std;

TePDIEspecData::TePDIEspecData()
{
};

TePDIEspecData::~TePDIEspecData()
{
};

bool TePDIEspecData::Init(int nband)
{
  int b;
  int fat = (nband*nband + nband)/2;
  
  if( nban != nband )
  {
    //sum3 = new double[nband];
    //sum4 = new double[nband];
    //median = new double[nband];
    //sum = new double[nband];
    //sumbij = new double[fat];
  }
  
  npix = 0;
  nban = nband;
  for ( b = 0; b < nban; b++)
  {
    sum.push_back(0.0);
    //sum3[b] = 0.0;
    //sum4[b] = 0.0;
    //median[b] = 0.0;
  }
  for ( b = 0; b < fat; b++)
    sumbij.push_back(0.0);
    
  return true;
  
}

void TePDIEspecData::Add(double *pixel)
{
  npix++;
  int k = 0;
  double aux;
  for (int b = 0; b < nban; b++)
  {
    sum[b] += (double) pixel[b];
    aux = (double) pixel[b] * (double) pixel[b] * (double) pixel[b];
    //sum3[b] += aux;
    //sum4[b] += aux * (double) pixel[b];
    for( int bo = 0; bo <= b; bo++, k++)
      sumbij[k] = sumbij[k] + ((double) pixel[b] * (double) pixel[bo]);
  }
}

float TePDIEspecData::Covar(int i, int j)const
{
  float cov = 0;
  if(i < nban &&  j < nban && npix > 0)
  {
    int k;
    if(i >= j)
      k = (i * (i + 1)) / 2 + j;
    else 
      k = (j * (j + 1)) / 2 + i;
    cov = (float) (sumbij[k] / (double) npix - sum[i] / (double) npix * sum[j] / (double) npix);
  }
  else cov = 0;

  return cov;
}

TeMatrix TePDIEspecData::Covariance()const
{
  TeMatrix cov;
  cov.Init(nban, nban, 0.0);

  for(int i = 0; i < nban; i++)
    for(int j = 0; j < nban; j++)
      cov(i,j) = (double) Covar(i,j);

  return cov;
}

float TePDIEspecData::Mean(int i)const
{
  if(i < nban && i >= 0 && npix > 0)
    return (float) (sum[i] / npix);

  return 0.0;
}
