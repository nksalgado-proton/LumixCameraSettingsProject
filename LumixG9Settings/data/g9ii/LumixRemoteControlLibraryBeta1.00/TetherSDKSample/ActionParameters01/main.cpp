// (C) 2020 Panasonic Corporation

#include <stdio.h>
#include <iostream>
#include <string>

#include "LMX_func_api.h"

// Function Definitions
int SelectDevice(void);
int SendCommand(void);

int ChangeRecordingMode(void);
int PhotoControlCommand(void);
int VideoControlCommand(void);
int AFAELControlCommand(void);
int LensControlCommand(void);
int PowerZoomCommand(void);
int ChangeTargetDevide(void);
int WINAPI NotifyCallbackFunction(UINT32 cb_event_type, UINT32 cb_event_param);

int CopyFileToPC(UINT32 event_type, UINT32 object_handle);



int main(int argc, char* argv[])
{
	UINT8 ret;
	UINT32 retError;

	// Initiaize
	LMX_func_api_Init();

	// Select device
	int devIndex = SelectDevice();
	if (devIndex < 0) {
		return -1;
	}

	// Open session
	UINT32 deviceConnectVer;
	ret = LMX_func_api_Open_Session(0x00010001, &deviceConnectVer);

	// Register callback function
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_ADD, NotifyCallbackFunction);
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER, NotifyCallbackFunction);

	while (1) {
		// Send command
		if (SendCommand() < 0) {
			break;
		}
	}

	// Unregister callback function
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_ADD);
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER);

	// Close session
	ret = LMX_func_api_Close_Session(&retError);

	// Close device
	ret = LMX_func_api_Close_Device(&retError);

	std::cout << std::endl;
	std::cout << "Program end. Push any key." << std::endl;
	getchar();

	return 0;
}

// Callback function
int WINAPI NotifyCallbackFunction(UINT32 cb_event_type, UINT32 cb_event_param)
{
	switch (cb_event_type) {
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER:
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_ADD:
		CopyFileToPC(cb_event_type, cb_event_param);
		break;
	default:
		return -1;
	}

	return 0;
}


int SelectDevice(void)
{
	UINT8 ret;
	UINT32 retError;

	// Get device info
	LMX_CONNECT_DEVICE_INFO devInfo;
	ret = LMX_func_api_Get_PnPDeviceInfo(&devInfo, &retError);

	std::string key;

	std::cout << std::endl;
	std::cout << "Select Device:" << std::endl;
	for (UINT32 i = 0; i < devInfo.find_PnpDevice_Count; i++) {
		std::wcout << "  " << i + 1 << ") "
			<< devInfo.find_PnpDevice_Info[i].dev_ModelName
			<< std::endl;
	}
	std::cout << "  q) quit" << std::endl;
	std::cout << "? ";
	std::cin >> key;
	if (key.c_str()[0] == 'q') {
		return -1;
	}

	int index = std::stoi(key) - 1;

	// Select device
	ret = LMX_func_api_Select_PnPDevice(index, &devInfo, &retError);

	return index;
}

// Send command
int SendCommand(void)
{
	std::string key;

	std::cout << std::endl;
	std::cout << "Select command:" << std::endl;
	std::cout << "  1) Change Recording mode" << std::endl;
	std::cout << "  2) Photo control command" << std::endl;
	std::cout << "  3) Video control command" << std::endl;
	std::cout << "  4) AF/AE lock control command" << std::endl;
	std::cout << "  5) Lens control command" << std::endl;
	std::cout << "  6) Power zoom command" << std::endl;
	std::cout << "  q) quit" << std::endl;
	std::cout << "? ";
	std::cin >> key;
	if (key.c_str()[0] == 'q') {
		return -1;
	}

	int commandNo;
	try {
		commandNo = std::stoi(key);
	}
	catch (std::exception&) {
		return 0;
	}

	int ret = 0;
	switch (commandNo) {
	case 1:
		ret = ChangeRecordingMode();
		break;
	case 2:
		ret = PhotoControlCommand();
		break;
	case 3:
		ret = VideoControlCommand();
		break;
	case 4:
		ret = AFAELControlCommand();
		break;
	case 5:
		ret = LensControlCommand();
		break;
	case 6:
		ret = PowerZoomCommand();
		break;
	}

	return 0;
}

