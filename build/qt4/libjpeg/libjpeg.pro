TARGET = jpeg
win32 {
	win32-g++:CONFIG += shared
	else:CONFIG += staticlib
	DEFINES += _LIB
}
unix:CONFIG += staticlib
include (../config.pri)
# Path definitions
SRCPATH = $${TERRALIBPATH}/src/libjpeg
#-------------------------------------
INCLUDEPATH += $${SRCPATH}
HEADERS += $${SRCPATH}/cderror.h \
    $${SRCPATH}/cdjpeg.h \
    $${SRCPATH}/jchuff.h \
    $${SRCPATH}/jconfig.h \
    $${SRCPATH}/jdct.h \
    $${SRCPATH}/jdhuff.h \
    $${SRCPATH}/jerror.h \
    $${SRCPATH}/jinclude.h \
    $${SRCPATH}/jmemsys.h \
    $${SRCPATH}/jmorecfg.h \
    $${SRCPATH}/jpegint.h \
    $${SRCPATH}/jpeglib.h \
    $${SRCPATH}/jversion.h \
    $${SRCPATH}/transupp.h
SOURCES +=  $${SRCPATH}/jcapimin.c \
	$${SRCPATH}/jcapistd.c \
	$${SRCPATH}/jccoefct.c \
	$${SRCPATH}/jccolor.c \
	$${SRCPATH}/jcdctmgr.c \
	$${SRCPATH}/jchuff.c \
	$${SRCPATH}/jcinit.c \
	$${SRCPATH}/jcmainct.c \
	$${SRCPATH}/jcmarker.c \
	$${SRCPATH}/jcmaster.c \
	$${SRCPATH}/jcomapi.c \
	$${SRCPATH}/jcparam.c \
	$${SRCPATH}/jcphuff.c \
	$${SRCPATH}/jcprepct.c \
	$${SRCPATH}/jcsample.c \
	$${SRCPATH}/jctrans.c \
	$${SRCPATH}/jdapimin.c \
	$${SRCPATH}/jdapistd.c \
	$${SRCPATH}/jdatadst.c \
	$${SRCPATH}/jdatasrc.c \
	$${SRCPATH}/jdcoefct.c \
	$${SRCPATH}/jdcolor.c \
	$${SRCPATH}/jddctmgr.c \
	$${SRCPATH}/jdhuff.c \
	$${SRCPATH}/jdinput.c \
	$${SRCPATH}/jdmainct.c \
	$${SRCPATH}/jdmarker.c \
	$${SRCPATH}/jdmaster.c \
	$${SRCPATH}/jdmerge.c \
	$${SRCPATH}/jdphuff.c \
	$${SRCPATH}/jdpostct.c \
	$${SRCPATH}/jdsample.c \
	$${SRCPATH}/jdtrans.c \
	$${SRCPATH}/jerror.c \
	$${SRCPATH}/jfdctflt.c \
	$${SRCPATH}/jfdctfst.c \
	$${SRCPATH}/jfdctint.c \
	$${SRCPATH}/jidctflt.c \
	$${SRCPATH}/jidctfst.c \
	$${SRCPATH}/jidctint.c \
	$${SRCPATH}/jidctred.c \
	$${SRCPATH}/jmemmgr.c \
	$${SRCPATH}/jmemnobs.c \
	$${SRCPATH}/jquant1.c \
	$${SRCPATH}/jquant2.c \
	$${SRCPATH}/jutils.c \
	$${SRCPATH}/rdbmp.c \
	$${SRCPATH}/rdcolmap.c \
	$${SRCPATH}/rdgif.c \
	$${SRCPATH}/rdppm.c \
	$${SRCPATH}/rdrle.c \
	$${SRCPATH}/rdswitch.c \
	$${SRCPATH}/rdtarga.c \
	$${SRCPATH}/transupp.c \
	$${SRCPATH}/wrbmp.c \
	$${SRCPATH}/wrgif.c \
	$${SRCPATH}/wrppm.c \
	$${SRCPATH}/wrrle.c \
	$${SRCPATH}/wrtarga.c 
include (../install_cfg.pri)
CONFIG(copy_dir_files) {
    JPPATH = $${TERRALIBPATH}/src/libjpeg
    include.path = $${DEPLOY_DIR}/include/libjpeg
    include.files = $${JPPATH}/*.h
    INSTALLS += include
}

DEPENDPATH += $${INCLUDEPATH}
