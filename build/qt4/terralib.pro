#-------------------------------------------
# To configure compilation, you can select values for TEDRIVERS variable. TEDRIVERS = ALLDRIVERS compile full terralib drivers.
# Other values are HAS_FIREBIRD, HAS_MYSQL, HAS_ORACLE, HAS_POSTGRESQL, HAS_QWT, HAS_SHP, HAS_SPL, HAS_PDI.
# To customize drivers you can use for example: TEDRIVERS="HAS_ORACLE HAS_QWT ..." 
#-------------------------------------------
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = ./libjpeg \
        ./shapelib \
        ./tiff \
        ./terralib \
        ./te_functions \
        ./te_utils
contains(TEDRIVERS, ALLDRIVERS) {
	SUBDIRS += ./Firebird \
            ./MySQL
        !contains(QMAKE_HOST.arch, x86_64):SUBDIRS+=./Oracle
        SUBDIRS+=./PostgreSQL \
            ./te_shapelib
        !contains(QMAKE_HOST.arch, x86_64):SUBDIRS+=./libspl \
            ./te_spl
        SUBDIRS += ./dxflib \
            ./te_dxf
}
else {
    contains(TEDRIVERS, HAS_PDI):SUBDIRS += ./image_processing
    contains(TEDRIVERS, HAS_FIREBIRD):SUBDIRS += ./Firebird
    contains(TEDRIVERS, HAS_MYSQL):SUBDIRS += ./MySQL
    contains(TEDRIVERS, HAS_ORACLE):SUBDIRS += ./Oracle
    contains(TEDRIVERS, HAS_POSTGRESQL):SUBDIRS += ./PostgreSQL
    contains(TEDRIVERS, HAS_QWT):SUBDIRS += ./qwt ./te_qwt
    contains(TEDRIVERS, HAS_SHP):SUBDIRS += ./te_shapelib
    contains(TEDRIVERS, HAS_SPL):SUBDIRS += ./libspl ./te_spl
    contains(TEDRIVERS, HAS_DXF):SUBDIRS += ./dxflib ./te_dxf
}
