if TE_BUILD_QT_MODULE
include $(srcdir)/../m4/tlqtextras.am
endif

nobase_include_HEADERS = \
	terralib/kernel/TeUnixPlatformDefines.h

lib_LTLIBRARIES =

BUILT_SOURCES =

EXTRA_DIST =

CLEANFILES =

#########################################################################
# internal shapelib library	
#########################################################################

if TE_BUILD_SHAPELIB_MODULE
lib_LTLIBRARIES += ../bin/libterralib_shp.la
endif
		
___bin_libterralib_shp_la_SOURCES = \
	shapelib/shapefil.h \
	shapelib/dbfopen.c \
	shapelib/shpopen.c

#########################################################################
# spl library
#########################################################################

lib_LTLIBRARIES += ../bin/libterralib_spl.la

___bin_libterralib_spl_la_SOURCES = \
	libspl/spl.h \
	libspl/include/spl_pluginargs.h \
	libspl/include/spl_pluginserver_linux.h \
	libspl/include/spl_pluginserver_solaris.h \
	libspl/include/spl_pluginserver_win32.h \
	libspl/include/spl_threadlock.h \
	libspl/include/spl_mutex.h \
	libspl/include/spl_plugininfo.h \
	libspl/include/spl_typedefs.h \
	libspl/include/spl_platform.h \
	libspl/include/spl_pluginserver.h \
	libspl/source/spl_pluginserver.cpp \
	libspl/source/spl_pluginserver_linux.cpp \
	libspl/source/spl_pluginserver_solaris.cpp \
	libspl/source/spl_pluginserver_win32.cpp \
	libspl/source/spl_pluginargs.cpp \
	libspl/source/spl_mutex.cpp \
	libspl/source/spl_threadlock.cpp \
	libspl/source/spl_plugininfo.cpp

#########################################################################
# terralib module
#########################################################################

lib_LTLIBRARIES += ../bin/libterralib.la

nodist____bin_libterralib_la_SOURCES =
___bin_libterralib_la_SOURCES =
___bin_libterralib_la_CPPFLAGS = $(AM_CPPFLAGS)
___bin_libterralib_la_LDFLAGS = -ldl $(AM_LDFLAGS)

___bin_libterralib_la_LIBADD = ../bin/libterralib_spl.la
if TE_BUILD_SHAPELIB_MODULE
___bin_libterralib_la_LIBADD += ../bin/libterralib_shp.la
endif

#######################################
# kernel

