#include "stdafx.h"
#include <shlwapi.h>
#include "RyTreeListCtl.h"
 
#define COLUMN_MARGIN		1 // 列表项分割间距
#ifdef _OWNER_DRAWN_TREE
	#define DEFMINFIRSTCOLWIDTH 0	
#else
	#define DEFMINFIRSTCOLWIDTH 100
#endif
 
IMPLEMENT_DYNCREATE(CSWListTreeCtrl, CWnd)
CSWListTreeCtrl::CSWListTreeCtrl()
{
	CommonConstruct();
}
 
CSWListTreeCtrl::~CSWListTreeCtrl()
{
}
 
BEGIN_MESSAGE_MAP(CSWListTreeCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_SETTINGCHANGE()
	ON_WM_ENABLE()
	ON_NOTIFY(HDN_ITEMCHANGED, HeaderID, OnItemChangedHeader)
	ON_NOTIFY(HDN_ITEMCHANGING, HeaderID, OnItemChangingHeader)
	ON_MESSAGE(WM_USER + 1, OnSizeChanged)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()
 
void CSWListTreeCtrl::CommonConstruct()
{
	m_uMinFirstColWidth = DEFMINFIRSTCOLWIDTH;
	m_bHeaderChangesBlocked = FALSE;
	m_xOffset = 0;
}
 
BOOL CSWListTreeCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd::Create(_T(""), _T(""), dwStyle, rect, pParentWnd, nID);
	Initialize();
 
	return TRUE;
}
 
void CSWListTreeCtrl::PreSubclassWindow()
{
	Initialize();
}
 
void CSWListTreeCtrl::Initialize()
{
	if (m_childTree.m_hWnd) 
		return;
 
	CRect rcClient;
	GetClientRect(&rcClient);
	m_childTree.Create(WS_CHILD | WS_VISIBLE  | TVS_NOHSCROLL | TVS_NOTOOLTIPS|TVS_HASLINES |TVS_LINESATROOT |TVS_HASBUTTONS, CRect(), this, TreeID);
	m_childHeader.Create(WS_CHILD | HDS_BUTTONS | WS_VISIBLE | HDS_FULLDRAG  , rcClient, this, HeaderID);
	m_childHeader2.Create(WS_CHILD , rcClient, this, Header2ID);
	m_childScrollBar.Create(SBS_HORZ | WS_CHILD | SBS_BOTTOMALIGN, rcClient, this, HScrollID);
	CFont* pFont = m_childTree.GetFont();
	m_childHeader.SetFont(pFont);
	m_childHeader2.SetFont(pFont);
	BOOL bIsComCtl6 = FALSE;
	HMODULE hComCtlDll = LoadLibrary(_T("comctl32.dll"));
	if (hComCtlDll)
	{
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);
		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");
		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			HRESULT hRes = (*pfnDllGetVersion)(&dvi);
			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6)
				bIsComCtl6 = TRUE;
		}
 
		FreeLibrary(hComCtlDll);
	}
 
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_childHeader.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;
	m_xOffset = bIsComCtl6 ? 9 : 6;
	m_xPos = 0;
	UpdateColumns();
}
 
void CSWListTreeCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	m_childTree.SendMessage(WM_SETTINGCHANGE);
	m_childScrollBar.SendMessage(WM_SETTINGCHANGE);
	CRect rcClient;
	GetClientRect(&rcClient);
	m_childHeader.SendMessage(WM_SETTINGCHANGE);
	m_childHeader2.SendMessage(WM_SETTINGCHANGE);
	m_childHeader.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
	m_childHeader2.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_childHeader.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;
	RepositionControls();
}
 
void CSWListTreeCtrl::OnPaint()
{
	CPaintDC dc(this);
	CRect rcClient;
	GetClientRect(&rcClient);
	int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	CBrush brush;
	brush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	CRect rcRbBkgr;// 右下角背景区域
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	m_childTree.GetScrollInfo(SB_VERT, &scrinfo, SIF_ALL);
	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax ? TRUE : FALSE;
	if(bVScrollVisible)
	{
		rcRbBkgr.SetRect(rcClient.right-cxVScroll, rcClient.bottom-cyHScroll,
				rcClient.right, rcClient.bottom);
		dc.FillRect(rcRbBkgr, &brush);
	}
}
 
