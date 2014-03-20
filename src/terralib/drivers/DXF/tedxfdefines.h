#ifndef  __TEDXF_INTERNAL_DEFINES_H
#define  __TEDXF_INTERNAL_DEFINES_H

#ifdef WIN32

	#ifdef TEDXF_EXPORTS
		#define TEDXF_DLL  __declspec(dllexport)   // export DLL information
	#else
		#define TEDXF_DLL  __declspec(dllimport)   // import DLL information
	#endif
#else
	#define TEDXF_DLL
#endif

#endif
