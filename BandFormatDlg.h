#pragma once
#include "afxwin.h"


// CBandFormatDlg dialog

class CBandFormatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBandFormatDlg)

public:
	CBandFormatDlg(CWnd* pParent = NULL);   // standard constructor
	CBandFormatDlg(int nBandNum, CWnd* pParent = NULL);
	virtual ~CBandFormatDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int m_nBandNum;
public:
	CComboBox m_ctrlRed;
	CComboBox m_ctrlGreen;
	CComboBox m_ctrlBlue;
	int m_nRed;
	int m_nGreen;
	int m_nBlue;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
