#pragma once


// CAdibCtrlDlg 对话框
#include "InkStateCtrl.h"
#include "ImportListCtrl.h"
class CAdibCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdibCtrlDlg)

public:
	CAdibCtrlDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAdibCtrlDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_ADIBCTL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	void UpdateAdibInfo(BOOL bState);

	DECLARE_MESSAGE_MAP()
public:
	CInkStateCtrl m_SigLogicInput;
	CImportListCtrl m_AdibList;
	static UINT VTMonitorThread(LPVOID pvoid);
	CWinThread* m_pMonitorThread;
	BOOL	m_bMonitorRuning;
	BOOL	m_bStopMonitor;
	BOOL	m_bComState;
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg void OnBnCheckImport();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent); 
	float m_fAirHold;
	float  m_nLnkTime;
	float  m_fInkSupplyTime;
	BOOL m_bSetEnable;
	BOOL m_bCleanMode;
	BOOL m_bCleanPump1;
	BOOL m_bCleanHead;
	BOOL m_bCleanPump2;
	afx_msg void OnBnClickedButtonCleanmode();
	BOOL m_bIICRead;
	afx_msg void OnBnClickedButtonCleanPump2();
	afx_msg void OnBnClickedButtonCleanPump1();
	afx_msg void OnBnClickedLnkidok();
	afx_msg void OnBnClickedLnkidok2();
	afx_msg void OnBnClickedCleanheadidok();
	afx_msg void OnBnClickedCleanheadidisok();
	afx_msg void OnBnClickedLnkisok();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton2();
};
