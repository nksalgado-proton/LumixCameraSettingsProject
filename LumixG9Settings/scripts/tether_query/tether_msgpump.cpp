/**
 * tether_msgpump — Tether DLL with Windows message pump
 *
 * Theory: The Tether DLL uses COM objects in STA that require message
 * dispatching. The actual LUMIX Tether app is a GUI app with a message loop.
 * Without message pumping, COM callbacks and WPD events can't fire,
 * which might explain why IsConnectedUSB=0 and OpenSession fails.
 *
 * This test creates a message pump thread and pumps messages around
 * each critical DLL call.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>

#define TETHER_ARRAY_MAX 64
#define TETHER_STRING_MAX 256

typedef struct {
    UINT32 dev_Index;
    WCHAR  dev_MakerName[TETHER_STRING_MAX];
    UINT32 dev_MakerName_Length;
    WCHAR  dev_ModelName[TETHER_STRING_MAX];
    UINT32 dev_ModelName_Length;
} T_DEV_INFO;

typedef struct {
    UINT32       find_PnpDevice_Count;
    PWSTR        find_PnpDevice_IDs[TETHER_ARRAY_MAX];
    T_DEV_INFO   find_PnpDevice_Info[TETHER_ARRAY_MAX];
} T_CONNECT_DEVICE_INFO;

typedef int (WINAPI *LMX_CALLBACK_FUNC)(UINT32, UINT32);

int WINAPI myCallback(UINT32 event, UINT32 param) {
    printf("  [CB] event=0x%x param=0x%x\n", event, param);
    return 0;
}

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

// Pump COM/Windows messages for a given duration (ms)
void pumpMessages(DWORD durationMs) {
    MSG msg;
    DWORD start = GetTickCount();
    while (GetTickCount() - start < durationMs) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Use STA — required for WPD and many COM objects
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    printf("COM STA: 0x%08lx\n", hr);

    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    HMODULE dll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
    if (!dll) { printf("DLL load failed\n"); return 1; }

    auto pInit        = (void(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_Init");
    auto pRegCallback = (UINT32(WINAPI*)(UINT32, LMX_CALLBACK_FUNC))GetProcAddress(dll, "LMX_func_api_Reg_NotifyCallback");
    auto pSearchDevice= (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_Search_Device");
    auto pSelectDevice= (UINT8(WINAPI*)(UINT32, UINT32*))GetProcAddress(dll, "LMX_func_api_SelectDevice");
    auto pOpenSession = GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession= (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pIsUSB       = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");
    auto pGetFn       = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetDev      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");
    auto pGetCfg      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetSetupConfigInfo");
    auto pSaC         = GetProcAddress(dll, "LMX_func_api_Search_and_Connect");
    auto pSaCUSBID    = GetProcAddress(dll, "LMX_func_api_Search_and_Connect_USBID");
    auto pGetPnP      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");

    UINT32 err = 0;
    UINT8 ret;

    // Init
    printf("\n1. Init\n");
    pInit();
    pumpMessages(500);

    // Callback
    printf("2. RegCallback(1)\n");
    pRegCallback(1, myCallback);
    pumpMessages(500);

    // Search
    printf("3. Search_Device\n");
    auto* dev = new T_CONNECT_DEVICE_INFO();
    memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
    err = 0;
    ret = pSearchDevice(dev, &err);
    printf("   ret=%d count=%u\n", ret, dev->find_PnpDevice_Count);
    pumpMessages(1000);

    if (dev->find_PnpDevice_Count == 0) {
        printf("No camera!\n");
        goto cleanup;
    }

    {
        uint8_t* raw = (uint8_t*)dev;
        WCHAR* maker = (WCHAR*)(raw + 524);
        WCHAR* model = (WCHAR*)(raw + 1040);
        wprintf(L"   Camera: %s %s\n", maker, model);
    }

    // Select
    printf("4. SelectDevice(0)\n");
    err = 0;
    ret = pSelectDevice(0, &err);
    printf("   ret=%d err=0x%x\n", ret, err);
    printf("   Pumping messages for 3 seconds...\n");
    pumpMessages(3000);

    printf("   IsUSB: %d\n", pIsUSB ? pIsUSB() : -1);

    // OpenSession
    printf("5. OpenSession\n");
    {
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
            printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
            ret = 0;
        }
    }

    pumpMessages(2000);
    printf("   IsUSB after pump: %d\n", pIsUSB ? pIsUSB() : -1);

    if (ret != 1) {
        printf("   OpenSession failed. Trying Search_and_Connect_USBID...\n");

        // Reset
        __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        pumpMessages(500);

        pInit();
        pRegCallback(1, myCallback);
        pumpMessages(500);

        // Get device ID
        auto* dev2 = new T_CONNECT_DEVICE_INFO();
        memset(dev2, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        pGetPnP(dev2, &err);
        pumpMessages(500);

        if (dev2->find_PnpDevice_Count > 0) {
            PWSTR devId = *(PWSTR*)((uint8_t*)dev2 + 8);
            if (devId) {
                wprintf(L"   SaC_USBID with: %s\n", devId);
                err = 0;
                __try {
                    ret = ((UINT8(WINAPI*)(PWSTR, UINT32*))pSaCUSBID)(devId, &err);
                    printf("   ret=%d err=0x%x\n", ret, err);
                    pumpMessages(3000);
                    printf("   IsUSB after SaC: %d\n", pIsUSB ? pIsUSB() : -1);
                    if (ret == 1) {
                        printf("   *** CONNECTED! ***\n");
                        goto query;
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASHED\n");
                }
            }
        }
        delete dev2;

        // Try Search_and_Connect with just &err
        printf("   Trying Search_and_Connect(&err)...\n");
        __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        pInit();
        pRegCallback(1, myCallback);
        pumpMessages(500);

        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32*))pSaC)(&err);
            printf("   SaC(&err): ret=%d err=0x%x\n", ret, err);
            pumpMessages(3000);
            printf("   IsUSB: %d\n", pIsUSB ? pIsUSB() : -1);
            if (ret == 1) {
                printf("   *** CONNECTED! ***\n");
                goto query;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   SaC CRASHED\n");
        }

        printf("   All connection methods failed.\n");

        // Last resort: try queries without session, after SelectDevice
        printf("   Retrying: Search + Select + pump + queries (no OpenSession)...\n");
        __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        pInit();
        pRegCallback(1, myCallback);
        pumpMessages(500);

        memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        pSearchDevice(dev, &err);
        pumpMessages(500);
        err = 0;
        pSelectDevice(0, &err);
        pumpMessages(3000);
        printf("   IsUSB: %d\n", pIsUSB ? pIsUSB() : -1);
    }

query:
    printf("\n=== Queries ===\n");

    if (pGetDev) {
        printf("GetDeviceInfo:\n");
        uint8_t buf[8192]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = pGetDev(buf, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf), 16);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
        pumpMessages(500);
    }

    if (pGetFn) {
        printf("GetFnAssign:\n");
        uint8_t buf[16384]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = pGetFn(buf, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                printf("   Fn data (%zu bytes):\n", last + 1);
                hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf));
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
        pumpMessages(500);
    }

    if (pGetCfg) {
        printf("GetSetupConfigInfo:\n");
        uint8_t buf[65536]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = pGetCfg(buf, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                printf("   Config data (%zu bytes):\n", last + 1);
                hexdump(buf, 256 < last ? 256 : last + 32, 16);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

cleanup:
    printf("\nCleanup\n");
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    delete dev;
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
