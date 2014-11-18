#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

bool Optimize(CString strAllDomPath, CString strDxfPath, CString strRrlxPath);
bool EffectPoly(std::vector<CString>& vecImagePath);
bool Dxf2Dsm(CString strDxf, double cellsize);
bool Dsm2Tif(CString strDsmPath);
bool LineCrossPolygon(std::vector<double*>& vecx, std::vector<double*>& vecy, std::vector<int>& point_num, double px1, double py1, double px2, double py2);
bool LineCrossLine(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4);
bool GetCrossPoint(double px1, double py1, double px2, double py2, double* px, double* py, int point_num, double& result_x, double& result_y);
int  PtInRegionZXEx(double x, double y, double *pX, double *pY, int nSum, double lfSnap);
double CalDistance(double x1, double y1, double x2, double y2);

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
		, is_edge_(false)
	{

	}
	int shared_by_;
	CString index_name_n_[3];
	bool available_;
	bool is_edge_;
};

struct PolygonExt2
{
	PolygonExt2(int point_count, double* px, double* py, CString index_name)
		: point_count_(point_count)
		, px_(px)
		, py_(py)
		, index_name_(index_name)
		, is_cross_self_(-1)
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
	void RotatePoints(double startx, double starty, double startx_next, double starty_next, double endx, double endy, double endx_next, double endy_next)
	{
		int start_index = 0, end_index = 0;
		bool is_reverse = false;

		for (int i = 0; i < point_count_; ++i)
		{
			if (fabs(px_[i]-startx) < 1e-5 && fabs(py_[i]-starty) < 1e-5)
			{
				if (fabs(px_[(i+1)%point_count_]-startx_next) < 1e-5 && fabs(py_[(i+1)%point_count_]-starty_next) < 1e-5)
				{
					start_index = (i+1)%point_count_;
					is_reverse = false;
				}
				else if (fabs(px_[(i-1+point_count_)%point_count_]-startx_next) < 1e-5 && fabs(py_[(i-1+point_count_)%point_count_]-starty_next) < 1e-5)
				{
					start_index = (i-1+point_count_)%point_count_;
					is_reverse = true;
				}
			}
		}

		for (int i = 0; i < point_count_; ++i)
		{
			if (fabs(px_[i]-endx) < 1e-5 && fabs(py_[i]-endy) < 1e-5)
			{
				if (!is_reverse)
				{
					end_index = (i-1+point_count_)%point_count_;
				}
				else
				{
					end_index = (i+1)%point_count_;
				}
			}
		}

		int temp_count = 0;
		if (!is_reverse)
		{
			for (int i = start_index, j = end_index;
				(start_index-end_index)*(i-j) > 0;
				i = (i+1)%point_count_, j = (j-1+point_count_)%point_count_)
			{
				double temp = px_[i];
				px_[i] = px_[j];
				px_[j] = temp;

				temp = py_[i];
				py_[i] = py_[j];
				py_[j] = temp;

				NodeProperty temp_node = np_[i];
				np_[i] = np_[j];
				np_[j] = temp_node;
				++temp_count;
			}
		}
		else
		{
			for (int i = start_index, j = end_index;
				(start_index-end_index)*(i-j) > 0;
				i = (i-1+point_count_)%point_count_, j = (j+1)%point_count_)
			{
				double temp = px_[i];
				px_[i] = px_[j];
				px_[j] = temp;

				temp = py_[i];
				py_[i] = py_[j];
				py_[j] = temp;

				NodeProperty temp_node = np_[i];
				np_[i] = np_[j];
				np_[j] = temp_node;
				++temp_count;
			}
		}
		temp_count;
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
	int GetArea()
	{
		double area = 0;
		for (int i = 0; i < point_count_; ++i)
		{
			area += px_[i]*py_[(i+1)%point_count_]-px_[(i+1)%point_count_]*py_[i];
		}
		area /= 2;

		if (fabs(area) <1e-3)
		{
			return 0;
		}
		else if (area < 0)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	int GetDelArea()
	{
		vector<int> vecIndex;
		for (int i = 0; i < point_count_; ++i)
		{
			if (np_[i].available_)
			{
				vecIndex.push_back(i);
			}
		}

		double area = 0;
		int count = vecIndex.size();
		for (int i = 0; i < count; ++i)
		{
			area += px_[vecIndex[i]]*py_[vecIndex[(i+1)%count]]-px_[vecIndex[(i+1)%count]]*py_[vecIndex[i]];
		}
		area /= 2;

		vecIndex.clear();

		if (fabs(area) < 1e-3)
		{
			return 0;
		}
		else if (area < 0)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	bool CrossSelf()
	{
		vector<int> vectemp;
		for (int i = 0; i < point_count_; ++i)
		{
			if (np_[i].available_)
			{
				vectemp.push_back(i);
			}
		}

		int count = vectemp.size();
		for (int i = 0; i < count; ++i)
		{
			for (int j = (i+2)%count; j != (i-1+count)%count; j = (j+1)%count)
			{
				if (LineCrossLine(px_[vectemp[i]], py_[vectemp[i]], px_[vectemp[(i+1)%count]], py_[vectemp[(i+1)%count]],
					px_[vectemp[j]], py_[vectemp[j]], px_[vectemp[(j+1)%count]], py_[vectemp[(j+1)%count]]))
				{
					vectemp.clear();
					return true;
				}
			}
		}
		return false;
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
	int is_cross_self_;
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

static double GetDistance(double pointx, double pointy, double linestartx, double linestarty, 
	double lineendx, double lineendy)
{
	double a = linestarty-lineendy;
	double b = lineendx-linestartx;
	double c = linestartx*lineendy-lineendx*linestarty;

	return fabs(a*pointx+b*pointy+c)/(sqrt(a*a+b*b));
}

static void recurse(double* px, double* py, std::vector<int>& v, int start, int end, double limit, int point_count)
{
	if ((start+1)%point_count == end)
	{
		return;
	}

	int max = 0;
	int max_index = 0;

	for (int i = (start+1)%point_count; i != end; i = (i+1)%point_count)
	{
		int dis = GetDistance(px[i], py[i], px[start], py[start],
			px[end], py[end]);
		if (max == 0 || max < dis)
		{
			max = dis;
			max_index = i;
		}
	}
	
	if (max < limit)
	{
		return;
	}

	auto ite = v.begin();
	while (ite != v.end())
	{
		if (*ite == start)
		{
			v.insert(ite+1, max_index);
			break;
		}
		++ite;
	}

	recurse(px, py, v, start, max_index, limit, point_count);
	recurse(px, py, v, max_index, end, limit, point_count);
}
