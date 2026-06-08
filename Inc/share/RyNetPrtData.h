#if !defined(ROYAL_EMBED_NETPRTAPI_20180622)
#define ROYAL_EMBED_NETPRTAPI_20180622

#include "..\ryprtapi.h"

#define _MAX_DRV_PHSET     4     //每个驱动卡最多4个喷头
#define _MAX_PHROWCNT      32    //喷头的最大排嘴排数
#define _MAX_PHVOLTAGE     32    //单个喷头的最大电压数量
#define _MAX_PHTEPCTL      4     //每个喷头的最大温度控制数
#define _MAX_PHFIRMWARE    4     //每个喷头最大使用的WaveForm
#define _MAX_PH_FIREPARAM  32    //
#define _MAX_TRIGCOUNT     4     //最大触发数量
#define _MAX_PAGE_COUNT    64    //最大页数
#define _MAX_IMGBUF_CNT    8     //

typedef unsigned char       BYTE;
typedef int                 BOOL;
typedef unsigned int        UINT;

typedef struct tag_RYSS_TIME
{
	unsigned short     wYear;   //Year;
	char               wMonth;
	char               wDay;
}RYSS_TIME,*LPRYSS_TIME;

typedef struct tag_SecLicInfo
{
	int		 nResultMask;
	UINT     	 nLicenseID[2];
	RYSS_TIME	 gen_date;
	UINT     	 nCtVal;
	UINT     	 nFunMask;
	UINT     	 nSerialIndex;
	UINT     	 nRevITVal;
	UINT     	 nRevInfo[4];
}SecLicInfo,*LPSecLicInfo;

typedef struct tag_IMGDataTxPamrm
{
	UINT	nDataIndex;    //数据号 0~62
	BOOL	nPrtScanDir;       //打印扫描方向
	UINT	nDataSize;
	UINT	nValidPrtCtlMask;
	UINT	nPrtEncPosition[MCP_DRV_CNT];  //打印起始位置
	UINT	nPrtCols[MCP_DRV_CNT];			//打印列数
	UINT	nDataOffset[MCP_DRV_CNT];       //数据偏移量
	UINT    nColBytes[MCP_DRV_CNT];         //数据一列字节数
}IMGDataTxPamrm,*LPIMGDataTxPamrm;


