#if !defined(AFX_INKSTATECTRL_H__3EE00BF5_EA7E_4A86_AE62_687794FD73B7__INCLUDED_)
#define AFX_INKSTATECTRL_H__3EE00BF5_EA7E_4A86_AE62_687794FD73B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InkStateCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInkStateCtrl window

class CInkStateCtrl : public CStatic
{
// Construction
public:
	CInkStateCtrl();

// Attributes
public:
private:
	int  m_nOffset;
	int  m_nInkCnts;
	UINT m_nInkState;
	CString m_szInfo;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInkStateCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	
	void SetInkState(UINT nInkState);
	void SetInkCnt(int nInkCnts,int nOffset=0);
	void SetIOType(int nType);
	virtual ~CInkStateCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CInkStateCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INKSTATECTRL_H__3EE00BF5_EA7E_4A86_AE62_687794FD73B7__INCLUDED_)
