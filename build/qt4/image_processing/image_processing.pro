TARGET  = terralibpdi
CONFIG += shared 
include (../config.pri)
TERRALIB_SRC_DIR = $${TERRALIBPATH}/src
LIBPDI_SRC_DIR = $${TERRALIB_SRC_DIR}/terralib/image_processing
DEFINES += PDI_AS_DLL

LIBS += -ljpeg -lterralib

win32 {
	win32-g++:LIBS += -L../../../dependencies/win32/zlib/lib 
	else:QMAKE_LIBDIR += ../../../dependencies/win32/zlib/lib
	LIBS += -lzlibdll
}
unix:LIBS += -lz -lpthread
LIBS += -lterralibtiff
INCLUDEPATH = \
  $${TERRALIB_SRC_DIR}/terralib/kernel \
  $${TERRALIB_SRC_DIR}/terralib/functions \
  $${TERRALIB_SRC_DIR}/tiff \
  $${LIBPDI_SRC_DIR} \
  $${LIBPDI_SRC_DIR}/data_structs
INCLUDEPATH = \
  $${TERRALIB_SRC_DIR}/terralib/kernel \
  $${TERRALIB_SRC_DIR}/terralib/functions \
  $${TERRALIB_SRC_DIR}/tiff \
  $${LIBPDI_SRC_DIR} \
  $${LIBPDI_SRC_DIR}/data_structs

HEADERS = \
          $${LIBPDI_SRC_DIR}/TePDITileIndexer.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMatrix.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMatrixFunctions.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMathFunctions.hpp \
          $${LIBPDI_SRC_DIR}/TePDIUtils.hpp \
          $${LIBPDI_SRC_DIR}/TePDITypes.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPIManager.hpp \
          $${LIBPDI_SRC_DIR}/TePDIHistogram.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRgbPaletteNode.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRgbPalette.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRgbPaletteFunctions.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParameters.hpp \
          $${LIBPDI_SRC_DIR}/TePDIAlgorithm.hpp \
          $${LIBPDI_SRC_DIR}/TePDIAlgorithmFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIStatistic.hpp \
          $${LIBPDI_SRC_DIR}/TePDILevelRemap.hpp \
          $${LIBPDI_SRC_DIR}/TePDIContrast.hpp \
          $${LIBPDI_SRC_DIR}/TePDIContrastFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIFilterMask.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBufferedFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDILinearFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDILinearFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBDFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBDFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMorfFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMorfFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarLeeFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarLeeFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarKuanFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarKuanFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFrostFilter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFrostFilterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIArithmetic.hpp \
          $${LIBPDI_SRC_DIR}/TePDIArithmeticFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRaster2Vector.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRaster2VectorFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIColorTransform.hpp \
          $${LIBPDI_SRC_DIR}/TePDIColorTransformFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponents.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponentsFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMallatWavelets.hpp \
          $${LIBPDI_SRC_DIR}/TePDIGeoMosaic.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBatchGeoMosaic.hpp \
          $${LIBPDI_SRC_DIR}/TePDIStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIStrategyFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIGarguetFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIVenturaFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRegGrowSeg.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRegGrowSegFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRegister.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRegisterFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIInterpolator.hpp \
          $${LIBPDI_SRC_DIR}/TePDIEspecData.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRegion.hpp \
          $${LIBPDI_SRC_DIR}/TePDICluster.hpp \
          $${LIBPDI_SRC_DIR}/TePDIIsosegClas.hpp \
          $${LIBPDI_SRC_DIR}/TePDIIsosegClasFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIHaralick.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelComponent.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelComponentList.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModel.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelSpectralBand.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelSpectralBandList.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincoMixModelSF.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincoMixModelStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelStratFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIKMeansClas.hpp \
          $${LIBPDI_SRC_DIR}/TePDIKMeansClasFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIEMClas.hpp \
          $${LIBPDI_SRC_DIR}/TePDIEMClasFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDISensorSimulatorFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDISensorSimulator.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPIManagerGlobalSettings.hpp \
          $${LIBPDI_SRC_DIR}/TePDIOFMatching.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponentsFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIDecorrelationEnhancement.hpp \
          $${LIBPDI_SRC_DIR}/TePDICorrelationMatching.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBaatz.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMIMatching.hpp \
          $${LIBPDI_SRC_DIR}/TePDIJointHistogram.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMtxDoubleAdpt.hpp \
          $${LIBPDI_SRC_DIR}/TePDIMMIOMatching.hpp \
          $${LIBPDI_SRC_DIR}/TePDIIHSFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIFusionIndexes.hpp \
          $${LIBPDI_SRC_DIR}/TePDIGeneralizedIHSFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIIHSWaveletFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIPCAWaveletFusion.hpp \ 
          $${LIBPDI_SRC_DIR}/TePDIWaveletAtrousFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIWiSpeRFusion.hpp \
          $${LIBPDI_SRC_DIR}/TePDIWaveletAtrous.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParallelSegmenter.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegSegment.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegSegmentsBlock.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyFactoryParams.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegRegGrowStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegRegGrowStrategyFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegBaatzStrategy.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegBaatzStrategyFactory.hpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyParams.hpp \
          $${LIBPDI_SRC_DIR}/TePDIBlender.hpp \
          $${LIBPDI_SRC_DIR}/TePDITPMosaic.hpp \
          $${LIBPDI_SRC_DIR}/TePDISAMClassifier.hpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarGammaFilter.hpp


