"""Tests for WiFi API setting discovery (XML parsing)."""

from xlumix.wifi.discovery import (
    CameraCapabilities,
    SettingInfo,
    load_capabilities,
    parse_allmenu_xml,
    parse_capability_xml,
    save_capabilities,
)

# -- Sample XML for testing (based on known Lumix response patterns) --

SAMPLE_ALLMENU = """<?xml version="1.0" encoding="UTF-8"?>
<camrply><result>ok</result>
<menuset>
  <titleinfo title="Photo">
    <menuinfo id="iso" enable="yes"/>
    <menuinfo id="shtrspeed" enable="yes"/>
    <menuinfo id="whitebalance" enable="yes"/>
  </titleinfo>
  <titleinfo title="Custom">
    <menuinfo id="focusmode" enable="yes"/>
    <menuinfo id="peaking" enable="yes"/>
  </titleinfo>
</menuset>
</camrply>"""

SAMPLE_CAPABILITY = """<?xml version="1.0" encoding="UTF-8"?>
<camrply><result>ok</result>
<settinginfo>
  <iso>100,200,400,800,1600,3200,6400</iso>
  <shtrspeed>1/4000,1/2000,1/1000,1/500,1/250,1/125,1/60</shtrspeed>
  <whitebalance>awb,daylight,cloudy,shade,flash</whitebalance>
</settinginfo>
</camrply>"""


class TestParseAllmenu:
    def test_extracts_settings(self):
        settings = parse_allmenu_xml(SAMPLE_ALLMENU)
        names = [s.name for s in settings]
        assert "iso" in names
        assert "shtrspeed" in names
        assert "whitebalance" in names
        assert "focusmode" in names
        assert "peaking" in names

    def test_no_duplicates(self):
        settings = parse_allmenu_xml(SAMPLE_ALLMENU)
        names = [s.name for s in settings]
        assert len(names) == len(set(names))

    def test_empty_xml(self):
        assert parse_allmenu_xml("") == []

    def test_malformed_xml(self):
        assert parse_allmenu_xml("<not>valid") == []


class TestParseCapability:
    def test_extracts_values(self):
        result = parse_capability_xml(SAMPLE_CAPABILITY)
        assert "iso" in result
        assert result["iso"] == ["100", "200", "400", "800", "1600", "3200", "6400"]

    def test_shutter_speeds(self):
        result = parse_capability_xml(SAMPLE_CAPABILITY)
        assert "shtrspeed" in result
        assert "1/250" in result["shtrspeed"]

    def test_empty_xml(self):
        assert parse_capability_xml("") == {}


class TestCapabilitiesPersistence:
    def test_save_load_roundtrip(self, tmp_path):
        caps = CameraCapabilities(model="DC-G9M2", firmware="2.7")
        caps.settings["iso"] = SettingInfo(
            name="iso",
            current_value="400",
            valid_values=["100", "200", "400", "800"],
            is_settable=True,
        )
        caps.settings["wb"] = SettingInfo(
            name="wb",
            current_value="awb",
            valid_values=["awb", "daylight"],
        )

        path = tmp_path / "caps.json"
        save_capabilities(caps, path)
        loaded = load_capabilities(path)

        assert loaded.model == "DC-G9M2"
        assert len(loaded.settings) == 2
        assert loaded.settings["iso"].current_value == "400"
        assert loaded.settings["iso"].valid_values == ["100", "200", "400", "800"]
        assert loaded.settings["iso"].is_settable is True
