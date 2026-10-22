#!/usr/bin/env python3
"""Config and upload flag for a HIL flash. Run from this repo."""

import io
import json
import os
import re
import sys
import tempfile
from contextlib import redirect_stdout
from pathlib import Path

import upload

ROOT = Path(__file__).resolve().parent


def test_config_names_container_compile_and_boards():
    cfg = json.loads((ROOT / "hil_config.json").read_text())
    source = (ROOT / "compile.py").read_text()
    container = re.search(r'^CONTAINER = "([^"]+)"', source, re.M).group(1)
    assert cfg["containerName"] == container == "Rivanna3S_compile"
    assert "-DBOARD=STM32G474RET6" in cfg["compileCmd"]
    assert "STM32U5" not in cfg["compileCmd"]
    assert "STM32H743" not in cfg["compileCmd"]
    assert "cmake --build" in cfg["compileCmd"]
    assert set(cfg["boards"]) == {"TopDist", "BottomDist", "MotorBoard"}
    assert cfg["boards"]["TopDist"]["uploadCmd"] == "python3 upload.py TopDist --hil"
    assert cfg["boards"]["BottomDist"]["uploadCmd"] == "python3 upload.py BottomDist --hil"
    assert cfg["boards"]["MotorBoard"]["uploadCmd"] == "python3 upload.py MotorBoard --hil"


def _run_upload(argv, cwd):
    calls = []

    def fake_run(cmd, **kwargs):
        calls.append(cmd)

        class Proc:
            returncode = 0

        return Proc()

    real = upload.subprocess.run
    upload.subprocess.run = fake_run
    old_argv = sys.argv
    old_cwd = os.getcwd()
    sys.argv = argv
    os.chdir(cwd)
    out = io.StringIO()
    try:
        with redirect_stdout(out):
            try:
                upload.main()
            except SystemExit as e:
                code = e.code
            else:
                code = 0
    finally:
        upload.subprocess.run = real
        sys.argv = old_argv
        os.chdir(old_cwd)
    return code, calls, out.getvalue()


def _elf(tmp, name):
    path = Path(tmp) / "build" / "bin" / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"img")
    return path


def test_hil_flag_flashes_compiled_image():
    with tempfile.TemporaryDirectory() as tmp:
        _elf(tmp, "TopDistBoard.elf")
        code, calls, _out = _run_upload(["upload.py", "TopDist", "--hil"], tmp)
    assert code == 0
    assert "build/bin/TopDistBoard.elf" in calls[-1]
    assert "STM32_Programmer_CLI" in calls[-1]


def test_missing_hil_image_does_not_program():
    with tempfile.TemporaryDirectory() as tmp:
        _elf(tmp, "RelayBoard.elf")
        code, calls, out = _run_upload(["upload.py", "TopDist", "--hil"], tmp)
    assert code == 4
    assert calls == []
    assert "You need to compile." in out


def test_hil_flag_does_not_flash_relay_or_telemetry():
    with tempfile.TemporaryDirectory() as tmp:
        _elf(tmp, "RelayBoard.elf")
        _elf(tmp, "TelemetryBoard.elf")
        relay_code, relay_calls, _relay_out = _run_upload(["upload.py", "Relay", "--hil"], tmp)
        tel_code, tel_calls, _tel_out = _run_upload(["upload.py", "Telemetry", "--hil"], tmp)
    assert relay_code == 1 and relay_calls == []
    assert tel_code == 1 and tel_calls == []


def test_bench_upload_still_flashes_relay():
    with tempfile.TemporaryDirectory() as tmp:
        _elf(tmp, "RelayBoard.elf")
        code, calls, _out = _run_upload(["upload.py", "Relay"], tmp)
    assert code == 0
    assert "build/bin/RelayBoard.elf" in calls[-1]


if __name__ == "__main__":
    test_config_names_container_compile_and_boards()
    test_hil_flag_flashes_compiled_image()
    test_missing_hil_image_does_not_program()
    test_hil_flag_does_not_flash_relay_or_telemetry()
    test_bench_upload_still_flashes_relay()
    print("ok")
