EXTRA_DIST = \
	src/TeTestsBase.hpp \
	resources/tie_points_set1.csv
	
check_PROGRAMS =

#########################################################################
# decoders test

check_PROGRAMS += ../bin/terralib_unittest_Decoders

___bin_terralib_unittest_Decoders_SOURCES = \
	src/Decoders_test.cpp 

#########################################################################
# geometric transformations test

check_PROGRAMS += ../bin/terralib_unittest_GeometricTransformations

___bin_terralib_unittest_GeometricTransformations_SOURCES = \
	src/GeometricTransformations_test.cpp
	
#########################################################################
# TeCommunicator test

check_PROGRAMS += ../bin/terralib_unittest_TeCommunicator

___bin_terralib_unittest_TeCommunicator_SOURCES = \
	src/TeCommunicator_test.cpp
	
#########################################################################
# Mapped memory test

check_PROGRAMS += ../bin/terralib_unittest_TeMappedMemory

___bin_terralib_unittest_TeMappedMemory_SOURCES = \
	src/TeMappedMemory_test.cpp	
	
#########################################################################
# TeMultiContainer test

check_PROGRAMS += ../bin/terralib_unittest_TeMultiContainer

___bin_terralib_unittest_TeMultiContainer_SOURCES = \
	src/TeMultiContainer_test.cpp
	
#########################################################################
# TeMutex test

check_PROGRAMS += ../bin/terralib_unittest_TeMutex

___bin_terralib_unittest_TeMutex_SOURCES = \
	src/TeMutex_test.cpp		
	
#########################################################################
# TePolygonSetProperties test

check_PROGRAMS += ../bin/terralib_unittest_TePolygonSetProperties

___bin_terralib_unittest_TePolygonSetProperties_SOURCES = \
	src/TePolygonSetProperties_test.cpp
	
#########################################################################
# TeRaster test

check_PROGRAMS += ../bin/terralib_unittest_TeRaster

___bin_terralib_unittest_TeRaster_SOURCES = \
	src/TeRaster_test.cpp		
	
#########################################################################
# TeRasterMemManager test

check_PROGRAMS += ../bin/terralib_unittest_TeRasterMemManager

___bin_terralib_unittest_TeRasterMemManager_SOURCES = \
	src/TeRasterMemManager_test.cpp		
	
#########################################################################
# TeSemaphore test

check_PROGRAMS += ../bin/terralib_unittest_TeSemaphore

___bin_terralib_unittest_TeSemaphore_SOURCES = \
	src/TeSemaphore_test.cpp	
	
#########################################################################
# TeSharedPtr test

check_PROGRAMS += ../bin/terralib_unittest_TeSharedPtr

___bin_terralib_unittest_TeSharedPtr_SOURCES = \
	src/TeSharedPtr_test.cpp	
	
#########################################################################
# TeThreadFunctor test

check_PROGRAMS += ../bin/terralib_unittest_TeThreadFunctor

___bin_terralib_unittest_TeThreadFunctor_SOURCES = \
	src/TeThreadFunctor_test.cpp		
	
#########################################################################
# TeThreadJobsManager test

check_PROGRAMS += ../bin/terralib_unittest_TeThreadJobsManager

___bin_terralib_unittest_TeThreadJobsManager_SOURCES = \
	src/TeThreadJobsManager_test.cpp		
	
#########################################################################
# TeThreadSignal test

check_PROGRAMS += ../bin/terralib_unittest_TeThreadSignal

___bin_terralib_unittest_TeThreadSignal_SOURCES = \
	src/TeThreadSignal_test.cpp			
	
#########################################################################
# Adding all build binaries to test queue
	
TESTS = $(check_PROGRAMS)	
