Note to users of PostgreSQL driver:
===================================

1. The file "TePostgreSQL.h e TePostgreSQL.cpp" contains the especifics routines to access a PostgreSQL Server.

2. This version, makes use of RTree over GiST.
   So, it will be necessary installing this support, please, look the readme file in your GiST directory in the PostgreSQL contrib directory.

2. The tables for Raster, Polygon, Line, Cell and Point will use a column of type box to allow the use of GiST.
   An old database version can be upgraded by using TerraView or you can do via TerraLib API, with the 
   routines in the file "TeUpdateDBVersion.cpp" (directory application):
   - there are two specific routines to upgrade a existing database: updateDB302To310 and PostgreSQLUpdateDB302To310.

   
3. Any doubts or problems, please e-mail Gilberto Ribeiro de Queiroz at <gribeiro@dpi.inpe.br>


Note to users of PostGIS driver:
================================

1. The file "TePostGIS.h e TePostGIS.cpp" contains the especifics routines to access a PostgreSQL Server with a installed PostGIS extension.

2.This version, makes use of RTree over GiST.
  So, it will be necessary installing this support, please, look the readme file in your GiST directory in the PostgreSQL contrib directory.

3. The tables for Raster will use a column of type box to allow the use of R-Tree over GiST.
   An old database version can be upgraded by using TerraView or you can do via TerraLib API, with the 
   routines in the file "TeUpdateDBVersion.cpp" (directory application):
   - there are two specific routines to upgrade a existing database: updateDB302To310 and PostgreSQLUpdateDB302To310.

4. The table for Polygons, Lines, Cells and Points use PostGIS geometry type, so the index used were PostGIS R-Tree over GiST and they will be kept,
   only raster tables will change.

5. Any doubts or problems, please e-mail Gilberto Ribeiro de Queiroz at <gribeiro@dpi.inpe.br>


Note to all users of PostgreSQL and PostGIS drivers:
====================================================

1. Now, the binary cursor are supporting dates, times and timestamp in binary mode.

2. The files TePGUtils.h, TePGUtils.cpp, TePGInterface.h and TePGInterface.cpp are only for internal use, don't make use of it.


