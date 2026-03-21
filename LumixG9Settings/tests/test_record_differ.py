"""Tests for record-level CamSet differ."""

from xlumix.camset.parser import CamSetFile, CamSetHeader, CamSetRecord
from xlumix.camset.record_differ import diff_records


def _make_file(records: list[CamSetRecord]) -> CamSetFile:
    """Create a minimal CamSetFile for testing."""
    return CamSetFile(
        header=CamSetHeader(
            magic="Panasonic",
            model="DC-TEST",
            data_length=0,
            section_marker=b"\x00\x03\x00\x00",
            raw=b"\x00" * 52,
        ),
        records=records,
    )


def _rec(record_id: int, payload: bytes) -> CamSetRecord:
    return CamSetRecord(
        offset=0,
        record_id=record_id,
        total_size=4 + len(payload),
        is_extended=False,
        payload=payload,
    )


def test_identical_files():
    a = _make_file([_rec(1, b"\x01\x02\x03")])
    b = _make_file([_rec(1, b"\x01\x02\x03")])
    assert diff_records(a, b) == []


def test_single_byte_change():
    a = _make_file([_rec(1, b"\x01\x02\x03")])
    b = _make_file([_rec(1, b"\x01\xff\x03")])
    diffs = diff_records(a, b)
    assert len(diffs) == 1
    assert diffs[0].record_id == 1
    assert diffs[0].payload_diffs == [(1, 0x02, 0xFF)]


def test_multiple_records_one_changed():
    a = _make_file([_rec(1, b"\xaa"), _rec(2, b"\xbb"), _rec(3, b"\xcc")])
    b = _make_file([_rec(1, b"\xaa"), _rec(2, b"\xff"), _rec(3, b"\xcc")])
    diffs = diff_records(a, b)
    assert len(diffs) == 1
    assert diffs[0].record_id == 2


def test_record_only_in_a():
    a = _make_file([_rec(1, b"\xaa"), _rec(2, b"\xbb")])
    b = _make_file([_rec(1, b"\xaa")])
    diffs = diff_records(a, b)
    assert len(diffs) == 1
    assert diffs[0].record_id == 2
    assert diffs[0].only_in == "a"


def test_record_only_in_b():
    a = _make_file([_rec(1, b"\xaa")])
    b = _make_file([_rec(1, b"\xaa"), _rec(5, b"\xdd")])
    diffs = diff_records(a, b)
    assert len(diffs) == 1
    assert diffs[0].record_id == 5
    assert diffs[0].only_in == "b"


def test_size_difference():
    a = _make_file([_rec(1, b"\xaa\xbb")])
    b = _make_file([_rec(1, b"\xaa\xbb\xcc")])
    diffs = diff_records(a, b)
    assert len(diffs) == 1
    # Extra byte in B treated as diff against 0x00
    assert (2, 0x00, 0xCC) in diffs[0].payload_diffs


def test_empty_files():
    a = _make_file([])
    b = _make_file([])
    assert diff_records(a, b) == []


def test_diffs_sorted_by_id():
    a = _make_file([_rec(10, b"\xaa"), _rec(5, b"\xbb")])
    b = _make_file([_rec(10, b"\xff"), _rec(5, b"\xff")])
    diffs = diff_records(a, b)
    assert diffs[0].record_id == 5
    assert diffs[1].record_id == 10
