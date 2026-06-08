// CSysParamDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CSysParamDlg.h"
#include "afxdialogex.h"


// CSysParamDlg 对话框
extern RYUSR_PARAM g_sysParam;
IMPLEMENT_DYNAMIC(CSysParamDlg, CDialogEx)

CSysParamDlg::CSysParamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SYS_PARAM, pParent)
	, m_fCBDistance(0)
	, m_fCBWidth(0)
	, m_fCBInterval(0)
	, m_fCBInkDensity(0)
	, m_nPrtPeriod(0)
	, m_nCBValidMask(0)
	, m_nCBPosMode(0)
	, m_bEquiDis(FALSE)
	, m_nMicroJetUnit(0)
	, m_nMicroJetCount(0)
	, m_nMicroStpJet(0)
	, m_bEncReverse(FALSE)
	, m_bIdleFlash(FALSE)
	,m_BigInkDrops(FALSE)
	, m_szLogPath(_T(""))
	, m_nColorSel(0)
	, m_nOverlapProc(0)
	, m_fFlashPrtFrequecy(0)
	, m_fBrustValidSec(0)
	, m_fBrustCycleSec(0)
	, m_bAll(FALSE)
	, m_bUseAdib(FALSE)
{

}

CSysParamDlg::~CSysParamDlg()
{
}

void CSysParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CB_DISTANCE, m_fCBDistance);
	DDX_Text(pDX, IDC_EDIT_CB_WIDTH, m_fCBWidth);
	DDX_Text(pDX, IDC_EDIT_CB_INTERVAL, m_fCBInterval);
	DDX_Text(pDX, IDC_EDIT_CB_INK_DENISTY, m_fCBInkDensity);
	DDX_Text(pDX, IDC_EDIT_CB_PERIOD, m_nPrtPeriod);
	DDX_Text(pDX, IDC_EDIT_CB_COLORMASK, m_nCBValidMask);
	DDX_CBIndex(pDX, IDC_COMBO_CB_MODE, m_nCBPosMode);
	DDX_Check(pDX, IDC_CHECK_CB_OPTION, m_bEquiDis);
	DDX_Text(pDX, IDC_EDIT_MICROJET_UNIT, m_nMicroJetUnit);
	DDX_Text(pDX, IDC_EDIT_MICROJET_COUNT, m_nMicroJetCount);
	DDX_Text(pDX, IDC_EDIT_MICRO_STPJET, m_nMicroStpJet);
	DDX_Check(pDX, IDC_CHECK_AB_REVERSE, m_bEncReverse);
	DDX_Check(pDX, IDC_CHECK_IDLE_FLASH, m_bIdleFlash);
	DDX_Check(pDX, IDC_CHECK_IDLE_FLASH2, m_BigInkDrops);
	DDX_Text(pDX, IDC_LOG_PATH, m_szLogPath);
	DDX_CBIndex(pDX, IDC_COMBO_COLOR_SEL, m_nColorSel);
	DDX_CBIndex(pDX, IDC_COMBO_OVERLAPPROC, m_nOverlapProc);
	DDX_Text(pDX, IDC_EDIT_FLASH_FREQ, m_fFlashPrtFrequecy);
	DDX_Text(pDX, IDC_EDIT_FLASH_SEC, m_fBrustValidSec);
	DDX_Text(pDX, IDC_EDIT_FLASH_CYCLE, m_fBrustCycleSec);
	DDX_Control(pDX, IDC_STATIC_FLASH, m_FlashList);
	DDX_Check(pDX, IDC_CHECK_All, m_bAll);
	DDX_Check(pDX, IDC_CHECK_ADIBCTL, m_bUseAdib);
}


BEGIN_MESSAGE_MAP(CSysParamDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSysParamDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SET, &CSysParamDlg::OnBnClickedButtonSet)
END_MESSAGE_MAP()


// CSysParamDlg 消息处理程序


