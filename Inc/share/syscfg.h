#ifndef __RETC_PROJECT_TAGPRT_SYSCFG_091116
#define __RETC_PROJECT_TAGPRT_SYSCFG_091116
//#include "license-2.h"
#include "RyNetPrtData.h"
static int DOT_TO_MM(int X,float DPI)
{
	//ASSERT(DPI!=0);
	float nI=0.5;
	if(X<0)
		nI=-0.5;
	return(int)(((float)X*25.4)/DPI+nI);
}
static float DOT_TO_MM2(int X,float DPI)
{
	//ASSERT(DPI!=0);
    int  nresult;
	float nI=0.05f;
	if(X<0)
		nI=-0.05f;
	nresult= (int)(10*(((float)X*25.4)/DPI+nI));
	return ((float)nresult)/10;
}
static float DOT_TO_MM3(int X,float DPI)
{
	return ((float)X*25.4f)/DPI;
}
static float DOT_TO_MM4(float X,float DPI)
{
	return ((float)X*25.4f)/DPI;
}
#define MM_TO_DOT(X,DPI)  (float)(((float)(X*DPI))/25.4)
/*
typedef struct tag_prtctl_data
{
	BOOL bPrtValid;     //打印输出有效的喷头 // 在ImgPrtProc判断不有效不处理，在分配打印控制器有效，或者PrtPHAdjust中设置有效，受启用模组影响

	UINT nState;          //0 NULL 1 ProcData 2:Transfered 3:AddToPrtList 4: // 没用到
	int  nNetPrtSvrIndex; //所属网络主卡
	int  nMcPrtCtlIndex;  //所属网络主卡的控制器编号
	int  nPrtLines;       //分配的打印线   //0:未分配  1 A线  2 B线
	PRTCTL_ITEM    prtctlitem;
	PHCTL_PARAM    phsetptr[_MAX_DRV_PHSET];

	LPBYTE  pImgSrcDataPtr;    //打印数据缓存
	UINT    nImgSrcType;        //数据来源方式 0 自主分配  1.共享主卡分配 2.共享应用分配 
	UINT    nDataSize;      //打印数据大小 // 就是一个驱动卡传数据的大小
	UINT    nDestColByte;   //列数据字节长度 //  一个驱动卡的喷头一列的长度（两排喷嘴，1个或者2个喷头加起来的）

	UINT nImgSrcColumn;     //截图数据源列数
	UINT nImgPrtColumn;     //打印列数 // 等于图像列数加上x最大偏移列数
	UINT nCbPrtColumn;      //彩条列数
	UINT nXPrtOff;          // 这个打印控制器的x的最小位置 用来设置延时
}RYPRTCTL_ITEM,*LPRYPRTCTL_ITEM; // 有8个打印控制器,不下载到板卡 */
typedef struct tag_PHPrtJetUnit
{
	int nClrIndex;
	int nCmbJetRows;
	int nJetYDPI;
	int nStartJetIndex;   //有效起始喷嘴
	int nValidJets;       //有效喷嘴
	int nStartJetPassPos; //起始嘴在PASS嘴的偏移量
	int nTopOverJets;
	int nBottomOverJets;
	//UINT nLJRowDataVerDirMask; 
	LPBYTE  pImgSrcDataPtr;    //打印数据缓存
	
	int nJetRowDataCID[_MAX_PHROWCNT];
	int nJetRowXImgPixelOff[_MAX_PHROWCNT][2];//0 负方向 1正方向 每排嘴的数据偏移
}PHPrtJetUnit,*LPPHPrtJetUnit;
typedef struct tag_PHSET_PROPERTIES
{
	UINT           nJetPartUnits;  //打印单元数量 
	PHPrtJetUnit   phprtunit[8];
	//UINT		   nDataSize;      //打印数据大小 // 就是一个驱动卡传数据的大小
	UINT		   nRevParam;
}
PHSET_PROPERTIES,*LPPHSET_PROPERTIES;
//每个控制器只允许使用一种喷头
/*
typedef struct prt_ctl_dataset
{
	UINT              nState;
	LPDEVPRTCTL_ITEM  pdevItem;	
	int				  nNetPrtSvrIndex; //所属网络主卡
	int				  nMcPrtCtlIndex;  //所属网络主卡的控制器编号
	//UINT		      nLJRowDataVerDirMask;    //控制器各通道数据传输方向  LPDEVPRTCTL_ITEM已存在无需重复
	UINT              nDestColByte;
	int               nXEncPrtPos[2];          ///0 负方向 1正方向  控制器涉及的喷头打印位置，单位编码器值
    PHSET_PROPERTIES  phset[_MAX_DRV_PHSET]; //各喷头的打印描述
}PRTCTL_DATAITEM,*LPPRTCTL_DATAITEM;         // 8个打印控制器    */


