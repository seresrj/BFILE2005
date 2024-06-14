#define MAXTEXTSIZE 16384*2
#ifdef UNICODE
TCHAR * MakeTextWide(char str[]);
char * MakeTextSmall(TCHAR str[]);
#else
WCHAR * MakeTextWide(char str[]);
char * MakeTextSmall(WCHAR str[]);
#endif