BOOL CSWListTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	// 所有背景绘制在OnPaint完成，此处返回FALSE即可
	return FALSE;
}
 
void CSWListTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	PostMessage(WM_USER + 1);
}
 
void CSWListTreeCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	m_childTree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax ? TRUE : FALSE;
	int cxTotal = m_cxTotal + (bVScrollVisible ? GetSystemMetrics(SM_CXVSCROLL) : 0);
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();
	int xLast = m_xPos;
	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_xPos -= 15;
		break;
	case SB_LINERIGHT:
		m_xPos += 15;
		break;
	case SB_PAGELEFT:
		m_xPos -= cx;
		break;
	case SB_PAGERIGHT:
		m_xPos += cx;
		break;
	case SB_LEFT:
		m_xPos = 0;
		break;
	case SB_RIGHT:
		m_xPos = cxTotal - cx;
		break;
	case SB_THUMBTRACK:
		m_xPos = nPos;
		break;
	}
 
	if (m_xPos < 0)
		m_xPos = 0;
	else if (m_xPos > cxTotal - cx)
		m_xPos = cxTotal - cx;
 
	if (xLast == m_xPos)
		return;
 
	m_childTree.m_nOffsetX = m_xPos;
	SetScrollPos(SB_HORZ, m_xPos);
	CWnd::OnHScroll(nSBCode,nPos,pScrollBar);
	RepositionControls();
}
 
void CSWListTreeCtrl::OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = FALSE;
	if (m_bHeaderChangesBlocked)
	{
		m_bHeaderChangesBlocked = FALSE;
		*pResult = TRUE;
		return;
	}
	
	LPNMHEADER pnm = (LPNMHEADER)pNMHDR;
	if (pnm->iItem == 0 )
	{
		CRect rcItem;
		m_childHeader.GetItemRect(0, &rcItem);
		if (pnm->pitem->cxy < m_uMinFirstColWidth)
		{
			pnm->pitem->cxy = m_uMinFirstColWidth+1;
			*pResult = TRUE;
		}
	}
}
 
void CSWListTreeCtrl::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColumns();
	m_childTree.Invalidate();
}
 
