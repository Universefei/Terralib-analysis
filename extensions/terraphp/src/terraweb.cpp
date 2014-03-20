#ifdef HAVE_CONFIG_H

#include "config.h"

#endif

#ifdef PHP_WIN32
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include <set>
#endif

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/php_rand.h"
#include "ext/standard/php_lcg.h"
}

#include "php_terraweb.h"
#include "TerraManager.h"
//#include <TeBox.h>

/* If you declare any globals in php_terraweb.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(terraweb)
*/

/* True global resources - no need for thread safety here */
/* Note 1 -- here's the zend_class_entry. You'll also
need a list entry for the resources used to store the
actual C++ objects... */
static zend_class_entry terraweb_entry;
static int le_terraweb;

/* Note 2 -- all of the methods go into this
function_entry. Below are the actual methods that
map onto terraweb. */
/* {{{ terraweb_functions[]
 *
 * Every user visible function must have an entry in terraweb_functions[].
 */
function_entry terraweb_functions[] = {
	PHP_FE(terraweb,					           NULL)
	PHP_FE(terraweb_errormessage,           	   NULL)
	PHP_FE(terraweb_connect,				       NULL)
	PHP_FE(terraweb_getviews,       			   NULL)
	PHP_FE(terraweb_setcurrentview,				   NULL)
	PHP_FE(terraweb_getcurrentview,				   NULL)
	PHP_FE(terraweb_getcurrentviewbox,			   NULL)
	PHP_FE(terraweb_setcanvasbackgroundcolor,      NULL)
	PHP_FE(terraweb_setworld,				       NULL)
	PHP_FE(terraweb_settheme,				       NULL)
	PHP_FE(terraweb_gettheme,				       NULL)
	PHP_FE(terraweb_getthemerepresentation,		   NULL)
	PHP_FE(terraweb_getthemes,      			   NULL)
	PHP_FE(terraweb_setactivethemerepresentation,  NULL)
	PHP_FE(terraweb_getactivethemerepresentation,  NULL)
	PHP_FE(terraweb_getthemebox,				   NULL)	
	PHP_FE(terraweb_locateobject,				   NULL)
	PHP_FE(terraweb_locateobjectsinbox,	  		   NULL)
	PHP_FE(terraweb_locateobjects,			   	   NULL)
	PHP_FE(terraweb_locateobjectsinarray,		   NULL)
	PHP_FE(terraweb_locateobjectsusingreftheme,	   NULL)
	PHP_FE(terraweb_locateobjectinarraysusingreftheme, 	NULL)
	PHP_FE(terraweb_fetchattributes,		 NULL)
	PHP_FE(terraweb_pixelsize,				 NULL)
	PHP_FE(terraweb_getcanvasimage,			 NULL)
	PHP_FE(terraweb_getlegendimage,			 NULL)
	PHP_FE(terraweb_setdefaultvisual,		 NULL)
	PHP_FE(terraweb_drawpoint,				 NULL)
	PHP_FE(terraweb_drawline,                NULL)
	PHP_FE(terraweb_drawpolygon,             NULL)
	PHP_FE(terraweb_drawbox,				 NULL)
	PHP_FE(terraweb_drawcurrenttheme,		 NULL)
	PHP_FE(terraweb_drawsql,                 NULL)
	PHP_FE(terraweb_drawgroupsql,            NULL)
	PHP_FE(terraweb_drawselectedobjects,	 NULL)
	PHP_FE(terraweb_drawbufferzone,			 NULL)
	PHP_FE(terraweb_drawtext,				 NULL)
	PHP_FE(terraweb_drawlegend,				 NULL)
    PHP_FE(terraweb_savecanvasimage,		 NULL)
	PHP_FE(terraweb_savelegendimage,		 NULL)
	PHP_FE(terraweb_cleancanvas,			 NULL)
	PHP_FE(terraweb_close,                   NULL)
	PHP_FE(terraweb_getscale,                NULL)
	PHP_FE(terraweb_setautomaticscalecontrolenable,  NULL)
	PHP_FE(terraweb_setconflictdetect,  NULL)
	PHP_FE(terraweb_setpriorityfield,    NULL)
	PHP_FE(terraweb_setmincollisiontolerance,   NULL)
	PHP_FE(terraweb_clearconflictcache,         NULL)
	PHP_FE(terraweb_setgeneralizedpixels,       NULL)
	PHP_FE(terraweb_setlabelfield,              NULL)
	PHP_FE(terraweb_settextoutlineenable,    NULL)
	PHP_FE(terraweb_settextoutlinecolor,     NULL)
	PHP_FE(terraweb_setimagemapproperties,       NULL)
	PHP_FE(terraweb_closeimagemap,               NULL)
	PHP_FE(terraweb_setareashape,                NULL)
	PHP_FE(terraweb_setareaproperty,             NULL)
	PHP_FE(terraweb_setareacircleradius,         NULL)
	PHP_FE(terraweb_setarearectwidth,            NULL)
	PHP_FE(terraweb_getimagemap,                 NULL)
	PHP_FE(terraweb_drawcartogram,               NULL)
	PHP_FE(terraweb_savethemetofile,             NULL)       
	PHP_FE(terraweb_setthemevisual,               NULL)
	PHP_FE(terraweb_drawtext2,				 NULL)
	/*ALL FUNCTIONS BELOW ARE DEPRECATED*/
	PHP_FE(terraweb_getpng,               NULL)
	PHP_FE(terraweb_getjpg,               NULL)
	PHP_FE(terraweb_getlegendpng,               NULL)
	PHP_FE(terraweb_saveplot,               NULL)
	PHP_FE(terraweb_saveimage,               NULL)
	PHP_FE(terraweb_savelegend,               NULL)	
	{NULL, NULL, NULL}	/* Must be the last line in terraweb_functions[] */
};

