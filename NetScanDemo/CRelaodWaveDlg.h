#pragma once


// CRelaodWaveDlg 对话框

class CReloadWaveDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CReloadWaveDlg)

public:
	CReloadWaveDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CReloadWaveDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_RELOAD_WAVE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_nColorIndex;
	int m_nGroupIndex;
	CString m_szWavePath;
	virtual BOOL OnInitDialog();
};
