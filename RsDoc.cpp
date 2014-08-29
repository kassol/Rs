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

// RsDoc.cpp : CRsDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Rs.h"
#endif

#include "RsDoc.h"
#include "BandFormatDlg.h"

#include <propkey.h>
#include "RasterPane.h"
#include "VectorsPane.h"
#include <algorithm>
#include <fstream>
#include "clipper.hpp"
using namespace ClipperLib;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CRsDoc

IMPLEMENT_DYNCREATE(CRsDoc, CDocument)

BEGIN_MESSAGE_MAP(CRsDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CRsDoc::OnFileOpen)
	ON_COMMAND(ID_BANDCOMB, &CRsDoc::OnBandcomb)
	ON_COMMAND(ID_ADDRASTER, &CRsDoc::OnAddraster)
	ON_COMMAND(ID_ADDVECTOR, &CRsDoc::OnAddvector)
	ON_COMMAND(ID_GENERATElINE, &CRsDoc::OnGenerateline)
	ON_COMMAND(ID_DXF2DSM, &CRsDoc::OnDxf2dsm)
	ON_COMMAND(ID_OPTIMIZE, &CRsDoc::OnOptimize)
	ON_COMMAND(ID_EFFECTPOLY, &CRsDoc::OnEffectpoly)
	ON_COMMAND(ID_OPTIMIZE2, &CRsDoc::OnOptimize2)
	ON_COMMAND(ID_LOADMOSAIC, &CRsDoc::OnLoadmosaic)
END_MESSAGE_MAP()


// CRsDoc 构造/析构

CRsDoc::CRsDoc():m_pImage(NULL),
	m_lfMaxx(0),
	m_lfMaxy(0),
	m_lfMinx(0),
	m_lfMiny(0),
	m_lfResolution(1.0),
	m_lfScale(1.0),
	m_bIsGrey(TRUE),
	m_nBufWidth(0),
	m_nBufHeight(0),
	m_pCurBuf(NULL),
	m_pBacBuf(NULL),
	m_nRows(0),
	m_nCols(0),
	m_nBandNum(1),
	m_bIsReady(FALSE),
	m_nWndWidth(0),
	m_nWndHeight(0),
	m_nRealOriginx(0),
	m_nRealOriginy(0),
	m_nScrollSizex(0),
	m_nScrollSizey(0),
	m_nRed(0),
	m_nGreen(1),
	m_nBlue(2),
	m_nRealBandNum(0),
	m_pRasterState(NULL),
	m_pVectorState(NULL)
{
	// TODO: 在此添加一次性构造代码
	HRESULT hr = CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&m_pImage);
	if (FAILED(hr))
	{
		AfxMessageBox(_T("Failed to register the COM library"));
		return;
	}
}

CRsDoc::~CRsDoc()
{
	if (m_pImage != NULL)
	{
		m_pImage->Close();
		m_pImage->Release();
		m_pImage = NULL;
	}
	if (m_pBacBuf != NULL)
	{
		delete []m_pBacBuf;
		m_pBacBuf = NULL;
		m_pCurBuf = NULL;
	}
	if (m_pRasterState != NULL)
	{
		delete []m_pRasterState;
		m_pRasterState = NULL;
	}
	m_vecImagePath.clear();
	m_vecImageRect.clear();
	m_vecShapePath.clear();
	auto temIteX = m_vecX.begin();
	auto temIteY = m_vecY.begin();
	while(temIteX != m_vecX.end())
	{
		delete [](*temIteX);
		delete [](*temIteY);
		++temIteX;
		++temIteY;
	}
	m_vecX.clear();
	m_vecY.clear();
	m_vecPolygonNum.clear();
	m_vecPointNum.clear();
	m_recBac.Free();
	m_recCur.Free();
	UpdateRasterList();
	UpdateVectorList();
	CFrameWnd* pMainFrm = (CFrameWnd*)AfxGetApp()->GetMainWnd();
	CFrameWnd* pChildFrm = pMainFrm->GetActiveFrame();
	CRsDoc* pDoc = reinterpret_cast<CRsDoc*>(pChildFrm->GetActiveDocument());
	if (pDoc != NULL)
	{
		pDoc->UpdateRasterList();
		pDoc->UpdateVectorList();
	}
}

BOOL CRsDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	SetTitle(_T("New Doc"));
	return TRUE;
}




// CRsDoc 序列化

void CRsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CRsDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CRsDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CRsDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CRsDoc 诊断

#ifdef _DEBUG
void CRsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CRsDoc 命令


void CRsDoc::OnFileOpen()
{
	m_bIsReady = FALSE;
	BYTE* szFileFilter = new BYTE[500];
	m_pImage->GetSupExts(szFileFilter, modeRead);

	CString strFileFilter(szFileFilter);
	//AfxMessageBox(strFileFilter);
	delete []szFileFilter;
	szFileFilter = NULL;


	CFileDialog fdlg(TRUE, NULL, NULL, 
		OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		strFileFilter, NULL, 0, TRUE);

	const int MIN_FILE_NUMBER = 80000;
	fdlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH*MIN_FILE_NUMBER];
	memset(fdlg.m_ofn.lpstrFile, 0, _MAX_PATH*MIN_FILE_NUMBER);
	fdlg.m_ofn.nMaxFile = _MAX_PATH*MIN_FILE_NUMBER;

	if (IDOK == fdlg.DoModal())
	{
		m_nRed = 0;
		m_nGreen = 1;
		m_nBlue = 2;
		m_lfMinx = 0;
		m_lfMaxx = 0;
		m_lfMiny = 0;
		m_lfMaxy = 0;
		m_vecImagePath.clear();
		m_vecImageRect.clear();
		if (m_pBacBuf != NULL)
		{
			delete []m_pBacBuf;
			m_pBacBuf = NULL;
			m_pCurBuf = NULL;
		}
		if (m_pRasterState != NULL)
		{
			delete []m_pRasterState;
			m_pRasterState = NULL;
		}
		m_recBac.Free();
		m_recCur.Free();
		POSITION pos = fdlg.GetStartPosition();

		while (NULL != pos)
		{
			CString strFilePath = fdlg.GetNextPathName(pos);
			m_vecImagePath.push_back(strFilePath);
			m_pImage->Open(strFilePath.AllocSysString(), modeRead|modeAqlut);
			int nCols, nRows;
			m_pImage->GetCols(&nCols);
			m_pImage->GetRows(&nRows);
			m_pImage->GetBandNum(&m_nBandNum);
			m_nRealBandNum = m_nBandNum;
			double lfXOrigin, lfYOrigin, lfResolution;
			m_pImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfResolution);
			m_pImage->Close();
			_CrtDumpMemoryLeaks();
			if (m_nBandNum == 1)
			{
				m_bIsGrey = TRUE;
			}
			else
			{
				m_bIsGrey = FALSE;
			}
			m_vecImageRect.push_back(RectFExt(lfXOrigin, lfXOrigin+lfResolution*nCols, 
				lfYOrigin+lfResolution*nRows,  lfYOrigin));
			m_lfResolution = lfResolution;
			if (m_lfMinx == 0 || m_lfMinx-lfXOrigin > 0.000001)
			{
				m_lfMinx = lfXOrigin;
			}
			if (m_lfMiny == 0 || m_lfMiny-lfYOrigin > 0.0000001)
			{
				m_lfMiny = lfYOrigin;
			}
			if (m_lfMaxx == 0 || lfXOrigin+lfResolution*nCols-m_lfMaxx > 0.0000001)
			{
				m_lfMaxx = lfXOrigin+lfResolution*nCols;
			}
			if (m_lfMaxy == 0 || lfYOrigin+lfResolution*nRows-m_lfMaxy > 0.0000001)
			{
				m_lfMaxy = lfYOrigin+lfResolution*nRows;
			}
		}
		if (m_vecImagePath.size() > 1)
		{
			SetTitle(m_vecImagePath.front()+_T("..."));
		}
		else
		{
			SetTitle(m_vecImagePath.front());
		}
		m_pRasterState = new int[m_vecImagePath.size()];
		for (size_t index = 0; index < m_vecImagePath.size(); ++index)
		{
			m_pRasterState[index] = 1;
		}
		UpdateRasterList();
		UpdateRasterState();

		if (m_bIsGrey)
		{
			m_nBandNum = 1;
		}
		else
		{
			m_nBandNum = 3;
		}

		
		m_nCols = int((m_lfMaxx-m_lfMinx)/m_lfResolution);
		m_nRows = int((m_lfMaxy-m_lfMiny)/m_lfResolution);
		

		CFrameWnd* pMainFrm = (CFrameWnd*)AfxGetApp()->GetMainWnd();
		CFrameWnd* pChildFrm = pMainFrm->GetActiveFrame();
		
		CRect rect;
		pChildFrm->GetClientRect(&rect);

		m_nWndWidth = rect.Width();
		m_nWndHeight = rect.Height();


		m_lfScale = (rect.Width()/(double)m_nCols < rect.Height()/(double)m_nRows ?
			rect.Width()/(double)m_nCols : rect.Height()/(double)m_nRows);

		if (m_lfScale -1 > 0.0000001)
		{
			m_lfScale = 1;
		}
		else
		{
			for (int n = 1; n < 17; n *= 2)
			{
				if (m_lfScale < 1/double(n))
				{
					if (n != 6)
					{
						continue;
					}
					m_lfScale = 1/double(n);
				}
				else
				{
					m_lfScale = 1/double(n);
					break;
				}
			}
		}

		m_nBufWidth = int(m_nCols*m_lfScale);
		m_nBufHeight = int(m_nRows*m_lfScale);
		
		m_nScrollSizex = m_nBufWidth;
		m_nScrollSizey = m_nBufHeight;

		if (m_nBufHeight < m_nWndHeight || m_nBufWidth < m_nWndWidth)
		{
			m_nRealOriginx = (m_nWndWidth-m_nBufWidth)/2;
			m_nRealOriginy = (m_nWndHeight-m_nBufHeight)/2;
		}


		double left = 0, right = 0, top = 0, bottom = 0;

		Screen2Geo(0, 0, left, bottom);
		Screen2Geo(m_nBufWidth, m_nBufHeight, right, top);
		
		m_recBac = RectFExt(left, right, top, bottom);
		FillData(m_recBac);
		m_bIsReady = TRUE;
		UpdateAllViews(NULL);
	}
	delete []fdlg.m_ofn.lpstrFile;
}

void CRsDoc::GetBufSize(int &nBufWidth, int &nBufHeight)
{
	nBufWidth = m_nBufWidth;
	nBufHeight = m_nBufHeight;
}

unsigned char* CRsDoc::GetData()
{
	if (m_pCurBuf != NULL)
	{
		delete []m_pCurBuf;
		m_pCurBuf = NULL;
	}
	m_pCurBuf = m_pBacBuf;
	m_recCur = m_recBac;
	return m_pCurBuf;
}

BOOL CRsDoc::IsGrey()
{
	return m_bIsGrey;
}

BOOL CRsDoc::IsReady()
{
	return m_bIsReady;
}

void CRsDoc::SetReady(BOOL bIsReady)
{
	m_bIsReady = bIsReady;
}

void CRsDoc::GetRealOrigin(long& nRealOriginx, long& nRealOriginy)
{
	nRealOriginx = m_nRealOriginx;
	nRealOriginy = m_nRealOriginy;
}

void CRsDoc::SetRealOrigin(long nRealOriginx, long nRealOriginy)
{
	m_nRealOriginx = nRealOriginx;
	m_nRealOriginy = nRealOriginy;
}

void CRsDoc::Screen2Geo(long nxpos, long nypos, double &lfxpos, double &lfypos)
{
	lfxpos = m_lfMinx+(nxpos/m_lfScale)*m_lfResolution;
	lfypos = m_lfMiny+(nypos/m_lfScale)*m_lfResolution;
}

void CRsDoc::Geo2Screen(double lfxpos, double lfypos, long &nxpos, long &nypos)
{
	nxpos = long((lfxpos-m_lfMinx)/m_lfResolution*m_lfScale);
	nypos = long((lfypos-m_lfMiny)/m_lfResolution*m_lfScale);
}

void CRsDoc::Geo2Buf(RectFExt rect, double lfxpos, double lfypos, long &nxpos, long &nypos)
{
	if (m_lfScale-1 > 0.0000001)
	{
		nxpos = long((lfxpos-rect.left)/m_lfResolution);
		nypos = long((rect.top-lfypos)/m_lfResolution);
	}
	else
	{
		nxpos = long((lfxpos-rect.left)/m_lfResolution*m_lfScale);
		nypos = long((rect.top-lfypos)/m_lfResolution*m_lfScale);
	}
}

