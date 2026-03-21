/**
 * tether512_test — Use SDK-sized struct (ARRAY_MAX=512) with Tether DLL
 *
 * Theory: Tether DLL's GetPnPDeviceInfo writes at ARRAY_MAX=64 offsets,
 * but SelectPnPDevice/OpenSession might use ARRAY_MAX=512 internally
 * (since Tether DLL may link against SDK code internally).
 *
 * So: populate the struct manually at SDK offsets after reading Tether data.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>

// SDK struct definitions (ARRAY_MAX=512)
#define SDK_ARRAY_MAX  512
#define SDK_STRING_MAX 256

typedef struct {
    UINT32 dev_Index;
    WCHAR  dev_MakerName[SDK_STRING_MAX];
    UINT32 dev_MakerName_Length;
    WCHAR  dev_ModelName[SDK_STRING_MAX];
    UINT32 dev_ModelName_Length;
} SDK_DEV_INFO;

typedef struct {
    UINT32        find_PnpDevice_Count;
    PWSTR         find_PnpDevice_IDs[SDK_ARRAY_MAX];
    SDK_DEV_INFO  find_PnpDevice_Info[SDK_ARRAY_MAX];
} SDK_CONNECT_DEVICE_INFO;

typedef int (WINAPI *LMX_CALLBACK_FUNC)(UINT32, UINT32);

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

int WINAPI myCallback(UINT32 event, UINT32 param) {
    printf("  [CB] event=0x%x param=0x%x\n", event, param);
    return 0;
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
    auto pGetPnP      = GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");
    auto pSelectPnP   = GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");
    auto pOpenSession = GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession= (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pRegCallback = (UINT32(WINAPI*)(UINT32, LMX_CALLBACK_FUNC))GetProcAddress(dll, "LMX_func_api_Reg_NotifyCallback");
    auto pGetFnAssign = GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetCfgInfo  = GetProcAddress(dll, "LMX_func_api_GetSetupConfigInfo");
    auto pGetDevInfo  = GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");

    printf("sizeof(SDK_CONNECT_DEVICE_INFO) = %zu\n", sizeof(SDK_CONNECT_DEVICE_INFO));

    UINT32 err = 0;
    UINT8 ret;

    // 1. Init + Callback
    printf("\n1. Init\n");
    pInit();
    if (pRegCallback) {
        pRegCallback(1, myCallback);
        printf("   Callback registered.\n");
    }

    // 2. GetPnPDeviceInfo with SDK-sized struct
    printf("\n2. GetPnPDeviceInfo (SDK struct)\n");
    auto* devInfo = new SDK_CONNECT_DEVICE_INFO();
    memset(devInfo, 0, sizeof(SDK_CONNECT_DEVICE_INFO));

    ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetPnP)(devInfo, &err);
    printf("   ret=%d err=0x%x count=%u\n", ret, err, devInfo->find_PnpDevice_Count);

    if (devInfo->find_PnpDevice_Count == 0) {
        printf("   No camera!\n");
        return 1;
    }

    // Data is at TETHER layout (ARRAY_MAX=64):
    // - IDs[0] at offset 8 (same for SDK and Tether)
    // - MakerName at offset 524 (8 + 64*8 + 4)
    // - ModelName at offset 1040 (524 + 512 + 4)
    // SDK layout expects:
    // - IDs[0] at offset 8 (same)
    // - Info[0].dev_MakerName at offset 4108 (8 + 512*8 + 4)

    // Read from Tether offsets
    uint8_t* raw = (uint8_t*)devInfo;
    PWSTR deviceId = *(PWSTR*)(raw + 8);
    WCHAR* makerT = (WCHAR*)(raw + 524);   // Tether offset for maker
    WCHAR* modelT = (WCHAR*)(raw + 1040);  // Tether offset for model
    wprintf(L"   Tether layout: maker='%s' model='%s'\n", makerT, modelT);

    // SDK layout reads
    wprintf(L"   SDK layout: maker='%s' model='%s'\n",
            devInfo->find_PnpDevice_Info[0].dev_MakerName,
            devInfo->find_PnpDevice_Info[0].dev_ModelName);
    if (deviceId) wprintf(L"   ID: %s\n", deviceId);

    // Copy Tether data into SDK layout positions so SelectPnPDevice works
    printf("\n   Copying data to SDK layout positions...\n");
    wcscpy(devInfo->find_PnpDevice_Info[0].dev_MakerName, makerT);
    devInfo->find_PnpDevice_Info[0].dev_MakerName_Length = 10;
    wcscpy(devInfo->find_PnpDevice_Info[0].dev_ModelName, modelT);
    devInfo->find_PnpDevice_Info[0].dev_ModelName_Length = 8;
    devInfo->find_PnpDevice_Info[0].dev_Index = 0;

    wprintf(L"   SDK layout now: maker='%s' model='%s'\n",
            devInfo->find_PnpDevice_Info[0].dev_MakerName,
            devInfo->find_PnpDevice_Info[0].dev_ModelName);

    // 3. SelectPnPDevice with SDK struct
    printf("\n3. SelectPnPDevice(0, devInfo, &err)\n");
    err = 0;
    __try {
        ret = ((UINT8(WINAPI*)(UINT32, void*, UINT32*))pSelectPnP)(0, devInfo, &err);
        printf("   ret=%d err=0x%x\n", ret, err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("   CRASH: 0x%08lx\n", GetExceptionCode());
        ret = 0;
    }

    // 4. OpenSession
    if (ret == 1) {
        printf("\n4. OpenSession(0x00010001)\n");
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(0x00010001, &ver, &err);
            printf("   ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("   CRASH\n");
            ret = 0;
        }

        if (ret == 1) {
            printf("\n*** TETHER SESSION OPEN! ***\n");

            // Query Fn buttons
            if (pGetFnAssign) {
                printf("\n5. SetupConfig_Get_Fn_Assign\n");
                uint8_t fnBuf[16384]; memset(fnBuf, 0, sizeof(fnBuf)); err = 0;
                __try {
                    ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetFnAssign)(fnBuf, &err);
                    printf("   ret=%d err=0x%x\n", ret, err);
                    if (ret == 1) {
                        size_t last = 0;
                        for (size_t i = 0; i < sizeof(fnBuf); i++) if (fnBuf[i]) last = i;
                        printf("   Fn data (%zu bytes):\n", last + 1);
                        hexdump(fnBuf, last + 32 < sizeof(fnBuf) ? last + 32 : sizeof(fnBuf));
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASH\n");
                }
            }

            if (pGetCfgInfo) {
                printf("\n6. GetSetupConfigInfo\n");
                uint8_t cfgBuf[65536]; memset(cfgBuf, 0, sizeof(cfgBuf)); err = 0;
                __try {
                    ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetCfgInfo)(cfgBuf, &err);
                    printf("   ret=%d err=0x%x\n", ret, err);
                    if (ret == 1) {
                        size_t last = 0;
                        for (size_t i = 0; i < sizeof(cfgBuf); i++) if (cfgBuf[i]) last = i;
                        printf("   Config data (%zu bytes):\n", last + 1);
                        hexdump(cfgBuf, 512 < last ? 512 : last + 32, 32);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("   CRASH\n");
                }
            }
        }
    } else {
        printf("   SelectPnPDevice failed (ret=%d). Cannot open session.\n", ret);
    }

    // Cleanup
    printf("\nCleanup\n");
    __try {
        if (pCloseSession) { err = 0; pCloseSession(&err); }
        if (pCloseDevice) { err = 0; pCloseDevice(&err); }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    delete devInfo;
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
