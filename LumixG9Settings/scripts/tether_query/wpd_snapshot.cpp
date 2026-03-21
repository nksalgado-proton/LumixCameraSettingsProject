/**
 * wpd_snapshot — Save all Panasonic property values to a file
 *
 * Reads every known property via 0x9402 and writes hex values to a file.
 * Run twice (before/after a camera change) then diff the files to identify
 * which property corresponds to which setting.
 *
 * Also tries additional vendor operations (0x9400-0x94FF) and saves any data.
 *
 * Usage: wpd_snapshot [output_file]
 *   Default output: snapshot.bin
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
              UINT32 np=0, UINT32 p1=0) {
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
    if(np>=1){pv.ulVal=p1;pc->Add(&pv);}
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

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    const char* outfile = argc > 1 ? argv[1] : "snapshot.txt";

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IPortableDevice* dev = openCamera();
    if (!dev) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n");

    uint8_t buf[65536];
    size_t sz = 0;

    mtpNoData(dev, 0x9102, 1, 0x00010001);

    FILE* f = fopen(outfile, "w");
    if (!f) { printf("Cannot open %s\n", outfile); return 1; }

    // All known group codes
    UINT32 groups[] = {
        0x02000010, 0x02000020, 0x02000030, 0x02000040, 0x02000050,
        0x02000060, 0x02000070, 0x02000080, 0x020000a0, 0x020000b0,
        0x020000c0, 0x020000e0,
        0x02000110, 0x02000120, 0x02000130, 0x02000140, 0x02000170,
        0x020001a0, 0x020001b0, 0x020001c0, 0x020001d0, 0x020001f0,
    };

    fprintf(f, "=== 0x9402 GetProperty snapshot ===\n");
    int totalProps = 0;

    for (UINT32 group : groups) {
        ULONG resp = mtpRead(dev, 0x9402, buf, &sz, 1, group);
        if (resp != 0x2001 || sz == 0) continue;

        fprintf(f, "\nGROUP 0x%08x (%zuB):\n", group, sz);

        // Decode sub-properties
        size_t pos = 0;
        while (pos + 8 <= sz) {
            uint32_t id = *(uint32_t*)(buf + pos);
            uint16_t sc = *(uint16_t*)(buf + pos + 4);
            if (pos + 8 + sc > sz) break;

            fprintf(f, "  0x%08x [%u] =", id, sc);
            for (size_t i = 0; i < sc; i++)
                fprintf(f, " %02x", buf[pos + 8 + i]);

            if (sc == 2) fprintf(f, "  (=%u)", *(uint16_t*)(buf+pos+8));
            else if (sc == 4) fprintf(f, "  (=%u)", *(uint32_t*)(buf+pos+8));
            fprintf(f, "\n");

            totalProps++;
            pos += 8 + sc;
        }
    }

    // Also try vendor ops that might carry config data
    fprintf(f, "\n=== 0x9104 Device Status ===\n");
    {
        ULONG resp = mtpRead(dev, 0x9104, buf, &sz, 0);
        if (resp == 0x2001 && sz > 0) {
            fprintf(f, "  %zuB:", sz);
            for (size_t i = 0; i < sz; i++) fprintf(f, " %02x", buf[i]);
            fprintf(f, "\n");
        }
    }

    // Scan vendor ops 0x9400-0x9420 for any that return data
    fprintf(f, "\n=== Vendor operation scan ===\n");
    for (UINT16 op = 0x9400; op <= 0x9420; op++) {
        if (op == 0x9403) continue; // SetProperty - don't touch
        ULONG resp = mtpRead(dev, op, buf, &sz, 0);
        if (resp == 0x2001 && sz > 0) {
            fprintf(f, "  OP 0x%04x (no params): %zuB =", op, sz);
            for (size_t i = 0; i < sz && i < 64; i++) fprintf(f, " %02x", buf[i]);
            if (sz > 64) fprintf(f, " ...");
            fprintf(f, "\n");
        }
        // Try with param=1
        resp = mtpRead(dev, op, buf, &sz, 1, 1);
        if (resp == 0x2001 && sz > 0) {
            fprintf(f, "  OP 0x%04x (p1=1): %zuB =", op, sz);
            for (size_t i = 0; i < sz && i < 64; i++) fprintf(f, " %02x", buf[i]);
            if (sz > 64) fprintf(f, " ...");
            fprintf(f, "\n");
        }
    }

    // Also scan 0x9100-0x9120
    for (UINT16 op = 0x9100; op <= 0x9120; op++) {
        if (op == 0x9102 || op == 0x9103) continue;
        ULONG resp = mtpRead(dev, op, buf, &sz, 0);
        if (resp == 0x2001 && sz > 0) {
            fprintf(f, "  OP 0x%04x (no params): %zuB =", op, sz);
            for (size_t i = 0; i < sz && i < 64; i++) fprintf(f, " %02x", buf[i]);
            if (sz > 64) fprintf(f, " ...");
            fprintf(f, "\n");
        }
    }

    fclose(f);
    printf("Saved %d properties to %s\n", totalProps, outfile);

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
