#include "TePDIRegGrowSeg.hpp"

#include "TePDIRaster2Vector.hpp"
#include "TePDIUtils.hpp"


TePDIRegGrowSeg::TePDIRegGrowSeg()
{
        ListCell_      = NULL;
        WindowCell_    = NULL;
        tuple_         = NULL;
        OnLimbo_       = 0;
        CurrentWindow_ = 0;
        WindowOffset_  = 1;
        WindowLines_   = 0;
        WindowColumns_ = 0;
        Nlin_          = 0;
        Ncol_          = 0;
        Nban_          = 0;
        Areamin_       = 0;
        Difsim_       = (float)0;
}


TePDIRegGrowSeg::~TePDIRegGrowSeg()
{
        Imagein_.reset();
        Imagelab_.reset();
        Imageexc_.reset();

        if( WindowCell_ != NULL ) {delete []WindowCell_; WindowCell_=NULL;}
        if( tuple_      != NULL ) {delete []tuple_; tuple_=NULL;}
        NWindow_       = 0;
        WindowLines_   = 0;
        WindowColumns_ = 0;
        CurrentWindow_ = 0;
        WindowOffset_  = 1;
        Nlin_       = 0;
        Ncol_       = 0;
        Nban_       = 0;
        Areamin_    = 0;
        Difsim_     = (float)0;
        if( ListCell_  != NULL )
        {
                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end(); it++)
                        delete (*it).second;
                ListCell_->clear();
                delete ListCell_;
                ListCell_ = NULL;
        }
}


void TePDIRegGrowSeg::ResetState( const TePDIParameters& params )
{
  if( params != params_ ) {
    Imagein_.reset();
    Imagelab_.reset();
    Imageexc_.reset();
    
    if( WindowCell_ != NULL ) {delete []WindowCell_; WindowCell_=NULL;}
        
    if( tuple_ != NULL ) {delete []tuple_; tuple_=NULL;}
            
    if( ListCell_  != NULL )
    {
            TePDIRGSCellList::iterator it;
            for (it=ListCell_->begin(); it!=ListCell_->end(); it++)
                    delete (*it).second;
            ListCell_->clear();
            delete ListCell_;
            ListCell_ = NULL;
    }    
  }
}


bool TePDIRegGrowSeg::CheckParameters( const TePDIParameters& parameters ) const
{
  /* Checking for general required parameters */

  //parameter input_image
  TePDITypes::TePDIRasterPtrType inRaster;
  if( ! parameters.GetParameter( "input_image", inRaster ) ) {

    TEAGN_LOGERR( "Missing parameter: input_image" );
    return false;
  }
  if( ! inRaster.isActive() ) {

    TEAGN_LOGERR( "Invalid parameter: input_image inactive" );
    return false;
  }
  if( inRaster->params().status_ == TeRasterParams::TeNotReady ) {

    TEAGN_LOGERR( "Invalid parameter: input_image not ready" );
    return false;
  }
  TEAGN_TRUE_OR_RETURN( ( ! inRaster->params().useDummy_ ),
    "input_image must not contain dummy values" );

  //parameter output_image
  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "output_image" ) ) {
    
    TePDITypes::TePDIRasterPtrType outRaster;
    if( ! parameters.GetParameter( "output_image", outRaster ) ) {
  
      TEAGN_LOGERR( "Missing parameter: output_image" );
      return false;
    }
    if( ! outRaster.isActive() ) {
  
      TEAGN_LOGERR( "Invalid parameter: output_image inactive" );
      return false;
    }
    if( outRaster->params().status_ != TeRasterParams::TeReadyToWrite ) {
  
      TEAGN_LOGERR( "Invalid parameter: output_image not ready" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( 
      ( outRaster->params().dataType_[0] == TeUNSIGNEDLONG ),
      "Invalid parameter: output_image pixel type must be TeUNSIGNEDLONG" );
  }
  
  //parameter restriction_image
  TePDITypes::TePDIRasterPtrType restrictionRaster;
  if( parameters.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
    "restriction_image" ) ) {

    if( ! parameters.GetParameter( "restriction_image", restrictionRaster ) ) {

      TEAGN_LOGERR( "Missing parameter: restriction_image" );
      return false;
    }
    if( ! restrictionRaster.isActive() ) {

      TEAGN_LOGERR( "Invalid parameter: restriction_image inactive" );
      return false;
    }
    if( restrictionRaster->params().status_ == TeRasterParams::TeNotReady ) {

      TEAGN_LOGERR( "Invalid parameter: restriction_image not ready" );
      return false;
    }
    TEAGN_TRUE_OR_RETURN( ( ! restrictionRaster->params().useDummy_ ),
      "input_image must not contain dummy values" );    
  }

  if( ! parameters.CheckParameter< double >( "euc_treshold" ) ) {

      TEAGN_LOGERR( "Missing parameter: euc_treshold" );
      return false;
  }

  if( ! parameters.CheckParameter< int >( "area_min" ) ) {

        TEAGN_LOGERR( "Missing parameter: area_min" );
    return false;
  }
  
  if( parameters.CheckParameter< TePDITypes::TePDIPolSetMapPtrType >( 
    "output_polsets" ) ) {
            
    TePDITypes::TePDIPolSetMapPtrType output_polsets;
    parameters.GetParameter( "output_polsets", output_polsets );
            
    TEAGN_TRUE_OR_RETURN( output_polsets.isActive(),
      "Invalid parameter output_polsets : Inactive polygon set pointer" );   
  }   

  return true;
}


