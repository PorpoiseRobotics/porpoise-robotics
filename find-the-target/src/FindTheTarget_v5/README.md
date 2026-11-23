# Find the Target v5 — several stations, one field

*A Porpoise Robotics STEM project.*

Up to nine **stations** play on the same field. A station is one laptop and one vehicle. Each
laptop shows its vehicle's live video, a map with every vehicle on it, and the sensor numbers.
Stations can **chat** with each other and **drop marks on the map** (target, danger, waypoint,
meet here) that every other station sees.

> **Try it first with no hardware at all:** open the `Dashboard` folder and double-click
> **`Practice Mode (no hardware).bat`**. Four pretend stations drive around a pretend field.

---

## Contents

1. [What you need](#1-what-you-need)
2. [One-time setup (a grown-up, about an hour)](#2-one-time-setup)
3. [Game day: switching everything on](#3-game-day)
4. [Using the dashboard](#4-using-the-dashboard)
5. [When something does not work](#5-when-something-does-not-work)
6. [How it works (for the curious)](#6-how-it-works)
7. [Field test checklist](#7-field-test-checklist)

---

## 1. What you need

**For each station**

| Thing | Notes |
|---|---|
| A laptop (Windows) | Runs the dashboard. Needs Python 3 (free). |
| 1 ESP32 board with a USB cable | The **station radio**. Stays plugged into the laptop. Write the station number on it. |
| The vehicle, with: | |
| &nbsp;&nbsp;Drive board (ESP32) | Runs Op Program 13 from [`ground-vehicle/`](../../../ground-vehicle/README.md). Nothing changes there. |
| &nbsp;&nbsp;Sensor board (ESP32 on the [Find the Target sensor PCB](../../hardware/README.md)) | FK-A1 GPS + compass, HC-SR04 range finder, BMP280, 16-LED NeoPixel ring |
| &nbsp;&nbsp;Freenove ESP32-WROVER camera | Video over WiFi |

**Once, for the whole field**

- A small WiFi router (it does **not** need internet). The cameras and the laptops join it.
- The team's WiFi list: OneDrive folder **Porpoise Robotics Private → WiFi networks.txt**.

**How many stations?** Nine at most. See [what breaks above that](#limits).

---

## 2. One-time setup

Do this once. After that, game day is just switching things on.

### 2a. The laptop where you upload programs

1. Install the **Arduino IDE** (arduino.cc).
2. **Tools → Board → Boards Manager**: install **esp32 by Espressif Systems**, version **3.3.12**.
3. **Tools → Manage Libraries**: install **Adafruit BMP280**, **Adafruit NeoPixel**,
   **TinyGPSPlus**. (PorpoiseNet does not need installing: a copy sits in each sketch folder.)

### 2b. Upload the three programs

Every station uploads **exactly the same files**. Nothing needs editing.

| Board | Open this sketch | Tools → Board | Also set |
|---|---|---|---|
| Station radio | `StationRadio_v5/StationRadio_v5.ino` | ESP32 Dev Module | — |
| Vehicle sensor board | `VehicleSensors_v5/VehicleSensors_v5.ino` | ESP32 Dev Module | — |
| Vehicle camera | `VehicleCamera_v5/VehicleCamera_v5.ino` | ESP32 Wrover Module | Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)** |

### 2c. Give every board its station number

Each board saves its number in its own memory, so you do this once per board.

- **Station radio:** start the dashboard (step 2e) with the radio plugged in. It asks
  "Which station is this?" — press the number.
- **Vehicle sensor board:** plug it into the laptop, start the dashboard, open **⚙ Setup →
  Boards plugged into this laptop**, choose the number, press **Save to this board**.
  (Or, in the Arduino Serial Monitor at 115200 baud, type `station 3` and press Enter.)
- **Camera:** next step.

Check: when the sensor board powers up, its LED ring lights *N* LEDs in the station's color
(1 red, 2 blue, 3 green, 4 yellow, 5 purple, 6 orange, 7 cyan, 8 pink, 9 white). Blinking
orange means it has no number yet.

### 2d. Tell each camera its station number and the WiFi

Pick the easiest way. The password is in **WiFi networks.txt** in the private OneDrive folder.

- **Phone or laptop:** power the camera. It makes its own WiFi network called
  **`FTT-Camera-Setup`**. Join it; a setup page opens (if not, go to `http://192.168.4.1`).
  Choose the station number, pick the router's network, type the password, press **Save**.
- **Dashboard:** plug the camera into the laptop's USB, open **⚙ Setup → Boards plugged into
  this laptop**, fill in station number, WiFi name and password, press **Save to this board**.
  The password goes straight to the camera; the laptop does not keep it.
- **Serial Monitor:** type `station 3`, then `wifiname <network name>`, then
  `wifipass <password>`, then `wifisave`.

A camera remembers up to three networks and joins whichever it can see. If it cannot join any
of them for 30 seconds, it opens `FTT-Camera-3-Setup` again for three minutes so you can fix it.

### 2e. Each station's laptop

1. Install **Python 3** from python.org. On the first installer screen tick
   **"Add python.exe to PATH"**.
2. Copy this whole `FindTheTarget_v5` folder onto the laptop (or clone the repository).
3. With **internet** available, double-click **`Dashboard/Start Dashboard.bat`** once. It
   installs `pyserial` (the only extra piece) and opens the dashboard.
   After that, it works with no internet at all.

### 2f. The field router

- Give it a name and password and write them in **WiFi networks.txt** (OneDrive).
- In its settings, set the **2.4 GHz channel to 6 or 11** (not "Auto"). The radio network uses
  channel 1; keeping them apart helps both.
- If it is dual-band, put the laptops on its 5 GHz network: the cameras can only use 2.4 GHz,
  and that leaves 2.4 GHz to them.

### 2g. The field map (each laptop)

Open **⚙ Setup → Field**. Park a vehicle at the corner that should be the map's (0,0), facing
along the long side with the field on its left. Press **Use my vehicle's spot**, then
**Use my vehicle's facing**, enter the field's length and width in meters, press **Save field**.
(Every laptop needs this once. A mark dropped on one laptop appears in the right place on the
others because marks travel as GPS positions.)

---

## 3. Game day

Switch on in this order. Each step has a check.

1. **Router** on. Wait a minute.
2. **Cameras** on. *Check:* nothing to see yet; that is fine.
3. **Station radio** plugged into each laptop.
4. Double-click **`Dashboard/Start Dashboard.bat`** on each laptop. Leave the black window
   open. *Check:* the badge at the top shows your station number and color, **Radio ✔**.
5. **Vehicles** on (sensor board and drive board). *Check:* **Vehicle ✔** within a few seconds.
   LED 0 on the ring glows steadily in your station color.
6. Take the vehicles **outside**. *Check:* **GPS ✔** after about 30 seconds, and your vehicle
   appears on the map.
7. *Check:* **Camera ✔** and video. The dashboard finds the camera on the router by itself
   (it can take 15–30 seconds).

Press **🔦 Find my vehicle** to make your vehicle flash its color — handy when six vehicles
look the same.

---

## 4. Using the dashboard

- **Camera** (left): live video from your vehicle.
- **Numbers** (under the video): *Distance ahead* (green = clear, yellow = close, red = very
  close), *Facing* (N, NE, E…), *Speed*, *GPS satellites*, *Radio signal* bars, and how far it
  is *to the target* left/right and up/down, once a grown-up has set a target in Setup.
- **Map** (right): your vehicle is the big arrow in your color. Other stations' vehicles are
  smaller (untick the box under the map to hide them).
  - To **drop a mark**: press 🎯 Target, ⚠️ Danger, 📍 Waypoint or 🤝 Meet here, type a name if
    you like, then tap the map. Every station sees it, with your station number next to it.
  - To **remove** one of your marks: tap it. You can only remove your own station's marks.
- **Chat** (bottom): choose **📢 Everyone** or a station number, then press a quick button
  ("🎯 I found it!", "🆘 Need help!"…) or type and press **Send**. A message to one station
  shows **✔ delivered** once that station's radio confirms it. A faded number means that
  station has not been heard yet.
  - Your vehicle's lights flash white three times when a message or mark arrives.
  - Messages can be 120 letters long (emoji count as more than one).
  - **Anyone nearby can read these messages.** Use station numbers, never names.
  - Every message is saved on the laptop in `Dashboard/data/chat-log/<date>.txt`.
- **⚙ Setup** (for grown-ups): station number, camera address, field map, target spot, a route
  from a spreadsheet (`Dashboard/route-examples/` has two templates), and boards plugged in.

---

## 5. When something does not work

The dashboard shows the most important problem in a yellow or red box at the top. Here is what
each one means.

| You see | Try this |
|---|---|
| **Radio ✖** "The station radio is not connected" | Plug the radio board into the laptop. Close the Arduino Serial Monitor (only one program can use the USB port). Try another USB cable: some only charge. |
| "Which station is this?" keeps coming back | The radio is not saving its number. Upload `StationRadio_v5` again. |
| **Vehicle ✖** | Vehicle switched on? Its LED ring should light up. Blinking orange = no station number (step 2c). LED 0 blinking = it cannot hear the station radio: move closer. Wrong color = it has another station's number. |
| **GPS …** for more than 2 minutes | GPS only works outside with open sky. The FK-A1's blue light starts flashing when it has a fix. |
| Vehicle not on the map, but GPS ✔ | The field map is not set up on this laptop (step 2g). |
| **Camera ✖** | Is the camera powered and set up with this station's number (step 2d)? Is the laptop on the **same WiFi** as the camera? Open ⚙ Setup → Camera → **Search again**. If the router blocks it, type the camera's address (shown in its Serial Monitor) into Setup. |
| Chat says **✖ not delivered** | The other station's radio did not answer. Is that station's dashboard running and its radio plugged in? |
| Marks from another station never appear | Both stations' dashboards must be running. Marks are re-sent every 30 seconds. |
| `Start Dashboard.bat` says Python is not installed | Step 2e. Tick "Add python.exe to PATH" in the installer. |
| A board is in a strange state | Unplug it and plug it back in. Every board is safe to restart at any time. |

The sensor board also prints a self-test when it starts (Serial Monitor, 115200 baud): one PASS
or FAIL line per sensor with a wiring hint, the same as v4.

---

## 6. How it works

### Station numbers and node numbers

| Board | Node number on the radio network |
|---|---|
| Station radio | S (1–9) |
| Vehicle sensor board | 10 + S (11–19) |
| Camera | not on the radio network (WiFi only) |

All boards use network ID **50** and radio channel **1** (`FTT_NET_ID`, `FTT_CHANNEL` at the
top of the radio and sensor sketches). Two games side by side would use different network IDs.

### What goes over the radio

v5 uses [PorpoiseNet](../../../shared/PorpoiseNet/README.md). Every message has a 20-byte header
(who sent it, who it is for, a sequence number, how many hops it may still take). Four message
types were added for v5; nothing older was renumbered or changed.

| Type | Payload | Bytes | Sent | Confirmed? |
|---|---|---|---|---|
| `PN_TELEMETRY_FULL` (11) | health bits, satellites, battery, range, temperature, pressure, BMP altitude, latitude, longitude, speed, GPS altitude, course, compass heading | 52 | by each vehicle, every second, to everyone | no (the next one is a second away) |
| `PN_CHAT` (9) | the typed text, UTF-8, up to 120 bytes plus an end marker; only the typed bytes are sent | ≤ 121 | when someone presses Send | only when sent to **one** station, by that station |
| `PN_MARK` (10) | add/remove, kind, mark id, latitude, longitude, 24-letter label | 45 | when a mark is dropped or removed, and every 30 s | no, but re-sent every 30 s |
| `PN_MARK_LIST` (12) | how many, then up to 40 mark ids | ≤ 82 | every 30 s | no |

Plus two commands: **SYNC** ("I just started, send me your marks") and **NOTIFY** (the station
radio tells its own vehicle to flash).

A mark belongs to the station that made it and is named by *(station, mark id)*. The mark list
is how a station that missed a "remove" still drops the mark: if it is not in the owner's
list, it is gone.

### What goes over the USB cable

Board → laptop: one JSON object per line, always with a `"type"`:

```json
{"type":"hello","board":"station-radio","fw":"5.0","station":3,"node":3,"net":50,"ch":1,"mac":"..."}
{"type":"telemetry","station":2,"node":12,"seq":88,"sonar_ok":1,"distance_in":41.5,"heading_deg":271.0,"course_deg":265.2,"lat":32.7158,"lon":-117.1610,"rssi":-61,"relayed":false, ...}
{"type":"chat","id":"2-91","from":2,"to":0,"text":"Found it by the goal!"}
{"type":"mark","from":2,"action":"add","id":4,"kind":2,"lat":32.7159,"lon":-117.1612,"label":"Puddle"}
{"type":"marklist","from":2,"ids":[1,4]}
{"type":"sent","ref":12,"state":"confirmed"}
```

Lines starting with `# ` are for people (they appear in ⚙ Setup → Messages from the radio board).

Laptop → board: plain words, the same ones you can type into the Serial Monitor yourself:

```
chat 0 12 hello everyone        (0 = everyone; 12 = a number the dashboard uses to match the reply)
chat 2 13 can you help?         (to station 2 only; the radio answers "sent ... confirmed" or "failed")
mark add 4 2 32.7159 -117.1612 Puddle     (kinds: 1 target, 2 danger, 3 waypoint, 4 meet here)
mark remove 4
marklist 1,4
sync    identify    roster    info    station 3    help
```

### Who keeps what

- **The dashboard program** (`start_dashboard.py`) keeps your station's marks in
  `data/marks.json`, re-sends them every 30 seconds, and saves the chat log. If the laptop
  restarts, your marks come back. A station that joins late gets everyone's marks within
  30 seconds, or straight away, because its radio asks (SYNC) when it starts.
- **The boards** keep only their station number (and the camera its WiFi list). They hold no
  game state, so restarting any board is always safe.
- Chat history is not shared: a laptop that starts late sees only messages from then on.

### Limits

- **9 stations** is the most the numbering allows (radio S, vehicle 10+S).
- **The radio** holds 20 peers; 9 stations use 17 per board. Above that, boards fall back to
  broadcasting, which still works but uses more airtime.
- **Airtime:** each vehicle reading is repeated by one or two other boards, not all of them
  (PorpoiseNet's relay suppression). Nine stations use roughly 3–5 % of the channel.
- **The router** is usually the real limit: every camera sends a video stream through it. The
  cameras start at 320×240 to keep six or more streams smooth. Raise the resolution on the
  camera's own page (`http://<camera address>/`) only if the router keeps up.
- One chat message is at most 120 bytes; one station can have 40 marks.

### Files

| Path | What |
|---|---|
| `StationRadio_v5/` | The laptop's radio board. |
| `VehicleSensors_v5/` | The vehicle's sensor board (ported from `ESPNow_Sender_v4`). |
| `VehicleCamera_v5/` | The camera. `app_httpd.cpp` is Espressif's stock file plus the `/whoami` page. |
| `*/PorpoiseNet.h` | Copies of `shared/PorpoiseNet/src/PorpoiseNet.h`. After changing that file, copy it here too; `shared/PorpoiseNet/test/run_tests.sh` checks that they match. |
| `Dashboard/start_dashboard.py` | The dashboard program. `--simulate N` = practice mode. |
| `Dashboard/dashboard.html` | The page. Nothing in it comes from the internet. |
| `Dashboard/test_dashboard.py` | `python -m unittest test_dashboard -v` |
| `Dashboard/data/` | This laptop's settings, marks and chat logs. Not on GitHub. |

---

## 7. Field test checklist

The programs compile and the network and dashboard logic are tested on a computer, but **v5 has
not been run on real boards yet**. Before the first class, go through this with two stations:

- [ ] Each sensor board shows its station number and color at power-up; the self-test passes.
- [ ] Dashboard asks for the station number once, and not again after a restart.
- [ ] Vehicle ✔ on both laptops; each laptop's numbers match its own vehicle.
- [ ] Outside: GPS ✔, both vehicles on both maps, in the right place after field setup.
- [ ] Both cameras found by themselves (Camera ✔) and video plays on the right laptop.
- [ ] Camera setup page works from a phone; a camera with the router off opens it again.
- [ ] Chat to everyone arrives on the other laptop; chat to one station shows ✔ delivered.
- [ ] Chat to a station whose radio is unplugged shows ✖ not delivered after about 2 seconds.
- [ ] The vehicle flashes white when a chat arrives; 🔦 Find my vehicle flashes its color.
- [ ] A mark appears on the other laptop; removing it removes it there too.
- [ ] Restart one dashboard: its marks come back on both laptops within 30 seconds.
- [ ] Walk a vehicle to the far end of the field: does it stay connected (LED 0 steady)?
- [ ] With all stations running, the video stays smooth on every laptop.
- [ ] The heading on the dashboard matches a phone compass (else set `HEADING_OFFSET_DEG`).
