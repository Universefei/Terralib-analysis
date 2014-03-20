/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/
/*!
  \file TeCoverageDecoderCacheLRU.h
  
  \par This file defines a memory caching method for accessing data from
       generic Coverages.
*/
#ifndef  __TERRALIB_INTERNAL_COVERAGEDECODERCACHELRU_H
#define  __TERRALIB_INTERNAL_COVERAGEDECODERCACHELRU_H

#include "TeCoverageDecoder.h"
#include "TeCoverageDecoderDatabase.h"

/*!
  \def DEFAULT_CACHE_CAPACITY
  \brief The default cache capacity (in number of blocks).
*/
#define DEFAULT_CACHE_CAPACITY 20

/*!
  \struct TeBoxIntersectionPredicate
  \brief A predicate used to check intersection with a box.
*/
template <class T>
struct TeBoxIntersectionPredicate
{
    TeBox box; //!< Box to be checked for intersection with blocks
    std::set<int>& dontSelect; // IDs of blocks that must not be included

    //! Constructor from box
    TeBoxIntersectionPredicate(const TeBox& box, std::set<int>& dontSelect):
            box(box), dontSelect(dontSelect){}

    //! Return whether the block parameter intersects the box attribute
    /*!
      \param block coverage block to be checked for intersection
      \return whether the block parameter intersects the box attribute
    */
    bool operator()(const TeCoverageBlock<T>& block) const {
        if (dontSelect.find(block.id) == dontSelect.end()
                && TeIntersects(box, block.box))
        {
            dontSelect.insert(block.id);
            return true;
        }
        return false;
    }
};

/*!
  \class TeCoverageDecoderCacheLRU
  \brief Class to decode a generic Coverage using LRU caching method.

  \par This class assumes that the coverage data will be provided as
       coverage blocks by a support decoder, and uses a memory caching method
       to access this this support decoder less often.
  \par The caching policy used is LRU (which stands for Least Recently
       Used), meaning that the most recently used blocks are kept available,
       whereas the least recently used blocks are discarded.
  \par The cache capacity (in number of blocks) may be specified in the
       constructor, otherwise a default capacity is used.
*/
template <class T>
class TeCoverageDecoderCacheLRU : public TeCoverageDecoder<T>
{
public:

    //! Constructor from parameters and capacity
    TeCoverageDecoderCacheLRU(TeCoverageParams& params, const unsigned int capacity = 0) :
        TeCoverageDecoder<T>(params),
        capacity_(capacity),
        decoder_(NULL)
    {
    }

    //! Destructor
    virtual ~TeCoverageDecoderCacheLRU()
    {
        clear();
    }

    //! Initialize internal structures
    /*!
      Instantiate internal structures, must be called before any
      attempt to access the coverage data.
    */
    virtual void init()
    {
        if (capacity_ == 0)
        {
            capacity_ = DEFAULT_CACHE_CAPACITY;
        }

        if (!decoder_)
        {
            // Initialize the support decoder according to coverage parameters.
            if (params_.getPersistenceType() == TePERSISTENCE_DATABASE_CACHELRU)
            {
                decoder_ = new TeCoverageDecoderDatabase<T>(params_);
                decoder_->init();
            }
        }
    }

    //! Clear internal structures
    /*!
      Clear internal structures, must be called before disposing of the
      decoder.
    */
    virtual void clear()
    {
        if (decoder_)
        {
            delete(decoder_); // Destroy the support decoder. 
            decoder_ = NULL;
        }
    }

    //! Select generic coverage blocks from the coverage data
    /*!
      \par Select generic coverage blocks from the coverage data.
      \par The polygon parameter defines a selection area and the relation
           parameter specifies the kind of relation (e.g. intersection,
           crossing, overlapping) that holds between the selection area and
           at least one geometry in each block to be selected.
      \param poly an instance of TePolygon representing a selection area
      \param relation a spatial relation
      \param dontSelect list IDs of blocks that must not be retrieved
      \sa TeCoverageBlock TeSpatialRelation
    */
    virtual void selectBlocks(const TePolygon& poly, const TeSpatialRelation relation, std::vector<TeCoverageBlock<T> >& selected, std::set<int>& dontSelect)
    {
        selected.clear();

        // Move to the beginning the blocks on cache intersecting box
        std::list<TeCoverageBlock<T> >::iterator bound;
        bound = stable_partition(cache_.begin(), cache_.end(), TeBoxIntersectionPredicate<T>(poly.box(), dontSelect));

        // Include blocks on the beginning of the cache in the return
        selected.insert(selected.begin(), cache_.begin(), bound);

        // Find other blocks on the support decoder
        std::vector<TeCoverageBlock<T> >& blocksFromDecoder = std::vector<TeCoverageBlock<T> >();
        decoder_->selectBlocks(poly, relation, blocksFromDecoder, dontSelect);

        // For each block selected from the support decoder
        for (std::vector<TeCoverageBlock<T> >::iterator it = blocksFromDecoder.begin(); it != blocksFromDecoder.end(); it++)
        {
            if (cache_.size() >= capacity_) // Check if cache is full
            {
                cache_.pop_back(); // Remove the least recently used block
            }
            cache_.push_front(*it); // Put new block on the head of the list

            selected.push_back(*it); // And add block to the result
        }
    }

protected:

    TeCoverageDecoder<T>* decoder_; //!< Support coverage decoder

    std::list<TeCoverageBlock<T> > cache_; //!< List of cached blocks

    unsigned int capacity_; //!< Cache capacity

};

#endif // __TERRALIB_INTERNAL_COVERAGEDECODERCACHELRU_H