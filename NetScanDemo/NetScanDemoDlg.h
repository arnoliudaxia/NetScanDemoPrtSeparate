
// NetScanDemoDlg.h : header file
//

#pragma once

#define _MAX_LAYER_CNT 100
#include "RyTreeListCtl.h"
#include "PicScreen.h"
#include <atlimage.h>
#include <afxmt.h>
#include <string>
#include <vector>

struct SocketPressInkRequest;

struct PrintLayerFolder
{
	CString szPath;
	int nColorCount;
	LONG nWidth;
	LONG nHeight;
	LONG nBytesPerLine;
	int nDataSize;
};

class CPngStaticCtrl : public CStatic
{
public:
	CPngStaticCtrl();
	BOOL LoadPngResource(UINT nResourceID);
	BOOL HasImage() const;

protected:
	CImage m_Image;
	UINT m_nLoadedResourceID;
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};

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
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CToolTipCtrl m_ToolTip;
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
	CPngStaticCtrl m_JobStateImage;
	CMFCEditBrowseCtrl m_PrtFolderBrowse;
	CString m_szPrtFile;
	int m_nPrtDataSize;
	int m_nJobImgLayerCnts;
	int m_nCurrentDataChannelCount;
	int m_nCurrentPassCount;
	int m_nSinglePassLayerIndex;
	int m_nSinglePassIndex;
	unsigned int m_nSinglePassInitialCols;
	CString m_szImageInfo2Base;
	BOOL m_bStopMonitor;
	volatile LONG m_lStopSocketServer;
	SOCKET m_hListenSocket;
	CWinThread* m_pSocketServerThread;
	BOOL m_bSocketInitialized;
	CCriticalSection m_SocketEventLock;
	std::vector<std::string> m_SocketEventQueue;
	SocketPressInkRequest* m_pSocketPressInkRequest;
	DWORD m_dwSocketPressInkStartTick;
	DWORD m_dwSocketPressInkDuration;
	BOOL m_bImageLoaded;
	LPBYTE m_pPrtData[MAX_COLORS];
	std::vector<PrintLayerFolder> m_PrintLayers;

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
	static UINT MonitorThread(LPVOID lpvoid);
	static UINT SocketServerThread(LPVOID lpvoid);
	BOOL StartSocketServer();
	void StopSocketServer();
	void QueueSocketEvent(const std::string& eventText);
	void NotifySocketStartEvent(int nTotalLayers);
	void NotifySocketLayerStartEvent(int nLayerNumber);
	void NotifySocketPassReadyEvent(int nCurrentPass, int nTotalPass,
		int nStepVector, BOOL bIsNullPass);
	void NotifySocketPrintJobCompletedEvent(int nTotalLayers);
	afx_msg void OnBnClickedButtonReloadwave();
	afx_msg void OnBnClickedButtonVtSet();
	afx_msg void OnBnClickedButtonCali();
	afx_msg void OnBnClickedButtonSysParam();
	afx_msg void OnBnClickedButtonFlash();
	int ExecuteFlashToggle(BOOL bShowError, BOOL* pBeforeState, BOOL* pAfterState);
	afx_msg void OnBnClickedButtonResetEncoder();
	void UpdateButtonState();
	void UpdatePrtState(const PrtRunInfo& prtInfo);
	void ResetSinglePassProgress();
	void UpdateSinglePassProgress(const PrtRunInfo& prtInfo);
	void UpdateImageInfo2();
	afx_msg void OnEnChangeMfcPrtFilePath();
	BOOL ScanPrintRoot(const CString& szRootPath, CString& szError);
	BOOL LoadLayerData(const PrintLayerFolder& layer, CString& szError);
	void FreeLayerData();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
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
	afx_msg void OnStnClickedStaticRipView();
	afx_msg LRESULT OnUpdatePassCount(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAppendProcessLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSocketFlashControl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSocketPressInkControl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintJobFinished(WPARAM wParam, LPARAM lParam);
	CString m_szProcessLog;
};
