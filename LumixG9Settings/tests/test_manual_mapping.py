"""Tests for the offline manual mapping workflow."""

from pathlib import Path

import pytest

from xlumix.camset.bytemap import ByteMap, SettingMapping, load_bytemap, save_bytemap
from xlumix.camset.parser import parse_camset_file, serialize_camset
from xlumix.reveng.manual import map_single

MKII_DAT = Path("data/g9ii/MKIISET_FW2.7.DAT")


@pytest.fixture
def bytemap_path(tmp_path):
    return tmp_path / "test_bytemap.json"


@pytest.fixture
def modified_dat(tmp_path):
    """Create a DAT file with one record modified."""
    camset = parse_camset_file(str(MKII_DAT))

    # Modify record 0x0001 payload byte 0 (change from whatever to 0x42)
    target_rec = None
    for rec in camset.records:
        if rec.record_id == 0x0001:
            target_rec = rec
            break

    assert target_rec is not None
    original_byte = target_rec.payload[0]
    new_byte = 0x42 if original_byte != 0x42 else 0x43

    # Create modified payload
    new_payload = bytes([new_byte]) + target_rec.payload[1:]
    target_rec.payload = new_payload

    # Serialize and fix checksum
    data = serialize_camset(camset)
    out_path = tmp_path / "modified.DAT"
    out_path.write_bytes(data)
    return str(out_path), 0x0001, 0, original_byte, new_byte


def test_map_single_finds_change(modified_dat, bytemap_path):
    """map_single should detect the one-byte change and record it."""
    changed_path, expected_rid, expected_off, old_byte, new_byte = modified_dat

    result = map_single(
        baseline_path=str(MKII_DAT),
        changed_path=changed_path,
        setting_name="test_setting",
        value_label="test_value",
        bytemap_path=str(bytemap_path),
    )

    assert result is not None
    assert result.record_id == expected_rid
    assert result.byte_offset == expected_off
    assert result.new_bytes[0] == new_byte
    assert result.old_bytes[0] == old_byte

    # Check bytemap was saved
    assert bytemap_path.exists()
    bm = load_bytemap(bytemap_path)
    assert len(bm.mappings) == 1
    m = bm.mappings[0]
    assert m.setting_name == "test_setting"
    assert m.record_id == expected_rid
    assert m.value_map["test_value"] == new_byte


def test_map_single_no_diff(bytemap_path):
    """map_single with identical files should return None."""
    result = map_single(
        baseline_path=str(MKII_DAT),
        changed_path=str(MKII_DAT),
        setting_name="test",
        value_label="same",
        bytemap_path=str(bytemap_path),
    )

    assert result is None
    assert not bytemap_path.exists()


def test_map_single_accumulates_values(modified_dat, bytemap_path, tmp_path):
    """Running map_single twice with different values should merge value_map."""
    changed_path, _, _, _, new_byte = modified_dat

    # First mapping
    map_single(
        baseline_path=str(MKII_DAT),
        changed_path=changed_path,
        setting_name="test_setting",
        value_label="value_A",
        bytemap_path=str(bytemap_path),
    )

    # Create a second modified DAT with a different byte value
    camset = parse_camset_file(str(MKII_DAT))
    for rec in camset.records:
        if rec.record_id == 0x0001:
            second_byte = 0x99 if new_byte != 0x99 else 0x98
            rec.payload = bytes([second_byte]) + rec.payload[1:]
            break

    second_path = tmp_path / "modified2.DAT"
    second_path.write_bytes(serialize_camset(camset))

    # Second mapping with different value
    map_single(
        baseline_path=str(MKII_DAT),
        changed_path=str(second_path),
        setting_name="test_setting",
        value_label="value_B",
        bytemap_path=str(bytemap_path),
    )

    bm = load_bytemap(bytemap_path)
    assert len(bm.mappings) == 1  # Same setting, should merge
    m = bm.mappings[0]
    assert "value_A" in m.value_map
    assert "value_B" in m.value_map
    assert m.value_map["value_A"] == new_byte
    assert m.value_map["value_B"] == second_byte
