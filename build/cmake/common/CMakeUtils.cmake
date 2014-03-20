# Utilities to use with cmake.
# (c) Frederico A. Bedê, 2011
# ----------------------------------------------------------------------------

# Used to automatically find terralib dependencies
# Define T4_DEPS_DIR Environment variable - it must point the root directory of terralib dependencies.
# To make the automatic find to work, define the variable in the beggining of your session.

MACRO (getTePath T4_DPS_INC_DIR T4_DPS_LIB_DIR )

  IF (WIN32)
    set ( ${T4_DPS_INC_DIR} "$ENV{T4_DEPS_DIR}/include" )

    IF (MINGW)
      set ( ${T4_DPS_LIB_DIR} "$ENV{T4_DEPS_DIR}/bin/win32-g++" )
    ELSE (MINGW)
      set ( ${T4_DPS_LIB_DIR} "$ENV{T4_DEPS_DIR}/bin/win32-vs2010" )
    ENDIF (MINGW)
  ELSE (WIN32)
    set (${T4_DPS_LIB_DIR} /usr/lib /usr/local/lib /opt/local/lib)
  ENDIF (WIN32)
  
ENDMACRO ( getTePath T4_DPS_INC_DIR T4_DPS_LIB_DIR )

# ----------------------------------------------------------
# Utils functions
# ----------------------------------------------------------

# Macro MakeFilter
#
# brief Create filter for a set of files, to use in visual studio ide.
#
# This macro creates a filter in Visual Studio IDE. It's used for organization pourposes. To use the macro call it from somewhere in your cmake code, 
# like presented below:
#
# Example 1
# ... 
# set (h_files include/test1.h include/test2.h ...)
# set (src_files src/test1.cpp src/test2.cpp ...)
#
# MakeFilter ( test ${h_files} ) -> will create filter "test" in Header Files default filter.
# MakeFilter ( test ${src_files} ) -> will create filter "test" in Source Files default filter.
# ...
#
# param[input] FILTER_PATH Filter to be used. (Can use "/" to add subfilters.)
# param[input] FILES_LST List of files contained in the generated filter.
# param[input] H_FLAG Header flag. If ON the files are headers otherwise sources. We can then update the correct default filter.
# 
MACRO ( MakeFilter FILTER_PATH FILES_LST H_FLAG)

	set ( f_list ${FILES_LST} )
	set ( n_files 0 )
	
	list ( LENGTH f_list n_files )
	
	if ( ${n_files} )
		if ( ${H_FLAG} )
			set ( fh_path "Header Files/${FILTER_PATH}" )
		else ()
			set ( fh_path "Source Files/${FILTER_PATH}" )
		endif ()
		STRING ( REPLACE "/" "\\" fh_path ${fh_path} )
		source_group( ${fh_path} FILES ${FILES_LST} )
	endif()
	
ENDMACRO()

# Macro getFfiles
#
# brief Return the files in a folder with the required extension.
#
# param[input] fprefix Prefix to be append.
# param[input] folders List of folders.
# param[output] sfiles list of files in the folder.
# param[input] h_flag Headers flag. True if you want header files (*.h*), otherwise the required files are source(*.cpp).
# param[input] f_base Base filter. (Use in visual studio)
# 
MACRO ( getFiles folder extension sources )
	set ( ${sources} "" )
	FILE ( GLOB new_src ${folder}/${extension} )
	list ( APPEND ${sources} ${new_src} )
ENDMACRO ()

# Macro getFfiles
#
# brief Return the (header or source) files into a folder and creating visual studio filters. (Customized function)
#
# param[input] fprefix Prefix to be append.
# param[input] folders List of folders.
# param[output] sfiles list of files in the folder.
# param[input] h_flag Headers flag. True if you want header files (*.h*), otherwise the required files are source(*.cpp).
# param[input] f_base Base filter. (Use in visual studio)
# 
MACRO ( getFfiles fprefix folders sfiles h_flag f_base )
	set ( ${sfiles} "" )

	foreach ( folder ${folders} )
		
		set ( pfolder "${fprefix}/${folder}" )
		
		if( ${h_flag} )
			set ( ext "*.h*" )
		else()
			set ( ext "*.c*" )
		endif()
		
		set ( auxfiles "")
		getFiles ( "${pfolder}" ${ext} aux_files )

		list ( APPEND ${sfiles} ${aux_files} )
		
		if ( NOT ${f_base} EQUAL "" )
			set ( folder "${f_base}/${folder}" )
		endif()
		
		if( ("${folder}" STREQUAL "${f_base}/.") OR ("${folder}" STREQUAL ".") )
			MakeFilter ( "${f_base}" "${aux_files}" ${h_flag} )
		else ()
			MakeFilter ( "${folder}" "${aux_files}" ${h_flag} )
		endif ()
		
	endforeach ()
ENDMACRO ()

