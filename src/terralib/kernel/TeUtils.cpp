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

#include "TeUtils.h"
#include "TeDefines.h"
#include "TeException.h"
#include "TeAgnostic.h"
#include "TeProjection.h"
#include "TeVectorRemap.h"


#include <stdio.h>
#include <ctype.h>
#include <string>
#include <cstring> 
#include <cstdlib>

/* The following includes are needed for memory checking */
#if TePLATFORM == TePLATFORMCODE_MSWINDOWS
  #include <windows.h>
  #include <winbase.h>
#elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX
  #include <unistd.h>
  #include <string.h>
  #include <errno.h>
  #include <sys/resource.h>
  #include <sys/types.h>
  #include <sys/stat.h>  
  #include <dirent.h>
  #include <malloc.h>

  #if defined( __FreeBSD__ )
    #include <sys/sysctl.h> /* BSD workaround */
  #endif
#elif TePLATFORM == TePLATFORMCODE_APPLE
	#include <stdlib.h>
	#include <sys/sysctl.h> 
	#include <dirent.h>
  #include <sys/stat.h>  
#else
  #error "Unsuported plataform for physical memory checking"
#endif

#include <fstream>

string
Te2String ( const int value )
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%d", value );
	return name;
}

string
Te2String ( const unsigned int value )
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%u", value );
	return name;
}

string
Te2String ( const long value )
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%ld", value );
	return name;
}

string
Te2String ( const long long int value )
{
  char name [ TeNAME_LENGTH ];
  sprintf ( name, "%lld", value );
  return name;
}

string
Te2String ( const unsigned long value )
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%lu", value );
	return name;
}

void
TeTrim(string &str)
{
//empty strings are bad
    if(str.empty())
		return;
//remove left whitespace, \t and \n
	string::iterator it;
    for(it=str.begin();it!=str.end() &&
		((*it) == ' ' ||  (*it) == '\t' || (*it) == '\n');it++);
		str.erase(str.begin(),it);
//empty strings are bad
    if(str.empty())
		return;
//remove right whitespace, \t and \n
	it=str.end();
    for(--it;it!=str.begin() && 
	   ((*it) == ' ' ||  (*it) == '\t' || (*it) == '\n' || (*it) == 13);--it);  //13 = CR = Carriage Return
		str.erase(++it,str.end());
}

std::string
TeRemoveSpecialChars ( const std::string& str)
{
	string aux; 
	string::const_iterator it;
    for(it=str.begin();it!=str.end();it++)
	{
		char value = (*it);
		try
		{
			if(isalnum(value))
			{
				aux+=value;
			}
		}
		catch(...)
		{
		}
	}
	return aux;
}

std::string TeReplaceSpecialChars ( const std::string& str)
{
	bool changed;
	return TeReplaceSpecialChars (str, changed);
}

std::string TeReplaceSpecialChars ( const std::string& str, bool& changed)
{

	changed = false;

	std::vector<std::string> upperIn;
	std::vector<std::string> upperOut;
	std::vector<std::string> lowerIn;
	std::vector<std::string> lowerOut;
	std::vector<std::string> especialIn;
	std::vector<std::string> especialOut;

	TeGetAccentuatedUpperVector(upperIn, upperOut);
	TeGetAccentuatedLowerVector(lowerIn, lowerOut);
	TeGetEspecialCharsFixVector(especialIn, especialOut);


	std::string outputStr = str;
	for(unsigned int i = 0; i < outputStr.size(); ++i)
	{
		std::string value = "";
		value += outputStr[i];
		for(unsigned int j = 0; j < upperIn.size(); ++j)
		{
			if(outputStr[i] == upperIn[j][0])
			{
				outputStr[i] = upperOut[j][0];
				changed = true;
				break;
			}
		}
		
		for(unsigned int j = 0; j < lowerIn.size(); ++j)
		{
			if(value == lowerIn[j])
			{
				outputStr[i] = lowerOut[j][0];
				changed = true;
				break;
			}
		}

		for(unsigned int j = 0; j < especialIn.size(); ++j)
		{
			if(outputStr[i] == especialIn[j][0])
			{
				outputStr[i] = especialOut[j][0];
				changed = true;
				break;
			}
		}	
		
	}

	return outputStr;

}

void TeRemoveRightDecimalZeroes( const std::string& strIn,
  char separatorSymbol, std::string& strOut )
{
  const std::string::size_type separatorIdx = strIn.find( separatorSymbol );
  
  if( separatorIdx == std::string::npos )
  {
    strOut = strIn;
  }
  else
  {
    std::string::size_type firstNonZeroIdx = 
      strIn.find_last_not_of( '0' );
      
    if( firstNonZeroIdx != separatorIdx )
    {
      ++firstNonZeroIdx;
    }
    
    strOut.clear();
    
    for( unsigned int strInIdx = 0 ; strInIdx < firstNonZeroIdx ; 
      ++strInIdx )
    {
      strOut.push_back( strIn[ strInIdx ] );
    }
  }
}

string
Te2String ( const double value, int precision )
{
	char name [ TeNAME_LENGTH ];
	sprintf(name, "%.*f", precision, value );

	std::string strOut(name);
	size_t found = strOut.find(',');
	if(found != std::string::npos)
	{
		strOut[(int)found] = '.';
	}

	return strOut;
}


string
Te2String ( const double value )
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%e", value );

	std::string strOut(name);
	size_t found = strOut.find(',');
	if(found != std::string::npos)
	{
		strOut[(int)found] = '.';
	}

	return strOut;
}

