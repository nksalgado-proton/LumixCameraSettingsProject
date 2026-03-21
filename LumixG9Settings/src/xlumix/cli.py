"""Command-line interface for XLumix G9 Settings."""

import argparse
import sys


def _show_bytemap(bytemap):
    """Display a ByteMap in a rich table."""
    from rich.console import Console
    from rich.table import Table

    console = Console()
    console.print(f"\n[bold]ByteMap: {bytemap.camera_model} FW {bytemap.firmware_version}[/bold]")
    console.print(f"  Mappings: {len(bytemap.mappings)}")
    console.print(f"  Updated: {bytemap.updated_at}\n")

    table = Table(title="Setting Mappings")
    table.add_column("Setting", style="bold")
    table.add_column("Record ID", style="cyan")
    table.add_column("Offset")
    table.add_column("Width")
    table.add_column("Encoding")
    table.add_column("Confidence")
    table.add_column("Slot")
    table.add_column("Values", style="dim")

    for m in bytemap.mappings:
        values_str = ""
        if m.value_map:
            items = list(m.value_map.items())[:4]
            values_str = ", ".join(f"{k}={v}" for k, v in items)
            if len(m.value_map) > 4:
                values_str += " ..."
        table.add_row(
            m.setting_name,
            f"0x{m.record_id:04x}",
            f"+0x{m.byte_offset:03x}",
            str(m.byte_width),
            m.value_encoding,
            m.confidence,
            str(m.slot_index) if m.slot_index is not None else "-",
            values_str or "-",
        )

    console.print(table)


def _show_parsed(camset):
    """Display parsed CamSet file structure."""
    from rich.console import Console
    from rich.table import Table

    console = Console()
    h = camset.header
    console.print(f"\n[bold]CamSet File: {camset.source_path}[/bold]")
    console.print(f"  Model: {h.model}")
    console.print(f"  Magic: {h.magic}")
    console.print(f"  Data length: {h.data_length} bytes")
    console.print(f"  Records: {len(camset.records)}")
    console.print(f"  Checksum byte: 0x{camset.checksum_byte:02x}\n")

    # Size distribution
    sizes: dict[int, int] = {}
    extended_count = 0
    for rec in camset.records:
        sizes[rec.total_size] = sizes.get(rec.total_size, 0) + 1
        if rec.is_extended:
            extended_count += 1
    console.print(f"  Standard records: {len(camset.records) - extended_count}")
    console.print(f"  Extended records: {extended_count}\n")

    table = Table(title="Records")
    table.add_column("#", style="dim")
    table.add_column("ID", style="bold cyan")
    table.add_column("Offset")
    table.add_column("Size")
    table.add_column("Type")
    table.add_column("Payload (first 16 bytes)")

    for i, rec in enumerate(camset.records):
        payload_hex = rec.payload[:16].hex(" ")
        if len(rec.payload) > 16:
            payload_hex += " ..."
        table.add_row(
            str(i),
            f"0x{rec.record_id:04x}",
            f"0x{rec.offset:06x}",
            str(rec.total_size),
            "EXT" if rec.is_extended else "STD",
            payload_hex,
        )

    console.print(table)


