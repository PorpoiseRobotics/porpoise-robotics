# Find the Target

> A STEM mission activity: a camera-equipped vehicle hunts for targets while a base station
> tracks it.

---

## What this project is

A classroom mission rather than a vehicle platform. It combines three pieces of hardware and a
laptop:

- **CameraWebServer** — an ESP32-CAM streaming video over WiFi.
- **ESPNow_Sender / ESPNow_Receiver** — a pair of ESP32 boards passing data over ESP-NOW, which
  is a low-latency radio protocol that does not need a WiFi network.
- **BaseStation** — a Python program with an HTML dashboard, run on a laptop, that reads waypoints
  from a spreadsheet and shows what the vehicle is doing.

The teaching material for the activity lives in [`docs/`](docs/): the slide deck and the setup
guide students follow.

## Layout

```
find-the-target/
├── README.md
├── src/
│   ├── FindTheTarget_v4/    current version
│   ├── PorpoiseNet_Base/    fleet base station (PorpoiseNet radio)
│   ├── PorpoiseNet_Vehicle/ rover for the multi-rover game
│   └── legacy/              superseded versions, kept for reference
├── docs/                    slides, setup guide, design notes
└── hardware/                wiring, bill of materials
```

Work on **v4**. Version 3 is kept in `src/legacy/` only so old builds can be looked up; do not
start anything new from it. See [`src/legacy/README.md`](src/legacy/README.md).

### Which sketches do I flash?

There are two radio setups here, and they are not interchangeable. Pick by how many rovers you
are running.

| | Use when | How the radio works |
|---|---|---|
| **`FindTheTarget_v4/`** | One vehicle, one base, and you want the existing `base_station.py` dashboard. | The sender needs the receiver's MAC address pasted into it, so it only ever talks to one board. |
| **`PorpoiseNet_Base/` + `PorpoiseNet_Vehicle/`** | The actual game, with several rovers on the field at once. | [PorpoiseNet](../shared/PorpoiseNet/README.md) broadcasts, so adding a fourth rover mid-session needs no re-flashing of anything else. Finds are acknowledged and retried, and a rover in the middle of the field relays for one at the far end. |

The JSON telemetry keys are the same either way, so `base_station.py` and `dashboard.html` keep
working unchanged.

`PorpoiseNet_Base` reads its WiFi settings from a `secrets.h` that `.gitignore` blocks. Copy the
`secrets.example.h` next to it, fill in your own values, and never commit the result.

## Status

Moved here from `dump/` during a repository tidy-up. The code arrived as-is and has not been
reviewed or documented beyond the README that came with each version. Open questions worth
recording here as they are answered:

- Which vehicle the mission runs on — the [ground vehicle](../ground-vehicle/README.md), or
  something else.
- Whether the base station is expected to run on a school laptop, and what has to be installed
  on it first.
- What the WiFi and ESP-NOW setup assumes about the network it is used on.

## Safety and privacy

This project streams video from a camera. Before running it anywhere students are present,
settle who may view the stream, whether anything is recorded, and where recordings go. The
[homeland security camera project](../homeland-security-camera/README.md) has stricter rules for
the same reason — read those before pointing a camera at a campus.
