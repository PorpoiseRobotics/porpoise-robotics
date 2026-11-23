#!/usr/bin/env python3
"""
start_dashboard.py -- Find the Target v5 station dashboard
==========================================================

WHAT THIS PROGRAM DOES
  It is the bridge between your station's radio board (plugged into this
  laptop's USB port) and the dashboard page in your web browser.

    radio board --USB--> this program --web page--> your browser
    radio board <--USB-- this program <--buttons--- your browser

  1. Finds the StationRadio_v5 board on a USB port by itself.
  2. Reads the lines the board prints. Each one is JSON with a "type"
     (telemetry, chat, mark, ...), and is passed to the browser.
  3. When you press Send or drop a mark, it writes a plain command such
     as "chat 0 12 hello" back to the board.
  4. Keeps YOUR station's map marks in data/marks.json and re-sends them
     every 30 seconds, so a station that joins late still gets them.
  5. Saves every chat line to data/chat-log/<date>.txt on this laptop.
  6. Finds your vehicle's camera on the WiFi network by itself.

HOW TO RUN
  Easiest: double-click "Start Dashboard.bat" in this folder.
  Or from a terminal in this folder:
      python start_dashboard.py              (real radio board)
      python start_dashboard.py --simulate 4 (no hardware: 4 fake stations)
  Then open http://localhost:8000 (the .bat file does that for you).

REQUIREMENTS
  Python 3.8 or newer, and pyserial (the .bat file installs it once,
  which needs internet that one time). --simulate works without pyserial.

Students may read this code: it is commented to be read.
"""

import argparse
import concurrent.futures
import datetime as dt
import ipaddress
import json
import math
import queue
import random
import socket
import threading
import time
import urllib.request
import xml.etree.ElementTree as ET
import zipfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from io import BytesIO
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:          # fine for --simulate; checked again in main()
    serial = None

HERE = Path(__file__).resolve().parent
DATA = HERE / "data"                  # this laptop's own files (not on GitHub)
SETTINGS_FILE = DATA / "settings.json"
MARKS_FILE = DATA / "marks.json"
CHAT_LOG_DIR = DATA / "chat-log"

MAX_STATIONS = 9
CHAT_MAX_BYTES = 120                  # what one radio message can carry
MAX_OWN_MARKS = 40                    # what one mark list can carry
MARK_LABEL_MAX = 24
MARK_REFRESH_S = 30                   # re-send our marks this often
MARK_FORGET_S = 120                   # forget another station's mark if not refreshed
VEHICLE_LOST_S = 5                    # no reading for this long = "not heard"

# Settings a grown-up can change on the Setup page. Saved per laptop.
DEFAULT_SETTINGS = {
    "camera_url": "",          # blank = find the camera by itself
    "field_origin_lat": 0.0,   # GPS of the map's bottom-left corner
    "field_origin_lon": 0.0,
    "field_bearing_deg": 0.0,  # compass direction along the field LENGTH
    "field_length_m": 100.0,
    "field_width_m": 64.0,
    "dest_x_ft": None,         # optional destination, feet from the origin corner
    "dest_y_ft": None,
    "show_other_vehicles": True,
}


def now_s():
    return time.time()


# =====================================================================
# Small helpers that are easy to test on their own
# =====================================================================
def clean_text(text, max_bytes):
    """Make text safe to send over the radio: one line, no control
    characters, and at most max_bytes bytes of UTF-8 (never cutting a
    letter or emoji in half)."""
    text = "".join(ch if ch.isprintable() else " " for ch in str(text))
    text = " ".join(text.split())                       # collapse spaces
    data = text.encode("utf-8")[:max_bytes]
    return data.decode("utf-8", errors="ignore").strip()


def parse_board_line(line):
    """A line from a board: JSON object -> dict, "# note" -> ("note", text),
    anything else -> None."""
    line = line.strip()
    if not line:
        return None
    if line.startswith("{"):
        try:
            obj = json.loads(line)
            return obj if isinstance(obj, dict) and "type" in obj else None
        except json.JSONDecodeError:
            return None
    if line.startswith("#"):
        return {"type": "note", "text": line.lstrip("# ").rstrip()}
    return None


