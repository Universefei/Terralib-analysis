# TerraView Source installer package code

cmake_minimum_required(VERSION 2.8)

#set ( PKG_NAME "TerraView" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "TerraLib / TerraView source instalation." )

set ( CPACK_PACKAGE_NAME "terraview-src" )
set ( CPACK_PACKAGE_VENDOR "www.dpi.inpe.br" )
set ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README" )
set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/Copyright.txt" )
set ( CPACK_PACKAGE_VERSION "4.2.0" )
set ( CPACK_PACKAGE_VERSION_MAJOR "4" )
set ( CPACK_PACKAGE_VERSION_MINOR "2" )
set ( CPACK_PACKAGE_VERSION_PATCH "0" )
set ( CPACK_PACKAGE_INSTALL_DIRECTORY "terraview-src" )
set ( CPACK_PACKAGE_FILE_NAME "terraview-src-${CPACK_PACKAGE_VERSION}" )

set ( CPACK_COMPONENTS_ALL HEADER SOURCE DOCUMENTATION TVHEADER TVSOURCE TVDOC )

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\terralib.bmp")
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\terralib_install_icon.ico")
  SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\\\terralib_uninstall_icon.ico")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\terraView.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PKG_NAME}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.dpi.inpe.br")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.funcate.org.br")
#  SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/terraView")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

#SET(CPACK_PACKAGE_EXECUTABLES "terraView" "terraView")