std::string GetStringRecordingMode(UINT32  value)
{
	std::string str = "";

	switch (value) {
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_P:
		str = "P";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_A:
		str = "A";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_S:
		str = "S";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_M:
		str = "M";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_COLOR:
		str = "Color mode";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_MOVREC:
		str = "Video recording mode";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_SCENE:
		str = "Scene";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_AUTO:
		str = "Auto";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_CUSTOM:
		str = "Custom1";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_CUSTOM2:
		str = "Custom2";
		break;
	case Lmx_def_lib_Camera_Mode_Info_Mode_Pos::LMX_DEF_CAMERA_MODE_INFO_MODE_POS_CUSTOM3:
		str = "Custom3";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

int ChangeRecordingMode(void)
{
	do {
		std::string key;

		// Get recording mode capability and current mode
		LMX_STRUCT_RECINFO_CAMERA_MODE_CAPA_INFO capaInfo;
		LMX_func_api_CameraMode_Get_Capability(&capaInfo);

		std::cout << std::endl;
		std::cout << "Change recording mode:" << std::endl;
		std::cout << "  (current: " << GetStringRecordingMode(capaInfo.CurVal_mode_pos) << ")" << std::endl;
		if (capaInfo.Capa_Enum_mode_pos.NumOfVal > 0) {
			for (int i = 0; i < capaInfo.Capa_Enum_mode_pos.NumOfVal; i++) {
				std::cout << "  " << i + 1 << ") " << GetStringRecordingMode(capaInfo.Capa_Enum_mode_pos.SupportVal[i]) << std::endl;
			}
		}
		else {
			std::cout << "  This model does not support to change recording mode." << std::endl;
		}
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int selectNo;
		try {
			selectNo = std::stoi(key);
		}
		catch (std::exception&) {
			selectNo = 0xffffffff;
			continue;
		}

		UINT8 ret = LMX_func_api_CameraMode_Set_Mode_Pos(capaInfo.Capa_Enum_mode_pos.SupportVal[selectNo - 1]);

	} while (1);

	return 0;
}


UINT16 targetDevice;

// Photo control command
int PhotoControlCommand(void)
{
	LMX_func_api_SetupFilesConfig_Get_Target(&targetDevice);

	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select photo control command:" << std::endl;
		std::cout << "  1) Single-shot" << std::endl;
		std::cout << "  2) Change target device to save file" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int commandNo;
		try {
			commandNo = std::stoi(key);
		}
		catch (std::exception&) {
			commandNo = 0xffffffff;
			continue;
		}

		int ret = 0;
		LMX_STRUCT_REC_CTRL lmx_rec_ctrl = { 0 };

		switch (commandNo) {
		case 1:
			// Single-shot
			lmx_rec_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_Release::LMX_DEF_LIB_TAG_REC_CTRL_RELEASE_ONESHOT;
			lmx_rec_ctrl.ParamData.NumOfVal = 0;
			ret = LMX_func_api_Rec_Ctrl_Release(&lmx_rec_ctrl);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute Single-shot command" << std::endl;
			}
			break;
		case 2:
			// Change target device to save file
			ChangeTargetDevide();
			break;
		}
	} while (1);

	return 0;
}

// Video control command
int VideoControlCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select video control command:" << std::endl;
		std::cout << "  1) Start recording movie" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int commandNo;
		try {
			commandNo = std::stoi(key);
		}
		catch (std::exception&) {
			commandNo = 0xffffffff;
			continue;
		}

		int ret = 0;

		switch (commandNo) {
		case 1:
			// Start capture
			ret = LMX_func_api_MoveRec_Ctrl_Start();

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute Start capture command" << std::endl;
				break;
			}
			std::cout << "  e) End recording movie" << std::endl;
			std::cout << std::endl;
			std::cin >> key;
			if (key.c_str()[0] == 'e') {
				// Stop recording movie
				ret = LMX_func_api_MoveRec_Ctrl_Stop(0);
				if (ret != LMX_BOOL_TRUE) {
					std::cout << "Can't execute stop recording movie" << std::endl;
					break;
				}
				break;
			}
			break;
		}
	} while (1);

	return 0;
}

