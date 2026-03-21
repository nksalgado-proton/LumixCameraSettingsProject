// (C) 2020 Panasonic Corporation

#include <iostream>
#include <string>

#include "LMX_func_api.h"


// Function Definitions
int SelectDevice(void);
int SendCommand(void);

std::string GetStringISO(UINT32 value);
std::string GetStringSS(UINT32 value);
std::string GetStringAperture(UINT16 value);
std::string GetStringWB(UINT16 value);
std::string GetStringExposure(UINT16 value);

// Parameter Definitions
UINT32 curIsoValue = 0;
UINT32 curSsValue = 0;
UINT32 curApetureValue = 0;
UINT32 curWbValue = 0;
UINT32 curExposureCompValue = 0;



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

	while (1) {
		// Send command
		if (SendCommand() < 0) {
			break;
		}
	}

	// Close session
	ret = LMX_func_api_Close_Session(&retError);

	// Close device
	ret = LMX_func_api_Close_Device(&retError);


	std::cout << std::endl;
	std::cout << "Program end. Push any key." << std::endl;
	getchar();

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
	std::cout << "Select device:" << std::endl;
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

	// Select Device
	ret = LMX_func_api_Select_PnPDevice(index, &devInfo, &retError);

	return index;
}

// Get capability command
int GetCapabilityCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select get capability command:" << std::endl;
		std::cout << "  1) Get ISO capability command" << std::endl;
		std::cout << "  2) Get shutter speed capability command" << std::endl;
		std::cout << "  3) Get aperture capability command" << std::endl;
		std::cout << "  4) Get white balance capability command" << std::endl;
		std::cout << "  5) Get exposure compensation capability command" << std::endl;
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
			//Get ISO capability
			LMX_STRUCT_ISO_CAPA_INFO Iso_CapaInfo;
			ret = LMX_func_api_ISO_Get_Capability(&Iso_CapaInfo);

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get ISO capability command" << std::endl;
				return ret;
			}
			std::cout << "Iso_CapaInfo.Capa_Enum.NumOfVal = " << Iso_CapaInfo.Capa_Enum.NumOfVal << std::endl;
			for (int i = 0; i < Iso_CapaInfo.Capa_Enum.NumOfVal; i++) {
				std::cout << "Iso_CapaInfo.Capa_Enum.SupportVal[" << i << "]=" << GetStringISO(Iso_CapaInfo.Capa_Enum.SupportVal[i]) << std::endl;
			}
			std::cout << "Iso_CapaInfo.Capa_Range.MaxVal = " << GetStringISO(Iso_CapaInfo.Capa_Range.MaxVal) << std::endl;
			std::cout << "Iso_CapaInfo.CurVal = " << GetStringISO(Iso_CapaInfo.CurVal) << std::endl;
			break;
		case 2:
			// Get shutter speed capability
			LMX_STRUCT_SS_CAPA_INFO SS_CapaInfo;

			ret = LMX_func_api_SS_Get_Capability(&SS_CapaInfo);

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get shutter speed capability command" << std::endl;
				return ret;
			}

			std::cout << "SS_CapaInfo.Capa_Enum.NumOfVal = " << SS_CapaInfo.Capa_Enum.NumOfVal << std::endl;
			for (int i = 0; i < SS_CapaInfo.Capa_Enum.NumOfVal; i++) {
				std::cout << "SS_CapaInfo.Capa_Enum.SupportVal[" << i << "]=" << GetStringSS(SS_CapaInfo.Capa_Enum.SupportVal[i]) << std::endl;
			}
			std::cout << "SS_CapaInfo.CurVal_Range.MinVal = " << GetStringSS(SS_CapaInfo.CurVal_Range.MinVal) << std::endl;
			std::cout << "SS_CapaInfo.CurVal_Range.MaxVal = " << GetStringSS(SS_CapaInfo.CurVal_Range.MaxVal) << std::endl;
			std::cout << "SS_CapaInfo.CurVal = " << GetStringSS(SS_CapaInfo.CurVal) << std::endl;
			break;
		case 3:
			// Get aperture capability
			LMX_STRUCT_APERTURE_CAPA_INFO Aperture_CapaInfo;

			ret = LMX_func_api_Aperture_Get_Capability(&Aperture_CapaInfo);

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get aperture capability command" << std::endl;
				return ret;
			}

			std::cout << "Aperture_CapaInfo.Capa_Enum.NumOfVal = " << Aperture_CapaInfo.Capa_Enum.NumOfVal << std::endl;
			for (int i = 0; i < Aperture_CapaInfo.Capa_Enum.NumOfVal; i++) {
				std::cout << "Aperture_CapaInfo.Capa_Enum.SupportVal[" << i << "]=" << GetStringAperture(Aperture_CapaInfo.Capa_Enum.SupportVal[i]) << std::endl;
			}
			std::cout << "Aperture_CapaInfo.CurVal_Range.MinVal = " << GetStringAperture(Aperture_CapaInfo.CurVal_Range.MinVal) << std::endl;
			std::cout << "Aperture_CapaInfo.CurVal_Range.MaxVal = " << GetStringAperture(Aperture_CapaInfo.CurVal_Range.MaxVal) << std::endl;
			std::cout << "Aperture_CapaInfo.CurVal = " << GetStringAperture(Aperture_CapaInfo.CurVal) << std::endl;
			break;
		case 4:
			// Get white balance capability
			LMX_STRUCT_WB_CAPA_INFO WB_CapaInfo;

			ret = LMX_func_api_WB_Get_Capability(&WB_CapaInfo);

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get white balance capability command" << std::endl;
				return ret;
			}

			std::cout << "WBCapaInfo.Capa_Enum.NumOfVal = " << WB_CapaInfo.Capa_Enum_WB.NumOfVal << std::endl;
			for (int i = 0; i < WB_CapaInfo.Capa_Enum_WB.NumOfVal; i++) {
				std::cout << "WB_CapaInfo.Capa_Enum.SupportVal[" << i << "]=" << GetStringWB(WB_CapaInfo.Capa_Enum_WB.SupportVal[i]) << std::endl;
			}
			std::cout << "WB_CapaInfo.CurVal = " << GetStringWB(WB_CapaInfo.CurVal_WB) << std::endl;
			break;
		case 5:
			// Get exposure compensation capability
			LMX_STRUCT_EXPOSURE_CAPA_INFO Exposure_CapaInfo;

			ret = LMX_func_api_Exposure_Get_Capability(&Exposure_CapaInfo);

			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute getexposure compensation capability command" << std::endl;
				return ret;
			}

			std::cout << "Exposure_CapaInfo.Capa_Enum.NumOfVal = " << Exposure_CapaInfo.Capa_Enum.NumOfVal << std::endl;
			for (int i = 0; i < Exposure_CapaInfo.Capa_Enum.NumOfVal; i++) {
				std::cout << "Exposure_CapaInfo.Capa_Enum.SupportVal[" << i << "]=" << GetStringExposure(Exposure_CapaInfo.Capa_Enum.SupportVal[i]) << std::endl;
			}
			std::cout << "Exposure_CapaInfo.CurVal_Range.MinVal = " << GetStringExposure(Exposure_CapaInfo.CurVal_Range.MinVal) << std::endl;
			std::cout << "Exposure_CapaInfo.CurVal_Range.MaxVal = " << GetStringExposure(Exposure_CapaInfo.CurVal_Range.MaxVal) << std::endl;
			std::cout << "Exposure_CapaInfo.CurVal = " << GetStringExposure(Exposure_CapaInfo.CurVal) << std::endl;

			break;
		}
	} while (1);

	return 0;
}


