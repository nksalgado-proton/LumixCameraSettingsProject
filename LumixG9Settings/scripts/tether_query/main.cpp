/**
 * tether_query — Minimal test: SelectPnPDevice works, test OpenSession.
 *
 * IMPORTANT: Camera must be in "PC (Tether)" USB mode, not storage mode!
 * On G9 II: Menu > Setup > USB > USB Mode > PC (Tether)
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>

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

void scan_nonzero(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i += 64) {
        size_t end = (i + 64 < len) ? i + 64 : len;
        bool has = false;
        for (size_t j = i; j < end; j++) if (data[j]) { has = true; break; }
        if (has) {
            for (size_t j = i; j < end; j++) {
                if (data[j]) {
                    printf("  [+0x%04zx]:\n", j);
                    size_t dl = 128;
                    if (j + dl > len) dl = len - j;
                    hexdump(data + j, dl, 8);
                    break;
                }
            }
        }
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    AddDllDirectory(L"C:\\Program Files\\Panasonic\\LUMIX Tether");
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

    HMODULE dll = LoadLibraryW(L"C:\\Program Files\\Panasonic\\LUMIX Tether\\Lmxptpif.dll");
    if (!dll) { printf("DLL load failed\n"); return 1; }

    auto pInit              = (void(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_Init");
    auto pGetPnPDeviceInfo  = (UINT8(WINAPI*)(void*, UINT32*))GetProcAddress(dll, "LMX_func_api_GetPnPDeviceInfo");
    auto pSelectPnPDevice   = GetProcAddress(dll, "LMX_func_api_SelectPnPDevice");
    auto pOpenSession       = GetProcAddress(dll, "LMX_func_api_OpenSession");
    auto pCloseSession      = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseSession");
    auto pCloseDevice       = (UINT8(WINAPI*)(UINT32*))GetProcAddress(dll, "LMX_func_api_CloseDevice");
    auto pIsConnectedUSB    = (UINT8(WINAPI*)(void))GetProcAddress(dll, "LMX_func_api_IsConnectedUSBDevice");
    auto pSearchDevice      = GetProcAddress(dll, "LMX_func_api_Search_Device");
    auto pSelectDevice      = GetProcAddress(dll, "LMX_func_api_SelectDevice");

    auto pGetFnAssign       = GetProcAddress(dll, "LMX_func_api_SetupConfig_Get_Fn_Assign");
    auto pGetSetupConfigInfo= GetProcAddress(dll, "LMX_func_api_GetSetupConfigInfo");
    auto pGetDeviceInfo     = GetProcAddress(dll, "LMX_func_api_GetDeviceInfo");

    UINT32 err = 0;
    UINT8 ret;

    printf("1. Init\n");
    pInit();

    printf("2. GetPnPDeviceInfo\n");
    const size_t BUF = 1024 * 1024;
    uint8_t* devBuf = new uint8_t[BUF];
    memset(devBuf, 0, BUF);
    ret = pGetPnPDeviceInfo(devBuf, &err);
    UINT32 cnt = *(UINT32*)devBuf;
    if (cnt == 0) {
        printf("   No camera found.\n");
        printf("   - Is camera connected via USB?\n");
        printf("   - Is LUMIX Tether closed?\n");
        printf("   - Is USB mode set to 'PC (Tether)' on camera?\n");
        return 1;
    }
    wprintf(L"   Found: %s\n", (WCHAR*)(devBuf + 0x410));

    // === Test BOTH paths in sequence ===

    // --- PATH 1: Search_Device + SelectDevice (safe, doesn't crash) ---
    printf("\n--- PATH 1: Search_Device + SelectDevice ---\n");
    printf("3. Search_Device... ");
    err = 0;
    __try {
        ret = ((UINT8(WINAPI*)(void*, UINT32*))pSearchDevice)(devBuf, &err);
        printf("ret=%d err=0x%x\n", ret, err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("CRASH\n");
    }

    printf("4. SelectDevice(0)... ");
    err = 0;
    __try {
        ret = ((UINT8(WINAPI*)(UINT32, UINT32*))pSelectDevice)(0, &err);
        printf("ret=%d err=0x%x\n", ret, err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("CRASH\n");
    }

    printf("   IsConnected: %d\n", pIsConnectedUSB());

    printf("5. OpenSession(1)... ");
    {
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(1, &ver, &err);
            printf("ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   IsConnected: %d\n", pIsConnectedUSB());

    // Try queries even if OpenSession returned 0
    printf("6. Test queries:\n");

    printf("   GetDeviceInfo... ");
    {
        uint8_t buf[4096]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetDeviceInfo)(buf, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) scan_nonzero(buf, sizeof(buf));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   GetFnAssign... ");
    {
        uint8_t buf[8192]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetFnAssign)(buf, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) scan_nonzero(buf, sizeof(buf));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   GetSetupConfigInfo... ");
    {
        uint8_t buf[65536]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetSetupConfigInfo)(buf, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) scan_nonzero(buf, sizeof(buf));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    // --- PATH 2: SelectPnPDevice (raw, sometimes crashes OpenSession) ---
    printf("\n--- PATH 2: SelectPnPDevice ---\n");

    // Re-init to clean state
    printf("7. Re-init + GetPnPDeviceInfo\n");
    memset(devBuf, 0, BUF);
    pInit();
    ret = pGetPnPDeviceInfo(devBuf, &err);
    cnt = *(UINT32*)devBuf;
    printf("   Devices: %u\n", cnt);
    if (cnt == 0) { printf("Lost camera after re-init.\n"); goto cleanup; }

    printf("8. SelectPnPDevice(0)... ");
    err = 0;
    __try {
        ret = ((UINT8(WINAPI*)(UINT32, void*, UINT32*))pSelectPnPDevice)(0, devBuf, &err);
        printf("ret=%d err=0x%x\n", ret, err);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("CRASH\n");
        goto cleanup;
    }

    printf("   IsConnected: %d\n", pIsConnectedUSB());

    // Only try the non-crashing OpenSession signature
    printf("9. OpenSession(1)... ");
    {
        UINT32 ver = 0; err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(UINT32, UINT32*, UINT32*))pOpenSession)(1, &ver, &err);
            printf("ret=%d ver=0x%x err=0x%x\n", ret, ver, err);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   IsConnected: %d\n", pIsConnectedUSB());

    printf("10. Queries after SelectPnPDevice:\n");
    printf("   GetDeviceInfo... ");
    {
        uint8_t buf[4096]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetDeviceInfo)(buf, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) scan_nonzero(buf, sizeof(buf));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

    printf("   GetFnAssign... ");
    {
        uint8_t buf[8192]; memset(buf, 0, sizeof(buf)); err = 0;
        __try {
            ret = ((UINT8(WINAPI*)(void*, UINT32*))pGetFnAssign)(buf, &err);
            printf("ret=%d err=0x%x\n", ret, err);
            if (ret == 1) scan_nonzero(buf, sizeof(buf));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("CRASH\n");
        }
    }

cleanup:
    printf("\nCleanup...\n");
    __try {
        if (pCloseSession) { err = 0; pCloseSession(&err); }
        if (pCloseDevice) { err = 0; pCloseDevice(&err); }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    delete[] devBuf;
    FreeLibrary(dll);
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
