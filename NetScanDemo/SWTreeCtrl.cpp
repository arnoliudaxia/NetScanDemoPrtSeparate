#include "stdafx.h"
#include <shlwapi.h>
#include "SWTreeCtrl.h"
 
// IE 5.0 or higher required
#ifndef TVS_NOHSCROLL
#error CSWTreeCtrl requires IE 5.0 or higher; _WIN32_IE should be greater than 0x500.
#endif
 
 
// 绘画树控件横向节点连接虚线
static void _DotHLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w > 0; w -= 2, x += 2)
		SetPixel(hdc, x, y, cr);
}
 
// 绘画树控件竖向节点连接虚线
static void _DotVLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w > 0; w -= 2, y += 2)
		SetPixel(hdc, x, y, cr);
}
 
IMPLEMENT_DYNAMIC(CSWTreeCtrl, CTreeCtrl)
CSWTreeCtrl::CSWTreeCtrl()
{
	CommonConstruct();
}
 
CSWTreeCtrl::~CSWTreeCtrl()
{
}
 
BEGIN_MESSAGE_MAP(CSWTreeCtrl, CTreeCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()
 
void CSWTreeCtrl::CommonConstruct()
{
#ifdef _OWNER_DRAWN_TREE 
	m_bkImage.hbm = NULL;
	m_bkImage.xOffsetPercent = 0;
	m_bkImage.yOffsetPercent = 0;
#endif //_OWNER_DRAWN_TREE
}
 
#ifdef _OWNER_DRAWN_TREE
// 设置树控件+/-按钮图标
void CSWTreeCtrl::SetTreeBtnBitmap(CBitmap* pBitmap, COLORREF clrMask)
{
	ASSERT(pBitmap);
	if (pBitmap && pBitmap->GetSafeHandle())
	{
		m_imgBtns.Add(pBitmap, clrMask);
		BITMAP bmp;
		pBitmap->GetBitmap(&bmp);
		if (m_imgBtns.GetSafeHandle())
			m_imgBtns.DeleteImageList();
		m_imgBtns.Create(bmp.bmWidth / 2, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 2, 1);
	}
}
#endif //_OWNER_DRAWN_TREE
 
BOOL CSWTreeCtrl::GetBkImage(LVBKIMAGE* plvbkImage) const
{
	memcpy(plvbkImage, &m_bkImage, sizeof(LVBKIMAGE));
	return TRUE;
}
 
BOOL CSWTreeCtrl::SetBkImage(LVBKIMAGE* plvbkImage)
{
	memcpy(&m_bkImage, plvbkImage, sizeof(LVBKIMAGE));
	Invalidate();
	return TRUE;
}
 
void CSWTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Invalidate();
 
	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
 
void CSWTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (CheckHit(point))
	{
		HTREEITEM hItem = HitTest(point);
		if (hItem)
		{
#ifdef _OWNER_DRAWN_TREE
			CRect rcItem, rcClient;
			GetClientRect(&rcClient);
			GetItemRect(hItem, &rcItem, FALSE);
			if (rcItem.bottom > rcClient.bottom)
			{
				Invalidate();
				EnsureVisible(hItem);
				SelectItem(hItem);
				return;
			}
#endif //_OWNER_DRAWN_TREE
 
			SelectItem(hItem);
		}
 
		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
	else
	{
		SetFocus();
	}
}
 
void CSWTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (CheckHit(point))
	{
		HTREEITEM hItem = HitTest(point);
		if (hItem)
		{
#ifdef _OWNER_DRAWN_TREE
			CRect rcItem, rcClient;
			GetClientRect(&rcClient);
			GetItemRect(hItem, &rcItem, FALSE);
			if (rcItem.bottom > rcClient.bottom)
			{
				Invalidate();
				CTreeCtrl::OnLButtonDown(nFlags, point);
				return;
			}
#endif //_OWNER_DRAWN_TREE
 
			SelectItem(hItem);
		}
 
		CTreeCtrl::OnLButtonDblClk(nFlags, point);
	}
	else
	{
		SetFocus();
	}
}
 
void CSWTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (CheckHit(point))
	{
		CTreeCtrl::OnMouseMove(nFlags, point);
	}
}
 
