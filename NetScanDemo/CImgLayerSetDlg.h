#pragma once


// CImgLayerSetDlg 对话框

class CImgLayerSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImgLayerSetDlg)

public:
	CImgLayerSetDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CImgLayerSetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMG_LAYER_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	long m_nLayerIndex;
	int m_nStartJetOff;
	int m_nPrtDir;
	float m_fXPosOff;
	float m_fPrtYOffet;
	int m_nColorCnts;
	float m_fXDPI;
	long m_nYDPI;
	int m_nStartPassInedx;
	int m_nFeatherMode;
	int m_nCustomFeatherJets;
	int m_nEdgeScalePixel;
	float m_fRotateAngle;
	float m_fDstXScale;
	float m_fDstYScale;
	float m_fLayerDensity;
	float m_fLayerExtractDensity;
	float m_fXScanSpd;
	int m_nImgType;
	int m_nScanCtlValue;
	BOOL m_bDoubleDir;
	BOOL m_bYReverse;
	BOOL m_bXInsert;
	BOOL m_bSwitchByWhite;
	BOOL m_bEdgeNoReduce;
	BOOL m_bPrtArea;
	afx_msg void OnBnClickedOk();
	BOOL m_bGrayBits;
	virtual BOOL OnInitDialog();
	int m_nPassVol;
	int m_nMulityInk;
};
