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

#ifndef TEPDIREGGROWSEG_HPP
  #define TEPDIREGGROWSEG_HPP

  #include "TePDIAlgorithm.hpp"

  #include "../kernel/TeSharedPtr.h"
  
  #include <map>
  
  #define MAXCLOSECELLS   5
  #define SQR_NC          65025
  #define TAMJAN          128
  #define STATDEAD        (char)2    
  
  /** @brief Forward declaration */
  class TePDIRGSCell;
  
  
  /** @brief Forward declaration */
  class CloserCells;
  
  /**
  * @typedef map<long, TePDIRGSCell *> TePDIRGSCellList
  * Cell list type definition.
  */
  typedef map<long, TePDIRGSCell *> TePDIRGSCellList;    
  
  
  /**
  * @brief This is the basic element in the region growing process. The cell represents an area
  * in the image being segmented. It contains spectral, geometrical and contextual
  * information of this area. The behaviour of this class allows it to merge
  *
  * The behaviour of this class allows it to merge with other cells and to update its
  * information. At the initial stage in the region growing process each cell is
  * composed by one pixel of the image. It uses 4-Neighbourhood (contextual information).
  */
class PDI_DLL TePDIRGSCell
{
public:
   /**
    * @brief Default Constructor.
    */
        TePDIRGSCell ();

   /**
    * @brief Alternative Constructor.
    *
    * @param tuple: pixel tuple containing spectral information.
    * @param id: Id number of the cell.
    * @param lin: line position of tuple in the image.
    * @param col: column position of tuple in the image.
    * @param nban: Number of bands.
    */
   TePDIRGSCell( unsigned long *tuple, long id, short lin, short col, short nban );

   /**
    * @brief Default destructor.
    */
   ~TePDIRGSCell();

   /**
    * @brief Initializes an existing TePDIRGSCell
    *
    * @param tuple: pixel tuple containing spectral information.
    * @param id: Id number of the cell.
    * @param lin: line position of tuple in the image.
    * @param col: column position of tuple in the image.
    * @param nban: Number of bands.
    */
   void Init( unsigned long *tuple, long id, short lin, short col, short nban );

   /**
    * @brief Changes the Id  number of the cell
        *
    * @param idc: new id number
    *
    */
   void ResetId( long idc ) { Idnumber_ = idc; }

   /**
    * @brief Returns The status of the cell
    *
    * @return If cell is dead return 1, otherwise, 0.
    */
   char Dead()  { return (Stat_ & STATDEAD); }

   /**
    * @brief Turns cell dead
    *
        * When the cell is dead it does not represent
    * any region in the image.
    */
   void Kill() { Stat_ = STATDEAD; }

   /**
    * @brief Return the bounding box of the cell
    *
    * @param lmin: Minimum image row wich contains a pixel belonging to the cell
    * @param cmin: Minimum image column wich contains a pixel belonging to the cell
    * @param lmax: Maximum image row wich contains a pixel belonging to the cell
    * @param cmax: Maximum image column wich contains a pixel belonging to the cell
    */
   void BoundingRectangle( int& lmin, int& cmin, int& lmax, int& cmax )
        { lmin = LinMin_; cmin = ColMin_; lmax = LinMax_; cmax = ColMax_; }

   /**
    * @brief LinMax
    *
        * @return Maximum image line wich contains a pixel belonging to the cell
    */
   int LineMax () { return LinMax_;}

   /**
    * @brief Merge cell c with the "this" cell.
    *
    * @return true if OK. false on error.
    */
   bool Merge( TePDIRGSCell *c );

   /**
    * @brief Remove dead cells from the list of neighbour cells and closest cells
    */
   void AdjustNeighborhood();//{ Cc_->Adjust(); /*Neighbors_->Adjust();*/ }

   /**
    * @brief Add neighbour to neighbour cells list and closest cells list.
    *
    * @param c being added.
    * @param dist euclidian distance to "this" cell
    * @return true if OK. false on error.
    */
   bool AddNeighbor( TePDIRGSCell* c, float dist );

