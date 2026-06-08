// PicScreen.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "PicScreen.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//#include "extern\ImgDataSrc.h"
//extern CImgDataSrc *g_pGPImgSrc;

/////////////////////////////////////////////////////////////////////////////
// CPicScreen
CPicScreen::CPicScreen()
{
	m_hBmp=NULL;
	m_bLoading=FALSE;
	
	//m_tracker.m_nStyle ^= (CRectTracker::solidLine|CRectTracker::resizeOutside|CRectTracker::hatchInside);

	m_bYPrtDir=FALSE;
	m_bkcolor=GetSysColor(COLOR_3DFACE);
}

CPicScreen::~CPicScreen()
{
	if(m_hBmp)
	{
		DeleteObject(m_hBmp);
		m_hBmp=NULL;
	}
}


BEGIN_MESSAGE_MAP(CPicScreen, CStatic)
	//{{AFX_MSG_MAP(CPicScreen)
	//ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPicScreen message handlers

void CPicScreen::DrawImg(HBITMAP hBmp,CDC* pDC)
{
   BOOL bGenDC=FALSE;
   if(pDC==NULL)
   {
	   pDC=GetDC();
	   bGenDC=TRUE;
   }
   if(m_hBmp!=hBmp)
   {
	   DeleteObject(m_hBmp);
	   m_hBmp=NULL;
	   m_hBmp=hBmp;
   }
   RECT rectClient;
   GetClientRect(&rectClient);
   //m_bShowPercent=FALSE;
   pDC->FillSolidRect(&rectClient,m_bkcolor);//GetSysColor(COLOR_3DFACE));//RGB(220,220,220));
   if(m_bLoading)
   {
	   pDC->DrawText(_T("Loading..."),&rectClient,DT_CENTER |DT_VCENTER|DT_SINGLELINE);
	   return;
   }
   if(m_hBmp==NULL)
	   return;
   CDC dcMem;
   dcMem.CreateCompatibleDC(pDC);
   BITMAP bmpInfo;
   GetObject(m_hBmp,sizeof(BITMAP),&bmpInfo);
   HBITMAP  pOldBmp=(HBITMAP)SelectObject(dcMem.m_hDC,m_hBmp);
   float m_fWRate=((float)(rectClient.right-rectClient.left))/bmpInfo.bmWidth;
   float m_fHRate=((float)(rectClient.bottom-rectClient.top-1))/bmpInfo.bmHeight;
   float m_fSet=m_fWRate;
   if(m_fWRate>m_fHRate)
      m_fSet=m_fHRate;
   int nWidth=bmpInfo.bmWidth*m_fSet;
   int nHeight=bmpInfo.bmHeight*m_fSet;
   int nWB=(rectClient.right-rectClient.left-nWidth)/2;
   int nHB=(rectClient.bottom-rectClient.top-nHeight-1)/2;
   ::SetRect(&Picrect,nWB,nHB,nWB+nWidth,nHB+nHeight);
   pDC->SetStretchBltMode(HALFTONE);
   pDC->StretchBlt(nWB,nHB,nWidth,nHeight,&dcMem,0,0,bmpInfo.bmWidth,bmpInfo.bmHeight,SRCCOPY);//);
   dcMem.SelectObject(pOldBmp);
   //if(m_bShowRect)
   //   pDC->DrawEdge(&m_Rectprintvalid,EDGE_SUNKEN ,BDR_SUNKENOUTER|BF_RECT);
   pDC->Draw3dRect(&rectClient,PIC_BOARD_UL_COLOR,PCI_BOARD_DR_COLOR);
   if(bGenDC)
	   ReleaseDC(pDC);
}
void CPicScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	//RECT rectClient;
	CDC dcMem;
	CBitmap bitmap;
	int nWidth,nHeight;
	GetClientRect(&m_ClientRect);
	nWidth=m_ClientRect.right-m_ClientRect.left;
	nHeight=m_ClientRect.bottom-m_ClientRect.top;
    dcMem.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc,nWidth,nHeight);
	CBitmap* poldbmp=dcMem.SelectObject(&bitmap);
	DrawImg(m_hBmp,&dcMem);
	
	
	dcMem.Draw3dRect(&m_ClientRect,PIC_BOARD_UL_COLOR,PCI_BOARD_DR_COLOR);
	dc.BitBlt(0,0,nWidth,nHeight,&dcMem,0,0,SRCCOPY);
	
	dcMem.SelectObject(poldbmp);
	dcMem.DeleteDC();
	// TODO: Add your message handler code here
	
	// Do not call CStatic::OnPaint() for painting messages
}




BOOL CPicScreen::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL bRet=CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	return bRet;

}
void CPicScreen::ShowPreview(LPCTSTR lpszRipFileName)
{
	RECT rect;
	if(lpszRipFileName&&m_szRipFile.CompareNoCase(lpszRipFileName)==0)
		return;
	if(lpszRipFileName==NULL)
	{
		return;
	}
	m_bFailedLoad=FALSE;
	//m_pCtable=NULL;
	m_szRipFile=lpszRipFileName;
	m_szPreviewFile=m_szRipFile;
	//m_szPreviewFile.Replace(_T('.'),_T('-'));
	if (!m_szPreviewFile.Find(_T(".bmp")))
		m_szPreviewFile+=_T(".pvw");
	m_bLoading=FALSE;
	/*if(pheader==NULL)
	{
		CERipFile file;
		if(file.Open(lpszRipFileName,CFile::modeRead))
		{
            file.Read(&ripheader,sizeof(ripheader));
			file.Close();
		}
		else
		{
			m_bFailedLoad=TRUE;
			ZeroMemory(&ripheader,sizeof(ripheader));
		}
	}
	else
	    ripheader=*pheader;*/
	if(m_hBmp)
	{
		DeleteObject(m_hBmp);
		m_hBmp=NULL;
	}
	
	m_hBmp=(HBITMAP)::LoadImage(AfxGetApp()->m_hInstance,m_szPreviewFile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	int nLastError=::GetLastError();
    GetClientRect(&rect);
	//m_bShowRect=FALSE;	
	m_nLastPercentLinePos=-1;
	DrawImg(m_hBmp);
}
void CPicScreen::OnDestroy() 
{
	
	CStatic::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
BOOL CPicScreen::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	//if (pWnd == this &&m_tracker.SetCursor(this, nHitTest))
	//	return TRUE;
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CPicScreen::SetBkColor(COLORREF color)
{
	m_bkcolor=color;
}

		