nobase_include_HEADERS += \
	terralib/kernel/TeViewNode.h \
	terralib/kernel/TeDatabaseFactoryParams.h \
	terralib/kernel/TeThreadFunctor.h \
	terralib/kernel/TeCoverageParams.h \
	terralib/kernel/TeDecoderMemoryMap.h \
	terralib/kernel/TeRasterParams.h \
	terralib/kernel/TeFileTheme.h \
	terralib/kernel/TeView.h \
	terralib/kernel/TeOverlay.h \
	terralib/kernel/TeLayer.h \
	terralib/kernel/TeFragmentation.h \
	terralib/kernel/TeLegendEntry.h \
	terralib/kernel/TeAssertions.h \
	terralib/kernel/TeCoverageDecoderCacheLRU.h \
	terralib/kernel/TeAgnostic.h \
	terralib/kernel/TeAffineGTFactory.h \
	terralib/kernel/TeSemaphore.h \
	terralib/kernel/TeProxMatrixConstructionStrategy.h \
	terralib/kernel/Gra_algo.h \
	terralib/kernel/TeDatum.h \
	terralib/kernel/TeErrorLog.h \
	terralib/kernel/TeMultiGeometry.h \
	terralib/kernel/TeQuerierParams.h \
	terralib/kernel/TeAbstractTheme.h \
	terralib/kernel/TeProxMatrixWeightsStrategy.h \
	terralib/kernel/TeComposite.h \
	terralib/kernel/TeSingleton.h \
	terralib/kernel/TeAsciiFile.h \
	terralib/kernel/TeThreadDatatypes.h \
	terralib/kernel/TeImportRaster.h \
	terralib/kernel/TeGroupingAlgorithms.h \
	terralib/kernel/checkvec.h \
	terralib/kernel/TeGeometry.h \
	terralib/kernel/TeQuerierImpl.h \
	terralib/kernel/TeGeometryAlgorithms.h \
	terralib/kernel/TeViewTreeUtils.h \
	terralib/kernel/TeInitRasterDecoders.h \
	terralib/kernel/TeProjectiveGT.h \
	terralib/kernel/TeBlockLoader.h \
	terralib/kernel/TeCoverage.h \
	terralib/kernel/TeGeometricTransformation.h \
	terralib/kernel/TeGeneralizedProxMatrix.h \
	terralib/kernel/TeCommunicator.h \
	terralib/kernel/TeTimeInterval.h \
	terralib/kernel/TeUtils.h \
	terralib/kernel/TeCoverageDecoderDatabase.h \
	terralib/kernel/TeProxMatrixImplementation.h \
	terralib/kernel/TeRTree.h \
	terralib/kernel/TeMappedMemory.h \
	terralib/kernel/TeProjectiveGTFactory.h \
	terralib/kernel/TeCoverageUtils.h \
	terralib/kernel/TeRasterRemap.h \
	terralib/kernel/TeMultiContainer.h \
	terralib/kernel/TeFunctionCallThreadJob.h \
	terralib/kernel/TeQuerierDBStr1.h \
	terralib/kernel/TeTemporalSeries.h \
	terralib/kernel/TeBox.h \
	terralib/kernel/TeProgressBase.h \
	terralib/kernel/TeDecoderVirtualMemory.h \
	terralib/kernel/TeRasterMemManager.h \
	terralib/kernel/TeDecoderSPR.h \
	terralib/kernel/TeExternalTheme.h \
	terralib/kernel/TeThreadParameters.h \
	terralib/kernel/TeSTElementSet.h \
	terralib/kernel/TeTempFilesRemover.h \
	terralib/kernel/TeComputeAttributeStrategies.h \
	terralib/kernel/TeDecoderDatabase.h \
	terralib/kernel/TeCoverageInterpolatorNN.h \
	terralib/kernel/TeThreadJob.h \
	terralib/kernel/TeVersion.h \
	terralib/kernel/TeRasterTransform.h \
	terralib/kernel/TePrototype.h \
	terralib/kernel/TeDecoderASCIIGrid.h \
	terralib/kernel/TeDBConnectionsPool.h \
	terralib/kernel/TeSTEFunctionsDB.h \
	terralib/kernel/TeAffineGT.h \
	terralib/kernel/TeDecoderSmartMem.h \
	terralib/kernel/TePrecision.h \
	terralib/kernel/TeGeoDataDriver.h \
	terralib/kernel/TePolygonSetProperties.h \
	terralib/kernel/TeDataTypes.h \
	terralib/kernel/TeGDriverFactory.h \
	terralib/kernel/TeProject.h \
	terralib/kernel/TeTable.h \
	terralib/kernel/TeSlice.h \
	terralib/kernel/TePieBar.h \
	terralib/kernel/TeDatabaseIndex.h \
	terralib/kernel/TeSharedPtr.h \
	terralib/kernel/TeIntersector.h \
	terralib/kernel/TeProgress.h \
	terralib/kernel/TeOverlayUtils.h \
	terralib/kernel/TeStdFile.h \
	terralib/kernel/TeBufferRegion.h \
	terralib/kernel/TeDatabaseFactory.h \
	terralib/kernel/TeFactory.h \
	terralib/kernel/yyTemporal.h \
	terralib/kernel/TeDecoder.h \
	terralib/kernel/TeAttribute.h \
	terralib/kernel/TeTheme.h \
	terralib/kernel/TeGTParams.h \
	terralib/kernel/TeCoverageImportUtils.h \
	terralib/kernel/TeCoord2D.h \
	terralib/kernel/TeDecoderMemory.h \
	terralib/kernel/TeProjection.h \
	terralib/kernel/TeBaseSTInstanceSet.h \
	terralib/kernel/TeGraph.h \
	terralib/kernel/TeVisual.h \
	terralib/kernel/TeQuerierDBStr3.h \
	terralib/kernel/TeMeasure.h \
	terralib/kernel/TeViewTreeIterator.h \
	terralib/kernel/dynpq.h \
	terralib/kernel/TeTin.h \
	terralib/kernel/TeStatistics.h \
	terralib/kernel/TeGTFactory.h \
	terralib/kernel/TeVectorRemap.h \
	terralib/kernel/TeKdTree.h \
	terralib/kernel/TeCoverageDecoder.h \
	terralib/kernel/graph.h \
	terralib/kernel/TeQuerierDB.h \
	terralib/kernel/TeBaseSTInstance.h \
	terralib/kernel/TeProxMatrixSlicingStrategy.h \
	terralib/kernel/TeRepresentation.h \
	terralib/kernel/TeThreadSignal.h \
	terralib/kernel/gra_util.h \
	terralib/kernel/TeRaster.h \
	terralib/kernel/TeRedBlackTree.h \
	terralib/kernel/TeCounted.h \
	terralib/kernel/Te2ndDegPolinomialGTFactory.h \
	terralib/kernel/TeQuerierDBStr2.h \
	terralib/kernel/TeAbstractFactory.h \
	terralib/kernel/TeStdIOProgress.h \
	terralib/kernel/TeSTEvent.h \
	terralib/kernel/TeCoverageInterpolator.h \
	terralib/kernel/showseq.h \
	terralib/kernel/TeSparseMatrix.h \
	terralib/kernel/TeCoverageImport.h \
	terralib/kernel/TeQuerier.h \
	terralib/kernel/TeSpatialOperations.h \
	terralib/kernel/TeTime.h \
	terralib/kernel/TeErrorMessage.h \
	terralib/kernel/TeThread.h \
	terralib/kernel/TeGridIndex.h \
	terralib/kernel/TeConsoleErrorMessage.h \
	terralib/kernel/TeDecoderFile.h \
	terralib/kernel/Te2ndDegPolinomialGT.h \
	terralib/kernel/TeThreadJobsManager.h \
	terralib/kernel/TeSTInstance.h \
	terralib/kernel/TeMutex.h \
	terralib/kernel/TeNeighbours.h \
	terralib/kernel/TeException.h \
	terralib/kernel/TeNetwork.h \
	terralib/kernel/TeMetaModelCache.h \
	terralib/kernel/TeDefines.h \
	terralib/kernel/TeDatabase.h \
	terralib/kernel/TeMatrix.h \
	terralib/kernel/TeProfile1.h \
	terralib/kernel/TeConnection.h \
	terralib/kernel/TeConnectionPool.h \
	terralib/drivers/libjpeg/TeLibJpegWrapper.h \
	terralib/drivers/libjpeg/jmemdstsrc.h \
	terralib/kernel/TeGeoFileInfo.h

