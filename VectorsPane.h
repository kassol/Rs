#pragma once


// CVectorsPane

class CVectorsPane : public CDockablePane
{
	DECLARE_DYNAMIC(CVectorsPane)

public:
	CVectorsPane();
	virtual ~CVectorsPane();

protected:
	static CListCtrl m_ctrlVectorList;

public:
	static CListCtrl& GetListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


