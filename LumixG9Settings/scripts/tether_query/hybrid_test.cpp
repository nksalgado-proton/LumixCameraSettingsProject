/**
 * hybrid_test — SDK DLL for PTP session + Tether DLL for Fn queries
 *
 * SDK DLL can open PTP sessions. Tether DLL has Fn button functions.
 * Theory: PTP session is at WPD level, Tether DLL needs Init+callback
 * to access the shared session.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>
#include "LMX_func_api.h"  // SDK headers

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 64) {
    for (size_t i = 0; i < len && i / 16 < max_lines; i += 16) {
        printf("  %04zx: ", i);
        for (size_t j = 0; j < 16 && i + j < len; j++)
            printf("%02x ", data[i + j]);
        for (size_t j = (len - i < 16 ? len - i : 16); j < 16; j++)
            printf("   ");
        printf(" ");
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = data[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("\n");
    }
}

int WINAPI sdkCallback(UINT32 event, UINT32 param) {
    printf("  [SDK CB] event=0x%x param=0x%x\n", event, param);
    return 0;
}

int WINAPI tetherCallback(UINT32 event, UINT32 param) {
    printf("  [TETHER CB] event=0x%x param=0x%x\n", event, param);
    return 0;
}

// Tether DLL function types
typedef void   (WINAPI *pfn_TInit)(void);
typedef UINT32 (WINAPI *pfn_TRegCB)(UINT32, int(WINAPI*)(UINT32,UINT32));
typedef UINT8  (WINAPI *pfn_TQuery)(void*, UINT32*);
typedef UINT8  (WINAPI *pfn_TClose)(UINT32*);

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    // ============================================================
    // Phase 1: SDK DLL — establish PTP session
    // ============================================================
    printf("\n=== Phase 1: SDK DLL Session ===\n");

    LMX_func_api_Init();
    LMX_func_api_Reg_NotifyCallback(1, sdkCallback);

    auto* devInfo = new LMX_CONNECT_DEVICE_INFO();
    memset(devInfo, 0, sizeof(LMX_CONNECT_DEVICE_INFO));
    UINT32 err = 0;

    UINT8 ret = LMX_func_api_Get_PnPDeviceInfo(devInfo, &err);
    printf("SDK GetPnP: ret=%d devices=%u\n", ret, devInfo->find_PnpDevice_Count);

    if (devInfo->find_PnpDevice_Count == 0) {
        printf("No camera!\n");
        return 1;
    }
    wprintf(L"Camera: %s\n", devInfo->find_PnpDevice_Info[0].dev_ModelName);

    ret = LMX_func_api_Select_PnPDevice(0, devInfo, &err);
    printf("SDK SelectPnP: ret=%d\n", ret);

    UINT32 deviceVer = 0;
    ret = LMX_func_api_Open_Session(0x00010001, &deviceVer, &err);
    printf("SDK OpenSession: ret=%d ver=0x%x\n", ret, deviceVer);

    if (ret != 1) {
        printf("SDK session failed!\n");
        return 1;
    }
    printf("*** SDK SESSION OPEN ***\n");

    // Verify SDK works
    UINT32 modePos = 0;
    LMX_func_api_CameraMode_Get_Mode_Pos(&modePos, &err);
    printf("SDK Mode: 0x%x\n", modePos);

    // ============================================================
    // Phase 2: Load Tether DLL and try queries
    // ============================================================
    printf("\n=== Phase 2: Tether DLL Queries ===\n");

    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    HMODULE tDll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
    if (!tDll) { printf("Tether DLL load failed\n"); goto cleanup; }

    {
        auto tInit    = (pfn_TInit)GetProcAddress(tDll, "LMX_func_api_Init");
        auto tRegCB   = (pfn_TRegCB)GetProcAddress(tDll, "LMX_func_api_Reg_NotifyCallback");
        auto tGetFn   = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
        auto tGetCfg  = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_GetSetupConfigInfo");
        auto tGetDev  = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_GetDeviceInfo");
        auto tGetHDMI = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_SetupConfig_Get_HDMI_Mode");
        auto tGetLCD  = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_SetupConfig_Get_LCD_Visibility");
        auto tGetSysFreq = (pfn_TQuery)GetProcAddress(tDll, "LMX_func_api_SetupConfig_Get_SystemFreq_Mode");

        // Initialize Tether DLL and register callback
        printf("Tether Init...\n");
        if (tInit) tInit();
        if (tRegCB) {
            UINT32 r = tRegCB(1, tetherCallback);
            printf("Tether Reg_NotifyCallback(1): %u\n", r);
        }

        // Try Tether queries with SDK session active
        printf("\n--- GetDeviceInfo ---\n");
        if (tGetDev) {
            uint8_t buf[8192]; memset(buf, 0, sizeof(buf)); err = 0;
            __try {
                ret = tGetDev(buf, &err);
                printf("   ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    size_t last = 0;
                    for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                    hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf));
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        printf("\n--- SetupConfig_Get_Fn_Assign ---\n");
        if (tGetFn) {
            uint8_t buf[16384]; memset(buf, 0, sizeof(buf)); err = 0;
            __try {
                ret = tGetFn(buf, &err);
                printf("   ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    size_t last = 0;
                    for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                    printf("   Data: %zu bytes\n", last + 1);
                    hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf));
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        printf("\n--- GetSetupConfigInfo ---\n");
        if (tGetCfg) {
            uint8_t buf[65536]; memset(buf, 0, sizeof(buf)); err = 0;
            __try {
                ret = tGetCfg(buf, &err);
                printf("   ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    size_t last = 0;
                    for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                    printf("   Data: %zu bytes\n", last + 1);
                    hexdump(buf, 256 < last ? 256 : last + 32, 16);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        // Try simpler Tether queries
        printf("\n--- HDMI Mode ---\n");
        if (tGetHDMI) {
            UINT32 val = 0; err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32*, UINT32*))tGetHDMI)(&val, &err);
                printf("   ret=%d val=0x%x err=0x%x\n", ret, val, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        printf("\n--- LCD Visibility ---\n");
        if (tGetLCD) {
            UINT32 val = 0; err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32*, UINT32*))tGetLCD)(&val, &err);
                printf("   ret=%d val=0x%x err=0x%x\n", ret, val, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        printf("\n--- SystemFreq Mode ---\n");
        if (tGetSysFreq) {
            UINT32 val = 0; err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32*, UINT32*))tGetSysFreq)(&val, &err);
                printf("   ret=%d val=0x%x err=0x%x\n", ret, val, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASH\n");
            }
        }

        FreeLibrary(tDll);
    }

cleanup:
    printf("\n=== Cleanup ===\n");
    LMX_func_api_Close_Session(&err);
    LMX_func_api_Close_Device(&err);
    delete devInfo;
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