static function_entry terraweb_methods[] = {
	{"errormessage",		 	PHP_FN(terraweb_errormessage),				NULL},
	{"connect",			 	PHP_FN(terraweb_connect),				NULL},
	{"getviews",			 	PHP_FN(terraweb_getviews),         			NULL},
	{"setcurrentview",		 	PHP_FN(terraweb_setcurrentview),    			NULL},
	{"getcurrentview",		 	PHP_FN(terraweb_getcurrentview),    			NULL},
	{"getcurrentviewbox",		 	PHP_FN(terraweb_getcurrentviewbox),     		NULL},
	{"setcanvasbackgroundcolor",    PHP_FN(terraweb_setcanvasbackgroundcolor),      NULL},
	{"setworld",			 	PHP_FN(terraweb_setworld),				NULL},
	{"settheme", 			 	PHP_FN(terraweb_settheme),       			NULL},
	{"gettheme", 			 	PHP_FN(terraweb_gettheme),       			NULL},
	{"getthemerepresentation",	 	PHP_FN(terraweb_getthemerepresentation),		NULL},
	{"getthemes", 			 	PHP_FN(terraweb_getthemes),       			NULL},
	{"setactivethemerepresentation", 	PHP_FN(terraweb_setactivethemerepresentation),		NULL},
	{"getactivethemerepresentation", 	PHP_FN(terraweb_getactivethemerepresentation),		NULL},
	{"getthemebox",				PHP_FN(terraweb_getthemebox),				NULL},
	{"locateobject",		 	PHP_FN(terraweb_locateobject),       			NULL},
	{"locateobjectsinbox",		 	PHP_FN(terraweb_locateobjectsinbox),			NULL},
	{"locateobjects",       	 	PHP_FN(terraweb_locateobjects),				NULL},
	{"locateobjectsinarray",	 	PHP_FN(terraweb_locateobjectsinarray),			NULL},
	{"locateobjectsusingreftheme",	 	PHP_FN(terraweb_locateobjectsusingreftheme),   		NULL},
	{"locateobjectinarraysusingreftheme", 	PHP_FN(terraweb_locateobjectinarraysusingreftheme),	NULL},
	{"fetchattributes",		 	PHP_FN(terraweb_fetchattributes),       NULL},
	{"pixelsize",			 	PHP_FN(terraweb_pixelsize),				NULL},	
	{"getcanvasimage",			 	PHP_FN(terraweb_getcanvasimage),			NULL},
	{"getlegendimage",			 	PHP_FN(terraweb_getlegendimage),			NULL},
	{"setdefaultvisual",		 	PHP_FN(terraweb_setdefaultvisual),			NULL},
	{"drawpoint",			 	PHP_FN(terraweb_drawpoint),		  				NULL},
	{"drawline",			 	PHP_FN(terraweb_drawline),		  				NULL},
	{"drawpolygon",			 	PHP_FN(terraweb_drawpolygon),		  			NULL},
	{"drawbox",			 	PHP_FN(terraweb_drawbox),							NULL},
	{"drawcurrenttheme",			 	PHP_FN(terraweb_drawcurrenttheme),		NULL},
	{"drawsql",			 	PHP_FN(terraweb_drawsql),							NULL},
	{"drawgroupsql",			PHP_FN(terraweb_drawgroupsql),					NULL},
	{"drawselectedobjects",			 	PHP_FN(terraweb_drawselectedobjects),	NULL},
	{"drawbufferzone",			PHP_FN(terraweb_drawbufferzone),			NULL},
	{"drawtext",			PHP_FN(terraweb_drawtext),						NULL},
	{"drawlegend",				PHP_FN(terraweb_drawlegend),				NULL},
	{"savecanvasimage",				PHP_FN(terraweb_savecanvasimage),		NULL},
	{"savelegendimage",				PHP_FN(terraweb_savelegendimage),		NULL},
	{"cleancanvas",				PHP_FN(terraweb_cleancanvas),	NULL},
	{"close",			 	PHP_FN(terraweb_close),				NULL},
	{"getscale",            PHP_FN(terraweb_getscale),          NULL},
	{"setautomaticscalecontrolenable", PHP_FN(terraweb_setautomaticscalecontrolenable),          NULL},	
	{"setconflictdetect", PHP_FN(terraweb_setconflictdetect), NULL},
	{"setpriorityfield", PHP_FN(terraweb_setpriorityfield),     NULL},
	{"setmincollisiontolerance", PHP_FN(terraweb_setmincollisiontolerance),   NULL},
	{"clearconflictcache", PHP_FN(terraweb_clearconflictcache),               NULL},	
	{"setgeneralizedpixels", PHP_FN(terraweb_setgeneralizedpixels),           NULL},
	{"setlabelfield", PHP_FN(terraweb_setlabelfield),                         NULL},	
	{"settextoutlineenable",		 	PHP_FN(terraweb_settextoutlineenable),	NULL},
	{"settextoutlinecolor",		 	PHP_FN(terraweb_settextoutlinecolor),		NULL},
	{"setimagemapproperties", PHP_FN(terraweb_setimagemapproperties),       NULL},
	{"closeimagemap", PHP_FN(terraweb_closeimagemap),         				NULL},	
	{"setareashape", PHP_FN(terraweb_setareashape),          				NULL},
	{"setareaproperty", PHP_FN(terraweb_setareaproperty),				    NULL},
	{"setareacircleradius", PHP_FN(terraweb_setareacircleradius),          	NULL},
	{"setarearectwidth", PHP_FN(terraweb_setarearectwidth),          		NULL},
	{"getimagemap", PHP_FN(terraweb_getimagemap),          					NULL},
	{"drawcartogram", PHP_FN(terraweb_drawcartogram),          				NULL},
	{"savethemetofile", PHP_FN(terraweb_savethemetofile),					NULL},
	{"setthemevisual", PHP_FN(terraweb_setthemevisual),          				NULL},
	{"drawtext2",			PHP_FN(terraweb_drawtext2),						NULL},
	/* DEPRECATED*/
	{"getpng", PHP_FN(terraweb_getpng),          				NULL},
	{"getjpg", PHP_FN(terraweb_getjpg),          				NULL},
	{"getlegendpng", PHP_FN(terraweb_getlegendpng),          				NULL},
	{"plotpoint", PHP_FN(terraweb_drawpoint),          				NULL},
	{"plotbox", PHP_FN(terraweb_drawbox),          				NULL},
	{"plotvector", PHP_FN(terraweb_drawcurrenttheme),          				NULL},
	{"plotraster", PHP_FN(terraweb_drawcurrenttheme),          				NULL},
	{"plotselectedobjects", PHP_FN(terraweb_drawselectedobjects),          				NULL},
	{"plotbufferzone", PHP_FN(terraweb_drawbufferzone),          				NULL},
	{"plottext", PHP_FN(terraweb_drawtext),          				NULL},
	{"plotlegend", PHP_FN(terraweb_drawlegend),          				NULL},
	{"saveplot", PHP_FN(terraweb_saveplot),          				NULL},
	{"saveimage", PHP_FN(terraweb_saveimage),          				NULL},
	{"savelegend", PHP_FN(terraweb_savelegend),          				NULL},
	{"plotline", PHP_FN(terraweb_drawline),          				NULL},
	{"plotpolygon", PHP_FN(terraweb_drawpolygon),          				NULL},
	{"plotsql", PHP_FN(terraweb_drawsql),          				NULL},
	{"plotgroupsql", PHP_FN(terraweb_drawgroupsql),          				NULL},
	{NULL, NULL, NULL}
    };
/* }}} */

/* {{{ terraweb_module_entry
 */
zend_module_entry terraweb_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"terraweb",
	terraweb_functions,
	PHP_MINIT(terraweb),
	PHP_MSHUTDOWN(terraweb),
	PHP_RINIT(terraweb),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(terraweb),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(terraweb),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TERRAWEB
