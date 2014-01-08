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

// RsView.cpp : CRsView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Rs.h"
#endif

#include "RsDoc.h"
#include "RsView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRsView

IMPLEMENT_DYNCREATE(CRsView, CView)

BEGIN_MESSAGE_MAP(CRsView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_SHOW_EDGE, &CRsView::OnShowEdge)
	ON_UPDATE_COMMAND_UI(ID_SHOW_EDGE, &CRsView::OnUpdateShowEdge)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CRsView 构造/析构

CRsView::CRsView():m_hRC(NULL),
	m_pDC(NULL),
	m_pData(NULL),
	m_lfScale(0),
	m_bShowEdge(FALSE),
	m_bIsPress(FALSE),
	m_nRealOrix(0),
	m_nRealOriy(0),
	m_nxoff(0),
	m_nyoff(0)
{
	// TODO: 在此处添加构造代码

}

CRsView::~CRsView()
{
}

BOOL CRsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	
	if (!CView::PreCreateWindow(cs))
	{
		return 0;
	}

	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	#define CUSTOM_CLASSNAME _T("GL_WINDOW_CLASS")

	if (!(::GetClassInfo(hInst, CUSTOM_CLASSNAME, &wndcls)))
	{
		if (::GetClassInfo(hInst, cs.lpszClass, &wndcls))
		{
			wndcls.lpszClassName = CUSTOM_CLASSNAME;
			wndcls.style |= (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);
			wndcls.hbrBackground = NULL;

			if (!AfxRegisterClass(&wndcls))
			{
				AfxThrowResourceException();
			}
		}
		else
		{
			AfxThrowResourceException();
		}
	}

	cs.lpszClass = CUSTOM_CLASSNAME;


	return 1;
}

// CRsView 绘制

void CRsView::OnDraw(CDC* /*pDC*/)
{
	CRsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderScene();

	SwapBuffers(m_pDC->GetSafeHdc());

	wglMakeCurrent(NULL, NULL);
}

void CRsView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void CRsView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CRsView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CRsView 诊断

#ifdef _DEBUG
void CRsView::AssertValid() const
{
	CView::AssertValid();
}

void CRsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRsDoc* CRsView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRsDoc)));
	return (CRsDoc*)m_pDocument;
}
#endif //_DEBUG


