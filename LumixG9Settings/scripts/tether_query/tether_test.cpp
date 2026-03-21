/**
 * tether_test — Connect via Tether DLL using corrected struct sizes
 *
 * Key differences from SDK DLL:
 * - ARRAY_MAX = 127 (not 512)
 * - Function names without underscores (SelectPnPDevice vs Select_PnPDevice)
 * - Must use MTA COM init (STA returns 0x80010106)
 * - Callback type 1 must be registered before OpenSession
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>

// --- Tether DLL struct definitions ---
// ARRAY_MAX=64 (NOT 512 like SDK, NOT 127 as previously guessed)
// Confirmed by raw byte analysis:
//   "Panasonic" at offset 524 = 8 + 64*8 + 4 (Info[0].dev_MakerName)
//   MakerName_Length at offset 1036 = 524 + 512, value=10
//   "DC-G9M2" at offset 1040 = 1036 + 4 (Info[0].dev_ModelName)
//   ModelName_Length at offset 1552 = 1040 + 512, value=8
#define TETHER_ARRAY_MAX  64
#define TETHER_STRING_MAX 256

#pragma pack(push, 8)  // Match DLL alignment

typedef struct {
    UINT32 dev_Index;
    WCHAR  dev_MakerName[TETHER_STRING_MAX];
    UINT32 dev_MakerName_Length;
    WCHAR  dev_ModelName[TETHER_STRING_MAX];
    UINT32 dev_ModelName_Length;
} TETHER_DEV_INFO;

typedef struct {
    UINT32          find_PnpDevice_Count;
    PWSTR           find_PnpDevice_IDs[TETHER_ARRAY_MAX];
    TETHER_DEV_INFO find_PnpDevice_Info[TETHER_ARRAY_MAX];
} TETHER_CONNECT_DEVICE_INFO;

#pragma pack(pop)

// Callback type matching SDK definition
typedef int (WINAPI *TETHER_CALLBACK_FUNC)(UINT32, UINT32);

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

int WINAPI tetherCallback(UINT32 event, UINT32 param) {
    printf("  [TETHER CB] event=0x%08x param=0x%08x\n", event, param);
    return 0;
}

int main() {
    const wchar_t* DLL_PATH = L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll";

    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    // COM init — try STA first (WPD requires STA for some operations)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    printf("COM STA: 0x%08lx\n", hr);
    if (FAILED(hr)) {
        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        printf("COM MTA: 0x%08lx\n", hr);
    }

    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

    HMODULE dll = LoadLibraryW(DLL_PATH);
    if (!dll) { printf("DLL load failed: %lu\n", GetLastError()); return 1; }

    // --- Resolve functions (Tether naming = no underscores) ---
    typedef void    (WINAPI *pfn_Init)(void);
    typedef UINT8   (WINAPI *pfn_GetPnP)(TETHER_CONNECT_DEVICE_INFO*, UINT32*);
    typedef UINT8   (WINAPI *pfn_SelectPnP)(UINT32, TETHER_CONNECT_DEVICE_INFO*, UINT32*);
    typedef UINT8   (WINAPI *pfn_OpenSession)(UINT32, UINT32*, UINT32*);
    typedef UINT8   (WINAPI *pfn_CloseSession)(UINT32*);
    typedef UINT8   (WINAPI *pfn_CloseDevice)(UINT32*);
    typedef UINT32  (WINAPI *pfn_RegCallback)(UINT32, TETHER_CALLBACK_FUNC);
    typedef UINT8   (WINAPI *pfn_IsConnUSB)(void);
    typedef UINT8   (WINAPI *pfn_Query)(void*, UINT32*);

    auto pInit         = (pfn_Init)GetProcAddress(dll, "LMX_func_api_Init");
    auto pGetPnP       = (pfn_GetPnP)GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");
    auto pSelectPnP    = (pfn_SelectPnP)GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");
    auto pOpenSession  = (pfn_OpenSession)GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession = (pfn_CloseSession)GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice  = (pfn_CloseDevice)GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pRegCallback  = (pfn_RegCallback)GetProcAddress(dll, "LMX_func_api_Reg_NotifyCallback");
    auto pIsConnUSB    = (pfn_IsConnUSB)GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");

    auto pGetFnAssign     = (pfn_Query)GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetSetupConfig  = (pfn_Query)GetProcAddress(dll, "LMX_func_api_GetSetupConfigInfo");
    auto pGetDeviceInfo   = (pfn_Query)GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");

    printf("Struct sizes: DEV_INFO=%zu CONNECT=%zu\n",
           sizeof(TETHER_DEV_INFO), sizeof(TETHER_CONNECT_DEVICE_INFO));

    if (!pInit || !pGetPnP || !pSelectPnP || !pOpenSession) {
        printf("Missing required functions!\n");
        return 1;
    }

    UINT32 err = 0;
    UINT8 ret;

    // === 1. Init ===
    printf("\n1. Init\n");
    pInit();

    // === 2. Register callback (type=1, matching SDK's successful call) ===
    printf("2. Reg_NotifyCallback\n");
    if (pRegCallback) {
        UINT32 cbRet;
        cbRet = pRegCallback(0, tetherCallback);
        printf("   type=0: %u\n", cbRet);
        cbRet = pRegCallback(1, tetherCallback);
        printf("   type=1: %u\n", cbRet);
    }

    // === 3. GetPnPDeviceInfo ===
    printf("\n3. GetPnPDeviceInfo\n");
    auto* devInfo = new TETHER_CONNECT_DEVICE_INFO();
    memset(devInfo, 0, sizeof(TETHER_CONNECT_DEVICE_INFO));

    ret = pGetPnP(devInfo, &err);
    printf("   result=%d, err=0x%x, devices=%u\n", ret, err, devInfo->find_PnpDevice_Count);

    if (devInfo->find_PnpDevice_Count == 0) {
        printf("   No camera!\n");
        delete devInfo;
        FreeLibrary(dll);
        CoUninitialize();
        return 1;
    }

    // Print device info
    for (UINT32 i = 0; i < devInfo->find_PnpDevice_Count; i++) {
        wprintf(L"   Device %u: maker='%s' model='%s'\n",
                i,
                devInfo->find_PnpDevice_Info[i].dev_MakerName,
                devInfo->find_PnpDevice_Info[i].dev_ModelName);
        if (devInfo->find_PnpDevice_IDs[i])
            wprintf(L"   ID: %s\n", devInfo->find_PnpDevice_IDs[i]);
    }

    // === 4. SelectPnPDevice — try multiple signatures ===
    printf("\n4. SelectPnPDevice\n");
    bool selected = false;
    auto pSelectRaw = GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");

    // 4a: SDK signature (index, struct, &err)
    printf("   4a: (0, devInfo, &err)... ");
    err = 0;
    __try {
        ret = ((UINT8(WINAPI*)(UINT32, void*, UINT32*))pSelectRaw)(0, devInfo, &err);
        printf("ret=%d err=0x%x\n", ret, err);
        if (ret == 1) selected = true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("CRASH\n");
    }

    // 4b: Reversed (struct, index, &err)
    if (!selected) {
        printf("   4b: (devInfo, 0, &err)... ");
        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32, UINT32*))pSelectRaw)(devInfo, 0, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) selected = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    // 4c: Just index + err (no struct)
    if (!selected) {
        printf("   4c: (0, &err)... ");
        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSelectRaw)(0, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) selected = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    // 4d: Device ID string + err
    if (!selected && devInfo->find_PnpDevice_IDs[0]) {
        printf("   4d: (deviceID, &err)... ");
        err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(PWSTR, UINT32*))pSelectRaw)(devInfo->find_PnpDevice_IDs[0], &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) selected = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    // 4e: SelectDevice instead (worked in earlier tests with Search_Device)
    if (!selected) {
        auto pSelectDev = GetProcAddress(dll, "LMX_func_api_SelectDevice");
        if (pSelectDev) {
            printf("   4e: SelectDevice(0, &err)... ");
            err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSelectDev)(0, &err);
                printf("ret=%d err=0x%x\n", ret, err);
                if (ret == 1) selected = true;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("CRASH\n");
            }
        }
    }

    // 4f: Search_Device then SelectDevice
    if (!selected) {
        auto pSearchDev = GetProcAddress(dll, "LMX_func_api_Search_Device");
        auto pSelectDev = GetProcAddress(dll, "LMX_func_api_SelectDevice");
        if (pSearchDev && pSelectDev) {
            printf("   4f: Search_Device(devInfo, &err)... ");
            err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(void*, UINT32*))pSearchDev)(devInfo, &err);
                printf("ret=%d err=0x%x\n", ret, err);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("CRASH\n");
            }
            printf("   4f: SelectDevice(0, &err)... ");
            err = 0;
            __try {
                ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSelectDev)(0, &err);
                printf("ret=%d err=0x%x\n", ret, err);
                if (ret == 1) selected = true;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("CRASH\n");
            }
        }
    }

    printf("   Selected: %s\n", selected ? "YES" : "NO");
    if (!selected) {
        printf("   Trying OpenSession anyway...\n");
    }

    // === 5. OpenSession — try multiple signatures ===
    printf("\n5. OpenSession\n");
    bool sessionOk = false;
    auto pOpenRaw = GetProcAddress(dll, "LMX_func_api_OpenSession");

    struct { const char* desc; int id; } osAttempts[] = {
        {"(0x10001, &v, &e)", 0},
        {"(1, &v, &e)", 1},
        {"(0, &v, &e)", 2},
        {"(&v, &e)", 3},
        {"(&e)", 4},
    };

    for (int i = 0; i < 5 && !sessionOk; i++) {
        UINT32 ver = 0; err = 0;
        printf("   %s... ", osAttempts[i].desc);
        __try {
            switch(i) {
                case 0: ret = ((UINT8(WINAPI*)(UINT32,UINT32*,UINT32*))pOpenRaw)(0x00010001, &ver, &err); break;
                case 1: ret = ((UINT8(WINAPI*)(UINT32,UINT32*,UINT32*))pOpenRaw)(1, &ver, &err); break;
                case 2: ret = ((UINT8(WINAPI*)(UINT32,UINT32*,UINT32*))pOpenRaw)(0, &ver, &err); break;
                case 3: ret = ((UINT8(WINAPI*)(UINT32*,UINT32*))pOpenRaw)(&ver, &err); break;
                case 4: ret = ((UINT8(WINAPI*)(UINT32*))pOpenRaw)(&err); break;
            }
            printf("ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
            if (ret == 1) sessionOk = true;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   IsConnected=%d\n", pIsConnUSB ? pIsConnUSB() : -1);

    if (sessionOk) {
        printf("\n   *** TETHER SESSION OPEN! ***\n");
    } else {
        printf("   All OpenSession attempts failed, trying queries anyway...\n");
    }

    // === 6. Query Fn Button Assignments ===
    if (pGetFnAssign) {
        printf("\n6. SetupConfig_Get_Fn_Assign\n");
        uint8_t fnBuf[16384];
        memset(fnBuf, 0, sizeof(fnBuf));
        err = 0;
        __try {
            ret = pGetFnAssign(fnBuf, &err);
            printf("   result=%d, err=0x%x\n", ret, err);
            if (ret == 1) {
                // Find extent of data
                size_t last = 0;
                for (size_t i = 0; i < sizeof(fnBuf); i++)
                    if (fnBuf[i]) last = i;
                printf("   Data extent: %zu bytes\n", last + 1);
                hexdump(fnBuf, last + 32 < sizeof(fnBuf) ? last + 32 : sizeof(fnBuf));
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
        }
    }

    // === 7. GetSetupConfigInfo ===
    if (pGetSetupConfig) {
        printf("\n7. GetSetupConfigInfo\n");
        uint8_t cfgBuf[65536];
        memset(cfgBuf, 0, sizeof(cfgBuf));
        err = 0;
        __try {
            ret = pGetSetupConfig(cfgBuf, &err);
            printf("   result=%d, err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(cfgBuf); i++)
                    if (cfgBuf[i]) last = i;
                printf("   Data extent: %zu bytes\n", last + 1);
                // Show first 256 bytes and then any interesting sections
                hexdump(cfgBuf, 256 < last + 32 ? 256 : last + 32, 16);
                if (last > 256) {
                    printf("   ... (showing end)\n");
                    hexdump(cfgBuf + last - 64, 96, 8);
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
        }
    }

    // === 8. GetDeviceInfo ===
    if (pGetDeviceInfo) {
        printf("\n8. GetDeviceInfo\n");
        uint8_t infoBuf[8192];
        memset(infoBuf, 0, sizeof(infoBuf));
        err = 0;
        __try {
            ret = pGetDeviceInfo(infoBuf, &err);
            printf("   result=%d, err=0x%x\n", ret, err);
            if (ret == 1) {
                size_t last = 0;
                for (size_t i = 0; i < sizeof(infoBuf); i++)
                    if (infoBuf[i]) last = i;
                printf("   Data extent: %zu bytes\n", last + 1);
                hexdump(infoBuf, last + 32 < sizeof(infoBuf) ? last + 32 : sizeof(infoBuf));
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASHED: 0x%08lx\n", GetExceptionCode());
        }
    }

    // === Cleanup ===
    printf("\n=== Cleanup ===\n");
    __try {
        err = 0; pCloseSession(&err);
        printf("   CloseSession: err=0x%x\n", err);
        err = 0; pCloseDevice(&err);
        printf("   CloseDevice: err=0x%x\n", err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("   Cleanup crashed\n");
    }

    delete devInfo;
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