// Get Command
int GetCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select get command:" << std::endl;
		std::cout << "  1) Get current ISO" << std::endl;
		std::cout << "  2) Get current shutter speed" << std::endl;
		std::cout << "  3) Get current apature" << std::endl;
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
		UINT32 param;
		UINT32 retError;
		switch (commandNo) {
		case 1:
			// Get ISO
			param = 0;
			ret = LMX_func_api_ISO_Get_Param(&param, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get ISO command" << std::endl;
				return ret;
			}
			std::cout << "Current ISO is ";
			std::cout << GetStringISO(param) << std::endl;
			break;
		case 2:
			// Get shutter speed			
			param = 0;
			ret = LMX_func_api_SS_Get_Param(&param, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get shutter speed command" << std::endl;
				return ret;
			}
			std::cout << "Current shutter speed is ";
			std::cout << GetStringSS(param) << std::endl;
			break;
		case 3:
			// Get apature
			param = 0;
			ret = LMX_func_api_Aperture_Get_Param(&param, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute get aperture command" << std::endl;
				return ret;
			}
			std::cout << "Current apature is ";
			std::cout << GetStringAperture(param) << std::endl;
			break;
		}
	} while (1);

	return 0;
}


// Set command
int SetCommand(void)
{
	do {
		std::string key;

		std::cout << std::endl;
		std::cout << "Select set command:" << std::endl;
		std::cout << "  1) Set ISO" << std::endl;
		std::cout << "  2) Set shutter speed" << std::endl;
		std::cout << "  3) Set apature" << std::endl;
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
			return 0;
		}
		
		key = "";

		int ret = 0;
		UINT32 value;
		UINT32 retError;
		switch (commandNo) {
		case 1:
			// Set ISO
			std::cout << "Input ISO value" << std::endl;
			std::cin >> key;
			value = std::stoi(key);
			ret = LMX_func_api_ISO_Set_Param(value, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute set ISO command" << std::endl;
				return ret;
			}
			std::cout << "ISO is ";
			std::cout << GetStringISO(value) << std::endl;
			break;
		case 2:
			// Set shutter speed			
			std::cout << "Input shutter speed value (Hex)" << std::endl;
			std::cin >> key;
			value = std::stoul(key, nullptr, 16);
			ret = LMX_func_api_SS_Set_Param(value, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute set shutter speed command" << std::endl;
				return ret;
			}
			std::cout << "Shutter speed is ";
			std::cout << GetStringSS(value) << std::endl;
			break;
		case 3:
			// Set apature
			std::cout << "Input aperture value" << std::endl;
			std::cin >> key;
			value = std::stoi(key);
			ret = LMX_func_api_Aperture_Set_Param(value, &retError);
			if (ret == LMX_BOOL_FALSE) {
				std::cout << "Can't execute set aperture command" << std::endl;
				return ret;
			}
			std::cout << "Apature is ";
			std::cout << GetStringAperture(value) << std::endl;
			break;
		}
	} while (1);

	return 0;
}