BEGIN_EXTERN_C()
ZEND_GET_MODULE(terraweb)
END_EXTERN_C()
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("terraweb.global_value",      "42", PHP_INI_ALL, OnUpdateInt, global_value, zend_terraweb_globals, terraweb_globals)
    STD_PHP_INI_ENTRY("terraweb.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_terraweb_globals, terraweb_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_terraweb_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_terraweb_init_globals(zend_terraweb_globals *terraweb_globals)
{
	terraweb_globals->global_value = 0;
	terraweb_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(terraweb)
{
	//If you have INI entries, uncomment these lines
	//ZEND_INIT_MODULE_GLOBALS(terraweb, php_terraweb_init_globals, NULL);
	//REGISTER_INI_ENTRIES();
	
	le_terraweb = zend_register_list_destructors_ex(destroy_terraweb, NULL, "terraweb-obj", module_number);

	INIT_CLASS_ENTRY(terraweb_entry, "terraweb", terraweb_methods);
	zend_register_internal_class(&terraweb_entry TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(terraweb)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(terraweb)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(terraweb)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(terraweb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "terraweb support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */



/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/
/* {{{ proto terraweb object terraweb()
       Returns a new terraweb object. This is basically the constructor. */
PHP_FUNCTION(terraweb)
{
	/* Note 4 -- here's the constructor. First, initalize return_value as an
       object, then make a handle to the C++ object. Register the handle as a resource
       and store the resource within the PHP object so we can get to it in the method
       calls.
	*/
    
    if(ZEND_NUM_ARGS() != 0)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	TeMANAGER::TerraManager *obj;
        
	zval *handle;
		
    obj = new TeMANAGER::TerraManager();

    object_init_ex(return_value, &terraweb_entry);
	
    MAKE_STD_ZVAL(handle);
        
	ZEND_REGISTER_RESOURCE(handle, obj, le_terraweb);
	
	zend_hash_update(Z_OBJPROP_P(return_value), "handle", sizeof("handle") + 1, &handle, sizeof(zval *), NULL);
}
/* }}} */


/* Note 5 -- this macro gets the resource handle for the C++ object so we can
   use the C++ object in our methods mappings below. */

#define CPP_GET_THIS() \
        TeMANAGER::TerraManager *obj; \
        zval *id, **handle; \
        int type; \
        id = getThis(); \
        if(!id) { \
            php_error(E_ERROR, "this isn't an object?! %s()", get_active_function_name(TSRMLS_C)); \
        } \
        if (zend_hash_find(Z_OBJPROP_P(id), "handle", sizeof("handle")+1, (void **) &handle) == FAILURE) { \
            php_error(E_ERROR, "underlying object missing, can't find 'this'(%d) in %s()", id,get_active_function_name(TSRMLS_C)); \
        } \
        obj = (TeMANAGER::TerraManager *) zend_list_find(Z_LVAL_PP(handle), &type); \
        if (!obj || type != le_terraweb) { \
            php_error(E_ERROR, "underlying object is of wrong type in %s()", get_active_function_name(TSRMLS_C)); \
        }


PHP_FUNCTION(terraweb_errormessage)
{
	int argc = ZEND_NUM_ARGS();

	if(argc != 0)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	CPP_GET_THIS();

	string errorMsg = obj->getErrorMessage();

	RETVAL_STRING((char*)(errorMsg.c_str()), 1);
}

PHP_FUNCTION(terraweb_connect)
{
	char *host = NULL;
	char *user = NULL;
	char *password = NULL;
	char *database = NULL;        
	int argc = ZEND_NUM_ARGS();
	int host_len;
	int user_len;
	int password_len;
	int database_len;
	long port = -1;
	long dbType = 1;
	
	if(argc < 4 || argc > 6)
	{	
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}		

	if(zend_parse_parameters(argc TSRMLS_CC, "ssss|l|l", &host, &host_len, &user, &user_len, &password, &password_len, &database, &database_len, &port, &dbType) == FAILURE)
	{
		RETVAL_FALSE;		
		return;
	}

	string strHost = "";

	if(host != NULL)
		strHost = host;

	string strUser = "";

	if(user != NULL)
		strUser = user;

	string strPassword = "";

	if(password != NULL)
		strPassword = password;

	string strDatabase = "";

	if(database != NULL)
		strDatabase = database;

	CPP_GET_THIS();	
	
	try
	{
		obj->connect((TeMANAGER::TeDBMSType)dbType, strHost, strUser, strPassword, strDatabase, port);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_getviews)
{
	char *user = NULL;
	int user_len;

	int argc = ZEND_NUM_ARGS();

	if(argc > 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "|s", &user, &user_len) == FAILURE)
	{
		RETVAL_FALSE;
		
		return;
	}
	
	CPP_GET_THIS();

	string userName = "";

	if(user != NULL)
	  userName = user;
	
	vector<string> views;
	try
	{
		obj->getViews(views, userName);
		
		array_init(return_value);
		
		for(unsigned int i = 0; i < views.size(); ++i)
			add_next_index_string(return_value, (char*)(views[i].c_str()), 1);
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_setcurrentview)
{
	char *view = NULL;
	int argc = ZEND_NUM_ARGS();
	int view_len;

	if(argc != 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "s", &view, &view_len) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	string strView = "";

	if(view != NULL)
		strView = view;

	CPP_GET_THIS();

	try
	{
		obj->setCurrentView(strView);
		obj->setWorkProjection(*(obj->getCurrentView()->projection())) ;
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
	
}

PHP_FUNCTION(terraweb_getcurrentview)
{
	if(ZEND_NUM_ARGS() != 0)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
	
	CPP_GET_THIS();
	
	string viewName = obj->getCurrentViewName();
	
	if(viewName.empty())
	{
		RETVAL_FALSE;
	}
	else
	{
		RETVAL_STRING((char*)(viewName.c_str()), 1);
	}
}

PHP_FUNCTION(terraweb_getcurrentviewbox)
{
	if(ZEND_NUM_ARGS() != 0)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
	
	CPP_GET_THIS();

	TeBox box;

	try
	{
		obj->getCurrentViewBox(box);
		
		array_init(return_value);
	
		add_next_index_double(return_value, box.x1());
		add_next_index_double(return_value, box.y1());
		add_next_index_double(return_value, box.x2());
		add_next_index_double(return_value, box.y2());	
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_setcanvasbackgroundcolor)
{
	int argc = ZEND_NUM_ARGS();
	int r;
	int g;
	int b;

	if(argc != 3)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "lll", &r, &g, &b) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	//php_error(E_WARNING, "setworld: not yet implemented");
	obj->setCanvasBackgroundColor(r, g, b);
	
	RETVAL_TRUE;
}


PHP_FUNCTION(terraweb_setworld)
{
	int argc = ZEND_NUM_ARGS();
	long width;
	long height;
	double x1;
	double y1;
	double x2;
	double y2;

	if(argc != 6)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "ddddll", &x1, &y1, &x2, &y2, &width, &height) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	//php_error(E_WARNING, "setworld: not yet implemented");
	TeBox box = obj->setWorld(x1, y1, x2, y2, width, height);
	
	array_init(return_value);
	
	add_next_index_double(return_value, box.x1());
	add_next_index_double(return_value, box.y1());
	add_next_index_double(return_value, box.x2());
	add_next_index_double(return_value, box.y2());
}

PHP_FUNCTION(terraweb_settheme)
{
	char *theme = NULL;	
	int themeLen;
	int themeType = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc > 2)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "s|l", &theme, &themeLen, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	string strTheme = "";

	if(theme != NULL)
		strTheme = theme;

	CPP_GET_THIS();	

	try
	{
		obj->setTheme(strTheme, themeType);
		
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_gettheme)
{
	int argc = ZEND_NUM_ARGS();

	int themeType = 0;

	if(argc > 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "|l", &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}
	
	CPP_GET_THIS();	

	string themeName = obj->getThemeName(themeType);

	if(themeName.empty())
	{
		RETVAL_FALSE;
	}
	else
	{
		RETVAL_STRING((char*)(themeName.c_str()), 1);
	}
}

PHP_FUNCTION(terraweb_getthemerepresentation)
{
	int argc = ZEND_NUM_ARGS();

	int themeType = 0;

	if(argc > 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "|l", &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}
	
	CPP_GET_THIS();
	
	int repres = obj->getThemeRepresentation(themeType);

	RETVAL_LONG(repres);
}

PHP_FUNCTION(terraweb_getthemes)
{
	int argc = ZEND_NUM_ARGS();

	if(argc > 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
	
	int visible = 0;
	
	
	if(zend_parse_parameters(argc TSRMLS_CC, "|l", &visible) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();
	
	vector<string> themes;
	
	try
	{
		obj->getThemes(themes, ((visible == 0) ? false : true));
	
		
		array_init(return_value);

		for(unsigned int i = 0; i < themes.size(); ++i)
			add_next_index_string(return_value, (char*)(themes[i].c_str()), 1);
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_setactivethemerepresentation)
{
	int repres;	
	int themeType = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc > 2 || argc < 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l|l", &repres, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	try
	{
		obj->setActiveRepresentation(repres, themeType);
		
		RETVAL_TRUE;	
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_getactivethemerepresentation)
{
	int argc = ZEND_NUM_ARGS();

	int themeType = 0;

	if(argc > 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "|l", &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}
	
	CPP_GET_THIS();
	
	int repres = obj->getActiveRepresentation(themeType);

	RETVAL_LONG(repres);
}

PHP_FUNCTION(terraweb_getthemebox)
{
	char *restriction = NULL;
	int restrictionLen = 0;

	int themeType = 0;	

	int argc = ZEND_NUM_ARGS();

	if((argc != 0) && (argc != 1) && (argc != 2))
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "|d|s", &themeType, &restriction, &restrictionLen) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}
	
	CPP_GET_THIS();	

	string strRestric;

	if(restriction != NULL)
		strRestric = restriction;	

	try
	{
		TeBox box = obj->getThemeBox(themeType, strRestric);

		array_init(return_value);
	
		add_next_index_double(return_value, box.x1());
		add_next_index_double(return_value, box.y1());
		add_next_index_double(return_value, box.x2());
		add_next_index_double(return_value, box.y2());	
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobject)
{
	int argc = ZEND_NUM_ARGS();

	int themeType = 0;
	double x;
	double y;
	double tol = 0.0;

	if((argc > 4) || (argc < 2))
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "dd|d|l", &x, &y, &tol, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}
	
	CPP_GET_THIS();

	TeCoord2D c(x, y);

	pair<string, string> identification;
	
	try
	{
		obj->locateObject(identification, c, tol, themeType);
		
		array_init(return_value);
		add_assoc_string( return_value, "object_id", (char*)(identification.first.c_str()), 1 );
		add_assoc_string( return_value, "geom_id", (char*)(identification.second.c_str()), 1 );
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobjectsinbox)
{
	int argc = ZEND_NUM_ARGS();
	int themeType = 0;
	long relation = 64;
	double x1;
	double y1;
	double x2;
	double y2;

	if(argc < 4 || argc > 6)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "dddd|l|l", &x1, &y1, &x2, &y2, &relation, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	TeBox box(x1, y1, x2, y2);
	TeKeys objIds;

	try
	{
		obj->locateObjects(box, relation, objIds, themeType);

		array_init(return_value);

		TeKeys::iterator it = objIds.begin();

		while(it != objIds.end())
		{
			string objstr = *it;

			add_next_index_string(return_value, (char*)(objstr.c_str()), 1);
			
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobjects)
{
	int argc = ZEND_NUM_ARGS();
	int themeType = 0;
	long relation = 64;
	char *objid = NULL;	
	int objidLen;

	if(argc != 3)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "sll", &objid, &objidLen, &relation, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	TeKeys objIds;

	string strObjId = "";

	if(objid != NULL)
		strObjId = objid;

	try
	{
		obj->locateObjects(strObjId, relation, objIds, themeType);

		array_init(return_value);

		TeKeys::iterator it = objIds.begin();

		while(it != objIds.end())
		{
			string objstr = *it;

			add_next_index_string(return_value, (char*)(objstr.c_str()), 1);
			
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobjectsinarray)
{
	int argc = ZEND_NUM_ARGS();
	int themeType = 0;
	long relation = 64;
	zval *objIdsIn = NULL;
	zval** item;

	if(argc != 3)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "all", &objIdsIn, &relation, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	// get number of elements in the array
	int count = zend_hash_num_elements(Z_ARRVAL_P(objIdsIn));

	if(count <= 0)
	{
		RETVAL_FALSE;

		return;
	}

	TeKeys keysIn;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P( objIdsIn ));

	for(int i = 0; i < count; ++i)
	{
		// get the data in the current array element and coerce into a string
		zend_hash_get_current_data(Z_ARRVAL_P(objIdsIn), (void**)&item);
		convert_to_string_ex(item);
		string id = Z_STRVAL_PP(item);
		keysIn.push_back(id);
		zend_hash_move_forward( Z_ARRVAL_P( objIdsIn ) );
	}			

	TeKeys keysOut;

	try
	{
		obj->locateObjects(keysIn, relation, keysOut, themeType);

		array_init(return_value);

		TeKeys::iterator it = keysOut.begin();

		while(it != keysOut.end())
		{
			string objstr = *it;

			add_next_index_string(return_value, (char*)(objstr.c_str()), 1);
			
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobjectsusingreftheme)
{
	int argc = ZEND_NUM_ARGS();
	long relation = 64;
	char *objid = NULL;	
	int objidLen;

	if(argc != 2)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "sl", &objid, &objidLen, &relation) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	TeKeys objIds;

	string strObjId = "";
	
	if(objid != NULL)
		strObjId = objid;

	try
	{
		obj->locateObjects(strObjId, relation, objIds);

		array_init(return_value);

		TeKeys::iterator it = objIds.begin();

		while(it != objIds.end())
		{
			string objstr = *it;

			add_next_index_string(return_value, (char*)(objstr.c_str()), 1);
			
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_locateobjectinarraysusingreftheme)
{
	int argc = ZEND_NUM_ARGS();
	long relation = 64;
	zval *objIdsIn = NULL;
	zval** item;

	if(argc != 2)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "al", &objIdsIn, &relation) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	CPP_GET_THIS();

	// get number of elements in the array
	int count = zend_hash_num_elements(Z_ARRVAL_P(objIdsIn));

	if(count <= 0)
	{
		RETVAL_FALSE;

		return;
	}

	TeKeys keysIn;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P( objIdsIn ));

	for(int i = 0; i < count; ++i)
	{
		// get the data in the current array element and coerce into a string
		zend_hash_get_current_data(Z_ARRVAL_P(objIdsIn), (void**)&item);
		convert_to_string_ex(item);
		string id = Z_STRVAL_PP(item);
		keysIn.push_back(id);
		zend_hash_move_forward( Z_ARRVAL_P( objIdsIn ) );
	}			

	TeKeys keysOut;

	try
	{
		obj->locateObjects(keysIn, relation, keysOut);

		array_init(return_value);

		TeKeys::iterator it = keysOut.begin();

		while(it != keysOut.end())
		{
			string objstr = *it;

			add_next_index_string(return_value, (char*)(objstr.c_str()), 1);
			
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_fetchattributes)
{	
	char *objectid = NULL;	
	int objectidLen;
	int themeType = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc < 1 || argc > 2)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "s|l", &objectid, &objectidLen, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	string strObjId = "";
	
	if(objectid != NULL)
		strObjId = objectid;

	CPP_GET_THIS();
	
	map<string, string> objVal;

	try
	{
		obj->fetchAttributes(strObjId, objVal, themeType);

		array_init(return_value);

		map<string, string>::iterator it = objVal.begin();
		while(it!= objVal.end ())
		{
			string s1 = it->first;
			string s2 = it->second;
			add_assoc_string( return_value, (char*)(s1.c_str()), (char*)(s2.c_str()), 1 );
			++it;
		}
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_pixelsize)
{
	if(ZEND_NUM_ARGS() != 0)
		WRONG_PARAM_COUNT;
	
	CPP_GET_THIS();
	
	double d = obj->getPixelSizeX();

	RETVAL_DOUBLE(d);
}

PHP_FUNCTION(terraweb_getcanvasimage)
{
	int argc = ZEND_NUM_ARGS();
	int imgtype = 0;
	int imageSize = 0;
	int opaque = 1;
	int quality = 75;

	if((argc > 4) || (argc < 1))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "l|l|l", &imgtype, &opaque, &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	buff = (char*) (obj->getCanvasImage(imgtype, imageSize, opaque, quality));
	
   	if(imageSize > 0)
	{
		RETURN_STRINGL(buff, imageSize, 1);
		TeMANAGER::TerraManager::freeImage((void*)buff);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_getlegendimage)
{
	int argc = ZEND_NUM_ARGS();
	int imgtype = 0;
	int imageSize = 0;
	int opaque = 1;
	int quality = 75;

	if((argc > 4) || (argc < 1))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "l|l|l", &imgtype, &opaque, &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	buff = (char*) (obj->getLegendImage(imgtype, imageSize, opaque, quality));
	
   	if(imageSize > 0)
	{
		RETURN_STRINGL(buff, imageSize, 1);
		TeMANAGER::TerraManager::freeImage((void*)buff);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_setdefaultvisual)
{
	int red;
    int green;
    int blue;
	int style;
	int width;
	int rcontour;	
	int gcontour;
	int bcontour;
	int stylecontour;
	int widthcontour;
	int rep;
	char* fontName = NULL;
	int fontNameLen;
	
	int argc = ZEND_NUM_ARGS();

	if(argc != 7 && argc != 12)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "lllllls|lllll", &rep, &red, &green, &blue, &style, &width, &fontName, &fontNameLen, &rcontour, &gcontour, &bcontour, &stylecontour, &widthcontour) == FAILURE) 
	{
		RETVAL_FALSE;
		
		return;
	}

	CPP_GET_THIS();

	TeColor cor (red, green, blue);
	TeVisual vis;
	vis.color(cor);
	vis.style(style);

	if(fontName != NULL)
		vis.family(fontName);	

	if(rep & TePOINTS)
	{
		vis.size(width);
	}
	else if(rep & TeLINES)
	{
		vis.width(width);
	}
	else if(rep & TeTEXT)
	{
		vis.width(width);
	}
	else if(((rep & TePOLYGONS) || (rep & TeCELLS)) && (argc == 12))
	{
		vis.transparency(width);
		TeColor contourColor(rcontour, gcontour, bcontour);
		vis.contourColor(contourColor);
		vis.contourStyle(stylecontour);
		vis.contourWidth(widthcontour);
		rep = TePOLYGONS;
	}
	else
	{
		RETVAL_FALSE;
		
		return;
	}

	obj->setDefaultVisual(vis, (TeGeomRep)rep);

	RETVAL_TRUE;
}

PHP_FUNCTION(terraweb_drawpoint)
{
	int argc = ZEND_NUM_ARGS();

	double x;
	double y;

	if(argc != 2)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "dd", &x, &y) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();

	TeCoord2D c(x, y);

	try
	{
		obj->draw(c);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawline)
{
	zval *line = NULL;

	zval** item = NULL;
	zval** item2 = NULL;

	int count = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "a", &line) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	TeLine2D l;

	count = zend_hash_num_elements(Z_ARRVAL_P(line));
	
   	zend_hash_internal_pointer_reset(Z_ARRVAL_P(line));

	for(unsigned int i = 0; i < count; ++i)
   	{
		// get the data in the current array element and coerce into a string
		zend_hash_get_current_data(Z_ARRVAL_P(line), (void**)&item);

		int count2 = zend_hash_num_elements(Z_ARRVAL_PP(item));

		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(item));

		if((count2 != 2))
		{
			RETVAL_FALSE;
			return;
		}

		zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);

		double x = Z_DVAL_PP(item2);

		zend_hash_move_forward(Z_ARRVAL_PP(item));

		zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);

		double y = Z_DVAL_PP(item2);

		TeCoord2D c(x, y);

		l.add(c);		

		zend_hash_move_forward(Z_ARRVAL_P(line));
   	}
	
	CPP_GET_THIS();

	try
	{
		obj->draw(l);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawpolygon)
{
	zval *line = NULL;

	zval** item = NULL;
	zval** item2 = NULL;

	int count = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "a", &line) == FAILURE) 
	{
		RETVAL_FALSE;

		return;
	}

	TeLine2D l;

	count = zend_hash_num_elements(Z_ARRVAL_P(line));
	
   	zend_hash_internal_pointer_reset(Z_ARRVAL_P(line));

	for(unsigned int i = 0; i < count; ++i)
   	{
		// get the data in the current array element and coerce into a string
		zend_hash_get_current_data(Z_ARRVAL_P(line), (void**)&item);

		int count2 = zend_hash_num_elements(Z_ARRVAL_PP(item));

		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(item));

		if((count2 != 2))
		{
			RETVAL_FALSE;

			return;
		}

		zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);

		double x = Z_DVAL_PP(item2);

		zend_hash_move_forward(Z_ARRVAL_PP(item));

		zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);

		double y = Z_DVAL_PP(item2);

		TeCoord2D c(x, y);

		l.add(c);		

		zend_hash_move_forward(Z_ARRVAL_P(line));
   	}

	TeLinearRing r(l);

	TePolygon p;
	p.add(r);
	
	CPP_GET_THIS();

	try
	{
		obj->draw(p);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawbox)
{
	int argc = ZEND_NUM_ARGS();

	double x1;
	double y1;
	double x2;
	double y2;

	int fill = 0; 

	if(argc != 4)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "dddd", &x1, &y1, &x2, &y2) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();

	TeBox box(x1, y1, x2, y2);

	try
	{
		obj->draw(box);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawcurrenttheme)
{
	zval *ramp = NULL;
	zval** item;
	int count;
	unsigned int i = 0;

	int argc = ZEND_NUM_ARGS();

	if((argc != 0) && (argc != 1) && (argc != 8))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	char *groupAttName = NULL;
	int groupAttNameLen = 0;
	char *groupAttNorm = NULL;
	int groupAttNormLen = 0;
	int numSlices = 0;
	int groupType = 0;
	char *restriction = NULL;
	int restrictionLen = 0;
	int prec = 0;
	double stdDev = 1.0;

	if(argc > 0)
	{
		if(zend_parse_parameters(argc TSRMLS_CC, "|s|ssllald", &restriction, &restrictionLen, &groupAttName, &groupAttNameLen, &groupAttNorm, &groupAttNormLen, &numSlices, &groupType, &ramp, &prec, &stdDev) == FAILURE) 
		{
			RETVAL_FALSE;
			return;
		}
	}


	CPP_GET_THIS();

	TeLegendEntryVector legends;
	TeGrouping g;
	string strRest;
	vector<string> rampColors;

	if(argc == 1)
	{
		if(restriction != NULL)
			strRest = restriction;
	}
	else if(argc > 1)
	{
		g.groupAttribute_.name_ = groupAttName;
		g.groupNormAttribute_ = groupAttNorm;
		g.groupNumSlices_ = numSlices;
		g.groupAttribute_.type_ = TeREAL;
		g.groupPrecision_ = prec;
		g.groupStdDev_ = stdDev;

		switch(groupType)
		{
			case 0  : g.groupMode_ = TeEqualSteps;	  break;
			case 1  : g.groupMode_ = TeQuantil;	      break;
			case 2  : g.groupMode_ = TeStdDeviation;  break;
			case 3  : g.groupMode_ = TeUniqueValue;   break;
			case 4  : g.groupMode_ = TeRasterSlicing; break;
			default : g.groupMode_ = TeEqualSteps;
		}

		// get number of elements in the array
		count = zend_hash_num_elements(Z_ARRVAL_P(ramp));		

		if(count > 0)
		{
			zend_hash_internal_pointer_reset(Z_ARRVAL_P( ramp ));

			for(i = 0; i < count; ++i)
   			{
				// get the data in the current array element and coerce into a string
				zend_hash_get_current_data(Z_ARRVAL_P(ramp), (void**)&item);
				convert_to_string_ex(item);
				string rampColor = Z_STRVAL_PP(item);
				rampColors.push_back(rampColor);

				zend_hash_move_forward( Z_ARRVAL_P( ramp ) );
		   	}			
		}
		else
		{
			rampColors.push_back("BLUE");
		}		

		if(restriction != NULL)
			strRest = restriction;
	}
	
	try
	{
		obj->drawCurrentTheme(legends, g, rampColors, strRest);
	}
	catch(...)
	{
		RETVAL_FALSE;
		return;
	}
	
// monta um vetor com a legenda
	array_init(return_value);		

	for(i = 0; i < legends.size(); ++i)
	{
		zval* newItem;

		MAKE_STD_ZVAL(newItem);
		array_init(newItem);
		
		TeGeomRepVisualMap::iterator it = legends[i].getVisualMap().begin();
		
		while(it != legends[i].getVisualMap().end())
		{
			zval* newNewItem;	// representation-style-R-G-B-transparency-width-ContourStyle-ContourR-ContourG-ContourB-ContourWidth-pointsize-from-to-label

			MAKE_STD_ZVAL(newNewItem);
			array_init(newNewItem);
			
			TeVisual& vis = *(it->second);

			add_next_index_long(newNewItem, it->first);
			add_next_index_long(newNewItem, vis.style());
			add_next_index_long(newNewItem, vis.color().red_);
			add_next_index_long(newNewItem, vis.color().green_);
			add_next_index_long(newNewItem, vis.color().blue_);			
			add_next_index_long(newNewItem, vis.transparency());
			add_next_index_long(newNewItem, vis.width());
			add_next_index_long(newNewItem, vis.contourStyle());
			add_next_index_long(newNewItem, vis.contourColor().red_);
			add_next_index_long(newNewItem, vis.contourColor().green_);
			add_next_index_long(newNewItem, vis.contourColor().blue_);
			add_next_index_long(newNewItem, vis.contourWidth());
			add_next_index_long(newNewItem, vis.size());
		    add_next_index_string(newNewItem, (char*)(legends[i].from().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].to().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].label().c_str()), 1);

			add_next_index_zval(newItem, newNewItem);
			
			++it;
		}	
			
		add_next_index_zval(return_value, newItem);
	}			
}

PHP_FUNCTION(terraweb_drawsql)
{
	zval *ramp = NULL;
	zval** item;
	int count;
	unsigned int i = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 3)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}

	char *from = NULL;
	int fromLen = 0;

	char *linkAttr = NULL;
	int linkAttrLen = 0;

	char *restriction = NULL;
	int restrictionLen = 0;

	if(argc > 0)
	{
		if(zend_parse_parameters(argc TSRMLS_CC, "sss", &from, &fromLen, &linkAttr, &linkAttrLen, &restriction, &restrictionLen) == FAILURE) 
		{
			RETVAL_FALSE;
			return;
		}
	}

	CPP_GET_THIS();	

	TeLegendEntryVector legends;

	string strfrom;
	string strlinkattr;
	string strwhere;

	if(from != NULL)
		strfrom = from;

	if(linkAttr != NULL)
		strlinkattr = linkAttr;

	if(restriction != NULL)
		strwhere = restriction;

	try
	{
		obj->drawCurrentTheme(legends, from, linkAttr, restriction);
	}
	catch(...)
	{
		RETVAL_FALSE;
		return;
	}

// monta um vetor com a legenda
	array_init(return_value);		

	for(i = 0; i < legends.size(); ++i)
	{
		zval* newItem;

		MAKE_STD_ZVAL(newItem);
		array_init(newItem);
		
		TeGeomRepVisualMap::iterator it = legends[i].getVisualMap().begin();
		
		while(it != legends[i].getVisualMap().end())
		{
			zval* newNewItem;	// representation-style-R-G-B-transparency-width-ContourStyle-ContourR-ContourG-ContourB-ContourWidth-pointsize-from-to-label

			MAKE_STD_ZVAL(newNewItem);
			array_init(newNewItem);
			
			TeVisual& vis = *(it->second);

			add_next_index_long(newNewItem, it->first);
			add_next_index_long(newNewItem, vis.style());
			add_next_index_long(newNewItem, vis.color().red_);
			add_next_index_long(newNewItem, vis.color().green_);
			add_next_index_long(newNewItem, vis.color().blue_);			
			add_next_index_long(newNewItem, vis.transparency());
			add_next_index_long(newNewItem, vis.width());
			add_next_index_long(newNewItem, vis.contourStyle());
			add_next_index_long(newNewItem, vis.contourColor().red_);
			add_next_index_long(newNewItem, vis.contourColor().green_);
			add_next_index_long(newNewItem, vis.contourColor().blue_);
			add_next_index_long(newNewItem, vis.contourWidth());
			add_next_index_long(newNewItem, vis.size());
		    add_next_index_string(newNewItem, (char*)(legends[i].from().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].to().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].label().c_str()), 1);

			add_next_index_zval(newItem, newNewItem);
			
			++it;
		}	
			
		add_next_index_zval(return_value, newItem);
	}		
}

PHP_FUNCTION(terraweb_drawgroupsql)
{
	zval *ramp = NULL;
	zval** item;
	int count;
	unsigned int i = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 9)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	char *fields = NULL;
	int fieldsLen = 0;

	char *from = NULL;
	int fromLen = 0;

	char *linkAttr = NULL;
	int linkAttrLen = 0;

	char *restriction = NULL;
	int restrictionLen = 0;

	int numSlices = 0;
	int groupType = 0;
	int prec = 0;
	double stdDev = 1.0;

	if(argc > 0)
	{
		if(zend_parse_parameters(argc TSRMLS_CC, "ssssllald", &fields, &fieldsLen, &from, &fromLen, &linkAttr, &linkAttrLen, &restriction, &restrictionLen, &numSlices, &groupType, &ramp, &prec, &stdDev) == FAILURE) 
		{
			RETVAL_FALSE;
			return;
		}
	}

	CPP_GET_THIS();	

	TeLegendEntryVector legends;

	string strfrom;
	string strlinkattr;
	string strwhere;
	string strfields;

	if(from != NULL)
		strfrom = from;

	if(linkAttr != NULL)
		strlinkattr = linkAttr;

	if(restriction != NULL)
		strwhere = restriction;

	if(fields != NULL)
		strfields = fields;

	TeGrouping g;

	g.groupNumSlices_ = numSlices;
	g.groupPrecision_ = prec;
	g.groupStdDev_ = stdDev;

	switch(groupType)
	{
		case 0 : g.groupMode_ = TeEqualSteps;	break;
		case 1 : g.groupMode_ = TeQuantil;	break;
		case 2 : g.groupMode_ = TeStdDeviation; break;
		case 3 : g.groupMode_ = TeUniqueValue; break;
		case 4 : g.groupMode_ = TeRasterSlicing; break;
		default : g.groupMode_ = TeEqualSteps;
	}

	// get number of elements in the array
	count = zend_hash_num_elements(Z_ARRVAL_P(ramp));

	vector<string> rampColors;

	if(count > 0)
	{
		zend_hash_internal_pointer_reset(Z_ARRVAL_P( ramp ));

		for(i = 0; i < count; ++i)
   		{
			// get the data in the current array element and coerce into a string
			zend_hash_get_current_data(Z_ARRVAL_P(ramp), (void**)&item);
			convert_to_string_ex(item);
			string rampColor = Z_STRVAL_PP(item);
			rampColors.push_back(rampColor);

			zend_hash_move_forward( Z_ARRVAL_P( ramp ) );
	   	}			
	}
	else
	{
		rampColors.push_back("BLUE");
	}		
		
	try
	{
		obj->drawCurrentTheme(legends, g, rampColors, strfields, strfrom, strlinkattr, strwhere);
	}
	catch(...)
	{
		RETVAL_FALSE;
		return;
	}

// monta um vetor com a legenda
	array_init(return_value);		

	for(i = 0; i < legends.size(); ++i)
	{
		zval* newItem;

		MAKE_STD_ZVAL(newItem);
		array_init(newItem);
		
		TeGeomRepVisualMap::iterator it = legends[i].getVisualMap().begin();
		
		while(it != legends[i].getVisualMap().end())
		{
			zval* newNewItem;	// representation-style-R-G-B-transparency-width-ContourStyle-ContourR-ContourG-ContourB-ContourWidth-pointsize-from-to-label

			MAKE_STD_ZVAL(newNewItem);
			array_init(newNewItem);
			
			TeVisual& vis = *(it->second);

			add_next_index_long(newNewItem, it->first);
			add_next_index_long(newNewItem, vis.style());
			add_next_index_long(newNewItem, vis.color().red_);
			add_next_index_long(newNewItem, vis.color().green_);
			add_next_index_long(newNewItem, vis.color().blue_);			
			add_next_index_long(newNewItem, vis.transparency());
			add_next_index_long(newNewItem, vis.width());
			add_next_index_long(newNewItem, vis.contourStyle());
			add_next_index_long(newNewItem, vis.contourColor().red_);
			add_next_index_long(newNewItem, vis.contourColor().green_);
			add_next_index_long(newNewItem, vis.contourColor().blue_);
			add_next_index_long(newNewItem, vis.contourWidth());
			add_next_index_long(newNewItem, vis.size());
		    add_next_index_string(newNewItem, (char*)(legends[i].from().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].to().c_str()), 1);
			add_next_index_string(newNewItem, (char*)(legends[i].label().c_str()), 1);

			add_next_index_zval(newItem, newNewItem);
			
			++it;
		}	
			
		add_next_index_zval(return_value, newItem);
	}		
}

