# Homeland Security Camera

> A security camera system built for deployment at a school.

**Status: first code landed, nothing deployed.** Two sketches now live in [`src/`](src/) — a
sensor node and a camera node, both talking over
[PorpoiseNet](../shared/PorpoiseNet/README.md). Neither has been flashed to a board yet, and the
privacy questions below still block any deployment. Everything marked TBD is waiting on the team.

### What is in `src/`

| Sketch | What it does |
|---|---|
| [`PorpoiseNet_Sensor/`](src/PorpoiseNet_Sensor/) | PIR motion sensor and door switch. Broadcasts one alert when tripped, and listens so the base can arm and disarm it. |
| [`PorpoiseNet_CameraNode/`](src/PorpoiseNet_CameraNode/) | Hears an alert, captures a frame, writes it to SD, optionally uploads it over WiFi, then broadcasts a receipt saying what it saved. |

**Images never travel over the radio.** An ESP-NOW frame holds 250 bytes; a small JPEG is around
fifteen thousand. The radio carries the *event* ("zone 3, motion") and the *receipt* ("saved
`n21_000007.jpg`, 18 kB"). The picture goes to the SD card, or to a server over real WiFi. That
split is deliberate, and it is also a privacy property: nothing recognisable is ever broadcast
over an unencrypted link.

Both sketches read their WiFi settings from a `secrets.h` that `.gitignore` blocks. Copy the
`secrets.example.h` sitting next to the sketch, fill in your own values, and never commit the
result.

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
├── src/           source code
├── docs/          design notes, architecture, decisions
└── hardware/      camera specs, mounting notes, bill of materials
```

Those folders are empty. Each holds a `.gitkeep` file so git preserves the empty folder —
[here is why that is needed](../docs/05-gitkeep-and-empty-folders.md).

Delete the `.gitkeep` once a folder has real files in it.

## Hardware

**TBD.** No cameras, compute boards, or mounting hardware have been selected. Record the bill of
materials in `hardware/` once decided — with model numbers and specs, but without anything
describing installation locations.

## Software

**TBD.** The organization publicly describes using Python, OpenCV, TensorFlow, and ROS2 across
its platforms, which is a reasonable starting point for discussion, but nothing has been decided
for this project.

Document the choice in `docs/` when it is made, including why — future contributors will want the
reasoning, not just the conclusion.

## Getting started

Nothing to run yet. Once code exists, this section should give a new contributor a working setup
in under fifteen minutes: prerequisites, install steps, and one command that proves it works.

## Open questions

- Scope: what does the system actually need to do? (see above)
- Privacy, consent, and data retention policy — blocking, must be answered first
- Hardware selection
- Where processing happens: on-device or on a server
- How and to whom alerts are delivered