def read_table_file(name, data):
    """Rows (lists of strings) from a .csv or .xlsx file. Excel files are
    zip files full of XML; we read the first sheet ourselves so the
    dashboard needs nothing downloaded from the internet."""
    if name.lower().endswith(".csv"):
        text = data.decode("utf-8-sig", errors="replace")
        return [[c.strip() for c in ln.split(",")] for ln in text.splitlines() if ln.strip()]
    ns = {"m": "http://schemas.openxmlformats.org/spreadsheetml/2006/main"}
    with zipfile.ZipFile(BytesIO(data)) as z:
        shared = []
        if "xl/sharedStrings.xml" in z.namelist():
            root = ET.fromstring(z.read("xl/sharedStrings.xml"))
            for si in root.findall("m:si", ns):
                shared.append("".join(t.text or "" for t in si.iter(f"{{{ns['m']}}}t")))
        sheets = sorted(n for n in z.namelist() if n.startswith("xl/worksheets/sheet"))
        root = ET.fromstring(z.read(sheets[0]))
    rows = []
    for row in root.iter(f"{{{ns['m']}}}row"):
        cells = {}
        for c in row.findall("m:c", ns):
            ref = c.get("r", "A1")
            col = 0
            for ch in ref:
                if ch.isalpha():
                    col = col * 26 + (ord(ch.upper()) - 64)
            v = c.find("m:v", ns)
            if c.get("t") == "s" and v is not None:
                val = shared[int(v.text)]
            elif c.get("t") == "inlineStr":
                val = "".join(t.text or "" for t in c.iter(f"{{{ns['m']}}}t"))
            else:
                val = v.text if v is not None else ""
            cells[col - 1] = val or ""
        if cells:
            rows.append([cells.get(i, "") for i in range(max(cells) + 1)])
    return rows


# =====================================================================
# Shared state: everything the browser can see
# =====================================================================
class Hub:
    """One object that the radio thread writes to and the web server
    reads from. The lock keeps the two from tripping over each other."""

    def __init__(self, simulate=False):
        self.lock = threading.RLock()
        self.simulate = simulate
        self.station = 0                 # our station number (from the radio)
        self.radio = {"connected": False, "port": "", "fw": "", "needs_station": False}
        self.vehicles = {}               # node -> latest telemetry dict
        self.chats = []                  # newest last, at most 200
        self.my_marks = {}               # id -> {kind, lat, lon, label}
        self.next_mark_id = 1
        self.other_marks = {}            # "from-id" -> {from, id, kind, lat, lon, label, seen}
        self.roster = []
        self.notes = []                  # recent "# ..." lines from the board
        self.boards = {}                 # port -> hello dict, for the Setup page
        self.camera = {"url": "", "found_by": "", "searching": False, "ip": ""}
        self.listeners = []              # one queue per open browser tab
        self.next_ref = 1
        self.pending_refs = {}           # ref -> chat dict (to show "delivered")
        self.settings = load_settings()
        self.load_marks()

    # ---------------- browser events ----------------
    def publish(self, event, data):
        """Send one named event to every open dashboard tab."""
        with self.lock:
            for q in list(self.listeners):
                try:
                    q.put_nowait((event, data))
                except queue.Full:
                    pass                  # a stuck tab must not block the radio

    def snapshot(self):
        with self.lock:
            return {
                "station": self.station,
                "radio": dict(self.radio),
                "vehicles": {str(k): self._with_age(v) for k, v in self.vehicles.items()},
                "chats": self.chats[-200:],
                "my_marks": [dict(id=i, **m) for i, m in sorted(self.my_marks.items())],
                "other_marks": list(self.other_marks.values()),
                "roster": self.roster,
                "notes": self.notes[-50:],
                "boards": [dict(port=p, **h) for p, h in self.boards.items()],
                "camera": dict(self.camera),
                "settings": dict(self.settings),
                "simulate": self.simulate,
                "limits": {"chat_bytes": CHAT_MAX_BYTES, "marks": MAX_OWN_MARKS,
                           "label": MARK_LABEL_MAX, "stations": MAX_STATIONS},
            }

    @staticmethod
    def _with_age(v):
        out = dict(v)
        out["age_s"] = round(now_s() - v.get("_at", 0), 1)
        out.pop("_at", None)
        return out

    def note(self, text):
        with self.lock:
            self.notes.append(f"{time.strftime('%H:%M:%S')} {text}")
            self.notes = self.notes[-200:]
        self.publish("note", {"text": text})

    # ---------------- marks: saved on this laptop ----------------
    def load_marks(self):
        try:
            data = json.loads(MARKS_FILE.read_text(encoding="utf-8"))
            self.my_marks = {int(k): v for k, v in data.get("marks", {}).items()}
            self.next_mark_id = int(data.get("next_id", 1))
        except (OSError, ValueError):
            self.my_marks, self.next_mark_id = {}, 1

    def save_marks(self):
        DATA.mkdir(exist_ok=True)
        MARKS_FILE.write_text(json.dumps(
            {"next_id": self.next_mark_id, "marks": self.my_marks}, indent=2), encoding="utf-8")


