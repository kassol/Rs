// VectorsPane.cpp : implementation file
//

#include "stdafx.h"
#include "Rs.h"
#include "VectorsPane.h"


// CVectorsPane



CListCtrl CVectorsPane::m_ctrlVectorList;

IMPLEMENT_DYNAMIC(CVectorsPane, CDockablePane)

CVectorsPane::CVectorsPane()
{

}

CVectorsPane::~CVectorsPane()
{
}


BEGIN_MESSAGE_MAP(CVectorsPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CVectorsPane message handlers




int CVectorsPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_ctrlVectorList.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT/*|LVS_NOCOLUMNHEADER*/, CRect(0, 0, 0, 0), this, 0))
	{
		TRACE0("创建影像列表失败\n");
		return -1;
	}
	m_ctrlVectorList.InsertColumn(0, _T("矢量列表"));
	m_ctrlVectorList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES);
	m_ctrlVectorList.SetColumnWidth(0, 500);

	return 0;
}


void CVectorsPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	if (m_ctrlVectorList.GetSafeHwnd() != NULL)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		m_ctrlVectorList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

CListCtrl& CVectorsPane::GetListCtrl()
{
	return m_ctrlVectorList;
}
