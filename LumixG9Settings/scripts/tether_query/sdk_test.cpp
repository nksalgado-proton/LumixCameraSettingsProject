/**
 * sdk_test — Connect to Lumix G9 II via official SDK DLL (Lmxptpif.dll)
 *
 * Uses the exact SDK headers and .lib for proper struct/function definitions.
 * If this works, PTP is functional and the Tether DLL issue is separate.
 */

// SDK headers define everything we need
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>
#include "LMX_func_api.h"

int WINAPI myCallback(UINT32 event, UINT32 param) {
    printf("  [SDK CALLBACK] event=0x%08x param=0x%08x\n", event, param);
    return 0;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        printf("COM init failed: 0x%08lx, trying MTA...\n", hr);
        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            printf("COM MTA also failed: 0x%08lx\n", hr);
            // Try without COM — some functions may still work
        }
    }
    printf("COM init: 0x%08lx\n", hr);

    printf("sizeof(LMX_DEV_INFO) = %zu\n", sizeof(LMX_DEV_INFO));
    printf("sizeof(LMX_CONNECT_DEVICE_INFO) = %zu\n", sizeof(LMX_CONNECT_DEVICE_INFO));

    // 1. Init
    printf("\n1. Init\n");
    LMX_func_api_Init();

    // 2. Register callback (correct SDK signature: type, func)
    printf("2. Register callback\n");
    UINT32 cbRet = LMX_func_api_Reg_NotifyCallback(0, myCallback);
    printf("   Reg_NotifyCallback(0, cb) = %u\n", cbRet);
    cbRet = LMX_func_api_Reg_NotifyCallback(1, myCallback);
    printf("   Reg_NotifyCallback(1, cb) = %u\n", cbRet);

    // 3. Get PnP Device Info
    printf("\n3. GetPnPDeviceInfo\n");
    auto* devInfo = new LMX_CONNECT_DEVICE_INFO();
    memset(devInfo, 0, sizeof(LMX_CONNECT_DEVICE_INFO));
    UINT32 retError = 0;

    UINT8 ret = LMX_func_api_Get_PnPDeviceInfo(devInfo, &retError);
    printf("   result=%d, error=0x%08x\n", ret, retError);
    printf("   Devices found: %u\n", devInfo->find_PnpDevice_Count);

    if (devInfo->find_PnpDevice_Count == 0) {
        printf("\n   No camera found!\n");
        printf("   - Camera connected via USB?\n");
        printf("   - LUMIX Tether closed?\n");
        printf("   - USB Mode = PC (Tether)?\n");
        delete devInfo;
        CoUninitialize();
        return 1;
    }

    for (UINT32 i = 0; i < devInfo->find_PnpDevice_Count; i++) {
        wprintf(L"   Device %u: maker='%s' model='%s'\n",
                i,
                devInfo->find_PnpDevice_Info[i].dev_MakerName,
                devInfo->find_PnpDevice_Info[i].dev_ModelName);
        if (devInfo->find_PnpDevice_IDs[i])
            wprintf(L"   ID: %s\n", devInfo->find_PnpDevice_IDs[i]);
    }

    // 4. Select PnP Device
    printf("\n4. SelectPnPDevice(0)\n");
    retError = 0;
    ret = LMX_func_api_Select_PnPDevice(0, devInfo, &retError);
    printf("   result=%d, error=0x%08x\n", ret, retError);

    if (ret != LMX_BOOL_TRUE) {
        printf("   SelectPnPDevice failed!\n");
        delete devInfo;
        CoUninitialize();
        return 1;
    }

    // 5. Open Session
    printf("\n5. OpenSession\n");
    UINT32 deviceVer = 0;
    retError = 0;
    ret = LMX_func_api_Open_Session(0x00010001, &deviceVer, &retError);
    printf("   result=%d, deviceVer=0x%08x, error=0x%08x\n", ret, deviceVer, retError);

    if (ret != LMX_BOOL_TRUE) {
        printf("   OpenSession failed!\n");
        printf("   Trying with session=1...\n");
        deviceVer = 0; retError = 0;
        ret = LMX_func_api_Open_Session(1, &deviceVer, &retError);
        printf("   result=%d, deviceVer=0x%08x, error=0x%08x\n", ret, deviceVer, retError);
    }

    if (ret == LMX_BOOL_TRUE) {
        printf("\n   *** SESSION OPEN! ***\n");

        // 6. Get camera mode
        printf("\n6. Query camera info\n");
        UINT32 modePos = 0;
        retError = 0;
        ret = LMX_func_api_CameraMode_Get_Mode_Pos(&modePos, &retError);
        printf("   Mode position: result=%d, mode=0x%x, err=0x%x\n", ret, modePos, retError);

        UINT32 isoParam = 0;
        retError = 0;
        ret = LMX_func_api_ISO_Get_Param(&isoParam, &retError);
        printf("   ISO: result=%d, iso=0x%x, err=0x%x\n", ret, isoParam, retError);

        UINT32 ssParam = 0;
        retError = 0;
        ret = LMX_func_api_SS_Get_Param(&ssParam, &retError);
        printf("   Shutter: result=%d, ss=0x%x, err=0x%x\n", ret, ssParam, retError);

        UINT32 apertureParam = 0;
        retError = 0;
        ret = LMX_func_api_Aperture_Get_Param(&apertureParam, &retError);
        printf("   Aperture: result=%d, ap=0x%x, err=0x%x\n", ret, apertureParam, retError);

        UINT32 wbParam = 0;
        retError = 0;
        ret = LMX_func_api_WB_Get_Param(&wbParam, &retError);
        printf("   WB: result=%d, wb=0x%x, err=0x%x\n", ret, wbParam, retError);

        UINT32 afMode = 0;
        retError = 0;
        ret = LMX_func_api_AF_Config_Get_AF_Mode_Param(&afMode, &retError);
        printf("   AF Mode: result=%d, af=0x%x, err=0x%x\n", ret, afMode, retError);

        UINT32 driveMode = 0;
        retError = 0;
        ret = LMX_func_api_CameraMode_Get_DriveMode(&driveMode, &retError);
        printf("   Drive Mode: result=%d, drive=0x%x, err=0x%x\n", ret, driveMode, retError);

        // 6b. Try Tether DLL functions while SDK session is open
        printf("\n6b. Load Tether DLL for Fn_Assign\n");
        AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        HMODULE tetherDll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
        if (tetherDll) {
            printf("   Tether DLL loaded.\n");
            auto pGetFnAssign = GetProcAddress(tetherDll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
            auto pGetSetupConfigInfo = GetProcAddress(tetherDll, "LMX_func_api_GetSetupConfigInfo");
            auto pGetDevInfo = GetProcAddress(tetherDll, "LMX_func_api_GetDeviceInfo");

            printf("   FnAssign=%p SetupConfig=%p DevInfo=%p\n",
                   (void*)pGetFnAssign, (void*)pGetSetupConfigInfo, (void*)pGetDevInfo);

            if (pGetFnAssign) {
                printf("   Calling GetFnAssign...\n");
                uint8_t fnBuf[8192];
                memset(fnBuf, 0, sizeof(fnBuf));
                UINT32 fnErr = 0;
                __try {
                    UINT8 fnRet = ((UINT8(WINAPI*)(void*, UINT32*))pGetFnAssign)(fnBuf, &fnErr);
                    printf("   result=%d, err=0x%x\n", fnRet, fnErr);
                    if (fnRet == 1) {
                        // Find non-zero data
                        size_t last = 0;
                        for (size_t i = 0; i < sizeof(fnBuf); i++)
                            if (fnBuf[i]) last = i;
                        printf("   Fn data (%zu bytes):\n", last + 1);
                        for (size_t i = 0; i < last + 32 && i < sizeof(fnBuf); i += 16) {
                            printf("   %04zx: ", i);
                            for (size_t j = 0; j < 16 && i+j < sizeof(fnBuf); j++)
                                printf("%02x ", fnBuf[i+j]);
                            printf("\n");
                        }
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
                }
            }

            if (pGetSetupConfigInfo) {
                printf("   Calling GetSetupConfigInfo...\n");
                uint8_t cfgBuf[65536];
                memset(cfgBuf, 0, sizeof(cfgBuf));
                UINT32 cfgErr = 0;
                __try {
                    UINT8 cfgRet = ((UINT8(WINAPI*)(void*, UINT32*))pGetSetupConfigInfo)(cfgBuf, &cfgErr);
                    printf("   result=%d, err=0x%x\n", cfgRet, cfgErr);
                    if (cfgRet == 1) {
                        size_t last = 0;
                        for (size_t i = 0; i < sizeof(cfgBuf); i++)
                            if (cfgBuf[i]) last = i;
                        printf("   Config data (%zu bytes):\n", last + 1);
                        for (size_t i = 0; i < last + 32 && i < sizeof(cfgBuf); i += 16) {
                            bool hasData = false;
                            for (size_t j = 0; j < 16 && i+j < sizeof(cfgBuf); j++)
                                if (cfgBuf[i+j]) hasData = true;
                            if (hasData) {
                                printf("   %04zx: ", i);
                                for (size_t j = 0; j < 16 && i+j < sizeof(cfgBuf); j++)
                                    printf("%02x ", cfgBuf[i+j]);
                                printf("\n");
                            }
                        }
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
                }
            }

            FreeLibrary(tetherDll);
        } else {
            printf("   Failed to load Tether DLL: %lu\n", GetLastError());
        }

        // Close session
        printf("\n7. Close session\n");
        retError = 0;
        LMX_func_api_Close_Session(&retError);
        printf("   Close session: error=0x%08x\n", retError);
    }

    // Cleanup
    printf("\n8. Cleanup\n");
    retError = 0;
    LMX_func_api_Close_Device(&retError);
    printf("   Close device: error=0x%08x\n", retError);

    delete devInfo;
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
