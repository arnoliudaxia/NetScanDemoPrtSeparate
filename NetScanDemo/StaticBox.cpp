// StaticBox.cpp : implementation file
//

#include "stdafx.h"
#include "StaticBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticBox
#define _BACK_COLOR RGB(192,192,192)
CStaticBox::CStaticBox()
{
	m_nInfoType=0;
	m_szText=_T("");
	m_bFlashMode=FALSE;
	m_bFlashState=FALSE;
	m_nColor=RGB(0,220,0);
	m_rectClient.left=0;
	m_rectClient.top=0;
	m_rectClient.bottom=20;
	m_rectClient.right=20;
}

CStaticBox::~CStaticBox()
{
}


BEGIN_MESSAGE_MAP(CStaticBox, CStatic)
	//{{AFX_MSG_MAP(CStaticBox)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	//ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticBox message handlers

void CStaticBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	GetClientRect(&m_rectClient);
    DrawItem(&dc);
	// TODO: Add your message handler code here
	
	// Do not call CStatic::OnPaint() for painting messages
}
void  CStaticBox::DrawItem(CDC* pDC)
{
	BOOL bGetDC=FALSE;
	if(pDC==NULL)
	{
		pDC=GetDC();
		bGetDC=TRUE;
	}
	if(!m_bFlashState)
	{
	   RECT rectItem=m_rectClient;
	   int nGrids=(m_rectClient.bottom-m_rectClient.top)/60;
	   if(nGrids<1)
		   nGrids=1;
	   int nColorDec=60/(m_rectClient.bottom-m_rectClient.top);
	   if(nColorDec<1)
		   nColorDec=1;
	   int nCurDecColor=0;
	   for(int i=m_rectClient.top;i<m_rectClient.bottom;i+=nGrids)
	   {
		   rectItem.top=i;
		   rectItem.bottom=i+nGrids;
		   if(nCurDecColor>30)
			  nCurDecColor-=nColorDec;
		   else
			  nCurDecColor+=nColorDec;
		   if(m_nInfoType==0)
		      m_nColor=RGB(nCurDecColor,220+nCurDecColor,nCurDecColor);
	       else if(m_nInfoType==1)
		      m_nColor=RGB(220+nCurDecColor,nCurDecColor,nCurDecColor);
	       else
		      m_nColor=RGB(220+nCurDecColor,220+nCurDecColor,0);
		   pDC->FillSolidRect(&rectItem,m_nColor);
	   }
	}
	else
		pDC->FillSolidRect(&m_rectClient,_BACK_COLOR);
	CString szTxt=m_szText;
	if(szTxt.IsEmpty())
	   GetWindowText(szTxt);
	if(!szTxt.IsEmpty())
	{
		CFont* pfont=pDC->SelectObject(GetParent()->GetFont());
	    pDC->SetTextColor(RGB(0,0,0));
	    pDC->SetBkMode(TRANSPARENT);
	    pDC->DrawText(szTxt,&m_rectClient,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		pDC->SelectObject(pfont);
	}
	if(bGetDC)
		ReleaseDC(pDC);
}
void CStaticBox::EnableFlashMode(BOOL bEnable)
{
	if(bEnable)
	{
		if(!m_bFlashMode)
			SetTimer(10,800,NULL);
	}
	else if(m_bFlashMode)
	{
		KillTimer(10);
	}
	m_bFlashMode=bEnable;
}
void CStaticBox::SetInfo(UINT nType, UINT nResID,BOOL bAnsy)
{
	m_nInfoType=nType;
	/*if(m_nInfoType==0)
		m_nColor=RGB(0,220,0);
	else if(m_nInfoType==1)
		m_nColor=RGB(220,0,0);
	else
		m_nColor=RGB(220,220,0);*/
	//if(nResID>0)
	 // LoadUIString(m_szText,nResID);
	if(bAnsy&&m_hWnd&&IsWindowVisible())
		DrawItem(NULL);
}
void CStaticBox::SetInfo(UINT nType, LPCTSTR pszTxt,BOOL bAnsy)
{
	m_nInfoType=nType;
	/*if(m_nInfoType==0)
		m_nColor=RGB(0,220,0);
	else if(m_nInfoType==1)
		m_nColor=RGB(220,0,0);
	else
		m_nColor=RGB(220,220,0);*/
	m_szText=pszTxt;
	if(bAnsy&&m_hWnd&&IsWindowVisible())
		DrawItem(NULL);
}
void CStaticBox::OnDestroy() 
{
	if(m_bFlashMode)
		KillTimer(10);
	m_bFlashMode=FALSE;
	CStatic::OnDestroy();
	// TODO: Add your message handler code here
}
void CStaticBox::OnTimer(UINT_PTR nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if((nIDEvent==10)&&(m_nInfoType>0))
	{
		m_bFlashState=!m_bFlashState;
		if(IsWindowVisible())
		   DrawItem(NULL);
	}
	CStatic::OnTimer(nIDEvent);
}