BOOL CRsView::SetupPixelformat()
{
	static PIXELFORMATDESCRIPTOR pfd =   
	{  
		sizeof(PIXELFORMATDESCRIPTOR),  // pfd结构的大小   
		1,                              // 版本号   
		PFD_DRAW_TO_WINDOW |            // 支持在窗口中绘图   
		PFD_SUPPORT_OPENGL |            // 支持 OpenGL   
		PFD_DOUBLEBUFFER,               // 双缓存模式   
		PFD_TYPE_RGBA,                  // RGBA 颜色模式   
		24,                             // 24 位颜色深度   
		0, 0, 0, 0, 0, 0,               // 忽略颜色位   
		0,                              // 没有非透明度缓存   
		0,                              // 忽略移位位   
		0,                              // 无累计缓存   
		0, 0, 0, 0,                     // 忽略累计位   
		32,                             // 32 位深度缓存       
		0,                              // 无模板缓存   
		0,                              // 无辅助缓存   
		PFD_MAIN_PLANE,                 // 主层   
		0,                              // 保留   
		0, 0, 0                         // 忽略层,可见性和损毁掩模
	};  
	int pixelFormat;  
	// 为设备描述表得到最匹配的像素格式   
	if((pixelFormat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0)  
	{  
		MessageBox(_T("ChoosePixelFormat failed"));  
		return FALSE;  
	}  
	// 设置最匹配的像素格式为当前的像素格式   
	if(SetPixelFormat(m_pDC->GetSafeHdc(), pixelFormat, &pfd) == FALSE)  
	{  
		MessageBox( _T("SetPixelFormat failed") );  
		return FALSE;  
	}  
	return TRUE;  
}

BOOL CRsView::InitializeOpenGL()
{
	PIXELFORMATDESCRIPTOR pfd;

	int n;
	m_pDC = new CClientDC(this);
	ASSERT(m_pDC != NULL);

	if (!SetupPixelformat())
	{
		return FALSE;
	}

	n = GetPixelFormat(m_pDC->GetSafeHdc());
	DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

	m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
	if (m_hRC == NULL)
	{
		return FALSE;
	}

	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	glClearColor(0.f, 0.f, 0.f, 1.0f);

	wglMakeCurrent(NULL, NULL);

	return TRUE;
}

void CRsView::RenderScene()
{
	CRsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	int nBufWidth = 0, nBufHeight = 0;
	pDoc->GetBufSize(nBufWidth, nBufHeight);
	pDoc->GetViewScale(m_lfScale);

	CRect rect;
	GetClientRect(&rect);
	int nTexWidth = 0, nTexHeight = 0;
	nTexHeight = nBufHeight;
	nTexWidth = nBufWidth;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	if (pDoc->IsReady())
	{
		if (g_tex != 0)
		{
			glDeleteTextures(1, &g_tex);
		}
		unsigned char* pData = pDoc->GetData();

		pDoc->GetRealOrigin(m_nRealOrix, m_nRealOriy);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &g_tex);
		glBindTexture(GL_TEXTURE_2D, g_tex);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		
		if (pDoc->IsGrey())
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, nTexWidth, nTexHeight, 0, 
				GL_LUMINANCE, GL_UNSIGNED_BYTE, pData);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nTexWidth, nTexHeight, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pData);
		}
		pDoc->SetReady(FALSE);
	}

	glLoadIdentity();
	long nRealOriginx = 0, nRealOriginy = 0;
	pDoc->GetRealOrigin(nRealOriginx, nRealOriginy);

	long left = 0, right = 0, top = 0, bottom = 0;

	if (nRealOriginx != m_nRealOrix)
	{
		if (m_nRealOrix>=0 && nRealOriginx>=0)
		{
			left = nRealOriginx;
			if (m_lfScale -1.0 > 0.0000001)
			{
				right = left+nBufWidth;
			}
			else
			{
				right = left+nBufWidth;
			}
		}
		else if (m_nRealOrix<=0 && nRealOriginx<=0)
		{
			left = nRealOriginx-m_nRealOrix;
			if (m_lfScale-1.0 > 0.0000001)
			{
				right = left+nBufWidth;
			}
			else
			{
				right = left+nBufWidth;
			}
		}
		else if (m_nRealOrix<=0 && nRealOriginx>=0)
		{
			left = nRealOriginx-m_nRealOrix;
			if (m_lfScale -1.0 > 0.0000001)
			{
				right = left+nBufWidth;
			}
			else
			{
				right = left+nBufWidth;
			}
		}
		else if (m_nRealOrix >= 0 && nRealOriginx <= 0)
		{
			left = nRealOriginx;
			if (m_lfScale -1.0 > 0.0000001)
			{
				right = left+nBufWidth;
			}
			else
			{
				right = left+nBufWidth;
			}
		}
	}
	else
	{
		left = nRealOriginx>0 ? nRealOriginx : (m_lfScale-1.0 > 0.0000001 ? -(abs(nRealOriginx)%int(m_lfScale)) : 0);
		if (m_lfScale -1.0 > 0.0000001)
		{
			right = left+nBufWidth;
		}
		else
		{
			right = left+nBufWidth;
		}
	}

	if (nRealOriginy != m_nRealOriy)
	{
		if (m_nRealOriy>=0 && nRealOriginy>=0)
		{
			bottom = nRealOriginy;
			if (m_lfScale -1.0 > 0.0000001)
			{
				top = bottom+nBufHeight;
			}
			else
			{
				top = bottom+nBufHeight;
			}
		}
		else if (m_nRealOriy<=0 && nRealOriginy<=0)
		{
			bottom = nRealOriginy-m_nRealOriy;
			if (m_lfScale -1.0 > 0.0000001)
			{
				top = bottom+nBufHeight;
			}
			else
			{
				top = bottom+nBufHeight;
			}
		}
		else if (m_nRealOriy<=0 && nRealOriginy>=0)
		{
			bottom = nRealOriginy-m_nRealOriy;
			if (m_lfScale -1.0 > 0.0000001)
			{
				top = bottom+nBufHeight;
			}
			else
			{
				top = bottom+nBufHeight;
			}
		}
		else if (m_nRealOriy >= 0 && nRealOriginy <= 0)
		{
			bottom = nRealOriginy;
			if (m_lfScale -1.0 > 0.0000001)
			{
				top = bottom+nBufHeight;
			}
			else
			{
				top = bottom+nBufHeight;
			}
		}
	}
	else
	{
		bottom = nRealOriginy>0 ? nRealOriginy : (m_lfScale-1.0 > 0.0000001 ? -(abs(nRealOriginy)%int(m_lfScale)) : 0);
		if (m_lfScale -1.0 > 0.0000001)
		{
			top = bottom+nBufHeight;
		}
		else
		{
			top = bottom+nBufHeight;
		}
	}
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_tex);
	glLoadIdentity();
	glColor3f(1.0, 1.0, 1.0);
	if (m_lfScale-1 > 0.0000001)
	{
		if (nBufWidth*m_lfScale < rect.Width())
		{
			right = left+nBufWidth*m_lfScale;
		}
		else
		{
			right = left+rect.Width()+m_lfScale;
		}
		if (nBufHeight*m_lfScale < rect.Height())
		{
			top = bottom+nBufHeight*m_lfScale;
		}
		else
		{
			top = bottom+rect.Height()+m_lfScale;
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);glVertex2f(left, bottom);
		glTexCoord2f(0.0, 1.0);glVertex2f(left, top);
		glTexCoord2f(1.0, 1.0);glVertex2f(right, top);
		glTexCoord2f(1.0, 0.0);glVertex2f(right, bottom);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);glVertex2f(left, bottom);
		glTexCoord2f(0.0, 1.0);glVertex2f(left, top);
		glTexCoord2f(1.0, 1.0);glVertex2f(right, top);
		glTexCoord2f(1.0, 0.0);glVertex2f(right, bottom);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);

	if (m_bShowEdge)
	{
		std::vector<RectFExt>::iterator temIte = pDoc->m_vecImageRect.begin();
		std::vector<RectFExt>::iterator IteEnd = pDoc->m_vecImageRect.end();
		long edgeleft = 0, edgeright = 0, edgetop = 0, edgebottom = 0;
		long nRealOriginx = 0, nRealOriginy = 0;
		pDoc->GetRealOrigin(nRealOriginx, nRealOriginy);
		CRect rect;
		GetClientRect(&rect);
		while (temIte != IteEnd)
		{
			pDoc->Geo2Screen(temIte->left, temIte->bottom, edgeleft, edgebottom);
			pDoc->Geo2Screen(temIte->right, temIte->top, edgeright, edgetop);
			
			edgeright = int((edgeright-edgeleft)/m_lfScale+0.99999)*m_lfScale+edgeleft;
			edgetop = int((edgetop-edgebottom)/m_lfScale+0.99999)*m_lfScale+edgebottom;

			edgeleft += nRealOriginx;
			edgebottom += nRealOriginy;
			edgeright += nRealOriginx;
			edgetop += nRealOriginy;
			
			glColor3f(1.0, 0, 0);
			glPushMatrix();
			glTranslatef(edgeleft, edgebottom, 0);
			glBegin(GL_LINE_STRIP);
			glVertex2i(0, edgetop-edgebottom);
			glVertex2i(0, 0);
			glVertex2i(edgeright-edgeleft, 0);
			glVertex2i(edgeright-edgeleft, edgetop-edgebottom);
			glVertex2i(0, edgetop-edgebottom);
			glEnd();
			glPopMatrix();
			++temIte;
		}
	}
}


