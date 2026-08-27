
// NetScanDemoDlg.cpp : implementation file
//
#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <objidl.h>
#include <vector>
#include <string>
#include "NetScanDemo.h"
#include "NetScanDemoDlg.h"
#include "afxdialogex.h"
#include "../inc/RYPrtCtler.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Ole32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATE_PASS_COUNT      (WM_APP + 2)
#define WM_APPEND_PROCESS_LOG     (WM_APP + 3)
#define WM_SOCKET_FLASH_CONTROL   (WM_APP + 4)
#define WM_SOCKET_PRESS_INK_CONTROL (WM_APP + 5)
#define WM_PRINT_JOB_FINISHED     (WM_APP + 6)

static const UINT_PTR SOCKET_PRESS_INK_TIMER_ID = 0x1200;

BEGIN_MESSAGE_MAP(CPngStaticCtrl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CPngStaticCtrl::CPngStaticCtrl()
	: m_nLoadedResourceID(0)
{
}

BOOL CPngStaticCtrl::LoadPngResource(UINT nResourceID)
{
	if (m_nLoadedResourceID == nResourceID && !m_Image.IsNull())
		return TRUE;

	m_nLoadedResourceID = 0;
	if (!m_Image.IsNull())
		m_Image.Destroy();

	HINSTANCE hResourceInstance = AfxGetResourceHandle();
	HRSRC hResource = ::FindResource(
		hResourceInstance, MAKEINTRESOURCE(nResourceID), _T("PNG"));
	if (hResource == NULL)
		return FALSE;

	const DWORD nResourceSize = ::SizeofResource(hResourceInstance, hResource);
	HGLOBAL hLoadedResource = ::LoadResource(hResourceInstance, hResource);
	const void* pResourceData = hLoadedResource != NULL
		? ::LockResource(hLoadedResource)
		: NULL;
	if (pResourceData == NULL || nResourceSize == 0)
		return FALSE;

	HGLOBAL hResourceCopy = ::GlobalAlloc(GMEM_MOVEABLE, nResourceSize);
	if (hResourceCopy == NULL)
		return FALSE;

	void* pResourceCopy = ::GlobalLock(hResourceCopy);
	if (pResourceCopy == NULL)
	{
		::GlobalFree(hResourceCopy);
		return FALSE;
	}
	::CopyMemory(pResourceCopy, pResourceData, nResourceSize);
	::GlobalUnlock(hResourceCopy);

	IStream* pStream = NULL;
	HRESULT hResult = ::CreateStreamOnHGlobal(hResourceCopy, TRUE, &pStream);
	if (FAILED(hResult))
	{
		::GlobalFree(hResourceCopy);
		return FALSE;
	}

	hResult = m_Image.Load(pStream);
	pStream->Release();
	if (FAILED(hResult))
	{
		m_Image.Destroy();
		return FALSE;
	}

	m_nLoadedResourceID = nResourceID;
	Invalidate();
	return TRUE;
}

BOOL CPngStaticCtrl::HasImage() const
{
	return !m_Image.IsNull();
}

void CPngStaticCtrl::OnPaint()
{
	CPaintDC dc(this);
	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(&rcClient, ::GetSysColor(COLOR_BTNFACE));
	if (m_Image.IsNull() || rcClient.IsRectEmpty())
		return;

	const int nImageWidth = m_Image.GetWidth();
	const int nImageHeight = m_Image.GetHeight();
	if (nImageWidth <= 0 || nImageHeight <= 0)
		return;

	const double fScaleX = static_cast<double>(rcClient.Width()) / nImageWidth;
	const double fScaleY = static_cast<double>(rcClient.Height()) / nImageHeight;
	const double fScale = fScaleX < fScaleY ? fScaleX : fScaleY;
	const int nDrawWidth = static_cast<int>(nImageWidth * fScale + 0.5);
	const int nDrawHeight = static_cast<int>(nImageHeight * fScale + 0.5);
	const int nDrawX = (rcClient.Width() - nDrawWidth) / 2;
	const int nDrawY = (rcClient.Height() - nDrawHeight) / 2;
	m_Image.Draw(dc.GetSafeHdc(), nDrawX, nDrawY, nDrawWidth, nDrawHeight,
		0, 0, nImageWidth, nImageHeight);
}

extern RYUSR_PARAM g_sysParam;
extern MOV_Config g_movConfig;
PRTJOB_ITEM g_testJob;
PRTIMG_LAYER g_PrtImgLayer;
RYCalbrationParam g_Calbration;
BOOL g_bPHValid[MAX_PH_CNT] = { 0 };
// CAboutDlg dialog used for App About
LPPRINTER_INFO g_pSysInfo;
UINT g_nPHType = 0;
bool g_IsRoladWave = false;

static void WriteProcessLog(CNetScanDemoDlg* pDlg, LPCTSTR pszFormat, ...)
{
	if (pDlg == NULL || !::IsWindow(pDlg->GetSafeHwnd()))
		return;
	CString* pMessage = new CString;
	va_list args;
	va_start(args, pszFormat);
	pMessage->FormatV(pszFormat, args);
	va_end(args);
	if (!pDlg->PostMessage(WM_APPEND_PROCESS_LOG, 0, reinterpret_cast<LPARAM>(pMessage)))
		delete pMessage;
}

static const u_short SOCKET_SERVER_PORT = 19090;

static std::string FormatSocketReceivedData(const char* pData, int nLength);

struct SocketFlashResult
{
	BOOL bBeforeState;
	BOOL bAfterState;
	int nError;
};

struct SocketPressInkRequest
{
	double fSeconds;
	HANDLE hCompletedEvent;
	int nError;
};

BOOL CNetScanDemoDlg::StartSocketServer()
{
	if (m_pSocketServerThread != NULL)
		return TRUE;

	WSADATA wsaData = {};
	const int nWsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nWsaResult != 0)
	{
		WriteProcessLog(this, _T("Socket Server初始化失败：WSAStartup=%d"), nWsaResult);
		return FALSE;
	}
	m_bSocketInitialized = TRUE;
	InterlockedExchange(&m_lStopSocketServer, 0);
	{
		CSingleLock lock(&m_SocketEventLock, TRUE);
		m_SocketEventQueue.clear();
	}

	CWinThread* pThread = AfxBeginThread(SocketServerThread, this,
		THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (pThread == NULL)
	{
		InterlockedExchange(&m_lStopSocketServer, 1);
		WSACleanup();
		m_bSocketInitialized = FALSE;
		WriteProcessLog(this, _T("Socket Server线程创建失败"));
		return FALSE;
	}
	pThread->m_bAutoDelete = FALSE;
	m_pSocketServerThread = pThread;
	pThread->ResumeThread();
	return TRUE;
}

void CNetScanDemoDlg::QueueSocketEvent(const std::string& eventText)
{
	CSingleLock lock(&m_SocketEventLock, TRUE);
	m_SocketEventQueue.push_back(eventText);
}

void CNetScanDemoDlg::NotifySocketStartEvent(int nTotalLayers)
{
	char szEvent[64] = {};
	sprintf_s(szEvent, "EVENT START_JOB TOTAL_LAYERS=%d\r\n", nTotalLayers);
	QueueSocketEvent(szEvent);
}

void CNetScanDemoDlg::NotifySocketLayerStartEvent(int nLayerNumber)
{
	char szEvent[64] = {};
	sprintf_s(szEvent, "EVENT LAYER_START LAYER=%d\r\n", nLayerNumber);
	QueueSocketEvent(szEvent);
}

void CNetScanDemoDlg::NotifySocketPassReadyEvent(int nCurrentPass, int nTotalPass,
	int nStepVector, BOOL bIsNullPass)
{
	char szEvent[128] = {};
	sprintf_s(szEvent,
		"EVENT PASS_READY CURRENT=%d TOTAL=%d STEP=%d EMPTY=%d\r\n",
		nCurrentPass, nTotalPass, nStepVector, bIsNullPass ? 1 : 0);
	QueueSocketEvent(szEvent);
}

void CNetScanDemoDlg::NotifySocketPrintJobCompletedEvent(int nTotalLayers)
{
	char szEvent[80] = {};
	sprintf_s(szEvent,
		"EVENT PRINT_JOB_COMPLETED TOTAL_LAYERS=%d\r\n", nTotalLayers);
	QueueSocketEvent(szEvent);
}

void CNetScanDemoDlg::StopSocketServer()
{
	InterlockedExchange(&m_lStopSocketServer, 1);
	if (m_pSocketServerThread != NULL)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_pSocketServerThread->m_hThread, 3000);
		if (dwWaitResult == WAIT_TIMEOUT && m_hListenSocket != INVALID_SOCKET)
		{
			shutdown(m_hListenSocket, SD_BOTH);
			closesocket(m_hListenSocket);
			m_hListenSocket = INVALID_SOCKET;
			dwWaitResult = WaitForSingleObject(m_pSocketServerThread->m_hThread, 1000);
		}
		if (dwWaitResult == WAIT_OBJECT_0)
		{
			delete m_pSocketServerThread;
			m_pSocketServerThread = NULL;
		}
	}
	if (m_bSocketInitialized)
	{
		WSACleanup();
		m_bSocketInitialized = FALSE;
	}
}

