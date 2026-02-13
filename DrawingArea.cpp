// DrawingArea.cpp: файл реализации
//

#include "pch.h"
#include "Electrostatic_Potential.h"
#include "DrawingArea.h"

using namespace std;
// DrawingArea

IMPLEMENT_DYNAMIC(DrawingArea, CStatic)

DrawingArea::DrawingArea()
{
	Gdiplus::GdiplusStartup(&token, &input, NULL);
	U = UMax = UMin = 0;
	drawFieldlines = false;
	drawIsolines = false;
}

DrawingArea::~DrawingArea()
{
	AfxOleTerm(FALSE);
	Gdiplus::GdiplusShutdown(token);
}

BEGIN_MESSAGE_MAP(DrawingArea, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

double sign(double x)
{
	return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

// сортировка используется в алгоритме рисовки изолиний
void Sort(double arr[3], Gdiplus::PointF points[3])
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				swap(arr[j], arr[j + 1]);
				swap(points[j], points[j + 1]);
			}
		}
	}
}

void DrawingArea::DrawIsolines(Gdiplus::Graphics* grr, Gdiplus::Pen& p, vector<vector<double>> tempPotential)
{
	for (int i = 1; i < numIsolines; i++)
	{
		double isoline = UMin + i * (UMax - UMin) / numIsolines;
		double nodes[3];
		Gdiplus::PointF points[3];
		for (int i = 0; i < tempPotential.size() - 1; i++)
		{
			for (int j = 0; j < tempPotential.size() - 1; j++)
			{
				for (int k = 0; k < 2; k++) {
					double values[2], keys[2];
					if (k == 0) {
						nodes[0] = tempPotential[i][j];
						nodes[1] = tempPotential[i + 1][j];
						nodes[2] = tempPotential[i][j + 1];

						points[0] = Gdiplus::PointF(i, j);
						points[1] = Gdiplus::PointF(i + 1, j);
						points[2] = Gdiplus::PointF(i, j + 1);
					}
					else {
						nodes[0] = tempPotential[i + 1][j];
						nodes[1] = tempPotential[i + 1][j + 1];
						nodes[2] = tempPotential[i][j + 1];

						points[0] = Gdiplus::PointF(i + 1, j);
						points[1] = Gdiplus::PointF(i + 1, j + 1);
						points[2] = Gdiplus::PointF(i, j + 1);
					}
					Sort(nodes, points);

					if (isoline < nodes[0] || isoline > nodes[2])
						continue;

					Gdiplus::REAL x1, y1, x2, y2;

					if (isoline >= nodes[0] && isoline < nodes[1]) {
						x1 = points[1].X * 1. - (nodes[1] - isoline) *
							(points[1].X * 1. - points[0].X * 1.) / (nodes[1] - nodes[0]);
						y1 = points[1].Y * 1. - (nodes[1] - isoline) *
							(points[1].Y * 1. - points[0].Y * 1.) / (nodes[1] - nodes[0]);
					}
					else {
						x1 = points[2].X * 1. - (nodes[2] - isoline) *
							(points[2].X * 1. - points[1].X * 1.) / (nodes[2] - nodes[1]);
						y1 = points[2].Y * 1. - (nodes[2] - isoline) *
							(points[2].Y * 1. - points[1].Y * 1.) / (nodes[2] - nodes[1]);
					}

					x2 = points[2].X * 1. - (nodes[2] - isoline) * (points[2].X * 1. - points[0].X * 1.) / (nodes[2] - nodes[0]);
					y2 = points[2].Y * 1. - (nodes[2] - isoline) * (points[2].Y * 1. - points[0].Y * 1.) / (nodes[2] - nodes[0]);

					//Gdiplus::Pen p(Gdiplus::Color(128, 128, 128), 0.1);
					grr->DrawLine(&p, x1, y1, x2, y2);
				}
			}
		}
	}
}