___bin_libterralib_la_SOURCES += \
	terralib/kernel/TeDecoder.cpp \
	terralib/kernel/TeDecoderMemory.cpp \
	terralib/kernel/TeGroupingAlgorithms.cpp \
	terralib/kernel/TeDecoderVirtualMemory.cpp \
	terralib/kernel/TePolygonSetProperties.cpp \
	terralib/kernel/TeRasterTransform.cpp \
	terralib/kernel/TeIntersector.cpp \
	terralib/kernel/TeGeometricTransformation.cpp \
	terralib/kernel/TeProjectiveGT.cpp \
	terralib/kernel/Te2ndDegPolinomialGTFactory.cpp \
	terralib/kernel/TeRasterParams.cpp \
	terralib/kernel/TeGTFactory.cpp \
	terralib/kernel/TeLayer.cpp \
	terralib/kernel/TeProjectiveGTFactory.cpp \
	terralib/kernel/TeThreadJobsManager.cpp \
	terralib/kernel/TeInitRasterDecoders.cpp \
	terralib/kernel/TeTempFilesRemover.cpp \
	terralib/kernel/TeDecoderFile.cpp \
	terralib/kernel/TeFileTheme.cpp \
	terralib/kernel/TeBox.cpp \
	terralib/kernel/TeAgnostic.cpp \
	terralib/kernel/Te2ndDegPolinomialGT.cpp \
	terralib/kernel/TeMutex.cpp \
	terralib/kernel/TeRasterMemManager.cpp \
	terralib/kernel/TeViewTreeUtils.cpp \
	terralib/kernel/TeProgress.cpp \
	terralib/kernel/TeExternalTheme.cpp \
	terralib/kernel/TeDatabaseFactory.cpp \
	terralib/kernel/TeProxMatrixConstructionStrategy.cpp \
	terralib/kernel/TeBufferRegion.cpp \
	terralib/kernel/TeTemporalSeries.cpp \
	terralib/kernel/TeProject.cpp \
	terralib/kernel/TeQuerierDBStr1.cpp \
	terralib/kernel/TeQuerierDBStr3.cpp \
	terralib/kernel/lexTemporal.cpp \
	terralib/kernel/TeThreadJob.cpp \
	terralib/kernel/TeDatum.cpp \
	terralib/kernel/TeVisual.cpp \
	terralib/kernel/TeStdFile.cpp \
	terralib/kernel/TeSTEvent.cpp \
	terralib/kernel/TeMatrix.cpp \
	terralib/kernel/TeTimeInterval.cpp \
	terralib/kernel/TeDecoderASCIIGrid.cpp \
	terralib/kernel/TeNeighbours.cpp \
	terralib/kernel/TeDecoderDatabase.cpp \
	terralib/kernel/TeSTElementSet.cpp \
	terralib/kernel/TeSpatialOperations.cpp \
	terralib/kernel/TeAffineGT.cpp \
	terralib/kernel/TeViewTreeIterator.cpp \
	terralib/kernel/TeSTInstance.cpp \
	terralib/kernel/TeViewNode.cpp \
	terralib/kernel/TeDatabaseIndex.cpp \
	terralib/kernel/TeGTParams.cpp \
	terralib/kernel/TeQuerierDB.cpp \
	terralib/kernel/TeSemaphore.cpp \
	terralib/kernel/TeQuerier.cpp \
	terralib/kernel/TeRaster.cpp \
	terralib/kernel/TeMetaModelCache.cpp \
	terralib/kernel/TeFragmentation.cpp \
	terralib/kernel/TeCoverageImportUtils.cpp \
	terralib/kernel/TeRasterRemap.cpp \
	terralib/kernel/TeVectorRemap.cpp \
	terralib/kernel/TeOverlay.cpp \
	terralib/kernel/TeGeometry.cpp \
	terralib/kernel/TeQuerierParams.cpp \
	terralib/kernel/TeAffineGTFactory.cpp \
	terralib/kernel/TeTheme.cpp \
	terralib/kernel/TeCentroid.cpp \
	terralib/kernel/TeDBConnectionsPool.cpp \
	terralib/kernel/TeProxMatrixImplementation.cpp \
	terralib/kernel/TeErrorMessage.cpp \
	terralib/kernel/TeDatabaseFactoryParams.cpp \
	terralib/kernel/TeConsoleErrorMessage.cpp \
	terralib/kernel/TeException.cpp \
	terralib/kernel/TeComputeAttributeStrategies.cpp \
	terralib/kernel/TeBlockLoader.cpp \
	terralib/kernel/TeLegendEntry.cpp \
	terralib/kernel/TeThread.cpp \
	terralib/kernel/TeDecoderMemoryMap.cpp \
	terralib/kernel/TeFunctionCallThreadJob.cpp \
	terralib/kernel/TeTime.cpp \
	terralib/kernel/TeThreadSignal.cpp \
	terralib/kernel/TeDatabase.cpp \
	terralib/kernel/TeErrorLog.cpp \
	terralib/kernel/TeGDriverFactory.cpp \
	terralib/kernel/TeRepresentation.cpp \
	terralib/kernel/TeUtils.cpp \
	terralib/kernel/TeAbstractTheme.cpp \
	terralib/kernel/yyTemporal.cpp \
	terralib/kernel/TeThreadFunctor.cpp \
	terralib/kernel/TeQuerierDBStr2.cpp \
	terralib/kernel/TeNetwork.cpp \
	terralib/kernel/TeAsciiFile.cpp \
	terralib/kernel/TeCoverageUtils.cpp \
	terralib/kernel/TeMultiGeometry.cpp \
	terralib/kernel/TeGeometryAlgorithms.cpp \
	terralib/kernel/TeGraph.cpp \
	terralib/kernel/TeProxMatrixStrategies.cpp \
	terralib/kernel/TeTable.cpp \
	terralib/kernel/TeDecoderSPR.cpp \
	terralib/kernel/TeStdIOProgress.cpp \
	terralib/kernel/TeOverlayUtils.cpp \
	terralib/kernel/TeTin.cpp \
	terralib/kernel/TeProjection.cpp \
	terralib/kernel/TeImportRaster.cpp \
	terralib/kernel/TeMappedMemory.cpp \
	terralib/kernel/TeQuerierImpl.cpp \
	terralib/kernel/TeDecoderSmartMem.cpp \
	terralib/kernel/TeProfile1.cpp \
	terralib/kernel/TeConnection.cpp \
	terralib/kernel/TeConnectionPool.cpp \
	terralib/kernel/TeGeoFileInfo.cpp	

#######################################
# geotiff

if TE_BUILD_GEOTIFF_MODULE
nobase_include_HEADERS += \
	terralib/kernel/TeDecoderTIFF.h
	
___bin_libterralib_la_SOURCES += \
	terralib/kernel/TeDecoderTIFF.cpp
endif

#######################################
#jpeg

if TE_BUILD_JPEG_MODULE
nobase_include_HEADERS += \
	terralib/kernel/TeDecoderJPEG.h
	
___bin_libterralib_la_SOURCES += \
	terralib/drivers/libjpeg/TeLibJpegWrapper.cpp \
	terralib/drivers/libjpeg/jmemdst.c \
	terralib/drivers/libjpeg/jmemsrc.c \
	terralib/kernel/TeDecoderJPEG.cpp
endif

#######################################
# stat 

nobase_include_HEADERS += \
	terralib/stat/heap.h \
	terralib/stat/TeBayesFunctions.h \
	terralib/stat/TeStatDataStructures.h \
	terralib/stat/filaDouble.h \
	terralib/stat/hash.h \
	terralib/stat/TeTreeMinimum.h \
	terralib/stat/erro.h \
	terralib/stat/TeSpatialStatistics.h \
	terralib/stat/TeKMeansGrouping.h \
	terralib/stat/TeSemivarModelFactory.h \
	terralib/stat/filaInt.h \
	terralib/stat/filaR.h \
	terralib/stat/TeSkaterGrafo.h \
	terralib/stat/lista.h \
	terralib/stat/TeStatDefines.h \
	terralib/stat/TeSkaterArvore.h \
	terralib/stat/TeMSVFactory.h \
	terralib/stat/TeSkaterFunctions.h \
	terralib/stat/TeKernelFunctions.h \
	terralib/stat/TeKernelParams.h

