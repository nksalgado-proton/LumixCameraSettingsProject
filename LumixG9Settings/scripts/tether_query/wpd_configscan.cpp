/**
 * wpd_configscan — Try to find Panasonic setup config via various PTP mechanisms
 *
 * Approach 1: PTP GetNumObjects/GetObjectHandles to see if config is a PTP object
 * Approach 2: Vendor ops 0x9400-0x9500 with various parameter patterns
 * Approach 3: 0x9402 with 3 parameters (maybe config needs extra addressing)
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

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 16) {
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
    if (!camId) { for(DWORD i=0;i<cnt;i++) CoTaskMemFree(ids[i]); delete[] ids; mgr->Release(); return nullptr; }
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
    for(DWORD i=0;i<cnt;i++) CoTaskMemFree(ids[i]); delete[] ids; mgr->Release();
    if (FAILED(hr)) { dev->Release(); return nullptr; }
    return dev;
}

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

ULONG mtpNoData(IPortableDevice* dev, UINT16 op, UINT32 np=0, UINT32 p1=0, UINT32 p2=0, UINT32 p3=0) {
    ULONG resp=0;
    IPortableDeviceValues *p=nullptr, *r=nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&p);
    p->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)op);
    IPortableDevicePropVariantCollection *pc=nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDevicePropVariantCollection, (void**)&pc);
    PROPVARIANT pv; PropVariantInit(&pv); pv.vt=VT_UI4;
    UINT32 pvals[]={p1,p2,p3};
    for(UINT32 i=0;i<np;i++){pv.ulVal=pvals[i];pc->Add(&pv);}
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

    // Open vendor session
    ULONG resp = mtpNoData(dev, 0x9102, 1, 0x00010001);
    printf("Vendor session: 0x%04lx\n", resp);

    // 1. Try standard PTP GetNumObjects (0x1006) and GetObjectHandles (0x1007)
    printf("\n=== PTP Object Enumeration ===\n");
    {
        // GetNumObjects: params = StorageID, ObjectFormatCode, Parent
        resp = mtpRead(dev, 0x1006, buf, &sz, 3, 0xFFFFFFFF, 0, 0);
        printf("  GetNumObjects(all): resp=0x%04lx %zuB\n", resp, sz);
        if (sz > 0) hexdump(buf, sz, 4);

        // GetObjectHandles
        resp = mtpRead(dev, 0x1007, buf, &sz, 3, 0xFFFFFFFF, 0, 0);
        printf("  GetObjectHandles(all): resp=0x%04lx %zuB\n", resp, sz);
        if (sz > 0) hexdump(buf, sz, 8);

        // Try storage-specific
        resp = mtpRead(dev, 0x1004, buf, &sz, 0); // GetStorageIDs
        printf("  GetStorageIDs: resp=0x%04lx %zuB\n", resp, sz);
        if (sz > 0) hexdump(buf, sz, 4);
    }

    // 2. Try 0x9402 with 3 parameters — maybe config needs a "section" param
    printf("\n=== 0x9402 with 3 params ===\n");
    {
        // Try: (group_code, section_id, sub_id)
        UINT32 tests[][3] = {
            {0x02000080, 0, 0},
            {0x02000080, 1, 0},
            {0x02000080, 0x0100, 0},
            {0x02000080, 0xFFFF, 0},
            {1, 0, 0},             // section 1?
            {2, 0, 0},             // section 2?
            {0x0100, 0, 0},        // config section?
            {0x0200, 0, 0},
            {0x0300, 0, 0},
            {0x0400, 0, 0},        // Fn section?
            {0x0500, 0, 0},
        };
        for (auto& t : tests) {
            resp = mtpRead(dev, 0x9402, buf, &sz, 3, t[0], t[1], t[2]);
            if (resp == 0x2001 && sz > 0) {
                printf("  (0x%08x, 0x%x, 0x%x): %zuB OK!\n", t[0], t[1], t[2], sz);
                hexdump(buf, sz < 128 ? sz : 128, 8);
            }
        }
    }

    // 3. Try vendor operations 0x9405-0x940F (skip known ones)
    printf("\n=== Vendor ops 0x9405-0x9420 (no params) ===\n");
    for (UINT16 op = 0x9405; op <= 0x9420; op++) {
        resp = mtpNoData(dev, op, 0);
        if (resp != 0x2005 && resp != 0) {
            printf("  0x%04x (no-data, no params): resp=0x%04lx\n", op, resp);
        }
    }

    // 4. Try vendor operations with data-to-read, various param patterns
    printf("\n=== Vendor ops data-to-read with params ===\n");
    {
        // These ops might be config-related: try each with specific params
        UINT16 ops[] = {0x9405, 0x9406, 0x9407, 0x9408, 0x9409, 0x940a, 0x940b,
                        0x940c, 0x940d, 0x940e, 0x940f, 0x9410, 0x9411, 0x9412};
        UINT32 params[] = {0, 1, 2, 0x100, 0x200, 0x0400, 0x1000, 0x02000010, 0xFFFFFFFF};
        for (UINT16 op : ops) {
            if (op == 0x9403) continue;
            for (UINT32 param : params) {
                resp = mtpRead(dev, op, buf, &sz, 1, param);
                if (resp == 0x2001 && sz > 0) {
                    printf("  0x%04x(0x%08x): %zuB OK!\n", op, param, sz);
                    hexdump(buf, sz < 256 ? sz : 256, 16);
                }
            }
        }
    }

    // 5. Try Panasonic-specific "get all settings" pattern
    printf("\n=== Panasonic bulk read patterns ===\n");
    {
        // Some cameras support bulk reads with operation 0x9402 + specific "all" codes
        UINT32 bulk_codes[] = {
            0x02000000, 0x02FFFFFF, 0x0200FFFF, 0x020000FF,
            0x01000000, 0x03000000, 0x04000000, 0x05000000,
            0x10000000, 0x20000000, 0xFF000000,
            0x00000000, 0xFFFFFFFF,
        };
        for (UINT32 code : bulk_codes) {
            resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x9402(0x%08x): %zuB OK!\n", code, sz);
                hexdump(buf, sz < 256 ? sz : 256, 16);
            }
        }
    }

    // 6. Try 0x9107 and 0x9108 with non-0x02 prefixed codes
    printf("\n=== 0x9107/0x9108 with different prefixes ===\n");
    {
        UINT32 codes[] = {0x01000010, 0x03000010, 0x04000010, 0x05000010,
                          0x06000010, 0x07000010, 0x08000010, 0x09000010,
                          0x0A000010, 0x0B000010, 0x0C000010, 0x0D000010};
        for (UINT32 code : codes) {
            resp = mtpRead(dev, 0x9107, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x9107(0x%08x): %zuB\n", code, sz);
            }
            resp = mtpRead(dev, 0x9108, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x9108(0x%08x): %zuB\n", code, sz);
            }
        }
    }

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
