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

#ifndef TEPDITILEINDEXER_HPP
#define TEPDITILEINDEXER_HPP

#include "../kernel/TeGeometry.h"
#include "../kernel/TeAgnostic.h"

#include <algorithm>

#include <math.h>

/**
 * @class Polygon tile indexing class for optmized geometrical relational tests.
 * @author Gilberto Ribeiro <gribeiro@dpi.inpe.br>
 * @note The related polygon instance must always be valid ( only the polygon
 * reference is stored internally ).
 */
class PDI_DLL TePDITileIndexer
{
  public :
  
    /**
     * @typedef Indexed elements node type( pair< ring index, seg index  ).
     */
    typedef vector< pair< unsigned int, unsigned int > > TeTileSegIndex;

  protected:
    
    //!< Reference polygon.
    const TePolygon& refPol_;
    
    //!< Tile resolution along "y" axis.
    double dy_;

    //!< Each tile segments index vector.
    std::vector<TeTileSegIndex*> tileIndex_;
    
    TePDITileIndexer( const TePDITileIndexer& );
    
    /** Copy overload */
    const TePDITileIndexer& operator=( const TePDITileIndexer& );
    
    /*! Gets tile index intervals in y direction for a given segment (it does 
        NOT need to be oriented).
      \param c1         First segment coordinate.
      \param c2         Second segment coordinate.
      \param firstTile  The first tile index that this segment intersects.
      \param lastTile   The last tile index that this segment intersects.
     */
    void getTileIndex(const TeCoord2D& c1, const TeCoord2D& c2, 
      unsigned int& firstTile, unsigned int& lastTile) const
    {
      TEAGN_DEBUG_CONDITION( ( dy_ > 0 ), 
        "Invalid tile indexer Y resolution" );
            
      double lowerY = ((c1.y_ < c2.y_) ? c1.y_ : c2.y_);
      double upperY = ((c1.y_ > c2.y_) ? c1.y_ : c2.y_);

      firstTile = int((lowerY - refPol_.box().y1()) / dy_);
      lastTile  = int((upperY - refPol_.box().y1()) / dy_);  
    }    
    
    //! Gets tile index for y coordinate value.
    /*!
      \param y         Value of "y" coordinate.
      \param tileIndex Index of corresponding tile.
     */
    void getTileIndex(const double& y, unsigned int& tileIndex) const
    {
      TEAGN_DEBUG_CONDITION( ( dy_ > 0 ), 
        "Invalid tile indexer Y resolution" );
        
      tileIndex = int( (y - refPol_.box().y1() ) / dy_ );
    }    
    
    /**
     * @brief Init internal variables.
     */
    void init()
    {
      dy_ = 0;
    }      
    
  public:
  
    //! Alternative Constructor.
    /*!
      \param pol The polygon to index.
      \param dy Tile size along "y" axis.
     */
    TePDITileIndexer( const TePolygon& pol, const double& dy )
      : refPol_( pol )
    {
      TEAGN_DEBUG_CONDITION( ( dy > 0 ),
        "Invalid tile index dy" );
        
      init();
      
      dy_ = dy;
      
      /* Building new index */
      
      if( refPol_.size() > 0 ) {
        TEAGN_DEBUG_CONDITION( ( dy_ > 0 ), 
          "Invalid tile indexer Y resolution" );
              
        unsigned int total_tiles_number = 1 + 
          ( (unsigned int)floor( refPol_.box().height() / dy_ ) );
              
        for(unsigned int i = 0; i < total_tiles_number; ++i)
        {
          tileIndex_.push_back( new TeTileSegIndex );
        }

        const unsigned int rings_number = refPol_.size();

        // for each polygon ring, we need to index its segments
        for(unsigned int i = 0; i < rings_number; ++i)
        {
          addRing( i );
        }
      }
    }    
    
    /**
     * @brief Clear all internal resources.
     */
    void clear()
    {
      vector<TeTileSegIndex*>::iterator it = tileIndex_.begin();
      vector<TeTileSegIndex*>::iterator it_end = tileIndex_.end();
      
      while( it != it_end ) {
        delete ( *it );
        
        ++it;
      }
      
      tileIndex_.clear();
      
      init();
    }     

    ~TePDITileIndexer()
    {
      clear();
    }
    
    //! Update the tile index with the information of the supplied ring.
    void addRing( const unsigned int& ring_index )
    {
      TEAGN_DEBUG_CONDITION( ( ring_index < refPol_.size() ),
        "Invalid ring_index" );

      unsigned int num_points = refPol_[ring_index].size();
        
      if( num_points > 0 ) {
        unsigned int numSegments = num_points - 1;
  
        for(unsigned int j = 0; j < numSegments; ++j)
        {
          // creates a pointer to the segment
          pair<unsigned int, unsigned int> segPointer(
            ring_index, j);
  
          // finds the tiles that this segments intersects
          unsigned int firstTileIndex = 0;
          unsigned int lastTileIndex = 0;
            
          getTileIndex( refPol_[ring_index][j], 
            refPol_[ring_index][j + 1], 
            firstTileIndex, lastTileIndex);
              
          TEAGN_DEBUG_CONDITION( 
            ( firstTileIndex < tileIndex_.size() ),
            "Invalid firstTileIndex" );
          TEAGN_DEBUG_CONDITION(
            ( lastTileIndex < tileIndex_.size() ),
            "Invalid lastTileIndex" );
  
          // associates the pointer segment to the tiles
          for(unsigned int k = firstTileIndex; k <= lastTileIndex; ++k)
          {
            tileIndex_[k]->push_back(segPointer);
          }
        }
      }
    }     
    
    /*! Gets tile index.
      \param y Y value.
      \param index Output tile pointer ( NULL if not found ).
    */    
    void getTile( const double& y, TeTileSegIndex** index ) const
    {
      unsigned int tidx;
      getTileIndex( y, tidx );
      
      if( tidx < tileIndex_.size() ) {
        ( *index ) = tileIndex_[ tidx ];
      } else {
        ( *index ) = 0;
      }
    }
    
    const TePolygon& getPol() const
    {
      return refPol_;
    }
    
    unsigned int getTilesNumber()
    {
      return (unsigned int)tileIndex_.size();
    }

};


#endif
