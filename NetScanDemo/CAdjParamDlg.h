#pragma once


// CAdjParamDlg 对话框
#include "RyTreeListCtl.h"
#include "../inc/ryprtapi.h"
class CAdjParamDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAdjParamDlg)

public:
	CAdjParamDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAdjParamDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_ADJ };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	float m_nXGoffset;
	int m_nClrIndex;
	int m_nGGroupIndex;
	int m_nGDir;
	afx_msg void OnBnClickedButtonGroupOffset();
	int m_nGIClrIndex;
	int m_nGIGroupIndex;
	int m_nGIGDir;
	int m_nGIJetIndex;
	float m_nGIXoffset;
	BOOL m_bPrinting;
	BOOL m_bStop;
	afx_msg void OnBnClickedButtonGigroupOffset();
	CSWListTreeCtrl m_GoupList;
	CSWListTreeCtrl m_GIList;
	int m_nPassCount;
	void InitGroupList();
	void InitGIGroupList();
	void UpdateGroupList();
	void UpdateGIGroupList();
	afx_msg void OnCbnSelchangeComboGidir();
	afx_msg void OnCbnSelchangeComboGdir();
	float m_nBiDirEncPrtOff;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonLackJet();
	RYCalbrationParam m_calParam;
	afx_msg void OnBnClickedButtonPhstatus();
	float m_fCtlVAL;
	int nGrayBits;
	afx_msg void OnBnClickedButtonVtical();
	afx_msg void OnBnClickedButtonStep();
	afx_msg void OnBnClickedButtonXoffGroup();
	afx_msg void OnBnClickedButtonXoff();
	afx_msg void OnBnClickedButton6();
	static UINT PrtCaliBrationThread(LPVOID pvoid);
	afx_msg void OnBnClickedButtonStop();
};
