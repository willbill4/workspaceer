// 9proxy5.h : main header file for the 9PROXY5 application
//

#if !defined(AFX_9PROXY5_H__F031C5B7_267A_407C_AF95_08D637978E33__INCLUDED_)
#define AFX_9PROXY5_H__F031C5B7_267A_407C_AF95_08D637978E33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMy9proxy5App:
// See 9proxy5.cpp for the implementation of this class
//

class CMy9proxy5App : public CWinApp
{
public:
	CMy9proxy5App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy9proxy5App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMy9proxy5App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_9PROXY5_H__F031C5B7_267A_407C_AF95_08D637978E33__INCLUDED_)
