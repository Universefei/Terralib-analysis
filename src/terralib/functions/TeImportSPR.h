/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef Te_SPR_H
#define Te_SPR_H
#include <string>
using namespace std;

class TeLayer;

void TeImportSPR ( TeLayer& layer, const string& fileName );

#endif