void CSWListTreeCtrl::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCUSTOMDRAW* pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
	NMTVCUSTOMDRAW* pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;
 
	switch (pNMCustomDraw->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
 
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		break;
 
	case CDDS_ITEMPOSTPAINT:
		{
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
			CRect rcItem = pNMCustomDraw->rc;
			if (rcItem.IsRectEmpty())
			{
				*pResult = CDRF_DODEFAULT;
				break;
			}
 
			CDC dc;
			dc.Attach(pNMCustomDraw->hdc);
			CRect rcLabel;
			m_childTree.GetItemRect(hItem, &rcLabel, TRUE);
			COLORREF clrTextBk = pNMTVCustomDraw->clrTextBk;
			COLORREF crWnd = GetSysColor((IsWindowEnabled()?COLOR_WINDOW:COLOR_BTNFACE));
			
#ifndef _OWNER_DRAWN_TREE
			dc.FillSolidRect(&rcLabel, crWnd);
#endif //_OWNER_DRAWN_TREE
 
			int nCount = m_childHeader.GetItemCount();
			int xOffset = 0;
			for (int i=0; i< nCount; i++)
			{
				xOffset += m_arrColWidths[i];
				rcItem.right = xOffset;
				dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_RIGHT);
			}
 
			dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_BOTTOM);
			CString strText = m_childTree.GetItemText(hItem);
			CString strSub;
			AfxExtractSubString(strSub, strText, 0, '\t');
			CRect rcText(0,0,0,0);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_CALCRECT);
			rcLabel.right = min(rcLabel.left + rcText.right + 4, m_arrColWidths[0] - 4);
			BOOL bFullRowSelect = m_childTree.GetStyle() & TVS_FULLROWSELECT;
			if (rcLabel.Width() < 0)
				clrTextBk = crWnd;
			if (clrTextBk != crWnd)
			{
				CRect rcSelect =  rcLabel;
				if(bFullRowSelect) rcSelect.right = rcItem.right;
				dc.FillSolidRect(&rcSelect, clrTextBk);
				if (pNMCustomDraw->uItemState & CDIS_FOCUS)
					dc.DrawFocusRect(&rcSelect);
			}
			
			CFont* pOldFont = NULL;
			if (m_childTree.GetStyle()&TVS_TRACKSELECT && pNMCustomDraw->uItemState && CDIS_HOT)
			{
				LOGFONT lf;
				pOldFont = m_childTree.GetFont();
				pOldFont->GetLogFont(&lf);
				lf.lfUnderline = 1;
				CFont newFont;
				newFont.CreateFontIndirect(&lf);
				dc.SelectObject(newFont);
			}
 
			rcText = rcLabel;
			rcText.DeflateRect(2, 1);
			dc.SetTextColor(pNMTVCustomDraw->clrText);
			dc.DrawText(strSub, &rcText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
			xOffset = m_arrColWidths[0];
			dc.SetBkMode(TRANSPARENT);
			if (IsWindowEnabled() && !bFullRowSelect)	
				dc.SetTextColor(::GetSysColor(COLOR_MENUTEXT));
			
			if( pOldFont &&  !(m_childTree.GetStyle()& TVS_FULLROWSELECT)) 
				dc.SelectObject(pOldFont);
 
			for (int i=1; i<nCount; i++)
			{
				if (AfxExtractSubString(strSub, strText, i, '\t'))
				{
					rcText = rcLabel;
					rcText.left = xOffset+ COLUMN_MARGIN;
					rcText.right = xOffset + m_arrColWidths[i]-COLUMN_MARGIN;
					rcText.DeflateRect(m_xOffset, 1, 2, 1);
					if(rcText.left<0 || rcText.right<0 || rcText.left>=rcText.right)
					{
						xOffset += m_arrColWidths[i];
						continue;
					}
 
					DWORD dwFormat = m_arrColFormats[i] & HDF_RIGHT ?
						DT_RIGHT : (m_arrColFormats[i] & HDF_CENTER ? DT_CENTER : DT_LEFT);
					dc.DrawText(strSub, &rcText, DT_SINGLELINE |DT_VCENTER 
						| DT_NOPREFIX | DT_END_ELLIPSIS | dwFormat);
				}
				xOffset += m_arrColWidths[i];
			}
 
			if(pOldFont) dc.SelectObject(pOldFont);
			dc.Detach();
		}
 
		*pResult = CDRF_DODEFAULT;
		break;
 
	default:
		*pResult = CDRF_DODEFAULT;
	}
}
 
void CSWListTreeCtrl::UpdateColumns()
{
	m_cxTotal = 0;
	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	int nCount = m_childHeader.GetItemCount();
	ASSERT(nCount <=MAX_COLUMN_COUNT);
	for (int i=0; i< nCount; i++)
	{
		if (m_childHeader.GetItem(i, &hditem))
		{
			m_cxTotal += m_arrColWidths[i] = hditem.cxy;
			if (i==0)
				m_childTree.m_nFirstColumnWidth = hditem.cxy;
		}
	}
 
	m_bHeaderChangesBlocked = TRUE;
	RepositionControls();
}
 
LRESULT CSWListTreeCtrl::OnSizeChanged(WPARAM, LPARAM)
{
	RepositionControls();
	return S_OK;
}
 
void CSWListTreeCtrl::RepositionControls()
{
	if (m_childTree.m_hWnd)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		int cx = rcClient.Width();
		int cy = rcClient.Height();
		int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
		int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
		SCROLLINFO scrinfo;
		scrinfo.cbSize = sizeof(scrinfo);
		m_childTree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
		BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax ? TRUE : FALSE;
		int cxTotal = m_cxTotal + (bVScrollVisible ? cxVScroll : 0);
		if (m_xPos > cxTotal - cx) m_xPos = cxTotal - cx;
		if (m_xPos < 0)	m_xPos = 0;
		scrinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrinfo.nPage = cx;
		scrinfo.nMin = 0;
		scrinfo.nMax = cxTotal;
		scrinfo.nPos = m_xPos;
		m_childScrollBar.SetScrollInfo(&scrinfo);
		CRect rcTree;
		m_childTree.GetClientRect(&rcTree);
		int x = 0;
		if (cx < cxTotal)
		{
			x = m_childScrollBar.GetScrollPos();
			cx += x;
		}
	
		BOOL bHScrollVisible = rcClient.Width() < cxTotal && rcClient.Height() >= (cyHScroll + m_cyHeader);
		m_childScrollBar.ShowWindow(bHScrollVisible?SW_SHOW:SW_HIDE);
		m_childHeader.MoveWindow(-x, 0, cx  - (bVScrollVisible?cxVScroll:0), m_cyHeader);
		m_childHeader2.MoveWindow(rcClient.Width()-cxVScroll, 0, cxVScroll, m_cyHeader);
		m_childTree.MoveWindow(-x, m_cyHeader, cx, cy-m_cyHeader-(bHScrollVisible?cyHScroll:0));
		m_childScrollBar.MoveWindow(0, rcClient.Height() - cyHScroll, rcClient.Width() - (bVScrollVisible ? cxVScroll : 0), cyHScroll);
		m_childHeader2.ShowWindow(bVScrollVisible?SW_SHOW:SW_HIDE);
		RedrawWindow();
	}
}
 