UINT CNetScanDemoDlg::SocketServerThread(LPVOID lpvoid)
{
	CNetScanDemoDlg* pDlg = static_cast<CNetScanDemoDlg*>(lpvoid);
	if (pDlg == NULL)
		return 1;

	SOCKET hListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hListenSocket == INVALID_SOCKET)
	{
		WriteProcessLog(pDlg, _T("Socket Server创建监听Socket失败：%d"), WSAGetLastError());
		return 1;
	}
	pDlg->m_hListenSocket = hListenSocket;

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SOCKET_SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(hListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		WriteProcessLog(pDlg, _T("Socket Server绑定127.0.0.1:%u失败：%d"),
			SOCKET_SERVER_PORT, WSAGetLastError());
		closesocket(hListenSocket);
		pDlg->m_hListenSocket = INVALID_SOCKET;
		return 1;
	}
	if (listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		WriteProcessLog(pDlg, _T("Socket Server监听失败：%d"), WSAGetLastError());
		closesocket(hListenSocket);
		pDlg->m_hListenSocket = INVALID_SOCKET;
		return 1;
	}

	WriteProcessLog(pDlg, _T("Socket Server已启动：127.0.0.1:%u"), SOCKET_SERVER_PORT);
	std::vector<SOCKET> clientSockets;
	std::vector<std::string> receiveBuffers;
	while (InterlockedCompareExchange(&pDlg->m_lStopSocketServer, 0, 0) == 0)
	{
		std::vector<std::string> pendingEvents;
		{
			CSingleLock lock(&pDlg->m_SocketEventLock, TRUE);
			pendingEvents.swap(pDlg->m_SocketEventQueue);
		}
		for (size_t nEventIndex = 0; nEventIndex < pendingEvents.size(); ++nEventIndex)
		{
			const std::string& eventText = pendingEvents[nEventIndex];
			for (size_t i = 0; i < clientSockets.size();)
			{
				const int nSent = send(clientSockets[i], eventText.data(),
					static_cast<int>(eventText.size()), 0);
				if (nSent != static_cast<int>(eventText.size()))
				{
					shutdown(clientSockets[i], SD_BOTH);
					closesocket(clientSockets[i]);
					clientSockets.erase(clientSockets.begin() + i);
					receiveBuffers.erase(receiveBuffers.begin() + i);
					continue;
				}
				++i;
			}
			const std::string eventLog = FormatSocketReceivedData(
				eventText.data(), static_cast<int>(eventText.size()));
			const CString szEventLog(eventLog.c_str());
			WriteProcessLog(pDlg, _T("Socket已广播事件：%s，客户端数=%u"),
				static_cast<LPCTSTR>(szEventLog),
				static_cast<unsigned int>(clientSockets.size()));
		}

		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(hListenSocket, &readSet);
		for (size_t i = 0; i < clientSockets.size(); ++i)
			FD_SET(clientSockets[i], &readSet);

		timeval timeout = {};
		timeout.tv_usec = 200000;
		const int nSelectResult = select(0, &readSet, NULL, NULL, &timeout);
		if (nSelectResult == SOCKET_ERROR)
		{
			if (InterlockedCompareExchange(&pDlg->m_lStopSocketServer, 0, 0) == 0)
				WriteProcessLog(pDlg, _T("Socket Server select失败：%d"), WSAGetLastError());
			break;
		}
		if (nSelectResult == 0)
			continue;

		if (FD_ISSET(hListenSocket, &readSet))
		{
			sockaddr_in clientAddr = {};
			int nAddrLength = sizeof(clientAddr);
			SOCKET hClientSocket = accept(hListenSocket,
				reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLength);
			if (hClientSocket != INVALID_SOCKET)
			{
				if (clientSockets.size() >= FD_SETSIZE - 1)
				{
					WriteProcessLog(pDlg, _T("Socket客户端连接被拒绝：已达到最大连接数"));
					closesocket(hClientSocket);
				}
				else
				{
					clientSockets.push_back(hClientSocket);
					receiveBuffers.push_back(std::string());
					const char szReady[] = "NETSCAN_SERVER_READY\r\n";
					send(hClientSocket, szReady, sizeof(szReady) - 1, 0);
					WriteProcessLog(pDlg, _T("Socket客户端已连接：127.0.0.1:%u，当前连接数=%u"),
						ntohs(clientAddr.sin_port), static_cast<unsigned int>(clientSockets.size()));
				}
			}
		}

		for (size_t i = 0; i < clientSockets.size();)
		{
			SOCKET hClientSocket = clientSockets[i];
			if (!FD_ISSET(hClientSocket, &readSet))
			{
				++i;
				continue;
			}
			char szBuffer[1024];
			const int nReceived = recv(hClientSocket, szBuffer, sizeof(szBuffer), 0);
			if (nReceived <= 0)
			{
				closesocket(hClientSocket);
				clientSockets.erase(clientSockets.begin() + i);
				receiveBuffers.erase(receiveBuffers.begin() + i);
				WriteProcessLog(pDlg, _T("Socket客户端已断开，当前连接数=%u"),
					static_cast<unsigned int>(clientSockets.size()));
				continue;
			}
			const std::string receivedLog = FormatSocketReceivedData(szBuffer, nReceived);
			const CString szReceivedLog(receivedLog.c_str());
			WriteProcessLog(pDlg, _T("Socket收到数据：%d字节，内容：%s"),
				nReceived, static_cast<LPCTSTR>(szReceivedLog));
			receiveBuffers[i].append(szBuffer, nReceived);
			if (receiveBuffers[i].size() > 4096)
			{
				const char szReply[] = "ERR COMMAND_TOO_LONG\r\n";
				send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
				receiveBuffers[i].clear();
				WriteProcessLog(pDlg, _T("Socket命令过长，已丢弃"));
			}
			else
			{
				size_t nLineEnd = std::string::npos;
				while ((nLineEnd = receiveBuffers[i].find('\n')) != std::string::npos)
				{
					std::string command = receiveBuffers[i].substr(0, nLineEnd);
					receiveBuffers[i].erase(0, nLineEnd + 1);
					while (!command.empty() &&
						(command[command.size() - 1] == '\r' || command[command.size() - 1] == ' ' ||
						 command[command.size() - 1] == '\t'))
					{
						command.erase(command.size() - 1);
					}
					if (_stricmp(command.c_str(), "FLASH") == 0)
					{
						if (!::IsWindow(pDlg->GetSafeHwnd()))
						{
							const char szReply[] = "ERR SERVER_STOPPING\r\n";
							send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
						}
						else
						{
							SocketFlashResult flashResult = {};
							pDlg->SendMessage(WM_SOCKET_FLASH_CONTROL, 0,
								reinterpret_cast<LPARAM>(&flashResult));
							char szReply[128] = {};
							if (flashResult.nError == 0)
							{
								sprintf_s(szReply, "OK FLASH BEFORE=%s AFTER=%s\r\n",
									flashResult.bBeforeState ? "ON" : "OFF",
									flashResult.bAfterState ? "ON" : "OFF");
							}
							else if (flashResult.nError == 1)
							{
								sprintf_s(szReply,
									"ERR FLASH GET_STATE_FAILED BEFORE=UNKNOWN AFTER=UNKNOWN\r\n");
							}
							else
							{
								sprintf_s(szReply, "ERR FLASH CONTROL_FAILED BEFORE=%s AFTER=%s\r\n",
									flashResult.bBeforeState ? "ON" : "OFF",
									flashResult.bAfterState ? "ON" : "OFF");
							}
							send(hClientSocket, szReply, static_cast<int>(strlen(szReply)), 0);
						}
					}
					else if (_strnicmp(command.c_str(), "PRESS_INK", 9) == 0 &&
						(command.size() == 9 || command[9] == ' ' || command[9] == '\t'))
					{
						double fSeconds = 0.0;
						char chExtra = 0;
						const int nFields = sscanf_s(command.c_str(), "PRESS_INK %lf %c",
							&fSeconds, &chExtra, 1);
						if (nFields != 1 || !(fSeconds > 0.0) || fSeconds > 3600.0)
						{
							const char szReply[] = "ERR PRESS_INK INVALID_TIME RANGE=(0,3600]\r\n";
							send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
							WriteProcessLog(pDlg, _T("Socket压墨命令参数无效：%S"), command.c_str());
						}
						else if (!::IsWindow(pDlg->GetSafeHwnd()))
						{
							const char szReply[] = "ERR SERVER_STOPPING\r\n";
							send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
						}
						else
						{
							SocketPressInkRequest request = {};
							request.fSeconds = fSeconds;
							request.nError = 0;
							request.hCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
							if (request.hCompletedEvent == NULL)
							{
								const char szReply[] = "ERR PRESS_INK INTERNAL_ERROR\r\n";
								send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
							}
							else
							{
								WriteProcessLog(pDlg, _T("Socket收到压墨命令：%.3f秒"), fSeconds);
								pDlg->SendMessage(WM_SOCKET_PRESS_INK_CONTROL, 0,
									reinterpret_cast<LPARAM>(&request));
								if (request.nError == 0)
								{
									const DWORD dwWaitMilliseconds =
										static_cast<DWORD>(fSeconds * 1000.0) + 5000;
									if (WaitForSingleObject(request.hCompletedEvent,
										dwWaitMilliseconds) != WAIT_OBJECT_0)
									{
										request.nError = 5;
										if (::IsWindow(pDlg->GetSafeHwnd()))
											pDlg->SendMessage(WM_SOCKET_PRESS_INK_CONTROL, 1,
												reinterpret_cast<LPARAM>(&request));
									}
								}

								char szReply[160] = {};
								if (request.nError == 0)
									sprintf_s(szReply,
										"OK PRESS_INK SECONDS=%.3f RESULT=COMPLETED STATE=OFF\r\n",
										fSeconds);
								else if (request.nError == 2)
									sprintf_s(szReply, "ERR PRESS_INK BUSY\r\n");
								else if (request.nError == 3)
									sprintf_s(szReply, "ERR PRESS_INK START_FAILED STATE=OFF\r\n");
								else if (request.nError == 4)
									sprintf_s(szReply, "ERR PRESS_INK STOP_FAILED STATE=UNKNOWN\r\n");
								else
									sprintf_s(szReply, "ERR PRESS_INK TIMEOUT STATE=OFF\r\n");
								send(hClientSocket, szReply, static_cast<int>(strlen(szReply)), 0);
								CloseHandle(request.hCompletedEvent);
							}
						}
					}
					else if (!command.empty())
					{
						const char szReply[] = "ERR UNKNOWN_COMMAND\r\n";
						send(hClientSocket, szReply, sizeof(szReply) - 1, 0);
						WriteProcessLog(pDlg, _T("Socket收到未知命令"));
					}
				}
			}
			++i;
		}
	}

	for (size_t i = 0; i < clientSockets.size(); ++i)
	{
		shutdown(clientSockets[i], SD_BOTH);
		closesocket(clientSockets[i]);
	}
	clientSockets.clear();
	receiveBuffers.clear();
	if (pDlg->m_hListenSocket == hListenSocket)
	{
		closesocket(hListenSocket);
		pDlg->m_hListenSocket = INVALID_SOCKET;
	}
	WriteProcessLog(pDlg, _T("Socket Server已停止"));
	return 0;
}
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNetScanDemoDlg dialog

void InitImgLayerInfo()
{
	//////////////////////层信息
	ZeroMemory(&g_PrtImgLayer, sizeof(PRTIMG_LAYER));
	g_PrtImgLayer.nLayerIndex = -1;	//保持递增关系
	g_PrtImgLayer.nXDPI = 720;
	g_PrtImgLayer.nYDPI = 600;
	g_PrtImgLayer.nColorCnts = 1;
	g_PrtImgLayer.nGrayBits=1;
	g_PrtImgLayer.nPrtDir = 1;
	g_PrtImgLayer.nValidClrMask = 0xFFFF;
	g_PrtImgLayer.fLayerDensity = 1;
	g_PrtImgLayer.nPrtFlag = 1;	//双向打印 bit[0] 控制单双向打印 bit1 Y反向打印

}

void InitJobInfo()
{
	//////////////////////Job信息
	ZeroMemory(&g_testJob, sizeof(PRTJOB_ITEM));
	g_testJob.nJobID = 0;
	CopyMemory(g_testJob.szJobName, "Scan Demo", 9);
	g_testJob.fPrtXPos = 100;
	g_testJob.fClipHeight = 0;
	g_testJob.fClipWidth = 0;
	g_testJob.fOutXdpi = 600;
	g_testJob.fOutYdpi = 600;
	g_testJob.nFileType = 0;
	g_testJob.nOutPixelBits = 1;
	
}

void InitCalibrationInfo()
{
	ZeroMemory(&g_Calbration, sizeof(PRTJOB_ITEM));
	g_Calbration.fPrtXPos = 10;
	g_Calbration.fPrtYPos = 0;
	g_Calbration.fStpSize = 0;
	g_Calbration.fxadjdpi = 600;
	g_Calbration.fXMaxPrtWidth = 600;
	g_Calbration.fXRunSpd = 10;
	g_Calbration.fYMaxPrtHeight = 600;
	g_Calbration.nAdjType = 0;
	g_Calbration.nCtrlValue = 0;
	g_Calbration.nGrayBits = 1;
	g_Calbration.nPrtDir = 1;
	
}

CNetScanDemoDlg::CNetScanDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNetScanDemoDlg::IDD, pParent)
	, m_bCloseAutoCheck(FALSE)
	, m_szPrtFile(_T(""))
	, m_bEnableInkWacth(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bJobStarted = false;
	m_bPrinting = false;
	m_bStopMonitor = false;
	m_lStopSocketServer = 1;
	m_hListenSocket = INVALID_SOCKET;
	m_pSocketServerThread = NULL;
	m_bSocketInitialized = FALSE;
	m_pSocketPressInkRequest = NULL;
	m_dwSocketPressInkStartTick = 0;
	m_dwSocketPressInkDuration = 0;
	m_nPrtRevColumn = 0;
	m_nPrtEncoderVal = 0;
	m_nJobImgLayerCnts=1;
	m_nCurrentDataChannelCount = 0;
	m_nCurrentPassCount = -1;
	m_nSinglePassLayerIndex = -1;
	m_nSinglePassIndex = -1;
	m_nSinglePassInitialCols = 0;
	m_bImageLoaded=FALSE;
	for(int c=0;c<MAX_COLORS;c++)
		m_pPrtData[c]=NULL;

}