void CRsDoc::FillData(RectFExt rect)
{
	if (!m_recCur.IsEmpty() && m_recCur.Intersects(rect))
	{
		m_pBacBuf = new unsigned char[m_nBufWidth*m_nBufHeight*m_nBandNum];
		memset(m_pBacBuf, 0, m_nBufWidth*m_nBufHeight*m_nBandNum);
		RectFExt recResult = m_recCur.Intersected(rect);
		long left = 0, top = 0, bottom = 0;
		long left2 = 0, top2 = 0, bottom2 = 0;
		Geo2Buf(rect, recResult.left, recResult.top, left, top);
		Geo2Buf(m_recCur, recResult.left, recResult.top, left2, top2);

		int nWidth = int(recResult.Width()/m_lfResolution*m_lfScale+0.99999);
		int nHeight = int(recResult.Height()/m_lfResolution*m_lfScale+0.99999);


		bottom = m_nBufHeight-top;
		bottom2 = m_nBufHeight-top2;

		if (bottom < 0)
		{
			bottom = 0;
		}
		if (bottom2 < 0)
		{
			bottom2 = 0;
		}
		
		for (int y = 0; y < nHeight; ++y)
		{
			memcpy(m_pBacBuf+(bottom-1-y)*m_nBufWidth*m_nBandNum+left*m_nBandNum,
				m_pCurBuf+(bottom2-1-y)*m_nBufWidth*m_nBandNum+left2*m_nBandNum, nWidth*m_nBandNum);
		}
		if (left == 0 && top == 0)
		{
			FillData(RectFExt(recResult.right, rect.right, rect.top, recResult.bottom));
			FillData(RectFExt(rect.left, rect.right, recResult.bottom, rect.bottom));
		}
		else
		{
			FillData(RectFExt(rect.left, rect.right, rect.top, recResult.top));
			FillData(RectFExt(rect.left, recResult.left, recResult.top, rect.bottom));
		}
	}
	else
	{
		m_pBacBuf = new unsigned char[m_nBufWidth*m_nBufHeight*m_nBandNum];
		memset(m_pBacBuf, 0, m_nBufWidth*m_nBufHeight*m_nBandNum);
		auto PathNameIte = m_vecImagePath.begin();
		int i = 0;
		for (auto temIte = m_vecImageRect.begin(); temIte != m_vecImageRect.end(); ++temIte, ++PathNameIte)
		{
			RectFExt recResult;
			if (rect.Intersects(*temIte) && m_pRasterState[i] == TRUE)
			{
				recResult = rect.Intersected(*temIte);
				m_pImage->Open((*PathNameIte).AllocSysString(), modeRead|modeAqlut);
				float nSrcLeft = 0, nSrcRight = 0, nSrcTop = 0, nSrcBottom = 0;
				m_pImage->World2Image(recResult.left, recResult.bottom, &nSrcLeft, &nSrcTop);
				m_pImage->World2Image(recResult.right, recResult.top, &nSrcRight, &nSrcBottom);
				
				int nXCount, nYCount;
				if (m_lfScale-1 > 0.0000001)
				{
					nXCount = int(nSrcRight-nSrcLeft+0.99999);
					nYCount = int(nSrcBottom-nSrcTop+0.99999);
				}
				else
				{
					nXCount = int((nSrcRight-nSrcLeft)*m_lfScale+0.99999);
					nYCount = int((nSrcBottom-nSrcTop)*m_lfScale+0.99999);
				}

				long left = 0, right = 0, top = 0, bottom = 0;

				Geo2Buf(m_recBac, recResult.left, recResult.top, left, top);

				right = left+nXCount;
				bottom = m_nBufHeight-top;
				top = bottom-nYCount;

				if (left < 0)
				{
					left = 0;
				}
				if (right > m_nBufWidth)
				{
					right = m_nBufWidth;
				}
				if (top < 0)
				{
					top = 0;
				}
				if (bottom > m_nBufHeight)
				{
					bottom = m_nBufHeight;
				}
				if (m_bIsGrey)
				{
					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, -1, 0);
				}
				else
				{
					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nRed, 0);
					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nGreen, 1);
					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nBlue, 2);
// 					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
// 						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, -1, 0);
				}
				m_pImage->Close();
			}
			++i;
		}
	}
}

void CRsDoc::ZoomIn(CPoint &pt)
{
	m_recCur.Free();
	double geox = 0, geoy = 0;
	Screen2Geo(pt.x, pt.y, geox, geoy);
	m_lfScale /= 2.0;
	if (m_lfScale-1.0 >0.0000001 && 2.0-m_lfScale > 0.0000001)
	{
		m_lfScale = 1.0;
	}
	long newx = 0, newy = 0;
	Geo2Screen(geox, geoy, newx, newy);
	m_nRealOriginx -= (newx-pt.x);
	m_nRealOriginy -= (newy-pt.y);

	m_nScrollSizex = int(m_nCols*m_lfScale);
	m_nScrollSizey = int(m_nRows*m_lfScale);

	int nClientRectLeft = int(-m_nRealOriginx);
	int nClientRectRight = int(m_nWndWidth-m_nRealOriginx);
	int nClientRectTop = int(m_nWndHeight-m_nRealOriginy);
	int nClientRectBottom = int(-m_nRealOriginy);

	if (nClientRectLeft < 0)
	{
		nClientRectLeft = 0;
	}
	if (nClientRectRight > m_nScrollSizex)
	{
		nClientRectRight = m_nScrollSizex;
	}
	if (nClientRectTop > m_nScrollSizey)
	{
		nClientRectTop = m_nScrollSizey;
	}
	if (nClientRectBottom < 0)
	{
		nClientRectBottom = 0;
	}
	double left = 0, right = 0, top = 0, bottom = 0;

	Screen2Geo(nClientRectLeft, nClientRectTop, left, top);
	Screen2Geo(nClientRectRight, nClientRectBottom, right, bottom);

	m_recBac = RectFExt(left, right, top, bottom);
	if (m_lfScale-1.0 > 0.0000001)
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution+0.99999);
	}
	else
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution*m_lfScale+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution*m_lfScale+0.99999);
	}

	FillData(m_recBac);
	m_bIsReady = TRUE;
	UpdateAllViews(NULL);
}

void CRsDoc::ZoomOut(CPoint &pt)
{
	m_recCur.Free();
	double geox = 0, geoy = 0;
	Screen2Geo(pt.x, pt.y, geox, geoy);
	m_lfScale *= 2.0;
	if (m_lfScale-1.0 > 0.0000001 && 2.0-m_lfScale > 0.0000001)
	{
		m_lfScale = 1.0;
	}
	if (m_lfScale*2.0-m_nWndHeight > 0.0000001 || m_lfScale*2.0-m_nWndWidth > 0.0000001)
	{
		m_lfScale /= 2.0;
	}
	long newx = 0, newy = 0;
	Geo2Screen(geox, geoy, newx, newy);
	m_nRealOriginx -= (newx-pt.x);
	m_nRealOriginy -= (newy-pt.y);
	
	m_nScrollSizex = int(m_nCols*m_lfScale);
	m_nScrollSizey = int(m_nRows*m_lfScale);

	int nClientRectLeft = int(-m_nRealOriginx);
	int nClientRectRight = int(m_nWndWidth-m_nRealOriginx);
	int nClientRectTop = int(m_nWndHeight-m_nRealOriginy);
	int nClientRectBottom = int(-m_nRealOriginy);

	if (nClientRectLeft < 0)
	{
		nClientRectLeft = 0;
	}
	if (nClientRectRight > m_nScrollSizex)
	{
		nClientRectRight = m_nScrollSizex;
	}
	if (nClientRectTop > m_nScrollSizey)
	{
		nClientRectTop = m_nScrollSizey;
	}
	if (nClientRectBottom < 0)
	{
		nClientRectBottom = 0;
	}
	double left = 0, right = 0, top = 0, bottom = 0;

	Screen2Geo(nClientRectLeft, nClientRectTop, left, top);
	Screen2Geo(nClientRectRight, nClientRectBottom, right, bottom);

	m_recBac = RectFExt(left, right, top, bottom);
	if (m_lfScale-1.0 > 0.0000001)
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution+0.99999);
	}
	else
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution*m_lfScale+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution*m_lfScale+0.99999);
	}

	FillData(m_recBac);
	m_bIsReady = TRUE;
	UpdateAllViews(NULL);
}

void CRsDoc::GetViewScale(double& lfScale)
{
	lfScale = m_lfScale;
}

void CRsDoc::UpdateData()
{
	m_recCur.Free();
	m_nScrollSizex = int(m_nCols*m_lfScale);
	m_nScrollSizey = int(m_nRows*m_lfScale);

	int nClientRectLeft = int(-m_nRealOriginx);
	int nClientRectRight = int(m_nWndWidth-m_nRealOriginx);
	int nClientRectTop = int(m_nWndHeight-m_nRealOriginy);
	int nClientRectBottom = int(-m_nRealOriginy);

	if (nClientRectLeft < 0)
	{
		nClientRectLeft = 0;
	}
	if (nClientRectRight > m_nScrollSizex)
	{
		nClientRectRight = m_nScrollSizex;
	}
	if (nClientRectTop > m_nScrollSizey)
	{
		nClientRectTop = m_nScrollSizey;
	}
	if (nClientRectBottom < 0)
	{
		nClientRectBottom = 0;
	}
	double left = 0, right = 0, top = 0, bottom = 0;

	Screen2Geo(nClientRectLeft, nClientRectTop, left, top);
	Screen2Geo(nClientRectRight, nClientRectBottom, right, bottom);

	m_recBac = RectFExt(left, right, top, bottom);
	if (m_lfScale-1.0 > 0.0000001)
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution+0.99999);
	}
	else
	{
		m_nBufWidth = int(m_recBac.Width()/m_lfResolution*m_lfScale+0.99999);
		m_nBufHeight = int(m_recBac.Height()/m_lfResolution*m_lfScale+0.99999);
	}

	FillData(m_recBac);
	m_bIsReady = TRUE;
	UpdateAllViews(NULL);
}



void CRsDoc::OnBandcomb()
{
	CBandFormatDlg dlg(m_nRealBandNum);
	if (IDOK == dlg.DoModal())
	{
		m_nRed = dlg.m_nRed;
		m_nGreen = dlg.m_nGreen;
		m_nBlue = dlg.m_nBlue;
		m_recCur.Free();
		FillData(m_recBac);
		m_bIsReady = TRUE;
		UpdateAllViews(NULL);
	}
}

void CRsDoc::UpdateRasterList()
{
	CListCtrl& ctrlRasterList = CRasterPane::GetListCtrl();
	ctrlRasterList.DeleteAllItems();

	auto rasterIte = m_vecImagePath.begin();
	int i = 0;
	while(rasterIte != m_vecImagePath.end())
	{
		ctrlRasterList.InsertItem(ctrlRasterList.GetItemCount(), *rasterIte);
		if (m_pRasterState[i] == 0)
		{
			ctrlRasterList.SetCheck(ctrlRasterList.GetItemCount()-1, 0);
		}
		else
		{
			ctrlRasterList.SetCheck(ctrlRasterList.GetItemCount()-1);
		}
		++rasterIte;
		++i;
	}
}

void CRsDoc::UpdateVectorList()
{
	CListCtrl& ctrlVectorList = CVectorsPane::GetListCtrl();
	ctrlVectorList.DeleteAllItems();

	auto vectorIte = m_vecShapePath.begin();
	int i = 0;
	while(vectorIte != m_vecShapePath.end())
	{
		ctrlVectorList.InsertItem(ctrlVectorList.GetItemCount(), *vectorIte);
		if (m_pVectorState[i] == 0)
		{
			ctrlVectorList.SetCheck(ctrlVectorList.GetItemCount()-1, 0);
		}
		else
		{
			ctrlVectorList.SetCheck(ctrlVectorList.GetItemCount()-1);
		}
		++vectorIte;
		++i;
	}
}

void CRsDoc::SetRasterState(int nIndex, BOOL bState)
{
	bool bChanged = false;
	if (m_pRasterState[nIndex] != bState)
	{
		bChanged = true;
	}
	m_pRasterState[nIndex] = bState;
	if (bChanged)
	{
		m_recCur.Free();
		FillData(m_recBac);
		m_bIsReady = TRUE;
		UpdateAllViews(NULL);
	}
}

void CRsDoc::SetVectorState(int nIndex, BOOL bState)
{
	bool bChanged = false;
	if (m_pVectorState[nIndex] != bState)
	{
		bChanged = true;
	}
	m_pVectorState[nIndex] = bState;

	if (bChanged)
	{
		m_recCur.Free();
		FillData(m_recBac);
		m_bIsReady = TRUE;
		UpdateAllViews(NULL);
	}
}

void CRsDoc::UpdateRasterState()
{
	CListCtrl& ctrlRasterList = CRasterPane::GetListCtrl();

	for (int i = 0; i < ctrlRasterList.GetItemCount(); ++i)
	{
		m_pRasterState[i] = ctrlRasterList.GetCheck(i);
	}
}

void CRsDoc::UpdateVectorState()
{
	CListCtrl& ctrlVectorList = CVectorsPane::GetListCtrl();

	for (int i = 0; i < ctrlVectorList.GetItemCount(); ++i)
	{
		m_pVectorState[i] = ctrlVectorList.GetCheck(i);
	}
}

int* CRsDoc::GetRasterState()
{
	return m_pRasterState;
}

int* CRsDoc::GetVectorState()
{
	return m_pVectorState;
}