PHP_FUNCTION(terraweb_drawselectedobjects)
{
	int argc = ZEND_NUM_ARGS();
	zval *objArray = NULL;
	zval** item;
	int count;
	int i;
	int themeType = 0;

	if((argc != 1) && (argc != 2))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "a|l", &objArray, &themeType) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}

	// get number of elements in the array
	count = zend_hash_num_elements(Z_ARRVAL_P(objArray));
	
	CPP_GET_THIS();
	
   	zend_hash_internal_pointer_reset(Z_ARRVAL_P(objArray));

	TeKeys objIds;
	
	for(i = 0; i < count; ++i)
   	{
		// get the data in the current array element and coerce into a string
		zend_hash_get_current_data(Z_ARRVAL_P(objArray), (void**)&item);
		convert_to_string_ex(item);
		
		string objId = Z_STRVAL_PP(item);

		objIds.push_back(objId);

		zend_hash_move_forward(Z_ARRVAL_P(objArray));
   	}

	try
	{
		obj->drawSelectedObjects(objIds, themeType);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_drawbufferzone)
{
	int argc = ZEND_NUM_ARGS();
	char* objId = NULL;
	int objIdLen;
	double dist;
	int themeType = 0;

	if(argc > 3 || argc < 2)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "sd|l", &objId, &objIdLen, &dist, &themeType) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();	

	try
	{
		obj->drawBufferZone(objId, dist, themeType);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawtext)
{
	int argc = ZEND_NUM_ARGS();
	double x;
	double y;
	double angle = 0.;
	double alignh = 1.;
	double alignv = -1.;
	char *text = NULL;	
	int textLen = 0;
	
	if(argc != 6)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "ddsddd", &x, &y, &text, &textLen, &angle, &alignh, &alignv) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();

	TeCoord2D c(x, y);

	string texts = text;

	try
	{
		obj->draw(c, texts, angle, alignh, alignv);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_drawlegend)
{
	int argc = ZEND_NUM_ARGS();
	int count = 0;
	int count2 = 0;
	int count3 = 0;
	int count4 = 0;
	int i = 0;

	int width = 150;
	zval* objArray = NULL;
	zval** item;
	zval** item2;
	zval** item3;
	zval** item4;

	if(argc != 2 && argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "a|l", &objArray, &width) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();

	count = zend_hash_num_elements(Z_ARRVAL_P(objArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(objArray));

	if(count <= 0)
	{
		RETVAL_FALSE;
		//php_error(E_WARNING, "A legenda esta sem elementos");
		return;
	}

	vector<TeLegendEntryVector> legends;
	vector<string> legendTitle;
	
	for(i = 0; i < count; ++i)
   	{
		char* key;
		unsigned long ind;

// a chave de cada linha da matriz principal informa o texto de cada camada de legenda
		zend_hash_get_current_key(Z_ARRVAL_P(objArray), &key, &ind, 0);
		legendTitle.push_back(key);

// pega a linha da matriz => corresponde as informacoes de uma camada de legenda (que por sua vez pode ter varias linhas, uma para cada representacao)
       	zend_hash_get_current_data(Z_ARRVAL_P(objArray), (void**)&item);		

		int count2 = zend_hash_num_elements(Z_ARRVAL_PP(item));

		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(item));

		if(count2 <= 0)
		{
			RETVAL_FALSE;
			//php_error(E_WARNING, "A legenda de uma das camadas esta sem elementos");
			return;
		}

		// Juntar as linhas de uma camada de legenda em um unico vetor de legendas
		TeLegendEntryVector legendVec;
		
		for(int j = 0; j < count2; ++j)
		{
			zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);

			count3 = zend_hash_num_elements(Z_ARRVAL_PP(item2));

			TeLegendEntry legend;			

// acessar o visual de cada legenda
			for(int k = 0; k < count3; ++k)
			{
				zend_hash_get_current_data(Z_ARRVAL_PP(item2), (void**)&item4);

				count4 = zend_hash_num_elements(Z_ARRVAL_PP(item4));

				if(count4 != 16)
				{
					RETVAL_FALSE;
					//php_error(E_WARNING, "A legenda esta montada de forma errada");
					return;
				}				

				TeVisual vis;				
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				TeGeomRep drep = (TeGeomRep)(Z_LVAL_PP(item3));

				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.style(Z_LVAL_PP(item3));

				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.color().red_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.color().green_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.color().blue_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.transparency(Z_LVAL_PP(item3));
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.width(Z_LVAL_PP(item3));
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.contourStyle(Z_LVAL_PP(item3));
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.contourColor().red_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.contourColor().green_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.contourColor().blue_ = Z_LVAL_PP(item3);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.contourWidth(Z_LVAL_PP(item3));
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				vis.size(Z_LVAL_PP(item3));
		
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				convert_to_string_ex(item3);
				string from = Z_STRVAL_PP(item3);
				legend.from(from);

				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				convert_to_string_ex(item3);
				string to = Z_STRVAL_PP(item3);
				legend.to(to);
				
				zend_hash_move_forward(Z_ARRVAL_PP(item4));
				zend_hash_get_current_data(Z_ARRVAL_PP(item4), (void**)&item3);
				convert_to_string_ex(item3);
				string label = Z_STRVAL_PP(item3);
				legend.label(label);				

				legend.setVisual(vis, drep);

				zend_hash_move_forward(Z_ARRVAL_PP(item2));
			}

			legendVec.push_back(legend);

			zend_hash_move_forward(Z_ARRVAL_PP(item));			
		}

		legends.push_back(legendVec);

		zend_hash_move_forward(Z_ARRVAL_P(objArray));	// Move para a proxima camada de legenda
	}

	try
	{
		obj->drawLegend(legends, legendTitle, width);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_savecanvasimage)
{
    int argc = ZEND_NUM_ARGS();
	int imgtype = 0;
	char *fileName = NULL;	
	int fileNameLen;
	int opaque = 1;
	int quality = 75;

	if((argc > 4) || (argc < 3))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "lsl|l", &imgtype, &fileName, &fileNameLen, &opaque, &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	try
	{
		obj->saveCanvasImage(imgtype, strFileName, opaque, quality);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_savelegendimage)
{
	int argc = ZEND_NUM_ARGS();
	int imgtype = 0;
	char *fileName = NULL;	
	int fileNameLen;
	int opaque = 1;
	int quality = 75;

	if((argc > 4) || (argc < 3))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "lsl|l", &imgtype, &fileName, &fileNameLen, &opaque, &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	try
	{
		obj->saveLegendImage(imgtype, strFileName, opaque, quality);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_cleancanvas)
{
	int argc = ZEND_NUM_ARGS();

	if(argc != 0)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	CPP_GET_THIS();	

	obj->cleanCanvas();
	
	RETVAL_TRUE;	
}

PHP_FUNCTION(terraweb_close)
{
	int argc = ZEND_NUM_ARGS();

	if(argc != 0)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	CPP_GET_THIS();	

	obj->closeConnection();
	
	RETVAL_TRUE;	
}

PHP_FUNCTION(terraweb_getscale)
{
	int argc = ZEND_NUM_ARGS();

	if(argc != 0)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	CPP_GET_THIS();
	
	double scale = obj->getScale();
	
	RETVAL_DOUBLE(scale);
	
}

PHP_FUNCTION(terraweb_setautomaticscalecontrolenable)
{
	int enable = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &enable) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setAutomaticScaleControlEnable(enable);	
}

PHP_FUNCTION(terraweb_setconflictdetect)
{
	int enable = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &enable) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setConflictDetect(enable);
}

