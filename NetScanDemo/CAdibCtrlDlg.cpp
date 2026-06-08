// CAdibCtrlDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CAdibCtrlDlg.h"
#include "afxdialogex.h"
#include "..\Inc\RYPrtCtler.h"

ADIB_PARAM		adibCurState;
ADIB_PARAM		_adibSettings;
extern LPPRINTER_INFO g_pSysInfo;
// CAdibCtrlDlg 对话框
void InitAdibParam()
{
	ZeroMemory(&_adibSettings, sizeof(ADIB_PARAM));
	_adibSettings.fAirThreshold = 0.5f;
	for (int i = 0; i < 8; i++)
	{
		_adibSettings.fcurAirPress[i] = 5.2f;
		_adibSettings.fcurvoltage[i] = 24.0f;
	}
}
IMPLEMENT_DYNAMIC(CAdibCtrlDlg, CDialog)

CAdibCtrlDlg::CAdibCtrlDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_ADIBCTL, pParent)
	, m_fAirHold(0)
	, m_nLnkTime(0)
	,m_fInkSupplyTime(0)
	, m_bSetEnable(FALSE)
	, m_bIICRead(FALSE)
{
	m_bMonitorRuning = TRUE;
	m_bStopMonitor = FALSE;
	m_bSetEnable = FALSE;
	m_bCleanMode = FALSE;
	m_bCleanPump1 = FALSE;
	m_bCleanHead = FALSE;
	m_bCleanPump2 = FALSE;
	InitAdibParam();
}

CAdibCtrlDlg::~CAdibCtrlDlg()
{
}

void CAdibCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_AIDBINPUT, m_SigLogicInput);
	DDX_Control(pDX, IDC_LIST_ADIB, m_AdibList);
	DDX_Text(pDX, IDC_EDIT_THRESHOLD, m_fAirHold);
	DDX_Text(pDX, IDC_EDIT_THRESHOLD2, m_nLnkTime);
	DDX_Text(pDX, INKSUPLYTIME, m_fInkSupplyTime);
	
	DDX_Check(pDX, IDC_CHECK_ENABLE_SET, m_bSetEnable);
}