void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PRT_INFO, m_ListPrtInfo);
	DDX_Control(pDX, IDC_STATIC_VT_INFO, m_ListDrvInfo);
	DDX_Check(pDX, IDC_CHECK_CLOSE_VT_MONITOR, m_bCloseAutoCheck);
	DDX_Control(pDX, IDC_STATIC_RIP_VIEW, m_ImgPreView);
	DDX_Control(pDX, IDC_STATIC_JOB_STATE_IMAGE, m_JobStateImage);
	DDX_Control(pDX, IDC_MFC_PRT_FILE_PATH, m_PrtFolderBrowse);
	DDX_Text(pDX, IDC_MFC_PRT_FILE_PATH, m_szPrtFile);
	DDX_Check(pDX, IDC_CHECK_INK_WATCH, m_bEnableInkWacth);
}

BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
	ON_BN_CLICKED(IDC_BUTTON_LOADIMG, &CNetScanDemoDlg::OnBnClickedButtonLoadimg)
	ON_BN_CLICKED(IDC_BUTTON_RELOADWAVE, &CNetScanDemoDlg::OnBnClickedButtonReloadwave)
	ON_BN_CLICKED(IDC_BUTTON_VT_SET, &CNetScanDemoDlg::OnBnClickedButtonVtSet)
	ON_BN_CLICKED(IDC_BUTTON_CALI, &CNetScanDemoDlg::OnBnClickedButtonCali)
	ON_BN_CLICKED(IDC_BUTTON_SYS_PARAM, &CNetScanDemoDlg::OnBnClickedButtonSysParam)
	ON_BN_CLICKED(IDC_BUTTON_FLASH, &CNetScanDemoDlg::OnBnClickedButtonFlash)
	ON_BN_CLICKED(IDC_BUTTON_RESET_ENCODER, &CNetScanDemoDlg::OnBnClickedButtonResetEncoder)
	ON_EN_CHANGE(IDC_MFC_PRT_FILE_PATH, &CNetScanDemoDlg::OnEnChangeMfcPrtFilePath)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MOVCTL, &CNetScanDemoDlg::OnBnClickedButtonMovctl)
	ON_BN_CLICKED(IDC_BUTTON_AIR_INK, &CNetScanDemoDlg::OnBnClickedButtonAirInk)
	ON_BN_CLICKED(IDC_BUTTON_ADIB_CTL, &CNetScanDemoDlg::OnBnClickedButtonAdibCtl)
	ON_BN_CLICKED(IDC_CHECK_INK_WATCH, &CNetScanDemoDlg::OnBnClickedCheckInkWatch)
	ON_BN_CLICKED(IDC_BUTTON1, &CNetScanDemoDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CNetScanDemoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CNetScanDemoDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_CHECK_CLOSE_VT_MONITOR, &CNetScanDemoDlg::OnBnClickedCheckCloseVtMonitor)
	ON_STN_CLICKED(IDC_STATIC_RIP_VIEW, &CNetScanDemoDlg::OnStnClickedStaticRipView)
	ON_MESSAGE(WM_UPDATE_PASS_COUNT, &CNetScanDemoDlg::OnUpdatePassCount)
	ON_MESSAGE(WM_APPEND_PROCESS_LOG, &CNetScanDemoDlg::OnAppendProcessLog)
	ON_MESSAGE(WM_SOCKET_FLASH_CONTROL, &CNetScanDemoDlg::OnSocketFlashControl)
	ON_MESSAGE(WM_SOCKET_PRESS_INK_CONTROL, &CNetScanDemoDlg::OnSocketPressInkControl)
	ON_MESSAGE(WM_PRINT_JOB_FINISHED, &CNetScanDemoDlg::OnPrintJobFinished)
END_MESSAGE_MAP()


// CNetScanDemoDlg message handlers


void CNetScanDemoDlg::InitsysParam() {
	ZeroMemory(&g_sysParam, sizeof(RYUSR_PARAM));
}



void CNetScanDemoDlg::UpdateVTListInfo() {
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hPrtItem, hDrvItem;
	int nTmpCnt = g_pSysInfo->clrph_info[0].nTmpQueryCnt;
	int nVolCnt = g_pSysInfo->clrph_info[0].nVolQueryCnt;
	int nSplit = 1;
	if (g_pSysInfo->clrph_info[0].nPhType == 19) // XAAR2001
		nSplit = 4;
    nVolCnt /= nSplit;
	if(g_pSysInfo->clrph_info[0].nPhType == 28)//s3200
	{
		nVolCnt =2;
	}
	
	int nCurIndex = 1;
	hRoot = m_ListDrvInfo.m_childTree.GetRootItem();
	hPrtItem = m_ListDrvInfo.m_childTree.GetChildItem(hRoot);
	for (int p = 0; p < MAX_MCP_CNT; p++) {
		if (g_pSysInfo->nMcpValidMask & (1 << p)) {
			hDrvItem = m_ListDrvInfo.m_childTree.GetChildItem(hPrtItem);
			for (int d = 0; d < MCP_DRV_CNT; d++) {
				if (g_pSysInfo->nMcpDrvLinkMask[p] & (1 << d)) {
					for (int s = 0; s < nSplit; s++) {
						for (int t = 0; t < nTmpCnt; t++) {
							szTxt.Format(_T("%.1f"), g_pSysInfo->sysDrvInfo[p][d].phinfo[0].fCurTemp[t]);
							m_ListDrvInfo.SetItemText(hDrvItem, nCurIndex, szTxt);
							nCurIndex++;
						}
						for (int v = 0; v < nVolCnt; v++) {
							szTxt.Format(_T("%.1f"), g_pSysInfo->sysDrvInfo[p][d].phinfo[0].fCurVol[s*nVolCnt + v]);
							m_ListDrvInfo.SetItemText(hDrvItem, nCurIndex, szTxt);
							nCurIndex++;
						}
						nCurIndex = 1;
						hDrvItem = m_ListDrvInfo.m_childTree.GetNextSiblingItem(hDrvItem);
					}
				}
			}
			hPrtItem = m_ListDrvInfo.m_childTree.GetNextSiblingItem(hPrtItem);
		}
	}

}