SOURCES = \
          $${LIBPDI_SRC_DIR}/TePDIMathFunctions.cpp \
          $${LIBPDI_SRC_DIR}/TePDIUtils.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMatrixFunctions.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPIManager.cpp \
          $${LIBPDI_SRC_DIR}/TePDIHistogram.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRgbPalette.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRgbPaletteFunctions.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParameters.cpp \
          $${LIBPDI_SRC_DIR}/TePDIAlgorithm.cpp \
          $${LIBPDI_SRC_DIR}/TePDIAlgorithmFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIStatistic.cpp \
          $${LIBPDI_SRC_DIR}/TePDILevelRemap.cpp \
          $${LIBPDI_SRC_DIR}/TePDIContrast.cpp \
          $${LIBPDI_SRC_DIR}/TePDIContrastFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIFilterMask.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBufferedFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDILinearFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDILinearFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBDFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBDFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMorfFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMorfFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarLeeFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarLeeFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarKuanFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarKuanFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFrostFilter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarFrostFilterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIArithmetic.cpp \
          $${LIBPDI_SRC_DIR}/TePDIArithmeticFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRaster2Vector.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRaster2VectorFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIColorTransform.cpp \
          $${LIBPDI_SRC_DIR}/TePDIColorTransformFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponents.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponentsFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMallatWavelets.cpp \
          $${LIBPDI_SRC_DIR}/TePDIGeoMosaic.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBatchGeoMosaic.cpp \
          $${LIBPDI_SRC_DIR}/TePDIStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIStrategyFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIGarguetFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIVenturaFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRegGrowSeg.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRegGrowSegFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRegister.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRegisterFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIInterpolator.cpp \
          $${LIBPDI_SRC_DIR}/TePDIEspecData.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRegion.cpp \
          $${LIBPDI_SRC_DIR}/TePDICluster.cpp \
          $${LIBPDI_SRC_DIR}/TePDIIsosegClas.cpp \
          $${LIBPDI_SRC_DIR}/TePDIIsosegClasFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIHaralick.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelComponent.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelComponentList.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModel.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelSpectralBand.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelSpectralBandList.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincoMixModelSF.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincoMixModelStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMixModelStratFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIKMeansClas.cpp \
          $${LIBPDI_SRC_DIR}/TePDIKMeansClasFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIEMClas.cpp \
          $${LIBPDI_SRC_DIR}/TePDIEMClasFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDISensorSimulatorFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDISensorSimulator.cpp \
          $${LIBPDI_SRC_DIR}/TePDIOFMatching.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPrincipalComponentsFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIDecorrelationEnhancement.cpp \
          $${LIBPDI_SRC_DIR}/TePDICorrelationMatching.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBaatz.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMIMatching.cpp \
          $${LIBPDI_SRC_DIR}/TePDIJointHistogram.cpp \
          $${LIBPDI_SRC_DIR}/TePDIMMIOMatching.cpp \
          $${LIBPDI_SRC_DIR}/TePDIIHSFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIFusionIndexes.cpp \
          $${LIBPDI_SRC_DIR}/TePDIGeneralizedIHSFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIIHSWaveletFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIPCAWaveletFusion.cpp \ 
          $${LIBPDI_SRC_DIR}/TePDIWaveletAtrousFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIWiSpeRFusion.cpp \
          $${LIBPDI_SRC_DIR}/TePDIWaveletAtrous.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegSegment.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegSegmentsBlock.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyParams.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParallelSegmenter.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegRegGrowStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegBaatzStrategy.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyFactoryParams.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegStrategyFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegRegGrowStrategyFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIParaSegBaatzStrategyFactory.cpp \
          $${LIBPDI_SRC_DIR}/TePDIBlender.cpp \
          $${LIBPDI_SRC_DIR}/TePDITPMosaic.cpp \
          $${LIBPDI_SRC_DIR}/TePDISAMClassifier.cpp \
          $${LIBPDI_SRC_DIR}/TePDIRadarGammaFilter.cpp

win32 {
  !win32-g++ {
	  CONFIG += staticlib console

	  DEFINES += WIN32 __WIN32__

	  TEMPLATE = vclib
  }
  
  USE_TERRALIB_STL = yes
  contains( QMAKESPEC , win32-msvc.net ) {
    USE_TERRALIB_STL = no
  }
  equals( USE_TERRALIB_STL , yes ) {
    INCLUDEPATH = $${TERRALIB_SRC_DIR}/STLport $${INCLUDEPATH}
    message( "TerraLib internal STL will be used" )
  }    
}

include (../install_cfg.pri)

DEPENDPATH += $${INCLUDEPATH}