bool TePDIRegGrowSeg::RunImplementation()
{
        params_.GetParameter( "input_image", Imagein_ ) ;
        params_.GetParameter( "area_min", Areamin_ ) ;
        params_.GetParameter( "euc_treshold", Difsim_ ) ;

        if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
          "restriction_image" ) ) {

          params_.GetParameter( "restriction_image", Imageexc_ );
        }

        Nban_ = (int)Imagein_->params().nBands();

        if( params_.CheckParameter< TePDITypes::TePDIRasterPtrType >( 
          "output_image" ) ) {

          params_.GetParameter( "output_image", Imagelab_ );
            
          TeRasterParams Imagelab_params = Imagelab_->params();
  
          Imagelab_params.nBands( 1 );
          if( Imagein_->projection() != 0 ) {
            TeSharedPtr< TeProjection > proj( TeProjectionFactory::make( 
              Imagein_->projection()->params() ) );          
            Imagelab_params.projection( proj.nakedPointer() );
          }
          Imagelab_params.boxResolution( Imagein_->params().box().x1(), 
            Imagein_->params().box().y1(), Imagein_->params().box().x2(), 
            Imagein_->params().box().y2(), Imagein_->params().resx_, 
            Imagein_->params().resy_ );  

          TEAGN_TRUE_OR_RETURN( Imagelab_->init( Imagelab_params ),
            "Output raster reset error" );            
        } else {
          TeRasterParams Imagelab_params = Imagein_->params();
          Imagelab_params.nBands( 1 );
          Imagelab_params.setDataType( TeUNSIGNEDLONG, -1 );
          
          TEAGN_TRUE_OR_RETURN( 
            TePDIUtils::TeAllocRAMRaster( Imagelab_, Imagelab_params,
            TePDIUtils::TePDIUtilsAutoMemPol ),
            "Internal raster allocation error");
        }

        Nlin_ = Imagelab_->params().nlines_;
        Ncol_ = Imagelab_->params().ncols_;

        // Initialize colection of regions
        ListCell_ = new TePDIRGSCellList;
        TEAGN_TRUE_OR_RETURN( ( ListCell_ != NULL ),
          "Unable to create the cells list" )

        // Initialize the array of regions of window

        WindowCell_ = new TePDIRGSCell*[TAMJAN*TAMJAN];
        TEAGN_TRUE_OR_RETURN( ( WindowCell_ != NULL ),
          "Unable to create TePDIRGSCell instance" )
                

        // Set initial values

        tuple_ = new unsigned long[Nban_] ;


        WindowLines_ = ( Nlin_ % TAMJAN ) ? ( Nlin_ / TAMJAN + 1 ) : ( Nlin_ / TAMJAN );
        WindowColumns_ = ( Ncol_ % TAMJAN ) ? ( Ncol_ / TAMJAN + 1 ) : ( Ncol_ / TAMJAN );
        NWindow_    = WindowLines_ * WindowColumns_;

        // Start Region Growing
        short canceled = 0;
        WindowOffset_ = 1L;


        int step = 0;
        TePDIPIManager progress( "Region Growing...", NWindow_,
          progress_enabled_ );

        for( CurrentWindow_ = 0; CurrentWindow_ < NWindow_; CurrentWindow_++ )
        {
                TEAGN_TRUE_OR_RETURN( InitWindow(),
                  "Unable to init window" )
                  
                Adjust();
                
                TEAGN_TRUE_OR_RETURN( MergeMutuallyClosestCells(),
                  "Unable to merge cells" )
                                        
                TEAGN_TRUE_OR_RETURN( MergeSmallCells( 5 , Nlin_),
                  "Unable to merge small cells" )
                                        
                TEAGN_FALSE_OR_RETURN( progress.Update( step ),
                  "Canceled by the user" );

                ++step;
        }


        if (!canceled)
        {
                if( MergeSmallCells( Areamin_ , Nlin_) == 0 )
                                canceled=1;
                if( Resort() == 0 )
                                canceled=1;
        }
        
        if (!canceled) {
          if( params_.CheckParameter< TePDITypes::TePDIPolSetMapPtrType >( 
            "output_polsets" ) ) {
            
            TePDIRaster2Vector vectorizer;
            
            TePDIParameters vectorizer_params = params_;
            vectorizer_params.SetParameter( "rotulated_image", Imagelab_ );
            vectorizer_params.SetParameter( "channel", (unsigned int)0 );
            
            TEAGN_TRUE_OR_RETURN( vectorizer.Reset( vectorizer_params ),
              "Unable to set vectorize parameters" );
            TEAGN_TRUE_OR_RETURN( vectorizer.Apply(),
              "Unable to create output polygon set" );
          }          
        }

        return !canceled;
}


