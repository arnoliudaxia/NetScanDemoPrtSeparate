// CMoveCtlDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CMoveCtlDlg.h"
#include "afxdialogex.h"

extern MOV_Config g_movConfig;
// CMoveCtlDlg 对话框

IMPLEMENT_DYNAMIC(CMoveCtlDlg, CDialog)

CMoveCtlDlg::CMoveCtlDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MOVE_CTL, pParent)
	, m_fSpd(0)
	, m_fXAccTime(0)
	, m_fXMoveRate(0)
	, m_fXMoveUnit(0)
	, m_fXSysDpi(0)
	, m_fYspd(0)
	, m_fYAccTime(0)
	, m_fYMoveRate(0)
	, m_fYMoveUnit(0)
	, m_fYSysDpi(0)
	, nMachineType(0)
	, m_bXReverse(FALSE)
	, m_bYReverse(FALSE)
	, m_fMovBuf(50)
{

}

CMoveCtlDlg::~CMoveCtlDlg()
{
}

void CMoveCtlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_XSPD, m_fSpd);
	DDX_Text(pDX, IDC_EDIT_XACC, m_fXAccTime);
	DDX_Text(pDX, IDC_EDIT_XMOVRATE, m_fXMoveRate);
	DDX_Text(pDX, IDC_EDIT_XMOV_UNIT, m_fXMoveUnit);
	DDX_Text(pDX, IDC_EDIT_XDPI, m_fXSysDpi);
	DDX_Text(pDX, IDC_EDIT_YSPD, m_fYspd);
	DDX_Text(pDX, IDC_EDIT_YACC, m_fYAccTime);
	DDX_Text(pDX, IDC_EDIT_YMOVERATE, m_fYMoveRate);
	DDX_Text(pDX, IDC_EDIT_YMOVE_UNIT, m_fYMoveUnit);
	DDX_Text(pDX, IDC_EDIT_YDPI, m_fYSysDpi);
	DDX_CBIndex(pDX, IDC_COMBO_MACHINE_TYPE, nMachineType);
	DDX_Check(pDX, IDC_CHECK_XREVERSE, m_bXReverse);
	DDX_Check(pDX, IDC_CHECK_YREVERSE, m_bYReverse);
	DDX_Text(pDX, IDC_EDIT_MOVBUF, m_fMovBuf);
}


BEGIN_MESSAGE_MAP(CMoveCtlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CMoveCtlDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_FRONT, &CMoveCtlDlg::OnBnClickedButtonFront)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CMoveCtlDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CMoveCtlDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CMoveCtlDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMoveCtlDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CMoveCtlDlg::OnBnClickedButtonApply)
	ON_EN_CHANGE(IDC_EDIT_XMOVRATE, &CMoveCtlDlg::OnEnChangeEditXmovrate)
END_MESSAGE_MAP()


// CMoveCtlDlg 消息处理程序


void CMoveCtlDlg::OnBnClickedButtonReset()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	DEM_StopAll(TRUE);
	DEM_UpdateXMovCfg(g_movConfig.fxMovAcc,g_movConfig.fxIoOption);
	DEM_UpdateYMovCfg(g_movConfig.fyMovAcc, g_movConfig.fyIoOption);
	DEM_EnableRun();
#endif

}


void CMoveCtlDlg::OnBnClickedButtonFront()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	UpdateData(FALSE);
	BOOL bDir = m_bYReverse?TRUE:FALSE;
	int nSpd = MM_TO_DOT(g_movConfig.fyMovSpd,g_movConfig.fySysdpi)*48;
	int nStp = MM_TO_DOT(g_movConfig.fyMovUnit, g_movConfig.fySysdpi)*48;
	DEM_MoveY(bDir,nSpd,nStp,false,false,g_movConfig.fyMovRate);
#endif

	
}


void CMoveCtlDlg::OnBnClickedButtonLeft()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	UpdateData(FALSE);
	BOOL bDir = m_bXReverse ? FALSE : TRUE;
	int nSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
	int nStp = MM_TO_DOT(g_movConfig.fxMovUnit, g_movConfig.fxSysdpi);
	DEM_MoveX(bDir, nSpd, nStp, false, g_movConfig.fxMovRate);
#endif
}


void CMoveCtlDlg::OnBnClickedButtonBack()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	UpdateData(FALSE);
	BOOL bDir = m_bYReverse ? FALSE : TRUE;
	int nSpd = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
	int nStp = MM_TO_DOT(g_movConfig.fyMovUnit, g_movConfig.fySysdpi) * 48;
	DEM_MoveY(bDir, nSpd, nStp, false, false, g_movConfig.fyMovRate);
#endif
}


void CMoveCtlDlg::OnBnClickedButtonRight()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	UpdateData(FALSE);
	BOOL bDir = m_bXReverse ? TRUE : FALSE;
	int nSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
	int nStp = MM_TO_DOT(g_movConfig.fxMovUnit, g_movConfig.fxSysdpi);
	DEM_MoveX(bDir, nSpd, nStp, false, g_movConfig.fxMovRate);
#endif
}


void CMoveCtlDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	DEM_StopAll(TRUE);
#endif
}


void CMoveCtlDlg::OnBnClickedButtonApply()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	g_movConfig.fxMovSpd = m_fSpd;
	g_movConfig.fxMovAcc = m_fXAccTime;
	g_movConfig.fxMovUnit = m_fXMoveUnit;
	g_movConfig.fyMovSpd = m_fYspd;
	g_movConfig.fyMovAcc = m_fYAccTime;
	g_movConfig.fyMovUnit = m_fYMoveUnit;
	g_movConfig.fxSysdpi = m_fXSysDpi;
	g_movConfig.fySysdpi = m_fYSysDpi;
	g_movConfig.fxMovRate = m_fXMoveRate;
	g_movConfig.fyMovRate = m_fYMoveRate;
	g_movConfig.fMovBuf = m_fMovBuf;
}


BOOL CMoveCtlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_fSpd = g_movConfig.fxMovSpd;
	m_fXAccTime = g_movConfig.fxMovAcc;// TODO:  在此添加额外的初始化
	m_fXMoveUnit = g_movConfig.fxMovUnit;
	m_fYspd = g_movConfig.fyMovSpd;
    m_fYAccTime = g_movConfig.fyMovAcc;			  // 异常: OCX 属性页应返回 FALSE
	m_fYMoveUnit = g_movConfig.fyMovUnit;
	m_fXSysDpi = g_movConfig.fxSysdpi;
	m_fYSysDpi = g_movConfig.fySysdpi;
	m_fXMoveRate = g_movConfig.fxMovRate;
	m_fYMoveRate = g_movConfig.fyMovRate;
	m_fMovBuf = g_movConfig.fMovBuf;
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a contro
}

void CMoveCtlDlg::OnEnChangeEditXmovrate()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
