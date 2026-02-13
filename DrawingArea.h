#pragma once
#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>

struct PotPt
{
	//потенциал
	double phi;
	//int i;
	//int j;
	//координата
	double x, y;

	PotPt& operator=(PotPt& other)
	{
		//i = other.i;
		//j = other.j;
		x = other.x;
		y = other.y;

		phi = other.phi;
		return *this;
	}

};


//			   1  -----------  2
//				 |   	   / |
//				 Y   	 /   |
//				 |     / pt  |
//				 |	 /       |
//				 | /	     |
//        	   4  -------X---  3

//ячейка
struct Cell
{

	bool upperTri; //верхний треугольник
	bool lowerTri; //нижний треугольник
	PotPt  pt1, pt2, pt3, pt4;
	double nx_upper, ny_upper;
	double nx_lower, ny_lower;

	//проверка, что точка в ячейке
	bool IsPointIn(double x, double y)
	{
		/// Эта функция смотрит где относительно диагональной оси в квадрате находится точка. Диагон ось рассматриваю как прямую y=kx
		//
		//		Y2 - Y1
		//	K = --------	(X2; Y2) - Моя верхняя правая точка квадрата, (X1; Y1) - координаты рассматриваемой точки
		//		X2 - X1
		//	
		// Так как У увлечивается вниз, а Х вправо, если k>= -1, то точка лежит в нижнем треугольнике
		double k;
		if (x >= pt1.x && x <= pt3.x && y >= pt1.y && y <= pt3.y)
		{
			k = (y - pt2.y) / (x - pt2.x);
			if (k >= -1) { upperTri = false; lowerTri = true; }
			else { upperTri = true; lowerTri = false; }

			return true;
		}
		else return false;
	}
	//рассчет нормалей
	void CalculateNormals()
	{

		double length;
		nx_upper = (pt3.y - pt1.y) * (pt2.phi - pt1.phi) - (pt2.y - pt1.y) * (pt3.phi - pt1.phi);
		ny_upper = -(pt3.x - pt1.x) * (pt2.phi - pt1.phi) + (pt2.x - pt1.x) * (pt3.phi - pt1.phi);

		nx_lower = (pt4.y - pt1.y) * (pt3.phi - pt1.phi) - (pt3.y - pt1.y) * (pt4.phi - pt1.phi);
		ny_lower = -(pt4.x - pt1.x) * (pt3.phi - pt1.phi) + (pt3.x - pt1.x) * (pt4.phi - pt1.phi);


		length = sqrt(ny_upper * ny_upper + nx_upper * nx_upper);
		if (length != 0) {
			nx_upper /= length;
			ny_upper /= length;
		}
		length = sqrt(ny_lower * ny_lower + nx_lower * nx_lower);
		if (length != 0)
		{
			nx_lower /= length;
			ny_lower /= length;
		}
	}

	// где окажется в следующий момент времени
	void Propagate(double& x, double& y, double& step)
	{
		if (upperTri)
		{
			x += step * nx_upper;
			y += step * ny_upper;
		}
		else if (lowerTri)
		{
			x += step * nx_lower;
			y += step * ny_lower;
		}
	}
};

// DrawingArea

class DrawingArea : public CStatic
{
	DECLARE_DYNAMIC(DrawingArea)

public:
	Gdiplus::GdiplusStartupInput input;
	ULONG_PTR token;
	//размеры поля
	int width;
	int height;
	//мин и макс потенциала (для градуировки рисования)
	double UMax;
	double UMin;

	//лог рисовать потенциальные, силовые линии
	bool drawFieldlines;
	bool drawIsolines;
protected:
	DECLARE_MESSAGE_MAP()

public:
	std::vector<std::vector<double>> potentialStart; //значения потенциала стартовые
	std::vector<std::vector<double>> potentialInTime; //значения потенциала меняющиеся во времени
	std::vector<std::vector<Cell>> cells; // ячейки
	double U; //каким значением потенциала рисовать
	int numIsolines; //количество изолний
	int stepFieldlinesCenter; //количество изолний
	int stepFieldlinesArea; //количество изолний
	DrawingArea();
	virtual ~DrawingArea();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void DrawIsolines(Gdiplus::Graphics* grr, Gdiplus::Pen& p, std::vector<std::vector<double>> tempPotential); //рисовка изолиний
};


