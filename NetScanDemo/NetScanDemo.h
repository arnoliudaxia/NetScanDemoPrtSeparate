
// NetScanDemo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNetScanDemoApp:
// See NetScanDemo.cpp for the implementation of this class
//
#define MM_TO_DOT(X,DPI)  (int)(((float)(X*DPI))/25.4+0.45f)
typedef struct movconfig {
	float fxSysdpi;
	float fySysdpi;
	float fxMovSpd;
	float fyMovSpd;
	float fxMovAcc;
	float fyMovAcc;
	float fxMovUnit;
	float fyMovUnit;
	float fxMovRate;
	float fyMovRate;
	float fxIoOption;
	float fyIoOption;
	float fMovBuf;
}MOV_Config;

class CNetScanDemoApp : public CWinApp
{
public:
	CNetScanDemoApp();

// Overrides
public:
	CString m_szAppPath;
	virtual BOOL InitInstance();
public:
	int LoadParam(LPRYUSR_PARAM lparam);
	int SaveParam(LPRYUSR_PARAM lparam);
	int SaveLayerParam();
	int SaveJobParam();
	int SaveCalibrationParam();
	int LoadLayerParam();
	int LoadJobParam();
	int LoadCalibrationParam();
#ifdef  RY_MOVE_CTL
	int LoadMoveParam();
	int SaveMoveParam();
#endif
	
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNetScanDemoApp theApp;