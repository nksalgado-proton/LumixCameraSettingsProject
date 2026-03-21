"""Diagnose camera WiFi API — try different handshake sequences."""

import httpx
import time

IP = "192.168.54.1"
BASE = f"http://{IP}"


def try_request(desc: str, **params):
    """Send a request and print the result."""
    try:
        resp = httpx.get(f"{BASE}/cam.cgi", params=params, timeout=5.0)
        text = resp.text.strip()
        # Extract just the result
        if "<result>" in text:
            start = text.index("<result>") + 8
            end = text.index("</result>")
            result = text[start:end]
        else:
            result = text[:200]
        print(f"  {desc}: [{result}] ({len(resp.text)} chars)")
        return resp.text
    except Exception as e:
        print(f"  {desc}: ERROR - {e}")
        return ""


print("=== Step 1: Raw connectivity ===")
try_request("getstate", mode="getstate")

print("\n=== Step 2: Try different accctrl formats ===")
# Format 1: Original
try_request("accctrl v1 (hex-PC)", mode="accctrl", type="req_acc",
            value="0000000000000000-PC", value2="XLumixG9Settings")
# Format 2: Simple 0
try_request("accctrl v2 (value=0)", mode="accctrl", type="req_acc",
            value="0", value2="XLumixG9Settings")
# Format 3: No value2
try_request("accctrl v3 (no value2)", mode="accctrl", type="req_acc",
            value="0")
# Format 4: MAC-like format
try_request("accctrl v4 (MAC format)", mode="accctrl", type="req_acc",
            value="00:00:00:00:00:00", value2="XLumixG9Settings")
# Format 5: Just app name
try_request("accctrl v5 (name only)", mode="accctrl", type="req_acc",
            value="XLumixG9Settings")

print("\n=== Step 3: Try recmode before and after accctrl ===")
try_request("recmode (before acc)", mode="camcmd", value="recmode")

print("\n=== Step 4: Try different modes ===")
try_request("playmode", mode="camcmd", value="playmode")
try_request("getinfo allmenu", mode="getinfo", type="allmenu")
try_request("getinfo capability", mode="getinfo", type="capability")
try_request("getinfo curmenu", mode="getinfo", type="curmenu")
try_request("getsetting iso", mode="getsetting", type="iso")

print("\n=== Step 5: Try startstream (some cameras need this) ===")
try_request("startstream", mode="startstream", value="49199")
try_request("stopstream", mode="stopstream")

print("\n=== Step 6: Check other endpoints ===")
# Some newer cameras have different endpoints
for path in ["/cam.cgi", "/server0.cgi", "/navi.cgi"]:
    try:
        resp = httpx.get(f"{BASE}{path}", params={"mode": "getstate"}, timeout=3.0)
        print(f"  {path}: HTTP {resp.status_code} ({len(resp.text)} chars)")
    except Exception as e:
        print(f"  {path}: {e}")

print("\n=== Step 7: SSDP/UPnP discovery ===")
try:
    resp = httpx.get(f"{BASE}:60606/Server0/CDS_control", timeout=3.0)
    print(f"  UPnP CDS: HTTP {resp.status_code}")
except Exception as e:
    print(f"  UPnP CDS: {e}")

try:
    resp = httpx.get(f"{BASE}/dd.xml", timeout=3.0)
    print(f"  dd.xml: HTTP {resp.status_code} ({len(resp.text)} chars)")
    if resp.status_code == 200:
        print(f"  Content: {resp.text[:500]}")
except Exception as e:
    print(f"  dd.xml: {e}")

print("\nDone.")
