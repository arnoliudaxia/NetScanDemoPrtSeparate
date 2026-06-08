// EditList.cpp : implementation file
//

#include "stdafx.h"
//#include "PrintDemo.h"
#include "ImportListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
//CImportListCtrl
//
CImportListCtrl::CImportListCtrl()
{
	crWindow     = ::GetSysColor(COLOR_WINDOW);
	crWindowText = ::GetSysColor(COLOR_WINDOWFRAME);

	m_bEditable=FALSE;
	m_bFixbkWhite=FALSE;
	
	for(int s=0;s<_MAX_SUBITEM;s++)
	{
		for(int i=0;i<_MAX_ITEM;i++)
		{
			fcurvt[s][i]=0.0f;
			m_fVolRage[s][i][0]=8;	//正常取值范围
			m_fVolRage[s][i][1]=22;	//正常取值范围
			m_bErrShow[s][i]=FALSE;
			m_crItemtext[i]=0;//RGB(0,255,255);
		}
	}

	m_nItemCount=3;
//	m_nItemHeight=20;
	m_nSubItemCount=4;

	m_bEnBytesCtl=FALSE;
	m_nBytesPerAd=1;
	m_nRegBytes=4;
	m_pWCallBack=NULL;
	m_nEditCurItem =-1;
	m_nEditCurSubItem =-1;
	m_bEntryOK=FALSE;
	plistFont = new CFont;
	plistFont->CreateFont(20,		// nHeight
     0,                         // nWidth
     0,                         // nEscapement
     0,                         // nOrientation
     FW_NORMAL,                 // nWeight
     FALSE,                     // bItalic
     FALSE,                     // bUnderline
     0,                         // cStrikeOut
     OEM_CHARSET,               // nCharSet
     OUT_DEFAULT_PRECIS,        // nOutPrecision
     CLIP_DEFAULT_PRECIS,       // nClipPrecision
     DEFAULT_QUALITY,           // nQuality
     DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
     _T("Arial"));
//	imFont.CreatePointFont(120,_T("Arial"));
}

CImportListCtrl::~CImportListCtrl()
{
	delete plistFont;
}


BEGIN_MESSAGE_MAP(CImportListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CImportListCtrl)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CImportListCtrl::OnNMCustomdraw)
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportListCtrl message handlers

void CImportListCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
//	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR); pLVCD->iSubItem
	NM_LISTVIEW *phdn = (NM_LISTVIEW*) pNMHDR;
	if(phdn->iSubItem)
		ChangeEditPos(-1,-1);
	*pResult = 0;
}
void CImportListCtrl::ChangeEditPos(int iItem,int iSubItem)
{
	CString strTemp,str2;
	TCHAR szvol[_VOLUM_SIZE];
	UINT    nRegBytes=0;
	int     nadc=m_nRegBytes/m_nBytesPerAd;
	if(m_nEditCurItem>-1&&m_nEditCurSubItem>-1)
	{
		nRegBytes=GetListItemParam(m_nEditCurItem);
		m_cEditCtl.GetWindowText(strTemp);
		memcpy(szvol,strTemp,_VOLUM_SIZE);
		str2=GetItemText(m_nEditCurItem,m_nEditCurSubItem);
		if(strTemp.Compare(str2)!=0)
		{
			double fx=_ttof(szvol);		//原定义为float型
			strTemp.Format(_T("%4.2f"),fx);
			SetItemText(m_nEditCurItem,m_nEditCurSubItem,strTemp);
		}
		m_cEditCtl.SetWindowText(strTemp);
	}	//*/
	if(iItem>-1&&iSubItem>-1)
	{
		CRect ref;
		GetSubItemRect(iItem, iSubItem, LVIR_LABEL , ref );
		m_cEditCtl.SetWindowText(GetItemText(iItem,iSubItem));
		m_cEditCtl.MoveWindow(&ref);
		m_cEditCtl.ShowWindow(SW_SHOW);
		m_cEditCtl.SetFocus();
	}
	m_nEditCurItem = iItem;			//编辑框的位置
	m_nEditCurSubItem =iSubItem;		//
}

