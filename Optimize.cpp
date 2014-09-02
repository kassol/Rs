#include "stdafx.h"
#include "Optimize.h"
#include "clipper.hpp"
#include <algorithm>
#include <io.h>


#define _SnapSame(x1, x2, lfSnap)		(0==lfSnap?x1==x2:fabs(x1-x2)<lfSnap)
#define _SnapLarge(x1, x2, lfSnap)	    (x1>x2-lfSnap)
#define _SnapLarge2(x1, x2, lfSnap)	    (x1>x2+lfSnap)

using namespace ClipperLib;

double GetDistance(double pointx, double pointy, double linestartx, double linestarty, double lineendx, double lineendy)
{
	double a = linestarty-lineendy;
	double b = lineendx-linestartx;
	double c = linestartx*lineendy-lineendx*linestarty;

	return fabs(a*pointx+b*pointy+c)/(sqrt(a*a+b*b));
}

double CalDistance(double x1, double y1, double x2, double y2)
{
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}

bool Optimize(CString strAllDomPath, CString strDxfPath, CString strRrlxPath)
{
	CString path;
	std::vector<CString> vecImagePath;
	
	while (1)
	{
		int index = strAllDomPath.ReverseFind(';');
		if (index == -1)
		{
			vecImagePath.push_back(strAllDomPath);
			break;
		}
		else
		{
			vecImagePath.push_back(strAllDomPath.Right(strAllDomPath.GetLength()-index-1));
			strAllDomPath = strAllDomPath.Left(index);
		}
	}
	
	
	auto path_ite = vecImagePath.begin();
	std::fstream infile;
	std::vector<PolygonExt2> polygons;
	
	while (path_ite != vecImagePath.end())
	{
		CString image_path = *path_ite;
		path = image_path.Left(image_path.ReverseFind('\\')+1);
		CString index_name = image_path.Right(image_path.GetLength()-image_path.ReverseFind('\\')-1);
		index_name = index_name.Left(index_name.ReverseFind('.'));
		CString rrlx_path = strRrlxPath+index_name+_T(".rrlx");
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
		++polygon_ite;
	}
	
// 	if (!EffectPoly(vecImagePath))
// 	{
// 		return false;
// 	}
	
	//读取有效区域
	std::vector<PolygonExt2> EffPolygons;
	path_ite = vecImagePath.begin();
	while (path_ite != vecImagePath.end())
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
	
	//根据有效区域移点
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
					return false;
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
						return false;
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
					if (result.size() == 0)
					{
						return false;
					}
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
										delete []limit_poly_x;
										delete []limit_poly_y;
										limit_poly_x = NULL;
										limit_poly_y = NULL;
										return false;
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
							delete []limit_poly_x;
							delete []limit_poly_y;
							limit_poly_x = NULL;
							limit_poly_y = NULL;
							return false;
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
									delete []limit_poly_x;
									delete []limit_poly_y;
									limit_poly_x = NULL;
									limit_poly_y = NULL;
									return false;
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
					delete []limit_poly_x;
					delete []limit_poly_y;
					limit_poly_x = NULL;
					limit_poly_y = NULL;
					return false;
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

					if (-1 == PtInRegionZXEx(geox, geoy, tempx, tempy, count, 0.000001))
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
		polygon_ite->Output(strRrlxPath);
		++polygon_ite;
	}
	
	//dxf转dem
	IImageX* pImage = NULL;
	CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	if (S_FALSE == pImage->Open(vecImagePath.front().AllocSysString(), modeRead))
	{
		pImage->Release();
		return false;
	}
	double tmp_cellsize = 0, tmp = 0;
	pImage->GetGrdInfo(&tmp, &tmp, &tmp_cellsize);
	pImage->Close();
	if (!Dxf2Dsm(strDxfPath, tmp_cellsize))
	{
		return false;
	}
	CString strDsmPath = strDxfPath.Left(strDxfPath.ReverseFind('.'));
	strDsmPath += _T(".dem");
	
	//dem转tif
	if (!Dsm2Tif(strDsmPath))
	{
		return false;
	}
	CString strTifPath = strDsmPath.Left(strDsmPath.ReverseFind('.'));
	strTifPath += _T(".tif");
	
	
	//根据dsm移点
	if (S_FALSE == pImage->Open(strTifPath.AllocSysString(), modeRead))
	{
		pImage->Release();
		return false;
	}
	
	
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
	
	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		double* px = polygon_ite->px_;
		double* py = polygon_ite->py_;
		int num = polygon_ite->point_count_;

		CString image_path = path+polygon_ite->index_name_+_T(".tif");
		if (S_FALSE == tempImage->Open(image_path.AllocSysString(), modeRead))
		{
			tempImage->Release();
			return false;
		}
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
				delete []buf;
				buf = NULL;
			}
		}
		++polygon_ite;
	}
	
	//获取dxf中的多边形
	CDrawing m_dxffile;
	std::vector<int> vecPointNum;
	std::vector<double*> vecX;
	std::vector<double*> vecY;
	
	if (m_dxffile.Create() && m_dxffile.LoadDXFFile(strDxfPath) == TRUE)
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
	else
	{
		pImage->Close();
		pImage->Release();
		tempImage->Release();
		return false;
	}
	
	//走最短路径
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
					if (-1 != PtInRegionZXEx(px[point_index], py[point_index], pEdgex, pEdgey, 4, 1e-5) &&
						-1 != PtInRegionZXEx(px[(point_index+1)%point_count], py[(point_index+1)%point_count], pEdgex, pEdgey, 4, 1e-5))
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
							if (-1 != PtInRegionZXEx(px[point_index], py[point_index], pEdgex, pEdgey, 4, 1e-5))
							{
								short_start_x = px[point_index];
								short_start_y = py[point_index];
								short_end_x = result_x;
								short_end_y = result_y;
							}
							else if (-1 != PtInRegionZXEx(px[(point_index+1)%point_count], py[(point_index+1)%point_count], pEdgex, pEdgey, 4, 1e-5))
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

					if(S_FALSE == shortpath->ShortestPathviaPoly(strDsmPath.AllocSysString(), short_start_x, short_start_y,
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
							if (-1 != PtInRegionZXEx(short_start_x, short_start_y, rect_x, rect_y, 4, 1e-5))
							{
								short_end_x = result_x;
								short_end_y = result_y;
							}
							else if (-1 != PtInRegionZXEx(short_end_x, short_end_y, rect_x, rect_y, 4, 1e-5))
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
						shortpath->ShortestPathviaPoly(strDsmPath.AllocSysString(), short_start_x, short_start_y,
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
	
	//删除造成相交的点
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
						isin |= (-1 != PtInRegionZXEx(px[count], py[count], polygon_ite2->px_, polygon_ite2->py_, polygon_ite2->point_count_, 0.00001));
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
		polygon_ite->Output(strRrlxPath.GetBuffer(0));
		++polygon_ite;
	}
	
	pImage->Close();
	pImage->Release();
	tempImage->Release();
	
	polygon_ite = polygons.begin();
	while (polygon_ite != polygons.end())
	{
		polygon_ite->Free();
		++polygon_ite;
	}
	return true;
}

bool EffectPoly(std::vector<CString>& vecImagePath)
{
	const int BG_COLOR = 255;
	std::vector<CString>::const_iterator image_path = vecImagePath.begin();
	
	IImageX* pImage = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
	if (FAILED(hRes))
	{
		return false;
	}
	
	while (image_path != vecImagePath.end())
	{
		CString point_path = (*image_path)+_T(".ep");
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

		int scan_count = 50;
		int interval = nYSize/scan_count;

		for (int y = 0; y < nYSize; y += interval)
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
	
	pImage->Release();
	
	return true;
}

bool Dxf2Dsm(CString strDxf, double tmp_cellsize)
{
	CString strDsmPath = strDxf.Left(strDxf.ReverseFind('.'));
	strDsmPath += _T(".dem");
	if (_access(strDsmPath, 0) == -1)
	{
		CDrawing m_dxffile;
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
	
			
			double cellsize = tmp_cellsize*20;
	
			double lfXOrigin = int(lfMinx/cellsize)*cellsize;
			double lfYOrigin = int(lfMiny/cellsize)*cellsize;
			double lfXEnd = int(lfMaxx/cellsize+1)*cellsize;
			double lfYEnd = int(lfMaxy/cellsize+1)*cellsize;
			int nXSize = int((lfXEnd-lfXOrigin)/cellsize);
			int nYSize = int((lfYEnd-lfYOrigin)/cellsize);
	
			std::fstream out;
			out<<std::fixed;
			out.open(strDsmPath.GetBuffer(0), std::ios::out);
			out<<"NSDTF-DEM\n";
			CString tmp;
			tmp.Format("%.6f\n", cellsize);
			out<<tmp.GetBuffer(0);
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

			float* pbuf = new float[nXSize*nYSize];
			memset(pbuf, 0, nXSize*nYSize*sizeof(float));
			auto itex = vecX.begin();
			auto itey = vecY.begin();
			auto itenum = vecPointNum.begin();
			auto itexx = vecEXx.begin();
			auto iteyy = vecExy.begin();
			while (itex != vecX.end())
			{
				int tmp_startx = 0, tmp_starty = 0;
				int tmp_xsize = 0, tmp_ysize = 0;

				tmp_startx = ((*itexx)[0]-lfXOrigin)/cellsize;
				for (double y = (*iteyy)[0]; y > (*iteyy)[1]; y -= cellsize)
				{
					for (double x = (*itexx)[0]; x < (*itexx)[2]; x += cellsize)
					{
						if (-1 != PtInRegionZXEx(x, y, *itex, *itey, *itenum, 1e-2))
						{
							pbuf[int((lfYEnd-y)/cellsize+0.5)*nXSize+int((x-lfXOrigin)/cellsize+0.5)] = high;
						}
					}
				}

				++itex;
				++itey;
				++itenum;
				++itexx;
				++iteyy;
			}

			for (int y = 0; y < nYSize; ++y)
			{
				for (int x = 0; x < nXSize; ++x)
				{
					out<<pbuf[y*nXSize+x]<<" ";
				}
				out<<"\n";
			}
			delete []pbuf;
			pbuf = NULL;
			out<<"\n";

			out.close();
	
			
			itex = vecX.begin();
			itey = vecY.begin();
			itexx = vecEXx.begin();
			iteyy = vecExy.begin();
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
			return false;
		}
	}
	return true;
}

bool Dsm2Tif(CString strDsmPath)
{
	CString strTifPath = strDsmPath.Left(strDsmPath.ReverseFind('.'));
	strTifPath += _T(".tif");
	if (_access(strTifPath.GetBuffer(0), 0) == -1)
	{
		IImageX* pImage = NULL;
		HRESULT hr = CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&pImage);
		if (FAILED(hr))
		{
			return false;
		}
		
		const int nBlockSize = 128;
		ifstream infile;
		float lfXOrigin;
		float lfYOrigin;
	
		float lfXResolution;
		float lfYResolution;
	
		int nXSize;
		int nYSize;
	
		double lfXStart;
		double lfYStart;
		
		infile.open(strDsmPath.GetBuffer(0), ios::in);
		string tmp;
		infile>>tmp;
		double lfZoom = 1;
		
		if (tmp == "NSDTF-DEM")
		{
			infile>>tmp;
			infile>>tmp;
			infile>>tmp;
			infile>>tmp;
			infile>>lfXOrigin;
			infile>>lfYOrigin;
			infile>>lfXResolution;
			infile>>lfYResolution;
			infile>>nYSize;
			infile>>nXSize;
			infile>>lfZoom;
		}
		else
		{
			infile.close();
			infile.open(strDsmPath.GetBuffer(0), ios::in);
			float Keyp[7];
			infile>>Keyp[0];
			infile>>Keyp[1];
			infile>>Keyp[2];
			infile>>Keyp[3];
			infile>>Keyp[4];
			infile>>Keyp[5];
			infile>>Keyp[6];
	
			lfXOrigin = Keyp[0];
			lfYOrigin = Keyp[1];
	
			lfXResolution = Keyp[3];
			lfYResolution = Keyp[4];
	
			nXSize = (int)Keyp[5];
			nYSize = (int)Keyp[6];
		}
		
		lfXStart = lfXOrigin-lfXResolution/2;
		lfYStart = lfYOrigin+lfYResolution/2-nYSize*lfYResolution;
		
		pImage->CreateImg(strTifPath.AllocSysString(), modeCreate, nXSize, nYSize, 
			Pixel_Int16, 1, BIP, lfXStart, lfYStart, lfXResolution);
		
		unsigned short* temBuf = new unsigned short[nXSize*nBlockSize];
		memset(temBuf, 0, sizeof(unsigned short)*nXSize*nBlockSize);
		float* fBuf = new float[nXSize*nBlockSize];
		memset(fBuf, 0, sizeof(float)*nXSize*nBlockSize);
		for (int y = 0; y < nYSize;)
		{
			if (y+nBlockSize < nYSize)
			{
				for (int n = nBlockSize-1; n >= 0; --n)
				{
					for (int m = 0; m < nXSize; ++m)
					{
						infile>>fBuf[n*nXSize+m];
					}
				}
				for (int n = 0; n < nBlockSize; ++n)
				{
					for (int m = 0; m < nXSize; ++m)
					{
						if (fBuf[n*nXSize+m] < 0)
						{
							temBuf[n*nXSize+m] = 65535;
						}
						else
						{
							temBuf[n*nXSize+m] = (unsigned short)(fBuf[n*nXSize+m]/lfZoom);
						}
					}
				}
				pImage->WriteImg(0, nYSize-y-nBlockSize, nXSize, nYSize-y, (unsigned char*)temBuf, nXSize, nBlockSize,
					1, 0, 0, nXSize, nBlockSize, -1, 0);
				y += nBlockSize;
			}
			else
			{
				for (int n = nYSize-y-1; n >= 0; --n)
				{
					for (int m = 0; m < nXSize; ++m)
					{
						infile>>fBuf[n*nXSize+m];
					}
				}
				for (int n = 0; n < nBlockSize; ++n)
				{
					for (int m = 0; m < nXSize; ++m)
					{
						if (fBuf[n*nXSize+m] < 0)
						{
							temBuf[n*nXSize+m] = 65535;
						}
						else
						{
							temBuf[n*nXSize+m] = (unsigned short)(fBuf[n*nXSize+m]/lfZoom);
						}
					}
				}
				pImage->WriteImg(0, 0, nXSize, nYSize-y, (unsigned char*)temBuf, nXSize, nBlockSize, 1,
					0, 0, nXSize, nYSize-y, -1, 0);
				y = nYSize;
			}
		}
		pImage->Close();
		pImage->Release();
		delete []fBuf;
		fBuf = NULL;
		delete []temBuf;
		temBuf = NULL;
	}
	return true;
}

