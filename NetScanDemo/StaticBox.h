#if !defined(AFX_STATICBOX_H__6D80BFA9_96C8_464D_9DA4_8CA1E61638D6__INCLUDED_)
#define AFX_STATICBOX_H__6D80BFA9_96C8_464D_9DA4_8CA1E61638D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticBox window
#define STATIC_NORMAL_FLAG   0
#define STATIC_ERROR_FLAG    1
#define STATIC_WARN_FLAG     2

class CStaticBox : public CStatic
{
// Construction
public:
	CStaticBox();

// Attributes
private:
    RECT  m_rectClient;
	UINT  m_nInfoType;
	UINT  m_bFlashState;
	BOOL  m_bFlashMode;
	COLORREF m_nColor;
	CString m_szText;
	void  DrawItem(CDC* pDC);
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticBox)
	//}}AFX_VIRTUAL

// Implementation
public:
    void SetInfo(UINT nType, UINT nResID,BOOL bAnsy=TRUE);
	void SetInfo(UINT nType,LPCTSTR pszTxt,BOOL bAnsy=TRUE);
	void EnableFlashMode(BOOL bEnable);
	virtual ~CStaticBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticBox)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
protected:
	//afx_msg void OnTimer(UINT_PTR nIDEvent);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICBOX_H__6D80BFA9_96C8_464D_9DA4_8CA1E61638D6__INCLUDED_)
