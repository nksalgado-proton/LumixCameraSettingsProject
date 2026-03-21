/**
 * tether_connect_test — Try all Tether DLL connection paths
 *
 * Path A: Search_and_Connect (all-in-one)
 * Path B: Search_and_Connect_USBID (with device ID)
 * Path C: Search_and_Connect_DEVICE_TYPE
 * Path D: Search_Device + SelectDevice + OpenSession (the old path that got Search=1, Select=1, Open=0)
 *
 * Goal: find a path that gets OpenSession to succeed so we can call GetFnAssign.
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

// Try querying Fn buttons
void tryFnQuery(HMODULE dll) {
    auto pGetFn = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    if (!pGetFn) { printf("   No GetFnAssign export!\n"); return; }

    uint8_t buf[16384];
    memset(buf, 0, sizeof(buf));
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

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    HMODULE dll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
    if (!dll) { printf("DLL load failed\n"); return 1; }

    auto pInit         = (void(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_Init");
    auto pRegCallback  = (UINT32(WINAPI*)(UINT32, LMX_CALLBACK_FUNC))GetProcAddress(dll, "LMX_func_api_Reg_NotifyCallback");
    auto pGetPnP       = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");
    auto pSelectPnP    = (UINT8(WINAPI*)(UINT32, void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");
    auto pSearchDevice = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_Search_Device");
    auto pSelectDevice = (UINT8(WINAPI*)(UINT32, UINT32*))GetProcAddress(dll, "LMX_func_api_SelectDevice");
    auto pOpenSession  = (UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice  = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pIsUSB        = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");

    // Search_and_Connect variants — unknown signatures, try common patterns
    auto pSaC          = GetProcAddress(dll, "LMX_func_api_Search_and_Connect");
    auto pSaCUSBID     = GetProcAddress(dll, "LMX_func_api_Search_and_Connect_USBID");
    auto pSaCDevType   = GetProcAddress(dll, "LMX_func_api_Search_and_Connect_DEVICE_TYPE");

    printf("Exports:\n");
    printf("  Init=%p RegCB=%p GetPnP=%p SelectPnP=%p\n", (void*)pInit, (void*)pRegCallback, (void*)pGetPnP, (void*)pSelectPnP);
    printf("  SearchDevice=%p SelectDevice=%p OpenSession=%p\n", (void*)pSearchDevice, (void*)pSelectDevice, (void*)pOpenSession);
    printf("  SaC=%p SaC_USBID=%p SaC_DevType=%p\n", (void*)pSaC, (void*)pSaCUSBID, (void*)pSaCDevType);
    printf("  IsUSB=%p\n", (void*)pIsUSB);

    UINT32 err = 0;
    UINT8 ret;

    // === Init + Callback ===
    printf("\n=== Init ===\n");
    pInit();
    if (pRegCallback) {
        UINT32 r = pRegCallback(1, myCallback);
        printf("   RegCallback(1): %u\n", r);
    }

    // === PATH A: Search_and_Connect ===
    printf("\n=== PATH A: Search_and_Connect ===\n");
    if (pSaC) {
        // Likely signature: UINT8 Search_and_Connect(UINT32* retError) or similar
        // Try: (devInfo, &err) like GetPnP
        auto* devA = new T_CONNECT_DEVICE_INFO();
        memset(devA, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        __try {
            // Try signature: UINT8(void*, UINT32*)
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pSaC)(devA, &err);
            printf("   SaC(devInfo, &err): ret=%d err=0x%x count=%u\n", ret, err, devA->find_PnpDevice_Count);
            if (ret == 1) {
                printf("   *** Search_and_Connect succeeded! ***\n");
                UINT8 usb = pIsUSB ? pIsUSB() : 255;
                printf("   IsUSB: %d\n", usb);
                tryFnQuery(dll);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   SaC(devInfo, &err) CRASHED: 0x%08lx\n", GetExceptionCode());
        }
        delete devA;
    }

    // Reset state
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Re-init
    pInit();
    if (pRegCallback) pRegCallback(1, myCallback);

    // === PATH B: Search_and_Connect_USBID ===
    printf("\n=== PATH B: Search_and_Connect_USBID ===\n");
    if (pSaCUSBID) {
        // Get device ID first
        auto* devB = new T_CONNECT_DEVICE_INFO();
        memset(devB, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        pGetPnP(devB, &err);

        if (devB->find_PnpDevice_Count > 0) {
            PWSTR devId = *(PWSTR*)((uint8_t*)devB + 8); // IDs[0] at offset 8
            if (devId) {
                wprintf(L"   Device ID: %s\n", devId);
                err = 0;
                __try {
                    // Try: UINT8(PWSTR usbId, UINT32* retError)
                    ret = ((UINT8(WINAPI*)(PWSTR, UINT32*))pSaCUSBID)(devId, &err);
                    printf("   SaC_USBID(id, &err): ret=%d err=0x%x\n", ret, err);
                    if (ret == 1) {
                        printf("   *** Search_and_Connect_USBID succeeded! ***\n");
                        tryFnQuery(dll);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   SaC_USBID CRASHED: 0x%08lx\n", GetExceptionCode());
                }
            }
        }
        delete devB;
    }

    // Reset state
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Re-init
    pInit();
    if (pRegCallback) pRegCallback(1, myCallback);

    // === PATH C: Search_and_Connect_DEVICE_TYPE ===
    printf("\n=== PATH C: Search_and_Connect_DEVICE_TYPE ===\n");
    if (pSaCDevType) {
        err = 0;
        __try {
            // Try: UINT8(UINT32 deviceType, UINT32* retError)
            // deviceType=0 might mean USB
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSaCDevType)(0, &err);
            printf("   SaC_DevType(0, &err): ret=%d err=0x%x\n", ret, err);
            if (ret == 1) {
                printf("   *** Search_and_Connect_DEVICE_TYPE(0) succeeded! ***\n");
                tryFnQuery(dll);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   SaC_DevType(0) CRASHED: 0x%08lx\n", GetExceptionCode());
        }

        if (ret != 1) {
            // Reset and try type=1
            __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
            __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
            pInit();
            if (pRegCallback) pRegCallback(1, myCallback);

            err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSaCDevType)(1, &err);
                printf("   SaC_DevType(1, &err): ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    printf("   *** Search_and_Connect_DEVICE_TYPE(1) succeeded! ***\n");
                    tryFnQuery(dll);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   SaC_DevType(1) CRASHED: 0x%08lx\n", GetExceptionCode());
            }
        }
    }

    // Reset state
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}

    // Re-init
    pInit();
    if (pRegCallback) pRegCallback(1, myCallback);

    // === PATH D: GetPnP + SelectPnP + OpenSession (reference path) ===
    printf("\n=== PATH D: GetPnP + SelectPnP + OpenSession ===\n");
    {
        auto* devD = new T_CONNECT_DEVICE_INFO();
        memset(devD, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        ret = pGetPnP(devD, &err);
        printf("   GetPnP: ret=%d count=%u\n", ret, devD->find_PnpDevice_Count);

        if (devD->find_PnpDevice_Count > 0) {
            err = 0;
            __try {
                ret = pSelectPnP(0, devD, &err);
                printf("   SelectPnP(0): ret=%d err=0x%x\n", ret, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   SelectPnP CRASHED\n");
                ret = 0;
            }

            if (ret == 1) {
                UINT32 ver = 0; err = 0;
                __try {
                    ret = pOpenSession(0x00010001, &ver, &err);
                    printf("   OpenSession: ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
                    if (ret == 1) {
                        printf("   *** SESSION OPEN! ***\n");
                        tryFnQuery(dll);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   OpenSession CRASHED\n");
                }
            }
        }
        delete devD;
    }

    // === PATH E: Search_Device + SelectDevice + OpenSession ===
    printf("\n=== PATH E: Search_Device + SelectDevice + OpenSession ===\n");
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    pInit();
    if (pRegCallback) pRegCallback(1, myCallback);

    {
        // Search_Device might populate internal state
        auto* devE = new T_CONNECT_DEVICE_INFO();
        memset(devE, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        __try {
            ret = pSearchDevice(devE, &err);
            printf("   Search_Device: ret=%d err=0x%x count=%u\n", ret, err, devE->find_PnpDevice_Count);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   Search_Device CRASHED\n");
            ret = 0;
        }

        if (ret == 1) {
            err = 0;
            __try {
                ret = pSelectDevice(0, &err);
                printf("   SelectDevice(0): ret=%d err=0x%x\n", ret, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   SelectDevice CRASHED\n");
                ret = 0;
            }

            if (ret == 1) {
                // Try OpenSession with different params
                UINT32 ver = 0; err = 0;
                __try {
                    ret = pOpenSession(0x00010001, &ver, &err);
                    printf("   OpenSession(0x10001): ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   OpenSession CRASHED\n");
                    ret = 0;
                }

                if (ret == 1) {
                    printf("   *** SESSION OPEN via Search_Device path! ***\n");
                    tryFnQuery(dll);
                } else {
                    // Maybe OpenSession has a different signature in Tether?
                    // Tether ordinal 5500 (0xF0) vs SDK. Try different arg patterns.
                    printf("   Trying alternative OpenSession signatures...\n");

                    // Maybe: UINT8 OpenSession(UINT32* retError) only
                    err = 0;
                    __try {
                        ret = ((UINT8(WINAPI*)(UINT32*))pOpenSession)(&err);
                        printf("   OpenSession(&err): ret=%d err=0x%x\n", ret, err);
                        if (ret == 1) {
                            printf("   *** SESSION OPEN (1-arg)! ***\n");
                            tryFnQuery(dll);
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        printf("   OpenSession(1-arg) CRASHED\n");
                    }
                }
            }
        }
        delete devE;
    }

    // Cleanup
    printf("\n=== Cleanup ===\n");
    __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