def load_settings():
    s = dict(DEFAULT_SETTINGS)
    try:
        s.update({k: v for k, v in json.loads(SETTINGS_FILE.read_text(encoding="utf-8")).items()
                  if k in DEFAULT_SETTINGS})
    except (OSError, ValueError):
        pass
    return s


def save_settings(s):
    DATA.mkdir(exist_ok=True)
    SETTINGS_FILE.write_text(json.dumps(s, indent=2), encoding="utf-8")


def log_chat(line):
    """Append one line to today's chat log on this laptop."""
    try:
        CHAT_LOG_DIR.mkdir(parents=True, exist_ok=True)
        f = CHAT_LOG_DIR / f"{dt.date.today().isoformat()}.txt"
        with f.open("a", encoding="utf-8") as out:
            out.write(f"{time.strftime('%H:%M:%S')}  {line}\n")
    except OSError:
        pass


def who(n):
    return "everyone" if n == 0 else f"Station {n}"


# =====================================================================
# The station logic: what to do with each line from the radio
# =====================================================================
class Station:
    def __init__(self, hub):
        self.hub = hub
        self.link = None                  # object with .send(line); set by the radio side
        self.last_refresh = 0.0
        self.last_sync_reply = 0.0

    def send(self, line):
        if self.link is None:
            return False
        return self.link.send(line)

    # ---------------- board -> dashboard ----------------
    def handle(self, msg):
        h = self.hub
        t = msg.get("type")
        if t == "note":
            h.note(msg["text"])
        elif t == "hello":
            with h.lock:
                h.station = int(msg.get("station") or 0)
                h.radio.update(connected=True, fw=msg.get("fw", ""),
                               needs_station=(h.station == 0))
            h.publish("status", self.status())
            if h.station:
                # We may have restarted while the radio kept running: ask
                # everyone for their marks, and send ours.
                self.send("sync")
                self.refresh_marks(force=True)
        elif t == "telemetry":
            msg["_at"] = now_s()
            with h.lock:
                h.vehicles[int(msg.get("node", 0))] = msg
            h.publish("telemetry", Hub._with_age(msg))
        elif t == "chat":
            frm, to = int(msg.get("from", 0)), int(msg.get("to", 0))
            chat = {"id": msg.get("id"), "from": frm, "to": to,
                    "text": clean_text(msg.get("text", ""), 400), "time": time.strftime("%H:%M")}
            with h.lock:
                if any(c.get("id") == chat["id"] for c in h.chats[-50:]):
                    return                # already shown
                h.chats.append(chat)
                h.chats = h.chats[-200:]
            log_chat(f"{who(frm)} -> {who(to)}: {chat['text']}")
            h.publish("chat", chat)
        elif t == "mark":
            frm, mid = int(msg.get("from", 0)), int(msg.get("id", 0))
            key = f"{frm}-{mid}"
            with h.lock:
                if msg.get("action") == "remove":
                    h.other_marks.pop(key, None)
                else:
                    h.other_marks[key] = {
                        "key": key, "from": frm, "id": mid, "kind": int(msg.get("kind", 3)),
                        "lat": float(msg.get("lat", 0)), "lon": float(msg.get("lon", 0)),
                        "label": clean_text(msg.get("label", ""), 60), "seen": now_s()}
            h.publish("marks", self.marks_payload())
        elif t == "marklist":
            frm, ids = int(msg.get("from", 0)), {int(i) for i in msg.get("ids", [])}
            with h.lock:
                for key in [k for k, m in h.other_marks.items() if m["from"] == frm]:
                    if h.other_marks[key]["id"] not in ids:
                        del h.other_marks[key]   # its removal message was lost
                    else:
                        h.other_marks[key]["seen"] = now_s()
            h.publish("marks", self.marks_payload())
        elif t == "sync":
            if now_s() - self.last_sync_reply > 5:      # many stations may ask at once
                self.last_sync_reply = now_s()
                self.refresh_marks(force=True)
        elif t == "sent":
            ref, state = int(msg.get("ref", -1)), msg.get("state", "")
            with h.lock:
                chat = h.pending_refs.get(ref)
                if chat:
                    chat["state"] = state
                    if state in ("confirmed", "failed", "sent", "busy"):
                        h.pending_refs.pop(ref, None)
            if chat:
                h.publish("chat_state", {"ref": ref, "state": state})
        elif t == "roster":
            with h.lock:
                h.roster = msg.get("nodes", [])
        elif t == "error":
            h.note("Radio says: " + str(msg.get("text", "")))

    def status(self):
        h = self.hub
        with h.lock:
            own = h.vehicles.get(10 + h.station) if h.station else None
            return {"station": h.station, "radio": dict(h.radio),
                    "vehicle_age": round(now_s() - own["_at"], 1) if own else None,
                    "camera": dict(h.camera)}

    def marks_payload(self):
        h = self.hub
        with h.lock:
            for key in [k for k, m in h.other_marks.items() if now_s() - m["seen"] > MARK_FORGET_S]:
                del h.other_marks[key]
            return {"mine": [dict(id=i, **m) for i, m in sorted(h.my_marks.items())],
                    "others": list(h.other_marks.values())}

    # ---------------- dashboard -> board ----------------
    def send_chat(self, to, text):
        h = self.hub
        text = clean_text(text, CHAT_MAX_BYTES)
        if not text:
            return {"ok": False, "error": "Type a message first."}
        if not h.station:
            return {"ok": False, "error": "This station has no number yet."}
        if to < 0 or to > MAX_STATIONS or to == h.station:
            return {"ok": False, "error": "Pick who to send it to."}
        with h.lock:
            ref = h.next_ref
            h.next_ref += 1
            chat = {"id": f"me-{ref}", "ref": ref, "from": h.station, "to": to, "text": text,
                    "time": time.strftime("%H:%M"), "state": "sending", "mine": True}
            h.chats.append(chat)
            h.pending_refs[ref] = chat
        if not self.send(f"chat {to} {ref} {text}"):
            chat["state"] = "failed"
            return {"ok": False, "error": "The radio is not connected."}
        log_chat(f"{who(h.station)} (this laptop) -> {who(to)}: {text}")
        h.publish("chat", chat)
        return {"ok": True, "chat": chat}

    def add_mark(self, kind, lat, lon, label):
        h = self.hub
        if not h.station:
            return {"ok": False, "error": "This station has no number yet."}
        if kind not in (1, 2, 3, 4):
            return {"ok": False, "error": "Unknown kind of mark."}
        if not (-90 <= lat <= 90 and -180 <= lon <= 180):
            return {"ok": False, "error": "That spot is not on the map."}
        with h.lock:
            if len(h.my_marks) >= MAX_OWN_MARKS:
                return {"ok": False, "error": f"Your station already has {MAX_OWN_MARKS} marks. Remove one first."}
            mid = h.next_mark_id
            h.next_mark_id = mid + 1 if mid < 65535 else 1
            h.my_marks[mid] = {"kind": kind, "lat": lat, "lon": lon,
                               "label": clean_text(label, MARK_LABEL_MAX)}
            h.save_marks()
        self._send_mark(mid)
        self.send_marklist()
        h.publish("marks", self.marks_payload())
        return {"ok": True, "id": mid}

    def remove_mark(self, mid):
        h = self.hub
        with h.lock:
            if mid not in h.my_marks:
                return {"ok": False, "error": "You can only remove your own station's marks."}
            del h.my_marks[mid]
            h.save_marks()
        self.send(f"mark remove {mid}")
        self.send_marklist()
        h.publish("marks", self.marks_payload())
        return {"ok": True}

    def _send_mark(self, mid):
        m = self.hub.my_marks.get(mid)
        if m:
            self.send(f"mark add {mid} {m['kind']} {m['lat']:.7f} {m['lon']:.7f} {m['label']}")

    def send_marklist(self):
        ids = ",".join(str(i) for i in sorted(self.hub.my_marks))
        self.send(f"marklist {ids}")

    def refresh_marks(self, force=False):
        """Re-send every one of our marks, then the list of their ids."""
        if not self.hub.station or (not force and now_s() - self.last_refresh < MARK_REFRESH_S):
            return
        self.last_refresh = now_s()
        for mid in sorted(self.hub.my_marks):
            self._send_mark(mid)
            time.sleep(0.05)              # spread them out a little on the radio
        self.send_marklist()


