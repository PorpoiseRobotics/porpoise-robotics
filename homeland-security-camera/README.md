# Homeland Security Camera

> A security camera system built for deployment at a school.

**Status: first code landed.** `src/` now holds a working two-way radio layer — a camera node
and a perimeter sensor node that talk to each other over ESP-NOW. Everything about *policy* that
is marked TBD is still waiting on the team, and none of it is answered by code.

---

## Read this before contributing

This project is different from the other two in one important way: **it is a camera system that
will operate on a school campus.** That means details about how it is configured and where it is
installed are sensitive in a way that rover firmware is not.

This repository is public. Anyone on the internet can read it, including its full history.

**Never commit any of the following:**

- Camera passwords, admin credentials, stream URLs with embedded logins, or API keys
- IP addresses, hostnames, Wi-Fi network names, port numbers, router or VPN configuration
- Floor plans, campus maps, or anything describing where cameras are physically mounted
- Camera coverage diagrams or blind-spot analysis
- Any recorded footage, still frames containing identifiable people, or student information
- The name or address of a specific partner school, unless the organization has said in writing
  that this is fine

**What is fine to commit:** source code, algorithms, generic architecture diagrams, hardware
notes written without site-specific details, and configuration files whose real values have been
replaced with placeholders.

Configuration follows the `.env` pattern: real values go in a `.env` file that `.gitignore`
blocks from ever being committed, and a `.env.example` with the same keys and empty values gets
committed so others know what settings exist. Details in
[CONTRIBUTING.md](../CONTRIBUTING.md#rule-4-never-commit-secrets).

If something is already committed that should not be, do not quietly delete it — deleting does
not remove it from history. Read
[the recovery guide](../docs/04-when-things-go-wrong.md#i-pushed-something-i-shouldnt-have).

---

## What this project is

A camera-based monitoring system intended for a school environment. It fits Porpoise Robotics'
broader focus on computer vision and AI on unmanned platforms — the organization's public
material describes computer vision as one of the capability areas of its Prometheus AI system.

Beyond that, the specifics are **TBD**. Before writing code, the team should agree on and record
here:

- **Purpose and scope** — what the system is meant to detect, alert on, or record, and what it
  explicitly is not for. TBD.
- **Who operates it** — school staff, the robotics team, or both. TBD.
- **Deployment model** — fixed cameras, mobile platform, or both. TBD.

### Questions to settle before writing code

Because this involves cameras in a place where minors are present, these are not optional:

- Who owns and who can access the footage? How long is it kept?
- What consent, notice, or approval does the school require? What does district policy say?
- What legal requirements apply to recording minors in this jurisdiction?
- What happens if the system misidentifies someone?

**Get answers from the school and from Porpoise Robotics leadership in writing before
deployment.** Record the decisions in `docs/` — with site-specific details left out of this
public repository.

---

## Where code goes

```
homeland-security-camera/
├── README.md      this file
├── src/
│   ├── PorpoiseNet_CameraNode/   the camera: hears an alert, takes a picture
│   └── PorpoiseNet_Sensor/       a fixed PIR + door-switch node
├── docs/          design notes, architecture, decisions
└── hardware/      camera specs, mounting notes, bill of materials
```

`docs/` and `hardware/` are still empty and hold a `.gitkeep` file so git preserves them —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md). Delete the `.gitkeep`
once a folder has real files in it.

### What is in `src/` today

Both sketches are nodes on **[PorpoiseNet](../shared/PorpoiseNet/)**, the shared two-way ESP-NOW
layer that the ground vehicle's Find the Target mission also runs. Read that README first — it
covers how the boards find each other, the three settings that must match, and the channel trap
that breaks a camera the moment it joins a WiFi router.

- **`PorpoiseNet_Sensor/`** — a plain ESP32 with a PIR on one pin and a reed switch on another.
  When either trips it broadcasts one alert. It also *listens*, which is how the base station can
  arm and disarm it.
- **`PorpoiseNet_CameraNode/`** — an ESP32-CAM. It hears an alert from any sensor within range,
  takes a picture within roughly a quarter of a second, writes it to an SD card and optionally
  uploads it over ordinary WiFi, then broadcasts a receipt saying what it saved. The sensor that
  tripped hears that receipt, so it knows its alert was acted on.

No router, no server and no cabling are needed between the sensor and the camera.

### The one architectural rule

**Images never travel over ESP-NOW.** A frame holds 250 bytes; a small JPEG is fifteen thousand.
What goes over the radio is the *event* ("zone 3, motion") and the *receipt* ("saved
`n21_000007.jpg`, 18 kB"). The image goes to an SD card or over real WiFi to a server you
control. Small facts on the mesh, big data on real infrastructure.

That is also a privacy property, not only a bandwidth one: nothing recognisable is ever broadcast
over an unencrypted link.

### What the code does and does not protect

ESP-NOW messages are **not encrypted**. Anyone nearby with a cheap board can read every alert and
can forge one. Two things the sketches do about it:

- alerts carry a zone **number**, never a room name or a person's name;
- no message is allowed to unlock, disarm, or disable anything physical.

Those help. **They are not consent, and they are not a security review.** The questions in this
README still have to be answered by people, in writing, before anything is mounted on a wall.

## Hardware

**Not formally selected.** The code targets an **ESP32-CAM (AI-Thinker)** or the Freenove
ESP32-WROVER kit for the camera node, and any plain ESP32 dev board with an HC-SR501 PIR and a
reed switch for the sensor nodes, because that is what a school budget reaches and what the team
already owns. Change `CAMERA_MODEL` at the top of the camera sketch to switch boards.

Two hardware notes worth knowing before ordering anything:

- The AI-Thinker board's GPIO is nearly all spoken for by the camera. The pin budget, and which of
  the leftovers are safe, is written out at the top of `PorpoiseNet_CameraNode.ino`. If you need
  more pins than that, add a second five-dollar ESP32 as a sensor node rather than overloading the
  camera.
- The camera pulls a lot of current the instant it powers up. A weak USB port causes a brownout
  that looks exactly like a broken camera.

Record the real bill of materials in `hardware/` once decided — with model numbers and specs, but
without anything describing installation locations.

## Software

What is actually here today: **Arduino C++ on ESP32**, sharing
[`shared/PorpoiseNet/`](../shared/PorpoiseNet/) with the ground-vehicle project. No build system —
sketches are opened and flashed from the Arduino IDE.

Everything above the radio layer is still **TBD**. The organization publicly describes using
Python, OpenCV, TensorFlow, and ROS2 across its platforms, which is a reasonable starting point
for discussion about where recorded images actually go and what, if anything, looks at them.

Document the choice in `docs/` when it is made, including why — future contributors will want the
reasoning, not just the conclusion.

## Getting started

1. Read [`shared/PorpoiseNet/README.md`](../shared/PorpoiseNet/README.md) — especially the three
   settings that must match on every board.
2. Install PorpoiseNet: copy `shared/PorpoiseNet/` into `Documents/Arduino/libraries/` and restart
   the Arduino IDE.
3. Flash `PorpoiseNet_Sensor/` to a plain ESP32 and open its serial monitor at 115200. Wave at the
   PIR; it prints an alert.
4. Flash `PorpoiseNet_CameraNode/` to an ESP32-CAM. Wave at the PIR again — the camera captures,
   and both boards print what happened.
5. For an operator's view, flash `PorpoiseNet_Base/` from the ground-vehicle project onto a third
   board. The same base station serves both projects.

To check the network logic without any hardware at all:
`cd ../shared/PorpoiseNet/test && ./run_tests.sh`

## Open questions

- **Privacy, consent, and data retention policy — blocking, must be answered first.** No amount of
  code touches this one.
- Scope: what does the system actually need to do? (see above)
- Where captured images go. The camera can write to SD and can POST over WiFi, but nobody has
  decided what receives them, who may read that, or how long anything is kept.
- Where processing happens: on-device or on a server. Nothing analyses an image today.
- How and to whom alerts are delivered beyond the base station's serial output.
- Whether sensor nodes run on mains or batteries. As written they listen continuously so they can
  be armed and disarmed remotely, which costs battery life; deep sleep would buy months but gives
  up the remote control. The trade-off is spelled out in `PorpoiseNet_Sensor.ino`.
