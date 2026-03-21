/**
 * wpd_deepscan — Fine-grained scan of ALL Panasonic property codes
 *
 * Previous scan found groups at 0x02000010-0x020000e0 (step 0x10).
 * This scanner tries:
 * 1. Every 0x020000XX from 0x00-0xFF individually
 * 2. Extended ranges: 0x020001XX, 0x020002XX, etc. up to 0x0200FFXX
 * 3. Individual sub-property IDs that might not be in groups
 * 4. Alternate operation codes for Fn buttons / custom menus
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

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 8) {
    for (size_t i = 0; i < len && i / 16 < max_lines; i += 16) {
        printf("    %04zx: ", i);
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

void decode_props(const uint8_t* data, size_t len) {
    size_t pos = 0;
    while (pos + 8 <= len) {
        uint32_t id = *(uint32_t*)(data + pos);
        uint16_t sc = *(uint16_t*)(data + pos + 4);
        size_t valsz = sc;
        if (pos + 8 + valsz > len) break;
        printf("    0x%08x [%u] =", id, (unsigned)valsz);
        for (size_t i = 0; i < valsz && i < 16; i++)
            printf(" %02x", data[pos + 8 + i]);
        if (valsz > 16) printf(" ...");
        if (valsz == 2) { uint16_t v = *(uint16_t*)(data+pos+8); printf("  (=%u)", v); }
        else if (valsz == 4) { uint32_t v = *(uint32_t*)(data+pos+8); printf("  (=%u)", v); }
        printf("\n");
        pos += 8 + valsz;
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

    // Open vendor session
    mtpNoData(dev, 0x9102, 1, 0x00010001);

    // Phase 1: Scan ALL 0x020000XX from 0x00 to 0xFF (every value, not just multiples of 0x10)
    printf("\n=== Phase 1: All 0x020000XX (0x00-0xFF) ===\n");
    int found = 0;
    for (UINT32 i = 0; i <= 0xFF; i++) {
        UINT32 code = 0x02000000 | i;
        ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
        if (resp == 0x2001 && sz > 0) {
            printf("  0x%08x: %4zuB OK\n", code, sz);
            decode_props(buf, sz);
            found++;
        }
    }
    printf("  Found %d codes\n", found);

    // Phase 2: Extended ranges 0x0200XXYY
    printf("\n=== Phase 2: Extended 0x0200XX00 (XX=01..FF) ===\n");
    found = 0;
    for (UINT32 hi = 0x01; hi <= 0xFF; hi++) {
        // Try group-aligned codes
        for (UINT32 lo = 0x00; lo <= 0xF0; lo += 0x10) {
            UINT32 code = 0x02000000 | (hi << 8) | lo;
            ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x%08x: %4zuB OK\n", code, sz);
                decode_props(buf, sz);
                found++;
            }
        }
    }
    printf("  Found %d extended codes\n", found);

    // Phase 3: Try 0x9402 with 2 parameters (some cameras need param2 for sub-addressing)
    printf("\n=== Phase 3: 0x9402 with 2 params ===\n");
    {
        // Try known group + sub-index as param2
        UINT32 test_codes[][2] = {
            {0x02000080, 1}, {0x02000080, 2}, {0x02000080, 3},
            {0x020000b0, 1}, {0x020000b0, 2},
            {0x02000070, 1}, {0x02000070, 2},
        };
        for (auto& tc : test_codes) {
            ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 2, tc[0], tc[1]);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x%08x p2=%u: %zuB OK\n", tc[0], tc[1], sz);
                decode_props(buf, sz);
            }
        }
    }

    // Phase 4: Try other vendor operation codes
    printf("\n=== Phase 4: Other vendor operations ===\n");
    {
        // Try 0x9108 with a broader range (it worked for 0x02000010)
        printf("  0x9108 ListProperty scan:\n");
        for (UINT32 i = 0x60; i <= 0xF0; i += 0x10) {
            UINT32 code = 0x02000000 | i;
            ULONG resp = mtpRead(dev, 0x9108, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("    LIST 0x%08x: %4zuB\n", code, sz);
            }
        }
    }

    // Phase 5: Try to read a large block via 0x9402 with code 0 (maybe returns all properties)
    printf("\n=== Phase 5: Special codes ===\n");
    {
        UINT32 special[] = {
            0x02000000, // all properties?
            0x020000FF, // boundary
            0x02FFFFFF, // max
            0x0200FF00, // high byte range
            0x02010000, // extended
        };
        for (UINT32 code : special) {
            ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, code);
            if (resp == 0x2001 && sz > 0) {
                printf("  0x%08x: %zuB OK\n", code, sz);
                if (sz <= 64) { for(size_t i=0;i<sz;i++) printf(" %02x",buf[i]); printf("\n"); }
                else hexdump(buf, sz < 512 ? sz : 512, 32);
            } else {
                printf("  0x%08x: resp=0x%04lx\n", code, resp);
            }
        }
    }

    // Phase 6: Scan all vendor operations 0x9100-0x91FF and 0x9400-0x94FF for data reads
    printf("\n=== Phase 6: Vendor operation scan (data-to-read) ===\n");
    {
        UINT16 ranges[][2] = {{0x9100, 0x9110}, {0x9400, 0x9410}};
        for (auto& rng : ranges) {
            for (UINT16 op = rng[0]; op <= rng[1]; op++) {
                if (op == 0x9103) continue; // NEVER touch CloseSession!
                ULONG resp = mtpRead(dev, op, buf, &sz, 1, 0x02000010);
                if (resp == 0x2001 && sz > 0) {
                    printf("  OP 0x%04x(0x02000010): %zuB\n", op, sz);
                    hexdump(buf, sz < 128 ? sz : 128, 8);
                } else if (resp != 0x2005 && resp != 0x2002 && resp != 0) {
                    printf("  OP 0x%04x(0x02000010): resp=0x%04lx %zuB\n", op, resp, sz);
                }
            }
        }
    }

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