DWORD CSWListTreeCtrl::GetStyle() const
{
	return m_childTree.GetStyle();
}
 
DWORD CSWListTreeCtrl::GetExStyle() const
{
	return m_childTree.GetExStyle();
}
 
BOOL CSWListTreeCtrl::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return m_childTree.ModifyStyle(dwRemove, dwAdd, nFlags);
}
 
BOOL CSWListTreeCtrl::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return m_childTree.ModifyStyleEx(dwRemove, dwAdd, nFlags);
}
 
UINT CSWListTreeCtrl::GetCount() const
{
	return m_childTree.GetCount();
}
 
CImageList* CSWListTreeCtrl::GetImageList(_In_ int nImageList) const
{
	return m_childTree.GetImageList(nImageList);
}
 
CImageList* CSWListTreeCtrl::SetImageList(_In_opt_ CImageList* pImageList, _In_ int nImageList)
{
	return m_childTree.SetImageList(pImageList, nImageList);
}
 
HTREEITEM CSWListTreeCtrl::GetNextItem(_In_ HTREEITEM hItem, _In_ UINT nCode) const
{
	return m_childTree.GetNextItem(hItem, nCode);
}
 
HTREEITEM CSWListTreeCtrl::GetChildItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetChildItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetNextSiblingItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetNextSiblingItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetPrevSiblingItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetPrevSiblingItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetParentItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetParentItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetFirstVisibleItem() const
{
	return m_childTree.GetFirstVisibleItem();
}
 
HTREEITEM CSWListTreeCtrl::GetNextVisibleItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetNextVisibleItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetPrevVisibleItem(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetPrevVisibleItem(hItem);
}
 
HTREEITEM CSWListTreeCtrl::GetLastVisibleItem() const
{
	return m_childTree.GetLastVisibleItem();
}
 
HTREEITEM CSWListTreeCtrl::GetSelectedItem() const
{
	return m_childTree.GetSelectedItem();
}
 
HTREEITEM CSWListTreeCtrl::GetDropHilightItem() const
{
	return m_childTree.GetDropHilightItem();
}
 
HTREEITEM CSWListTreeCtrl::GetRootItem() const
{
	return m_childTree.GetRootItem();
}
 
DWORD_PTR CSWListTreeCtrl::GetItemData(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetItemData(hItem);
}
 
BOOL CSWListTreeCtrl::SetItemImage(_In_ HTREEITEM hItem, _In_ int nImage, _In_ int nSelectedImage)
{
	return m_childTree.SetItemImage(hItem, nImage, nSelectedImage);
}
 
BOOL CSWListTreeCtrl::SetItemState(_In_ HTREEITEM hItem, _In_ UINT nState, _In_ UINT nStateMask)
{
	return m_childTree.SetItemState(hItem, nState, nStateMask);
}
 
BOOL CSWListTreeCtrl::SetItemData(_In_ HTREEITEM hItem, _In_ DWORD_PTR dwData)
{
	return m_childTree.SetItemData(hItem, dwData);
}
 
BOOL CSWListTreeCtrl::ItemHasChildren(_In_ HTREEITEM hItem) const
{
	return m_childTree.ItemHasChildren(hItem);
}
 