string 
TeGetExtension ( const char* value )
{
	if (!value)
		return string("");
	string name = string(value);
	int len = strlen ( value );
	int ip = name.rfind('.');
	if( ( ip == (int)std::string::npos ) || ( ip == -1 ) )
		return "";
	else
		return name.substr(ip+1,len-1);
}

string 
TeGetName ( const char* value )
{
	if (!value)
		return string("");
	string name = string(value);
	int len = strlen ( value );

	int sp = name.rfind('\\')+1;
	int ip = (name.substr(sp,len-1)).rfind('.');
	int bp = (name.substr(sp,len-1)).rfind('/');
	if ((ip>0)&& (ip > bp))
		return name.substr(0,sp+ip);
	else 
		return name;
}

string
TeGetBaseName ( const char* value )
{
	if (!value)
		return string("");
	string name = string(value);
	size_t ip = name.rfind('.');
	size_t is = name.rfind('\\');
	if (is == std::string::npos)
		is = name.rfind('/');
	return name.substr(is+1,ip-is-1);
}

string 
TeGetPath(const char* value)
{
	if (!value)
		return string(".");
	string name = string (value);
	std::string::size_type ip = name.rfind('/');
	if (ip == std::string::npos)
		return string(".");
	else
		return name.substr(0,ip);
}

void
TeConvertToUpperCase ( const string& name, char upName[] )
{
	
	const char* old = name.c_str();

	const char* p;
	int i = 0;

	for( p = old; p < old + strlen ( old) ; p++ )
	{
		if ( islower ( *p  ) )
			upName[i] = toupper( *p );
		else
			upName [i] = *p;
		i++;
	}
	upName[i] = '\0';
}

string
TeConvertToUpperCase (const string &name)
{
	std::vector<std::string> upperIn;
	std::vector<std::string> lowerIn;
	std::vector<std::string> upperOut;
	std::vector<std::string> lowerOut;
	TeGetAccentuatedLowerVector(lowerIn, lowerOut);
	TeGetAccentuatedUpperVector(upperIn, upperOut);	

	bool flag = false;
 
	string aux = "";
	for(unsigned int i=0; i < strlen(name.c_str()); i++)
	{
		if((name[i] >= 97) && (name[i] <= 122))
			aux += name[i] - 32;
		else
		{
			for(unsigned j = 0; j < lowerIn.size(); j++)
			{
				if(name[i] == lowerIn[j][0])
				{
					aux += upperIn[j][0];
					flag = true;
					break;
				}				
			}

			if(!flag)
				aux += name[i];		
			
		}
			
    }
	return aux;
}


string
TeConvertToLowerCase (const string &name)
{
	std::vector<std::string> upperIn;
	std::vector<std::string> lowerIn;
	std::vector<std::string> upperOut;
	std::vector<std::string> lowerOut;
	TeGetAccentuatedLowerVector(lowerIn, lowerOut);
	TeGetAccentuatedUpperVector(upperIn, upperOut);

	bool flag = false;

	string aux = "";
	for(unsigned int i=0; i < strlen(name.c_str()); i++)
	{
		if((name[i] >= 65) && (name[i] <= 90))
			aux += name[i] + 32;
		else
		{
			for(unsigned j = 0; j < upperIn.size(); j++)
			{
				if(name[i] == upperIn[j][0])
				{
					aux += lowerIn[j][0];
					flag = true;
					break;
				}
				
			}

			if(!flag)
				aux += name[i];	
		}		

    }
	return aux;
}


//-----------------------------------------------------------------------
//
//      Purpose:        STL split string utility
//      Author:         Paul J. Weiss
//      Extracted from: The Code Project (http://www.codeproject.com/)
//
//------------------------------------------------------------------------

int TeSplitString(const string& input, const string& delimiter, vector<string>& results)
{
   int iPos = 0;
   int newPos = -1;
   int sizeS2 = delimiter.size();
   int isize = input.size();

   if (input.empty())
	   return 0;
  
   vector<int> positions;

   newPos = input.find (delimiter, 0);

   if( newPos < 0 ) 
   { 
       results.push_back(input);
	   return 1;
   }

   int numFound = 0;
   while( newPos >= iPos )
   {
       numFound++;
       positions.push_back(newPos);
       iPos = newPos;
       newPos = input.find (delimiter, iPos+sizeS2+1);
   }

   for( unsigned int i=0; i <= positions.size(); i++ )
   {
       string s;
       if( i == 0 )
	   {
	   s = input.substr( i, positions[i] );
	   }
	   else
	   {
       int offset=0;
	   if(i>0)	 offset= positions[i-1] + sizeS2;
       if( offset < isize )
       {
           if( i == positions.size() )
           {
		s = input.substr(offset);
           }
           else if( i > 0 )
           {
                s = input.substr( positions[i-1] + sizeS2, positions[i] - positions[i-1] - sizeS2 );
           }
       }
	   }
       if( s.size() > 0 )
       {
               results.push_back(s);
       }
   }
   return numFound;
}

bool TeNoCaseCmp(const char&  c1, const char& c2)
{
	return toupper(c1) == toupper(c2);
}


bool
TeStringCompare(const string& str1, const string& str2, bool caseS)
{
	if (!caseS)
	{
		return ((str1.size() == str2.size()) &&
			    equal(str1.begin(),str1.end(),str2.begin(),TeNoCaseCmp));
	}
	else
		return (str1 == str2);
}