___bin_libterralib_la_SOURCES += \
	terralib/stat/TeStatDataStructures.cpp \
	terralib/stat/filaR.cpp \
	terralib/stat/filaInt.cpp \
	terralib/stat/lista.cpp \
	terralib/stat/hash.cpp \
	terralib/stat/TeSkaterArvore.cpp \
	terralib/stat/TeSemivarModelFactory.cpp \
	terralib/stat/filaDouble.cpp \
	terralib/stat/TeSkaterFunctions.cpp \
	terralib/stat/TeSkaterGrafo.cpp \
	terralib/stat/TeBayesFunctions.cpp \
	terralib/stat/TeMSVFactory.cpp \
	terralib/stat/heap.cpp \
	terralib/stat/TeKernelFunctions.cpp
	
#######################################	
# pdi 

nobase_include_HEADERS += \
	terralib/image_processing/TePDIRegGrowSegFactory.hpp \
	terralib/image_processing/TePDIBufferedFilter.hpp \
	terralib/image_processing/TePDIUtils.hpp \
	terralib/image_processing/TePDIInterpolator.hpp \
	terralib/image_processing/TePDIAlgorithmFactory.hpp \
	terralib/image_processing/TePDIGeoMosaic.hpp \
	terralib/image_processing/TePDIParaSegSegmentsBlock.hpp \
	terralib/image_processing/TePDIParameters.hpp \
	terralib/image_processing/TePDIOFMatching.hpp \
	terralib/image_processing/TePDIParaSegStrategyFactoryParams.hpp \
	terralib/image_processing/TePDIRgbPaletteNode.hpp \
	terralib/image_processing/TePDIEspecData.hpp \
	terralib/image_processing/TePDIPrincipalComponents.hpp \
	terralib/image_processing/TePDIContrast.hpp \
	terralib/image_processing/TePDIMatrixFunctions.hpp \
	terralib/image_processing/TePDIWaveletAtrous.hpp \
	terralib/image_processing/TePDIRadarLeeFilter.hpp \
	terralib/image_processing/TePDIPCAWaveletFusion.hpp \
	terralib/image_processing/TePDIKMeansClas.hpp \
	terralib/image_processing/TePDIMixModelSpectralBand.hpp \
	terralib/image_processing/TePDIRgbPaletteFunctions.hpp \
	terralib/image_processing/TePDIIHSWaveletFusion.hpp \
	terralib/image_processing/TePDIGeneralizedIHSFusion.hpp \
	terralib/image_processing/TePDISensorSimulator.hpp \
	terralib/image_processing/TePDIParaSegStrategy.hpp \
	terralib/image_processing/TePDIRgbPalette.hpp \
	terralib/image_processing/TePDIMixModelStratFactory.hpp \
	terralib/image_processing/TePDISAMClassifier.hpp \
	terralib/image_processing/TePDIRadarFrostFilter.hpp \
	terralib/image_processing/TePDIParallelSegmenter.hpp \
	terralib/image_processing/TePDIRasterRUBuffer.hpp \
	terralib/image_processing/TePDIPrincoMixModelSF.hpp \
	terralib/image_processing/TePDILinearFilter.hpp \
	terralib/image_processing/TePDICluster.hpp \
	terralib/image_processing/TePDILinearFilterFactory.hpp \
	terralib/image_processing/TePDIContrastFactory.hpp \
	terralib/image_processing/TePDIMixModelComponent.hpp \
	terralib/image_processing/TePDIParaSegSegment.hpp \
	terralib/image_processing/TePDIRegisterFactory.hpp \
	terralib/image_processing/TePDIEMClas.hpp \
	terralib/image_processing/TePDIMIMatching.hpp \
	terralib/image_processing/TePDIWaveletAtrousFusion.hpp \
	terralib/image_processing/TePDIRadarLeeFilterFactory.hpp \
	terralib/image_processing/TePDIParaSegRegGrowStrategy.hpp \
	terralib/image_processing/TePDIMallatWavelets.hpp \
	terralib/image_processing/TePDIColorTransform.hpp \
	terralib/image_processing/TePDIRegGrowSeg.hpp \
	terralib/image_processing/TePDIHistogram.hpp \
	terralib/image_processing/TePDIParaSegBaatzStrategyFactory.hpp \
	terralib/image_processing/TePDIMtxDoubleAdpt.hpp \
	terralib/image_processing/TePDIParaSegBaatzStrategy.hpp \
	terralib/image_processing/TePDIRadarKuanFilter.hpp \
	terralib/image_processing/TePDIRadarKuanFilterFactory.hpp \
	terralib/image_processing/TePDIBatchGeoMosaic.hpp \
	terralib/image_processing/TePDIWiSpeRFusion.hpp \
	terralib/image_processing/TePDIMixModel.hpp \
	terralib/image_processing/TePDIAlgorithm.hpp \
	terralib/image_processing/TePDIFilterMask.hpp \
	terralib/image_processing/TePDIRegion.hpp \
	terralib/image_processing/TePDIVenturaFusion.hpp \
	terralib/image_processing/TePDIMixModelSpectralBandList.hpp \
	terralib/image_processing/TePDIMMIOMatching.hpp \
	terralib/image_processing/TePDIParaSegStrategyFactory.hpp \
	terralib/image_processing/TePDIRadarGammaFilter.hpp \
	terralib/image_processing/TePDITileIndexer.hpp \
	terralib/image_processing/TePDIRaster2Vector.hpp \
	terralib/image_processing/TePDIMatrix.hpp \
	terralib/image_processing/TePDIMixModelStrategy.hpp \
	terralib/image_processing/TePDIJointHistogram.hpp \
	terralib/image_processing/TePDIRadarFrostFilterFactory.hpp \
	terralib/image_processing/TePDIIsosegClas.hpp \
	terralib/image_processing/TePDIDecorrelationEnhancement.hpp \
	terralib/image_processing/TePDIParaSegRegGrowStrategyFactory.hpp \
	terralib/image_processing/TePDIFusionIndexes.hpp \
	terralib/image_processing/TePDIPIManager.hpp \
	terralib/image_processing/TePDIRegister.hpp \
	terralib/image_processing/TePDICorrelationMatching.hpp \
	terralib/image_processing/TePDIPrincipalComponentsFusion.hpp \
	terralib/image_processing/TePDIBlender.hpp \
	terralib/image_processing/TePDIParaSegStrategyParams.hpp \
	terralib/image_processing/TePDITypes.hpp \
	terralib/image_processing/TePDIMorfFilterFactory.hpp \
	terralib/image_processing/TePDIArithmetic.hpp \
	terralib/image_processing/TePDITPMosaic.hpp \
	terralib/image_processing/TePDIMixModelComponentList.hpp \
	terralib/image_processing/TePDIMorfFilter.hpp \
	terralib/image_processing/TePDIStrategyFactory.hpp \
	terralib/image_processing/TePDIBDFilterFactory.hpp \
	terralib/image_processing/TePDIPrincoMixModelStrategy.hpp \
	terralib/image_processing/TePDIMathFunctions.hpp \
	terralib/image_processing/TePDIArithmeticFactory.hpp \
	terralib/image_processing/TePDIBDFilter.hpp \
	terralib/image_processing/TePDIRadarFilter.hpp \
	terralib/image_processing/TePDIStrategy.hpp \
	terralib/image_processing/TePDIColorTransformFactory.hpp \
	terralib/image_processing/TePDIIHSFusion.hpp \
	terralib/image_processing/TePDIHaralick.hpp \
	terralib/image_processing/TePDIMixModelFactory.hpp \
	terralib/image_processing/TePDIKMeansClasFactory.hpp \
	terralib/image_processing/TePDIIsosegClasFactory.hpp \
	terralib/image_processing/TePDIPrincipalComponentsFactory.hpp \
	terralib/image_processing/TePDIDefines.hpp \
	terralib/image_processing/TePDIGarguetFusion.hpp \
	terralib/image_processing/TePDILevelRemap.hpp \
	terralib/image_processing/TePDISensorSimulatorFactory.hpp \
	terralib/image_processing/TePDIRaster2VectorFactory.hpp \
	terralib/image_processing/TePDIBaatz.hpp \
	terralib/image_processing/TePDIEMClasFactory.hpp \
	terralib/image_processing/TePDIStatistic.hpp \
	terralib/image_processing/TePDIPIManagerGlobalSettings.hpp \
	terralib/image_processing/TePDISimplex.hpp \
	terralib/image_processing/TePDIFFT.hpp \
	terralib/image_processing/TePDIHants.hpp

