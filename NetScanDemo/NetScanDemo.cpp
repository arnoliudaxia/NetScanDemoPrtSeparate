
// NetScanDemo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "NetScanDemoDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetScanDemoApp
RYUSR_PARAM g_sysParam;
MOV_Config g_movConfig;
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern RYCalbrationParam g_Calbration;
BEGIN_MESSAGE_MAP(CNetScanDemoApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CNetScanDemoApp construction

CNetScanDemoApp::CNetScanDemoApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CNetScanDemoApp object

CNetScanDemoApp theApp;


// CNetScanDemoApp initialization

BOOL CNetScanDemoApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings();
	///////////////////»ñÈ¡APPÂ·¾¶
	TCHAR   m_FilePath[MAX_PATH];
	TCHAR   TempCh;
	BOOL    bSecInited = TRUE;
	CString szAppPath;
	ZeroMemory(m_FilePath, MAX_PATH);
	::GetModuleFileName(m_hInstance, m_FilePath, MAX_PATH);
	szAppPath = m_FilePath;
	int nsl = szAppPath.GetLength();

	int i = 0;
	for (i = nsl - 1; i > 0; i--)
	{
		TempCh = szAppPath.GetAt(i);
		if (TempCh == _T('\\'))
			break;
	}
	m_szAppPath = szAppPath.Left(i + 1);
#ifdef RY_MOVE_CTL
	LoadMoveParam();
#endif
	//////////////////////
	CNetScanDemoDlg dlg;
	dlg.m_szAppPath = m_szAppPath;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
	SaveParam(&g_sysParam);
	SaveCalibrationParam();
#ifdef RY_MOVE_CTL
	SaveMoveParam();
#endif
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


int CNetScanDemoApp::SaveParam(LPRYUSR_PARAM lparam)
{
	CFile file;
	CString szParamName;
	//if(nDevIndex == 0)
	//{
		szParamName = m_szAppPath + _T("\\userparam.dat");
	//}
	//else
	//{
	//	szParamName = m_szAppPath + _T("\\userparam-1.dat");
	//}
	CFileException pError;
	if ((file.Open(szParamName, CFile::modeCreate | CFile::modeReadWrite) <= 0))
	{
		return -1;
	}


	
	file.Write(&g_sysParam, sizeof(RYUSR_PARAM));

	
	file.Close();
	return 0;
}
 UINT StructSize = 0;
int CNetScanDemoApp::LoadParam(LPRYUSR_PARAM lparam)
{
	CFile file;
	UINT* pStartFlag = NULL;
	UINT* pEndFlag = NULL;
	UINT nBufSize = sizeof(RYUSR_PARAM);
	LPBYTE pDataBuf = new BYTE[nBufSize];
	CString szParamName;
	
	szParamName = m_szAppPath + _T("\\userparam.dat");

	if ((file.Open(szParamName, CFile::modeRead)<=0))
	{
		delete pDataBuf;
		return -1;
	}
	file.Read(pDataBuf, nBufSize);
	file.Close();
	
	CopyMemory(&g_sysParam, pDataBuf, sizeof(RYUSR_PARAM));
	

	delete pDataBuf;
	return 0;
}

#ifdef RY_MOVE_CTL
int CNetScanDemoApp::SaveMoveParam() {
	CFile file;
	CString szParamName = m_szAppPath + _T("\\moveParam.dat");
	if (!file.Open(szParamName, CFile::modeCreate | CFile::modeWrite))
		return -1;
	file.Write(&g_movConfig, sizeof(MOV_Config));
	file.Close();
}
int CNetScanDemoApp::LoadMoveParam() {
	CFile file;
	CString szParamName = m_szAppPath + _T("\\moveParam.dat");
	if (!file.Open(szParamName, CFile::modeRead))
		return -1;
	file.Read(&g_movConfig, sizeof(MOV_Config));
	file.Close();
}
#endif

int CNetScanDemoApp::SaveLayerParam()
{

	CFile file;
	CString szParamName;
	//if(nDevIndex == 0)
	//{
	szParamName = m_szAppPath + _T("\\\\pmc.dat");
	//}
	//else
	//{
	//	szParamName = m_szAppPath + _T("\\userparam-1.dat");
	//}
	CFileException pError;
	if ((file.Open(szParamName, CFile::modeCreate | CFile::modeReadWrite) <= 0))
	{
		return -1;
	}



	file.Write(&g_PrtImgLayer, sizeof(PRTIMG_LAYER));


	file.Close();
	return 0;
}

int CNetScanDemoApp::SaveJobParam()
{
	CFile file;
	CString szParamName = m_szAppPath + _T("\\job.dat");
	if (!file.Open(szParamName, CFile::modeCreate | CFile::modeWrite))
		return -1;
	file.Write(&g_testJob, sizeof(PRTJOB_ITEM));
	file.Close();
	return 0;
}

int CNetScanDemoApp::LoadLayerParam()
{
	CFile file;
	UINT nBufSize = sizeof(PRTIMG_LAYER);
	StructSize= sizeof(PRTIMG_LAYER);
	LPBYTE pDataBuf = new BYTE[nBufSize];
	CString szParamName = m_szAppPath + _T("\\pmc.dat");
	if (!file.Open(szParamName, CFile::modeRead))
	{
		delete pDataBuf;
		return -1;
	}
	file.Read(pDataBuf, nBufSize);
	file.Close();
	CopyMemory(&g_PrtImgLayer, pDataBuf, sizeof(PRTIMG_LAYER));
	delete pDataBuf;
	return 0;
}

int CNetScanDemoApp::LoadJobParam()
{
	CFile file;
	UINT nBufSize = sizeof(PRTJOB_ITEM);
	LPBYTE pDataBuf = new BYTE[nBufSize];
	StructSize= sizeof(PRTJOB_ITEM);
	CString szParamName = m_szAppPath + _T("\\job.dat");
	if (!file.Open(szParamName, CFile::modeRead))
	{
		delete pDataBuf;
		return -1;
	}
	file.Read(pDataBuf, nBufSize);
	file.Close();
	CopyMemory(&g_testJob, pDataBuf, sizeof(PRTJOB_ITEM));
	delete pDataBuf;
	return 0;
}

int CNetScanDemoApp::SaveCalibrationParam()
{
	CFile file;
	CString szParamName = m_szAppPath + _T("\\Calibration.dat");
	if (!file.Open(szParamName, CFile::modeCreate | CFile::modeWrite))
		return -1;
	file.Write(&g_Calbration, sizeof(RYCalbrationParam));
	file.Close();
	return 0;
}

int CNetScanDemoApp::LoadCalibrationParam()
{
	CFile file;
	UINT nBufSize = sizeof(RYCalbrationParam);
	StructSize= sizeof(PRTIMG_LAYER);
	LPBYTE pDataBuf = new BYTE[nBufSize];
	CString szParamName = m_szAppPath + _T("\\Calibration.dat");
	if (!file.Open(szParamName, CFile::modeRead))
	{
		delete pDataBuf;
		return -1;
	}
	file.Read(pDataBuf, nBufSize);
	file.Close();
	CopyMemory(&g_Calbration, pDataBuf, sizeof(RYCalbrationParam));
	delete pDataBuf;
	return 0;
}