double 
TeRoundD(double val, int precision)
{
	char name [ TeNAME_LENGTH ];
	sprintf ( name, "%.*f", precision, val);
	return atof(name);

}

bool
TeCompareDouble(double a, double b, int precision)
{
	char bufa [ TeNAME_LENGTH ];
	char bufb [ TeNAME_LENGTH ];
	if (precision == 0)
	{
		sprintf ( bufa, "%f", a );
		sprintf ( bufb, "%f", b );
	}
	else
	{
		sprintf ( bufa, "%.*f", precision, a );
		sprintf ( bufb, "%.*f", precision, b );
	}

	string A = bufa;
	string B = bufb;
	return (A == B);
}

void TeWriteToFile(const string& fileName, const string& text, const string& mode)
{
	FILE *f;

	f = fopen(fileName.c_str(), mode.c_str());

	if(f != NULL)
	{
		fprintf(f, "%s", text.c_str());

		fclose(f);
	}

	return;
}

double TeAdjustToPrecision(double val, int precision, bool reduce)
{
	double p = pow(10.0, (double)-precision);
	
	if (reduce)
		return (val - p);

	return (val + p);
}

string TeCheckName(const string& name, bool& changed, string& invalidChar)
{
  changed = false;

  if(name.empty())
  {
    return "";
  }

  string newName = name;

	if(newName[0] >= 0x30 && newName[0] <= 0x39)
	{
		invalidChar = "begin with a numeric character\n";
        changed = true;
	}
    if(newName[0] == '_')
    {
        invalidChar += "begin with a invalid character: underscore _\n";
        changed = true;
    }

	int ff = newName.find(" ");
	if(ff >= 0)
	{
        invalidChar += "invalid character: blank space\n";
        changed = true;
	}

	ff = newName.find(".");
	if(ff >= 0)
	{
        invalidChar += "invalid character: dot '.'\n";
        changed = true;
	}

	ff = newName.find("*");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '*'\n";
        changed = true;
	}

	ff = newName.find("/");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '/'\n";
        changed = true;
	}

	ff = newName.find("(");
	if(ff >= 0)
	{
        invalidChar += "invalid character: parentheses '('\n";
        changed = true;
	}

	ff = newName.find(")");
	if(ff >= 0)
	{
        invalidChar += "invalid character: parentheses ')'\n";
        changed = true;
	}
	ff = newName.find("-");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '-'\n";
        changed = true;
	}

	ff = newName.find("+");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '+'\n";
        changed = true;
	}

	ff = newName.find("%");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '%'\n";
        changed = true;
	}

	ff = newName.find(">");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '>'\n";
        changed = true;
	}

	ff = newName.find("<");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '<'\n";
        changed = true;
	}

	ff = newName.find("&");
	if(ff >= 0)
	{
        invalidChar += "invalid character: mathematical symbol '&'\n";
        changed = true;
	}

	ff = newName.find("$");
	if(ff >= 0)
	{
        invalidChar += "invalid symbol: '$'\n";
        changed = true;
	}
	
ff = newName.find(";");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: ';'\n";
        changed = true;
  }

  ff = newName.find("=");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: '='\n";
        changed = true;
  }

  ff = newName.find("!");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: '!'\n";
        changed = true;
  }

  ff = newName.find("?");
  if(ff >= 0)
  {
	  invalidChar += "invalid symbol: '?'\n";
	  changed = true;
  }

  ff = newName.find("#");
  if(ff >= 0)
  {
    invalidChar += "invalid symbol: '#'\n";
        changed = true;
  }

  ff = newName.find("¨");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: '¨'\n";
        changed = true;
  }

  ff = newName.find(",");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: ','\n";
        changed = true;
  }

  ff = newName.find("/");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: '/'\n";
        changed = true;
  }

  ff = newName.find("@");
  if(ff >= 0)
  {
        invalidChar += "invalid symbol: '@'\n";
        changed = true;
  }	

  ff = newName.find("{");
  if(ff >= 0)
  {
	  invalidChar += "invalid symbol: '{'\n";
	  changed = true;
  }

  ff = newName.find("}");
  if(ff >= 0)
  {
	  invalidChar += "invalid symbol: '}'\n";
	  changed = true;
  }
	std::vector<std::string> vecInvalidChars;
	vecInvalidChars.push_back("ª");
	vecInvalidChars.push_back("º");
	vecInvalidChars.push_back("¹");
	vecInvalidChars.push_back("²");
	vecInvalidChars.push_back("³");

	for(unsigned int i = 0; i < vecInvalidChars.size(); ++i)
	{
		std::string invalidItem = vecInvalidChars[i];

		ff = newName.find(invalidItem);
		if(ff >= 0)
		{
			invalidChar += "invalid symbol: '" + invalidItem + "'\n";
			changed = true;
		}
	}
	
  for(unsigned int i = 0; i < name.size(); ++i)
  { 
    char value = name[i];
    if(value < 0)
    {
      invalidChar += "invalid symbol\n";
      changed = true;
    }
  }	


	string u = TeConvertToUpperCase(newName);
	if(u=="OR" || u=="AND" || u=="NOT" || u=="LIKE" ||
	   u=="SELECT" || u=="FROM" || u=="UPDATE" || u=="DELETE" ||u=="BY" || u=="GROUP" || u=="ORDER" ||
	   u=="DROP" || u=="INTO" || u=="VALUE" || u=="IN" || u=="ASC" || u=="DESC"|| u=="COUNT" || u=="JOIN" ||
	   u=="LEFT" || u=="RIGHT" || u=="INNER" || u=="UNION" || u=="IS" || u=="NULL" || u=="WHERE" ||
	   u=="BETWEEN" || u=="DISTINCT" || u=="TRY" || u=="IT" || u=="INSERT" || u=="ALIASES" || u=="CREATE" ||
	   u=="ALTER" || u=="TABLE" || u=="INDEX" || u=="ALL" || u=="HAVING" || u=="EXEC" || u== "SET" ||
	   u == "AVG" || u == "MAX" || u == "MIN" || u == "SUM" || u == "FILTER" || u == "OFFSET"  || u == "LENGHT" )
	{
        invalidChar += "invalid name: using reserved word " + u + "\n";	
        changed = true;
	}

	// check against geometry tables field names
	string n = TeConvertToLowerCase(newName); 

	if (n == "x" || n == "y" || n == "object_id" ||
	   n == "geom_id" || n == "num_coords" || 
	   n == "lower_x" || n == "lower_y" ||
	   n == "upper_x" || n == "upper_y" ||
	   n == "ext_max" || n == "spatial_data" ||
	   n == "num_holes" || n == "parent_id" ||
	   n == "col_number" || n == "row_number" || 
	   n == "text_value" || n == "angle" ||
	   n == "height" || n == "alignment_vert" ||
	   n == "alignment_horiz" || n=="from_node" ||
	   n == "to_node")
	{
        invalidChar += "invalid name: using reserved word " + n + "\n";	
        changed = true;
	}

	// reserved words
	if( (n=="zone") || (n=="comp") || (n=="no") || (n=="local") ||
		(n=="level") || (n=="long"))
	{
        invalidChar += "invalid name: using reserved word " + n + "\n";	
        changed = true;
	}

	return newName;
}