int CImportListCtrl::GetListItemParam(int nIndex)
{
	LVITEM lvItem;
	lvItem.mask = LVIF_PARAM;
	lvItem.iSubItem=0;
	lvItem.iItem = nIndex;
	if(GetItem(&lvItem))
		return (int)lvItem.lParam;
	else
		return 4;
}
BOOL CImportListCtrl::SetListItemParam(int nIndex,int nVal)
{
	LVITEM lvItem;
	lvItem.mask = LVIF_PARAM;
	lvItem.iSubItem=0;
	lvItem.iItem = nIndex;
	lvItem.lParam= nVal;
	return SetItem(&lvItem);
}
void CImportListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW *phdn = (NM_LISTVIEW*) pNMHDR;
	long nItem=phdn->iItem;
	ChangeEditPos(-1,-1);
	m_cEditCtl.ShowWindow(SW_HIDE);
	*pResult = 0;
}
void CImportListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW *phdn = (NM_LISTVIEW*) pNMHDR;
	long nItem=phdn->iItem;
	if(m_bEditable)
	{
		if(phdn->iSubItem>0)
		{
			ChangeEditPos(nItem,phdn->iSubItem);
			SetCheck(nItem,TRUE);	//20170714
		}
	}
	*pResult = 0;
}
void CImportListCtrl::EntryPress()
{
	if(m_cEditCtl.IsWindowVisible())
	{
		m_bEntryOK=TRUE;
		ChangeEditPos(-1,-1);
		m_bEntryOK=FALSE;
	}
}

void CImportListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	int nDelArray[100];
	int nip=0;
	if(nChar==46)
	{
		POSITION pos=GetFirstSelectedItemPosition();
		while(pos)
		{
			int nindex=GetNextSelectedItem(pos);
			if((nindex>=0)&&(nip<100))
				nDelArray[nip++]=nindex;
		}
		while(nip>0)
		{
		  nip--;
		  DeleteItem(nDelArray[nip]);
		}
		//SetSelectionMark(nip);
	    //SetHotItem(nindex);
	}
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CImportListCtrl::InitColum(int nSubCount, TCHAR** szHeader,DWORD dwStyle,int nHeaderWidth)
{
//	DWORD dw=LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_TRACKSELECT;
	m_curStyle=dwStyle;
	m_nSubItemCount=nSubCount;
    SetExtendedStyle(dwStyle);
    SetTextColor(RGB(20,100,100));
	SetFont(plistFont,FALSE);
	CRect rcwin;
	GetWindowRect(rcwin);

	LV_COLUMN lvColumn;
	lvColumn.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	lvColumn.fmt=LVCFMT_CENTER;
	lvColumn.cx=nHeaderWidth;
	lvColumn.iSubItem=0;
	lvColumn.pszText=szHeader[0];
	InsertColumn(0,&lvColumn);		//表头

	for(int i=1;i<nSubCount;i++)
	{
		lvColumn.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
		lvColumn.fmt=LVCFMT_CENTER;
		lvColumn.cx=((rcwin.Width()-nHeaderWidth)/(nSubCount-1));
		lvColumn.iSubItem=i;
//	#ifdef UNICODE
//		USES_CONVERSION;
		lvColumn.pszText=szHeader[i];
//	#else
//		lvColumn.pszText=szHeader[i];
//	#endif
		InsertColumn(i,&lvColumn);
	}

	CreateEditCtrl();
}

void CImportListCtrl::InitItem(int nItem, TCHAR* szHeader,COLORREF crItemtext)
{
	m_crItemtext[nItem]=crItemtext;

	LVITEM curItem;
	curItem.mask=LVIF_TEXT|LVIF_IMAGE;
	curItem.lParam=NULL;
	curItem.iSubItem=0;
	curItem.iImage=0;
	curItem.iItem=nItem;
	curItem.pszText=(LPTSTR)(LPCTSTR)szHeader;
	InsertItem(&curItem);
}