# Macro appFPrefix
#
# brief Appends a prefix to a set of directories.
#
# param[input] prefix Prefix to be append.
# param[input] folders List of folders to use the prefix
# param[output] result Result union "${prefix}/${folder}" for each folder in the set.
# 
MACRO ( appFPrefix prefix folders result )
	set ( ${result} "" )
	
	foreach(folder ${folders})
		if( ${folder} STREQUAL "." )
			list ( APPEND ${result} "${prefix}" )
		else ()
			list ( APPEND ${result} "${prefix}/${folder}" )
		endif ()
	endforeach()
ENDMACRO ()

# Macro findTranslation
#
# brief Find tranlation executables.


MACRO ( findTranslation )

	
	# find lrelease binary
	if(NOT QT_LRELEASE_EXECUTABLE)
	find_program(QT_LRELEASE_EXECUTABLE
		NAMES lrelease
		PATHS $ENV{QTDIR}/bin
		NO_DEFAULT_PATH
	)
	endif(NOT QT_LRELEASE_EXECUTABLE)

	# find lupdate binary
	if(NOT QT_LUPDATE_EXECUTABLE)
	find_program(QT_LUPDATE_EXECUTABLE
		NAMES lupdate
		PATHS $ENV{QTDIR}/bin
		NO_DEFAULT_PATH
	)
	endif(NOT QT_LUPDATE_EXECUTABLE)

	
ENDMACRO ()


# Macro generateTS
#
# brief Generate or update ts files.
#
# param[input] ts Ts file.
# param[input] filename Name of .pro file
# 
MACRO ( generateTS proFile  )
	findTranslation()	
	execute_process( COMMAND ${QT_LUPDATE_EXECUTABLE} ${proFile} )
ENDMACRO ()


# Macro generateQM
#
# brief Generate QM files.
#
# param[input] ts TS file.
# param[input] qm QM file.
# 
MACRO ( generateQM ts qm  )
	execute_process( COMMAND ${QT_LRELEASE_EXECUTABLE} "${ts}" -qm "${qm}" )
ENDMACRO ()

# Macro generatePRO
#
# brief Generate .pro file.
#
# param[input] fileName Name of .pro file.
# param[input] headers Headers files.
# param[input] sources Sources files.
# param[input] uifiles UI files.
# param[input] tsList List of TS files.
# 

MACRO ( generatePRO fileName headers sources uifiles tsList )

	foreach( P_SRC ${sources})
		set ( PRO_SRCS "${PRO_SRCS} ${P_SRC}" )
	endforeach()
	foreach( P_HEADERS ${headers})
		set ( PRO_HEADERS "${PRO_HEADERS} ${P_HEADERS}" )
	endforeach()
	foreach( P_UIFILES ${uifiles})
		set ( PRO_UIFILES "${PRO_UIFILES} ${P_UIFILES}" )
	endforeach()
	file (WRITE ${fileName}
		"SOURCES += ${PRO_SRCS} \n"
		"HEADERS += ${PRO_HEADERS} \n"
		"FORMS += ${PRO_UIFILES} \n"
		"TRANSLATIONS = ${tsList} \n" )
			
ENDMACRO ()


# Macro generatePRO
#
# brief Generate .pro file.
#
# param[input] projName Project's name.
# param[input] fileName Name of .pro file.
# param[input] img_dir Image's dir.
# param[input] img_file Name of image file.
# 

