/**
 * wpd_camset2 — Fast CamSet DAT download via directory traversal
 *
 * Instead of scanning all 2295 objects, navigate:
 *   root → AD_LUMIX → CAMSET → *.DAT
 * using GetObjectHandles with parent filtering.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
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
                    size_t copyLen = ds < bufSz ? ds : bufSz;
                    memcpy(buf, d, copyLen); *sz=ds; CoTaskMemFree(d);
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

// Get children of a directory (parent handle), return handles
std::vector<uint32_t> getChildren(IPortableDevice* dev, uint32_t storageId, uint32_t parent, uint8_t* buf, size_t bufSz) {
    std::vector<uint32_t> handles;
    size_t sz = 0;
    // GetObjectHandles: storage, format=0(all), parent
    ULONG resp = mtpRead(dev, 0x1007, buf, bufSz, &sz, 3, storageId, 0, parent);
    if (resp == 0x2001 && sz >= 4) {
        uint32_t n = *(uint32_t*)buf;
        for (uint32_t i = 0; i < n && (4 + (i+1)*4) <= sz; i++) {
            handles.push_back(*(uint32_t*)(buf + 4 + i * 4));
        }
    }
    return handles;
}

// Get object name for a handle
std::string getObjectName(IPortableDevice* dev, uint32_t handle, uint8_t* buf, uint32_t* outSize = nullptr) {
    size_t sz = 0;
    ULONG resp = mtpRead(dev, 0x1008, buf, 4096, &sz, 1, handle);
    if (resp != 0x2001 || sz < 54) return "";
    if (outSize) *outSize = *(uint32_t*)(buf + 8);
    return extractPTPString(buf, 52, sz);
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string outfile = "";
    if (argc > 1) outfile = argv[1];

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IPortableDevice* dev = openCamera();
    if (!dev) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n");

    uint8_t* buf = new uint8_t[1024*1024]; // 1MB buffer for file downloads
    size_t sz = 0;

    mtpNoData(dev, 0x9102, 1, 0x00010001);

    // Get storages
    ULONG resp = mtpRead(dev, 0x1004, buf, 65536, &sz, 0);
    if (resp != 0x2001 || sz < 8) { printf("No storage\n"); goto done; }

    uint32_t numStorages = *(uint32_t*)buf;
    printf("%u storage(s)\n", numStorages);

    // Strategy 1: Try GetObjectHandles with format=0x3001 (folders only) to find AD_LUMIX fast
    // Strategy 2: If that fails, get all handles and scan from the END (system dirs are last)

    printf("\n--- Approach 1: Get folders only (format=0x3001) ---\n");
    uint32_t adLumixHandle = 0, camsetHandle = 0;
    bool found = false;

    {
        size_t dirSz = 0;
        resp = mtpRead(dev, 0x1007, buf, 65536, &dirSz, 3, 0xFFFFFFFF, 0x3001, 0xFFFFFFFF);
        printf("  GetObjectHandles(all, fmt=0x3001, all): resp=0x%04lx, %zu bytes\n", resp, dirSz);
        if (resp == 0x2001 && dirSz >= 8) {
            uint32_t n = *(uint32_t*)buf;
            printf("  %u directories found\n", n);
            for (uint32_t i = 0; i < n && (4 + (i+1)*4) <= dirSz; i++) {
                uint32_t h = *(uint32_t*)(buf + 4 + i * 4);
                size_t infoSz = 0;
                ULONG r2 = mtpRead(dev, 0x1008, buf + 60000, 4096, &infoSz, 1, h);
                if (r2 != 0x2001 || infoSz < 53) continue;
                std::string name = extractPTPString(buf + 60000, 52, infoSz);
                uint32_t parent = *(uint32_t*)(buf + 60000 + 38);
                printf("    [0x%08x] %s (parent=0x%08x)\n", h, name.c_str(), parent);
                if (name == "AD_LUMIX") adLumixHandle = h;
                if (name == "CAMSET" && adLumixHandle && parent == adLumixHandle) camsetHandle = h;
            }
        }
    }

    // If format filter didn't work, scan all handles from the END (system dirs are last)
    if (!adLumixHandle) {
        printf("\n--- Approach 2: Scan from end of object list ---\n");
        uint8_t* allBuf = new uint8_t[65536];
        size_t allSz = 0;
        resp = mtpRead(dev, 0x1007, allBuf, 65536, &allSz, 3, 0xFFFFFFFF, 0, 0xFFFFFFFF);
        if (resp == 0x2001 && allSz >= 4) {
            uint32_t n = *(uint32_t*)allBuf;
            printf("  %u objects. Scanning last 100...\n", n);
            uint32_t start = n > 100 ? n - 100 : 0;
            for (uint32_t i = start; i < n && (4 + (i+1)*4) <= allSz; i++) {
                uint32_t h = *(uint32_t*)(allBuf + 4 + i * 4);
                size_t infoSz = 0;
                ULONG r2 = mtpRead(dev, 0x1008, buf, 4096, &infoSz, 1, h);
                if (r2 != 0x2001 || infoSz < 53) continue;
                std::string name = extractPTPString(buf, 52, infoSz);
                uint16_t fmt = *(uint16_t*)(buf + 4);
                uint32_t parent = *(uint32_t*)(buf + 38);
                printf("    [%u] 0x%08x %s (fmt=0x%04x parent=0x%08x)\n", i, h, name.c_str(), fmt, parent);
                if (name == "AD_LUMIX") adLumixHandle = h;
                if (name == "CAMSET" && adLumixHandle && parent == adLumixHandle) camsetHandle = h;
            }
        }
        delete[] allBuf;
    }

    // Now navigate into CAMSET and download DAT files
    if (camsetHandle) {
        printf("\n  Found CAMSET: 0x%08x — getting children...\n", camsetHandle);
        // Get ALL handles and filter by parent=CAMSET, since parent-filtered queries may not work
        uint8_t* allBuf = new uint8_t[65536];
        size_t allSz = 0;
        resp = mtpRead(dev, 0x1007, allBuf, 65536, &allSz, 3, 0xFFFFFFFF, 0, 0xFFFFFFFF);
        if (resp == 0x2001 && allSz >= 4) {
            uint32_t n = *(uint32_t*)allBuf;
            for (uint32_t i = 0; i < n && (4 + (i+1)*4) <= allSz; i++) {
                uint32_t h = *(uint32_t*)(allBuf + 4 + i * 4);
                size_t infoSz = 0;
                ULONG r2 = mtpRead(dev, 0x1008, buf, 4096, &infoSz, 1, h);
                if (r2 != 0x2001 || infoSz < 53) continue;
                uint32_t parent = *(uint32_t*)(buf + 38);
                if (parent != camsetHandle) continue;
                std::string name = extractPTPString(buf, 52, infoSz);
                uint32_t fileSize = *(uint32_t*)(buf + 8);
                printf("    [0x%08x] %s (%u bytes)\n", h, name.c_str(), fileSize);

                std::string ext = name.size() >= 4 ? name.substr(name.size() - 4) : "";
                if (ext == ".DAT" || ext == ".dat") {
                    std::string saveName = outfile.empty() ? name : outfile;
                    printf("  >>> Downloading %s (%u bytes)...\n", saveName.c_str(), fileSize);
                    size_t dlSz = 0;
                    ULONG r4 = mtpRead(dev, 0x1009, buf, 1024*1024, &dlSz, 1, h);
                    if (r4 == 0x2001 && dlSz > 0) {
                        FILE* f = fopen(saveName.c_str(), "wb");
                        if (f) {
                            fwrite(buf, 1, dlSz, f);
                            fclose(f);
                            printf("  >>> Saved %zu bytes to %s\n", dlSz, saveName.c_str());
                            if (dlSz > 32 && memcmp(buf, "Panasonic", 9) == 0) {
                                printf("  >>> *** Valid CamSet header! ***\n");
                                char model[32] = {};
                                memcpy(model, buf + 0x12, 16);
                                printf("  >>> Model: %s\n", model);
                            }
                            found = true;
                        }
                    } else {
                        printf("  >>> Download failed: resp=0x%04lx sz=%zu\n", r4, dlSz);
                    }
                }
            }
        }
        delete[] allBuf;
    } else if (adLumixHandle) {
        printf("\n  AD_LUMIX found but CAMSET not found inside it!\n");
    } else {
        printf("\n  AD_LUMIX directory not found!\n");
    }

done:
    delete[] buf;
    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