___bin_libterralib_la_SOURCES += \
	terralib/image_processing/TePDIWiSpeRFusion.cpp \
	terralib/image_processing/TePDIMorfFilterFactory.cpp \
	terralib/image_processing/TePDIMixModel.cpp \
	terralib/image_processing/TePDIMorfFilter.cpp \
	terralib/image_processing/TePDIParaSegStrategyFactory.cpp \
	terralib/image_processing/TePDIRegion.cpp \
	terralib/image_processing/TePDIIHSWaveletFusion.cpp \
	terralib/image_processing/TePDIRadarKuanFilterFactory.cpp \
	terralib/image_processing/TePDIGarguetFusion.cpp \
	terralib/image_processing/TePDIRaster2VectorFactory.cpp \
	terralib/image_processing/TePDIGeneralizedIHSFusion.cpp \
	terralib/image_processing/TePDIFilterMask.cpp \
	terralib/image_processing/TePDIMixModelFactory.cpp \
	terralib/image_processing/TePDIRadarKuanFilter.cpp \
	terralib/image_processing/TePDIParaSegStrategyParams.cpp \
	terralib/image_processing/TePDIIsosegClas.cpp \
	terralib/image_processing/TePDIBlender.cpp \
	terralib/image_processing/TePDIPrincipalComponentsFactory.cpp \
	terralib/image_processing/TePDIContrast.cpp \
	terralib/image_processing/TePDIKMeansClas.cpp \
	terralib/image_processing/TePDIAlgorithm.cpp \
	terralib/image_processing/TePDICorrelationMatching.cpp \
	terralib/image_processing/TePDIKMeansClasFactory.cpp \
	terralib/image_processing/TePDIMathFunctions.cpp \
	terralib/image_processing/TePDIGeoMosaic.cpp \
	terralib/image_processing/TePDIRadarLeeFilter.cpp \
	terralib/image_processing/TePDIBDFilter.cpp \
	terralib/image_processing/TePDIParameters.cpp \
	terralib/image_processing/TePDIMixModelStratFactory.cpp \
	terralib/image_processing/TePDIVenturaFusion.cpp \
	terralib/image_processing/TePDIIHSFusion.cpp \
	terralib/image_processing/TePDIRegGrowSeg.cpp \
	terralib/image_processing/TePDIArithmetic.cpp \
	terralib/image_processing/TePDIEspecData.cpp \
	terralib/image_processing/TePDIMMIOMatching.cpp \
	terralib/image_processing/TePDIEMClas.cpp \
	terralib/image_processing/TePDITPMosaic.cpp \
	terralib/image_processing/TePDISensorSimulator.cpp \
	terralib/image_processing/TePDIStrategyFactory.cpp \
	terralib/image_processing/TePDIBatchGeoMosaic.cpp \
	terralib/image_processing/TePDIParaSegSegment.cpp \
	terralib/image_processing/TePDIWaveletAtrous.cpp \
	terralib/image_processing/TePDIColorTransform.cpp \
	terralib/image_processing/TePDICluster.cpp \
	terralib/image_processing/TePDIBDFilterFactory.cpp \
	terralib/image_processing/TePDIHaralick.cpp \
	terralib/image_processing/TePDIMatrixFunctions.cpp \
	terralib/image_processing/TePDIMixModelSpectralBand.cpp \
	terralib/image_processing/TePDIParallelSegmenter.cpp \
	terralib/image_processing/TePDIStrategy.cpp \
	terralib/image_processing/TePDILinearFilter.cpp \
	terralib/image_processing/TePDIRegGrowSegFactory.cpp \
	terralib/image_processing/TePDIParaSegRegGrowStrategy.cpp \
	terralib/image_processing/TePDILevelRemap.cpp \
	terralib/image_processing/TePDIBaatz.cpp \
	terralib/image_processing/TePDIRadarLeeFilterFactory.cpp \
	terralib/image_processing/TePDIMallatWavelets.cpp \
	terralib/image_processing/TePDIRadarFrostFilterFactory.cpp \
	terralib/image_processing/TePDIArithmeticFactory.cpp \
	terralib/image_processing/TePDIRegister.cpp \
	terralib/image_processing/TePDIParaSegStrategyFactoryParams.cpp \
	terralib/image_processing/TePDIDecorrelationEnhancement.cpp \
	terralib/image_processing/TePDIParaSegStrategy.cpp \
	terralib/image_processing/TePDIMixModelComponentList.cpp \
	terralib/image_processing/TePDIPIManager.cpp \
	terralib/image_processing/TePDIMIMatching.cpp \
	terralib/image_processing/TePDIEMClasFactory.cpp \
	terralib/image_processing/TePDIIsosegClasFactory.cpp \
	terralib/image_processing/TePDIBufferedFilter.cpp \
	terralib/image_processing/TePDIParaSegBaatzStrategyFactory.cpp \
	terralib/image_processing/TePDIRadarFrostFilter.cpp \
	terralib/image_processing/TePDISAMClassifier.cpp \
	terralib/image_processing/TePDIInterpolator.cpp \
	terralib/image_processing/TePDIRgbPalette.cpp \
	terralib/image_processing/TePDIColorTransformFactory.cpp \
	terralib/image_processing/TePDIPCAWaveletFusion.cpp \
	terralib/image_processing/TePDIFusionIndexes.cpp \
	terralib/image_processing/TePDIPrincipalComponents.cpp \
	terralib/image_processing/TePDISensorSimulatorFactory.cpp \
	terralib/image_processing/TePDIParaSegBaatzStrategy.cpp \
	terralib/image_processing/TePDIStatistic.cpp \
	terralib/image_processing/TePDIPrincoMixModelSF.cpp \
	terralib/image_processing/TePDIUtils.cpp \
	terralib/image_processing/TePDIPrincipalComponentsFusion.cpp \
	terralib/image_processing/TePDIParaSegRegGrowStrategyFactory.cpp \
	terralib/image_processing/TePDIParaSegSegmentsBlock.cpp \
	terralib/image_processing/TePDIRaster2Vector.cpp \
	terralib/image_processing/TePDIPrincoMixModelStrategy.cpp \
	terralib/image_processing/TePDIHistogram.cpp \
	terralib/image_processing/TePDIJointHistogram.cpp \
	terralib/image_processing/TePDIContrastFactory.cpp \
	terralib/image_processing/TePDIOFMatching.cpp \
	terralib/image_processing/TePDIRgbPaletteFunctions.cpp \
	terralib/image_processing/TePDIWaveletAtrousFusion.cpp \
	terralib/image_processing/TePDIRasterRUBuffer.cpp \
	terralib/image_processing/TePDIRadarFilter.cpp \
	terralib/image_processing/TePDIRadarGammaFilter.cpp \
	terralib/image_processing/TePDIMixModelStrategy.cpp \
	terralib/image_processing/TePDIMixModelComponent.cpp \
	terralib/image_processing/TePDIAlgorithmFactory.cpp \
	terralib/image_processing/TePDIRegisterFactory.cpp \
	terralib/image_processing/TePDILinearFilterFactory.cpp \
	terralib/image_processing/TePDIMixModelSpectralBandList.cpp \
	terralib/image_processing/TePDISimplex.cpp \
	terralib/image_processing/TePDIFFT.cpp \
	terralib/image_processing/TePDIHants.cpp