// AF/AE lock control command
int AFAELControlCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select AF/AE lock control command:" << std::endl;
		std::cout << "  1) AE lock" << std::endl;
		std::cout << "  2) AF lock" << std::endl;
		std::cout << "  3) AF/AE lock" << std::endl;
		std::cout << "  4) Oneshot AF" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int commandNo;
		try {
			commandNo = std::stoi(key);
		}
		catch (std::exception&) {
			commandNo = 0xffffffff;
			continue;
		}

		int ret = 0;
		LMX_STRUCT_REC_CTRL lmx_afael_ctrl = { 0 };

		switch (commandNo) {
		case 1:
			// AE Lock
			lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_AE;
			lmx_afael_ctrl.ParamData.NumOfVal = 0;
			ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute AE Lock command" << std::endl;
			}
			std::cout << "  c) Clear Lock" << std::endl;
			std::cout << std::endl;
			std::cin >> key;
			if (key.c_str()[0] == 'c') {
				// End capture
				lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_CLEAR;
				lmx_afael_ctrl.ParamData.NumOfVal = 0;
				ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
				if (ret != LMX_BOOL_TRUE) {
					std::cout << "Can't execute Clear lock command" << std::endl;
					break;
				}
				break;
			}
			break;
		case 2:
			// AF Lock
			lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_AF;
			lmx_afael_ctrl.ParamData.NumOfVal = 0;
			ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute AF Lock command" << std::endl;
			}
			std::cout << "  c) Clear Lock" << std::endl;
			std::cout << std::endl;
			std::cin >> key;
			if (key.c_str()[0] == 'c') {
				// End capture
				lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_CLEAR;
				lmx_afael_ctrl.ParamData.NumOfVal = 0;
				ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
				if (ret != LMX_BOOL_TRUE) {
					std::cout << "Can't execute Clear lock command" << std::endl;
					break;
				}
				break;
			}
			break;
		case 3:
			// AF/AE Lock
			lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_AFAE;
			lmx_afael_ctrl.ParamData.NumOfVal = 0;
			ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute AF/AE Lock command" << std::endl;
			}
			std::cout << "  c) Clear Lock" << std::endl;
			std::cout << std::endl;
			std::cin >> key;
			if (key.c_str()[0] == 'c') {
				// End capture
				lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_LOCK_CLEAR;
				lmx_afael_ctrl.ParamData.NumOfVal = 0;
				ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
				if (ret != LMX_BOOL_TRUE) {
					std::cout << "Can't execute Clear lock command" << std::endl;
					break;
				}
				break;
			}
			break;
		case 4:
			// Oneshot AF
			lmx_afael_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_AFAE::LMX_DEF_LIB_TAG_REC_CTRL_AFAE_AF_ONESHOT;
			lmx_afael_ctrl.ParamData.NumOfVal = 0;
			ret = LMX_func_api_Rec_Ctrl_AF_AE(&lmx_afael_ctrl);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute Oneshot AF command" << std::endl;
			}
			break;
		}
	} while (1);
	return 0;
}

// Lens control command
int LensControlCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select lens control command:" << std::endl;
		std::cout << "  1) MF bar control request (Stop)" << std::endl;
		std::cout << "  2) MF bar control request (Far Fast)" << std::endl;
		std::cout << "  3) MF bar control request (Far Slow)" << std::endl;
		std::cout << "  4) MF bar control request (Near Slow)" << std::endl;
		std::cout << "  5) MF bar control request (Near Fast)" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int commandNo;
		try {
			commandNo = std::stoi(key);
		}
		catch (std::exception&) {
			commandNo = 0xffffffff;
			continue;
		}

		int ret = 0;
		LMX_STRUCT_REC_CTRL lmx_lens_ctrl = { 0 };

		// MF bar control request
		lmx_lens_ctrl.CtrlID = Lmx_TagID_Rec_Ctrl_Lens::LMX_DEF_LIB_TAG_REC_CTRL_LENS_MF_BAR;
		lmx_lens_ctrl.ParamData.NumOfVal = 1;

		switch (commandNo) {
		case 1:	// Stop
			lmx_lens_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_LENS_MF_PINTO_ADJUST_STOP;
			break;
		case 2:	// Far Fast
			lmx_lens_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_LENS_MF_PINTO_ADJUST_FAR_FAST;
			break;
		case 3:	// Far Slow
			lmx_lens_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_LENS_MF_PINTO_ADJUST_FAR_SLOW;
			break;
		case 4:	// Near Slow
			lmx_lens_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_LENS_MF_PINTO_ADJUST_NEAR_SLOW;
			break;
		case 5:	// Near Fast
			lmx_lens_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_LENS_MF_PINTO_ADJUST_NEAR_FAST;
			break;
		}
		ret = LMX_func_api_Rec_Ctrl_Lens(&lmx_lens_ctrl);

		if (ret == LMX_BOOL_FALSE) {
			std::cout << "Can't execute MF bar control request command" << std::endl;
			break;
		}
	} while (1);

	return 0;
}


