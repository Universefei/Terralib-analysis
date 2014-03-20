/*
** ---------------------------------------------------------------
** teviewtreeutils.h:
**
*/


#ifndef TEVIEWTREEUTILS_H_
#define TEVIEWTREEUTILS_H_

/*
** ---------------------------------------------------------------
** Includes:
*/

#include "TeViewNode.h"
#include "TeTheme.h"


/*
** ---------------------------------------------------------------
** Functions Prototypes:
*/

TL_DLL void TeClearViewTree(TeViewTree* vtree);
TL_DLL TeTheme* TeFindTheme(TeViewTree& vtree, char* name, bool case_sensitive = false);

/*
** ---------------------------------------------------------------
** Inline Functions:
*/


/*
** ---------------------------------------------------------------
** End:
*/

#endif
