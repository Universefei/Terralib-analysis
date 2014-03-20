#--------------------------------------------------------------
# Installing configurations
# You can choose the path where the the package will be copied.
# To do so set the $DEPLOY_DIR to the path desired.
# Ex : 'qmake DEPLOY_DIR=/home/usr/terralib'(This will copy
# headers and binaries to the folder '/home/usr/terralib').
# Note: use 'install' as argument on make command.
# Note: you can choose to copy also directories. To do so add
# the option CONFIG+=copy_dir_files to qmake command.
#--------------------------------------------------------------
isEmpty(DEPLOY_DIR) {
    DEPLOY_DIR = $${TERRALIBPATH}/br.org.terralib
    unix:DEPLOY_DIR = $${DEPLOY_DIR}/linux-g++
    win32 {
        win32-g++:DEPLOY_DIR = $${DEPLOY_DIR}/win32-g++
        else:DEPLOY_DIR = $${DEPLOY_DIR}/win32-msvc
    }
}
win32 {
    win32-g++ {
        IDIR = $${DESTDIR}/$${TARGET}.dll
        ODIR = $${DEPLOY_DIR}/bin

        IDIR = $$replace(IDIR, /, \\)
        ODIR = $$replace(ODIR, /, \\)

        mdir.commands = $(CHK_DIR_EXISTS) $${ODIR} $(MKDIR) $${ODIR}
        dst.commands = $${QMAKE_COPY} \"$${IDIR}\" \"$${ODIR}\"

        QMAKE_EXTRA_TARGETS += mdir dst
        target.depends = mdir dst
    }
    else:DLLDESTDIR = $${DEPLOY_DIR}/bin
}
!CONFIG(copy_dir_files) {
    headers.files = $${HEADERS}
    headers.path = $${DEPLOY_DIR}/include
    INSTALLS += headers
}
target.path = $${DEPLOY_DIR}/lib
INSTALLS += target
