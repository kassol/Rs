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
#include <vld.h>
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
		//m_pImage->Close();
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

	const int MIN_FILE_NUMBER = 10;
	fdlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH*MIN_FILE_NUMBER];
	memset(fdlg.m_ofn.lpstrFile, 0, _MAX_PATH*MIN_FILE_NUMBER);
	fdlg.m_ofn.nMaxFile = _MAX_PATH*MIN_FILE_NUMBER;

	if (IDOK == fdlg.DoModal())
	{
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
	//AfxMessageBox(strFileFilter);
	delete []szFileFilter;
	szFileFilter = NULL;


	CFileDialog fdlg(TRUE, NULL, NULL, 
		OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		strFileFilter, NULL, 0, TRUE);

	const int MIN_FILE_NUMBER = 10;
	fdlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH*MIN_FILE_NUMBER];
	memset(fdlg.m_ofn.lpstrFile, 0, _MAX_PATH*MIN_FILE_NUMBER);
	fdlg.m_ofn.nMaxFile = _MAX_PATH*MIN_FILE_NUMBER;

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
		/*auto temIte = m_vecShapePath.begin();
		while(temIte != m_vecShapePath.end())
		{

		++temIte;
		}*/



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