MACRO ( generateImgCol projName fileName img_dir img_file )
	
	file (GLOB imgs ${img_dir}/*.bmp ${img_dir}/*.png ${img_dir}/*.gif ${img_dir}/*.xpm)
	#erases the file
	FILE(REMOVE ${fileName} )
	FOREACH( VAR ${imgs} )
		FILE(APPEND ${fileName} "${VAR}\r")	
	ENDFOREACH(VAR)

	add_Custom_Command( 
		OUTPUT ${img_file}
		COMMAND ${QT_UIC_EXECUTABLE} -embed ${projName} -f ${fileName} -o ${img_file} 
	)	

	
			
ENDMACRO ()

MACRO ( generateBAT fileName depDirs projName binaryDir buildType)
set( DEPS "" )
set( QWT_DIR "")
if( $ENV{QTDIR} MATCHES "3.2.0") 
	set (QWT_DIR %T4_DEPS_DIR%/bin/win32-vs2010/qwt_qt-3.2.0 )
elseif($ENV{QTDIR} MATCHES "3.3.8")
	set (QWT_DIR %T4_DEPS_DIR%/bin/win32-vs2010/qwt_qt-3.3.8 )
elseif($ENV{QTDIR} MATCHES "4.7.2")
	set (QWT_DIR %T4_DEPS_DIR%/bin/win32-vs2010/qwt_qt-4.7.2 )
endif()
FOREACH( dir ${depDirs})
	set( DEPS "SET PATH=${dir}/${buildType};%PATH%\n${DEPS}" )
ENDFOREACH()
file (WRITE ${fileName}-${buildType}.bat
	"SET T4_DEPS_DIR=$ENV{T4_DEPS_DIR}\n"
	"SET QTDIR=$ENV{QTDIR}\n"
	"SET PATH=%QTDIR%/bin;%T4_DEPS_DIR%/bin/win32-vs2010;${QWT_DIR};%PATH%\n"
	"${DEPS}"
	"${binaryDir}/${projName}.sln" )
	
			
ENDMACRO ()

# Macro getLibraryForOS
#
# brief Generate the aproprieate name for the binary file according to the Operating System.
# 
# Example: 
# 		set (result "the result must be started before the call to getLibraryForOS macro") <- this variable MUST be named as "result".
#		getLibraryForOS (${result} "terralib")
#		message ($result) <-- if the OS is windows the message is "terralib.dll", otherwise "libterralib.so".		
#
# param[output] result the result of this macro
# param[input] basename the name that will be used in the result


MACRO ( getLibraryForOS result basename )

	IF( CMAKE_GENERATOR MATCHES "Visual Studio 10")
		set ( result "${basename}.dll" )
	else ()
		set ( result "lib${basename}.so" )
	endif ()
	
ENDMACRO ()



# Breaks up a string in the form n1.n2.n3 into three parts and stores
# them in major, minor, and patch.  version should be a value, not a
# variable, while major, minor and patch should be variables.
MACRO(THREE_PART_VERSION version major minor patch)
	SET(THREE_PART_VERSION_REGEX "[0-9]+\\.[0-9]+\\.[0-9]+")
	IF(${version} MATCHES ${THREE_PART_VERSION_REGEX})
		STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" ${major} "${version}")
		STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" ${minor} "${version}")
		STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" ${patch} "${version}")
	ELSE(${version} MATCHES ${THREE_PART_VERSION_REGEX})
		MESSAGE("MACRO(THREE_PART_VERSION_TO_VARS ${version} ${major} ${minor} ${patch}")
		MESSAGE(FATAL_ERROR "Problem parsing version string, I can't parse it properly.")
	ENDIF(${version} MATCHES ${THREE_PART_VERSION_REGEX})
ENDMACRO(THREE_PART_VERSION)

# Computes the realtionship between two version strings.  A version
# string is a number delineated by '.'s such as 1.3.2 and 0.99.9.1.
# You can feed version strings with different number of dot versions,
# and the shorter version number will be padded with zeros: 9.2 <
# 9.2.1 will actually compare 9.2.0 < 9.2.1.
#
# Input: a_in - value, not variable
#        b_in - value, not variable
#        result_out - variable with value:
#                         -1 : a_in <  b_in
#                          0 : a_in == b_in
#                          1 : a_in >  b_in
#
# Written by James Bigler.
MACRO(COMPARE_VERSION_STRINGS a_in b_in result_out)
  # Since SEPARATE_ARGUMENTS using ' ' as the separation token,
  # replace '.' with ' ' to allow easy tokenization of the string.
  STRING(REPLACE "." " " a ${a_in})
  STRING(REPLACE "." " " b ${b_in})
  SEPARATE_ARGUMENTS(a)
  SEPARATE_ARGUMENTS(b)

  # Check the size of each list to see if they are equal.
  LIST(LENGTH a a_length)
  LIST(LENGTH b b_length)

  # Pad the shorter list with zeros.

  # Note that range needs to be one less than the length as the for
  # loop is inclusive (silly CMake).
  IF(a_length LESS b_length)
    # a is shorter
    SET(shorter a)
    MATH(EXPR range "${b_length} - 1")
    MATH(EXPR pad_range "${b_length} - ${a_length} - 1")
  ELSE(a_length LESS b_length)
    # b is shorter
    SET(shorter b)
    MATH(EXPR range "${a_length} - 1")
    MATH(EXPR pad_range "${a_length} - ${b_length} - 1")
  ENDIF(a_length LESS b_length)

  # PAD out if we need to
  IF(NOT pad_range LESS 0)
    FOREACH(pad RANGE ${pad_range})
      # Since shorter is an alias for b, we need to get to it by by dereferencing shorter.
      LIST(APPEND ${shorter} 0)
    ENDFOREACH(pad RANGE ${pad_range})
  ENDIF(NOT pad_range LESS 0)

  SET(result 0)
  FOREACH(index RANGE ${range})
    IF(result EQUAL 0)
      # Only continue to compare things as long as they are equal
      LIST(GET a ${index} a_version)
      LIST(GET b ${index} b_version)
      # LESS
      IF(a_version LESS b_version)
        SET(result -1)
      ENDIF(a_version LESS b_version)
      # GREATER
      IF(a_version GREATER b_version)
        SET(result 1)
      ENDIF(a_version GREATER b_version)
    ENDIF(result EQUAL 0)
  ENDFOREACH(index)

  # Copy out the return result
  SET(${result_out} ${result})
ENDMACRO(COMPARE_VERSION_STRINGS)