unsigned long int TeGetFreePhysicalMemory()
{
  unsigned long int freemem = 0;

  #if defined __unix__  || TePLATFORM == TePLATFORMCODE_APPLE
    #if defined( __FreeBSD__ ) || TePLATFORM == TePLATFORMCODE_APPLE
      /* BSD workaround */
      
      unsigned int usermem;
      size_t usermem_len = sizeof( usermem );
      int mib[2] = { CTL_HW, HW_USERMEM };
      
      if( sysctl( mib, 2, &usermem, &usermem_len, NULL, 0 ) 
        == 0 ) {
        
        freemem = (unsigned long int)usermem;
      } else {
        TEAGN_LOG_AND_THROW( "TeGetFreePhysicalMemory error" );
      }
    #else
      /* Other linux stuff */
      
      freemem = (unsigned long int) sysconf( _SC_PAGESIZE ) *
        (unsigned long int) sysconf( _SC_AVPHYS_PAGES );
    #endif
  #elif defined WIN32
      LPMEMORYSTATUS status_buffer = new MEMORYSTATUS;
      GlobalMemoryStatus( status_buffer );
      freemem = (unsigned long int) status_buffer->dwAvailPhys;
      delete status_buffer;
  #else
      #error "Unsuported plataform for physical memory checking"
  #endif

  return freemem;
}


unsigned long int TeGetTotalPhysicalMemory()
{
  unsigned long int totalmem = 0;

  #if defined __unix__ || TePLATFORM == TePLATFORMCODE_APPLE
    #if defined( __FreeBSD__ ) || TePLATFORM == TePLATFORMCODE_APPLE
      /* BSD workaround */
      
      unsigned int physmem;
      size_t physmem_len = sizeof( physmem );
      int mib[2] = { CTL_HW, HW_PHYSMEM };
      
      if( sysctl( mib, 2, &physmem, &physmem_len, NULL, 0 ) 
        == 0 ) {
        
        totalmem = (unsigned long int)physmem; 
      } else {
        TEAGN_LOG_AND_THROW( "TeGetTotalPhysicalMemory error" );
      }
    #else
      /* Other linux stuff */  
  
      totalmem = (unsigned long int) sysconf( _SC_PAGESIZE ) *
        (unsigned long int) sysconf( _SC_PHYS_PAGES );
    #endif
  #elif defined WIN32
    LPMEMORYSTATUS status_buffer = new MEMORYSTATUS;
    GlobalMemoryStatus( status_buffer );
    totalmem = (unsigned long int) status_buffer->dwTotalPhys;
    delete status_buffer;
  #else
    #error "Unsuported plataform for physical memory checking"
  #endif

  return totalmem;
}

unsigned long int TeGetUsedVirtualMemory()
{
  unsigned long int usedmem = 0;
  
  #if TePLATFORM == TePLATFORMCODE_LINUX
    #if defined( __FreeBSD__ )
      struct rusage rusageinfo;
      getrusage( RUSAGE_SELF, &rusageinfo );
      usedmem = (unsigned long int)( 1024 * rusageinfo.ru_maxrss );
    #else
      std::string pid, comm, state, ppid, pgrp, session, tty_nr, 
        tpgid, flags, minflt, cminflt, majflt, cmajflt,
        utime, stime, cutime, cstime, priority, nice,
        stringO, itrealvalue, starttime;
    
      std::ifstream stat_stream("/proc/self/stat",std::ios_base::in); 
      
      stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr 
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt 
        >> utime >> stime >> cutime >> cstime >> priority >> nice 
        >> stringO >> itrealvalue >> starttime >> usedmem;    
    #endif
  #elif TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    struct rusage rusageinfo;
    getrusage( RUSAGE_SELF, &rusageinfo );
    usedmem = (unsigned long int)( 1024 * rusageinfo.ru_maxrss );
  #elif TePLATFORM == TePLATFORMCODE_MSWINDOWS
    LPMEMORYSTATUS status_buffer = new MEMORYSTATUS;
    GlobalMemoryStatus( status_buffer );
    usedmem = (unsigned long int)( status_buffer->dwTotalVirtual -
      status_buffer->dwAvailVirtual );
    delete status_buffer;
  #else
      #error "Unsuported plataform for virtual memory checking"
  #endif

  return usedmem;
}


