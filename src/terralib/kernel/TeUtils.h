/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright  2001-2004 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeUtils.h
     \brief This file contains some general purpose utilitary  functions
*/
#ifndef  __TERRALIB_INTERNAL_UTILS_H
#define  __TERRALIB_INTERNAL_UTILS_H

#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning(disable: 4786)
#endif

#include <string>
#include <vector>
#include <cstdio>
#include <time.h>


#include "TeCoord2D.h"
#include "TeProgress.h"
#include "TeAttribute.h"
#include "TeBox.h"

class TeProjection;

using namespace std;

/** @defgroup Utils General purpose utilitary functions
 *  General purpose utilitary functions
 *  @{
 */

/** @defgroup FileNameFunctions Functions to manipulate file names 
	@ingroup  Utils
 *  @{
 */
	//! Get the extension part (right to the dot) of a string
	TL_DLL string TeGetExtension ( const char* value );

	//! Get the name part (left to the dot) of a string, path included
	TL_DLL string TeGetName ( const char* value );

	//! Get the base name part of a string, strip path information
	TL_DLL string TeGetBaseName ( const char* value );

	//! Get the path part of a file name
	TL_DLL string TeGetPath(const char* value);

	//! Writes a string to a file.
	/*!
		\param fileName the file name to write to, including the directory.
		\param text     the string with the text to write to.
		\param mode     the mode used to write to the file (a | w | r).
	*/
	TL_DLL void TeWriteToFile(const string& fileName, const string& text, const string& mode);
/** @} */ 

/** @defgroup C2StringFunctions Functions convert numbers to strings
	@ingroup  Utils
 *  @{
 */
	//! Transforms an integer to string
	TL_DLL string Te2String ( const int value );

	//! Transforms an unsigned integer to string
	TL_DLL string Te2String ( const unsigned int value );

	//! Transforms a long to string
	TL_DLL string Te2String ( const long value );
  
  //! Transforms a long long int to string
  TL_DLL string Te2String ( const long long int value );

	//! Transforms an unsigned long to string
	TL_DLL string Te2String ( const unsigned long value );

	//! Transforms a double to string floating point notation with precision decimal places
	TL_DLL string Te2String ( const double value, int precision );

	//! Transforms a double to string in exponential notation
	TL_DLL string Te2String ( const double value );

/** @} */ 

/** @defgroup StringFunctions Functions to manipulate strings
 	@ingroup  Utils
*  @{
 */
	//! Converts a string to upper case
	TL_DLL void TeConvertToUpperCase ( const string& , char* );

	//! Converts a string to upper case
	TL_DLL string TeConvertToUpperCase (const string &name);

	//! Converts a string to lower case
	TL_DLL string TeConvertToLowerCase (const string &name);

	//! Removes special characteres from a string
	TL_DLL std::string TeRemoveSpecialChars ( const std::string& str);

	//! Replace special characteres from a string
	TL_DLL std::string TeReplaceSpecialChars ( const std::string& str);

	//! Replace special characteres from a string 
	TL_DLL std::string TeReplaceSpecialChars ( const std::string& str,  bool& changed);

	//! Rename special field names
	TL_DLL bool TeRenameInvalidFieldName(TeAttributeList &attr);
  
  //! Remove all occurrences right decimal zeros from the real number represented by strIn.
  /*
    \param strIn Input String.
    \param separatorSymbol The symbol between the decimal string part
    and the remaining characteres.
    \param strOut Output string.
  */
  TL_DLL void TeRemoveRightDecimalZeroes( const std::string& strIn, 
    char separatorSymbol, std::string& strOut );
  

	//! Removes left and right blank, tab and carriage return characters of a string
	TL_DLL void TeTrim(string &str);

	//! Splits a string, given a separator, in a vector of parts
	TL_DLL int TeSplitString(const string& input, const string& delimiter, vector<string>& results);

	//! Compares two strings
	/*
		\params caseS flag indicating if it is a case sensitive comparation
	*/
	TL_DLL bool TeStringCompare(const string& str1, const string& str2, bool caseS=false);


	//! Validate a string to check if it can be used as a column name
	/*
		\param name			string to be checked
		\param changed		output flag to identify that string has changed
		\param invalidChar	output or sequence of chars that are invalid in the name
		\return the modified valid name
	*/
	TL_DLL string TeCheckName(const string& name, bool& changed, string& invalidChar);
/** @} */ 

