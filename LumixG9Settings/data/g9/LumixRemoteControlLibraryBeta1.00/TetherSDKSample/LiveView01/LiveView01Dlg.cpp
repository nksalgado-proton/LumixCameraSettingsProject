// (C) 2020 Panasonic Corporation
//
// LiveView01Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "LiveView01.h"
#include "LiveView01Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLiveView01Dlg dialog



CLiveView01Dlg::CLiveView01Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LIVEVIEW01_DIALOG, pParent)
	, liveviewQuality(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLiveView01Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_QUALITY_LOW, liveviewQuality);
	DDX_Control(pDX, IDC_COMBO_DEVICE_LIST, cmboxDeviceList);
	DDX_Control(pDX, IDC_BUTTON_SELECT, btnSelect);
	DDX_Control(pDX, IDC_BUTTON_LIVEVIEW, btnLiveview);
	DDX_Control(pDX, IDC_STATIC_IMAGE, liveviewImagePlane);
}

BEGIN_MESSAGE_MAP(CLiveView01Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO_QUALITY_LOW, &CLiveView01Dlg::OnBnClickedRadioQuality)
	ON_BN_CLICKED(IDC_RADIO_QUALITY_STANDARD, &CLiveView01Dlg::OnBnClickedRadioQuality)
	ON_BN_CLICKED(IDC_RADIO_QUALITY_FINE, &CLiveView01Dlg::OnBnClickedRadioQuality)
	ON_BN_CLICKED(IDCANCEL, &CLiveView01Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_LIVEVIEW, &CLiveView01Dlg::OnBnClickedButtonLiveview)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CLiveView01Dlg::OnBnClickedButtonSelect)
END_MESSAGE_MAP()


// CLiveView01Dlg message handlers

BOOL CLiveView01Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	deviceSelected = FALSE;
	liveviewRunning = FALSE;

	UINT8 ret;
	UINT32 retError;

	// Initiaize
	LMX_func_api_Init();

	// Get device info
	ret = LMX_func_api_Get_PnPDeviceInfo(&devInfo, &retError);

	// 
	for (UINT32 i = 0; i < devInfo.find_PnpDevice_Count; i++) {
		int index = cmboxDeviceList.AddString(devInfo.find_PnpDevice_Info[i].dev_ModelName);
		cmboxDeviceList.SetItemData(index, i);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLiveView01Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLiveView01Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLiveView01Dlg::OnBnClickedCancel()
{
	UINT8 ret;
	UINT32 retError;

	// Close session
	ret = LMX_func_api_Close_Session(&retError);

	// Close device
	ret = LMX_func_api_Close_Device(&retError);

	CDialogEx::OnCancel();
}


void CLiveView01Dlg::OnBnClickedRadioQuality()
{
	UINT8 ret;
	UINT32 retError;

	UpdateData();

	if (deviceSelected) {
		// Set liveview quality
		ret = LMX_func_api_LiveView_Str_Set_Trans_Img(liveviewQualityList.RecomImgQualityListData[liveviewQuality], &retError);
	}
}


void CLiveView01Dlg::DrawLiveviewImage(UINT8* jpegData, UINT32 jpegDataSize)
{
	CDC* liveviewImagePlaneDC = liveviewImagePlane.GetDC();

	RECT liveviewImagePlaneRect;
	liveviewImagePlane.GetWindowRect(&liveviewImagePlaneRect);

	IStream* pIstream = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &pIstream);
	pIstream->Write(jpegData, jpegDataSize, NULL);

	CImage image;
	image.Load(pIstream);

	CBitmap *bitmap = CBitmap::FromHandle(image);
	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(liveviewImagePlaneDC);
	CBitmap *prevBitmap = bitmapDC.SelectObject(bitmap);

	liveviewImagePlaneDC->SetStretchBltMode(STRETCH_HALFTONE);
	liveviewImagePlaneDC->SetBrushOrg(0, 0);

	int srcWidth = image.GetWidth();
	int srcHeight = image.GetHeight();
	double srcAspectRatio = (double)srcWidth / (double)srcHeight;
	int targetX = 0;
	int targetY = 0;
	int targetWidth = liveviewImagePlaneRect.right - liveviewImagePlaneRect.left;
	int targetHeight = liveviewImagePlaneRect.bottom - liveviewImagePlaneRect.top;
	if ((double)targetWidth / (double)targetHeight >= srcAspectRatio) {
		int newWidth = (int)(targetHeight * srcAspectRatio);
		targetX = (targetWidth - newWidth) / 2;
		targetWidth = newWidth;
	}
	else {
		int newHeight = (int)(targetWidth / srcAspectRatio);
		targetY = (targetHeight - newHeight) / 2;
		targetHeight = newHeight;
}

	liveviewImagePlaneDC->StretchBlt(
		targetX, targetY, targetWidth, targetHeight,
		&bitmapDC,
		0, 0, srcWidth, srcHeight,
		SRCCOPY);

	bitmapDC.SelectObject(prevBitmap);
	bitmap->DeleteObject();
	bitmapDC.DeleteDC();

	pIstream->Release();

	ReleaseDC(liveviewImagePlaneDC);
}

