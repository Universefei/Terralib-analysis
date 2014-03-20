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

#ifndef TEPDIARITHMETIC_HPP
  #define TEPDIARITHMETIC_HPP

  #include "TePDIAlgorithm.hpp"
  
  #include <vector>
  #include <string>

  /**
   * @class TePDIArithmetic
   * @brief  Performs arithmetic operation over raster data.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   * @ingroup TePDIGeneralAlgoGroup
   *
   * @note The general required parameters are:
   *
   * @param input_rasters ( TePDITypes::TePDIRasterVectorType ) - A vector of all used 
   * rasters pointers ( NOTE: All rasters must have the same number of lines/columns ).   
   * @param arithmetic_string ( std::string ) - Arithmetic string 
   * to execute. It must be formed by a combination of the following elements, separated by spaces:
   * * Operators: +, -, *, /
   * * Real Numbers (negative numbers must follow the form "-1.0")
   * * Raster bands: R0:1, R0:1, R1:0, .... (R0:1 is a reference to the first raster - with index 0 - from input_rasters, second band - with index 1).
   * @note Examples of arithmetic strings: "( R0:0 + 1.0 ) / R1:0", "R0:0 / -2.0"
   * @param output_raster (TePDITypes::TePDIRasterPtrType) - The output
   * raster pointer.
   *
   * @note The following parameters are optional:
   * 
   * @param normalize_output (int) - If present(with any value) an output
   * values normalization will be performed to fit the allowed values range 
   * dictated by the output_raster data type (default:no normalization performed).
   *
   * @note The input raster with index 0 will be used as reference for
   * projection, number of lines, number of columns and pixel resolution.
   *
   * @example TePDIArithmetic_test.cpp Shows how to use this class.
   */
  class PDI_DLL TePDIArithmetic : public TePDIAlgorithm 
  {
    public :

      TePDIArithmetic();

      ~TePDIArithmetic();

      //overloaded
      bool CheckParameters( const TePDIParameters& parameters ) const;

    protected :
    
      /**
      * @class ExecStackElement
      * @brief Execution stack element.
      */    
      class PDI_DLL ExecStackElement
      {
        public :
        
          /** true if this is a raster pointer element. */
          bool isRaster_;
          
          /** true if this is a real number element. */
          bool isRealNumber_;
          
          /** Real number value. */
          double realNumberValue_;
          
          /** Raster band index. */
          unsigned int rasterBand_;
          
          /** Raster pointer. */
          TePDITypes::TePDIRasterPtrType rasterPtr_;
        
          ExecStackElement()
          : isRaster_( false ), isRealNumber_( false ), realNumberValue_( 0 ),
            rasterBand_( 0 )
          {};
          
          ~ExecStackElement() {};
        
      };
      
      /** @typedef Execution stack type definition. */
      typedef std::stack< ExecStackElement > ExecStackT;
      
      /** Internal progress manager instance. */
      mutable TePDIPIManager progress_;
      
      //overloaded
      bool RunImplementation();

      //overloaded
      void ResetState( const TePDIParameters& params );
      
      /**
      * @brief Execute the automata parsing the given input string.
      * @param aStr The input arithmetic expression string.
      * @param inRasters Input rasters pointers.
      * @param outRaster Output raster pointer (pre-initiated).
      * @param generateOutput If true, the output raster data will be generated,
      * if false only the automata execution will be performed..
      */      
      bool executeString( const std::string& aStr, 
        const TePDITypes::TePDIRasterVectorType& inRasters,
        TePDITypes::TePDIRasterPtrType& outRaster,
        bool generateOutput ) const;
        
      /**
      * @brief Convert the input tokens vector from the infix notation
      * to postfix notation.
      * @param input The input tokens vector.
      * @param output The output tokens vector.
      */         
      void inFix2PostFix( const std::vector< std::string >& input, 
        std::vector< std::string >& output ) const;
        
      /**
      * @brief Print tokens to stout.
      * @param input The input tokens vector.
      */        
      void printTokens( const std::vector< std::string >& input ) const;        
        
      /**
      * @brief Returns true if the given token is an operator.
      * @param inputToken Input token.
      * @return Returns true if the given token is an operator.
      */         
      bool isOperator( const std::string& inputToken ) const;
      
      /**
      * @brief Returns true if the given token is a binary operator.
      * @param inputToken Input token.
      * @return Returns true if the given token is a binary operator.
      */               
      bool isBinaryOperator( const std::string& inputToken ) const;
      
      /**
      * @brief Returns true if the given token is a unary operator.
      * @param inputToken Input token.
      * @return Returns true if the given token is a unary operator.
      */            
      bool isUnaryOperator( const std::string& inputToken ) const;
      
      /**
      * @brief Returns true if operator1 has greater of equal precedence
      * over operator2.
      * @param operator1 Operator1 input token.
      * @param operator2 Operator2 input token.
      * @return Returns true if operator1 has greater of equal precedence
      * over operator2.
      */      
      bool op1HasGreaterOrEqualPrecedence( const std::string& operator1, 
        const std::string& operator2 ) const;
        
      /**
      * @brief Returns true if the given token is a raster data token.
      * @param token Input token.
      * @param rasterIdx The output converted raster index value.
      * @param bandIdx The output converted band index value.
      * @return Returns true if the given token is a raster data token.
      */           
      bool isRasterBandToken( const std::string& token, unsigned int& rasterIdx,
        unsigned int& bandIdx ) const;
      
      /**
      * @brief Execute the given binary operator using the current given
      * execution stack.
      * @param token Operator token.
      * @param execStack Execution stack.
      * @param generateOutput if true the execution will generate valid
      * output data, if false only dummy stack elements will be generated.
      * @return true if OK, false on errors..
      */       
      bool execBinaryOperator( const std::string& token, ExecStackT& 
        execStack, bool generateOutput ) const;
      
      /**
      * @brief Execute the given unary operator using the current given
      * execution stack.
      * @param token Operator token.
      * @param execStack Execution stack.
      * @param generateOutput if true the execution will generate valid
      * output data, if false only dummy stack elements will be generated.
      * @return true if OK, false on errors..
      */         
      bool execUnaryOperator( const std::string& token, ExecStackT& 
        execStack, bool generateOutput ) const;
        
      /**
      * @brief Returns true if the given token is a real number.
      * @param token Input token.
      * @param realValue The output converted value.
      * @return Returns true if the given token is a real number.
      */         
      bool isRealNumberToken( const std::string& token, double& realValue ) const;
      
      /**
      * @brief Allocate a new RAM memory raster.
      * @param nLines Number of raster lines.
      * @param nCols Number of raster columns.
      * @param rasterPtr The output raster pointer.
      * @return Returns true if OK, false on errors.
      */        
      bool allocResultRaster( unsigned int nLines, unsigned int nCols,
        TePDITypes::TePDIRasterPtrType& rasterPtr ) const;
        
      /**
      * @brief Split the input string into a vector of token strings
      * @param inputStr The input string.
      * @param outTokens The generated output tokens vector.
      */         
      void getTokensStrs( const std::string& inputStr,
        std::vector< std::string >& outTokens ) const;
  };
  
#endif