/** @defgroup MathFunctions Mathematical functions
 	@ingroup  Utils
*  @{
 */
	//! Rounds a double to int
    inline int TeRound(double val)
	{	
		if (val>=0)
			return (int)(val+.5);
		else
			return (int)(val-.5);
	}

	//! Rounds a double value to a given number of decimal digits
	TL_DLL double TeRoundD(double val, int precision=8);

	//! Compares two doubles
	TL_DLL bool TeCompareDouble(double a, double b, int precision);

	//! Adjust a number to a given precision 
	/*
		\param val the number to be adjusted
		\param precision the number of decimals places to be used
		\param reduce flag to indicate that the number should be reduced instead to increased
		\return the adjusted number
	*/
	TL_DLL double TeAdjustToPrecision(double val, int precision, bool reduce=false);

	//! Rounds a double raster element index to an integer
	/*
		Raster elements have area, so raster element in upper-left position has
		index from [-0.5,+0.5) in i and j dimension.
	*/
    inline int TeRoundRasterIndex(double val)
	{	
		int ind = (int) val;
		if (val < (ind-0.5))
			ind -= 1;
		else if (val >= (ind+0.5))
			ind += 1;
		return ind;
	}
	/**
	* Cubic root from x.
	*
	* @param x X.
	* @return The cubic root from x.
	*/         
    inline double TeCubicRoot( double x )
	{
		if( x < 0 ) {
		return ( -1. ) * pow(  ( -1. ) * x, ( 1. / 3. ) );
		} else {
			return pow(  x, ( 1. / 3. ) );
		}
	};
	
	/*! Comparassion of two floating points, considering a given precision */
    inline bool TeFPEquals(double d1, double d2, double precision)
	{
		double eps1 = fabs(d1), 
			eps2 = fabs(d2), 
			eps;
		eps = (eps1 > eps2) ? eps1 : eps2;
		if (eps == 0.0)
			return true; //both numbers are 0.0
		eps *= precision;
		return (fabs(d1 - d2) < eps);
	}

	//! Swap the bytes of a short value
	static inline short swaps(short value)
	{
		short svalue = ((value & 0x00ff) << 8) | ((value >> 8) & 0x00ff);
		return svalue;
	}
  
  /**
   * Returns the amount of free physical memory (bytes).
   *
   * @return The amount of free physical memory (bytes).
   */
  TL_DLL unsigned long int TeGetFreePhysicalMemory();

  /**
   * Returns the amount of total physical memory (bytes).
   *
   * @return The amount of total physical memory (bytes).
   */
  TL_DLL unsigned long int TeGetTotalPhysicalMemory();
  
  /**
   * Returns the amount of used virtual memory (bytes)
   * for the current process (physical + swapped).
   *
   * @return The amount of free virtual memory (bytes).
   */
  TL_DLL unsigned long int TeGetUsedVirtualMemory();

  /**
   * Returns the amount of total virtual memory (bytes)
   * that can be claimed by the current process 
   * (physical + swapped).
   *
   * @return The amount of total virtual memory (bytes).
   */
  TL_DLL unsigned long int TeGetTotalVirtualMemory();  
  
  /**
   * Returns the number of physical processors.
   *
   * @return The number of physical processors.
   */
  TL_DLL unsigned int TeGetPhysProcNumber();  
    
  /**
   * Generates a temporary unique file name.
   *
   * @param filename The generated file name.
   * @return true if ok, false errors.
   */
  TL_DLL bool TeGetTempFileName( std::string& filename );    
  
  /**
   * @brief The file size (bytes).
   *
   * @note Throws an exception if file not found. 
   * @param filename The file name.
   * @return The file size (bytes).
   */
  TL_DLL unsigned long int TeGetFileSize( const std::string& filename );  
  
  /**
   * @brief Check the file existence.
   *
   * @param filename The file name.
   * @return true if the file exists, false if not.
   */
  TL_DLL bool TeCheckFileExistence( const std::string& filename );
  
  //! Get the full names of all files inside the given directory.
  /*
    \param path The directory path.
    \param recursive If true, a recursive search will be performed.
    \param filesnames The files names.
    \return true if OK, false on errors.
  */
  TL_DLL bool 
  TeGetDirFullFilesNames( const std::string& path,
    const bool& recursive,
    std::vector< std::string >& filesnames );  
    
  /**
   * @brief Creates a copy from the given input file name.
   *
   * @param inputFileName The input full file name.
   * @param outputFileName The output full file name.
   * @return true if OK, false if not.
   */
  TL_DLL bool TeCopyFile( const std::string& inputFileName,
    const std::string& outputFileName );   
    
  /**
   * @brief Compare two files.
   *
   * @param inputFileName1 The input full file 1 name.
   * @param inputFileName2 The input full file 2 name.
   * @return true if the two files are identical, false if not.
   */
  TL_DLL bool TeCompareFiles( const std::string& inputFileName1,
    const std::string& inputFileName2 );      
    
  /**
   * @brief Creates a copy from the given input text file name
   * replacing all ocurrences of
   *
   * @param inputFileName The input full file name.
   * @param inputFileName The output full file name.
   * @param oldSubString The sub-string to be replaced from the input file.
   * @param newSubString The new sub-string.
   * @return true if OK, false if not.
   */
  TL_DLL bool TeReplaceTextFileSubString( const std::string& inputFileName,
    const std::string& outputFileName, const std::string& oldSubString,
    const std::string& newSubString );    
    
  /**
   * @brief Creates a hash number from an input string.
   * @details This algorithm was created for sdbm (a public-domain 
   * reimplementation of ndbm) database library. It was found to do well in 
   * scrambling bits, causing better distribution of the keys and fewer 
   * splits. It also happens to be a good general hashing function with 
   * good distribution. the actual function is 
   * hash(i) = hash(i - 1) * 65599 + str[i]
   * @param inputStringFileName The input full file name.
   * @param inputStringSize Input string size.
   * @return The generated hash value.
   */
  TL_DLL unsigned long int TeCreateHashFromString( 
    unsigned char const* inputString,
    const unsigned int& inputStringSize );      


