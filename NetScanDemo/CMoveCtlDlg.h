#pragma once


// CMoveCtlDlg 对话框

class CMoveCtlDlg : public CDialog
{
	DECLARE_DYNAMIC(CMoveCtlDlg)

public:
	CMoveCtlDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMoveCtlDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MOVE_CTL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	float m_fSpd;
	float m_fXAccTime;
	float m_fXMoveRate;
	float m_fXMoveUnit;
	float m_fXSysDpi;
	float m_fYspd;
	float m_fYAccTime;
	float m_fYMoveRate;
	float m_fYMoveUnit;
	float m_fYSysDpi;
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonFront();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonStop();
	int nMachineType;
	afx_msg void OnBnClickedButtonApply();
	virtual BOOL OnInitDialog();
	BOOL m_bXReverse;
	BOOL m_bYReverse;
	float m_fMovBuf;
	afx_msg void OnEnChangeEditXmovrate();
};
