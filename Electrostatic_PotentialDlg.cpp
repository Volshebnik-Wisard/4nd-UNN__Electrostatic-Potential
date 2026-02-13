
// Electrostatic_PotentialDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "Electrostatic_Potential.h"
#include "Electrostatic_PotentialDlg.h"
#include "afxdialogex.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
// Диалоговое окно CElectrostatic_PotentialDlg

CElectrostatic_PotentialDlg::CElectrostatic_PotentialDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PASHAPOTENTIALDRAWING_DIALOG, pParent)
	, sizeOfDrawingArea(100)
	, H1(2)
	, U2(10)
	, D(20)

	, R(10)
	, H2(20)
	, H_weig(2)
	, U1(0.001)
	, numIsolines(70)
	, stepFieldlinesCenter(3)
	, stepFieldlinesArea(10)
	, C(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CElectrostatic_PotentialDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_drawArea, myDrawingArea); //связываем область рисования
	DDX_Text(pDX, IDC_D, D);
	DDX_Text(pDX, IDC_NUM_ISO, numIsolines);
	DDX_Text(pDX, IDC_H, H1);
	DDX_Text(pDX, IDC_H2, R);
	DDX_Text(pDX, IDC_H3, H2);
	DDX_Text(pDX, IDC_EDIT1, U1);
	DDX_Text(pDX, IDC_EDIT2, U2);
	DDX_Text(pDX, IDC_H4, H_weig);
	DDX_Text(pDX, IDC_EDIT3, sizeOfDrawingArea);
	DDX_Text(pDX, IDC_EDIT4, stepFieldlinesCenter);
	DDX_Text(pDX, IDC_EDIT5, stepFieldlinesArea);
	DDX_Text(pDX, IDC_EDIT6, C);
}

BEGIN_MESSAGE_MAP(CElectrostatic_PotentialDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDDraw, &CElectrostatic_PotentialDlg::OnBnClickedDraw)
	ON_BN_CLICKED(IDCReset, &CElectrostatic_PotentialDlg::OnBnClickedCreset)
END_MESSAGE_MAP()

// Обработчики сообщений CElectrostatic_PotentialDlg

BOOL CElectrostatic_PotentialDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию
	myDrawingArea.cells = cells = vector<vector<Cell>>(sizeOfDrawingArea, vector<Cell>(sizeOfDrawingArea));

	myDrawingArea.potentialStart = myDrawingArea.potentialInTime = vector<vector<double>>(sizeOfDrawingArea, vector<double>(sizeOfDrawingArea, 0));
	myDrawingArea.Invalidate(TRUE);

	CRect rect;
	GetDlgItem(IDC_drawArea)->GetClientRect(&rect);
	myDrawingArea.width = rect.Width();
	myDrawingArea.height = rect.Height();
	myDrawingArea.Invalidate(TRUE);
	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CElectrostatic_PotentialDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CElectrostatic_PotentialDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// кнопка рисовать