void CRsDoc::OnAddraster()
{
	m_bIsReady = FALSE;
	BYTE* szFileFilter = new BYTE[500];
	m_pImage->GetSupExts(szFileFilter, modeRead);

	CString strFileFilter(szFileFilter);
	delete []szFileFilter;
	szFileFilter = NULL;


	CFileDialog fdlg(TRUE, NULL, NULL, 
		OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		strFileFilter, NULL, 0, TRUE);

	const int MIN_FILE_NUMBER = 80000;
	fdlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH*MIN_FILE_NUMBER];
	memset(fdlg.m_ofn.lpstrFile, 0, _MAX_PATH*MIN_FILE_NUMBER);
	fdlg.m_ofn.nMaxFile = _MAX_PATH*MIN_FILE_NUMBER;


	CString strAllPath;
	if (IDOK == fdlg.DoModal())
	{
		m_recBac.Free();
		m_recCur.Free();
		POSITION pos = fdlg.GetStartPosition();

		while (NULL != pos)
		{
			CString strFilePath = fdlg.GetNextPathName(pos);
			m_vecImagePath.push_back(strFilePath);
			m_pImage->Open(strFilePath.AllocSysString(), modeRead|modeAqlut);
			int nCols, nRows;
			m_pImage->GetCols(&nCols);
			m_pImage->GetRows(&nRows);
			m_pImage->GetBandNum(&m_nBandNum);
			m_nRealBandNum = m_nBandNum;
			double lfXOrigin, lfYOrigin, lfResolution;
			m_pImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfResolution);
			m_pImage->Close();
			if (m_nBandNum == 1)
			{
				m_bIsGrey = TRUE;
			}
			else
			{
				m_bIsGrey = FALSE;
			}
			m_vecImageRect.push_back(RectFExt(lfXOrigin, lfXOrigin+lfResolution*nCols, 
				lfYOrigin+lfResolution*nRows,  lfYOrigin));
			m_lfResolution = lfResolution;
			if (m_lfMinx == 0 || m_lfMinx-lfXOrigin > 0.000001)
			{
				m_lfMinx = lfXOrigin;
			}
			if (m_lfMiny == 0 || m_lfMiny-lfYOrigin > 0.0000001)
			{
				m_lfMiny = lfYOrigin;
			}
			if (m_lfMaxx == 0 || lfXOrigin+lfResolution*nCols-m_lfMaxx > 0.0000001)
			{
				m_lfMaxx = lfXOrigin+lfResolution*nCols;
			}
			if (m_lfMaxy == 0 || lfYOrigin+lfResolution*nRows-m_lfMaxy > 0.0000001)
			{
				m_lfMaxy = lfYOrigin+lfResolution*nRows;
			}
		}

		if (m_vecImagePath.size() > 1)
		{
			SetTitle(m_vecImagePath.front()+_T("..."));
		}
		else
		{
			SetTitle(m_vecImagePath.front());
		}

		if (m_pRasterState != NULL)
		{
			int* pRasterState = new int[m_vecImagePath.size()];
			CListCtrl& ctrlRasterList = CRasterPane::GetListCtrl();
			int nOldRasterCount = ctrlRasterList.GetItemCount();
			memcpy(pRasterState, m_pRasterState, sizeof(int)*nOldRasterCount);
			for (size_t index = nOldRasterCount; index < m_vecImagePath.size(); ++index)
			{
				pRasterState[index] = 1;
			}
			delete []m_pRasterState;
			m_pRasterState = pRasterState;
			pRasterState = NULL;
		}
		else
		{
			m_pRasterState = new int[m_vecImagePath.size()];
			for (size_t index = 0; index < m_vecImagePath.size(); ++index)
			{
				m_pRasterState[index] = 1;
			}
		}
		
		UpdateRasterList();
		UpdateRasterState();

		if (m_bIsGrey)
		{
			m_nBandNum = 1;
		}
		else
		{
			m_nBandNum = 3;
		}


		m_nCols = int((m_lfMaxx-m_lfMinx)/m_lfResolution);
		m_nRows = int((m_lfMaxy-m_lfMiny)/m_lfResolution);


		CFrameWnd* pMainFrm = (CFrameWnd*)AfxGetApp()->GetMainWnd();
		CFrameWnd* pChildFrm = pMainFrm->GetActiveFrame();

		CRect rect;
		pChildFrm->GetClientRect(&rect);

		m_nWndWidth = rect.Width();
		m_nWndHeight = rect.Height();


		m_lfScale = (rect.Width()/(double)m_nCols < rect.Height()/(double)m_nRows ?
			rect.Width()/(double)m_nCols : rect.Height()/(double)m_nRows);

		if (m_lfScale -1 > 0.0000001)
		{
			m_lfScale = 1;
		}
		else
		{
			for (int n = 1; n < 17; n *= 2)
			{
				if (m_lfScale < 1/double(n))
				{
					if (n != 6)
					{
						continue;
					}
					m_lfScale = 1/double(n);
				}
				else
				{
					m_lfScale = 1/double(n);
					break;
				}
			}
		}

		m_nBufWidth = int(m_nCols*m_lfScale);
		m_nBufHeight = int(m_nRows*m_lfScale);

		m_nScrollSizex = m_nBufWidth;
		m_nScrollSizey = m_nBufHeight;

		if (m_nBufHeight < m_nWndHeight || m_nBufWidth < m_nWndWidth)
		{
			m_nRealOriginx = (m_nWndWidth-m_nBufWidth)/2;
			m_nRealOriginy = (m_nWndHeight-m_nBufHeight)/2;
		}


		double left = 0, right = 0, top = 0, bottom = 0;

		Screen2Geo(0, 0, left, bottom);
		Screen2Geo(m_nBufWidth, m_nBufHeight, right, top);

		m_recBac = RectFExt(left, right, top, bottom);
		FillData(m_recBac);
		m_bIsReady = TRUE;
		UpdateAllViews(NULL);
	}
	delete []fdlg.m_ofn.lpstrFile;
}

void CRsDoc::ParsePolygon()
{
	if (!m_vecMosaicLine.empty())
	{
		std::for_each(m_vecMosaicLine.begin(), m_vecMosaicLine.end(), [](PolygonExt& poly)
		{
			poly.Free();
		});
		m_vecMosaicLine.clear();
	}

	CString strOutPath = _T("D:\\output\\");
	std::for_each(m_vecImagePath.begin(), m_vecImagePath.end(), [&](CString imagepath)
	{
		CString rrlx = strOutPath+imagepath.Right(
			imagepath.GetLength()-imagepath.ReverseFind('\\')-1);
		rrlx = rrlx.Left(rrlx.ReverseFind('.'))+_T(".rrlx");
		std::fstream infile;
		infile.open(rrlx.GetBuffer(0), std::ios::in);
		int point_count = 0;
		infile>>point_count;
		double* px = new double[point_count];
		memset(px, 0, sizeof(double)*point_count);
		double* py = new double[point_count];
		memset(py, 0, sizeof(double)*point_count);
		int temp = 0;
		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i]>>temp;
		}
		m_vecMosaicLine.push_back(PolygonExt(point_count, px, py));
	});
}

void CRsDoc::ParseEffective()
{
	if (!m_vecEffectivePoly.empty())
	{
		std::for_each(m_vecEffectivePoly.begin(), m_vecEffectivePoly.end(),
			[&](PolygonExt& poly)
		{
			poly.Free();
		});
		m_vecEffectivePoly.clear();
	}
	std::for_each(m_vecImagePath.begin(), m_vecImagePath.end(),
		[&](CString imagepath)
	{
		CString ep = imagepath+_T(".ep");
		std::fstream infile;
		infile.open(ep.GetBuffer(0), std::ios::in);
		int point_count = 0;
		infile>>point_count;
		double* px = new double[point_count];
		memset(px, 0, sizeof(double)*point_count);
		double* py = new double[point_count];
		memset(py, 0, sizeof(double)*point_count);
		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i];
		}
		PolygonExt poly(point_count, px, py);
		ConvexHull(poly);
		m_vecEffectivePoly.push_back(poly);
	});
}

std::vector<PolygonExt>& CRsDoc::GetPolygonVec()
{
	return m_vecMosaicLine;
}

std::vector<PolygonExt>& CRsDoc::GetEffectivepoly()
{
	return m_vecEffectivePoly;
}

void CRsDoc::OnAddvector()
{
	CString strFileFilter = _T("AutoCAD ShapeFile(*.dxf)|*.dxf|All File(*.*)|*.*||");

	CFileDialog fdlg(TRUE, NULL, NULL, 
		OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		strFileFilter, NULL, 0, TRUE);

	const int MIN_FILE_NUMBER = 10;
	fdlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH*MIN_FILE_NUMBER];
	memset(fdlg.m_ofn.lpstrFile, 0, _MAX_PATH*MIN_FILE_NUMBER);
	fdlg.m_ofn.nMaxFile = _MAX_PATH*MIN_FILE_NUMBER;

	if (IDOK == fdlg.DoModal())
	{
		POSITION pos = fdlg.GetStartPosition();
		while(NULL != pos)
		{
			CString strVectorPath = fdlg.GetNextPathName(pos);
			m_vecShapePath.push_back(strVectorPath);
			if (m_dxffile.Create() && m_dxffile.LoadDXFFile(strVectorPath.GetBuffer(0)) == TRUE)
			{
				ENTITYHEADER EntityHeader;
				char	 EntityData[4096];
				OBJHANDLE hEntity;
				hEntity = m_dxffile.FindEntity(FIND_FIRST, &EntityHeader, EntityData, NULL);
				int nPolygonCount = 0;
				while (hEntity)
				{
					switch(EntityHeader.EntityType)
					{
					case ENT_POLYLINE:
					case ENT_LINE3D:
						{
							PENTPOLYLINE pPolyline = (PENTPOLYLINE)EntityData;
							int nVertexNum = pPolyline->nVertex;
							if (nVertexNum > 2)
							{
								double *pX = new double[pPolyline->nVertex];
								double *pY = new double[pPolyline->nVertex];
								m_vecPointNum.push_back(pPolyline->nVertex);

								memset(pX, 0, pPolyline->nVertex*sizeof(double));
								memset(pY, 0, pPolyline->nVertex*sizeof(double));

								for (int nIndex = 0; nIndex < pPolyline->nVertex; ++ nIndex)
								{
									pX[nIndex] = pPolyline->pVertex[nIndex].Point.x;
									pY[nIndex] = pPolyline->pVertex[nIndex].Point.y;
								}
								m_vecX.push_back(pX);
								m_vecY.push_back(pY);
								++nPolygonCount;
							}
						}
					}
					hEntity = m_dxffile.FindEntity(FIND_NEXT, &EntityHeader, EntityData, NULL);
				}
				m_vecPolygonNum.push_back(nPolygonCount);
			}
			else
			{
				CString temp = _T("加载dxf失败!");
				AfxMessageBox(temp);
				return;
			}
			m_dxffile.Destroy();
		}

		if (m_pVectorState != NULL)
		{
			int* pVectorState = new int[m_vecImagePath.size()];
			CListCtrl& ctrlVectorList = CVectorsPane::GetListCtrl();
			int nOldVectorCount = ctrlVectorList.GetItemCount();
			memcpy(pVectorState, m_pRasterState, sizeof(int)*nOldVectorCount);
			for (size_t index = nOldVectorCount; index < m_vecShapePath.size(); ++index)
			{
				pVectorState[index] = 1;
			}
			delete []m_pVectorState;
			m_pVectorState = pVectorState;
			pVectorState = NULL;
		}
		else
		{
			m_pVectorState = new int[m_vecShapePath.size()];
			for (size_t index = 0; index < m_vecShapePath.size(); ++index)
			{
				m_pVectorState[index] = 1;
			}
		}

		UpdateVectorList();
		UpdateVectorState();

		UpdateAllViews(NULL);
	}
}

void CRsDoc::GetShapeIterator(std::vector<double*>::iterator& iteX, std::vector<double*>::iterator& iteY, std::vector<int>::iterator& iteNum, std::vector<int>::iterator& itePolyNum)
{
	iteX = m_vecX.begin();
	iteY = m_vecY.begin();
	iteNum = m_vecPointNum.begin();
	itePolyNum = m_vecPolygonNum.begin();
}

void CRsDoc::GetShapeIterEnd(std::vector<double*>::iterator& iteX, std::vector<double*>::iterator& iteY, std::vector<int>::iterator& iteNum, std::vector<int>::iterator& itePolyNum)
{
	iteX = m_vecX.end();
	iteY = m_vecY.end();
	iteNum = m_vecPointNum.end();
	itePolyNum = m_vecPolygonNum.end();
}

void CRsDoc::OnGenerateline()
{
	CString strAllPath;
	std::for_each(m_vecImagePath.begin(), m_vecImagePath.end(), [&](CString ImagePath)
	{
		strAllPath += ImagePath;
		strAllPath += ";";
	});

	strAllPath = strAllPath.Left(strAllPath.GetLength()-1);

	IMosaic* pMosaic = NULL;
	HRESULT hr = CoCreateInstance(CLSID_Mosaic, NULL, CLSCTX_ALL, IID_IMosaic, (void**)&pMosaic);
	if (FAILED(hr))
	{
		return;
	}
	int novaliddomnum;
	int *index = NULL;
	pMosaic->InitialOrthoProMosaicLines(strAllPath.AllocSysString(), _bstr_t("D:\\output\\"), &novaliddomnum, index);
	ParsePolygon();
	UpdateAllViews(NULL);
}