BOOL CNetScanDemoDlg::InitDeviceInfo()
{
	if (g_pSysInfo == NULL)
		return FALSE;
	int nTmpCount = g_pSysInfo->clrph_info[0].nTmpQueryCnt;
	int nVolCount = g_pSysInfo->clrph_info[0].nVolQueryCnt;
	int nSplit = 1;
	if (g_pSysInfo->clrph_info[0].nPhType == 19)
		nSplit = 4;

	if (g_pSysInfo->clrph_info[0].nPhType == 28)
	{
		nVolCount = 2;
	}
	CRect rect; CString szTxt, szItem;
	HTREEITEM hRoot, hPrtItem, hDrvItem;
	m_ListPrtInfo.GetClientRect(&rect);
	int nUnitWidth = rect.Width() / 8;
	m_ListPrtInfo.InsertColumn(0, _T("网络板卡"), LVCFMT_CENTER, nUnitWidth * 2, 0);
	m_ListPrtInfo.InsertColumn(1, _T("主卡"), LVCFMT_CENTER, nUnitWidth, 1);
	m_ListPrtInfo.InsertColumn(2, _T("设备号"), LVCFMT_CENTER, nUnitWidth, 2);
	m_ListPrtInfo.InsertColumn(3, _T("FPGA版本"), LVCFMT_CENTER, nUnitWidth, 3);
	m_ListPrtInfo.InsertColumn(4, _T("MCU版本"), LVCFMT_CENTER, nUnitWidth, 4);
	m_ListPrtInfo.InsertColumn(5, _T("Appmain版本"), LVCFMT_CENTER, nUnitWidth, 5);
	m_ListPrtInfo.InsertColumn(6, _T("连接掩码"), LVCFMT_CENTER, nUnitWidth, 6);
	int  nDrvIndex;
	CImageList m_imgBtns;
	hRoot = m_ListPrtInfo.m_childTree.InsertItem(_T("网络板"), 0, 0);
	for (int p = 0; p < MAX_MCP_CNT; p++) {
		if (g_pSysInfo->nMcpValidMask & (1 << p)) {
			szTxt = "";
			szItem.Format(_T("网络卡%d"), p + 1);
			szTxt += szItem;
			hPrtItem = m_ListPrtInfo.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
			if (g_pSysInfo->nMaiMcpIndex == p) {
				szTxt.Format(_T("是"));
				m_ListPrtInfo.SetItemText(hPrtItem, 1, szTxt);
			}
			else {
				szTxt.Format(_T("否"));
				m_ListPrtInfo.SetItemText(hPrtItem, 1, szTxt);
			}
			szTxt.Format(_T("%08X"), g_pSysInfo->nMcpDeviceID[p]);
			m_ListPrtInfo.SetItemText(hPrtItem, 2, szTxt);
			szTxt.Format(_T("%08X"), g_pSysInfo->nMcpFpgaVer[p]);
			m_ListPrtInfo.SetItemText(hPrtItem, 3, szTxt);
			szTxt.Format(_T("%08X"), g_pSysInfo->nMcpMcuVersion[p]);
			m_ListPrtInfo.SetItemText(hPrtItem, 4, szTxt);
			szTxt.Format(_T("%08X"), g_pSysInfo->nMcpAppVersion[p]);
			m_ListPrtInfo.SetItemText(hPrtItem, 5, szTxt);
			szTxt.Format(_T("%08X"), g_pSysInfo->nMcpDrvLinkMask[p]);
			m_ListPrtInfo.SetItemText(hPrtItem, 6, szTxt);
			m_ListPrtInfo.m_childTree.Expand(hPrtItem, TVE_EXPAND);
		}
	}
	m_ListPrtInfo.m_childTree.Expand(hRoot, TVE_EXPAND);
	m_ListDrvInfo.GetClientRect(&rect);
	nVolCount /= nSplit;
	nUnitWidth = rect.Width() / (nTmpCount + nVolCount + 2 + 2);
	m_ListDrvInfo.InsertColumn(0, _T("网络板卡"), LVCFMT_CENTER, nUnitWidth * 2, 0);
	CString szText;
	for (int i = 0; i < nTmpCount; i++) {
		szText.Format(_T("温度T%d"), i);
		m_ListDrvInfo.InsertColumn(i + 1, szText, LVCFMT_CENTER, nUnitWidth, i + 1);
	}
	for (int i = 0; i < nVolCount; i++) {
		szText.Format(_T("电压V%d"), i);
		m_ListDrvInfo.InsertColumn(i + nTmpCount + 1, szText, LVCFMT_CENTER, nUnitWidth, i + 1 + nTmpCount);
	}
	int nCurIndex = nTmpCount + nVolCount + 1;
	m_ListDrvInfo.InsertColumn(nCurIndex, _T("fpga版本"), LVCFMT_CENTER, nUnitWidth, nCurIndex);
	m_ListDrvInfo.InsertColumn(nCurIndex + 1, _T("mcu版本"), LVCFMT_CENTER, nUnitWidth, nCurIndex + 1);
	hRoot = m_ListDrvInfo.m_childTree.InsertItem(_T("网络板"), 0, 0);
	for (int p = 0; p < MAX_MCP_CNT; p++) {
		if (g_pSysInfo->nMcpValidMask & (1 << p)) {
			szTxt = "";
			szItem.Format(_T("网络卡%d"), p + 1);
			szTxt += szItem;
			hPrtItem = m_ListDrvInfo.m_childTree.InsertItem(szTxt, 1, 1, hRoot, TVI_LAST);
			for (int d = 0; d < MCP_DRV_CNT; d++) {
				if (g_pSysInfo->nMcpDrvLinkMask[p] & (1 << d)) {
					for (int s = 0; s < nSplit; s++) {
						szTxt = "";
						if (s == 0)
							szTxt.Format(_T("驱动卡%d"), d + 1);
						hDrvItem = m_ListDrvInfo.m_childTree.InsertItem(szTxt, 1, 1, hPrtItem, TVI_LAST);
						for (int i = 0; i < nTmpCount; i++) {
							szText.Format(_T("%.1f"), g_pSysInfo->sysDrvInfo[p][d].phinfo[0].fCurTemp[i]);
							m_ListDrvInfo.SetItemText(hDrvItem, i + 1, szText);
						}
						nCurIndex = nTmpCount + 1;
						for (int i = 0; i < nVolCount; i++) {
							szText.Format(_T("%.1f"), g_pSysInfo->sysDrvInfo[p][d].phinfo[0].fCurVol[s * nVolCount + i]);
							m_ListDrvInfo.SetItemText(hDrvItem, nCurIndex + i, szText);
						}
						nCurIndex += nVolCount;
						szText.Format(_T("%x"), g_pSysInfo->sysDrvInfo[p][d].nFpgaVersion);
						m_ListDrvInfo.SetItemText(hDrvItem, nCurIndex, szText);
						nCurIndex += 1;
						szText.Format(_T("%x"), g_pSysInfo->sysDrvInfo[p][d].nFMVersion);
						m_ListDrvInfo.SetItemText(hDrvItem, nCurIndex, szText);
					}
					m_ListDrvInfo.m_childTree.Expand(hDrvItem, TVE_EXPAND);
				}
			}
			m_ListDrvInfo.m_childTree.Expand(hPrtItem, TVE_EXPAND);
		}
	}
	m_ListDrvInfo.m_childTree.Expand(hRoot, TVE_EXPAND);
	CWnd* pFilePath = GetDlgItem(IDC_MFC_PRT_FILE_PATH);
	if (pFilePath && m_ToolTip.Create(this, TTS_ALWAYSTIP))
	{
		m_ToolTip.AddTool(pFilePath,
			_T("请选择根文件夹。子文件夹按1、2、3...命名；每层图片按1.bmp、2.bmp、3.bmp...命名"));
		m_ToolTip.Activate(TRUE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CNetScanDemoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_ToolTip.GetSafeHwnd())
	{
		m_ToolTip.RelayEvent(pMsg);
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CNetScanDemoDlg::InitMoveSys() {
#ifdef RY_MOVE_CTL
	CString szErr;
	int nResult = DEM_Initialize(m_szAppPath.GetBuffer(0));
	if (nResult != 0) {
		szErr.Format(_T("[%d] >> 连接运动卡失败"), nResult);
	}
#endif // RY_MOVE_CTL

}
void CNetScanDemoDlg::InitSystem()
{
	EnableWindow(FALSE);
	CString szErrInfo;
	///执行设备连接
	USES_CONVERSION;
	int nResult=0;
	clock_t EndTime;
	clock_t StartTime;
	g_IsRoladWave = true;
	//CString AppmianRuteTemp = m_szAppPath  + "appmian";
	//char* AppmianRute = (char*)(AppmianRuteTemp.GetBuffer(0));
	//CString AppmianRute = m_szAppPath  + "appmian";
	DEV_OpenDevice(m_hWnd, (unsigned char*)(m_szAppPath.GetBuffer(0)));	//(unsigned char*)(LPCTSTR)m_szAppPath
	StartTime = clock();
	while (!DEV_DeviceIsConnected())
	{
		Sleep(500);
		nResult = DEV_DeviceIsConnected();
		EndTime = clock();
	//	break;
		if(EndTime - StartTime > 1000)
		{
			AfxMessageBox("板卡超时未连接");
			break;
		}
	}
		
	
	//初始化参数，需要在 DEV_InitDevice 之前执行		
	DEV_UpdateParam(&g_sysParam);
	//if(!DEV_UpdateParam(&g_sysParam))
	//{
		//AfxMessageBox("DEV_UpdateParam error");
	//}
	//初始化设备
    nResult = DEV_InitDevice(ZERO_POSITION);	//包含加载波形
	if (nResult < 0)
	{
		szErrInfo.Format(_T("[%d] >> 初始化设备失败"), nResult);
		AfxMessageBox(szErrInfo);
	}
	////获取设备当前状态信息，用来判断初始化状态
	g_pSysInfo = DEV_GetDeviceInfo();
	for (int c = 0; c < g_pSysInfo->nSysColors; c++) {
		for (int g = 0; g < MAX_GROUP; g++) {
			g_bPHValid[g_pSysInfo->nPhIDLKT[c][g]] = TRUE;
		}
	}
	for (int p = 0; p < MAX_MCP_CNT; p++) {
		for (int d = 0; d < MCP_DRV_CNT; d++) {
			if (g_pSysInfo->sysDrvInfo[p][d].nState == 1) {
				g_nPHType = g_pSysInfo->sysDrvInfo[p][d].phinfo[0].nPhType;
			}
		}
	}
	InitDeviceInfo();
	if(nResult>0){
	    AfxMessageBox(_T("初始化成功"));
	}
	AfxBeginThread(MonitorThread, this);
	//SetTimer(0x1000, 300, NULL);

	EnableWindow(TRUE);

}
typedef    BOOL(CALLBACK* LPRY_EXTFUN_RFIDOpen)(int);
LPRY_EXTFUN_RFIDOpen          RY_RFIDOpen = NULL;
BOOL CNetScanDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_JobStateImage.LoadPngResource(IDB_PNG1);
	m_JobStateImage.ShowWindow(SW_HIDE);
	SetDlgItemText(IDC_STATIC_ImageInfo3, _T("PASS数量: --"));
	SetDlgItemText(IDC_STATIC_ImageInfo4, _T(""));
	SendDlgItemMessage(IDC_PROGRESS_singlePass, PBM_SETRANGE32, 0, 100);
	ResetSinglePassProgress();
	m_szImageInfo2Base = _T("请选择包含 1\\1.bmp 的打印根文件夹");
	m_nCurrentPassCount = -1;
	UpdateImageInfo2();
	SetDlgItemText(IDC_STATIC_REMAIN_COLS,
		_T("作业打印运行中: --    当前图层打印完成: --    剩余列数: --\r\n")
		_T("当前打印图层: --    当前图层PASS总数: --    当前打印PASS: --\r\n")
		_T("当前数据处理图层: --    打印数据总层数: --"));
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	CNetScanDemoApp* pApp = (CNetScanDemoApp*)AfxGetApp();
	if (pApp->LoadParam(&g_sysParam) < 0)
		InitsysParam();
	if (pApp->LoadJobParam() < 0)
		InitJobInfo();
	if (pApp->LoadLayerParam() <0)
		InitImgLayerInfo();
	m_PrtFolderBrowse.EnableFolderBrowseButton();

	if(pApp->LoadCalibrationParam() < 0)
	{
		InitCalibrationInfo();
	}
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
#ifdef RY_MOVE_CTL
	InitMoveSys();
#endif
	InitSystem();
	WriteProcessLog(this, _T("程序初始化完成，等待打印任务"));
	StartSocketServer();
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNetScanDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNetScanDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNetScanDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CNetScanDemoDlg::UpdatePrtState(const PrtRunInfo& prtInfo)
{
	//	CWnd* pWnd=(CWnd*)GetDlgItem(IDC_BTN_PRINT);
	UpdateSinglePassProgress(prtInfo);
	CString szTxt;
	CString szRunInfo;
	CString szTotalLayers = _T("--");
	CString szPassInfo = _T("PASS数量: --");
	if (!m_PrintLayers.empty())
	{
		szTotalLayers.Format(_T("%d"), m_nJobImgLayerCnts);
	}
	if (m_nCurrentPassCount >= 0)
		szPassInfo.Format(_T("PASS数量: %d"), m_nCurrentPassCount);
	SetDlgItemText(IDC_STATIC_ImageInfo3, szPassInfo);
	UpdateImageInfo2();
	szRunInfo.Format(
		_T("作业打印运行中: %s    当前图层打印完成: %s    剩余列数: %u\r\n")
		_T("当前打印图层: %d    当前图层PASS总数: %d    当前打印PASS: %d\r\n")
		_T("当前数据处理图层: %d    打印数据总层数: %s"),
		prtInfo.bJobPrtRuning ? _T("是") : _T("否"),
		prtInfo.bLayerPrtIsOver ? _T("是") : _T("否"),
		prtInfo.nRevPrtCols,
		prtInfo.nPrintLayerIndex,
		prtInfo.nLayerPassCount,
		prtInfo.nPrintPassIndex,
		prtInfo.nProcLayerIndex,
		static_cast<LPCTSTR>(szTotalLayers));
	SetDlgItemText(IDC_STATIC_REMAIN_COLS, szRunInfo);
	const UINT nJobStateImageResource = prtInfo.bJobPrtRuning ? IDB_PNG1 : IDB_PNG2;
	const BOOL bJobStateImageLoaded =
		m_JobStateImage.LoadPngResource(nJobStateImageResource);
	m_JobStateImage.ShowWindow(bJobStateImageLoaded ? SW_SHOW : SW_HIDE);
	switch (prtInfo.nPrtState)
	{
	case 0:
		szTxt.Format(_T("Royal 3DP Demo 当前编码位置：%08X"), m_nPrtEncoderVal);
		break;
	case 1:
		szTxt.Format(_T("打印 当前位置编码:%08X"), m_nPrtEncoderVal);
		break;
	case 2:
		szTxt.Format(_T("暂停 当前位置编码:%08X"), m_nPrtEncoderVal);
		break;
	case 3:
		szTxt = _T("闪喷");
		break;
	case 4:
		szTxt.Format(_T("Royal 3DP Demo 当前编码位置：%08X"), m_nPrtEncoderVal);
		break;
	}
	SetWindowText(szTxt);
}

void CNetScanDemoDlg::ResetSinglePassProgress()
{
	m_nSinglePassLayerIndex = -1;
	m_nSinglePassIndex = -1;
	m_nSinglePassInitialCols = 0;
	if (GetDlgItem(IDC_PROGRESS_singlePass) != NULL)
		SendDlgItemMessage(IDC_PROGRESS_singlePass, PBM_SETPOS, 0, 0);
}

void CNetScanDemoDlg::UpdateSinglePassProgress(const PrtRunInfo& prtInfo)
{
	if (!prtInfo.bJobPrtRuning ||
		prtInfo.nPrintLayerIndex < 0 || prtInfo.nPrintPassIndex < 0)
	{
		ResetSinglePassProgress();
		return;
	}

	const BOOL bNewPass =
		m_nSinglePassLayerIndex != prtInfo.nPrintLayerIndex ||
		m_nSinglePassIndex != prtInfo.nPrintPassIndex;
	if (bNewPass)
	{
		m_nSinglePassLayerIndex = prtInfo.nPrintLayerIndex;
		m_nSinglePassIndex = prtInfo.nPrintPassIndex;
		m_nSinglePassInitialCols = prtInfo.nRevPrtCols;
	}
	else
	{
		if (prtInfo.nRevPrtCols > m_nSinglePassInitialCols)
		{
			// PASS 刚切换时 SDK 可能先上报 0，随后才上报完整剩余列数。
			m_nSinglePassInitialCols = prtInfo.nRevPrtCols;
		}
	}

	int nRemainingPercent = 0;
	if (m_nSinglePassInitialCols > 0)
	{
		const unsigned int nRemainingCols = min(
			prtInfo.nRevPrtCols, m_nSinglePassInitialCols);
		nRemainingPercent = static_cast<int>(
			(static_cast<ULONGLONG>(nRemainingCols) * 100ULL) /
			m_nSinglePassInitialCols);
	}
	SendDlgItemMessage(
		IDC_PROGRESS_singlePass, PBM_SETPOS, nRemainingPercent, 0);
}

UINT CNetScanDemoDlg::MonitorThread(LPVOID lpvoid)
{
	PrtRunInfo prtInfo = {};

	bool tetetttt = true;
	UINT  nLastPrtState = 0xFF;
	DWORD dwTick = GetTickCount();
	DWORD dwChkTick = GetTickCount();
	//bool bstatus = false;
	CNetScanDemoDlg* pCtlDlg = (CNetScanDemoDlg*)lpvoid;
	while (!pCtlDlg->m_bStopMonitor)
	{
		//tetetttt = DEV_DeviceIsConnected();
		//if(tetetttt)
		//{
		//	AfxMessageBox("11111111");
		//}else{
		//	AfxMessageBox("2222222");
		//}
		//test start
		//g_pSysInfo = DEV_GetDeviceInfo();
		//DEV_GetPrinterEncValue();
		//test end
		 DEV_DeviceIsConnected();
		if (!IDP_GetPrintState(&prtInfo))
		{
			WriteProcessLog(pCtlDlg, _T("IDP_GetPrintState失败，本次状态不更新"));
			Sleep(100);
			continue;
		}
		if (prtInfo.nPrtState == 0 || prtInfo.nPrtState == 3)
		{
			if ((GetTickCount() - dwChkTick) > 200)
			{
				if (!pCtlDlg->m_bCloseAutoCheck)
				{
					g_pSysInfo = DEV_GetDeviceInfo();

					if(17 == g_pSysInfo->nHWInitResult && g_IsRoladWave)
					{
						AfxMessageBox("波形文件失败");
						g_IsRoladWave = false;
					}

					if (g_pSysInfo)
						pCtlDlg->UpdateVTListInfo();
					
					//pCtlDlg->ReDraw();
				}
				dwChkTick = GetTickCount();
			}
		}
		pCtlDlg->m_nPrtRevColumn = prtInfo.nRevPrtCols;
		pCtlDlg->m_nPrtEncoderVal = DEV_GetPrinterEncValue();
		if (1)
		{
			pCtlDlg->UpdatePrtState(prtInfo);
			nLastPrtState = prtInfo.nPrtState;
		}
		Sleep(100);
	}
	AfxEndThread(0, TRUE);
	return 0;
}
#include "CPrtJobDlg.h"
// 启动打印按钮
void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
	WriteProcessLog(this, _T("按钮操作：%s"),
		m_bJobStarted ? _T("停止任务") : _T("启动任务"));	

	if (!m_bJobStarted)
	{
		UpdateData(TRUE);
		if (g_PrtImgLayer.nImgType != 0)
		{
			AfxMessageBox(_T("文件夹分层打印仅支持 BMP 图片类型，请先在图层设置中选择 BMP。"));
			return;
		}

		CString szErr;
		if (!ScanPrintRoot(m_szPrtFile, szErr))
		{
			AfxMessageBox(szErr);
			return;
		}

		m_nJobImgLayerCnts = static_cast<int>(m_PrintLayers.size());
		WriteProcessLog(this, _T("文件夹检查完成：图层数=%d"), m_nJobImgLayerCnts);
		UpdateImageInfo2();

		CPrtJobDlg dlg;
		dlg.m_nImageLayerCount=m_nJobImgLayerCnts;
		if (dlg.DoModal() == IDOK) 
		{
			ResetSinglePassProgress();
			m_bJobStarted = true;
			m_bImageLoaded = FALSE;
			m_PrtFolderBrowse.EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_LOADIMG)->EnableWindow(true);
			GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("停止任务"));
			g_PrtImgLayer.nLayerIndex = -1;
			WriteProcessLog(this, _T("启动顺序打印线程：图层数=%d"), m_nJobImgLayerCnts);
			CWinThread* pPrintThread = AfxBeginThread(PrintThread, this,
				THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
			if (pPrintThread == NULL)
			{
				m_bJobStarted = false;
				UpdateButtonState();
				AfxMessageBox(_T("创建打印线程失败。"));
				return;
			}
			NotifySocketStartEvent(m_nJobImgLayerCnts);
			pPrintThread->ResumeThread();
			CNetScanDemoApp* pApp = (CNetScanDemoApp*)AfxGetApp();
			if (pApp) {
				pApp->SaveJobParam();
			}
		}
	}
	else 
	{
		WriteProcessLog(this, _T("用户请求停止任务"));
		IDP_StopPrintJob();
		m_bJobStarted = false;
		ResetSinglePassProgress();
		GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("启动任务"));
	}
}

