// bfile.cpp : Defines the initialization routines for the DLL.
//

//#include "stdafx.h"
 
#include "stdafx.h"
 
#include "wpicc.h"
#include "bfile.h"

//august 7th 2003 RJS added reference function to ensure we're using the right ADZERO rather than one defined elsewhere
//since it is possible to inadvertently have more than one defined 
long	GetADZERO()
{
	return ADZERO;
}
//may 15th 2005 RJS made reference call for windows version
void	SetADZERO(long Adzero)
{
	ADZERO = Adzero;
}
//#include "bfilelib.h"

/*

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CBfileApp

BEGIN_MESSAGE_MAP(CBfileApp, CWinApp)
	//{{AFX_MSG_MAP(CBfileApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBfileApp construction

CBfileApp::CBfileApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBfileApp object

CBfileApp theApp;
*/
