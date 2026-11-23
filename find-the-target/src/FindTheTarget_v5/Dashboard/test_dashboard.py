"""
Tests for start_dashboard.py. No hardware needed.

Run from this folder:
    python -m unittest test_dashboard -v
"""
import hashlib
import importlib.util
import io
import json
import os
import subprocess
import tempfile
import threading
import time
import unittest
import urllib.request
import zipfile
from pathlib import Path

import start_dashboard as sd


class FakeLink:
    """Stands in for the radio board: remembers what was sent to it."""
    def __init__(self):
        self.lines = []

    def send(self, line):
        self.lines.append(line)
        return True


def make_station(station_number=3):
    tmp = Path(tempfile.mkdtemp())
    sd.DATA, sd.MARKS_FILE = tmp, tmp / "marks.json"
    sd.SETTINGS_FILE, sd.CHAT_LOG_DIR = tmp / "settings.json", tmp / "chat-log"
    hub = sd.Hub()
    st = sd.Station(hub)
    st.link = FakeLink()
    st.handle({"type": "hello", "board": "station-radio", "station": station_number})
    st.link.lines.clear()
    return hub, st


class TextCleaning(unittest.TestCase):
    def test_newlines_become_spaces(self):
        self.assertEqual(sd.clean_text("hi\nthere\r\n", 120), "hi there")

    def test_long_text_is_cut_to_120_bytes(self):
        self.assertEqual(len(sd.clean_text("x" * 300, 120).encode()), 120)

    def test_emoji_is_never_cut_in_half(self):
        out = sd.clean_text("a" * 118 + "🎯", 120)   # the emoji is 4 bytes and does not fit
        self.assertEqual(out, "a" * 118)
        out.encode("utf-8")                          # would raise if broken


class BoardLines(unittest.TestCase):
    def test_json_line(self):
        self.assertEqual(sd.parse_board_line('{"type":"chat","from":2}')["from"], 2)

    def test_human_note(self):
        self.assertEqual(sd.parse_board_line("# Radio up"), {"type": "note", "text": "Radio up"})

    def test_garbage_is_ignored(self):
        self.assertIsNone(sd.parse_board_line("{not json"))
        self.assertIsNone(sd.parse_board_line('{"no":"type"}'))


class ChatAndMarks(unittest.TestCase):
    def test_hello_asks_everyone_for_marks(self):
        hub, st = make_station()
        st.handle({"type": "hello", "board": "station-radio", "station": 3})
        self.assertIn("sync", st.link.lines)
        self.assertEqual(hub.station, 3)

    def test_chat_to_one_station_becomes_a_command(self):
        hub, st = make_station()
        r = st.send_chat(2, "come\nhere")
        self.assertTrue(r["ok"])
        self.assertEqual(st.link.lines[-1], f"chat 2 {r['chat']['ref']} come here")

    def test_cannot_chat_to_yourself(self):
        hub, st = make_station()
        self.assertFalse(st.send_chat(3, "hi")["ok"])

    def test_delivery_report_updates_the_chat(self):
        hub, st = make_station()
        r = st.send_chat(2, "hi")
        st.handle({"type": "sent", "ref": r["chat"]["ref"], "state": "confirmed"})
        self.assertEqual(hub.chats[-1]["state"], "confirmed")

    def test_incoming_chat_is_kept_once_and_logged(self):
        hub, st = make_station()
        msg = {"type": "chat", "id": "2-9", "from": 2, "to": 0, "text": "found it"}
        st.handle(msg)
        st.handle(msg)
        self.assertEqual(sum(1 for c in hub.chats if c["id"] == "2-9"), 1)
        log = next(sd.CHAT_LOG_DIR.glob("*.txt")).read_text(encoding="utf-8")
        self.assertIn("Station 2 -> everyone: found it", log)

    def test_own_marks_are_saved_and_sent(self):
        hub, st = make_station()
        r = st.add_mark(2, 32.7, -117.1, "Big rock")
        self.assertTrue(r["ok"])
        self.assertTrue(st.link.lines[0].startswith(f"mark add {r['id']} 2 32.7000000 -117.1000000 Big rock"))
        self.assertEqual(st.link.lines[1], f"marklist {r['id']}")
        saved = json.loads(sd.MARKS_FILE.read_text())
        self.assertIn(str(r["id"]), saved["marks"])

    def test_mark_limit(self):
        hub, st = make_station()
        for _ in range(sd.MAX_OWN_MARKS):
            self.assertTrue(st.add_mark(3, 1.0, 1.0, "")["ok"])
        self.assertFalse(st.add_mark(3, 1.0, 1.0, "")["ok"])

    def test_cannot_remove_another_stations_mark(self):
        hub, st = make_station()
        st.handle({"type": "mark", "from": 5, "action": "add", "id": 1, "kind": 1, "lat": 1, "lon": 1, "label": ""})
        self.assertFalse(st.remove_mark(1)["ok"])

    def test_mark_list_drops_marks_whose_removal_was_lost(self):
        hub, st = make_station()
        for mid in (1, 2):
            st.handle({"type": "mark", "from": 5, "action": "add", "id": mid, "kind": 1,
                       "lat": 1, "lon": 1, "label": ""})
        st.handle({"type": "marklist", "from": 5, "ids": [2]})
        self.assertEqual([m["id"] for m in hub.other_marks.values()], [2])

    def test_sync_request_resends_our_marks(self):
        hub, st = make_station()
        st.add_mark(1, 1.0, 2.0, "x")
        st.link.lines.clear()
        st.last_sync_reply = 0
        st.handle({"type": "sync", "from": 4})
        self.assertTrue(any(l.startswith("mark add") for l in st.link.lines))
        self.assertTrue(st.link.lines[-1].startswith("marklist"))


