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

/**
 *  @brief This file contains the class of structures and definitions
 *   to deal with parameters used by all PDI classes.
 */

#ifndef TEPDIPARAMETERS_HPP
  #define TEPDIPARAMETERS_HPP

  #include "TePDITypes.hpp"
  
  #include "../kernel/TeMultiContainer.h"
  
  #include <string>

/**
 * @class TePDIParameters
 * @brief Store parameters used by all PDI classes.
 * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
 * @ingroup PDIAux
 * @note Do not use this class with polymorphic types !!
 */
  class PDI_DLL TePDIParameters : protected 
    TeMultiContainer< std::string >
  {
    public :
    
      /** @typedef TeSharedPtr< TePDIParameters > pointer 
          Type definition for a instance pointer */
      typedef TeSharedPtr< TePDIParameters > pointer;
      
      /** @typedef const TeSharedPtr< TePDIParameters > const_pointer 
          Type definition for a const instance pointer */
      typedef const TeSharedPtr< TePDIParameters > const_pointer;

      /**
       * @brief Default Constructor
       */
      TePDIParameters();
      
      /**
       * @brief Alternative Constructor
       * @param external External reference.
       */
      TePDIParameters( const TePDIParameters& external );      

      /**
       * @brief  Default Destructor
       */
      ~TePDIParameters();
      
      /**
       * @brief Set the a name descriptor for the current parameters.
       *
       * @param dec_name A a name descriptor for the current parameters. 
       */                                                      
      void SetdecName( const std::string& dec_name );                            
                           
      /**
       * @brief Returns the a name descriptor for the current parameters.
       *
       * @return A a name descriptor for the current parameters. 
       */                                                      
      std::string decName() const;                           
      
      /**
       * @brief Operator == overload.
       *
       * @param ext_instance External instance reference.
       * @return true if this instance have the same internal
       * parameters that the external instance.
       */
      bool operator==( const TePDIParameters& ext_instance ) const;
      
      /**
       * @brief Operator != overload.
       *
       * @param ext_instance External instance reference.
       * @return true if this instance don't have the same internal
       * parameters that the external instance.
       */
      bool operator!=( const TePDIParameters& ext_instance ) const;
      
      /**
       * @brief operator= overload.
       *
       * @param ext_instance External instance reference.
       * @return The external instance reference.
       */
      const TePDIParameters& operator=( const TePDIParameters& ext_instance );
      
      /**
       * @brief Clear all parameter contents.
       *
       */
      void Clear();
                         
      /**
       * @brief Store a parameter object copy.
       *
       * @param pname Parameter name.
       * @param data Parameter object instance.
       */                         
      template< typename DataT >
      void SetParameter( const std::string& pname,
                         const DataT& data )
      {
        TeMultiContainer< std::string >::store( pname, data );
      };
      
      /**
       * @brief Retrive a copy of a stored object.
       *
       * @param pname Parameter name.
       * @param data Output parameter data.
       * @return true if OK, false if the parameter was no found or error.
       */      
      template< typename DataT >
      bool GetParameter( const std::string& pname,
                         DataT& data ) const
      {
        return TeMultiContainer< std::string >::retrive( pname, data );
      };
      
      /** 
       * @brief Checks if a parameter was set.
       * 
       * @param pname Parameter name.
       * @note The parameter data type must be specified.
       * @return true if OK, false if the parameter was not set.
       */
      template< typename DataT >
      bool CheckParameter( const std::string& pname ) const
      {
        return TeMultiContainer< std::string >::isStored< DataT >( pname );
      };
      
      /** 
       * @brief Remove a stored parameter.
       * 
       * @param pname Parameter name.
       */
      void RemoveParameter( const std::string& pname )
      {
        TeMultiContainer< std::string >::remove( pname );
      };      

};

/** @example TePDIParameters_test.cpp
 *    Shows how to use this class.
 */  

#endif //TEPDIPARAMETERS_HPP
