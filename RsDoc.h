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

// RsDoc.h : CRsDoc 类的接口
//


#pragma once
#include <algorithm>
#include <fstream>


struct RectFExt
{
	RectFExt(){left = 0; right = 0; top = 0; bottom = 0;}
	RectFExt(double l, double r, double t, double b)
	{
		left = l;
		right = r;
		top = t;
		bottom = b;
	}
	double Width(){return (right-left);}
	double Height(){return (top-bottom);}
	bool IsEmpty()
	{
		if (right-left == 0 || top-bottom == 0)
		{
			return true;
		}
		return false;
	}
	bool Intersects(RectFExt& rect)
	{
		if ((left-rect.right)*(right-rect.left) < 0 && (top-rect.bottom)*(bottom-rect.top) < 0)
		{
			return true;
		}
		return false;
	}
	RectFExt Intersected(RectFExt& rect)
	{
		if (this->Intersects(rect))
		{
			return RectFExt(left > rect.left ? left : rect.left,
				right < rect.right ? right : rect.right,
				top < rect.top ? top : rect.top,
				bottom > rect.bottom ? bottom : rect.bottom);
		}
		return RectFExt();
	}
	void Free(){left = 0; right = 0; top = 0; bottom = 0;}

	double left;
	double right;
	double top;			//the big one
	double bottom;
};

struct PolygonExt
{
	PolygonExt(int point_count, double* px, double* py)
		: point_count_(point_count)
		, px_(px)
		, py_(py)
		, index_name_("")
	{

	}

	PolygonExt(int point_count, double* px, double* py, CString index_name)
		: point_count_(point_count)
		, px_(px)
		, py_(py)
		, index_name_(index_name)
	{

	}

	void Free()
	{
		point_count_ = 0;
		delete []px_;
		px_ = NULL;
		py_ = NULL;
	}

	int point_count_;
	double* px_;
	double* py_;
	CString index_name_;
};

struct NodeProperty
{
	NodeProperty(int shared_by)
		: shared_by_(shared_by)
		, available_(true)
	{

	}
	int shared_by_;
	CString index_name_n_[3];
	bool available_;
};

static bool NotImportant(NodeProperty& center, NodeProperty& left, NodeProperty& right)
{
	CString leftstr = center.index_name_n_[0];
	CString rightstr = center.index_name_n_[1];
	if (leftstr != left.index_name_n_[0] && leftstr != left.index_name_n_[1]
	&& leftstr != left.index_name_n_[2])
	{
		return false;
	}

	if (rightstr != left.index_name_n_[0] && rightstr != left.index_name_n_[1]
	&& rightstr != left.index_name_n_[2])
	{
		return false;
	}

	if (leftstr != right.index_name_n_[0] && leftstr != right.index_name_n_[1]
	&& leftstr != right.index_name_n_[2])
	{
		return false;
	}

	if (rightstr != right.index_name_n_[0] && rightstr != right.index_name_n_[1]
	&& rightstr != right.index_name_n_[2])
	{
		return false;
	}

	return true;
}

struct PolygonExt2
{
	PolygonExt2(int point_count, double* px, double* py, CString index_name)
		: point_count_(point_count)
		, px_(px)
		, py_(py)
		, index_name_(index_name)
	{
		np_.resize(point_count_, 1);
	}

	void DeletePoint()
	{
		int temp_point_count = 0;
		std::for_each(np_.begin(), np_.end(),
			[&temp_point_count](NodeProperty np)
		{
			if (np.available_)
			{
				++temp_point_count;
			}
		});

		double* temp_px = new double[temp_point_count];
		double* temp_py = new double[temp_point_count];
		memset(temp_px, 0, sizeof(double)*temp_point_count);
		memset(temp_py, 0, sizeof(double)*temp_point_count);

		for (int i = 0, offset = 0; i < temp_point_count; ++i)
		{
			while(!np_[i+offset].available_)
			{
				++offset;
			}
			temp_px[i] = px_[i+offset];
			temp_py[i] = py_[i+offset];
		}

		point_count_ = temp_point_count;
		delete []px_;
		delete []py_;
		px_ = temp_px;
		py_ = temp_py;
	}
	void Output(CString path)
	{
		path += index_name_;
		path += _T(".rrlx");
		std::fstream outfile;
		outfile.open(path.LockBuffer(), std::ios::out);
		if (outfile != NULL)
		{
			outfile<<std::fixed;
			outfile<<point_count_<<std::endl;
			for (int i = 0; i < point_count_; ++i)
			{
				outfile<<px_[i]<<"   "<<py_[i]<<"   "<<0<<std::endl;
			}
		}
		outfile.close();
	}
	void Free()
	{
		delete []px_;
		delete []py_;
		px_ = NULL;
		py_ = NULL;
	}

