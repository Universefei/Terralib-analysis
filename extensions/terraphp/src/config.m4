dnl $Id: config.m4 7886 2009-06-03 13:54:00Z gribeiro $
dnl config.m4 for extension terraweb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(terraweb, for terraweb support,
[  --with-terraweb             Include terraweb support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(terraweb, whether to enable terraweb support,
dnl Make sure that the comment is aligned:
dnl [  --enable-terraweb           Enable terraweb support])

if test "$PHP_TERRAWEB" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-terraweb -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/terraweb.h"  # you most likely want to change this
  dnl if test -r $PHP_TERRAWEB/; then # path given as parameter
  dnl   TERRAWEB_DIR=$PHP_TERRAWEB
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for terraweb files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TERRAWEB_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TERRAWEB_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the terraweb distribution])
  dnl fi

  dnl # --with-terraweb -> add include path
  dnl PHP_ADD_INCLUDE($TERRAWEB_DIR/include)

  dnl # --with-terraweb -> check for lib and symbol presence
  dnl LIBNAME=terraweb # you may want to change this
  dnl LIBSYMBOL=terraweb # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TERRAWEB_DIR/lib, TERRAWEB_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TERRAWEBLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong terraweb lib version or lib not found])
  dnl ],[
  dnl   -L$TERRAWEB_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(TERRAWEB_SHARED_LIBADD)

  PHP_ADD_INCLUDE(/usr/local/include/terralib)
  PHP_ADD_LIBRARY(stdc++, 1, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY(gd, 1, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(terramanager, /usr/local/lib/terralib, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(terralib, /usr/local/lib/terralib, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(terralibtiff, /usr/local/lib/terralib, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(jpeg, /usr/local/lib/terralib, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(shapelib, /usr/local/lib/terralib, TERRAWEB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(pq, /usr/local/pgsql/lib, TERRAWEB_SHARED_LIBADD)


  PHP_SUBST(TERRAWEB_SHARED_LIBADD)
  PHP_NEW_EXTENSION(terraweb, terraweb.cpp,  $ext_shared)
  PHP_REQUIRE_CXX()
fi
