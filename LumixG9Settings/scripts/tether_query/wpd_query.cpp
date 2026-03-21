/**
 * wpd_query — Send structured queries via Panasonic vendor PTP ops
 *
 * Camera supports 3 vendor ops: 0x9102, 0x9103, 0x9104
 * This test systematically probes all three with proper error handling.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <objbase.h>
#include <portabledeviceapi.h>
#include <portabledevice.h>
#include <wpdmtpextensions.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "PortableDeviceGUIDs.lib")

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

IPortableDevice* openCamera() {
    IPortableDeviceManager* mgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_IPortableDeviceManager, (void**)&mgr);
    if (FAILED(hr)) { printf("  DeviceManager failed: 0x%08lx\n", hr); return nullptr; }

    DWORD deviceCount = 0;
    mgr->GetDevices(nullptr, &deviceCount);
    if (deviceCount == 0) { printf("  No WPD devices\n"); mgr->Release(); return nullptr; }

    PWSTR* deviceIds = new PWSTR[deviceCount];
    mgr->GetDevices(deviceIds, &deviceCount);

    PWSTR cameraId = nullptr;
    for (DWORD i = 0; i < deviceCount; i++) {
        if (wcsstr(deviceIds[i], L"vid_04da")) { cameraId = deviceIds[i]; break; }
    }
    if (!cameraId) {
        printf("  Camera not found among %lu WPD devices\n", deviceCount);
        for (DWORD i = 0; i < deviceCount; i++) CoTaskMemFree(deviceIds[i]);
        delete[] deviceIds; mgr->Release(); return nullptr;
    }

    IPortableDevice* device = nullptr;
    CoCreateInstance(CLSID_PortableDevice, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevice, (void**)&device);

    IPortableDeviceValues* clientInfo = nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDeviceValues, (void**)&clientInfo);
    clientInfo->SetStringValue(WPD_CLIENT_NAME, L"XLumixG9Settings");
    clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
    clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
    clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 0);
    clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ | GENERIC_WRITE);

    hr = device->Open(cameraId, clientInfo);
    clientInfo->Release();

    for (DWORD i = 0; i < deviceCount; i++) CoTaskMemFree(deviceIds[i]);
    delete[] deviceIds; mgr->Release();

    if (FAILED(hr)) {
        printf("  Open failed: 0x%08lx\n", hr);
        device->Release(); return nullptr;
    }
    return device;
}

// Read data via MTP vendor command — returns response code
ULONG readVendorCmd(IPortableDevice* device, UINT16 opCode,
                    uint8_t* outBuf, size_t* outSize,
                    UINT32 numParams = 0, UINT32 p1 = 0, UINT32 p2 = 0, UINT32 p3 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    *outSize = 0;
    ULONG respCode = 0;

    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_IPortableDeviceValues, (void**)&params);
    if (FAILED(hr)) { printf("    [ERR CoCreate params: 0x%08lx]\n", hr); return 0xFFFF; }

    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* paramColl = nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevicePropVariantCollection, (void**)&paramColl);
    if (paramColl) {
        PROPVARIANT pv; PropVariantInit(&pv); pv.vt = VT_UI4;
        UINT32 pvals[] = {p1, p2, p3};
        for (UINT32 i = 0; i < numParams; i++) { pv.ulVal = pvals[i]; paramColl->Add(&pv); }
        params->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, paramColl);
    }
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, 0);

    hr = device->SendCommand(0, params, &results);
    if (FAILED(hr)) {
        printf("    [ERR SendCommand: 0x%08lx]\n", hr);
        if (paramColl) paramColl->Release();
        params->Release();
        return 0xFFFF;
    }

    if (results) {
        HRESULT cmdHr = S_OK;
        results->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &cmdHr);
        if (FAILED(cmdHr)) printf("    [CMD HRESULT: 0x%08lx]\n", cmdHr);

        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &respCode);

        LPWSTR transferCtx = nullptr;
        hr = results->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &transferCtx);
        if (SUCCEEDED(hr) && transferCtx) {
            IPortableDeviceValues* rp = nullptr;
            IPortableDeviceValues* rr = nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&rp);
            rp->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_READ_DATA.fmtid);
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_READ_DATA.pid);
            rp->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, 65536);
            BYTE* tmp = new BYTE[65536]; memset(tmp, 0, 65536);
            rp->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, tmp, 65536);
            delete[] tmp;

            hr = device->SendCommand(0, rp, &rr);
            if (SUCCEEDED(hr) && rr) {
                BYTE* data = nullptr; ULONG dataSize = 0;
                hr = rr->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &data, &dataSize);
                if (SUCCEEDED(hr) && data && dataSize > 0) {
                    memcpy(outBuf, data, dataSize < 65536 ? dataSize : 65536);
                    *outSize = dataSize;
                    CoTaskMemFree(data);
                }
                rr->Release();
            }
            rp->Release();

            // End transfer — get real response code
            IPortableDeviceValues* ep = nullptr; IPortableDeviceValues* er = nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&ep);
            ep->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
            ep->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
            ep->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            device->SendCommand(0, ep, &er);
            if (er) {
                ULONG endResp = 0;
                er->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &endResp);
                if (endResp != 0) respCode = endResp;
                er->Release();
            }
            ep->Release();
            CoTaskMemFree(transferCtx);
        }
        results->Release();
    }

    if (paramColl) paramColl->Release();
    params->Release();
    return respCode;
}

// Write data via MTP vendor command
ULONG writeVendorCmd(IPortableDevice* device, UINT16 opCode,
                     const uint8_t* data, size_t dataSize,
                     UINT32 numParams = 0, UINT32 p1 = 0, UINT32 p2 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    ULONG respCode = 0;

    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDeviceValues, (void**)&params);

    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* paramColl = nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevicePropVariantCollection, (void**)&paramColl);
    if (paramColl) {
        PROPVARIANT pv; PropVariantInit(&pv); pv.vt = VT_UI4;
        UINT32 pvals[] = {p1, p2};
        for (UINT32 i = 0; i < numParams; i++) { pv.ulVal = pvals[i]; paramColl->Add(&pv); }
        params->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, paramColl);
    }
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, (ULONG)dataSize);

    HRESULT hr = device->SendCommand(0, params, &results);
    if (FAILED(hr)) {
        printf("    [ERR SendCommand: 0x%08lx]\n", hr);
        if (paramColl) paramColl->Release();
        params->Release();
        return 0xFFFF;
    }

    if (results) {
        HRESULT cmdHr = S_OK;
        results->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &cmdHr);
        if (FAILED(cmdHr)) printf("    [CMD HRESULT: 0x%08lx]\n", cmdHr);

        LPWSTR transferCtx = nullptr;
        hr = results->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &transferCtx);
        if (SUCCEEDED(hr) && transferCtx) {
            IPortableDeviceValues* wp = nullptr; IPortableDeviceValues* wr = nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&wp);
            wp->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_WRITE_DATA.fmtid);
            wp->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_WRITE_DATA.pid);
            wp->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            wp->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_WRITE, (ULONG)dataSize);
            wp->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, (BYTE*)data, (ULONG)dataSize);

            hr = device->SendCommand(0, wp, &wr);
            if (FAILED(hr)) printf("    [ERR write: 0x%08lx]\n", hr);
            if (wr) wr->Release();
            wp->Release();

            IPortableDeviceValues* ep = nullptr; IPortableDeviceValues* er = nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&ep);
            ep->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
            ep->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
            ep->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            device->SendCommand(0, ep, &er);
            if (er) {
                er->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &respCode);
                er->Release();
            }
            ep->Release();
            CoTaskMemFree(transferCtx);
        } else {
            printf("    [No transfer context]\n");
        }
        results->Release();
    }

    if (paramColl) paramColl->Release();
    params->Release();
    return respCode;
}

// No-data command
ULONG noDataCmd(IPortableDevice* device, UINT16 opCode,
                UINT32 numParams = 0, UINT32 p1 = 0, UINT32 p2 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    ULONG respCode = 0;

    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDeviceValues, (void**)&params);
    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* paramColl = nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevicePropVariantCollection, (void**)&paramColl);
    if (paramColl) {
        PROPVARIANT pv; PropVariantInit(&pv); pv.vt = VT_UI4;
        if (numParams >= 1) { pv.ulVal = p1; paramColl->Add(&pv); }
        if (numParams >= 2) { pv.ulVal = p2; paramColl->Add(&pv); }
        params->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, paramColl);
    }

    HRESULT hr = device->SendCommand(0, params, &results);
    if (SUCCEEDED(hr) && results) {
        HRESULT cmdHr = S_OK;
        results->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &cmdHr);
        if (FAILED(cmdHr)) printf("    [CMD HRESULT: 0x%08lx]\n", cmdHr);
        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &respCode);
        results->Release();
    }

    if (paramColl) paramColl->Release();
    params->Release();
    return respCode;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    IPortableDevice* device = openCamera();
    if (!device) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n\n");

    uint8_t buf[65536];
    size_t bufSize = 0;

    // Step 1: Read device status (0x9104)
    printf("=== 0x9104 device status ===\n");
    ULONG resp = readVendorCmd(device, 0x9104, buf, &bufSize);
    printf("   resp=0x%04lx, %zu bytes\n", resp, bufSize);
    if (bufSize > 0) hexdump(buf, bufSize, 4);

    // Step 2: Check session state
    printf("\n=== 0x9102 session check ===\n");
    resp = noDataCmd(device, 0x9102, 1, 0x00010001);
    printf("   0x9102(0x10001): resp=0x%04lx\n", resp);

    // Step 3: 0x9103 data-to-write with small payloads
    printf("\n=== 0x9103 data-to-write ===\n");
    {
        uint32_t v;
        v = 0; resp = writeVendorCmd(device, 0x9103, (uint8_t*)&v, 4);
        printf("   4B [%08x]: resp=0x%04lx\n", v, resp);

        v = 0x07000001; resp = writeVendorCmd(device, 0x9103, (uint8_t*)&v, 4);
        printf("   4B [%08x]: resp=0x%04lx\n", v, resp);

        // Check 0x9104 after write
        resp = readVendorCmd(device, 0x9104, buf, &bufSize);
        printf("   0x9104 after: resp=0x%04lx, %zu bytes\n", resp, bufSize);
        if (bufSize > 0) hexdump(buf, bufSize, 4);
    }

    // Step 4: 0x9103 with params + data
    printf("\n=== 0x9103 with PTP params + data ===\n");
    {
        uint32_t v = 0;
        resp = writeVendorCmd(device, 0x9103, (uint8_t*)&v, 4, 1, 0x07000001);
        printf("   p1=0x07000001 data=[0]: resp=0x%04lx\n", resp);
    }

    // Step 5: 0x9103 data-to-read
    printf("\n=== 0x9103 data-to-read ===\n");
    {
        resp = readVendorCmd(device, 0x9103, buf, &bufSize, 1, 0x07000001);
        printf("   p1=0x07000001: resp=0x%04lx, %zu bytes\n", resp, bufSize);
        if (bufSize > 0) hexdump(buf, bufSize < 512 ? bufSize : 512, 32);
    }

    // Step 6: Battery (standard PTP, to verify WPD is working)
    printf("\n=== Standard PTP: Battery (0x1015/0x5001) ===\n");
    resp = readVendorCmd(device, 0x1015, buf, &bufSize, 1, 0x5001);
    printf("   resp=0x%04lx, %zu bytes:", resp, bufSize);
    for (size_t i = 0; i < bufSize; i++) printf(" %02x", buf[i]);
    printf("\n");

    device->Close();
    device->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