//打印相关的实时信息
typedef struct tag_RYPRTER_RTINFO
{
	UINT	nPrtEncHighVal;  //当前编码计数值 高32位
	UINT	nPrtEncAxis;     //当前编码计数值 低32位
	UINT	nIoState;         //IO输入状态，每个bit代表一个IO状态，预留
	UINT	nTrigCount[_MAX_TRIGCOUNT]; //触发模式下的触发计数，最多4个触发源，当前只用第一个
	UINT	nPrtState;       //打印状态 [0:3] 驱动卡1 [4:7] 驱动卡2 [8:11] 驱动卡3 [12:15] 驱动卡4 [16:19] 驱动卡5  [20:23] 驱动卡6  [24:27] 驱动卡7  [28:31] 驱动卡8  //VAL 0 IDLE 1 Print 2 Pause 3 Flash 
	UINT	nPrtRevCol[MCP_DRV_CNT];   //当前pass/当前页 打印剩余未打印列数，为0则打印结束
	UINT	nCurPrtPageID[MCP_DRV_CNT]; //当前正在打印的页区，板卡分最多63个打印数据存放区
}RYPRTER_RTINFO,*LPRYPRTER_RTINFO;
//喷头温度电压信息
typedef struct tag_RYVT_RTInfo
{
	float   fAnalogVolVal[4];            //主卡模拟采用的电压值
    float   fPhTempVal[MCP_DRV_CNT][_MAX_DRV_PHSET][_MAX_PHTEPCTL];      //每个喷头最多2个温度
    float   fPhVolOutVal[MCP_DRV_CNT][_MAX_DRV_PHSET][_MAX_PHVOLTAGE]; //每个喷头最多32个电压
	float   fDrvExtTemp[MCP_DRV_CNT][2]; //驱动卡温度
}RYVT_RTINFO,*LPRYVT_RTINFO;
//
typedef struct tag_InkCtlInfor
{
	float fCTLTmp;
	float fInkTemperature[2];
	float fInkAirPressure[2];
	float Rev[2];
}INKCTLINFOR,*LPINKCTLINFOR;
//#define _MAX_DRV_COUNT 8
//typedef struct SPPRTER_INFO
//{
//	UINT	nVersion;                   //打印机软件系统版本号
//	UINT    	nCustomIerD;                //客户号
//	UINT    	nSysHWCode;                 //硬件编码
//	UINT    	nDeviceID[2];               //设备序列号
//	UINT    	nParamLoadFlag;             //0 CONFIG 1:Waveform 2 PhParam           
//	UINT    	nWaveFormID;                //使用的波形ID
//	UINT    	nMainFPGAVer;               //主卡FPGA版本号
//	UINT    	nMcuFMVer;                  //主卡MCU版本号
//	UINT    	nDrvFMVer[_MAX_DRV_COUNT];  //驱动卡FPGA程序版本 
//	UINT    	nDrvMcuVer[_MAX_DRV_COUNT]; //驱动卡MCU程序版本
//	UINT    	nPrtCtlCount;               //有效的打印控制卡数量
//	UINT    	nPrtCtlValidMask;
//
//	UINT    	nPrtCtlState[_MAX_DRV_COUNT]; //各打印控制器状态  bit0 1存在波形下载错误 bit1 VCOM 打开失败 bit2 com失败 bit3 电压设置失效 
//									//bit4 温度设置失效  bit5 电源关闭状态 bit8~bit23 DrvStatus;  
//	UINT    	nPrtOverFreqCount[_MAX_DRV_COUNT];      //各控制器超频计数
//	UINT    	nPrtCtlNoPageErrorCnt[_MAX_DRV_COUNT];  //各控制器数据缺页错误
//	UINT    	nPrtCtlError1[_MAX_DRV_COUNT];
//	UINT    	nDrvWaveID[_MAX_DRV_COUNT][_MAX_DRV_PHSET];//每个喷头采用WaveFormID
//	UINT    	nPrtCurPage[_MAX_DRV_COUNT];
//	UINT    	nPrtFinishPageCnt[_MAX_DRV_COUNT];
//
//	UINT    	nTrigCount[_MAX_TRIGCOUNT];  //触发计数
//	UINT    	nSysInitResult;
//	UINT    	nDrvLinkState;
//	int     	nLastSysError;
//	int     	nErrorPrtIndex;
//	float   	fRollSpd;                  //皮带速度   米/分
//	UINT    	nIoState;                  //[0:15]输出状态  [16:31]：输入状态  需详细定义
//	UINT    	nHighEncPos;
//	UINT    	nPrtEncPosition;           // 
//	UINT    	nSec_Mode;                 //BIT0：墨量控制有效
//	UINT    	nSec_ExpirationSecond;     // 剩余使用时间
//	int     	nSec_curissue;             // 当前期号
//	UINT    	nSyncDataLinkErr;          //同步异常  
//	UINT    	nPhParamCRC;
//	INKCTLINFOR	InkCtlInfo;
//	UINT        nPhLinkErrMask;  //20230821 ths I3200
//	float       fPhTempVal[_MAX_DRV_COUNT][_MAX_DRV_PHSET][_MAX_PHTEPCTL];      //每个喷头最�?个温�?
//	float       fPhVolOutVal[_MAX_DRV_COUNT][_MAX_DRV_PHSET][_MAX_PHVOLTAGE];   //每个喷头最�?2个电�?
//	UINT        nEpsonVTResult[2];   //20200707  一个字节一个驱动卡 
//	UINT        nRev[32];
//	SecLicInfo  licinfo;
//}SPPRTER_INFO, * LPSPPRTER_INFO;
typedef struct tag_SPPRTER_INFO
{
    UINT		 nVersion;                   //打印机软件系统版本号
    UINT		 nCustomerID;                //客户号
    UINT		 nSysHWCode;                 //硬件编码
    UINT		 nDeviceID[2];               //设备序列号
    UINT		 nParamLoadFlag;             //0 CONFIG 1:Waveform 2 PhParam           
    UINT		 nWaveFormID;                //使用的波形ID
    UINT		 nMainFPGAVer;               //主卡FPGA版本号
    UINT		 nMcuFMVer;                  //主卡MCU版本号
    UINT		 nDrvFMVer[MCP_DRV_CNT];  //驱动卡FPGA程序版本 
    UINT		 nDrvMcuVer[MCP_DRV_CNT]; //驱动卡MCU程序版本
    UINT		 nPrtCtlCount;               //有效的打印控制卡数量
    UINT		 nPrtCtlValidMask;			
    UINT		 nPrtCtlState[MCP_DRV_CNT];           //各打印控制器状态  bit0 1存在波形下载错误 bit1 VCOM 打开失败 bit2 com失败 bit3 电压设置失效 
															//bit4 温度设置失效  bit5 电源关闭状态 bit8~bit23 DrvStatus;  
    UINT		 nPrtOverFreqCount[MCP_DRV_CNT];      //各控制器超频计数
    UINT		 nPrtCtlNoPageErrorCnt[MCP_DRV_CNT];  //各控制器数据缺页错误
    UINT		 nPrtCtlError1[MCP_DRV_CNT];
    UINT		 nDrvWaveID[MCP_DRV_CNT][_MAX_DRV_PHSET];//每个喷头采用WaveFormID
    UINT		 nPrtCurPage[MCP_DRV_CNT];
    UINT		 nPrtFinishPageCnt[MCP_DRV_CNT];
  
    UINT		 nTrigCount[_MAX_TRIGCOUNT];  //触发计数
    UINT		 nSysInitResult;
    UINT		 nDrvLinkState;
    int			 nLastSysError;
    int			 nErrorPrtIndex;
    float		 fRollSpd;                  //皮带速度   米/分
    UINT		 nIoState;                  //[0:15]输出状态  [16:31]：输入状态  需详细定义
    UINT		 nHighEncPos;
    UINT		 nPrtEncPosition;           // 
    UINT		 nSec_Mode;                 //BIT0：墨量控制有效
    UINT		 nSec_ExpirationSecond;     // 剩余使用时间
    int			 nSec_curissue;             // 当前期号
    UINT		 nSyncDataLinkErr;          //同步异常  
	UINT		 nPhParamCRC; 
	INKCTLINFOR  InkCtlInfor;				//20190828  注意，新增字节，嵌入式必须增加，不然字节对不上
    //float		 fRev[3];
	UINT        nPhLinkErrMask;  //20230821 ths I3200
	RYVT_RTINFO rt_data;
	//float       fPhTempVal[MCP_DRV_CNT][_MAX_DRV_PHSET][_MAX_PHTEPCTL];      //每个喷头最�?个温�?
	//float       fPhVolOutVal[MCP_DRV_CNT][_MAX_DRV_PHSET][_MAX_PHVOLTAGE];   //每个喷头最�?2个电�?
	char        szPHSID[MCP_DRV_CNT][_MAX_DRV_PHSET][32];                      //每个喷头的序列号
	UINT        nEpsonVTResult[2];   //20200707  一个字节一个驱动卡 
	UINT        nRev[32];
	SecLicInfo  licinfo;
	//SecLicInfo	 LicIfo;
}SPPRTER_INFO,*LPSPPRTER_INFO;

