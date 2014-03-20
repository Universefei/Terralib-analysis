# Cortex installer package code

cmake_minimum_required(VERSION 2.8)

set ( PROJ_NAME "br.org.terralib" )

set (INSTALL_INCLUDE_DIR "include")
set (INSTALL_LIB_DIR "lib")
set (INSTALL_BIN_DIR "bin")

project ( ${PROJ_NAME} )

set ( TERRALIB_PRO ${CMAKE_CURRENT_BINARY_DIR}/br.org.terralib.pro )
set ( EXTENSION_XML ${CMAKE_CURRENT_BINARY_DIR}/extension.xml )

file (WRITE ${EXTENSION_XML} 
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<extension xmlns=\"urn:br:mil:eb:cortex\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:br:mil:eb:cortex ../platform.xsd\">\n"
	"	<info id=\"br.org.terralib\" version=\"${CPACK_PACKAGE_VERSION}\">\n"
	"		<title>\n"
	"			TerraLib\n"
	"		</title>\n"

	"		<description>\n"
	"			TerraLib is a c++ software library for manage spatial data. It supports Firebird, MySQL, Oracle and PostgreSQL databases.\n"
	"		</description>\n"
	"	</info>\n" )

if (WIN32 AND NOT UNIX)

	set ( TE_CORTEX_DEPENDS "libtiff-3.dll" "libtiffxx-3.dll" "zlib1.dll" "libjpeg-8.dll" )
	
	if ( TE_PGRES_ENABLED )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libeay32</fileName>\n" )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libiconv-2</fileName>\n" )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libintl-2</fileName>\n" )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">ssleay32</fileName>\n" )		
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libpq</fileName>\n" )
		
		list ( APPEND TE_CORTEX_DEPENDS "libeay32.dll" "libiconv-2.dll" "libintl-2.dll" "ssleay32.dll" "libpq.dll")
	endif()
	
	if ( TE_MYSQL_ENABLED )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libmysql</fileName>\n" )
		list ( APPEND TE_CORTEX_DEPENDS "libmysql.dll" )
	endif()
	if ( TE_ORACLE_ENABLED )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">oci</fileName>\n" )
		list ( APPEND TE_CORTEX_DEPENDS "oci.dll" )
	endif()
	if ( TE_FBIRD_ENABLED )
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">fbclient</fileName>\n" )
		list ( APPEND TE_CORTEX_DEPENDS "fbclient.dll" )
	endif()
	
	
	file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">zlib1</fileName>\n" )
	file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libjpeg-8</fileName>\n" ) 
	file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libtiff-3</fileName>\n" )
	file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"win32\">libtiffxx-3</fileName>\n" )

else ()
	if(TE_FBIRD_ENABLED)
		file ( APPEND ${EXTENSION_XML}  "	<fileName os=\"unix\">${FBIRD_LIBRARY}</fileName>\n" )
	endif()
endif()

file ( APPEND ${EXTENSION_XML}  "	<fileName>libterralib_shp</fileName>\n" )
file ( APPEND ${EXTENSION_XML}  "	<fileName>libterralib</fileName>\n" )
file ( APPEND ${EXTENSION_XML}  "</extension>" )

# Cortex installer package code

set ( TERRALIB_PRO ${CMAKE_CURRENT_BINARY_DIR}/br.org.terralib.pro )
set ( EXTENSION_XML ${CMAKE_CURRENT_BINARY_DIR}/extension.xml )



file (WRITE ${TERRALIB_PRO} 
	"#------------------------------------------------\n"
	"# General\n"
	"#------------------------------------------------\n"
	"EXTENSION_ID = br.org.terralib\n"
	"EXTENSION_VERSION = ${CPACK_PACKAGE_VERSION}\n"
	"include(../../runtime/extension.pri)\n"
	"#------------------------------------------------\n"
	"# Specific Settings\n"
	"#------------------------------------------------\n"
	"QT -= core gui\n"
	"QMAKE_POST_LINK = $$collectGarbage(org)\n\n"
)