void CImportListCtrl::InitItem(int nItemCount, TCHAR** szHeader,COLORREF crItemtext[])
{
	CRect rcwin;
	GetWindowRect(rcwin);

	m_nItemCount=nItemCount;
//	m_nItemHeight=rcwin.Height()/nItemCount;

	for(int i=0;i<nItemCount;i++)
		InitItem(i,szHeader[i],crItemtext[i]);
}

void CImportListCtrl::CreateEditCtrl()
{
	m_cEditCtl.Create(WS_CHILD|WS_BORDER|ES_AUTOHSCROLL,CRect(10,10,100,100),this,1);  	
    m_cEditCtl.ShowWindow(SW_HIDE);
	m_cEditCtl.SetFont(plistFont,FALSE);
}

void CImportListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// TODO: Add your control notification handler code here
	CString szText;
	char szvol[_VOLUM_SIZE];

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
        *pResult = CDRF_NOTIFYITEMDRAW;  
	}
    else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
        // This is the notification message for an item. We'll request  
        // notifications before each subitem's prepaint stage.  
        *pResult = CDRF_NOTIFYSUBITEMDRAW;  
	}
    else if((CDDS_ITEMPREPAINT|CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF clrNewTextColor, clrNewBkColor;
        int nItem=static_cast<int>(pLVCD->nmcd.dwItemSpec );	//行
		int nSubItem=pLVCD->iSubItem;							//列
		szText=GetItemText(nItem,nSubItem);
		memcpy(szvol,szText,_VOLUM_SIZE);
////		if(!szText.IsEmpty())
////			TRACE("行：%d 列：%d---%s\r\n",nItem,nSubItem,szText);
		if((nItem<GetItemCount())&&(nSubItem<m_nSubItemCount))
			fcurvt[nSubItem][nItem]=atof(szvol);
		if(0==nSubItem)
		{
			//clrNewTextColor = RGB(0,0,0);
			clrNewTextColor = RGB(GetRValue(m_crItemtext[nItem])/2,GetGValue(m_crItemtext[nItem])/2,GetBValue(m_crItemtext[nItem])/2);
			clrNewBkColor = RGB(255,255,255)-clrNewTextColor;
			if(m_bFixbkWhite)
				clrNewBkColor = RGB(255,255,255);
 
			pLVCD->clrText =clrNewTextColor;  
			pLVCD->clrTextBk =clrNewBkColor;  
			*pResult = CDRF_DODEFAULT;    
		}
		else
		{
			if(m_bErrShow[nSubItem][nItem])
			{
				if(fcurvt[nSubItem][nItem]<m_fVolRage[nSubItem][nItem][0]||fcurvt[nSubItem][nItem]>m_fVolRage[nSubItem][nItem][1])
				{
					clrNewTextColor = RGB(255,0,0);  
					clrNewBkColor = RGB(255,255,128);
				}
				else{
					//clrNewTextColor = RGB(0,0,0);  
					clrNewTextColor = m_crItemtext[nItem];//RGB(198,202,198);
					clrNewBkColor = RGB(255,255,255)-clrNewTextColor;
					if(m_bFixbkWhite)
						clrNewBkColor = RGB(255,255,255);
				}
			}
			else
			{
				//clrNewTextColor = RGB(0,0,0);  
				clrNewTextColor = m_crItemtext[nItem];//RGB(255,255,255);
				clrNewBkColor = RGB(255,255,255)-clrNewTextColor;
				if(m_bFixbkWhite)
					clrNewBkColor = RGB(255,255,255);
			}

			pLVCD->clrText = clrNewTextColor;
			pLVCD->clrTextBk = clrNewBkColor; 
			*pResult = CDRF_DODEFAULT;  
		}
    }  
//	*pResult = 0;
}

