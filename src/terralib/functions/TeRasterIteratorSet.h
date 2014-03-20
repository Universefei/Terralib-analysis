/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeRasterIteratorSet.h
*/

#ifndef _RASTER_ITERATOR_SET_H_
#define _RASTER_ITERATOR_SET_H_

#include <vector>

/*! Classe responsável por gerar um iterador que percorre em sequência
    todos os pontos retornados por um conjunto de iteradores raster.

Implementada para ser utilizada apenas com tipos descendentes de 
TeRaster::iterator.

Para utilizá-la, use a função addIterator para incluir no conjunto 
iteradores para os polígonos a serem percorridos. Utilize as funções
begin() e end() para obter um novo conjunto de iteradores que percorre
os pontos dos poligonos, na ordem em que eles foram inseridos no conjunto.

Não faz nenhuma verificação de sobreposição entre os pontos retornados
pelos iteradores de cada polígono.  Se houver, o novo iterador irá
passar mais de uma vez por cada ponto.
*/    
template <class It> 
class RasterIteratorSet
{
public:

  class iterator : public It
	{
	  public:
	  
	    iterator()
	      : _set(NULL), _currIndex(0) {}
	  
	    iterator(RasterIteratorSet<It>* set, unsigned index, It current)
	      : It(current), _set(set), _currIndex(index) {}

			iterator& operator++()
			{
			  if(It::operator++() == _set->_endSet.at(_currIndex) && 
			     _currIndex < (_set->_endSet.size()-1))
			  {   
			    _currIndex++;
			    // Altera estado para aquele do novo iterador
    			It* ptr = (It*)this;
		  	  *ptr = _set->_beginSet.at(_currIndex);
			  }  
			  return *this;
			}

			bool operator==(const iterator& rhs) const
			{
			  return (It::operator==(rhs) && _set == rhs._set && _currIndex == rhs._currIndex);
			}

			bool operator!=(const iterator& rhs) const
			{
			  return (It::operator!=(rhs) || _set != rhs._set || _currIndex != rhs._currIndex);
			}
	  
	  private:
	    RasterIteratorSet<It>* _set;
	    unsigned               _currIndex;
	};
		
	RasterIteratorSet() {}
			
	void addIterator(It begin, It end)
	{
	  _beginSet.push_back(begin);
	  _endSet.push_back(end);
	}

	iterator begin() { return _beginSet.size() ? iterator(this, 0, _beginSet.at(0)) : iterator(); }
	iterator end()	 { int i = _endSet.size()-1; return i >= 0 ? iterator(this, i, _endSet.at(i)) : iterator(); }

private:
  std::vector<It> _beginSet;
  std::vector<It> _endSet;	
};			

#endif

