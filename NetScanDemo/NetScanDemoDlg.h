
// NetScanDemoDlg.h : header file
//

#pragma once

#define _MAX_LAYER_CNT 100
#include "RyTreeListCtl.h"
#include "PicScreen.h"
//#include<Vector>

// CNetScanDemoDlg dialog
class CNetScanDemoDlg : public CDialogEx
{
// Construction
public:
	CNetScanDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NETSCANDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	bool m_bJobStarted;
	bool m_bPrinting;
	int	m_nPassCount[_MAX_LAYER_CNT];	//保存每个图层计算下来的PASS数
	CSWListTreeCtrl m_ListPrtInfo;
	CSWListTreeCtrl m_ListDrvInfo;
	CString m_szAppPath;
	int m_nPrtRevColumn;
	int m_nPrtEncoderVal;
	BOOL m_bCloseAutoCheck;
	CPicScreen m_ImgPreView;
	CString m_szPrtFile;
	int m_nPrtDataSize;
	int m_nJobImgLayerCnts;
	BOOL m_bStopMonitor;
	BOOL m_bImageLoaded;
	LPBYTE m_pPrtData[MAX_COLORS];

	//std::Vector<LPBYTE>  charVector;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStartjob();
	afx_msg void OnBnClickedButtonLoadimg();
	static UINT PrintThread(LPVOID pvoid);
	static UINT PrintThreadSeparate(LPVOID pvoid);
	//static UINT WriteDataThread(LPVOID pvoid);		//添加图层的线程
	static UINT MonitorThread(LPVOID lpvoid);
	afx_msg void OnBnClickedButtonReloadwave();
	afx_msg void OnBnClickedButtonVtSet();
	afx_msg void OnBnClickedButtonCali();
	afx_msg void OnBnClickedButtonSysParam();
	afx_msg void OnBnClickedButtonFlash();
	afx_msg void OnBnClickedButtonResetEncoder();
	void UpdateButtonState();
	void UpdatePrtState(UINT nState);
	afx_msg void OnEnChangeMfcPrtFilePath();
	int GetSrcData(CString lpSrcFile);
	afx_msg void OnDestroy();
	void InitsysParam();
	void InitSystem();
	void InitMoveSys();
	BOOL InitDeviceInfo();
	void UpdateVTListInfo();
	afx_msg void OnBnClickedButtonMovctl();
	afx_msg void OnBnClickedButtonAirInk();
	afx_msg void OnBnClickedButtonAdibCtl();
	BOOL m_bEnableInkWacth;
	afx_msg void OnBnClickedCheckInkWatch();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedCheckCloseVtMonitor();
};
