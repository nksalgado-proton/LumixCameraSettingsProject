"""Validate a ByteMap against a CamSet DAT file.

Cross-checks that every mapping in the bytemap can be read from the DAT file,
and optionally compares decoded values against the camera's live WiFi state.
"""

import struct
from dataclasses import dataclass, field

from rich.console import Console
from rich.table import Table

from xlumix.camset.bytemap import ByteMap, SettingMapping
from xlumix.camset.parser import CamSetFile

console = Console()


@dataclass
class ValidationResult:
    """Result of validating one setting mapping."""

    setting_name: str
    slot_index: int | None = None
    record_found: bool = False
    dat_raw_value: int | None = None
    dat_decoded: str = ""
    wifi_value: str = ""
    match: bool | None = None  # None = no WiFi comparison
    error: str = ""


@dataclass
class ValidationReport:
    """Aggregate validation results."""

    bytemap_file: str = ""
    dat_file: str = ""
    results: list[ValidationResult] = field(default_factory=list)

    @property
    def total(self) -> int:
        return len(self.results)

    @property
    def records_found(self) -> int:
        return sum(1 for r in self.results if r.record_found)

    @property
    def matched(self) -> int:
        return sum(1 for r in self.results if r.match is True)

    @property
    def mismatched(self) -> int:
        return sum(1 for r in self.results if r.match is False)


def _decode_value(payload: bytes, mapping: SettingMapping) -> tuple[int, str]:
    """Decode a raw value from record payload using the mapping spec.

    Returns (raw_int, decoded_string).
    """
    offset = mapping.byte_offset
    width = mapping.byte_width

    if offset + width > len(payload):
        raise ValueError(f"Offset {offset}+{width} exceeds payload length {len(payload)}")

    raw_bytes = payload[offset : offset + width]

    if width == 1:
        raw_int = raw_bytes[0]
    elif width == 2:
        raw_int = struct.unpack_from("<H", raw_bytes)[0]
    elif width == 4:
        raw_int = struct.unpack_from("<I", raw_bytes)[0]
    else:
        raw_int = int.from_bytes(raw_bytes, "little")

    # Reverse-lookup in value_map
    if mapping.value_map:
        for api_val, dat_val in mapping.value_map.items():
            if dat_val == raw_int:
                return raw_int, api_val
        return raw_int, f"0x{raw_int:x} (unmapped)"

    return raw_int, str(raw_int)


def validate_bytemap(
    bytemap: ByteMap,
    camset: CamSetFile,
) -> ValidationReport:
    """Validate all bytemap mappings against a parsed CamSet file."""
    report = ValidationReport()

    for mapping in bytemap.mappings:
        result = ValidationResult(
            setting_name=mapping.setting_name,
            slot_index=mapping.slot_index,
        )

        record = camset.find_record(mapping.record_id)
        if record is None:
            result.error = f"Record 0x{mapping.record_id:04x} not found"
            report.results.append(result)
            continue

        result.record_found = True

        try:
            raw_int, decoded = _decode_value(record.payload, mapping)
            result.dat_raw_value = raw_int
            result.dat_decoded = decoded
        except ValueError as e:
            result.error = str(e)

        report.results.append(result)

    return report


def validate_bytemap_with_wifi(
    bytemap: ByteMap,
    camset: CamSetFile,
    wifi_values: dict[str, str],
) -> ValidationReport:
    """Validate bytemap against DAT file and WiFi current values.

    Args:
        bytemap: The bytemap to validate.
        camset: Parsed CamSet file.
        wifi_values: Dict of setting_name -> current WiFi value.
    """
    report = validate_bytemap(bytemap, camset)

    for result in report.results:
        wifi_val = wifi_values.get(result.setting_name, "")
        if wifi_val and result.dat_decoded:
            result.wifi_value = wifi_val
            result.match = result.dat_decoded == wifi_val

    return report


def show_validation_report(report: ValidationReport):
    """Display validation results in a rich table."""
    console.print("\n[bold]ByteMap Validation[/bold]")
    console.print(f"  Mappings: {report.total}")
    console.print(f"  Records found: {report.records_found}/{report.total}")
    if report.matched + report.mismatched > 0:
        console.print(f"  WiFi match: {report.matched}, mismatch: {report.mismatched}\n")
    else:
        console.print()

    table = Table(title="Validation Results")
    table.add_column("Setting", style="bold")
    table.add_column("Slot")
    table.add_column("Record")
    table.add_column("Raw Value")
    table.add_column("Decoded")
    table.add_column("WiFi Value")
    table.add_column("Status")

    for r in report.results:
        if r.error:
            status = f"[red]{r.error}[/red]"
        elif r.match is True:
            status = "[green]OK[/green]"
        elif r.match is False:
            status = "[red]MISMATCH[/red]"
        else:
            status = "[dim]no wifi[/dim]"

        table.add_row(
            r.setting_name,
            str(r.slot_index) if r.slot_index is not None else "-",
            f"0x{r.dat_raw_value:04x}" if r.dat_raw_value is not None else "-",
            f"0x{r.dat_raw_value:x}" if r.dat_raw_value is not None else "-",
            r.dat_decoded or "-",
            r.wifi_value or "-",
            status,
        )

    console.print(table)