# =====================================================================
# The real radio: USB serial ports
# =====================================================================
class SerialLink:
    """One open USB port. Reads lines in its own thread."""

    def __init__(self, port, on_line, on_close):
        self.port = port
        self.ser = serial.Serial(port, 115200, timeout=0.5)
        self.on_line, self.on_close = on_line, on_close
        self.wlock = threading.Lock()
        threading.Thread(target=self._run, daemon=True).start()

    def send(self, line):
        try:
            with self.wlock:
                self.ser.write((line.replace("\n", " ") + "\n").encode("utf-8"))
            return True
        except (serial.SerialException, OSError):
            return False

    def _run(self):
        time.sleep(1.5)                   # opening the port resets the board
        self.send("info")
        buf = b""
        try:
            while True:
                chunk = self.ser.read(512)
                if not chunk:
                    continue
                buf += chunk
                while b"\n" in buf:
                    raw, buf = buf.split(b"\n", 1)
                    self.on_line(self, raw.decode("utf-8", errors="replace"))
        except (serial.SerialException, OSError):
            pass
        finally:
            try:
                self.ser.close()
            except Exception:
                pass
            self.on_close(self)


class PortWatcher:
    """Opens every USB serial port, waits for each board to say which
    board it is, and connects the station radio to the station logic.
    Other boards (a camera or vehicle plugged in for setup) are listed
    on the Setup page instead."""

    def __init__(self, hub, station, fixed_port=None):
        self.hub, self.station, self.fixed = hub, station, fixed_port
        self.links = {}                   # port -> SerialLink
        self.kind = {}                    # port -> board name from its hello

    def run(self):
        while True:
            try:
                ports = [self.fixed] if self.fixed else [
                    p.device for p in serial.tools.list_ports.comports() if p.vid is not None]
                for port in ports:
                    if port not in self.links:
                        try:
                            self.links[port] = SerialLink(port, self.on_line, self.on_close)
                        except (serial.SerialException, OSError):
                            pass          # busy (Serial Monitor open?) - try again later
            except Exception as e:        # never let the watcher die
                self.hub.note(f"USB check failed: {e}")
            time.sleep(3)

    def on_line(self, link, line):
        msg = parse_board_line(line)
        if msg is None:
            return
        if msg["type"] == "hello":
            self.kind[link.port] = msg.get("board", "")
            with self.hub.lock:
                if msg.get("board") == "station-radio":
                    self.hub.boards.pop(link.port, None)
                else:
                    self.hub.boards[link.port] = msg
            self.hub.publish("boards", self.hub.snapshot()["boards"])
        board = self.kind.get(link.port)
        if board == "station-radio":
            if self.station.link is not link:
                self.station.link = link
                with self.hub.lock:
                    self.hub.radio.update(connected=True, port=link.port)
                self.hub.note(f"Station radio found on {link.port}")
            self.station.handle(msg)
        elif board and msg["type"] in ("saved", "error", "note"):
            self.hub.publish("board_reply", {"port": link.port, **msg})

    def on_close(self, link):
        self.links.pop(link.port, None)
        self.kind.pop(link.port, None)
        with self.hub.lock:
            self.hub.boards.pop(link.port, None)
        if self.station.link is link:
            self.station.link = None
            with self.hub.lock:
                self.hub.radio.update(connected=False, port="")
            self.hub.note("The station radio was unplugged.")
            self.hub.publish("status", self.station.status())
        self.hub.publish("boards", self.hub.snapshot()["boards"])

    def board_send(self, port, lines):
        link = self.links.get(port)
        if not link:
            return False
        for ln in lines:
            link.send(ln)
            time.sleep(0.15)
        return True


