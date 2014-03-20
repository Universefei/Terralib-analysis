#------------------------------------------------
# General
#------------------------------------------------
EXTENSION_ID = br.org.terralib
EXTENSION_VERSION = 4.0.0
include(../extension.pri)
#------------------------------------------------
# Specific Settings
#------------------------------------------------
QT -= core gui

QMAKE_POST_LINK = $$collectGarbage(org)
#------------------------------------------------
# Extensions
#------------------------------------------------

unix {
    make_links.commands = cd lib && ln -sf libclntsh.so.10.1 libclntsh.so \
                    && ln -sf libicudata.so.30 libicudata.so && ln -sf libicui18n.so.30 libicui18n.so \
					&& ln -sf libicuuc.so.30 libicuuc.so
	QMAKE_EXTRA_UNIX_TARGETS += make_links
	POST_TARGETDEPS += make_links
}