bool LineCrossPolygon(std::vector<double*>& vecx, std::vector<double*>& vecy, std::vector<int>& point_num, double px1, double py1, double px2, double py2)
{
	auto itex = vecx.begin();
	auto itey = vecy.begin();
	auto ite_point_count = point_num.begin();
	while (itex != vecx.end())
	{
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

bool LineCrossLine(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
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

bool GetCrossPoint(double px1, double py1, double px2, double py2, double* px, double* py, int point_num, double& result_x, double& result_y)
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

int PtInRegionZXEx(double x, double y, double *pX, double *pY, int nSum, double lfSnap)
{
	if (nSum <= 0 || NULL == pX || NULL == pY)
	{
		return FALSE;
	}
	int i0,i1,i2, ret=0;
	double xmin1, xmax1, ymin1, ymax1;
	for( int i=0; i<nSum; i++ )
	{
		i0 = i;
		i1 = (i0+1)%nSum;

		// ¶¥µãÖØºÏÅÐ¶Ï
		//		if( pX[i0] == x && pY[i0] == y )
		if( _SnapSame( pX[i0],x,lfSnap ) && _SnapSame( pY[i0],y,lfSnap ) )
			return 0;

		// ÅÐ¶ÏÊÇ·ñ´æÔÚÏà½»µÄ¿ÉÄÜ
		if( pX[i0]<pX[i1] ){ xmin1 = pX[i0]; xmax1 = pX[i1]; }
		else { xmin1 = pX[i1]; xmax1 = pX[i0]; }
		if( pY[i0]<pY[i1] ){ ymin1 = pY[i0]; ymax1 = pY[i1]; }
		else { ymin1 = pY[i1]; ymax1 = pY[i0]; }
		//		if( y<ymin1 || y>ymax1 || x>xmax1 )continue;
		if( _SnapLarge2(ymin1,y,lfSnap) || _SnapLarge2(y,ymax1,lfSnap) || _SnapLarge2(x,xmax1,lfSnap) )
			continue;

		//ÅÐ¶ÏÊÇ·ñÔÚ±ßÉÏ


		// Ë®Æ½±ß½ç´¦Àí£¬Ö»ÐèÒªÅÐ¶ÏµãÊÇ·ñÔÚ±ß½çÉÏ
		if( pY[i1] == pY[i0] )
			//		if( _SnapSame(pY[i1],pY[i0],lfSnap) )
		{
			//			if( pY[i0]==y )
			if( _SnapSame(pY[i0],y,lfSnap) )
			{
				//				if( ( pX[i0]>x && pX[i1]<x ) ||
				//					( pX[i0]<x && pX[i1]>x ) )
				if( ( _SnapLarge(pX[i0],x,lfSnap) && _SnapLarge(x,pX[i1],lfSnap) ) ||
					( _SnapLarge(x,pX[i0],lfSnap) && _SnapLarge(pX[i1],x,lfSnap) ) )
					return 0;
			}
		}
		// Ïà½»¼«ÏÞÅÐ¶Ï£ºÖ»Óë¶¥µãÏà½»
		// ÕâÀï²»»áÓÐµãÔÚ±ß½çÉÏµÄ¿ÉÄÜ
		//		if( y==ymin1 || y==ymax1 )
		else if( _SnapSame(y,ymin1,lfSnap) || _SnapSame(y,ymax1,lfSnap) )
		{
			if(_SnapSame(y,ymin1,lfSnap) && _SnapSame(y,ymax1,lfSnap)&&
				(( _SnapLarge(pX[i0],x,lfSnap) && _SnapLarge(x,pX[i1],lfSnap) ) ||
				( _SnapLarge(x,pX[i0],lfSnap) && _SnapLarge(pX[i1],x,lfSnap) )))
			{
				return 0;
			}
			//			if( y==pY[i0] && x<pX[i0] )
			if( _SnapSame(y,pY[i0],lfSnap) && _SnapLarge(pX[i0],x,lfSnap) )
			{
				// ÅÐ¶ÏÇ°Ò»¸öµãºÍºóÒ»¸öµãÊÇ·ñÔÚÁ½²à£¬ÔÚÁ½²àµÄ»°ret¼Ó1
				i2 = i0;
				while( true )
				{
					i2 = (i2-1+nSum)%nSum;
					if( pY[i2] == pY[i0] )
					{
						if( i2 == i1 || i2 == i0 )break;
						continue;
					}
					//					if( ( pY[i2]<y && pY[i1]>y ) ||
					//						( pY[i2]>y && pY[i1]<y ) )
					if( ( _SnapLarge(y,pY[i2],lfSnap) && _SnapLarge(pY[i1],y,lfSnap) ) ||
						( _SnapLarge(pY[i2],y,lfSnap) && _SnapLarge(y,pY[i1],lfSnap) ) )
					{
						ret ++;
					}
					break;
				}
			}
		}
		//		else if( x==xmin1 || x==xmax1 )
		else if( _SnapSame(x,xmin1,lfSnap) || _SnapSame(x,xmax1,lfSnap))
		{
			if(_SnapSame(x,xmin1,lfSnap) && _SnapSame(x,xmax1,lfSnap)&&
				(( _SnapLarge(y,pY[i0],lfSnap) && _SnapLarge(pY[i1],y,lfSnap) ) ||
				( _SnapLarge(pY[i0],y,lfSnap) && _SnapLarge(y,pY[i1],lfSnap) )))
			{
				return 0;
			}
			//			if( x==xmin1 )
			if( _SnapSame(x,xmin1,lfSnap) )
			{
				ret ++;
			}
		}
		//		if( x<xmin1 )ret++;
		else if( _SnapLarge(xmin1,x,lfSnap) )
			ret ++;
		else
		{
			xmax1 = (y-pY[i0])*(pX[i1]-pX[i0])/(pY[i1]-pY[i0])+pX[i0];
			//			if( x==xmax1 )return 0;//±ß½çÉÏ
			if( _SnapSame(x,xmax1,lfSnap) )return 0;
			//			if( x<xmax1 )ret++;
			if( _SnapLarge(xmax1,x,lfSnap) )ret ++;
		}
	}
	return 1==ret%2? 1:-1;
}


