BEGIN_MESSAGE_MAP(CAdibCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_APPLY, &CAdibCtrlDlg::OnApply)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SET, &CAdibCtrlDlg::OnBnCheckImport)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLEANMODE, &CAdibCtrlDlg::OnBnClickedButtonCleanmode)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN_PUMP2, &CAdibCtrlDlg::OnBnClickedButtonCleanPump2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN_PUMP1, &CAdibCtrlDlg::OnBnClickedButtonCleanPump1)
	//ON_BN_CLICKED(LNKIDOK, &CAdibCtrlDlg::OnBnClickedLnkidok)
	//ON_BN_CLICKED(LNKIDOK2, &CAdibCtrlDlg::OnBnClickedLnkidok2)
	//ON_BN_CLICKED(CLEANHEADIDOK, &CAdibCtrlDlg::OnBnClickedCleanheadidok)
	ON_BN_CLICKED(CLEANHEADIDISOK, &CAdibCtrlDlg::OnBnClickedCleanheadidisok)
	ON_BN_CLICKED(LNKISOK, &CAdibCtrlDlg::OnBnClickedLnkisok)
	ON_BN_CLICKED(IDC_BUTTON1, &CAdibCtrlDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CAdibCtrlDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON2, &CAdibCtrlDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

TCHAR* _szHes[16] = { _T("参数列表"),_T("1"),_T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7"),_T("8") };
TCHAR* _szVes[16] = { _T("电压(V)"),_T("温度(℃)"),_T("负压(kpa)") };
COLORREF   _crText[16] = { RGB(50,50,50),RGB(80,5,15),RGB(75,5,90),RGB(100,100,100) };

BOOL CAdibCtrlDlg::OnInitDialog()
{
	g_pSysInfo = DEV_GetDeviceInfo();
	if (g_pSysInfo->nAdibInfo.fAirThreshold > 0.0f)
		m_fAirHold = g_pSysInfo->nAdibInfo.fAirThreshold;
	//int test = m_nLnkTime;
	if(g_pSysInfo->nAdibInfo.fPressInkTime>0)
	{
		//m_nLnkTime = g_pSysInfo->nAdibInfo.fPressInkTime;
	}
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	DWORD dw = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TRACKSELECT;
	m_AdibList.InitColum(9, _szHes, dw, 100);//,dwStyle);
	m_AdibList.InitItem(3, _szVes, _crText);
	m_AdibList.SetListFont(22, _T("Arial"), FALSE);
	::DEV_AdibControl(&_adibSettings, 0x40000, 0, NULL, 0);
	_adibSettings.fcurAirPress[0] = g_pSysInfo->nAdibInfo.I2C.fAirPress[0];
	_adibSettings.fcurAirPress[1] = g_pSysInfo->nAdibInfo.I2C.fAirPress[1];
	AfxBeginThread(VTMonitorThread, this);
	SetTimer(0x300, 1000, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
// CAdibCtrlDlg 消息处理程序
UINT CAdibCtrlDlg::VTMonitorThread(LPVOID pvoid)
{

	int test  = sizeof(ADIB_PARAM);
	CString szTitle;
	CAdibCtrlDlg* pdlg = (CAdibCtrlDlg*)pvoid;
	//	CAdibDemoApp* pApp=(CAdibDemoApp*)AfxGetApp();
	UINT nIoOption = 0xF;
	if (pdlg)
	{
		while (!pdlg->m_bStopMonitor)
		{
			if (!pdlg->m_bSetEnable)
			{
				//nIoOption=0x1FF000F;
				nIoOption |= 0x10;
				pdlg->m_bComState = ::DEV_AdibControl(&adibCurState, nIoOption, 0, (bool*)&pdlg->m_bSetEnable,0);
				if (pdlg->m_bComState)
					nIoOption = 0xE;
				else
					nIoOption = 0xF;

			}
			Sleep(100);
		}
	}
	AfxEndThread(0, TRUE);
	return 0;
}
bool bLinkstatus = false;
bool bClickLinkWin = false;
bool g_bLink2status = false;
bool g_bClickLink2Win = false;
void CAdibCtrlDlg::UpdateAdibInfo(BOOL bState)
{
	CString szInfo;
	static BOOL bLastSet = FALSE;
	int nResult = 0;
	bool tetetttt= false;
	
	/////////////////更新连接状态
	/*if (bState)
	szInfo.Format(_T("ADIB  [FMVer: %X]"), adibCurState.nFMver);
	else
	szInfo = _T("ADIB >> disconnected ");*/

	//if (g_pSysInfo->nAdibInfo.nFMver == 0x20240927 || g_pSysInfo->nAdibInfo.nFMver == 0x20240830)
		szInfo.Format(_T("ADIB  [FMVer: %X]"), g_pSysInfo->nAdibInfo.nFMver);
	//else
		//szInfo = _T("ADIB >> disconnected ");
	SetWindowText(szInfo);
	//////////////////更新参数	
	if (m_hWnd && (!bLastSet))		//编辑时不查询
	{
		bLastSet = m_bSetEnable;
		//LPADIB_PARAM lpAdibparam = &adibCurState;
		g_pSysInfo = DEV_GetDeviceInfo();
		LPADIB_PARAM lpAdibparam = &g_pSysInfo->nAdibInfo;
		if (bLastSet)
			lpAdibparam = &_adibSettings;


		if((lpAdibparam->nStatus & 0x400))
		{
			if(!bLinkstatus)
			{
				
				if(!bClickLinkWin)
				{
					bClickLinkWin = true;
					bLinkstatus = true;
					nResult = MessageBox("是否继续供墨1", _T("提示"), MB_ICONINFORMATION | IDRETRY);
					if(nResult == IDYES)
					{
						int nIoOption =  0x100;
						nIoOption |= 0x400;
						::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);

						bClickLinkWin = false;
						bLinkstatus = false;
					}
					else
					{
						bClickLinkWin = false;
						bLinkstatus = true;
					}
				}
				
			}
		}

		if((lpAdibparam->nStatus & 0x800))
		{
			if(!g_bLink2status)
			{

				if(!g_bClickLink2Win)
				{
					g_bClickLink2Win = true;
					g_bLink2status = true;
					nResult = MessageBox("是否继续供墨2", _T("提示"), MB_ICONINFORMATION | IDRETRY);
					if(nResult == IDYES)
					{
						int nIoOption =  0x100;
						nIoOption |= 0x400;
						::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);

						g_bClickLink2Win = false;
						g_bLink2status = false;
					}
					else
					{
						g_bClickLink2Win = false;
						g_bLink2status = true;
					}
				}

			}
		}
		
		
		lpAdibparam->nLgStatus |= ((lpAdibparam->nStatus) << 8);
		m_SigLogicInput.SetInkState(~(lpAdibparam->nLgStatus & 0xFFFFFF));

		for (int i = 0; i < 8; i++)
		{
			szInfo.Format(_T("%0.2f"), lpAdibparam->fcurvoltage[i]);
			m_AdibList.SetItemText(0, i + 1, szInfo);

			szInfo.Format(_T("%0.2f"), lpAdibparam->fcurInkTankTemp[i]);
			m_AdibList.SetItemText(1, i + 1, szInfo);

			szInfo.Format(_T("%0.2f"), lpAdibparam->fcurAirPress[i]);
			m_AdibList.SetItemText(2, i + 1, szInfo);
		}

		/*if(lpAdibparam->nReverse[0] & 0x200 )
		{
		nResult = MessageBox("是否继续供墨1", _T("提示"), MB_ICONINFORMATION | IDRETRY);
		if(nResult == IDYES)
		{
		int nIoOption =  0x100;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		}

		}*/
		/*if(lpAdibparam->nStatus & 0x400)
		{
		nResult = MessageBox("是否继续供墨2", _T("提示"), MB_ICONINFORMATION | IDRETRY);
		if(nResult == IDYES)
		{
		int nIoOption =  0x400;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		}*/
		//}
	}
	else if (!m_bSetEnable)
		bLastSet = FALSE;
}

void CAdibCtrlDlg::OnApply()
{
	if (!UpdateData())
		return;
	float fcurval[8];
	UINT nIoOption = 0;
	for (int i = 0; i < 3; i++)	//电压暂时没用 0 -> 1
	{
		if (m_AdibList.GetCheck(i))
		{
			m_AdibList.GetLineData(i, fcurval, FALSE);	//20171223 支持正负压 TRUE-> FALSE
			if (1 == i)
			{
				nIoOption |= 0x2;
				CopyMemory(_adibSettings.fcurInkTankTemp, fcurval, 4 * 8);
			}
			else if (2 == i)
			{
				nIoOption |= 0x4;
				CopyMemory(_adibSettings.fcurAirPress, fcurval, 4 * 8);
			}
			else
			{
				nIoOption |= 0x10;
				CopyMemory(_adibSettings.fcurvoltage, fcurval, 4 * 8);
			}

		}
	}
	if (m_fAirHold > 0.0f)
	{
		nIoOption |= 0x20;
		_adibSettings.fAirThreshold = m_fAirHold;
	}

	if(m_nLnkTime > 0)
	{
		nIoOption |= 0x10000;
		_adibSettings.fPressInkTime = m_nLnkTime;
	}
	nIoOption |= 0x80000000; // 保存IIC
	nIoOption |= 0x40000000; // 读取IIC
	::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL,0);

	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_BTN_APPLY);
	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_ENABLE_SET);
	if (pBtn && pWnd)
	{
		m_bSetEnable = FALSE;
		pBtn->SetCheck(FALSE);
		m_AdibList.EnableInput(FALSE);
		pWnd->ShowWindow(SW_HIDE);
	}
}