#ifdef _OWNER_DRAWN_TREE
LRESULT CSWTreeCtrl::CustomDrawNotify(LPNMTVCUSTOMDRAW lpNMTVCustomDraw)
{
	lpNMTVCustomDraw->nmcd.hdr.hwndFrom = GetSafeHwnd();
	lpNMTVCustomDraw->nmcd.hdr.code = NM_CUSTOMDRAW;
	lpNMTVCustomDraw->nmcd.hdr.idFrom = GetWindowLong(m_hWnd, GWL_ID);
	return GetParent()->SendMessage(WM_NOTIFY, (WPARAM)0, (LPARAM)lpNMTVCustomDraw);
}
 
void CSWTreeCtrl::OwnerDrawBackground(CDC* pDC)
{
	NMTVCUSTOMDRAW stuNMTVCustomDraw;
	DWORD dwFlags;
	DWORD dwRet;
	CRect rcClient;
	GetClientRect(&rcClient);
	memset(&stuNMTVCustomDraw, 0, sizeof(NMTVCUSTOMDRAW));
	stuNMTVCustomDraw.nmcd.dwDrawStage = CDDS_PREPAINT;
	stuNMTVCustomDraw.nmcd.hdc = pDC->m_hDC;
	stuNMTVCustomDraw.nmcd.rc = rcClient;
	dwFlags = (DWORD)CustomDrawNotify(&stuNMTVCustomDraw); // CDDS_PREPAINT
 
	// 填充背景色
	COLORREF crBkgnd = IsWindowEnabled() ? pDC->GetBkColor() : GetSysColor(COLOR_BTNFACE);
	pDC->FillSolidRect(rcClient, crBkgnd);
	if (m_bkImage.hbm && IsWindowEnabled())
	{
		// 画背景图片
		int xOffset = m_nOffsetX;
		int yOffset = rcClient.left;
		int yHeight = rcClient.Height();
		SCROLLINFO srlInfo;
		if (GetScrollInfo(SB_VERT, &srlInfo, SIF_POS | SIF_RANGE))
		{
			yOffset = -srlInfo.nPos;
			yHeight = max(srlInfo.nMax + 1, rcClient.Height());
		}
 
		CDC dcMem;
		dcMem.CreateCompatibleDC(pDC);
		BITMAP bmp;
		::GetObject(m_bkImage.hbm, sizeof(BITMAP), &bmp);
		CBitmap* pOldBitmap =
			dcMem.SelectObject(CBitmap::FromHandle(m_bkImage.hbm));
 
		// copy the background bitmap from temporary DC to painting DC
		float x = (float)m_bkImage.xOffsetPercent / 100 * (float)rcClient.Width();
		float y = (float)m_bkImage.yOffsetPercent / 100 * (float)rcClient.Height();
		pDC->BitBlt(/*xOffset*/+(int)x,
			/*yOffset+*/(int)y,
			bmp.bmWidth, bmp.bmHeight, &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(pOldBitmap);
	}
 
	if (dwFlags&CDRF_NOTIFYPOSTERASE)
	{
		stuNMTVCustomDraw.nmcd.dwDrawStage = CDDS_POSTERASE;
		dwRet = (DWORD)CustomDrawNotify(&stuNMTVCustomDraw); // CDDS_POSTERASE
	}
 
	CFont* pOldFont = pDC->SelectObject(GetFont());
	CImageList* pstateList = GetImageList(TVSIL_STATE);
	CImageList* pimgList = GetImageList(TVSIL_NORMAL);
	CSize szIcon, szStateImg;
	if (pimgList)
	{
		IMAGEINFO ii;
		if (pimgList->GetImageInfo(0, &ii))
			szIcon = CSize(ii.rcImage.right - ii.rcImage.left, ii.rcImage.bottom - ii.rcImage.top);
	}
 
	if (pstateList)
	{
		IMAGEINFO ii;
		if (pstateList->GetImageInfo(0, &ii))
			szStateImg = CSize(ii.rcImage.right - ii.rcImage.left, ii.rcImage.bottom - ii.rcImage.top);
	}
 
	// 开始画树节点
	HTREEITEM hItem = GetFirstVisibleItem();
	while (hItem)
	{
		int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
		DWORD dwStyle = GetStyle();
		DWORD dwState = GetItemState(hItem, 0xFFFF);
		BOOL bEnabled = IsWindowEnabled();
		BOOL bSelected = dwState & TVIS_SELECTED;
		BOOL bHasFocus = (GetFocus() && GetFocus()->m_hWnd == m_hWnd) ? TRUE : FALSE;
 
		// 更新NMCUSTOMDRAW结果内容
		stuNMTVCustomDraw.nmcd.dwItemSpec = (DWORD_PTR)hItem;
		if (bEnabled)
		{
			if (bHasFocus)
			{
				stuNMTVCustomDraw.clrTextBk = bSelected ? GetSysColor(COLOR_HIGHLIGHT) : crBkgnd;
				stuNMTVCustomDraw.clrText = ::GetSysColor(bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT);
				stuNMTVCustomDraw.nmcd.uItemState = dwState | (bSelected ? CDIS_FOCUS : 0);
			}
			else
			{
				if (GetStyle()&TVS_SHOWSELALWAYS)
				{
					stuNMTVCustomDraw.clrTextBk = bSelected ? GetSysColor(COLOR_INACTIVEBORDER) : crBkgnd;
					stuNMTVCustomDraw.clrText = ::GetSysColor(COLOR_MENUTEXT);
				}
				else
				{
					stuNMTVCustomDraw.clrTextBk = crBkgnd;
					stuNMTVCustomDraw.clrText = ::GetSysColor(COLOR_MENUTEXT);
				}
			}
		}
		else
		{
			stuNMTVCustomDraw.clrTextBk = bSelected ? GetSysColor(COLOR_BTNSHADOW) : crBkgnd;
			stuNMTVCustomDraw.clrText = ::GetSysColor(COLOR_GRAYTEXT);
		}
 
		GetItemRect(hItem, &stuNMTVCustomDraw.nmcd.rc, 0);
		CRgn rgn;
		rgn.CreateRectRgn(stuNMTVCustomDraw.nmcd.rc.left, stuNMTVCustomDraw.nmcd.rc.top, stuNMTVCustomDraw.nmcd.rc.left + m_nFirstColumnWidth, stuNMTVCustomDraw.nmcd.rc.bottom);
		pDC->SelectClipRgn(&rgn);
		dwRet = CDRF_DODEFAULT;
		if (dwFlags&CDRF_NOTIFYITEMDRAW)
		{
			stuNMTVCustomDraw.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
			dwRet = (DWORD)CustomDrawNotify(&stuNMTVCustomDraw); // CDDS_ITEMPREPAINT
		}
 
		if (!(dwFlags&CDRF_SKIPDEFAULT))
		{
			// 画树节点图标和+/-按钮
			CRect rcItem;
			int nImage, nSelImage;
			GetItemRect(hItem, &rcItem, TRUE);
			GetItemImage(hItem, nImage, nSelImage);
			int x = rcItem.left - 3;
			int yCenterItem = rcItem.top + (rcItem.bottom - rcItem.top) / 2;
			if (pimgList)
			{
				int nImageIndex = dwState & TVIS_SELECTED ? nImage : nSelImage;
				x -= szIcon.cx + 1;
				pimgList->Draw(pDC, nImageIndex, CPoint(x, yCenterItem - szIcon.cy / 2), ILD_TRANSPARENT);
			}
 
			if (GetStyle()&TVS_CHECKBOXES && pstateList != NULL)
			{
				DWORD dwStateImg = GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12;
				x -= szStateImg.cx;
				pstateList->Draw(pDC, dwStateImg, CPoint(x, yCenterItem - szStateImg.cy / 2), ILD_TRANSPARENT);
			}
 
			if (dwStyle&TVS_HASLINES)
			{
				CPen pen;
				pen.CreatePen(PS_DOT, 1, GetLineColor());
				CPen* pOldPen = pDC->SelectObject(&pen);
				HTREEITEM hItemParent = GetParentItem(hItem);
				if (hItemParent != NULL || dwStyle & TVS_LINESATROOT)
				{
					_DotHLine(pDC->m_hDC, x - szIcon.cx / 2 - 2, yCenterItem, szIcon.cx / 2 + 2, RGB(128, 128, 128));
				}
 
				if (hItemParent != NULL || dwStyle & TVS_LINESATROOT && GetPrevSiblingItem(hItem) != NULL)
				{
					_DotVLine(pDC->m_hDC, x - szIcon.cx / 2 - 2, rcItem.top,
						yCenterItem - rcItem.top, RGB(128, 128, 128));
				}
 
				if (GetNextSiblingItem(hItem) != NULL)
				{
					_DotVLine(pDC->m_hDC, x - szIcon.cx / 2 - 2, yCenterItem,
						rcItem.bottom - yCenterItem, RGB(128, 128, 128));
				}
 
				int x1 = x - szIcon.cx / 2 - 2;
				while (hItemParent != NULL)
				{
					x1 -= szIcon.cx + 3;
					if (GetNextSiblingItem(hItemParent) != NULL)
					{
						_DotVLine(pDC->m_hDC, x1, rcItem.top, rcItem.Height(), RGB(128, 128, 128));
					}
					hItemParent = GetParentItem(hItemParent);
				}
 
				pDC->SelectObject(pOldPen);
 
			}
 
			if (dwStyle&TVS_HASBUTTONS && ItemHasChildren(hItem))
			{
				// 画+/-符号按钮
				int nImg = GetItemState(hItem, TVIF_STATE) & TVIS_EXPANDED ? 1 : 0;
				m_imgBtns.Draw(pDC, nImg, CPoint(x - szIcon.cx / 2 - 6, yCenterItem - 4),	ILD_TRANSPARENT);
			}
		}
 
		pDC->SelectClipRgn(NULL);
		if (dwRet&CDRF_NOTIFYPOSTPAINT)
		{
			stuNMTVCustomDraw.nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
			dwRet = (DWORD)CustomDrawNotify(&stuNMTVCustomDraw); // CDDS_ITEMPOSTPAINT
		}
 
		pDC->SetBkMode(nOldBkMode);
		hItem = GetNextVisibleItem(hItem);
	};
 
	pDC->SelectObject(pOldFont);
}
 
BOOL CSWTreeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	Invalidate();
	return CTreeCtrl::OnMouseWheel(nFlags, zDelta, pt);
}
 
void CSWTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	Invalidate();
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}
#endif //_OWNER_DRAWN_TREE
 
void CSWTreeCtrl::OnPaint()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CPaintDC dc(this);
	CDC dcMem;
	CBitmap bmpMem;
	dcMem.CreateCompatibleDC(&dc);
	if (bmpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height()))
	{
		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
#ifdef _OWNER_DRAWN_TREE	// if owner-drawn
		OwnerDrawBackground(&dcMem);	// use our code
#else						// else use standard code
		CWnd::DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, 0);
#endif //_OWNER_DRAWN_TREE
		dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(pOldBmp);
		bmpMem.DeleteObject();
	}
 
	dcMem.DeleteDC();
}
 
BOOL CSWTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
 
BOOL CSWTreeCtrl::CheckHit(CPoint point)
{
	UINT fFlags;
	HTREEITEM hItem = HitTest(point, &fFlags);
 
	CRect rcItem, rcClient;
	GetClientRect(rcClient);
	GetItemRect(hItem, &rcItem, TRUE);
	if (fFlags & TVHT_ONITEMICON ||
		fFlags & TVHT_ONITEMBUTTON ||
		fFlags & TVHT_ONITEMSTATEICON)
		return TRUE;
 
	if (GetStyle()&TVS_FULLROWSELECT)
	{
		rcItem.right = rcClient.right;
		if (rcItem.PtInRect(point))
			return TRUE;
 
		return FALSE;
	}
 
	if (fFlags & TVHT_ONITEMLABEL)
	{
		rcItem.right = m_nFirstColumnWidth;
		if (!rcItem.PtInRect(point))
			return FALSE;
 
		CString strSub;
		AfxExtractSubString(strSub, GetItemText(hItem), 0, '\t');
		CDC* pDC = GetDC();
		pDC->SelectObject(GetFont());
		rcItem.right = rcItem.left + pDC->GetTextExtent(strSub).cx + 6;
		ReleaseDC(pDC);
		if (!rcItem.PtInRect(point))
			return FALSE;
 
		return TRUE;
	}
 
	return FALSE;
}