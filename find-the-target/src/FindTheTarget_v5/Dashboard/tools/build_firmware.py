#!/usr/bin/env python3
"""
build_firmware.py -- build the three v5 programs for the dashboard's installer
==============================================================================

WHO RUNS THIS
  Whoever changes StationRadio_v5, VehicleSensors_v5 or VehicleCamera_v5.
  Teammates who only set up boards never run it: they use the dashboard's
  "Install a program on a board" buttons, which flash the files this
  script leaves in Dashboard/firmware/.

WHAT IT DOES
  1. Builds each sketch with arduino-cli and esp32 3.3.12, with the same
     board settings as the README's upload table (the camera gets the
     "Huge APP" partition layout).
  2. Copies the four pieces the Arduino IDE writes to a board into
     Dashboard/firmware/<program>/:
        bootloader.bin  partitions.bin  boot_app0.bin  app.bin
     They go to the same addresses the IDE uses. We do NOT use the
     ".merged.bin": esp32 3.3.12 pads that to the whole 4 MB, which would
     also wipe the board's saved station number and the camera's WiFi.
  3. Writes Dashboard/firmware/manifest.json: version, board settings,
     each file's flash address, size and SHA-256, and a fingerprint of the
     sketch's source files (test_dashboard.py checks that fingerprint, so a
     sketch edited without rebuilding is caught).

HOW TO RUN (from the Dashboard folder)
      python tools/build_firmware.py               build all three
      python tools/build_firmware.py camera        build just one
  About 3 minutes per program. Commit the firmware folder afterwards.

NEEDS
  The Arduino IDE 2 (it contains arduino-cli), with esp32 3.3.12 and the
  libraries from the README installed. The IDE's own arduino-cli settings
  file is used, so the libraries in Documents/Arduino are found.
"""

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

DASHBOARD = Path(__file__).resolve().parent.parent
V5 = DASHBOARD.parent
FIRMWARE = DASHBOARD / "firmware"
CORE = "esp32:esp32"
CORE_VERSION = "3.3.12"

# "id" is the same word each board says in its hello line, so the
# dashboard can tell which program a plugged-in board is running now.
PROGRAMS = [
    {"id": "station-radio", "title": "Station Radio", "sketch": "StationRadio_v5",
     "fqbn": "esp32:esp32:esp32", "board_name": "ESP32 Dev Module"},
    {"id": "vehicle-sensors", "title": "Vehicle Sensors", "sketch": "VehicleSensors_v5",
     "fqbn": "esp32:esp32:esp32", "board_name": "ESP32 Dev Module"},
    {"id": "camera", "title": "Camera", "sketch": "VehicleCamera_v5",
     "fqbn": "esp32:esp32:esp32wrover:PartitionScheme=huge_app",
     "board_name": "ESP32 Wrover Module, Huge APP (3MB No OTA/1MB SPIFFS)"},
]

# Where each piece comes from in the build folder, by its name in flash_args.
PIECE_NAMES = {".bootloader.bin": "bootloader.bin", ".partitions.bin": "partitions.bin",
               "boot_app0.bin": "boot_app0.bin", ".ino.bin": "app.bin"}


def source_fingerprint(sketch_dir):
    """SHA-256 over every file in the sketch folder, by name. Line endings
    are made the same first, so a Windows and a Mac checkout agree."""
    h = hashlib.sha256()
    for f in sorted(p for p in Path(sketch_dir).rglob("*") if p.is_file()):
        rel = f.relative_to(sketch_dir).as_posix()
        if rel.startswith("build/"):
            continue                       # the IDE's "Export binary" folder
        h.update(rel.encode() + b"\0")
        h.update(f.read_bytes().replace(b"\r\n", b"\n") + b"\0")
    return h.hexdigest()


