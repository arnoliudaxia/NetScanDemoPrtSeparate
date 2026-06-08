#pragma once


// CPrtJobDlg 对话框

class CPrtJobDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPrtJobDlg)

public:
	CPrtJobDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPrtJobDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PRT_JOB_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_nGrayBits;
	float m_fYOutDPI;
	float m_fXOutDPI;
	float m_fClipWidth;
	float m_fClipHeight;
	float m_fYPrtPos;
	float m_fXPrtPos;
	int m_nFileType;
	CString m_szJobFilePath;
	CString m_szJobName;
	int  m_nJobID;
	int  m_nImageLayerCount;
	BOOL m_bWhiteJump;
	BOOL m_bCycleOff;
	BOOL m_bRadomJetOff;
	BOOL m_bXMirror;
	BOOL m_bYMirror;
	BOOL m_bDoubleYDpi;
	BOOL m_bScreenshotPrt;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
