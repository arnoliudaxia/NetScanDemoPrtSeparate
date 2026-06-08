// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RYPRTCTLER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RYPRTCTLER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef RYPRTCTLER_EXPORTS
#define RYPRTCTLER_API extern "C" __declspec(dllexport)
#else
#define RYPRTCTLER_API extern "C" __declspec(dllimport)
#endif
#include "ryprtapi.h"

/*
int  DEV_OpenDevice(void* notifyhand,unsigned char* szDllFilePath)
返回值定义
	1: 连接正常 
	-2: pcs文件加载失败 
	-4:graycfg文件加载失败 
	-3:网络卡连接失败 
	-5: 上位机软件多开
*/
RYPRTCTLER_API int				__stdcall DEV_OpenDevice(void* notifyhand,unsigned char* szDllFilePath);

RYPRTCTLER_API int				__stdcall DEV_InitDevice(unsigned int nXEncInitVal);

/*
bool  DEV_DeviceIsConnected()
返回值定义
	true : 连接成功
	false : 重新连接中
*/
RYPRTCTLER_API bool				__stdcall DEV_DeviceIsConnected();
/*
bool  DEV_CloseDevice()
返回值定义
	true : 连接成功
	false : 重新连接中
*/
RYPRTCTLER_API bool				__stdcall DEV_CloseDevice();
RYPRTCTLER_API bool				__stdcall DEV_UpdateParam(LPRYUSR_PARAM pParam);
RYPRTCTLER_API bool				__stdcall DEV_GetCurParam(LPRYUSR_PARAM pParam);	//20200407
RYPRTCTLER_API LPPRINTER_INFO	__stdcall DEV_GetDeviceInfo();
RYPRTCTLER_API int				__stdcall DEV_ReloadWaveForm(unsigned nClrMask,unsigned int nPHGMask,char* szWaveFile); //重新加载波形
RYPRTCTLER_API bool				__stdcall DEV_ResetPrinterEncValue(unsigned int nPos);    //重新设置当前位置值
RYPRTCTLER_API bool				__stdcall DEV_SetVirtualPrtEncoder(bool bEnable,bool bEncDir,unsigned int nClkFreq);//测试用途
RYPRTCTLER_API unsigned int		__stdcall DEV_GetPrinterEncValue();    //查询当前位置值


///////图像PASS打印处理
RYPRTCTLER_API LPPassDataItem	__stdcall IDP_GetPassItem(unsigned int  nLayerIndex,int nPassID);
RYPRTCTLER_API int	__stdcall IDP_SartPrintJob(LPPRTJOB_ITEM pJobItem);
RYPRTCTLER_API bool	__stdcall IDP_StopPrintJob();
RYPRTCTLER_API int	__stdcall IDP_WriteImgLayerData(LPPRTIMG_LAYER lpLayerInfo,unsigned char* pSrcBuf[],int nBytes);
RYPRTCTLER_API int	__stdcall IDP_WriteImgLayerFile(LPPRTIMG_LAYER lpLayerInfo,char* szFile);
RYPRTCTLER_API int  __stdcall IDP_StartLayerPrint(int nLayerIndex);
RYPRTCTLER_API int  __stdcall IDP_FreeImageLayer(int nLayerIndex);
RYPRTCTLER_API int  __stdcall IDP_StartCalibration(LPRYCalbrationParam pParam);
RYPRTCTLER_API int	__stdcall IDP_DoPassPrint(LPPassDataItem pPassItem);
RYPRTCTLER_API bool	__stdcall IDP_GetPrintState(LPPrtRunInfo pRTinfo);
RYPRTCTLER_API bool __stdcall IDP_FlashPrtCtl(bool bOpen);
RYPRTCTLER_API void __stdcall IDP_SetCaliDataCallBack(LPRY_CALI_PHGDCall pFun);
//////温度电压控制
RYPRTCTLER_API bool	__stdcall MVT_UpdatePhVoltage(float* fstdVcom,unsigned int nPhID);
RYPRTCTLER_API bool	__stdcall MVT_SetPhStdTemp(float* fstdTmp,unsigned int nPhID);
RYPRTCTLER_API bool	__stdcall MVT_CurPhVoltage(float* fCurVcom,unsigned int nPhID);
RYPRTCTLER_API bool	__stdcall MVT_GetCurPhTemp(float* fCurTemp,unsigned int nPhID);
/// 负压控制接口
RYPRTCTLER_API bool	__stdcall MVT_SetAirPressCtlVal(unsigned int nPrtIndex);
/// 手动输出控制接口
RYPRTCTLER_API bool	__stdcall MVT_SetOutPut(int nPrtIndex, int nOutMask, bool bEnable);
RYPRTCTLER_API bool	__stdcall MVT_SetOutPutConfig(int nPrtIndex);
/// ADIB板卡控制接口
RYPRTCTLER_API bool	__stdcall  DEV_AdibControl(LPADIB_PARAM lParam, unsigned int nOption, bool bSetParam, bool* bAbort, unsigned int nPrtIndex);
//////校准打印
// 墨滴观测仪
RYPRTCTLER_API bool	__stdcall DEV_EnableInkWatch(bool bOpen, unsigned int nPrtIndex, unsigned int nDrvIndex);
//生成寄存器报告
RYPRTCTLER_API void	__stdcall DEV_GenAllRegReport(const char* szFilePath);
//更新appmain
RYPRTCTLER_API int	__stdcall DEV_UpdateAppFile(const char* szFilePath, unsigned int nPrtIndex);
//更新fpga
RYPRTCTLER_API int	__stdcall DEV_UpdateFpgaFile(const char* szFilePath, unsigned int nPrtIndex);