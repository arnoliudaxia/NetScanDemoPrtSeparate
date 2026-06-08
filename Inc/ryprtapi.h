#ifndef  _RETC_PROJECT_EXTERNPARAM_231116
#define  _RETC_PROJECT_EXTERNPARAM_231116



#define  MAX_MCP_CNT         8
#define  MAX_PRTCTL_CNT      8
#define  MAX_COLORS         16
#define  MAX_GROUP          16
#define  MAX_JETROW         32  
#define  MAX_DRVCHCNT       32
#define  DIR_COUNT          2
#define  MAX_CHANNAL_SIZE   1024  
#define  MCP_DRV_CNT		8
#define  MAX_PH_CNT         256
#define  _PHXPOS_RESIZE		128
//校准数据回调函数，用于修改自动生成的校准图内容
typedef  int  (*LPRY_CALI_PHGDCall)(unsigned char* pPhgData,int c,int g,int nWidth,int Height,int nByteLines,int nGrayBits);

//图层描述
typedef struct tag_ImgLayer
{
	long		nLayerIndex;	               //图层索引
	int			nStartJetOffset;               //图像起始对应喷嘴偏移量，实现不同层同一位置使用不同喷嘴打印 
	int			nPrtDir;                       //图像打印起始PASS方向//  0 编码负方向 1编码正方向
	float		fPrtXOffet;                    //图层针对任务X起点偏移 单位 mm
	float		fPrtYOffet;                    //图层针对任务Y起点偏移 单位 mm
	int			nColorCnts;                    //颜色通道数量
	float	    nXDPI;				           //图像XIDPI
	long	    nYDPI;				           //图像YIDPI 
	long	    nBytesPerLine;		           //每行数据字节数
	long	    nHeight;			           //图像高度  单位：像素
	long	    nWidth;				           //图像宽度  单位：像素
	long	    nGrayBits;			           //图像数据灰度位数 1 1bit, 2 2bits 
	unsigned int    nStartPassIndex;           //起始PASS，用于一层打印的恢复
	unsigned int    nFeatherMode;              //羽化方式  0 不羽化 1~6分别Level->Level6 羽化程度逐步增加 7自定义
	unsigned int    nCustomFeatherJets;        //自定义羽化喷嘴数
	int				nEdgeScalePixel;	       //图像边界像素;//>0 边缘增加像素 <0 边缘减少像素数 =0 边缘不变化
	float			fRotateAngle;			   //以图像中心为参考的角度旋转  弧度制 逆时针为正角度(注意图像数据起始是在图底部)
	float			fDstXScale;                //X向输出缩放系数  
	float			fDstYScale;                //Y向输出缩放系数  
	float           fLayerDensity;             //层像素密度  >=1 100%的输出/ 0<密度<1.0: 0~100%的像素抽点输出
	float           fXScanSpd;                 //X扫描速度 mm/s   
	unsigned int    nScanCtlValue;			   //扫描计算方式 0 PASS逐步叠加  1:喷头重复覆盖+微步进 2
	unsigned int 	nImgType;				   //源文件图片类型 0：BMP 1:PRT 2:CLI（启动任务时给定了CLI文件路径，层打印不需要再指定数据） 
	unsigned int    nPrtFlag;                  //bit0:  双向打印 bit1: Y反方向打印 bit2:X向插点   bit3: 方向切换依据跳白计算  bit4:边缘单独抽点 bit5:喷头覆盖控制在出图区域(防安装干涉) bit6:起始方向采用最近打印的反方向,bit7缩放有效 bit16:数据分配指定颜色通道
	unsigned int    nValidClrMask;             //有效打印颜色 指定某些颜色参与数据PASS计算
	unsigned char   nDataClrIndex[16];         //每个数据通道对应的颜色号
	unsigned int    nInkMultication;		   //INK 墨量输出倍率 //1:原始墨量 2~8: 墨量倍数 
	unsigned int    nPassVolRate;              //PASS网点抽点比率 >1 2:50% 3:33.3% 4:25% 5:20% 6:16.6% 8:12.5%
	float        	fEdgeDensity;              //边界单独抽点的密度 >=1 100%的输出/ 0<密度<1.0: 0~100%的像素抽点输出
}PRTIMG_LAYER,*LPPRTIMG_LAYER;
//PASS描述
typedef struct tag_PassDataItem
{
	unsigned int	nLayerIndex;		//所属图层号
	unsigned int	nLayerPassCount;	//所属图层PASS总数
	unsigned int    nLayerPassIndex;	//PASS序号
	unsigned int	nValidPassJets;		//PASS有效喷嘴数	20180818 新增
	unsigned int    nProcState;			//图层处理状态 //0 数据处理阶段  1源数据处理就绪  2数据行列转换结束  3 数据已写入硬件  4 已添加到打印队列  5打印已结束 6 打印内存已释放 7ERROR
	float			fYMinJetImgPos;     //Y向偏差最小喷嘴号离任务Y起点的距离 单位:mm
	bool            bEncPrtDir;		    //打印方向  0 编码负方向 1正方向
	bool            bScanPrtDir;	    //车头运动的扫描方向  false: 从右往左 true:从左往右
	bool            bIsNullPass;        //空PASS标志 跳白标志
	int             nStpVector;         //打印此PASS前需要的步进量  单位 um >0 Y正步进 <0 Y负步进
	int				nValidStartCols;	//扫描的X起始数据列，相对扫描正方向第一列，用于X向跳白动作计算
	int				nValidEndCols;		//扫描的X结束数据列，相对扫描正向最后一列，用于X向跳白动作计算
	int             nImgColumns;        //图像数据的有效列数
	int             nXPrtColumns;       //实际需要打印的列数
	unsigned int    nStartEncPos;		//起始位置 ，编码值
	float			fPrtPrecession;		//打印分频值
	int             nErrorResult;       //出错码
	unsigned int    nDeviceIndex;       //PASS所属设备Index,用于多设备支持 20240722
	int             nReverse[31];
	void*           pInnerParam;        //内控参数地址，
	tag_PassDataItem* pPrevItem;        //前PASS
	tag_PassDataItem* pNextItem;
}PassDataItem,*LPPassDataItem;
//任务描述
typedef struct tag_PrtJobItem
{
	unsigned int	nJobID;
	char			szJobName[64];
	char            szJobFilePath[260];	//涉及需处理的3D文件路径 用于CLIl类似文件解析
	unsigned int    nFileType;          //文件类型 0：任务无文件指定，通过图层增加　１：ＣＬＩ文件
	float       	fPrtXPos;	        //任务的X向起打位置,距离零位的距离
	float			fPrtYPos;           //保留 离Y零位位置
	float			fClipWidth;         //打印截图尺寸 Width  unit:mm                        
    float			fClipHeight;        //打印截图尺寸 Height unit:m
	float           fOutXdpi;           //需要数据解析文件的XDPI
	float           fOutYdpi;           //需要数据解析文件的YDPI 
	unsigned int    nOutPixelBits;      //需要数据解析文件灰度指定        
	unsigned int	nPrtCtl;		    //bit0 跳白支持  bit1 循环喷嘴偏移  bit2 每层起点随机偏移嘴 bit4 X镜像 bit5 Y镜像 bit6 两倍YDPI打印 bit7按截图尺寸打印 
}PRTJOB_ITEM,*LPPRTJOB_ITEM;
typedef struct tag_printhead_info
{
    UINT   nPhType;           //喷头类型
    int    nVolCnt;
    int    nVolQueryCnt;
    int    nTmpCtlCnt;
    int    nTmpQueryCnt;
    int    nJetCounts;
    int    nJetRows;
    int    nCmbJetRows; //FireWare 使用数量
    int    nRowDataLines;
    int    nRowDataLineOff;
	UINT   nLJRowDataVerDirMask;
    float  fPhyDPI;
}RYPHI_INFO,*LPRYPH_INFO;
//喷头运行信息
typedef struct tag_PHRUN_INFO    
{
	unsigned int    nWaveID;          //使用的波形ID
	unsigned int    nPhIndex;         //喷头号
	unsigned int    nPhType;          //喷头类型
	unsigned int    nDataLKT;         //数据输出对照表
	int				nClrUseCnt;       //喷头配置了颜色数量 
	int             nClrIndex[8];     //颜色配置  一个喷头可能涉及多个颜色
	int             nGroupIndex[8];   //组序号  每个颜色所在组号
	char			szWaveName[32];   //使用的波形名称
	char            szSerialID[32];   //喷头序列号
	float           fCurVol[32];      //电压[0:3]ABCD
	float           fCurTemp[4];      //温度  [0]喷头加热温度  
	unsigned int    nRevParam[16];    //备用参数
}PHRUN_INFO,*LPPHRUN_INFO;
//设备驱动卡运行信息
typedef struct tag_DRVINFO
{
	unsigned int 	nState;			//连接状态 1 表示已连接
	unsigned int	nNextState;		//连级下个驱动卡连接状态
	unsigned int 	nSignature;		//驱动卡序列号
	unsigned int 	nPCBVersion;	//驱动卡PCB版本
	unsigned int 	nFMVersion;		//驱动卡MCU版本
	unsigned int 	nFpgaVersion;	//驱动卡控制程序版本
	unsigned int	nPtvwarnState;  //1喷头不存在 2,8喷头温度超过控制范围
	unsigned int    nPhLinkCount;         //连接的喷头数
	unsigned int    nPhLinkMask;          //喷头连接的通道掩码
	unsigned int    nDrvType;             //驱动卡类型
	PHRUN_INFO      phinfo[4];            //喷头运行信息
	float           fDrvTmp[2];           //板卡温度
	unsigned int    nRevParam[16];
}DRVINFO,*LPDRVINFO;
//打印运行信息
typedef struct tag_PrtRunInfo
{
	bool			bJobPrtRuning;			//作业打印运行中  true 运行中
	bool			bLayerPrtIsOver;		//当前图层打印完成标志 true 当前图层打印完成
	int				nPrtState;				//板卡打印控制器状态 //打印状态 [0:3] 驱动卡1 [4:7] 驱动卡2 [8:11] 驱动卡3 [12:15] 驱动卡4 [16:19] 驱动卡5  [20:23] 驱动卡6  [24:27] 驱动卡7  [28:31] 驱动卡8  //VAL 0 IDLE 1 Print 2 Pause 3 Flash 
	int				nPrintLayerIndex;		//当前打印的图层
	int				nLayerPassCount;		//当前打印图层PASS总数
	int				nPrintPassIndex;		//当前打印的PASS
	int				nCurPrtDir;				//当前的打印方向
	unsigned int    nPrtEncHighVal;         //打印编码器高位值
	unsigned int    nPrtEncoderValue;       //打印编码器低32位值 (扫描系统只有32位编码值）
	unsigned int	nRevPrtCols;			//当前剩余的打印列数
	int				nProcLayerIndex;		//当前数据处理的图层
	int				nDTLayerPassIndex;		//当前数据传输的图层
	int             nReverse[16];
}PrtRunInfo,*LPPrtRunInfo;