// Power Zoom Command
int PowerZoomCommand(void)
{
	bool zooom_running = false;

	do {
		std::string key;

		int ret = 0;
		LMX_STRUCT_REC_CTRL lmx_zoom_ctrl = { 0 };

		if (!zooom_running) {
		std::cout << std::endl;
		std::cout << "Select Power zoom command:" << std::endl;
			std::cout << "  1) Power zooml request (Tele Fast)" << std::endl;
			std::cout << "  2) Power zoom request (Tele Slow)" << std::endl;
			std::cout << "  3) Power zoom request (Wide Slow)" << std::endl;
			std::cout << "  4) Power zoom request (Wide Fast)" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int commandNo;
		try {
			commandNo = std::stoi(key);
		}
		catch (std::exception&) {
			commandNo = 0xffffffff;
			continue;
		}

			lmx_zoom_ctrl.CtrlID = LMX_DEF_LIB_TAG_REC_CTRL_ZOOM_START_REQ;
			lmx_zoom_ctrl.ParamData.NumOfVal = 2;

		// Power zoom request
		switch (commandNo) {
			case 1:	// Tele Fast
			lmx_zoom_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_ZOOM_DIR_TELE;
			lmx_zoom_ctrl.ParamData.SupportVal[1] = LMX_DEF_REC_CTRL_ZOOM_SPEED_HIGH;
			break;
			case 2:	// Tele Slow
			lmx_zoom_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_ZOOM_DIR_TELE;
			lmx_zoom_ctrl.ParamData.SupportVal[1] = LMX_DEF_REC_CTRL_ZOOM_SPEED_LOW;
			break;
			case 3:	// Wide Slow
			lmx_zoom_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_ZOOM_DIR_WIDE;
			lmx_zoom_ctrl.ParamData.SupportVal[1] = LMX_DEF_REC_CTRL_ZOOM_SPEED_LOW;
			break;
			case 4:	// Wide Fast
			lmx_zoom_ctrl.ParamData.SupportVal[0] = LMX_DEF_REC_CTRL_ZOOM_DIR_WIDE;
			lmx_zoom_ctrl.ParamData.SupportVal[1] = LMX_DEF_REC_CTRL_ZOOM_SPEED_HIGH;
			break;
		}
		}
		else {
			std::cout << std::endl;
			std::cout << "Press Enter key to stop zooming.";
			std::cin.ignore();
			int a = getchar();

			lmx_zoom_ctrl.CtrlID = LMX_DEF_LIB_TAG_REC_CTRL_ZOOM_STOP_REQ;
			lmx_zoom_ctrl.ParamData.NumOfVal = 0;
		}

		ret = LMX_func_api_Rec_Ctrl_Lens(&lmx_zoom_ctrl);

		if (ret == LMX_BOOL_FALSE) {
			std::cout << "Can't execute Power zoom request command" << std::endl;
			break;
		}

		zooom_running = !zooom_running;
	} while (1);

	return 0;
}

// Change target device to save file
int ChangeTargetDevide(void)
{
	do {
		std::string key;

		std::string targetDeviceName;
		LMX_func_api_SetupFilesConfig_Get_Target(&targetDevice);
		switch (targetDevice) {
		case Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_ONLY_CAMERA:
			targetDeviceName = "Camera";
			break;
		case Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_ONLY_PC:
			targetDeviceName = "PC";
			break;
		case Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_PC_AND_CAMERA:
			targetDeviceName = "Both Camera and PC";
			break;
		default:
			targetDeviceName = "Unknown";
			break;
		}

		std::cout << std::endl;
		std::cout << "Change target device to save file:" << std::endl;
		std::cout << "  (current: " << targetDeviceName << ")" << std::endl;
		std::cout << "  1) Camera" << std::endl;
		std::cout << "  2) PC" << std::endl;
		std::cout << "  3) Both Camera and PC" << std::endl;
		std::cout << "  b) back to previous menu" << std::endl;
		std::cout << "? ";
		std::cin >> key;
		if (key.c_str()[0] == 'b') {
			break;
		}

		int selectNo;
		try {
			selectNo = std::stoi(key);
		}
		catch (std::exception&) {
			selectNo = 0xffffffff;
			continue;
		}

		switch (selectNo) {
		case 1:	// Camera
			targetDevice = Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_ONLY_CAMERA;
			break;
		case 2:	// PC
			targetDevice = Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_ONLY_PC;
			break;
		case 3:	// Both Camera and PC
			targetDevice = Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_PC_AND_CAMERA;
			break;
		}
		LMX_func_api_SetupFilesConfig_Set_Target(targetDevice);
	} while (1);

	return 0;
}


