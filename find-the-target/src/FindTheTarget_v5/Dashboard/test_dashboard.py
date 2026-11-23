"""
Tests for start_dashboard.py. No hardware needed.

Run from this folder:
    python -m unittest test_dashboard -v
"""
import io
import json
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

    def test_page_works_offline(self):
        page = self.get("/").decode("utf-8")
        self.assertNotIn("https://", page.split("<script>")[0].replace("https://www.python.org", ""))


if __name__ == "__main__":
    unittest.main()
