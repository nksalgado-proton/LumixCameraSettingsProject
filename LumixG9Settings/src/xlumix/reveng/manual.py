"""Offline manual mapping workflow — no WiFi required.

Maps camera settings to CamSet byte positions by diffing DAT files.
The user changes one setting on the camera, saves CamSet, and provides
the changed DAT file. This module diffs it against a baseline and records
the mapping.

Designed for settings that can't be changed via WiFi (Fn buttons, Q.Menu,
My Menu, etc.).
"""

import json
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path

from rich.console import Console
from rich.panel import Panel
from rich.prompt import Prompt
from rich.table import Table

from xlumix.camset.bytemap import ByteMap, SettingMapping, load_bytemap, save_bytemap
from xlumix.camset.parser import parse_camset_file
from xlumix.camset.record_differ import diff_records

console = Console()


@dataclass
class ManualMapping:
    """Record of one manual mapping step."""

    setting_name: str
    value_label: str  # human description, e.g. "Preview", "AF-ON"
    baseline_dat: str
    changed_dat: str
    record_id: int = 0
    byte_offset: int = 0
    byte_width: int = 0
    old_bytes: list[int] = field(default_factory=list)
    new_bytes: list[int] = field(default_factory=list)
    timestamp: str = ""


@dataclass
class ManualSession:
    """Persistent session for manual mapping."""

    camera_model: str = ""
    firmware_version: str = ""
    baseline_path: str = ""
    bytemap_path: str = ""
    history: list[ManualMapping] = field(default_factory=list)
    created_at: str = ""

    def __post_init__(self):
        if not self.created_at:
            self.created_at = datetime.now(timezone.utc).isoformat()


