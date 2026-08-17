# Ground Vehicle

> An unmanned ground vehicle platform — a robot that drives on land.

**Status: working code, undocumented.** Two sets of programs are in `src/` — the Pathfinder
vehicle firmware and the "Find the Target" telemetry mission. They came in as a folder of files
exported from OneDrive and have now been sorted into place. Everything marked TBD is still
waiting on the team.

---

## What this project is

An unmanned ground vehicle (UGV) — a rover. It is one of three vehicle platforms in this
repository, alongside the [submersible](../submersible-vehicle/README.md) and the
[security camera system](../homeland-security-camera/README.md).

Porpoise Robotics teaches STEM and AI through unmanned vehicles, and this is the land-based one.
A ground platform is often the easiest place for students to start: you can watch it, you can
reach it when it gets stuck, and a mistake means a bumped wall rather than a flooded electronics
bay.

The specifics are **TBD**. The team should agree on and record here:

- **Purpose** — teaching platform, competition entry, research testbed, or several of those. TBD.
- **Audience** — which students, at what level, in which course. TBD.
- **Autonomy target** — remote-controlled, assisted, or fully autonomous. TBD.
- **Environment** — indoor floors, outdoor pavement, or rough terrain. This drives nearly every
  hardware decision. TBD.

---

## Where code goes

```
ground-vehicle/
├── README.md      this file
├── src/           source code — motor control, sensors, navigation, autonomy
├── docs/          design notes, architecture, decisions
└── hardware/      chassis, motors, wiring diagrams, bill of materials
```

`hardware/` is still empty and holds a `.gitkeep` file so git preserves it —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md). Delete the `.gitkeep`
once that folder has real files in it.

### What is in `src/` today

```
src/
├── pathfinder/         firmware for the Pathfinder vehicle itself
│   ├── Pathfinder_Op_Program11.2/        current operating program — gamepad
│   │                                     driving over Bluepad32, motor ramping,
│   │                                     WS2812B lighting, INA219 current sensing
│   ├── PRPV_System_Test24_Gen3_Bluepad32/  Gen 3 system test, Bluepad32 gamepad
│   ├── Pathfinder_System_Test24_WORKING/   older system test, PS3 controller
│   └── CameraWebServerKiln/              ESP32 camera web server for the vehicle
└── find-the-target/    the "Find the Target" telemetry mission
    ├── README.md                 start here — wiring, bring-up order, calibration
    ├── ESPNow_Sender_v3/         vehicle ESP32: sensors, GPS, NeoPixels
    ├── ESPNow_Receiver_v3/       ESP32 on the base-station computer's USB port
    ├── CameraWebServer_v3/       Freenove ESP32-WROVER camera
    └── BaseStation/              Python server + browser dashboard
```

Each folder ending in a sketch name is an **Arduino sketch folder**: the folder name and the
`.ino` file inside it must stay identical, or the Arduino IDE will not open it. If you rename
one, rename both.

**TBD:** nobody has yet written down how `pathfinder/` and `find-the-target/` relate — whether
Find the Target runs on the Pathfinder vehicle or on a different chassis. Record the answer here.

### What is in `docs/` today

- `find-the-target-setup-guide-v2.pdf` — the printed setup guide. Note it is **v2** while the
  code in `src/find-the-target/` is v3, so expect it to disagree with the code in places.
- `find-the-target-mission-07-stem.pptx` — the STEM mission slide deck.

## Hardware

**TBD.** No chassis, motors, motor controllers, batteries, compute board, or sensors have been
selected.

When decisions are made, record in `hardware/`: the bill of materials with part numbers and
suppliers, wiring diagrams, power budget, and mechanical drawings. Include *why* a part was
chosen — the next person will need the reasoning as much as the part number.

## Software

What is actually here today: **Arduino C++ on ESP32** boards, plus a small **Python 3** base
station using only `pyserial` and the standard library, with a plain HTML/JavaScript dashboard.
No ROS2, no OpenCV, no build system — sketches are opened and flashed from the Arduino IDE.

Porpoise Robotics publicly describes using Python, ROS2, OpenCV, and TensorFlow across its
platforms, so whether this project eventually moves to ROS2 is still an open decision — ROS2 is
powerful and industry-standard but adds real learning overhead for beginners. Document the
choice and the reasoning in `docs/` once made.

## Getting started

**Find the Target** has real instructions: read
[`src/find-the-target/README.md`](src/find-the-target/README.md). It covers which board each
sketch goes on, the order to flash them in, and how to calibrate the field map.

**Pathfinder** has no setup guide yet. `src/pathfinder/Pathfinder_Op_Program11.2/` is the
current operating program; its header comment lists the hardware, the gamepad control mapping,
and the Arduino board-manager URLs needed for Bluepad32. Someone who has flashed it should turn
that header into a proper getting-started section here.

Before flashing anything with WiFi, read the secrets rule below.

## A note on WiFi credentials

`src/pathfinder/CameraWebServerKiln/CameraWebServerKiln.ino` and
`src/find-the-target/CameraWebServer_v3/CameraWebServer_v3.ino` both need a real WiFi name and
password typed into the file before you flash the board. **This repository is public.** Type
them in on your own computer, flash, then put the placeholders back before you commit. See
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-4-never-commit-secrets).

The base station's `BaseStation/config.json` holds the camera's IP address and the field's real
GPS coordinates. It is created automatically when you run `base_station.py` and is listed in
`.gitignore`, so it will not be committed. Leave it that way.

## Safety

Fill in before anyone drives this. A ground vehicle with motors can pinch fingers, run over
cables, and hurt someone if it is heavy or fast enough. At minimum, decide and document:

- How to cut power immediately (physical emergency stop, not a software one)
- What happens on loss of radio or network link — it should stop, not keep going
- Battery handling rules, especially for lithium chemistries: charging, storage, damage response
- Who may operate it, and under what supervision

## Open questions

- Purpose and audience (see above)
- Indoor or outdoor — determines the whole hardware direction
- ROS2 or a simpler custom stack
- Level of autonomy for the first version
- Whether this shares code with the submersible platform, and where that shared code lives