static BOOL ParsePositiveSequenceName(const CString& szName, int& nValue)
{
	if (szName.IsEmpty() || (szName.GetLength() > 1 && szName[0] == _T('0')))
		return FALSE;
	for (int i = 0; i < szName.GetLength(); ++i)
	{
		if (szName[i] < _T('0') || szName[i] > _T('9'))
			return FALSE;
	}

	const __int64 nParsed = _ttoi64(szName);
	if (nParsed < 1 || nParsed > (std::numeric_limits<int>::max)())
		return FALSE;
	nValue = static_cast<int>(nParsed);
	return TRUE;
}

static BOOL ReadBmpMetadata(const CString& szFilePath, LONG& nWidth, LONG& nHeight,
	LONG& nBytesPerLine, int& nDataSize, CString& szError)
{
	CFile file;
	if (!file.Open(szFilePath, CFile::modeRead | CFile::shareDenyNone))
	{
		szError.Format(_T("无法打开图片：\r\n%s"), static_cast<LPCTSTR>(szFilePath));
		return FALSE;
	}

	BITMAPFILEHEADER fileHeader = {};
	BITMAPINFOHEADER infoHeader = {};
	if (file.Read(&fileHeader, sizeof(fileHeader)) != sizeof(fileHeader) ||
		file.Read(&infoHeader, sizeof(infoHeader)) != sizeof(infoHeader) ||
		fileHeader.bfType != 0x4D42 || infoHeader.biSize < sizeof(BITMAPINFOHEADER))
	{
		szError.Format(_T("BMP 文件头无效：\r\n%s"), static_cast<LPCTSTR>(szFilePath));
		return FALSE;
	}
	if (infoHeader.biBitCount != 1 || infoHeader.biCompression != BI_RGB)
	{
		szError.Format(_T("仅支持未压缩的 1-bit BMP：\r\n%s"), static_cast<LPCTSTR>(szFilePath));
		return FALSE;
	}
	if (infoHeader.biWidth <= 0 || infoHeader.biHeight <= 0)
	{
		szError.Format(_T("BMP 宽度和高度必须为正数：\r\n%s"), static_cast<LPCTSTR>(szFilePath));
		return FALSE;
	}

	const ULONGLONG nLineBytes =
		(static_cast<ULONGLONG>(infoHeader.biWidth) + 31ULL) / 32ULL * 4ULL;
	const ULONGLONG nImageBytes = nLineBytes * static_cast<ULONGLONG>(infoHeader.biHeight);
	const ULONGLONG nFileLength = file.GetLength();
	const ULONGLONG nPaletteEnd = sizeof(BITMAPFILEHEADER) +
		static_cast<ULONGLONG>(infoHeader.biSize) + 2ULL * sizeof(RGBQUAD);
	if (nLineBytes > static_cast<ULONGLONG>((std::numeric_limits<LONG>::max)()) ||
		nImageBytes > static_cast<ULONGLONG>((std::numeric_limits<int>::max)()) ||
		fileHeader.bfOffBits < nPaletteEnd ||
		static_cast<ULONGLONG>(fileHeader.bfOffBits) + nImageBytes > nFileLength)
	{
		szError.Format(_T("BMP 像素数据长度无效或文件不完整：\r\n%s"),
			static_cast<LPCTSTR>(szFilePath));
		return FALSE;
	}

	nWidth = infoHeader.biWidth;
	nHeight = infoHeader.biHeight;
	nBytesPerLine = static_cast<LONG>(nLineBytes);
	nDataSize = static_cast<int>(nImageBytes);
	return TRUE;
}