BOOL CSysParamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString sz;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COLOR_SEL);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int c = 0; c < MAX_COLORS; c++) {
			sz.Format(_T("C-%d"), c);
			pComboBox->AddString(sz);
		}
		pComboBox->SetCurSel(0);
	}
	LoadOption();
	InitFlashList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CSysParamDlg::LoadOption() {
	m_fCBDistance = g_sysParam.clrbar_param.fCBDistance;
	m_fCBWidth = g_sysParam.clrbar_param.fCBWidth;
	m_fCBInterval = g_sysParam.clrbar_param.fCBInterval;
	m_fCBInkDensity = g_sysParam.clrbar_param.fCBInkDenisty;
	m_nPrtPeriod = g_sysParam.clrbar_param.nPrtPeriod;
	m_nCBValidMask = g_sysParam.clrbar_param.nCBValidMask;
	m_nCBPosMode = g_sysParam.clrbar_param.nCBPosMode;
	m_bEquiDis = (g_sysParam.clrbar_param.nCBOption & 0x1) ? TRUE : FALSE;
	m_nMicroJetCount = g_sysParam.nLayerJetMcUint;
	m_nMicroJetUnit = g_sysParam.nLayerMaxRevJets;
	m_nMicroStpJet = g_sysParam.nMicroStpJet;
	m_szLogPath.Format(_T("%s"), g_sysParam.szLogPath);
	m_bIdleFlash = (g_sysParam.nSysFunOption & 0x1)?TRUE:FALSE;
	m_bEncReverse = (g_sysParam.nSysFunOption & 0x2)?TRUE:FALSE;
	m_bUseAdib = (g_sysParam.nSysFunOption & 0x4) ? TRUE : FALSE;
	m_BigInkDrops = (g_sysParam.nSysFunOption & 0x10) ? TRUE : FALSE;
	m_fBrustCycleSec = g_sysParam.fBrustCycleSec[0];
	m_fBrustValidSec = g_sysParam.fBrustValidSec[0];
	m_fFlashPrtFrequecy = g_sysParam.fFlashPrtFrequecy[0];
	UpdateData(FALSE);
}


void CSysParamDlg::InitFlashList() {
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hColorItem;
	m_FlashList.GetClientRect(&rect);
	int nUnitWidth = rect.Width() / 6;
	CString szText;
	m_FlashList.InsertColumn(0, _T("颜色"), LVCFMT_CENTER, nUnitWidth * 2);
	m_FlashList.InsertColumn(1, _T("闪喷频率(HZ)"), LVCFMT_CENTER, nUnitWidth);
	m_FlashList.InsertColumn(2, _T("闪喷周期(S)"), LVCFMT_CENTER, nUnitWidth);
	m_FlashList.InsertColumn(3, _T("闪喷时间(S)"), LVCFMT_CENTER, nUnitWidth);
	m_FlashList.InsertColumn(4, _T("重叠嘴处理方式"), LVCFMT_CENTER, nUnitWidth);
	hRoot = m_FlashList.m_childTree.InsertItem(_T("颜色ID"), 0, 0);
	for (int c = 0; c < MAX_COLORS; c++) {
		szTxt.Format(_T("C-%d"), c + 1);
		hColorItem = m_FlashList.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
		szText.Format(_T("%.1f"), g_sysParam.fFlashPrtFrequecy[c]);
		m_FlashList.SetItemText(hColorItem, 1, szText);
		szText.Format(_T("%.1f"), g_sysParam.fBrustCycleSec[c]);
		m_FlashList.SetItemText(hColorItem, 2, szText);
		szText.Format(_T("%.1f"), g_sysParam.fBrustValidSec[c]);
		m_FlashList.SetItemText(hColorItem, 3, szText);
		szText.Format(_T("%d"), g_sysParam.nOverlapJetProcType[c]);
		m_FlashList.SetItemText(hColorItem, 4, szText);
	}
	m_FlashList.m_childTree.Expand(hRoot, TVE_EXPAND);
	
}

