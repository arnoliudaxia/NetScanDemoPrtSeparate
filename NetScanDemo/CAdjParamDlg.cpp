// CAdjParamDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CAdjParamDlg.h"
#include "afxdialogex.h"
#include "../inc/RYPrtCtler.h"
// CAdjParamDlg 对话框
extern RYUSR_PARAM g_sysParam;
extern RYCalbrationParam g_Calbration;
extern LPPRINTER_INFO g_pSysInfo;
IMPLEMENT_DYNAMIC(CAdjParamDlg, CDialogEx)

UINT PrtStatueType = 0;

CAdjParamDlg::CAdjParamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_ADJ, pParent)
	, m_nXGoffset(0)
	, m_nClrIndex(0)
	, m_nGGroupIndex(0)
	, m_nGDir(0)
	, m_nGIClrIndex(0)
	, m_nGIGroupIndex(0)
	, m_nGIGDir(0)
	, m_nGIJetIndex(0)
	, m_nGIXoffset(0)
	, m_nBiDirEncPrtOff(0)
{
	ZeroMemory(&m_calParam, sizeof(RYCalbrationParam));
	m_calParam.nGrayBits = 1;
	m_bPrinting = false;
	m_bStop = false;
}

CAdjParamDlg::~CAdjParamDlg()
{
}

void CAdjParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_XGOFFSET, m_nXGoffset);
	DDX_CBIndex(pDX, IDC_COMBO_GCOLOR, m_nClrIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GGROUP, m_nGGroupIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GDIR, m_nGDir);
	DDX_CBIndex(pDX, IDC_COMBO_GICOLOR, m_nGIClrIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GIGROUP, m_nGIGroupIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GIDIR, m_nGIGDir);
	DDX_CBIndex(pDX, IDC_COMBO_GIJET, m_nGIJetIndex);
	DDX_Text(pDX, IDC_EDIT_XIGOFFSET, m_nGIXoffset);
	DDX_Control(pDX, IDC_STATIC_GOFFLIST, m_GoupList);
	DDX_Control(pDX, IDC_STATIC_GOFFLIST2, m_GIList);
	DDX_Text(pDX, IDC_EDIT_BIOFF, m_nBiDirEncPrtOff);
	DDX_CBIndex(pDX, IDC_COMBO_PRTDIR, m_calParam.nPrtDir);
	DDX_Text(pDX, IDC_EDIT_XADJ_DPI, m_calParam.fxadjdpi);
	//DDX_Text(pDX, IDC_EDIT_STEP_SIZE, m_calParam.fStpSize);
	DDX_Text(pDX, IDC_EDIT_RUNSPD, m_calParam.fXRunSpd);
	DDX_Text(pDX, IDC_EDIT_MAX_HEIGHT, m_calParam.fYMaxPrtHeight);
	DDX_Text(pDX, IDC_EDIT_MAX_WIDTH, m_calParam.fXMaxPrtWidth);
	DDX_Text(pDX, IDC_EDIT_CTL, m_calParam.fPrtXPos);
	DDX_Text(pDX, IDC_EDIT_GRAYBITS, m_calParam.nGrayBits);
}