void CRsDoc::OnDxf2dsm()
{
	CString strFileFilter = _T("DXF(*.dxf)|*.dxf||");
	CFileDialog fdlg(TRUE, NULL, NULL, 
			OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		strFileFilter, NULL, 0, TRUE);
	CString strDxf;
	if (IDOK == fdlg.DoModal())
	{
		strDxf = fdlg.GetPathName();

		if (m_dxffile.Create() && m_dxffile.LoadDXFFile(strDxf) == TRUE)
		{
			ENTITYHEADER EntityHeader;
			char	 EntityData[4096];
			OBJHANDLE hEntity;
			hEntity = m_dxffile.FindEntity(FIND_FIRST, &EntityHeader, EntityData, NULL);
			int nPolygonCount = 0;
			std::vector<int> vecPointNum;
			std::vector<double*> vecX;
			std::vector<double*> vecY;
			std::vector<double*>vecEXx;
			std::vector<double*>vecExy;

			double lfMinx = 0, lfMiny = 0, lfMaxx = 0, lfMaxy = 0;

			while (hEntity)
			{
				switch(EntityHeader.EntityType)
				{
				case ENT_POLYLINE:
				case ENT_LINE3D:
					{
						PENTPOLYLINE pPolyline = (PENTPOLYLINE)EntityData;
						int nVertexNum = pPolyline->nVertex;
						if (nVertexNum > 2)
						{
							double *pX = new double[pPolyline->nVertex];
							double *pY = new double[pPolyline->nVertex];
							vecPointNum.push_back(pPolyline->nVertex);

							memset(pX, 0, pPolyline->nVertex*sizeof(double));
							memset(pY, 0, pPolyline->nVertex*sizeof(double));

							double minx = 0, miny = 0, maxx = 0, maxy = 0;
							double *ppX = new double[5];
							double *ppY = new double[5];
							for (int nIndex = 0; nIndex < pPolyline->nVertex; ++ nIndex)
							{
								pX[nIndex] = pPolyline->pVertex[nIndex].Point.x;
								pY[nIndex] = pPolyline->pVertex[nIndex].Point.y;
								


								if (lfMinx == 0 || lfMinx > pX[nIndex])
								{
									lfMinx = pX[nIndex];
								}
								if (lfMiny == 0 || lfMiny > pY[nIndex])
								{
									lfMiny = pY[nIndex];
								}
								if (lfMaxx == 0 || lfMaxx < pX[nIndex])
								{
									lfMaxx = pX[nIndex];
								}
								if (lfMaxy == 0 || lfMaxy < pY[nIndex])
								{
									lfMaxy = pY[nIndex];
								}

								if (minx == 0 || minx > pX[nIndex])
								{
									minx = pX[nIndex];
								}
								if (miny == 0 || miny > pY[nIndex])
								{
									miny = pY[nIndex];
								}
								if (maxx == 0 || maxx < pX[nIndex])
								{
									maxx = pX[nIndex];
								}
								if (maxy == 0 || maxy < pY[nIndex])
								{
									maxy = pY[nIndex];
								}
							}

							ppX[0] = minx;
							ppX[1] = minx;
							ppX[2] = maxx;
							ppX[3] = maxx;
							ppX[4] = minx;
							ppY[0] = maxy;
							ppY[1] = miny;
							ppY[2] = miny;
							ppY[3] = maxy;
							ppY[4] = maxy;

							vecEXx.push_back(ppX);
							vecExy.push_back(ppY);

							vecX.push_back(pX);
							vecY.push_back(pY);
							++nPolygonCount;
						}
					}
				}
				hEntity = m_dxffile.FindEntity(FIND_NEXT, &EntityHeader, EntityData, NULL);
			}
			m_dxffile.Destroy();

			
			double cellsize = 2.0;

			double lfXOrigin = int(lfMinx/cellsize)*cellsize;
			double lfYOrigin = int(lfMiny/cellsize)*cellsize;
			double lfXEnd = int(lfMaxx/cellsize+1)*cellsize;
			double lfYEnd = int(lfMaxy/cellsize+1)*cellsize;
			int nXSize = int((lfXEnd-lfXOrigin)/cellsize);
			int nYSize = int((lfYEnd-lfYOrigin)/cellsize);

			std::fstream out;
			out<<std::fixed;
			out.open("D:\\out_2m.dem", std::ios::out);
			out<<"NSDTF-DEM\n";
			out<<"1.0\n";
			out<<"M\n";
			out<<"0.000000\n";
			out<<"0.000000\n";
			out<<lfXOrigin<<"\n";
			out<<lfYEnd<<"\n";
			out<<cellsize<<"\n";
			out<<cellsize<<"\n";
			out<<nYSize<<"\n";
			out<<nXSize<<"\n";
			out<<"1\n";

			float high = 100.0;
			float low = 0.0;

			float* pbuf = new float[nXSize];

			for (int y = 0; y < nYSize; ++y)
			{
				memset(pbuf, 0, nXSize*sizeof(float));
				for (int x = 0; x < nXSize; ++x)
				{
					auto itex = vecX.begin();
					auto itey = vecY.begin();
					auto itenum = vecPointNum.begin();
					auto itexx = vecEXx.begin();
					auto iteyy = vecExy.begin();
					bool isIn = false;

					while(itex != vecX.end())
					{
						isIn = isIn || (-1 != PtInRegionEx(lfXOrigin+x*cellsize, lfYEnd-y*cellsize, *itexx, *iteyy, 5, 0.000001) &&
							-1 != PtInRegionEx(lfXOrigin+x*cellsize, lfYEnd-y*cellsize, *itex, *itey, *itenum, 0.000001));
						if (isIn)
						{
							break;
						}
						++itex;
						++itey;
						++itenum;
						++itexx;
						++iteyy;
					}
					if (isIn)
					{
						pbuf[x] = high;
					}
				}

				for (int i = 0; i < nXSize-1; ++i)
				{
					out<<pbuf[i]<<" ";
				}
				out<<pbuf[nXSize-1]<<"\n";
			}

			delete []pbuf;
			pbuf = NULL;
			out<<"\n";

			out.close();
			auto itex = vecX.begin();
			auto itey = vecY.begin();
			auto itexx = vecEXx.begin();
			auto iteyy = vecExy.begin();
			while(itex != vecX.end())
			{
				delete [](*itex);
				delete [](*itey);
				delete [](*itexx);
				delete [](*iteyy);
				++itex;
				++itey;
				++itexx;
				++iteyy;
			}
			vecX.clear();
			vecY.clear();
			vecPointNum.clear();
		}
		else
		{
			CString temp = _T("加载dxf失败!");
			AfxMessageBox(temp);
			return;
		}
	}
}

double CalDistance(double x1, double y1, double x2, double y2)
{
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}

