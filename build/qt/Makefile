all: release

debug: install_deps_dbg qmake_dbg build_dbg

release: install_deps qmake build


#Debug config
#----------------
install_deps_dbg:
	mkdir -p ../../Debug/linux-g++
	cd ../../Debug/linux-g++; rm -f libclntsh.so; ln -s ../../dependencies/linux/Oracle/lib/libclntsh.so.10.1 libclntsh.so
	cd ../../Debug/linux-g++; rm -f libclntsh.so.10.1; ln -s ../../dependencies/linux/Oracle/lib/libclntsh.so.10.1 libclntsh.so.10.1
	cd ../../Debug/linux-g++; rm -f libnnz10.so; ln -s ../../dependencies/linux/Oracle/lib/libnnz10.so libnnz10.so
	cd ../../Debug/linux-g++; rm -f libpq.a; ln -s ../../dependencies/linux/PostgreSQL/lib/libpq.a libpq.a
	cd ../../Debug/linux-g++; rm -f libmysqlclient.a; ln -s ../../dependencies/linux/MySQL/lib/libmysqlclient.a libmysqlclient.a
	cd ../../Debug/linux-g++; rm -f libGL.so; ln -s ../../dependencies/linux/opengl/lib/libGL.so libGL.so
	cd ../../Debug/linux-g++; rm -f libGL.so.1; ln -s ../../dependencies/linux/opengl/lib/libGL.so libGL.so.1
	cd ../../Debug/linux-g++; rm -f libGLU.so; ln -s ../../dependencies/linux/opengl/lib/libGLU.so libGLU.so
	cd ../../Debug/linux-g++; rm -f libGLU.so.1; ln -s ../../dependencies/linux/opengl/lib/libGLU.so libGLU.so.1
	cd ../../Debug/linux-g++; rm -f libdrm.so; ln -s ../../dependencies/linux/opengl/lib/libdrm.so libdrm.so
	cd ../../Debug/linux-g++; rm -f libdrm.so.2; ln -s ../../dependencies/linux/opengl/lib/libdrm.so libdrm.so.2
	cd ../../Debug/linux-g++; rm -f libfbembed.so; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so
	cd ../../Debug/linux-g++; rm -f libfbembed.so.2; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so.2
	cd ../../Debug/linux-g++; rm -f libfbembed.so.2.0.3; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so.2.0.3
	cd ../../Debug/linux-g++; rm -f libicudata.so; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so
	cd ../../Debug/linux-g++; rm -f libicudata.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so.30
	cd ../../Debug/linux-g++; rm -f libicudata.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so.30.0
	cd ../../Debug/linux-g++; rm -f libicui18n.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so.30
	cd ../../Debug/linux-g++; rm -f libicui18n.so; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so
	cd ../../Debug/linux-g++; rm -f libicui18n.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so.30.0
	cd ../../Debug/linux-g++; rm -f libicuuc.so; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so
	cd ../../Debug/linux-g++; rm -f libicuuc.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so.30
	cd ../../Debug/linux-g++; rm -f libicuuc.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so.30.0

qmake_dbg:
	qmake terraView.pro MAKEFILE=Makefile.Debug TE_PROJECT_TYPE=DEBUG TEDRIVERS=ALLDRIVERS
	
build_dbg:
	make -f Makefile.Debug && make -f Makefile.Debug install
	
clean_debug:
	make -f Makefile.Debug distclean; rm -rf ../../../Debug

#Release config
#---------------
install_deps:
	mkdir -p ../../Release/linux-g++
	cd ../../Release/linux-g++; rm -f libclntsh.so; ln -s ../../dependencies/linux/Oracle/lib/libclntsh.so.10.1 libclntsh.so
	cd ../../Release/linux-g++; rm -f libclntsh.so.10.1; ln -s ../../dependencies/linux/Oracle/lib/libclntsh.so.10.1 libclntsh.so.10.1
	cd ../../Release/linux-g++; rm -f libnnz10.so; ln -s ../../dependencies/linux/Oracle/lib/libnnz10.so libnnz10.so
	cd ../../Release/linux-g++; rm -f libpq.a; ln -s ../../dependencies/linux/PostgreSQL/lib/libpq.a libpq.a
	cd ../../Release/linux-g++; rm -f libmysqlclient.a; ln -s ../../dependencies/linux/MySQL/lib/libmysqlclient.a libmysqlclient.a
	cd ../../Release/linux-g++; rm -f libGL.so; ln -s ../../dependencies/linux/opengl/lib/libGL.so libGL.so
	cd ../../Release/linux-g++; rm -f libGL.so.1; ln -s ../../dependencies/linux/opengl/lib/libGL.so libGL.so.1
	cd ../../Release/linux-g++; rm -f libGLU.so; ln -s ../../dependencies/linux/opengl/lib/libGLU.so libGLU.so
	cd ../../Release/linux-g++; rm -f libGLU.so.1; ln -s ../../dependencies/linux/opengl/lib/libGLU.so libGLU.so.1
	cd ../../Release/linux-g++; rm -f libdrm.so; ln -s ../../dependencies/linux/opengl/lib/libdrm.so libdrm.so
	cd ../../Release/linux-g++; rm -f libdrm.so.2; ln -s ../../dependencies/linux/opengl/lib/libdrm.so libdrm.so.2
	cd ../../Release/linux-g++; rm -f libfbembed.so; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so
	cd ../../Release/linux-g++; rm -f libfbembed.so.2; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so.2
	cd ../../Release/linux-g++; rm -f libfbembed.so.2.0.3; ln -s ../../dependencies/linux/Firebird/lib/libfbembed.so libfbembed.so.2.0.3
	cd ../../Release/linux-g++; rm -f libicudata.so; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so
	cd ../../Release/linux-g++; rm -f libicudata.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so.30
	cd ../../Release/linux-g++; rm -f libicudata.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicudata.so.30 libicudata.so.30.0
	cd ../../Release/linux-g++; rm -f libicui18n.so; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so
	cd ../../Release/linux-g++; rm -f libicui18n.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so.30
	cd ../../Release/linux-g++; rm -f libicui18n.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicui18n.so.30 libicui18n.so.30.0
	cd ../../Release/linux-g++; rm -f libicuuc.so; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so
	cd ../../Release/linux-g++; rm -f libicuuc.so.30; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so.30
	cd ../../Release/linux-g++; rm -f libicuuc.so.30.0; ln -s ../../dependencies/linux/Firebird/lib/libicuuc.so.30 libicuuc.so.30.0

qmake:
	qmake terraView.pro MAKEFILE=Makefile.Release TEDRIVERS=ALLDRIVERS
	
build:
	make -f Makefile.Release && make -f Makefile.Release install
	
clean:
	make -f Makefile.Release distclean; rm -rf ../../Release

