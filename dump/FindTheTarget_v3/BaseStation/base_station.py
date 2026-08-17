#!/usr/bin/env python3
"""
base_station.py -- Find the Target base-station server
=============================================================

WHAT THIS PROGRAM DOES
  1. Opens the serial port of the ESPNow_Receiver_v3 board (USB).
  2. Reads its output: "# ..." status lines are echoed to this console,
     "{...}" JSON lines are parsed as telemetry.
  3. Serves the dashboard web page (dashboard.html) and pushes every
     telemetry packet to the browser in real time (Server-Sent Events).

REQUIREMENTS
  Python 3.8+ and one package:
      pip install pyserial

HOW TO RUN
  1. Plug in the receiver ESP32 and close the Arduino serial monitor
     (only one program can hold the port at a time).
  2. From this folder run:
         python base_station.py
     The receiver port is auto-detected; to pick one explicitly:
         python base_station.py --port COM5            (Windows)
         python base_station.py --port /dev/ttyUSB0    (Linux/Mac)
  3. Open the printed address (default http://localhost:8000) in a
     browser on this computer.

HOW TO TELL IT IS WORKING
  - "Serial: connected on <port>" appears at startup.
  - Every receiver status line ("# pkt ...") scrolls in this console.
  - The dashboard's LINK pill turns green when packets flow.

ENDPOINTS (used by dashboard.html; handy for debugging with a browser)
  /            the dashboard page
  /latest      most recent telemetry packet as JSON
  /events      live telemetry stream (Server-Sent Events)
  /config      GET/POST persistent settings (camera URL, field calibration)
"""

import argparse
import json
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    raise SystemExit("pyserial is not installed. Run:  pip install pyserial")

HERE = Path(__file__).resolve().parent
CONFIG_FILE = HERE / "config.json"

# Settings the dashboard can edit; saved to config.json so they survive
# a restart. field_* values calibrate GPS -> soccer-field coordinates.
DEFAULT_CONFIG = {
    "camera_url": "",          # e.g. http://192.168.1.42:81/stream
    "field_origin_lat": 0.0,   # GPS latitude of the origin corner (map's bottom-left)
    "field_origin_lon": 0.0,   # GPS longitude of the origin corner
    "field_bearing_deg": 0.0,  # compass bearing along the field LENGTH
    "field_length_m": 100.0,   # goal-line to goal-line
    "field_width_m": 64.0,     # sideline to sideline
    "dest_x_ft": None,         # optional destination target, feet from the
    "dest_y_ft": None,         #   origin corner (map x/y; None = not set)
}

# ---------------------------------------------------------------------
# Shared state between the serial thread and the web server
# ---------------------------------------------------------------------
class State:
    def __init__(self):
        self.lock = threading.Lock()
        self.latest = None            # last telemetry dict
        self.latest_at = 0.0          # time.time() it arrived
        self.serial_ok = False
        self.serial_port_name = ""
        self.listeners = []           # one Event + queue slot per browser

    def publish(self, packet):
        with self.lock:
            self.latest = packet
            self.latest_at = time.time()
            for slot in self.listeners:
                slot["packet"] = packet
                slot["event"].set()

    def snapshot(self):
        with self.lock:
            return {
                "packet": self.latest,
                "age_s": round(time.time() - self.latest_at, 1) if self.latest else None,
                "serial_ok": self.serial_ok,
                "serial_port": self.serial_port_name,
            }

STATE = State()

# ---------------------------------------------------------------------
# Config persistence
# ---------------------------------------------------------------------
def load_config():
    cfg = dict(DEFAULT_CONFIG)
    if CONFIG_FILE.exists():
        try:
            cfg.update(json.loads(CONFIG_FILE.read_text()))
        except (json.JSONDecodeError, OSError) as e:
            print(f"Config: could not read {CONFIG_FILE.name} ({e}); using defaults")
    return cfg

def save_config(cfg):
    CONFIG_FILE.write_text(json.dumps(cfg, indent=2))

# ---------------------------------------------------------------------
# Serial reader thread
# ---------------------------------------------------------------------
def find_receiver_port():
    """Best-guess the receiver's port from typical USB-UART chip names."""
    hints = ("CP210", "CH340", "CH910", "FTDI", "USB Serial", "usbserial", "SLAB")
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        text = f"{p.description} {p.manufacturer or ''}"
        if any(h.lower() in text.lower() for h in hints):
            return p.device
    return ports[0].device if ports else None

