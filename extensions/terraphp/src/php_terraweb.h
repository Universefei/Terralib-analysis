#ifndef PHP_TERRAWEB_H
#define PHP_TERRAWEB_H

extern zend_module_entry terraweb_module_entry;
#define phpext_terraweb_ptr &terraweb_module_entry

#ifdef PHP_WIN32
#define PHP_TERRAWEB_API __declspec(dllexport)
#else
#define PHP_TERRAWEB_API
#endif

#ifdef ZTS

#include "TSRM.h"

#endif

PHP_MINIT_FUNCTION(terraweb);
PHP_MSHUTDOWN_FUNCTION(terraweb);
PHP_RINIT_FUNCTION(terraweb);
PHP_RSHUTDOWN_FUNCTION(terraweb);
PHP_MINFO_FUNCTION(terraweb);

PHP_FUNCTION(terraweb);							/* Constructor */
PHP_FUNCTION(terraweb_errormessage);			/* Returns error message when a method returns false */
PHP_FUNCTION(terraweb_connect);					/* Opens a connection to a database server with parameters and returns TRUE with success otherwise returns FALSE */
PHP_FUNCTION(terraweb_getviews);				/* Returns an array with the names for yhe user view in the current database */
PHP_FUNCTION(terraweb_setcurrentview);			/* Set the current view name */
PHP_FUNCTION(terraweb_getcurrentview);			/* Returns the current view name */
PHP_FUNCTION(terraweb_getcurrentviewbox);		/* Returns an array with the view MBR - with the lower left corner x and y (x1 and y1) and the upper right corner x and y (x2 and y2) - including all themes boxes in the view projection */
PHP_FUNCTION(terraweb_setcanvasbackgroundcolor);
PHP_FUNCTION(terraweb_setworld);				/* Sets the image output  size (width x height) and the data world box, and returns the adjusted world box as an array with the lower left corner x and y (x1 and y1) and the upper right corner (x2 and y2) */
PHP_FUNCTION(terraweb_settheme);				/* Set the current or reference theme */
PHP_FUNCTION(terraweb_gettheme);				/* Returns the current or refernce theme */
PHP_FUNCTION(terraweb_getthemerepresentation);			/* Returns the current or reference theme visible representations */
PHP_FUNCTION(terraweb_getthemes);							/* Returns the current view themes */
PHP_FUNCTION(terraweb_setactivethemerepresentation);		/* Sets the current or reference theme default representation */ 
PHP_FUNCTION(terraweb_getactivethemerepresentation);		/* Gets the current or reference theme default representation */
PHP_FUNCTION(terraweb_getthemebox);							/* Returns a theme box */
PHP_FUNCTION(terraweb_locateobject);						/* Returns the object identifier (objectId) and the geometry identifier (geomId) of a object that contains the point pt */
PHP_FUNCTION(terraweb_locateobjectsinbox);					/* Given a box and a relation find objects related to the box in the current or reference theme, the objectid of found objects will be in set objIds */
PHP_FUNCTION(terraweb_locateobjects);						/* Given an objectid and a relation find objects related in the current or reference theme */
PHP_FUNCTION(terraweb_locateobjectsinarray);				/* Given objectids and a relation find objects related in the current or reference theme */
PHP_FUNCTION(terraweb_locateobjectsusingreftheme);			/* Given an objectid in the current theme find objects related in the reference theme */
PHP_FUNCTION(terraweb_locateobjectinarraysusingreftheme);	/* Given objectids and a relation find objects from current theme related to objects in reference theme */
PHP_FUNCTION(terraweb_fetchattributes);			/* Returns the object attributes given its objectId */
PHP_FUNCTION(terraweb_pixelsize);				/* Pixel size in world coordinates */
PHP_FUNCTION(terraweb_getcanvasimage);			/* Returns the canvas image in the specified format */
PHP_FUNCTION(terraweb_getlegendimage);			/* Returns the canvas legend PNG image */
PHP_FUNCTION(terraweb_setdefaultvisual);		/* Sets the visual of selected objects to draw */
PHP_FUNCTION(terraweb_drawpoint);				/* Draw a point on canvas */
PHP_FUNCTION(terraweb_drawline);				/*  */
PHP_FUNCTION(terraweb_drawpolygon);				/*  */
PHP_FUNCTION(terraweb_drawbox);					/* Draw a box on canvas, box coordinates must be on view coordinates */
PHP_FUNCTION(terraweb_drawcurrenttheme);		/* Draw current theme content on canvas */
PHP_FUNCTION(terraweb_drawsql);					/*  */
PHP_FUNCTION(terraweb_drawgroupsql);			/*  */
PHP_FUNCTION(terraweb_drawselectedobjects);		/* Draw the selected objects in objIdVector */
PHP_FUNCTION(terraweb_drawbufferzone);			/* Generates a buffer zone around the object with objectId */
PHP_FUNCTION(terraweb_drawtext);				/* Draw a text on canvas */
PHP_FUNCTION(terraweb_drawlegend);				/* Draw legend*/
PHP_FUNCTION(terraweb_savecanvasimage);			/*  */
PHP_FUNCTION(terraweb_savelegendimage);			/*  */
PHP_FUNCTION(terraweb_cleancanvas);				/*  */
PHP_FUNCTION(terraweb_close);					/*  */
PHP_FUNCTION(terraweb_getscale);				/*  */
PHP_FUNCTION(terraweb_setautomaticscalecontrolenable);	/*  */
PHP_FUNCTION(terraweb_setconflictdetect);	/*  */
PHP_FUNCTION(terraweb_setpriorityfield);	/*  */
PHP_FUNCTION(terraweb_setmincollisiontolerance);	/*  */
PHP_FUNCTION(terraweb_clearconflictcache);	/*  */
PHP_FUNCTION(terraweb_setgeneralizedpixels); /**/
PHP_FUNCTION(terraweb_setlabelfield);	/*  */
PHP_FUNCTION(terraweb_settextoutlineenable);	/*  */
PHP_FUNCTION(terraweb_settextoutlinecolor);		/*  */
PHP_FUNCTION(terraweb_setimagemapproperties);			/*  */
PHP_FUNCTION(terraweb_closeimagemap);					/*  */
PHP_FUNCTION(terraweb_setareashape);					/*  */
PHP_FUNCTION(terraweb_setareaproperty);					/*  */
PHP_FUNCTION(terraweb_setareacircleradius);				/*  */
PHP_FUNCTION(terraweb_setarearectwidth);				/*  */
PHP_FUNCTION(terraweb_getimagemap);						/*  */
PHP_FUNCTION(terraweb_drawcartogram);					/*  */
PHP_FUNCTION(terraweb_savethemetofile);					/*  */
PHP_FUNCTION(terraweb_setthemevisual);					/*  */
PHP_FUNCTION(terraweb_drawtext2);						/*  */