bool
TePDIRegGrowSeg :: InitWindow()
{
        int             lin, col;
        long    ind;
        unsigned long   idcell,
                        idwindow,
                        cellid;
        float   dist;
        TePDIRGSCell *c = 0;
        TePDIRGSCell *cell = 0;
        TePDIRGSCell *cwindow = 0;

        ilin_ = (int)( ( CurrentWindow_ / WindowColumns_ ) * TAMJAN );
        icol_ = (int)( ( CurrentWindow_ % WindowColumns_ ) * TAMJAN );
        flin_ = ilin_ + TAMJAN - 1;
        if( flin_ >= Nlin_ ) flin_ = Nlin_ - 1;
        fcol_ = icol_ + TAMJAN - 1;
        if( fcol_ >= Ncol_ ) fcol_ = Ncol_ - 1;
        sizelin_   = flin_ - ilin_ + 1;
        sizecol_   = fcol_ - icol_ + 1;
        int     proLimbo = 6*TAMJAN;


//CTX Drop cells that are farther than proLimbo from current window base line
        if (icol_ < TAMJAN && ilin_ > proLimbo)
        {
                short   flag=0;
                MergeSmallCells( Areamin_ , ilin_ - proLimbo);

                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end() ; ++it) {
                        c=(*it).second;

                        if( c == NULL ) continue;
                        if ((ilin_ - c->LineMax()) > proLimbo)
                        {
                                flag = 1;
                                c->Kill();
                                OnLimbo_++;
                        }
                }
                if (flag)
                        Adjust();
        }

        // Constructs the array of regions belonging to this window
        idcell = (unsigned long)WindowOffset_;
        double val;

        for( lin = ilin_; lin <= flin_; lin++ ) {
                for( col = icol_; col <= fcol_; col++ ) {

                        if( Imageexc_.isActive() ){
                                (*Imageexc_).getElement(col, lin ,val,0);
                                if( val == 0 ){
                                        (*Imagelab_).setElement(col,lin,0L);
                                        WindowCell_[idcell-WindowOffset_] = NULL;
                                        idcell++;
                                        continue;
                                }
                        }

                        if (!((*Imagelab_).setElement(col,lin,(double) idcell,0))){
                                                         return false;
                        }

                        double d;
                        (*Imagelab_).getElement(col, lin,d,0);
                        for( int ban = 0; ban < Nban_; ban++ ) {
                                (*Imagein_).getElement(col,lin ,val,ban);
                                tuple_[ban] = (unsigned long)val;
                        }

//FAMI1095 (it's really not necessary to free memory)
//if(WindowCell_[idcell-WindowOffset_]) {delete WindowCell_[idcell-WindowOffset_]; WindowCell_[idcell-WindowOffset_]=NULL;}
                        //WindowCell_[idcell-WindowOffset_] = new TePDIRGSCell( tuple_, idcell, lin, col, Nban_ );
                        WindowCell_[idcell-WindowOffset_] = cellStack.Pop();

                        WindowCell_[idcell-WindowOffset_]->Init( tuple_, idcell, lin, col, Nban_ );
                        if( WindowCell_[idcell-WindowOffset_] == NULL )
                        {
                                for( unsigned int i = 0; i < idcell-WindowOffset_; i++ )
                                        delete WindowCell_[i];
                        }
                        idcell++;
                }
        }


        // Take the neighborhood of regions lying inside the window
        for( lin = ilin_; lin <= flin_; lin++ )
        {
                for( col = icol_; col <= fcol_; col++ )
                {
                        ind = (lin - ilin_) * sizecol_ + ( col - icol_ );
                        c = WindowCell_[ind];
                        if( c == NULL )
                                continue;
                        if( col < fcol_ && WindowCell_[ind+1] != NULL )
                        {
                                dist = WindowCell_[ind]->Distance( WindowCell_[ind+1] );
                                WindowCell_[ind]->InsertNeighbor(WindowCell_[ind+1], dist );
                                WindowCell_[ind+1]->InsertNeighbor(WindowCell_[ind], dist );
                        }
                        if( lin < flin_ && WindowCell_[ind+sizecol_] != NULL )
                        {
                                dist = WindowCell_[ind]->Distance( WindowCell_[ind+sizecol_] );
                                WindowCell_[ind]->InsertNeighbor(WindowCell_[ind+sizecol_], dist );
                                WindowCell_[ind+sizecol_]->InsertNeighbor(WindowCell_[ind], dist );
                        }
                }
        }

        // Atualiza Cells de fronteira a esquerda
        double tmp;
        if( icol_ > 0 )
        {
                cellid = 0L;
                for( lin = ilin_; lin <= flin_; lin++ )
                {
                        (*Imagelab_).getElement(icol_-1,lin,tmp,0);
                        idcell = (unsigned long)tmp;
                        (*Imagelab_).getElement(icol_,lin,tmp,0);
                        idwindow = (unsigned long)tmp;
                        if( idcell == 0L || idwindow == 0L )
                                continue;
                        if( cellid != idcell ){
                                TePDIRGSCellList::iterator it;
                                if ( (it = ListCell_->find( idcell ) ) == ListCell_->end() ) {
                                    //Fatal error : we didn t found the id in the list
                                                                        return false;
                                }
                                cell = (*it).second;
                                cellid = cell->Id();
                        }
                        cwindow = WindowCell_[ (lin-ilin_) * sizecol_ ];
                        dist = cell->Distance( cwindow );
                        cell->AddNeighbor( cwindow, dist );
                        cwindow->AddNeighbor( cell, dist );
                }
        }

        // Atualiza Cells de fronteira acima
        if( ilin_ > 0 )
        {
                cellid = 0L;
                for( col = icol_; col <= fcol_; col++ )
                {
                        (*Imagelab_).getElement(col,ilin_-1,tmp,0);
                        idcell = (unsigned long)tmp;
                        (*Imagelab_).getElement(col,ilin_,tmp,0);
                        idwindow = (unsigned long)tmp;
                        if( idcell == 0L || idwindow == 0L )
                                continue;
                        if( cellid != idcell )
                        {
                                TePDIRGSCellList::iterator it;
                                if ( (it = ListCell_->find( idcell ) ) == ListCell_->end() ) {
                                                                        return false;
                                }
                                cell = (*it).second;
                                cellid = cell->Id();
                        }
                        cwindow = WindowCell_[ col - icol_ ];
                        dist = cell->Distance( cwindow );
                        cell->AddNeighbor( cwindow, dist );
                        cwindow->AddNeighbor( cell, dist );
                }
        }

        // Add Regions in WindowCell_ in the list ListCell_;
        for( ind = 0; ind < sizelin_ * sizecol_; ind++ ){
                if( WindowCell_[ ind ] != NULL )
                        ListCell_->insert(make_pair(WindowCell_[ ind ]->Id(), WindowCell_[ ind ]));
        }                                               //stCell[WindowCell_[ ind ]->Id()]=WindowCell_[ ind ];


        WindowOffset_ += ((long)sizelin_ * sizecol_ );
        return true;

}

