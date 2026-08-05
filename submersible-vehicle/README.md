# Submersible Vehicle

> An underwater vehicle platform.

**Status: scaffold only.** No code has been written yet. Everything marked TBD is waiting on the
team.

---

## What this project is

An underwater unmanned vehicle. This sits closest to the heart of what Porpoise Robotics does —
the organization is named after a marine mammal, its public site features two ROV platforms,
**Explorer ROV** and **Pathfinder ROV**, and its president's background is described as being in
deep-diving submersible electronic systems.

**First thing to settle: which vehicle is this?** Is this folder for Explorer, for Pathfinder,
for both, or for something new? Record the answer here — it will confuse every future contributor
otherwise. TBD.

Also TBD:

- **Type** — an ROV (tethered, remotely operated) or an AUV (untethered, autonomous). These are
  substantially different engineering problems.
- **Depth rating** — drives the pressure housing design and most of the hardware budget.
- **Purpose** — teaching platform, competition entry, or research tool.
- **Audience** — which students, at what level.

---

## Where code goes

```
submersible-vehicle/
├── README.md      this file
├── src/           source code — thruster control, sensors, camera, telemetry
├── docs/          design notes, architecture, decisions
└── hardware/      pressure housing, thrusters, seals, tether, bill of materials
```

Those folders are empty. Each holds a `.gitkeep` file so git preserves the empty folder —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md).

Delete the `.gitkeep` once a folder has real files in it.

Suggested structure inside `src/` once code exists — adjust freely:

- Thruster and motor control
- Sensors: depth, pressure, orientation, leak detection
- Camera and video streaming
- Surface communication over the tether
- Pilot control and telemetry
- Shared utilities and configuration

## Hardware

**TBD.** No thrusters, pressure housing, seals, tether, compute board, or sensors have been
selected.

Underwater hardware is unforgiving, so `hardware/` matters more here than on the other projects.
When decisions are made, record the bill of materials, the pressure housing design and its
depth rating, seal and O-ring specifications with replacement intervals, tether specification,
wiring diagrams, and buoyancy and ballast calculations.

Water ingress destroys electronics in seconds and there is no partial credit. Write down what
worked so nobody has to rediscover it after a flood.

## Software

**TBD.** Porpoise Robotics publicly describes using Python, ROS2, OpenCV, and TensorFlow across
its platforms. Nothing has been decided for this project.

Consider from the start how a pilot at the surface controls the vehicle and sees its video, since
that shapes the whole architecture. Document the decision in `docs/`.

## Getting started

Nothing to run yet. Once code exists, this section should get a new contributor from a fresh
clone to a working setup in under fifteen minutes — including how to test without putting the
vehicle in water.

## Safety

Fill in before this goes near water. Combining electricity, water, and people needs written
procedure, not improvisation. At minimum, decide and document:

- Pre-dive checklist — seals, leak detection, battery charge, tether condition, comms check
- Leak response: what the vehicle does and what the operators do
- Battery handling, especially lithium chemistries, and what to do with a battery that got wet
- Tether management, so nobody gets tangled and nothing gets cut
- Who may operate it, in what water, with what supervision, and never alone
- Recovery plan for a vehicle that loses power or comms while submerged

## Open questions

- Which vehicle this is: Explorer, Pathfinder, or new (see above)
- ROV or AUV
- Target depth rating
- Tether design: power, data, or both
- Whether this shares code with the ground vehicle, and where that shared code lives
