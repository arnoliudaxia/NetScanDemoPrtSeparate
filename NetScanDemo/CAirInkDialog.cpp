// CAirInkDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CAirInkDialog.h"
#include "afxdialogex.h"
#include "../inc/RYPrtCtler.h"

// CAirInkDialog 对话框
extern RYUSR_PARAM g_sysParam;
extern LPPRINTER_INFO g_pSysInfo;

IMPLEMENT_DYNAMIC(CAirInkDialog, CDialog)

CAirInkDialog::CAirInkDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_AIR_INK, pParent)
	, m_nPushInkSec(2)
	, m_bEnableSupply(FALSE)
	, m_bEnablePress(FALSE)
	, m_bEnableAutoPress(FALSE)
	, m_bEnableOut1(FALSE)
{
	for (int i = 0; i < 2; i++) {
		m_fAirPressCtl[i] = 0;
		m_fAirPressSafeCtl[i] = 0;
		m_fInkTemp[i] = 0;
	}
	m_bStartPushInk = FALSE;
}

CAirInkDialog::~CAirInkDialog()
{
}

void CAirInkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_AIR_PRESS_CTL1, m_fAirPressCtl[0]);
	DDX_Text(pDX, IDC_EDIT_AIR_PRESS_CTL2, m_fAirPressCtl[1]);
	DDX_Text(pDX, IDC_EDIT_AIR_SAFE_CTL1, m_fAirPressSafeCtl[0]);
	DDX_Text(pDX, IDC_EDIT_AIR_SAFE_CTL2, m_fAirPressSafeCtl[1]);
	DDX_Text(pDX, IDC_EDIT_INK_TEMP1, m_fInkTemp[0]);
	DDX_Text(pDX, IDC_EDIT_INK_TEMP2, m_fInkTemp[1]);
	DDX_Text(pDX, IDC_EDIT_PUSH_INK, m_nPushInkSec);
	DDX_Check(pDX, IDC_CHECK_ENABLE_SUPPLY, m_bEnableSupply);
	DDX_Check(pDX, IDC_CHECK_ENABLE_PRESS, m_bEnablePress);
	DDX_Check(pDX, IDC_CHECK2, m_bEnableAutoPress);
	//DDX_Check(pDX, IDC_CHECK_OUT1, m_bEnableOut1);
	//DDX_Check(pDX, IDC_CHECK_OUT2, m_bEnableOut2);
	//DDX_Check(pDX, IDC_CHECK_OUT3, m_bEnableOut3);
	//DDX_Check(pDX, IDC_CHECK_OUT4, m_bEnableOut4);
	//DDX_Check(pDX, IDC_CHECK_OUT5, m_bEnableOut5);
	//DDX_Check(pDX, IDC_CHECK_OUT6, m_bEnableOut6);
}


BEGIN_MESSAGE_MAP(CAirInkDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SET, &CAirInkDialog::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_PUSH_INK, &CAirInkDialog::OnBnClickedButtonPushInk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SUPPLY, &CAirInkDialog::OnBnClickedCheckEnableSupply)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_PRESS, &CAirInkDialog::OnBnClickedCheckEnablePress)
	ON_BN_CLICKED(IDC_CHECK_OUT1, &CAirInkDialog::OnBnClickedCheckOut1)
	ON_BN_CLICKED(IDC_CHECK_OUT2, &CAirInkDialog::OnBnClickedCheckOut2)
	ON_BN_CLICKED(IDC_CHECK_OUT3, &CAirInkDialog::OnBnClickedCheckOut3)
	ON_BN_CLICKED(IDC_CHECK_OUT4, &CAirInkDialog::OnBnClickedCheckOut4)
	ON_BN_CLICKED(IDC_CHECK_OUT5, &CAirInkDialog::OnBnClickedCheckOut5)
	ON_BN_CLICKED(IDC_CHECK_OUT6, &CAirInkDialog::OnBnClickedCheckOut6)
	ON_BN_CLICKED(IDC_CHECK2, &CAirInkDialog::OnBnClickedCheck2)
END_MESSAGE_MAP()


// CAirInkDialog 消息处理程序

