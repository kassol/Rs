#pragma once


// CRasterPane

class CRasterPane : public CDockablePane
{
	DECLARE_DYNAMIC(CRasterPane)

public:
	CRasterPane();
	virtual ~CRasterPane();

protected:
	CListCtrl m_ctrlImageList;


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