COLORREF CSWListTreeCtrl::GetBkColor() const
{
	return m_childTree.GetBkColor();
}
 
COLORREF CSWListTreeCtrl::SetBkColor(_In_ COLORREF clr)
{
	return m_childTree.SetBkColor(clr);
}
 
SHORT CSWListTreeCtrl::GetItemHeight() const
{
	return m_childTree.GetItemHeight();
}
 
SHORT CSWListTreeCtrl::SetItemHeight(_In_ SHORT cyHeight)
{
	return m_childTree.SetItemHeight(cyHeight);
}
 
BOOL CSWListTreeCtrl::GetCheck(_In_ HTREEITEM hItem) const
{
	return m_childTree.GetCheck(hItem);
}
 
BOOL CSWListTreeCtrl::SetCheck(_In_ HTREEITEM hItem, _In_ BOOL fCheck/* = TRUE*/)
{
	return m_childTree.SetCheck(hItem, fCheck);
}
 
COLORREF CSWListTreeCtrl::GetLineColor() const
{
	return m_childTree.GetLineColor();
}
 
COLORREF CSWListTreeCtrl::SetLineColor(_In_ COLORREF clrNew/* = CLR_DEFAULT*/)
{
	return m_childTree.SetLineColor(clrNew);
}
 
BOOL CSWListTreeCtrl::DeleteItem(_In_ HTREEITEM hItem)
{
	return m_childTree.DeleteItem(hItem);
}
 
BOOL CSWListTreeCtrl::DeleteAllItems()
{
	return m_childTree.DeleteAllItems();
}
 
BOOL CSWListTreeCtrl::Expand(_In_ HTREEITEM hItem, _In_ UINT nCode)
{
	return m_childTree.Expand(hItem, nCode);
}
 
BOOL CSWListTreeCtrl::Select(_In_ HTREEITEM hItem, _In_ UINT nCode)
{
	return m_childTree.Select(hItem, nCode);
}
 
BOOL CSWListTreeCtrl::SelectItem(_In_opt_ HTREEITEM hItem)
{
	return m_childTree.SelectItem(hItem);
}
 
BOOL CSWListTreeCtrl::SelectDropTarget(_In_opt_ HTREEITEM hItem)
{
	return m_childTree.SelectDropTarget(hItem);
}
 
BOOL CSWListTreeCtrl::SelectSetFirstVisible(_In_ HTREEITEM hItem)
{
	return m_childTree.SelectSetFirstVisible(hItem);
}
 
BOOL CSWListTreeCtrl::EnsureVisible(_In_ HTREEITEM hItem)
{
	return m_childTree.EnsureVisible(hItem);
}
 
int CSWListTreeCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	CHeaderCtrl& header = GetHeaderCtrl();
	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = nFormat;
	hditem.cxy = nWidth;
	hditem.pszText = (LPTSTR)lpszColumnHeading;
	m_arrColFormats[nCol] = nFormat;
	int nIndex =  header.InsertItem(nCol, &hditem);
	if(m_childHeader.GetItemCount()>0) 
	{
		hditem.pszText = _T("");
		hditem.cxy = GetSystemMetrics(SM_CXVSCROLL)+5;
		m_childHeader2.InsertItem(0, &hditem);
	};
 
	UpdateColumns();	
	return nIndex;
}
 
BOOL  CSWListTreeCtrl::DeleteColumn(int nCol)
{
	BOOL bResult = m_childHeader.DeleteItem(nCol);
	if(m_childHeader.GetItemCount()==0) 
	{
		m_childHeader2.DeleteItem(0);
	}
 
	UpdateColumns();
	return bResult;
}
 
CString CSWListTreeCtrl::GetItemText(HTREEITEM hItem, int nColumn)
{
	CString szText = m_childTree.GetItemText(hItem);
	CString szSubItem;
	AfxExtractSubString(szSubItem,szText,nColumn,'\t');
	return szSubItem;
}
 
void CSWListTreeCtrl::SetItemText(HTREEITEM hItem,int nColumn,LPCTSTR lpszItem)
{
	CString szText = m_childTree.GetItemText(hItem);
	CString szNewText, szSubItem;
	for (int i=0; i < m_childHeader.GetItemCount();i++)
	{
		AfxExtractSubString(szSubItem,szText,i,'\t');
		if(i!=nColumn) szNewText += szSubItem+_T("\t");
		else szNewText += CString(lpszItem)+_T("\t");
	}
 
	m_childTree.SetItemText(hItem,szNewText);
}
 
