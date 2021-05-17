# SPDX-FileCopyrightText: 2021 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only
# SPDX-License-Identifier: MIT

import logging
import os
import pathlib
import re
import subprocess
import sys
from typing import Sequence

import pytest
import PIL.Image

_LOGGER = logging.getLogger(__name__)


def compare_images(*, golden: pathlib.Path, result: pathlib.Path) -> float:
    """Compare images loaded from the provided paths, returns ratio of different pixels."""

    golden_image = PIL.Image.open(golden)
    result_image = PIL.Image.open(result)

    if golden_image.size != result_image.size:
        _LOGGER.error(
            f"image sizes don't match: {golden} {golden_image.size} != {result} {result_image.size}"
        )
        return float("inf")

    total_pixels = golden_image.width * golden_image.height
    different_pixels = sum(
        1 if golden_image.getpixel((x, y)) != result_image.getpixel((x, y)) else 0
        for x in range(golden_image.width)
        for y in range(golden_image.height)
    )

    return different_pixels / total_pixels


def run_unpaper(
    *cmdline: Sequence[str], check: bool = True
) -> subprocess.CompletedProcess:
    unpaper_path = os.getenv("TEST_UNPAPER_BINARY", "unpaper")

    return subprocess.run(
        [unpaper_path, "-v"] + list(cmdline),
        stdout=sys.stdout,
        stderr=sys.stderr,
        check=check,
    )


@pytest.fixture(name="imgsrc_path")
def get_imgsrc_directory() -> pathlib.Path:
    return pathlib.Path(os.getenv("TEST_IMGSRC_DIR", "tests/source_images/"))


@pytest.fixture(name="goldendir_path")
def get_golden_directory() -> pathlib.Path:
    return pathlib.Path(os.getenv("TEST_GOLDEN_DIR", "tests/golden_images/"))