def map_single(
    baseline_path: str,
    changed_path: str,
    setting_name: str,
    value_label: str,
    bytemap_path: str | None = None,
) -> ManualMapping | None:
    """Map one setting change by diffing baseline vs changed DAT.

    Returns the ManualMapping if exactly one record changed, or None if
    the diff is ambiguous or empty.
    """
    baseline = parse_camset_file(baseline_path)
    changed = parse_camset_file(changed_path)

    diffs = diff_records(baseline, changed)

    # Filter to records with payload changes (ignore records only in one file)
    payload_diffs = [d for d in diffs if d.only_in is None and d.payload_diffs]

    if not payload_diffs:
        console.print("[yellow]No byte differences found between the two files.[/yellow]")
        console.print("Possible causes:")
        console.print("  - The setting is not stored in CamSet")
        console.print("  - The wrong file was provided")
        console.print("  - The setting was already at the target value")
        return None

    mapping = ManualMapping(
        setting_name=setting_name,
        value_label=value_label,
        baseline_dat=baseline_path,
        changed_dat=changed_path,
        timestamp=datetime.now(timezone.utc).isoformat(),
    )

    # Show all diffs
    console.print(f"\n[bold]Changes for: {setting_name} = {value_label}[/bold]")

    if len(payload_diffs) > 1:
        console.print(
            f"[yellow]Warning: {len(payload_diffs)} records changed. "
            f"Multiple settings may have been modified.[/yellow]"
        )

    table = Table()
    table.add_column("Record ID", style="bold cyan")
    table.add_column("Payload Offset", style="dim")
    table.add_column("Old", style="red")
    table.add_column("New", style="green")

    for d in payload_diffs:
        for i, (off, va, vb) in enumerate(d.payload_diffs):
            rid_str = f"0x{d.record_id:04x}" if i == 0 else ""
            table.add_row(rid_str, f"+0x{off:03x}", f"0x{va:02x}", f"0x{vb:02x}")

    console.print(table)

    if len(payload_diffs) == 1:
        d = payload_diffs[0]
        mapping.record_id = d.record_id
        # Use first contiguous run of changed bytes
        offsets = [off for off, _, _ in d.payload_diffs]
        mapping.byte_offset = offsets[0]
        mapping.byte_width = len(offsets)
        mapping.old_bytes = [va for _, va, _ in d.payload_diffs]
        mapping.new_bytes = [vb for _, _, vb in d.payload_diffs]

        console.print(
            f"\n[green]Mapped: {setting_name} -> record 0x{d.record_id:04x} "
            f"offset +0x{offsets[0]:03x} ({len(offsets)} byte(s))[/green]"
        )
    else:
        # Multiple records changed — let user pick which one
        console.print("\nMultiple records changed. Which record is this setting?")
        choices = [f"0x{d.record_id:04x}" for d in payload_diffs]
        choice = Prompt.ask("Record ID", choices=choices + ["skip"])
        if choice == "skip":
            return None
        rid = int(choice, 16)
        d = next(x for x in payload_diffs if x.record_id == rid)
        mapping.record_id = d.record_id
        offsets = [off for off, _, _ in d.payload_diffs]
        mapping.byte_offset = offsets[0]
        mapping.byte_width = len(offsets)
        mapping.old_bytes = [va for _, va, _ in d.payload_diffs]
        mapping.new_bytes = [vb for _, _, vb in d.payload_diffs]

    # Update bytemap if path provided
    if bytemap_path and mapping.record_id:
        bm_path = Path(bytemap_path)
        if bm_path.exists():
            bytemap = load_bytemap(bm_path)
        else:
            bytemap = ByteMap(
                camera_model=baseline.header.model,
                firmware_version="unknown",
            )

        # Build value encoding
        if mapping.byte_width == 1:
            encoding = "uint8"
        elif mapping.byte_width == 2:
            encoding = "uint16_le"
        elif mapping.byte_width == 4:
            encoding = "uint32_le"
        else:
            encoding = "raw"

        # Build value from new bytes
        if mapping.byte_width <= 4:
            new_val = int.from_bytes(bytes(mapping.new_bytes), "little")
            old_val = int.from_bytes(bytes(mapping.old_bytes), "little")
        else:
            new_val = mapping.new_bytes[0]
            old_val = mapping.old_bytes[0]

        # Check if mapping already exists — merge value_map
        existing = bytemap.find_mapping(setting_name)
        value_map = {}
        if existing and existing.value_map:
            value_map = dict(existing.value_map)
        value_map[value_label] = new_val

        bytemap.add_mapping(
            SettingMapping(
                setting_name=setting_name,
                record_id=mapping.record_id,
                byte_offset=mapping.byte_offset,
                byte_width=mapping.byte_width,
                value_encoding=encoding,
                value_map=value_map,
                confidence="tentative",
                notes=f"old=0x{old_val:x} new=0x{new_val:x}",
            )
        )
        save_bytemap(bytemap, bm_path)
        console.print(f"[dim]ByteMap saved to {bm_path}[/dim]")

    return mapping


