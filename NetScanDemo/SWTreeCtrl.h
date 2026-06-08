#pragma once
 
//#define _OWNER_DRAWN_TREE  // 自绘CTreeCtrl，可支持背景图片显示功能
 
class CSWTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CSWTreeCtrl)
 
	// 成员私有结构定义
 
	// 构造/析构函数
public:
	CSWTreeCtrl();
	virtual ~CSWTreeCtrl();
	int m_nFirstColumnWidth; // 第一列宽度
	int m_nOffsetX;      	 // 相对于父窗体X偏移量
    CImageList m_imgBtns;	 // 数控件展开的+/-图标符号 
	// 私有成员变量
private:
 
	// 私有成员函数
private:
	void CommonConstruct(); // 初始化
 
	// 受保护成员变量
protected:

	LVBKIMAGE m_bkImage;	 // 背景图片
	//CImageList m_imgBtns;	 // 数控件展开的+/-图标符号
 
	// 受保护成员函数

protected:
	BOOL CheckHit(CPoint point);
#ifdef _OWNER_DRAWN_TREE
	LRESULT CustomDrawNotify(LPNMTVCUSTOMDRAW lpNMTVCustomDraw);
	void OwnerDrawBackground(CDC* pDC);
#endif //_OWNER_DRAWN_TREE
 
	// 虚函数
protected:
 
	// 消息函数
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#ifdef _OWNER_DRAWN_TREE
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
#endif //_OWNER_DRAWN_TREE
 
	// 共有成员变量
public:
 
	// 共有成员函数
public:
	// 获取背景图片
	BOOL GetBkImage(LVBKIMAGE* plvbkImage) const;
 
	// 设置背景图片
	BOOL SetBkImage(LVBKIMAGE* plvbkImage);
 
#ifdef _OWNER_DRAWN_TREE
	// 设置树控件+/-按钮图标
	void SetTreeBtnBitmap(CBitmap* pBitmap, COLORREF clrMask = RGB(255, 0, 255));
#endif //_OWNER_DRAWN_TREE
 
	// 静态成员变量
public:
 
	// 静态成员函数
public:
 
};