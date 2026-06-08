// CVTSetDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CVTSetDlg.h"
#include "afxdialogex.h"

// CVTSetDlg 对话框
extern RYUSR_PARAM g_sysParam;
int g_nPhTBL[MAX_PH_CNT];
extern BOOL g_bPHValid[MAX_PH_CNT];
extern UINT g_nPHType;
IMPLEMENT_DYNAMIC(CVTSetDlg, CDialogEx)

CVTSetDlg::CVTSetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VT_SET_DLG, pParent)
	, m_nPHID(0)
	, nTempIndex(0)
	, m_nVolIndex(0)
	, m_fSetVal(45)
	, m_fVolVal(20)
	, m_bTempAll(FALSE)
	, m_bVolAll(FALSE)
	, m_nSplit(1)
{

}

CVTSetDlg::~CVTSetDlg()
{
}

void CVTSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_PHID, m_nPHID);
	DDX_CBIndex(pDX, IDC_COMBO_TEMP_SEL, nTempIndex);
	DDX_CBIndex(pDX, IDC_COMBO_VOL_SEL, m_nVolIndex);
	DDX_Text(pDX, IDC_EDIT_TEMP_VAL, m_fSetVal);
	DDX_Text(pDX, IDC_EDIT_VOL_VAL, m_fVolVal);
	DDX_Check(pDX, IDC_CHECK_TEMP_ALL, m_bTempAll);
	DDX_Check(pDX, IDC_CHECK_VOL_ALL, m_bVolAll);
	DDX_Control(pDX, IDC_STATIC_VTLIST, m_VTSetList);
}


BEGIN_MESSAGE_MAP(CVTSetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_TEMPSET, &CVTSetDlg::OnBnClickedButtonTempset)
	ON_BN_CLICKED(IDC_BUTTON_VOLSET, &CVTSetDlg::OnBnClickedButtonVolset)
	ON_BN_CLICKED(IDOK, &CVTSetDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CVTSetDlg 消息处理程序


void CVTSetDlg::OnBnClickedButtonTempset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (!m_bTempAll) {
		g_sysParam.phctl_param[g_nPhTBL[m_nPHID]].fDestTemp[nTempIndex] = m_fSetVal;
	}
	else {
		for (int ph = 0; ph < MAX_PH_CNT; ph++) {
			for (int t = 0; t < m_nTempCount; t++) {
				g_sysParam.phctl_param[ph].fDestTemp[t] = m_fSetVal;
			}
		}
	}
	ReDrawVTList();
}


void CVTSetDlg::OnBnClickedButtonVolset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (!m_bVolAll) {
		if (m_nVolIndex == 0) {
			g_sysParam.phctl_param[g_nPhTBL[m_nPHID]].fStdVoltage = m_fVolVal;
		}
		else {
			g_sysParam.phctl_param[g_nPhTBL[m_nPHID]].fVoltage[m_nVolIndex-1] = m_fVolVal;
		}
	}
	else {
		for (int ph = 0; ph < MAX_PH_CNT; ph++) {
			for (int t = 0; t < m_nVolCount; t++) {
				for (int s = 0; s < m_nSplit; s++) {
					g_sysParam.phctl_param[ph].fVoltage[s* m_nVolCount + t] = m_fVolVal;
				}
			}
		}
	}
	ReDrawVTList();
}


