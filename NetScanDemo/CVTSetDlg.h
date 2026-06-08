#pragma once


// CVTSetDlg 对话框
#include "RyTreeListCtl.h"
class CVTSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVTSetDlg)

public:
	CVTSetDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CVTSetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VT_SET_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_nPHID;
	int m_nTempCount;
	int m_nVolCount;
	int m_nSplit;
	int nTempIndex;
	int m_nVolIndex;
	float m_fSetVal;
	float m_fVolVal;
	BOOL m_bTempAll;
	BOOL m_bVolAll;
	CSWListTreeCtrl m_VTSetList;
	afx_msg void OnBnClickedButtonTempset();
	afx_msg void OnBnClickedButtonVolset();
	virtual BOOL OnInitDialog();
	void InitVTList();
	void ReDrawVTList();

	afx_msg void OnBnClickedOk();
};
