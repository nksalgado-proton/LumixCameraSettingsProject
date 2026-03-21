/**
 * wpd_vendor — Probe Panasonic vendor PTP operations via WPD
 *
 * Camera supports 3 vendor ops: 0x9102, 0x9103, 0x9104
 * 0x9104 returned OK — probe it with various parameters to find
 * Fn button assignments and other settings data.
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
    if (FAILED(hr)) return nullptr;

    DWORD deviceCount = 0;
    mgr->GetDevices(nullptr, &deviceCount);
    if (deviceCount == 0) { mgr->Release(); return nullptr; }

    PWSTR* deviceIds = new PWSTR[deviceCount];
    mgr->GetDevices(deviceIds, &deviceCount);

    PWSTR cameraId = nullptr;
    for (DWORD i = 0; i < deviceCount; i++) {
        if (wcsstr(deviceIds[i], L"vid_04da")) {
            cameraId = deviceIds[i];
            break;
        }
    }

    if (!cameraId) {
        for (DWORD i = 0; i < deviceCount; i++) CoTaskMemFree(deviceIds[i]);
        delete[] deviceIds;
        mgr->Release();
        return nullptr;
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
    delete[] deviceIds;
    mgr->Release();

    if (FAILED(hr)) { device->Release(); return nullptr; }
    return device;
}

// Send MTP command with data to read back
bool sendReadCommand(IPortableDevice* device, UINT16 opCode,
                     uint8_t* outBuf, size_t* outSize,
                     UINT32 numParams = 0, UINT32 p1 = 0, UINT32 p2 = 0, UINT32 p3 = 0, UINT32 p4 = 0, UINT32 p5 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    HRESULT hr;
    *outSize = 0;

    hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceValues, (void**)&params);
    if (FAILED(hr)) return false;

    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* paramColl = nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevicePropVariantCollection, (void**)&paramColl);
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_UI4;
    UINT32 pvals[] = {p1, p2, p3, p4, p5};
    for (UINT32 i = 0; i < numParams; i++) {
        pv.ulVal = pvals[i];
        paramColl->Add(&pv);
    }
    params->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, paramColl);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, 0);

    hr = device->SendCommand(0, params, &results);

    bool success = false;
    if (SUCCEEDED(hr) && results) {
        ULONG responseCode = 0;
        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &responseCode);

        LPWSTR transferCtx = nullptr;
        hr = results->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &transferCtx);
        if (SUCCEEDED(hr) && transferCtx) {
            // Read data
            IPortableDeviceValues* readParams = nullptr;
            IPortableDeviceValues* readResults = nullptr;

            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&readParams);

            readParams->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                                     WPD_COMMAND_MTP_EXT_READ_DATA.fmtid);
            readParams->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                                WPD_COMMAND_MTP_EXT_READ_DATA.pid);
            readParams->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            readParams->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, 65536);

            BYTE* tmpBuf = new BYTE[65536];
            memset(tmpBuf, 0, 65536);
            readParams->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, tmpBuf, 65536);
            delete[] tmpBuf;

            hr = device->SendCommand(0, readParams, &readResults);
            if (SUCCEEDED(hr) && readResults) {
                BYTE* data = nullptr;
                ULONG dataSize = 0;
                hr = readResults->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &data, &dataSize);
                if (SUCCEEDED(hr) && data && dataSize > 0) {
                    memcpy(outBuf, data, dataSize < 65536 ? dataSize : 65536);
                    *outSize = dataSize;
                    success = true;
                    CoTaskMemFree(data);
                }
                readResults->Release();
            }
            readParams->Release();

            // End transfer
            IPortableDeviceValues* endParams = nullptr;
            IPortableDeviceValues* endResults = nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                             IID_IPortableDeviceValues, (void**)&endParams);
            endParams->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                                    WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
            endParams->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                               WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
            endParams->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, transferCtx);
            device->SendCommand(0, endParams, &endResults);
            if (endResults) endResults->Release();
            endParams->Release();

            CoTaskMemFree(transferCtx);
        } else {
            // No transfer context — check response code
            printf("   (no data phase, response=0x%04lx)\n", responseCode);
        }
        results->Release();
    }

    if (paramColl) paramColl->Release();
    params->Release();
    return success;
}

// Send MTP command without data phase
ULONG sendNoDataCommand(IPortableDevice* device, UINT16 opCode,
                         UINT32 numParams = 0, UINT32 p1 = 0, UINT32 p2 = 0, UINT32 p3 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    HRESULT hr;
    ULONG responseCode = 0;

    hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceValues, (void**)&params);
    if (FAILED(hr)) return 0;

    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* paramColl = nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDevicePropVariantCollection, (void**)&paramColl);
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_UI4;
    UINT32 pvals[] = {p1, p2, p3};
    for (UINT32 i = 0; i < numParams; i++) {
        pv.ulVal = pvals[i];
        paramColl->Add(&pv);
    }
    params->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, paramColl);

    hr = device->SendCommand(0, params, &results);
    if (SUCCEEDED(hr) && results) {
        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &responseCode);

        IPortableDevicePropVariantCollection* respParams = nullptr;
        hr = results->GetIPortableDevicePropVariantCollectionValue(
            WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &respParams);
        if (SUCCEEDED(hr) && respParams) {
            DWORD count = 0;
            respParams->GetCount(&count);
            if (count > 0) {
                printf("   response params:");
                for (DWORD i = 0; i < count; i++) {
                    PROPVARIANT rpv;
                    PropVariantInit(&rpv);
                    respParams->GetAt(i, &rpv);
                    if (rpv.vt == VT_UI4) printf(" 0x%lx", rpv.ulVal);
                    PropVariantClear(&rpv);
                }
                printf("\n");
            }
            respParams->Release();
        }
        results->Release();
    }

    if (paramColl) paramColl->Release();
    params->Release();
    return responseCode;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    IPortableDevice* device = openCamera();
    if (!device) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n");

    uint8_t buf[65536];
    size_t bufSize = 0;

    // First, get DeviceInfo to understand what we're working with
    printf("\n=== PTP GetDeviceInfo (0x1001) ===\n");
    if (sendReadCommand(device, 0x1001, buf, &bufSize)) {
        printf("   Got %zu bytes\n", bufSize);

        // Parse supported operations from DeviceInfo
        // DeviceInfo format: StandardVersion(2), VendorExtensionID(4), VendorExtensionVersion(2),
        // VendorExtensionDesc(string), FunctionalMode(2), OperationsSupported(array), ...
        if (bufSize > 10) {
            uint16_t stdVer = *(uint16_t*)buf;
            uint32_t vendorId = *(uint32_t*)(buf + 2);
            uint16_t vendorVer = *(uint16_t*)(buf + 6);
            printf("   PTP version: %d.%d, Vendor ID: 0x%08x, Vendor ver: %d\n",
                   stdVer / 100, stdVer % 100, vendorId, vendorVer);

            // Skip vendor extension desc string
            size_t offset = 8;
            if (offset < bufSize) {
                uint8_t strLen = buf[offset];
                offset += 1 + strLen * 2; // PTP string: length byte + UTF-16 chars
            }

            // Functional mode
            if (offset + 2 <= bufSize) {
                uint16_t funcMode = *(uint16_t*)(buf + offset);
                printf("   Functional mode: 0x%04x\n", funcMode);
                offset += 2;
            }

            // Operations supported array
            if (offset + 4 <= bufSize) {
                uint32_t numOps = *(uint32_t*)(buf + offset);
                offset += 4;
                printf("   Supported operations (%u):\n   ", numOps);
                for (uint32_t i = 0; i < numOps && offset + 2 <= bufSize; i++) {
                    uint16_t op = *(uint16_t*)(buf + offset);
                    printf(" 0x%04x", op);
                    offset += 2;
                }
                printf("\n");
            }

            // Events supported array
            if (offset + 4 <= bufSize) {
                uint32_t numEvts = *(uint32_t*)(buf + offset);
                offset += 4;
                printf("   Supported events (%u):\n   ", numEvts);
                for (uint32_t i = 0; i < numEvts && offset + 2 <= bufSize; i++) {
                    uint16_t evt = *(uint16_t*)(buf + offset);
                    printf(" 0x%04x", evt);
                    offset += 2;
                }
                printf("\n");
            }

            // Device properties supported
            if (offset + 4 <= bufSize) {
                uint32_t numProps = *(uint32_t*)(buf + offset);
                offset += 4;
                printf("   Supported device properties (%u):\n   ", numProps);
                for (uint32_t i = 0; i < numProps && offset + 2 <= bufSize; i++) {
                    uint16_t prop = *(uint16_t*)(buf + offset);
                    printf(" 0x%04x", prop);
                    offset += 2;
                }
                printf("\n");
            }
        }
    }

    // Try 0x9104 with various parameters
    printf("\n=== Probe vendor op 0x9104 ===\n");

    // Try with no params
    printf("\n--- 0x9104 no params (data read) ---\n");
    if (sendReadCommand(device, 0x9104, buf, &bufSize, 0)) {
        printf("   Got %zu bytes:\n", bufSize);
        hexdump(buf, bufSize < 512 ? bufSize : 512, 32);
    }

    // Try with 1 param - various values
    printf("\n--- 0x9104 with 1 param ---\n");
    UINT32 testParams[] = {0, 1, 2, 3, 4, 5, 0x0100, 0x0200, 0x0300, 0x0400,
                            0x1000, 0x2000, 0x3000, 0x5000, 0x9000,
                            0xD000, 0xD001, 0xD002, 0xD003, 0xD100, 0xD200,
                            0x0001D000, 0x0002D000, 0x07000000};
    for (int i = 0; i < sizeof(testParams)/sizeof(testParams[0]); i++) {
        printf("0x9104(0x%08x): ", testParams[i]);
        if (sendReadCommand(device, 0x9104, buf, &bufSize, 1, testParams[i])) {
            printf("   %zu bytes:\n", bufSize);
            hexdump(buf, bufSize < 256 ? bufSize : 256, 16);
        }
    }

    // Try 0x9103 (returned 0x2003 before)
    printf("\n=== Probe vendor op 0x9103 ===\n");
    printf("0x9103 no params: ");
    if (sendReadCommand(device, 0x9103, buf, &bufSize, 0)) {
        printf("   %zu bytes:\n", bufSize);
        hexdump(buf, bufSize < 512 ? bufSize : 512, 32);
    }
    printf("0x9103(0): ");
    if (sendReadCommand(device, 0x9103, buf, &bufSize, 1, 0)) {
        printf("   %zu bytes:\n", bufSize);
        hexdump(buf, bufSize < 256 ? bufSize : 256, 16);
    }
    printf("0x9103(1): ");
    if (sendReadCommand(device, 0x9103, buf, &bufSize, 1, 1)) {
        printf("   %zu bytes:\n", bufSize);
        hexdump(buf, bufSize < 256 ? bufSize : 256, 16);
    }

    // Try 0x9102 (Open Session variant)
    printf("\n=== Probe vendor op 0x9102 ===\n");
    printf("0x9102 no-data: resp=0x%04lx\n", sendNoDataCommand(device, 0x9102, 0));
    printf("0x9102(1) no-data: resp=0x%04lx\n", sendNoDataCommand(device, 0x9102, 1, 1));
    printf("0x9102(0x10001) no-data: resp=0x%04lx\n", sendNoDataCommand(device, 0x9102, 1, 0x10001));

    // Try standard PTP GetDevicePropValue for various properties
    printf("\n=== PTP GetDevicePropValue (0x1015) ===\n");
    UINT16 props[] = {0x5001, 0x5003, 0x5004, 0x5005, 0x5007, 0x500C, 0x500D, 0x500E, 0x500F,
                       0xD001, 0xD002, 0xD003, 0xD010, 0xD011, 0xD100, 0xD200, 0xD201};
    for (int i = 0; i < sizeof(props)/sizeof(props[0]); i++) {
        printf("GetDeviceProp(0x%04x): ", props[i]);
        if (sendReadCommand(device, 0x1015, buf, &bufSize, 1, props[i])) {
            printf("   %zu bytes:", bufSize);
            for (size_t j = 0; j < bufSize && j < 16; j++) printf(" %02x", buf[j]);
            printf("\n");
        }
    }

    // Also try GetDevicePropDesc (0x1014)
    printf("\n=== PTP GetDevicePropDesc (0x1014) ===\n");
    for (int i = 0; i < sizeof(props)/sizeof(props[0]); i++) {
        printf("GetDevicePropDesc(0x%04x): ", props[i]);
        if (sendReadCommand(device, 0x1014, buf, &bufSize, 1, props[i])) {
            printf("   %zu bytes\n", bufSize);
            hexdump(buf, bufSize < 128 ? bufSize : 128, 8);
        }
    }

    device->Close();
    device->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