BEGIN_MESSAGE_MAP(CAdjParamDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GROUP_OFFSET, &CAdjParamDlg::OnBnClickedButtonGroupOffset)
	ON_BN_CLICKED(IDC_BUTTON_GIGROUP_OFFSET, &CAdjParamDlg::OnBnClickedButtonGigroupOffset)
	ON_CBN_SELCHANGE(IDC_COMBO_GIDIR, &CAdjParamDlg::OnCbnSelchangeComboGidir)
	ON_CBN_SELCHANGE(IDC_COMBO_GDIR, &CAdjParamDlg::OnCbnSelchangeComboGdir)
	ON_BN_CLICKED(IDOK, &CAdjParamDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_LACK_JET, &CAdjParamDlg::OnBnClickedButtonLackJet)
	ON_BN_CLICKED(IDC_BUTTON_PHSTATUS, &CAdjParamDlg::OnBnClickedButtonPhstatus)
	ON_BN_CLICKED(IDC_BUTTON_VTICAL, &CAdjParamDlg::OnBnClickedButtonVtical)
	ON_BN_CLICKED(IDC_BUTTON_STEP, &CAdjParamDlg::OnBnClickedButtonStep)
	ON_BN_CLICKED(IDC_BUTTON_XOFF_GROUP, &CAdjParamDlg::OnBnClickedButtonXoffGroup)
	ON_BN_CLICKED(IDC_BUTTON_XOFF, &CAdjParamDlg::OnBnClickedButtonXoff)
	ON_BN_CLICKED(IDC_BUTTON6, &CAdjParamDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CAdjParamDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CAdjParamDlg 消息处理程序
void CAdjParamDlg::InitGroupList()
{
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hColorItem;
	m_GoupList.GetClientRect(&rect);
	int nUnitWidth = rect.Width() / MAX_COLORS;
	CString szText;
	m_GoupList.InsertColumn(0, _T("颜色/组"), LVCFMT_CENTER, nUnitWidth * 2);
	for (int c = 0; c < MAX_COLORS; c++) {
		szText.Format(_T("C-%d"), c+1);
		m_GoupList.InsertColumn(c + 1, szText, LVCFMT_CENTER, nUnitWidth);
	}
	hRoot = m_GoupList.m_childTree.InsertItem(_T("分组"), 0, 0);
	for (int g = 0; g < MAX_GROUP; g++) {
		szTxt.Format(_T("G-%d"), g + 1);
		hColorItem = m_GoupList.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
		for (int c = 0; c < MAX_COLORS; c++) {
			szText.Format(_T("%d"), (int)(g_sysParam.nPhgXGroupOff[c][g][0]*g_Calbration.fxadjdpi)/(g_pSysInfo->nXSysEncDPI));
			m_GoupList.SetItemText(hColorItem, c + 1, szText);
		}
		m_GoupList.m_childTree.Expand(hColorItem, TVE_EXPAND);
	}
	m_GoupList.m_childTree.Expand(hRoot, TVE_EXPAND);
}
void CAdjParamDlg::InitGIGroupList()
{
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hGroupItem,hJetItem;
	m_GIList.GetClientRect(&rect);
	int nUnitWidth = rect.Width() / MAX_COLORS;
	CString szText;
	m_GIList.InsertColumn(0, _T("颜色/组"), LVCFMT_CENTER, nUnitWidth * 2);
	for (int c = 0; c < MAX_COLORS; c++) {
		szText.Format(_T("C-%d"), c + 1);
		m_GIList.InsertColumn(c + 1, szText, LVCFMT_CENTER, nUnitWidth);
	}
	hRoot = m_GIList.m_childTree.InsertItem(_T("分组"), 0, 0);
	for (int g = 0; g < MAX_GROUP; g++) {
		szTxt.Format(_T("G-%d"), g + 1);
		hGroupItem = m_GIList.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
		for (int j = 0; j < 8; j++) {
			szTxt.Format(_T("Jet-%d"), j + 1);
			hJetItem = m_GIList.m_childTree.InsertItem(szTxt, 1, 1, hGroupItem, TVI_LAST);
			for (int c = 0; c < MAX_COLORS; c++) {
				szText.Format(_T("%d"), (int)(g_sysParam.nPhgJetRowOff[c][g][j][0]*g_Calbration.fxadjdpi)/(g_pSysInfo->nXSysEncDPI));
				m_GIList.SetItemText(hJetItem, c + 1, szText);
			}
			m_GoupList.m_childTree.Expand(hJetItem, TVE_EXPAND);
		}
		m_GoupList.m_childTree.Expand(hGroupItem, TVE_EXPAND);
	}
	m_GoupList.m_childTree.Expand(hRoot, TVE_EXPAND);
}
void CAdjParamDlg::UpdateGroupList()
{
	HTREEITEM hRootItem = m_GoupList.GetRootItem();
	HTREEITEM hGroupItem = m_GoupList.GetChildItem(hRootItem);
	CString szText;
	for (int g = 0; g < MAX_GROUP; g++) {
		for (int c = 0; c < MAX_COLORS; c++) {
			szText.Format(_T("%d"), g_sysParam.nPhgXGroupOff[c][g][m_nGDir]);
			m_GoupList.SetItemText(hGroupItem, c + 1, szText);
		}
		hGroupItem = m_GoupList.GetNextSiblingItem(hGroupItem);
	}
}

void CAdjParamDlg::UpdateGIGroupList()
{
	HTREEITEM hRootItem = m_GIList.GetRootItem();
	HTREEITEM hGroupItem = m_GIList.GetChildItem(hRootItem);
	HTREEITEM hJetItem;
	CString szText;
	for (int g = 0; g < MAX_GROUP; g++) {
		hJetItem = m_GIList.GetChildItem(hGroupItem);
		for (int j = 0; j < 8; j++) {
			for (int c = 0; c < MAX_COLORS; c++) {
				szText.Format(_T("%d"), g_sysParam.nPhgJetRowOff[c][g][j][m_nGIGDir]);
				m_GIList.SetItemText(hJetItem, c + 1, szText);
			}
			hJetItem = m_GIList.GetNextSiblingItem(hJetItem);
		}
		hGroupItem = m_GoupList.GetNextSiblingItem(hGroupItem);
	}
}

BOOL CAdjParamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString szText;
	// TODO:  在此添加额外的初始化
	InitGroupList();
	InitGIGroupList();
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GCOLOR);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int c = 0; c < MAX_COLORS; c++) {
			szText.Format(_T("C-%d"), c+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GICOLOR);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int c = 0; c < MAX_COLORS; c++) {
			szText.Format(_T("C-%d"), c+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GGROUP);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int g = 0; g < MAX_GROUP; g++) {
			szText.Format(_T("G-%d"), g+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GIGROUP);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int g = 0; g < MAX_GROUP; g++) {
			szText.Format(_T("G-%d"), g+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GDIR);
	if (pComboBox) {
		pComboBox->ResetContent();
		szText.Format(_T("负方向"));
		pComboBox->AddString(szText);
		szText.Format(_T("正方向"));
		pComboBox->AddString(szText);
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GIDIR);
	if (pComboBox) {
		pComboBox->ResetContent();
		szText.Format(_T("负方向"));
		pComboBox->AddString(szText);
		szText.Format(_T("正方向"));
		pComboBox->AddString(szText);
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GIJET);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int j = 0; j < MAX_JETROW; j++) {
			szText.Format(_T("Jet-%d"), j+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	m_nBiDirEncPrtOff = (int)((g_sysParam.nBiDirEncPrtOff*g_Calbration.fxadjdpi)/(g_pSysInfo->nXSysEncDPI));
	CopyMemory(&m_calParam, &g_Calbration, sizeof(RYCalbrationParam));
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

extern LPPRINTER_INFO g_pSysInfo;
void CAdjParamDlg::OnBnClickedButtonGroupOffset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int XGoffset = (int)g_pSysInfo->nXSysEncDPI/g_Calbration.fxadjdpi * m_nXGoffset;
	g_sysParam.nPhgXGroupOff[m_nClrIndex][m_nGGroupIndex][m_nGDir] = XGoffset;
	UpdateGroupList();


}


void CAdjParamDlg::OnBnClickedButtonGigroupOffset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int GIXoffset = g_pSysInfo->nXSysEncDPI/g_Calbration.fxadjdpi * m_nGIXoffset;
	g_sysParam.nPhgJetRowOff[m_nGIClrIndex][m_nGIGroupIndex][m_nGIJetIndex][m_nGIGDir] = GIXoffset;
	UpdateGIGroupList();
}