#######################################
# utils

nobase_include_HEADERS += \
	terralib/utils/TeWKTGeometryDecoder.h \
	terralib/utils/TeDatabaseUtils.h \
	terralib/utils/TeUtilsDefines.h \
	terralib/utils/mtrand.h \
	terralib/utils/TeColorUtils.h \
	terralib/utils/TeUpdateDBVersion.h \
	terralib/utils/TeWKBGeometryDecoder.h

___bin_libterralib_la_SOURCES += \
	terralib/utils/TeUpdateDBVersion.cpp \
	terralib/utils/TeWKTGeometryDecoder.cpp \
	terralib/utils/TeColorUtils.cpp \
	terralib/utils/mtrand.cpp \
	terralib/utils/TeWKBGeometryDecoder.cpp \
	terralib/utils/TeDatabaseUtils.cpp
	
#######################################
# functions	
	
nobase_include_HEADERS += \
	terralib/functions/TeFunctionsDefines.h \
	terralib/functions/TeCoordAlgorithms.h \
	terralib/functions/TeSPRFile.h \
	terralib/functions/TeDriverMIDMIF.h \
	terralib/functions/TeDriverSPRING.h \
	terralib/functions/TeGeoProcessingFunctions.h \
	terralib/functions/TeCellAlgorithms.h \
	terralib/functions/TeAddressLocator.h \
	terralib/functions/TeDriverBNA.h \
	terralib/functions/TeDriverCSV.h \
	terralib/functions/TeLayerFunctions.h \
	terralib/functions/TeMIFProjection.h \
	terralib/functions/TeThemeFunctions.h 

___bin_libterralib_la_SOURCES += \
	terralib/functions/TeCoordAlgorithms.cpp \
	terralib/functions/TeSimilarity.cpp \
	terralib/functions/TeSPRFile.cpp \
	terralib/functions/TeExportMIF.cpp \
	terralib/functions/TeExportSPR.cpp \
	terralib/functions/TeGeoProcessingFunctions.cpp \
	terralib/functions/TeCellAlgorithms.cpp \
	terralib/functions/TeAddressLocator.cpp \
	terralib/functions/TeImportBNA.cpp \
	terralib/functions/TeImportCSV.cpp \
	terralib/functions/TeImportGeo.cpp \
	terralib/functions/TeImportMIF.cpp \
	terralib/functions/TeLayerFunctions.cpp \
	terralib/functions/TeMIFProjection.cpp \
	terralib/functions/TeExportCSV.cpp \
	terralib/functions/TeThemeFunctions.cpp
	
#######################################	
# mysql	

if TE_BUILD_MYSQL_MODULE
nobase_include_HEADERS += \
	terralib/drivers/MySQL/TeMySQLDefines.h \
	terralib/drivers/MySQL/TeMySQL.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/MySQL/TeMySQL.cpp	
endif	

#######################################
# postgis

if TE_BUILD_POSTGIS_MODULE
nobase_include_HEADERS += \
	terralib/drivers/PostgreSQL/TePGInterface.h \
	terralib/drivers/PostgreSQL/TePGUtils.h \
	terralib/drivers/PostgreSQL/TePostgreSQL.h \
	terralib/drivers/PostgreSQL/TePostGIS.h \
	terralib/drivers/PostgreSQL/TePostgreSQLDefines.h \
	terralib/drivers/PostgreSQL/TePostgreSQLConnection.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/PostgreSQL/TePGInterface.cpp \
	terralib/drivers/PostgreSQL/TePGUtils.cpp \
	terralib/drivers/PostgreSQL/TePostgreSQL.cpp \
	terralib/drivers/PostgreSQL/TePostGIS.cpp \
	terralib/drivers/PostgreSQL/TePostgreSQLConnection.cpp
endif

#######################################
# mrsid