typedef struct dev_prt_ctl_set
{
	BOOL			nValid;            //配置有效
	BOOL			nLinked;           //备用参数
	UINT			nDrvCardType;      //连接的驱动卡类型
	RYPHI_INFO		phi_info;    //
	UINT			nWaveSelection;    //使用的波形数据索引
	UINT			nCtlVal;           //驱动卡控制字  bit0:自动待机 bit1 自动刷新 
	UINT			nTrigMode;         //触发信号源       0~3 
	int				nSupportPhCount;
	int				nPhValidCount; 
	UINT			nPhValidMask;
	UINT			nPhColDataStdBytes;      //////////////////
	UINT			nRevParam2[_MAX_DRV_PHSET];
}DEVPRTCTL_ITEM,*LPDEVPRTCTL_ITEM;         // 8个打印控制器
typedef struct tag_MotionParam    //20190828 新增
{
	float  fMoverate;                        //脉冲当量unit:pulse/mm  
	float  fMaxMovSpd;                       //unit:pulse/mm
	float  fMaxMovRange;                     //最大运动行程 
	float  fAccRate;                         //加速度  mm/S2 
	float  fEncmmUnit;                       //脉冲反馈精度 每mm多少编码数 
	float  fRstMovspd;                       //复位速度
	float  fRunSpd;                          //运行速度
	unsigned int  nSigOption;                //// 0b 输出方向反转1反转  1b:正负限位极性1表示开路有效   2b:编码器计数反   3b 零位有效信号反  4b 正限位有效信号反  5b 负限位有效信号反 6b 输出方式[0:方向+脉冲 1:双向脉冲],
	// 7b 零位信号源采用OPD 8b: Z编码器采用指令脉冲计数 9b:液位信号反  10b 输出方向采用硬件控制  11b Z编码器信号源 1 脉冲计数 0 编码计数
}MOTION_PARAM,*LPMOTION_PARAM;
typedef struct tag_InkCtlParam   //20190829新增
{
	float fAirPressureCtlValue[2];     //负压设定值
	float fAirPressureSafetyValue[2];     //负压安全值
	float fInkTemperature[2];
	//	float Rev[3];
}INKCTLPARAM,*LPINKCTLPARAM;
typedef struct tag_PHMaintParam
{
	float   fWiperHeight;    //刮墨高度       相对Z零位  单位 mm
	//float   fCapHeight;      //吸墨保湿高度， 相对Z零位  单位 mm
	float   fWipeLength;     //刮墨动作, 刮墨长度        单位 mm
	float   fAcutoClnTimer;  //压墨时间      单位 sec
	//float   fSuckInkTime;    //吸墨时间                  单位 sec
	float   fWipeSpeed;      //刮墨运动速度
	//float   fFlashHeight;    //清洗闪喷高度
	float   fResever[7];     //保留参数  //20190523 8->7
}PHMaintParam,*LPPHMaintParam;
/************************************************/
typedef struct tag_devconfig
{
    UINT			nCfgVersion;          //0x1001
    UINT			nCfgID;               //配置ID
    float			fPrtEncdpi;
    float			fPhGdpi;              //喷头组DPI
    BOOL			bUseMode;             //0 从卡 1主卡
    BOOL			bPrtMode;             //0 分页  1 连续  2.扫描打印
    BOOL			bXPrtDir;             //打印方向   //true 从左往右 

    int				nSysClrCount;         //系统颜色总数
    int				nClrPhGCount;         //每个颜色的排数量
    int				nValidPrtCtlCount;    //控制器数量  
    UINT			nValidPrtCtlMask;     //有效的控制器掩码
    UINT			nTrigParam;         //0~3 触发参数 0b 表示触发有效电平 
    UINT			nPrtCtlVal;         //  bit3 输出处理提示 bit4 编码器计数反向
    DEVPRTCTL_ITEM     prtctl_item[MCP_DRV_CNT];
	MOTION_PARAM	motorParam;
	INKCTLPARAM		InkCtlParam;
	PHMaintParam	ClearParam;
	UINT            nEncPreFactor;// 20220602 103-1=102
	UINT            nIoOption;  // bit 0 自动供墨使能 bit 2 负压控制使能
	UINT            nPhaseDir; //  喷头相位反向 
	UINT			nReveCfg[100];  //20190828 增加电机参数  128->120 增加墨路控制参数 120->114 增加清洗参数 114->103
	UINT			nCRC;
}SYS_DEVCONFIG,*LPSYS_DEVCONFIG;


