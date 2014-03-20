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

#ifndef TEPDICLUSTER_HPP
  #define TEPDICLUSTER_HPP
  
#include "TePDIRegion.hpp"
#include <iostream>
#include <vector>
#include "../kernel/TeAgnostic.h"  

using namespace std;

  /**
   * @brief This is the class for dealing with image clusters.
   * @author Thales Sehn Korting <tkorting@dpi.inpe.br>
   * @ingroup PDIAux
   *
   */
class PDI_DLL TePDICluster
{
  private:
    /**
    * @brief identifier
    */
    int id;

    /**
    * @brief number of bands
    */
    int nbands;
      
  /**
    * @brief number of bands
    */
    int nElems;

    /**
    * @brief number of pixels for each band
    */
    long area;

    /**
    * @brief Number of regions
    */
    long nreg;

    /**
    * @brief variance value vector
    */
    vector<double> variance;

  /**
    * @brief eVec ( e ) value vector (used to calculate variance)
    */
    vector<double> eVec;

  /**
    * @brief e2Vec (e x e) value vector (used to calculate variance)
    */
    vector<double> e2Vec;

  /**
    * @brief mean value vector
    */
    vector<double> mean;

    /**
    * @brief covariance matrix
    */
    TeMatrix covar;

    /**
    * @brief lu decomposition of covariance matrix
    */
    TeMatrix lu;

    /**
    * @brief ln of determinant of covariance matrix
    */
    double lndet;

    /**
    * @brief flag if the cluster was initialized
    */
    bool initialized;

  public:

    /**
    * Default constructor
    */
    TePDICluster()
    { id = 0; nbands = 0; area = 0L; lndet = 0.0; initialized = false; };

    /**
    * Alternative constructor.
    * @param nBands The bands number.
    */
    TePDICluster( int nBands );

    /**
    * Default Destructor
    */
    ~TePDICluster();

    /**
    * Assignment operator
    */
    const TePDICluster& operator=( const TePDICluster& clu);

    /**
    * Equal operator
    */
    bool operator==(TePDICluster clu);

    /**
    * Initialize cluster with its identifier and with statistics
    *
    * @param ident The id of the cluster
    * @param reg The First Region to be inserted in the Cluster
    */
    bool Init(int ident, TePDIRegion reg);
  
    /**
    * Add a region statistics to the cluster.
    *
    * @param reg The Region to be added
    */
    bool AddRegion(TePDIRegion reg);
  
    /**
    * Compute the Mahalanobis distance between the center of a region and the current cluster
    *
    * @param reg The region to be computed the distance
    */
    double Distance(TePDIRegion reg);
  
    /**
    * Compute the Mahalanobis distance between the center of a cluster and the current cluster
    *
    * @param clu The cluster to be computed the distance
    */
    double Distance(TePDICluster clu);
  
    /**
    * Set new identification to cluster
    *
    * @param newid The new ID
    */
    void SetId( int newid )
    { id = newid; }

    /**
    * Return  cluster identification
    */
    int GetId(){ return id; }

    /**
    * Get mean value vector
    *
    * @param ind The cluster ID to get the mean
    */
    double Mean(int ind);

    /**
    * Return cluster covariance matrix.
    */
    TeMatrix GetCovariance()
    { return covar; }

    /**
    * Get cluster size in pixels.
    */
    long GetArea()
    { return area; }

    /**
    * Get number o bands
    */
    int GetNban()
    { return nbands; }

    /**
    * Gets the Ln of Det
    */
    double GetLnDet()
    { return lndet; }

    /**
    * Merges two clusters
    *
    * @param clu The cluster to be merged
    */
    void Merge(TePDICluster clu);

    /**
    * Get if the cluster was initialized
    */
    bool GetInitialized()
    { return initialized; }

  /**
    * Insert a new pixel value in cluster
    */
  void add( vector<double> pixValue );

  /**
    * Calculate a distance over pixel value
    */
  double  dist( vector<double> pixValue )const;

  /**
    * Get variance value vector
    *
    * @param ind The cluster ID to get the variance
    */
  double getVariance( int ind )const;

  /**
    * Get the value of major variance
    *
    */
  double maxVar()const;

  /**
    * Get the band value of major variance
    *
    */
  int maxVarBand()const;

  /**
    * reset values used to get the variance
    *
    */
  void resetCount();

  /**
    * update the information about mean and variance
    *
    */
  void updateStatistics();

  /**
    * split the cluster
    *
    */
  TePDICluster splitLeft()const;

  /**
  * split the cluster
    *
    */
  TePDICluster splitRight()const;

  /**
    * Clear cluster
    *
    */
    void clear();

};

#endif
