#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

struct PointEx{
	PointEx(){x = 0; y = 0;}
	PointEx(int xpos, int ypos):x(xpos), y(ypos){}
	int x;
	int y;

	bool operator==(const PointEx& pt)const {return (x == pt.x && y == pt.y);}
	void operator=(const PointEx& pt){x = pt.x; y = pt.y;}
};

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
		if (fabs(right-left) < 0.000001 || fabs(top-bottom) < 0.000001)
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

		auto ite = np_.begin();
		while (ite != np_.end())
		{
			if (ite->available_ == false)
			{
				ite = np_.erase(ite);
				continue;
			}
			++ite;
		}
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
	void ResetPoint(CString index, double px, double py, double newx, double newy)
	{
		if (index == index_name_ || index == _T(""))
		{
			for (int i = 0; i < point_count_; ++i)
			{
				if (fabs(px_[i]-px) < 0.000001 && fabs(py_[i]-py) < 0.000001)
				{
					px_[i] = newx;
					py_[i] = newy;
				}
			}
		}
	}
	void InsertPoints(double startx, double starty, double endx, double endy, double* pxin, double* pyin, long point_count, CString strIndex, CString strIndex2)
	{
		long new_point_count_ = point_count+point_count_;
		double* new_px_  = new double[new_point_count_];
		memset(new_px_, 0, sizeof(double)*new_point_count_);
		double* new_py_ = new double[new_point_count_];
		memset(new_py_, 0, sizeof(double)*new_point_count_);
		auto ite = np_.begin();
		for (int i = 0; i < point_count_; ++i)
		{
			if (fabs(startx-px_[i]) < 0.000001 && fabs(starty-py_[i]) < 0.000001)
			{
				if (fabs(endx-px_[(i+1)%point_count_]) < 0.000001 && fabs(endy-py_[(i+1)%point_count_]) < 0.000001)
				{
					memcpy(new_px_, px_, sizeof(double)*(i+1));
					memcpy(new_py_, py_, sizeof(double)*(i+1));
					memcpy(new_px_+i+1, pxin, sizeof(double)*point_count);
					memcpy(new_py_+i+1, pyin, sizeof(double)*point_count);
					std::vector<NodeProperty> np;

					NodeProperty temp_np(2);
					temp_np.available_ = false;
					if (index_name_.CompareNoCase(strIndex) == 0)
					{
						temp_np.index_name_n_[0] = strIndex2;
					}
					else if (index_name_.CompareNoCase(strIndex2) == 0)
					{
						temp_np.index_name_n_[0] = strIndex;
					}

					for (int index = 0; index <point_count; ++index)
					{
						np.push_back(temp_np);
					}

					if (i != point_count_-1)
					{
						memcpy(new_px_+i+1+point_count, px_+i+1, sizeof(double)*(point_count_-i-1));
						memcpy(new_py_+i+1+point_count, py_+i+1, sizeof(double)*(point_count_-i-1));
					}

					np_.insert(ite+1, np.begin(), np.end());
					delete []px_;
					delete []py_;
					px_ = new_px_;
					py_ = new_py_;
					point_count_ = new_point_count_;
					return;
				}
				else if (fabs(endx-px_[(i-1+point_count_)%point_count_]) < 0.000001 && fabs(endy-py_[(i-1+point_count_)%point_count_]) < 0.000001)
				{
					double* temp_px = new double[point_count];
					double* temp_py = new double[point_count];
					memset(temp_px, 0, sizeof(double)*point_count);
					memset(temp_py, 0, sizeof(double)*point_count);
					for (int n = 0; n < point_count; ++n)
					{
						temp_px[n] = pxin[point_count-n-1];
						temp_py[n] = pyin[point_count-n-1];
					}
					InsertPoints(endx, endy, startx, starty, temp_px, temp_py, point_count, strIndex, strIndex2);

					delete []temp_px;
					delete []temp_py;
					temp_px = NULL;
					temp_py = NULL;
					return;
				}
			}
			++ite;
		}
		delete []new_px_;
		new_px_ = NULL;
		delete []new_py_;
		new_py_ = NULL;
	}
	void Free()
	{
		delete []px_;
		delete []py_;
		px_ = NULL;
		py_ = NULL;
	}

	bool operator==(const PolygonExt2& poly)const
	{
		return index_name_ == poly.index_name_;
	}

	int point_count_;
	double* px_;
	double* py_;
	CString index_name_;
	std::vector<NodeProperty> np_;
};




static bool NotImportant(NodeProperty& center, NodeProperty& left, NodeProperty& right)
{
	CString leftstr = center.index_name_n_[0];

	if (leftstr != left.index_name_n_[0] && leftstr != left.index_name_n_[1]
	&& leftstr != left.index_name_n_[2])
	{
		return false;
	}

	if (leftstr != right.index_name_n_[0] && leftstr != right.index_name_n_[1]
	&& leftstr != right.index_name_n_[2])
	{
		return false;
	}

	return true;
}

bool Optimize(CString strAllDomPath, CString strDxfPath, CString strRrlxPath);
bool EffectPoly(std::vector<CString>& vecImagePath);
bool Dxf2Dsm(CString strDxf, double cellsize);
bool Dsm2Tif(CString strDsmPath);
bool LineCrossPolygon(std::vector<double*>& vecx, std::vector<double*>& vecy, std::vector<int>& point_num, double px1, double py1, double px2, double py2);
bool LineCrossLine(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4);
bool GetCrossPoint(double px1, double py1, double px2, double py2, double* px, double* py, int point_num, double& result_x, double& result_y);
int  PtInRegionZXEx(double x, double y, double *pX, double *pY, int nSum, double lfSnap);
double CalDistance(double x1, double y1, double x2, double y2);
double GetDistance(double pointx, double pointy, double linestartx, double linestarty, double lineendx, double lineendy);