if TE_BUILD_MRSID_MODULE
nobase_include_HEADERS += \
	terralib/drivers/MrSID/TeDecoderMrSID.h \
	terralib/drivers/MrSID/TeMrSID.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/MrSID/TeDecoderMrSID.cpp \
	terralib/drivers/MrSID/TeMrSID.cpp
endif

#######################################
# gdal

if TE_BUILD_GDAL_MODULE
nobase_include_HEADERS += \
	terralib/drivers/gdal/TeOGRDriverFactory.h \
	terralib/drivers/gdal/TeGDALUtils.h \
	terralib/drivers/gdal/TeOGRUtils.h \
	terralib/drivers/gdal/TeGDALDefines.h \
	terralib/drivers/gdal/TeGDALDecoder.h \
	terralib/drivers/gdal/TeOGRDriver.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/gdal/TeGDALUtils.cpp \
	terralib/drivers/gdal/TeOGRUtils.cpp \
	terralib/drivers/gdal/TeOGRDriver.cpp \
	terralib/drivers/gdal/TeOGRDriverFactory.cpp \
	terralib/drivers/gdal/TeGDALDecoder.cpp
endif	

#######################################
# shapelib driver

if TE_BUILD_SHAPELIB_MODULE
nobase_include_HEADERS += \
	terralib/drivers/shapelib/TeSTEFunctionsSHP.h \
	terralib/drivers/shapelib/TeQuerierSHP.h \
	terralib/drivers/shapelib/TeSHPDefines.h \
	terralib/drivers/shapelib/TeSHPDriverFactory.h \
	terralib/drivers/shapelib/TeDriverSHPDBF.h
	
___bin_libterralib_la_SOURCES += \
	terralib/drivers/shapelib/TeDriverSHPDBF.cpp \
	terralib/drivers/shapelib/TeSHPDriverFactory.cpp
endif

#######################################	
# LibNetPBM driver

if TE_BUILD_NETPBM_MODULE
nobase_include_HEADERS += \
	terralib/drivers/PAMDecoder/TeDecoderPAM.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/PAMDecoder/TeDecoderPAM.cpp	
endif	
	
#########################################################################
# spl driver module


if TE_BUILD_QT_MODULE
nobase_include_HEADERS += \
	terralib/drivers/spl/TeSPLDefines.h \
	terralib/drivers/spl/PluginInfo.h \
	terralib/drivers/spl/PluginMetadata.h \
	terralib/drivers/spl/PluginParameters.h \
	terralib/drivers/spl/PluginsManager.h \
	terralib/drivers/spl/PluginsParametersContainer.h \
	terralib/drivers/spl/PluginsSignal.h

___bin_libterralib_la_SOURCES += \
	terralib/drivers/spl/PluginMetadata.cpp \
	terralib/drivers/spl/PluginParameters.cpp \
	terralib/drivers/spl/PluginsManager.cpp \
	terralib/drivers/spl/PluginsSignal.cpp
endif

#########################################################################
# qt driver
if TE_BUILD_QT_MODULE
nobase_include_HEADERS += \
	terralib/drivers/qt/resultVisual.h \
	terralib/drivers/qt/TeAddressNotifyFunctorWindow.h \
	terralib/drivers/qt/TeApplicationUtils.h \
	terralib/drivers/qt/TeAppTheme.h \
	terralib/drivers/qt/TeDecoderQtImage.h \
	terralib/drivers/qt/TeGUIUtils.h \
	terralib/drivers/qt/TePlotTheme.h \
	terralib/drivers/qt/TePlotView.h \
	terralib/drivers/qt/TeQtAnimaThread.h \
	terralib/drivers/qt/TeQtBigTable.h \
	terralib/drivers/qt/TeQtCanvas.h \
	terralib/drivers/qt/TeQtChartItem.h \
	terralib/drivers/qt/TeQtCheckListItem.h \
	terralib/drivers/qt/TeQtColorBar.h \
	terralib/drivers/qt/TeQtDatabaseItem.h \
	terralib/drivers/qt/TeQtDatabasesListView.h \
	terralib/drivers/qt/TeQtDataSource.h \
	terralib/drivers/qt/TeQtFrame.h \
	terralib/drivers/qt/TeQtGLWidget.h \
	terralib/drivers/qt/TeQtGraph.h \
	terralib/drivers/qt/TeQtGrid.h \
	terralib/drivers/qt/TeQtGridSource.h \
	terralib/drivers/qt/TeQtLayerItem.h \
	terralib/drivers/qt/TeQtLegendItem.h \
	terralib/drivers/qt/TeQtLegendSource.h \
	terralib/drivers/qt/TeQtProgress.h \
	terralib/drivers/qt/TeQtTable.h \
	terralib/drivers/qt/TeQtTextEdit.h \
	terralib/drivers/qt/TeQtThemeItem.h \
	terralib/drivers/qt/TeQtViewItem.h \
	terralib/drivers/qt/TeQtViewsListView.h \
	terralib/drivers/qt/TeWaitCursor.h