void CRsDoc::OnOptimize()
{
	auto path_ite = m_vecImagePath.begin();
	std::fstream infile;
	std::vector<PolygonExt2> polygons;
	CString path;
	while (path_ite != m_vecImagePath.end())
	{
		CString image_path = *path_ite;
		path = image_path.Left(image_path.ReverseFind('\\')+1);
		CString index_name = image_path.Right(image_path.GetLength()-image_path.ReverseFind('\\')-1);
		index_name = index_name.Left(index_name.ReverseFind('.'));
		CString rrlx_path = _T("D:\\output\\")+index_name+_T(".rrlx");
		infile.open(rrlx_path.GetBuffer(0), std::ios::in);

		int point_count = 0;
		infile>>point_count;

		double* px = new double[point_count];
		memset(px, 0, sizeof(double)*point_count);
		double* py = new double[point_count];
		memset(py, 0, sizeof(double)*point_count);

		int temp = 0;
		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i]>>temp;
		}

		polygons.push_back(PolygonExt2(point_count, px, py, index_name));

		infile.close();
		++path_ite;
	}

	auto polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int num = polygon_ite->point_count_;

		for (auto polygon_ite2 = polygons.begin();
			polygon_ite2 < polygons.end();
			++polygon_ite2)
		{
			if (polygon_ite2 != polygon_ite)
			{
				double* px2 = polygon_ite2->px_;
				double* py2 = polygon_ite2->py_;
				int num2 = polygon_ite2->point_count_;
				for (int n = 0; n < num; ++n)
				{
					for (int m = 0; m < num2; ++m)
					{
						if (fabs(px[n]-px2[m]) < 0.000001 && fabs(py[n] - py2[m]) < 0.000001)
						{
							polygon_ite->np_[n].index_name_n_[polygon_ite->np_[n].shared_by_-1] = polygon_ite2->index_name_;
							++(polygon_ite->np_[n].shared_by_);
						}
					}
				}
			}
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		auto ite = polygon_ite->np_.begin();
		while (ite != polygon_ite->np_.end())
		{
			if (ite == polygon_ite->np_.begin())
			{
				if (ite->shared_by_ == 2 && polygon_ite->np_.back().shared_by_ != 1
					&& (ite+1)->shared_by_ != 1
					&& NotImportant(*ite, polygon_ite->np_.back(), *(ite+1)))
				{
					ite->available_ = false;
				}
			}
			else if (ite == polygon_ite->np_.end()-1)
			{
				if (ite->shared_by_ == 2 && (ite-1)->shared_by_ != 1
					&& polygon_ite->np_.front().shared_by_ != 1
					&& NotImportant(*ite, *(ite-1), polygon_ite->np_.front()))
				{
					ite->available_ = false;
				}
			}
			else
			{
				if (ite->shared_by_ == 2 && (ite-1)->shared_by_ != 1
					&& (ite+1)->shared_by_ != 1
					&& NotImportant(*ite, *(ite+1), *(ite-1)))
				{
					ite->available_ = false;
				}
			}
			++ite;
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->DeletePoint();
		polygon_ite->Output("D:\\output\\");
		//polygon_ite->Free();
		++polygon_ite;
	}

	std::vector<PolygonExt2> EffPolygons;
	path_ite = m_vecImagePath.begin();
	while (path_ite != m_vecImagePath.end())
	{
		CString index_name = *path_ite;
		index_name = index_name.Right(index_name.GetLength()-index_name.ReverseFind('\\')-1);
		index_name = index_name.Left(index_name.ReverseFind('.'));

		CString ep_name = *path_ite+_T(".ep");
		std::fstream infile;
		infile.open(ep_name.GetBuffer(0), std::ios::in);
		int point_count = 0;
		infile>>point_count;
		double* px = new double[point_count];
		double* py = new double[point_count];

		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i];
		}
		EffPolygons.push_back(PolygonExt2(point_count, px, py, index_name));

		infile.close();
		++path_ite;
	}

	
	polygon_ite = polygons.begin();
	while(polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int point_count = polygon_ite->point_count_;

		for (int i = 0; i < point_count; ++i)
		{
			int shared_by = polygon_ite->np_[i].shared_by_;
			double geox = px[i];
			double geoy = py[i];
			if (shared_by >= 3)
			{
				Paths polys(shared_by);
				auto tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
					PolygonExt2(0, NULL, NULL, polygon_ite->index_name_));
				if (tempoly == EffPolygons.end())
				{
					return;
				}
				for (int n = 0; n < tempoly->point_count_; ++n)
				{
					polys[0]<<IntPoint(tempoly->px_[n]*10, tempoly->py_[n]*10);
				}
				for (int s = 0; s < shared_by-1; ++s)
				{
					tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
						PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[s]));
					if (tempoly == EffPolygons.end())
					{
						return;
					}
					for (int n = 0; n < tempoly->point_count_; ++n)
					{
						polys[s+1]<<IntPoint(tempoly->px_[n]*10, tempoly->py_[n]*10);
					}
				}
				Paths result;
				Clipper clip;
				clip.AddPath(polys[0], ptSubject, true);
				for (int p = 1; p < shared_by; ++p)
				{
					clip.AddPath(polys[p], ptClip, true);
					clip.Execute(ctIntersection, result);
					if (p != shared_by-1)
					{
						clip.Clear();
						clip.AddPath(result[0], ptSubject, true);
						result.clear();
					}
				}
				double* limit_poly_x = new double[shared_by];
				double* limit_poly_y = new double[shared_by];
				memset(limit_poly_x, 0, sizeof(double)*shared_by);
				memset(limit_poly_y, 0, sizeof(double)*shared_by);

				limit_poly_x[0] = px[(i+1)%point_count];
				limit_poly_y[0] = py[(i+1)%point_count];
				limit_poly_x[1] = px[(i-1+point_count)%point_count];
				limit_poly_y[1] = py[(i-1+point_count)%point_count];
				//获取相邻点所围成的区域点
				if (shared_by == 3)
				{
					auto limit_ite = std::find(polygons.begin(), polygons.end(),
						PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[0]));
					if (limit_ite != polygons.end())
					{
						int temp_count = limit_ite->point_count_;
						for (int count = 0; count < temp_count; ++count)
						{
							if (fabs(limit_ite->px_[count]-px[i]) < 0.000001 &&
								fabs(limit_ite->py_[count]-py[i]) < 0.000001)
							{
								if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									break;
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									break;
								}
								else
								{
									if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) < 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) < 0.000001)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
										break;
									}
									else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) < 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) < 0.000001)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
										break;
									}
									else
									{
										return;
									}
								}
							}
						}
					}
				}
				else if (shared_by == 4)
				{
					for (int temp = 0; temp < 2; ++temp)
					{
						auto limit_ite = std::find(polygons.begin(), polygons.end(),
							PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[temp]));
						if (limit_ite == polygons.end())
						{
							return;
						}
						int temp_count = limit_ite->point_count_;
						for (int count = 0; count < temp_count; ++count)
						{
							if (fabs(limit_ite->px_[count]-px[i]) < 0.000001 &&
								fabs(limit_ite->py_[count]-py[i]) < 0.000001)
							{
								if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[2]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[2]) < 0.000001)
								{
									limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
								}
								else
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}

								if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[2]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[2]) < 0.000001)
								{
									if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) > 0.000001 &&
										fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) > 0.000001 &&
										fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) > 0.000001 &&
										fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) > 0.000001)
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else
								{
									if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) > 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) > 0.000001 &&
										fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) > 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) > 0.000001)
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}

								if (limit_poly_x[2] == 0)
								{
									return;
								}
								else if (limit_poly_x[3] != 0)
								{
									break;
								}
							}
						}
					}
				}
				else
				{
					return;
				}//获取区域结束

				if (shared_by == 4)
				{
					double d1 = 0, d2 = 0, d3 = 3, d4 = 0;
					d1 = (limit_poly_x[2]-limit_poly_x[1])*(limit_poly_y[0]-limit_poly_y[1])
						-(limit_poly_x[0]-limit_poly_x[1])*(limit_poly_y[2]-limit_poly_y[1]);
					d2 = (limit_poly_x[2]-limit_poly_x[1])*(limit_poly_y[3]-limit_poly_y[1])
						-(limit_poly_x[3]-limit_poly_x[1])*(limit_poly_y[2]-limit_poly_y[1]);
					d3 = (limit_poly_x[3]-limit_poly_x[0])*(limit_poly_y[1]-limit_poly_y[0])
						-(limit_poly_x[1]-limit_poly_x[0])*(limit_poly_y[3]-limit_poly_y[0]);
					d4 = (limit_poly_x[3]-limit_poly_x[0])*(limit_poly_y[2]-limit_poly_y[0])
						-(limit_poly_x[2]-limit_poly_x[0])*(limit_poly_y[3]-limit_poly_y[0]);
					if (d1*d2 < 0 && d3*d4 < 0)
					{
						double temp = limit_poly_x[2];
						limit_poly_x[2] = limit_poly_x[3];
						limit_poly_x[3] = temp;
						temp = limit_poly_y[2];
						limit_poly_y[2] = limit_poly_y[3];
						limit_poly_y[3] = temp;
					}
				}

				Path poly;
				for (int count = 0; count < shared_by; ++count)
				{
					poly<<IntPoint(limit_poly_x[count]*10, limit_poly_y[count]*10);
				}

				clip.Clear();
				clip.AddPath(result[0], ptSubject, true);
				result.clear();
				clip.AddPath(poly, ptClip, true);
				clip.Execute(ctIntersection, result);

				delete []limit_poly_x;
				delete []limit_poly_y;
				limit_poly_x = NULL;
				limit_poly_y = NULL;

				if (result.size() == 1)
				{
					int count = result[0].size();
					double* tempx = new double[count];
					double* tempy = new double[count];
					memset(tempx, 0, sizeof(double)*count);
					memset(tempy, 0, sizeof(double)*count);
					for (int p = 0; p < count; ++p)
					{
						tempx[p] = result[0][p].X/10.0;
						tempy[p] = result[0][p].Y/10.0;
					}

					if (-1 == PtInRegionEx(geox, geoy, tempx, tempy, count, 0.000001))
					{
						int min_index_1 = 0, min_index_2 = 0;
						double min_distance_1 = 0, min_distance_2 = 0;

						for (int m = 0; m < count; ++m)
						{
							double temp_distance = CalDistance(geox, geoy, tempx[m], tempy[m]);
							if (min_distance_1 == 0 || min_distance_1 > temp_distance)
							{
								min_distance_2 = min_distance_1;
								min_index_2 = min_index_1;
								min_distance_1 = temp_distance;
								min_index_1 = m;
							}
							else if (min_distance_2 == 0 || min_distance_2 > temp_distance)
							{
								min_distance_2 = temp_distance;
								min_index_2 = m;
							}
						}
						double new_geox = 0, new_geoy = 0;
						if (abs(min_index_1-min_index_2) == 1)
						{
							new_geox = tempx[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
								tempx[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
							new_geoy = tempy[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
								tempy[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
						}
						else
						{
							int next_index = (min_index_1+1)%count;
							double temp_distance = CalDistance(geox, geoy, tempx[next_index], tempy[next_index]);
							new_geox = tempx[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
								tempx[next_index]*min_distance_1/(min_distance_1+temp_distance);
							new_geoy = new_geoy = tempy[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
								tempy[next_index]*min_distance_1/(min_distance_1+temp_distance);
						}

						auto poly = polygons.begin();
						while (poly != polygons.end())
						{
							poly->ResetPoint("", geox, geoy, new_geox, new_geoy);
							++poly;
						}
					}
					delete []tempx;
					tempx = NULL;
					delete []tempy;
					tempy = NULL;
				}
				else if (result.size() > 0)
				{
					//AfxMessageBox("Some thing went wrong!");
				}
				else
				{
					//AfxMessageBox("No intersection!");
				}
			}
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->Output("D:\\output\\");
		//polygon_ite->Free();
		++polygon_ite;
	}

	IImageX* pImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	pImage->Open(_bstr_t("D:\\out.tif"), modeRead);
	double resolution = 0;
	double lfXOrigin = 0, lfYOrigin = 0;
	pImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &resolution);
	int nWidth = 0, nHeight = 0;
	pImage->GetCols(&nWidth);
	pImage->GetRows(&nHeight);
	double lfXEnd = 0, lfYEnd = 0;
	lfXEnd = lfXOrigin+nWidth*resolution;
	lfYEnd = lfYOrigin+nHeight*resolution;

	double pEdgex[4];
	double pEdgey[4];

	pEdgex[0] = lfXOrigin;
	pEdgex[1] = lfXOrigin;
	pEdgex[2] = lfXEnd;
	pEdgex[3] = lfXEnd;
	pEdgey[0] = lfYEnd;
	pEdgey[1] = lfYOrigin;
	pEdgey[2] = lfYOrigin;
	pEdgey[3] = lfYEnd;

	IImageX* tempImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&tempImage);

	//const int blockArea = 50;
	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int num = polygon_ite->point_count_;

		CString image_path = path+polygon_ite->index_name_+_T(".tif");
		tempImage->Open(image_path.AllocSysString(), modeRead);
		RectFExt the_rect;
		int nXSize = 0, nYSize = 0;
		double lfCellSize = 0;
		double lfXOrigin = 0, lfYOrigin = 0;

		tempImage->GetCols(&nXSize);
		tempImage->GetRows(&nYSize);
		tempImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfCellSize);
		tempImage->Close();

		the_rect.left = lfXOrigin;
		the_rect.right = lfXOrigin+nXSize*lfCellSize;
		the_rect.bottom = lfYOrigin;
		the_rect.top = lfYOrigin+nYSize*lfCellSize;

		for (int i = 0; i < num; ++i)
		{
			double geox = px[i];
			double geoy = py[i];
			float fx = 0, fy = 0;
			pImage->World2Image(geox, geoy, &fx, &fy);
			unsigned char height;
			pImage->ReadImg((int)fx, (int)fy, (int)(fx+1), (int)(fy+1),
				&height, 1, 1, 1, 0, 0,
				1, 1, -1, 0);
			if (height == 0)
			{
				continue;
			}
			RectFExt result_result = the_rect;
			if (polygon_ite->np_[i].shared_by_ >= 2)
			{
				for (int j = 0; j < polygon_ite->np_[i].shared_by_-1; ++j)
				{
					image_path = path+polygon_ite->np_[i].index_name_n_[j]+_T(".tif");
					tempImage->Open(image_path.AllocSysString(), modeRead);
					RectFExt temp_rect;

					int nx = 0, ny = 0;
					double cellsize = 0;
					double xorigin = 0, yorigin = 0;

					tempImage->GetCols(&nx);
					tempImage->GetRows(&ny);
					tempImage->GetGrdInfo(&xorigin, &yorigin, &cellsize);
					tempImage->Close();

					temp_rect.left = xorigin;
					temp_rect.right = xorigin+nx*cellsize;
					temp_rect.bottom = yorigin;
					temp_rect.top = yorigin+ny*cellsize;

					result_result = result_result.Intersected(temp_rect);
				}

				int blockArea = min(result_result.Width(), result_result.Height())/resolution;

				//获取公共区域的中心
				double intersect_center_x = 0, intersect_center_y = 0;
				intersect_center_x = (result_result.left+result_result.right)/2;
				intersect_center_y = (result_result.top+result_result.bottom)/2;


				//限制buffer范围
				RectFExt buf_rect;
				buf_rect.left = px[i]-blockArea*resolution;
				buf_rect.right = px[i]+blockArea*resolution;
				buf_rect.bottom = py[i]-blockArea*resolution;
				buf_rect.top = py[i]+blockArea*resolution;
				buf_rect = buf_rect.Intersected(result_result);
				if (buf_rect.IsEmpty())
				{
					continue;
				}
				float buffer_left = 0, buffer_right = 0,
					buffer_bottom = 0, buffer_top = 0;
				pImage->World2Image(buf_rect.left, buf_rect.bottom,
					&buffer_left, &buffer_top);
				pImage->World2Image(buf_rect.right, buf_rect.top,
					&buffer_right, &buffer_bottom);

				float intersect_center_buffer_x = 0, intersect_center_buffer_y = 0;
				pImage->World2Image(intersect_center_x, intersect_center_y,
					&intersect_center_buffer_x, &intersect_center_buffer_y);

				int buffer_height = int(buffer_bottom-buffer_top+0.99999);
				int buffer_width = int(buffer_right-buffer_left+0.99999);

				unsigned short* buf = new unsigned short[buffer_height*buffer_width];
				memset(buf, 0, buffer_height*buffer_width*sizeof(unsigned short));
				pImage->ReadImg(buffer_left, buffer_top, buffer_right, buffer_bottom,
					(unsigned char*)buf, buffer_width, buffer_height, 1, 0, 0,
					buffer_width, buffer_height, -1, 0);
				int start_col = int(fx-buffer_left+0.99999);
				int start_row = int(fy-buffer_top+0.99999);
				int end_col = int(intersect_center_buffer_x-buffer_left+0.99999);
				int end_row = int(intersect_center_buffer_y-buffer_top+0.99999);

				if (start_col >= buffer_width || start_row >= buffer_height)
				{
					delete []buf;
					buf = NULL;
					continue;
				}
				bool isFind = false;
				int ncount = 0;
				const int count_limit = 8;

				//往公共区域中心移点
				int xoff = 0, yoff = 0;
				xoff = end_col-start_col;
				yoff = end_row-start_row;
				double xite = 0, yite = 0;

				if (abs(xoff) > abs(yoff)  && xoff != 0)
				{
					yite = yoff/(double)abs(xoff);
					xite = xoff > 0 ? 1 : -1;
				}
				else if (abs(yoff) > abs(xoff) && yoff != 0)
				{
					xite = xoff/(double)abs(yoff);
					yite = yoff > 0 ? 1 : -1;
				}
				else if (xoff = 0)
				{
					xite = 0;
					yite = yoff >= 0 ? 1 : -1;
				}
				else if (yoff = 0)
				{
					yite = 0;
					xite = xite >= 0 ? 1 : -1;
				}
				else if (abs(xoff) == abs(yoff))
				{
					xite = xite >= 0 ? 1 : -1;
					yite = yoff >= 0 ? 1 : -1;
				}

				int limit_x = 0, limit_y = 0;
				if (end_col < start_col)
				{
					limit_x = min(end_col, 0);
				}
				else if (end_col > start_col)
				{
					limit_x = max(end_col, buffer_width);
				}
				if (limit_x < 0)
				{
					limit_x = 0;
				}
				else if (limit_x > buffer_width)
				{
					limit_x = buffer_width;
				}
				if (end_row < start_row)
				{
					limit_y = min(end_row, 0);
				}
				else if (end_row > start_row)
				{
					limit_y = max(end_row, buffer_height);
				}
				if (limit_y < 0)
				{
					limit_y = 0;
				}
				else if (limit_y > buffer_height)
				{
					limit_y = buffer_height;
				}

				double findx = start_col, findy = start_row;
				if (xite == 0)
				{
					while ((findy-limit_y)*(findy-yite-limit_y) > 0)
					{
						if (buf[int(findy)*buffer_width+int(findx)] != 0)
						{
							ncount = 0;
						}
						else
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox+(findx-start_col)*resolution, geoy+(findy-start_row)*resolution);
									++poly;
								}
								break;
							}
						}
						findy += yite;
						if (findy < 0 || int(findy) >= buffer_width)
						{
							break;
						}
					}
				}
				else if (yite == 0)
				{
					while ((findx-limit_x)*(findx-xite-limit_x) > 0)
					{
						if (buf[int(findy)*buffer_width+int(findx)] != 0)
						{
							ncount = 0;
						}
						else
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox+(findx-start_col)*resolution, geoy+(findy-start_row)*resolution);
									++poly;
								}
								break;
							}
						}
						findx += xite;
						if (findx < 0 || int(findx) >= buffer_width)
						{
							break;
						}
					}
				}
				else
				{
					while ((findx-limit_x)*(findx-xite-limit_x) > 0 && (findy-limit_y)*(findy-yite-limit_y) > 0)
					{
						if (buf[int(findy)*buffer_width+int(findx)] != 0)
						{
							ncount = 0;
						}
						else
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox+(findx-start_col)*resolution, geoy+(findy-start_row)*resolution);
									++poly;
								}
								break;
							}
						}
						findy += yite;
						findx += xite;
						if (findx < 0 || int(findx) >= buffer_width)
						{
							break;
						}
						if (findy < 0 || int(findy) >= buffer_width)
						{
							break;
						}
					}
				}

				/*
				if (!isFind)
				{
					ncount = 0;
					for (int f = start_col-1; f >= 0; --f)
					{
						if (buf[start_row*buffer_width+f] != 0 && ncount != 0)
						{
							break;
						}
						if (buf[start_row*buffer_width+f] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox-(start_col-f)*resolution, geoy);
									++poly;
								}
								break;
							}
						}
					}
				}
				if (!isFind)
				{
					ncount = 0;
					for (int f = start_col+1; start_col < buffer_width; ++f)
					{
						if (buf[start_row*buffer_width+f] != 0 && ncount != 0)
						{
							break;
						}
						if (buf[start_row*buffer_width+f] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox+(f-start_col)*resolution, geoy);
									++poly;
								}
								break;
							}
						}
					}
				}
				if (!isFind)
				{
					ncount = 0;
					for (int f = start_row-1; f >= 0; --f)
					{
						if (buf[start_row*buffer_width+f] != 0 && ncount != 0)
						{
							break;
						}
						if (buf[f*buffer_width+start_row] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox, geoy-(start_row-f)*resolution);
									++poly;
								}
								break;
							}
						}
					}
				}
				if (!isFind)
				{
					ncount = 0;
					for (int f = start_row+1; f < buffer_height; ++f)
					{
						if (buf[start_row*buffer_width+f] != 0 && ncount != 0)
						{
							break;
						}
						if (buf[f*buffer_width+start_row] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								auto poly = polygons.begin();
								while (poly != polygons.end())
								{
									poly->ResetPoint("", geox, geoy,
										geox, geoy+(f-start_row)*resolution);
									++poly;
								}
								break;
							}
						}
					}
				}*/
				delete []buf;
				buf = NULL;
			}
		}
		++polygon_ite;
	}

	CString strDxf = _T("D:\\data\\resample\\房屋.dxf");

	std::vector<int> vecPointNum;
	std::vector<double*> vecX;
	std::vector<double*> vecY;

	if (m_dxffile.Create() && m_dxffile.LoadDXFFile(strDxf) == TRUE)
	{
		ENTITYHEADER EntityHeader;
		char	 EntityData[4096];
		OBJHANDLE hEntity;
		hEntity = m_dxffile.FindEntity(FIND_FIRST, &EntityHeader, EntityData, NULL);

		double lfMinx = 0, lfMiny = 0, lfMaxx = 0, lfMaxy = 0;

		while (hEntity)
		{
			switch(EntityHeader.EntityType)
			{
			case ENT_POLYLINE:
			case ENT_LINE3D:
				{
					PENTPOLYLINE pPolyline = (PENTPOLYLINE)EntityData;
					int nVertexNum = pPolyline->nVertex;
					if (nVertexNum > 2)
					{
						double *pX = new double[pPolyline->nVertex];
						double *pY = new double[pPolyline->nVertex];
						vecPointNum.push_back(pPolyline->nVertex);

						memset(pX, 0, pPolyline->nVertex*sizeof(double));
						memset(pY, 0, pPolyline->nVertex*sizeof(double));

						double minx = 0, miny = 0, maxx = 0, maxy = 0;
						double *ppX = new double[5];
						double *ppY = new double[5];
						for (int nIndex = 0; nIndex < pPolyline->nVertex; ++ nIndex)
						{
							pX[nIndex] = pPolyline->pVertex[nIndex].Point.x;
							pY[nIndex] = pPolyline->pVertex[nIndex].Point.y;
						}
						vecX.push_back(pX);
						vecY.push_back(pY);
					}
				}
			}
			hEntity = m_dxffile.FindEntity(FIND_NEXT, &EntityHeader, EntityData, NULL);
		}
		m_dxffile.Destroy();
	}

	IShortPaths* shortpath = NULL;

	CoCreateInstance(CLSID_ShortPaths, NULL, CLSCTX_ALL, IID_IShortPaths, (void**)&shortpath);