class Spreadsheets(unittest.TestCase):
    def test_csv(self):
        rows = sd.read_table_file("r.csv", b"name,x_ft,y_ft\nA,10,20\n")
        self.assertEqual(rows, [["name", "x_ft", "y_ft"], ["A", "10", "20"]])

    def test_xlsx_examples_from_v4_still_read(self):
        # The v4 templates live in legacy/ after the move; read whichever exists.
        here = Path(__file__).resolve().parent
        candidates = list(here.parent.parent.glob("**/waypoints_example_xy.xlsx"))
        if not candidates:
            self.skipTest("example spreadsheet not found")
        rows = sd.read_table_file("w.xlsx", candidates[0].read_bytes())
        self.assertEqual([c.lower() for c in rows[0][:3]], ["name", "x_ft", "y_ft"])
        self.assertGreater(len(rows), 1)


# =====================================================================
# Installing a program on a board
# =====================================================================
def make_firmware(folder):
    """A small pretend firmware folder, laid out like the real one."""
    folder = Path(folder)
    files = []
    for off, name, size in (("0x1000", "bootloader.bin", 100), ("0x8000", "partitions.bin", 50),
                            ("0xe000", "boot_app0.bin", 20), ("0x10000", "app.bin", 1000)):
        p = folder / "vehicle-sensors" / name
        p.parent.mkdir(parents=True, exist_ok=True)
        data = bytes([len(name)]) * size
        p.write_bytes(data)
        files.append({"offset": off, "path": f"vehicle-sensors/{name}", "size": size,
                      "sha256": hashlib.sha256(data).hexdigest()})
    manifest = {"format": 1, "built": "today", "programs": [
        {"id": "vehicle-sensors", "title": "Vehicle Sensors", "fw": "5.0", "chip": "esp32", "files": files}]}
    (folder / "manifest.json").write_text(json.dumps(manifest))
    return manifest