# =====================================================================
# Pretend radio for --simulate (no hardware needed)
# =====================================================================
class SimRadio:
    """Prints the same lines a real StationRadio_v5 would, for our own
    station plus some made-up ones driving around the field."""

    FRIENDLY = ["Found something by the goal!", "Heading to the center circle",
                "Watch out for the cone", "Need help over here", "On my way",
                "Yes!", "Great job everyone", "Meet at the corner flag"]

    def __init__(self, hub, station, count, my_station=1):
        self.hub, self.station, self.count = hub, station, max(1, min(count, MAX_STATIONS))
        self.me = my_station
        self.seq = 0
        self.sim_marks = {}               # (station, id) -> mark
        station.link = self

    def emit(self, obj):
        self.station.handle(obj)

    def send(self, line):
        """A command from the dashboard, answered the way the board would."""
        words = line.split(" ", 3)
        cmd = words[0]
        if cmd == "chat" and len(words) >= 4:
            to, ref = int(words[1]), int(words[2])
            if to == 0:
                self.emit({"type": "sent", "ref": ref, "state": "sent"})
            else:
                self.emit({"type": "sent", "ref": ref, "state": "sending"})
                ok = to <= self.count
                threading.Timer(0.6, self.emit, [{"type": "sent", "ref": ref,
                                                  "state": "confirmed" if ok else "failed"}]).start()
        elif cmd == "station" and len(words) > 1:
            self.me = int(words[1])
            self.hello()
        elif cmd == "info":
            self.hello()
        elif cmd == "sync":
            threading.Timer(0.3, self.send_sim_marks).start()
        return True

    def hello(self):
        self.emit({"type": "hello", "board": "station-radio", "fw": "5.0-sim",
                   "station": self.me, "node": self.me, "net": 50, "ch": 1})

    def origin(self):
        s = self.hub.settings
        return s["field_origin_lat"], s["field_origin_lon"], s["field_bearing_deg"]

    def field_to_gps(self, x, y):
        lat0, lon0, brg = self.origin()
        b = math.radians(brg)
        dn = x * math.cos(b) + y * math.sin(b)
        de = x * math.sin(b) - y * math.cos(b)
        return lat0 + dn / 111320, lon0 + de / (111320 * math.cos(math.radians(lat0)))

    def send_sim_marks(self):
        for (st, mid), m in self.sim_marks.items():
            self.emit({"type": "mark", "from": st, "action": "add", "id": mid, **m})
        for st in {k[0] for k in self.sim_marks}:
            self.emit({"type": "marklist", "from": st,
                       "ids": [k[1] for k in self.sim_marks if k[0] == st]})

    def run(self):
        hub = self.hub
        if not hub.settings["field_origin_lat"]:
            # A pretend field so the map has something to show.
            hub.settings.update(field_origin_lat=32.7157, field_origin_lon=-117.1611,
                                field_bearing_deg=0.0)
        self.hello()
        L, W = hub.settings["field_length_m"], hub.settings["field_width_m"]
        lat, lon = self.field_to_gps(L * 0.8, W * 0.3)
        self.sim_marks[(2 if self.count >= 2 else self.me, 1)] = {
            "kind": 2, "lat": lat, "lon": lon, "label": "Puddle"}
        t0 = time.time()
        next_chat = t0 + 8
        while True:
            t = time.time() - t0
            self.seq += 1
            for s in range(1, self.count + 1):
                # Each vehicle drives its own loop around the field.
                a = t / (20 + 4 * s) + s
                x = L / 2 + (L * 0.35) * math.cos(a) * (0.6 + 0.08 * s)
                y = W / 2 + (W * 0.35) * math.sin(a) * (0.6 + 0.08 * s)
                la, lo = self.field_to_gps(x, y)
                heading = (math.degrees(-a) + 360 + hub.settings["field_bearing_deg"]) % 360
                rng = 30 + 25 * math.sin(t / 3 + s)
                self.emit({"type": "telemetry", "station": s, "node": 10 + s, "seq": self.seq,
                           "uptime_ms": int(t * 1000), "sonar_ok": 1, "bmp_ok": 1, "gps_fix": 1,
                           "mag_ok": 1, "distance_in": round(rng, 1), "temp_f": 72.4,
                           "pressure_atm": 1.0021, "bmp_alt_ft": 45.0, "sats": 9,
                           "lat": round(la, 7), "lon": round(lo, 7), "speed_mph": 2.3,
                           "gps_alt_ft": 44.0, "course_deg": round(heading, 1),
                           "heading_deg": round(heading, 1), "rssi": -55 - 3 * s,
                           "relayed": False})
            if self.count > 1 and time.time() > next_chat:
                next_chat = time.time() + random.uniform(15, 30)
                frm = random.choice([s for s in range(1, self.count + 1) if s != self.me])
                self.emit({"type": "chat", "id": f"{frm}-{self.seq}", "from": frm,
                           "to": random.choice([0, 0, self.me]),
                           "text": random.choice(self.FRIENDLY)})
            if self.seq % 30 == 1:
                self.send_sim_marks()
            if self.seq % 5 == 0:
                self.emit({"type": "roster", "nodes": [
                    {"id": n, "role": "base" if n < 10 else "vehicle", "rssi": -60, "age_s": 1}
                    for s in range(1, self.count + 1) for n in (s, 10 + s) if n != self.me]})
            time.sleep(1)


