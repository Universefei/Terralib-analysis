# This script builds the ado library
#
# To use it, you should adjust ROOT 
# change ROOT to the directory where libspl source is located

# Export variables
# ADO_INST_DIRS - used from instalation tree
# ADO_DIRS - used from build tree
# ADO_LIB - library name

cmake_minimum_required(VERSION 2.8)


SET(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/terralib/drivers/ado") # change this if ado is somewhere else
SET(SRCDIR ${ROOT}) # this is the directory where ado sources will be located
SET(INCLDIR ${ROOT}) # this is the directory where ado header files will be located

SET (PROJ_NAME "terralib_ado")
project( ${PROJ_NAME} ) 
SET( LIB_NAME ${PROJ_NAME} ) 
SET ( LIBS ${LIB_NAME} )
remove_definitions ( /MP )

#Ado
#Have you installed Acces? please update the following line
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)
		
	set ( _DIRS
		.
	)  

	# Instalation tree directories
	appFPrefix( "terralib/drivers/ado" "${_DIRS}" ADO_INST_DIRS )
	list ( APPEND TE_INST_DIRS "${ADO_INST_DIRS}" "${INSTALL_BIN_DIR}")

	# Build tree directories
	appFPrefix( "${INCLDIR}" "${_DIRS}" ADO_DIRS )
	list ( APPEND TE_DIRS "${ADO_DIRS}" "${ADO_INC_DIRS}" )

	getFfiles ( "${INCLDIR}" "${_DIRS}" ado_h_files true "" )
	getFfiles ( "${SRCDIR}" "${_DIRS}" ado_srcs_files false "" )
	
	list ( APPEND SRCS "${ado_h_files}" "${ado_srcs_files}" )
	
	add_definitions(-DTLADO_AS_DLL)

	
	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT MINGW)

include_directories ( "${ADO_DIRS}" ${TE_DIRS} $ENV{T4_DEPS_DIR}/bin/win32-vs2010 )

add_library ( ${LIB_NAME} SHARED ${SRCS} )
target_link_libraries( ${LIB_NAME} terralib )

# Export variables
set ( ADO_DIRS ${ADO_DIRS} PARENT_SCOPE )
set ( ADO_INST_DIRS ${ADO_INST_DIRS} PARENT_SCOPE )
set ( ADO_LIB ${LIB_NAME} PARENT_SCOPE )

install(
	TARGETS ${PROJ_NAME}
	EXPORT teLibraryDepends
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT BINARIES
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT LIBRARIES
)

