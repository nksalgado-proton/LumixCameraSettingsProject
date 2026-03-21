/**
 * wpd_camset — Download CamSet DAT files from camera via PTP
 *
 * 1. Enumerate PTP objects to find AD_LUMIX/CAMSET/*.DAT files
 * 2. Download them via GetObject
 * 3. Try triggering CamSet save via 0x9406
 *
 * Usage: wpd_camset [download|save|list]
 *   list     — list all objects on SD card (find DAT files)
 *   download — download CamSet DAT file to current directory
 *   save     — try to trigger CamSet save via 0x9406, then download
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
              UINT32 np=0, UINT32 p1=0, UINT32 p2=0, UINT32 p3=0, UINT32 p4=0) {
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
    UINT32 pvals[]={p1,p2,p3,p4};
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
            // Use larger buffer for file downloads
            ULONG readSz = 1024*1024; // 1MB
            rp->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, readSz);
            BYTE* tmp=new BYTE[readSz]; memset(tmp,0,readSz);
            rp->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, tmp, readSz); delete[] tmp;
            hr = dev->SendCommand(0, rp, &rr);
            if (SUCCEEDED(hr) && rr) {
                BYTE* d=nullptr; ULONG ds=0;
                if (SUCCEEDED(rr->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &d, &ds)) && d && ds) {
                    size_t copyLen = ds < 1024*1024 ? ds : 1024*1024;
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

ULONG mtpNoData(IPortableDevice* dev, UINT16 op, UINT32 np=0, UINT32 p1=0, UINT32 p2=0) {
    ULONG resp=0;
    IPortableDeviceValues *p=nullptr, *r=nullptr;
    CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDeviceValues, (void**)&p);
    p->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);
    p->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, (ULONG)op);
    IPortableDevicePropVariantCollection *pc=nullptr;
    CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_IPortableDevicePropVariantCollection, (void**)&pc);
    PROPVARIANT pv; PropVariantInit(&pv); pv.vt=VT_UI4;
    UINT32 pvals[]={p1,p2};
    for(UINT32 i=0;i<np;i++){pv.ulVal=pvals[i];pc->Add(&pv);}
    p->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, pc);
    HRESULT hr = dev->SendCommand(0, p, &r);
    if (SUCCEEDED(hr) && r) { r->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &resp); r->Release(); }
    pc->Release(); p->Release();
    return resp;
}

// Extract PTP string from ObjectInfo data (PTP string: length byte + UTF-16 chars)
std::string extractPTPString(const uint8_t* data, size_t offset, size_t maxLen) {
    if (offset >= maxLen) return "";
    uint8_t len = data[offset]; // number of UTF-16 chars including null
    if (len == 0 || offset + 1 + len * 2 > maxLen) return "";
    std::string result;
    for (int i = 0; i < len - 1; i++) { // -1 to skip null terminator
        uint16_t ch = *(uint16_t*)(data + offset + 1 + i * 2);
        if (ch < 128) result += (char)ch;
        else result += '?';
    }
    return result;
}

struct ObjectInfo {
    uint32_t handle;
    uint32_t storageId;
    uint16_t format;
    uint32_t compressedSize;
    uint32_t parent;
    std::string filename;
};

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string mode = "list";
    if (argc > 1) mode = argv[1];

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IPortableDevice* dev = openCamera();
    if (!dev) { printf("Failed to open camera!\n"); return 1; }
    printf("Camera opened.\n");

    uint8_t* bigbuf = new uint8_t[1024*1024];
    uint8_t buf[65536];
    size_t sz = 0;

    // Open vendor session (may already be open)
    mtpNoData(dev, 0x9102, 1, 0x00010001);

    // Get storage IDs
    printf("\n=== Storage IDs ===\n");
    ULONG resp = mtpRead(dev, 0x1004, buf, &sz, 0); // GetStorageIDs
    if (resp != 0x2001 || sz < 8) {
        printf("Failed to get storage IDs: 0x%04lx\n", resp);
        goto done;
    }
    {
        uint32_t numStorages = *(uint32_t*)buf;
        printf("  %u storage(s):\n", numStorages);
        for (uint32_t i = 0; i < numStorages && i < 4; i++) {
            uint32_t sid = *(uint32_t*)(buf + 4 + i * 4);
            printf("  [%u] StorageID = 0x%08x\n", i, sid);
        }
    }

    // Get object handles (all objects, all storages)
    printf("\n=== Object Handles ===\n");
    resp = mtpRead(dev, 0x1007, bigbuf, &sz, 3, 0xFFFFFFFF, 0, 0); // GetObjectHandles
    if (resp != 0x2001 || sz < 4) {
        printf("Failed to get handles: 0x%04lx\n", resp);
        goto done;
    }
    {
        uint32_t numObjects = *(uint32_t*)bigbuf;
        printf("  %u objects total\n", numObjects);

        // Collect object handles
        std::vector<uint32_t> handles;
        for (uint32_t i = 0; i < numObjects && (4 + (i+1)*4) <= sz; i++) {
            handles.push_back(*(uint32_t*)(bigbuf + 4 + i * 4));
        }

        // For each object, get info and look for DAT files
        std::vector<ObjectInfo> datFiles;
        std::vector<ObjectInfo> allDirs;

        printf("  Scanning for CamSet DAT files...\n");

        for (uint32_t handle : handles) {
            // GetObjectInfo (0x1008)
            resp = mtpRead(dev, 0x1008, buf, &sz, 1, handle);
            if (resp != 0x2001 || sz < 52) continue;

            ObjectInfo info;
            info.handle = handle;
            info.storageId = *(uint32_t*)(buf + 0);
            info.format = *(uint16_t*)(buf + 4);
            // protectionStatus at +6
            info.compressedSize = *(uint32_t*)(buf + 8);
            // thumbFormat at +12, thumbCompSize at +16, thumbPixWidth at +20, thumbPixHeight at +24
            // imagePixWidth at +28, imagePixHeight at +32, imageBitDepth at +36
            info.parent = *(uint32_t*)(buf + 40);
            // associationType at +44, associationDesc at +48
            // sequenceNumber at +52
            // filename starts at offset 53 (PTP string)
            info.filename = extractPTPString(buf, 53, sz);

            // 0x3001 = Association (folder), 0x3000 = Undefined
            bool isDir = (info.format == 0x3001);

            if (isDir) {
                allDirs.push_back(info);
            }

            // Check for DAT files
            if (info.filename.size() >= 4) {
                std::string ext = info.filename.substr(info.filename.size() - 4);
                if (ext == ".DAT" || ext == ".dat") {
                    datFiles.push_back(info);
                }
            }
        }

        // Show directories that might be AD_LUMIX
        printf("\n  Directories found:\n");
        for (auto& d : allDirs) {
            // Show dirs that look camera-related
            if (d.filename.find("LUMIX") != std::string::npos ||
                d.filename.find("lumix") != std::string::npos ||
                d.filename.find("AD_") != std::string::npos ||
                d.filename.find("CAMSET") != std::string::npos ||
                d.filename.find("DCIM") != std::string::npos ||
                d.filename.find("PRIVATE") != std::string::npos ||
                d.parent == 0) {
                printf("    handle=0x%08x parent=0x%08x \"%s\"\n",
                       d.handle, d.parent, d.filename.c_str());
            }
        }

        // Show DAT files
        printf("\n  DAT files found: %zu\n", datFiles.size());
        for (auto& f : datFiles) {
            printf("    handle=0x%08x parent=0x%08x size=%u \"%s\"\n",
                   f.handle, f.parent, f.compressedSize, f.filename.c_str());
        }

        // Download mode
        if (mode == "download" && !datFiles.empty()) {
            printf("\n=== Downloading DAT files ===\n");
            for (auto& f : datFiles) {
                printf("  Downloading \"%s\" (%u bytes)...\n", f.filename.c_str(), f.compressedSize);

                // GetObject (0x1009)
                resp = mtpRead(dev, 0x1009, bigbuf, &sz, 1, f.handle);
                if (resp == 0x2001 && sz > 0) {
                    FILE* out = fopen(f.filename.c_str(), "wb");
                    if (out) {
                        fwrite(bigbuf, 1, sz, out);
                        fclose(out);
                        printf("    Saved %zu bytes to %s\n", sz, f.filename.c_str());

                        // Quick validation: check for "Panasonic" header
                        if (sz > 16 && memcmp(bigbuf, "Panasonic", 9) == 0) {
                            printf("    Valid CamSet header!\n");
                        }
                    }
                } else {
                    printf("    Download failed: resp=0x%04lx sz=%zu\n", resp, sz);
                }
            }
        }

        // Save mode: try triggering CamSet save then download
        if (mode == "save") {
            printf("\n=== Attempting CamSet save via 0x9406 ===\n");
            // 0x9406 = Setup Ctrl (Format, Sensor Cleaning, Menu Save, firmware update)
            // Try various parameter patterns
            UINT32 saveParams[][2] = {
                {0x15c00021, 0},  // menu save event code from gphoto2
                {0x15c00020, 0},  // variant
                {1, 0},
                {2, 0},
                {3, 0},
                {0x0100, 0},
                {0x0200, 0},
            };
            for (auto& sp : saveParams) {
                printf("  0x9406(0x%08x): ", sp[0]);
                resp = mtpNoData(dev, 0x9406, 1, sp[0]);
                printf("resp=0x%04lx\n", resp);
                if (resp == 0x2001) {
                    printf("  *** SUCCESS! Waiting for save to complete... ***\n");
                    Sleep(3000);
                    break;
                }
            }
        }
    }

done:
    delete[] bigbuf;
    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("\nDone.\n");
    return 0;
}
