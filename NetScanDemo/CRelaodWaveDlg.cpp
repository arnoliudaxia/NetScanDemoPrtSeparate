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
	DDX_Text(pDX, IDC_MFCEDITBROWSE_WAVE, m_szWavePath);
}


BEGIN_MESSAGE_MAP(CReloadWaveDlg, CDialogEx)
END_MESSAGE_MAP()


// CRelaodWaveDlg 消息处理程序


BOOL CReloadWaveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
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
