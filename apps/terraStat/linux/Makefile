all: terraStat

shapelib:
	cd ../../../terralibx/shapelib; make
	cp ../../../terralibx/shapelib/libshapelib.so.1 .

tiff:
	cd ../../../terralibx/tiff; make
	cp ../../../terralibx/tiff/libtiff.so.1 .

terralib:
	cd ../../../terralibx/terralib; make
	cp ../../../terralibx/terralib/libterralib.so.1 .

stat:
	cd ../../../terralibx/stat; make
	cp ../../../terralibx/stat/libstat.so.1 .

terraStat: shapelib tiff terralib stat
	qmake -o makeTerraStat terraStat.pro; make -f makeTerraStat

clean:
	cd ../../../terralibx/shapelib; make clean
	cd ../../../terralibx/tiff; make clean
	cd ../../../terralibx/terralib; make clean
	cd ../../../terralibx/stat; make clean
	make -f makeTerraStat distclean; rm -f makeTerraStat libshapelib.so.1 libtiff.so.1 libterralib.so.1 libstat.so.1