def run_interactive_manual(baseline_path: str, output_dir: str):
    """Run an interactive offline mapping session.

    Loop:
    1. Ask user what setting they changed and to what value
    2. Accept path to the changed DAT file
    3. Diff, record mapping
    4. Repeat
    """
    output = Path(output_dir)
    output.mkdir(parents=True, exist_ok=True)

    baseline = parse_camset_file(baseline_path)
    bytemap_path = output / "bytemap.json"
    session_path = output / "manual_session.json"

    # Load or create session
    if session_path.exists():
        raw = json.loads(session_path.read_text(encoding="utf-8"))
        history_raw = raw.pop("history", [])
        session = ManualSession(**raw, history=[ManualMapping(**h) for h in history_raw])
        console.print(f"[dim]Resuming session with {len(session.history)} previous mappings[/dim]")
    else:
        session = ManualSession(
            camera_model=baseline.header.model,
            baseline_path=baseline_path,
            bytemap_path=str(bytemap_path),
        )

    console.print(
        Panel(
            f"[bold]Manual CamSet Mapping Session[/bold]\n\n"
            f"Camera: {baseline.header.model}\n"
            f"Baseline: {baseline_path}\n"
            f"Output: {output_dir}\n\n"
            "Workflow:\n"
            "  1. Change ONE setting on camera\n"
            "  2. Save CamSet (Menu > Setup > Save/Restore Camera Setting > Save)\n"
            "  3. Copy DAT file to PC (from SD card AD_LUMIX/CAMSET/)\n"
            "  4. Enter the details below\n\n"
            "Type 'quit' at any prompt to stop.",
            border_style="blue",
        )
    )

    while True:
        console.print("\n" + "=" * 60)

        setting_name = Prompt.ask("\n[bold]Setting name[/bold] (e.g. fn1, fn2, qmenu_slot1)")
        if setting_name.lower() == "quit":
            break

        value_label = Prompt.ask(f"[bold]Value of {setting_name}[/bold] (e.g. Preview, AF-ON)")
        if value_label.lower() == "quit":
            break

        dat_path = Prompt.ask("[bold]Path to changed DAT file[/bold]")
        if dat_path.lower() == "quit":
            break
        dat_path = dat_path.strip().strip('"').strip("'")

        if not Path(dat_path).exists():
            console.print(f"[red]File not found: {dat_path}[/red]")
            continue

        result = map_single(
            baseline_path=baseline_path,
            changed_path=dat_path,
            setting_name=setting_name,
            value_label=value_label,
            bytemap_path=str(bytemap_path),
        )

        if result:
            session.history.append(result)

        # Save session
        session_data = asdict(session)
        session_path.write_text(
            json.dumps(session_data, indent=2, ensure_ascii=False), encoding="utf-8"
        )
        console.print(f"[dim]Session saved ({len(session.history)} mappings)[/dim]")

        # Show current progress
        if bytemap_path.exists():
            bm = load_bytemap(bytemap_path)
            console.print(f"\n[bold]Current ByteMap: {len(bm.mappings)} setting(s) mapped[/bold]")
            for m in bm.mappings:
                vals = ", ".join(f"{k}=0x{v:x}" for k, v in m.value_map.items())
                console.print(
                    f"  {m.setting_name}: rec 0x{m.record_id:04x} +0x{m.byte_offset:03x} "
                    f"({m.byte_width}B) [{vals}]"
                )

    console.print("\n[bold]Session complete.[/bold]")
    if bytemap_path.exists():
        bm = load_bytemap(bytemap_path)
        console.print(f"Total mappings: {len(bm.mappings)}")
        console.print(f"ByteMap: {bytemap_path}")


def add_values_to_mapping(
    dat_path: str,
    bytemap_path: str,
    setting_name: str,
    value_label: str,
):
    """Read a DAT file and add the current byte value for a known mapping.

    Useful when you already know a setting's position and want to record
    what value corresponds to a specific camera function.
    """
    bm = load_bytemap(Path(bytemap_path))
    camset = parse_camset_file(dat_path)

    mapping = bm.find_mapping(setting_name)
    if not mapping:
        console.print(f"[red]No mapping found for '{setting_name}'[/red]")
        return

    # Find the record
    record = None
    for rec in camset.records:
        if rec.record_id == mapping.record_id:
            record = rec
            break

    if not record:
        console.print(f"[red]Record 0x{mapping.record_id:04x} not found in DAT[/red]")
        return

    # Read bytes
    raw = record.payload[mapping.byte_offset : mapping.byte_offset + mapping.byte_width]
    if mapping.byte_width <= 4:
        val = int.from_bytes(raw, "little")
    else:
        val = raw[0]

    mapping.value_map[value_label] = val
    bm.updated_at = datetime.now(timezone.utc).isoformat()
    save_bytemap(bm, Path(bytemap_path))

    console.print(
        f"[green]Added: {setting_name} '{value_label}' = 0x{val:x} "
        f"(record 0x{mapping.record_id:04x} +0x{mapping.byte_offset:03x})[/green]"
    )