PHP_FUNCTION(terraweb_setpriorityfield)
{
	char* lfield = NULL;
	int lfieldSize = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "s", &lfield, &lfieldSize) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setPriorityField(lfield);
}

PHP_FUNCTION(terraweb_setmincollisiontolerance)
{
	int npixels = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &npixels) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setMinCollisionTolerance(npixels);
}

PHP_FUNCTION(terraweb_clearconflictcache)
{
	int argc = ZEND_NUM_ARGS();

	if(argc != 0)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	CPP_GET_THIS();

	obj->clearConflictCache();
}

PHP_FUNCTION(terraweb_setgeneralizedpixels)
{
    int npixels = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &npixels) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setGeneralizedPixels(npixels);
}


PHP_FUNCTION(terraweb_setlabelfield)
{
	char* lfield = NULL;
	int lfieldSize = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "s", &lfield, &lfieldSize) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setLabelField(lfield);
}

PHP_FUNCTION(terraweb_settextoutlineenable)
{
	int enable = 0;	

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &enable) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setTextOutLineEnable(enable);
}

PHP_FUNCTION(terraweb_settextoutlinecolor)
{
	int r = 0;
	int g = 0;
	int b = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 3)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "lll", &r, &g, &b) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();

	obj->setTextOutLineColor(r, g, b);
}

