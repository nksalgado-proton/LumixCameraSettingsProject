// (C) 2020 Panasonic Corporation
//
// LiveView01.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLiveView01App:
// See LiveView01.cpp for the implementation of this class
//

class CLiveView01App : public CWinApp
{
public:
	CLiveView01App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLiveView01App theApp;