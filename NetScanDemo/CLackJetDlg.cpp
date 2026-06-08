// CLackJetDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CLackJetDlg.h"
#include "afxdialogex.h"


// CLackJetDlg 对话框
extern RYUSR_PARAM g_sysParam;
IMPLEMENT_DYNAMIC(CLackJetDlg, CDialogEx)

CLackJetDlg::CLackJetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_LACK_JET, pParent)
	, m_nColorIndex(0)
	, m_nGroupIndex(0)
	, m_nLackJet(0)
{

}

CLackJetDlg::~CLackJetDlg()
{
}

void CLackJetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_COLOR, m_nColorIndex);
	DDX_CBIndex(pDX, IDC_COMBO_GROUP, m_nGroupIndex);
	DDX_Text(pDX, IDC_EDIT_LACKJET, m_nLackJet);
}


BEGIN_MESSAGE_MAP(CLackJetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CLackJetDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CLackJetDlg::OnBnClickedButtonDelete)
END_MESSAGE_MAP()


// CLackJetDlg 消息处理程序


BOOL CLackJetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString szText;
	// TODO:  在此添加额外的初始化
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COLOR);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int c = 0; c < MAX_COLORS; c++) {
			szText.Format(_T("C-%d"), c+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUP);
	if (pComboBox) {
		pComboBox->ResetContent();
		for (int g = 0; g < MAX_GROUP; g++) {
			szText.Format(_T("G-%d"), g+1);
			pComboBox->AddString(szText);
		}
		pComboBox->SetCurSel(0);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CLackJetDlg::OnBnClickedButtonAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(FALSE);
	if (g_sysParam.nLackJetCount[m_nColorIndex][m_nGroupIndex] <= 31) {
		g_sysParam.nLackJetTbl[m_nColorIndex][m_nGroupIndex][g_sysParam.nLackJetCount[m_nColorIndex][m_nGroupIndex]] = m_nLackJet;
	    g_sysParam.nLackJetCount[m_nColorIndex][m_nGroupIndex]++;
	}
}


void CLackJetDlg::OnBnClickedButtonDelete()
{
	for (int c = 0; c < MAX_COLORS; c++) {
		for (int g = 0; g < MAX_GROUP; g++) {
			for (int i = 0; i < 32; i++) {
				g_sysParam.nLackJetTbl[c][g][i] = 0;
			}
			g_sysParam.nLackJetCount[c][g] = 0;
		}
	}
}
