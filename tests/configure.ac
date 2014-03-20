m4_include(../m4/tlglobals.m4)

AC_INIT([terralib_tests],TL_VERSION_MACRO)
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
m4_include(../m4/tllink.m4)

# ------------------------------------------------------------
# Global unit tests defines

AC_DEFINE_UNQUOTED(
  [TE_DATA_UNITTEST_LOCALE],
  ["$srcdir/resources/"],
  [This flag tells where is the base folder for the data used in tests]
)

AC_DEFINE_UNQUOTED(
  [TE_DATA_UNITTEST_BIN],
  ["./../bin/"],
  [This flag tells where is the base folder for the binaries used/generated in tests]
)

# ------------------------------------------------------------
# Generate outputs

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
