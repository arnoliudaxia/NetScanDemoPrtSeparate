#pragma once


// CAirInkDialog 对话框
#include "StaticBox.h"
class CAirInkDialog : public CDialog
{
	DECLARE_DYNAMIC(CAirInkDialog)

public:
	CAirInkDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAirInkDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_AIR_INK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	float m_fAirPressCtl[2];
	float m_fAirPressSafeCtl[2];
	float m_fInkTemp[2];
	CStaticBox m_InkState[5];
	BOOL m_bStopPushInk;
	BOOL m_bStartPushInk;
	DWORD m_dwTick;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSet();
	int m_nPushInkSec;
	afx_msg void OnBnClickedButtonPushInk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	BOOL m_bEnableSupply;
	BOOL m_bEnablePress;
	BOOL m_bEnableAutoPress;
	afx_msg void OnBnClickedCheckEnableSupply();
	afx_msg void OnBnClickedCheckEnablePress();
	BOOL m_bEnableOut1;
	BOOL m_bEnableOut2;
	BOOL m_bEnableOut3;
	BOOL m_bEnableOut4;
	BOOL m_bEnableOut5;
	BOOL m_bEnableOut6;
	afx_msg void OnBnClickedCheckOut1();
	afx_msg void OnBnClickedCheckOut2();
	afx_msg void OnBnClickedCheckOut3();
	afx_msg void OnBnClickedCheckOut4();
	afx_msg void OnBnClickedCheckOut5();
	afx_msg void OnBnClickedCheckOut6();
	afx_msg void OnBnClickedCheck2();
};
