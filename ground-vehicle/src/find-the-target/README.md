# Find the Target — Vehicle Telemetry System

*A Porpoise Robotics STEM project.*

There are **two versions of this mission in this folder**, and they solve
different problems.

| | **v3** (`ESPNow_Sender_v3` + `ESPNow_Receiver_v3`) | **PorpoiseNet** (`PorpoiseNet_Vehicle` + `PorpoiseNet_Base`) |
|---|---|---|
| Direction | one-way: vehicle → base | **two-way: everyone ↔ everyone** |
| Vehicles | one | as many as you like |
| Rover-to-rover | no | **yes — a find is announced to the whole fleet** |
| Base can send commands | no | **yes — start, stop, recall, ping, identify** |
| Setup per board | paste the receiver's MAC into the sender | change one number |
| Confirmed delivery | no | **yes — a find is resent until the base confirms** |
| Range beyond one hop | no | **yes — rovers relay for each other** |
| Camera | separate WiFi stream | also a node; can photograph a find |
| Dashboard | works | works — the JSON output is unchanged |

**Use v3** if you have one vehicle and want the existing dashboard, unchanged.
**Use PorpoiseNet** for the actual game, where several vehicles hunt at once and
finding the target has to tell everybody. It runs on
[`shared/PorpoiseNet/`](../../../shared/PorpoiseNet/) — read that README first;
it explains how the boards find each other and the three settings that must
match.

The rest of this document describes the **v3** system: three ESP32 programs plus
a base-station dashboard that shows the live camera feed and the vehicle's
position on a soccer-field map, side by side.

## How the pieces connect

```
 VEHICLE                                   BASE STATION COMPUTER
 ┌─────────────────────┐   ESP-NOW    ┌──────────────────────┐
 │ Sender ESP32        │ ───────────► │ Receiver ESP32 (USB) │
 │ ESPNow_Sender_v3    │   channel 1  │ ESPNow_Receiver_v3   │
 │ HC-SR04/BMP280/GPS  │              └──────────┬───────────┘
 │ 16 NeoPixels        │                  serial │ JSON lines
 └─────────────────────┘                         ▼
 ┌─────────────────────┐    WiFi      ┌──────────────────────┐
 │ Freenove camera     │ ───────────► │ base_station.py      │
 │ CameraWebServer_v3  │ MJPEG :81    │  → dashboard.html    │
 └─────────────────────┘              │    (browser, :8000)  │
                                      └──────────────────────┘
```

## Folder guide

| Folder | Board / runs on | IDE board selection |
|---|---|---|
| `ESPNow_Sender_v3/` | Vehicle ESP32 (sensors + LEDs) | ESP32 Dev Module |
| `ESPNow_Receiver_v3/` | ESP32 on the computer's USB | ESP32 Dev Module |
| `CameraWebServer_v3/` | Freenove ESP32-WROVER camera | ESP32 Wrover Module, Partition Scheme: **Huge APP (3MB)** |
| `BaseStation/` | The computer (Python 3 + browser) | — |
| `PorpoiseNet_Vehicle/` | *(two-way version)* each rover | ESP32 Dev Module |
| `PorpoiseNet_Base/` | *(two-way version)* ESP32 on the computer's USB | ESP32 Dev Module |

All serial monitors run at **115200 baud** (up from 9600 in v2).
Arduino libraries needed (sender only): Adafruit BMP280, Adafruit
NeoPixel, TinyGPSPlus. The camera folder already contains the stock
Espressif support files (`app_httpd.cpp`, `camera_index.h`, `camera_pins.h`).

## First-time bring-up (in this order)

1. **Receiver** — flash `ESPNow_Receiver_v3`, open the serial monitor,
   copy the MAC address it prints.
2. **Sender** — paste that MAC into `PEER_MAC` in
   `ESPNow_Sender_v3.ino`, flash, and watch the SELF-TEST report. Each
   sensor prints PASS/FAIL with a wiring hint. Each transmission then
   prints `DELIVERED` or `MISSED`.
3. **Camera** — add your WiFi network(s) to the `NETWORKS[]` list in
   `CameraWebServer_v3.ino`, flash, and copy the **Video stream** URL it
   prints (`http://<ip>:81/stream`).
