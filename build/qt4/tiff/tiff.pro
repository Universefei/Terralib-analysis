TARGET  = terralibtiff
CONFIG += shared 
win32:DEFINES += _WINDOWS \
    _USRDLL \
    _BUILD_AS_DLL

include (../config.pri)

TIFFPATH = $${TERRALIBPATH}/src/tiff
win32 {
    win32-g++ {
        contains(QMAKE_HOST.arch, x86_64):LIBS += -L$${TERRALIBPATH}/dependencies/win64/zlib/lib \
                    -lzlibstat
        else:LIBS += -L$${TERRALIBPATH}/dependencies/win32/zlib/lib \
                    -lzlibdll
    }
    else:QMAKE_LIBDIR += $${TERRALIBPATH}/dependencies/win32/zlib/lib \
                        -lzlibdll
}
unix:LIBS += -lz
LIBS += -ljpeg

INCLUDEPATH	+= $${TERRALIBPATH}/src/libjpeg \
		   $${TERRALIBPATH}/src/zlib
HEADERS += $${TIFFPATH}/cpl_csv.h \
    $${TIFFPATH}/cpl_serv.h \
    $${TIFFPATH}/geo_config.h \
    $${TIFFPATH}/geo_keyp.h \
    $${TIFFPATH}/geo_normalize.h \
    $${TIFFPATH}/geo_tiffp.h \
    $${TIFFPATH}/geokeys.h \
    $${TIFFPATH}/geonames.h \
    $${TIFFPATH}/geotiff.h \
    $${TIFFPATH}/geotiffio.h \
    $${TIFFPATH}/geovalues.h \
    $${TIFFPATH}/port.h \
    $${TIFFPATH}/t4.h \
    $${TIFFPATH}/tif_config.h \
    $${TIFFPATH}/tif_dir.h \
    $${TIFFPATH}/tif_fax3.h \
    $${TIFFPATH}/tif_predict.h \
    $${TIFFPATH}/tiff.h \
    $${TIFFPATH}/tiffcomp.h \
    $${TIFFPATH}/tiffconf.h \
    $${TIFFPATH}/tiffio.h \
    $${TIFFPATH}/tiffiop.h \
    $${TIFFPATH}/tiffvers.h \
    $${TIFFPATH}/uvcode.h \
    $${TIFFPATH}/xtiffio.h \
    $${TIFFPATH}/xtiffiop.h
SOURCES += $${TIFFPATH}/cpl_csv.c \
    $${TIFFPATH}/cpl_serv.c \
    $${TIFFPATH}/geo_extra.c \
    $${TIFFPATH}/geo_free.c \
    $${TIFFPATH}/geo_get.c \
    $${TIFFPATH}/geo_names.c \
    $${TIFFPATH}/geo_new.c \
    $${TIFFPATH}/geo_normalize.c \
    $${TIFFPATH}/geo_print.c \
    $${TIFFPATH}/geo_set.c \
    $${TIFFPATH}/geo_tiffp.c \
    $${TIFFPATH}/geo_trans.c \
    $${TIFFPATH}/geo_write.c \
    $${TIFFPATH}/geotiff_proj4.c \
    $${TIFFPATH}/tif_aux.c \
    $${TIFFPATH}/tif_close.c \
    $${TIFFPATH}/tif_codec.c \
    $${TIFFPATH}/tif_color.c \
    $${TIFFPATH}/tif_compress.c \
    $${TIFFPATH}/tif_dir.c \
    $${TIFFPATH}/tif_dirinfo.c \
    $${TIFFPATH}/tif_dirread.c \
    $${TIFFPATH}/tif_dirwrite.c \
    $${TIFFPATH}/tif_dumpmode.c \
    $${TIFFPATH}/tif_error.c \
    $${TIFFPATH}/tif_extension.c \
    $${TIFFPATH}/tif_fax3.c \
    $${TIFFPATH}/tif_fax3sm.c \
    $${TIFFPATH}/tif_flush.c \
    $${TIFFPATH}/tif_getimage.c \
    $${TIFFPATH}/tif_jpeg.c \
    $${TIFFPATH}/tif_luv.c \
    $${TIFFPATH}/tif_lzw.c \
    $${TIFFPATH}/tif_machdep.c \
    $${TIFFPATH}/tif_next.c \
    $${TIFFPATH}/tif_open.c \
    $${TIFFPATH}/tif_packbits.c \
    $${TIFFPATH}/tif_pixarlog.c \
    $${TIFFPATH}/tif_predict.c \
    $${TIFFPATH}/tif_print.c \
    $${TIFFPATH}/tif_read.c \
    $${TIFFPATH}/tif_strip.c \
    $${TIFFPATH}/tif_swab.c \
    $${TIFFPATH}/tif_thunder.c \
    $${TIFFPATH}/tif_tile.c \
    $${TIFFPATH}/tif_version.c \
    $${TIFFPATH}/tif_warning.c \
    $${TIFFPATH}/tif_write.c \
    $${TIFFPATH}/tif_zip.c \
    $${TIFFPATH}/xtiff.c
win32:SOURCES += $${TIFFPATH}/tif_win32.c 
unix:SOURCES += $${TIFFPATH}/tif_unix.c

include (../install_cfg.pri)

CONFIG(copy_dir_files) {
    include.path = $${DEPLOY_DIR}/include/tiff
    include.files = $${HEADERS}

    INSTALLS += include
}

DEPENDPATH += $${INCLUDEPATH}
