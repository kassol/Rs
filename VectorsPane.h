#pragma once


// CVectorsPane

class CVectorsPane : public CDockablePane
{
	DECLARE_DYNAMIC(CVectorsPane)

public:
	CVectorsPane();
	virtual ~CVectorsPane();

protected:
	CListCtrl m_ctrlVectorList;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