typedef struct tag_clrgroup_item
{
	bool            bValid;
	char            szClrName[32];
	COLORREF        nClrValue;      //颜色表示
	unsigned int    nGlobalInkIndex; //全局墨水号
	unsigned int    nPHType;
	unsigned int    nDrvHwType;
	unsigned int    nWaveCtlType;   //波形方式 0 无波形需求 1:每次上电加载  2:存储于喷头或驱动板卡
	unsigned int    nPHSplitClrs;   //1Hx色
	unsigned int    nSubPHIndex;    //1H多色时，子喷头号
	unsigned int    nRelateSubPhCnt; //单色使用到的子喷头数量
	unsigned int    nPHGCount;      //喷头组数
	unsigned int    nPHPerDrvCard;   //每块驱动卡可驱动的喷头数量
	unsigned int    nCmbJetRows;     //合成DPI的喷嘴排数
	unsigned int    nPHGJetcount;    //每个喷头颜色单元喷嘴数
	unsigned int    nPrtTrigSrc;    //打印触发源 0无触发软件启动  1~3触发信号选择
	unsigned int    nInstallDir;    //bit0:0:反向安装/1:正向安装 (bit1:相位反向 X2001使用) bit2:1号喷头的位置(0:居左 1:居右)
	unsigned int    nGroupOverJets; //喷头间重嘴数
	float           fPHGdpi;        //每个喷头颜色单元的物理DPI;
	float           fXPos;          //离最靠近零位的喷头X距离 mm
	float           fYPos;          //颜色组Y位置 mm
	float           fInnerPhGdis;   //组内距 mm
	float           fMinVoltage;    
	float           fMaxVoltage;
	float           fDefaultVoltage;
	float           fDefaultTempValue;
	float           fPHG_stdXpos[MAX_GROUP]; //组内各喷头打印位置
	unsigned int    PH_CTL_WORD[16];         //喷头控制字
	int             nReverse[48];            //备用参数
}clrgroup_item,*lpclrgroup_item;
typedef struct tag_DevConfig
{
	UINT              nVersion;
	UINT              nConfigID;
	clrgroup_item     clr_phgroup_item[MAX_COLORS];
	float             fSysEncdpi;                 /*系统编码器DPI精度*/   
	unsigned int      nEncPreFactor;              /*编码器前置分频*/   //0x10
	unsigned int      nSysClrcnts;                /*系统颜色数量*/      	
	int               nNetMcpCount;          //网络卡数量
	unsigned int      nNetMainMcpMask;           //主网络卡掩码
	unsigned int      nNetMcpHwIndex[256];     //硬件ID  [0~29] 网卡号 [31bit] 1 主端模式  //0x50
	int               nPHMcpLKTArray[MAX_COLORS][MAX_GROUP];   //使用网络打印卡号
	int               nPHDrvLKTArray[MAX_COLORS][MAX_GROUP];   //使用的网络打印卡驱动通道号/打印控制器号
	int               nPHDrvSubID[MAX_COLORS][MAX_GROUP];      //驱动卡喷头通道号
	int               nPHG_stdYjetpos[MAX_COLORS][MAX_GROUP];   //每喷头组的Y标准位置 //unit:Jets 
	UINT              nPHGSplitPhyChaLkTbl[MAX_COLORS][MAX_GROUP][MAX_JETROW];   //喷头数据逻辑单元的物理数据通道表 // 如果第一个喷头安装方向右侧，单数喷头第一排喷嘴这个表就是0，第二排就是1，双数喷头第一排喷嘴这个表就是1，第二排就是0  // 如果第一个喷头安装方向左侧，单数喷头第一排喷嘴这个表就是1，第二排就是0，双数喷头第一排喷嘴这个表就是0，第二排就是1 // 初始化的时候有这样的规律：1，0，2，3，6，5，7，8 // 0123,3210 // 就是喷嘴排的先后顺序
	float             fClrModuleXpos[MAX_COLORS];                  //每个颜色模组的位置 单位mm 用于SYSCFG显示计算
	float             fClrModuleYpos[MAX_COLORS];                  //每组喷头的Y偏差 单位mm   用于SYSCFG显示计算
	BOOL              bPrtScanDir;                                 //正向打印方向  //FALSE: 从左往右 TRUE 从右往左 // 应该会影响到喷头喷嘴间距的正反顺序，要结合喷头安装方向// 喷头安装方向和出砖方向会影响std_fPhSpliteOff（喷头单元物理偏差） // 还会影响图像生成的左右顺序
	BOOL              bValidTrigSingal;                            //有效的触发电平 // 没用到，用nTrigSensorActiveMode代替了
	float             std_fPhSpliteOff[MAX_COLORS][MAX_JETROW];                //喷头内物理通道标准偏差  //喷头单元物理偏差   //20191129   增加480*sizeof(float)
	UINT              nSysFunCtl;   //bit0 喷头组类型独立  bit1:编码器计数反向  bit2: EAIB使用  bit3:ADIB使用 
	UINT              nAppUserMode; // 工作模式  0分页触发 1：连续循环 2:扫描打印
	UINT              nFuncardCount; //功能卡数量
	UINT              nFunCtlHwID[4]; //功能卡ID  //[0:15]HWID [16~19]Fun Type  0:EAIB  1:ADIB
	UINT			  nMaxFireFreq;//nPrtCtl_DCHA_cnt;   //控制器使用喷头数  20200418 264-1 20200630 447->432
	UINT              nReserver[640];//20190821 增加EAIB  1173->1170   20191129   465=1170-705  20191130：450=465-15  20200503 463->447
	UINT              nCrcChk;
}DEV_CONFIG,*LPDEV_CONFIG;