void CVTSetDlg::InitVTList() {
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hPrtItem, hDrvItem;
	m_VTSetList.GetClientRect(&rect);
	int nUnitWidth = rect.Width() / (m_nTempCount + m_nVolCount + 3);
	m_VTSetList.InsertColumn(0, _T("喷头ID"), LVCFMT_CENTER, nUnitWidth * 2, 0);
	int nCurIndex = 0;
	CString szText;
	for (int i = 0; i < m_nTempCount; i++) {
		szText.Format(_T("温度T%d"), i);
		m_VTSetList.InsertColumn(i + 1, szText, LVCFMT_CENTER, nUnitWidth, i + 1);
	}
	szText.Format(_T("基准电压V%d"), m_nTempCount+1);
	m_VTSetList.InsertColumn(m_nTempCount + 1, szText, LVCFMT_CENTER, nUnitWidth, 1 + m_nTempCount);
	for (int i = 0; i < m_nVolCount; i++) {
		szText.Format(_T("校准电压V%d"), i);
		m_VTSetList.InsertColumn(i + m_nTempCount + 2, szText, LVCFMT_CENTER, nUnitWidth, i + 2 + m_nTempCount);
	}
	hRoot = m_VTSetList.m_childTree.InsertItem(_T("喷头ID"), 0, 0);
	for (int ph = 0; ph < MAX_PH_CNT; ph++) {
		for (int s = 0; s < m_nSplit; s++) {
			szTxt = "";
			if (s == 0)
				szTxt.Format(_T("喷头ID-%d"), ph + 1);
			hPrtItem = m_VTSetList.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
			szTxt = "";
			if (s == 0) {
				for (int i = 0; i < m_nTempCount; i++) {
					szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fDestTemp[i]);
					m_VTSetList.SetItemText(hPrtItem, i + 1, szText);
				}
			}
			nCurIndex = m_nTempCount + 1;
			szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fStdVoltage);
			m_VTSetList.SetItemText(hPrtItem, nCurIndex, szText);
			nCurIndex += 1;
			for (int i = 0; i < m_nVolCount; i++) {
				szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fVoltage[i+ s * m_nVolCount]);
				m_VTSetList.SetItemText(hPrtItem, nCurIndex + i, szText);
			}
		}
	}
	m_VTSetList.m_childTree.Expand(hRoot, TVE_EXPAND);
}


void CVTSetDlg::ReDrawVTList() {
	HTREEITEM hRootItem = m_VTSetList.GetRootItem();
	HTREEITEM hPHItem = m_VTSetList.GetChildItem(hRootItem);
	CString szText;
	for (int ph = 0; ph < MAX_PH_CNT; ph++) {
		for (int s = 0; s < m_nSplit; s++) {
			if (s == 0) {
				for (int t = 0; t < m_nTempCount; t++) {
					szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fDestTemp[t]);
					m_VTSetList.SetItemText(hPHItem, t + 1, szText);
				}
			}
			int nCurIndex = m_nTempCount + 1;
			szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fStdVoltage);
			m_VTSetList.SetItemText(hPHItem, nCurIndex, szText);
			nCurIndex += 1;
			for (int v = 0; v < m_nVolCount; v++) {
				szText.Format(_T("%.1f"), g_sysParam.phctl_param[ph].fVoltage[v + s * m_nVolCount]);
				m_VTSetList.SetItemText(hPHItem, nCurIndex + v, szText);
			}
			hPHItem = m_VTSetList.GetNextSiblingItem(hPHItem);
		}
	}
}
extern LPPRINTER_INFO g_pSysInfo;
BOOL CVTSetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString sz;
	m_nTempCount = g_pSysInfo->clrph_info[0].nTmpCtlCnt;
	m_nVolCount = g_pSysInfo->clrph_info[0].nVolCnt;
	m_nSplit = 1;
	if (g_pSysInfo->clrph_info[0].nPhType == 19) // Xaar
		m_nSplit = 4;
	int nCurSel = 0;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PHID);
	if (pComboBox) {
		for (int ph = 0; ph < MAX_PH_CNT; ph++) {
			if (g_bPHValid[ph]) {
				sz.Format(_T("PH-%d"), ph);
				pComboBox->AddString(sz);
				g_nPhTBL[nCurSel] = ph;
				nCurSel++;
			}
		}
		pComboBox->SetCurSel(0);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_TEMP_SEL);
	if (pComboBox) {
		for (int t = 0; t < m_nTempCount; t++) {
			sz.Format(_T("temp-%d"), t);
			pComboBox->AddString(sz);
		}
		pComboBox->SetCurSel(0);
		if (m_nTempCount == 0)
			pComboBox->EnableWindow(FALSE);
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_VOL_SEL);
	if (pComboBox) {
		pComboBox->AddString(_T("标准电压"));
		for (int v = 0; v < m_nVolCount; v++) {
			sz.Format(_T("vol-%d"), v);
			pComboBox->AddString(sz);
		}
		pComboBox->SetCurSel(0);
	}
	m_fSetVal = g_sysParam.phctl_param[0].fDestTemp[0];
	m_fVolVal = g_sysParam.phctl_param[0].fStdVoltage;
	UpdateData(FALSE);
	m_nVolCount /= m_nSplit;
	InitVTList();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CVTSetDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