# =====================================================================
# Finding the camera on the WiFi network
# =====================================================================
def local_ipv4_addresses():
    found = set()
    try:
        for info in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET):
            found.add(info[4][0])
    except OSError:
        pass
    try:                                  # the address used to reach the router
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("10.255.255.255", 1))  # no packet is actually sent
        found.add(s.getsockname()[0])
        s.close()
    except OSError:
        pass
    return [ip for ip in found if ipaddress.ip_address(ip).is_private
            and not ip.startswith(("127.", "169.254."))]


def ask_whoami(host, timeout=0.6):
    try:
        with urllib.request.urlopen(f"http://{host}/whoami", timeout=timeout) as r:
            info = json.loads(r.read(200).decode("utf-8", errors="replace"))
            return info if info.get("board") == "camera" else None
    except Exception:
        return None


def find_camera(station):
    """Return the IP of the camera for this station, or None. Tries the
    camera's name first, then asks every address on this laptop's
    network (about 2 seconds)."""
    try:
        ip = socket.gethostbyname(f"ftt-camera-{station}.local")
        info = ask_whoami(ip, 1.0)
        if info and int(info.get("station", 0)) == station:
            return ip
    except OSError:
        pass
    hosts = []
    for ip in local_ipv4_addresses():
        net = ipaddress.ip_network(f"{ip}/24", strict=False)
        hosts += [str(h) for h in net.hosts() if str(h) != ip]
    with concurrent.futures.ThreadPoolExecutor(max_workers=64) as pool:
        for host, info in zip(hosts, pool.map(ask_whoami, hosts)):
            if info and int(info.get("station", 0)) == station:
                return host
    return None