EXTRA_DIST += \
	../app_icons/check.xpm \
	../app_icons/cursor_distance_mask.xpm \
	../app_icons/cursor_distance.xpm \
	../app_icons/cursor_edit_mask.xpm \
	../app_icons/cursor_edit.xpm \
	../app_icons/cursor_info_mask.xpm \
	../app_icons/cursor_info.xpm \
	../app_icons/cursor_zoom_in_mask.xpm \
	../app_icons/cursor_zoom_in.xpm \
	../app_icons/cursor_zoom_out_mask.xpm \
	../app_icons/cursor_zoom_out.xpm \
	../app_icons/database.xpm \
	../app_icons/edit.xpm \
	../app_icons/externalTheme.xpm \
	../app_icons/fileTheme.xpm \
	../app_icons/importDisabled.xpm \
	../app_icons/import.xpm \
	../app_icons/infolayer.xpm \
	../app_icons/invalidExternalTheme.xpm \
	../app_icons/nonEdit.xpm \
	../app_icons/themeDisabled.xpm \
	../app_icons/theme.xpm \
	../app_icons/uncheck.xpm \
	../app_icons/viewDisabled.xpm \
	../app_icons/view.xpm \
	../app_icons/visual.xpm \
	../app_icons/wmsTheme.xpm \
	terralib/drivers/qt/ui/animation.ui \
	terralib/drivers/qt/ui/animation.ui.h \
	terralib/drivers/qt/ui/help.ui \
	terralib/drivers/qt/ui/help.ui.h \
	terralib/drivers/qt/ui/mediaDescription.ui \
	terralib/drivers/qt/ui/mediaDescription.ui.h \
	terralib/drivers/qt/ui/urlWindow.ui \
	terralib/drivers/qt/ui/urlWindow.ui.h \
	terralib/drivers/qt/TeApplicationUtils.cpp \
	terralib/drivers/qt/TeAppTheme.cpp \
	terralib/drivers/qt/TeDecoderQtImage.cpp \
	terralib/drivers/qt/TeGUIUtils.cpp \
	terralib/drivers/qt/TeQtAnimaThread.cpp \
	terralib/drivers/qt/TeQtBigTable.cpp \
	terralib/drivers/qt/TeQtCanvas.cpp \
	terralib/drivers/qt/TeQtChartItem.cpp \
	terralib/drivers/qt/TeQtCheckListItem.cpp \
	terralib/drivers/qt/TeQtColorBar.cpp \
	terralib/drivers/qt/TeQtDatabaseItem.cpp \
	terralib/drivers/qt/TeQtDatabasesListView.cpp \
	terralib/drivers/qt/TeQtFrame.cpp \
	terralib/drivers/qt/TeQtGLWidget.cpp \
	terralib/drivers/qt/TeQtGraph.cpp \
	terralib/drivers/qt/TeQtGrid.cpp \
	terralib/drivers/qt/TeQtGridSource.cpp \
	terralib/drivers/qt/TeQtLayerItem.cpp \
	terralib/drivers/qt/TeQtLegendItem.cpp \
	terralib/drivers/qt/TeQtLegendSource.cpp \
	terralib/drivers/qt/TeQtTable.cpp \
	terralib/drivers/qt/TeQtTextEdit.cpp \
	terralib/drivers/qt/TeQtThemeItem.cpp \
	terralib/drivers/qt/TeQtViewItem.cpp \
	terralib/drivers/qt/TeQtViewsListView.cpp \
	terralib/drivers/qt/TeWaitCursor.cpp

TLQTDRIVER_BUILTHEADERS = \
	terralib/drivers/qt/ui/mediaDescription.h \
	terralib/drivers/qt/ui/urlWindow.h \
	terralib/drivers/qt/ui/help.h \
	terralib/drivers/qt/ui/animation.h  
  
TLQTDRIVER_BUILTSOURCES = \
	terralib/drivers/qt/resultVisual.moc \
	terralib/drivers/qt/ui/mediaDescription.uic \
	terralib/drivers/qt/ui/urlWindow.uic \
		terralib/drivers/qt/ui/help.uic \
	terralib/drivers/qt/ui/animation.uic \
	terralib/drivers/qt/TeAddressNotifyFunctorWindow.moc \
	terralib/drivers/qt/TeApplicationUtils.moc \
	terralib/drivers/qt/TeAppTheme.moc \
	terralib/drivers/qt/TeDecoderQtImage.moc \
	terralib/drivers/qt/TeGUIUtils.moc \
	terralib/drivers/qt/TeQtAnimaThread.moc \
	terralib/drivers/qt/TeQtBigTable.moc \
	terralib/drivers/qt/TeQtCanvas.moc \
	terralib/drivers/qt/TeQtChartItem.moc \
	terralib/drivers/qt/TeQtCheckListItem.moc \
	terralib/drivers/qt/TeQtColorBar.moc \
	terralib/drivers/qt/TeQtDatabaseItem.moc \
	terralib/drivers/qt/TeQtDatabasesListView.moc \
	terralib/drivers/qt/TeQtDataSource.moc \
	terralib/drivers/qt/TeQtFrame.moc \
	terralib/drivers/qt/TeQtGLWidget.moc \
	terralib/drivers/qt/TeQtGraph.moc \
	terralib/drivers/qt/TeQtGrid.moc \
	terralib/drivers/qt/TeQtGridSource.moc \
	terralib/drivers/qt/TeQtLayerItem.moc \
	terralib/drivers/qt/TeQtLegendItem.moc \
	terralib/drivers/qt/TeQtLegendSource.moc \
	terralib/drivers/qt/TeQtProgress.moc \
	terralib/drivers/qt/TeQtTable.moc \
	terralib/drivers/qt/TeQtTextEdit.moc \
	terralib/drivers/qt/TeQtThemeItem.moc \
	terralib/drivers/qt/TeQtViewItem.moc \
	terralib/drivers/qt/TeQtViewsListView.moc \
	terralib/drivers/qt/TeWaitCursor.moc
	
BUILT_SOURCES += $(TLQTDRIVER_BUILTHEADERS) $(TLQTDRIVER_BUILTSOURCES)

nobase_include_HEADERS += $(TLQTDRIVER_BUILTHEADERS)

___bin_libterralib_la_CPPFLAGS += \
	-I$(srcdir)/terralib/drivers/qt \
	-I$(srcdir)/terralib/drivers/qt/ui
	
nodist____bin_libterralib_la_SOURCES += $(TLQTDRIVER_BUILTSOURCES)

___bin_libterralib_la_SOURCES += \
	terralib/drivers/qt/TeInitRasterQtDecoder.cpp
endif

	
#########################################################################
# QWT driver

if TE_BUILD_QWT_MODULE
nobase_include_HEADERS += \
	terralib/drivers/qwt/TeQwtPlot.h \
	terralib/drivers/qwt/TeQwtDefines.h \
	terralib/drivers/qwt/TeQwtPlotCurve.h \
	terralib/drivers/qwt/TeQwtPlotPicker.h \
	terralib/drivers/qwt/TeQwtPlotZoomer.h
	
EXTRA_DIST += \
	terralib/drivers/qwt/TeQwtPlot.cpp \
	terralib/drivers/qwt/TeQwtPlotCurve.cpp \
	terralib/drivers/qwt/TeQwtPlotPicker.cpp \
	terralib/drivers/qwt/TeQwtPlotZoomer.cpp	

TLQWT_BUILTSOURCES = \
	terralib/drivers/qwt/TeQwtPlot.moc \
	terralib/drivers/qwt/TeQwtPlotCurve.moc \
	terralib/drivers/qwt/TeQwtPlotPicker.moc \
	terralib/drivers/qwt/TeQwtPlotZoomer.moc

BUILT_SOURCES += $(TLQWT_BUILTSOURCES)
  
nodist____bin_libterralib_la_SOURCES += $(TLQWT_BUILTSOURCES)

endif	

######################################################################### 
# Adding all built sources to the distclean step

CLEANFILES += $(BUILT_SOURCES)	
	
			