   /**
    * @brief Insert neighbour to neighbour cells list and closest cells list.
    *
    * @param c being added.
    * @param dist: euclidian distance to "this" cell
    * @return true if OK. false on error.
    */
   bool InsertNeighbor( TePDIRGSCell* c, float dist );

   /**
    * @brief Return the cell with the minimum euclidian distance.
    *
    * @param dist: minimum distance.
    * @return Pointer to cell which has the minimum euclidian distance.
    */
   TePDIRGSCell* ClosestNeighbor( float& dist );

   /**
    * @brief Return the euclidian distance between cell c and "this" cell.
    *
        * @param c: input cell
    * @return euclidian distance between cell c and "this" cell.
    */
   float Distance( TePDIRGSCell *c );

   /**
    * @brief Returns the Id number of the cell.
    *
    * @return Id number of the cell.
    */
   long Id() { return Idnumber_; }

   /**
    * @brief Return the number of pixels of the cell
    *
    * @return Number of pixels of the cell
    */
   long Area() { return Npix_; }

   /**
    * @brief Return the number of espectral bands used
    *
    * @return Number of espectral bands used
    */
   int GetNban() { return Nban_; }

   /**
    * @brief Prints cell state
    */
   void Print();

private:
        /** @brief Status STATDEAD = dead, otherwise, alive. */
        char            Stat_;
        
        /** @brief Id number of cell */
        long            Idnumber_;
        
        /** @brief Number of pixels it contains */
        long            Npix_;
        
        /** @brief Number of spectral bands used */
        int             Nban_;
        
        /** @brief Delta */
        float           delta_;
        
        /** @brief Bounding box of cell */
        int             LinMin_;
        
        /** @brief Bounding box of cell */
        int             LinMax_;
        
        /** @brief Bounding box of cell */
        int             ColMin_;
        
        /** @brief Bounding box of cell */
        int             ColMax_;
        
        /** @brief Array of spectral mean. */
        float           *Media_;
        
        /** @brief Array containing the previous spectral mean */
        float           *PreviousMedia_;
        
        /** @brief List of neighbour cells */
        TePDIRGSCellList        *Neighbors_;
        
        /** @brief List of closest cells */
        CloserCells     *Cc_;

};  


/**
 * @brief Simple cell stack class.
 */
class PDI_DLL CellStack : public vector< TePDIRGSCell* >
{
public:

        /** 
         * @brief Add element to top of stack.
         * @param c Cell.
         */
        void Push(TePDIRGSCell* c)
        { push_back(c); }

        /** 
         * @brief Peek at top element of stack.
         */
        TePDIRGSCell* Peek()
    {
                if (size()==0){
                        return NULL;
                } else {
                        return back();
                }
        }

        /** 
         * @brief Pop top element off stack.
         * @return Cell.
         */
        TePDIRGSCell* Pop()
        {
                TePDIRGSCell* c;
                if (size()==0)
                {
                        c = new TePDIRGSCell();
                } else {
                        c = back();
                        pop_back();
                }
                return c;
        }

        /**
         * @brief Destructor.
         */
        ~CellStack ()
        {
                TePDIRGSCell *b;
                while (size()!=0)
                {
                        b = Pop();
                        delete b;
                }
        }
        
        /**
         * @brief Clear.
         */        
        void Clear ()
        {
                TePDIRGSCell *b;
                while (size()!=0)
                {
                        b = Pop();
                        delete b;
                }
        }
};


/**
  * @brief Keep a number (given by MAXCLOSECELLS) of closest  neighbour cells of a cell.
  * Its an ordered array of closest neighbour cells of a cell. The contents of array
  * are sorted in ascending order by the euclidian distance between the cell and its
  * neighbour cell.
  *
  * The class cell hold some of the closest neighbour cells in this class. Every time
  * the cell wants to the closest neighbour cell it calls methods of this class to
  *  retrieve closest neighbour.
  */