/*THE METHODS BELOW ARE ALL DEPRECATED*/
PHP_FUNCTION(terraweb_getpng);					
PHP_FUNCTION(terraweb_getjpg);					
PHP_FUNCTION(terraweb_getlegendpng);				
//PHP_FUNCTION(terraweb_plotpoint);				
//PHP_FUNCTION(terraweb_plotbox);					
//PHP_FUNCTION(terraweb_plotvector);				
//PHP_FUNCTION(terraweb_plotraster);				
//PHP_FUNCTION(terraweb_plotselectedobjects);			
//PHP_FUNCTION(terraweb_plotbufferzone);				
//PHP_FUNCTION(terraweb_plottext);				
//PHP_FUNCTION(terraweb_plotlegend);				
PHP_FUNCTION(terraweb_saveplot);
PHP_FUNCTION(terraweb_saveimage);
PHP_FUNCTION(terraweb_savelegend);
//PHP_FUNCTION(terraweb_plotline);
//PHP_FUNCTION(terraweb_plotpolygon);
//PHP_FUNCTION(terraweb_plotsql);
//PHP_FUNCTION(terraweb_plotgroupsql);






static void destroy_terraweb(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(terraweb)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(terraweb)
*/

/* In every utility function you add that needs to use variables 
   in php_terraweb_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as TERRAWEB_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define TERRAWEB_G(v) TSRMG(terraweb_globals_id, zend_terraweb_globals *, v)
#else
#define TERRAWEB_G(v) (terraweb_globals.v)
#endif

#endif	/* PHP_TERRAWEB_H */
