#ifndef TETESTSBASE_HPP
  #define TETESTSBASE_HPP

  #include <terralib/kernel/TeDefines.h>

  #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
    #define TETESTSRESPATH "..\\..\\resources\\"
    #define TETESTSBINPATH "..\\..\\bin\\"
  #elif TePLATFORM == TePLATFORMCODE_LINUX
    #ifdef TE_DATA_UNITTEST_LOCALE
      #define TETESTSRESPATH TE_DATA_UNITTEST_LOCALE
      #define TETESTSBINPATH TE_DATA_UNITTEST_BIN
    #else
      #define TETESTSRESPATH "../../resources/"
      #define TETESTSBINPATH "../bin/"
    #endif
  #else
    #error "ERROR: Unsupported platform"
  #endif

#endif
