/**
 * wpd_test — Direct WPD/MTP access to camera, bypassing both DLLs
 *
 * Uses Windows Portable Devices API to:
 * 1. Enumerate PTP devices
 * 2. Open the camera
 * 3. Get device info (supported operations, vendor extensions)
 * 4. Send MTP vendor commands to read Fn button assignments
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

// Send an MTP vendor command and read response data
HRESULT sendMTPCommand(IPortableDevice* device, UINT16 opCode, UINT32 param1 = 0, UINT32 param2 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    IPortableDevicePropVariantCollection* opcodeCollection = nullptr;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceValues, (void**)&params);
    if (FAILED(hr)) return hr;

    // Set the MTP command
    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    // Create parameter collection
    hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDevicePropVariantCollection, (void**)&opcodeCollection);
    if (SUCCEEDED(hr)) {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        pv.vt = VT_UI4;
        pv.ulVal = param1;
        opcodeCollection->Add(&pv);
        pv.ulVal = param2;
        opcodeCollection->Add(&pv);

        params->SetIPortableDevicePropVariantCollectionValue(
            WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, opcodeCollection);
    }

    // Set transfer size hint
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, 0);

    // Send command
    hr = device->SendCommand(0, params, &results);

    if (SUCCEEDED(hr) && results) {
        ULONG responseCode = 0;
        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &responseCode);
        printf("   MTP 0x%04x: response=0x%04lx\n", opCode, responseCode);

        // Check for transfer context
        LPWSTR transferCtx = nullptr;
        hr = results->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &transferCtx);
        if (SUCCEEDED(hr) && transferCtx) {
            printf("   Transfer context obtained, reading data...\n");

            // Read the data
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
            readParams->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, new BYTE[65536], 65536);

            hr = device->SendCommand(0, readParams, &readResults);
            if (SUCCEEDED(hr) && readResults) {
                BYTE* data = nullptr;
                ULONG dataSize = 0;
                hr = readResults->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &data, &dataSize);
                if (SUCCEEDED(hr) && data && dataSize > 0) {
                    printf("   Received %lu bytes:\n", dataSize);
                    hexdump(data, dataSize, 32);
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
        }
        results->Release();
    }

    if (opcodeCollection) opcodeCollection->Release();
    params->Release();
    return hr;
}

// Send a simple MTP command (no data phase)
HRESULT sendMTPCommandNoData(IPortableDevice* device, UINT16 opCode,
                              UINT32 param1 = 0, UINT32 param2 = 0, UINT32 param3 = 0) {
    IPortableDeviceValues* params = nullptr;
    IPortableDeviceValues* results = nullptr;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceValues, (void**)&params);
    if (FAILED(hr)) return hr;

    params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                         WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                    WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)opCode);

    IPortableDevicePropVariantCollection* opcodeCollection = nullptr;
    hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDevicePropVariantCollection, (void**)&opcodeCollection);
    if (SUCCEEDED(hr)) {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        pv.vt = VT_UI4;
        if (param1) { pv.ulVal = param1; opcodeCollection->Add(&pv); }
        if (param2) { pv.ulVal = param2; opcodeCollection->Add(&pv); }
        if (param3) { pv.ulVal = param3; opcodeCollection->Add(&pv); }
        params->SetIPortableDevicePropVariantCollectionValue(
            WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, opcodeCollection);
    }

    hr = device->SendCommand(0, params, &results);

    if (SUCCEEDED(hr) && results) {
        ULONG responseCode = 0;
        results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &responseCode);
        printf("   MTP 0x%04x: response=0x%04lx", opCode, responseCode);

        // Get response params
        IPortableDevicePropVariantCollection* respParams = nullptr;
        hr = results->GetIPortableDevicePropVariantCollectionValue(
            WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &respParams);
        if (SUCCEEDED(hr) && respParams) {
            DWORD count = 0;
            respParams->GetCount(&count);
            for (DWORD i = 0; i < count; i++) {
                PROPVARIANT pv;
                PropVariantInit(&pv);
                respParams->GetAt(i, &pv);
                if (pv.vt == VT_UI4) printf(" p%lu=0x%lx", i, pv.ulVal);
                PropVariantClear(&pv);
            }
            respParams->Release();
        }
        printf("\n");
        results->Release();
    }

    if (opcodeCollection) opcodeCollection->Release();
    params->Release();
    return hr;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    printf("COM: 0x%08lx\n", hr);

    // Enumerate WPD devices
    IPortableDeviceManager* mgr = nullptr;
    hr = CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceManager, (void**)&mgr);
    if (FAILED(hr)) { printf("Failed to create device manager: 0x%08lx\n", hr); return 1; }

    DWORD deviceCount = 0;
    mgr->GetDevices(nullptr, &deviceCount);
    printf("WPD devices: %lu\n", deviceCount);

    if (deviceCount == 0) { printf("No devices!\n"); return 1; }

    PWSTR* deviceIds = new PWSTR[deviceCount];
    mgr->GetDevices(deviceIds, &deviceCount);

    PWSTR cameraId = nullptr;
    for (DWORD i = 0; i < deviceCount; i++) {
        DWORD nameLen = 0;
        mgr->GetDeviceFriendlyName(deviceIds[i], nullptr, &nameLen);
        WCHAR* name = new WCHAR[nameLen + 1];
        mgr->GetDeviceFriendlyName(deviceIds[i], name, &nameLen);
        wprintf(L"  Device %lu: %s\n", i, name);
        wprintf(L"    ID: %s\n", deviceIds[i]);

        // Check if this is our Panasonic camera
        if (wcsstr(deviceIds[i], L"vid_04da") || wcsstr(name, L"LUMIX") ||
            wcsstr(name, L"Panasonic") || wcsstr(name, L"DC-G9")) {
            cameraId = deviceIds[i];
            printf("  ^^^ This is our camera! ^^^\n");
        }
        delete[] name;
    }

    if (!cameraId) {
        printf("Camera not found in WPD devices!\n");
        return 1;
    }

    // Open the device
    printf("\nOpening device...\n");
    IPortableDevice* device = nullptr;
    hr = CoCreateInstance(CLSID_PortableDevice, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPortableDevice, (void**)&device);
    if (FAILED(hr)) { printf("Failed to create device: 0x%08lx\n", hr); return 1; }

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

    if (FAILED(hr)) {
        printf("Failed to open device: 0x%08lx\n", hr);
        // Try read-only
        CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                         IID_IPortableDeviceValues, (void**)&clientInfo);
        clientInfo->SetStringValue(WPD_CLIENT_NAME, L"XLumixG9Settings");
        clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
        clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
        clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 0);
        clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
        hr = device->Open(cameraId, clientInfo);
        clientInfo->Release();
        if (FAILED(hr)) {
            printf("Read-only open also failed: 0x%08lx\n", hr);
            return 1;
        }
        printf("Opened read-only.\n");
    } else {
        printf("Device opened successfully!\n");
    }

    // Get supported MTP operations via GetDeviceInfo (0x1001)
    printf("\n=== PTP GetDeviceInfo (0x1001) ===\n");
    sendMTPCommand(device, 0x1001);

    // Try Panasonic vendor operations
    // Common Panasonic PTP vendor codes (discovered from gphoto2 and other sources)
    printf("\n=== Scanning Panasonic vendor operations ===\n");

    // Known Panasonic vendor operation codes:
    // 0x9101 - GetObjectInfo (vendor extension?)
    // 0x9401-0x9499 - Common vendor range
    // Let's scan a range to find supported operations

    // First try some known codes
    UINT16 vendorOps[] = {
        0x9101, 0x9102, 0x9103, 0x9104, 0x9105,
        0x9401, 0x9402, 0x9403, 0x9404, 0x9405, 0x9406, 0x9407, 0x9408, 0x9409,
        0x9410, 0x9411, 0x9412, 0x9413, 0x9414, 0x9415,
        0x940A, 0x940B, 0x940C, 0x940D, 0x940E, 0x940F,
    };

    printf("\nTesting known vendor operation codes:\n");
    for (int i = 0; i < sizeof(vendorOps)/sizeof(vendorOps[0]); i++) {
        sendMTPCommandNoData(device, vendorOps[i]);
    }

    // Broader scan in 0x9000-0x9FFF range (sample every 0x10)
    printf("\nScanning vendor operations 0x9000-0x9FFF:\n");
    for (UINT16 op = 0x9000; op <= 0x9FFF; op += 0x01) {
        // Just test if the operation is supported (response != 0x2005 "Operation Not Supported")
        IPortableDeviceValues* params = nullptr;
        IPortableDeviceValues* results = nullptr;

        CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER,
                         IID_IPortableDeviceValues, (void**)&params);

        params->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                             WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
        params->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                        WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
        params->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)op);

        IPortableDevicePropVariantCollection* opcodeCollection = nullptr;
        CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER,
                         IID_IPortableDevicePropVariantCollection, (void**)&opcodeCollection);
        params->SetIPortableDevicePropVariantCollectionValue(
            WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, opcodeCollection);

        hr = device->SendCommand(0, params, &results);

        if (SUCCEEDED(hr) && results) {
            ULONG responseCode = 0;
            results->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &responseCode);
            // 0x2001 = OK, 0x2005 = Operation Not Supported, 0x2019 = Invalid Transaction ID
            // Anything other than 0x2005 means the operation is recognized
            if (responseCode != 0x2005 && responseCode != 0) {
                printf("   0x%04x: response=0x%04lx ***\n", op, responseCode);
            }
            results->Release();
        }

        if (opcodeCollection) opcodeCollection->Release();
        params->Release();
    }

    printf("\nDone scanning.\n");

    device->Close();
    device->Release();
    mgr->Release();

    for (DWORD i = 0; i < deviceCount; i++)
        CoTaskMemFree(deviceIds[i]);
    delete[] deviceIds;

    CoUninitialize();
    return 0;
}