// ADIB板卡控制参数
typedef struct tag_ADIB_PARAM
{
	UINT	nFMver;				//程序版本号
	float	fcurvoltage[8];		//当前电压
	float	fcurInkTankTemp[8];	//当前温度
	float	fcurAirPress[8];	//当前负压
	UINT	nLgStatus;			//保留输入
	UINT	nVolState;			//当前电压状态
	struct	I2C_DATA { float	fvoltage[8]; float	fInkTankTemp[8]; float	fAirPress[8]; }I2C;	//I2C保存的数据
	float	fAirThreshold;		//负压阈值
	float   fPressInkTime;      // 设置压墨时间
	UINT    nStatus;            // 板卡状态
	float	fInkSupplyTime;		//
}ADIB_PARAM, * LPADIB_PARAM;
typedef struct tag_InkCtlInfo
{
	float fCTLTmp;
	float fInkTemperature[2];
	float fInkAirPressure[2];
	float Rev[2];
}INKCTLINFO, * LPINKCTLINFO;

//打印机设备信息
typedef struct tag_Printer_Info
{
	unsigned int	nVersion;	                    // 打印机软件系统版本号
	unsigned int	nCustomerID;                    // 客户号
	unsigned int	nXSysEncDPI;			        // 系统光栅DPI
	unsigned int    nMcpCount;						// 网络打印卡数量
	unsigned int    nMaiMcpIndex;                   // 网络主卡索引
	unsigned int	nMcpValidMask;				    // 网络打印卡卡的掩码
	unsigned int    nMcpDeviceID[MAX_MCP_CNT];      // 网络打印卡设备号
	unsigned int	nMcpFpgaVer[MAX_MCP_CNT];       // 网络打印卡FPGA版本 
	unsigned int	nMcpMcuVersion[MAX_MCP_CNT];	// 网络打印卡卡MCU版本
	unsigned int    nMcpAppVersion[MAX_MCP_CNT];    // 网络打印卡APP版本
	unsigned int	nMcpDrvLinkCount[MAX_MCP_CNT];  // 驱动卡连接数量 nMcpDrvLinkCount[0] = 8;
	unsigned int	nMcpDrvLinkMask[MAX_MCP_CNT];   // 驱动卡连接掩码 nMcpDrvLinkMask[0] = 0xff;
	unsigned int    nMcpConfigPrtMask[MAX_MCP_CNT]; // 每个主卡的喷头配置码
	unsigned int  	nSysStatus;						// 系统状态  0 未初始化  1 以初始化待机 2 打印中   (10~1000)系统错误
	unsigned int  	nPrintStatus;					// 0 待机  1打印 2 暂停  3 待机闪喷
	unsigned int    nSysColors;                     // 系统配置颜色数
	unsigned int    nMaxFireFreq;                   // 最大扫描频率
	unsigned int    nLicenseResult;                 // 系统授权结果
	unsigned int    nClrPHGroups[MAX_COLORS];       // 每个颜色的喷头数  
	unsigned int    nCMHomgPosition;					     // 车头零位位置   0:在左边 1:在右边
	int             nPhIDLKT[MAX_COLORS][MAX_GROUP];		 // 喷头的ID查找表	//bit0~bit15: PHID bit16:~bit19 subPhID(1头多色)	
	int             nDevIndexLKT[MAX_COLORS][MAX_GROUP];     // 颜色喷头组的控制卡位置对照表 [23:16]喷头通道位置 [15:8]驱动卡位置 [7:0]主卡位置
	int				nHWInitResult;							 // 硬件初始化结果
	int             nLastError;                              // 最近的错误号
	int             nLastMcpIndex;                           // 最近发生错误的控制卡
	int             nLastErrClrModule;
	int             nLastErrPhgIndex; 
	unsigned int    nMcpOnLineMask;                          //网络卡有效连接在线的卡掩码 
	RYPHI_INFO      clrph_info[MAX_COLORS];                  //每个颜色使用的喷头信息
	DRVINFO			sysDrvInfo[MAX_MCP_CNT][MCP_DRV_CNT];	 //驱动卡状态信息
	PrtRunInfo      prt_rtinfo;       
	INKCTLINFO      InkCtlInfo[MAX_MCP_CNT];                 // 负压外部温度状态
	int             nIoState[MAX_MCP_CNT];                   // 输入输出状态
	ADIB_PARAM      nAdibInfo;
}PRINTER_INFO,*LPPRINTER_INFO;
//喷头控制参数
typedef struct tag_PHCTL_PARAM    
{
	//int     nPhGJetYOff;                      //Y向偏移量  单位喷嘴数
	float   fStdVoltage;                      //喷头基础电压  
	float   fVoltage[32];                     //电压[0:3]ABCD
	float   fDestTemp[4];                     //温度  [0]喷头加热温度
	unsigned int  nRevParam[16];         
}PHCTL_PARAM, *LPPHCTL_PARAM;



