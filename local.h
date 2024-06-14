#define LOCAL TRUE
//#define HPUX TRUE  //Using PC
#ifdef HPUX
	#define SCC_INI "/usr/local/bin/scc.ini" // Sleep and Chronobiology Center INI file
#else
    #define SCC_INI "c:\\bin\\scc.ini"   // Sleep and Chronobiology Center INI file
	#define WSCC_INI L"c:\\scc.ini"   // Sleep and Chronobiology Center INI file
#endif