def test_a1(imgsrc_path, goldendir_path, tmp_path):
    """[A1] Single-Page Template Layout, Black+White, Full Processing."""
    source_path = imgsrc_path / "imgsrc001.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenA1.pbm"

    run_unpaper(str(source_path), str(result_path))

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_b1(imgsrc_path, goldendir_path, tmp_path):
    """[B1] Combined Color/Gray, No Processing."""

    source1_path = imgsrc_path / "imgsrc003.png"
    source2_path = imgsrc_path / "imgsrc004.png"
    result_path = tmp_path / "result.ppm"
    golden_path = goldendir_path / "goldenB1.ppm"

    run_unpaper(
        "-n",
        "--input-pages",
        "2",
        str(source1_path),
        str(source2_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_b2(imgsrc_path, goldendir_path, tmp_path):
    """[B2] Combined Color/Black+White, No Processing."""

    source1_path = imgsrc_path / "imgsrc003.png"
    source2_path = imgsrc_path / "imgsrc005.png"
    result_path = tmp_path / "result.ppm"
    golden_path = goldendir_path / "goldenB2.ppm"

    run_unpaper(
        "-n",
        "--input-pages",
        "2",
        str(source1_path),
        str(source2_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_b3(imgsrc_path, goldendir_path, tmp_path):
    """[B3] Combined Gray/Black+White, No Processing."""

    source1_path = imgsrc_path / "imgsrc004.png"
    source2_path = imgsrc_path / "imgsrc005.png"
    result_path = tmp_path / "result.ppm"
    golden_path = goldendir_path / "goldenB3.ppm"

    run_unpaper(
        "-n",
        "--input-pages",
        "2",
        str(source1_path),
        str(source2_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_sheet_background_black(imgsrc_path, goldendir_path, tmp_path):
    """[C1] Black sheet background color."""

    source_path = imgsrc_path / "imgsrc002.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenC1.pbm"

    run_unpaper(
        "-n",
        "--sheet-size",
        "a4",
        "--sheet-background",
        "black",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_pre_shift_both(imgsrc_path, goldendir_path, tmp_path):
    """[C2] Explicit shifting."""

    source_path = imgsrc_path / "imgsrc002.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenC2.pbm"

    run_unpaper(
        "-n",
        "--sheet-size",
        "a4",
        "--pre-shift",
        "-5cm,9cm",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_negative_shift(imgsrc_path, goldendir_path, tmp_path):
    """[C2] Explicit -1 size shifting."""

    source_path = imgsrc_path / "imgsrc002.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenC3.pbm"

    run_unpaper(
        "-n",
        "--sheet-size",
        "a4",
        "--pre-shift",
        "-1cm",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_sheet_crop(imgsrc_path, goldendir_path, tmp_path):
    """[D1] Crop to sheet size."""
    source_path = imgsrc_path / "imgsrc003.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenD1.ppm"

    run_unpaper(
        "-n",
        "--sheet-size",
        "20cm,10cm",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_sheet_fit(imgsrc_path, goldendir_path, tmp_path):
    """[D2] Fit to sheet size."""
    source_path = imgsrc_path / "imgsrc003.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenD2.ppm"

    run_unpaper(
        "-n",
        "--size",
        "20cm,10cm",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_sheet_stretch(imgsrc_path, goldendir_path, tmp_path):
    """[D3] Stretch to sheet size."""
    source_path = imgsrc_path / "imgsrc003.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenD3.ppm"

    run_unpaper(
        "-n",
        "--stretch",
        "20cm,10cm",
        str(source_path),
        str(result_path),
    )

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_e1(imgsrc_path, goldendir_path, tmp_path):
    """[E1] Splitting 2-page layout into separate output pages (with input and output wildcard)."""

    source_path = imgsrc_path / "imgsrcE%03d.png"
    result_path = tmp_path / "results-%02d.pbm"

    run_unpaper(
        "--layout", "double", "--output-pages", "2", str(source_path), str(result_path)
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 6

    for result in all_results:
        name_match = re.match(r"^results-([0-9]{2})\.pbm$", str(result.name))
        assert name_match

        golden_path = goldendir_path / f"goldenE1-{name_match.group(1)}.pbm"

        assert compare_images(golden=golden_path, result=result) < 0.05


def test_e2(imgsrc_path, goldendir_path, tmp_path):
    """[E2] Splitting 2-page layout into separate output pages (with output wildcard only)."""

    source_path = imgsrc_path / "imgsrcE001.png"
    result_path = tmp_path / "results-%02d.pbm"

    run_unpaper(
        "--layout", "double", "--output-pages", "2", str(source_path), str(result_path)
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 2

    for result in all_results:
        name_match = re.match(r"^results-([0-9]{2})\.pbm$", str(result.name))
        assert name_match

        golden_path = goldendir_path / f"goldenE1-{name_match.group(1)}.pbm"

        assert compare_images(golden=golden_path, result=result) < 0.05


def test_e3(imgsrc_path, goldendir_path, tmp_path):
    """[E3] Splitting 2-page layout into separate output pages (with explicit input and output)."""

    source_path = imgsrc_path / "imgsrcE001.png"
    result_path_1 = tmp_path / "results-1.pbm"
    result_path_2 = tmp_path / "results-2.pbm"

    run_unpaper(
        "--layout",
        "double",
        "--output-pages",
        "2",
        str(source_path),
        str(result_path_1),
        str(result_path_2),
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 2
    assert compare_images(golden=(goldendir_path / "goldenE1-01.pbm"), result=result_path_1) < 0.05
    assert compare_images(golden=(goldendir_path / "goldenE1-02.pbm"), result=result_path_2) < 0.05


def test_f1(imgsrc_path, goldendir_path, tmp_path):
    """[F1] Merging 2-page layout into single output page (with input and output wildcard)."""

    source_path = imgsrc_path / "imgsrcE%03d.png"
    output_path = tmp_path / "results-%d.pbm"
    result_path = tmp_path / "results-1.pbm"
    golden_path = goldendir_path / "goldenF.pbm"

    run_unpaper(
        "--end-sheet",
        "1",
        "--layout",
        "double",
        "--input-pages",
        "2",
        str(source_path),
        str(output_path),
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 1
    assert all_results[0] == result_path

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_f2(imgsrc_path, goldendir_path, tmp_path):
    """[F2] Merging 2-page layout into single output page (with output wildcard only)."""

    source_path_1 = imgsrc_path / "imgsrcE001.png"
    source_path_2 = imgsrc_path / "imgsrcE002.png"
    output_path = tmp_path / "results-%d.pbm"
    result_path = tmp_path / "results-1.pbm"
    golden_path = goldendir_path / "goldenF.pbm"

    run_unpaper(
        "--layout",
        "double",
        "--input-pages",
        "2",
        str(source_path_1),
        str(source_path_2),
        str(output_path),
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 1
    assert all_results[0] == result_path

    assert compare_images(golden=golden_path, result=result_path) < 0.05

def test_f3(imgsrc_path, goldendir_path, tmp_path):
    """[F3] Merging 2-page layout into single output page (with explicit input and output)."""

    source_path_1 = imgsrc_path / "imgsrcE001.png"
    source_path_2 = imgsrc_path / "imgsrcE002.png"
    result_path = tmp_path / "result.pbm"
    golden_path = goldendir_path / "goldenF.pbm"

    run_unpaper(
        "--layout",
        "double",
        "--input-pages",
        "2",
        str(source_path_1),
        str(source_path_2),
        str(result_path),
    )

    all_results = sorted(tmp_path.iterdir())
    assert len(all_results) == 1
    assert all_results[0] == result_path

    assert compare_images(golden=golden_path, result=result_path) < 0.05


def test_overwrite_no_file(imgsrc_path, tmp_path):
    source_path = imgsrc_path / "imgsrc001.png"
    result_path = tmp_path / "result.pbm"

    run_unpaper(
        "--overwrite", "--no-processing", "1", str(source_path), str(result_path)
    )

    assert compare_images(golden=source_path, result=result_path) == 0


def test_overwrite_existing_file(imgsrc_path, tmp_path):
    source_path = imgsrc_path / "imgsrc001.png"
    result_path = tmp_path / "result.pbm"

    # Create an empty file first, which should be overwritten
    result_path.touch(exist_ok=False)

    run_unpaper(
        "--overwrite", "--no-processing", "1", str(source_path), str(result_path)
    )

    assert compare_images(golden=source_path, result=result_path) == 0


def test_no_overwrite_existing_file(imgsrc_path, tmp_path):
    source_path = imgsrc_path / "imgsrc001.png"
    result_path = tmp_path / "result.pbm"

    # Create an empty file first, which should be overwritten
    result_path.touch(exist_ok=False)

    unpaper_result = run_unpaper(
        "--no-processing", "1", str(source_path), str(result_path), check=False
    )
    assert unpaper_result.returncode != 0
    assert result_path.stat().st_size == 0


def test_invalid_multi_index(imgsrc_path, tmp_path):
    source_path = imgsrc_path / "imgsrc001.png"
    result_path = tmp_path / "result.pbm"
    unpaper_result = run_unpaper(
        "--no-processing", "1-", str(source_path), str(result_path), check=False
    )
    assert unpaper_result.returncode != 0
