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
END_MESSAGE_MAP()

// CRsView 构造/析构

CRsView::CRsView():m_hRC(NULL),
	m_pDC(NULL),
	m_pData(NULL),
	m_lfScale(0),
	m_bShowEdge(FALSE)
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

	if (pDoc->IsReady())
	{
		unsigned char* pData = pDoc->GetData();

		glLoadIdentity();

		double lfRealOriginx = 0, lfRealOriginy = 0;
		pDoc->GetRealOrigin(lfRealOriginx, lfRealOriginy);

		double xoff = 0, yoff = 0;

		if (lfRealOriginx < 0)
		{
			if (m_lfScale-1.0 > 0.0000001)
			{
				xoff = int(fabs(lfRealOriginx))%int(m_lfScale);
			}
			lfRealOriginx = 0;
			//xoff = (lfRealOriginx-int(lfRealOriginx))*m_lfScale;
		}
		if (lfRealOriginy < 0)
		{
			if (m_lfScale-1.0 > 0.0000001)
			{
				yoff = int(fabs(lfRealOriginy))%int(m_lfScale);
			}
			lfRealOriginy = 0;
			//yoff = (lfRealOriginy-int(lfRealOriginy))*m_lfScale;
		}

		glRasterPos2i(lfRealOriginx, lfRealOriginy);
		glBitmap(0, 0, 0, 0, -xoff, -yoff, NULL);

		if (m_lfScale-1 > 0.0000001)
		{
			glPixelZoom(m_lfScale, m_lfScale);
		}
		else
		{
			glPixelZoom(1.0f, 1.0f);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		if (!pDoc->IsGrey())
		{
			glDrawPixels(nBufWidth, nBufHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);
			if (m_pData != NULL)
			{
				delete []m_pData;
			}
			m_pData = new unsigned char[nBufWidth*nBufHeight*3];
			memcpy(m_pData, pData, nBufWidth*nBufHeight*3);
			/*memset(m_pData, 0, nBufWidth*nBufHeight*3);
			glReadPixels(0, 0, nBufWidth, nBufHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pData);*/
		}
		else
		{
			glDrawPixels(nBufWidth, nBufHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, pData);
			if (m_pData != NULL)
			{
				delete []m_pData;
			}
			m_pData = new unsigned char[nBufWidth*nBufHeight];
			memcpy(m_pData, pData, nBufWidth*nBufHeight);
			/*memset(m_pData, 0, nBufWidth*nBufHeight);
			glReadPixels(0, 0, nBufWidth, nBufHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pData);*/
		}
		pDoc->SetReady(FALSE);
	}
	else
	{
		glLoadIdentity();

		double lfRealOriginx = 0, lfRealOriginy = 0;
		pDoc->GetRealOrigin(lfRealOriginx, lfRealOriginy);

		double xoff = 0, yoff = 0;


		if (lfRealOriginx < 0)
		{
			if (m_lfScale-1.0 > 0.0000001)
			{
				xoff = int(fabs(lfRealOriginx))%int(m_lfScale);
			}
			lfRealOriginx = 0;
		}
		if (lfRealOriginy < 0)
		{
			if (m_lfScale-1.0 > 0.0000001)
			{
				yoff = int(fabs(lfRealOriginy))%int(m_lfScale);
			}
			lfRealOriginy = 0;
		}

		glRasterPos2i(lfRealOriginx, lfRealOriginy);
		glBitmap(0, 0, 0, 0, -xoff, -yoff, NULL);

		if (m_lfScale-1 > 0.0000001)
		{
			glPixelZoom(m_lfScale, m_lfScale);
		}
		else
		{
			glPixelZoom(1.0f, 1.0f);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		if (!pDoc->IsGrey())
		{
			glDrawPixels(nBufWidth, nBufHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pData);
		}
		else
		{
			glDrawPixels(nBufWidth, nBufHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pData);
		}
	}
	if (m_bShowEdge)
	{
		std::vector<RectFExt>::iterator temIte = pDoc->m_vecImageRect.begin();
		std::vector<RectFExt>::iterator IteEnd = pDoc->m_vecImageRect.end();
		double left = 0, right = 0, top = 0, bottom = 0;
		double lfRealOriginx = 0, lfRealOriginy = 0;
		pDoc->GetRealOrigin(lfRealOriginx, lfRealOriginy);
		CRect rect;
		GetClientRect(&rect);
		while (temIte != IteEnd)
		{
			pDoc->Geo2Screen(temIte->left, temIte->bottom, left, bottom);
			pDoc->Geo2Screen(temIte->right, temIte->top, right, top);
			right = int((right-left)/m_lfScale+0.99999)*m_lfScale+left;
			top = int((top-bottom)/m_lfScale+0.99999)*m_lfScale+bottom;

			left += lfRealOriginx;
			bottom += lfRealOriginy;
			right += lfRealOriginx;
			top += lfRealOriginy;
			
			
			glColor3f(1.0, 0, 0);
			glBegin(GL_LINE_STRIP);
			glVertex2i(left, top);
			glVertex2i(left, bottom);
			glVertex2i(right, bottom);
			glVertex2i(right, top);
			glVertex2i(left, top);
			glEnd();
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

	double lfRealOriginx = 0, lfRealOriginy = 0;
	pDoc->GetRealOrigin(lfRealOriginx, lfRealOriginy);
	pt.x -= lfRealOriginx;
	pt.y = rect.Height()-pt.y;
	pt.y -= lfRealOriginy;
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