bool
TePDIRegGrowSeg::MergeMutuallyClosestCells() {
        float   dist;
        float   diff;
        char    status;
        TePDIRGSCell    *c,
                *cgo,
                *cback;

        for( int step = 1; step <= (int)Difsim_ ; step++ )
        {
           diff = (float)step * step;
           do {
                status  = 0;
                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end() ; ++it) {
                        c=(*it).second;

                        while( !c->Dead() )
                        {
                                if( ( cgo = c->ClosestNeighbor(dist) ) == NULL ) break;

                                cback = cgo->ClosestNeighbor(dist);
                                if( c == cback ) {
                                        if( dist <= diff  ) {
                                                c = MergeCells( c, cgo );
                                                status = 1;
                                        }else
                                                break;
                                }else
                                        break;
                        }
                }
//              Adjust();
           }while( status );
        }
        Adjust();

        return true;
}

bool
TePDIRegGrowSeg :: MergeSimilarCells()
{
        float   d1;
        float   diff;
        char    status;
        TePDIRGSCell    *c,
                *cgo;

        for( int step = 1; step <= (int)Difsim_ ; step++ )
        {
           diff = (float)step * step;
           do {
                status  = 0;

                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end() ; ++it) {
                        c=(*it).second;

                        while( !c->Dead() )
                        {
                                if( ( cgo = c->ClosestNeighbor(d1) ) == NULL ) break;
                                if( d1 <= diff )
                                {
                                        c = MergeCells( c, cgo );
                                        status = 1;
                                }
                                else    break;
                        }
                }
                Adjust();
           }while( status );
        }
        return true;
}

