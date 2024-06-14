#include "stdafx.h"

#include "TextCompatibility.h"

TCHAR WideString[MAXTEXTSIZE];
WCHAR wWideString[MAXTEXTSIZE];
char SmallString[MAXTEXTSIZE];

#ifdef UNICODE
TCHAR * MakeTextWide(char str[])
{
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, strlen(str) + 1, WideString, MAXTEXTSIZE );
	return WideString;
}
char * MakeTextSmall(TCHAR str[])
{
	WideCharToMultiByte(CP_ACP, 0, str, sizeof(TCHAR) * (short)wcslen(str), SmallString, MAXTEXTSIZE, NULL, NULL );
	return SmallString;
}

#else
WCHAR * MakeTextWide(char str[])
{
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, strlen(str) + 1, wWideString, MAXTEXTSIZE );
	return wWideString;
}
char * MakeTextSmall(WCHAR str[])
{
	WideCharToMultiByte(CP_ACP, 0, str, sizeof(TCHAR) * (short)wcslen(str), SmallString, MAXTEXTSIZE, NULL, NULL );
	return SmallString;
}

#endif
