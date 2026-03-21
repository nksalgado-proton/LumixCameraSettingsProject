"""Tests for ByteMap validation against CamSet files."""

import struct

from xlumix.camset.bytemap import ByteMap, SettingMapping
from xlumix.camset.parser import CamSetFile, CamSetHeader, CamSetRecord
from xlumix.camset.validator import (
    ValidationReport,
    validate_bytemap,
    validate_bytemap_with_wifi,
)


def _make_camset(records: list[CamSetRecord]) -> CamSetFile:
    """Helper to build a minimal CamSetFile for testing."""
    return CamSetFile(
        header=CamSetHeader(
            magic="Panasonic",
            model="DC-G9M2",
            data_length=100,
            section_marker=b"\x00\x03\x00\x00",
            raw=b"\x00" * 52,
        ),
        records=records,
        end_marker_offset=200,
        checksum_byte=0xAB,
    )


def _make_record(record_id: int, payload: bytes) -> CamSetRecord:
    return CamSetRecord(
        offset=0x34,
        record_id=record_id,
        total_size=4 + len(payload),
        is_extended=False,
        payload=payload,
    )


class TestValidateBytemap:
    def test_record_found(self):
        payload = struct.pack("<H", 400)  # uint16 LE = 400
        camset = _make_camset([_make_record(0x00CF, payload + b"\x00" * 14)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=0,
                byte_width=2,
                value_encoding="uint16_le",
            )
        )

        report = validate_bytemap(bm, camset)
        assert report.total == 1
        assert report.records_found == 1
        assert report.results[0].dat_raw_value == 400
        assert report.results[0].dat_decoded == "400"

    def test_record_not_found(self):
        camset = _make_camset([_make_record(0x0001, b"\x00" * 16)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=0,
                byte_width=2,
            )
        )

        report = validate_bytemap(bm, camset)
        assert report.records_found == 0
        assert "not found" in report.results[0].error

    def test_value_map_decode(self):
        payload = struct.pack("<B", 0x03) + b"\x00" * 15
        camset = _make_camset([_make_record(0x0050, payload)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="fn1",
                record_id=0x0050,
                byte_offset=0,
                byte_width=1,
                value_encoding="enum",
                value_map={"AF-ON": 1, "AE Lock": 2, "Preview": 3},
            )
        )

        report = validate_bytemap(bm, camset)
        assert report.results[0].dat_raw_value == 3
        assert report.results[0].dat_decoded == "Preview"

    def test_unmapped_value(self):
        payload = struct.pack("<B", 0xFF) + b"\x00" * 15
        camset = _make_camset([_make_record(0x0050, payload)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="fn1",
                record_id=0x0050,
                byte_offset=0,
                byte_width=1,
                value_encoding="enum",
                value_map={"AF-ON": 1},
            )
        )

        report = validate_bytemap(bm, camset)
        assert "unmapped" in report.results[0].dat_decoded

    def test_offset_exceeds_payload(self):
        camset = _make_camset([_make_record(0x00CF, b"\x00\x01")])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=10,
                byte_width=2,
            )
        )

        report = validate_bytemap(bm, camset)
        assert report.results[0].error != ""


class TestValidateWithWifi:
    def test_matching_values(self):
        payload = struct.pack("<H", 400) + b"\x00" * 14
        camset = _make_camset([_make_record(0x00CF, payload)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=0,
                byte_width=2,
                value_encoding="uint16_le",
                value_map={"400": 400},
                wifi_type="iso",
            )
        )

        report = validate_bytemap_with_wifi(bm, camset, {"iso": "400"})
        assert report.matched == 1
        assert report.mismatched == 0
        assert report.results[0].match is True

    def test_mismatching_values(self):
        payload = struct.pack("<H", 400) + b"\x00" * 14
        camset = _make_camset([_make_record(0x00CF, payload)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=0,
                byte_width=2,
                value_encoding="uint16_le",
                value_map={"400": 400, "800": 800},
                wifi_type="iso",
            )
        )

        report = validate_bytemap_with_wifi(bm, camset, {"iso": "800"})
        assert report.mismatched == 1
        assert report.results[0].match is False

    def test_no_wifi_value_available(self):
        payload = struct.pack("<H", 400) + b"\x00" * 14
        camset = _make_camset([_make_record(0x00CF, payload)])
        bm = ByteMap(camera_model="DC-G9M2", firmware_version="2.7")
        bm.add_mapping(
            SettingMapping(
                setting_name="iso",
                record_id=0x00CF,
                byte_offset=0,
                byte_width=2,
            )
        )

        report = validate_bytemap_with_wifi(bm, camset, {})
        assert report.results[0].match is None


class TestValidationReport:
    def test_empty_report(self):
        report = ValidationReport()
        assert report.total == 0
        assert report.records_found == 0
        assert report.matched == 0
        assert report.mismatched == 0
