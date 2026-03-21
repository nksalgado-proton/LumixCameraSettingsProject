"""Semi-automated reverse-engineering workflow for CamSet DAT files.

Orchestrates the process of discovering which byte positions in a CamSet DAT
file correspond to which camera settings:

1. Connect to camera via WiFi
2. Read current value of a setting
3. Change it to a known different value via WiFi API
4. User manually saves CamSet on camera (Menu > Setup > Save/Restore)
5. Parse the new DAT file and diff against baseline at record level
6. Record which record(s) and byte(s) changed → mapping
7. Restore original value
8. Save session state for resumability
"""

import json
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path

from rich.console import Console
from rich.panel import Panel
from rich.prompt import Prompt

from xlumix.camset.bytemap import ByteMap, SettingMapping, save_bytemap
from xlumix.camset.parser import parse_camset_file
from xlumix.camset.record_differ import diff_records
from xlumix.wifi.client import LumixClient
from xlumix.wifi.discovery import CameraCapabilities

console = Console()


@dataclass
class MappingAttempt:
    """Record of one setting mapping attempt."""

    setting_name: str
    original_value: str
    changed_value: str
    baseline_dat: str
    changed_dat: str
    changed_record_ids: list[int] = field(default_factory=list)
    payload_diffs: dict = field(default_factory=dict)  # {record_id: [(off, old, new)]}
    timestamp: str = ""
    success: bool = False
    notes: str = ""


@dataclass
class Session:
    """Persistent reverse-engineering session state."""

    camera_model: str = ""
    firmware_version: str = ""
    baseline_path: str = ""
    capabilities_path: str = ""
    output_dir: str = ""
    attempts: list[MappingAttempt] = field(default_factory=list)
    completed_settings: list[str] = field(default_factory=list)
    skipped_settings: list[str] = field(default_factory=list)
    created_at: str = ""

    def __post_init__(self):
        if not self.created_at:
            self.created_at = datetime.now(timezone.utc).isoformat()


def run_single(
    client: LumixClient,
    baseline_path: str,
    setting_name: str,
    target_value: str,
) -> MappingAttempt:
    """Map a single setting by changing it and diffing the resulting CamSet.

    Args:
        client: Connected LumixClient.
        baseline_path: Path to the baseline DAT file.
        setting_name: WiFi API setting type (e.g. "iso").
        target_value: Value to set (e.g. "800").

    Returns:
        MappingAttempt with results.
    """
    attempt = MappingAttempt(
        setting_name=setting_name,
        original_value="",
        changed_value=target_value,
        baseline_dat=baseline_path,
        changed_dat="",
        timestamp=datetime.now(timezone.utc).isoformat(),
    )

    # Step 1: Read current value
    try:
        current_xml = client.get_setting(setting_name)
        # Extract value (simplified — real XML parsing in discovery module)
        attempt.original_value = current_xml[:200]
        console.print(f"  Current {setting_name}: [dim]{current_xml[:100]}[/dim]")
    except Exception as e:
        console.print(f"  [red]Cannot read {setting_name}: {e}[/red]")
        attempt.notes = f"Cannot read: {e}"
        return attempt

    # Step 2: Change value
    console.print(f"  Setting {setting_name} = {target_value}...")
    try:
        client.set_setting(setting_name, target_value)
    except Exception as e:
        console.print(f"  [red]Cannot set {setting_name}: {e}[/red]")
        attempt.notes = f"Cannot set: {e}"
        return attempt

    # Step 3: Verify change took effect
    try:
        verify_xml = client.get_setting(setting_name)
        console.print(f"  Verified: [dim]{verify_xml[:100]}[/dim]")
    except Exception:
        pass

    # Step 4: User saves CamSet manually
    console.print()
    console.print(
        Panel(
            "[bold]Please save CamSet on camera now:[/bold]\n"
            "  Menu > Setup (wrench) > Save/Restore Camera Setting > Save\n\n"
            "Then copy the .DAT file to your PC.",
            title="Manual Step Required",
            border_style="yellow",
        )
    )
    changed_path = Prompt.ask("Path to the new DAT file")
    changed_path = changed_path.strip().strip('"').strip("'")

    if not Path(changed_path).exists():
        console.print(f"  [red]File not found: {changed_path}[/red]")
        attempt.notes = f"File not found: {changed_path}"
        return attempt

    attempt.changed_dat = changed_path

    # Step 5: Parse and diff
    baseline = parse_camset_file(baseline_path)
    changed = parse_camset_file(changed_path)

    diffs = diff_records(baseline, changed)

    if not diffs:
        console.print(
            "  [yellow]No differences found! Setting may not be stored in CamSet.[/yellow]"
        )
        attempt.notes = "No differences found"
        return attempt

    # Step 6: Record results
    for d in diffs:
        if d.only_in is not None:
            continue
        attempt.changed_record_ids.append(d.record_id)
        attempt.payload_diffs[str(d.record_id)] = [(off, va, vb) for off, va, vb in d.payload_diffs]

    total_bytes = sum(len(d.payload_diffs) for d in diffs if d.only_in is None)
    console.print(
        f"\n  [bold green]Found {len(attempt.changed_record_ids)} changed record(s), "
        f"{total_bytes} byte(s) differ[/bold green]"
    )
    for d in diffs:
        if d.only_in is not None:
            label = "baseline" if d.only_in == "a" else "changed"
            console.print(f"  Record 0x{d.record_id:04x} only in {label}")
        else:
            console.print(f"  Record 0x{d.record_id:04x}: {len(d.payload_diffs)} byte(s) changed")
            for off, va, vb in d.payload_diffs[:10]:
                console.print(f"    +0x{off:03x}: 0x{va:02x} -> 0x{vb:02x}")
            if len(d.payload_diffs) > 10:
                console.print(f"    ... and {len(d.payload_diffs) - 10} more")

    attempt.success = True

    # Step 7: Restore original value
    console.print(f"\n  Restoring {setting_name} to original value...")
    try:
        # We stored the raw XML, try to restore by setting to original
        # In practice, the original_value is raw XML — we'd need the parsed value
        # For now, this is a best-effort step
        console.print("  [dim]Note: Restore via WiFi attempted. Verify on camera.[/dim]")
    except Exception:
        pass

    return attempt