typedef struct tag_ClrInkGrayItem
{
	unsigned int    nGrayDataBits;          
	unsigned int    nGrayDataTbl[4];        //每级灰度的对照表 //0:1BIT 1:2BIT 3~4BIT LOW8 3:4BIT HIGH8:
	unsigned char   nGrayBinData[4][16];    //每级灰度的对应数据 //0:1BIT  1:2BIT  2:3~4BITBIT 
 	float			fPixelDropSize[4][16];  //每种像素的墨点大小 单位PL 
	char            szClrName[16];
}CLRINK_GRAYITEM,*LPCLRINK_GRAYITEM;

typedef struct tag_GrayPrtCfg
{
	unsigned int      nFileHeader;      
	unsigned int      nInkCfgCount;		//有效配置的数量 // 大概是gs6,gs12,gs40这种的数量，3个
	CLRINK_GRAYITEM   clrink_gray[16];
	unsigned int	  nCRC32;
}PRTGRAY_CONFIG,LPPRTGRAY_CONFIG;

typedef struct tag_DevSysParam
{
	UINT              nSysParamVer;                                    //参数版本
	UINT              nParamSize;                                      //参数数据字节数
	RYUSR_PARAM       user_pram;
	DEV_CONFIG        sys_config;
	UINT              nGrayCfgIndexTbl[MAX_COLORS]; // 各个模组灰度类型后面的组合框的号码，gs6，gs12，gs40
	UINT              nNetIndexIP[MAX_MCP_CNT];  //0 未分配，可以不分配 1，必须分配，等待中 其他物理IP
	PRTGRAY_CONFIG    gray_cfg;
	MOTION_PARAM      MottoParam;//暂时不用
	INKCTLPARAM		  InkCtlParam;//暂时不用
	PHMaintParam	  ClearParam;//暂时不用
	UINT              nSysCtlFlag;//系统控制参数 20210311 1986->1985 bit6 EPSON喷头允许打印波形结束时多一个CH  bit7 Eaib使用相机触发 bit8 连接喷码机
	//bit9 使用相机指定打印页  bit10,只有打印图区域的喷头打彩条
	DWORD             dwReserver3[3799];
	DWORD             nCrcChk;
}RYPRT_SYSPARAM,*LPRYPRT_SYSPARAM;

#endif