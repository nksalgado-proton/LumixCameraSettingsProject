/**
 * wpd_getprop — Probe Panasonic GetProperty (0x9402) and ListProperty (0x9108)
 *
 * Camera supports 0x9402/0x9403/0x9108 even though GetDeviceInfo doesn't list them.
 * The property code goes as PTP parameter 1.
 * Data phase returns property value.
 *
 * IMPORTANT: Do NOT send 0x9103 (CloseSession) with data — it kills the connection!
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

IPortableDevice* openCamera() {
    IPortableDeviceManager* mgr = nullptr;
    CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER,
                     IID_IPortableDeviceManager, (void**)&mgr);
    if (!mgr) return nullptr;

    DWORD cnt = 0;
    mgr->GetDevices(nullptr, &cnt);
    if (!cnt) { mgr->Release(); return nullptr; }

    PWSTR* ids = new PWSTR[cnt];
    mgr->GetDevices(ids, &cnt);
    PWSTR camId = nullptr;
    for (DWORD i = 0; i < cnt; i++)
        if (wcsstr(ids[i], L"vid_04da")) { camId = ids[i]; break; }
    if (!camId) { for (DWORD i=0;i<cnt;i++) CoTaskMemFree(ids[i]); delete[] ids; mgr->Release(); return nullptr; }

    IPortableDevice* dev = nullptr;
    CoCreateInstance(CLSID_PortableDevice, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDevice, (void**)&dev);
    IPortableDeviceValues* ci = nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&ci);
    ci->SetStringValue(WPD_CLIENT_NAME, L"XLumix");
    ci->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
    ci->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
    ci->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 0);
    ci->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ | GENERIC_WRITE);
    HRESULT hr = dev->Open(camId, ci);
    ci->Release();
    for (DWORD i=0;i<cnt;i++) CoTaskMemFree(ids[i]); delete[] ids; mgr->Release();
    if (FAILED(hr)) { dev->Release(); return nullptr; }
    return dev;
}

// Send MTP command with data-to-read, return response code
ULONG mtpRead(IPortableDevice* dev, UINT16 op, uint8_t* buf, size_t* sz,
              UINT32 np=0, UINT32 p1=0, UINT32 p2=0, UINT32 p3=0) {
    *sz = 0;
    ULONG resp = 0;
    IPortableDeviceValues *p=nullptr, *r=nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&p);
    p->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)op);
    IPortableDevicePropVariantCollection *pc=nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDevicePropVariantCollection, (void**)&pc);
    PROPVARIANT pv; PropVariantInit(&pv); pv.vt=VT_UI4;
    UINT32 pvals[]={p1,p2,p3};
    for(UINT32 i=0;i<np;i++){pv.ulVal=pvals[i];pc->Add(&pv);}
    p->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, pc);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, 0);
    HRESULT hr = dev->SendCommand(0, p, &r);
    if (SUCCEEDED(hr) && r) {
        HRESULT ch=S_OK; r->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &ch);
        r->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &resp);
        LPWSTR ctx=nullptr;
        if (SUCCEEDED(r->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &ctx)) && ctx) {
            IPortableDeviceValues *rp=nullptr, *rr=nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&rp);
            rp->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_READ_DATA.fmtid);
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_READ_DATA.pid);
            rp->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, ctx);
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, 65536);
            BYTE* tmp=new BYTE[65536]; memset(tmp,0,65536);
            rp->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, tmp, 65536); delete[] tmp;
            hr = dev->SendCommand(0, rp, &rr);
            if (SUCCEEDED(hr) && rr) {
                BYTE* d=nullptr; ULONG ds=0;
                if (SUCCEEDED(rr->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &d, &ds)) && d && ds) {
                    memcpy(buf, d, ds<65536?ds:65536); *sz=ds; CoTaskMemFree(d);
                }
                rr->Release();
            }
            rp->Release();
            // End
            IPortableDeviceValues *ep=nullptr, *er=nullptr;
            CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&ep);
            ep->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
            ep->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
            ep->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, ctx);
            dev->SendCommand(0, ep, &er);
            if (er) { ULONG er2=0; er->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE,&er2); if(er2) resp=er2; er->Release(); }
            ep->Release();
            CoTaskMemFree(ctx);
        }
        r->Release();
    }
    pc->Release(); p->Release();
    return resp;
}

// No-data command
ULONG mtpNoData(IPortableDevice* dev, UINT16 op, UINT32 np=0, UINT32 p1=0) {
    ULONG resp=0;
    IPortableDeviceValues *p=nullptr, *r=nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&p);
    p->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)op);
    IPortableDevicePropVariantCollection *pc=nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDevicePropVariantCollection, (void**)&pc);
    if(np>=1){PROPVARIANT pv; PropVariantInit(&pv); pv.vt=VT_UI4; pv.ulVal=p1; pc->Add(&pv);}
    p->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, pc);
    HRESULT hr = dev->SendCommand(0, p, &r);
    if (SUCCEEDED(hr) && r) { r->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &resp); r->Release(); }
    pc->Release(); p->Release();
    return resp;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IPortableDevice* dev = openCamera();
    if (!dev) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n");

    uint8_t buf[65536];
    size_t sz = 0;

    // Verify connection with battery read
    printf("\n=== Battery (standard PTP 0x1015/0x5001) ===\n");
    ULONG resp = mtpRead(dev, 0x1015, buf, &sz, 1, 0x5001);
    printf("   resp=0x%04lx, %zu bytes:", resp, sz);
    for (size_t i = 0; i < sz; i++) printf(" %02x", buf[i]);
    printf("\n");

    // Confirm vendor session
    printf("\n=== Vendor session (0x9102) ===\n");
    resp = mtpNoData(dev, 0x9102, 1, 0x00010001);
    printf("   resp=0x%04lx\n", resp);

    // Test 0x9402 (GetProperty) with standard PTP property codes
    printf("\n=== 0x9402 GetProperty — standard PTP codes ===\n");
    {
        struct { UINT32 code; const char* name; } props[] = {
            {0x5001, "Battery"}, {0x5003, "ImageSize"}, {0x5004, "CompressionSetting"},
            {0x5005, "WhiteBalance"}, {0x5007, "FNumber"}, {0x500a, "FocusMode"},
            {0x500c, "FlashMode"}, {0x500d, "ExposureTime"}, {0x500e, "ExposureProgram"},
            {0x500f, "ExposureIndex(ISO)"}, {0x5013, "StillCaptureMode"},
            {0xD001, "VendorProp1"}, {0xD002, "VendorProp2"},
        };
        for (auto& pr : props) {
            resp = mtpRead(dev, 0x9402, buf, &sz, 1, pr.code);
            printf("   0x%04x %-20s: resp=0x%04lx %zuB", pr.code, pr.name, resp, sz);
            if (sz > 0 && sz <= 32) { printf(" ="); for(size_t i=0;i<sz;i++) printf(" %02x",buf[i]); }
            printf("\n");
            if (sz > 32) hexdump(buf, sz, 4);
        }
    }

    // Test 0x9402 with Panasonic-style property codes (high 32-bit values)
    printf("\n=== 0x9402 GetProperty — Panasonic-style codes ===\n");
    {
        UINT32 codes[] = {
            // Try different high-byte prefixes
            0x02000010, 0x02000020, 0x02000030, 0x02000040, 0x02000050,
            0x03000010, 0x03000020, 0x03000030, 0x03000040,
            0x04000010, 0x04000020,
            0x05000010, 0x05000020,
            0x06000010, 0x06000020,
            0x07000001, 0x07000002, 0x07000003, 0x07000004,
            0x08000001, 0x08000002, 0x08000003, 0x08000004,
            0x0D000010, 0x0D000020, 0x0D000030,
            // Try smaller values
            1, 2, 3, 4, 5, 0x100, 0x200, 0x300, 0x400, 0x500,
        };
        for (UINT32 code : codes) {
            resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
            if (resp != 0x2002 && resp != 0x2005 && resp != 0xFFFF) {
                printf("   0x%08x: resp=0x%04lx %zuB", code, resp, sz);
                if (sz > 0 && sz <= 32) { printf(" ="); for(size_t i=0;i<sz;i++) printf(" %02x",buf[i]); }
                printf(" ***\n");
                if (sz > 32) hexdump(buf, sz, 8);
            } else if (resp == 0x2002) {
                printf("   0x%08x: GeneralError\n", code);
            }
        }
    }

    // Test 0x9108 (ListProperty)
    printf("\n=== 0x9108 ListProperty ===\n");
    {
        UINT32 codes[] = {0x5001, 0x5007, 0x500f, 0xD001,
                          0x02000010, 0x07000001, 1, 0};
        for (UINT32 code : codes) {
            resp = mtpRead(dev, 0x9108, buf, &sz, 1, code);
            if (resp != 0x2005 && resp != 0xFFFF) {
                printf("   0x%08x: resp=0x%04lx %zuB\n", code, resp, sz);
                if (sz > 0) hexdump(buf, sz < 256 ? sz : 256, 16);
            }
        }
    }

    // Test 0x9401 (no-data, various params)
    printf("\n=== 0x9401 no-data ===\n");
    resp = mtpNoData(dev, 0x9401, 0);
    printf("   no params: resp=0x%04lx\n", resp);
    resp = mtpNoData(dev, 0x9401, 1, 1);
    printf("   p1=1: resp=0x%04lx\n", resp);

    // Test 0x9401 data-to-read
    printf("\n=== 0x9401 data-to-read ===\n");
    resp = mtpRead(dev, 0x9401, buf, &sz, 0);
    printf("   no params: resp=0x%04lx %zuB\n", resp, sz);
    if (sz > 0) hexdump(buf, sz < 512 ? sz : 512, 32);

    resp = mtpRead(dev, 0x9401, buf, &sz, 1, 1);
    printf("   p1=1: resp=0x%04lx %zuB\n", resp, sz);
    if (sz > 0) hexdump(buf, sz < 512 ? sz : 512, 32);

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
