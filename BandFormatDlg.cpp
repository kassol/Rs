// BandFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Rs.h"
#include "BandFormatDlg.h"
#include "afxdialogex.h"


// CBandFormatDlg dialog

IMPLEMENT_DYNAMIC(CBandFormatDlg, CDialogEx)

CBandFormatDlg::CBandFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBandFormatDlg::IDD, pParent)
	, m_nRed(0)
	, m_nGreen(0)
	, m_nBlue(0)
{

}

CBandFormatDlg::CBandFormatDlg(int nBandNum, CWnd* pParent /* = NULL */)
	:CDialogEx(CBandFormatDlg::IDD, pParent)
{
	m_nBandNum = nBandNum;
}

CBandFormatDlg::~CBandFormatDlg()
{
}

void CBandFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RED, m_ctrlRed);
	DDX_Control(pDX, IDC_GREEN, m_ctrlGreen);
	DDX_Control(pDX, IDC_BLUE, m_ctrlBlue);
}


BEGIN_MESSAGE_MAP(CBandFormatDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBandFormatDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CBandFormatDlg message handlers


BOOL CBandFormatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString temp;
	for (int i = 0; i < m_nBandNum; ++i)
	{
		temp.Format(_T("%d"), i+1);
		m_ctrlRed.AddString(temp);
		m_ctrlGreen.AddString(temp);
		m_ctrlBlue.AddString(temp);
	}
	if (m_nBandNum >= 3)
	{
		m_ctrlRed.SetCurSel(0);
		m_ctrlGreen.SetCurSel(1);
		m_ctrlBlue.SetCurSel(2);
	}
	else
	{
		m_ctrlRed.SetCurSel(0);
		m_ctrlGreen.SetCurSel(0);
		m_ctrlBlue.SetCurSel(0);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CBandFormatDlg::OnBnClickedOk()
{
	m_nRed = m_ctrlRed.GetCurSel();
	m_nGreen = m_ctrlGreen.GetCurSel();
	m_nBlue = m_ctrlBlue.GetCurSel();
	CDialogEx::OnOK();
}
