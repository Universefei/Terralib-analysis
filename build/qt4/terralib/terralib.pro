TARGET  = terralib
CONFIG += shared
include (../config.pri)
# Path definitions
# ----------------------------------------------------------
TESRCPATH = $${TERRALIBPATH}/src/terralib
TEKERNELPATH = $${TESRCPATH}/kernel
contains(QMAKE_HOST.arch, x86_64):ARCH=win64
else:ARCH=win32
TEDEP = $${TERRALIBPATH}/dependencies/$${ARCH}/zlib
# ----------------------------------------------------------

DEFINES += _USRDLL TL_AS_DLL
win32 {
	win32-g++:LIBS += -L$${TEDEP}/lib -lzlibstat
    else:QMAKE_LIBDIR += $${TEDEP}/lib -lzlibdll
}
unix:LIBS += -lpthread -lz
LIBS += -ljpeg -lterralibtiff
INCLUDEPATH	= $${TEKERNELPATH} \
        $${TESRCPATH}/functions \
        $${TESRCPATH}/utils \
	$${TERRALIBPATH}/src/tiff \
	$${TERRALIBPATH}/src/zlib \
        $${TESRCPATH}/drivers/libjpeg \
	$${TERRALIBPATH}/src/libjpeg
HEADERS	+= $${TEKERNELPATH}/checkvec.h \
        $${TEKERNELPATH}/dynpq.h \
        $${TEKERNELPATH}/Gra_algo.h \
        $${TEKERNELPATH}/graph.h \
        $${TEKERNELPATH}/TeAbstractFactory.h \
        $${TEKERNELPATH}/TeAbstractTheme.h \
        $${TEKERNELPATH}/TeAffineGT.h \
        $${TEKERNELPATH}/TeAffineGTFactory.h \
        $${TEKERNELPATH}/TeAgnostic.h \
        $${TEKERNELPATH}/TeAsciiFile.h \
        $${TEKERNELPATH}/TeAttribute.h \
        $${TEKERNELPATH}/TeBox.h \
        $${TEKERNELPATH}/TeBufferRegion.h \
        $${TEKERNELPATH}/TeComposite.h \
        $${TEKERNELPATH}/TeCommunicator.h \
        $${TEKERNELPATH}/TeComputeAttributeStrategies.h \
		$${TEKERNELPATH}/TeConnection.h \
		$${TEKERNELPATH}/TeConnectionPool.h \
        $${TEKERNELPATH}/TeCoord2D.h \
        $${TEKERNELPATH}/TeCounted.h \
        $${TEKERNELPATH}/TeDatabase.h \
        $${TEKERNELPATH}/TeDatabaseFactory.h \
        $${TEKERNELPATH}/TeDataTypes.h \
        $${TEKERNELPATH}/TeDatum.h \
        $${TEKERNELPATH}/TeDBConnectionsPool.h \
        $${TEKERNELPATH}/TeDecoder.h \
        $${TEKERNELPATH}/TeDecoderASCIIGrid.h \
        $${TEKERNELPATH}/TeDecoderDatabase.h \
        $${TEKERNELPATH}/TeDecoderFile.h \
        $${TEKERNELPATH}/TeDecoderJPEG.h \
        $${TEKERNELPATH}/TeDecoderMemory.h \
        $${TEKERNELPATH}/TeDecoderMemoryMap.h \
        $${TEKERNELPATH}/TeDecoderSPR.h \
        $${TEKERNELPATH}/TeDecoderTIFF.h \
        $${TEKERNELPATH}/TeDecoderVirtualMemory.h \
        $${TEKERNELPATH}/TeDecoderSmartMem.h \
        $${TEKERNELPATH}/TeDefines.h \
        $${TEKERNELPATH}/TeErrorLog.h \
        $${TEKERNELPATH}/TeErrorMessage.h \
        $${TEKERNELPATH}/TeException.h \
        $${TEKERNELPATH}/TeExternalTheme.h \
        $${TEKERNELPATH}/TeFactory.h \
        $${TEKERNELPATH}/TeFragmentation.h \
        $${TEKERNELPATH}/TeFileTheme.h \
        $${TEKERNELPATH}/TeGeneralizedProxMatrix.h \
        $${TEKERNELPATH}/TeGDriverFactory.h \
        $${TEKERNELPATH}/TeGeoDataDriver.h \
        $${TEKERNELPATH}/TeGeometricTransformation.h \
        $${TEKERNELPATH}/TeGeometry.h \
        $${TEKERNELPATH}/TeGeometryAlgorithms.h \
        $${TEKERNELPATH}/TeGTParams.h \
        $${TEKERNELPATH}/TeGTFactory.h \
        $${TEKERNELPATH}/TeGroupingAlgorithms.h \
        $${TEKERNELPATH}/TeImportRaster.h \
        $${TEKERNELPATH}/TeIntersector.h \
        $${TEKERNELPATH}/TeLayer.h \
        $${TEKERNELPATH}/TeLegendEntry.h \
        $${TEKERNELPATH}/TeMatrix.h \
        $${TEKERNELPATH}/TeMeasure.h \
        $${TEKERNELPATH}/TeMetaModelCache.h \
        $${TEKERNELPATH}/TeMultiGeometry.h \
        $${TEKERNELPATH}/TeNeighbours.h \
        $${TEKERNELPATH}/TeNetwork.h \
        $${TEKERNELPATH}/TeOverlay.h \
        $${TEKERNELPATH}/TeOverlayUtils.h \
        $${TEKERNELPATH}/TePrecision.h \
        $${TEKERNELPATH}/TeProgress.h \
        $${TEKERNELPATH}/TeProgressBase.h \
        $${TEKERNELPATH}/TeProject.h \
        $${TEKERNELPATH}/TeProjection.h \
        $${TEKERNELPATH}/TeProjectiveGT.h \
        $${TEKERNELPATH}/TeProjectiveGTFactory.h \
        $${TEKERNELPATH}/TePrototype.h \
        $${TEKERNELPATH}/TeProxMatrixConstructionStrategy.h \
        $${TEKERNELPATH}/TeProxMatrixImplementation.h \
        $${TEKERNELPATH}/TeProxMatrixSlicingStrategy.h \
        $${TEKERNELPATH}/TeProxMatrixWeightsStrategy.h \
        $${TEKERNELPATH}/TeQuerier.h\
        $${TEKERNELPATH}/TeQuerierDB.h\
        $${TEKERNELPATH}/TeQuerierDBStr1.h\
        $${TEKERNELPATH}/TeQuerierDBStr2.h\
        $${TEKERNELPATH}/TeQuerierDBStr3.h \
        $${TEKERNELPATH}/TeQuerierImpl.h\
        $${TEKERNELPATH}/TeQuerierParams.h\
        $${TEKERNELPATH}/TeRaster.h \
        $${TEKERNELPATH}/TeRasterParams.h \
        $${TEKERNELPATH}/TeRasterRemap.h \
        $${TEKERNELPATH}/TeRasterTransform.h \
        $${TEKERNELPATH}/TeRedBlackTree.h \
        $${TEKERNELPATH}/TeRepresentation.h \
        $${TEKERNELPATH}/TeSharedPtr.h \
        $${TEKERNELPATH}/TeSingleton.h \
        $${TEKERNELPATH}/TeSlice.h \
        $${TEKERNELPATH}/TeSpatialOperations.h \
        $${TEKERNELPATH}/TeStatistics.h \
        $${TEKERNELPATH}/TeStdFile.h \
        $${TEKERNELPATH}/TeSTEFunctionsDB.h \
        $${TEKERNELPATH}/TeBaseSTInstance.h \
        $${TEKERNELPATH}/TeBaseSTInstanceSet.h \
        $${TEKERNELPATH}/TeRTree.h \
        $${TEKERNELPATH}/TeSTElementSet.h \
        $${TEKERNELPATH}/TeSTEvent.h \
        $${TEKERNELPATH}/TeSTInstance.h\
        $${TEKERNELPATH}/TeTable.h \
        $${TEKERNELPATH}/TeTemporalSeries.h \
        $${TEKERNELPATH}/TeTheme.h \
        $${TEKERNELPATH}/TeTime.h \
        $${TEKERNELPATH}/TeTimeInterval.h \
        $${TEKERNELPATH}/TeTin.h \
        $${TEKERNELPATH}/TeUtils.h \
        $${TEKERNELPATH}/TeVectorRemap.h \
        $${TEKERNELPATH}/TeView.h \
        $${TEKERNELPATH}/TeViewNode.h \
        $${TEKERNELPATH}/TeVisual.h \
        $${TEKERNELPATH}/yyTemporal.h \
        $${TEKERNELPATH}/TeRasterMemManager.h \
        $${TEKERNELPATH}/TeMutex.h \
        $${TEKERNELPATH}/TeThreadParameters.h \
        $${TEKERNELPATH}/TeThread.h \
        $${TEKERNELPATH}/TeThreadFunctor.h \
        $${TEKERNELPATH}/TeThreadSignal.h \
        $${TEKERNELPATH}/TeDatabaseFactoryParams.h \
        $${TEKERNELPATH}/TeMappedMemory.h \
        $${TESRCPATH}/drivers/libjpeg/TeLibJpegWrapper.h \
        $${TESRCPATH}/drivers/libjpeg/jmemdstsrc.h \
        $${TEKERNELPATH}/TeMultiContainer.h \
        $${TEKERNELPATH}/TeStdIOProgress.h \
        $${TEKERNELPATH}/TeThreadJobsManager.h \
        $${TEKERNELPATH}/TeThreadJob.h \
        $${TEKERNELPATH}/TeFunctionCallThreadJob.h \
        $${TEKERNELPATH}/TeTempFilesRemover.h \
        $${TEKERNELPATH}/TeSemaphore.h \
        $${TEKERNELPATH}/TePolygonSetProperties.h \
        $${TEKERNELPATH}/Te2ndDegPolinomialGT.h \
        $${TEKERNELPATH}/Te2ndDegPolinomialGTFactory.h \
        $${TEKERNELPATH}/TeDatabaseIndex.h
