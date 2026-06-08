#if !defined(AFX_PICSCREEN_H__F9C209CA_9BA3_4DDE_9EF0_CDB941EA6703__INCLUDED_)
#define AFX_PICSCREEN_H__F9C209CA_9BA3_4DDE_9EF0_CDB941EA6703__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PicScreen.h : header file
//
//#include "..\include\define.h"

/////////////////////////////////////////////////////////////////////////////
// CPicScreen window
#define PIC_BOARD_UL_COLOR RGB(160,160,160)  //RGB(125,150,200)
#define PCI_BOARD_DR_COLOR RGB(100,100,100)  //RGB(125,150,200)
#define   WM_GENPREIMG_NOTIFY WM_USER+777
#define  GEN_PREVIEW_WIDTH   640
#define  GEN_PREVIEW_HEIGHT  640
class CPicScreen : public CStatic
{
// Construction
public:
	CPicScreen();
    HBITMAP     m_hBmp;
	RECT        Picrect;
	CString     m_szRipFile;
	CString     m_szPreviewFile;

	BOOL        m_bGenCancel;
	CWinThread* m_hGenThread;
	BOOL        m_bLoading;
	BOOL        m_bFailedLoad;
	//LPRIPCOLORTABLE m_pCtable;
	//RIPHEADER   ripheader;
private:
	int         m_nxstart;
	int         m_nxend;
	int         m_nLastPercentLinePos;
	RECT        m_Rectprintvalid;
	RECT        m_ClientRect;
	BOOL        m_bShowPercent;
	BOOL        m_bYPrtDir;

    COLORREF    m_bkcolor;
private:

// Attributes
public:
	void ShowPreview(LPCTSTR lpszRipFileName);

// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPicScreen)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetBkColor(COLORREF color);
	void DrawImg(HBITMAP,CDC* pDC=NULL);
	virtual ~CPicScreen();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPicScreen)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICSCREEN_H__F9C209CA_9BA3_4DDE_9EF0_CDB941EA6703__INCLUDED_)