void CSysParamDlg::ReDrawFlashList() {
	HTREEITEM hRootItem = m_FlashList.GetRootItem();
	HTREEITEM hColorItem = m_FlashList.GetChildItem(hRootItem);
	CString szText;
	for (int c = 0; c < MAX_COLORS; c++) {
		szText.Format(_T("%.1f"), g_sysParam.fFlashPrtFrequecy[c]);
		m_FlashList.SetItemText(hColorItem, 1, szText);
		szText.Format(_T("%.1f"), g_sysParam.fBrustCycleSec[c]);
		m_FlashList.SetItemText(hColorItem, 2, szText);
		szText.Format(_T("%.1f"), g_sysParam.fBrustValidSec[c]);
		m_FlashList.SetItemText(hColorItem, 3, szText);
		szText.Format(_T("%d"), g_sysParam.nOverlapJetProcType[c]);
		m_FlashList.SetItemText(hColorItem, 4, szText);
		hColorItem = m_FlashList.GetNextSiblingItem(hColorItem);
	}
}

void CSysParamDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	g_sysParam.clrbar_param.fCBDistance = m_fCBDistance;
	g_sysParam.clrbar_param.fCBWidth = m_fCBWidth;
	g_sysParam.clrbar_param.fCBInterval = m_fCBInterval;
	g_sysParam.clrbar_param.fCBInkDenisty = m_fCBInkDensity;
	g_sysParam.clrbar_param.nPrtPeriod = m_nPrtPeriod;
	g_sysParam.clrbar_param.nCBValidMask = m_nCBValidMask;
	g_sysParam.clrbar_param.nCBPosMode = m_nCBPosMode;
	if (m_bEquiDis)
		g_sysParam.clrbar_param.nCBOption |= 0x1;
	else
		g_sysParam.clrbar_param.nCBOption &= ~0x1;
	g_sysParam.nLayerJetMcUint = m_nMicroJetCount;
	g_sysParam.nLayerMaxRevJets = m_nMicroJetUnit;
	g_sysParam.nMicroStpJet = m_nMicroStpJet;
	CopyMemory(g_sysParam.szLogPath,m_szLogPath.GetBuffer(0),260);
	if (m_bIdleFlash)
		g_sysParam.nSysFunOption |= 0x1;
	else
		g_sysParam.nSysFunOption &= ~0x1;
	if (m_bEncReverse)
		g_sysParam.nSysFunOption |= 0x2;
	else
		g_sysParam.nSysFunOption &= ~0x2;
	if (m_bUseAdib)
		g_sysParam.nSysFunOption |= 0x4;
	else
		g_sysParam.nSysFunOption &= ~0x4;
	if(m_BigInkDrops)
	{
		g_sysParam.nSysFunOption |= 0x10;
	}
	else
	{
		g_sysParam.nSysFunOption &= ~0x10;
	}

	CDialogEx::OnOK();
}


void CSysParamDlg::OnBnClickedButtonSet()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_bAll) {
		for (int c = 0; c < MAX_COLORS; c++) {
			g_sysParam.fBrustCycleSec[c] = m_fBrustCycleSec;
			g_sysParam.fBrustValidSec[c] = m_fBrustValidSec;
			g_sysParam.fFlashPrtFrequecy[c] = m_fFlashPrtFrequecy;
			g_sysParam.nOverlapJetProcType[c] = m_nOverlapProc;
		}
	}
	else {
		g_sysParam.fBrustCycleSec[m_nColorSel] = m_fBrustCycleSec;
		g_sysParam.fBrustValidSec[m_nColorSel] = m_fBrustValidSec;
		g_sysParam.fFlashPrtFrequecy[m_nColorSel] = m_fFlashPrtFrequecy;
		g_sysParam.nOverlapJetProcType[m_nColorSel] = m_nOverlapProc;
	}
	ReDrawFlashList();
}
