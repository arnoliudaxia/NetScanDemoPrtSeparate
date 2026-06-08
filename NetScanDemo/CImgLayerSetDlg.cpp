// CImgLayerSetDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetScanDemo.h"
#include "CImgLayerSetDlg.h"
#include "afxdialogex.h"


// CImgLayerSetDlg 对话框
extern PRTIMG_LAYER g_PrtImgLayer;
IMPLEMENT_DYNAMIC(CImgLayerSetDlg, CDialogEx)

CImgLayerSetDlg::CImgLayerSetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IMG_LAYER_DLG, pParent)
	, m_nLayerIndex(0)
	, m_nStartJetOff(0)
	, m_nPrtDir(0)
	, m_fXPosOff(0)
	, m_fPrtYOffet(0)
	, m_nColorCnts(1)
	, m_fXDPI(720)
	, m_nYDPI(600)
	, m_nStartPassInedx(0)
	, m_nFeatherMode(0)
	, m_nCustomFeatherJets(0)
	, m_nEdgeScalePixel(0)
	, m_fRotateAngle(0)
	, m_fDstXScale(1)
	, m_fDstYScale(1)
	, m_fLayerDensity(1)
	, m_fLayerExtractDensity(1)
	, m_fXScanSpd(0)
	, m_nImgType(0)
	, m_nScanCtlValue(0)
	, m_bDoubleDir(FALSE)
	, m_bYReverse(FALSE)
	, m_bXInsert(FALSE)
	, m_bSwitchByWhite(FALSE)
	, m_bEdgeNoReduce(FALSE)
	, m_bPrtArea(FALSE)
	, m_bGrayBits(FALSE)
	, m_nPassVol(0)
	, m_nMulityInk(0)
{

}

CImgLayerSetDlg::~CImgLayerSetDlg()
{
}

void CImgLayerSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT_LAYER_INDEX, m_nLayerIndex);
	DDX_Text(pDX, IDC_EDIT_STARTJET_OFF, m_nStartJetOff);
	DDX_CBIndex(pDX, IDC_COMBO_PRT_DIR, m_nPrtDir);
	DDX_Text(pDX, IDC_EDIT_LAYER_XOFF, m_fXPosOff);
	DDX_Text(pDX, IDC_EDIT_LAYER_YOFF, m_fPrtYOffet);
	DDX_Text(pDX, IDC_EDIT_LAYER_COLORCNT, m_nColorCnts);
	DDX_Text(pDX, IDC_EDIT_LAYER_XDPI, m_fXDPI);
	DDX_Text(pDX, IDC_EDIT_LAYER_YDPI, m_nYDPI);
	DDX_Text(pDX, IDC_EDIT_LAYER_STARTPASS, m_nStartPassInedx);
	DDX_CBIndex(pDX, IDC_COMBO_FEATHER_MODE, m_nFeatherMode);
	DDX_Text(pDX, IDC_EDIT_LAYER_CUSTON_FEATHER_JET, m_nCustomFeatherJets);
	DDX_Text(pDX, IDC_EDIT_LAYER_EDGESCALE, m_nEdgeScalePixel);
	DDX_Text(pDX, IDC_EDIT_LAYER_ROTATE_ANGLE, m_fRotateAngle);
	DDX_Text(pDX, IDC_EDIT_LAYER_DSTXSCALE, m_fDstXScale);
	DDX_Text(pDX, IDC_EDIT_LAYER_DSTYSCALE, m_fDstYScale);
	DDX_Text(pDX, IDC_EDIT_LAYER_DENSITY, m_fLayerDensity);
	DDX_Text(pDX, IDC_EDIT_INTENSITY, m_fLayerExtractDensity);
	DDX_Text(pDX, IDC_EDIT_LAYER_SCAN_SPD, m_fXScanSpd);
	DDX_CBIndex(pDX, IDC_COMBO_IMGTYPE, m_nImgType);
	DDX_CBIndex(pDX, IDC_COMBO_SCAN_CTL, m_nScanCtlValue);
	DDX_Check(pDX, IDC_CHECK_DOUBLE, m_bDoubleDir);
	DDX_Check(pDX, IDC_CHECK_REVERSEY, m_bYReverse);
	DDX_Check(pDX, IDC_CHECK_INSERT_X, m_bXInsert);
	DDX_Check(pDX, IDC_CHECK_DIR_SWITCH, m_bSwitchByWhite);
	DDX_Check(pDX, IDC_CHECK_EDGE_REDUCE, m_bEdgeNoReduce);
	DDX_Check(pDX, IDC_CHECK_INSTALL, m_bPrtArea);
	DDX_Check(pDX, IDC_CHECK_GRAYBITS, m_bGrayBits);
	DDX_CBIndex(pDX, IDC_COMBO_PASS_VOL, m_nPassVol);
	DDX_CBIndex(pDX, IDC_COMBO_MULITY_INK, m_nMulityInk);
}