bool
TePDIRegGrowSeg :: MergeSmallCells( long area ,int linmax)
{
        char    status;
        TePDIRGSCell    *c,
                *cgo,
                *cback;
        float   dist;
        float   diff;
        int step;

        if( area > Areamin_ )area = Areamin_;

// Elimina regioes pequenas atraves do merging com a regiao vizinha mais proxima
        for( step = 1; step <= 6; step++ )
        {
           diff = (float)(Difsim_ + step * 2);
           diff = diff * diff;
           do {         status  = 0;

                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end() ; ++it) {
                        c=(*it).second;

                        if( c->Dead() || c->Area() > area || c->LineMax() > linmax)
                                continue;
                        if( ( cgo = c->ClosestNeighbor(dist) ) == NULL )
                                break;
                        cback = cgo->ClosestNeighbor(dist);
                        if( c != cback ) continue;
                        if( dist <= diff || step > 5 )
                        {
                                c = MergeCells( c, cgo );
                                status = 1;
                        }
                }
//CTX           Adjust();
           }while( status );
        }
        Adjust();

        for( step = 1; step <= 6; step++ )
        {
           diff = (float)(Difsim_ + step * 2);
           diff = diff * diff;
           do {         status  = 0;
                TePDIRGSCellList::iterator it;
                for (it=ListCell_->begin(); it!=ListCell_->end() ; ++it) {
                        c=(*it).second;

                        if( c->Dead() || c->Area() > area || c->LineMax() > linmax)
                                continue;
                        if( ( cgo = c->ClosestNeighbor(dist) ) == NULL ) break;
                        if( dist <= diff || step > 5 )
                        {
                                c = MergeCells( c, cgo );
                                status = 1;
                        }
                }
//              Adjust();
           }while( status );
        }
        Adjust();

        return true;
}

