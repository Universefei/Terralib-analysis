/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright � 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeDecoderSmartMem.h
    \brief This file deals with decoding of raster structures as a mulitdimensional matrix in memory
*/
#ifndef  __TERRALIB_INTERNAL_DECODERSMARTMEM_H
#define  __TERRALIB_INTERNAL_DECODERSMARTMEM_H

#include "TeDecoder.h"
#include "TeRasterMemManager.h"
#include "TeAgnostic.h"

/** 
 * @brief Implements a decoder to a raster stored as a as a mulitdimensional 
 * matrix in memory (and disk when necessary).
 * @note This decoder accepts the following optional parameters from TeRasterParams::decoderParams_
 * - Name:"TeDecoderSmartMem_MaxMemPercentUsage" Type:unsigned char Description: The maximum amount of RAM memory to use when necessary - valid range: [0,100] - Suggested default:40 See TeRasterMemManager for reference.
 * @note No interleaving support.
 */
class TL_DLL TeDecoderSmartMem : public TeDecoder
{

public:

  //! @brief Empty constructor
  TeDecoderSmartMem ();

  //! @brief Constructor from parameters
  TeDecoderSmartMem ( const TeRasterParams& par );

  //! @brief Destructor
  virtual ~TeDecoderSmartMem ();

  //! @brief Sets the value of a specific raster element 
  /*!
    \param col  element column identifier
    \param lin  element line identifier
    \param val  element value being inserted
    \param band element band identifier
  */  
  inline bool setElement (int col,int lin, double val, int band=0)
  {
    TEAGN_DEBUG_CONDITION( ( setelement_ptrs_vec_ != 0 ),
      "invalid setElement function pointers vector" )
    TEAGN_DEBUG_CONDITION( ( col < params_.ncols_ ),
      "Invalid number of columns" )
    TEAGN_DEBUG_CONDITION( ( lin < params_.nlines_ ),
      "Invalid number of lines" )
    TEAGN_DEBUG_CONDITION( ( band < params_.nBands() ),
      "Invalid band" )
    TEAGN_DEBUG_CONDITION( 
      ( params_.status_ != TeRasterParams::TeNotReady),
      "Raster not Ready" );      
    
    ( this->*( setelement_ptrs_vec_[ band ] ) )( col, lin, val, band );
  
    return true;  
  };
  
  //! @brief Gets an specific element (col, lin, band) of a raster data
  /*!
    \param col  element column identifier
    \param lin  element line identifier
    \param val  element value being retrieved
    \param band element band identifier
  */  
  inline bool getElement (int col,int lin, double &val,int band=0)
  {
    TEAGN_DEBUG_CONDITION( ( getelement_ptrs_vec_ != 0 ),
      "invalid getElement function pointers vector" )
    TEAGN_DEBUG_CONDITION( ( col < params_.ncols_ ),
      "Invalid number of columns" )
    TEAGN_DEBUG_CONDITION( ( lin < params_.nlines_ ),
      "Invalid number of lines" )
    TEAGN_DEBUG_CONDITION( ( band < params_.nBands() ),
      "Invalid band" )    
    TEAGN_DEBUG_CONDITION( 
      ( params_.status_ != TeRasterParams::TeNotReady),
      "Raster not Ready" );      
    
    ( this->*( getelement_ptrs_vec_[ band ] ) )( col, lin, val, band );
    
    return true;  
  };
  