//校准参数
typedef struct tag_CalbrationParam
{
	unsigned int  nAdjType;   //0 喷嘴状态图 1垂直校准 2步进 3 X喷头组套色 4 X内距套色 5往返套色
	unsigned int  nCtrlValue; //校准控制字;
	float         fPrtXPos;          //打印位置
	float         fPrtYPos;          //打印位置
	float         fxadjdpi;         //校准打印的DPI
	int           nGrayBits;        //采用灰度位数
	int           nPrtDir;          //校准图打印方向 0 负反向 1正方向
	float         fStpSize;         //步进校准单位量，Y向的步进长度，单位mm
	float         fXRunSpd;         //mm/s  车头运行速度
	float         fXMaxPrtWidth;    //设备最大打印宽度
	float         fYMaxPrtHeight;
	unsigned int  nReverse[32];
}
RYCalbrationParam,*LPRYCalbrationParam;
//用户态参数
typedef struct tag_colorbar_param
{
	float  fCBDistance;     //离画面距离
	float  fCBWidth;        //彩条宽度
	float  fCBInterval;     //彩条位置间隔
	float  fCBInkDenisty;   //彩条墨量密度
	int    nPrtPeriod;      //出彩条周期，没打印几次出一次彩条
	unsigned int nCBValidMask; //喷彩条涉及的颜色掩码
	unsigned int nCBPosMode;  // 彩条位置模式 0 无彩条 1画面左端 2画面右端 3画面两端 4固定位置
	unsigned int nCBOption;   //bit0:与画面等距
}colorbar_param,*lpcolorbar_param;
typedef struct tag_RYUSR_SYSPARAM
{
	long			nParamVer;			//参数版本
	long			nParamSize;			//参数字节数
	unsigned int    nPhgValidCtl[MAX_COLORS][MAX_GROUP];                // 每喷头组打印允许
	int             nPhgYJetOffset[MAX_COLORS][MAX_GROUP];              // Y向偏差
	int             nOverJetInc[MAX_COLORS][MAX_GROUP];                 // 重叠处理喷嘴偏移量 //不影响Y数据位置 头部
	int             nPhgXGroupOff[MAX_COLORS][MAX_GROUP][DIR_COUNT];    // 喷头组套色偏移 //Dir 0 负方向 1正反向  单位:打印编码器数值
	int             nPhgJetRowOff[MAX_COLORS][MAX_GROUP][MAX_JETROW][DIR_COUNT]; //组内套色偏移 [MAX_JETROW逻辑嘴] 单位:打印编码器数值
	unsigned int    nLackJetCount[MAX_COLORS][MAX_GROUP];               // 缺嘴数量
	unsigned int    nLackJetTbl[MAX_COLORS][MAX_GROUP][32];             // 缺嘴索引表，最多32个缺嘴
	float           fPrtScanSpd;                                        // 打印扫描速度 mm/s
	PHCTL_PARAM     phctl_param[MAX_PH_CNT];
	colorbar_param  clrbar_param;
	int		        nBiDirEncPrtOff;            // 当前打印使用的双向偏差 编码
	unsigned int	nLayerJetMcUint; 			// PASS层启动偏移微动时变化单位Jet
	unsigned int	nLayerMaxRevJets;			// PASS预留空嘴用于支持层循环微动和随机最大嘴
	unsigned int	nMicroStpJet;		        // 20220103 微步进扫描喷嘴数
	unsigned int	nOverlapJetProcType[MAX_COLORS];             // 重叠嘴处理方式   0 后喷头舍弃与前喷头重叠部分 1:渐变 2:交叉
	float			fBrustCycleSec[MAX_COLORS];	                 // 闪喷的周期
	float			fBrustValidSec[MAX_COLORS];				     // 闪喷的有效时间
	float			fFlashPrtFrequecy[MAX_COLORS];				 // 闪喷频率
	unsigned int    nSysFunOption;                  //bit0 :待机自动闪喷0数据 bit 1 编码计数反向 bit 2使用adib板卡  bit 4 x2001启用大墨量模式
	char			szLogPath[260];		            //日志文件路径
	unsigned int    nValidColorMask;                //允许输出的颜色掩码
    unsigned int    nIoOption;                      //bit0 使用虚拟编码测试模式 bit1 跳白考虑UV的覆盖范围
    float           fUVPHDistance;                  //UV灯离0喷嘴的距离 用于跳白时UV覆盖范围的计算 20240610
	unsigned int    nRevParam1[6];
	float           fAirCtlPress[MAX_MCP_CNT][2];
	float           fSafeAirPress[MAX_MCP_CNT][2];
	float           fInkTemp[MAX_MCP_CNT][2];
	unsigned int	nRevParam[41];		            //20211007 20200819 20191101 126->110->109->108->92->91 //20240315 96->80 温度气压控制 // 
}RYUSR_PARAM,*LPRYUSR_PARAM;
#endif
