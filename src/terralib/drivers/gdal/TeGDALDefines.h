#ifndef  __TEGDAL_INTERNAL_DEFINES_H
#define  __TEGDAL_INTERNAL_DEFINES_H

#ifdef WIN32
	#ifdef TEGDAL_EXPORTS
		#define TEGDAL_DLL  __declspec(dllexport)   // export DLL information
	#else
		#define TEGDAL_DLL  __declspec(dllimport)   // import DLL information
	#endif
#else
	#define TEGDAL_DLL
#endif

#endif
