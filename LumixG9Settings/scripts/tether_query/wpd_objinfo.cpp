/**
 * wpd_objinfo — Dump raw GetObjectInfo for first few objects to understand format
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

    // Get all object handles
    ULONG resp = mtpRead(dev, 0x1007, buf, 65536, &sz, 3, 0xFFFFFFFF, 0, 0);
    if (resp != 0x2001 || sz < 8) { printf("No objects\n"); goto done; }

    {
        uint32_t n = *(uint32_t*)buf;
        printf("%u objects total\n\n", n);

        // Dump GetObjectInfo for first 10 objects
        uint32_t limit = n < 10 ? n : 10;
        for (uint32_t i = 0; i < limit; i++) {
            uint32_t h = *(uint32_t*)(buf + 4 + i * 4);
            printf("=== Object 0x%08x ===\n", h);

            uint8_t info[4096];
            size_t isz = 0;
            resp = mtpRead(dev, 0x1008, info, 4096, &isz, 1, h);
            printf("  GetObjectInfo: resp=0x%04lx, %zu bytes\n", resp, isz);
            if (resp == 0x2001 && isz > 0) {
                hexdump(info, isz < 256 ? isz : 256, 16);

                // Try to decode PTP ObjectInfo fields:
                // StorageID(4) + ObjectFormatCode(2) + ProtectionStatus(2) +
                // ObjectCompressedSize(4) + ThumbFormat(2) + ThumbCompressedSize(4) +
                // ThumbPixWidth(4) + ThumbPixHeight(4) + ImagePixWidth(4) +
                // ImagePixHeight(4) + ImageBitDepth(4) + ParentObject(4) +
                // AssociationType(2) + AssociationDesc(4) + SequenceNumber(4)
                // = 52 bytes fixed, then PTP strings: Filename, CaptureDate, ModificationDate, Keywords
                if (isz >= 52) {
                    printf("  StorageID: 0x%08x\n", *(uint32_t*)(info));
                    printf("  Format: 0x%04x\n", *(uint16_t*)(info+4));
                    printf("  Size: %u\n", *(uint32_t*)(info+8));
                    printf("  Parent: 0x%08x\n", *(uint32_t*)(info+38));
                    printf("  AssocType: 0x%04x\n", *(uint16_t*)(info+42));
                    // Filename at offset 52
                    if (isz > 52) {
                        uint8_t strLen = info[52];
                        printf("  Filename string length: %u chars\n", strLen);
                        if (strLen > 0 && 53 + strLen * 2 <= isz) {
                            printf("  Filename: ");
                            for (int j = 0; j < strLen - 1; j++) {
                                uint16_t ch = *(uint16_t*)(info + 53 + j * 2);
                                printf("%c", (ch < 128) ? (char)ch : '?');
                            }
                            printf("\n");
                        }
                    }
                }
            }
            printf("\n");
        }
    }

done:
    dev->Close();
    dev->Release();
    CoUninitialize();
    printf("Done.\n");
    return 0;
}