4. **Base station** — close the receiver's Arduino serial monitor
   (only one program can use the port), then:
   ```
   pip install pyserial
   cd BaseStation
   python base_station.py
   ```
   Open http://localhost:8000, paste the camera stream URL into Setup,
   and enter the field calibration.

## Field calibration (GPS → map)

The map is a traditional graph: the **origin (0,0) is the bottom-left
corner** of the drawn pitch, **x** runs right along the long side, and
**y** runs up across the width (small axes are drawn at the corner as a
reminder).

Stand at the corner of the field you want as the origin, facing down the
**long** side with the field on your **left**. Enter:

- `origin lat/lon` — GPS position of that corner (park the vehicle there
  and read lat/lon off the dashboard, or use a phone),
- `bearing` — the compass direction you are facing,
- `length / width` — field size in meters.

The vehicle then appears at the correct spot on the drawn pitch, with a
heading arrow and a breadcrumb trail.

## Destination target

In Setup → Destination, enter an x/y point in feet from the origin
corner (the same graph coordinates as x/y waypoints). A gold crosshair
marks it on the map, and two telemetry cards — **Dest ΔX** and
**Dest ΔY** — show how many feet remain along each axis. The numbers
count down toward 0 as the vehicle closes in (signed: + means the
destination is further right / up), and both turn green once the
vehicle is within 10 ft on each axis. **Clear** removes the target.

## Waypoints from Excel

Load a `.xlsx` (or `.csv`) in the Waypoints card. Format — first sheet,
headers in row 1, one waypoint per row. Two column layouts are accepted:

- `name | x_ft | y_ft` — **feet from the origin corner**: x along the
  field length, y up across the width, exactly like plotting points on
  a graph. (Plain `x | y` headers also work; values are read as feet.)
  Template: `BaseStation/waypoints_example_xy.xlsx`.
- `name | lat | lon` — GPS degrees.
  Template: `BaseStation/waypoints_example.xlsx`.

Optionally add a second sheet named `field` with key/value rows
(`origin_lat`, `origin_lon`, `bearing_deg`, `length_m`, `width_m`) to
auto-fill the calibration; the GPS template shows this. The x/y template
has no `field` sheet, so loading it never overwrites your calibration.

## Troubleshooting quick hits

- **Sender prints MISSED** — receiver powered? Both on `WIFI_CHANNEL 1`?
  Correct `PEER_MAC`?
- **Receiver silent** — it prints a warning after 5 s of no packets;
  check the sender first.
- **`Serial: no port found`** — receiver unplugged, or the Arduino
  serial monitor still has the port open.
- **Camera won't join WiFi** — it prints a scan of visible networks so
  you can spot SSID typos; it only supports 2.4 GHz.
- **Camera image but no dashboard video** — the computer must be on the
  same network as the camera; test the stream URL directly in a browser
  tab.
- **Vehicle dot missing** — needs a GPS fix (GPS pill green) *and* a
  non-zero origin latitude in Field setup.


---

# The two-way version — running the game

This section covers `PorpoiseNet_Vehicle/` and `PorpoiseNet_Base/`. For how the
network itself works, read
[`shared/PorpoiseNet/README.md`](../../../shared/PorpoiseNet/README.md) first.

## What happens when a rover finds the target

1. A student presses the FIND button on rover 11 (or its sonar sees something
   closer than 8 inches).
2. Rover 11 broadcasts a target report. **Every rover in range hears it at the
   same instant** — it is not sent to the base for the base to pass on.
3. Rovers 12 and 13 print who found it, where, and how sure they were, and
   flash purple so their drivers see it without looking at a screen.
4. The base station prints the find and sends an acknowledgement back.
5. Rover 11 keeps resending until that acknowledgement arrives, then prints
   `ACK ... confirmed received`. A find that nobody heard is the same as no
   find, so it is not allowed to fail quietly.
6. If a camera node is on the network, it photographs the find and broadcasts
   what it saved.

If the base is too far away, a rover in the middle of the field relays for the
one at the far end, with no configuration at all.

## Setting up a fleet

Every rover runs the identical sketch. The only edit per board:

```cpp
#define MY_NODE_ID   11   // 11, 12, 13, ... one per rover, all different
```

