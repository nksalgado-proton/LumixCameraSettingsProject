"""
Query Lumix G9 II via Tether DLL — proper struct alignment version.
"""

import ctypes
import sys
import os

DLL_PATH = r"C:\Program Files\Panasonic\LUMIX Tether\Lmxptpif.dll"

DEVINFO_DEF_ARRAY_MAX = 512
DEVINFO_DEF_STRING_MAX = 256


class LMX_DEV_INFO(ctypes.Structure):
    _fields_ = [
        ("dev_Index", ctypes.c_uint32),
        # 4 bytes padding here (implicit, for wchar alignment)
        ("dev_MakerName", ctypes.c_wchar * DEVINFO_DEF_STRING_MAX),
        ("dev_MakerName_Length", ctypes.c_uint32),
        # padding
        ("dev_ModelName", ctypes.c_wchar * DEVINFO_DEF_STRING_MAX),
        ("dev_ModelName_Length", ctypes.c_uint32),
    ]


class LMX_CONNECT_DEVICE_INFO(ctypes.Structure):
    _fields_ = [
        ("find_PnpDevice_Count", ctypes.c_uint32),
        # 4 bytes padding (implicit, for pointer alignment on x64)
        ("find_PnpDevice_IDs", ctypes.c_wchar_p * DEVINFO_DEF_ARRAY_MAX),
        ("find_PnpDevice_Info", LMX_DEV_INFO * DEVINFO_DEF_ARRAY_MAX),
    ]


