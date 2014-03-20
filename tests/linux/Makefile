all: 
	qmake -o Makefile_tests;
	make -f Makefile_tests;
  
distclean: 
	qmake -o Makefile_tests;
	make -f Makefile_tests distclean;	
	rm -f Makefile_tests

clean : distclean	
  
runtests :	
	./run_all_tests.sh

check : distclean all runtests	

  