#define NEXT(a) ((a+1==polygon_ite->np_.end()) ? (polygon_ite->np_.begin()) : (a+1))

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int point_count = polygon_ite->point_count_;

		auto ite = polygon_ite->np_.begin();
		int point_index = 0;

		while (ite != polygon_ite->np_.end())
		{
			if (ite->shared_by_ > 1 && ite->available_)
			{
				if ((NEXT(ite))->shared_by_ > 1 && (NEXT(ite))->available_)
				{
					double short_start_x = 0, short_start_y = 0, short_end_x = 0, short_end_y = 0;
					if (-1 != PtInRegionEx(px[point_index], py[point_index], pEdgex, pEdgey, 4, 1e-5) &&
						-1 != PtInRegionEx(px[(point_index+1)%point_count], py[(point_index+1)%point_count], pEdgex, pEdgey, 4, 1e-5))
					{
						short_start_x = px[point_index];
						short_start_y = py[point_index];
						short_end_x = px[(point_index+1)%point_count];
						short_end_y = py[(point_index+1)%point_count];
					}
					else
					{
						double result_x = 0, result_y = 0;
						if (GetCrossPoint(px[point_index], py[point_index], px[(point_index+1)%point_count],
							py[(point_index+1)%point_count], pEdgex, pEdgey, 4, result_x, result_y))
						{
							if (-1 != PtInRegionEx(px[point_index], py[point_index], pEdgex, pEdgey, 4, 1e-5))
							{
								short_start_x = px[point_index];
								short_start_y = py[point_index];
								short_end_x = result_x;
								short_end_y = result_y;
							}
							else if (-1 != PtInRegionEx(px[(point_index+1)%point_count], py[(point_index+1)%point_count], pEdgex, pEdgey, 4, 1e-5))
							{
								short_start_x = result_x;
								short_start_y = result_y;
								short_end_x = px[(point_index+1)%point_count];
								short_end_y = py[(point_index+1)%point_count];
							}
							else
							{
								++point_index;
								++ite;
								continue;
							}
						}
						else
						{
							++point_index;
							++ite;
							continue;
						}
					}

					//判断连线本身是否穿过任何建筑物
					bool is_cross_building = LineCrossPolygon(vecX, vecY, vecPointNum,
						short_start_x, short_start_y, short_end_x, short_end_y);

					if (is_cross_building == false)
					{
						++point_index;
						++ite;
						continue;
					}

					//找出另一关联影像
					CString strIndexName = "";
					for (int name_index = 0; name_index < ite->shared_by_-1; ++name_index)
					{
						for (int name_index2 = 0; name_index2 < (NEXT(ite))->shared_by_-1; ++name_index2)
						{
							if (ite->index_name_n_[name_index] == (NEXT(ite))->index_name_n_[name_index2])
							{
								strIndexName = ite->index_name_n_[name_index];
								break;
							}
						}
						if (strIndexName != "")
						{
							break;
						}
					}

					//取有效多边形求交
					auto poly = std::find(EffPolygons.begin(), EffPolygons.end(),
						PolygonExt2(0, NULL, NULL, polygon_ite->index_name_));
					if (poly == EffPolygons.end())
					{
						continue;
					}
					Path subj;
					for (int count = 0; count < poly->point_count_; ++count)
					{
						subj<<IntPoint(int(poly->px_[count]*10), int(poly->py_[count]*10));
					}
					poly = std::find(EffPolygons.begin(), EffPolygons.end(),
						PolygonExt2(0, NULL, NULL, strIndexName));
					if (poly == EffPolygons.end())
					{
						continue;
					}
					Path clip;
					for (int count = 0; count < poly->point_count_; ++count)
					{
						clip<<IntPoint(int(poly->px_[count]*10), int(poly->py_[count]*10));
					}

					Clipper c;
					c.AddPath(subj, ptSubject, true);
					c.AddPath(clip, ptClip, true);
					Paths effect;
					c.Execute(ctIntersection, effect);

					subj.clear();
					clip.clear();
					c.Clear();

					for (int count = 0; count < polygon_ite->point_count_; ++count)
					{
						subj<<IntPoint(int(polygon_ite->px_[count]*10), int(polygon_ite->py_[count]*10));
					}

					poly = std::find(polygons.begin(), polygons.end(),
						PolygonExt2(0, NULL, NULL, strIndexName));
					if (poly == polygons.end())
					{
						continue;
					}
					for (int count = 0; count < poly->point_count_; ++count)
					{
						clip<<IntPoint(int(poly->px_[count]*10), int(poly->py_[count]*10));
					}

					c.AddPath(subj, ptSubject, true);
					c.AddPath(clip, ptClip, true);
					Paths temp;
					c.Execute(ctUnion, temp);

					c.Clear();

					c.AddPaths(effect, ptSubject, true);
					c.AddPaths(temp, ptClip, true);
					Paths result;
					c.Execute(ctIntersection, result);

					if (result.size() == 0)
					{
						continue;
					}

					int effect_point_count = result[0].size();
					double* tempx = new double[effect_point_count];
					memset(tempx, 0, sizeof(double)*effect_point_count);
					double* tempy = new double[effect_point_count];
					memset(tempy, 0, sizeof(double)*effect_point_count);

					for (int count = 0; count < effect_point_count; ++count)
					{
						tempx[count] = result[0][count].X/10.0;
						tempy[count] = result[0][count].Y/10.0;
					}

					//最短路径
					double* lpXout = NULL;
					double* lpYout = NULL;
					long point_count_out = 0;

					if(S_FALSE == shortpath->ShortestPathviaPoly(_bstr_t("D:\\out.dem"), short_start_x, short_start_y,
						short_end_x, short_end_y, tempx, tempy, effect_point_count,
						&lpXout, &lpYout, &point_count_out))
					{
						//返回错误则取图幅范围相交区域和起始点连线的交点作为另一个点走最短路径
						CString image_path = path+polygon_ite->index_name_+_T(".tif");
						tempImage->Open(image_path.AllocSysString(), modeRead);
						RectFExt the_rect;
						int nXSize = 0, nYSize = 0;
						double lfCellSize = 0;
						double lfXOrigin = 0, lfYOrigin = 0;

						tempImage->GetCols(&nXSize);
						tempImage->GetRows(&nYSize);
						tempImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfCellSize);
						tempImage->Close();

						the_rect.left = lfXOrigin;
						the_rect.right = lfXOrigin+nXSize*lfCellSize;
						the_rect.bottom = lfYOrigin;
						the_rect.top = lfYOrigin+nYSize*lfCellSize;

						RectFExt result_result = the_rect;
						for (int j = 0; j < ite->shared_by_-1; ++j)
						{
							image_path = path+ite->index_name_n_[j]+_T(".tif");
							tempImage->Open(image_path.AllocSysString(), modeRead);
							RectFExt temp_rect;

							int nx = 0, ny = 0;
							double cellsize = 0;
							double xorigin = 0, yorigin = 0;

							tempImage->GetCols(&nx);
							tempImage->GetRows(&ny);
							tempImage->GetGrdInfo(&xorigin, &yorigin, &cellsize);
							tempImage->Close();

							temp_rect.left = xorigin;
							temp_rect.right = xorigin+nx*cellsize;
							temp_rect.bottom = yorigin;
							temp_rect.top = yorigin+ny*cellsize;

							result_result = result_result.Intersected(temp_rect);
						}

						double rect_x[4];
						double rect_y[4];
						rect_x[0] = result_result.left;
						rect_x[1] = result_result.left;
						rect_x[2] = result_result.right;
						rect_x[3] = result_result.right;
						rect_y[0] = result_result.top;
						rect_y[1] = result_result.bottom;
						rect_y[2] = result_result.bottom;
						rect_y[3] = result_result.top;
						double result_x = 0, result_y = 0;
						if (GetCrossPoint(short_start_x, short_start_y, short_end_x, short_end_y, rect_x, rect_y, 4, result_x, result_y))
						{
							if (-1 != PtInRegionEx(short_start_x, short_start_y, rect_x, rect_y, 4, 1e-5))
							{
								short_end_x = result_x;
								short_end_y = result_y;
							}
							else if (-1 != PtInRegionEx(short_end_x, short_end_y, rect_x, rect_y, 4, 1e-5))
							{
								short_start_x = result_x;
								short_start_y = result_y;
							}
							else
							{
								delete []tempx;
								tempx = NULL;
								delete []tempy;
								tempy = NULL;
								++point_index;
								++ite;
								continue;
							}
						}
						else
						{
							delete []tempx;
							tempx = NULL;
							delete []tempy;
							tempy = NULL;
							++point_index;
							++ite;
							continue;
						}
						shortpath->ShortestPathviaPoly(_bstr_t("D:\\out.dem"), short_start_x, short_start_y,
							short_end_x, short_end_y, tempx, tempy, effect_point_count,
							&lpXout, &lpYout, &point_count_out);
					}

					delete []tempx;
					tempx = NULL;
					delete []tempy;
					tempy = NULL;

					if (point_count_out >= 2)
					{
						double startx = px[point_index];
						double starty = py[point_index];
						double endx = px[(point_index+1)%point_count];
						double endy = py[(point_index+1)%point_count];
						auto temp_ite = polygons.begin();
						while (temp_ite != polygons.end())
						{
							temp_ite->InsertPoints(startx, starty, endx, endy,
								lpXout, lpYout, point_count_out, strIndexName, polygon_ite->index_name_);
							++temp_ite;
						}
						ite = polygon_ite->np_.begin();
						delete []lpXout;
						lpXout = NULL;
						delete []lpYout;
						lpYout = NULL;
						point_index = 0;
						px = polygon_ite->px_;
						py = polygon_ite->py_;
						point_count = polygon_ite->point_count_;
						continue;
					}//插点结束
				}
			}
			++point_index;
			++ite;
		}

		++polygon_ite;
	}

	vecPointNum.clear();
	auto itex = vecX.begin();
	while (itex != vecX.end())
	{
		delete [](*itex);
		*itex = NULL;
		++itex;
	}
	vecX.clear();
	auto itey = vecY.begin();
	while (itey != vecY.end())
	{
		delete [](*itey);
		*itey = NULL;
		++itey;
	}
	vecY.clear();

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int point_count = polygon_ite->point_count_;
		for (int count = 0; count < point_count; ++count)
		{
			if (polygon_ite->np_[count].available_ == false)
			{
				auto polygon_ite2 = polygons.begin();
				bool isin = false;
				while (polygon_ite2 != polygons.end())
				{
					CString strID = polygon_ite2->index_name_;
					if (strID.CompareNoCase(polygon_ite->np_[count].index_name_n_[0]) != 0 &&
						strID.CompareNoCase(polygon_ite->index_name_) != 0)
					{
						isin |= (-1 != PtInRegionEx(px[count], py[count], polygon_ite2->px_, polygon_ite2->py_, polygon_ite2->point_count_, 0.00001));
					}
					++polygon_ite2;
				}
				if (!isin)
				{
					polygon_ite->np_[count].available_ = true;
				}
			}
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->DeletePoint();
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->Output("D:\\output\\");
		//polygon_ite->Free();
		++polygon_ite;
	}

	pImage->Close();
	pImage->Release();
	tempImage->Release();

	ParsePolygon();
	UpdateAllViews(NULL);

	//OutputResultImg(polygons);
	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		//polygon_ite->Output("D:\\output\\");
		polygon_ite->Free();
		++polygon_ite;
	}
}

double GetDistance(double pointx, double pointy, double linestartx, double linestarty, 
	double lineendx, double lineendy)
{
	double a = linestarty-lineendy;
	double b = lineendx-linestartx;
	double c = linestartx*lineendy-lineendx*linestarty;

	return fabs(a*pointx+b*pointy+c)/(sqrt(a*a+b*b));
}