void CAdibCtrlDlg::OnBnCheckImport()
{
	// TODO: Add your control notification handler code here
	if (!UpdateData())
		return;
	CString szInfo;
	BOOL bEnable = FALSE;
	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_BTN_APPLY);
	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_ENABLE_SET);
	if (pBtn && pWnd)
	{
		
		bEnable = pBtn->GetCheck();
		m_AdibList.EnableInput(bEnable);
		pWnd->ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
	}
}


void CAdibCtrlDlg::OnDestroy()
{
	KillTimer(0x300);
	::DEV_AdibControl(&_adibSettings, 0xA00, 1, NULL, 0); // 退出界面关闭供墨防止一直供墨
	CDialog::OnDestroy();
	m_bSetEnable = TRUE;		//停止查询
	m_bStopMonitor = TRUE;
	// TODO: Add your message handler code here
}


void CAdibCtrlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (0x300 == nIDEvent)
	{
		UpdateAdibInfo(m_bComState);
	}
	CDialog::OnTimer(nIDEvent);
}



void CAdibCtrlDlg::OnBnClickedButtonCleanmode()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd;
	if (!m_bCleanMode) {
		int nIoOption = 0x80;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		m_bCleanMode = TRUE;
		pWnd = GetDlgItem(IDC_BUTTON_CLEANMODE);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗模式关"));
	}
	else {
		int nIoOption = 0x40;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		pWnd = GetDlgItem(IDC_BUTTON_CLEANMODE);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗模式开"));
		m_bCleanMode = FALSE;
	}
	
}