def main():
    if not os.path.exists(DLL_PATH):
        print(f"ERROR: DLL not found")
        sys.exit(1)

    # COM init
    ole32 = ctypes.windll.ole32
    ole32.CoInitializeEx(None, 0)

    dll_dir = os.path.dirname(DLL_PATH)
    os.add_dll_directory(dll_dir)
    dll = ctypes.WinDLL(DLL_PATH)

    # Init
    dll.LMX_func_api_Init.restype = None
    dll.LMX_func_api_Init.argtypes = []
    dll.LMX_func_api_Init()
    print("Init OK")

    # Print struct sizes for debugging
    print(f"LMX_DEV_INFO size: {ctypes.sizeof(LMX_DEV_INFO)}")
    print(f"LMX_CONNECT_DEVICE_INFO size: {ctypes.sizeof(LMX_CONNECT_DEVICE_INFO)}")

    # Allocate struct on heap (it's huge)
    dev_info = LMX_CONNECT_DEVICE_INFO()
    ret_error = ctypes.c_uint32(0)

    print("\nGetPnPDeviceInfo...")
    dll.LMX_func_api_GetPnPDeviceInfo.restype = ctypes.c_uint8
    dll.LMX_func_api_GetPnPDeviceInfo.argtypes = [
        ctypes.POINTER(LMX_CONNECT_DEVICE_INFO),
        ctypes.POINTER(ctypes.c_uint32),
    ]
    result = dll.LMX_func_api_GetPnPDeviceInfo(
        ctypes.byref(dev_info), ctypes.byref(ret_error)
    )
    print(f"    result={result}, error=0x{ret_error.value:08x}")
    print(f"    Devices: {dev_info.find_PnpDevice_Count}")

    if dev_info.find_PnpDevice_Count == 0:
        print("    No camera!")
        return

    # Print device info
    for i in range(dev_info.find_PnpDevice_Count):
        info = dev_info.find_PnpDevice_Info[i]
        dev_id = dev_info.find_PnpDevice_IDs[i]
        print(f"    Device {i}: maker='{info.dev_MakerName}' model='{info.dev_ModelName}'")
        print(f"    Device ID pointer: {dev_id}")

    # SelectPnPDevice with proper struct
    print("\nSelectPnPDevice(0)...")
    dll.LMX_func_api_SelectPnPDevice.restype = ctypes.c_uint8
    dll.LMX_func_api_SelectPnPDevice.argtypes = [
        ctypes.c_uint32,
        ctypes.POINTER(LMX_CONNECT_DEVICE_INFO),
        ctypes.POINTER(ctypes.c_uint32),
    ]
    ret_error = ctypes.c_uint32(0)
    try:
        result = dll.LMX_func_api_SelectPnPDevice(
            0, ctypes.byref(dev_info), ctypes.byref(ret_error)
        )
        print(f"    result={result}, error=0x{ret_error.value:08x}")
    except OSError as e:
        print(f"    CRASHED: {e}")
        print("    Falling back to SelectDevice...")
        dll.LMX_func_api_SelectDevice.restype = ctypes.c_uint8
        dll.LMX_func_api_SelectDevice.argtypes = [
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_uint32),
        ]
        ret_error = ctypes.c_uint32(0)
        result = dll.LMX_func_api_SelectDevice(0, ctypes.byref(ret_error))
        print(f"    SelectDevice: result={result}, error=0x{ret_error.value:08x}")

    # OpenSession
    print("\nOpenSession...")
    dll.LMX_func_api_OpenSession.restype = ctypes.c_uint8
    dll.LMX_func_api_OpenSession.argtypes = [
        ctypes.c_uint32,
        ctypes.POINTER(ctypes.c_uint32),
        ctypes.POINTER(ctypes.c_uint32),
    ]
    device_ver = ctypes.c_uint32(0)
    ret_error = ctypes.c_uint32(0)
    try:
        result = dll.LMX_func_api_OpenSession(
            0x00010001, ctypes.byref(device_ver), ctypes.byref(ret_error)
        )
        print(f"    result={result}, device_ver=0x{device_ver.value:08x}, error=0x{ret_error.value:08x}")
    except OSError as e:
        print(f"    CRASHED: {e}")

    # Test ISO
    print("\nISO_GetParam...")
    iso = ctypes.c_uint32(0)
    ret_error = ctypes.c_uint32(0)
    dll.LMX_func_api_ISO_GetParam.restype = ctypes.c_uint8
    dll.LMX_func_api_ISO_GetParam.argtypes = [
        ctypes.POINTER(ctypes.c_uint32),
        ctypes.POINTER(ctypes.c_uint32),
    ]
    try:
        result = dll.LMX_func_api_ISO_GetParam(
            ctypes.byref(iso), ctypes.byref(ret_error)
        )
        print(f"    result={result}, ISO=0x{iso.value:08x}, error=0x{ret_error.value:08x}")
        if result == 1 and iso.value == 0xFFFFFFFF:
            print("    ISO: Auto")
    except OSError as e:
        print(f"    CRASHED: {e}")

    # Fn Assign
    print("\nSetupConfig_Get_Fn_Assign...")
    fn = ctypes.c_uint32(0)
    ret_error = ctypes.c_uint32(0)
    dll.LMX_func_api_SetupConfig_Get_Fn_Assign.restype = ctypes.c_uint8
    dll.LMX_func_api_SetupConfig_Get_Fn_Assign.argtypes = [
        ctypes.POINTER(ctypes.c_uint32),
        ctypes.POINTER(ctypes.c_uint32),
    ]
    try:
        result = dll.LMX_func_api_SetupConfig_Get_Fn_Assign(
            ctypes.byref(fn), ctypes.byref(ret_error)
        )
        print(f"    result={result}, value=0x{fn.value:08x}, error=0x{ret_error.value:08x}")
    except OSError as e:
        print(f"    CRASHED: {e}")

    # GetSetupConfigInfo
    print("\nGetSetupConfigInfo...")
    buf = (ctypes.c_uint8 * 8192)()
    ret_error = ctypes.c_uint32(0)
    dll.LMX_func_api_GetSetupConfigInfo.restype = ctypes.c_uint8
    dll.LMX_func_api_GetSetupConfigInfo.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_uint32),
    ]
    try:
        result = dll.LMX_func_api_GetSetupConfigInfo(
            ctypes.byref(buf), ctypes.byref(ret_error)
        )
        print(f"    result={result}, error=0x{ret_error.value:08x}")
        if result == 1:
            last_nz = max((i for i in range(8192) if buf[i] != 0), default=0)
            print(f"    Data ({last_nz+1} bytes):")
            for i in range(0, min(last_nz+16, 512), 16):
                hex_str = " ".join(f"{buf[i+j]:02x}" for j in range(16) if i+j < 8192)
                ascii_str = "".join(chr(buf[i+j]) if 32 <= buf[i+j] < 127 else "." for j in range(16) if i+j < 8192)
                print(f"    {i:04x}: {hex_str}  {ascii_str}")
    except OSError as e:
        print(f"    CRASHED: {e}")

    # Cleanup
    try:
        dll.LMX_func_api_CloseSession.restype = ctypes.c_uint8
        dll.LMX_func_api_CloseSession.argtypes = [ctypes.POINTER(ctypes.c_uint32)]
        ret_error = ctypes.c_uint32(0)
        dll.LMX_func_api_CloseSession(ctypes.byref(ret_error))
    except OSError:
        pass
    try:
        dll.LMX_func_api_CloseDevice.restype = ctypes.c_uint8
        dll.LMX_func_api_CloseDevice.argtypes = [ctypes.POINTER(ctypes.c_uint32)]
        ret_error = ctypes.c_uint32(0)
        dll.LMX_func_api_CloseDevice(ctypes.byref(ret_error))
    except OSError:
        pass
    ole32.CoUninitialize()
    print("\nDone.")


if __name__ == "__main__":
    main()
