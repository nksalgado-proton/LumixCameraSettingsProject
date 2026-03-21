"""WiFi API setting discovery — parse XML responses to build a complete settings catalog.

Queries the camera's getinfo (allmenu, capability) and getsetting endpoints
to discover all available settings, their current values, and valid options.
Results are cached to JSON for offline use.
"""

import json
import xml.etree.ElementTree as ET
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path

from rich.console import Console
from rich.table import Table

from xlumix.wifi.client import LumixClient

console = Console()


@dataclass
class SettingInfo:
    """Information about a single camera setting."""

    name: str  # WiFi API type parameter
    current_value: str = ""  # from getsetting
    valid_values: list[str] = field(default_factory=list)  # from capability/allmenu
    menu_path: list[str] = field(default_factory=list)  # menu hierarchy
    is_settable: bool = False  # whether setsetting works


@dataclass
class CameraCapabilities:
    """Complete catalog of a camera's settings."""

    model: str = ""
    firmware: str = ""
    captured_at: str = ""
    settings: dict[str, SettingInfo] = field(default_factory=dict)

    def __post_init__(self):
        if not self.captured_at:
            self.captured_at = datetime.now(timezone.utc).isoformat()


def discover_all_settings(client: LumixClient) -> CameraCapabilities:
    """Run full discovery against a live camera.

    1. Parse allmenu XML to find all setting names
    2. Parse capability XML to find valid values
    3. Query each setting individually
    4. Test settability (set to same value)
    """
    caps = CameraCapabilities()

    console.print("[bold]Discovering camera settings...[/bold]\n")

    # Step 0: Pair and switch to rec mode
    console.print("  Pairing with camera...")
    try:
        resp = client.connect()
        console.print(f"  [green]Paired[/green] ({resp[:80].strip()})")
    except Exception as e:
        console.print(f"  [yellow]Pairing: {e}[/yellow]")

    console.print("  Switching to rec mode...")
    try:
        resp = client.cam_cmd("recmode")
        console.print(f"  [green]Rec mode[/green] ({resp[:80].strip()})")
    except Exception as e:
        console.print(f"  [yellow]Rec mode: {e}[/yellow]")

    # Step 1: Get model info from state
    try:
        state_xml = client.get_state()
        caps.model = _extract_model_from_state(state_xml)
        console.print(f"  Model: {caps.model}")
    except Exception as e:
        console.print(f"  [yellow]Could not get state: {e}[/yellow]")

    # Step 2: Parse allmenu to discover setting names
    console.print("  Querying allmenu...")
    try:
        allmenu_xml = client.get_info("allmenu")
        menu_settings = parse_allmenu_xml(allmenu_xml)
        for si in menu_settings:
            caps.settings[si.name] = si
        console.print(f"  [green]Found {len(menu_settings)} settings in allmenu[/green]")
    except Exception as e:
        console.print(f"  [red]Failed to parse allmenu: {e}[/red]")

    # Step 3: Parse capability to get valid values
    console.print("  Querying capability...")
    try:
        cap_xml = client.get_info("capability")
        valid_values = parse_capability_xml(cap_xml)
        for name, values in valid_values.items():
            if name in caps.settings:
                caps.settings[name].valid_values = values
            else:
                caps.settings[name] = SettingInfo(name=name, valid_values=values)
        console.print(f"  [green]Found valid values for {len(valid_values)} settings[/green]")
    except Exception as e:
        console.print(f"  [red]Failed to parse capability: {e}[/red]")

    # Step 4: Query each setting for current value
    console.print(f"  Querying {len(caps.settings)} individual settings...")
    for name, si in caps.settings.items():
        try:
            resp = client.get_setting(name)
            si.current_value = _extract_value_from_setting(resp, name)
        except Exception:
            pass  # Setting may not support getsetting

    # Step 5: Test settability
    console.print("  Testing settability...")
    settable_count = 0
    for name, si in caps.settings.items():
        if si.current_value:
            try:
                client.set_setting(name, si.current_value)
                si.is_settable = True
                settable_count += 1
            except Exception:
                pass
    console.print(f"  [green]{settable_count} settings are settable via WiFi[/green]\n")

    return caps


def parse_allmenu_xml(xml_text: str) -> list[SettingInfo]:
    """Parse allmenu XML response to extract setting names and menu paths.

    The allmenu response typically has a structure like:
    <camrply><result>ok</result>
      <menuset>
        <titleinfo ...>
          <menuinfo id="..." enable="yes">
            <menuinfo id="settingname" .../>
          </menuinfo>
        </titleinfo>
      </menuset>
    </camrply>
    """
    settings = []
    try:
        root = ET.fromstring(xml_text)
    except ET.ParseError:
        return settings

    # Walk all elements looking for menuinfo with id attributes
    for elem in root.iter():
        if elem.tag in ("menuinfo", "settinginfo"):
            name = elem.get("id", "")
            if name and name not in ("", "menu"):
                path = _build_menu_path(elem, root)
                settings.append(SettingInfo(name=name, menu_path=path))

    # Deduplicate by name
    seen = set()
    unique = []
    for s in settings:
        if s.name not in seen:
            seen.add(s.name)
            unique.append(s)
    return unique