// Callback function
int WINAPI NotifyCallbackFunction(UINT32 cb_event_type, UINT32 cb_event_param)
{
	UINT32 retError;
	UINT32 param;
	std::string str;

	switch (cb_event_type) {
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_ISO:
		LMX_func_api_ISO_Get_Param(&param, &retError);
		str = "ISO: " + GetStringISO(curIsoValue) + " -> " + GetStringISO(param);
		curIsoValue = param;
		break;
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_SHUTTER:
		LMX_func_api_SS_Get_Param(&param, &retError);
		str = "SS: " + GetStringSS(curSsValue) + " -> " + GetStringSS(param);
		curSsValue = param;
		break;
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_APERTURE:
		LMX_func_api_Aperture_Get_Param(&param, &retError);
		str = "Aperture: " + GetStringAperture((UINT16)curApetureValue) + " -> " + GetStringAperture((UINT16)param);
		curApetureValue = param;
		break;
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_WHITEBALANCE:
		LMX_func_api_WB_Get_Param(&param, &retError);
		str = "White balance: " + GetStringWB((UINT16)curWbValue) + " -> " + GetStringWB((UINT16)param);
		curWbValue = param;
		break;
	case Lmx_event_id::LMX_DEF_LIB_EVENT_ID_EXPOSURE:
		LMX_func_api_Exposure_Get_Param(&param, &retError);
		str = "Exposure comp.: " + GetStringExposure((UINT16)curExposureCompValue) + " -> " + GetStringExposure((UINT16)param);
		curExposureCompValue = param;
		break;
	default:
		return -1;
	}

	std::cout << std::endl;
	std::cout << str << std::endl;

	return 0;
}


// Start Monitoring parameteres
int StartMonitoringParameters()
{
	UINT32 retError;

	// Get current parameter values
	LMX_func_api_ISO_Get_Param(&curIsoValue, &retError);
	LMX_func_api_SS_Get_Param(&curSsValue, &retError);
	LMX_func_api_Aperture_Get_Param(&curApetureValue, &retError);
	LMX_func_api_WB_Get_Param(&curWbValue, &retError);
	LMX_func_api_Exposure_Get_Param(&curExposureCompValue, &retError);

	// Register callback function
	UINT32 ret;
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_ISO, NotifyCallbackFunction);
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_SHUTTER, NotifyCallbackFunction);
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_APERTURE, NotifyCallbackFunction);
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_WHITEBALANCE, NotifyCallbackFunction);
	ret = LMX_func_api_Reg_NotifyCallback(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_EXPOSURE, NotifyCallbackFunction);

	return 0;
}

// Stop Monitoring parameteres
int StopMonitoringParameters()
{
	// Unregister callback function
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_ISO);
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_SHUTTER);
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_APERTURE);
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_WHITEBALANCE);
	LMX_func_api_Delete_CallBackInfo(Lmx_event_id::LMX_DEF_LIB_EVENT_ID_EXPOSURE);

	return 0;
}