void CElectrostatic_PotentialDlg::OnBnClickedDraw()
{
	UpdateData();
	if (D <= 0 || H1 <= 0 || H2 <= 0 || H_weig <= 0 || R <= 0) {
		MessageBox(L"Нельзя ставить этот параметр <= 0", L"");
		return;
	}

	if (D <= H2)
	{
		double D11 = 1 - D / H2;
		C = 2 * D11 * 8.85418781 * H2 / 1000000000000 / log((R + H_weig / 2) / (H1 / 2));
	}
	else
	{
		C = 0;
	}

	UpdateData(FALSE);
	//проинициализируем положение брусков магнита
	cells.clear();
	cells = vector<vector<Cell>>(sizeOfDrawingArea, vector<Cell>(sizeOfDrawingArea));
	//коорд центра
	int x_mid = sizeOfDrawingArea / 2,
		y_mid = sizeOfDrawingArea / 2;

	//начальные значения потенциала
	vector<vector<double>> phi_start(sizeOfDrawingArea, vector<double>(sizeOfDrawingArea, 0));
	//значения посчитанного потенциала 
	vector<vector<double>> phi_next = phi_start;

	//инициализация клеток

	for (int x = 0; x < sizeOfDrawingArea; x++)
	{
		for (int y = 0; y < sizeOfDrawingArea; y++)
		{
			cells[x][y].pt1.x = x;
			cells[x][y].pt1.y = y;
			cells[x][y].pt1.phi = 0;

			cells[x][y].pt2.x = x + 1;
			cells[x][y].pt2.y = y;
			cells[x][y].pt2.phi = 0;

			cells[x][y].pt3.x = x + 1;
			cells[x][y].pt3.y = y + 1;
			cells[x][y].pt3.phi = 0;

			cells[x][y].pt4.x = x;
			cells[x][y].pt4.y = y + 1;
			cells[x][y].pt4.phi = 0;

		}
	}
	////стакан
	////дно
	//for (int x = x_mid - D / 2 - H_weig / 2; x < x_mid - D / 2 + H_weig / 2; x++) {
	//	//+
	//	for (int y = y_mid - R; y < y_mid + R; y++) {
	//		cells[x][y].pt1.phi = U1;
	//		cells[x][y].pt2.phi = U1;
	//		cells[x][y].pt3.phi = U1;
	//		cells[x][y].pt4.phi = U1;
	//		phi_start[x][y] = U1;
	//	}

	//}
	////вверх
	//for (int x = x_mid - D / 2; x < x_mid - D / 2 + H2; x++) {

	//	for (int y = y_mid - R; y < y_mid + H_weig - R; y++) {
	//		cells[x][y].pt1.phi = U1;
	//		cells[x][y].pt2.phi = U1;
	//		cells[x][y].pt3.phi = U1;
	//		cells[x][y].pt4.phi = U1;
	//		phi_start[x][y] = U1;

	//	}

	//}
	////низ
	//for (int x = x_mid - D / 2; x < x_mid - D / 2 + H2; x++) {

	//	for (int y = y_mid - H_weig + R; y < y_mid + R; y++) {
	//		cells[x][y].pt1.phi = U1;
	//		cells[x][y].pt2.phi = U1;
	//		cells[x][y].pt3.phi = U1;
	//		cells[x][y].pt4.phi = U1;
	//		phi_start[x][y] = U1;

	//	}

	//}

	////правый брусок -
	//for (int x = x_mid + D / 2; x < x_mid + D / 2 + H2; x++) {
	//	//-
	//	for (int y = y_mid - H1 / 2; y < y_mid + H1 / 2; y++) {
	//		cells[x][y].pt1.phi = U2;
	//		cells[x][y].pt2.phi = U2;
	//		cells[x][y].pt3.phi = U2;
	//		cells[x][y].pt4.phi = U2;
	//		phi_start[x][y] = U2;

	//	}

	//}

	//стакан
	//дно
	for (int x = x_mid - R - H_weig / 2; x < x_mid + R + H_weig / 2; x++) {

		for (int y = y_mid + D / 2 - H_weig / 2; y < y_mid + D / 2 + H_weig / 2; y++) {
			cells[x][y].pt1.phi = U1;
			cells[x][y].pt2.phi = U1;
			cells[x][y].pt3.phi = U1;
			cells[x][y].pt4.phi = U1;
			phi_start[x][y] = U1;

		}


	}

	//право
	for (int x = x_mid + R - H_weig / 2; x < x_mid + R + H_weig / 2; x++) {
		for (int y = y_mid + D / 2 - H2 - H_weig / 2; y < y_mid + D / 2; y++) {
			cells[x][y].pt1.phi = U1;
			cells[x][y].pt2.phi = U1;
			cells[x][y].pt3.phi = U1;
			cells[x][y].pt4.phi = U1;
			phi_start[x][y] = U1;
		}


	}

	//лево
	for (int x = x_mid - R - H_weig / 2; x < x_mid - R + H_weig / 2; x++) {
		for (int y = y_mid + D / 2 - H2 - H_weig / 2; y < y_mid + D / 2; y++) {
			cells[x][y].pt1.phi = U1;
			cells[x][y].pt2.phi = U1;
			cells[x][y].pt3.phi = U1;
			cells[x][y].pt4.phi = U1;
			phi_start[x][y] = U1;
		}


	}

	//брусок -
	for (int x = x_mid - H1 / 2; x < x_mid + H1 / 2; x++) {
		//-
		for (int y = y_mid - D / 2 - H2; y < y_mid - D / 2; y++) {
			cells[x][y].pt1.phi = U2;
			cells[x][y].pt2.phi = U2;
			cells[x][y].pt3.phi = U2;
			cells[x][y].pt4.phi = U2;
			phi_start[x][y] = U2;

		}
	}



	//рассчитаем потенциал
	for (int i = 0; i < 5000; i++)
	{
		phi_next = PotentialInNextTime(phi_next, phi_start);
	}

	//сохраним значения в клетки
	for (int x = 0; x < sizeOfDrawingArea - 1; x++) {
		for (int y = 0; y < sizeOfDrawingArea - 1; y++) {
			cells[x][y].pt1.phi = phi_next[x][y];
			cells[x][y].pt2.phi = phi_next[x + 1][y];
			cells[x][y].pt3.phi = phi_next[x + 1][y + 1];
			cells[x][y].pt4.phi = phi_next[x][y + 1];

		}
	}

	//посчитаем нормали для силовых линий
	for (int x = 0; x < sizeOfDrawingArea; x++) {
		for (int y = 0; y < sizeOfDrawingArea; y++) {
			cells[x][y].CalculateNormals();
		}
	}

	//рисовать изолинии
	if (((CButton*)GetDlgItem(IDC_CHECK_ISO))->GetCheck() == BST_UNCHECKED)
		myDrawingArea.drawIsolines = false;
	else
		myDrawingArea.drawIsolines = true;

	//рисовать силовые лини
	if (((CButton*)GetDlgItem(IDC_CHECK_FIELD))->GetCheck() == BST_UNCHECKED)
		myDrawingArea.drawFieldlines = false;
	else
		myDrawingArea.drawFieldlines = true;

	if (numIsolines < 0) { numIsolines = 0; UpdateData(FALSE); }

	myDrawingArea.numIsolines = numIsolines;
	myDrawingArea.stepFieldlinesArea = stepFieldlinesArea;
	myDrawingArea.stepFieldlinesCenter = stepFieldlinesCenter;
	if (U1 >= U2)
	{
		myDrawingArea.UMax = U1;
		myDrawingArea.UMin = U2;
	}
	else
	{
		myDrawingArea.UMax = U2;
		myDrawingArea.UMin = U1;
	}

	myDrawingArea.potentialStart = phi_start;
	myDrawingArea.potentialInTime = phi_next;
	myDrawingArea.cells = cells;

	myDrawingArea.Invalidate();
}