`MY_NET_ID` and `MY_CHANNEL` must be **the same** on every board including the
base. If two classes are running on one field at the same time, give each class
a different `MY_NET_ID` and they will not hear each other.

Write the node number on the board with a marker.

## Wiring a rover

Only the button is required. Everything else is optional and switched off by
default, so the sketch compiles on a fresh Arduino install with no libraries
added.

| Part | Pin | Notes |
|---|---|---|
| FIND button | GPIO 4 to GND | no resistor — the internal pull-up is used |
| HC-SR04 sonar | TRIG 32, ECHO 25 | **ECHO must go through a voltage divider** — it is 5 V and the pin is 3.3 V. 1k from ECHO to the pin, 2k from the pin to GND. |
| NeoPixels | GPIO 15 | set `USE_NEOPIXEL 1`; needs the Adafruit NeoPixel library |
| GPS GT-U7 | GPS TX → GPIO 16 | set `USE_GPS 1`; needs TinyGPSPlus |

Without GPS a find still gets announced — it just carries no position, and the
dashboard plots nothing for it. The announcement is the part that matters.

## Running the base station

The base station joins your WiFi network, so you can see the fleet from a phone
or a browser without a laptop plugged in. Before flashing it:

1. Copy `PorpoiseNet_Base/secrets.example.h` to `PorpoiseNet_Base/secrets.h`
2. Put your network name and password in `secrets.h` — **not** in the `.ino`.
   `secrets.h` is gitignored so it can never be committed; this repository is
   public and has been burned by a committed password before.
3. The network must be **2.4 GHz**. An ESP32 cannot see 5 GHz at all, and a
   5 GHz-only network looks exactly like a wrong password from the board.

Set `USE_WIFI 0` at the top of the sketch to go back to a USB-only base station.

### The channel consequence — read this once, properly

Joining a router means **the router decides the base station's radio channel**,
and every other board has to be on that same channel or the fleet cannot hear
the base at all. Nothing prints an error; the roster is just empty.

The sketch prints the channel it ended up on at boot, in capitals, and prints a
much louder warning if it disagrees with `MY_CHANNEL`. Either set `MY_CHANNEL`
to that number on every other board, or — better — lock the router's 2.4 GHz
channel to a fixed 1 and leave `MY_CHANNEL 1` everywhere. Do not leave the
router on "Auto": it will move channel by itself, usually overnight, and the
fleet that worked yesterday will not work today.

If WiFi fails to connect, the sketch falls back to `MY_CHANNEL` and carries on
over USB. A typo'd password does not take the game down.

### The status page

Once connected, the serial output prints an address. Open it in a browser:

- `http://<base-ip>/` — a live page showing the roster, signal strength per
  node, how long since each was heard, and the last find and alert. It refreshes
  itself every 2 seconds.
- `http://<base-ip>/status.json` — the same data as JSON, for `base_station.py`
  or anything else to poll.

The USB serial output is unchanged and still works exactly as before. The WiFi
is an addition, not a replacement.

### Commands

Open the serial monitor at **115200 with line ending "Newline"**, and type:

| Type | Effect |
|---|---|
| `start` | every rover: begin the hunt |
| `stop` | every rover: stop reporting finds |
| `recall` | every rover: come back to the start line |
| `reset` | clear the found-target state, start a new round |
| `ping` | every node answers — the fastest possible network test |
| `ping 12` | just node 12 answers |
| `id 12` | node 12 flashes, so you can tell which board it is |
| `roster` | list every node heard recently |
| `wifi` | show the base station's IP address and radio channel |
| `snapshot` | any camera on the network takes a picture now |
| `help` | the list |

The JSON telemetry lines are unchanged from v3, so `base_station.py` and
`dashboard.html` work with this sketch without modification.

**One program at a time can hold the USB port.** Close the Arduino serial
monitor before starting `base_station.py`, or Python reports "no port found"
while looking straight at the board.

## Trying it with no vehicle at all

Two ESP32 boards on a desk are enough to see the whole thing work: flash one as
the base and one as a vehicle, type `ping` at the base, then short GPIO 4 to GND
on the vehicle with a piece of wire. That is a complete two-way exchange, and it
takes about ten minutes including installing the library.