class InstallerCommand(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.prog = make_firmware(self.tmp)["programs"][0]

    def test_every_piece_goes_to_its_address_in_order(self):
        cmd = sd.esptool_command(["esptool"], "COM7", self.prog, self.tmp)
        tail = cmd[cmd.index("--flash_size") + 2:]
        self.assertEqual(tail[0::2], ["0x1000", "0x8000", "0xe000", "0x10000"])
        self.assertEqual(Path(tail[7]), self.tmp / "vehicle-sensors" / "app.bin")

    def test_port_chip_and_speed(self):
        cmd = sd.esptool_command(["py", "-m", "esptool"], "COM7", self.prog, self.tmp, baud=115200)
        self.assertEqual(cmd[:3], ["py", "-m", "esptool"])
        joined = " ".join(cmd)
        self.assertIn("--chip esp32 --port COM7 --baud 115200", joined)
        # Underscore spellings: esptool 4 needs them and esptool 5 still takes them.
        self.assertIn("--before default_reset --after hard_reset write_flash -z", joined)
        self.assertIn("--flash_mode keep --flash_freq keep --flash_size keep", joined)

    def test_saved_settings_area_is_never_written(self):
        # The board keeps its station number and WiFi at 0x9000-0xDFFF (NVS).
        cmd = sd.esptool_command(["esptool"], "COM7", self.prog, self.tmp)
        sizes = {f["offset"]: f["size"] for f in self.prog["files"]}
        for off in cmd[cmd.index("--flash_size") + 2::2]:
            start, end = int(off, 16), int(off, 16) + sizes[off]
            self.assertFalse(start < 0xE000 and end > 0x9000, off)

    def test_esptool_python_package_comes_first(self):
        self.assertEqual(sd.find_esptool(self.tmp, have_module=True)[1:], ["-m", "esptool"])

    def test_esptool_falls_back_to_the_newest_arduino_copy(self):
        tools = self.tmp / "packages" / "esp32" / "tools" / "esptool_py"
        for v in ("4.5.1", "4.10.0", "4.9.0"):
            (tools / v).mkdir(parents=True)
            (tools / v / "esptool.exe").write_bytes(b"")
        self.assertEqual(Path(sd.find_esptool(self.tmp, have_module=False)[0]).parent.name, "4.10.0")

    def test_no_esptool_anywhere(self):
        self.assertIsNone(sd.find_esptool(self.tmp, have_module=False))

    def test_firmware_check_finds_a_damaged_file(self):
        self.assertEqual(sd.load_firmware(self.tmp)[1], "")
        (self.tmp / "vehicle-sensors" / "app.bin").write_bytes(b"oops")
        self.assertIn("damaged", sd.load_firmware(self.tmp)[1])

    def test_firmware_check_finds_a_missing_folder(self):
        self.assertIn("missing", sd.load_firmware(self.tmp / "nothing")[1])


class InstallerProgress(unittest.TestCase):
    FILES = [{"offset": "0x1000", "size": 1000}, {"offset": "0x10000", "size": 9000}]

    def test_esptool_5_lines(self):
        p = sd.FlashProgress(self.FILES)
        self.assertEqual(p.feed("Connecting....")["stage"], "connecting")
        self.assertEqual(p.feed("Writing at 0x000011f4 [=====      ]  50.0% 500/1000 bytes [00:00]")["percent"], 5)
        self.assertEqual(p.feed("Hash of data verified.")["percent"], 10)
        self.assertEqual(p.feed("Writing at 0x00011194 [===   ]  50.0% 4500/9000 bytes")["percent"], 55)
        self.assertEqual(p.feed("Hard resetting via RTS pin...")["percent"], 100)

    def test_esptool_4_lines(self):
        p = sd.FlashProgress(self.FILES)
        self.assertEqual(p.feed("Writing at 0x00011388... (55 %)")["percent"], 60)

    def test_percent_never_goes_backwards(self):
        p = sd.FlashProgress(self.FILES)
        p.feed("Writing at 0x00011388... (55 %)")
        self.assertEqual(p.feed("Writing at 0x00001000... (0 %)")["percent"], 60)

    def test_other_lines_are_ignored(self):
        self.assertIsNone(sd.FlashProgress(self.FILES).feed("Chip is ESP32-D0WD-V3"))


class InstallerErrors(unittest.TestCase):
    def test_port_used_by_another_program(self):
        msg = sd.explain_flash_error("could not open port 'COM5': PermissionError(13, 'Access is denied.')")
        self.assertIn("Serial Monitor", msg)

    def test_unplugged(self):
        msg = sd.explain_flash_error("could not open port 'COM5': FileNotFoundError(2, 'The system cannot find')")
        self.assertIn("unplugged", msg)

    def test_esptool_5_busy_or_missing_wording_is_read_correctly(self):
        # esptool 5.6 prints this for a port that is not there (seen on Windows 11).
        missing = ("ERROR: A fatal error occurred: Could not open COM99, the port is busy or doesn't exist.\n"
                   "(could not open port 'COM99': FileNotFoundError(2, 'The system cannot find the file specified.'))")
        self.assertIn("unplugged", sd.explain_flash_error(missing))
        busy = missing.replace("FileNotFoundError(2, 'The system cannot find the file specified.')",
                               "PermissionError(13, 'Access is denied.', None, 5)")
        self.assertIn("Serial Monitor", sd.explain_flash_error(busy))

    def test_board_does_not_answer(self):
        msg = sd.explain_flash_error("A fatal error occurred: Failed to connect to ESP32: No serial data received.")
        self.assertIn("BOOT", msg)

    def test_wrong_kind_of_board(self):
        self.assertIn("different kind", sd.explain_flash_error("This chip is ESP32-S3, not ESP32. Wrong chip argument?"))

    def test_anything_else_says_nothing_is_broken(self):
        self.assertIn("Nothing is broken", sd.explain_flash_error("something odd"))


class FakeSerialLink:
    """Stands in for SerialLink: closing it calls on_close from another
    thread, like the real reading thread does."""
    def __init__(self, port, on_line, on_close):
        self.port, self.on_line, self.on_close, self.closed = port, on_line, on_close, False

    def close(self):
        self.closed = True
        threading.Thread(target=self.on_close, args=(self,)).start()

    def send(self, line):
        return True


def make_watcher(ports=("COM5",)):
    hub = sd.Hub()
    w = sd.PortWatcher(hub, sd.Station(hub), list_ports=lambda: [{"port": p, "desc": "USB"} for p in ports],
                       open_link=FakeSerialLink)
    return w


class PortHandOff(unittest.TestCase):
    def test_release_closes_the_port_and_keeps_it_closed(self):
        w = make_watcher()
        w.scan_once()
        link = w.links["COM5"]
        w.release("COM5", settle=0)
        self.assertTrue(link.closed)
        self.assertNotIn("COM5", w.links)
        w.scan_once()                                  # the watcher's loop must not grab it back
        self.assertNotIn("COM5", w.links)
        self.assertTrue(w.port_list()[0]["busy"])

    def test_resume_opens_it_again(self):
        w = make_watcher()
        w.scan_once()
        old = w.links["COM5"]
        w.release("COM5", settle=0)
        w.resume("COM5")
        self.assertIn("COM5", w.links)
        self.assertIsNot(w.links["COM5"], old)
        self.assertFalse(w.port_list()[0]["busy"])

    def test_a_late_close_of_the_old_link_leaves_the_new_one(self):
        w = make_watcher()
        w.scan_once()
        old = w.links["COM5"]
        w.release("COM5", settle=0)
        w.resume("COM5")
        w.on_close(old)
        self.assertIn("COM5", w.links)

    def test_other_ports_stay_open(self):
        w = make_watcher(("COM5", "COM6"))
        w.scan_once()
        keep = w.links["COM6"]
        w.release("COM5", settle=0)
        self.assertIs(w.links["COM6"], keep)

    def test_hello_after_install_is_seen(self):
        w = make_watcher()
        w.scan_once()
        w.on_line(w.links["COM5"], '{"type":"hello","board":"camera","station":0}')
        w.release("COM5", settle=0)
        self.assertNotIn("COM5", w.hello)              # the old hello is from the old program
        w.resume("COM5")
        threading.Timer(0.3, w.on_line, [w.links["COM5"], '{"type":"hello","board":"vehicle-sensors","station":4}']).start()
        self.assertEqual(w.wait_for_hello("COM5", timeout=3)["board"], "vehicle-sensors")
        self.assertEqual(w.port_list()[0]["station"], 4)


class FakeProcess:
    def __init__(self, output, rc):
        self.stdout = io.BytesIO(output.encode("utf-8"))
        self.rc = rc

    def wait(self, timeout=None):
        return self.rc

    def kill(self):
        pass


class FakeWatcher:
    def __init__(self, events, hello=None):
        self.events, self._hello = events, hello

    def port_list(self):
        return [{"port": "COM5"}]

    def release(self, port):
        self.events.append(("release", port))

    def resume(self, port):
        self.events.append(("resume", port))

    def wait_for_hello(self, port, timeout=15):
        return self._hello


GOOD_RUN = ("esptool v5.6.0\nConnecting....\nConnected to ESP32 on COM5:\n"
            "Writing at 0x00001064 [=>   ]  10.0% 10/100 bytes\nHash of data verified.\n"
            "Writing at 0x00010200 [==>  ]  50.0% 500/1000 bytes\r"
            "Writing at 0x000103e8 [=====] 100.0% 1000/1000 bytes\nHash of data verified.\n"
            "Hard resetting via RTS pin...\n")


class FlasherRuns(unittest.TestCase):
    def run_install(self, output, rc, hello=None, popen=None):
        tmp = Path(tempfile.mkdtemp())
        make_firmware(tmp)
        events, published = [], []
        hub = sd.Hub()
        hub.publish = lambda ev, data: published.append((ev, data))

        def fake_popen(cmd, **kw):
            events.append(("esptool", cmd[cmd.index("--port") + 1]))
            if popen:
                return popen(cmd, **kw)
            return FakeProcess(output, rc)

        f = sd.Flasher(hub, FakeWatcher(events, hello), folder=tmp, esptool=["esptool"], popen=fake_popen)
        self.assertTrue(f.start("COM5", "vehicle-sensors")["ok"])
        end = time.time() + 5
        while f.state["busy"] and time.time() < end:
            time.sleep(0.02)
        return f, events, [d for e, d in published if e == "flash"]

    def test_port_is_handed_over_and_back(self):
        f, events, _ = self.run_install(GOOD_RUN, 0)
        self.assertEqual(events, [("release", "COM5"), ("esptool", "COM5"), ("resume", "COM5")])

    def test_success_shows_progress_then_asks_for_the_station(self):
        hello = {"type": "hello", "board": "vehicle-sensors", "station": 0}
        f, _, flashes = self.run_install(GOOD_RUN, 0, hello)
        stages = [s["stage"] for s in flashes]
        self.assertEqual(stages[0], "releasing")
        self.assertIn("connecting", stages)
        self.assertIn("writing", stages)
        self.assertEqual(stages[-1], "done")
        percents = [s["percent"] for s in flashes]
        self.assertEqual(percents, sorted(percents))
        self.assertTrue(f.state["ok"])
        self.assertEqual(f.state["hello"]["board"], "vehicle-sensors")   # the page asks for the number next

    def test_failure_is_explained_and_the_port_is_given_back(self):
        f, events, _ = self.run_install("Connecting......\nA fatal error occurred: Failed to connect to ESP32\n", 2)
        self.assertFalse(f.state["ok"])
        self.assertIn("BOOT", f.state["error"])
        self.assertIn("Failed to connect", f.state["log"])
        self.assertEqual(events[-1], ("resume", "COM5"))

    def test_esptool_that_cannot_start_still_gives_the_port_back(self):
        def broken(cmd, **kw):
            raise OSError("no such program")
        f, events, _ = self.run_install("", 0, popen=broken)
        self.assertFalse(f.state["ok"])
        self.assertEqual(events[-1], ("resume", "COM5"))

    def test_one_install_at_a_time(self):
        tmp = Path(tempfile.mkdtemp())
        make_firmware(tmp)
        f = sd.Flasher(sd.Hub(), FakeWatcher([]), folder=tmp, esptool=["esptool"])
        f.state["busy"] = True
        self.assertFalse(f.start("COM5", "vehicle-sensors")["ok"])

    def test_unplugged_board_and_unknown_program_are_refused(self):
        tmp = Path(tempfile.mkdtemp())
        make_firmware(tmp)
        f = sd.Flasher(sd.Hub(), FakeWatcher([]), folder=tmp, esptool=["esptool"])
        self.assertIn("not plugged in", f.start("COM9", "vehicle-sensors")["error"])
        self.assertFalse(f.start("COM5", "toaster")["ok"])

    def test_missing_esptool_is_explained(self):
        tmp = Path(tempfile.mkdtemp())
        make_firmware(tmp)
        f = sd.Flasher(sd.Hub(), FakeWatcher([]), folder=tmp)
        f.esptool = lambda: None
        self.assertIn("esptool", f.info()["problem"])
        self.assertFalse(f.start("COM5", "vehicle-sensors")["ok"])


def partition_table(data):
    """ESP32 partition table -> {name: (type, subtype, offset, size)}."""
    out = {}
    for i in range(0, len(data) - 31, 32):
        e = data[i:i + 32]
        if e[:2] != b"\xaa\x50":
            break
        name = e[12:28].split(b"\0")[0].decode()
        out[name] = (e[2], e[3], int.from_bytes(e[4:8], "little"), int.from_bytes(e[8:12], "little"))
    return out


class ShippedFirmware(unittest.TestCase):
    """The real files in firmware/, made by tools/build_firmware.py."""

    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.problem = sd.load_firmware()

    def test_all_three_programs_are_there_and_intact(self):
        self.assertEqual(self.problem, "")
        self.assertEqual([p["id"] for p in self.manifest["programs"]],
                         ["station-radio", "vehicle-sensors", "camera"])

    def test_built_from_the_sketches_as_they_are_now(self):
        spec = importlib.util.spec_from_file_location("build_firmware", sd.HERE / "tools" / "build_firmware.py")
        bf = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(bf)
        for p in self.manifest["programs"]:
            self.assertEqual(p["source_sha256"], bf.source_fingerprint(sd.HERE.parent / p["sketch"]),
                             f"{p['sketch']} changed since the firmware was built. "
                             "Run: python tools/build_firmware.py")

    def test_camera_has_the_huge_app_layout(self):
        def table(pid):
            prog = next(p for p in self.manifest["programs"] if p["id"] == pid)
            f = next(f for f in prog["files"] if f["path"].endswith("partitions.bin"))
            return partition_table((sd.FIRMWARE_DIR / f["path"]).read_bytes())
        self.assertEqual(table("camera")["app0"][3], 0x300000)     # 3 MB
        self.assertLess(table("vehicle-sensors")["app0"][3], 0x300000)

    def test_settings_area_is_where_the_installer_leaves_it_alone(self):
        for p in self.manifest["programs"]:
            f = next(f for f in p["files"] if f["path"].endswith("partitions.bin"))
            nvs = partition_table((sd.FIRMWARE_DIR / f["path"]).read_bytes())["nvs"]
            self.assertEqual((nvs[2], nvs[3]), (0x9000, 0x5000), p["id"])
            for piece in p["files"]:
                start = int(piece["offset"], 16)
                self.assertFalse(start < 0xE000 and start + piece["size"] > 0x9000, piece["path"])

    def test_no_login_names_in_the_programs(self):
        # The compiler writes file paths into programs. This repository is
        # public, so no home-folder paths (which contain a login name).
        for p in self.manifest["programs"]:
            for f in p["files"]:
                data = (sd.FIRMWARE_DIR / f["path"]).read_bytes().lower()
                for bad in (b":\\users\\", b":/users/", b"/home/", b"/users/"):
                    self.assertNotIn(bad, data, f"{f['path']} contains a home-folder path")

    def test_real_esptool_accepts_the_command(self):
        tool = sd.find_esptool()
        if not tool:
            self.skipTest("esptool is not installed")
        cmd = sd.esptool_command(tool, "COM_DOES_NOT_EXIST", self.manifest["programs"][0])
        out = subprocess.run(cmd, capture_output=True, text=True, timeout=120,
                             env=dict(os.environ, PYTHONIOENCODING="utf-8", NO_COLOR="1"))
        text = (out.stdout + out.stderr).lower()
        self.assertNotEqual(out.returncode, 0)
        self.assertNotIn("usage:", text)               # every argument was understood...
        self.assertIn("port", text)                    # ...and it stopped at opening the port


class PracticeModeServer(unittest.TestCase):
    """Start the real web server in practice mode and use it like a browser."""

    @classmethod
    def setUpClass(cls):
        tmp = Path(tempfile.mkdtemp())
        sd.DATA, sd.MARKS_FILE = tmp, tmp / "marks.json"
        sd.SETTINGS_FILE, sd.CHAT_LOG_DIR = tmp / "settings.json", tmp / "chat-log"
        hub = sd.Hub(simulate=True)
        st = sd.Station(hub)
        sim = sd.SimRadio(hub, st, 3, 1)
        threading.Thread(target=sim.run, daemon=True).start()
        cls.server = sd.ThreadingHTTPServer(("127.0.0.1", 0), sd.make_handler(hub, st, None))
        cls.base = f"http://127.0.0.1:{cls.server.server_address[1]}"
        threading.Thread(target=cls.server.serve_forever, daemon=True).start()
        time.sleep(2.5)

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()

    def get(self, path):
        with urllib.request.urlopen(self.base + path, timeout=5) as r:
            return r.read()

    def post(self, path, body):
        req = urllib.request.Request(self.base + path, json.dumps(body).encode(),
                                     {"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=5) as r:
            return json.loads(r.read())

    def test_page_and_state(self):
        self.assertIn(b"Find the Target", self.get("/"))
        state = json.loads(self.get("/state"))
        self.assertEqual(state["station"], 1)
        self.assertIn("11", state["vehicles"])           # our own vehicle is driving

    def test_chat_round_trip(self):
        r = self.post("/chat", {"to": 2, "text": "hello"})
        self.assertTrue(r["ok"])
        time.sleep(1.0)
        chats = json.loads(self.get("/state"))["chats"]
        mine = [c for c in chats if c.get("ref") == r["chat"]["ref"]]
        self.assertEqual(mine[0]["state"], "confirmed")

    def test_installer_is_off_in_practice_mode(self):
        ports = json.loads(self.get("/ports"))
        self.assertFalse(ports["available"])
        self.assertIn("practice mode", ports["problem"])
        self.assertFalse(self.post("/install", {"port": "COM5", "program": "camera"})["ok"])

    def test_page_works_offline(self):
        page = self.get("/").decode("utf-8")
        self.assertNotIn("https://", page.split("<script>")[0].replace("https://www.python.org", ""))


if __name__ == "__main__":
    unittest.main()
