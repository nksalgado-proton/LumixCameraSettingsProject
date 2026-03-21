"""ByteMap schema — maps camera settings to byte positions in CamSet DAT files.

A ByteMap is the output of the reverse-engineering process: it records which
record ID and byte offset corresponds to each camera setting, along with
the value encoding and known value mappings.
"""

import json
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path


@dataclass
class SettingMapping:
    """Maps one camera setting to its position in the CamSet binary."""

    setting_name: str  # e.g. "iso", "fn1_assignment"
    record_id: int  # CamSet record ID
    byte_offset: int  # offset within record payload
    byte_width: int  # 1, 2, or 4
    value_encoding: str = "uint8"  # "uint8", "uint16_le", "enum", "bitfield"
    value_map: dict[str, int] = field(default_factory=dict)  # API value -> DAT byte
    wifi_type: str = ""  # WiFi API type parameter, if discoverable
    confidence: str = "unverified"  # "confirmed", "tentative", "unverified"
    slot_index: int | None = None  # for multi-slot records (C1/C2/C3)
    notes: str = ""


@dataclass
class ByteMap:
    """Complete byte map for a camera model and firmware version."""

    camera_model: str  # "DC-G9M2" or "DC-G9"
    firmware_version: str  # "2.7"
    created_at: str = ""
    updated_at: str = ""
    record_format: dict = field(default_factory=dict)
    mappings: list[SettingMapping] = field(default_factory=list)

    def __post_init__(self):
        now = datetime.now(timezone.utc).isoformat()
        if not self.created_at:
            self.created_at = now
        if not self.updated_at:
            self.updated_at = now

    def add_mapping(self, mapping: SettingMapping):
        """Add or replace a setting mapping."""
        # Remove existing mapping with same name + slot_index
        self.mappings = [
            m
            for m in self.mappings
            if not (m.setting_name == mapping.setting_name and m.slot_index == mapping.slot_index)
        ]
        self.mappings.append(mapping)
        self.updated_at = datetime.now(timezone.utc).isoformat()

    def find_mapping(
        self,
        setting_name: str,
        slot_index: int | None = None,
    ) -> SettingMapping | None:
        """Find a mapping by setting name and optional slot index."""
        for m in self.mappings:
            if m.setting_name == setting_name and m.slot_index == slot_index:
                return m
        return None

    def find_by_record_id(self, record_id: int) -> list[SettingMapping]:
        """Find all mappings for a given record ID."""
        return [m for m in self.mappings if m.record_id == record_id]


def save_bytemap(bytemap: ByteMap, path: Path):
    """Save a ByteMap to a JSON file."""
    path.parent.mkdir(parents=True, exist_ok=True)
    data = asdict(bytemap)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


def load_bytemap(path: Path) -> ByteMap:
    """Load a ByteMap from a JSON file."""
    raw = json.loads(path.read_text(encoding="utf-8"))
    mappings = [SettingMapping(**m) for m in raw.pop("mappings", [])]
    return ByteMap(**raw, mappings=mappings)