def main():
    parser = argparse.ArgumentParser(
        prog="xlumix",
        description="Configure Panasonic Lumix G9 II settings from desktop",
    )
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # Probe command - discover camera WiFi API capabilities
    probe_parser = subparsers.add_parser("probe", help="Probe camera WiFi API capabilities")
    probe_parser.add_argument(
        "--ip", default="192.168.54.1", help="Camera IP address (default: 192.168.54.1)"
    )
    probe_parser.add_argument(
        "--output", "-o", default=None, help="Save probe results to JSON file"
    )

    # Dump command - dump current camera settings
    dump_parser = subparsers.add_parser("dump", help="Dump current camera settings")
    dump_parser.add_argument(
        "--ip", default="192.168.54.1", help="Camera IP address (default: 192.168.54.1)"
    )

    # CamSet commands (future)
    camset_parser = subparsers.add_parser("camset", help="Work with CamSet DAT files")
    camset_sub = camset_parser.add_subparsers(dest="camset_command")
    camset_sub.add_parser("read", help="Read and display a CamSet DAT file").add_argument(
        "file", help="Path to .DAT file"
    )
    diff_parser = camset_sub.add_parser("diff", help="Diff two CamSet DAT files")
    diff_parser.add_argument("file_a", help="First (baseline) .DAT file")
    diff_parser.add_argument("file_b", help="Second (changed) .DAT file")
    camset_sub.add_parser("parse", help="Parse DAT file structure").add_argument(
        "file", help="Path to .DAT file"
    )
    rdiff_parser = camset_sub.add_parser("record-diff", help="Record-level diff of two DAT files")
    rdiff_parser.add_argument("file_a", help="First (baseline) .DAT file")
    rdiff_parser.add_argument("file_b", help="Second (changed) .DAT file")

    # camset map — single offline mapping (no WiFi)
    map_parser = camset_sub.add_parser(
        "map", help="Map a setting by diffing baseline vs changed DAT (no WiFi needed)"
    )
    map_parser.add_argument("baseline", help="Baseline .DAT file")
    map_parser.add_argument("changed", help="Changed .DAT file (after one setting change)")
    map_parser.add_argument(
        "-s", "--setting", required=True, help="Setting name (e.g. fn1, qmenu_slot1)"
    )
    map_parser.add_argument(
        "-v", "--value", required=True, help="Value label (e.g. Preview, AF-ON)"
    )
    map_parser.add_argument(
        "-b", "--bytemap", default="data/bytemaps/DC-G9M2.json",
        help="ByteMap JSON file to update (default: data/bytemaps/DC-G9M2.json)",
    )

    # camset map-session — interactive offline mapping
    mapsess_parser = camset_sub.add_parser(
        "map-session", help="Interactive manual mapping session (no WiFi needed)"
    )
    mapsess_parser.add_argument("baseline", help="Baseline .DAT file")
    mapsess_parser.add_argument(
        "--output-dir", "-o", default="data/bytemaps",
        help="Output directory for bytemap and session (default: data/bytemaps)",
    )

    # camset read-value — read a mapped setting value from a DAT file
    readval_parser = camset_sub.add_parser(
        "read-value", help="Read a mapped setting's value from a DAT file"
    )
    readval_parser.add_argument("dat", help=".DAT file to read from")
    readval_parser.add_argument("-s", "--setting", required=True, help="Setting name")
    readval_parser.add_argument("-v", "--value-label", help="Label for the value (adds to bytemap)")
    readval_parser.add_argument(
        "-b", "--bytemap", default="data/bytemaps/DC-G9M2.json",
        help="ByteMap JSON file",
    )

    # Discover command - full setting discovery via WiFi API
    discover_parser = subparsers.add_parser(
        "discover", help="Discover all camera settings via WiFi API"
    )
    discover_parser.add_argument("--ip", default="192.168.54.1", help="Camera IP address")
    discover_parser.add_argument("--output", "-o", default=None, help="Save capabilities JSON")
    discover_parser.add_argument(
        "--save-xml", default=None, help="Save raw XML responses to directory (for test fixtures)"
    )

    # Reveng command - reverse-engineering workflow
    reveng_parser = subparsers.add_parser("reveng", help="Reverse-engineer CamSet DAT format")
    reveng_sub = reveng_parser.add_subparsers(dest="reveng_command")
    single_parser = reveng_sub.add_parser("single", help="Map a single setting")
    single_parser.add_argument("--ip", default="192.168.54.1", help="Camera IP address")
    single_parser.add_argument("--baseline", required=True, help="Baseline DAT file")
    single_parser.add_argument("-s", "--setting", required=True, help="Setting name")
    single_parser.add_argument("-v", "--value", required=True, help="Target value")
    start_parser = reveng_sub.add_parser("start", help="Start interactive session")
    start_parser.add_argument("--ip", default="192.168.54.1", help="Camera IP address")
    start_parser.add_argument("--baseline", required=True, help="Baseline DAT file")
    start_parser.add_argument("--capabilities", required=True, help="Capabilities JSON file")
    start_parser.add_argument("--output-dir", required=True, help="Output directory")

    # ByteMap command
    bytemap_parser = subparsers.add_parser("bytemap", help="View or validate byte maps")
    bytemap_sub = bytemap_parser.add_subparsers(dest="bytemap_command")
    bytemap_sub.add_parser("show", help="Display byte map").add_argument(
        "file", help="ByteMap JSON file"
    )
    validate_parser = bytemap_sub.add_parser(
        "validate", help="Validate byte map against a DAT file"
    )
    validate_parser.add_argument("file", help="ByteMap JSON file")
    validate_parser.add_argument("dat", help="CamSet DAT file to validate against")
    validate_parser.add_argument(
        "--ip", default=None, help="Camera IP for WiFi cross-check (optional)"
    )

    args = parser.parse_args()
    if not args.command:
        parser.print_help()
        sys.exit(1)

    if args.command == "discover":
        from pathlib import Path

        from xlumix.wifi.client import LumixClient
        from xlumix.wifi.discovery import (
            discover_all_settings,
            save_capabilities,
            save_raw_xml,
            show_capabilities,
        )

        with LumixClient(ip=args.ip) as client:
            # Handshake: pair and switch to rec mode
            try:
                client.connect()
            except Exception:
                pass
            try:
                client.cam_cmd("recmode")
            except Exception:
                pass

            if args.save_xml:
                save_raw_xml(client, Path(args.save_xml))
            caps = discover_all_settings(client)
            show_capabilities(caps)
            if args.output:
                save_capabilities(caps, Path(args.output))
    elif args.command == "probe":
        from xlumix.wifi.probe import run_probe

        run_probe(args.ip, args.output)
    elif args.command == "dump":
        from xlumix.wifi.probe import run_dump

        run_dump(args.ip)
    elif args.command == "camset":
        if args.camset_command == "read":
            from xlumix.camset.reader import read_camset

            read_camset(args.file)
        elif args.camset_command == "diff":
            from xlumix.camset.differ import diff_camset

            diff_camset(args.file_a, args.file_b)
        elif args.camset_command == "parse":
            from xlumix.camset.parser import parse_camset_file

            _show_parsed(parse_camset_file(args.file))
        elif args.camset_command == "record-diff":
            from xlumix.camset.parser import parse_camset_file
            from xlumix.camset.record_differ import show_record_diff

            show_record_diff(parse_camset_file(args.file_a), parse_camset_file(args.file_b))
        elif args.camset_command == "map":
            from xlumix.reveng.manual import map_single

            map_single(
                baseline_path=args.baseline,
                changed_path=args.changed,
                setting_name=args.setting,
                value_label=args.value,
                bytemap_path=args.bytemap,
            )
        elif args.camset_command == "map-session":
            from xlumix.reveng.manual import run_interactive_manual

            run_interactive_manual(args.baseline, args.output_dir)
        elif args.camset_command == "read-value":
            from xlumix.reveng.manual import add_values_to_mapping

            if args.value_label:
                add_values_to_mapping(args.dat, args.bytemap, args.setting, args.value_label)
            else:
                # Just read and display the current value
                from pathlib import Path

                from xlumix.camset.bytemap import load_bytemap
                from xlumix.camset.parser import parse_camset_file

                bm = load_bytemap(Path(args.bytemap))
                camset = parse_camset_file(args.dat)
                mapping = bm.find_mapping(args.setting)
                if not mapping:
                    print(f"No mapping found for '{args.setting}'")
                    sys.exit(1)
                for rec in camset.records:
                    if rec.record_id == mapping.record_id:
                        end = mapping.byte_offset + mapping.byte_width
                        raw = rec.payload[mapping.byte_offset : end]
                        val = int.from_bytes(raw, "little") if mapping.byte_width <= 4 else raw[0]
                        # Reverse lookup in value_map
                        label = next((k for k, v in mapping.value_map.items() if v == val), "?")
                        print(f"{args.setting} = 0x{val:x} ({label})")
                        break
        else:
            camset_parser.print_help()
            sys.exit(1)
    elif args.command == "reveng":
        if args.reveng_command == "single":
            from xlumix.reveng.workflow import run_single
            from xlumix.wifi.client import LumixClient

            with LumixClient(ip=args.ip) as client:
                run_single(client, args.baseline, args.setting, args.value)
        elif args.reveng_command == "start":
            from xlumix.reveng.workflow import run_interactive
            from xlumix.wifi.client import LumixClient
            from xlumix.wifi.discovery import load_capabilities

            caps = load_capabilities(Path(args.capabilities))
            with LumixClient(ip=args.ip) as client:
                run_interactive(client, args.baseline, caps, args.output_dir)
        else:
            reveng_parser.print_help()
            sys.exit(1)
    elif args.command == "bytemap":
        if args.bytemap_command == "show":
            from xlumix.camset.bytemap import load_bytemap

            _show_bytemap(load_bytemap(Path(args.file)))
        elif args.bytemap_command == "validate":
            from xlumix.camset.bytemap import load_bytemap
            from xlumix.camset.parser import parse_camset_file
            from xlumix.camset.validator import (
                show_validation_report,
                validate_bytemap,
                validate_bytemap_with_wifi,
            )

            bm = load_bytemap(Path(args.file))
            camset = parse_camset_file(args.dat)
            if args.ip:
                from xlumix.wifi.client import LumixClient

                with LumixClient(ip=args.ip) as client:
                    wifi_values = {}
                    for m in bm.mappings:
                        if m.wifi_type:
                            try:
                                resp = client.get_setting(m.wifi_type)
                                from xlumix.wifi.discovery import (
                                    _extract_value_from_setting,
                                )

                                wifi_values[m.setting_name] = _extract_value_from_setting(
                                    resp, m.wifi_type
                                )
                            except Exception:
                                pass
                    report = validate_bytemap_with_wifi(bm, camset, wifi_values)
            else:
                report = validate_bytemap(bm, camset)
            show_validation_report(report)
        else:
            bytemap_parser.print_help()
            sys.exit(1)
