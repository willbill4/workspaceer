// 9proxy5Dlg.h : header file
//

#if !defined(AFX_9PROXY5DLG_H__7512EAB5_A91D_4F20_912F_232005E376BC__INCLUDED_)
#define AFX_9PROXY5DLG_H__7512EAB5_A91D_4F20_912F_232005E376BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMy9proxy5Dlg dialog
#include "Proxy.h"

class CMy9proxy5Dlg : public CDialog
{
// Construction
public:
	CMy9proxy5Dlg(CWnd* pParent = NULL);	// standard constructor

	CProxy cp;

// Dialog Data
	//{{AFX_DATA(CMy9proxy5Dlg)
	enum { IDD = IDD_MY9PROXY5_DIALOG };
	CString	m_port;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy9proxy5Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMy9proxy5Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_9PROXY5DLG_H__7512EAB5_A91D_4F20_912F_232005E376BC__INCLUDED_)
