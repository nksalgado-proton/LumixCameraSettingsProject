"""Record-level diff of two parsed CamSet files.

Unlike the raw byte differ, this compares records by ID, making it stable
across firmware versions where the same setting may appear at different
file offsets.
"""

from dataclasses import dataclass, field

from rich.console import Console
from rich.table import Table

from xlumix.camset.parser import CamSetFile

console = Console()


@dataclass
class RecordDiff:
    """Difference in a single record between two CamSet files."""

    record_id: int
    payload_diffs: list[tuple[int, int, int]] = field(default_factory=list)
    """List of (byte_offset_in_payload, val_a, val_b)."""
    only_in: str | None = None
    """'a' or 'b' if record exists only in one file."""


def diff_records(file_a: CamSetFile, file_b: CamSetFile) -> list[RecordDiff]:
    """Compare two CamSetFiles at the record level.

    Records are matched by record_id. Returns a list of RecordDiff for
    records that differ or exist in only one file.
    """
    records_a = {rec.record_id: rec for rec in file_a.records}
    records_b = {rec.record_id: rec for rec in file_b.records}

    all_ids = sorted(set(records_a) | set(records_b))
    diffs = []

    for rid in all_ids:
        rec_a = records_a.get(rid)
        rec_b = records_b.get(rid)

        if rec_a is None:
            diffs.append(RecordDiff(record_id=rid, only_in="b"))
        elif rec_b is None:
            diffs.append(RecordDiff(record_id=rid, only_in="a"))
        else:
            payload_diffs = _diff_payloads(rec_a.payload, rec_b.payload)
            if payload_diffs:
                diffs.append(RecordDiff(record_id=rid, payload_diffs=payload_diffs))

    return diffs


def show_record_diff(file_a: CamSetFile, file_b: CamSetFile):
    """Print a record-level diff between two CamSet files."""
    diffs = diff_records(file_a, file_b)

    console.print("\n[bold]Record-Level Diff[/bold]")
    console.print(f"  A: {file_a.header.model} ({len(file_a.records)} records)")
    console.print(f"  B: {file_b.header.model} ({len(file_b.records)} records)\n")

    if not diffs:
        console.print("[green]Files are identical at the record level.[/green]")
        return diffs

    # Separate by type
    only_a = [d for d in diffs if d.only_in == "a"]
    only_b = [d for d in diffs if d.only_in == "b"]
    changed = [d for d in diffs if d.only_in is None]

    if only_a:
        console.print(f"[yellow]{len(only_a)} records only in A:[/yellow]")
        for d in only_a:
            console.print(f"  0x{d.record_id:04x}")

    if only_b:
        console.print(f"[yellow]{len(only_b)} records only in B:[/yellow]")
        for d in only_b:
            console.print(f"  0x{d.record_id:04x}")

    if changed:
        console.print(f"\n[bold]{len(changed)} records with payload changes:[/bold]\n")

        table = Table(title="Changed Records")
        table.add_column("Record ID", style="bold cyan")
        table.add_column("Offset", style="dim")
        table.add_column("A (hex)", style="red")
        table.add_column("B (hex)", style="green")
        table.add_column("Bytes Changed")

        for d in changed:
            for i, (off, va, vb) in enumerate(d.payload_diffs):
                rid_str = f"0x{d.record_id:04x}" if i == 0 else ""
                table.add_row(
                    rid_str,
                    f"+0x{off:03x}",
                    f"0x{va:02x}",
                    f"0x{vb:02x}",
                    str(len(d.payload_diffs)) if i == 0 else "",
                )
            if d != changed[-1]:
                table.add_row("---", "---", "---", "---", "---")

        console.print(table)

    return diffs


def _diff_payloads(payload_a: bytes, payload_b: bytes) -> list[tuple[int, int, int]]:
    """Compare two payloads byte by byte.

    Returns list of (offset, val_a, val_b) for differing bytes.
    """
    diffs = []
    min_len = min(len(payload_a), len(payload_b))

    for i in range(min_len):
        if payload_a[i] != payload_b[i]:
            diffs.append((i, payload_a[i], payload_b[i]))

    # If sizes differ, treat extra bytes as diffs against 0x00
    if len(payload_a) > min_len:
        for i in range(min_len, len(payload_a)):
            diffs.append((i, payload_a[i], 0x00))
    elif len(payload_b) > min_len:
        for i in range(min_len, len(payload_b)):
            diffs.append((i, 0x00, payload_b[i]))

    return diffs