def parse_capability_xml(xml_text: str) -> dict[str, list[str]]:
    """Parse capability XML response to extract valid values per setting.

    Capability response typically has:
    <camrply><result>ok</result>
      <settinginfo>
        <settingname>value1,value2,value3</settingname>
      </settinginfo>
    </camrply>
    """
    result = {}
    try:
        root = ET.fromstring(xml_text)
    except ET.ParseError:
        return result

    # Look for elements that contain comma-separated values
    for elem in root.iter():
        if elem.text and "," in elem.text and elem.tag not in ("result", "camrply"):
            values = [v.strip() for v in elem.text.split(",") if v.strip()]
            if values:
                result[elem.tag] = values

    return result


def save_capabilities(caps: CameraCapabilities, path: Path):
    """Save capabilities to JSON for offline use."""
    path.parent.mkdir(parents=True, exist_ok=True)
    data = {
        "model": caps.model,
        "firmware": caps.firmware,
        "captured_at": caps.captured_at,
        "settings": {name: asdict(si) for name, si in caps.settings.items()},
    }
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


def load_capabilities(path: Path) -> CameraCapabilities:
    """Load capabilities from a JSON file."""
    raw = json.loads(path.read_text(encoding="utf-8"))
    settings = {name: SettingInfo(**si_data) for name, si_data in raw.get("settings", {}).items()}
    return CameraCapabilities(
        model=raw.get("model", ""),
        firmware=raw.get("firmware", ""),
        captured_at=raw.get("captured_at", ""),
        settings=settings,
    )


def save_raw_xml(client: LumixClient, output_dir: Path):
    """Capture raw XML responses from camera and save as fixture files.

    Use this to create test fixtures for offline development.
    """
    output_dir.mkdir(parents=True, exist_ok=True)

    for info_type in ("allmenu", "curmenu", "capability", "lens"):
        console.print(f"  Capturing getinfo/{info_type}...")
        try:
            xml = client.get_info(info_type)
            (output_dir / f"getinfo_{info_type}.xml").write_text(xml, encoding="utf-8")
            console.print(f"    [green]Saved ({len(xml)} chars)[/green]")
        except Exception as e:
            console.print(f"    [red]Failed: {e}[/red]")

    console.print("  Capturing getstate...")
    try:
        xml = client.get_state()
        (output_dir / "getstate.xml").write_text(xml, encoding="utf-8")
        console.print(f"    [green]Saved ({len(xml)} chars)[/green]")
    except Exception as e:
        console.print(f"    [red]Failed: {e}[/red]")

    # Capture a sample of getsetting responses
    from xlumix.wifi.probe import KNOWN_SETTINGS

    for setting in KNOWN_SETTINGS:
        try:
            xml = client.get_setting(setting)
            (output_dir / f"getsetting_{setting}.xml").write_text(xml, encoding="utf-8")
        except Exception:
            pass
    console.print(f"  [green]Captured getsetting for {len(KNOWN_SETTINGS)} known settings[/green]")


def show_capabilities(caps: CameraCapabilities):
    """Display capabilities in a rich table."""
    console.print("\n[bold]Camera Capabilities[/bold]")
    console.print(f"  Model: {caps.model}")
    console.print(f"  Settings: {len(caps.settings)}\n")

    table = Table(title="Discovered Settings")
    table.add_column("Name", style="bold")
    table.add_column("Current Value")
    table.add_column("Settable")
    table.add_column("Valid Values")
    table.add_column("Menu Path", style="dim")

    for name, si in sorted(caps.settings.items()):
        values_str = ", ".join(si.valid_values[:5])
        if len(si.valid_values) > 5:
            values_str += f" ... (+{len(si.valid_values) - 5})"
        table.add_row(
            name,
            si.current_value or "-",
            "[green]Yes[/green]" if si.is_settable else "[red]No[/red]",
            values_str or "-",
            " > ".join(si.menu_path) if si.menu_path else "-",
        )

    console.print(table)


def _extract_model_from_state(xml_text: str) -> str:
    """Extract camera model from getstate XML response."""
    try:
        root = ET.fromstring(xml_text)
        for elem in root.iter():
            if elem.tag == "product_name" and elem.text:
                return elem.text
    except ET.ParseError:
        pass
    return ""


def _extract_value_from_setting(xml_text: str, setting_name: str) -> str:
    """Extract the current value from a getsetting XML response."""
    try:
        root = ET.fromstring(xml_text)
        # Common patterns: <settingname>value</settingname> or
        # <camrply><result>ok</result><settingname>value</settingname></camrply>
        for elem in root.iter():
            if elem.tag == setting_name and elem.text:
                return elem.text.strip()
        # Fallback: look for any element with a value-like text
        for elem in root.iter():
            if elem.text and elem.tag not in ("result", "camrply") and elem.text.strip():
                return elem.text.strip()
    except ET.ParseError:
        pass
    return ""


def _build_menu_path(elem: ET.Element, root: ET.Element) -> list[str]:
    """Build menu path for an element by traversing parents."""
    # Simple approach: collect parent titleinfo/menuinfo ids
    path = []
    # ElementTree doesn't have parent references, so we just return the id
    title = elem.get("title", "")
    if title:
        path.append(title)
    return path
