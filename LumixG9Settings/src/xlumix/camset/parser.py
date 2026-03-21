"""Parse CamSet DAT files into structured objects.

CamSet files have the following binary layout:
  [Header: 48 bytes]
  [Section marker: 4 bytes at 0x30]
  [Records: variable length, 4-byte aligned]
  [End marker: FF FF 00 00]
  [Checksum: 1 byte]

Records come in two formats:
  Standard: [id:2 LE][total_size:2 LE][payload: total_size-4 bytes]
  Extended: [id:2 LE][0x0000:2][total_size:4 LE][payload: total_size-8 bytes]
  Extended format is triggered when the 2-byte size field is 0.

Records are padded with zero bytes to maintain 4-byte alignment.

data_length at header offset 0x2C = (end_marker_offset - 0x30 + 4).
File size = end_marker_offset + 5 (4 bytes marker + 1 byte checksum).
"""

import struct
from dataclasses import dataclass, field
from pathlib import Path

HEADER_SIZE = 48
SECTION_MARKER_OFFSET = 0x30
RECORDS_START = 0x34
DATA_LENGTH_OFFSET = 0x2C
MODEL_OFFSET = 0x12
END_MARKER = b"\xff\xff\x00\x00"


@dataclass
class CamSetHeader:
    """Parsed header from a CamSet DAT file."""

    magic: str  # "Panasonic"
    model: str  # "DC-G9M2" or "DC-G9"
    data_length: int  # uint32 LE at 0x2C
    section_marker: bytes  # 4 bytes at 0x30
    raw: bytes  # first 52 bytes preserved (header + section marker)


@dataclass
class CamSetRecord:
    """A single record in the CamSet file."""

    offset: int  # byte offset in file
    record_id: int  # uint16 LE
    total_size: int  # total bytes including header
    is_extended: bool  # True if 8-byte header, False if 4-byte
    payload: bytes  # raw data after record header

    @property
    def header_size(self) -> int:
        return 8 if self.is_extended else 4

    @property
    def data_offset(self) -> int:
        """Absolute file position of payload start."""
        return self.offset + self.header_size


@dataclass
class CamSetFile:
    """Fully parsed CamSet DAT file."""

    header: CamSetHeader
    records: list[CamSetRecord] = field(default_factory=list)
    end_marker_offset: int = 0
    checksum_byte: int = 0
    source_path: str | None = None

    def find_record(self, record_id: int) -> CamSetRecord | None:
        """Find first record with given ID."""
        for rec in self.records:
            if rec.record_id == record_id:
                return rec
        return None

    def find_records(self, record_id: int) -> list[CamSetRecord]:
        """Find all records with given ID."""
        return [rec for rec in self.records if rec.record_id == record_id]


def parse_camset(data: bytes, source_path: str | None = None) -> CamSetFile:
    """Parse a CamSet DAT file from raw bytes.

    Args:
        data: Raw file bytes.
        source_path: Optional path for reference.

    Returns:
        Parsed CamSetFile object.

    Raises:
        ValueError: If the file format is invalid.
    """
    if len(data) < RECORDS_START:
        raise ValueError(f"File too small ({len(data)} bytes), minimum is {RECORDS_START}")

    header = _parse_header(data)
    records, end_marker_offset = _parse_records(data)

    if end_marker_offset + 4 >= len(data):
        raise ValueError(f"No checksum byte after end marker at 0x{end_marker_offset:04x}")
    checksum_byte = data[end_marker_offset + 4]

    return CamSetFile(
        header=header,
        records=records,
        end_marker_offset=end_marker_offset,
        checksum_byte=checksum_byte,
        source_path=source_path,
    )


def parse_camset_file(path: str | Path) -> CamSetFile:
    """Parse a CamSet DAT file from disk."""
    path = Path(path)
    data = path.read_bytes()
    return parse_camset(data, source_path=str(path))


