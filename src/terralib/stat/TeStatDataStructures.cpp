/************************************************************************************
Exploring and analysis of geographical data using TerraLib and TerraView
Copyright � 2003-2007 INPE and LESTE/UFMG.

Partially funded by CNPq - Project SAUDAVEL, under grant no. 552044/2002-4,
SENASP-MJ and INPE

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed hoping it will be useful, however, WITHOUT ANY 
WARRANTIES; neither to the implicit warranty of MERCHANTABILITY OR SUITABILITY FOR
AN SPECIFIC FINALITY. Consult the GNU General Public License for more details.

You must have received a copy of the GNU General Public License with this program.
In negative case, write to the Free Software Foundation, Inc. in the following
address: 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.
***********************************************************************************/


#include "TeStatDataStructures.h"
#include "../kernel/TeSTElementSet.h"


TeSTStatInstance::TeSTStatInstance(int nDlbProp) : TeSTInstance() 
{
     initDbl(nDlbProp);
}

        
TeSTStatInstance::TeSTStatInstance(TeSTInstance& other, int nd, int ad): TeSTInstance(other)
{
      initDbl(nd+ad);
      int n;
      //Copia propriedades de um como double
      TeProperty prop;
      for(n = 0; n < nd; n++) 
	  {
        other.getProperty(prop, n);
        if ((prop.attr_.rep_.type_ != TeREAL) &&
          (prop.attr_.rep_.type_ != TeINT)) 
		{
          dblProperties_[n] = 0.0;
        }
        else 
		{
          dblProperties_[n] = atof(prop.value_.c_str());
        }
      }
}

    
TeSTStatInstance::TeSTStatInstance(const TeSTStatInstance& other) : TeSTInstance(other) 
{
    dblProperties_.clear();  
	for(unsigned int n = 0; n < other.dblProperties_.size(); n++) 
	{
       dblProperties_.push_back (other.dblProperties_[n]);
    }
}
    
TeSTStatInstance& 
TeSTStatInstance::operator=(const TeSTStatInstance& other)
{
	if ( this != &other )
	{
		this->dblProperties_.clear();
		for(unsigned int n = 0; n < other.dblProperties_.size(); n++) 
		{
			dblProperties_.push_back (other.dblProperties_[n]);
		}
	}
	return *this;
}
    

TeSTStatInstanceSet& 
TeSTStatInstanceSet::operator=(const TeSTStatInstanceSet& other)
{
	if ( this != &other )
	{
		this->objs_=other.objs_;
	}
	return *this;
}

void
TeSTStatInstanceSet::Copy(TeSTElementSet& stoSet, int nDbl, int aDbl) 
{
      TeSTElementSet::iterator it = stoSet.begin();
      while (it != stoSet.end()) 
	  {
        //Insere elemento no mapa atual e inicializa double
        TeSTStatInstance st(*it, nDbl, aDbl);
        objs_.push_back(st);  
		++it;
      }
}
    
    
int TeSTStatInstanceSet::numObjects() 
{
      return objs_.size();
}
    
