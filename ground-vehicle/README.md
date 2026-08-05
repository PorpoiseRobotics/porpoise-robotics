# Ground Vehicle

> An unmanned ground vehicle platform — a robot that drives on land.

**Status: scaffold only.** No code has been written yet. Everything marked TBD is waiting on the
team.

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

Those folders are empty. Each holds a `.gitkeep` file so git preserves the empty folder —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md).

Delete the `.gitkeep` once a folder has real files in it.

Suggested structure inside `src/` once code exists — adjust freely, this is a starting point:

- Motor and drive control
- Sensor input
- Teleoperation and communications
- Navigation and autonomy
- Shared utilities and configuration

## Hardware

**TBD.** No chassis, motors, motor controllers, batteries, compute board, or sensors have been
selected.

When decisions are made, record in `hardware/`: the bill of materials with part numbers and
suppliers, wiring diagrams, power budget, and mechanical drawings. Include *why* a part was
chosen — the next person will need the reasoning as much as the part number.

## Software

**TBD.** Porpoise Robotics publicly describes using Python, ROS2, OpenCV, and TensorFlow across
its platforms. Whether this project uses ROS2 or something simpler is an open decision — ROS2 is
powerful and industry-standard but adds real learning overhead for beginners.

Document the choice and the reasoning in `docs/` once made.

## Getting started

Nothing to run yet. Once code exists, this section should get a new contributor from a fresh
clone to a working setup in under fifteen minutes.

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