/** @} */ 
/** @} */ 
  
   /**
   * @brief Popule vectors with especial chars with accentuation and without 
   *
   * @param vectorIn Vector with accentuation.
   * @param vectorOut Vector without accentuation.
   * @return void
   */
  TL_DLL void TeGetEspecialCharsFixVector(std::vector<std::string> & especialIn, std::vector<std::string> & especialOut);

   /**
   * @brief Popule vectors with upper chars with accentuation and without 
   *
   * @param vectorIn Vector with accentuation.
   * @param vectorOut Vector without accentuation.
   * @return void
   */
  TL_DLL void TeGetAccentuatedUpperVector(std::vector<std::string> & upperIn, std::vector<std::string> & upperOut);

   /**
   * @brief Popule vectors with lower chars with accentuation and without 
   *
   * @param vectorIn Vector with accentuation.
   * @param vectorOut Vector without accentuation.
   * @return void
   */
  TL_DLL void TeGetAccentuatedLowerVector(std::vector<std::string> & lowerIn, std::vector<std::string> & lowerOut);

   /**
   * @brief Checks if the given box at the given projection is inside the LatLong's representation limits
   *
   * @param box The box to be verified
   * @param boxProjection The projection of the box.
   * @return TRUE if the box is consisnt, FALSE otherwise.
   */
  TL_DLL bool TeCheckBoxConsistency(const TeBox& box, TeProjection* boxProjection);

  /**
   * @brief From a given unit enum value, returns the std::string associated to it
   *
   * @param units The enum value
   * @return The std::string associated to it
   */
  TL_DLL std::string TeGetUnitName(const TeUnits& units);

   /**
   * @brief From a given std::string, returns the unit enum value associated to it
   *
   * @param units The std::string associated to it
   * @return The enum value
   */
  TL_DLL TeUnits TeGetUnit(const std::string& unitName);

  /**
   * @brief Converts the units of the given 'value' from the 'unitFrom' to the 'unitTo'
   *
   * @param value The value to be converted
   * @param unitFrom The current unit of 'value'
   * @param unitTo The output converted unit
   * @return The converted value.
   */
  TL_DLL double TeConvertUnits(const double& value, const TeUnits& unitFrom, const TeUnits& unitTo);
  
  /**
   * @brief From a given unit enum value, returns the std::string associated to it
   *
   * @param units The enum value
   * @return The std::string associated to it
   */
  TL_DLL std::string TeGetAreaUnitName(const TeAreaUnits& units);

   /**
   * @brief From a given std::string, returns the unit enum value associated to it
   *
   * @param units The std::string associated to it
   * @return The enum value
   */
  TL_DLL TeAreaUnits TeGetAreaUnit(const std::string& unitName);

  /**
   * @brief Converts the units of the given 'value' from the 'unitFrom' to the 'unitTo'
   *
   * @param value The value to be converted
   * @param unitFrom The current unit of 'value'
   * @param unitTo The output converted unit
   * @return The converted value.
   */
  TL_DLL double TeConvertAreaUnits(const double& value, const TeAreaUnits& unitFrom, const TeAreaUnits& unitTo);

  /** 
   * @brief Checks if the given file name exists of the disk
   *
   * @param fileName The name of the file
   * @return TRUE if the file exists. FALSE otherwise
   */
  TL_DLL bool TeFileExists(const std::string& fileName);

  /** 
   * @brief Copies the given fileNameIn to the given fileNameOut. If fileNameOut already exists, it will be overwritten.
   *
   * @param fileNameIn The name of the input file 
   * @param fileNameIn The name of the output file 
   * @return TRUE if the file was successfully copied. FALSE otherwise
   */
  TL_DLL bool TeFileCopy(const std::string& fileNameIn, const std::string& fileNameOut);

  /** 
   * @brief Deletes a file from the the disk
   *
   * @param fileName The name of the file to be deleted
   * @return TRUE if the file was deleted. FALSE otherwise
   */
  TL_DLL bool TeFileDelete(const std::string& fileName);

 /** 
   * @brief Renames a file
   *
   * @param oldFileName The actual name of the file 
   * @param newFileName The new name of the file 
   * @return TRUE if the file was successfully renamed. FALSE otherwise
   */
  TL_DLL bool TeFileRename(const std::string& oldFileName, const std::string& newFileName);

#endif