def camera_worker(hub):
    while True:
        try:
            manual = hub.settings.get("camera_url", "").strip()
            if manual:
                with hub.lock:
                    hub.camera.update(url=manual, found_by="typed in Setup", searching=False)
            elif hub.station:
                ip = hub.camera.get("ip")
                if not (ip and ask_whoami(ip, 1.5)):
                    with hub.lock:
                        hub.camera.update(searching=True)
                    hub.publish("camera", dict(hub.camera))
                    ip = find_camera(hub.station)
                    with hub.lock:
                        hub.camera.update(searching=False, ip=ip or "",
                                          url=f"http://{ip}:81/stream" if ip else "",
                                          found_by="found on the WiFi" if ip else "")
                    if ip:
                        hub.note(f"Found the camera for station {hub.station} at {ip}")
            hub.publish("camera", dict(hub.camera))
        except Exception as e:
            hub.note(f"Camera search problem: {e}")
        time.sleep(15)


# =====================================================================
# Web server
# =====================================================================
def make_handler(hub, station, watcher):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):     # keep the console quiet
            pass

        def _send(self, code, body, ctype="application/json"):
            data = body if isinstance(body, bytes) else json.dumps(body).encode("utf-8")
            self.send_response(code)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(data)

        def _body(self):
            n = int(self.headers.get("Content-Length", 0) or 0)
            return self.rfile.read(min(n, 2_000_000)) if n else b""

        def _json(self):
            try:
                v = json.loads(self._body() or b"{}")
                return v if isinstance(v, dict) else {}
            except ValueError:
                return {}

        def do_GET(self):
            path = self.path.split("?")[0]
            if path in ("/", "/index.html"):
                self._send(200, (HERE / "dashboard.html").read_bytes(), "text/html; charset=utf-8")
            elif path == "/state":
                self._send(200, hub.snapshot())
            elif path == "/events":
                self.stream_events()
            else:
                self._send(404, {"error": "unknown page"})

        def do_POST(self):
            path = self.path.split("?")[0]
            if path == "/chat":
                b = self._json()
                try:
                    to = int(b.get("to", 0))
                except (TypeError, ValueError):
                    to = -1
                self._send(200, station.send_chat(to, b.get("text", "")))
            elif path == "/mark":
                b = self._json()
                try:
                    if b.get("action") == "remove":
                        self._send(200, station.remove_mark(int(b["id"])))
                    else:
                        self._send(200, station.add_mark(int(b["kind"]), float(b["lat"]),
                                                         float(b["lon"]), b.get("label", "")))
                except (KeyError, TypeError, ValueError):
                    self._send(400, {"ok": False, "error": "bad mark"})
            elif path == "/settings":
                b = self._json()
                with hub.lock:
                    hub.settings.update({k: b[k] for k in DEFAULT_SETTINGS if k in b})
                    if "camera_url" in b:
                        hub.camera.update(ip="", url="")
                    save_settings(hub.settings)
                hub.publish("settings", dict(hub.settings))
                self._send(200, {"ok": True, "settings": hub.settings})
            elif path == "/station":
                n = int(self._json().get("station", 0) or 0)
                ok = 1 <= n <= MAX_STATIONS and station.send(f"station {n}")
                self._send(200, {"ok": bool(ok)})
            elif path == "/identify":
                self._send(200, {"ok": station.send("identify")})
            elif path == "/board":
                b = self._json()
                lines = [clean_text(x, 180) for x in b.get("lines", [])][:6]
                ok = watcher.board_send(b.get("port", ""), lines) if watcher else False
                self._send(200, {"ok": ok})
            elif path == "/table":
                name = self.headers.get("X-Filename", "file.csv")
                try:
                    self._send(200, {"ok": True, "rows": read_table_file(name, self._body())})
                except Exception as e:
                    self._send(200, {"ok": False, "error": f"Could not read that file ({e})"})
            else:
                self._send(404, {"error": "unknown page"})

        def stream_events(self):
            """Server-Sent Events: each line is 'event: <type>' + 'data: <json>'."""
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            q = queue.Queue(maxsize=500)
            with hub.lock:
                hub.listeners.append(q)
            try:
                while True:
                    try:
                        event, data = q.get(timeout=2)
                        self.wfile.write(f"event: {event}\ndata: {json.dumps(data)}\n\n".encode("utf-8"))
                    except queue.Empty:
                        # Every 2 s: the station's status (also notices closed tabs).
                        self.wfile.write(f"event: status\ndata: {json.dumps(station.status())}\n\n".encode())
                    self.wfile.flush()
            except (BrokenPipeError, ConnectionResetError, OSError):
                pass
            finally:
                with hub.lock:
                    if q in hub.listeners:
                        hub.listeners.remove(q)
    return Handler


