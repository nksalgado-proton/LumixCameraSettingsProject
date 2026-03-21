/**
 * tether_search_first — Call Search_Device before SelectPnP
 *
 * Hypothesis: Search_Device initializes internal state that SelectPnP needs.
 * In tether_connect_test, PATH D (SelectPnP) worked after PATH A (Search_and_Connect)
 * had already been called, which internally calls Search_Device.
 *
 * Also try: Search_Device + SelectDevice + OpenSession as alternative.
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
    auto pSearchDevice= (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_Search_Device");
    auto pSelectDevice= (UINT8(WINAPI*)(UINT32, UINT32*))GetProcAddress(dll, "LMX_func_api_SelectDevice");
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

    // Init + Callback
    printf("Init + RegCallback(1)\n");
    pInit();
    pRegCallback(1, myCallback);

    // First approach: Search_Device + SelectDevice + OpenSession
    printf("\n=== Approach 1: Search_Device + SelectDevice + OpenSession ===\n");
    {
        auto* dev = new T_CONNECT_DEVICE_INFO();
        memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;

        printf("Search_Device...\n");
        __try {
            ret = pSearchDevice(dev, &err);
            printf("   ret=%d err=0x%x count=%u\n", ret, err, dev->find_PnpDevice_Count);

            if (ret == 1 && dev->find_PnpDevice_Count > 0) {
                uint8_t* raw = (uint8_t*)dev;
                WCHAR* maker = (WCHAR*)(raw + 524);
                WCHAR* model = (WCHAR*)(raw + 1040);
                wprintf(L"   Camera: %s %s\n", maker, model);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
            ret = 0;
        }

        if (ret == 1) {
            printf("SelectDevice(0)...\n");
            err = 0;
            __try {
                ret = pSelectDevice(0, &err);
                printf("   ret=%d err=0x%x\n", ret, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASHED\n");
                ret = 0;
            }

            if (ret == 1) {
                printf("   SelectDevice OK!\n");
                if (pIsUSB) printf("   IsUSB: %d\n", pIsUSB());

                Sleep(1000);

                printf("OpenSession(0x10001, &ver, &err)...\n");
                UINT32 ver = 0; err = 0;
                __try {
                    ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
                    printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASHED\n");
                    ret = 0;
                }

                if (ret == 1) {
                    printf("\n*** SESSION OPEN via Search_Device path! ***\n");
                    goto query;
                }

                // Try query without session — SelectDevice might have opened it internally
                printf("   OpenSession failed. Trying queries anyway...\n");
                goto query;
            }
        }
        delete dev;
    }

    // Second approach: GetPnP + Search_Device (for state) + SelectPnP + OpenSession
    printf("\n=== Approach 2: GetPnP + Search_Device + SelectPnP + OpenSession ===\n");
    {
        // Reset
        __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        pInit();
        pRegCallback(1, myCallback);

        auto* dev = new T_CONNECT_DEVICE_INFO();
        memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;

        printf("GetPnP...\n");
        ret = pGetPnP(dev, &err);
        printf("   ret=%d count=%u\n", ret, dev->find_PnpDevice_Count);
        if (dev->find_PnpDevice_Count == 0) { printf("No camera\n"); goto cleanup; }

        // Call Search_Device too, to set internal state
        auto* dev2 = new T_CONNECT_DEVICE_INFO();
        memset(dev2, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        printf("Search_Device (for state)...\n");
        __try {
            ret = pSearchDevice(dev2, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
        delete dev2;

        printf("SelectPnPDevice(0)...\n");
        err = 0;
        __try {
            ret = pSelectPnP(0, dev, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
            ret = 0;
        }

        if (ret == 1) {
            Sleep(1000);
            printf("OpenSession(0x10001, &ver, &err)...\n");
            UINT32 ver = 0; err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
                printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
                if (ret == 1) {
                    printf("\n*** SESSION OPEN via Approach 2! ***\n");
                    delete dev;
                    goto query;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   CRASHED\n");
            }

            // Try queries anyway
            printf("   OpenSession failed. Trying queries...\n");
            delete dev;
            goto query;
        }
        delete dev;
    }
    goto cleanup;

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
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