class PDI_DLL CloserCells
{
public :
        /**
         * @brief Array of closest cells.
         */
        TePDIRGSCell    *cmin[MAXCLOSECELLS];

        /**
         * @brief Array of euclidian distances between the cell containing 
         * this object (CloserCells) and cmin[i].
         */
        float   dmin[MAXCLOSECELLS];
        
        /**
         * @brief Defalt constructor.
         */
        CloserCells()
        {
                for(short i = 0; i < MAXCLOSECELLS; i++ )
                {
                        cmin[i] = NULL;
                        dmin[i] = 1000000.;
                }
        }

        /**
         * @brief New cell insertion.
         * @param cell Cell pointer.
         * @param dist Distance.
         */
        void
        Insert( TePDIRGSCell *cell, float dist )
        {
                TePDIRGSCell    **pci,
                        **p;
                float   *di,
                        *d;

                for( pci = &cmin[0], di = &dmin[0]; pci < &cmin[MAXCLOSECELLS]; pci++, di++ )
                {
                        if( *pci == NULL )
                        {
                                *pci = cell;
                                *di  = dist;
                                return;
                        }
                        if( dist <= *di )
                        {
                                if( dist == *di && (*pci)->Id() < cell->Id() )
                                        continue;
                                if( pci < &cmin[MAXCLOSECELLS] )
                                {
                                        for( p = &cmin[MAXCLOSECELLS-1], d = &dmin[MAXCLOSECELLS-1]; p > pci; p--, d-- )
                                        {
                                                *p = *(p-1);
                                                *d = *(d-1);
                                        }
                                }
                                *pci = cell;
                                *di  = dist;
                                return;
                        }
                }
        }

        /**
         * @brief Cell update.
         * @param cell Cell pointer.
         * @param dist Distance.
         */        
        void
        Update( TePDIRGSCell *cell, float dist )
        {
                char    ok = 1;
                TePDIRGSCell    **pci,
                        **pc0,
                        **pc01,
                        **pc02;
                float *di = 0;
                float *d0 = 0;
                float *d01 = 0;

                pc0 = NULL;
                for( pci = &cmin[0], di = &dmin[0]; pci < &cmin[MAXCLOSECELLS]; pci++, di++ )
                {
                        if( *pci == NULL )
                        {
                                pc0 = pci;
                                d0  = di;
                        }
                        else if( (*pci)->Dead() )
                        {
                                *pci = NULL;
                                pc0 = pci;
                                d0  = di;
                        }
                        else if( *pci == cell )
                        {
                                *pci = NULL;
                                pc0  = pci;
                                d0   = di;
                        }

                        if( ok && dist <= *di )
                        {
                                if( dist == *di )
                                {
                                        if( *pci == NULL )
                                        {
                                                if( pci < &cmin[MAXCLOSECELLS-1] )
                                                        continue;
                                        }
                                        else if( (*pci)->Id() < cell->Id() )
                                                continue;
                                }
                                if( pc0 != NULL )
                                {
                                        for( pc01 = pc0, d01 = d0; pc01 < pci-1; pc01++, d01++ )
                                        {
                                                *pc01 = *(pc01+1);
                                                *d01  = *(d01+1);
                                        }
                                        *pc01 = cell;
                                        *d01  = dist;
                                }
                                else if( pci < &cmin[MAXCLOSECELLS-1] )
                                {
                                        for( pc01 = pci+1, d01 = di+1; pc01 < &cmin[MAXCLOSECELLS-1]; pc01++, d01++ )
                                        {
                                                if( (*pc01) == NULL ) break;
                                                if( (*pc01)->Dead() ) break;
                                                if( *pc01 == cell )   break;
                                        }
                                        for( pc02 = pc01; pc02 > pci; pc02-- )
                                        {
                                                *pc02 = *(pc02-1);
                                                *d01  = *(d01-1);
                                                d01--;
                                        }
                                        *pci = cell;
                                        *di  = dist;
                                }
                                else
                                {
                                        *pci = cell;
                                        *di  = dist;
                                }
                                ok   = 0;
                        }
                }
                return;
        }

