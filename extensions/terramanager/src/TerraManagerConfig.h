#ifndef  __TERRAMANAGER_INTERNAL_TERRAMANAGERCONFIG_H
#define  __TERRAMANAGER_INTERNAL_TERRAMANAGERCONFIG_H

#if defined(_MSC_VER) /* MSVC Compiler */
    #pragma warning(disable : 4251)
    #pragma warning(disable : 4275)
#endif

// No caso de queremos criar uma DLL no Windows, precisamos exportar os símbolos!
#ifdef WIN32
	#ifdef TERRAMANAGERDLL
		#define TERRAMANAGEREXPORT  __declspec(dllexport)   // export DLL information
	#else
		#define TERRAMANAGEREXPORT  __declspec(dllimport)   // import DLL information
	#endif 
#else
	#define TERRAMANAGEREXPORT
#endif


#endif	// __TERRAMANAGER_INTERNAL_TERRAMANAGERCONFIG_H