typedef struct tag_DevPHCtlParam
{
	int             nClrCnt;     //1Hx色
	int             nClrIndex[8];  //可以不用  1
	int             nPHGIndex[8];  //可以不用  1
    UINT			nWaveFormID;    //波形ID   
    float			fFPWParam[_MAX_PH_FIREPARAM];    //脉宽参数  
    float			fStdVol;                         //喷头基准电压
    float			fVolOutOff[32];      //喷头分区电压 // 就是每个电压框的加减电压
    float			fDestTmpCtl[4];      //喷头控制温度
    UINT			nRevParam[32];
}DevPHCtlParam,*LPDevPHCtlParam;
//系统运行参数
typedef struct tag_spsys_param
{
    UINT          nVersion;
	UINT          nLastModifyTime;
	float   	  fIdleFlashFreq;     //H Z
    float         fBrustFlashCycle;   //间隙闪喷周期       sec
    float         fBrustFlashVT;      //间隙闪喷有效时间   sec
    UINT          nAcOption;          //bit0 1 打印彩条  
	UINT          nPrtCtlDataLKT[4][MCP_DRV_CNT];   //数据对照表 0->1bit      1->2bit 2->3/4bit 
    UINT          nPrtEncOffVal[MCP_DRV_CNT];  
	DevPHCtlParam    phctl_param[MCP_DRV_CNT][_MAX_DRV_PHSET];
	UINT          nXZeroPolSet;// 20230629 128-1=127 0位极性选择
	int           nPowerOnDeley;
	BOOL          bCtlCameraSingle;
	UINT          nCleanFlashGrayValue;   //20220611  114-1=113清洗闪喷的灰度选择
	UINT          nParamRev[124];
	UINT          nCrc32;
}SPSYS_PARAM,*LPSPSYS_PARAM;  