static BOOL InspectLayerFolder(const CString& szFolderPath, int nLayerNumber,
	PrintLayerFolder& layer, CString& szError)
{
	std::vector<int> bmpNumbers;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(szFolderPath + _T("\\*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || finder.IsDirectory())
			continue;
		const CString szName = finder.GetFileName();
		if (szName.GetLength() < 5 || szName.Right(4).CompareNoCase(_T(".bmp")) != 0)
			continue;

		int nBmpNumber = 0;
		if (!ParsePositiveSequenceName(szName.Left(szName.GetLength() - 4), nBmpNumber))
		{
			szError.Format(_T("第 %d 层包含未按正整数命名的 BMP：\r\n%s"),
				nLayerNumber, static_cast<LPCTSTR>(finder.GetFilePath()));
			return FALSE;
		}
		bmpNumbers.push_back(nBmpNumber);
	}
	finder.Close();

	if (bmpNumbers.empty())
	{
		szError.Format(_T("第 %d 层没有找到 1.bmp。\r\n目录：%s"),
			nLayerNumber, static_cast<LPCTSTR>(szFolderPath));
		return FALSE;
	}
	std::sort(bmpNumbers.begin(), bmpNumbers.end());
	if (bmpNumbers.size() > MAX_COLORS)
	{
		szError.Format(_T("第 %d 层有 %d 个颜色通道，超过 SDK 上限 %d。"),
			nLayerNumber, static_cast<int>(bmpNumbers.size()), MAX_COLORS);
		return FALSE;
	}
	for (size_t i = 0; i < bmpNumbers.size(); ++i)
	{
		if (bmpNumbers[i] != static_cast<int>(i) + 1)
		{
			szError.Format(_T("第 %d 层的 BMP 序号不连续，应从 1.bmp 开始依次递增。"),
				nLayerNumber);
			return FALSE;
		}
	}

	layer.szPath = szFolderPath;
	layer.nColorCount = static_cast<int>(bmpNumbers.size());
	layer.nWidth = 0;
	layer.nHeight = 0;
	layer.nBytesPerLine = 0;
	layer.nDataSize = 0;
	for (int i = 0; i < layer.nColorCount; ++i)
	{
		CString szBmpPath;
		szBmpPath.Format(_T("%s\\%d.bmp"), static_cast<LPCTSTR>(szFolderPath), i + 1);
		LONG nWidth = 0;
		LONG nHeight = 0;
		LONG nBytesPerLine = 0;
		int nDataSize = 0;
		if (!ReadBmpMetadata(szBmpPath, nWidth, nHeight, nBytesPerLine, nDataSize, szError))
			return FALSE;

		if (i == 0)
		{
			layer.nWidth = nWidth;
			layer.nHeight = nHeight;
			layer.nBytesPerLine = nBytesPerLine;
			layer.nDataSize = nDataSize;
		}
		else if (nWidth != layer.nWidth || nHeight != layer.nHeight)
		{
			szError.Format(
				_T("第 %d 层图片尺寸不一致。\r\n1.bmp：%ld x %ld\r\n%d.bmp：%ld x %ld"),
				nLayerNumber, layer.nWidth, layer.nHeight,
				i + 1, nWidth, nHeight);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CNetScanDemoDlg::ScanPrintRoot(const CString& szRootPath, CString& szError)
{
	m_PrintLayers.clear();
	CString szRoot = szRootPath;
	szRoot.Trim();
	while (szRoot.GetLength() > 3 &&
		(szRoot.Right(1) == _T("\\") || szRoot.Right(1) == _T("/")))
		szRoot = szRoot.Left(szRoot.GetLength() - 1);

	const DWORD dwAttributes = GetFileAttributes(szRoot);
	if (szRoot.IsEmpty() || dwAttributes == INVALID_FILE_ATTRIBUTES ||
		!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		szError = _T("请选择有效的打印根文件夹。");
		return FALSE;
	}

	std::vector<int> folderNumbers;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(szRoot + _T("\\*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || !finder.IsDirectory())
			continue;
		int nFolderNumber = 0;
		if (ParsePositiveSequenceName(finder.GetFileName(), nFolderNumber))
			folderNumbers.push_back(nFolderNumber);
	}
	finder.Close();

	if (folderNumbers.empty())
	{
		szError = _T("根文件夹中没有找到编号为 1 的图层子文件夹。");
		return FALSE;
	}
	std::sort(folderNumbers.begin(), folderNumbers.end());
	for (size_t i = 0; i < folderNumbers.size(); ++i)
	{
		if (folderNumbers[i] != static_cast<int>(i) + 1)
		{
			szError = _T("图层子文件夹序号不连续，应从 1 开始按 1、2、3...依次递增。");
			return FALSE;
		}
	}

	for (size_t i = 0; i < folderNumbers.size(); ++i)
	{
		CString szLayerPath;
		szLayerPath.Format(_T("%s\\%d"), static_cast<LPCTSTR>(szRoot), folderNumbers[i]);
		PrintLayerFolder layer = {};
		if (!InspectLayerFolder(szLayerPath, folderNumbers[i], layer, szError))
		{
			m_PrintLayers.clear();
			return FALSE;
		}
		m_PrintLayers.push_back(layer);
	}
	m_nCurrentDataChannelCount = m_PrintLayers.front().nColorCount;
	return TRUE;
}

void CNetScanDemoDlg::FreeLayerData()
{
	for (int i = 0; i < MAX_COLORS; ++i)
	{
		delete[] m_pPrtData[i];
		m_pPrtData[i] = NULL;
	}
	m_bImageLoaded = FALSE;
}

BOOL CNetScanDemoDlg::LoadLayerData(const PrintLayerFolder& layer, CString& szError)
{
	FreeLayerData();
	for (int i = 0; i < layer.nColorCount; ++i)
	{
		CString szBmpPath;
		szBmpPath.Format(_T("%s\\%d.bmp"), static_cast<LPCTSTR>(layer.szPath), i + 1);
		LONG nWidth = 0;
		LONG nHeight = 0;
		LONG nBytesPerLine = 0;
		int nDataSize = 0;
		if (!ReadBmpMetadata(szBmpPath, nWidth, nHeight, nBytesPerLine, nDataSize, szError))
		{
			FreeLayerData();
			return FALSE;
		}
		if (nWidth != layer.nWidth || nHeight != layer.nHeight ||
			nBytesPerLine != layer.nBytesPerLine || nDataSize != layer.nDataSize)
		{
			szError.Format(_T("打印前检测到图片尺寸发生变化：\r\n%s"),
				static_cast<LPCTSTR>(szBmpPath));
			FreeLayerData();
			return FALSE;
		}
		CFile file;
		if (!file.Open(szBmpPath, CFile::modeRead | CFile::shareDenyNone))
		{
			szError.Format(_T("打印前无法重新打开图片：\r\n%s"), static_cast<LPCTSTR>(szBmpPath));
			FreeLayerData();
			return FALSE;
		}

		BITMAPFILEHEADER fileHeader = {};
		BITMAPINFOHEADER infoHeader = {};
		RGBQUAD zeroColor = {};
		if (file.Read(&fileHeader, sizeof(fileHeader)) != sizeof(fileHeader) ||
			file.Read(&infoHeader, sizeof(infoHeader)) != sizeof(infoHeader))
		{
			szError.Format(_T("打印前读取 BMP 文件头失败：\r\n%s"), static_cast<LPCTSTR>(szBmpPath));
			FreeLayerData();
			return FALSE;
		}
		file.Seek(sizeof(BITMAPFILEHEADER) + infoHeader.biSize, CFile::begin);
		if (file.Read(&zeroColor, sizeof(zeroColor)) != sizeof(zeroColor))
		{
			szError.Format(_T("打印前读取 BMP 调色板失败：\r\n%s"), static_cast<LPCTSTR>(szBmpPath));
			FreeLayerData();
			return FALSE;
		}

		m_pPrtData[i] = new BYTE[layer.nDataSize];
		file.Seek(fileHeader.bfOffBits, CFile::begin);
		if (file.Read(m_pPrtData[i], layer.nDataSize) != static_cast<UINT>(layer.nDataSize))
		{
			szError.Format(_T("打印前读取 BMP 像素数据失败：\r\n%s"), static_cast<LPCTSTR>(szBmpPath));
			FreeLayerData();
			return FALSE;
		}

		if (RGB(zeroColor.rgbRed, zeroColor.rgbGreen, zeroColor.rgbBlue) == 0)
		{
			for (int n = 0; n < layer.nDataSize; ++n)
				m_pPrtData[i][n] = ~m_pPrtData[i][n];
		}
	}

	m_nPrtDataSize = layer.nDataSize;
	m_nCurrentDataChannelCount = layer.nColorCount;
	g_PrtImgLayer.nColorCnts = layer.nColorCount;
	g_PrtImgLayer.nBytesPerLine = layer.nBytesPerLine;
	g_PrtImgLayer.nHeight = layer.nHeight;
	g_PrtImgLayer.nWidth = layer.nWidth / g_PrtImgLayer.nGrayBits;
	// 有效颜色掩码对应设备颜色编号，并不等同于当前 BMP 数据通道数量。
	// 保持 SDK 示例使用的全颜色掩码，由 nColorCnts 决定传入的数据通道数。
	g_PrtImgLayer.nValidClrMask = 0xFFFF;
	m_bImageLoaded = TRUE;
	return TRUE;
}
#include "CImgLayerSetDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonLoadimg()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CString szErr;
	int nResult = 0;
	CImgLayerSetDlg dlg;
	dlg.DoModal();
	CNetScanDemoApp* pApp = (CNetScanDemoApp*)AfxGetApp();
	if (pApp) {
		pApp->SaveLayerParam();
	}
	/*
	if (dlg.DoModal() == IDOK) 
	{
	}*/
}
//
//WriteDataThread
/********************************************
打印执行的处理
**********************************************/


bool teststtt = false;
using namespace std;

static LPCTSTR GetPassProcStateText(unsigned int nProcState)
{
	switch (nProcState)
	{
	case 0:
		return _T("数据处理阶段");
	case 1:
		return _T("源数据处理就绪");
	case 2:
		return _T("数据行列转换结束");
	case 3:
		return _T("数据已写入硬件");
	case 4:
		return _T("已添加到打印队列");
	case 5:
		return _T("打印结束");
	case 6:
		return _T("打印内存已释放");
	case 7:
		return _T("错误状态");
	default:
		return _T("未知状态");
	}
}


#define DP0(fmt) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt));OutputDebugString(sOut);} 
UINT  CNetScanDemoDlg::PrintThread(LPVOID pvoid)
{
	CNetScanDemoDlg* pdlg = (CNetScanDemoDlg*)pvoid;
	PrtRunInfo	RTinfo;CString szErr;int nPassCount;
	pdlg->m_bPrinting = true;
	LPPassDataItem pPrtPassDes = NULL;
	clock_t Start;
	clock_t End;
	char buf[1000] = {0};
	double cpu_time_used = 0;
	int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
	int nDestPos = 0;
	int ndistance = 0;
	int nSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
	clock_t Start4;
	clock_t Start5;
	BOOL bAllLayersCompleted = TRUE;
	WriteProcessLog(pdlg, _T("调用 IDP_SartPrintJob"));
	const int nStartJobResult = IDP_SartPrintJob(&g_testJob);
	WriteProcessLog(pdlg, _T("IDP_SartPrintJob 返回=%d"), nStartJobResult);
	if (nStartJobResult < 0)
	{
		szErr.Format(_T("启动任务失败，错误号为：%d"), nStartJobResult);
		AfxMessageBox(szErr);
		pdlg->m_bPrinting = false;
		pdlg->m_bJobStarted = false;
		pdlg->PostMessage(WM_PRINT_JOB_FINISHED, 0, 0);
		return -1;
	}

	for(int n=0;n<pdlg->m_nJobImgLayerCnts;n++)
	{
		if (!pdlg->m_bJobStarted)
		{
			bAllLayersCompleted = FALSE;
			break;
		}

		const PrintLayerFolder& layer = pdlg->m_PrintLayers[n];
		pdlg->PostMessage(
			WM_UPDATE_PASS_COUNT,
			static_cast<WPARAM>(static_cast<INT_PTR>(-1)),
			0);
		WriteProcessLog(pdlg, _T("开始读取第%d层：%s"),
			n + 1, static_cast<LPCTSTR>(layer.szPath));
		if (!pdlg->LoadLayerData(layer, szErr))
		{
			WriteProcessLog(pdlg, _T("第%d层读取失败：%s"), n + 1, static_cast<LPCTSTR>(szErr));
			AfxMessageBox(szErr);
			bAllLayersCompleted = FALSE;
			break;
		}

		LPBYTE pBmpFile[MAX_COLORS] = {};
		for (int i = 0; i < layer.nColorCount; ++i)
			pBmpFile[i] = pdlg->m_pPrtData[i];
		g_PrtImgLayer.nLayerIndex = n;
		WriteProcessLog(pdlg,
			_T("写入第%d层：颜色通道=%d，有效颜色掩码=0x%04X，尺寸=%ldx%ld，数据=%d字节/通道"),
			n + 1, layer.nColorCount, g_PrtImgLayer.nValidClrMask,
			layer.nWidth, layer.nHeight, layer.nDataSize);
		const int nWriteResult = IDP_WriteImgLayerData(
			&g_PrtImgLayer, pBmpFile, pdlg->m_nPrtDataSize);
		WriteProcessLog(pdlg, _T("IDP_WriteImgLayerData(图层%d) 返回=%d"), n, nWriteResult);
		if (nWriteResult != 1)
		{
			szErr.Format(_T("写入第 %d 层失败，错误码：%d"), n + 1, nWriteResult);
			AfxMessageBox(szErr);
			pdlg->FreeLayerData();
			bAllLayersCompleted = FALSE;
			break;
		}

		/*
		int  IDP_StartLayerPrint(int nLayerIndex);
		返回值定义：
		>0  返回PASS总数
		-1   图层数据未就绪
		-120000~-120016  初始化失败
		-121000                   板卡内存分区不足
		-122000                   启动数据分发线程失败
		*/
		WriteProcessLog(pdlg, _T("图层%d数据已写入，开始计算PASS"), n);
		nPassCount = IDP_StartLayerPrint(n);
		WriteProcessLog(pdlg, _T("IDP_StartLayerPrint(图层%d) 首次返回=%d"), n, nPassCount);

		int nLayerReadyWait = 0;
		while(nPassCount == -1 && pdlg->m_bJobStarted)
		{
			Sleep(1);
			nPassCount = IDP_StartLayerPrint(n);
			if (++nLayerReadyWait % 1000 == 0)
				WriteProcessLog(pdlg, _T("图层%d仍未就绪，等待%d秒"), n, nLayerReadyWait / 1000);
		}
		if (!pdlg->m_bJobStarted)
		{
			IDP_FreeImageLayer(n);
			pdlg->FreeLayerData();
			bAllLayersCompleted = FALSE;
			break;
		}
		WriteProcessLog(pdlg, _T("图层%d PASS计算结束，返回=%d"), n, nPassCount);
		pdlg->PostMessage(
			WM_UPDATE_PASS_COUNT,
			static_cast<WPARAM>(static_cast<INT_PTR>(nPassCount)),
			0);

		if(nPassCount>0)
		{
			pdlg->NotifySocketLayerStartEvent(n + 1);
			int Lentest = 0;
			for(int i=0;i<nPassCount;i++)
			{
				//Sleep(1000);
				Start4 = clock();
				//::OutputDebugString(_T("[RYZ] start IDP_GetPassItem3333333333333333333"));
				do
				{
					//::OutputDebugString(_T("[RY] Start Query Pass!"));
					pPrtPassDes=::IDP_GetPassItem(n,i);
					//memset(buf,0,sizeof(buf));
					//sprintf(buf,"%s %d","[RY] End Query Pass!  ",pPrtPassDes->nProcState);
					//::OutputDebugString(_T("[RY] End Query Pass!"));
					if(pPrtPassDes)
					{
						if((pPrtPassDes->nProcState>2)&&(pPrtPassDes->nProcState<6))
						{
							//3,4,5状态都可以执行打印
							break;
						}
						if(!pdlg->m_bJobStarted)
						{
							//作业停止
							pPrtPassDes=NULL;
							break;
						}
					 }
					Start5 = clock();

					if(pPrtPassDes && Start5 - Start4 >  60 * 1000)
					{
						sprintf(buf,"%s %d  %d  %d","pPrtPassDes->nProcState ",pPrtPassDes->nProcState,pPrtPassDes->nLayerIndex,pPrtPassDes->nLayerPassIndex);
						AfxMessageBox(buf);
					}
				}
				while(1);//等待PASS就绪
				if (!pdlg->m_bJobStarted || !pPrtPassDes)
					break;
				//::OutputDebugString(_T("[RYZ] end IDP_GetPassItem3333333333333333333"));
				WriteProcessLog(pdlg, _T("PASS %d/%d就绪：状态=%s，步进=%d um，空=%d"),
					i + 1, nPassCount, GetPassProcStateText(pPrtPassDes->nProcState),
					pPrtPassDes->nStpVector, pPrtPassDes->bIsNullPass ? 1 : 0);
				pdlg->NotifySocketPassReadyEvent(i + 1, nPassCount,
					pPrtPassDes->nStpVector, pPrtPassDes->bIsNullPass);
				if (pPrtPassDes->bIsNullPass)
					continue;
				if(pPrtPassDes)
				{
					if(0==i)
					{
						//第一PASS Y位置定位，
						//pPrtPassDes->fYMinJetImgPos 为Y起始打印位置里Y零位的距离
						//MOV_Y(pPrtPassDes->fYMinJetImgPos);
					}
					else
					{
						//打印之前需要的Y步进量
                        //MOV_Y（pPrtPassDes->nStpVector ）//um
#ifdef RY_MOVE_CTL
						int nYStep = MM_TO_DOT(pPrtPassDes->nStpVector/1000.0, g_movConfig.fySysdpi) * 48;
						int nSpd = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
						DEM_MoveY(TRUE,nSpd,nYStep,true,false,g_movConfig.fyMovRate);
#endif

					}
					//X打印起始位置
					//pPrtPassDes->nStartEncPos
					//打印的运动起点计算
					//正向打印的运动起点(PrtPassDes->bEncPrtDir:true)： nXPos=pPrtPassDes->nStartEncPos-MovBuf(运动缓冲)
#ifdef RY_MOVE_CTL
					if (pPrtPassDes->bEncPrtDir) {
						nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION - nMovBuf;
						ndistance = nDestPos - DEV_GetPrinterEncValue();
					}
					else {
						nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION + nMovBuf;
						ndistance = nDestPos - DEV_GetPrinterEncValue();
					}
					DEM_MoveX(FALSE,nSpd,ndistance,TRUE,g_movConfig.fxMovRate);
					//反向打印的运动起点(PrtPassDes->bEncPrtDir:false)： nXPos=pPrtPassDes->nStartEncPos+MovBuf(运动缓冲)
					//X运动长度的计算
					int nMoveLen = pPrtPassDes->nXPrtColumns * pPrtPassDes->fPrtPrecession + nMovBuf * 2;
#endif

				    //按X编码计算的运动长度L=pPrtPassDes->nXPrtColumns*pPrtPassDes->fPrtPrecession
					//********************************
					//运动起点保证：正向打印时  当前编码位置<nXPos ;反向打印时  当前编码位置>nXPos
					///::OutputDebugString(_T("[RY] Start pass print#######################################"));
					teststtt = pPrtPassDes->fYMinJetImgPos;
					cpu_time_used = static_cast<float>(pPrtPassDes->nStpVector);
					//sprintf(buf,"%s %f","pPrtPassDes->fYMinJetImgPos ",cpu_time_used);
					//AfxMessageBox(buf);
					
					
					/*if(i == 4)
					{
					sprintf(buf,"%s %f","第5pass累计步进量 =  ",pPrtPassDes->fYMinJetImgPos);
					AfxMessageBox(buf);
					}
					if(i == 5)
					{
					sprintf(buf,"%s %f","第6pass累计步进量 =  ",pPrtPassDes->fYMinJetImgPos);
					AfxMessageBox(buf);
					}*/
					//if(i == 4)
					
					const int nDoPassResult = ::IDP_DoPassPrint(pPrtPassDes);
					WriteProcessLog(pdlg, _T("IDP_DoPassPrint(PASS %d) 返回=%d"), i + 1, nDoPassResult);
					if(nDoPassResult>0)
					{
						//::OutputDebugString(_T("[RY] End pass print######################################"));
						//当前位置XCURPOS
						//正向打印的运动长度
						//负向打印运动目标位置XDEST=nXPos-L-MovBuf
						//负向打印运动目标位置XDEST=nXPos+L+MovBuf
						//运动长度 MOV_X(XDEST-XCURPOS); 启动运动
#ifdef RY_MOVE_CTL
						DEM_MoveX(!pPrtPassDes->bEncPrtDir, nSpd, nMoveLen, TRUE, g_movConfig.fxMovRate);
						// 获取打印状态确认打印停止 prtinfo.nPrtState
						// IDP_GetPrintState(&prtInfo);

#endif
					}
					else
					{
						//AfxMessageBox("IDP_DoPassPrint eer");
						//打印错误停止
						//break;
					}
					::OutputDebugString(_T("[RYZ] end IDP_DoPassPrint 44444444444444444444444"));



					//先等待编码器离开零位，避免初始位置等于ZERO_POSITION时直接进入下一PASS。
					while (pdlg->m_bJobStarted && DEV_GetPrinterEncValue() <= ZERO_POSITION + 100)
					{
						Sleep(1);
					}
					//等待编码器回到零位左侧，替代原先每个PASS后的人工确认弹窗。
					while (pdlg->m_bJobStarted && DEV_GetPrinterEncValue() > ZERO_POSITION)
					{
						Sleep(1);
					}
					if (!pdlg->m_bJobStarted)
					{
						break;
					}
				}
			}
				
		}
		else
		{
			szErr.Format(_T("图层处理错误。错误号：%dd"), nPassCount);
			AfxMessageBox(szErr);
			IDP_FreeImageLayer(n);
			pdlg->FreeLayerData();
			bAllLayersCompleted = FALSE;
			break;
		}
		IDP_FreeImageLayer(n);
		pdlg->FreeLayerData();
		WriteProcessLog(pdlg, _T("第%d层打印完成；准备读取下一层"), n + 1);
		if (!pdlg->m_bJobStarted)
		{
			bAllLayersCompleted = FALSE;
			break;
		}
	} //end for
	if (bAllLayersCompleted)
	{
		WriteProcessLog(pdlg, _T("全部图层处理结束，停止打印任务"));
		pdlg->NotifySocketPrintJobCompletedEvent(pdlg->m_nJobImgLayerCnts);
		AfxMessageBox(_T("打印完成"));
	}
	else
	{
		WriteProcessLog(pdlg, _T("打印任务已停止，未处理后续图层"));
	}
	::IDP_StopPrintJob();
	IDP_FreeImageLayer(-1);
	pdlg->FreeLayerData();
	//DEV_GetPrinterEncValue() - 0x10000;
#ifdef RY_MOVE_CTL
	DEM_MoveX(FALSE, nSpd, ndistance, TRUE, g_movConfig.fxMovRate);
#endif

	pdlg->m_bPrinting = false;
	pdlg->m_bJobStarted=false;//notify windows
	pdlg->PostMessage(WM_PRINT_JOB_FINISHED, 0, 0);
	AfxEndThread(0, TRUE);
	return 0;
}

LRESULT CNetScanDemoDlg::OnPrintJobFinished(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	ResetSinglePassProgress();
	UpdateButtonState();
	return 0;
}

void CNetScanDemoDlg::UpdateButtonState()
{
	//IDP_StopPrintJob();
	m_bJobStarted = false;
	CWnd* pWnd=GetDlgItem(IDC_BUTTON_STARTJOB);
	if(pWnd)
		pWnd->SetWindowText(m_bJobStarted?_T("结束任务"):_T("启动任务"));
	m_PrtFolderBrowse.EnableWindow(TRUE);
}

#include "CRelaodWaveDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonReloadwave()
{
	// TODO: 在此添加控件通知处理程序代码
	
	CReloadWaveDlg dlg;
	UINT nHeadData[8];
	CFile file;
	if (g_pSysInfo) {
		if (dlg.DoModal() == IDOK) {
			USES_CONVERSION;
			int nResult = DEV_ReloadWaveForm((1 << dlg.m_nColorIndex), (1 << dlg.m_nGroupIndex), T2A(dlg.m_szWavePath.GetBuffer(0)));
			g_IsRoladWave = true;
			if (nResult < 0)
			{
				//g_pSysInfo = DEV_GetDeviceInfo();
				if ((file.Open(T2A(dlg.m_szWavePath.GetBuffer(0)), CFile::modeRead)<=0))
				{
					//delete pDataBuf;
					//return -1;
				}
				file.Read(nHeadData, 8*sizeof(UINT));
				file.Close();
				if(nHeadData[5] != g_pSysInfo->nCustomerID)
				{
					AfxMessageBox("波形客户号和主板客户号不匹配或者波形设置可读了");
				}

				CString szErr;
				szErr.Format(_T("加载波形失败：%d"), nResult);
				AfxMessageBox(szErr);
			}
			else {
				AfxMessageBox(_T("加载成功"));
			}
			
		}
	}
}

#include "CVTSetDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonVtSet()
{
	// TODO: 在此添加控件通知处理程序代码
	CVTSetDlg dlg;
	float fVoltage[33];
	if (dlg.DoModal() == IDOK) {
		//while(1)
		{
			for (int ph = 0; ph < MAX_PH_CNT;ph++) {
				if (g_bPHValid[ph]) {
					fVoltage[0] = g_sysParam.phctl_param[ph].fStdVoltage;
					CopyMemory(&fVoltage[1], g_sysParam.phctl_param[ph].fVoltage, sizeof(float) * 32);
					if(!MVT_UpdatePhVoltage(fVoltage, ph))
					{
						AfxMessageBox("设置电压失败");
					}
					if(!MVT_SetPhStdTemp(g_sysParam.phctl_param[ph].fDestTemp, ph))
					{
						AfxMessageBox("设置温度失败");
					}
				}
			}
			//Sleep(1000);
		}
		
		AfxMessageBox(_T("设置电压温度成功"));
	}
}

#include "CAdjParamDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonCali()
{
	// TODO: 在此添加控件通知处理程序代码
	CAdjParamDlg dlg;
	if (dlg.DoModal() == IDOK) {
		if (!DEV_UpdateParam(&g_sysParam))
			AfxMessageBox(_T("更新参数失败"));
		else
			AfxMessageBox(_T("更新参数成功"));
	}
}

#include "CSysParamDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonSysParam()
{
	// TODO: 在此添加控件通知处理程序代码
	CSysParamDlg dlg;
	if (dlg.DoModal() == IDOK) {
		if (!DEV_UpdateParam(&g_sysParam))
			AfxMessageBox(_T("更新参数失败"));
		else
			AfxMessageBox(_T("更新参数成功"));
	}
}


int CNetScanDemoDlg::ExecuteFlashToggle(BOOL bShowError, BOOL* pBeforeState, BOOL* pAfterState)
{
	if (pBeforeState != NULL)
		*pBeforeState = FALSE;
	if (pAfterState != NULL)
		*pAfterState = FALSE;

	PrtRunInfo runInfo = {};
	if (!IDP_GetPrintState(&runInfo))
	{
		WriteProcessLog(this, _T("闪喷控制失败：无法获取当前打印状态"));
		if (bShowError)
			AfxMessageBox(_T("获取打印状态失败"));
		return 1;
	}

	const BOOL bBeforeState = (runInfo.nPrtState == 3);
	const BOOL bAfterState = !bBeforeState;
	if (pBeforeState != NULL)
		*pBeforeState = bBeforeState;
	if (pAfterState != NULL)
		*pAfterState = bBeforeState;

	if (!IDP_FlashPrtCtl(bAfterState))
	{
		WriteProcessLog(this, _T("闪喷控制失败：当前=%s，目标=%s"),
			bBeforeState ? _T("开启") : _T("关闭"),
			bAfterState ? _T("开启") : _T("关闭"));
		if (bShowError)
			AfxMessageBox(_T("闪喷控制失败"));
		return 2;
	}

	if (pAfterState != NULL)
		*pAfterState = bAfterState;
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_FLASH);
	if (pWnd != NULL)
		pWnd->SetWindowText(bAfterState ? _T("关闭闪喷") : _T("启动闪喷"));
	WriteProcessLog(this, _T("闪喷状态修改成功：%s -> %s"),
		bBeforeState ? _T("开启") : _T("关闭"),
		bAfterState ? _T("开启") : _T("关闭"));
	return 0;
}

void CNetScanDemoDlg::OnBnClickedButtonFlash()
{
	ExecuteFlashToggle(TRUE, NULL, NULL);
}


void CNetScanDemoDlg::OnBnClickedButtonResetEncoder()
{

		if(!DEV_ResetPrinterEncValue(ZERO_POSITION))
		{
			AfxMessageBox("失败");
		}

	
}

void CNetScanDemoDlg::OnEnChangeMfcPrtFilePath()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData();
	m_PrintLayers.clear();
	m_nCurrentDataChannelCount = 0;
	m_nCurrentPassCount = -1;
	CString  szText;
	CString szPreviewFile = m_szPrtFile;
	while (szPreviewFile.GetLength() > 3 &&
		(szPreviewFile.Right(1) == _T("\\") || szPreviewFile.Right(1) == _T("/")))
		szPreviewFile = szPreviewFile.Left(szPreviewFile.GetLength() - 1);
	szPreviewFile += _T("\\1\\1.bmp");
	m_ImgPreView.ShowPreview(szPreviewFile);
	if (m_ImgPreView.m_hBmp)
	{
		CFile file;
		BITMAPFILEHEADER   bmpfileheader;
		BITMAPINFOHEADER   bmpinfoheader;
		if (file.Open(szPreviewFile, CFile::modeRead))
		{
			file.Read(&bmpfileheader, sizeof(BITMAPFILEHEADER));
			file.Read(&bmpinfoheader, sizeof(BITMAPINFOHEADER));
			const long nPixelHeight = bmpinfoheader.biHeight < 0
				? -bmpinfoheader.biHeight
				: bmpinfoheader.biHeight;
			if (g_PrtImgLayer.nXDPI > 0 && g_PrtImgLayer.nYDPI > 0)
			{
				const double fXActualWidth =
					static_cast<double>(bmpinfoheader.biWidth) / g_PrtImgLayer.nXDPI * 25.4;
				const double fYActualHeight =
					static_cast<double>(nPixelHeight) / g_PrtImgLayer.nYDPI * 25.4;
				szText.Format(_T("%dx%d X:%.3fmm Y:%.3fmm"),
					bmpinfoheader.biWidth, nPixelHeight,
					fXActualWidth, fYActualHeight);
			}
			else
			{
				szText.Format(_T("%dx%d XDPI/YDPI无效"),
					bmpinfoheader.biWidth, nPixelHeight);
			}
			if (bmpinfoheader.biBitCount > 1)
				szText += _T(" \r\nNot Support!");
			m_szImageInfo2Base = szText;
			UpdateImageInfo2();
			file.Close();
		}
	}
	else
	{
		m_szImageInfo2Base = _T("请选择包含 1\\1.bmp 的打印根文件夹");
		UpdateImageInfo2();
	}
	m_bImageLoaded = FALSE;
}



void CNetScanDemoDlg::OnDestroy()
{
	if (m_pSocketPressInkRequest != NULL)
	{
		KillTimer(SOCKET_PRESS_INK_TIMER_ID);
		MVT_SetOutPut(0, 0x8, false);
		m_pSocketPressInkRequest->nError = 5;
		SetEvent(m_pSocketPressInkRequest->hCompletedEvent);
		m_pSocketPressInkRequest = NULL;
		WriteProcessLog(this, _T("程序退出，Socket压墨已停止"));
	}
	StopSocketServer();
	m_bStopMonitor = TRUE;
	for(int c=0;c<MAX_COLORS;c++)
	{
		///if(m_pPrtData[c])
		//	delete m_pPrtData[c];
		//m_pPrtData[c]=NULL;
	}

	DEV_CloseDevice();
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}

static std::string FormatSocketReceivedData(const char* pData, int nLength)
{
	std::string result;
	result.reserve(static_cast<size_t>(nLength) * 2);
	for (int i = 0; i < nLength; ++i)
	{
		const unsigned char ch = static_cast<unsigned char>(pData[i]);
		if (ch == '\r')
			result += "\\r";
		else if (ch == '\n')
			result += "\\n";
		else if (ch == '\t')
			result += "\\t";
		else if (ch >= 0x20 && ch <= 0x7E)
			result += static_cast<char>(ch);
		else
		{
			char szHex[5] = {};
			sprintf_s(szHex, "\\x%02X", ch);
			result += szHex;
		}
	}
	return result;
}

void CNetScanDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SOCKET_PRESS_INK_TIMER_ID && m_pSocketPressInkRequest != NULL)
	{
		if (GetTickCount() - m_dwSocketPressInkStartTick >= m_dwSocketPressInkDuration)
		{
			KillTimer(SOCKET_PRESS_INK_TIMER_ID);
			SocketPressInkRequest* pRequest = m_pSocketPressInkRequest;
			m_pSocketPressInkRequest = NULL;
			pRequest->nError = MVT_SetOutPut(0, 0x8, false) ? 0 : 4;
			WriteProcessLog(this, pRequest->nError == 0
				? _T("Socket压墨完成：J62已关闭")
				: _T("Socket压墨结束，但关闭J62失败"));
			SetEvent(pRequest->hCompletedEvent);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

#include "CMoveCtlDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonMovctl()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef RY_MOVE_CTL
	CMoveCtlDlg dlg;
	dlg.DoModal();
#endif
}


#include "CAirInkDialog.h"
void CNetScanDemoDlg::OnBnClickedButtonAirInk()
{
	// TODO: 在此添加控件通知处理程序代码
	CAirInkDialog inkDlg;
	inkDlg.DoModal();
}

#include "CAdibCtrlDlg.h"
void CNetScanDemoDlg::OnBnClickedButtonAdibCtl()
{
	// TODO: 在此添加控件通知处理程序代码
	CAdibCtrlDlg adibDlg;
	adibDlg.DoModal();
}


void CNetScanDemoDlg::OnBnClickedCheckInkWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_bEnableInkWacth)
		DEV_EnableInkWatch(TRUE, 0, 0);
	else
		DEV_EnableInkWatch(FALSE, 0, 0);
}


