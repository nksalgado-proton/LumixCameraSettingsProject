"""Tests for CamSet DAT file parser."""

from pathlib import Path

import pytest

from xlumix.camset.parser import (
    CamSetFile,
    parse_camset,
    parse_camset_file,
    serialize_camset,
)

DATA_DIR = Path(__file__).parent.parent / "data"
G9II_DAT = DATA_DIR / "g9ii" / "MKIISET_FW2.7.DAT"
G9_DAT = DATA_DIR / "g9" / "MKISET_FW2.5.DAT"


# -- G9 Mark II tests --


@pytest.fixture
def g9ii() -> CamSetFile:
    return parse_camset_file(G9II_DAT)


@pytest.fixture
def g9() -> CamSetFile:
    return parse_camset_file(G9_DAT)


class TestG9IIHeader:
    def test_magic(self, g9ii: CamSetFile):
        assert g9ii.header.magic == "Panasonic"

    def test_model(self, g9ii: CamSetFile):
        assert g9ii.header.model == "DC-G9M2"

    def test_data_length(self, g9ii: CamSetFile):
        assert g9ii.header.data_length == 8959240

    def test_section_marker(self, g9ii: CamSetFile):
        assert g9ii.header.section_marker == b"\x00\x03\x00\x00"


class TestG9IIRecords:
    def test_record_count(self, g9ii: CamSetFile):
        assert len(g9ii.records) == 968

    def test_first_record(self, g9ii: CamSetFile):
        rec = g9ii.records[0]
        assert rec.record_id == 0x0001
        assert rec.total_size == 34
        assert rec.is_extended is False
        assert rec.offset == 0x34

    def test_standard_record_size_20(self, g9ii: CamSetFile):
        """Most G9 II records are 20 bytes."""
        rec = g9ii.find_record(0x0004)
        assert rec is not None
        assert rec.total_size == 20
        assert rec.is_extended is False
        assert len(rec.payload) == 16

    def test_extended_records_exist(self, g9ii: CamSetFile):
        """G9 II has extended records (size field = 0, 8-byte header)."""
        extended = [r for r in g9ii.records if r.is_extended]
        assert len(extended) == 121

    def test_extended_record_0x0043(self, g9ii: CamSetFile):
        rec = g9ii.find_record(0x0043)
        assert rec is not None
        assert rec.is_extended is True
        assert rec.total_size == 628
        assert len(rec.payload) == 620  # 628 - 8

    def test_large_extended_record(self, g9ii: CamSetFile):
        """Record 0x0520 is ~8.5MB (image/LUT data)."""
        rec = g9ii.find_record(0x0520)
        assert rec is not None
        assert rec.is_extended is True
        assert rec.total_size == 8840924

    def test_record_ids_ascending(self, g9ii: CamSetFile):
        """Record IDs should be roughly ascending."""
        ids = [r.record_id for r in g9ii.records]
        # Not strictly sorted but should trend upward
        assert ids[0] <= ids[-1]


class TestG9IIEndMarker:
    def test_end_marker_offset(self, g9ii: CamSetFile):
        assert g9ii.end_marker_offset == 0x88B534

    def test_checksum_byte(self, g9ii: CamSetFile):
        assert g9ii.checksum_byte == 0x35

    def test_data_length_matches_end_marker(self, g9ii: CamSetFile):
        expected = g9ii.end_marker_offset - 0x30 + 4
        assert g9ii.header.data_length == expected


# -- G9 Mark I tests --


class TestG9Header:
    def test_magic(self, g9: CamSetFile):
        assert g9.header.magic == "Panasonic"

    def test_model(self, g9: CamSetFile):
        assert g9.header.model == "DC-G9"

    def test_data_length(self, g9: CamSetFile):
        assert g9.header.data_length == 10908


class TestG9Records:
    def test_record_count(self, g9: CamSetFile):
        assert len(g9.records) == 529

    def test_first_record(self, g9: CamSetFile):
        rec = g9.records[0]
        assert rec.record_id == 0x0002
        assert rec.total_size == 12
        assert rec.is_extended is False

    def test_no_extended_records(self, g9: CamSetFile):
        """G9 MkI has no extended records."""
        extended = [r for r in g9.records if r.is_extended]
        assert len(extended) == 0

    def test_standard_record_size_12(self, g9: CamSetFile):
        """Most G9 MkI records are 12 bytes."""
        size_12 = [r for r in g9.records if r.total_size == 12]
        assert len(size_12) == 414


class TestG9EndMarker:
    def test_end_marker_offset(self, g9: CamSetFile):
        assert g9.end_marker_offset == 0x2AC8

    def test_checksum_byte(self, g9: CamSetFile):
        assert g9.checksum_byte == 0x1A

    def test_data_length_matches_end_marker(self, g9: CamSetFile):
        expected = g9.end_marker_offset - 0x30 + 4
        assert g9.header.data_length == expected


# -- Roundtrip serialization --


class TestRoundtrip:
    def test_g9ii_roundtrip(self, g9ii: CamSetFile):
        """serialize(parse(data)) == data for G9 MkII."""
        original = G9II_DAT.read_bytes()
        rebuilt = serialize_camset(g9ii)
        assert rebuilt == original

    def test_g9_roundtrip(self, g9: CamSetFile):
        """serialize(parse(data)) == data for G9 MkI."""
        original = G9_DAT.read_bytes()
        rebuilt = serialize_camset(g9)
        assert rebuilt == original


# -- Lookup helpers --


class TestHelpers:
    def test_find_record(self, g9ii: CamSetFile):
        rec = g9ii.find_record(0x0005)
        assert rec is not None
        assert rec.record_id == 0x0005

    def test_find_record_missing(self, g9ii: CamSetFile):
        assert g9ii.find_record(0x9999) is None

    def test_find_records_multiple(self, g9ii: CamSetFile):
        # Record ID 0x0001 appears once in G9 II
        recs = g9ii.find_records(0x0001)
        assert len(recs) >= 1
        assert all(r.record_id == 0x0001 for r in recs)


# -- Error handling --


class TestErrors:
    def test_too_small(self):
        with pytest.raises(ValueError, match="File too small"):
            parse_camset(b"\x00" * 10)

    def test_bad_magic(self):
        data = b"NotPanaso" + b"\x00" * 100
        with pytest.raises(ValueError, match="Invalid magic"):
            parse_camset(data)
