import os
import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--build-version",
        default=os.environ.get("VERSION", ""),
        help="Expected version string (e.g. 1.0.6)",
    )
    parser.addoption(
        "--build-dir",
        default=os.environ.get("BUILD_DIR", "build"),
        help="Build directory path containing binaries and ISO",
    )


@pytest.fixture(scope="session")
def build_version(request):
    val = request.config.getoption("--build-version") or os.environ.get("VERSION", "")
    if not val:
        pytest.fail("VERSION not specified (pass --build-version or set VERSION env var)")
    return val


@pytest.fixture(scope="session")
def expected_image_version(build_version):
    parts = [int(p) for p in build_version.split(".") if p.isdigit()]
    major = parts[0] if len(parts) > 0 else 1
    minor = parts[1] if len(parts) > 1 else 0
    return (major, minor)


@pytest.fixture(scope="session")
def build_dir(request):
    return request.config.getoption("--build-dir")
