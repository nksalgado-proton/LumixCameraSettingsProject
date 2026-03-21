/**
 * wpd_fnprobe — Dump large property blocks and scan extended ranges
 *
 * Focus on finding Fn button assignments:
 * - Full hex dump of 0x020001d0 group (especially 0x020001d4 = 317 bytes)
 * - Scan 0x020002XX through 0x0200FFXX
 * - Dump 0x9107 operation data
 * - Try reading individual Fn button IDs if pattern found
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

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 128) {
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
              UINT32 np=0, UINT32 p1=0, UINT32 p2=0) {
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
    UINT32 pvals[]={p1,p2,0};
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

    mtpNoData(dev, 0x9102, 1, 0x00010001);

    // 1. Full dump of 0x020001d0 group (contains 317-byte block)
    printf("\n=== 0x020001d0 group (full dump) ===\n");
    {
        ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, 0x020001d0);
        printf("  resp=0x%04lx %zuB\n", resp, sz);
        if (sz > 0) hexdump(buf, sz);
    }

    // 2. Individual dump of 0x020001d4 (317 bytes)
    printf("\n=== 0x020001d4 individual (317B block) ===\n");
    {
        ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, 0x020001d4);
        printf("  resp=0x%04lx %zuB\n", resp, sz);
        if (sz > 0) hexdump(buf, sz);
    }

    // 3. Extended ranges 0x020002XX through 0x020009XX
    printf("\n=== Extended scan 0x0200XXYY (XX=02..09) ===\n");
    for (UINT32 hi = 0x02; hi <= 0x09; hi++) {
        for (UINT32 lo = 0x00; lo <= 0xF0; lo += 0x10) {
            UINT32 code = 0x02000000 | (hi << 8) | lo;
            ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x%08x: %4zuB\n", code, sz);
                // Show sub-properties
                size_t pos = 0;
                while (pos + 8 <= sz) {
                    uint32_t id = *(uint32_t*)(buf + pos);
                    uint16_t sc = *(uint16_t*)(buf + pos + 4);
                    if (pos + 8 + sc > sz) break;
                    printf("    0x%08x [%u]", id, sc);
                    if (sc <= 8) { printf(" ="); for(size_t i=0;i<sc;i++) printf(" %02x",buf[pos+8+i]); }
                    if (sc == 2) printf("  (=%u)", *(uint16_t*)(buf+pos+8));
                    else if (sc == 4) printf("  (=%u)", *(uint32_t*)(buf+pos+8));
                    printf("\n");
                    pos += 8 + sc;
                }
            }
        }
    }

    // 4. Even more extended 0x0200XX00 (XX=0A..FF)
    printf("\n=== Extended scan 0x0200XX00 (XX=0A..FF) ===\n");
    for (UINT32 hi = 0x0A; hi <= 0xFF; hi++) {
        UINT32 code = 0x02000000 | (hi << 8);
        ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
        if (resp == 0x2001 && sz > 0) {
            printf("  0x%08x: %4zuB\n", code, sz);
        }
        // Also try group base
        code = 0x02000000 | (hi << 8) | 0x10;
        resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
        if (resp == 0x2001 && sz > 0) {
            printf("  0x%08x: %4zuB\n", code, sz);
        }
    }

    // 5. Dump 0x9107 operation fully for 0x02000010
    printf("\n=== OP 0x9107 for various groups ===\n");
    {
        UINT32 groups[] = {0x02000010, 0x02000080, 0x020001b0, 0x020001d0, 0x020001f0,
                           0x02000110, 0x02000120};
        for (UINT32 code : groups) {
            ULONG resp = mtpRead(dev, 0x9107, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x9107(0x%08x): %zuB\n", code, sz);
                hexdump(buf, sz < 512 ? sz : 512, 32);
            }
        }
    }

    // 6. Dump ListProperty for extended groups
    printf("\n=== 0x9108 ListProperty for extended groups ===\n");
    {
        UINT32 groups[] = {0x02000110, 0x02000120, 0x020001b0, 0x020001d0, 0x020001f0};
        for (UINT32 code : groups) {
            ULONG resp = mtpRead(dev, 0x9108, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  LIST 0x%08x: %zuB\n", code, sz);
                hexdump(buf, sz < 1024 ? sz : 1024, 64);
            }
        }
    }

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