//потенциал в следующий момент времени
vector<vector<double>> PotentialInNextTime(vector<vector<double>> potentialInTime, vector<vector<double>> potentialStart)
{
	vector<vector<double>> out(potentialInTime.size(), vector<double>(potentialInTime.size()));
	//усредняем значения в узлах соседних
	//шаг = 1
	for (int i = 0; i < potentialInTime.size(); i++) {
		for (int j = 0; j < potentialInTime.size(); j++) {
			//граничные условия
			if (i == 0 || j == 0 || i == potentialInTime.size() - 1 || j == potentialInTime.size() - 1) {
				out[i][j] = 0;
			}
			else if (potentialStart[i][j] != 0) {
				out[i][j] = potentialStart[i][j];
			}
			else {
				double summa = 0;
				int count = 0;
				if (i > 0) {
					summa += potentialInTime[i - 1][j];
					count++;
				}
				if (j > 0) {
					summa += potentialInTime[i][j - 1];
					count++;
				}
				if (i < potentialInTime.size() - 1) {
					summa += potentialInTime[i + 1][j];
					count++;
				}
				if (j < potentialInTime.size() - 1) {
					summa += potentialInTime[i][j + 1];
					count++;
				}
				out[i][j] = summa / count;
			}
		}
	}
	return out;
}

void CElectrostatic_PotentialDlg::OnBnClickedCreset()
{

	myDrawingArea.UMax = 0;
	myDrawingArea.UMin = 0;
	myDrawingArea.potentialStart = myDrawingArea.potentialInTime = vector<vector<double>>(sizeOfDrawingArea, vector<double>(sizeOfDrawingArea, 0));
	myDrawingArea.Invalidate(FALSE);
	vector<double> proc;
}