PHP_FUNCTION(terraweb_setimagemapproperties)
{
	int argc = ZEND_NUM_ARGS();
	int mapNameLen = 0;
	char *mapName = NULL;	
	int mapIdLen = 0;
	char *mapId = NULL;	

	if(argc != 2)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "ss", &mapName, &mapNameLen, &mapId, &mapIdLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if((mapName == NULL) || (mapId == NULL))
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();
	
	obj->setImageMapProperties(mapName, mapId);
}

PHP_FUNCTION(terraweb_closeimagemap)
{
	int hasToClose = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &hasToClose) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();
	
	obj->closeImageMap(hasToClose);
}

PHP_FUNCTION(terraweb_setareashape)
{
	int areaShapeType = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &areaShapeType) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();
	
	obj->setAreaShape((TeMANAGER::TeImageMapCanvas::TeImageMapShapeType)areaShapeType);
}

PHP_FUNCTION(terraweb_setareaproperty)
{
	int argc = ZEND_NUM_ARGS();
	int propertyNameLen = 0;
	char *propertyName = NULL;	
	int propertyValueLen = 0;
	char *propertyValue = NULL;	
	int valueSrcLen = 0;
	char *valueSrc = NULL;	

	if((argc != 2) && (argc != 3))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "ss|s", &propertyName, &propertyNameLen, &propertyValue, &propertyValueLen, &valueSrc, &valueSrcLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if((propertyName == NULL) || (propertyValue == NULL))
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();
	
	obj->setAreaProperty(propertyName, propertyValue, valueSrc);
}

