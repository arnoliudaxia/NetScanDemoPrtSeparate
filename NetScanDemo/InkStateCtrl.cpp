// InkStateCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "AdibCtrl.h"
#include "InkStateCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInkStateCtrl

CInkStateCtrl::CInkStateCtrl()
{
	m_nOffset=0;
	m_nInkCnts=17;
	m_nInkState=0;
	m_szInfo=_T("");
}
CInkStateCtrl::~CInkStateCtrl()
{
}
BEGIN_MESSAGE_MAP(CInkStateCtrl, CStatic)
	//{{AFX_MSG_MAP(CInkStateCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInkStateCtrl message handlers
const TCHAR _DES[][3][6]={_T(""),_T("EI-"),_T("EO-")};
void CInkStateCtrl::SetIOType(int nType)
{
	m_szInfo=_DES[0][nType];
}

void CInkStateCtrl::SetInkCnt(int nInkCnts,int nOffset)
{
	m_nOffset=nOffset;
	m_nInkCnts=nInkCnts;
	if(m_hWnd)
		Invalidate();
}

void CInkStateCtrl::SetInkState(UINT nInkState)
{
	m_nInkState=nInkState;
	if(m_hWnd)
		Invalidate();
}

void CInkStateCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	UINT nCtlValue;
	CString szTxt;
	RECT rectClient,rectItem;
	GetClientRect(&rectClient);
	int nxUnit=(rectClient.right-rectClient.left)/m_nInkCnts;
	int nyUnit=rectClient.bottom-rectClient.top;
	for(int i=m_nOffset;i<m_nInkCnts+m_nOffset;i++)
	{
		nCtlValue=(1<<(i-m_nOffset));
		SetRect(&rectItem,(i-m_nOffset)*nxUnit,0,(i+1-m_nOffset<m_nInkCnts)?(i+1-m_nOffset)*nxUnit:rectClient.right,nyUnit);
		dc.Draw3dRect(&rectItem,RGB(192,192,192),RGB(100,100,100));
		rectItem.left+=1;rectItem.top+=1;
		rectItem.right-=1;rectItem.bottom-=1;
		//if(i<8)
		//{
			dc.FillSolidRect(&rectItem,(m_nInkState&nCtlValue)?RGB(255,0,0):RGB(0,255,0));
		//}
		//else
		////{
		//	dc.FillSolidRect(&rectItem,(m_nInkState&nCtlValue)?RGB(255,0,0):RGB(0,255,0));
		//}
		
		szTxt.Format(_T("%s%d"),m_szInfo,i+1);
		dc.DrawText(szTxt,&rectItem,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	}
	// TODO: Add your message handler code here
	
	// Do not call CStatic::OnPaint() for painting messages
}
