"""Low-level HTTP client for Lumix camera WiFi API.

All communication with the camera goes through cam.cgi on port 80.
Commands use query parameters: mode, type, value, value2.
Responses are XML.
"""

import xml.etree.ElementTree as ET
from dataclasses import dataclass, field

import httpx


@dataclass
class LumixClient:
    """HTTP client for Lumix camera WiFi API."""

    ip: str = "192.168.54.1"
    port: int = 80
    timeout: float = 5.0
    _client: httpx.Client = field(init=False, repr=False)

    def __post_init__(self):
        self._client = httpx.Client(
            base_url=f"http://{self.ip}:{self.port}",
            timeout=self.timeout,
        )

    @property
    def base_url(self) -> str:
        return f"http://{self.ip}:{self.port}"

    def cam_cgi(self, **params: str) -> str:
        """Send a cam.cgi request and return raw response text.

        Usage:
            client.cam_cgi(mode="getinfo", type="allmenu")
            client.cam_cgi(mode="setsetting", type="iso", value="400")
        """
        resp = self._client.get("/cam.cgi", params=params)
        resp.raise_for_status()
        return resp.text

    def cam_cgi_xml(self, **params: str) -> ET.Element:
        """Send a cam.cgi request and parse XML response."""
        text = self.cam_cgi(**params)
        return ET.fromstring(text)

    def is_reachable(self) -> bool:
        """Check if the camera is reachable on the network."""
        try:
            self._client.get("/cam.cgi", params={"mode": "getstate"}, timeout=2.0)
            return True
        except (httpx.ConnectError, httpx.TimeoutException):
            return False

    def connect(self) -> str:
        """Send access control request to pair with camera.

        Returns the raw response text.
        """
        return self.cam_cgi(
            mode="accctrl",
            type="req_acc",
            value="0000000000000000-PC",
            value2="XLumixG9Settings",
        )

    def get_state(self) -> str:
        """Get current camera state."""
        return self.cam_cgi(mode="getstate")

    def get_info(self, info_type: str) -> str:
        """Get camera info. Types: allmenu, curmenu, capability, lens."""
        return self.cam_cgi(mode="getinfo", type=info_type)

    def get_setting(self, setting_type: str) -> str:
        """Get a specific setting value."""
        return self.cam_cgi(mode="getsetting", type=setting_type)

    def set_setting(self, setting_type: str, value: str) -> str:
        """Set a camera setting."""
        return self.cam_cgi(mode="setsetting", type=setting_type, value=value)

    def cam_cmd(self, value: str) -> str:
        """Send a camera command (recmode, capture, menu_entry, etc.)."""
        return self.cam_cgi(mode="camcmd", value=value)

    def close(self):
        self._client.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()