def serialize_camset(camset: CamSetFile, recalculate_checksum: bool = False) -> bytes:
    """Reconstruct binary data from a parsed CamSetFile.

    Args:
        camset: Parsed CamSet file object.
        recalculate_checksum: If True, recompute the checksum byte so the file
            is valid after payload modifications. The checksum ensures that the
            sum of all bytes in the file is 0 mod 256.
    """
    parts = [camset.header.raw]

    for i, rec in enumerate(camset.records):
        # Alignment padding before this record (not before the first one)
        if i == 0:
            expected_offset = RECORDS_START
        else:
            prev = camset.records[i - 1]
            expected_offset = prev.offset + prev.total_size
            # Align to 4 bytes
            if expected_offset % 4 != 0:
                expected_offset += 4 - (expected_offset % 4)

        current_len = len(parts[0])  # header.raw length
        for p in parts[1:]:
            current_len += len(p)

        padding_needed = rec.offset - current_len
        if padding_needed > 0:
            parts.append(b"\x00" * padding_needed)

        # Record header
        if rec.is_extended:
            parts.append(struct.pack("<HHI", rec.record_id, 0, rec.total_size))
        else:
            parts.append(struct.pack("<HH", rec.record_id, rec.total_size))

        # Payload
        parts.append(rec.payload)

    # Padding before end marker
    current_len = sum(len(p) for p in parts)
    padding_needed = camset.end_marker_offset - current_len
    if padding_needed > 0:
        parts.append(b"\x00" * padding_needed)

    # End marker + checksum
    parts.append(END_MARKER)

    if recalculate_checksum:
        data_without_checksum = b"".join(parts)
        checksum = (256 - sum(data_without_checksum) % 256) & 0xFF
        parts.append(bytes([checksum]))
    else:
        parts.append(bytes([camset.checksum_byte]))

    return b"".join(parts)


def _parse_header(data: bytes) -> CamSetHeader:
    """Parse the 48-byte header + 4-byte section marker."""
    magic_bytes = data[0:9]
    magic = magic_bytes.rstrip(b"\x00").decode("ascii", errors="replace")
    if magic != "Panasonic":
        raise ValueError(f"Invalid magic: expected 'Panasonic', got '{magic}'")

    # Model string at offset 0x12, null-terminated
    model_end = data.index(b"\x00", MODEL_OFFSET)
    model = data[MODEL_OFFSET:model_end].decode("ascii", errors="replace")

    data_length = struct.unpack_from("<I", data, DATA_LENGTH_OFFSET)[0]
    section_marker = data[SECTION_MARKER_OFFSET : SECTION_MARKER_OFFSET + 4]

    return CamSetHeader(
        magic=magic,
        model=model,
        data_length=data_length,
        section_marker=section_marker,
        raw=bytes(data[:RECORDS_START]),
    )


def _parse_records(data: bytes) -> tuple[list[CamSetRecord], int]:
    """Parse all records from the data section.

    Returns:
        Tuple of (records list, end_marker_offset).
    """
    records = []
    offset = RECORDS_START

    while offset + 4 <= len(data):
        rec_id = struct.unpack_from("<H", data, offset)[0]

        # Check for end marker
        if rec_id == 0xFFFF:
            if data[offset : offset + 4] == END_MARKER:
                return records, offset
            # Not a real end marker, treat as record
            pass

        raw_size = struct.unpack_from("<H", data, offset + 2)[0]

        if raw_size == 0:
            # Extended format: 8-byte header
            if offset + 8 > len(data):
                raise ValueError(f"Truncated extended record header at 0x{offset:04x}")
            total_size = struct.unpack_from("<I", data, offset + 4)[0]
            if total_size < 8:
                raise ValueError(f"Invalid extended record size {total_size} at 0x{offset:04x}")
            payload = bytes(data[offset + 8 : offset + total_size])
            records.append(
                CamSetRecord(
                    offset=offset,
                    record_id=rec_id,
                    total_size=total_size,
                    is_extended=True,
                    payload=payload,
                )
            )
        else:
            # Standard format: 4-byte header
            total_size = raw_size
            if total_size < 4:
                raise ValueError(f"Invalid standard record size {total_size} at 0x{offset:04x}")
            payload = bytes(data[offset + 4 : offset + total_size])
            records.append(
                CamSetRecord(
                    offset=offset,
                    record_id=rec_id,
                    total_size=total_size,
                    is_extended=False,
                    payload=payload,
                )
            )

        # Advance past record, then align to 4 bytes
        offset += total_size
        if offset % 4 != 0:
            offset += 4 - (offset % 4)

    raise ValueError(f"No end marker found, reached offset 0x{offset:04x}")
