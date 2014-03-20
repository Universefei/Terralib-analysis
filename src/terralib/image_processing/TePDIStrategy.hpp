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

#ifndef TEPDISTRATEGY_HPP
  #define TEPDISTRATEGY_HPP

  #include "../kernel/TeSharedPtr.h"
  #include "TePDIParameters.hpp"
  
  #include <string>

  /**
   * @brief This is the base class for digital image processing algorithms 
   * strategies.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @note All TePDIStrategy derived classes must be a friend of the respective
   * TePDIStrategyFactory derived class ( for private constructor access ).
   * @ingroup PDIStrategies
   */
  class PDI_DLL TePDIStrategy
  {
//    friend class TePDIStrategyFactory;
  
    public :
      /** @typedef TeSharedPtr< TePDIStrategy > pointer 
        Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIStrategy > pointer;
      
      /** @typedef const TeSharedPtr< TePDIStrategy > const_pointer 
        Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIStrategy > const_pointer;
      
      /**
       * @brief Default Destructor
       */
      virtual ~TePDIStrategy();
      
      /**
       * @brief Returns a default object.
       *
       * @return A default object.
       */
      static TePDIStrategy* DefaultObject( 
        const TePDIParameters& params );     
        
      /**
       * @brief Checks if the supplied parameters fits the requirements of each
       * PDI strategy.
       *
       * @note Error log messages must be generated. No exceptions generated.
       *
       * @param parameters The parameters to be checked.
       * @return true if the parameters are OK. false if not.
       */
      virtual bool CheckParameters( 
        const TePDIParameters& parameters ) const = 0;
        
      /**
       * @brief Applies the strategy.
       *
       * @note Parameters checking will be performed.
       *
       * @return true if OK. false on error.
       */
      bool Apply( const TePDIParameters& params );
      
      /**
       * @brief Enable / Disable the progress interface.
       *
       * @param enabled Flag to enable ( true ) or disable ( false ).
       */
      void ToggleProgInt( bool enabled );                          

    protected :
    
      /** @brief Progress interface enabled/disable flag */
      mutable bool progress_interface_enabled_;
      
      /**
       * @brief Default constructor
       *
       * @param factoryName Factory name.
       */
      TePDIStrategy();
      
      /**
       * @brief Runs the algorithm implementation.
       *
       * @return true if OK. false on errors.
       */
      virtual bool Implementation( const TePDIParameters& params ) = 0;
      
  };
  
#endif