int CopyFileToPC(UINT32 event_type, UINT32 object_handle)
{
	UINT32 retError;
	UINT32 formatType;
	UINT64 dataSize;
	LMX_STRUCT_PTP_ARRAY_STRING fileNameArrStr;
	std::string fileName;

	LMX_func_api_Get_Object_FormatType(object_handle, &formatType, &retError);
	LMX_func_api_Get_Object_DataSize(object_handle, &dataSize, &retError);
	LMX_func_api_Get_Object_FileName(object_handle, &fileNameArrStr, &retError);
	fileName.assign((const char*)fileNameArrStr.StringChars, (std::string::size_type)fileNameArrStr.NumChars - 1);

	if (formatType == Lmx_def_lib_object_format::LMX_DEF_OBJ_FORMAT_JPEG
		|| formatType == Lmx_def_lib_object_format::LMX_DEF_OBJ_FORMAT_RAW) {
		// Copy stillimage file to PC
		if (targetDevice == Lmx_def_lib_setup_cfg_filesetting_target_cfg::LMX_DEF_SETUP_CFG_FILE_TARGET_CFG_ONLY_CAMERA) {
			// not copy file to pc
			return 2;
		}

		if (event_type != Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER) {
			// copy file only when LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER occured
			return 2;
		}
	}
	else if (formatType == Lmx_def_lib_object_format::LMX_DEF_OBJ_FORMAT_MOVIE_MP4
		|| formatType == Lmx_def_lib_object_format::LMX_DEF_OBJ_FORMAT_MOVIE_MOV) {
		// Copy movie file to PC

	}
	else {
		return 1;	// not target file
	}

#define MAX_DATA_BUF_SIZE (10 * 1024 * 1024)
	UINT64 dataBufSize = min((UINT64)MAX_DATA_BUF_SIZE, dataSize);
	UINT8* dataBuf = new UINT8[dataBufSize];
	if (dataBuf == NULL) {
		if (event_type == Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER) {
			LMX_func_api_Skip_Object_Transfer(object_handle);
		}
		std::cout << std::endl;
		std::cout << "Failed to copy file: " << fileName << std::endl;

		return -1;
	}

#if defined(LMX_PLAT_WINDOWS)
#define FILE_OPEN_MODE "wb"
#elif defined(LMX_PLAT_MACOS)
#define FILE_OPEN_MODE "w"
#endif

	// create file
	FILE* fp = fopen(fileName.c_str(), FILE_OPEN_MODE);
	if (fp == NULL) {
		if (event_type == Lmx_event_id::LMX_DEF_LIB_EVENT_ID_OBJCT_REQ_TRNSFER) {
			LMX_func_api_Skip_Object_Transfer(object_handle);
		}
		std::cout << std::endl;
		std::cout << "Failed to copy file: " << fileName << std::endl;
		
		return -2;
	}
	
	std::cout << std::endl;
	std::cout << "Now copying file \"" << fileName << "\"(" << dataSize << "byte)";

	UINT64 dataOffset = 0;
	UINT64 remainDataSize = dataSize;
	while (remainDataSize > 0) {
		UINT64 copySize = min(dataBufSize, remainDataSize);
		LMX_func_api_Get_Partial_ObjectEx(object_handle, dataBuf, dataOffset, (UINT32)copySize);
		copySize = fwrite(dataBuf, 1, copySize, fp);

		dataOffset += copySize;
		remainDataSize -= copySize;
		std::cout << ".";
	}

	fclose(fp);

	delete[] dataBuf;

	std::cout << "Done." << std::endl;

	return 0;
}