void CAdjParamDlg::OnCbnSelchangeComboGidir()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	UpdateGIGroupList();
}


void CAdjParamDlg::OnCbnSelchangeComboGdir()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	UpdateGroupList();
}


void CAdjParamDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int  BiDirEncPrtOff =  g_pSysInfo->nXSysEncDPI/g_Calbration.fxadjdpi * m_nBiDirEncPrtOff;
	g_sysParam.nBiDirEncPrtOff = BiDirEncPrtOff;

	CDialogEx::OnOK();
}

#include "CLackJetDlg.h"
void CAdjParamDlg::OnBnClickedButtonLackJet()
{
	// TODO: 在此添加控件通知处理程序代码
	CLackJetDlg dlg;
	dlg.DoModal();
}


void BmtInsStr(BYTE* pBuf,int xDot,int yDot,int xp,int yp,LPCTSTR lpText,int nFontWidth,float frate,BOOL bWhite,BOOL bAdaptive,int nAngle)
{

	
	//int lineByte = (xDot*1 / 8 + 3) / 4 * 4;
	//pBmpBuf = new unsigned char[lineByte*height];

	//memset(pBuf,0x0,((xDot*1 / 8 + 3) / 4 * 4));
	int nBytesPerline=(xDot+31)/32*4;
	int nWidth=nBytesPerline*8;
	int nBufLen=nBytesPerline*yDot;
	//LPBYTE pData=new BYTE[nBufLen];
	HBITMAP hBitmap=CreateBitmap(nWidth,yDot,1,1,pBuf);

	HDC hdc,hMemDC;
	HWND m_hWnd; 
	hdc =GetDC(NULL);

	hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldbmp=(HBITMAP)SelectObject(hMemDC,hBitmap);
	SetMapMode(hMemDC,MM_TEXT);
	HFONT hOldFont;
	HFONT hNewFont=CreateFont(nFontWidth,                        // nHeight
		frate,                         // nWidth
		nAngle,//-1800,                         // nEscapement  //-1800
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		TEXT("Arial"));//"MS Reference Sans Serif");//"Dotum");//"Arial");
	hOldFont =(HFONT)SelectObject(hMemDC,hNewFont);
	SetBkMode(hMemDC,TRANSPARENT);
	if(bWhite)
		SetTextColor(hMemDC,RGB(0,0,0));
	else
		SetTextColor(hMemDC,RGB(255,255,255));

	//	if(bAdaptive)	
	//		MemDC.BitBlt(0,0,880,440,&MemDC,860,0,PATINVERT);	
	TextOut(hMemDC,xp,yp,lpText,lstrlen(lpText));
	SelectObject(hMemDC,hOldFont);
	LPBYTE pTarget=0;
	hOldbmp=(HBITMAP)SelectObject(hMemDC,hOldbmp);
	GetBitmapBits(hOldbmp,nBufLen,pBuf);

	DeleteObject(hOldbmp);
	DeleteDC(hMemDC);
}
bool DifferenceStatus = true;
int tetettt(unsigned char* pPhgData,int c,int gg,int nWidth,int Height,int nByteLines,int nGrayBits)
{
	//return 0;
	if(PrtStatueType == 0)
	{
		unsigned char* pBmpBuf1;
		char szTxt[512];
		SYSTEMTIME curtime;
		pBmpBuf1 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf1,0x0,nByteLines*Height);

		GetLocalTime(&curtime);
			//sprintf(szTxt," 成立时间于 %d-%02d-%02d  %02d:%02d:%02d ",curtime.wYear,curtime.wMonth,curtime.wDay,curtime.wHour,curtime.wMinute,curtime.wSecond);
		sprintf(szTxt,"RY 3DP status %d-%02d-%02d  %02d:%02d:%02d ",curtime.wYear,curtime.wMonth,curtime.wDay,curtime.wHour,curtime.wMinute,curtime.wSecond);
		BmtInsStr(pBmpBuf1,nWidth,Height,(nWidth/2 - 350),Height,szTxt,110,25,0,0,900);
		//BmtInsStr(pBmpBuf1,nWidth,Height,(nWidth/2 - 350),Height,szTxt,110,25,0,0,900);


		unsigned char* pBmpBuf2;
		pBmpBuf2 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf2,0x0,nByteLines*Height);

		for (int i = 0; i < Height; ++i){
			for (int j = 0; j < nByteLines; ++j){
				unsigned char *p1, *p2;
				p1 = (unsigned char *)(pBmpBuf1 + (Height - 1 - i)*nByteLines + j);
				p2 = (unsigned char *)(pBmpBuf2 + i*nByteLines + j);
				(*p2) = (*p1);
			}
		}

		for(int i=0;i<Height;i++)
		{
			for(int j=0;j<nByteLines;j++)
			{
				if(pPhgData[i*nByteLines+j] ==0)
				{
					pPhgData[i*nByteLines+j] = pBmpBuf2[i*nByteLines+j];
				}
			}
		}

		delete pBmpBuf2;
		delete pBmpBuf1;
		return 0;
	}

	if(PrtStatueType == 1)
	{
		unsigned char* pBmpBuf1;
		char szTxt[256];
		SYSTEMTIME curtime;
		pBmpBuf1 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf1,0x0,nByteLines*Height);

		GetLocalTime(&curtime);
		//	sprintf(szTxt,"恒建有限集团 成立时间于 %d-%02d-%02d  %02d:%02d:%02d ",curtime.wYear,curtime.wMonth,curtime.wDay,curtime.wHour,curtime.wMinute,curtime.wSecond);
		sprintf(szTxt,"垂直校准");
		BmtInsStr(pBmpBuf1,nWidth,Height,(nWidth-150),Height,szTxt,110,25,0,0,900);


		unsigned char* pBmpBuf2;
		pBmpBuf2 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf2,0x0,nByteLines*Height);

		for (int i = 0; i < Height; ++i){
			for (int j = 0; j < nByteLines; ++j){
				unsigned char *p1, *p2;
				p1 = (unsigned char *)(pBmpBuf1 + (Height - 1 - i)*nByteLines + j);
				p2 = (unsigned char *)(pBmpBuf2 + i*nByteLines + j);
				(*p2) = (*p1);
			}
		}

		for(int i=0;i<Height;i++)
		{
			for(int j=0;j<nByteLines;j++)
			{
				if(pPhgData[i*nByteLines+j] ==0)
				{
					pPhgData[i*nByteLines+j] = pBmpBuf2[i*nByteLines+j];
				}
			}
		}

			delete pBmpBuf2;
		delete pBmpBuf1;
		return 0;
	}
	if(PrtStatueType == 5 && DifferenceStatus)
	{
		DifferenceStatus = false;
		unsigned char* pBmpBuf1;
		char szTxt[256];
		SYSTEMTIME curtime;
		pBmpBuf1 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf1,0x0,nByteLines*Height);

		GetLocalTime(&curtime);
		//	sprintf(szTxt,"恒建有限集团 成立时间于 %d-%02d-%02d  %02d:%02d:%02d ",curtime.wYear,curtime.wMonth,curtime.wDay,curtime.wHour,curtime.wMinute,curtime.wSecond);
		sprintf(szTxt,"往发差校准");
			BmtInsStr(pBmpBuf1,nWidth,Height,0,Height,szTxt,110,25,0,0,900);


		unsigned char* pBmpBuf2;
		pBmpBuf2 = new unsigned char[nByteLines*Height];
		memset(pBmpBuf2,0x0,nByteLines*Height);

		for (int i = 0; i < Height; ++i){
			for (int j = 0; j < nByteLines; ++j){
				unsigned char *p1, *p2;
				p1 = (unsigned char *)(pBmpBuf1 + (Height - 1 - i)*nByteLines + j);
				p2 = (unsigned char *)(pBmpBuf2 + i*nByteLines + j);
				(*p2) = (*p1);
			}
		}

		for(int i=0;i<Height;i++)
		{
			for(int j=0;j<nByteLines;j++)
			{
				//pPhgData[i*nByteLines+j] = 0;
				if(pPhgData[i*nByteLines+j] ==0)
				{
					pPhgData[i*nByteLines+j] = pBmpBuf2[i*nByteLines+j];
				}
			}
		}

		delete pBmpBuf2;
		delete pBmpBuf1;
		return 0;
	}
	
}
void CAdjParamDlg::OnBnClickedButtonPhstatus()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();	
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	//IDP_SetCaliDataCallBack(tetettt);
	m_calParam.nAdjType = 0;
	PrtStatueType = 0;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 600;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(5*1000);
	IDP_FlashPrtCtl(FALSE);*/
	m_calParam.fStpSize = 0;
	//Sleep(60*1000);
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam); 
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}