// Send command
int SendCommand(void)
{
	std::string key;

	std::cout << std::endl;
	std::cout << "Select command:" << std::endl;
	std::cout << "  1) Get capability command" << std::endl;
	std::cout << "  2) Get command" << std::endl;
	std::cout << "  3) Set command" << std::endl;
	std::cout << "  4) Start Monitoring parameteres" << std::endl;
	std::cout << "  5) Stop Monitoring parameteres" << std::endl;
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
		ret = GetCapabilityCommand();
		break;
	case 2:
		ret = GetCommand();
		break;
	case 3:
		ret = SetCommand();
		break;
	case 4:
		ret = StartMonitoringParameters();
		break;
	case 5:
		ret = StopMonitoringParameters();
		break;
	}

	return 0;
}

std::string GetStringISO(UINT32 value)
{
	std::string str = "";

	switch (value) {
	case Lmx_def_lib_ISO_param::LMX_DEF_ISO_AUTO:
		str = "Auto";
		break;
	case Lmx_def_lib_ISO_param::LMX_DEF_ISO_I_ISO:
		str = "i-ISO";
		break;
	case Lmx_def_lib_ISO_param::LMX_DEF_ISO_UNKNOWN:
		str = "Unknown";
		break;
	default:
		str = std::to_string(value & 0x0FFFFFFF);
		break;
	}

	return str;
}

std::string GetStringSS(UINT32 value)
{
	std::string str = "";

	switch (value) {
	case Lmx_def_lib_DevpropEx_ShutterSpeed_param::LMX_DEF_PTP_DEVPROP_EXT_LMX_SS_BULB:
		str = "Bulb";
		break;
	case Lmx_def_lib_DevpropEx_ShutterSpeed_param::LMX_DEF_PTP_DEVPROP_EXT_LMX_SS_AUTO:
		str = "Auto";
		break;
	case Lmx_def_lib_DevpropEx_ShutterSpeed_param::LMX_DEF_PTP_DEVPROP_EXT_LMX_SS_UNKNOWN:
		str = "Unknown";
		break;
	default:
		if ((value & 0x80000000) == 0x00000000) {
			str = "1/" + std::to_string(value / 1000);
		}
		else {
			value = (value &= 0x7fffffff);
			str = std::to_string(value / 1000);
		}
		str += " sec";
		break;
	}

	return str;
}


std::string GetStringAperture(UINT16 value)
{
	std::string str = "";

	switch (value) {
	case Lmx_def_lib_Aperture_param::LMX_DEF_F_UNKNOWN:
		str = "Unknown";
		break;
	case Lmx_def_lib_Aperture_param::LMX_DEF_F_AUTO:
		str = "Auto";
		break;
	default:
		str = "F" + std::to_string(value / 10) + "." + std::to_string(value % 10);
		break;
	}

	return str;
}

std::string GetStringWB(UINT16 value)
{
	std::string str = "";

	switch (value) {
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_AUTO:
		str = "Auto";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_DAYLIGHT:
			str = "Day light";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_CLOUD:
		str = "Cloud";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_TENGSTEN:
		str = "Incandescent";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_WHITESET:
		str = "White set";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_FLASH:
		str = "Flash";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_FLUORESCENT:
		str = "Flourescent";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_BLACK_WHITE:
		str = "Black white";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_KEEP:
		str = "WB setting 1";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_KEEP2:
		str = "WB setting 2";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_KEEP3:
		str = "WB setting 3";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_KEEP4:
		str = "WB setting 4";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_SHADE:
		str = "Shade";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_K_SET:
		str = "Color temperature";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_K_SET2:
		str = "Color temperature 2";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_K_SET3:
		str = "Color temperature 3";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_K_SET4:
		str = "Color temperature 4";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_AUTO_COOL:
		str = "Auto cool";
		break;
	case Lmx_def_lib_WhiteBalance_param::LMX_DEF_WB_AUTO_WARM:
		str = "Auto warm";
		break;
	default:
		str = "Unknown";
		break;
	}

	return str;
}

std::string GetStringExposure(UINT16 value)
{
	std::string str = "";
	
	if (value == 0x0000) {
		str = "0 EV";
		return str;
	}
	
	if ((value & 0x8000) == 0x0000) {
		str = "+";
	}
	else {
		str = "-";
		value &= 0x7fff;
	}

	str += std::to_string(value) + "/3 EV";

	return str;
}