void
TePDIRegGrowSeg :: Adjust()
{
        TePDIRGSCell    *c;
        TePDIRGSCellList::iterator itt;
                TePDIRGSCellList::iterator it=ListCell_->begin();
        while (it!=ListCell_->end()) {
            itt=it;
                        c=(*itt).second;
                        it++;
            if ( c == NULL ) {
                ListCell_->erase(itt);
            } else if( c->Dead() ) {
                ListCell_->erase(itt);
                cellStack.Push(c);
            } else {
                c->AdjustNeighborhood();
            }
        }
}

TePDIRGSCell*
TePDIRegGrowSeg :: MergeCells( TePDIRGSCell *c1, TePDIRGSCell *c2 )
{
        double val;
        int     lmin, lmax, cmin, cmax;
        TePDIRGSCell    *cret = NULL;

        if( c1 == NULL || c2 == NULL ) return NULL;
        if( c1 == c2 ) return NULL;

        if( c1->Area() >= c2->Area() )
        {
                if( c1->Merge( c2 ) == 0 ) return NULL;
                c2->BoundingRectangle( lmin, cmin, lmax, cmax );
                for( int lin = lmin; lin <= lmax; lin++ ) {
                        for( unsigned int col = cmin; col <= (unsigned int)cmax; col++ ) {
                                (*Imagelab_).getElement(col,lin,val,0);
                                if( (unsigned long)val == (unsigned long)c2->Id() ) (*Imagelab_).setElement(col,lin,(unsigned long)c1->Id(), 0);
                        }
                }
                cret = c1;
        }
        else
        {
                if( c2->Merge( c1 ) == 0 ) return NULL;
                c1->BoundingRectangle( lmin, cmin, lmax, cmax );
                for( int lin = lmin; lin <= lmax; lin++ ) {
                        for( int col = cmin; col <= cmax; col++ ) {
                                (*Imagelab_).getElement(col,lin,val,0);
                                if( (unsigned long)val == (unsigned long)c1->Id() )
                                        (*Imagelab_).setElement(col,lin,(unsigned long)c2->Id(),0);
                        }
                }
                cret = c2;
        }

        return cret;
}


bool
TePDIRegGrowSeg :: Resort()
{
// Evaluate total number of cells

        //long size = OnLimbo_ + ListCell_->size();

// Allocate array for resorting image
        map<long, long> m_map;
        long    newid,
                        oldid = 0L,
                        index=1,
                        nindex = 0L;
        double val;

        for( int lin = 0; lin < Nlin_; lin++ )
        {
                for( int col = 0; col < Ncol_ ; col++ )
                {

                        (*Imagelab_).getElement(col,lin,val,0);
                        newid=(long)val;
                        if( newid == 0L ) // 0L means excluded area for region growing....
                                continue;
                        if( newid != oldid || oldid == 0L )
                        {

                                if ( m_map.find(newid) == m_map.end() )
                                {
                                        nindex++;
                                        index = nindex;
                                        m_map[newid] = index;
                                }
                                else index = m_map[newid];              // AND & LEO 10/00
                                oldid = newid;
                        }
                        (*Imagelab_).setElement(col,lin,index,0);
                }
        }


// Free all memory
        ListCell_->clear();
        delete ListCell_;
        ListCell_ = NULL;
        cellStack.Clear();

        m_map.clear();
        return true;
}