def serial_worker(port, baud):
    """Read receiver lines forever; reconnect automatically on unplug."""
    while True:
        name = port or find_receiver_port()
        if not name:
            STATE.serial_ok = False
            print("Serial: no port found - is the receiver plugged in? Retrying in 3 s")
            time.sleep(3)
            continue
        try:
            with serial.Serial(name, baud, timeout=2) as ser:
                STATE.serial_ok = True
                STATE.serial_port_name = name
                print(f"Serial: connected on {name} @ {baud}")
                while True:
                    raw = ser.readline()
                    if not raw:
                        continue                       # timeout, keep waiting
                    line = raw.decode("utf-8", errors="replace").strip()
                    if not line:
                        continue
                    if line.startswith("{"):
                        # Machine-readable telemetry from the receiver.
                        try:
                            STATE.publish(json.loads(line))
                        except json.JSONDecodeError:
                            print(f"Serial: bad JSON skipped: {line[:80]}")
                    else:
                        # Human-readable status ("# ..."): echo for debugging.
                        print(f"Receiver: {line}")
        except (serial.SerialException, OSError) as e:
            STATE.serial_ok = False
            print(f"Serial: lost {name} ({e}) - retrying in 3 s")
            time.sleep(3)

# ---------------------------------------------------------------------
# Web server
# ---------------------------------------------------------------------
class Handler(BaseHTTPRequestHandler):
    def log_message(self, *args):     # silence per-request noise
        pass

    def _send(self, code, body, ctype="application/json"):
        data = body if isinstance(body, bytes) else json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path in ("/", "/index.html", "/dashboard.html"):
            page = HERE / "dashboard.html"
            if page.exists():
                self._send(200, page.read_bytes(), "text/html; charset=utf-8")
            else:
                self._send(404, {"error": "dashboard.html not found next to base_station.py"})
        elif self.path == "/latest":
            self._send(200, STATE.snapshot())
        elif self.path == "/config":
            self._send(200, load_config())
        elif self.path == "/events":
            self.stream_events()
        else:
            self._send(404, {"error": "unknown path"})

    def do_POST(self):
        if self.path != "/config":
            self._send(404, {"error": "unknown path"})
            return
        try:
            length = int(self.headers.get("Content-Length", 0))
            incoming = json.loads(self.rfile.read(length) or b"{}")
        except (ValueError, json.JSONDecodeError):
            self._send(400, {"error": "invalid JSON"})
            return
        cfg = load_config()
        # Only accept known keys so a bad client can't grow the file.
        cfg.update({k: incoming[k] for k in DEFAULT_CONFIG if k in incoming})
        save_config(cfg)
        print(f"Config: saved ({CONFIG_FILE.name})")
        self._send(200, cfg)

    def stream_events(self):
        """Server-Sent Events: push each new packet to this browser."""
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-store")
        self.end_headers()

        slot = {"event": threading.Event(), "packet": None}
        with STATE.lock:
            STATE.listeners.append(slot)
            if STATE.latest:                       # send last known state now
                slot["packet"] = STATE.latest
                slot["event"].set()
        try:
            while True:
                # Wake on a new packet, or every 5 s to send a keepalive
                # (also how we notice the browser tab was closed).
                if slot["event"].wait(timeout=5):
                    slot["event"].clear()
                    payload = json.dumps(slot["packet"])
                    self.wfile.write(f"data: {payload}\n\n".encode())
                else:
                    self.wfile.write(b": keepalive\n\n")
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, OSError):
            pass                                    # browser went away
        finally:
            with STATE.lock:
                if slot in STATE.listeners:
                    STATE.listeners.remove(slot)

# ---------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description="Find the Target base-station server")
    ap.add_argument("--port", help="receiver serial port (default: auto-detect)")
    ap.add_argument("--baud", type=int, default=115200, help="serial baud (default 115200)")
    ap.add_argument("--http", type=int, default=8000, help="web server port (default 8000)")
    args = ap.parse_args()

    threading.Thread(target=serial_worker, args=(args.port, args.baud),
                     daemon=True).start()

    server = ThreadingHTTPServer(("0.0.0.0", args.http), Handler)
    print("=======================================")
    print(" Find the Target base station")
    print(f" Dashboard:  http://localhost:{args.http}")
    print(" Stop with Ctrl+C")
    print("=======================================")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")

if __name__ == "__main__":
    main()
