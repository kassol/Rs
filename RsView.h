// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// RsView.h : CRsView 类的接口
//

#pragma once


class CRsView : public CView
{
protected: // 仅从序列化创建
	CRsView();
	DECLARE_DYNCREATE(CRsView)

// 特性
public:
	CRsDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用

// 实现
public:
	virtual ~CRsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HGLRC m_hRC;
	CDC* m_pDC;
	int m_wide;
	int m_height;
	double m_lfScale;
	BOOL m_bShowEdge;
	BOOL m_bIsPress;
	CPoint m_ptStart;
	long m_nRealOrix;
	long m_nRealOriy;
	long m_nxoff;
	long m_nyoff;

private:
	unsigned char* m_pData;

public:
	BOOL InitializeOpenGL();
	BOOL SetupPixelformat();
	void RenderScene();

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnShowEdge();
	afx_msg void OnUpdateShowEdge(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // RsView.cpp 中的调试版本
inline CRsDoc* CRsView::GetDocument() const
   { return reinterpret_cast<CRsDoc*>(m_pDocument); }
#endif