void CImportListCtrl::EnableInput(BOOL bEditable)
{
	m_bEditable=bEditable;

	if(bEditable)
		m_curStyle|=LVS_EX_CHECKBOXES;
	else
	{
		m_curStyle&=~LVS_EX_CHECKBOXES;
		m_cEditCtl.ShowWindow(SW_HIDE);
	}
	SetExtendedStyle(m_curStyle);
//	if(m_hWnd)
//		Invalidate();
}

void CImportListCtrl::SetValumRage(POINT item,RAGE valrage,BOOL bErrShow)
{
	m_bErrShow[item.y][item.x]=bErrShow;

	m_fVolRage[item.y][item.x][0]=valrage.fminvol;
	m_fVolRage[item.y][item.x][1]=valrage.fmaxvol;
}


void CImportListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default
//	lpMeasureItemStruct->itemHeight = m_nItemHeight;
	CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}


void CImportListCtrl::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	/*CRect rcwin;
	GetWindowRect(rcwin);

	m_nItemHeight=rcwin.Height()/m_nItemCount;

	WINDOWPOS wp;
	wp.hwnd=m_hWnd;
	wp.cx=rcwin.Width();
	wp.cy=rcwin.Height();
	wp.flags=SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER;
	SendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&wp);*/

	CListCtrl::PreSubclassWindow();
}


void CImportListCtrl::SetListFont(int nHeight, LPCTSTR lpszFontName,BOOL bFixbkWhite)
{
	m_bFixbkWhite=bFixbkWhite;

	if(plistFont)
		delete plistFont;
	plistFont = new CFont;
	plistFont->CreateFont(nHeight,		// nHeight
     0,                         // nWidth
     0,                         // nEscapement
     0,                         // nOrientation
     FW_NORMAL,                 // nWeight
     FALSE,                     // bItalic
     FALSE,                     // bUnderline
     0,                         // cStrikeOut
     OEM_CHARSET,               // nCharSet
     OUT_DEFAULT_PRECIS,        // nOutPrecision
     CLIP_DEFAULT_PRECIS,       // nClipPrecision
     DEFAULT_QUALITY,           // nQuality
     DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
     lpszFontName);

	SetFont(plistFont,TRUE);
}
void CImportListCtrl::UpdateEditValue()
{
	CString strTemp;
	TCHAR szvol[_VOLUM_SIZE];
	m_cEditCtl.GetWindowText(strTemp);
	memcpy(szvol,strTemp,_VOLUM_SIZE);
	double fx=_ttof(szvol);		//原定义为float型
	strTemp.Format(_T("%4.2f"),fx);
	SetItemText(m_nEditCurItem,m_nEditCurSubItem,strTemp);
	m_cEditCtl.ShowWindow(SW_HIDE);
}
void CImportListCtrl::GetLineData(int nItem,float* pDataBuf,BOOL bAbsolute)
{
	UpdateEditValue();
	for(int s=1;s<m_nSubItemCount;s++)
	{
		pDataBuf[(s-1)]=(float)_ttof(GetItemText(nItem,s));//(bAbsolute?abs(fcurvt[s][nItem]):fcurvt[s][nItem]);
		if(bAbsolute&&pDataBuf[(s-1)]<0)
			pDataBuf[(s-1)]=-pDataBuf[(s-1)];
	}
}

void CImportListCtrl::SetListData(float** fSetVal)
{
	CString szTxt;
	for(int s=1;s<m_nSubItemCount;s++)
	{
		for(int i=0;i<m_nItemCount;i++)
		{
			szTxt.Format(_T("%0.2f"),fSetVal[i][s]);//pDataBuf[(s-1)*m_nSubItemCount+i]);
			SetItemText(i,s,szTxt);
		}
	}
}

void CImportListCtrl::GetListData(float** fGetVal)
{
	UpdateEditValue();
	for(int s=1;s<m_nSubItemCount;s++)
	{
		for(int i=0;i<m_nItemCount;i++)
		{
			fGetVal[i][s]=(float)_ttof(GetItemText(i,s));
//			pDataBuf[(s-1)*m_nSubItemCount+i]=(float)atof(GetItemText(i,s));//fcurvt[s][i];
		}
	}
}
