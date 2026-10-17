import io
import os
import pycdlib
import pytest


@pytest.fixture(scope="module")
def iso(build_dir):
    iso_path = os.path.join(build_dir, "vmmouse.iso")
    assert os.path.isfile(iso_path), f"ISO not found: {iso_path}"
    cd = pycdlib.PyCdlib()
    cd.open(iso_path)
    yield cd
    cd.close()


def test_iso_volume_label(iso):
    assert iso.pvd.volume_identifier.decode("ascii").strip() == "VMMOUSE"


def test_iso_files_present(iso):
    files = {
        entry.file_identifier().decode("ascii").split(";")[0]
        for entry in iso.list_dir(iso_path="/")
    }
    expected = {
        "VMMOUSE.SYS",
        "INSTDRV.EXE",
        "INSTALL.BAT",
        "UNINSTALL.BAT",
        "README.TXT",
        "LICENSE.TXT",
    }
    assert expected.issubset(files)


def test_iso_readme_version(iso, build_version):
    bio = io.BytesIO()
    iso.get_file_from_iso_fp(bio, iso_path="/README.TXT;1")
    text = bio.getvalue().decode("ascii")

    assert "@VERSION@" not in text
    assert f"Version:    {build_version}" in text


def test_versioned_iso_exists(build_dir, build_version):
    versioned_iso = os.path.join(build_dir, f"vmmouse-{build_version}.iso")
    assert os.path.isfile(versioned_iso), f"Missing versioned ISO: {versioned_iso}"
