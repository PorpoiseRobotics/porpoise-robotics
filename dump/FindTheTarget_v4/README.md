# Find the Target — Vehicle Telemetry System (v4)

*A Porpoise Robotics STEM project.*

Three ESP32 programs plus a base-station dashboard that shows the live
camera feed and the vehicle's position on a soccer-field map, side by side.

## What's new in v4

The GT-U7 GPS is replaced by an **FK-A1 module** (u-blox M10 GPS with a
**QMC5883L compass** on the same board). In practice:

- The GPS serial link now runs at **38400 baud** (the FK-A1 default;
  the GT-U7 used 9600). Position accuracy and time-to-fix improve too.
- The compass rides the same I2C bus as the BMP280, so the dashboard's
  **Heading** number and map arrow now show which way the vehicle is
  *facing* — even standing still. (GPS course only worked while moving.)
- The telemetry packet is now **version 4** (adds `heading_deg`), so a
  v3 sender and a v4 receiver will not talk to each other — upgrade both.
- Handy hardware check: the FK-A1's blue **PPS LED** is solid at power-up
  and starts *flashing* once the GPS has a 3D fix.

## How the pieces connect

```
 VEHICLE                                   BASE STATION COMPUTER
 ┌─────────────────────┐   ESP-NOW    ┌──────────────────────┐
 │ Sender ESP32        │ ───────────► │ Receiver ESP32 (USB) │
 │ ESPNow_Sender_v4    │   channel 1  │ ESPNow_Receiver_v4   │
 │ HC-SR04/BMP280/     │              └──────────┬───────────┘
 │ FK-A1 GPS+compass   │                         │
 │ 16 NeoPixels        │                  serial │ JSON lines
 └─────────────────────┘                         ▼
 ┌─────────────────────┐    WiFi      ┌──────────────────────┐
 │ Freenove camera     │ ───────────► │ base_station.py      │
 │ CameraWebServer_v4  │ MJPEG :81    │  → dashboard.html    │
 └─────────────────────┘              │    (browser, :8000)  │
                                      └──────────────────────┘
```

## Folder guide

| Folder | Board / runs on | IDE board selection |
|---|---|---|
| `ESPNow_Sender_v4/` | Vehicle ESP32 (sensors + LEDs) | ESP32 Dev Module |
| `ESPNow_Receiver_v4/` | ESP32 on the computer's USB | ESP32 Dev Module |
| `CameraWebServer_v4/` | Freenove ESP32-WROVER camera | ESP32 Wrover Module, Partition Scheme: **Huge APP (3MB)** |
| `BaseStation/` | The computer (Python 3 + browser) | — |

All serial monitors run at **115200 baud**. Arduino libraries needed
(sender only): Adafruit BMP280, Adafruit NeoPixel, TinyGPSPlus — the
compass is driven with plain I2C, no library. The camera folder already
contains the stock Espressif support files (`app_httpd.cpp`,
`camera_index.h`, `camera_pins.h`).

## FK-A1 wiring (sender board)

The module's 6-pin connector, left to right: `GND  5V  Rx  Tx  SCL  SDA`.

| FK-A1 pin | ESP32 pin | Why |
|---|---|---|
| GND | GND | |
| 5V | 5V | module runs on 3.6–5.5 V |
| Rx | GPIO18 | module *receives*, so it hears the ESP32's TX |
| Tx | GPIO16 | module *transmits*, so it feeds the ESP32's RX |
| SCL | GPIO22 | compass clock — shared with the BMP280 |
| SDA | GPIO21 | compass data — shared with the BMP280 |

The UART is 3.3 V TTL: no voltage divider needed (unlike the HC-SR04).
After mounting, set `HEADING_OFFSET_DEG` in the sender sketch so the
dashboard heading matches a phone compass.

## First-time bring-up (in this order)

1. **Receiver** — flash `ESPNow_Receiver_v4`, open the serial monitor,
   copy the MAC address it prints.
2. **Sender** — paste that MAC into `PEER_MAC` in
   `ESPNow_Sender_v4.ino`, flash, and watch the SELF-TEST report. Each
   sensor prints PASS/FAIL with a wiring hint. Each transmission then
   prints `DELIVERED` or `MISSED`.
3. **Camera** — add your WiFi network(s) to the `NETWORKS[]` list in
   `CameraWebServer_v4.ino`, flash, and copy the **Video stream** URL it
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
- **[FK-A1] FAIL in self-test** — module powered (5V pin)? FK-A1 **Tx**
  to GPIO**16**? A module reconfigured away from 38400 baud also looks
  dead — check `GPS_BAUD` in the sender sketch.
- **Sats stay 0 / PPS LED never flashes** — GPS needs open sky; first
  fix outdoors takes ~30 s from cold. It will not fix indoors.
- **[COMPASS] FAIL in self-test** — FK-A1 SCL→GPIO22, SDA→GPIO21? If the
  BMP280 passes on the same bus, the wiring from the ESP32 is fine and
  the problem is between the splice and the FK-A1.
- **Heading is consistently off by a fixed amount** — set
  `HEADING_OFFSET_DEG` in the sender sketch (mounting rotation); keep
  the module away from the motors and power wires, which bend the
  magnetic field the compass measures.
- **Camera won't join WiFi** — it prints a scan of visible networks so
  you can spot SSID typos; it only supports 2.4 GHz.
- **Camera image but no dashboard video** — the computer must be on the
  same network as the camera; test the stream URL directly in a browser
  tab.
- **Vehicle dot missing** — needs a GPS fix (GPS pill green) *and* a
  non-zero origin latitude in Field setup.
