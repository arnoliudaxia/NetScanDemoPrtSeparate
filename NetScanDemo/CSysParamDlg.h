#pragma once


// CSysParamDlg 对话框
#include "RyTreeListCtl.h"
class CSysParamDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSysParamDlg)

public:
	CSysParamDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSysParamDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SYS_PARAM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	float m_fCBDistance;
	float m_fCBWidth;
	float m_fCBInterval;
	float m_fCBInkDensity;
	int m_nPrtPeriod;
	int m_nCBValidMask;
	int m_nCBPosMode;
	BOOL m_bEquiDis;
public:
	void LoadOption();
	void InitFlashList();
	void ReDrawFlashList();
	afx_msg void OnBnClickedOk();
	int m_nMicroJetUnit;
	int m_nMicroJetCount;
	int m_nMicroStpJet;
	BOOL m_bEncReverse;
	BOOL m_bIdleFlash;
	BOOL m_BigInkDrops;
	CString m_szLogPath;
	int m_nColorSel;
	int m_nOverlapProc;
	float m_fFlashPrtFrequecy;
	float m_fBrustValidSec;
	float m_fBrustCycleSec;
	CSWListTreeCtrl m_FlashList;
	afx_msg void OnBnClickedButtonSet();
	BOOL m_bAll;
	BOOL m_bUseAdib;
};
