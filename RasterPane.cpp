// RasterPane.cpp : implementation file
//

#include "stdafx.h"
#include "Rs.h"
#include "RasterPane.h"
#include "RsDoc.h"


// CRasterPane



CListCtrl CRasterPane::m_ctrlImageList;

IMPLEMENT_DYNAMIC(CRasterPane, CDockablePane)

CRasterPane::CRasterPane()
{

}

CRasterPane::~CRasterPane()
{
}


BEGIN_MESSAGE_MAP(CRasterPane, CDockablePane)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDD_RASTERLIST, &CRasterPane::OnCtrlItemChanged)
END_MESSAGE_MAP()



// CRasterPane message handlers




void CRasterPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (m_ctrlImageList.GetSafeHwnd() != NULL)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		m_ctrlImageList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}


int CRasterPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!m_ctrlImageList.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT/*|LVS_NOCOLUMNHEADER*/, CRect(0, 0, 0, 0), this, IDD_RASTERLIST))
	{
		TRACE0("创建影像列表失败\n");
		return -1;
	}
	m_ctrlImageList.InsertColumn(0, _T("影像列表"));
	m_ctrlImageList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES);
	m_ctrlImageList.SetColumnWidth(0, 500);
	return 0;
}

CListCtrl& CRasterPane::GetListCtrl()
{
	return m_ctrlImageList;
}

void CRasterPane::OnCtrlItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	CFrameWnd* pMainFrm = (CFrameWnd*)AfxGetApp()->GetMainWnd();
	CFrameWnd* pChildFrm = pMainFrm->GetActiveFrame();
	CRsDoc* pDoc = reinterpret_cast<CRsDoc*>(pChildFrm->GetActiveDocument());
	pDoc->SetState(pNMItemActivate->iItem, m_ctrlImageList.GetCheck(pNMItemActivate->iItem));
	*pResult = 0;
}
