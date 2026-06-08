// CPrtJobDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CPrtJobDlg.h"
#include "afxdialogex.h"


// CPrtJobDlg 对话框
extern PRTJOB_ITEM g_testJob;
IMPLEMENT_DYNAMIC(CPrtJobDlg, CDialogEx)

CPrtJobDlg::CPrtJobDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PRT_JOB_SET, pParent)
	, m_nGrayBits(1)
	, m_fYOutDPI(635)
	, m_fXOutDPI(600)
	, m_fClipWidth(0)
	, m_fClipHeight(0)
	, m_fYPrtPos(0)
	, m_fXPrtPos(100)
	, m_nFileType(0)
	, m_szJobName(_T("3D Net Scan"))
	, m_nJobID(0)
	, m_bWhiteJump(FALSE)
	, m_bCycleOff(FALSE)
	, m_bRadomJetOff(FALSE)
	, m_bXMirror(FALSE)
	, m_bYMirror(FALSE)
	, m_bDoubleYDpi(FALSE)
	,m_bScreenshotPrt(FALSE)
{

}

CPrtJobDlg::~CPrtJobDlg()
{
}

void CPrtJobDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT_OUT_GRAYBITS, m_nGrayBits);
	//DDX_Text(pDX, IDC_EDIT_X_OUT_DPI, m_fYOutDPI);
	//DDX_Text(pDX, IDC_EDIT_X_OUTDPI, m_fXOutDPI);
	//DDX_Text(pDX, IDC_EDIT_CLIPWIDTH, m_fClipWidth);
	//DDX_Text(pDX, IDC_EDIT_CLIPHEIGHT, m_fClipHeight);
	//DDX_Text(pDX, IDC_EDIT_Y_STARTPOS, m_fYPrtPos);
	DDX_Text(pDX, IDC_EDIT_X_STARTPOS, m_fXPrtPos);
	//DDX_CBIndex(pDX, IDC_COMBO1, m_nFileType);
	//DDX_Text(pDX, IDC_EDIT_JOBNAME, m_szJobName);
	//DDX_Text(pDX, IDC_EDIT_JOBID, m_nJobID);
	DDX_Text(pDX, IDC_EDIT_LAYERCNT,m_nImageLayerCount);
	DDX_Check(pDX, IDC_CHECK_WHITE_JUMP, m_bWhiteJump);
	DDX_Check(pDX, IDC_CHECK_CYCLE, m_bCycleOff);
	DDX_Check(pDX, IDC_CHECK_RDOM_JET, m_bRadomJetOff);
	DDX_Check(pDX, IDC_CHECK_X_MIRROR, m_bXMirror);
	DDX_Check(pDX, IDC_CHECK_Y_MIRROR, m_bYMirror);
	//DDX_Check(pDX, IDC_CHECK_Y_DOUBLE_DPI, m_bDoubleYDpi);
	//DDX_Check(pDX, IDC_CHECK_Y_DOUBLE_DPI2, m_bScreenshotPrt);
}


BEGIN_MESSAGE_MAP(CPrtJobDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPrtJobDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPrtJobDlg 消息处理程序


void CPrtJobDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	g_testJob.nJobID = 0;
	//CopyMemory(g_testJob.szJobName, m_szJobName, m_szJobName.GetLength());
	g_testJob.fPrtXPos = m_fXPrtPos;
	//g_testJob.fClipHeight = m_fClipHeight;
	//g_testJob.fClipWidth = m_fClipWidth;
	//g_testJob.fOutXdpi = m_fXOutDPI;
	//g_testJob.fOutYdpi = m_fYOutDPI;
	//g_testJob.nFileType = m_nFileType;
	//g_testJob.nOutPixelBits = m_nGrayBits;
	//GetDlgItemText(IDC_MFCEDITBROWSE, m_szJobFilePath);
	//CopyMemory(g_testJob.szJobFilePath, m_szJobFilePath, m_szJobFilePath.GetLength());
	g_testJob.nPrtCtl &= ~0xff;
	if (m_bWhiteJump)
		g_testJob.nPrtCtl |= 0x1;
	if (m_bCycleOff)
		g_testJob.nPrtCtl |= 0x2;
	if (m_bRadomJetOff)
		g_testJob.nPrtCtl |= 0x4;
	if (m_bXMirror)
		g_testJob.nPrtCtl |= 0x10;
	if (m_bYMirror)
		g_testJob.nPrtCtl |= 0x20;
	if (m_bDoubleYDpi)
		///g_testJob.nPrtCtl |= 0x40;
	if(m_bScreenshotPrt)
	{
		//g_testJob.nPrtCtl |= 0x80;
	}

	CDialogEx::OnOK();
}


BOOL CPrtJobDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//m_nJobID = g_testJob.nJobID;
	m_fXPrtPos = g_testJob.fPrtXPos;
	//m_fClipHeight = g_testJob.fClipHeight;
	//m_fClipWidth = g_testJob.fClipWidth;
	//m_fXOutDPI = g_testJob.fOutXdpi;
	//m_fYOutDPI = g_testJob.fOutYdpi;
	//m_nFileType = g_testJob.nFileType;
	//m_nGrayBits = g_testJob.nOutPixelBits;
	m_bWhiteJump = (g_testJob.nPrtCtl & 0x1) ? TRUE : FALSE;
	m_bCycleOff = (g_testJob.nPrtCtl & 0x2) ? TRUE : FALSE;
	m_bRadomJetOff = (g_testJob.nPrtCtl & 0x4) ? TRUE : FALSE;
	m_bXMirror = (g_testJob.nPrtCtl & 0x10) ? TRUE : FALSE;
	m_bYMirror = (g_testJob.nPrtCtl & 0x20) ? TRUE : FALSE;
	//m_bDoubleYDpi = (g_testJob.nPrtCtl & 0x40) ? TRUE : FALSE;
	//m_bScreenshotPrt = (g_testJob.nPrtCtl & 0x80) ? TRUE : FALSE;
	// TODO:  在此添加额外的初始化
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
