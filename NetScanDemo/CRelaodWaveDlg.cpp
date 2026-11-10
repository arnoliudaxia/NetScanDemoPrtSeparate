// CRelaodWaveDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CRelaodWaveDlg.h"
#include "afxdialogex.h"


// CRelaodWaveDlg 对话框

IMPLEMENT_DYNAMIC(CReloadWaveDlg, CDialogEx)

CReloadWaveDlg::CReloadWaveDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_RELOAD_WAVE, pParent)
	, m_nColorIndex(0)
	, m_nGroupIndex(0)
	, m_szWavePath(_T(""))
{

}

CReloadWaveDlg::~CReloadWaveDlg()
{
}

void CReloadWaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_COLOR, m_nColorIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GROUP, m_nGroupIndex);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_WAVE, m_WavePathBrowse);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_WAVE, m_szWavePath);
}


BEGIN_MESSAGE_MAP(CReloadWaveDlg, CDialogEx)
END_MESSAGE_MAP()


// CRelaodWaveDlg 消息处理程序

void CReloadWaveDlg::OnOK()
{
	if (!UpdateData(TRUE))
		return;
	m_szWavePath.Trim();
	// 文件筛选不能阻止手工输入其他类型，确认时再次校验。
	if (m_szWavePath.Right(6).CompareNoCase(_T(".rhdat")) != 0)
	{
		AfxMessageBox(_T("请选择后缀名为 .rhdat 的波形文件。"));
		m_WavePathBrowse.SetFocus();
		return;
	}
	const DWORD nAttributes = ::GetFileAttributes(m_szWavePath);
	if (nAttributes == INVALID_FILE_ATTRIBUTES || (nAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		AfxMessageBox(_T("波形文件不存在或无法访问，请重新选择。"));
		m_WavePathBrowse.SetFocus();
		return;
	}
	// 将去除首尾空白后的路径同步回控件，避免基类再次DDX时覆盖。
	m_WavePathBrowse.SetWindowText(m_szWavePath);
	CDialogEx::OnOK();
}

BOOL CReloadWaveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_WavePathBrowse.EnableFileBrowseButton(_T("rhdat"),
		_T("波形文件 (*.rhdat)|*.rhdat||"),
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR);
	CString sz;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COLOR);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int c = 0; c < MAX_COLORS; c++) {
			sz.Format(_T("颜色%d"), c);
			pComboBox->AddString(sz);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUP);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int g = 0; g < MAX_GROUP; g++) {
			sz.Format(_T("组%d"), g);
			pComboBox->AddString(sz);
		}
		pComboBox->SetCurSel(0);
	}
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
