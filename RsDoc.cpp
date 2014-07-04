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
// 					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
// 						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nRed, 0);
// 					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
// 						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nGreen, 1);
// 					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
// 						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, m_nBlue, 2);
					m_pImage->ReadImg((int)nSrcLeft, (int)nSrcTop, (int)nSrcRight, (int)nSrcBottom, m_pBacBuf, 
						m_nBufWidth, m_nBufHeight, m_nBandNum, left, top, right, bottom, -1, 0);
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
		m_vecEffectivePoly.push_back(PolygonExt(point_count, px, py));
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
		//polygon_ite->Free();
		++polygon_ite;
	}

	IImageX* pImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	pImage->Open(_bstr_t("D:\\out.tif"), modeRead);
	double resolution = 0;
	double temp;
	pImage->GetGrdInfo(&temp, &temp, &resolution);

	IImageX* tempImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&tempImage);

	const int blockArea = 50;
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
			pImage->GetPixel((int)fy, (int)fx, &height);
			if (height == 0)
			{
				continue;
			}
			RectFExt result_result = the_rect;
			if (polygon_ite->np_[i].shared_by_ != 1)
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

				int buffer_height = int(buffer_bottom-buffer_top+0.99999);
				int buffer_width = int(buffer_right-buffer_left+0.99999);

				unsigned short* buf = new unsigned short[buffer_height*buffer_width];
				memset(buf, 0, buffer_height*buffer_width*sizeof(unsigned short));
				pImage->ReadImg(buffer_left, buffer_top, buffer_right, buffer_bottom,
					(unsigned char*)buf, buffer_width, buffer_height, 1, 0, 0,
					buffer_width, buffer_height, -1, 0);
				int start_col = int(fx-buffer_left+0.99999);
				int start_row = int(fy-buffer_top+0.99999);
				if (start_col >= buffer_width || start_row >= buffer_height)
				{
					continue;
				}
				bool isFind = false;
				int ncount = 0;
				const int count_limit = 7;

				for (int f = start_col-1; f >= 0; --f)
				{
					if (buf[start_row*buffer_width+f] == 0)
					{
						++ncount;
						if (ncount >= count_limit)
						{
							isFind = true;
							std::for_each(polygons.begin(), polygons.end(),
								[&](PolygonExt2 & poly)
							{
								poly.ResetPoint("", geox, geoy,
									geox-(start_col-f)*resolution, geoy);
							});
							break;
						}
					}
				}
				if (!isFind)
				{
					ncount = 0;
					for (int f = start_col+1; start_col < buffer_width; ++f)
					{
						if (buf[start_row*buffer_width+f] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								std::for_each(polygons.begin(), polygons.end(),
									[&](PolygonExt2 & poly)
								{
									poly.ResetPoint("", geox, geoy,
										geox+(f-start_col)*resolution, geoy);
								});
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
						if (buf[f*buffer_width+start_row] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								std::for_each(polygons.begin(), polygons.end(),
									[&](PolygonExt2 & poly)
								{
									poly.ResetPoint("", geox, geoy,
										geox, geoy-(start_row-f)*resolution);
								});
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
						if (buf[f*buffer_width+start_row] == 0)
						{
							++ncount;
							if (ncount >= count_limit)
							{
								isFind = true;
								std::for_each(polygons.begin(), polygons.end(),
									[&](PolygonExt2 & poly)
								{
									poly.ResetPoint("", geox, geoy,
										geox, geoy+(f-start_row)*resolution);
								});
								break;
							}
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
		//polygon_ite->DeletePoint();
		polygon_ite->Output("D:\\output\\");
		polygon_ite->Free();
		++polygon_ite;
	}

	ParsePolygon();
	UpdateAllViews(NULL);
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
	OutputEffectivePoly(strAllDomPath, 0);

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
}
