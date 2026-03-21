"""Tests for ByteMap schema and persistence."""

from pathlib import Path

from xlumix.camset.bytemap import ByteMap, SettingMapping, load_bytemap, save_bytemap


def test_create_bytemap():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    assert bm.camera_model == "DC-G9M2"
    assert bm.firmware_version == "2.7"
    assert bm.created_at != ""
    assert bm.mappings == []


def test_add_mapping():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    m = SettingMapping(
        setting_name="iso",
        record_id=0x00CF,
        byte_offset=0,
        byte_width=2,
        value_encoding="uint16_le",
        value_map={"200": 200, "400": 400},
        wifi_type="iso",
        confidence="confirmed",
    )
    bm.add_mapping(m)
    assert len(bm.mappings) == 1


def test_add_mapping_replaces_existing():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    m1 = SettingMapping(setting_name="iso", record_id=0x00CF, byte_offset=0, byte_width=2)
    m2 = SettingMapping(setting_name="iso", record_id=0x00CF, byte_offset=0, byte_width=4)
    bm.add_mapping(m1)
    bm.add_mapping(m2)
    assert len(bm.mappings) == 1
    assert bm.mappings[0].byte_width == 4


def test_add_mapping_different_slots():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    m1 = SettingMapping(
        setting_name="iso",
        record_id=0x00CF,
        byte_offset=0,
        byte_width=2,
        slot_index=0,
    )
    m2 = SettingMapping(
        setting_name="iso",
        record_id=0x00CF,
        byte_offset=2,
        byte_width=2,
        slot_index=1,
    )
    bm.add_mapping(m1)
    bm.add_mapping(m2)
    assert len(bm.mappings) == 2


def test_find_mapping():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    bm.add_mapping(SettingMapping(setting_name="iso", record_id=1, byte_offset=0, byte_width=2))
    bm.add_mapping(SettingMapping(setting_name="wb", record_id=2, byte_offset=0, byte_width=1))
    assert bm.find_mapping("iso") is not None
    assert bm.find_mapping("iso").record_id == 1
    assert bm.find_mapping("nonexistent") is None


def test_find_by_record_id():
    bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
    bm.add_mapping(SettingMapping(setting_name="a", record_id=5, byte_offset=0, byte_width=1))
    bm.add_mapping(SettingMapping(setting_name="b", record_id=5, byte_offset=1, byte_width=1))
    bm.add_mapping(SettingMapping(setting_name="c", record_id=10, byte_offset=0, byte_width=1))
    assert len(bm.find_by_record_id(5)) == 2
    assert len(bm.find_by_record_id(10)) == 1
    assert len(bm.find_by_record_id(99)) == 0


def test_save_load_roundtrip(tmp_path: Path):
    bm = ByteMap(
        camera_model="DC-G9M2",
        firmware_version="2.7",
        record_format={"standard_header": 4, "extended_header": 8},
    )
    bm.add_mapping(
        SettingMapping(
            setting_name="iso",
            record_id=0x00CF,
            byte_offset=0,
            byte_width=2,
            value_encoding="uint16_le",
            value_map={"200": 200, "400": 400},
            wifi_type="iso",
            confidence="confirmed",
            notes="Direct numeric",
        )
    )
    bm.add_mapping(
        SettingMapping(
            setting_name="fn1",
            record_id=0x0050,
            byte_offset=0,
            byte_width=1,
            value_encoding="enum",
            slot_index=0,
        )
    )

    path = tmp_path / "test_bytemap.json"
    save_bytemap(bm, path)
    loaded = load_bytemap(path)

    assert loaded.camera_model == bm.camera_model
    assert loaded.firmware_version == bm.firmware_version
    assert loaded.record_format == bm.record_format
    assert len(loaded.mappings) == 2
    assert loaded.find_mapping("iso").value_map == {"200": 200, "400": 400}
    assert loaded.find_mapping("fn1", slot_index=0).value_encoding == "enum"