def sha256_file(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def firmware_version(sketch_dir, sketch):
    text = (Path(sketch_dir) / f"{sketch}.ino").read_text(encoding="utf-8", errors="replace")
    m = re.search(r'#define\s+FIRMWARE_VERSION\s+"([^"]+)"', text)
    return m.group(1) if m else "?"


def home_spellings():
    home = str(Path.home())
    return sorted({home, home.replace("\\", "/")})


def prefix_maps():
    return [f"-ffile-prefix-map={h}=~" for h in home_spellings()]


def home_path_in(path):
    """The home folder's spelling if it appears in the file, else None."""
    data = Path(path).read_bytes().lower()
    for h in home_spellings():
        if h.lower().encode() in data:
            return h
    return None


def find_arduino_cli():
    found = shutil.which("arduino-cli")
    if found:
        return found
    for base in (os.environ.get("ProgramFiles", r"C:\Program Files"),
                 os.path.join(os.environ.get("LOCALAPPDATA", ""), "Programs")):
        exe = Path(base) / "Arduino IDE" / "resources" / "app" / "lib" / "backend" / "resources" / "arduino-cli.exe"
        if exe.is_file():
            return str(exe)
    for mac in ("/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli",):
        if Path(mac).is_file():
            return mac
    return None


def cli_base(cli):
    cmd = [cli]
    cfg = Path.home() / ".arduinoIDE" / "arduino-cli.yaml"
    if cfg.is_file():
        cmd += ["--config-file", str(cfg)]
    return cmd


def check_core(base):
    out = subprocess.run(base + ["core", "list", "--json"], capture_output=True, text=True)
    try:
        data = json.loads(out.stdout or "{}")
        platforms = data.get("platforms", data) if isinstance(data, dict) else data
        for p in platforms or []:
            pid = p.get("id")
            ver = p.get("installed_version") or p.get("installed")
            if pid == CORE:
                return ver
    except ValueError:
        pass
    return None


def parse_flash_args(path):
    """flash_args (written by the esp32 core) -> (flash options, [(offset, file name)])."""
    lines = [ln.strip() for ln in Path(path).read_text().splitlines() if ln.strip()]
    pieces = []
    for ln in lines[1:]:
        off, name = ln.split(None, 1)
        pieces.append((off.lower(), name.strip()))
    return lines[0], pieces


def build_one(base, prog, scratch):
    """Compile one sketch and copy its pieces into firmware/<id>/."""
    src = V5 / prog["sketch"]
    # Build a copy: the IDE and arduino-cli share a build folder per sketch
    # path, and building the open sketch while the IDE has it open breaks both.
    work = Path(scratch) / prog["sketch"]
    shutil.copytree(src, work, ignore=shutil.ignore_patterns("build"))
    build_dir = Path(scratch) / f"build-{prog['id']}"
    print(f"\n=== {prog['title']} ({prog['sketch']}) ===", flush=True)
    # The compiler writes file paths into the program (for error messages).
    # Those paths start with this computer's home folder, which has the
    # builder's login name in it; this repository is public, so swap the
    # home folder for "~" in everything the compiler writes.
    props = []
    for kind in ("c", "cpp", "S"):
        props += ["--build-property", f"compiler.{kind}.extra_flags=" + " ".join(prefix_maps())]
    r = subprocess.run(base + ["compile", "--fqbn", prog["fqbn"], "--build-path", str(build_dir)]
                       + props + [str(work)])
    if r.returncode != 0:
        raise SystemExit(f"Building {prog['sketch']} failed (see the messages above).")
    leak = home_path_in(build_dir / f"{prog['sketch']}.ino.bin")
    if leak:
        raise SystemExit(f"{prog['sketch']}: the program still contains this computer's home folder "
                         f"({leak!r}). Not copying it.")

    flash_opts, pieces = parse_flash_args(build_dir / "flash_args")
    out_dir = FIRMWARE / prog["id"]
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in out_dir.glob("*.bin"):     # (not rmtree: OneDrive often locks the folder itself)
        old.unlink()
    files = []
    for off, name in pieces:
        dest = next((v for k, v in PIECE_NAMES.items() if name.endswith(k)), None)
        if dest is None:
            raise SystemExit(f"Unexpected piece in flash_args: {name}")
        shutil.copyfile(build_dir / name, out_dir / dest)
        files.append({"offset": off, "path": f"{prog['id']}/{dest}",
                      "size": (out_dir / dest).stat().st_size, "sha256": sha256_file(out_dir / dest)})
    app_size = next(f["size"] for f in files if f["path"].endswith("app.bin"))
    print(f"  app is {app_size / 1024:.0f} KB; pieces: " + ", ".join(f"{f['offset']} {f['path']}" for f in files))
    return {
        "id": prog["id"], "title": prog["title"], "sketch": prog["sketch"],
        "fw": firmware_version(src, prog["sketch"]), "chip": "esp32",
        "fqbn": prog["fqbn"], "board_name": prog["board_name"], "flash_args": flash_opts,
        "source_sha256": source_fingerprint(src), "files": files,
    }


def git_commit():
    try:
        return subprocess.run(["git", "-C", str(V5), "rev-parse", "--short", "HEAD"],
                              capture_output=True, text=True).stdout.strip()
    except OSError:
        return ""


def main():
    ap = argparse.ArgumentParser(description="Build the v5 programs for the dashboard's installer")
    ap.add_argument("only", nargs="*", help="program ids to build (default: all): "
                    + ", ".join(p["id"] for p in PROGRAMS))
    args = ap.parse_args()
    wanted = [p for p in PROGRAMS if not args.only or p["id"] in args.only]
    if not wanted:
        raise SystemExit("No such program. Choose from: " + ", ".join(p["id"] for p in PROGRAMS))

    cli = find_arduino_cli()
    if not cli:
        raise SystemExit("arduino-cli not found. Install the Arduino IDE 2, or put arduino-cli on the PATH.")
    base = cli_base(cli)
    ver = check_core(base)
    if ver != CORE_VERSION:
        raise SystemExit(f"Need {CORE} {CORE_VERSION} installed (found: {ver or 'none'}). "
                         "Arduino IDE -> Boards Manager -> esp32 by Espressif Systems.")

    manifest_path = FIRMWARE / "manifest.json"
    old = {}
    if manifest_path.is_file():
        old = {p["id"]: p for p in json.loads(manifest_path.read_text(encoding="utf-8")).get("programs", [])}

    FIRMWARE.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="ftt-fw-") as scratch:
        for prog in wanted:
            old[prog["id"]] = build_one(base, prog, scratch)

    manifest = {
        "format": 1,
        "built": dt.datetime.now().strftime("%Y-%m-%d %H:%M"),
        "core": f"{CORE} {CORE_VERSION}",
        "git_commit": git_commit(),
        "note": "Made by tools/build_firmware.py. Do not edit by hand.",
        "programs": [old[p["id"]] for p in PROGRAMS if p["id"] in old],
    }
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"\nWrote {manifest_path.relative_to(DASHBOARD)}. Commit the firmware folder.")


if __name__ == "__main__":
    sys.exit(main())