unsigned long int TeGetTotalVirtualMemory()
{
  unsigned long int totalmem = 0;

  #if defined __unix__ || TePLATFORM == TePLATFORMCODE_APPLE
    struct rlimit info;
     
    if( getrlimit( RLIMIT_AS, &info ) == 0 )
    {
      totalmem = (unsigned long int)info.rlim_max;
    };
  #elif defined WIN32
    LPMEMORYSTATUS status_buffer = new MEMORYSTATUS;
    GlobalMemoryStatus( status_buffer );
    totalmem = (unsigned long int) status_buffer->dwTotalVirtual;
    delete status_buffer;
  #else
    #error "Unsuported plataform for virtual memory checking"
  #endif

  return totalmem;
}

unsigned int TeGetPhysProcNumber()
{
  unsigned int procnmb = 0;
  
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    procnmb = (unsigned int)siSysInfo.dwNumberOfProcessors;
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    procnmb = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN );
  #else
    #error "ERROR: Unsupported platform"
  #endif    

  return procnmb;
}
  
  
bool TeGetTempFileName( std::string& filename )
{
  char* name = tempnam( 0, 0 );
  
  if( name == 0 ) {
    filename.clear();
    return false;
  } else {
    filename = std::string( name );
    return true;
  }
}  


unsigned long int TeGetFileSize( const std::string& filename )
{
  FILE* fileptr = fopen( filename.c_str(), "r" );
  
  if( fileptr == 0 ) {
    throw TeException( CANNOT_OPEN_FILE, "File not found", false );
  }
  
  fseek( fileptr, 0, SEEK_END );
  
  unsigned long int filesize = ( unsigned long int ) ftell( fileptr );
  
  fclose( fileptr );
  
  return filesize;
}


bool TeCheckFileExistence( const std::string& filename )
{
  FILE* fileptr = fopen( filename.c_str(), "r" );
  
  if( fileptr == 0 ) {
    return false;
  } else {
    fclose( fileptr );
    return true;
  }
}

bool 
TeGetDirFullFilesNames( const std::string& path,
  const bool& recursive, std::vector< std::string >& filesnames )
{
  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    const std::string separator( "\\" );
  
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    hFind = FindFirstFile( (LPCTSTR)(path + "\\*" ).c_str(), 
      &FindFileData);

    if ( hFind == INVALID_HANDLE_VALUE ) {
      return false;
    } else {
      while( FindNextFile( hFind, &FindFileData) != 0 ) 
      {
        if( FindFileData.dwFileAttributes & 
          FILE_ATTRIBUTE_DIRECTORY ) 
        {
          if( recursive && ( strcmp( (char*)(FindFileData.cFileName), "." ) != 0 ) && 
            ( strcmp( (char*)(FindFileData.cFileName), ".." ) != 0 ) ) 
          {
            std::string recursive_path = path + separator + (char*)(FindFileData.cFileName);
            
            if( ! ( TeGetDirFullFilesNames( recursive_path, true,
              filesnames ) ) ) 
            {
              return false;
            } 
          }
        } else {
          filesnames.push_back( path + separator + (char*)FindFileData.cFileName );
        }
      }

      FindClose( hFind );
    }
  #elif TePLATFORM == TePLATFORMCODE_LINUX || TePLATFORM == TePLATFORMCODE_AIX || TePLATFORM == TePLATFORMCODE_APPLE
    DIR* dirptr = opendir( path.c_str() );
    
    if( dirptr == 0 ) 
    {
      return false;
    }
    else
    {
      dirent* direntptr = readdir( dirptr );
      struct stat stat_buf;
      char filename[ 3000 ];
      
      while( direntptr )
      {
        filename[ 0 ] = '\0';
        strcpy( filename, path.c_str() );
        strcat( filename, "/" );
        strcat( filename, direntptr->d_name );
        
        if( stat( filename, &stat_buf) )
        {
          closedir( dirptr );
          return false;
        }
        else
        {
          if( S_ISDIR( stat_buf.st_mode ) )
          {
            if( recursive && ( strcmp( direntptr->d_name, "." ) != 0 ) && 
              ( strcmp( direntptr->d_name, ".." ) != 0 ) )
            {
              if( TeGetDirFullFilesNames( filename, true, filesnames ) == false )
              {
                closedir( dirptr );
                return false;
              }
            }
          }
          else
          {
            filesnames.push_back( filename );
          }
        }
        
        direntptr = readdir( dirptr );
      }
      
      closedir( dirptr );
    }
  #else
    #error "ERROR: Unsuported platform"
  #endif   
  
  return true; 
}

