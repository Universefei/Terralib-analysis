m4_include(../m4/tlglobals.m4)

AC_INIT([terralib_src],TL_VERSION_MACRO)
AC_CONFIG_AUX_DIR([.])
AM_PROG_CC_C_O
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG(C++)
AC_CONFIG_MACRO_DIR([m4])

# ------------------------------------------------------------
# Getting used defined options

m4_include(../m4/tloptions.m4)
m4_include(../m4/tldefines.m4)

# ------------------------------------------------------------
# Threads required stuff

CXXFLAGS="-pthread $CXXFLAGS"
CFLAGS="-pthread $CFLAGS"
LDFLAGS="-lpthread $LDFLAGS"

# ------------------------------------------------------------
# Configuring debug/release related stuf

AC_SUBST(AM_LDFLAGS,["-release TL_VERSION_MACRO"])

# ------------------------------------------------------------
# Generating config file

AC_CONFIG_HEADERS([config.h:config.h.in],[])

m4_include([m4/ax_prefix_config_h.m4])
AX_PREFIX_CONFIG_H($srcdir/terralib/kernel/TeUnixPlatformDefines.h,TE,config.h)

# ------------------------------------------------------------
# Optional modules

AM_CONDITIONAL([TE_BUILD_GEOTIFF_MODULE],[test "x$tlpkgs_geotiff_status" == xyes])

AM_CONDITIONAL([TE_BUILD_JPEG_MODULE],[test "x$tlpkgs_jpeg_status" == xyes])

AM_CONDITIONAL([TE_BUILD_MYSQL_MODULE],[test "x$tlpkgs_mysql_status" == xyes])

AM_CONDITIONAL([TE_BUILD_POSTGIS_MODULE],[test "x$tlpkgs_postgis_status" == xyes])

AM_CONDITIONAL([TE_BUILD_QT_MODULE],[test "x$tlpkgs_qt_status" == xyes])

AM_CONDITIONAL([TE_BUILD_GDAL_MODULE],[test "x$tlpkgs_gdal_status" == xyes])

AM_CONDITIONAL([TE_BUILD_NETPBM_MODULE],[test "x$tlpkgs_netpbm_status" == xyes])

AM_CONDITIONAL([TE_BUILD_QWT_MODULE],[test "x$tlpkgs_qwt_status" == xyes])

AM_CONDITIONAL([TE_BUILD_SHAPELIB_MODULE],[test "x$tlpkgs_shapelib_status" == xyes])

AM_CONDITIONAL([TE_BUILD_MRSID_MODULE],[test "x$tlpkgs_mrsid_status" == xyes])

# ------------------------------------------------------------
# Generate outputs

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