void CAdjParamDlg::OnBnClickedButtonVtical()
{
	// TODO: 在此添加控件通知处理程序代码	
	UpdateData();
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	//IDP_SetCaliDataCallBack(tetettt);
	m_calParam.nAdjType = 1;
	PrtStatueType =1;
	m_calParam.fStpSize = 0;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 635;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(1*1000);
	IDP_FlashPrtCtl(FALSE);*/
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam); 
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}


void CAdjParamDlg::OnBnClickedButtonStep()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	m_calParam.nAdjType = 2;
	PrtStatueType = 2;
	m_calParam.fStpSize = 10;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 635;
	//m_calParam.fStpSize = 100;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(1*1000);
	IDP_FlashPrtCtl(FALSE);*/
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam);
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}


void CAdjParamDlg::OnBnClickedButtonXoffGroup()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	m_calParam.nAdjType = 3;
	PrtStatueType = 3;
	m_calParam.fStpSize = 0;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 635;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(1*1000);
	IDP_FlashPrtCtl(FALSE);*/
	
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam);
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}


void CAdjParamDlg::OnBnClickedButtonXoff()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();	
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	//IDP_SetCaliDataCallBack(tetettt);
	m_calParam.nAdjType = 4;
	PrtStatueType = 4;
	m_calParam.fStpSize = 0;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 635;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(1*1000);
	IDP_FlashPrtCtl(FALSE);*/
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam);
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}


void CAdjParamDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_bPrinting) {
		AfxMessageBox(_T("其他校准处于打印中..."));
		return;
	}
	DifferenceStatus = true;
	IDP_SetCaliDataCallBack(tetettt);
	///DifferenceStatus = true;
	m_calParam.nAdjType = 5;
	PrtStatueType = 5;
	m_calParam.fStpSize = 0;
	//m_calParam.fXMaxPrtWidth = 900;
	//m_calParam.fxadjdpi = 635;
	//m_calParam.nPrtDir = 1;
	//DEV_ResetPrinterEncValue(ZERO_POSITION);
	/*IDP_FlashPrtCtl(TRUE);
	Sleep(1*1000);
	IDP_FlashPrtCtl(FALSE);*/
	CopyMemory(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
	m_nPassCount = IDP_StartCalibration(&m_calParam);
	AfxBeginThread(PrtCaliBrationThread, this);
	//PrtCaliBrationThread();
}

UINT CAdjParamDlg::PrtCaliBrationThread(LPVOID pvoid) {
	CString szErr;
	int nPassCount;
	LPPassDataItem pPrtPassDes;
	CAdjParamDlg* pdlg = (CAdjParamDlg*)pvoid;
	PrtRunInfo	RTinfo;
	pdlg->m_bPrinting = true;
	pdlg->m_bStop = FALSE;
	
	for (int i = 0; i < pdlg->m_nPassCount; i++)	//
	{
		do
		{
			::OutputDebugString(_T("[RY] Start Query Pass!"));
			pPrtPassDes = ::IDP_GetPassItem(0, i);
			::OutputDebugString(_T("[RY] End Query Pass!"));
			if(pdlg->m_bStop)
			{
				break;
			}
			if (pPrtPassDes)
			{
				if ((pPrtPassDes->nProcState > 2) && (pPrtPassDes->nProcState < 6))
				{
					//3,4,5状态都可以执行打印
					break;
				}
			}
		} while (1);//等待PASS就绪
		if(pdlg->m_bStop)
		{
			break;
		}
		if (pPrtPassDes)
		{
			if (IDP_DoPassPrint(pPrtPassDes) <0)	//执行指定PASS打印，如果执行单图层一次执行，调用IDP_DoPassPrint2， 并且nPassID = -1 ,传入
			{
				AfxMessageBox(_T("打印失败"));
				//break;
			}

			char buf[100] = {0};
			sprintf(buf,"%s %d","开始打印 步进 = ",pPrtPassDes->nStpVector);
			AfxMessageBox(buf);



			do
			{
				IDP_GetPrintState(&RTinfo); //获取打印运行状态，用来判断当前打印的状态
				if (pdlg->m_bStop) //结束打印
					break;
				Sleep(1);
			} while (RTinfo.nPrtState == 1);

			
		}
		else
		{
			break;
		}
	}

	
	pdlg->m_bPrinting = false;
	IDP_FreeImageLayer(-1);
	IDP_StopPrintJob();
	
	g_Calbration.fStpSize = 1;
	AfxMessageBox(_T("打印结束"));
	AfxEndThread(0, TRUE);
	return 0;
}

void CAdjParamDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bPrinting)
		m_bStop = TRUE;
	//m_bJobStarted = false;
	IDP_StopPrintJob();
}