const UINT _inkCtl[2] = { IDC_CHECK_OUT1 ,IDC_CHECK_OUT2 };
const UINT _pressCtl[6] = { IDC_CHECK_OUT3,IDC_CHECK_OUT4,IDC_CHECK_OUT5,IDC_CHECK_OUT6, IDC_CHECK_OUT1 ,IDC_CHECK_OUT2};
const UINT _singleOutCtl[6] = {IDC_CHECK_OUT1,IDC_CHECK_OUT2,IDC_CHECK_OUT3,IDC_CHECK_OUT4,IDC_CHECK_OUT5,IDC_CHECK_OUT6};
BOOL CAirInkDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	LPPRINTER_INFO pSysInfo = NULL;
	pSysInfo = DEV_GetDeviceInfo();
	// TODO:  在此添加额外的初始化
	for (int i = 0; i < 2; i++) {
		m_fAirPressCtl[i] = g_sysParam.fAirCtlPress[0][i];
		m_fAirPressSafeCtl[i] = g_sysParam.fSafeAirPress[0][i];
		m_fInkTemp[i] = g_sysParam.fInkTemp[0][i];
	}
	m_InkState[0].SubclassDlgItem(IDC_STATIC_INKLV1, this);
	m_InkState[1].SubclassDlgItem(IDC_STATIC_INKLV2, this);
	m_InkState[3].SubclassDlgItem(IDC_STATIC_INKLV3, this);
	m_InkState[4].SubclassDlgItem(IDC_STATIC_INKLV4, this);
	m_InkState[2].SubclassDlgItem(IDC_STATIC_INKLVP, this);
	SetTimer(0x101, 200, NULL);
	m_bEnableSupply = (g_sysParam.nIoOption & 0x1) ? TRUE : FALSE;
	m_bEnablePress = (g_sysParam.nIoOption & 0x2) ? TRUE : FALSE;
	m_bEnableAutoPress = (g_sysParam.nIoOption & 0x4) ? TRUE : FALSE;
	CWnd* pWnd;
	for (int i = 0; i < 2; i++) {
		pWnd = GetDlgItem(_inkCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(m_bEnableSupply);
	}
	for (int i = 0; i < 4; i++) {
		pWnd = GetDlgItem(_pressCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(!m_bEnablePress);
	}

	for (int i = 0; i < 6; i++) {
		pWnd = GetDlgItem(_pressCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(!m_bEnableAutoPress);
	}

	if(pSysInfo)
	{
		for (int i = 0; i < 6; i++) {
			pWnd = GetDlgItem(_singleOutCtl[i]);
			if (pWnd)
			{
				if(i == 0 && (pSysInfo->nIoState[0] & 0x1))
				{
					m_bEnableOut1 = TRUE;
					pWnd->SetWindowText("关闭J58");
				}

				if(i == 0 && !(pSysInfo->nIoState[0]& 0x1))
				{
					m_bEnableOut1 = FALSE;
					pWnd->SetWindowText("打开J58");
				}

				if(i == 1 && (pSysInfo->nIoState[0] & 0x2))
				{
					m_bEnableOut2 = TRUE;
					pWnd->SetWindowText("关闭J59");
				}
				if(i == 1 && !(pSysInfo->nIoState[0]& 0x2))
				{
					m_bEnableOut2 = FALSE;
					pWnd->SetWindowText("打开J59");
				}

				if(i == 2 && (pSysInfo->nIoState[0] & 0x4))
				{
					m_bEnableOut3 = TRUE;
					pWnd->SetWindowText("关闭J61");
				}
				if(i == 2 && !(pSysInfo->nIoState[0]& 0x4))
				{
					m_bEnableOut3 = FALSE;
					pWnd->SetWindowText("打开J61");
				}

				if(i == 3 && (pSysInfo->nIoState[0] & 0x8))
				{
					m_bEnableOut4 = TRUE;
					pWnd->SetWindowText("关闭J62");
				}
				if(i == 3 && !(pSysInfo->nIoState[0]& 0x8))
				{
					m_bEnableOut4 = FALSE;
					pWnd->SetWindowText("打开J62");
				}

				if(i == 4 && (pSysInfo->nIoState[0] & 0x10))
				{
					m_bEnableOut5 = TRUE;
					pWnd->SetWindowText("关闭J63");
				}
				if(i == 4 && !(pSysInfo->nIoState[0]& 0x10))
				{
					m_bEnableOut5 = FALSE;
					pWnd->SetWindowText("打开J63");
				}

				if(i == 5 && (pSysInfo->nIoState[0] & 0x20))
				{
					m_bEnableOut6 = TRUE;
					pWnd->SetWindowText("关闭J64");
				}
				if(i == 5 && !(pSysInfo->nIoState[0]& 0x20))
				{
					m_bEnableOut6 = FALSE;
					pWnd->SetWindowText("打开J64");
				}

			}

		}
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CAirInkDialog::OnBnClickedButtonSet()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!m_bEnablePress)
	{
		AfxMessageBox(_T("请打开负压允许功能"));
		return;
	}
	UpdateData();
	for (int i = 0; i < 2; i++) {
		g_sysParam.fAirCtlPress[0][i] = m_fAirPressCtl[i];
		g_sysParam.fSafeAirPress[0][i] = m_fAirPressSafeCtl[i];
		g_sysParam.fInkTemp[0][i] = m_fInkTemp[i];
	}
	if (DEV_UpdateParam(&g_sysParam)) {
		if (!MVT_SetAirPressCtlVal(0))
			AfxMessageBox(_T("设置温度气压失败"));
	}
}


void CAirInkDialog::OnBnClickedButtonPushInk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (!m_bStartPushInk) {
		m_dwTick = GetTickCount();
		MVT_SetOutPut(0,0x8,TRUE);
		SetTimer(0x100,100,NULL);
		m_bStartPushInk = TRUE;
	}
	else {
		m_bStartPushInk = FALSE;
	}
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_PUSH_INK);
	if (pWnd)
		pWnd->SetWindowText(m_bStartPushInk?_T("停止压墨"):_T("开始压墨"));
	
}


void CAirInkDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString sz;
	CWnd* pWnd;
	if (nIDEvent == 0X100) {
		if ((GetTickCount() - m_dwTick > m_nPushInkSec * 1000) || !m_bStartPushInk) {
			MVT_SetOutPut(0,0x8 , FALSE);
			m_bStartPushInk = FALSE;
			CWnd* pWnd = GetDlgItem(IDC_BUTTON_PUSH_INK);
			if (pWnd)
				pWnd->SetWindowText(_T("开始压墨"));
			KillTimer(0x100);
		}
	}
	else if (nIDEvent == 0x101){
		sz.Format(_T("%.1f"), g_pSysInfo->InkCtlInfo[0].fInkAirPressure[0]);
		pWnd = GetDlgItem(IDC_STATIC_CUR_PRESS1);
		if (pWnd)
			pWnd->SetWindowText(sz);
		sz.Format(_T("%.1f"), g_pSysInfo->InkCtlInfo[0].fInkAirPressure[1]);
		pWnd = GetDlgItem(IDC_STATIC_CUR_PRESS2);
		if (pWnd)
			pWnd->SetWindowText(sz);
		m_InkState[0].SetInfo((g_pSysInfo->nIoState[0] & (1 << 16) ? STATIC_ERROR_FLAG : STATIC_NORMAL_FLAG), 1);//显示液位1
		m_InkState[1].SetInfo((g_pSysInfo->nIoState[0] & (1 << 17) ? STATIC_ERROR_FLAG : STATIC_NORMAL_FLAG), 2);//显示液位2
		m_InkState[3].SetInfo((g_pSysInfo->nIoState[0] & (1 << 18) ? STATIC_ERROR_FLAG : STATIC_NORMAL_FLAG), 3);//显示液位3
		m_InkState[4].SetInfo((g_pSysInfo->nIoState[0] & (1 << 19) ? STATIC_ERROR_FLAG : STATIC_NORMAL_FLAG), 4);//显示液位4
		m_InkState[2].SetInfo((g_pSysInfo->nIoState[0] & (1 << 21) ? STATIC_ERROR_FLAG : STATIC_NORMAL_FLAG), 6);//显示液位保护信号

	}
	CDialog::OnTimer(nIDEvent);
}