//--------------------------------TePDIRGSCell ---------------------------------


TePDIRGSCell :: TePDIRGSCell( unsigned long* tuple, long id, short lin, short col, short Nban )
{
        Stat_          = 0;
        Idnumber_      = id;
        Npix_          = 1;
        Nban_          = Nban;
        LinMax_        = lin;
        LinMin_        = lin;
        ColMax_        = col;
        ColMin_        = col;
        Neighbors_     = new TePDIRGSCellList;
        Media_         = new float[Nban];
        PreviousMedia_ = new float[Nban];
        Cc_            = new CloserCells;
        for( int ban = 0; ban < Nban_; ban++ )
                Media_[ban] = PreviousMedia_[ban] = (float) tuple[ban];
}


void TePDIRGSCell :: Init( unsigned long* tuple, long id, short lin, short col, short Nban )
{
        Stat_          = 0;
        Idnumber_      = id;
        Npix_          = 1;
        Nban_          = Nban;
        LinMax_        = lin;
        LinMin_        = lin;
        ColMax_        = col;
        ColMin_        = col;
        if (Neighbors_==NULL)
        {
                Neighbors_     = new TePDIRGSCellList;
                Media_         = new float[Nban_];
                PreviousMedia_ = new float[Nban_];
                Cc_            = new CloserCells;

                /*if ( PreviousMedia_ == NULL || Media_ == NULL || Neighbors == NULL || Cc == NULL )
                        SGError.Handler(ALLOCATION,FATAL,"TePDIRGSCell constructor");*/
        }
        else
        {
                Cc_->Reset();
                Neighbors_->clear();
        }
        for( int ban = 0; ban < Nban_; ban++ )
                Media_[ban] = PreviousMedia_[ban] = (float) tuple[ban];
}


TePDIRGSCell :: TePDIRGSCell()
{
        Stat_          = 0;
        Idnumber_      = 0;
        Npix_          = 1;
        Nban_          = 0;
        LinMax_        = 0;
        LinMin_        = 0;
        ColMax_        = 0;
        ColMin_        = 0;
        Neighbors_     = NULL;
        Media_         = NULL;
        PreviousMedia_ = NULL;
        Cc_            = NULL;
}

TePDIRGSCell :: ~TePDIRGSCell()
{
        if(Media_) {delete []Media_; Media_=NULL;}
        if(Neighbors_) {delete Neighbors_; Neighbors_=NULL;}
        if(PreviousMedia_) {delete []PreviousMedia_; PreviousMedia_=NULL;}
        if(Cc_) {delete Cc_; Cc_=NULL;}
}

void
TePDIRGSCell::AdjustNeighborhood()
{
        Cc_->Adjust();
        TePDIRGSCellList::iterator itt;
                TePDIRGSCellList::iterator it=Neighbors_->begin();
        while (it!=Neighbors_->end()){
            itt=it;
                        it++;
                        if (( (*itt).second==NULL) || ( (*itt).second->Dead() ) ){
                  Neighbors_->erase(itt);
            }
        }
}


bool
TePDIRGSCell :: AddNeighbor( TePDIRGSCell *c, float dist )
{
        if( c == NULL ) return false;
        if( c->Dead() ) return false;
        Cc_->Update( c, dist );

//      Neighbors[c->id()]=c;
        Neighbors_->insert(make_pair(c->Id(),c));
        AdjustNeighborhood();
        return true;
}


bool
TePDIRGSCell :: InsertNeighbor( TePDIRGSCell* c, float dist )
{
        if( c == NULL ) return false;
        if( c->Dead() ) return false;
        Cc_->Insert( c, dist );

//  Neighbors[c->id()]=c;
        Neighbors_->insert(make_pair(c->Id(),c));
        AdjustNeighborhood();
        return true;
}


