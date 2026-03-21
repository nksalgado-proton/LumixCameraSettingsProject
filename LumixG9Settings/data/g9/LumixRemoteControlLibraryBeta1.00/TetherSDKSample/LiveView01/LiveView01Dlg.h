// (C) 2020 Panasonic Corporation
//
// LiveView01Dlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "LMX_func_api.h"


// CLiveView01Dlg dialog
class CLiveView01Dlg : public CDialogEx
{
// Construction
public:
	CLiveView01Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIVEVIEW01_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	LMX_CONNECT_DEVICE_INFO devInfo;
	BOOL deviceSelected;
	BOOL liveviewRunning;
	LMX_STRUCT_LIVEVIEW_STR_RECOM_IMG liveviewQualityList;

	void DrawLiveviewImage(UINT8* jpegData, UINT32 jpegDataSize);

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	int liveviewQuality;
	CComboBox cmboxDeviceList;
	CButton btnSelect;
	afx_msg void OnBnClickedRadioQuality();
	afx_msg void OnBnClickedCancel();
	CButton btnLiveview;
	afx_msg void OnBnClickedButtonLiveview();
	afx_msg void OnBnClickedButtonSelect();
	CStatic liveviewImagePlane;
};