void CNetScanDemoDlg::OnBnClickedButton1()
{
	
	// TODO: 在此添加控件通知处理程序代码
	DEV_GenAllRegReport("D:\\ROYAL\\3DSCAN\\");
}


void CNetScanDemoDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString AppmianRuteTemp = m_szAppPath  + "appmain";
	char* AppmianRute = (char*)(AppmianRuteTemp.GetBuffer(0));
	DEV_UpdateAppFile(AppmianRute,0);
	
}


void CNetScanDemoDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	CString AppmianRuteTemp = m_szAppPath  + "soc_system.rbf";
	char* AppmianRute = (char*)(AppmianRuteTemp.GetBuffer(0));
	DEV_UpdateFpgaFile(AppmianRute,0);
	
}

void CNetScanDemoDlg::OnBnClickedCheckCloseVtMonitor()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CNetScanDemoDlg::OnStnClickedStaticRipView()
{
	// TODO: 在此添加控件通知处理程序代码
}

LRESULT CNetScanDemoDlg::OnUpdatePassCount(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	m_nCurrentPassCount = static_cast<int>(static_cast<INT_PTR>(wParam));
	CString szPassInfo = _T("PASS数量: --");
	if (m_nCurrentPassCount >= 0)
		szPassInfo.Format(_T("PASS数量: %d"), m_nCurrentPassCount);
	SetDlgItemText(IDC_STATIC_ImageInfo3, szPassInfo);

	return 0;
}

