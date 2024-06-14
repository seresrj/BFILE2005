// bfile.h : main header file for the BFILE DLL
//

#if !defined(AFX_BFILE_H__4E32C156_F378_4C67_886A_D786D05B8A3C__INCLUDED_)
#define AFX_BFILE_H__4E32C156_F378_4C67_886A_D786D05B8A3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBfileApp
// See bfile.cpp for the implementation of this class
//

class CBfileApp : public CWinApp
{
public:
	CBfileApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBfileApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CBfileApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BFILE_H__4E32C156_F378_4C67_886A_D786D05B8A3C__INCLUDED_)