BOOL OutputEffectivePoly(CString strAllDomPath, int BG_COLOR = 0)
{
	std::vector<CString> vecImagePath;
	while (1)
	{
		int nIndex = strAllDomPath.ReverseFind(';');
		CString strDomPath("");
		if (nIndex == -1)
		{
			strDomPath = strAllDomPath;
			vecImagePath.push_back(strDomPath);
			break;
		}
		else
		{
			strDomPath = strAllDomPath.Right(strAllDomPath.GetLength()-nIndex-1);
			vecImagePath.push_back(strDomPath);
			strAllDomPath = strAllDomPath.Left(nIndex);
		}
	}

	std::vector<CString>::const_iterator image_path = vecImagePath.begin();

	IImageX* pImage = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	if (FAILED(hRes))
	{
		return FALSE;
	}

	while (image_path != vecImagePath.end())
	{
		std::vector<PointEx> point_left;
		std::vector<PointEx> point_right;
		hRes = pImage->Open(image_path->AllocSysString(), modeRead);
		if (hRes == S_FALSE)
		{
			if (pImage)
			{
				pImage->Release();
				pImage = NULL;
			}
			vecImagePath.clear();
			return FALSE;
		}

		int nXSize = 0, nYSize = 0, nBandNum = 0;
		double lfXOrigin = 0, lfYOrigin = 0, lfCellSize = 0;
		int BPB = 0;
		pImage->GetCols(&nXSize);
		pImage->GetRows(&nYSize);
		pImage->GetBandNum(&nBandNum);
		pImage->GetBPB(&BPB);
		pImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfCellSize);

		unsigned char* buffer = new unsigned char[nXSize*nBandNum*BPB];

		for (int y = 0; y < nYSize; y += 15)
		{
			memset(buffer, BG_COLOR, nXSize*nBandNum*BPB);
			pImage->ReadImg(0, y, nXSize, y+1, buffer, nXSize, 1, nBandNum,
				0, 0, nXSize, 1, -1, 0);
			for (int x = 0; x < nXSize; ++x)
			{
				int result = 0;
				if (BPB == 1)
				{
					for (int n = 0; n < nBandNum; ++n)
					{
						result += buffer[x*nBandNum+n];
					}
				}
				else if (BPB == 2)
				{
					unsigned short* data = (unsigned short*)buffer;
					for (int n = 0; n < nBandNum; ++n)
					{
						result += data[x*nBandNum+n];
					}
				}

				if (result != nBandNum*BG_COLOR)
				{
					point_left.push_back(PointEx(x, y));
					break;
				}
			}
			for(int x = nXSize-1; x >= 0; --x)
			{
				int result = 0;
				if (BPB == 1)
				{
					for (int n = 0; n < nBandNum; ++n)
					{
						result += buffer[x*nBandNum+n];
					}
				}
				else if (BPB == 2)
				{
					unsigned short* data = (unsigned short*)buffer;
					for (int n = 0; n < nBandNum; ++n)
					{
						result += data[x*nBandNum+n];
					}
				}

				if (result != nBandNum*BG_COLOR)
				{
					point_right.push_back(PointEx(x, y));
					break;
				}
			}
		}
		delete []buffer;
		buffer = NULL;
		pImage->Close();

		int point_count = point_left.size();
		double* px = new double[point_count];
		double* py = new double[point_count];

		for (int i = 0; i < point_count; ++i)
		{
			px[i] = point_left[i].x;
			py[i] = point_left[i].y;
		}

		point_left.clear();

		double limit = 2.5;
		point_left.push_back(PointEx(px[0], py[0]));
		for (int j = 1, k = 2; k < point_count;)
		{
			if(GetDistance(px[j], py[j], point_left.back().x, point_left.back().y,
				px[k], py[k])-limit < 0.000001)
			{
				++j;
				++k;
			}
			else
			{
				point_left.push_back(PointEx(px[j], py[j]));
				++j;
				++k;
			}
		}
		point_left.push_back(PointEx(px[point_count-1], py[point_count-1]));
		delete []px;
		px = NULL;
		delete []py;
		py = NULL;

		point_count = point_right.size();
		px = new double[point_count];
		py = new double[point_count];

		for (int i = 0; i < point_count; ++i)
		{
			px[i] = point_right[i].x;
			py[i] = point_right[i].y;
		}

		point_right.clear();

		point_right.push_back(PointEx(px[0], py[0]));
		for (int j = 1, k = 2; k < point_count;)
		{
			if(GetDistance(px[j], py[j], point_right.back().x, point_right.back().y,
				px[k], py[k])-limit < 0.000001)
			{
				++j;
				++k;
			}
			else
			{
				point_right.push_back(PointEx(px[j], py[j]));
				++j;
				++k;
			}
		}
		point_right.push_back(PointEx(px[point_count-1], py[point_count-1]));
		delete []px;
		px = NULL;
		delete []py;
		py = NULL;

		std::fstream outfile;
		CString point_path = (*image_path)+_T(".ep");
		outfile.open(point_path.GetBuffer(0), std::ios::out);
		outfile<<std::fixed;
		outfile<<point_left.size()+point_right.size()<<"\n";
		for (int i = 0; i < point_left.size(); ++i)
		{
			outfile<<point_left[i].x*lfCellSize+lfXOrigin<<"   "<<point_left[i].y*lfCellSize+lfYOrigin<<"\n";
		}

		for (int i = point_right.size()-1; i >= 0; --i)
		{
			outfile<<point_right[i].x*lfCellSize+lfXOrigin<<"   "<<point_right[i].y*lfCellSize+lfYOrigin<<"\n";
		}
		outfile.close();

		++image_path;
	}
	vecImagePath.clear();
	pImage->Release();

	return TRUE;
}


void CRsDoc::OnEffectpoly()
{
	CString strAllDomPath;
	auto ite = m_vecImagePath.begin();
	while(ite != m_vecImagePath.end())
	{
		strAllDomPath = strAllDomPath+*ite+_T(";");
		++ite;
	}
	strAllDomPath = strAllDomPath.Left(strAllDomPath.GetLength()-1);
	//OutputEffectivePoly(strAllDomPath, 0);

	ParseEffective();
	UpdateAllViews(NULL);
}

void CRsDoc::OnOptimize2()
{
	auto path_ite = m_vecImagePath.begin();
	std::fstream infile;
	std::vector<PolygonExt2> polygons;
	CString path;
	while (path_ite != m_vecImagePath.end())
	{
		CString image_path = *path_ite;
		path = image_path.Left(image_path.ReverseFind('\\')+1);
		CString index_name = image_path.Right(image_path.GetLength()-image_path.ReverseFind('\\')-1);
		index_name = index_name.Left(index_name.ReverseFind('.'));
		CString rrlx_path = _T("D:\\output\\")+index_name+_T(".rrlx");
		infile.open(rrlx_path.GetBuffer(0), std::ios::in);

		int point_count = 0;
		infile>>point_count;

		double* px = new double[point_count];
		memset(px, 0, sizeof(double)*point_count);
		double* py = new double[point_count];
		memset(py, 0, sizeof(double)*point_count);

		int temp = 0;
		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i]>>temp;
		}

		polygons.push_back(PolygonExt2(point_count, px, py, index_name));

		infile.close();
		++path_ite;
	}

	auto polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int num = polygon_ite->point_count_;

		for (auto polygon_ite2 = polygons.begin();
			polygon_ite2 < polygons.end();
			++polygon_ite2)
		{
			if (polygon_ite2 != polygon_ite)
			{
				double* px2 = polygon_ite2->px_;
				double* py2 = polygon_ite2->py_;
				int num2 = polygon_ite2->point_count_;
				for (int n = 0; n < num; ++n)
				{
					for (int m = 0; m < num2; ++m)
					{
						if (px[n] == px2[m] && py[n] == py2[m])
						{
							polygon_ite->np_[n].index_name_n_[polygon_ite->np_[n].shared_by_-1] = polygon_ite2->index_name_;
							++(polygon_ite->np_[n].shared_by_);
						}
					}
				}
			}
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		auto ite = polygon_ite->np_.begin();
		while (ite != polygon_ite->np_.end())
		{
			if (ite == polygon_ite->np_.begin())
			{
				if (ite->shared_by_ == 2 && polygon_ite->np_.back().shared_by_ != 1
					&& (ite+1)->shared_by_ != 1
					&& NotImportant(*ite, polygon_ite->np_.back(), *(ite+1)))
				{
					ite->available_ = false;
				}
			}
			else if (ite == polygon_ite->np_.end()-1)
			{
				if (ite->shared_by_ == 2 && (ite-1)->shared_by_ != 1
					&& polygon_ite->np_.front().shared_by_ != 1
					&& NotImportant(*ite, *(ite-1), polygon_ite->np_.front()))
				{
					ite->available_ = false;
				}
			}
			else
			{
				if (ite->shared_by_ == 2 && (ite-1)->shared_by_ != 1
					&& (ite+1)->shared_by_ != 1
					&& NotImportant(*ite, *(ite+1), *(ite-1)))
				{
					ite->available_ = false;
				}
			}
			++ite;
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->DeletePoint();
		polygon_ite->Output("D:\\output\\");
		++polygon_ite;
	}

	std::vector<PolygonExt2> EffPolygons;
	path_ite = m_vecImagePath.begin();
	while (path_ite != m_vecImagePath.end())
	{
		CString index_name = *path_ite;
		index_name = index_name.Right(index_name.GetLength()-index_name.ReverseFind('\\')-1);
		index_name = index_name.Left(index_name.ReverseFind('.'));

		CString ep_name = *path_ite+_T(".ep");
		std::fstream infile;
		infile.open(ep_name.GetBuffer(0), std::ios::in);
		int point_count = 0;
		infile>>point_count;
		double* px = new double[point_count];
		double* py = new double[point_count];

		for (int i = 0; i < point_count; ++i)
		{
			infile>>px[i]>>py[i];
		}
		EffPolygons.push_back(PolygonExt2(point_count, px, py, index_name));

		infile.close();
		++path_ite;
	}

// 	std::for_each(EffPolygons.begin(), EffPolygons.end(),
// 		[&](PolygonExt2& poly)
// 	{
// 		ConvexHull(poly);
// 	});


	polygon_ite = polygons.begin();
	while(polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int point_count = polygon_ite->point_count_;

		for (int i = 0; i < point_count; ++i)
		{
			int shared_by = polygon_ite->np_[i].shared_by_;
			double geox = px[i];
			double geoy = py[i];
			if (shared_by >= 3)
			{
				Paths polys(shared_by);
				auto tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
					PolygonExt2(0, NULL, NULL, polygon_ite->index_name_));
				if (tempoly == EffPolygons.end())
				{
					return;
				}
				for (int n = 0; n < tempoly->point_count_; ++n)
				{
					polys[0]<<IntPoint(tempoly->px_[n]*10, tempoly->py_[n]*10);
				}
				for (int s = 0; s < shared_by-1; ++s)
				{
					tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
						PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[s]));
					if (tempoly == EffPolygons.end())
					{
						return;
					}
					for (int n = 0; n < tempoly->point_count_; ++n)
					{
						polys[s+1]<<IntPoint(tempoly->px_[n]*10, tempoly->py_[n]*10);
					}
				}
				Paths result;
				Clipper clip;
				clip.AddPath(polys[0], ptSubject, true);
				for (int p = 1; p < shared_by; ++p)
				{
					clip.AddPath(polys[p], ptClip, true);
					clip.Execute(ctIntersection, result);
					if (p != shared_by-1)
					{
						clip.Clear();
						clip.AddPath(result[0], ptSubject, true);
						result.clear();
					}
				}
				double* limit_poly_x = new double[shared_by];
				double* limit_poly_y = new double[shared_by];
				memset(limit_poly_x, 0, sizeof(double)*shared_by);
				memset(limit_poly_y, 0, sizeof(double)*shared_by);

				limit_poly_x[0] = px[(i+1)%point_count];
				limit_poly_y[0] = py[(i+1)%point_count];
				limit_poly_x[1] = px[(i-1+point_count)%point_count];
				limit_poly_y[1] = py[(i-1+point_count)%point_count];
				//获取相邻点所围成的区域点
				if (shared_by == 3)
				{
					auto limit_ite = std::find(polygons.begin(), polygons.end(),
						PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[0]));
					if (limit_ite != polygons.end())
					{
						int temp_count = limit_ite->point_count_;
						for (int count = 0; count < temp_count; ++count)
						{
							if (fabs(limit_ite->px_[count]-px[i]) < 0.000001 &&
								fabs(limit_ite->py_[count]-py[i]) < 0.000001)
							{
								if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									break;
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									break;
								}
								else
								{
									if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) < 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) < 0.000001)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
										break;
									}
									else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) < 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) < 0.000001)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
										break;
									}
									else
									{
										return;
									}
								}
							}
						}
					}
				}
				else if (shared_by == 4)
				{
					for (int temp = 0; temp < 2; ++temp)
					{
						auto limit_ite = std::find(polygons.begin(), polygons.end(),
							PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[temp]));
						if (limit_ite == polygons.end())
						{
							return;
						}
						int temp_count = limit_ite->point_count_;
						for (int count = 0; count < temp_count; ++count)
						{
							if (fabs(limit_ite->px_[count]-px[i]) < 0.000001 &&
								fabs(limit_ite->py_[count]-py[i]) < 0.000001)
							{
								if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count+1)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[2]) < 0.000001 &&
									fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[2]) < 0.000001)
								{
									limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
									limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
								}
								else
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}

								if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) < 0.000001)
								{
									if (limit_poly_x[2] == 0)
									{
										limit_poly_x[2] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[2] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
									else
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[2]) < 0.000001 &&
									fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[2]) < 0.000001)
								{
									if (fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[0]) > 0.000001 &&
										fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[0]) > 0.000001 &&
										fabs(limit_ite->px_[(count-1+temp_count)%temp_count]-limit_poly_x[1]) > 0.000001 &&
										fabs(limit_ite->py_[(count-1+temp_count)%temp_count]-limit_poly_y[1]) > 0.000001)
									{
										limit_poly_x[3] = limit_ite->px_[(count-1+temp_count)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count-1+temp_count)%temp_count];
									}
								}
								else
								{
									if (fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[0]) > 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[0]) > 0.000001 &&
										fabs(limit_ite->px_[(count+1)%temp_count]-limit_poly_x[1]) > 0.000001 &&
										fabs(limit_ite->py_[(count+1)%temp_count]-limit_poly_y[1]) > 0.000001)
									{
										limit_poly_x[3] = limit_ite->px_[(count+1)%temp_count];
										limit_poly_y[3] = limit_ite->py_[(count+1)%temp_count];
									}
								}

								if (limit_poly_x[2] == 0)
								{
									return;
								}
								else if (limit_poly_x[3] != 0)
								{
									break;
								}
							}
						}
					}
				}
				else
				{
					return;
				}//获取区域结束

				if (shared_by == 4)
				{
					double d1 = 0, d2 = 0, d3 = 3, d4 = 0;
					d1 = (limit_poly_x[2]-limit_poly_x[1])*(limit_poly_y[0]-limit_poly_y[1])
						-(limit_poly_x[0]-limit_poly_x[1])*(limit_poly_y[2]-limit_poly_y[1]);
					d2 = (limit_poly_x[2]-limit_poly_x[1])*(limit_poly_y[3]-limit_poly_y[1])
						-(limit_poly_x[3]-limit_poly_x[1])*(limit_poly_y[2]-limit_poly_y[1]);
					d3 = (limit_poly_x[3]-limit_poly_x[0])*(limit_poly_y[1]-limit_poly_y[0])
						-(limit_poly_x[1]-limit_poly_x[0])*(limit_poly_y[3]-limit_poly_y[0]);
					d4 = (limit_poly_x[3]-limit_poly_x[0])*(limit_poly_y[2]-limit_poly_y[0])
						-(limit_poly_x[2]-limit_poly_x[0])*(limit_poly_y[3]-limit_poly_y[0]);
					if (d1*d2 < 0 && d3*d4 < 0)
					{
						double temp = limit_poly_x[2];
						limit_poly_x[2] = limit_poly_x[3];
						limit_poly_x[3] = temp;
						temp = limit_poly_y[2];
						limit_poly_y[2] = limit_poly_y[3];
						limit_poly_y[3] = temp;
					}
				}

				Path poly;
				for (int count = 0; count < shared_by; ++count)
				{
					poly<<IntPoint(limit_poly_x[count]*10, limit_poly_y[count]*10);
				}

				clip.Clear();
				clip.AddPath(result[0], ptSubject, true);
				result.clear();
				clip.AddPath(poly, ptClip, true);
				clip.Execute(ctIntersection, result);

				delete []limit_poly_x;
				delete []limit_poly_y;
				limit_poly_x = NULL;
				limit_poly_y = NULL;

				if (result.size() == 1)
				{
					int count = result[0].size();
					double* tempx = new double[count];
					double* tempy = new double[count];
					memset(tempx, 0, sizeof(double)*count);
					memset(tempy, 0, sizeof(double)*count);
					for (int p = 0; p < count; ++p)
					{
						tempx[p] = result[0][p].X/10.0;
						tempy[p] = result[0][p].Y/10.0;
					}

					if (-1 == PtInRegionEx(geox, geoy, tempx, tempy, count, 0.000001))
					{
						int min_index_1 = 0, min_index_2 = 0;
						double min_distance_1 = 0, min_distance_2 = 0;

						for (int m = 0; m < count; ++m)
						{
							double temp_distance = CalDistance(geox, geoy, tempx[m], tempy[m]);
							if (min_distance_1 == 0 || min_distance_1 > temp_distance)
							{
								min_distance_2 = min_distance_1;
								min_index_2 = min_index_1;
								min_distance_1 = temp_distance;
								min_index_1 = m;
							}
							else if (min_distance_2 == 0 || min_distance_2 > temp_distance)
							{
								min_distance_2 = temp_distance;
								min_index_2 = m;
							}
						}
						double new_geox = 0, new_geoy = 0;
						if (abs(min_index_1-min_index_2) == 1)
						{
							new_geox = tempx[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
								tempx[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
							new_geoy = tempy[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
								tempy[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
						}
						else
						{
							int next_index = (min_index_1+1)%count;
							double temp_distance = CalDistance(geox, geoy, tempx[next_index], tempy[next_index]);
							new_geox = tempx[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
								tempx[next_index]*min_distance_1/(min_distance_1+temp_distance);
							new_geoy = new_geoy = tempy[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
								tempy[next_index]*min_distance_1/(min_distance_1+temp_distance);
						}

						auto poly = polygons.begin();
						while (poly != polygons.end())
						{
							poly->ResetPoint("", geox, geoy, new_geox, new_geoy);
							++poly;
						}

						/*int not_in_count = 0;
						for (int s = 0; s < shared_by-1; ++s)
						{
							auto tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
								PolygonExt2(0, NULL, NULL, polygon_ite->np_[i].index_name_n_[s]));
							if (tempoly == EffPolygons.end())
							{
								return;
							}
							if (-1 != PtInRegionEx(geox, geoy, tempoly->px_, tempoly->py_, tempoly->point_count_, 0.000001))
							{
								++not_in_count;
							}
						}
						auto tempoly = std::find(EffPolygons.begin(), EffPolygons.end(),
							PolygonExt2(0, NULL, NULL, polygon_ite->index_name_));
						if (tempoly == EffPolygons.end())
						{
							return;
						}
						if (-1 != PtInRegionEx(geox, geoy, tempoly->px_, tempoly->py_, tempoly->point_count_, 0.000001))
						{
							++not_in_count;
						}
						if (not_in_count > 0 && not_in_count <= shared_by)
						{
							int min_index_1 = 0, min_index_2 = 0;
							double min_distance_1 = 0, min_distance_2 = 0;

							for (int m = 0; m < count; ++m)
							{
								double temp_distance = CalDistance(geox, geoy, tempx[m], tempy[m]);
								if (min_distance_1 == 0 || min_distance_1 > temp_distance)
								{
									min_distance_2 = min_distance_1;
									min_index_2 = min_index_1;
									min_distance_1 = temp_distance;
									min_index_1 = m;
								}
								else if (min_distance_2 == 0 || min_distance_2 > temp_distance)
								{
									min_distance_2 = temp_distance;
									min_index_2 = m;
								}
							}
							double new_geox = 0, new_geoy = 0;
							if (abs(min_index_1-min_index_2) == 1)
							{
								//new_geox = (tempx[min_index_1]+tempx[min_index_2])/2;
								//new_geoy = (tempy[min_index_1]+tempy[min_index_2])/2;
								new_geox = tempx[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
									tempx[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
								new_geoy = tempy[min_index_1]*min_distance_2/(min_distance_1+min_distance_2)+
									tempy[min_index_2]*min_distance_1/(min_distance_1+min_distance_2);
							}
							else
							{
								int next_index = (min_index_1+1)%count;
								double temp_distance = CalDistance(geox, geoy, tempx[next_index], tempy[next_index]);
								//new_geox = (tempx[min_index_1]+tempx[next_index])/2;
								//new_geoy = (tempy[min_index_1]+tempy[next_index])/2;
								new_geox = tempx[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
									tempx[next_index]*min_distance_1/(min_distance_1+temp_distance);
								new_geoy = tempy[min_index_1]*temp_distance/(min_distance_1+temp_distance)+
									tempy[next_index]*min_distance_1/(min_distance_1+temp_distance);
							}

							std::for_each(polygons.begin(), polygons.end(),
								[&](PolygonExt2& poly)
							{
								poly.ResetPoint("", geox, geoy, new_geox, new_geoy);
							});
							CString temp;
							temp.Format("%d, %lf, %lf, %lf, %lf", not_in_count, geox, geoy, new_geox, new_geoy);
							//AfxMessageBox(temp);
							//AfxMessageBox("Not in!");
						}*/
					}
					delete []tempx;
					tempx = NULL;
					delete []tempy;
					tempy = NULL;
				}
				else if (result.size() > 0)
				{
					//AfxMessageBox("Some thing went wrong!");
				}
				else
				{
					//AfxMessageBox("No intersection!");
				}
			}
		}
		++polygon_ite;
	}

	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->Output("D:\\output\\");
		//polygon_ite->Free();
		++polygon_ite;
	}

	ParsePolygon();
	UpdateAllViews(NULL);
	//OutputResultImg(polygons);
	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		//polygon_ite->Output("D:\\output\\");
		polygon_ite->Free();
		++polygon_ite;
	}
}


