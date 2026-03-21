"""Read and analyze CamSet DAT files from Lumix cameras.

CamSet files are binary files stored in AD_LUMIX/CAMSET/ on the SD card.
They contain all camera settings and can be loaded back onto the camera.
"""

from pathlib import Path

from rich.console import Console
from rich.panel import Panel

console = Console()


def read_camset(file_path: str):
    """Read a CamSet DAT file and display basic info about its structure."""
    path = Path(file_path)
    if not path.exists():
        console.print(f"[red]File not found: {path}[/red]")
        return

    data = path.read_bytes()
    console.print(f"\n[bold]CamSet File: {path.name}[/bold]")
    console.print(f"  Size: {len(data)} bytes ({len(data):#x} hex)\n")

    # Show structural info from parser
    try:
        from xlumix.camset.parser import parse_camset

        camset = parse_camset(data, source_path=str(path))
        h = camset.header
        console.print(f"  Model: {h.model}")
        console.print(f"  Magic: {h.magic}")
        console.print(f"  Data length: {h.data_length} bytes")
        console.print(f"  Records: {len(camset.records)}")
        ext = sum(1 for r in camset.records if r.is_extended)
        if ext:
            console.print(f"  Extended records: {ext}")
        console.print(f"  Checksum: 0x{camset.checksum_byte:02x}\n")
    except Exception as e:
        console.print(f"  [yellow]Could not parse structure: {e}[/yellow]\n")

    # Show hex dump of first 256 bytes (header area)
    console.print(Panel(_hex_dump(data, limit=256), title="Header (first 256 bytes)"))

    # Look for ASCII strings in the file
    strings = _extract_strings(data, min_length=4)
    if strings:
        console.print(f"\n[bold]Embedded strings ({len(strings)} found):[/bold]")
        for offset, s in strings[:50]:
            console.print(f"  {offset:#06x}: {s}")

    console.print()


def _hex_dump(data: bytes, limit: int = 256) -> str:
    """Format bytes as a hex dump with ASCII sidebar."""
    lines = []
    for i in range(0, min(len(data), limit), 16):
        chunk = data[i : i + 16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append(f"{i:06x}  {hex_part:<48}  {ascii_part}")
    if len(data) > limit:
        lines.append(f"  ... ({len(data) - limit} more bytes)")
    return "\n".join(lines)


def _extract_strings(data: bytes, min_length: int = 4) -> list[tuple[int, str]]:
    """Extract printable ASCII strings from binary data."""
    strings = []
    current = []
    start = 0
    for i, b in enumerate(data):
        if 32 <= b < 127:
            if not current:
                start = i
            current.append(chr(b))
        else:
            if len(current) >= min_length:
                strings.append((start, "".join(current)))
            current = []
    if len(current) >= min_length:
        strings.append((start, "".join(current)))
    return strings