void CAirInkDialog::OnDestroy()
{
	KillTimer(0x101);
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void CAirInkDialog::OnBnClickedCheckEnableSupply() {
	UpdateData();
	CWnd* pWnd;
	if (m_bEnableSupply) 
		g_sysParam.nIoOption |= 0x1;
	else
		g_sysParam.nIoOption &= ~0x1;

	for (int i = 0; i < 2; i++) {
		pWnd = GetDlgItem(_inkCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(m_bEnableSupply);
	}
	DEV_UpdateParam(&g_sysParam);
	MVT_SetOutPutConfig(0);
}

void CAirInkDialog::OnBnClickedCheckEnablePress() {
	UpdateData();
	if (m_bEnablePress)
		g_sysParam.nIoOption |= 0x2;
	else
		g_sysParam.nIoOption &= ~0x2;
	CWnd* pWnd;
	for (int i = 0; i < 4; i++) {
		pWnd = GetDlgItem(_pressCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(!m_bEnablePress);
	}
	DEV_UpdateParam(&g_sysParam);
	if(!MVT_SetOutPutConfig(0))
	{
		AfxMessageBox("MVT_SetOutPutConfig error");
	}

}
void CAirInkDialog::OnBnClickedCheckOut1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;

	pWnd = GetDlgItem(_inkCtl[0]);
	if(m_bEnableOut1)
	{
		pWnd->SetWindowText("打开J58");
		MVT_SetOutPut(0,0x1,FALSE);
		m_bEnableOut1 = FALSE;
	}else
	{
		pWnd->SetWindowText("关闭J58");
		MVT_SetOutPut(0,0x1,TRUE);
		m_bEnableOut1 = TRUE;
	}
}

void CAirInkDialog::OnBnClickedCheckOut2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	pWnd = GetDlgItem(_inkCtl[1]);
	if(m_bEnableOut2)
	{
		//IsOpenJ59 = !m_bEnableOut2;
		MVT_SetOutPut(0, 0x2, !m_bEnableOut2);
		pWnd->SetWindowText("打开J59");
		m_bEnableOut2 = FALSE;
	}
	else
	{
		//IsOpenJ59 = !m_bEnableOut2;
		MVT_SetOutPut(0, 0x2, !m_bEnableOut2);
		pWnd->SetWindowText("关闭J59");
		m_bEnableOut2 = TRUE;
	}
}


void CAirInkDialog::OnBnClickedCheckOut3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	pWnd = GetDlgItem(_inkCtl[2]);
	if(m_bEnableOut3)
	{
		MVT_SetOutPut(0, 0x4, !m_bEnableOut3);
		pWnd->SetWindowText("打开J61");
		m_bEnableOut3 = FALSE;
	}else
	{
		MVT_SetOutPut(0, 0x4, !m_bEnableOut3);
		pWnd->SetWindowText("关闭J61");
		m_bEnableOut3 = TRUE;
	}
}


void CAirInkDialog::OnBnClickedCheckOut4()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	pWnd = GetDlgItem(_inkCtl[3]);
	if(m_bEnableOut4)
	{
		MVT_SetOutPut(0, 0x8, !m_bEnableOut4);
		pWnd->SetWindowText("打开J62");
		m_bEnableOut4 = FALSE;
	}else
	{
		MVT_SetOutPut(0, 0x8, !m_bEnableOut4);
		pWnd->SetWindowText("关闭J62");
		m_bEnableOut4 = TRUE;
	}
}


void CAirInkDialog::OnBnClickedCheckOut5()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	pWnd = GetDlgItem(_inkCtl[4]);
	if(m_bEnableOut5)
	{
		MVT_SetOutPut(0, 0x10, !m_bEnableOut5);
		pWnd->SetWindowText("打开J63");
		m_bEnableOut5 = FALSE;
	}else
	{
		MVT_SetOutPut(0, 0x10, !m_bEnableOut5);
		pWnd->SetWindowText("关闭J63");
		m_bEnableOut5 = TRUE;
	}
}


void CAirInkDialog::OnBnClickedCheckOut6()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	pWnd = GetDlgItem(_inkCtl[5]);
	if(m_bEnableOut6)
	{
		MVT_SetOutPut(0, 0x20, !m_bEnableOut6);
		pWnd->SetWindowText("打开J64");
		m_bEnableOut6 = FALSE;
	}else
	{
		MVT_SetOutPut(0, 0x20, !m_bEnableOut6);
		pWnd->SetWindowText("关闭J64");
		m_bEnableOut6 = TRUE;
	}
}


void CAirInkDialog::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CWnd* pWnd;
	if (m_bEnableAutoPress) 
		g_sysParam.nIoOption |= 0x4;
	else
		g_sysParam.nIoOption &= ~0x4;

	for (int i = 0; i < 6; i++) {
		pWnd = GetDlgItem(_pressCtl[i]);
		if (pWnd)
			pWnd->EnableWindow(!m_bEnableAutoPress);
	}
	DEV_UpdateParam(&g_sysParam);
	MVT_SetOutPutConfig(0);
}