	int point_count_;
	double* px_;
	double* py_;
	CString index_name_;
	std::vector<NodeProperty> np_;
};


class CRsDoc : public CDocument
{
protected: // 仅从序列化创建
	CRsDoc();
	DECLARE_DYNCREATE(CRsDoc)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CRsDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


private:
	
	IImageX* m_pImage;
	CDrawing m_dxffile;
	std::vector<CString> m_vecImagePath;
	std::vector<CString> m_vecShapePath;
	double m_lfMinx;
	double m_lfMaxx;
	double m_lfMiny;
	double m_lfMaxy;
	double m_lfResolution;
	double m_lfScale;				//显示分辨率
	int m_nBandNum;
	BOOL m_bIsGrey;

	int m_nBufWidth;
	int m_nBufHeight;
	int m_nCols;
	int m_nRows;
	int m_nWndWidth;
	int m_nWndHeight;
	int m_nScrollSizex;
	int m_nScrollSizey;

	long m_nRealOriginx;
	long m_nRealOriginy;

	unsigned char* m_pCurBuf;
	unsigned char* m_pBacBuf;
	RectFExt m_recBac;
	RectFExt m_recCur;
	int m_nRed;
	int m_nGreen;
	int m_nBlue;

	int m_nRealBandNum;

	BOOL m_bIsReady;

	int* m_pRasterState;
	int* m_pVectorState;

	std::vector<double*> m_vecX;
	std::vector<double*> m_vecY;
	std::vector<int> m_vecPointNum;
	std::vector<int> m_vecPolygonNum;

	void  FillData(RectFExt rect);
	void Geo2Buf(RectFExt rect, double lfxpos, double lfypos, long &nxpos, long &nypos);

public:
	std::vector<RectFExt> m_vecImageRect;
	std::vector<PolygonExt> m_vecMosaicLine;

public:
	void GetBufSize(int &nBufWidth, int &nBufHeight);
	unsigned char* GetData();
	void Screen2Geo(long nxpos, long nypos, double &lfxpos, double &lfypos);
	void Geo2Screen(double lfxpos, double lfypos, long &nxpos, long &nypos);
	void ZoomOut(CPoint &pt);
	void ZoomIn(CPoint &pt);
	BOOL IsGrey();
	BOOL IsReady(); 
	void SetReady(BOOL bIsReady);
	void GetRealOrigin(long& nRealOriginx, long& nRealOriginy);
	void SetRealOrigin(long nRealOriginx, long nRealOriginy);
	void GetViewScale(double& lfScale);
	void UpdateData();
	void UpdateRasterList();
	void UpdateVectorList();
	void SetRasterState(int nIndex, BOOL bState);
	void SetVectorState(int nIndex, BOOL bState);
	void UpdateRasterState();
	void UpdateVectorState();
	int* GetRasterState();
	int* GetVectorState();
	void GetShapeIterator(std::vector<double*>::iterator& iteX, std::vector<double*>::iterator& iteY, std::vector<int>::iterator& iteNum, std::vector<int>::iterator& itePolyNum);
	void GetShapeIterEnd(std::vector<double*>::iterator& iteX, std::vector<double*>::iterator& iteY, std::vector<int>::iterator& iteNum, std::vector<int>::iterator& itePolyNum);
	std::vector<PolygonExt>& GetPolygonVec();
	void ParsePolygon();


// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnFileOpen();
	afx_msg void OnBandcomb();
	afx_msg void OnAddraster();
	afx_msg void OnAddvector();
	afx_msg void OnGenerateline();
	afx_msg void OnDxf2dsm();
	afx_msg void OnOptimize();
};
