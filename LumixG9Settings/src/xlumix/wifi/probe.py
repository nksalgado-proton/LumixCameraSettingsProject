"""Probe the Lumix camera WiFi API to discover capabilities and settings.

This module systematically queries the camera to build a complete map of
available settings, their current values, and valid options.
"""

import json
import sys
from pathlib import Path

from rich.console import Console
from rich.panel import Panel
from rich.table import Table
from rich.tree import Tree

from xlumix.wifi.client import LumixClient

console = Console()

# Info types to query
INFO_TYPES = ["allmenu", "curmenu", "capability", "lens"]

# Known setting types to probe individually
KNOWN_SETTINGS = [
    "iso",
    "shtrspeed",
    "focal",
    "exposure",
    "whitebalance",
    "colormode",
    "focusmode",
    "lightmetering",
    "videoquality",
    "mf_asst",
    "mf_asst_mag",
    "ex_tele_conv",
    "touch_type",
    "peaking",
    "resolution",
    "quality",
]

# Camera commands to test (safe, read-only-ish commands first)
SAFE_COMMANDS = [
    "recmode",
    "playmode",
]

# Commands to test carefully
EXPLORATORY_COMMANDS = [
    "menu_entry",
]


def run_probe(ip: str, output_path: str | None = None):
    """Run a full probe of the camera's WiFi API capabilities."""
    results: dict = {"ip": ip, "info": {}, "settings": {}, "commands": {}}

    with LumixClient(ip=ip) as client:
        # Step 1: Check reachability
        console.print(f"\n[bold]Probing Lumix camera at {ip}...[/bold]\n")

        if not client.is_reachable():
            console.print(
                f"[red]Camera not reachable at {ip}.[/red]\n"
                "Make sure:\n"
                "  1. Camera WiFi is enabled\n"
                "  2. PC is connected to camera's WiFi network\n"
                "  3. IP address is correct (default: 192.168.54.1)"
            )
            sys.exit(1)

        console.print("[green]Camera is reachable![/green]\n")

        # Step 2: Connect / pair
        console.print("[bold]Step 1:[/bold] Pairing with camera...")
        try:
            connect_resp = client.connect()
            results["connect"] = connect_resp
            console.print(f"  Response: {connect_resp[:200]}\n")
        except Exception as e:
            console.print(f"  [yellow]Pairing response: {e}[/yellow]\n")

        # Step 3: Get state
        console.print("[bold]Step 2:[/bold] Getting camera state...")
        try:
            state = client.get_state()
            results["state"] = state
            console.print(f"  State length: {len(state)} chars\n")
        except Exception as e:
            console.print(f"  [yellow]Error: {e}[/yellow]\n")

        # Step 4: Query all info types
        console.print("[bold]Step 3:[/bold] Querying info endpoints...")
        for info_type in INFO_TYPES:
            try:
                resp = client.get_info(info_type)
                results["info"][info_type] = resp
                console.print(f"  [green]✓[/green] {info_type}: {len(resp)} chars")
            except Exception as e:
                results["info"][info_type] = f"ERROR: {e}"
                console.print(f"  [red]✗[/red] {info_type}: {e}")
        console.print()

        # Step 5: Query known settings
        console.print("[bold]Step 4:[/bold] Probing individual settings...")
        for setting in KNOWN_SETTINGS:
            try:
                resp = client.get_setting(setting)
                results["settings"][setting] = resp
                # Try to extract a short summary
                summary = resp[:120].replace("\n", " ")
                console.print(f"  [green]✓[/green] {setting}: {summary}")
            except Exception as e:
                results["settings"][setting] = f"ERROR: {e}"
                console.print(f"  [red]✗[/red] {setting}: {e}")
        console.print()

        # Step 6: Test safe commands
        console.print("[bold]Step 5:[/bold] Testing camera commands...")
        for cmd in SAFE_COMMANDS:
            try:
                resp = client.cam_cmd(cmd)
                results["commands"][cmd] = resp
                console.print(f"  [green]✓[/green] {cmd}: {resp[:120]}")
            except Exception as e:
                results["commands"][cmd] = f"ERROR: {e}"
                console.print(f"  [red]✗[/red] {cmd}: {e}")

        # Test menu_entry separately with a warning
        console.print("\n[bold]Step 6:[/bold] Testing menu_entry command...")
        for cmd in EXPLORATORY_COMMANDS:
            try:
                resp = client.cam_cmd(cmd)
                results["commands"][cmd] = resp
                console.print(f"  [green]✓[/green] {cmd}: {resp[:200]}")
            except Exception as e:
                results["commands"][cmd] = f"ERROR: {e}"
                console.print(f"  [red]✗[/red] {cmd}: {e}")
        console.print()

    # Save results
    if output_path:
        path = Path(output_path)
        path.write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")
        console.print(f"[bold green]Results saved to {path}[/bold green]\n")
    else:
        console.print("[dim]Tip: use --output probe_results.json to save results[/dim]\n")

    # Print summary
    _print_summary(results)


def run_dump(ip: str):
    """Dump current camera settings in a readable format."""
    with LumixClient(ip=ip) as client:
        if not client.is_reachable():
            console.print(f"[red]Camera not reachable at {ip}.[/red]")
            sys.exit(1)

        console.print(f"\n[bold]Dumping settings from {ip}...[/bold]\n")

        try:
            client.connect()
        except Exception:
            pass

        # Get current menu state
        try:
            curmenu = client.get_info("curmenu")
            console.print(Panel(curmenu, title="Current Menu", border_style="blue"))
        except Exception as e:
            console.print(f"[red]Failed to get current menu: {e}[/red]")

        # Get individual settings
        table = Table(title="Individual Settings")
        table.add_column("Setting", style="bold")
        table.add_column("Response")

        for setting in KNOWN_SETTINGS:
            try:
                resp = client.get_setting(setting)
                short = resp[:150].replace("\n", " ").strip()
                table.add_row(setting, short)
            except Exception as e:
                table.add_row(setting, f"[red]{e}[/red]")

        console.print(table)


def _print_summary(results: dict):
    """Print a summary of probe results."""
    tree = Tree("[bold]Probe Summary[/bold]")

    info_branch = tree.add("[bold]Info Endpoints[/bold]")
    for k, v in results.get("info", {}).items():
        status = "[green]OK[/green]" if not str(v).startswith("ERROR") else "[red]FAIL[/red]"
        info_branch.add(f"{k}: {status}")

    settings_branch = tree.add("[bold]Settings[/bold]")
    ok = sum(1 for v in results.get("settings", {}).values() if not str(v).startswith("ERROR"))
    fail = len(results.get("settings", {})) - ok
    settings_branch.add(f"[green]{ok} accessible[/green], [red]{fail} failed[/red]")

    cmd_branch = tree.add("[bold]Commands[/bold]")
    for k, v in results.get("commands", {}).items():
        status = "[green]OK[/green]" if not str(v).startswith("ERROR") else "[red]FAIL[/red]"
        cmd_branch.add(f"{k}: {status}")

    console.print(tree)