void DrawingArea::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	vector<vector<double>> tempPotential = potentialInTime;
	CRect rect;
	GetClientRect(&rect);
	Gdiplus::Bitmap myBitmap(rect.Width(), rect.Height()); //создаем битовый образ
	Gdiplus::Graphics gr(lpDrawItemStruct->hDC); //создаем объект класса для рисования объектов, связанный с областью рисования
	Gdiplus::Graphics* grr = Gdiplus::Graphics::FromImage(&myBitmap); //создаем дополнительный объект класса для рисования объектов


	grr->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed); //устанавливаем сглаживание в режиме быстродействия

	//рисуем попиксельно, поэтому индексы - это пиксели
	Gdiplus::Matrix mtx(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);//создаем матрицу
	mtx.Scale(1. * rect.Width() / tempPotential.size(), 1. * rect.Height() / tempPotential.size()); //изменяем масштаб
	grr->SetTransform(&mtx);



	//шаг рисовки
	double stepx = (double)rect.Width() / tempPotential.size();
	double stepy = (double)rect.Height() / tempPotential.size();


	//рисуем по элементно
	for (int i = 0; i < tempPotential.size() - 1; i++) {
		for (int j = 0; j < tempPotential.size() - 1; j++) {
			double module = tempPotential[i][j];
			//красным рисуем плюс, а синим минус
			Gdiplus::SolidBrush br(Gdiplus::Color(module > 0 ? module * 255. / UMax : 0, 0, module < 0 ? module * 255. / UMin : 0));
			grr->FillRectangle(&br, Gdiplus::REAL(i), Gdiplus::REAL(j), Gdiplus::REAL(stepx), Gdiplus::REAL(stepy));
		}
	}
	gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());



	//рисовка силовых линий
	Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255), stepx / 30.);

	if (drawFieldlines) {
		//рисуем от краев к брускам
		int step_for = stepFieldlinesArea;
		for (int x = 1; x < potentialStart.size() - 1; x += step_for) {
			for (int y = 1; y < potentialStart.size() - 1; y += step_for) {
				if (potentialStart.size() - 2 - x < step_for) x = potentialStart.size() - 2;
				if (potentialStart.size() - 2 - y < step_for) y = potentialStart.size() - 2;
				if (x == 1 || y == 1 || x == potentialStart.size() - 2 || y == potentialStart.size() - 2) {
					if (tempPotential[x][y] == 0) break;
					double xx = x, yy = y, step = 0.4 * sign(tempPotential[x][y]);
					Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
					//перемещаем заряд
					cells[int(xx)][int(yy)].IsPointIn(xx, yy);
					cells[int(xx)][int(yy)].Propagate(xx, yy, step);
					//сохраняем в точку, чтобы нарисовать
					p2.X = xx; p2.Y = yy;
					grr->DrawLine(&pen, p1, p2);
					p1 = p2;
					//пока не дойдем до границ
					while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
						//перемещаем заряд
						cells[int(xx)][int(yy)].IsPointIn(xx, yy);
						cells[int(xx)][int(yy)].Propagate(xx, yy, step);
						//сохраняем в точку, чтобы нарисовать
						p2.X = xx; p2.Y = yy;
						grr->DrawLine(&pen, p1, p2);
						//запоминаем новое положение
						//если оказались в той же точке или шагнули в предыдущую
						if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
						p0 = p1;
						p1 = p2;
						//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
					}
					//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
				}
			}
		}

		//рисовка в центре для правого
		step_for = stepFieldlinesCenter;
		int x = potentialStart.size() / 1.9;
		for (int y = 1; y < potentialStart.size() - 1; y += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = -0.9 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}

		for (int y = 1; y < potentialStart.size() - 1; y += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = 0.9 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}
		////////////////////////////////////////////////////////////
		//рисовка в центре для левого
		x = potentialStart.size() / 2.1;
		for (int y = 1; y < potentialStart.size() - 1; y += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = -0.9 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}

		for (int y = 1; y < potentialStart.size() - 1; y += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = 0.9 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}
		////////////////////////////////////////////////////////////

		//рисовка в центре для нижнего
		int y = potentialStart.size() / 2;
		for (int x = 1; x < potentialStart.size() - 1; x += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = -0.4 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}

		for (int x = 1; x < potentialStart.size() - 1; x += step_for) {
			if (tempPotential[x][y] == 0) break;

			//координаты пробного заряда
			double xx = x, yy = y, step = 0.4 * sign(tempPotential[x][y]);
			Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
			//перемещаем заряд
			cells[int(xx)][int(yy)].IsPointIn(xx, yy);
			cells[int(xx)][int(yy)].Propagate(xx, yy, step);
			//сохраняем в точку, чтобы нарисовать
			p2.X = xx; p2.Y = yy;
			grr->DrawLine(&pen, p1, p2);
			p1 = p2;
			//пока не дойдем до границ
			while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
				//перемещаем заряд
				cells[int(xx)][int(yy)].IsPointIn(xx, yy);
				cells[int(xx)][int(yy)].Propagate(xx, yy, step);
				//сохраняем в точку, чтобы нарисовать
				p2.X = xx; p2.Y = yy;
				grr->DrawLine(&pen, p1, p2);
				//запоминаем новое положение
				//если оказались в той же точке или шагнули в предыдущую
				if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
				p0 = p1;
				p1 = p2;
				//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
			}
			//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		}

		//////////////////////////////////////////////////////////////
		////рисовка в центре для верхнего
		//y = potentialStart.size() / 1.9;
		//for (int x = 1; x < potentialStart.size() - 1; x += step_for) {
		//	if (tempPotential[x][y] == 0) break;

		//	//координаты пробного заряда
		//	double xx = x, yy = y, step = -0.4 * sign(tempPotential[x][y]);
		//	Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
		//	//перемещаем заряд
		//	cells[int(xx)][int(yy)].IsPointIn(xx, yy);
		//	cells[int(xx)][int(yy)].Propagate(xx, yy, step);
		//	//сохраняем в точку, чтобы нарисовать
		//	p2.X = xx; p2.Y = yy;
		//	grr->DrawLine(&pen, p1, p2);
		//	p1 = p2;
		//	//пока не дойдем до границ
		//	while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
		//		//перемещаем заряд
		//		cells[int(xx)][int(yy)].IsPointIn(xx, yy);
		//		cells[int(xx)][int(yy)].Propagate(xx, yy, step);
		//		//сохраняем в точку, чтобы нарисовать
		//		p2.X = xx; p2.Y = yy;
		//		grr->DrawLine(&pen, p1, p2);
		//		//запоминаем новое положение
		//		//если оказались в той же точке или шагнули в предыдущую
		//		if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
		//		p0 = p1;
		//		p1 = p2;
		//		//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		//	}
		//	//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		//}

		//for (int x = 1; x < potentialStart.size() - 1; x += step_for) {
		//	if (tempPotential[x][y] == 0) break;

		//	//координаты пробного заряда
		//	double xx = x, yy = y, step = 0.4 * sign(tempPotential[x][y]);
		//	Gdiplus::PointF p1(xx, yy), p2 = p1, p0 = p1;
		//	//перемещаем заряд
		//	cells[int(xx)][int(yy)].IsPointIn(xx, yy);
		//	cells[int(xx)][int(yy)].Propagate(xx, yy, step);
		//	//сохраняем в точку, чтобы нарисовать
		//	p2.X = xx; p2.Y = yy;
		//	grr->DrawLine(&pen, p1, p2);
		//	p1 = p2;
		//	//пока не дойдем до границ
		//	while (xx > 0 && xx < potentialStart.size() && yy > 0 && yy < potentialStart.size() - 1) {
		//		//перемещаем заряд
		//		cells[int(xx)][int(yy)].IsPointIn(xx, yy);
		//		cells[int(xx)][int(yy)].Propagate(xx, yy, step);
		//		//сохраняем в точку, чтобы нарисовать
		//		p2.X = xx; p2.Y = yy;
		//		grr->DrawLine(&pen, p1, p2);
		//		//запоминаем новое положение
		//		//если оказались в той же точке или шагнули в предыдущую
		//		if ((abs(p2.X - p1.X) < 0.01 && abs(p2.Y - p1.Y) < 0.01) || (abs(p2.X - p0.X) < 0.01 && abs(p2.Y - p0.Y) < 0.01)) break;
		//		p0 = p1;
		//		p1 = p2;
		//		//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		//	}
		//	//gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
		//}
		//////////////////////////////////////////////////////////////
	}



	if (drawIsolines) {
		DrawIsolines(grr, pen, tempPotential);
	}

	//рисовка из буфера
	gr.DrawImage(&myBitmap, 0, 0, rect.Width(), rect.Height());
	delete grr;//очистка памяти

}