// Liveview Thread
DWORD WINAPI LiveView_ThreadProc(LPVOID lpParameter)
{
	CLiveView01Dlg* liveViewDlg = (CLiveView01Dlg*)lpParameter;

	UINT32 returnedJpegSize;

	LMX_STRUCT_LIVEVIEW_INFO_HISTGRAM histgramBuf;
	UINT32 histgramBufSize = sizeof(LMX_STRUCT_LIVEVIEW_INFO_HISTGRAM);
	LMX_STRUCT_LIVEVIEW_INFO_POSTURE	postureBuf;
	UINT32 postureBufSize = sizeof(LMX_STRUCT_LIVEVIEW_INFO_POSTURE);
	LMX_STRUCT_LIVEVIEW_INFO_LEVEL	levelBuf;
	UINT32 levelBufSize = sizeof(LMX_STRUCT_LIVEVIEW_INFO_LEVEL);

	UINT8* jpegDataPos = new UINT8[LMX_DEF_LIVEVIEW_STREAMDATA_SIZE_MAX];
	if (jpegDataPos == NULL) {
		return 0;
	}

	DWORD fps = liveViewDlg->liveviewQualityList.RecomImgQualityListData[liveViewDlg->liveviewQuality].FrameRate;
	DWORD msecPerOneFrame = 1 * 1000 / fps;

	DWORD startTickCount;

	while (liveViewDlg->liveviewRunning) {
		startTickCount = GetTickCount();

		// retrieving LiveView data
		returnedJpegSize = 0;
		UINT32 retError;
		if (LMX_BOOL_FALSE == LMX_func_api_Get_LiveView_data(
			&histgramBuf, &histgramBufSize,
			&postureBuf, &postureBufSize,
			&levelBuf, &levelBufSize,
			jpegDataPos, &returnedJpegSize,
			&retError)) {
			if (LMX_DEF_ERR_COM_DATA_BUSY == retError) {
				SleepEx(5, FALSE);
				continue;
			}
		}

		// Draw Liveview Image
		if (returnedJpegSize > 0) {
			liveViewDlg->DrawLiveviewImage(jpegDataPos, returnedJpegSize);
		}

		DWORD diffTickCount = GetTickCount() - startTickCount;
		if (msecPerOneFrame > diffTickCount) {
			Sleep(msecPerOneFrame - diffTickCount);
		}
	}

	delete[] jpegDataPos;

	return 0;
}



void CLiveView01Dlg::OnBnClickedButtonLiveview()
{
	btnLiveview.EnableWindow(FALSE);

	if (liveviewRunning) {
		// stop Liveview
		btnLiveview.SetWindowText(_T("Start Liveview"));

		LMX_func_api_Ctrl_LiveView_Stop();

		liveviewRunning = FALSE;
	}
	else {
		// start Liveview
		btnLiveview.SetWindowText(_T("Stop Liveview"));

		liveviewRunning = TRUE;
		DWORD threadId;
		HANDLE liveViewThread = CreateThread(NULL, 0, LiveView_ThreadProc, this, 0, &threadId);

		LMX_func_api_Ctrl_LiveView_Start();
	}

	btnLiveview.EnableWindow(TRUE);
}


void CLiveView01Dlg::OnBnClickedButtonSelect()
{
	int curSel = cmboxDeviceList.GetCurSel();
	UINT32 devIndex = (UINT32)cmboxDeviceList.GetItemData(curSel);

	UINT8 ret;
	UINT32 retError;

	// Select Device
	ret = LMX_func_api_Select_PnPDevice(devIndex, &devInfo, &retError);

	// Get liveview quality list
	ret = LMX_func_api_LiveView_Str_Get_Recom_Img(&liveviewQualityList, &retError);

	if (ret != LMX_BOOL_TRUE) {
		return;
	}

	// Get current liveview quality
	LMX_STRUCT_LIVEVIEW_STR_TRANS_IMG curLiveviewQuality;
	ret = LMX_func_api_LiveView_Str_Get_Trans_Img(&curLiveviewQuality, &retError);

	int i;
	for (i = 0; i < 3; i++) {
		if (liveviewQualityList.RecomImgQualityListData[i].Resolution
			== curLiveviewQuality.Resolution) {
			break;
		}
	}
	if (i < 3) {
		liveviewQuality = i;
		UpdateData(FALSE);
	}

	btnLiveview.EnableWindow(TRUE);

	deviceSelected = TRUE;
}