PHP_FUNCTION(terraweb_setareacircleradius)
{
	int radius = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &radius) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();
	
	obj->setAreaCircleRadius(radius);
}

PHP_FUNCTION(terraweb_setarearectwidth)
{
	int width = 0;

	int argc = ZEND_NUM_ARGS();

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	if(zend_parse_parameters(argc TSRMLS_CC, "l", &width) == FAILURE) 
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();
	
	obj->setAreaRectWidth(width);
}

PHP_FUNCTION(terraweb_getimagemap)
{
	int argc = ZEND_NUM_ARGS();
	int fromLen = 0;
	char *from = NULL;
	int linkAttrLen = 0;
	char *linkAttr = NULL;	
	int restrictionExpressionLen = 0;
	char *restrictionExpression = NULL;	

	if(argc != 3)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "sss", &from, &fromLen, &linkAttr, &linkAttrLen, &restrictionExpression, &restrictionExpressionLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	CPP_GET_THIS();
	
	string immap = obj->getImageMap(from, linkAttr, restrictionExpression);
	
   	if(!immap.empty())
	{
	    RETVAL_STRING((char*)(immap.c_str()), 1);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_drawcartogram)
{
	int argc = ZEND_NUM_ARGS();
	int fromLen = 0;
	char *from = NULL;
	int linkAttrLen = 0;
	char *linkAttr = NULL;	
	int restrictionExpressionLen = 0;
	char *restrictionExpression = NULL;	
	int proportionalToFieldLen = 0;
	char *proportionalToField = NULL;
	zval* pFields = NULL;
	zval* pCartogramParameters = NULL;
	zval** item;
	zval** item2;
	zval** item3;
	char* fieldsFunc = NULL;
	int fieldsFuncLen = 0;

	if((argc != 3) && (argc != 7))
	{
	    php_error(E_WARNING, "argc != 7");
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "asa|ssss", &pFields, &fieldsFunc, &fieldsFuncLen, &pCartogramParameters, &proportionalToField, &proportionalToFieldLen, &from, &fromLen, &linkAttr, &linkAttrLen, &restrictionExpression, &restrictionExpressionLen) == FAILURE)
	{
		php_error(E_WARNING, "zend_parse_parameters");
		RETVAL_FALSE;
		return;
	}
	
// fields
	int count = zend_hash_num_elements(Z_ARRVAL_P(pFields));

	if(count <= 0)
	{
	    php_error(E_WARNING, "count <= 0");
		RETVAL_FALSE;
		return;
	}
	
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pFields));	
	
	std::vector<std::string> fields;
	
	for(int i = 0; i < count; ++i)
	{
		zend_hash_get_current_data(Z_ARRVAL_P(pFields), (void**)&item);
		convert_to_string_ex(item);
		string id = Z_STRVAL_PP(item);
		fields.push_back(id);
		zend_hash_move_forward(Z_ARRVAL_P(pFields));
	}

