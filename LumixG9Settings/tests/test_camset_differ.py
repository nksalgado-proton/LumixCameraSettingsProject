"""Tests for CamSet DAT file differ."""

from xlumix.camset.differ import _group_diffs


def test_group_diffs_single():
    diffs = [(10, 0x01, 0x02)]
    groups = _group_diffs(diffs)
    assert len(groups) == 1
    assert groups[0] == [(10, 0x01, 0x02)]


def test_group_diffs_consecutive():
    diffs = [(10, 0x01, 0x02), (11, 0x03, 0x04), (12, 0x05, 0x06)]
    groups = _group_diffs(diffs, gap=4)
    assert len(groups) == 1


def test_group_diffs_separate():
    diffs = [(10, 0x01, 0x02), (100, 0x03, 0x04)]
    groups = _group_diffs(diffs, gap=4)
    assert len(groups) == 2
