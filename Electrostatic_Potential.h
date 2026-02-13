
// Electrostatic_Potential.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CElectrostatic_PotentialApp:
// Сведения о реализации этого класса: CElectrostatic_Potential.cpp
//

class CElectrostatic_PotentialApp : public CWinApp
{
public:
	CElectrostatic_PotentialApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CElectrostatic_PotentialApp theApp;
