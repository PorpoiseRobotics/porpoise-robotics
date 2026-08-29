# Ground Vehicle

> An unmanned ground vehicle platform — a robot that drives on land.

**Status: code has landed, documentation has not.** The Pathfinder operating programs now live in
[`src/`](src/). Everything marked TBD below is still waiting on the team.

> **Naming warning.** The vehicle in this folder is the Pathfinder *rover*. Porpoise Robotics also
> publicly lists a "Pathfinder ROV", which is a submersible. Same name, different vehicle — check
> which one somebody means before acting on it.

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
├── src/           source code — the Pathfinder operating programs
│   ├── pathfinder_ps3/               beginner program, PS3 controller
│   ├── pathfinder_nintendoswitch/    beginner program, Switch controller
│   ├── pathfinder_find_controller/   setup tool: reads a controller's Bluetooth address
│   ├── Pathfinder_Op_Program12/      advanced program
│   ├── lessons/                      small follow-along sketches for the courses
│   └── legacy/                       superseded programs, kept for reference
├── docs/          design notes, architecture, decisions
│   └── courses/                      the lesson slides
└── hardware/      chassis, motors, wiring diagrams, bill of materials
```

`hardware/` is still empty. It holds a `.gitkeep` file so git preserves the empty folder —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md). Delete the `.gitkeep`
once it has real files in it.

### Which program to open

| If you are | Use |
|---|---|
| New to robotics, driving with a PS3 controller | `pathfinder_ps3` |
| New to robotics, driving with a Nintendo Switch controller | `pathfinder_nintendoswitch` |
| Setting up a Switch controller for a vehicle | `pathfinder_find_controller`, once |
| Experienced, and want current sensing, a self-test, and four servos | `Pathfinder_Op_Program12` |

The two beginner programs are deliberately near-identical, so a student who has read one can read
the other. Everything in [`src/legacy/`](src/legacy/) has been replaced — see the README there
before building any of it.

Each program's own header comment lists the board package and libraries it needs. The board
packages are **mutually exclusive** in the Arduino IDE, so check before switching between
programs.

## Teaching with these

Three courses, five three-hour lessons each, one per operating program.

| | Slides | Follow-along sketches |
|---|---|---|
| Beginner, PS3 | [`docs/courses/beginner-ps3/`](docs/courses/beginner-ps3/) | [`src/lessons/beginner_ps3/`](src/lessons/beginner_ps3/) |
| Beginner, Switch | [`docs/courses/beginner-switch/`](docs/courses/beginner-switch/) | [`src/lessons/beginner_switch/`](src/lessons/beginner_switch/) |
| Advanced | [`docs/courses/advanced/`](docs/courses/advanced/) | [`src/lessons/advanced/`](src/lessons/advanced/) |

The slides are editable PowerPoint files. The sketches are small programs students upload during
the lesson — one idea each, heavily commented, each ending with things to change and see what
happens. Every sketch uses the same Arduino IDE settings as the full program for its track.

Start at [`docs/courses/README.md`](docs/courses/README.md).

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