void CRsDoc::OnLoadmosaic()
{
	ParsePolygon();
	UpdateAllViews(NULL);
}

void CRsDoc::OutputResultImg(std::vector<PolygonExt2>& polygons)
{
	IImageX* pImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	int nCols = (m_lfMaxx-m_lfMinx)/m_lfResolution;
	int nRows = (m_lfMaxy-m_lfMiny)/m_lfResolution;
	pImage->CreateImg(_bstr_t("C:\\mosaic.tif"), modeCreate, nCols, nRows, Pixel_Byte, m_nBandNum, BIL, m_lfMinx, m_lfMiny, m_lfResolution);

	IImageX* tempImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&tempImage);


	CString strImagePath = m_vecImagePath.front();

	strImagePath = strImagePath.Left(strImagePath.ReverseFind('\\')+1);
	auto poly_ite = polygons.begin();
	while (poly_ite != polygons.end())
	{
		CString strImgName = poly_ite->index_name_;
		strImgName += _T(".tif");
		strImgName = strImagePath+strImgName;
		double* px = poly_ite->px_;
		double* py = poly_ite->py_;
		int point_count = poly_ite->point_count_;
		tempImage->Open(strImgName.AllocSysString(), modeRead);
		int cols = 0, rows = 0, nBandnum = 0;
		double lfXOrigin = 0, lfYOrigin = 0, lfCellSize = 0;
		tempImage->GetCols(&cols);
		tempImage->GetRows(&rows);
		tempImage->GetBandNum(&nBandnum);
		tempImage->GetGrdInfo(&lfXOrigin, &lfYOrigin, &lfCellSize);

		double minx= 0, maxx = 0, miny = 0, maxy = 0;
		for (int i = 0; i < point_count; ++i)
		{
			if (px[i] < minx || minx == 0)
			{
				minx = px[i];
			}
			if (px[i] > maxx || maxx == 0)
			{
				maxx = px[i];
			}
			if (py[i] < miny || miny == 0)
			{
				miny = py[i];
			}
			if (py[i] > maxy || maxy == 0)
			{
				maxy = py[i];
			}
		}

		int startx = int((minx-lfXOrigin)/lfCellSize-0.5);
		int endx = int((maxx-lfXOrigin)/lfCellSize+0.5);
		if (startx < 0)
		{
			startx = 0;
		}
		if (endx > cols)
		{
			endx = cols;
		}

		int starty = int((miny-lfYOrigin)/lfCellSize-0.5);
		int endy = int((maxy-lfYOrigin)/lfCellSize+0.5);
		if (starty < 0)
		{
			starty = 0;
		}
		if (endy > rows)
		{
			endy = rows;
		}

		int nXSize = endx-startx;
		int nYSize = endy-starty;

		unsigned char* pBuf = new unsigned char[nXSize*nYSize*nBandnum];
		tempImage->ReadImg(startx, starty, endx, endy, pBuf, nXSize, nYSize, nBandnum, 0, 0, nXSize, nYSize, -1, 0);
		for (int y = starty; y < endy; ++y)
		{
			for (int x = startx; x < endx; ++x)
			{
				double geox, geoy = 0;
				tempImage->Image2World(x, y, &geox, &geoy);
				if (-1 != PtInRegionEx(geox, geoy, px, py, point_count, 0.000001))
				{
					int dst_x = 0, dst_y = 0;
					float dst_lfx = 0, dst_lfy = 0;
					pImage->World2Image(geox, geoy, &dst_lfx, &dst_lfy);
					dst_x = int(dst_lfx);
					dst_y = int(dst_lfy);
					pImage->WriteImg(dst_x, dst_y, dst_x+1, dst_y+1, pBuf, nXSize, nYSize, nBandnum, x-startx, y-starty, x-startx+1, y-starty+1, -1, 0);
				}
			}
		}
		delete []pBuf;
		pBuf = NULL;
		tempImage->Close();
		++poly_ite;
	}
	pImage->Close();
	pImage->Release();
	AfxMessageBox("Finished!");
}

bool CRsDoc::LineCrossPolygon(std::vector<double*>& vecx, std::vector<double*>& vecy, std::vector<int>& point_num, double px1, double py1, double px2, double py2)
{
	auto itex = vecx.begin();
	auto itey = vecy.begin();
	auto ite_point_count = point_num.begin();
	while (itex != vecx.end())
	{
// 		if (-1 != PtInRegionEx(px1, py1, *itex, *itey, *ite_point_count, 0.00001))
// 		{
// 			return true;
// 		}
// 		if (-1 != PtInRegionEx(px2, py2, *itex, *itey, *ite_point_count, 0.00001))
// 		{
// 			return true;
// 		}
		for (int count = 0; count <*ite_point_count; ++count)
		{
			if (LineCrossLine(px1, py1, px2, py2, (*itex)[count], (*itey)[count],
				(*itex)[(count+1)%(*ite_point_count)], (*itey)[(count+1)%(*ite_point_count)]))
			{
				return true;
			}
		}

		++itex;
		++itey;
		++ite_point_count;
	}
	return false;
}

bool CRsDoc::LineCrossLine(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
{
	double d1 = 0, d2 = 0, d3 = 0, d4 = 0;

	d1 = (px2-px1)*(py3-py1)-(px3-px1)*(py2-py1);
	d2 = (px2-px1)*(py4-py1)-(px4-px1)*(py2-py1);
	d3 = (px4-px3)*(py1-py3)-(px1-px3)*(py4-py3);
	d4 = (px4-px3)*(py2-py3)-(px2-px3)*(py4-py3);
	if (d1*d2 < 0 && d3*d4 < 0)
	{
		return true;
	}
	return false;
}

bool CRsDoc::GetCrossPoint(double px1, double py1, double px2, double py2, double* px, double* py, int point_num, double& result_x, double& result_y)
{
	for (int count = 0; count < point_num; ++count)
	{
		if (LineCrossLine(px1, py1, px2, py2, px[count], py[count],
			px[(count+1)%point_num], py[(count+1)%point_num]))
		{
			if (fabs(px1-px2) < 1e-5)
			{
				if (fabs(py1-py[count]) > 1e-5 ||fabs(py2-py[count]) > 1e-5)
				{
					result_x = px1;
					result_y = py[count];
					return true;
				}
			}
			else if (fabs(py1-py2) < 1e-5)
			{
				if (fabs(px1-px[count]) > 1e-5 || fabs(px2-px[count]) > 1e-5)
				{
					result_x = px[count];
					result_y = py1;
					return true;
				}
			}
			else
			{
				double k = (py1-py2)/(px1-px2);
				double b = py1-k*px1;
				if (fabs(px[count]-px[(count+1)%point_num]) < 1e-5)
				{
					result_x = px[count];
					result_y = k*result_x+b;
					return true;
				}
				else if (fabs(py[count]-py[(count+1)%point_num]) < 1e-5)
				{
					result_y = py[count];
					result_x = (result_y-b)/k;
					return true;
				}
			}
		}
	}
	return false;
}
