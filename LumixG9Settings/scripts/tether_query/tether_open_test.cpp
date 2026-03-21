/**
 * tether_open_test — Focus on getting Tether DLL OpenSession to work
 *
 * SelectPnP(0) returns 1 (success). Now try every OpenSession signature variant.
 * Also try DbgLog_Init for debug output.
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

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 32) {
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

void tryFnQuery(HMODULE dll) {
    auto pGetFn = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    if (!pGetFn) return;
    uint8_t buf[16384]; memset(buf, 0, sizeof(buf));
    UINT32 err = 0;
    __try {
        UINT8 ret = pGetFn(buf, &err);
        printf("   GetFnAssign: ret=%d err=0x%x\n", ret, err);
        if (ret == 1) {
            size_t last = 0;
            for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
            printf("   Fn data (%zu bytes):\n", last + 1);
            hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf));
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("   GetFnAssign CRASHED: 0x%08lx\n", GetExceptionCode());
    }
}

void tryQueries(HMODULE dll) {
    tryFnQuery(dll);

    // Also try GetDeviceInfo and GetSetupConfigInfo
    auto pGetDev = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");
    if (pGetDev) {
        uint8_t buf[8192]; memset(buf, 0, sizeof(buf));
        UINT32 err = 0;
        __try {
            UINT8 ret = pGetDev(buf, &err);
            printf("   GetDeviceInfo: ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(buf); i++) if (buf[i]) last = i;
                hexdump(buf, last + 32 < sizeof(buf) ? last + 32 : sizeof(buf), 8);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   GetDeviceInfo CRASHED\n");
        }
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
    auto pDbgLogInit  = GetProcAddress(dll, "LMX_func_api_DbgLog_Init");
    auto pIsUSB       = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");

    printf("OpenSession=%p DbgLogInit=%p\n", (void*)pOpenSession, (void*)pDbgLogInit);

    // Try debug logging
    if (pDbgLogInit) {
        printf("\nTrying DbgLog_Init...\n");
        __try {
            // Maybe: void DbgLog_Init(UINT32 level) or DbgLog_Init(WCHAR* path)
            ((void(WINAPI*)(UINT32))pDbgLogInit)(0xFFFFFFFF);
            printf("   DbgLog_Init(0xFFFFFFFF) ok\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   DbgLog_Init CRASHED\n");
        }
    }

    // Init + callback
    pInit();
    pRegCallback(1, myCallback);

    // GetPnP
    auto* dev = new T_CONNECT_DEVICE_INFO();
    memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
    UINT32 err = 0;
    UINT8 ret = pGetPnP(dev, &err);
    printf("\nGetPnP: ret=%d count=%u\n", ret, dev->find_PnpDevice_Count);
    if (dev->find_PnpDevice_Count == 0) { printf("No camera!\n"); return 1; }

    // SelectPnP
    err = 0;
    ret = pSelectPnP(0, dev, &err);
    printf("SelectPnP(0): ret=%d err=0x%x\n", ret, err);

    if (ret != 1) {
        printf("SelectPnP failed! Aborting.\n");
        return 1;
    }

    printf("\n*** SelectPnP succeeded! Trying OpenSession variants ***\n\n");

    UINT8 usb = pIsUSB ? pIsUSB() : 255;
    printf("IsConnectedUSB: %d\n\n", usb);

    // Variant 1: SDK-style (UINT32 ver, UINT32* deviceVer, UINT32* err)
    {
        printf("--- Variant 1: OpenSession(0x10001, &ver, &err) ---\n");
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
            printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 2: Just error pointer
    {
        printf("--- Variant 2: OpenSession(&err) ---\n");
        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32*))pOpenSession)(&err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 3: No args
    {
        printf("--- Variant 3: OpenSession() ---\n");
        __try {
            ret = ((UINT8(WINAPI*)(void))pOpenSession)();
            printf("   ret=%d\n", ret);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 4: (UINT32 sessionId, UINT32* err)
    {
        printf("--- Variant 4: OpenSession(0x10001, &err) ---\n");
        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pOpenSession)(0x00010001, &err);
            printf("   ret=%d err=0x%x\n", ret, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 5: (1, &ver, &err)
    {
        printf("--- Variant 5: OpenSession(1, &ver, &err) ---\n");
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(1, &ver, &err);
            printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 6: (0, &ver, &err)
    {
        printf("--- Variant 6: OpenSession(0, &ver, &err) ---\n");
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0, &ver, &err);
            printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Variant 7: Maybe it needs callback type 0 registered too?
    {
        printf("\n--- Re-registering callback type 0 and retrying ---\n");
        UINT32 r = pRegCallback(0, myCallback);
        printf("   RegCallback(0): %u\n", r);
        err = 0;
        UINT32 ver = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
            printf("   OpenSession: ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
            if (ret == 1) { printf("   *** SUCCESS ***\n"); tryQueries(dll); goto cleanup; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED\n");
        }
    }

    // Maybe we can query WITHOUT opening a session? SelectPnP succeeded...
    printf("\n--- Try queries without OpenSession (SelectPnP was successful) ---\n");
    tryQueries(dll);

cleanup:
    printf("\n=== Cleanup ===\n");
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    delete dev;
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