// chart parameters
	count = zend_hash_num_elements(Z_ARRVAL_P(pCartogramParameters));
	
	if(count != 5)
	{
	    php_error(E_WARNING, "count != 5");
		RETVAL_FALSE;
		return;
	}
	
	TeMANAGER::TeCartogramParameters cartogramParameters;
	
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pCartogramParameters));		
	
// vetor de cores
	zend_hash_get_current_data(Z_ARRVAL_P(pCartogramParameters), (void**)&item);
	
	count = zend_hash_num_elements(Z_ARRVAL_PP(item));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(item));
	
	for(int i = 0; i < count; ++i)
	{
		zend_hash_get_current_data(Z_ARRVAL_PP(item), (void**)&item2);
		
		zend_hash_get_current_data(Z_ARRVAL_PP(item2), (void**)&item3);		
		int r = Z_LVAL_PP(item3);
		zend_hash_move_forward(Z_ARRVAL_PP(item2));
		
		zend_hash_get_current_data(Z_ARRVAL_PP(item2), (void**)&item3);		
		int g = Z_LVAL_PP(item3);
		zend_hash_move_forward(Z_ARRVAL_PP(item2));

		zend_hash_get_current_data(Z_ARRVAL_PP(item2), (void**)&item3);		
		int b = Z_LVAL_PP(item3);
		
		TeColor c(r, g, b);
		cartogramParameters.colors_.push_back(c);
		
		zend_hash_move_forward(Z_ARRVAL_PP(item));
	}
	
	zend_hash_move_forward(Z_ARRVAL_P(pCartogramParameters));
	
	zend_hash_get_current_data(Z_ARRVAL_P(pCartogramParameters), (void**)&item);
	cartogramParameters.minSize_ = Z_LVAL_PP(item);	
	zend_hash_move_forward(Z_ARRVAL_P(pCartogramParameters));
	
	zend_hash_get_current_data(Z_ARRVAL_P(pCartogramParameters), (void**)&item);
	cartogramParameters.maxSize_ = Z_LVAL_PP(item);	
	zend_hash_move_forward(Z_ARRVAL_P(pCartogramParameters));
	
	zend_hash_get_current_data(Z_ARRVAL_P(pCartogramParameters), (void**)&item);
	cartogramParameters.width_ = Z_LVAL_PP(item);	
	zend_hash_move_forward(Z_ARRVAL_P(pCartogramParameters));
	
	zend_hash_get_current_data(Z_ARRVAL_P(pCartogramParameters), (void**)&item);
	cartogramParameters.type_ = Z_LVAL_PP(item);
	
	CPP_GET_THIS();
	
	try
	{
		obj->drawCartogram(fields, fieldsFunc, cartogramParameters, proportionalToField, from , linkAttr, restrictionExpression);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

/* {{{ destructor for myphpclass objects */
static void destroy_terraweb(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	delete (TeMANAGER::TerraManager*)(rsrc->ptr);
}
/* }}} */

/**************************************************/
/*               DEPRECTATED                      */
/**************************************************/
PHP_FUNCTION(terraweb_getpng)
{
	int argc = ZEND_NUM_ARGS();
	int imageSize = 0;

	if(argc > 0)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	CPP_GET_THIS();
	
   	char *buff = 0;
	
	buff = (char*) (obj->getCanvasImage(0, imageSize, false));
	
   	if(imageSize > 0)
	{
		RETURN_STRINGL(buff, imageSize, 1);
		TeMANAGER::TerraManager::freeImage((void*)buff);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_getjpg)
{
	int argc = ZEND_NUM_ARGS();
	int imageSize = 0;
	int quality = 75;

	if(argc > 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}
	
	if(zend_parse_parameters(argc TSRMLS_CC, "|l", &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}

	CPP_GET_THIS();
	
   	char *buff = 0;
	
	buff = (char*) (obj->getCanvasImage(1, imageSize, true, quality));
	
   	if(imageSize > 0)
	{
		RETURN_STRINGL(buff, imageSize, 1);
		TeMANAGER::TerraManager::freeImage((void*)buff);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_getlegendpng)
{
	int argc = ZEND_NUM_ARGS();
	int imageSize = 0;

	if(argc > 0)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	CPP_GET_THIS();
	
   	char *buff = 0;
	
	buff = (char*) (obj->getLegendImage(0, imageSize, false));
	
   	if(imageSize > 0)
	{
		RETURN_STRINGL(buff, imageSize, 1);
		TeMANAGER::TerraManager::freeImage((void*)buff);
	}
   	else
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_saveplot)
{
    int argc = ZEND_NUM_ARGS();
	char *fileName = NULL;	
	int fileNameLen;

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "s", &fileName, &fileNameLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	try
	{
		obj->saveCanvasImage(0, strFileName, false);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_saveimage)
{
    int argc = ZEND_NUM_ARGS();
	char *fileName = NULL;	
	int fileNameLen;
	int quality = 75;

	if((argc > 2) || (argc < 1))
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "s|l", &fileName, &fileNameLen, &quality) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	try
	{
		obj->saveCanvasImage(1, strFileName, true, quality);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_savelegend)
{
	int argc = ZEND_NUM_ARGS();
	int imgtype = 0;
	char *fileName = NULL;	
	int fileNameLen;
	int opaque = 1;
	int quality = 75;

	if(argc !=1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "s", &fileName, &fileNameLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
   	char *buff = 0;
	
	try
	{
		obj->saveLegendImage(0, strFileName, false);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}

PHP_FUNCTION(terraweb_savethemetofile)
{
    int argc = ZEND_NUM_ARGS();
	char *fileName = NULL;
	int fileNameLen;

	if(argc != 1)
	{
		RETVAL_FALSE;
		WRONG_PARAM_COUNT;
	}

	if(zend_parse_parameters(argc TSRMLS_CC, "s", &fileName, &fileNameLen) == FAILURE)
	{
		RETVAL_FALSE;
		return;
	}
	
	if(fileName == NULL)
	{
		RETVAL_FALSE;
		return;
	}
	
	string strFileName = fileName;
	
	CPP_GET_THIS();
	
	try
	{
		obj->saveThemeToFile(strFileName);
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}
}

PHP_FUNCTION(terraweb_setthemevisual)
{
	int red;
    int green;
    int blue;
	int style;
	int width;
	int rcontour;	
	int gcontour;
	int bcontour;
	int stylecontour;
	int widthcontour;
	int rep;
	char* fontName = NULL;
	int fontNameLen;
	
	int argc = ZEND_NUM_ARGS();

	if(argc != 7 && argc != 12)
	{
		RETVAL_FALSE;

		WRONG_PARAM_COUNT;
	}
		
	if(zend_parse_parameters(argc TSRMLS_CC, "lllllls|lllll", &rep, &red, &green, &blue, &style, &width, &fontName, &fontNameLen, &rcontour, &gcontour, &bcontour, &stylecontour, &widthcontour) == FAILURE) 
	{
		RETVAL_FALSE;
		
		return;
	}

	CPP_GET_THIS();

	TeColor cor (red, green, blue);
	TeVisual vis;
	vis.color(cor);
	vis.style(style);

	if(fontName != NULL)
		vis.family(fontName);	

	if(rep & TePOINTS)
	{
		vis.size(width);
	}
	else if(rep & TeLINES)
	{
		vis.width(width);
	}
	else if(rep & TeTEXT)
	{
		vis.width(width);
	}
	else if(((rep & TePOLYGONS) || (rep & TeCELLS)) && (argc == 12))
	{
		vis.transparency(width);
		TeColor contourColor(rcontour, gcontour, bcontour);
		vis.contourColor(contourColor);
		vis.contourStyle(stylecontour);
		vis.contourWidth(widthcontour);
		rep = TePOLYGONS;
	}
	else
	{
		RETVAL_FALSE;
		
		return;
	}

	obj->setThemeVisual(vis, (TeGeomRep)rep);

	RETVAL_TRUE;
}

PHP_FUNCTION(terraweb_drawtext2)
{
	CPP_GET_THIS();
	
	try
	{
		obj->drawText();
		RETVAL_TRUE;
	}
	catch(...)
	{
		RETVAL_FALSE;
	}	
}