  /**
   * @brief Optmized method to set an element value.
   * @note Use this method only if the used ValueType matches 
   * the target band data type otherwise memory corruption may occur..
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  template< typename ValueType > 
  inline void setValue( const int& col, const int& lin, 
    const ValueType& val, const int& band )
  {
    TEAGN_DEBUG_CONDITION( ( getelement_ptrs_vec_ != 0 ),
      "invalid getElement function pointers vector" )
    TEAGN_DEBUG_CONDITION( ( col < params_.ncols_ ),
      "Invalid number of columns" )
    TEAGN_DEBUG_CONDITION( ( lin < params_.nlines_ ),
      "Invalid number of lines" )
    TEAGN_DEBUG_CONDITION( ( band < params_.nBands() ),
      "Invalid band" )    
    TEAGN_DEBUG_CONDITION( 
      ( params_.status_ != TeRasterParams::TeNotReady),
      "Raster not Ready" );  
        
    *( ( (ValueType*)man_manager_.getTilePointer( band, lin ) ) + col ) = val;  
  };  
  
  /**
   * @brief Optmized method to get an element value.
   * @note Use this method only if the used ValueType matches 
   * the target band data type otherwise memory corruption may occur.
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */    
  template< typename ValueType >    
  inline void getValue( const int& col, const int& lin, 
    ValueType& val, const int& band ) const
  {
    TEAGN_DEBUG_CONDITION( ( getelement_ptrs_vec_ != 0 ),
      "invalid getElement function pointers vector" )
    TEAGN_DEBUG_CONDITION( ( col < params_.ncols_ ),
      "Invalid number of columns" )
    TEAGN_DEBUG_CONDITION( ( lin < params_.nlines_ ),
      "Invalid number of lines" )
    TEAGN_DEBUG_CONDITION( ( band < params_.nBands() ),
      "Invalid band" )    
    TEAGN_DEBUG_CONDITION( 
      ( params_.status_ != TeRasterParams::TeNotReady),
      "Raster not Ready" );  
        
    val = ( *( ( (ValueType*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };  

  /**
   * @brief Provides direct access to each line.
   * @param lin Line.
   * @param band Band.
   * @return A pointer the the required line.
   * @note Use this method only if the target band data type is known
   * otherwise memory corruption may occur.   
   * @note Further accesses to this class instance invalidates the pointer.
   */  
  inline void* getScanLine( const int& lin, const int& band )
  {
    TEAGN_DEBUG_CONDITION( ( lin < params_.nlines_ ),
      "Invalid number of lines" )
    TEAGN_DEBUG_CONDITION( ( band < params_.nBands() ),
      "Invalid band" )    
    TEAGN_DEBUG_CONDITION( 
      ( params_.status_ != TeRasterParams::TeNotReady),
      "Raster not Ready" );  
        
    return man_manager_.getTilePointer( band, lin );
  };    

  //! @brief Initializes the internal structures of the decoder
  virtual void  init  ();
  
  //! @brief Clears its internal structures
  virtual bool  clear  ();

protected:

  /**
   * @brief Type definition for the getElement function pointer.
   *
   * @param line Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  typedef void (TeDecoderSmartMem::*GetEleFunctPtrT)( const int& col,
    const int& lin, double& val, const int& band );

  /**
   * @brief Type definition for the setElement function pointer.
   *
   * @param line Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  typedef void (TeDecoderSmartMem::*SetEleFunctPtrT)( const int& col, 
    const int& lin, const double& val, const int& band );
  
  /**
   * @brief Internal memory manager instance.
   */
  mutable TeRasterMemManager man_manager_;

  /**
   * @brief A vector of pointers to the current getElement method following the 
   * current data bype of each band (indexed by the band index).
   */
  GetEleFunctPtrT* getelement_ptrs_vec_;

  /**
   * @brief A vector of pointers to the current setElement method following the 
   * current data bype of each band (indexed by the band index).
   */
  SetEleFunctPtrT* setelement_ptrs_vec_;
  
  /**
   * @brief Set all internal variables to their initial values.
   */   
  void initVars();
  
  /**
   * @brief Update the function pointers vectors using the current
   * raster parameters.
   */   
  void updateFuncPtrVectors();  
  
  /**
   * @brief Fill data with dummy values.
   */   
  void dummyFill();    

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeBIT( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (unsigned char*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (unsigned char)val;  
  };
  
  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeUNSIGNEDCHAR( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (unsigned char*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (unsigned char)val;  
  };  
  
  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeCHAR( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (char*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (char)val;  
  };      

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeUNSIGNEDSHORT( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (unsigned short*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (unsigned short)val;  
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeSHORT( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (short*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (short)val;
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeINTEGER( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (int*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (int)val;
  }

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeUNSIGNEDLONG( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (unsigned long*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (unsigned long)val;  
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeLONG( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (long*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (long)val;  
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeFLOAT( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (float*)man_manager_.getTilePointer( band, lin ) ) + col ) = 
      (float)val;    
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void setElement_TeDOUBLE( const int& col, const int& lin, 
    const double& val, const int& band )
  {
    *( ( (double*)man_manager_.getTilePointer( band, lin ) ) + col ) = val;    
  };
  
  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeBIT( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (unsigned char*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };
    
  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeUNSIGNEDCHAR( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (unsigned char*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };
  
  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeCHAR( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (char*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeUNSIGNEDSHORT( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (unsigned short*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeSHORT( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (short*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeINTEGER( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (int*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeUNSIGNEDLONG( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (unsigned long*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeLONG( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (long*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param line Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeFLOAT( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = (double)( *( ( (float*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };

  /**
   * @brief Method overload.
   *
   * @param lin Line.
   * @param col Column.
   * @param band Band.
   * @param val Pixel value.
   */      
  inline void getElement_TeDOUBLE( const int& col, const int& lin, 
    double& val, const int& band )
  {
    val = ( *( ( (double*)man_manager_.getTilePointer( band, 
      lin ) ) + col ) );
  };
  
};

//! Implements a factory to build decoder to MEMORY raster
class TL_DLL TeDecoderSmartMemFactory : public TeDecoderFactory
{
public:

  //! Factory constructor
  TeDecoderSmartMemFactory(const string& name) : TeDecoderFactory(name) {}

  //! Builds the object
  virtual TeDecoder* build (const TeRasterParams& arg)
  {  return new TeDecoderSmartMem(arg); }
};
#endif

