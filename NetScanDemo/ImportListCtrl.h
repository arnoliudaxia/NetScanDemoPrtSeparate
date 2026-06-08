#if !defined(AFX_EDITABLELIST_H__4CDBF85A_E866_45C0_A567_E674FB566292__INCLUDED_)
#define AFX_EDITABLELIST_H__4CDBF85A_E866_45C0_A567_E674FB566292__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditList.h : header file
#define _VOLUM_SIZE		10
#define _MAX_SUBITEM	50	
#define _MAX_ITEM		200
// ListStyles Flags
#define _LVS_DEFAULT   (LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_TRACKSELECT)	//|LVS_EX_CHECKBOXES)
/////////////////////////////////////////////////////////////////////////////
// CImportListCtrl window
typedef  BOOL  (*LPFCALLFUNC)(UINT,UINT);

typedef  struct _tag_RAGE
{
	float	fminvol;
	float	fmaxvol;
}RAGE,*LPRAGE;

class CImportListCtrl : public CListCtrl
{
// Construction
public:
	CImportListCtrl();
	virtual ~CImportListCtrl();

	// Attributes
public:
	CEdit  m_cEditCtl;
    BOOL   m_bEnBytesCtl;
	int    m_nRegBytes;
	int    m_nBytesPerAd;
	int    m_nEditCurItem;
	int    m_nEditCurSubItem;
	BOOL   m_bEntryOK;
// Operations
public:
	BOOL (*m_pWCallBack)(UINT,UINT);
	//LPFCALLFUNC  m_pWCallBack;
	void EntryPress();
	BOOL SetListItemParam(int nIndex,int nVal);
	int  GetListItemParam(int nIndex);
    void ChangeEditPos(int iItem,int iSubItem);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportListCtrl)
	//}}AFX_VIRTUAL
private:
	CFont* plistFont;
	COLORREF m_crItemtext[_MAX_ITEM];
	double fcurvt[_MAX_SUBITEM][_MAX_ITEM];	//列表单元格最大个数
	BOOL m_bErrShow[_MAX_SUBITEM][_MAX_ITEM];
	float m_fVolRage[_MAX_SUBITEM][_MAX_ITEM][2];	//电压正常取值范围
	
	BOOL m_bFixbkWhite;
//	UINT m_nItemHeight;
	int  m_nItemCount;
	int  m_nSubItemCount;
	BOOL m_bEditable;
	DWORD m_curStyle;
	COLORREF crWindow;
	COLORREF crWindowText;
	void CreateEditCtrl();
// Implementation
	// Generated message map functions
protected:
	//{{AFX_MSG(CImportListCtrl)
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void PreSubclassWindow();
	//}}AFX_MSG
public:
	void SetListFont(int nHeight, LPCTSTR lpszFontName=_T("Arial"),BOOL bFixbkWhite=FALSE);
	void InitColum(int nSubCount, TCHAR** szHeader,DWORD dwStyle=_LVS_DEFAULT,int nHeaderWidth=100);
	void InitItem(int nItemCount, TCHAR** szHeader,COLORREF crItemtext[]);
	void InitItem(int nItem, TCHAR* szHeader,COLORREF crItemtext);	//=RGB(213,213,111)
	void SetValumRage(POINT item,RAGE valrage,BOOL bErrShow=TRUE);
	void GetLineData(int nItem,float* pDataBuf,BOOL bAbsolute=FALSE);
	void SetListData(float** fSetVal);
	void GetListData(float** fGetVal);
	void EnableInput(BOOL bEditable);
	void UpdateEditValue();


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLIST_H__4CDBF85A_E866_45C0_A567_E674FB566292__INCLUDED_)
