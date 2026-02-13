
// Electrostatic_PotentialDlg.h: файл заголовка
//

#pragma once
#include "DrawingArea.h"
#include "afxcmn.h"

// Диалоговое окно CElectrostatic_PotentialDlg
class CElectrostatic_PotentialDlg : public CDialogEx
{
	// Создание
public:
	CElectrostatic_PotentialDlg(CWnd* pParent = nullptr);	// стандартный конструктор

	// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum {
		IDD = IDD_PASHAPOTENTIALDRAWING_DIALOG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


	// Реализация
protected:
	HICON m_hIcon;
	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	DrawingArea myDrawingArea;
	afx_msg void OnBnClickedDraw(); //кнопка нарисовать
	afx_msg void OnBnClickedCreset(); //кнопка сбросить
	std::vector<std::vector<Cell>> cells; //Массив ячеек
	int sizeOfDrawingArea; //количество ячеек (100 на 100)

	//ширина брусков
	int numIsolines;
	double D;
	double H1;
	double R;
	double H2;
	double U1;
	double U2;
	double H_weig;
	double stepFieldlinesCenter;
	double stepFieldlinesArea;
	double C;
};

//расчет потенциала в следующий момент времени
std::vector<std::vector<double>> PotentialInNextTime(std::vector<std::vector<double>> potentialInTime, std::vector<std::vector<double>> potentialStart);