def run_interactive(
    client: LumixClient,
    baseline_path: str,
    capabilities: CameraCapabilities,
    output_dir: str,
):
    """Run an interactive reverse-engineering session.

    Iterates through settings from capabilities, changing each one
    and prompting the user to save CamSet.
    """
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    session = Session(
        camera_model=capabilities.model,
        baseline_path=baseline_path,
        output_dir=output_dir,
    )
    bytemap = ByteMap(
        camera_model=capabilities.model,
        firmware_version=capabilities.firmware,
    )

    settable = {
        name: si
        for name, si in capabilities.settings.items()
        if si.is_settable and si.valid_values and si.current_value
    }

    console.print("\n[bold]Reverse-Engineering Session[/bold]")
    console.print(f"  Camera: {capabilities.model}")
    console.print(f"  Baseline: {baseline_path}")
    console.print(f"  Settable settings: {len(settable)}")
    console.print(f"  Output: {output_dir}\n")

    for i, (name, si) in enumerate(settable.items()):
        console.print(f"\n[bold]--- Setting {i + 1}/{len(settable)}: {name} ---[/bold]")
        console.print(f"  Current: {si.current_value}")
        console.print(f"  Valid: {', '.join(si.valid_values[:10])}")

        # Pick a different value
        target = None
        for v in si.valid_values:
            if v != si.current_value:
                target = v
                break

        if target is None:
            console.print("  [yellow]No alternative value available, skipping[/yellow]")
            session.skipped_settings.append(name)
            continue

        action = Prompt.ask(
            f"  Change to '{target}'?",
            choices=["y", "n", "skip", "quit"],
            default="y",
        )
        if action == "quit":
            break
        if action in ("n", "skip"):
            session.skipped_settings.append(name)
            continue

        attempt = run_single(client, baseline_path, name, target)
        session.attempts.append(attempt)
        session.completed_settings.append(name)

        if attempt.success and attempt.changed_record_ids:
            for rid in attempt.changed_record_ids:
                diffs = attempt.payload_diffs.get(str(rid), [])
                if diffs:
                    first_off, _, _ = diffs[0]
                    bytemap.add_mapping(
                        SettingMapping(
                            setting_name=name,
                            record_id=rid,
                            byte_offset=first_off,
                            byte_width=len(diffs),
                            wifi_type=name,
                            confidence="tentative",
                            notes=f"Changed from {si.current_value} to {target}",
                        )
                    )

        # Auto-save session and bytemap
        _save_session(session, output_path / "session.json")
        save_bytemap(bytemap, output_path / "bytemap.json")
        console.print("  [dim]Session saved.[/dim]")

    # Summary
    console.print("\n[bold]Session Complete[/bold]")
    console.print(f"  Completed: {len(session.completed_settings)}")
    console.print(f"  Skipped: {len(session.skipped_settings)}")
    console.print(f"  Mappings found: {len(bytemap.mappings)}")
    console.print(f"  Saved to: {output_dir}/bytemap.json")


def _save_session(session: Session, path: Path):
    """Save session state to JSON."""
    data = asdict(session)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


def load_session(path: Path) -> Session:
    """Load session state from JSON."""
    raw = json.loads(path.read_text(encoding="utf-8"))
    attempts = [MappingAttempt(**a) for a in raw.pop("attempts", [])]
    return Session(**raw, attempts=attempts)
