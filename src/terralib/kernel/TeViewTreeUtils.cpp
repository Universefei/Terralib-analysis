/*
** ---------------------------------------------------------------
** teviewtreeutils.cpp - This module contains some functions to be 
** used with the TeViewTree class. These functions should probably be
** methods of the own TeView class.
**
*/

/*
** ---------------------------------------------------------------
** Includes:
*/


#include "TeViewTreeUtils.h"
#include "TeViewTreeIterator.h"

#include <string.h>

/*
** ----------------------------------------------------------------------------
** Constants:
*/


/*
** ---------------------------------------------------------------
** Internal Functions:
*/

/*
** ---------------------------------------------------------------
** Public Functions:
*/


void TeClearViewTree(TeViewTree* vtree)
{
  if(vtree==NULL) return;

  while(vtree->size() > 0)
  {
    TeViewNode* node = vtree->removeID(vtree->size() - 1);

    if(node == NULL) continue;

    if(node->type() == TeTREE)
      TeClearViewTree((TeViewTree*)node);
    delete node;
  }
}

/*!
   \brief Look at a View Tree for a theme with the given name.

   This function creates a simple theme with the given parameters. 
   A simple theme is a non thematic theme without any associated 
   where clause and with only one attribute table associated to it. 
   It is assumed that, given the feature (layer) name, 
   its the attributes table is named name_attr while the geometries
   tables are called namePontos, nameLinhas and namePoligonos for
   respectively the Points geometry table, Lines geometry table and
   Poligons geometry table.

   \param vtree theme name and also name of the theme associated feature(layer)
   \param name name of the theme to be found
   \param case_sensitive(optional) name of the key field of the associated 
                        attribute table. If no key is given it is
                        assumed that the key name is object_id.
*/
TeTheme* TeFindTheme(TeViewTree& vtree, char* name, bool case_sensitive)
{
  TeTheme* theme = NULL;
  TeViewTreeIterator it(&vtree);

  for(it.firstLeaf();!(it.isDone());it.nextLeaf())
  {
    TeViewNode* node = it.currentNode();
    int ret; // return value of the string comparation functions

    if(node->type() != TeTHEME) 
      continue;

    if(case_sensitive)
      ret = strcmp(name, node->name().c_str());
    else
    {
//      ret = stricmp(name, node->name().c_str()); // do not work for Linux
      string s1 = TeConvertToLowerCase(string(name));
      string s2 = TeConvertToLowerCase(node->name());
      if (s1 == s2)
      	ret = 0;
      else
      	ret = -1;
   }

    if(ret == 0)
    {
      theme = (TeTheme*)node;
      break;
    }

  }

  return theme;
}
