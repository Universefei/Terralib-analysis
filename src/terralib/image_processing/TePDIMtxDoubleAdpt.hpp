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

#ifndef TEPDIMTXDOUBLEADPT_HPP
  #define TEPDIMTXDOUBLEADPT_HPP

  #include "TePDIMatrix.hpp"

  /*!
    \class TePDIMtxDoubleAdptInt
    \brief Matrix double adapter interface.
    \author Emiliano F. Castejon <castejon@dpi.inpe.br>
    \ingroup PDIAux
   */
  class TePDIMtxDoubleAdptInt
  {
    public :
      
      virtual ~TePDIMtxDoubleAdptInt() {};
    
      /*! \brief Read values from the internal representation converting
       * to double.
       * \param lin Line.
       * \param col Column.
       * \param value The readed value. 
       */
      virtual void getValue( const unsigned int& lin, 
        const unsigned int col, double& value ) const = 0;
      
      /*! \brief Write double values converting to the internal 
       * representation.
       * \param lin Line.
       * \param col Column.
       * \param value The value to write. 
       */
      virtual void setValue( const unsigned int& lin, 
        const unsigned int col, const double& value ) = 0;
        
      /*! \brief Reallocate memory following the internal memory 
       * policy.
       * \param lines The new lines number.
       * \param columns The new columns number.
       * \return true if OK. 
       */
      virtual bool reset( unsigned int lines, 
         unsigned int columns ) = 0;
      
      /*! Return the number of matrix lines.
       * \return The number of Matrix lines.
       */
      virtual unsigned int getNLines() const = 0;
      
      /*! Return the number of matrix columns.
       * \return The number of Matrix columns.
       */
      virtual unsigned int getNCols() const = 0;      
      
    protected :
      
      TePDIMtxDoubleAdptInt() {};
      TePDIMtxDoubleAdptInt( const TePDIMtxDoubleAdptInt& ) {};
  };
  
  /*!
    \class TePDIMtxDoubleAdpt
    \brief Matrix double adapter.
    \author Emiliano F. Castejon <castejon@dpi.inpe.br>
    \ingroup PDIAux
   */
  template< class T >
  class TePDIMtxDoubleAdpt : public TePDIMatrix< T >,
    public TePDIMtxDoubleAdptInt
  {
    public :
      
      /*! Forward declaration for memory policy */
      typedef typename TePDIMatrix< T >::MemoryPolicy MemoryPolicy;
      
      TePDIMtxDoubleAdpt() {};
      
      ~TePDIMtxDoubleAdpt() {};

      // Overloaded.
      inline void getValue( const unsigned int& lin, 
        const unsigned int col, double& value ) const
      {
        TEAGN_DEBUG_CONDITION( col < TePDIMatrix< T >::GetColumns(),
          "Invalid column" );
        
        value = (double)TePDIMatrix< T >::getScanLine( lin )[ col ];
      };
      
      // Overloaded.
      inline void setValue( const unsigned int& lin, 
        const unsigned int col, const double& value )
      {
        TEAGN_DEBUG_CONDITION( col < TePDIMatrix< T >::GetColumns(),
          "Invalid column" );
        
        TePDIMatrix< T >::getScanLine( lin )[ col ] = (T)value;  
      };
      
      // Overloaded.
      inline bool reset( unsigned int lines, unsigned int columns )
      {
        return TePDIMatrix< T >::Reset( lines, columns );
      };
      
      // Overloaded
      inline unsigned int getNLines() const
      {
        return TePDIMatrix< T >::GetLines();
      };
      
      // Overloaded
      inline unsigned int getNCols() const
      {
        return TePDIMatrix< T >::GetColumns();          
      };
  };

#endif