        /**
         * @brief Undocumented.
         * @param dist Distance.
         * @return Cell pointer.
         */          
        TePDIRGSCell*
        Minimum( float& dist )
        {
                if( cmin[0] != NULL )
                {
                        if( !cmin[0]->Dead() )
                        {
                                dist = dmin[0];
                                return cmin[0];
                        }
                }
                for( short i = 1; i < MAXCLOSECELLS; i++ )
                {
                        if( cmin[i] != NULL )
                                if( !cmin[i]->Dead() )
                                {
                                        dist = dmin[i];
                                        return cmin[i];
                                }
                }

                return NULL;
        }

        /**
         * @brief Undocumented.
         */            
        void
        Adjust()
        {
                for( short i = 0; i < MAXCLOSECELLS; i++ )
                        if( cmin[i] != NULL )
                                if( cmin[i]->Dead() )
                                        cmin[i] = NULL;
        }

        /**
         * @brief Undocumented.
         */           
        void
        Reset()
        {
                for(short i = 0; i < MAXCLOSECELLS; i++ )
                {
                        cmin[i] = NULL;
                        dmin[i] = 1000000.0;
                }
        }
};


  /**
    * @brief This is the class models an image segmentation process. 
    * @author Nicolas Despres <nicolasdespres@wanadoo.fr>
    * @ingroup TePDISegmentationGroup
    *
    * @note The segmentation is performed through a region growing approach 
    * described in: S. A. Bins, L. M. G. Fonseca, G. J. Erthal e F. M. Ii, 
    * "Satellite Imagery segmentation: a region growing approach", 
    * VIII Simp�sio Brasileiro de Sensoriamento Remoto, Salvador, BA, 
    * 14-19 abril 1996.    
    *
    * @note The general required parameters :
    *
    * @param input_image (TePDITypes::TePDIRasterPtrType),
    * @param euc_treshold (double) - euclidian distance treshold value.
    * @param area_min (int) - cell pixel size min value.
    *
    * @note The following parameters are optional and will be used if present.
    *
    * @param restriction_image (TePDITypes::TePDIRasterPtrType) - Restriction (mask) image
    * ( restriction image pixels with value zero will not be inside 
    * any segments generated from the input_image, they will be ignored ). 
    * @param output_polsets ( TePDITypes::TePDIPolSetMapPtrType ) - 
    * The user supplied output polygon sets map where the generated polygons 
    * will be stored (each polygon set contains polygons related to the
    * same pixel value).   
    * @param output_image (TePDITypes::TePDIRasterPtrType),
    *       pointer to a Labelled image containing the final
    *       segmentation. Each pixel contains the cell id
    *       number to which it belongs. This image must have TeUNSIGNEDLONG 
    *       data type.
    */
  class PDI_DLL TePDIRegGrowSeg : public TePDIAlgorithm {
    public :
      
      /**
       * @brief Default Constructor.
       *
       */
      TePDIRegGrowSeg();

      /**
       * @brief Default Destructor
       */
      ~TePDIRegGrowSeg();
      
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

    protected :
    
      /** @brief Array of pointer to images (input images). */
      TePDITypes::TePDIRasterPtrType  Imagein_;
      
      /** @brief Pointer to the output image. */
      TePDITypes::TePDIRasterPtrType  Imagelab_;
      
      /** @brief Pointer to the image containing restrictions */
      TePDITypes::TePDIRasterPtrType  Imageexc_;
      
      /** @brief Pointer to the list of cells. */
      TePDIRGSCellList        *ListCell_;
      
      /** @brief Array of cells for a subimage. */
      TePDIRGSCell            **WindowCell_;
      
      /** @brief Number of cells which have been removed from ListCell due to distance criteria*/
      long            OnLimbo_;
      
      /** @brief tuple of pixel (auxiliar). */
      unsigned long   *tuple_;
      
      /** @brief Number of subimages in the image. */
      long            NWindow_;
      
      /** @brief Number of subimages along the lines of image */
      long            WindowLines_;
      
      /** @brief Number of subimages along the columns of image */
      long            WindowColumns_;
      
      /** @brief Current subimage. */
      long            CurrentWindow_;
      
      /** @brief Id number of the first cell in the subimage. */
      long            WindowOffset_;
      
      /** @brief Minimum area allowed to the final cells. */
      int             Areamin_;
      
      /** @brief Minimum euclidian distance between final cells. */
      double          Difsim_;
      
      /** @brief Number of lines in Imagelab. */
      int             Nlin_;
      
      /** @brief Number of colums in Imagelab. */
      int             Ncol_;
      
      /** @brief Number of bands in Imagein */
      int             Nban_;
      
      /** @brief auxiliar variables */
      int             ilin_;
       
      /** @brief auxiliar variables */
      int flin_;
      
      /** @brief auxiliar variables */
      int             icol_;
      
      /** @brief auxiliar variables */
      int fcol_;
      
      /** @brief auxiliar variables */
      int             sizelin_;
       
      /** @brief auxiliar variables */
      int sizecol_;    
      
      /** @brief Cell stack */
      CellStack cellStack;
    
      /**
       * @brief Reset the internal state to the initial state.
       *
       * @param params The new parameters referente at initial state.
       */
      void ResetState( const TePDIParameters& params );    
     
      /**
       * @brief Runs the current algorithm implementation.
       *
       * @return true if OK. false on error.
       */
      bool RunImplementation();
      
      /**
      * @brief Merges two cells
      *
      * @param c1 pointer to a cell to be merged
      * @param c2 pointer to a cell to be merged
      * @return pointer to cell resulting from merging..
      */
      TePDIRGSCell* MergeCells( TePDIRGSCell *c1, TePDIRGSCell *c2 );
  
      /**
      * @brief Initializes a window over a image
      *
      * @return true if OK. false on error.
      */
          bool InitWindow();
  
      /**
      * @brief Remove dead cells from list of cells.
      */
      void Adjust();
  
      /**
      * @brief Eliminates all cells with area smaller than argument area.
      * The cells with area smaller than argument area are merged with its
      * nearest neighbour cell.
      *
      * @param area size in pixels which defines de area treshold for merging.
      * @param linmax lin max of the image to consider
      * @return true if OK. false on error.
      */
      bool MergeSmallCells( long area, int linmax);
  
      /**
      * @brief Merges all pairs of neighbouring cells with euclidian distance
      * bellow a threshold.
      * The pair of cells say, c1 and c2 must satisfy the criteria:
      *              1 - c1 = closestneighbour( c2 ) or c2 = closestneighbour( c1 ).
      *              2 - dist( c1, c2) < threshold >
      *
      * @return true if OK. false on error.
      */
      bool MergeSimilarCells();
  
      /**
      * @brief Merges all pairs of neighbouring cells with euclidian distance
      * bellow a threshold.
      * The pair of cells say, c1 and c2 must satisfy the criteria:
      *              1 - c1 = closestneighbour( c2 ) or c2 = closestneighbour( c1 ).
      *              2 - dist( c1, c2) < threshold >
      *
      * @return true if OK. false on error.
      */
      bool MergeMutuallyClosestCells();
  
      /**
      * @brief Updates ImageLab by writing the cell id number for
      * every pixel belonging to the cell.
      *
      * @return true if OK. false on error.
      */
      bool Resort();      
  };
  
/** @example TePDISegmentation_test.cpp
 *    Shows how to use this class.
 */    

#endif