BEGIN_MESSAGE_MAP(CImgLayerSetDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CImgLayerSetDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CImgLayerSetDlg 消息处理程序


void CImgLayerSetDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	g_PrtImgLayer.nLayerIndex = m_nLayerIndex;
	g_PrtImgLayer.nStartJetOffset = m_nStartJetOff;
	g_PrtImgLayer.nPrtDir = m_nPrtDir;
	g_PrtImgLayer.fPrtXOffet = m_fXPosOff;
	g_PrtImgLayer.fPrtYOffet = m_fPrtYOffet;
	g_PrtImgLayer.nInkMultication = m_nMulityInk + 1;
	g_PrtImgLayer.nPassVolRate = m_nPassVol + 1;
	if(m_nColorCnts<1)
		m_nColorCnts=1;
	else if(m_nColorCnts>16)
		m_nColorCnts=16;
	g_PrtImgLayer.nColorCnts = m_nColorCnts;
	g_PrtImgLayer.nXDPI = m_fXDPI;
	g_PrtImgLayer.nYDPI = m_nYDPI;
	g_PrtImgLayer.nStartPassIndex = m_nStartPassInedx;
	g_PrtImgLayer.nFeatherMode = m_nFeatherMode;
	g_PrtImgLayer.nCustomFeatherJets = m_nCustomFeatherJets;
	g_PrtImgLayer.nEdgeScalePixel = m_nEdgeScalePixel;
	g_PrtImgLayer.fRotateAngle = m_fRotateAngle;
	g_PrtImgLayer.fDstXScale = m_fDstXScale;
	g_PrtImgLayer.fDstYScale = m_fDstYScale;
	g_PrtImgLayer.fLayerDensity = m_fLayerDensity;
	g_PrtImgLayer.fEdgeDensity = m_fLayerExtractDensity;
	g_PrtImgLayer.fXScanSpd = m_fXScanSpd;
	g_PrtImgLayer.nScanCtlValue = m_nScanCtlValue;
	g_PrtImgLayer.nImgType = m_nImgType;
	g_PrtImgLayer.nPrtFlag &= ~0xff;
	if (m_bDoubleDir)
		g_PrtImgLayer.nPrtFlag |= 0x1;
	if (m_bYReverse)
		g_PrtImgLayer.nPrtFlag |= 0x2;
	if (m_bXInsert)
		g_PrtImgLayer.nPrtFlag |= 0x4;
	if (m_bSwitchByWhite)
		g_PrtImgLayer.nPrtFlag |= 0x8;
	if (m_bEdgeNoReduce)
		g_PrtImgLayer.nPrtFlag |= 0x10;
	if (m_bPrtArea)
		g_PrtImgLayer.nPrtFlag |= 0x20;
	if (m_bGrayBits)
		g_PrtImgLayer.nGrayBits = 2;
	else
		g_PrtImgLayer.nGrayBits = 1;
	CDialogEx::OnOK();
}


BOOL CImgLayerSetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_nLayerIndex = g_PrtImgLayer.nLayerIndex + 1;
	m_nStartJetOff = g_PrtImgLayer.nStartJetOffset;
	m_nPrtDir = g_PrtImgLayer.nPrtDir;
	m_fXPosOff = g_PrtImgLayer.fPrtXOffet;
	m_fPrtYOffet = g_PrtImgLayer.fPrtYOffet;
	m_nColorCnts = g_PrtImgLayer.nColorCnts;
	m_fXDPI = g_PrtImgLayer.nXDPI;
	m_nYDPI = g_PrtImgLayer.nYDPI;
	m_nStartPassInedx = g_PrtImgLayer.nStartPassIndex;
	m_nFeatherMode = g_PrtImgLayer.nFeatherMode;
	m_nCustomFeatherJets = g_PrtImgLayer.nCustomFeatherJets;
	m_nEdgeScalePixel = g_PrtImgLayer.nEdgeScalePixel;
	m_fRotateAngle = g_PrtImgLayer.fRotateAngle;
	m_fDstXScale = g_PrtImgLayer.fDstXScale;
	m_fDstYScale = g_PrtImgLayer.fDstYScale;
	m_fLayerDensity = g_PrtImgLayer.fLayerDensity;
	m_fLayerExtractDensity = g_PrtImgLayer.fEdgeDensity;
	m_fXScanSpd = g_PrtImgLayer.fXScanSpd;
	m_nScanCtlValue = g_PrtImgLayer.nScanCtlValue;
	m_nImgType = g_PrtImgLayer.nImgType;
	m_bDoubleDir = (g_PrtImgLayer.nPrtFlag & 0x1) ? TRUE : FALSE;
	m_bYReverse = (g_PrtImgLayer.nPrtFlag & 0x2) ? TRUE : FALSE;
	m_bXInsert = (g_PrtImgLayer.nPrtFlag & 0x4) ? TRUE : FALSE;
	m_bSwitchByWhite = (g_PrtImgLayer.nPrtFlag & 0x8) ? TRUE : FALSE;
	m_bEdgeNoReduce = (g_PrtImgLayer.nPrtFlag & 0x10) ? TRUE : FALSE;
	m_bPrtArea = (g_PrtImgLayer.nPrtFlag & 0x20) ? TRUE : FALSE;
	m_bGrayBits = (g_PrtImgLayer.nGrayBits == 2) ? TRUE : FALSE;
	m_nMulityInk = g_PrtImgLayer.nInkMultication - 1;
	m_nPassVol = g_PrtImgLayer.nPassVolRate - 1;
	UpdateData(FALSE);
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