bool TeCopyFile( const std::string& inputFileName,
  const std::string& outputFileName )
{
  std::ifstream ifs(inputFileName.c_str(),std::ios::in | std::ios::binary);
  if( ifs.is_open() )
  {
    std::ofstream ofs(outputFileName.c_str(),std::ios::out | std::ios::binary);
    if( ofs.is_open() )
    {
      ofs << ifs.rdbuf();
      return ! ofs.bad();
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool TeCompareFiles( const std::string& inputFileName1,
    const std::string& inputFileName2 )
{
  std::ifstream ifs1(inputFileName1.c_str(),std::ios::in | std::ios::binary |
    std::ios_base::ate );
  std::ifstream ifs2(inputFileName2.c_str(),std::ios::in | std::ios::binary |
    std::ios_base::ate );
  
  if( ifs1.is_open() && ifs2.is_open() )
  {
    if( ifs1.tellg() == ifs2.tellg() )
    {  
      ifs1.seekg( 0, ios_base::beg );
      ifs2.seekg( 0, ios_base::beg );
      
      char data1 = 0;
      char data2 = 0;
      
      while( ! ifs1.eof() )
      {
        if( ifs1.read( &data1, 1 ).bad() )
          return false;
        if( ifs2.read( &data2, 1 ).bad() )
          return false;        
        if( data1 != data2 )
          return false;  
      }
      
      return true;
    }
    else
    {
      return false;
    }    
  }
  else
  {
    return false;
  }
}

bool TeReplaceTextFileSubString( const std::string& inputFileName,
    const std::string& outputFileName, const std::string& oldSubString,
    const std::string& newSubString )
{
  std::ifstream ifs(inputFileName.c_str(),std::ios::in);
  if( ifs.is_open() )
  {
    std::ofstream ofs(outputFileName.c_str(),std::ios::out);
    
    if( ofs.is_open() )
    {
      std::string lineStr;
      std::string::size_type findPos = 0;
      
      do
      {
        getline( ifs, lineStr );
        
        while( ( findPos = lineStr.find( oldSubString, 0 ) ) < 
          std::string::npos )
        {
          lineStr.erase( findPos, oldSubString.size() );
          lineStr.insert( findPos, newSubString );
        }
        
        ofs << lineStr << std::endl;
      }
      while( ! ifs.eof() );
      
      return ! ofs.bad();
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

unsigned long int TeCreateHashFromString( unsigned char const* inputString,
  const unsigned int& inputStringSize )
{
  unsigned long int hash = 0;

  for( register unsigned int idx = 0 ; idx < inputStringSize ; ++idx )
  {  
    hash = ((unsigned long int)inputString[ idx ]) + (hash << 6) + 
      (hash << 16) - hash;
  }
  
  return hash;
}


bool TeRenameInvalidFieldName(TeAttributeList &attr)
{
	std::string							fieldName;
	std::string							errorMessage;
	bool								changed;
	bool								renamed=false;

	//for(it=attr.begin();it!=attr.end();it++)
	for(unsigned int i = 0; i < attr.size(); ++i)
	{
		fieldName=attr[i].rep_.name_;
		if(!fieldName.empty() && isdigit(fieldName[0]))
		{
			fieldName="Te" + fieldName;
			renamed=true;
		}
		TeCheckName(fieldName, changed, errorMessage);
		if(changed)
		{
			fieldName=TeRemoveSpecialChars(fieldName);
			fieldName+="_A";
			fieldName+=Te2String(i+1);
			renamed=true;
		}
		attr[i].rep_.name_=fieldName;
	}
	return renamed;
}

void TeGetEspecialCharsFixVector(std::vector<std::string> & especialIn, std::vector<std::string> & especialOut)
{
	especialIn.push_back("ª"); 
	especialIn.push_back("º");
	especialIn.push_back("¹");
	especialIn.push_back("²");
	especialIn.push_back("³");	

	especialOut.push_back("a");
	especialOut.push_back("o");
	especialOut.push_back("1");
	especialOut.push_back("2");
	especialOut.push_back("3");	
}

void TeGetAccentuatedUpperVector(std::vector<std::string> & upperIn, std::vector<std::string> & upperOut)
{
	upperIn.push_back("Á");
	upperIn.push_back("É");
	upperIn.push_back("Í");
	upperIn.push_back("Ó");
	upperIn.push_back("Ú");
	upperIn.push_back("À");
	upperIn.push_back("È");
	upperIn.push_back("Ì");
	upperIn.push_back("Ò");
	upperIn.push_back("Ù");
	upperIn.push_back("Ä");
	upperIn.push_back("Ë");
	upperIn.push_back("Ï");
	upperIn.push_back("Ö");
	upperIn.push_back("Ü");
	upperIn.push_back("Â");
	upperIn.push_back("Ê");
	upperIn.push_back("Î");
	upperIn.push_back("Ô");
	upperIn.push_back("Û");
	upperIn.push_back("Ã");
	upperIn.push_back("Õ");
	upperIn.push_back("Ç");

	upperOut.push_back("A");
	upperOut.push_back("E");
	upperOut.push_back("I");
	upperOut.push_back("O");
	upperOut.push_back("U");
	upperOut.push_back("A");
	upperOut.push_back("E");
	upperOut.push_back("I");
	upperOut.push_back("O");
	upperOut.push_back("U");
	upperOut.push_back("A");
	upperOut.push_back("E");
	upperOut.push_back("I");
	upperOut.push_back("O");
	upperOut.push_back("U");
	upperOut.push_back("A");
	upperOut.push_back("E");
	upperOut.push_back("I");
	upperOut.push_back("O");
	upperOut.push_back("U");
	upperOut.push_back("A");
	upperOut.push_back("O");
	upperOut.push_back("C");
}

void TeGetAccentuatedLowerVector(std::vector<std::string> & lowerIn, std::vector<std::string> & lowerOut)
{
	lowerIn.push_back("á");
	lowerIn.push_back("é");
	lowerIn.push_back("í");
	lowerIn.push_back("ó");
	lowerIn.push_back("ú");
	lowerIn.push_back("à");
	lowerIn.push_back("è");
	lowerIn.push_back("ì");
	lowerIn.push_back("ò");
	lowerIn.push_back("ù");
	lowerIn.push_back("ä");
	lowerIn.push_back("ë");
	lowerIn.push_back("ï");
	lowerIn.push_back("ö");
	lowerIn.push_back("ü");
	lowerIn.push_back("â");
	lowerIn.push_back("ê");
	lowerIn.push_back("î");
	lowerIn.push_back("ô");
	lowerIn.push_back("û");
	lowerIn.push_back("ã");
	lowerIn.push_back("õ");

	std::string str = "ç";
	lowerIn.push_back(str);

	lowerOut.push_back("a");
	lowerOut.push_back("e");
	lowerOut.push_back("i");
	lowerOut.push_back("o");
	lowerOut.push_back("u");
	lowerOut.push_back("a");
	lowerOut.push_back("e");
	lowerOut.push_back("i");
	lowerOut.push_back("o");
	lowerOut.push_back("u");
	lowerOut.push_back("a");
	lowerOut.push_back("e");
	lowerOut.push_back("i");
	lowerOut.push_back("o");
	lowerOut.push_back("u");
	lowerOut.push_back("a");
	lowerOut.push_back("e");
	lowerOut.push_back("i");
	lowerOut.push_back("o");
	lowerOut.push_back("u");
	lowerOut.push_back("a");
	lowerOut.push_back("o");
	lowerOut.push_back("c");

}

 bool TeCheckBoxConsistency(const TeBox& box, TeProjection* boxProjection)
 {
	if(box.isValid() == false)
	{
		return false;
	}

	TeBox boxToUse(box);
	if(boxProjection->name() != "LatLong")
	{
		TeProjectionParams params;
		params.name = "LatLong";
		params.datum = TeDatumFactory::make("SAD69");

		TeProjection* projLatLong = TeProjectionFactory::make(params);

		boxToUse = TeRemapBox(box, boxProjection, projLatLong);
	}

	if(boxToUse.x1() < -180. || boxToUse.y1() < -90. || boxToUse.x2() > 180. || boxToUse.y2() > 90.)
	{
		return false;
	}

	return true;
 }

std::string TeGetUnitName(const TeUnits& units)
{
	TeUnits cUnits = units;
	switch(cUnits)
	{
		case TeMillimeters:
			return "Millimeters";
		case TeCentimeters:
			return "Centimeters";
		case TeMeters:
			return "Meters";
		case TeKilometers:
			return "Kilometers";
		case TeMiles:
			return "Miles";
		case TeInches:
			return "Inches";
		case TeFeet:
			return "Feet";
		case TeDecimalDegress:
			return "DecimalDegrees";
		case TeUndefinedUnit:
			return "DecimalDegrees";
		default:
			return "";
	}

	return "";
}

TeUnits TeGetUnit(const std::string& unitName)
{
	std::string cUnitName = TeConvertToLowerCase(unitName);
	if(cUnitName == "millimeters")
	{
		return TeMillimeters;
	}
	if(cUnitName == "centimeters")
	{
		return TeCentimeters;
	}
	if(cUnitName == "meters")
	{
		return TeMeters;
	}
	if(cUnitName == "kilometers")
	{
		return TeKilometers;
	}
	if(cUnitName == "miles")
	{
		return TeMiles;
	}
	if(cUnitName == "feet")
	{
		return TeFeet;
	}
	if(cUnitName == "decimaldegrees")
	{
		return TeDecimalDegress;
	}

	return TeUndefinedUnit;
}


double TeConvertUnits(const double& value, const TeUnits& unitFrom, const TeUnits& unitTo)
{
	double valueInMeters = 0.0;
	double valueOut = 0.0;
	
	if(unitFrom == TeMeters)
	{
		valueInMeters = value;
	}
	else if(unitFrom == TeDecimalDegress)
	{
		valueInMeters = ((2 * TePI * TeEARTHRADIUS * value) / 360.);
	}
	else if(unitFrom == TeMillimeters)
	{
		valueInMeters = value / 1000.;
	}
	else if(unitFrom == TeCentimeters)
	{
		valueInMeters = value / 100.;
	}	
	else if(unitFrom == TeKilometers)
	{
		valueInMeters = value * 1000.;
	}
	else if(unitFrom == TeMiles)
	{
		valueInMeters = value * 1609.3;
	}
	else if(unitFrom == TeInches)
	{
		valueInMeters = value / 100. * 2.54;
	}
	else if(unitFrom == TeFeet)
	{
		valueInMeters = value / 100. * 30.48;
	}	
	else
	{
		valueInMeters = value;
	}

	if(unitTo == TeMeters)
	{
		valueOut = valueInMeters;
	}
	else if(unitTo == TeDecimalDegress)
	{
		valueOut = ((360. * valueInMeters) / (2 * TePI * TeEARTHRADIUS));
	}
	else if(unitTo == TeMillimeters)
	{
		valueOut = valueInMeters * 1000.;
	}
	else if(unitTo == TeCentimeters)
	{
		valueOut = valueInMeters * 100.;
	}
	else if(unitTo == TeKilometers)
	{
		valueOut = valueInMeters / 1000.;
	}
	else if(unitTo == TeMiles)
	{
		valueOut = valueInMeters / 1609.3;
	}
	else if(unitTo == TeInches)
	{
		valueOut = valueInMeters * 100. / 2.54;
	}
	else if(unitTo == TeFeet)
	{
		valueOut = valueInMeters * 100. / 30.48;
	}	
	else
	{
		valueOut = valueInMeters;
	}

	return valueOut;
}

std::string TeGetAreaUnitName(const TeAreaUnits& units)
{
	TeAreaUnits cUnits = units;
	switch(cUnits)
	{
	case TeSquareMeters:
		return "Meters";
	case TeSquareKilometers:
		return "Kilometers";
	case TeHectares:
		return "Hectares";
	case TeSquareMillimeters:
		return "Millimeters";
	case TeSquareCentimeters:
		return "Centimeters";
	case TeSquareMiles:
		return "Miles";
	case TeSquareInches:
		return "Inches";
	case TeSquareFeet:
		return "Feet";
	default:
		return "Meters";
	}

}

TeAreaUnits TeGetAreaUnit(const std::string& unitName)
{
	std::string cUnitName = TeConvertToLowerCase(unitName);
	
	if(cUnitName == "meters")
	{
		return TeSquareMeters;
	}
	if(cUnitName == "kilometers")
	{
		return TeSquareKilometers;
	}
	if(cUnitName == "hectares")
	{
		return TeHectares;
	}
	if(cUnitName == "millimeters")
	{
		return TeSquareMillimeters;
	}
	if(cUnitName == "centimeters")
	{
		return TeSquareCentimeters;
	}
	if(cUnitName == "miles")
	{
		return TeSquareMiles;
	}
	if(cUnitName == "feet")
	{
		return TeSquareFeet;
	}	
	if(cUnitName == "inches")
	{
		return TeSquareInches;
	}
	return TeUndefinedAreaUnit;
}

double TeConvertAreaUnits(const double& value, const TeAreaUnits& unitFrom, const TeAreaUnits& unitTo)
{
	double valueInMeters = 0.0;
	
	if(unitTo == unitFrom)
	{
		return value;
	}

	switch (unitFrom)
	{
	case TeSquareMeters:
		valueInMeters = value;
		break;
	case TeSquareKilometers:
		valueInMeters = value * 1000000;
		break;	
	case TeHectares:
		valueInMeters =  value * 10000;
		break;	
	case TeSquareCentimeters:
		valueInMeters =  value / 1000;
		break;	
	case TeSquareMillimeters:
		valueInMeters =   value / 1000000;
		break;		
	case TeSquareFeet:
		valueInMeters =  value / 10.76391041671; 		
		break;	
	case TeSquareInches:
		valueInMeters =  value / 1550.0031000062;
		break;	
	case TeSquareMiles:
		valueInMeters =  value * 2589988.110336 ;	
		break;
	case TeUndefinedAreaUnit:
	default:
		return value;
	
	}
	
	switch (unitTo)
	{
	case TeSquareKilometers:
		return valueInMeters / 1000000;
	case TeHectares:
		return valueInMeters / 10000;
	case TeSquareCentimeters:
		return valueInMeters * 10000; 	
	case TeSquareMillimeters:
		return valueInMeters * 1000000; 	
	case TeSquareFeet:
		return valueInMeters * 10.76391041671; 	
	case TeSquareInches:
		return valueInMeters * 1550.0031000062;
	case TeSquareMiles:
		return valueInMeters / 2589988.110336 ;
	case TeUndefinedAreaUnit:
	default:
		return value;
	}
	return valueInMeters;
}

bool TeFileExists(const std::string& fileName)
{
	if(fileName.empty())
	{
		return false;
	}

	/* open source file */
	FILE* file = fopen(fileName.c_str(), "r");
	if(file == NULL)
	{
		return false;
	}

	fclose(file);

	return true;
}

bool TeFileCopy(const std::string& fileNameIn, const std::string& fileNameOut)
{
	if(fileNameIn.empty())
	{
		return false;
	}
	if(fileNameOut.empty())
	{
		return false;
	}

	FILE* from = 0;
	FILE* to = 0;
	char ch;

	/* open source file */
	if((from = fopen(fileNameIn.c_str(), "rb")) == 0)
	{
		return false;
	}

	/* open destination file */
	if((to = fopen(fileNameOut.c_str(), "wb"))== 0)
	{
		fclose(from);
		return false;
	}

	/* copy the file */
	while(!feof(from))
	{
		ch = fgetc(from);
		if(ferror(from))
		{
			fclose(from);
			fclose(to);
			return false;
		}

		if(!feof(from))
		{
			fputc(ch, to);
		}

		if(ferror(to))
		{
			fclose(from);
			fclose(to);
			return false;
		}
	}

	if(fclose(from)==EOF)
	{
		fclose(to);
		return false;
	}

	if(fclose(to)==EOF)
	{
		return false;
	}
	return true;
}

bool TeFileDelete(const std::string& fileName)
{
	if(TeFileExists(fileName) == false)
	{
		return false;
	}

	if(remove(fileName.c_str()) != 0)
	{
		return false;
	}

	return true;
}

bool TeFileRename(const std::string& oldFileName, const std::string& newFileName)
{
	if(TeFileExists(oldFileName) == false)
	{
		return false;
	}

//caso o arquivo destino exista, nao podemos renomear
	if(TeFileExists(newFileName) == true)
	{
		return false;
	}

	if(rename(oldFileName.c_str(), newFileName.c_str()) != 0)
	{
		return false;
	}

	return true;
}