typedef struct tag_passprt_req
{
    UINT		nprttype;		          //0 正常打印  1 转产  2 插打
    UINT		nPrtCtlMask;		      //有效的掩码
    UINT		nSwitchPageCount;		  //转产或插打的总页数条件
	int			nPassPageCount;			  //要打印的页数量  <= _MAX_PAGE_COUNT
	int			nStartPrtEncPos;          //起始打印编码值 
    int			nTailPageIndex;		      //最后跳回的页
    UINT		nCtlFlag;                 //0bit: 1使用打印请求的灰度设置	1bit:1 分页打印间隔打印彩条
    UINT		nPrtPageList[_MAX_PAGE_COUNT];     //0~7 页位置  //8~23 数量 //24~31 Reserver;  //
	UINT        nFreePageMask[2];         //需要释放的PASS页掩码，[0]高32页  [1]低32页
}passprt_req,*lppassprt_req;

/*============================================================================*/
typedef struct tag_PrtInitReq
{

	UINT    	nPrtCtlMask;       		      //有效的掩�?    
	float    	fPrecession;                  //20200420
	int 		nGrayBits;
	UINT    	nCtlFlag;          		      //0bit: 1使用打印请求的灰度设�?   1bit:1 分页打印间隔打印彩条
	UINT 		nPageMaxSize;
	float   	fPrtRollSpd;       		          //单位mm/sec
	UINT    	nGrayDataVal[MCP_DRV_CNT];     //灰度数据设置

	UINT        nRcv[32];
}ryprtInitreq, * lp_ryprtInitreq;

#endif