void CAdibCtrlDlg::OnBnClickedButtonCleanPump2()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd;
	if (!m_bCleanPump2) {
		int nIoOption = 0x1000;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		m_bCleanPump2 = TRUE;
		pWnd = GetDlgItem(IDC_BUTTON_CLEAN_PUMP2);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗泵2关"));
	}
	else {
		int nIoOption = 0x2000;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		pWnd = GetDlgItem(IDC_BUTTON_CLEAN_PUMP2);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗泵2开"));
		m_bCleanPump2 = FALSE;
	}
}


void CAdibCtrlDlg::OnBnClickedButtonCleanPump1()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd;
	char buf[1024] = {0};
	if (!m_bCleanPump1) {
		int nIoOption = 0x4000;
		//sprintf(buf,"%s %f %f","I2C_DATA 1 2",_adibSettings.I2C.fAirPress[0],_adibSettings.I2C.fAirPress[1]);
		//AfxMessageBox(_T(buf));
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		m_bCleanPump1 = TRUE;
		pWnd = GetDlgItem(IDC_BUTTON_CLEAN_PUMP1);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗泵1关"));
	}
	else {
		int nIoOption = 0x8000;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		pWnd = GetDlgItem(IDC_BUTTON_CLEAN_PUMP1);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗泵1开"));
		m_bCleanPump1 = FALSE;
	}
}




void CAdibCtrlDlg::OnBnClickedCleanheadidisok()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd;
	if(!m_bCleanHead)
	{
		MVT_SetOutPut(0,0x1,true);
		MVT_SetOutPut(0, 0x2, true);
		int nIoOption =  0x4000;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		m_bCleanHead = TRUE;
		pWnd = GetDlgItem(CLEANHEADIDOK);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗喷头关"));
	}
	else
	{
		MVT_SetOutPut(0,0x1,false);
		MVT_SetOutPut(0, 0x2, false);
		int nIoOption =  0x8000;
		::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
		m_bCleanHead = FALSE;
		pWnd = GetDlgItem(CLEANHEADIDOK);
		if (pWnd)
			pWnd->SetWindowText(_T("清洗喷头开"));
	}
}


void CAdibCtrlDlg::OnBnClickedLnkisok()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!UpdateData())
		return;
	int nIoOption =  0x10000;
	//int nIoOption =  0x100;
	_adibSettings.fPressInkTime = m_nLnkTime;
	::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
}


void CAdibCtrlDlg::OnBnClickedButton1()
{
	bLinkstatus = false;
	g_bLink2status = false;
	g_bClickLink2Win = false;
	bClickLinkWin = false;
	int nIoOption =  0x100;
	nIoOption |= 0x400;
	::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
	// TODO: 在此添加控件通知处理程序代码
}


void CAdibCtrlDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnOK();
}


void CAdibCtrlDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!UpdateData())
		return;
	/*bLinkstatus = false;
	g_bLink2status = false;
	g_bClickLink2Win = false;
	bClickLinkWin = false;
	int nIoOption =  0x100;
	nIoOption |= 0x400;
	::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);*/
	 int nIoOption =  0x20000;
	//int nIoOption =  0x100;
	_adibSettings.fInkSupplyTime = m_fInkSupplyTime;
	::DEV_AdibControl(&_adibSettings, nIoOption, 1, NULL, 0);
}
