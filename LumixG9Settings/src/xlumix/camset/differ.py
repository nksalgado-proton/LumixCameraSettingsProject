"""Diff two CamSet DAT files to identify which bytes changed.

This is the core tool for reverse-engineering the DAT format:
change one setting on the camera, save a new CamSet, then diff
against the baseline to find the byte offset for that setting.
"""

from pathlib import Path

from rich.console import Console
from rich.table import Table

console = Console()


def diff_camset(file_a: str, file_b: str, context: int = 4):
    """Compare two CamSet DAT files and show byte-level differences."""
    path_a, path_b = Path(file_a), Path(file_b)
    data_a, data_b = path_a.read_bytes(), path_b.read_bytes()

    console.print("\n[bold]Comparing:[/bold]")
    console.print(f"  A: {path_a.name} ({len(data_a)} bytes)")
    console.print(f"  B: {path_b.name} ({len(data_b)} bytes)\n")

    if len(data_a) != len(data_b):
        console.print(
            f"[yellow]Warning: files differ in size "
            f"({len(data_a)} vs {len(data_b)} bytes)[/yellow]\n"
        )

    # Find all differing byte positions
    min_len = min(len(data_a), len(data_b))
    diffs = [(i, data_a[i], data_b[i]) for i in range(min_len) if data_a[i] != data_b[i]]

    if not diffs:
        console.print("[green]Files are identical.[/green]")
        return diffs

    console.print(f"[bold]{len(diffs)} bytes differ:[/bold]\n")

    # Group consecutive diffs into regions
    regions = _group_diffs(diffs, gap=context)

    table = Table(title="Byte Differences")
    table.add_column("Offset", style="bold cyan")
    table.add_column("A (hex)", style="red")
    table.add_column("B (hex)", style="green")
    table.add_column("A (dec)")
    table.add_column("B (dec)")
    table.add_column("A (ascii)")
    table.add_column("B (ascii)")

    for region in regions:
        for offset, val_a, val_b in region:
            char_a = chr(val_a) if 32 <= val_a < 127 else "."
            char_b = chr(val_b) if 32 <= val_b < 127 else "."
            table.add_row(
                f"0x{offset:04x}",
                f"0x{val_a:02x}",
                f"0x{val_b:02x}",
                str(val_a),
                str(val_b),
                char_a,
                char_b,
            )
        # Add separator between regions
        if region != regions[-1]:
            table.add_row("---", "---", "---", "---", "---", "---", "---")

    console.print(table)
    console.print(
        "\n[dim]Tip: use 'xlumix camset record-diff' for structural record-level comparison[/dim]"
    )
    return diffs


def _group_diffs(
    diffs: list[tuple[int, int, int]], gap: int = 4
) -> list[list[tuple[int, int, int]]]:
    """Group consecutive diffs into regions separated by gaps."""
    if not diffs:
        return []
    regions = [[diffs[0]]]
    for diff in diffs[1:]:
        if diff[0] - regions[-1][-1][0] <= gap:
            regions[-1].append(diff)
        else:
            regions.append([diff])
    return regions
