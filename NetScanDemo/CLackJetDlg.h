#pragma once


// CLackJetDlg 对话框

class CLackJetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLackJetDlg)

public:
	CLackJetDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLackJetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_LACK_JET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int m_nColorIndex;
	int m_nGroupIndex;
	afx_msg void OnBnClickedButtonAdd();
	int m_nLackJet;
	afx_msg void OnBnClickedButtonDelete();
};