// CRsView 消息处理程序


int CRsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	InitializeOpenGL();
	return 0;
}


void CRsView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (cx > 0 && cy > 0)
	{
		m_wide = cx;
		m_height = cy;


		wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

		glViewport(0, 0, m_wide, m_height);

		glPushMatrix();

		glMatrixMode(GL_PROJECTION);

		glLoadIdentity();

		gluOrtho2D(0.0, (GLdouble)m_wide, 0.0, (GLdouble)m_height);

		glMatrixMode(GL_MODELVIEW);

		glPopMatrix();

		wglMakeCurrent(NULL,NULL);
		Invalidate(TRUE);
	}
	
}


BOOL CRsView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CScrollView::OnEraseBkgnd(pDC);
}


void CRsView::OnDestroy()
{
	if(wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC) == FALSE)
	{
		MessageBox(_T("error"));
	}

	if (m_hRC!=NULL)
	{
		wglDeleteContext(m_hRC);
	}

	if(m_pDC)
	{
		delete m_pDC;
	}

	m_pDC = NULL;

	CView::OnDestroy();
}



BOOL CRsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	CRsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return FALSE;

	CRect rect;
	GetClientRect(&rect);
	ScreenToClient(&pt);

	long nRealOriginx = 0, nRealOriginy = 0;
	pDoc->GetRealOrigin(nRealOriginx, nRealOriginy);
	pt.x -= nRealOriginx;
	pt.y = rect.Height()-pt.y;
	pt.y -= nRealOriginy;
	if (zDelta > 0)
	{
		pDoc->ZoomOut(pt);
	}
	else
	{
		pDoc->ZoomIn(pt);
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CRsView::OnShowEdge()
{
	// TODO: Add your command handler code here
	m_bShowEdge = !m_bShowEdge;
	Invalidate(TRUE);
}


void CRsView::OnUpdateShowEdge(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowEdge);
}


void CRsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;


	m_bIsPress = TRUE;
	ScreenToClient(&point);
	m_ptStart = point;
	pDoc->GetRealOrigin(m_nRealOrix, m_nRealOriy);
	CView::OnLButtonDown(nFlags, point);
}


void CRsView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_nxoff = 0;
	m_nyoff = 0;
	m_bIsPress = FALSE;
	CRsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	pDoc->GetRealOrigin(m_nRealOrix, m_nRealOriy);
	pDoc->UpdateData();
	CView::OnLButtonUp(nFlags, point);
}


void CRsView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsPress)
	{
		CRsDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		ScreenToClient(&point);
		if (m_lfScale-1.0 > 0.0000001)
		{
			m_nxoff = (point.x-m_ptStart.x)/*/int(m_lfScale)*/;
			m_nyoff = (m_ptStart.y-point.y)/*/int(m_lfScale)*/;
		}
		else
		{
			m_nxoff = point.x-m_ptStart.x;
			m_nyoff = m_ptStart.y-point.y;
		}
		pDoc->SetRealOrigin(m_nRealOrix+m_nxoff, m_nRealOriy+m_nyoff);
		Invalidate(TRUE);
	}
	
	CView::OnMouseMove(nFlags, point);
}
