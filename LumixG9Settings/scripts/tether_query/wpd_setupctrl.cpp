/**
 * wpd_setupctrl — Test 0x9406 SetupCtrl to trigger CamSet save
 *
 * From gphoto2 research:
 *   0x9406 = Setup_Ctrl (Format, Sensor Cleaning, Menu Save, FW Update)
 *   0x15c00021 = event "menu save complete"
 *
 * Try various param values to see what happens.
 * Also check storage 2 for AD_LUMIX.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
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

ULONG mtpRead(IPortableDevice* dev, UINT16 op, uint8_t* buf, size_t bufSz, size_t* sz,
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
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, (ULONG)bufSz);
            BYTE* tmp=new BYTE[bufSz]; memset(tmp,0,bufSz);
            rp->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, tmp, (ULONG)bufSz); delete[] tmp;
            hr = dev->SendCommand(0, rp, &rr);
            if (SUCCEEDED(hr) && rr) {
                BYTE* d=nullptr; ULONG ds=0;
                if (SUCCEEDED(rr->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &d, &ds)) && d && ds) {
                    memcpy(buf, d, ds<bufSz?ds:bufSz); *sz=ds; CoTaskMemFree(d);
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
    if (SUCCEEDED(hr) && r) {
        r->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &resp);
        // Also try to read response params
        IPortableDevicePropVariantCollection* rpc = nullptr;
        if (SUCCEEDED(r->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &rpc)) && rpc) {
            DWORD rn = 0; rpc->GetCount(&rn);
            if (rn > 0) {
                printf("    Response params (%lu): ", rn);
                for (DWORD i = 0; i < rn; i++) {
                    PROPVARIANT rpv; PropVariantInit(&rpv);
                    rpc->GetAt(i, &rpv);
                    printf("0x%08lx ", rpv.ulVal);
                    PropVariantClear(&rpv);
                }
                printf("\n");
            }
            rpc->Release();
        }
        r->Release();
    }
    pc->Release(); p->Release();
    return resp;
}

void hexdump(const uint8_t* data, size_t len, size_t max_lines = 8) {
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

std::string extractPTPString(const uint8_t* data, size_t offset, size_t maxLen) {
    if (offset >= maxLen) return "";
    uint8_t len = data[offset];
    if (len == 0 || offset + 1 + len * 2 > maxLen) return "";
    std::string result;
    for (int i = 0; i < len - 1; i++) {
        uint16_t ch = *(uint16_t*)(data + offset + 1 + i * 2);
        if (ch < 128) result += (char)ch;
        else result += '?';
    }
    return result;
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

    // === Check storage 2 for objects ===
    printf("\n=== Storage 2 objects ===\n");
    ULONG resp = mtpRead(dev, 0x1007, buf, 65536, &sz, 3, 0x00020001, 0, 0);
    printf("GetObjectHandles(storage2, all, all): resp=0x%04lx, %zu bytes\n", resp, sz);
    if (resp == 0x2001 && sz >= 4) {
        uint32_t n = *(uint32_t*)buf;
        printf("%u objects on storage 2\n", n);
        uint32_t limit = n < 20 ? n : 20;
        for (uint32_t i = 0; i < limit && (4 + (i+1)*4) <= sz; i++) {
            uint32_t h = *(uint32_t*)(buf + 4 + i * 4);
            size_t infoSz = 0;
            ULONG r2 = mtpRead(dev, 0x1008, buf + 60000, 4096, &infoSz, 1, h);
            if (r2 == 0x2001 && infoSz >= 53) {
                std::string name = extractPTPString(buf + 60000, 52, infoSz);
                uint16_t fmt = *(uint16_t*)(buf + 60000 + 4);
                uint32_t parent = *(uint32_t*)(buf + 60000 + 38);
                printf("  [0x%08x] %s (fmt=0x%04x parent=0x%08x)\n", h, name.c_str(), fmt, parent);
            }
        }
    }

    // === Check StorageInfo for both storages ===
    printf("\n=== StorageInfo ===\n");
    uint32_t storages[] = {0x00010001, 0x00020001};
    for (int s = 0; s < 2; s++) {
        printf("Storage 0x%08x:\n", storages[s]);
        sz = 0;
        resp = mtpRead(dev, 0x1005, buf, 4096, &sz, 1, storages[s]);
        printf("  GetStorageInfo: resp=0x%04lx, %zu bytes\n", resp, sz);
        if (resp == 0x2001 && sz > 0) {
            hexdump(buf, sz < 128 ? sz : 128);
            // StorageInfo: StorageType(2) + FSType(2) + AccessCapability(2) +
            // MaxCapacity(8) + FreeSpace(8) + FreeObjects(4) +
            // StorageDescription(PTP string) + VolumeLabel(PTP string)
            if (sz >= 26) {
                printf("  Type: %u, FSType: %u, Access: %u\n",
                    *(uint16_t*)buf, *(uint16_t*)(buf+2), *(uint16_t*)(buf+4));
                uint64_t maxCap = *(uint64_t*)(buf+6);
                uint64_t freeSp = *(uint64_t*)(buf+14);
                printf("  MaxCapacity: %llu MB, FreeSpace: %llu MB\n", maxCap/(1024*1024), freeSp/(1024*1024));
                printf("  FreeObjects: %u\n", *(uint32_t*)(buf+22));
                std::string desc = extractPTPString(buf, 26, sz);
                printf("  Description: %s\n", desc.c_str());
            }
        }
    }

    // === Test 0x9406 with safe read-only params ===
    // Only test with data-to-read variant to see what it returns, DO NOT execute without params
    printf("\n=== 0x9406 SetupCtrl probe (read variant) ===\n");
    // Try as data-to-read with various params
    uint32_t testParams[] = {0, 1, 2, 3, 4, 5, 0x10, 0x20, 0x100};
    for (int i = 0; i < 9; i++) {
        sz = 0;
        resp = mtpRead(dev, 0x9406, buf, 4096, &sz, 1, testParams[i]);
        printf("  0x9406 param=0x%x: resp=0x%04lx, %zu bytes\n", testParams[i], resp, sz);
        if (resp == 0x2001 && sz > 0) {
            hexdump(buf, sz < 64 ? sz : 64, 4);
        }
    }

    // === Test 0x940d (previously returned OK) ===
    printf("\n=== 0x940d probe ===\n");
    sz = 0;
    resp = mtpRead(dev, 0x940d, buf, 4096, &sz, 0);
    printf("  0x940d no params: resp=0x%04lx, %zu bytes\n", resp, sz);
    if (resp == 0x2001 && sz > 0) hexdump(buf, sz < 64 ? sz : 64, 4);

    for (int i = 0; i < 5; i++) {
        sz = 0;
        resp = mtpRead(dev, 0x940d, buf, 4096, &sz, 1, (uint32_t)i);
        printf("  0x940d param=%d: resp=0x%04lx, %zu bytes\n", i, resp, sz);
        if (resp == 0x2001 && sz > 0) hexdump(buf, sz < 64 ? sz : 64, 4);
    }

    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
