// MoveCtl.h : main header file for the MoveCtl DLL
//

#pragma once
#ifdef RYPRTCTLER_EXPORTS
#define RYPRTCTLER_API extern "C" __declspec(dllexport)
#else
#define RYPRTCTLER_API extern "C" __declspec(dllimport)
#endif

// 系统初始化
RYPRTCTLER_API int _stdcall DEM_Initialize(char* szDllFile);
// 初始化X轴运动参数
RYPRTCTLER_API void _stdcall DEM_UpdateXMovCfg(int nXMovAcc, int nIoOption);
// 初始化Y轴运动参数
RYPRTCTLER_API void _stdcall DEM_UpdateYMovCfg(int nYMovAcc, int nIoOption);
// 运动使能
RYPRTCTLER_API void _stdcall DEM_EnableRun();
//// 获取当前Y编码位置
RYPRTCTLER_API void _stdcall DEM_GetYPosition(int& nAxis1Pos,int& nAxis2Pos);
//// 获取当前X编码位置
RYPRTCTLER_API unsigned int _stdcall DEM_GetXPosition();
//// 停止X轴运动
RYPRTCTLER_API void _stdcall DEM_StopXMove(bool bImmedStop);
//// 停止Y轴运动
RYPRTCTLER_API void _stdcall DEM_StopYMove(bool bImmedStop);
// 停止Z轴运动
RYPRTCTLER_API void _stdcall DEM_StopAll(bool bImmedStop);
// 控制X轴运动
RYPRTCTLER_API bool _stdcall DEM_MoveX(bool bDir, unsigned int  nSpeed, float nStep, bool bWait,float fXMoveRate);
//// 控制Y轴运动
RYPRTCTLER_API bool _stdcall DEM_MoveY(bool bDir, unsigned int nSpeed, float nStep, bool bWait,bool bSynCtl,float fYMoveRate);
//// 等待X轴停止运动
RYPRTCTLER_API bool _stdcall DEM_WaitUntilXStop();
//// 等待Y轴停止运动
RYPRTCTLER_API bool _stdcall DEM_WaitUntilYStop();
//// 判断X轴是否在运动中
RYPRTCTLER_API bool _stdcall DEM_XIsMoving(bool* pDir, unsigned int* pRPos);
//// 判断Y轴是否在运动中
RYPRTCTLER_API bool _stdcall DEM_YIsMoving(bool* pDir, unsigned int* pRPos);
// 获取X轴零位状态
RYPRTCTLER_API bool _stdcall DEM_GetXZeroSig();
// 获取X轴正限位
RYPRTCTLER_API bool _stdcall DEM_GetXPosLimt(BOOL bXReverseLmt);
// 获取X轴负限位
RYPRTCTLER_API bool _stdcall DEM_GetXNegLimt(BOOL bXReverseLmt);
// 获取Y轴零位状态
RYPRTCTLER_API bool _stdcall DEM_GetYZeroSig();
// 获取Y轴正限位
RYPRTCTLER_API bool _stdcall DEM_GetYPosLimt(BOOL bYReverseLmt);
// 获取Y轴负限位
RYPRTCTLER_API bool _stdcall DEM_GetYNegLimt(BOOL bYReverseLmt);