
#pragma once
#include "SWTreeCtrl.h"
class CSWListTreeCtrl : public CWnd
{
	typedef struct _CTVHITTESTINFO
	{
		POINT pt;
		UINT flags;
		HTREEITEM hItem;
		int iSubItem;
	}CTVHITTESTINFO;
	DECLARE_DYNCREATE(CSWListTreeCtrl);
	// 成员私有结构定义
// 构造/析构函数
public:
	CSWListTreeCtrl();
	virtual ~CSWListTreeCtrl();
	CSWTreeCtrl m_childTree;
	CSWTreeCtrl& GetTreeCtrl() { return m_childTree; }
	// 私有成员变量
private:
	// 私有成员函数
private:
	void CommonConstruct(); // 初始化
	HTREEITEM HitTest(CPoint pt, UINT* pFlags = NULL) const;
	HTREEITEM HitTest(CTVHITTESTINFO* pHitTestInfo) const;
	//CSWTreeCtrl& GetTreeCtrl() { return m_childTree; }
	CHeaderCtrl& GetHeaderCtrl() { return m_childHeader; }
	// 受保护成员变量
protected:
	enum ChildrenIDs { HeaderID = 1, TreeID = 2, HScrollID = 3, Header2ID = 4 };
	enum { MAX_COLUMN_COUNT = 64 };
	
	CScrollBar m_childScrollBar; // 横向滚动条
	CHeaderCtrl m_childHeader;	// 列表项标题栏
	CHeaderCtrl m_childHeader2;	// 出现下拉滚动条时列表项标题栏右上角填补小方格
	int m_cyHeader;
	int m_cxTotal;
	int m_xPos;
	int m_xOffset;
	int m_uMinFirstColWidth;
	BOOL m_bHeaderChangesBlocked;
	int m_arrColWidths[MAX_COLUMN_COUNT];
	DWORD m_arrColFormats[MAX_COLUMN_COUNT];
	// 受保护成员函数
protected:
	void Initialize();
	void UpdateColumns();
	void RepositionControls();
	// CTreeCtrl
	DWORD GetStyle() const;
	DWORD GetExStyle() const;
	BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	UINT GetCount() const;
	CImageList* GetImageList(_In_ int nImageList) const;
	CImageList* SetImageList(_In_opt_ CImageList* pImageList, _In_ int nImageList);
	HTREEITEM GetNextItem(_In_ HTREEITEM hItem, _In_ UINT nCode) const;
	HTREEITEM GetPrevSiblingItem(_In_ HTREEITEM hItem) const;
	HTREEITEM GetParentItem(_In_ HTREEITEM hItem) const;
	HTREEITEM GetFirstVisibleItem() const;
	HTREEITEM GetNextVisibleItem(_In_ HTREEITEM hItem) const;
	HTREEITEM GetPrevVisibleItem(_In_ HTREEITEM hItem) const;
	HTREEITEM GetLastVisibleItem() const;
	HTREEITEM GetSelectedItem() const;
	HTREEITEM GetDropHilightItem() const;
	DWORD_PTR GetItemData(_In_ HTREEITEM hItem) const;
	BOOL SetItemImage(_In_ HTREEITEM hItem, _In_ int nImage, _In_ int nSelectedImage);
	BOOL SetItemState(_In_ HTREEITEM hItem, _In_ UINT nState, _In_ UINT nStateMask);
	BOOL SetItemData(_In_ HTREEITEM hItem, _In_ DWORD_PTR dwData);
	BOOL ItemHasChildren(_In_ HTREEITEM hItem) const;
	COLORREF GetBkColor() const;
	COLORREF SetBkColor(_In_ COLORREF clr);
	SHORT GetItemHeight() const;
	SHORT SetItemHeight(_In_ SHORT cyHeight);
	BOOL GetCheck(_In_ HTREEITEM hItem) const;
	BOOL SetCheck(_In_ HTREEITEM hItem, _In_ BOOL fCheck = TRUE);
	COLORREF GetLineColor() const;
	COLORREF SetLineColor(_In_ COLORREF clrNew = CLR_DEFAULT);
	BOOL DeleteItem(_In_ HTREEITEM hItem);
	BOOL Expand(_In_ HTREEITEM hItem, _In_ UINT nCode);
	BOOL Select(_In_ HTREEITEM hItem, _In_ UINT nCode);
	BOOL SelectItem(_In_opt_ HTREEITEM hItem);
	BOOL SelectDropTarget(_In_opt_ HTREEITEM hItem);
	BOOL SelectSetFirstVisible(_In_ HTREEITEM hItem);
	BOOL EnsureVisible(_In_ HTREEITEM hItem);
	// 虚函数
protected:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void PreSubclassWindow();
	virtual void OnDraw(CDC* pDC) {}
	// 消息函数
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCancelMode();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnItemChangedHeader(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangingHeader(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSizeChanged(WPARAM, LPARAM);
	afx_msg void OnNcPaint();
	// 共有成员变量
public:
	// 共有成员函数
public:
	// 插入列
	BOOL DeleteAllItems();
	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = 0, int nWidth = -1, int nSubItem = -1);
	// 删除列
	BOOL DeleteColumn(int nCol);
	// 设置第1列宽度
	void SetFirstColumnMinWidth(UINT uMinWidth);
	// 获取/设置单元格内容
	CString GetItemText(HTREEITEM hItem, int nSubItem);
	void SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszText);
	HTREEITEM GetRootItem() const;
	HTREEITEM GetChildItem(_In_ HTREEITEM hItem) const;
	HTREEITEM GetNextSiblingItem(_In_ HTREEITEM hItem) const;
	// 静态成员变量
public:
	// 静态成员函数
public:
};