void CSWListTreeCtrl::SetFirstColumnMinWidth(UINT uMinWidth)
{
	m_uMinFirstColWidth = uMinWidth;
}
 
HTREEITEM CSWListTreeCtrl::HitTest(CPoint pt, UINT* pFlags) const
{
	CSWListTreeCtrl::CTVHITTESTINFO htinfo = {pt, 0, NULL, 0};
	HTREEITEM hItem = HitTest(&htinfo);
	if(pFlags)
	{
		*pFlags = htinfo.flags;
	}
 
	return hItem;
}
 
HTREEITEM CSWListTreeCtrl::HitTest(CSWListTreeCtrl::CTVHITTESTINFO* pHitTestInfo) const
{
	UINT uFlags = 0;
	HTREEITEM hItem = NULL;
	CRect rcItem, rcClient;
	CPoint point = pHitTestInfo->pt;
	point.x += m_xPos;
	point.y -= m_cyHeader;
	hItem = m_childTree.HitTest(point, &uFlags);
	pHitTestInfo->hItem = hItem;
	pHitTestInfo->flags = uFlags;
	pHitTestInfo->iSubItem = 0;
	if (! (uFlags&TVHT_ONITEMLABEL || uFlags&TVHT_ONITEMRIGHT) )
		return hItem;
 
	int x = 0;
	for (int i=0; i<m_childHeader.GetItemCount(); i++)
	{
		if (point.x>=x && point.x<=x+m_arrColWidths[i])
		{
			pHitTestInfo->iSubItem = i;
			pHitTestInfo->flags = TVHT_ONITEMLABEL;
			return hItem;
		}
 
		x += m_arrColWidths[i];
	}	
	
	pHitTestInfo->hItem = NULL;
	pHitTestInfo->flags = TVHT_NOWHERE;
	return NULL;
}
 
 
void CSWListTreeCtrl::OnItemChangedHeader(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnHeaderItemChanged(pNMHDR, pResult);
}
 
void CSWListTreeCtrl::OnItemChangingHeader(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnHeaderItemChanging(pNMHDR, pResult);
}
 
BOOL CSWListTreeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	LPNMHDR pHdr = (LPNMHDR)lParam;
	if(pHdr->code==NM_CUSTOMDRAW && pHdr->idFrom == TreeID)
	{
		OnTreeCustomDraw(pHdr,pResult);
		return TRUE;
	}
	
	if(pHdr->code==HDN_ITEMCHANGING && pHdr->idFrom == HeaderID)
	{
		OnHeaderItemChanging(pHdr,pResult);
	}
	
	if(pHdr->code==HDN_ITEMCHANGED && pHdr->idFrom == HeaderID)
	{
		OnHeaderItemChanged(pHdr,pResult);
	}
 
#ifdef _OWNER_DRAWN_TREE
	if(pHdr->code==TVN_ITEMEXPANDING && pHdr->idFrom == TreeID)
	{
		Invalidate();
	}
#endif //_OWNER_DRAWN_TREE
 
	if(pHdr->code==TVN_ITEMEXPANDED && pHdr->idFrom == TreeID)
	{
		RepositionControls(); 
	}
 
	pHdr->hwndFrom = GetSafeHwnd();
	pHdr->idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
	return (BOOL)GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);
}
 
void CSWListTreeCtrl::OnCancelMode()
{
	m_childHeader.SendMessage(WM_CANCELMODE);
	m_childHeader2.SendMessage(WM_CANCELMODE);
	m_childTree.SendMessage(WM_CANCELMODE);
	m_childScrollBar.SendMessage(WM_CANCELMODE);
}
 
void CSWListTreeCtrl::OnEnable(BOOL bEnable)
{
	m_childHeader.EnableWindow(bEnable);
	m_childHeader2.EnableWindow(bEnable);
	m_childTree.EnableWindow(bEnable);
	m_childScrollBar.EnableWindow(bEnable);
}
 
void CSWListTreeCtrl::OnNcPaint()
{
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnNcPaint()
	PostMessage(WM_USER + 1);
}