bool
TePDIRGSCell :: Merge( TePDIRGSCell *c )
{
        TePDIRGSCell    *cviz;
        float   dist;
        long    area;
        int ban;

        if( c == NULL ) return false;

        if( LinMax_ < c->LinMax_ ) LinMax_ = c->LinMax_;
        if( LinMin_ > c->LinMin_ ) LinMin_ = c->LinMin_;
        if( ColMax_ < c->ColMax_ ) ColMax_ = c->ColMax_;
        if( ColMin_ > c->ColMin_ ) ColMin_ = c->ColMin_;

        area  = Npix_ + c->Npix_;
        delta_ = (float)0;
        for( ban = 0; ban < Nban_; ban++ )
        {
                Media_[ban] = (Media_[ban] * Npix_ + c->Media_[ban] * c->Npix_) / area;
                dist = Media_[ban] - PreviousMedia_[ban];
                delta_ += dist * dist;
        }
        Npix_ = area;

        c->Kill();

        if( delta_ > (float)1 )
        {
                Cc_->Reset();
                for( ban = 0; ban < Nban_; ban++ )
                        PreviousMedia_[ban] = Media_[ban];

                TePDIRGSCellList::iterator it;
                for (it=Neighbors_->begin(); it!=Neighbors_->end() ; ++it) {
                        cviz=(*it).second;
                        if( cviz == NULL ) return false;
                        if( cviz->Dead() ) continue;
                        dist = Distance( cviz );
                        Cc_->Insert( cviz, dist );
                        cviz->Cc_->Update( this, dist );
                }
        }

        TePDIRGSCellList::iterator it;
        for (it=c->Neighbors_->begin(); it!=c->Neighbors_->end() ; ++it) {
                cviz=(*it).second;

                if( cviz == NULL ) return false;
                if( cviz != this && !cviz->Dead())
                {
                        dist = Distance( cviz );
                        if( cviz->AddNeighbor( this, dist ) == 0 )
                                return false;
                        if( AddNeighbor( cviz, dist ) == 0 )
                                return false;
                }
        }

        return true;
}


float
TePDIRGSCell :: Distance( TePDIRGSCell *c )
{
        float   diff;
        float   dist;

        if( c == NULL ) return (float)100000;

        dist = 0;
        for (short ban = 0; ban < Nban_; ban++ )
        {
                diff  = PreviousMedia_[ban] - c->PreviousMedia_[ban];
                dist += (diff * diff);
        }

        return dist;
}

TePDIRGSCell*
TePDIRGSCell :: ClosestNeighbor( float& dist )
{
        TePDIRGSCell    *c;

        AdjustNeighborhood();
        if( Neighbors_->size() <= 0L ) return NULL;
        if( ( c = Cc_->Minimum( dist ) ) != NULL ) return c;

        Cc_->Reset();

        TePDIRGSCellList::iterator it;
        for (it=Neighbors_->begin(); it!=Neighbors_->end() ; ++it) {
                c=(*it).second;

                if( !c->Dead() )
                {
                        dist = Distance( c );
                        Cc_->Insert( c, dist );
                }
        }
        c = Cc_->Minimum( dist );

        return c;
}

void
TePDIRGSCell :: Print()
{
        TePDIRGSCell    *c;
        long i;

        printf( "\n%ld - %ld %f %i ", Idnumber_, Npix_, delta_, (int)Neighbors_->size() );
        for( short ban = 0; ban < Nban_; ban++ )
                printf( "%f ", Media_[ban] );
        printf("\n Mais proximos -> ");
        for( i = 0; i < MAXCLOSECELLS; i++ )
        {
                if( Cc_->cmin[i] != NULL )
                        printf("( %ld %f %d ) ", Cc_->cmin[i]->Id(), Cc_->dmin[i], Cc_->cmin[i]->Stat_ );
                else
                        printf("( NULL ) " );
        }
        printf("\n Neighbors -> ");
        TePDIRGSCellList::iterator it;
        for (it=Neighbors_->begin(); it!=Neighbors_->end() ; ++it) {
                c=(*it).second;
                if( c != NULL )
                        printf( "(%ld %f %d) ", c->Idnumber_, Distance( c ), c->Stat_ );
        }
        printf( "\n");
}

