/**
 * tether_usb_mode — Set connection type to USB before connecting
 *
 * Key discovery: Tether DLL has ptpip_Set_Connection_Type.
 * It may default to PTP/IP (WiFi). Setting it to USB might fix OpenSession.
 * Also try SetSearchCondition.
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
    auto pOpenSession = GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession= (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pIsUSB       = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");
    auto pGetFn       = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetDev      = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");

    // Connection type functions
    auto pSetConnType = GetProcAddress(dll, "LMX_func_api_ptpip_Set_Connection_Type");
    auto pGetConnType = GetProcAddress(dll, "LMX_func_api_ptpip_Get_Connection_Type");
    auto pSetSearch   = GetProcAddress(dll, "LMX_func_api_SetSearchCondition");
    auto pSaCUSBID    = GetProcAddress(dll, "LMX_func_api_Search_and_Connect_USBID");
    auto pSaC         = GetProcAddress(dll, "LMX_func_api_Search_and_Connect");
    auto pGetUSBSpeed = GetProcAddress(dll, "LMX_func_api_GetUSBSpeed");

    printf("SetConnType=%p GetConnType=%p SetSearch=%p\n",
           (void*)pSetConnType, (void*)pGetConnType, (void*)pSetSearch);
    printf("SaC=%p SaC_USBID=%p GetUSBSpeed=%p\n",
           (void*)pSaC, (void*)pSaCUSBID, (void*)pGetUSBSpeed);

    UINT32 err = 0;
    UINT8 ret;

    // Init
    printf("\n1. Init + RegCallback\n");
    pInit();
    pRegCallback(1, myCallback);

    // Check current connection type
    if (pGetConnType) {
        printf("\n2. Get current connection type\n");
        UINT32 connType = 0xDEADBEEF;
        __try {
            // Try: UINT32 Get_Connection_Type(void) or (UINT32*)
            connType = ((UINT32(WINAPI*)(void))pGetConnType)();
            printf("   Connection type (return value): 0x%x\n", connType);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED (return value)\n");
        }

        // Also try with output pointer
        connType = 0xDEADBEEF;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32*))pGetConnType)(&connType);
            printf("   Connection type (out ptr): ret=%d type=0x%x\n", ret, connType);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED (out ptr)\n");
        }
    }

    // Set connection type to USB
    if (pSetConnType) {
        printf("\n3. Set connection type to USB\n");
        // Try type=0 (USB?), type=1, type=2
        for (UINT32 t = 0; t <= 2; t++) {
            __try {
                // Try: void Set_Connection_Type(UINT32 type)
                ((void(WINAPI*)(UINT32))pSetConnType)(t);
                printf("   Set(%u) ok\n", t);

                // Check if USB is now detected
                if (pIsUSB) {
                    UINT8 usb = pIsUSB();
                    printf("   IsUSB after Set(%u): %d\n", t, usb);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   Set(%u) CRASHED\n", t);
            }
        }
    }

    // Try SetSearchCondition
    if (pSetSearch) {
        printf("\n4. SetSearchCondition\n");
        // Unknown signature. Try common patterns.
        // Maybe: void SetSearchCondition(UINT32 condition) or (UINT32, UINT32)
        __try {
            ((void(WINAPI*)(UINT32))pSetSearch)(0);
            printf("   SetSearch(0) ok\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   SetSearch(0) CRASHED\n");
        }
        __try {
            ((void(WINAPI*)(UINT32))pSetSearch)(1);
            printf("   SetSearch(1) ok\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   SetSearch(1) CRASHED\n");
        }
    }

    // Now try the full connection sequence
    printf("\n5. Set USB mode then Search_and_Connect\n");

    // Set to USB (type=0, guess)
    if (pSetConnType) {
        __try {
            ((void(WINAPI*)(UINT32))pSetConnType)(0);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Get PnP to find device ID
    auto* dev = new T_CONNECT_DEVICE_INFO();
    memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
    err = 0;
    ret = pGetPnP(dev, &err);
    printf("   GetPnP: ret=%d count=%u\n", ret, dev->find_PnpDevice_Count);

    if (dev->find_PnpDevice_Count > 0) {
        uint8_t* raw = (uint8_t*)dev;
        PWSTR devId = *(PWSTR*)(raw + 8);

        // Try Search_and_Connect_USBID with device ID
        if (pSaCUSBID && devId) {
            wprintf(L"   Device ID: %s\n", devId);
            err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(PWSTR, UINT32*))pSaCUSBID)(devId, &err);
                printf("   SaC_USBID: ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    printf("   *** CONNECTED! ***\n");
                    if (pIsUSB) printf("   IsUSB: %d\n", pIsUSB());
                    goto query;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   SaC_USBID CRASHED\n");
            }
        }

        // Try Search_and_Connect (no args? or with devInfo?)
        if (pSaC) {
            printf("\n   Trying Search_and_Connect with no args...\n");
            __try {
                ret = ((UINT8(WINAPI*)(UINT32*))pSaC)(&err);
                printf("   SaC(&err): ret=%d err=0x%x\n", ret, err);
                if (ret == 1) {
                    printf("   *** CONNECTED! ***\n");
                    goto query;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   SaC(&err) CRASHED\n");
            }
        }
    }

    // Fallback: Search_Device + SelectDevice + set USB + OpenSession
    printf("\n6. Search_Device + SelectDevice + OpenSession\n");
    {
        __try { if (pCloseSession) { err = 0; pCloseSession(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        __try { if (pCloseDevice) { err = 0; pCloseDevice(&err); } } __except(EXCEPTION_EXECUTE_HANDLER) {}
        pInit();
        pRegCallback(1, myCallback);

        // Set USB mode
        if (pSetConnType) {
            __try { ((void(WINAPI*)(UINT32))pSetConnType)(0); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }

        memset(dev, 0, sizeof(T_CONNECT_DEVICE_INFO));
        err = 0;
        __try {
            ret = pSearchDevice(dev, &err);
            printf("   Search: ret=%d\n", ret);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   Search CRASHED\n");
            ret = 0;
        }

        if (ret == 1) {
            err = 0;
            __try {
                ret = pSelectDevice(0, &err);
                printf("   Select: ret=%d\n", ret);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("   Select CRASHED\n");
                ret = 0;
            }

            if (ret == 1) {
                printf("   IsUSB: %d\n", pIsUSB ? pIsUSB() : -1);

                // Get USB speed
                if (pGetUSBSpeed) {
                    UINT32 speed = 0; err = 0;
                    __try {
                        ret = ((UINT8(WINAPI*)(UINT32*, UINT32*))pGetUSBSpeed)(&speed, &err);
                        printf("   USB Speed: ret=%d speed=0x%x err=0x%x\n", ret, speed, err);
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        printf("   USB Speed CRASHED\n");
                    }
                }

                Sleep(1000);

                UINT32 ver = 0; err = 0;
                __try {
                    ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
                    printf("   Open: ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
                    if (ret == 1) {
                        printf("   *** SESSION! ***\n");
                        goto query;
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   Open CRASHED\n");
                }
            }
        }
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
