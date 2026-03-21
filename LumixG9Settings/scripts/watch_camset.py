"""Watch a CamSet DAT file on the SD card for changes and auto-diff.

Usage:
    python scripts/watch_camset.py H:/AD_LUMIX/CAMSET/CAMSET01.DAT

The script watches the file for modifications. When you save CamSet on the
camera, it detects the change, diffs against the baseline, and reports
which records/bytes changed.

Press Ctrl+C to stop.
"""

import hashlib
import sys
import time
from pathlib import Path

# Add src to path so we can import xlumix
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from xlumix.camset.parser import parse_camset_file
from xlumix.camset.record_differ import diff_records


def md5(path: Path) -> str:
    return hashlib.md5(path.read_bytes()).hexdigest()


def show_diff(baseline_path: Path, changed_path: Path, label: str = ""):
    """Parse and diff two DAT files, showing results."""
    baseline = parse_camset_file(baseline_path)
    changed = parse_camset_file(changed_path)

    diffs = diff_records(baseline, changed)

    if not diffs:
        print(f"  No record-level differences found.")
        return

    print(f"  {len(diffs)} record(s) changed:")
    for d in diffs:
        if d.only_in:
            side = "baseline" if d.only_in == "a" else "changed"
            print(f"    Record 0x{d.record_id:04x} only in {side}")
        else:
            print(f"    Record 0x{d.record_id:04x}: {len(d.payload_diffs)} byte(s)")
            for off, va, vb in d.payload_diffs[:20]:
                # Try to show ASCII interpretation
                char_a = chr(va) if 32 <= va < 127 else "."
                char_b = chr(vb) if 32 <= vb < 127 else "."
                print(
                    f"      +0x{off:03x}: "
                    f"0x{va:02x} ({va:3d} '{char_a}') -> "
                    f"0x{vb:02x} ({vb:3d} '{char_b}')"
                )
            if len(d.payload_diffs) > 20:
                print(f"      ... and {len(d.payload_diffs) - 20} more bytes")

    if label:
        print(f"\n  Label this change as: {label}")


def main():
    if len(sys.argv) < 2:
        print("Usage: python scripts/watch_camset.py <path-to-CAMSET01.DAT> [baseline.DAT]")
        print("\nExample:")
        print("  python scripts/watch_camset.py H:/AD_LUMIX/CAMSET/CAMSET01.DAT")
        print("  python scripts/watch_camset.py H:/AD_LUMIX/CAMSET/CAMSET01.DAT data/g9ii/MKIISET_FW2.7.DAT")
        sys.exit(1)

    watch_path = Path(sys.argv[1])
    if not watch_path.exists():
        print(f"Error: {watch_path} not found")
        sys.exit(1)

    # Use explicit baseline or snapshot the current file as baseline
    if len(sys.argv) >= 3:
        baseline_path = Path(sys.argv[2])
        if not baseline_path.exists():
            print(f"Error: baseline {baseline_path} not found")
            sys.exit(1)
    else:
        baseline_path = watch_path  # will snapshot below

    # Take initial snapshot
    baseline_hash = md5(watch_path)
    baseline_snapshot = Path("data/g9ii/_watcher_baseline.DAT")
    baseline_snapshot.parent.mkdir(parents=True, exist_ok=True)

    if baseline_path == watch_path:
        # Snapshot current state as baseline
        baseline_snapshot.write_bytes(watch_path.read_bytes())
        print(f"Baseline: snapshot of {watch_path}")
    else:
        baseline_snapshot.write_bytes(baseline_path.read_bytes())
        print(f"Baseline: {baseline_path}")

    print(f"Watching: {watch_path}")
    print(f"Baseline hash: {baseline_hash[:12]}")
    print(f"\nChange a setting on camera, then Save CamSet.")
    print(f"The diff will appear here automatically.")
    print(f"Press Ctrl+C to stop.\n")

    change_count = 0
    was_connected = watch_path.exists()

    try:
        while True:
            time.sleep(1)

            is_connected = watch_path.exists()

            # Detect USB connect
            if is_connected and not was_connected:
                print(f"\n  USB connected at {time.strftime('%H:%M:%S')}")
                # Small delay to let filesystem settle
                time.sleep(2)

                try:
                    current_hash = md5(watch_path)
                except (PermissionError, OSError):
                    time.sleep(2)
                    try:
                        current_hash = md5(watch_path)
                    except Exception:
                        print("  Could not read file")
                        was_connected = is_connected
                        continue

                if current_hash != baseline_hash:
                    change_count += 1
                    print(f"\n{'='*60}")
                    print(f"  CHANGE #{change_count} at {time.strftime('%H:%M:%S')}")
                    print(f"  Hash: {baseline_hash[:12]} -> {current_hash[:12]}")
                    print(f"{'='*60}")

                    try:
                        changed_copy = Path(
                            f"data/g9ii/_watcher_change_{change_count:03d}.DAT"
                        )
                        changed_copy.write_bytes(watch_path.read_bytes())
                        show_diff(baseline_snapshot, changed_copy)
                    except Exception as e:
                        print(f"  Error diffing: {e}")

                    baseline_hash = current_hash
                    print(f"\n  Disconnect USB, change next setting, save, reconnect.")
                else:
                    print("  No changes since last check.")
                    print("  Disconnect USB, change a setting, save, reconnect.")

            elif not is_connected and was_connected:
                print(f"  USB disconnected. Change a setting and save CamSet...")

            # Also handle already-connected polling (e.g. card reader)
            elif is_connected and was_connected:
                try:
                    current_hash = md5(watch_path)
                except (PermissionError, OSError):
                    was_connected = is_connected
                    continue

                if current_hash != baseline_hash:
                    change_count += 1
                    print(f"\n{'='*60}")
                    print(f"  CHANGE #{change_count} at {time.strftime('%H:%M:%S')}")
                    print(f"  Hash: {baseline_hash[:12]} -> {current_hash[:12]}")
                    print(f"{'='*60}")

                    try:
                        changed_copy = Path(
                            f"data/g9ii/_watcher_change_{change_count:03d}.DAT"
                        )
                        changed_copy.write_bytes(watch_path.read_bytes())
                        show_diff(baseline_snapshot, changed_copy)
                    except Exception as e:
                        print(f"  Error diffing: {e}")

                    baseline_hash = current_hash
                    print(f"\nWaiting for next change...")

            was_connected = is_connected

    except KeyboardInterrupt:
        print(f"\n\nStopped. {change_count} change(s) detected.")
        if baseline_snapshot.exists():
            baseline_snapshot.unlink()


if __name__ == "__main__":
    main()