void CNetScanDemoDlg::UpdateImageInfo2()
{
	CString szImageInfo = m_szImageInfo2Base;
	if (!m_PrintLayers.empty())
	{
		CString szDataInfo;
		szDataInfo.Format(_T("    颜色通道数量: %d    数据总层数: %d"),
			m_nCurrentDataChannelCount,
			static_cast<int>(m_PrintLayers.size()));
		szImageInfo += szDataInfo;
	}
	else
	{
		szImageInfo += _T("    颜色通道数量: --    数据总层数: --");
	}
	SetDlgItemText(IDC_STATIC_ImageInfo2, szImageInfo);
}

LRESULT CNetScanDemoDlg::OnSocketFlashControl(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	SocketFlashResult* pResult = reinterpret_cast<SocketFlashResult*>(lParam);
	if (pResult == NULL)
		return 0;
	WriteProcessLog(this, _T("主线程执行Socket闪喷命令"));
	pResult->nError = ExecuteFlashToggle(FALSE,
		&pResult->bBeforeState, &pResult->bAfterState);
	return 0;
}

LRESULT CNetScanDemoDlg::OnSocketPressInkControl(WPARAM wParam, LPARAM lParam)
{
	SocketPressInkRequest* pRequest = reinterpret_cast<SocketPressInkRequest*>(lParam);
	if (pRequest == NULL)
		return 0;

	if (wParam == 1)
	{
		if (m_pSocketPressInkRequest == pRequest)
		{
			KillTimer(SOCKET_PRESS_INK_TIMER_ID);
			MVT_SetOutPut(0, 0x8, false);
			m_pSocketPressInkRequest = NULL;
			WriteProcessLog(this, _T("Socket压墨等待超时，J62已强制关闭"));
		}
		return 0;
	}

	if (m_pSocketPressInkRequest != NULL)
	{
		pRequest->nError = 2;
		WriteProcessLog(this, _T("Socket压墨命令被拒绝：当前正在压墨"));
		return 0;
	}
	if (!MVT_SetOutPut(0, 0x8, true))
	{
		pRequest->nError = 3;
		WriteProcessLog(this, _T("Socket压墨启动失败：无法打开J62"));
		return 0;
	}

	m_pSocketPressInkRequest = pRequest;
	m_dwSocketPressInkStartTick = GetTickCount();
	m_dwSocketPressInkDuration = static_cast<DWORD>(pRequest->fSeconds * 1000.0);
	if (SetTimer(SOCKET_PRESS_INK_TIMER_ID, 100, NULL) == 0)
	{
		MVT_SetOutPut(0, 0x8, false);
		m_pSocketPressInkRequest = NULL;
		pRequest->nError = 3;
		WriteProcessLog(this, _T("Socket压墨启动失败：无法创建定时器"));
		return 0;
	}
	WriteProcessLog(this, _T("Socket压墨已启动：J62打开，设定时间=%.3f秒"),
		pRequest->fSeconds);
	return 0;
}
LRESULT CNetScanDemoDlg::OnAppendProcessLog(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	CString* pMessage = reinterpret_cast<CString*>(lParam);
	if (pMessage == NULL)
		return 0;
	CString szMessage = *pMessage;
	delete pMessage;
	SYSTEMTIME timeNow;
	::GetLocalTime(&timeNow);
	CString szLine;
	szLine.Format(_T("[%02u:%02u:%02u.%03u] %s"), timeNow.wHour, timeNow.wMinute,
		timeNow.wSecond, timeNow.wMilliseconds, static_cast<LPCTSTR>(szMessage));
	if (!m_szProcessLog.IsEmpty())
		m_szProcessLog += _T("\r\n");
	m_szProcessLog += szLine;
	SetDlgItemText(IDC_STATIC_1111, m_szProcessLog);
	CWnd* pLogWnd = GetDlgItem(IDC_STATIC_1111);
	if (pLogWnd != NULL)
	{
		pLogWnd->SendMessage(EM_SETSEL, m_szProcessLog.GetLength(), m_szProcessLog.GetLength());
		pLogWnd->SendMessage(EM_SCROLLCARET, 0, 0);
	}
	return 0;
}