SOURCES	+= $${TEKERNELPATH}/lexTemporal.cpp \
    $${TEKERNELPATH}/TeAbstractTheme.cpp \
    $${TEKERNELPATH}/TeAffineGT.cpp \
    $${TEKERNELPATH}/TeAffineGTFactory.cpp \
    $${TEKERNELPATH}/TeAgnostic.cpp \
    $${TEKERNELPATH}/TeAsciiFile.cpp \
    $${TEKERNELPATH}/TeBox.cpp \
    $${TEKERNELPATH}/TeBufferRegion.cpp \
    $${TEKERNELPATH}/TeCentroid.cpp \
    $${TEKERNELPATH}/TeComputeAttributeStrategies.cpp \
	$${TEKERNELPATH}/TeConnection.cpp \
	$${TEKERNELPATH}/TeConnectionPool.cpp \
    $${TEKERNELPATH}/TeDatabase.cpp \
    $${TEKERNELPATH}/TeDatabaseFactory.cpp \
    $${TEKERNELPATH}/TeDatum.cpp \
    $${TEKERNELPATH}/TeDBConnectionsPool.cpp \
    $${TEKERNELPATH}/TeDecoderASCIIGrid.cpp \
    $${TEKERNELPATH}/TeDecoderDatabase.cpp \
    $${TEKERNELPATH}/TeDecoderFile.cpp \
    $${TEKERNELPATH}/TeDecoderJPEG.cpp \
    $${TEKERNELPATH}/TeDecoderMemory.cpp \
    $${TEKERNELPATH}/TeDecoderMemoryMap.cpp \
    $${TEKERNELPATH}/TeDecoderSPR.cpp \
    $${TEKERNELPATH}/TeDecoderTIFF.cpp \
    $${TEKERNELPATH}/TeDecoderVirtualMemory.cpp \
    $${TEKERNELPATH}/TeDecoderSmartMem.cpp \
    $${TEKERNELPATH}/TeErrorLog.cpp \
    $${TEKERNELPATH}/TeErrorMessage.cpp \
    $${TEKERNELPATH}/TeException.cpp \
    $${TEKERNELPATH}/TeExternalTheme.cpp \
    $${TEKERNELPATH}/TeFragmentation.cpp \
    $${TEKERNELPATH}/TeFileTheme.cpp \
    $${TEKERNELPATH}/TeGDriverFactory.cpp \
    $${TEKERNELPATH}/TeGeometricTransformation.cpp \
    $${TEKERNELPATH}/TeGeometry.cpp \
    $${TEKERNELPATH}/TeGeometryAlgorithms.cpp \
    $${TEKERNELPATH}/TeGTParams.cpp \
    $${TEKERNELPATH}/TeGTFactory.cpp \
    $${TEKERNELPATH}/TeGroupingAlgorithms.cpp \
    $${TEKERNELPATH}/TeImportRaster.cpp \
    $${TEKERNELPATH}/TeInitRasterDecoders.cpp \
    $${TEKERNELPATH}/TeIntersector.cpp \
    $${TEKERNELPATH}/TeLayer.cpp \
    $${TEKERNELPATH}/TeLegendEntry.cpp \
    $${TEKERNELPATH}/TeMatrix.cpp \
    $${TEKERNELPATH}/TeMetaModelCache.cpp \
    $${TEKERNELPATH}/TeMultiGeometry.cpp \
    $${TEKERNELPATH}/TeNeighbours.cpp \
    $${TEKERNELPATH}/TeNetwork.cpp \
    $${TEKERNELPATH}/TeOverlay.cpp \
    $${TEKERNELPATH}/TeOverlayUtils.cpp \
    $${TEKERNELPATH}/TeProgress.cpp \
    $${TEKERNELPATH}/TeProject.cpp \
    $${TEKERNELPATH}/TeProjection.cpp \
    $${TEKERNELPATH}/TeProjectiveGT.cpp \
    $${TEKERNELPATH}/TeProjectiveGTFactory.cpp \
    $${TEKERNELPATH}/TeProxMatrixConstructionStrategy.cpp \
    $${TEKERNELPATH}/TeProxMatrixImplementation.cpp \
    $${TEKERNELPATH}/TeProxMatrixStrategies.cpp \
    $${TEKERNELPATH}/TeQuerier.cpp \
    $${TEKERNELPATH}/TeQuerierDB.cpp \
    $${TEKERNELPATH}/TeQuerierDBStr1.cpp \
    $${TEKERNELPATH}/TeQuerierDBStr2.cpp \
    $${TEKERNELPATH}/TeQuerierDBStr3.cpp \
    $${TEKERNELPATH}/TeQuerierImpl.cpp \
    $${TEKERNELPATH}/TeQuerierParams.cpp \
    $${TEKERNELPATH}/TeRaster.cpp \
    $${TEKERNELPATH}/TeRasterParams.cpp \
    $${TEKERNELPATH}/TeRasterRemap.cpp \
    $${TEKERNELPATH}/TeRasterTransform.cpp \
    $${TEKERNELPATH}/TeRepresentation.cpp \
    $${TEKERNELPATH}/TeSpatialOperations.cpp \
    $${TEKERNELPATH}/TeStdFile.cpp \
    $${TEKERNELPATH}/TeSTElementSet.cpp \
    $${TEKERNELPATH}/TeSTEvent.cpp \
    $${TEKERNELPATH}/TeSTInstance.cpp \
    $${TEKERNELPATH}/TeTable.cpp \
    $${TEKERNELPATH}/TeTemporalSeries.cpp \
    $${TEKERNELPATH}/TeTheme.cpp \
    $${TEKERNELPATH}/TeTime.cpp \
    $${TEKERNELPATH}/TeTimeInterval.cpp \
    $${TEKERNELPATH}/TeTin.cpp \
    $${TEKERNELPATH}/TeUtils.cpp \
    $${TEKERNELPATH}/TeVectorRemap.cpp \
    $${TEKERNELPATH}/TeViewNode.cpp \
    $${TEKERNELPATH}/TeVisual.cpp \
    $${TEKERNELPATH}/yyTemporal.cpp \
    $${TEKERNELPATH}/TeDatabaseFactoryParams.cpp \
    $${TEKERNELPATH}/TeMappedMemory.cpp \
    $${TEKERNELPATH}/TeRasterMemManager.cpp \
    $${TEKERNELPATH}/TeMutex.cpp \
    $${TEKERNELPATH}/TeThread.cpp \
    $${TEKERNELPATH}/TeThreadFunctor.cpp \
    $${TEKERNELPATH}/TeThreadSignal.cpp \
    $${TESRCPATH}/drivers/libjpeg/TeLibJpegWrapper.cpp \
    $${TESRCPATH}/drivers/libjpeg/jmemdst.c \
    $${TESRCPATH}/drivers/libjpeg/jmemsrc.c \
    $${TEKERNELPATH}/TeStdIOProgress.cpp \
    $${TEKERNELPATH}/TeThreadJobsManager.cpp \
    $${TEKERNELPATH}/TeThreadJob.cpp \
    $${TEKERNELPATH}/TeFunctionCallThreadJob.cpp \
    $${TEKERNELPATH}/TeTempFilesRemover.cpp \
    $${TEKERNELPATH}/TeSemaphore.cpp \
    $${TEKERNELPATH}/TePolygonSetProperties.cpp \
    $${TEKERNELPATH}/Te2ndDegPolinomialGT.cpp \
    $${TEKERNELPATH}/Te2ndDegPolinomialGTFactory.cpp \
    $${TEKERNELPATH}/TeDatabaseIndex.cpp

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    include.path = $${DEPLOY_DIR}/include/kernel
    include.files = $${HEADERS}

    INSTALLS += include
}

win32 {
    bin.path = $${DEPLOY_DIR}/bin
    bin.files = $${TEDEP}/bin/*.dll

    INSTALLS += bin
}

prj.files = ./br.org.terralib.pro ./extension.xml
prj.path = $${DEPLOY_DIR}

INSTALLS += prj      

DEPENDPATH += $${INCLUDEPATH}