def housekeeping(station):
    while True:
        try:
            station.refresh_marks()
        except Exception as e:
            station.hub.note(f"Mark refresh problem: {e}")
        time.sleep(1)


def main():
    ap = argparse.ArgumentParser(description="Find the Target v5 station dashboard")
    ap.add_argument("--simulate", type=int, metavar="N", default=0,
                    help="no hardware: pretend there are N stations")
    ap.add_argument("--as-station", type=int, default=1,
                    help="with --simulate: which station this laptop pretends to be")
    ap.add_argument("--port", help="use this USB port instead of searching (e.g. COM5)")
    ap.add_argument("--http", type=int, default=8000, help="web page port (default 8000)")
    ap.add_argument("--open", action="store_true", help="open the dashboard in the web browser")
    args = ap.parse_args()

    if args.simulate:
        # Practice mode keeps its own files, so it never mixes pretend
        # marks and chats into this station's real ones.
        global DATA, SETTINGS_FILE, MARKS_FILE, CHAT_LOG_DIR
        DATA = HERE / "data-practice"
        SETTINGS_FILE, MARKS_FILE, CHAT_LOG_DIR = DATA / "settings.json", DATA / "marks.json", DATA / "chat-log"
    hub = Hub(simulate=args.simulate > 0)
    station = Station(hub)
    watcher = None
    if args.simulate:
        sim = SimRadio(hub, station, args.simulate, args.as_station)
        threading.Thread(target=sim.run, daemon=True).start()
    else:
        if serial is None:
            raise SystemExit("pyserial is not installed. Run:  python -m pip install pyserial\n"
                             "(or double-click Start Dashboard.bat, which does it for you)")
        watcher = PortWatcher(hub, station, args.port)
        threading.Thread(target=watcher.run, daemon=True).start()
        threading.Thread(target=camera_worker, args=(hub,), daemon=True).start()
    threading.Thread(target=housekeeping, args=(station,), daemon=True).start()

    # 127.0.0.1 = only this laptop can open the page. That also means
    # Windows does not pop up a firewall question.
    server = ThreadingHTTPServer(("127.0.0.1", args.http), make_handler(hub, station, watcher))
    server.daemon_threads = True
    print("==============================================")
    print(" Find the Target v5 - station dashboard")
    if args.simulate:
        print(f" SIMULATION: {args.simulate} pretend station(s), no hardware")
    print(f" Open:  http://localhost:{args.http}")
    print(" Leave this window open. Close it to stop.")
    print("==============================================")
    if args.open:
        import webbrowser
        threading.Timer(1.0, webbrowser.open, [f"http://localhost:{args.http}"]).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")


if __name__ == "__main__":
    main()
