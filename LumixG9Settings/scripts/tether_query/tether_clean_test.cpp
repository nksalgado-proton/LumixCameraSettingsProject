/**
 * tether_clean_test — Minimal clean Tether DLL session attempt
 *
 * Clean sequence: Init → RegCallback(1) → GetPnP → SelectPnP → Sleep → OpenSession
 * No debug logging, no extra calls, minimal code path.
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

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    HMODULE dll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
    if (!dll) { printf("DLL load failed\n"); return 1; }

    auto pInit        = (void(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_Init");
    auto pRegCallback = (UINT32(WINAPI*)(UINT32, LMX_CALLBACK_FUNC))GetProcAddress(dll, "LMX_func_api_Reg_NotifyCallback");
    auto pGetPnP      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");
    auto pSelectPnP   = (UINT8(WINAPI*)(UINT32, void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");
    auto pOpenSession = GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession= (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pIsUSB       = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");
    auto pGetFn       = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetDev      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");
    auto pGetCfg      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetSetupConfigInfo");

    UINT32 err = 0;
    UINT8 ret;

    // 1. Init
    printf("1. Init\n");
    pInit();

    // 2. Callback
    printf("2. RegCallback(1)\n");
    UINT32 cbr = pRegCallback(1, myCallback);
    printf("   result: %u\n", cbr);

    // 3. GetPnP
    printf("3. GetPnPDeviceInfo\n");
    auto* dev = new T_CONNECT_DEVICE_INFO();
    memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
    err = 0;
    ret = pGetPnP(dev, &err);
    printf("   ret=%d err=0x%x count=%u\n", ret, err, dev->find_PnpDevice_Count);
    if (dev->find_PnpDevice_Count == 0) { printf("No camera!\n"); return 1; }

    // Read Tether layout data
    uint8_t* raw = (uint8_t*)dev;
    WCHAR* maker = (WCHAR*)(raw + 524);
    WCHAR* model = (WCHAR*)(raw + 1040);
    wprintf(L"   Camera: %s %s\n", maker, model);

    // 4. SelectPnP
    printf("4. SelectPnPDevice(0)\n");
    err = 0;
    __try {
        ret = pSelectPnP(0, dev, &err);
        printf("   ret=%d err=0x%x\n", ret, err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
        goto cleanup;
    }

    if (ret != 1) {
        printf("   SelectPnP failed.\n");
        goto cleanup;
    }

    printf("   SelectPnP OK! Waiting 2 seconds...\n");
    Sleep(2000);

    // Check USB
    if (pIsUSB) {
        UINT8 usb = pIsUSB();
        printf("   IsConnectedUSB: %d\n", usb);
    }

    // 5. OpenSession — try SDK-compatible signature first
    printf("5. OpenSession\n");
    {
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
            printf("   (0x10001, &ver, &err): ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
            ret = 0;
        }
    }

    if (ret == 1) {
        printf("\n*** TETHER SESSION OPEN! ***\n");
        goto query;
    }

    // Try without OpenSession — maybe SelectPnP already opened it?
    printf("\n   OpenSession returned 0. Trying queries anyway...\n");

query:
    printf("\n6. Queries\n");

    if (pGetDev) {
        printf("--- GetDeviceInfo ---\n");
        uint8_t buf[8192]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = pGetDev(buf, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf));
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    if (pGetFn) {
        printf("--- GetFnAssign ---\n");
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
    }

    if (pGetCfg) {
        printf("--- GetSetupConfigInfo ---\n");
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
